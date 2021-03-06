/**
	Project Loader
	  project_loader.c

	Loading a Scratch project is broken up into three parts:

	  * reading the project.json and resources like costumes into memory
	  * parsing the project.json and organizing resources and what was parsed into sprites
	  * loading the organized resources and scripts into the peripherals and runtime

	This module handles the last two parts. The first part is handled by a separate module:
	zip_loader.c.

	No resources should be left over from loading. I've even been thinking about shoving all
	of the modules for loading into a dynamicly loaded library, so that even the code
	doesn't stick around.

	The data that makes up a project can be organized into sprites, and the project.json is
	also organized this way. For this reason, the parser focuses on a sprite at a time and
	has a global `sprite` variable. The runtime focuses heavily on the scripts/threads in
	each sprite though, and needs all scripts in the project organized by hat type. To
	satisfy the runtime, global variables for each hat type that store references to scripts
	are used.

	This module performs the second and third parts of the loading process. It parses the
	project.json into sprites and then loads data that was parsed into the runtime and
	peripherals. The third part's implementation is straight forward.

	Parsing is done by going though the project.json sequentially, and it only operates on
	one sprite at a time. Data is parsed into either permanent memory locations or into
	temporary, expandable data structures. Data in temporary data structures is "finalized"
	or "extracted" when no more data is going to be put into it.

	There are currently a few exceptions to the way parsing is done, but it works. For
	instance, when parsing scripts, the space needed is precalculated by breaking the
	sequential order.
**/

#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>
#include <SDL2/SDL_opengl.h>

#include <cmph.h>
#include "ut/uthash.h"

#include "types/primitives.h"

#include "jsmn/jsmn.h"
#include "ut/utarray.h"
#include "ut/dynarray.h"

#include "types/value.h"
#include "types/variables.h"
#include "thread.h"
#include "types/block.h"
#include "types/sprite.h"

#include "types/costume.h"

#include "zip_loader.h"

#include "value.h"
#include "variables.h"
#include "runtime.h"

static char *json;
static jsmntok_t *tokens;
static unsigned pos;

static struct Resource *resources;
//static struct Resource *sounds;

static void tokenizeJson(const size_t jsonLength) {
#define ERROR(s) {puts("[FATAL]"s); return;}
	jsmn_parser parser;

	jsmn_init(&parser);
	int nTokens = jsmn_parse(&parser, json, jsonLength, NULL, 0);
	if(nTokens == JSMN_ERROR_INVAL)
		ERROR("JSMN_ERROR_INVAL")
	else if(nTokens == JSMN_ERROR_PART)
		ERROR("JSMN_ERROR_PART")
	tokens = malloc(nTokens*sizeof(jsmntok_t));

	jsmn_init(&parser);
	nTokens = jsmn_parse(&parser, json, jsonLength, tokens, nTokens);
	if(nTokens == JSMN_ERROR_INVAL)
		ERROR("JSMN_ERROR_INVAL")
	else if(nTokens == JSMN_ERROR_PART)
		ERROR("JSMN_ERROR_PART")
	else if(nTokens == JSMN_ERROR_NOMEM)
		ERROR("JSMN_ERROR_NOMEM")
#undef ERROR
}

static inline cmph_t* loadBlockHashFunc(void) {
	FILE *stream = fopen("blockops.mphf", "r");
	if(stream == NULL) {
		puts("[FATAL]Could not load block hash function");
		return NULL;
	}
	cmph_t *mph = cmph_load(stream);
	fclose(stream);
	return mph;
}

#define TOKC (tokens[pos])
#define gjson(tok) (json+(tok).start)
#define toklen(tok) ((tok).end-(tok).start)
// calculate length of the current token
#define tokclen() (toklen(TOKC))
// check if the current token is equal to the given string.
#define tokceq(str) (strncmp(str, gjson(TOKC), tokclen()) == 0)

static void skip(void) {
	uint32 tokensToSkip = 1;
	do {
		tokensToSkip += TOKC.size; // add child tokens to tokensToSkip
		++pos; // skip token
	} while(--tokensToSkip != 0);
}

static iconv_t charCd;
static dynarray *charBuffer; // dynarray of chars to be reused for temporarily storing strings

/* Parses the string starting at str of length len into charBuffer. */
static void parseString(const char *str, const size_t len) {
	dynarray_clear(charBuffer);
	dynarray_ensure_size(charBuffer, len);
	const char *const end = str+len;
	union {
		char c;
		uint32 w;
	} character;

	char *inbuf, *outbuf;
	size_t inbytesleft, outbytesleft;

	for(; str != end; ++str) {
		character.c = *str;
		if(character.c == '\\') {
			++str;
			character.c = *str;
			switch(character.c) {
			case 'b': character.c = '\b'; break;
			case 'f': character.c = '\f'; break;
			case 'n': character.c = '\n'; break;
			case 'r': character.c = '\r'; break;
			case 't': character.c = '\t'; break;
			case 'u':
				++str;
				inbuf = (char*)str+4; // ugly variable reuse
				character.w = (uint32)strtol(str, &inbuf, 16);
				if(character.w < 128) { // if the codepoint is within the ASCII range
					character.c = (char)character.w;
					str+=3;
					break;
				}
				else {
					dynarray_reserve(charBuffer, 4);

					inbuf = (char*)&character.w; outbuf = charBuffer->d + charBuffer->i;
					inbytesleft = 4; outbytesleft = 4;
					if(iconv(charCd, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == (size_t)-1) {
						puts("[WARNING]Could not convert unicode codepoint to UTF-8");
						switch(errno) {
						case E2BIG: puts("\terrno = E2BIG"); break;
						case EILSEQ: puts("\terrno = EILSEQ"); break;
						case EINVAL: puts("\terrno = EINVAL"); break;
						}
					}

					charBuffer->i += 4 - outbytesleft;
					str += 3;
					continue;
				}
			}
		}
		dynarray_push_back(charBuffer, &character.c);
	}
	dynarray_extend_back(charBuffer); // append a terminator to the string
}

// extract the text pointed to by the current token into dst
#define tokcext(dst) {																									\
		parseString(gjson(TOKC), tokclen());																\
		dynarray_extract(charBuffer, (void**)&dst);													\
	}

static SpriteContext *sprite;

/* Token position should be pointing to the key "variables", just before the array of
	 variables, and it will be left pointing to the token just after the array. */
static void parseVariables(void) {
	++pos; // advance to array

	uint16 propertiesToGo, i = 0, nVariablesToGo = TOKC.size; // number of variable objects to parse
	Variable *variableBuffer = malloc(nVariablesToGo*sizeof(Variable)); // array of variables, and the first element is also the entry point of the hash table of variables TODO: free
	Variable *variables = NULL;

	char *name;
	size_t nameLen;
	Value value;

	do { // for each variable object
		++pos; // advance to variable object
		propertiesToGo = TOKC.size;
		do { // for each property
			++pos; // advance to key
			if(tokceq("name")) {
				++pos; // advance to value
				tokcext(name);
				nameLen = charBuffer->i-1;
			}
			else if(tokceq("value")) {
				++pos; // advance to value
				parseString(gjson(TOKC), tokclen());
				value = strnToValue(charBuffer->d, charBuffer->i-1);
			}
			else // isPersistent
				++pos;
		} while(--propertiesToGo != 0);
		variable_init(&variables, variableBuffer+i, name, nameLen, &value);
		free(name);
		value_free(value);
		++i;
	} while(--nVariablesToGo != 0);
	++pos; // advance from last property
	sprite->variables = variables;
}

/* Token position should be pointing to the key "lists", just before the array of lists,
	 and it will be left pointing to the token just after the array. */
static void parseLists(void) {
	++pos; // advance to array
	uint16 i = 0, nListsToGo = TOKC.size; // number of list objects to parse
	uint32 propertiesToGo;

	List *listBuffer = malloc(nListsToGo*sizeof(List)); // TODO: free
	List *lists = NULL;

	const jsmntok_t *valueToken;
	Value value;

	do { // for each list object
		++pos; // advance to list object
		propertiesToGo = TOKC.size;
		do { // for each property
			++pos; // advance to key
			if(tokceq("listName")) {
				++pos; // advance to value
				parseString(gjson(TOKC), tokclen());
			}
			else if(tokceq("contents")) {
				valueToken = tokens + ++pos;
				pos += TOKC.size;
			}
			else {
				skip();
				--pos;
			}
		} while(--propertiesToGo != 0);
		list_init(&lists, listBuffer+i, charBuffer->d, charBuffer->i-1);
		for(propertiesToGo = valueToken->size; propertiesToGo != 0; --propertiesToGo) {
			++valueToken;
			parseString(gjson(*valueToken), toklen(*valueToken));
			value = strnToValue(charBuffer->d, charBuffer->i-1);
			listAppend(&listBuffer[i].contents, &value);
			value_free(value);
		}
		++i;
	} while(--nListsToGo != 0);
	++pos; // advance from last property
	sprite->lists = lists;
}

/*static void parseCostumes(void) {
	++pos; // advance to array of costumes
	uint32 nCostumesToGo = TOKC.size, i = 0;
	if(nCostumesToGo == 0) { ++pos; return; }
	Costume *const costumes = malloc(nCostumesToGo*sizeof(Costume));

	do {
		++pos;
		ufastest nPropertiesToGo = TOKC.size;
		do {
			++pos; // advance to key
			if(tokceq("costumeName")) {
				++pos; // advance to value
				tokcext(costumes[i].name);
			}
			if(tokceq("baseLayerID")) {
				++pos;
				uint32 id = strtol(gjson(TOKC), NULL, 10);
				costumes[i].texturehandle = resources[i].data.textureHandle;
			}
			else
				skip();
			--pos;
		} while(--nPropertiesToGo != 0);
		++i;
	} while(--nCostumesToGo != 0);
	++pos;
	sprite->costumes = costumes;
	}*/

/* pos should point to first block of script, and will be left pointing after script */
static void allocateScript(Block **const blocks, Value **const values, uint16 nTokensToGo) {
	size_t nBlocks = 0, nValues = 0;
	do {
		nTokensToGo += TOKC.size;
		if(TOKC.type == JSMN_ARRAY) { // if it is a block
			++nBlocks;
			++pos; --nTokensToGo; nTokensToGo += TOKC.size; // skip past opstring, or to first block of stack input
			if(TOKC.type == JSMN_ARRAY) { // if it was just a stack input
				++pos; --nTokensToGo; nTokensToGo += TOKC.size; // skip past opstring, for real this time
			}
		}
		else {
			if(*gjson(TOKC) != 'n') // if it is not just an empty stack input
				++nValues; // count it as a value
		}
		++pos;
	} while(--nTokensToGo != 0);
	const size_t lenOfBlocks = (nBlocks + nValues)*sizeof(Block);
	const size_t len = lenOfBlocks + nValues*sizeof(Value);
	*blocks = malloc(len); // TODO: free
	memset(*blocks, 0, len);
	*values = (Value*)((byte*)*blocks + lenOfBlocks);
}

static cmph_t *blockMphf;

static inline uint32 hash(const char *const key, const size_t keyLen, cmph_t *mphf) {
	return cmph_search(mphf, key, keyLen);
}

static inline uint32 tokchash(cmph_t *mphf) {
	parseString(gjson(TOKC), tokclen());
	return hash(charBuffer->d, dynarray_len(charBuffer) - 1, mphf);
}

#include "blockhash/typestable.c"

static char **procedureParameters; // TODO: switch this to a dynarray
static uint16 nParameters;

/* Parses arguments to a block, using recursion when one of the arguments is another
	 block. pos should point to the first block, and is left pointing at the last token
	 parsed. *blocks is left pointing at the last item made. *values is left pointing after
	 the last item made. */
static enum BlockType parseBlock(Block **const blocks, Value **const values, const ubyte level) {
	Block *block = *blocks;
	Value *value = *values;

	uint16 argsToGo = TOKC.size;
	++pos; // advance to opstring
	const blockhash hash = tokchash(blockMphf);
	const blockfunc func = opsTable[hash];
	const enum BlockType type = blockTypesTable[hash];
	switch(type) {
	case BLOCK_TYPE_C: case BLOCK_TYPE_CF:
		--argsToGo; break;
	case BLOCK_TYPE_E:
		argsToGo -= 2; break;
	default:;
	}

	while(--argsToGo != 0) {
		++pos; // advance to next argument
		if(TOKC.type == JSMN_ARRAY) { // if argument is a block
			++pos; // advance to opstring
			if(!tokceq("getParam")) {
				--pos;
				parseBlock(&block, &value, level+1);
			}
			else { // if it is a procedure parameter
				++pos; // advance to argument (param name)
				parseString(gjson(TOKC), tokclen());
				uint16 i;
				for(i = 0; i < nParameters; ++i) {
					if(strcmp(charBuffer->d, procedureParameters[i]) == 0)
						break;
				}
				if(i == nParameters) {
					puts("[WARNING]Could not match procedure parameter to one of the defined parameters.");
					value->data.integer = 0;
				}
				else
					value->data.integer = i;
				value->type = FLOATING;
				block->func = NULL;
				block->p.value = value;
				block->level = level + 1;
				++value;
				++block;
				block->func = opsTable[cmph_search(blockMphf, "getParam", 8)];
				block->level = level;
				++pos; // advance to second argument
			}
		}
		else {
			if(TOKC.type == JSMN_STRING) { // argument is a string
				value->type = STRING;
				tokcext(value->data.string);
			}
			else { // else argument is a primitive
				*value = strnToValue(gjson(TOKC), tokclen()); // assume characters don't need special parsing
			}
			block->func = NULL;
			block->p.value = value;
			block->level = level;
			++value;
		}
		++block;
	}
	block->func = func;
	block->level = level - 1;
	block->p.next = NULL;

	*blocks = block;
	*values = value;
	return type;
}

/* pos should be pointing to first block of the stack, and will be left pointing after the
	 last token parsed. *blocks and *values will be left pointing after the last items made. */
static void parseStack(Block **const blocks, Value **const values, uint16 nStackBlocksToGo, Block **link) {
	Block *block = *blocks;
	if(nStackBlocksToGo != 0) {
		do {
			if(link != NULL)
				*link = block;

			switch(parseBlock(&block, values, 1)) {

			case BLOCK_TYPE_S: case BLOCK_TYPE_F: // 1 substack: 1 following (normal stack block)
				link = &(block->p.next);
				++block;
				++pos;
				break;

			case BLOCK_TYPE_C: // 2 substacks: 1 inner and 1 following
				block->p.substacks = malloc(2*sizeof(Block*));
				link = block->p.substacks+1;
				++block;
				pos += 2; // advance to first block of substack
				parseStack(&block, values, tokens[pos-1].size, (block-1)->p.substacks+0);
				break;

			case BLOCK_TYPE_CF: // 1 substack: 1 inner and no following
				block->p.substacks = malloc(1*sizeof(Block*));
				link = NULL;
				++block;
				pos += 2;
				parseStack(&block, values, tokens[pos-1].size, (block-1)->p.substacks+0);
				break;

				Block **substacks;
			case BLOCK_TYPE_E: // 3 substacks: 2 inner and 1 following
				substacks = malloc(3*sizeof(Block*));
				block->p.substacks = substacks;
				link = substacks+2;
				++block;
				pos += 2;
				parseStack(&block, values, tokens[pos-1].size, substacks+0);
				++pos;
				parseStack(&block, values, tokens[pos-1].size, substacks+1);
				break;

			default:
				puts("[ERROR]Encountered a non-stacking block where a stacking block was expected.");
				pos -= 2;
				skip();
			}
		} while(--nStackBlocksToGo != 0);
	}
	*link = NULL;
	*blocks = block;
}

// collections of references to threads to load into the runtime
static dynarray *greenFlagThreads; // dynarray of ThreadLink*s
static struct BroadcastThreads *broadcastsHashTable;

// temporary storage for collections of references to threads for each sprite
static dynarray *threads;
enum HatType {
	WHEN_GREEN_FLAG_CLICKED,
	WHEN_I_RECEIVE,
	WHEN_CLONED,
};
static dynarray *threadTypes; // for each ThreadLink in threads, a corresponding HatType is in here for organizing threads by hat typ

static uint16 nWhenClonedThreads;

static dynarray *broadcastTypes; // for each ThreadLink in threads for a WHEN_I_RECIEVE hat type, there is a pointer to a ThreadList in here
static dynarray *broadcastThreadLists; // a pointer to each ThreadList for broadcast threads is stored in here

static struct ProcedureLink *procedureHashTable;

static inline void addBroadcast(void) {
	// get message
	++pos; // advance to message
	char *msg;
	tokcext(msg);
	--pos; // return to opstring

	// create or get global entry in broadcastsHashTable
	struct BroadcastThreads *newBroadcast;
	HASH_FIND_STR(broadcastsHashTable, msg, newBroadcast);
	if(newBroadcast == NULL) { // create the entry
		newBroadcast = malloc(sizeof(struct BroadcastThreads));
		newBroadcast->msg = msg;
		newBroadcast->nullifyOnRestart = NULL;
		HASH_ADD_KEYPTR(hh, broadcastsHashTable, newBroadcast->msg, charBuffer->i-1, newBroadcast);
		newBroadcast->threadList = NULL;
	}

	// create or get the threadList
	if(newBroadcast->threadList != NULL) {
		if(newBroadcast->threadList->array == NULL)
			++newBroadcast->threadList->nThreads;
		else {
			struct ThreadList *threadList = malloc(sizeof(struct ThreadList));
			threadList->array = NULL;
			threadList->nThreads = 1;
			threadList_push(&newBroadcast->threadList, threadList);
			dynarray_push_back(broadcastThreadLists, &threadList);
		}
	}
	else {
		struct ThreadList *threadList = malloc(sizeof(struct ThreadList));
		threadList->array = NULL;
		threadList->nThreads = 1;
		threadList_push(&newBroadcast->threadList, threadList);
		dynarray_push_back(broadcastThreadLists, &threadList);
	}

	dynarray_push_back(broadcastTypes, &newBroadcast->threadList);
}

static inline Block** addProcedure(void) {
	struct ProcedureLink *newProc = malloc(sizeof(struct ProcedureLink));

	++pos; // advance to procedure label
	tokcext(newProc->label);

	++pos; // advance to array of parameter declarations
	nParameters = TOKC.size;
	newProc->nParameters = nParameters;

	HASH_ADD_KEYPTR(hh, procedureHashTable, newProc->label, charBuffer->i-1, newProc);

	if(nParameters != 0) { // store names of parameter names, in order
		procedureParameters = malloc(nParameters*sizeof(char*));
		for(uint16 i = 0; i < nParameters; ++i) {
			++pos;
			tokcext(procedureParameters[i]);
		}
	}
	pos -= nParameters + 2; // return to opstring of procedure

	return &newProc->script;
}

static inline void buildThreadCollections(void) {
	enum HatType *hatType = NULL;
	struct ThreadList **broadcastThreadList = NULL;
	ThreadLink *thread = sprite->threads;
#define PUSH_ONTO_THREADLIST(list_ptr) {					\
		ThreadLink **p = (list_ptr)->array+1;					\
		while(*p != NULL) ++p;												\
		*p = thread;																	\
	}

	while((hatType = dynarray_next(threadTypes, hatType)) != NULL) {
		switch(*hatType) {
		case WHEN_GREEN_FLAG_CLICKED:
			dynarray_push_back(greenFlagThreads, &thread);
			break;
		case WHEN_I_RECEIVE:
			broadcastThreadList = dynarray_next(broadcastTypes, broadcastThreadList);
			if((*broadcastThreadList)->array == NULL) {
				threadList_init(*broadcastThreadList, (*broadcastThreadList)->nThreads);
				(*broadcastThreadList)->array[0] = thread;
			}
			else
				PUSH_ONTO_THREADLIST((*broadcastThreadList));
			break;
		case WHEN_CLONED:
			if(sprite->whenClonedThreads.nThreads == 0) {
				threadList_init(&sprite->whenClonedThreads, nWhenClonedThreads);
				sprite->whenClonedThreads.array[0] = thread;
			}
			else
				PUSH_ONTO_THREADLIST(&sprite->whenClonedThreads);
			break;
		}
		++thread;
	}
#undef PUSH_ONTO_THREADLIST

	sprite->nBroadcastThreadLists = dynarray_len(broadcastThreadLists);
	dynarray_extract(broadcastThreadLists, (void**)&sprite->broadcastThreadLists);
}

static void parseScripts(void) {
	// cleanup after last run
	dynarray_clear(threads);
	dynarray_clear(threadTypes);
	nWhenClonedThreads = 0;
	dynarray_clear(broadcastTypes);
	dynarray_clear(broadcastThreadLists);
	procedureHashTable = NULL;

	// begin parsing
	++pos; // advance to array of scripts
	uint16 nScriptsToGo = TOKC.size;
	++pos; // advance to first script ([xpos, ypos, [blocks...]])
	do { // for each script
		pos += 3; // advance past script position to script (array of blocks)
		const uint16 nStackBlocks = TOKC.size - 1;
		if(nStackBlocks == 0) { // if it is a lone block
			skip();
			continue;
		}

		Block **scriptPointer;
		pos += 2; // advance to hat block's opstring
		bool isProcedure = false;
		if(tokceq("procDef")) {
			isProcedure = true;
			scriptPointer = addProcedure();
		}
		else { // TODO: use a hash table rather than repeatedly comparing strings
			dynarray_extend_back(threads);
			ThreadLink *newThread = (ThreadLink*)dynarray_back(threads);
			threadContext_init(&newThread->thread, NULL);
			newThread->sprite = sprite;

			scriptPointer = (Block**)&newThread->thread.topBlock;

			enum HatType hatType;
			if(tokceq("whenGreenFlag")) {
				hatType = WHEN_GREEN_FLAG_CLICKED;
			}
			else if(tokceq("whenIReceive")) {
				hatType = WHEN_I_RECEIVE;
				addBroadcast();
			}
			else if(tokceq("whenCloned")) {
				++nWhenClonedThreads;
				hatType = WHEN_CLONED;
			}
			else { // it is not a hat
				threadContext_done(&newThread->thread);
				dynarray_pop_back(threads);
				pos -= 5; // go back to script ([xpos, ypos, [blocks...]])
				skip(); // skip the script
				continue;
			}
			dynarray_push_back(threadTypes, &hatType);
		}
		--pos; // go back to hat block (array)
		skip(); // advance to past hat block

		// allocate enough space for all the Blocks and Values that will make it up
		Value *valueBuffer;
		const unsigned scriptPos = pos;
		allocateScript(scriptPointer, &valueBuffer, nStackBlocks);

		// parse it into Blocks and Values
		pos = scriptPos; // return to the top of the script
		Block *blocks = *scriptPointer; Value *values = valueBuffer;
		parseStack(&blocks, &values, nStackBlocks, NULL);

		// cleanup
		if(isProcedure) {
			if(nParameters != 0) {
				while(nParameters != 0)
					free(procedureParameters[--nParameters]);
				free(procedureParameters);
			}
		}
	} while(--nScriptsToGo != 0);

	// load into sprite
	sprite->nThreads = dynarray_len(threads);
	dynarray_extract(threads, (void**)&sprite->threads);
	sprite->procedureHashTable = procedureHashTable;
	buildThreadCollections();
}

static bool attemptToParseSpriteProperty(void) {
	if(tokceq("variables")) {
		puts("variables");
		parseVariables();
	}
	else if(tokceq("lists")) {
		puts("lists");
		parseLists();
	}
	else if(tokceq("scripts")) {
		puts("scripts");
		parseScripts();
	}
	/*else if(tokceq("costumes")) {
		puts("costumes");
		parseCostumes();
		}*/
	else if(tokceq("scratchX")) {
		++pos;
		sprite->xpos = strtod(json+tokens[pos].start, NULL);
		++pos;
	}
	else if(tokceq("scratchY")) {
		++pos;
		sprite->ypos = strtod(json+tokens[pos].start, NULL);
		++pos;
	}
	else if(tokceq("scale")) {
		++pos;
		sprite->size = strtod(json+tokens[pos].start, NULL);
		++pos;
	}
	else if(tokceq("direction")) {
		++pos;
		sprite->direction = strtod(json+tokens[pos].start, NULL);
	}
	else
		return false;
	return true;
}

static inline void initializeSpriteContext(SpriteContext *const c, const enum SpriteScope scope) {
	c->name = NULL;
	c->scope = scope;

	c->variables = NULL;
	c->lists = NULL;

	c->procedureHashTable = NULL;
	c->whenClonedThreads.array = NULL;
	c->whenClonedThreads.nThreads = 0;
	c->broadcastThreadLists = NULL;

	c->xpos = c->ypos = 0.0;
	c->direction = 90.0;
	c->size = 100.0;
	c->effects.color = c->effects.brightness = c->effects.ghost
		= c->effects.pixelate = c->effects.mosaic
		= c->effects.fisheye = c->effects.whirl
		= 0.0;
}

static dynarray *sprites; // array containing pointers to all sprites

SpriteContext *newSprite(const enum SpriteScope scope) {
	struct SpriteLink *const new = malloc(sizeof(struct SpriteLink));
	dynarray_push_back(sprites, (void*)&new);
	initializeSpriteContext(&new->context, scope);
	return &new->context;
}

/* Takes a pointer to the JSON in memory, and returns an array of all the blocks (as in
	 blocks of Blocks and Values) that need to be freed when the player is stopped. */
void parseJSON(void) {
	// initialize
	blockMphf = loadBlockHashFunc();

	charCd = iconv_open("UTF-8", "UTF-32LE"); // LE for little-endian
	if(charCd == (iconv_t)-1) puts("[ERROR]Could not create encoding conversion descriptor.");
	dynarray_new(charBuffer, sizeof(char));

	dynarray_new(greenFlagThreads, sizeof(ThreadLink*));
	broadcastsHashTable = NULL;

	dynarray_new(threads, sizeof(ThreadLink));
	dynarray_new(threadTypes, sizeof(enum HatType));
	dynarray_new(broadcastTypes, sizeof(struct ThreadList*));
	dynarray_new(broadcastThreadLists, sizeof(struct ThreadList*));

	dynarray_new(sprites, sizeof(struct SpriteLink*));

	// begin parsing
	sprite = newSprite(STAGE);
	ufastest nKeysToGo = tokens[0].size;
	pos = 1;
	do {
		if(!attemptToParseSpriteProperty()) {
			if(tokceq("children")) {
				puts("children");
				++pos; // advance to array of "children"
				ufastest nChildrenToGo = TOKC.size;
				++pos; // advance to first child
				if(nChildrenToGo != 0) {
					do {
						ufastest nSpriteKeysToGo = TOKC.size - 1; // subtract 1 because "objName" is handled outside the loop

						++pos; // advance to first key of next child
						if(tokceq("objName")) { // if it is a sprite
							sprite = newSprite(SPRITE);

							// extract sprite name
							++pos;
							tokcext(sprite->name);
							++pos;

							do {
								if(!attemptToParseSpriteProperty())
									skip();
							} while(--nSpriteKeysToGo != 0);

						}
						else { // it is not a sprite
							--pos; skip();
						}
					} while(--nChildrenToGo != 0);
					sprite = &(*(struct SpriteLink**)dynarray_front(sprites))->context;
				}
			}
			else if(tokceq("tempoBPM")) {
				++pos;
				setTempo(strtod(json+tokens[pos].start, NULL));
				++pos;
			}
			else // key isn't significant
				skip();
		}
	} while(--nKeysToGo != 0);
	setVolume(100.0);

	sprite->name = "_stage_";

	// cleanup
	cmph_destroy(blockMphf);
	dynarray_free(charBuffer);

	dynarray_free(threads);
	dynarray_free(threadTypes);
	dynarray_free(broadcastTypes);
	dynarray_free(broadcastThreadLists);
	procedureHashTable = NULL;
}

void loadIntoRuntime(void) {
	setStage(&(*(struct SpriteLink**)dynarray_front(sprites))->context);

	struct SpriteLink *spriteHashTable = NULL; // hash table of all sprites
	struct SpriteLink **sprite = NULL;
	while((sprite = (struct SpriteLink**)dynarray_next(sprites, sprite)) != NULL)
		HASH_ADD_KEYPTR(hh, spriteHashTable, (*sprite)->context.name, strlen((*sprite)->context.name), *sprite);
	dynarray_free(sprites);
	setSprites(spriteHashTable);

	ThreadLink **finalizedThreads;
	unsigned nFinalizedThreads = dynarray_len(greenFlagThreads);
	dynarray_finalize(greenFlagThreads, (void**)&finalizedThreads);
	setGreenFlagThreads(finalizedThreads, nFinalizedThreads);

	setBroadcastsHashTable(broadcastsHashTable);
}

bool loadProject(const char *const projectPath) {
	size_t jsonLength;
	resources = loadSB2(projectPath, (char **)&json, &jsonLength);
	if(resources == NULL) return true; // loadSB2 prints its own error message

	tokenizeJson(jsonLength);
	parseJSON();

	free(json);
	free(tokens);
	free(resources);

	loadIntoRuntime();
	return false;
}
