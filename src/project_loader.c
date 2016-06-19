/**
	Project Loader
	  project_loader.c

	This module takes a project.json that is already loaded into memory, parses it, and
	loads all the data needed to run the project into the runtime. The loading of the
	project.json, as well as other resources used by the project, into memory is handled by
	a different module.
**/

#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>

#include <cmph.h>

#include "jsmn/jsmn.h"
#include "ut/dynarray.h"

#include "types/primitives.h"
#include "types/value.h"
#include "types/variables.h"
#include "types/block.h"
#include "types/thread.h"
#include "types/sprite.h"

#include "value.h"
#include "variables.h"
#include "runtime.h"

static const char *json;
static jsmntok_t *tokens;
static unsigned pos;

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
	printf("nTokens: %u\n", nTokens);

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

static inline void initializeSpriteContext(SpriteContext *c) {
	c->variables = NULL;
	c->lists = NULL;
	c->xpos = c->ypos = c->layer = c->costumeNumber = 0;
	c->direction = 90; c->size = c->volume = 100; c->tempo = 60;
	c->effects.color = c->effects.brightness = c->effects.ghost
		= c->effects.pixelate = c->effects.mosaic
		= c->effects.fisheye = c->effects.whirl
		= 0;
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
	unsigned tokensToSkip = 1;
	do {
		tokensToSkip += TOKC.size; // add child tokens to tokensToSkip
		++pos; // skip token
	} while(--tokensToSkip != 0);
}

static iconv_t charCd;
static dynarray *charBuffer; // dynarray of chars to be reused for temporarily storing strings

/* Parses the string starting at str of length len into charBuffer. */
static void parseString(const char *str, const unsigned len) {
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

/* Token position should be pointing to the key "variables", just before the array of
	 variables, and it will be left pointing to the token just after the array. */
static Variable* parseVariables(void) {
	++pos; // advance to array

	uint16 propertiesToGo, i = 0, nVariablesToGo = TOKC.size; // number of variable objects to parse
	Variable *variableBuffer = malloc(nVariablesToGo*sizeof(Variable)); // array of variables, and the first element is also the entry point of the hash table of variables TODO: free
	Variable *variables = NULL;

	char *name;
	Value value;

	do { // for each variable object
		++pos; // advance to variable object
		propertiesToGo = TOKC.size;
		do { // for each property
			++pos; // advance to key
			if(tokceq("name")) {
				++pos; // advance to value
				tokcext(name);
			}
			else if(tokceq("value")) {
				++pos; // advance to value
				parseString(gjson(TOKC), tokclen());
				value = strnToValue(charBuffer->d, charBuffer->i-1);
			}
			else // isPersistent
				++pos;
		} while(--propertiesToGo != 0);
		variable_init(&variables, variableBuffer+i, name, &value);
		free(name);
		value_free(value);
		++i;
	} while(--nVariablesToGo != 0);
	++pos; // advance from last property
	return variables;
}

/* Token position should be pointing to the key "lists", just before the array of lists,
	 and it will be left pointing to the token just after the array. */
static List* parseLists(void) {
	++pos; // advance to array
	uint16 propertiesToGo, i = 0, nListsToGo = TOKC.size; // number of list objects to parse

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
			else
				++pos; // advance to value
		} while(--propertiesToGo != 0);
		list_init(&lists, listBuffer+i, charBuffer->d);
		for(propertiesToGo = valueToken->size; propertiesToGo != 0; --propertiesToGo) {
			++valueToken;
			parseString(gjson(*valueToken), toklen(*valueToken));
			value = strnToValue(charBuffer->d, charBuffer->i-1);
			listAppend(listBuffer+i, &value);
			value_free(value);
		}
		++i;
	} while(--nListsToGo != 0);
	++pos; // advance from last property
	return lists;
}

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
	printf("nBlocks: %zu\nnValues: %zu\n", nBlocks, nValues);
}

cmph_t *blockMphf;
blockhash noop_hash;

static inline uint32 hash(const char *const key, const size_t keyLen, cmph_t *mphf) {
	return cmph_search(mphf, key, keyLen);
}

static inline uint32 tokchash_new(cmph_t *mphf) {
	parseString(gjson(TOKC), tokclen());
	return hash(charBuffer->d, dynarray_len(charBuffer) - 1, mphf);
}

static inline uint32 tokchash(cmph_t *mphf) {
	return cmph_search(mphf, gjson(TOKC), tokclen());
}

#include "blockhash/typestable.c"

static char **procedureParameters;
static uint16 nParameters;

/* Parses arguments to a block, using recursion when one of the arguments is another
	 block. pos should point to the first argument, and is left pointing after the last
	 token parsed. */
static void parseBlockArgs(Block **const blocks, Value **const values, uint16 argsToGo, const ubyte level) {
	Block *block = *blocks;
	Value *value = *values;
	if(argsToGo == 0) return;
	do {
		if(TOKC.type == JSMN_ARRAY) { // if argument is a block
			++pos; // advance to opstring
			blockhash hash = tokchash_new(blockMphf); // hash opstring

			if(tokceq("getParam")) { // if it is a procedure parameter
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
				block->hash = noop_hash;
				block->p.value = value;
				block->level = level + 1;
				++value;
				++block;
				block->hash = hash;
				block->level = level;
				++block;
				++pos; // advance past argument
				continue;
			}

			++pos; // advance to first argument
			parseBlockArgs(&block, &value, tokens[pos - 2].size - 1, level+1); // parse the arguments
			block->hash = hash;
		}
		else {
			if(TOKC.type == JSMN_STRING) { // argument is a string
				value->type = STRING;
				tokcext(value->data.string);
				block->hash = noop_hash;
				block->p.value = value;
			}
			else { // else argument is a primitive
				*value = strnToValue(gjson(TOKC), tokclen()); // assume characters don't need special parsing
				block->hash = noop_hash;
				block->p.value = value;
			}
			++value;
			++pos; // advance to next argument/child
		}
		block->level = level;
		++block;
	} while(--argsToGo != 0);
	*blocks = block;
	*values = value;
}

/* pos should be pointing to first block of the stack, and will be left pointing after the last token parsed */
static void parseStack(Block **const blocks, Value **const values, uint16 nStackBlocksToGo, Block **link) {
	Block *block = *blocks;
	do {
		++pos; // advance to opstring
		if(link != NULL)
			*link = block;

		blockhash hash = (blockhash)tokchash(blockMphf);
		++pos; // advance to first argument (or next block if there is no arguments)

		switch(blockTypesTable[hash]) {

		case BLOCK_TYPE_S: // 1 substack: 1 following (normal stack block)
			parseBlockArgs(&block, values, tokens[pos-2].size - 1, 1);
			block->hash = hash;
			link = &(block->p.next);
			++block;
			break;

		case BLOCK_TYPE_C: // 2 substacks: 1 inner and 1 following
			parseBlockArgs(&block, values, tokens[pos-2].size - 2, 1);
			block->hash = hash;
			block->p.substacks = malloc(2*sizeof(Block*));
			link = block->p.substacks+1;
			++block;
			++pos; // advance to first block of substack
			parseStack(&block, values, tokens[pos-1].size, (block-1)->p.substacks+0);
			break;

		case BLOCK_TYPE_CF: // 1 substack: 1 inner and no following
			//parseBlockArgs(&block, values, tokens[pos - 2].size - 2 // no cf blocks have arguments currently
			block->hash = hash;
			block->p.substacks = malloc(1*sizeof(Block*));
			link = NULL;
			++block;
			++pos;
			parseStack(&block, values, tokens[pos-1].size, (block-1)->p.substacks+0);
			break;

		case BLOCK_TYPE_E: // 3 substacks: 2 inner and 1 following
			parseBlockArgs(&block, values, tokens[pos-2].size - 3, 1);
			block->hash = hash;
			Block **const substacks = malloc(3*sizeof(Block*));
			block->p.substacks = substacks;
			link = substacks+2;
			++block;
			++pos;
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
	*blocks = block;
}

enum HatType {
	GREEN_FLAG,
	PROCEDURE,
};

// dynarmic arrays of pointers to scripts under specific hat blocks, for easy lookup by
// the runtime
static dynarray *greenFlagThreads;

static cmph_t *procedureMphf;
static Block **procedureHashTable;
static uint16 *nProcedureArgsHashTable;

static cmph_t* generateMphf(char **keys, const uint16 nKeys, CMPH_ALGO algorithm) {
	cmph_io_adapter_t *keySource = cmph_io_vector_adapter(keys, nKeys);
	cmph_config_t *config = cmph_config_new(keySource);
	cmph_config_set_algo(config, algorithm);
	cmph_t *mphf = cmph_new(config);
	cmph_config_destroy(config);
	return mphf; // mphf needs to be destroyed
}

static ThreadLink* parseScripts(SpriteContext *sprite) {
	dynarray *threads;
	dynarray_new(threads, sizeof(ThreadLink));
	ThreadLink tmpThread = {
		{0},
		sprite,
		NULL
	};

	dynarray *procedureNames;
	dynarray *procedures;
	dynarray *nProcedureArgs;
	dynarray_new(procedureNames, sizeof(char*));
	dynarray_new(procedures, sizeof(Block*));
	dynarray_new(nProcedureArgs, sizeof(uint16));

	++pos; // advance to array of scripts
	uint16 nScriptsToGo = TOKC.size;
	++pos; // advance to first script ([xpos, ypos, [blocks...]])
	do { // for each script
		pos += 3; // advance past script position to script (array of blocks)
		const uint16 nStackBlocks = TOKC.size - 1;
		if(nStackBlocks == 0) {// if this is a lone block
			pos -= 3; // go back to script
			skip(); // and skip it
			continue;
		}

		// first, check if the script is topped by a hat block, and will be reached by the runtime
		enum HatType hatType;
		pos += 2; // advance to opstring of first block
		if(tokceq("whenGreenFlag"))
			hatType = GREEN_FLAG;
		else if(tokceq("procDef")) {
			hatType = PROCEDURE;
			++pos; // advance to procedure label
			const char *label;
			tokcext(label);
			dynarray_push_back(procedureNames, (void*)&label);
			++pos; // advance to array of parameter declarations
			nParameters = TOKC.size;
			dynarray_push_back(nProcedureArgs, &nParameters);
			if(nParameters != 0) {
				procedureParameters = malloc(nParameters*sizeof(char*));
				for(uint16 i = 0; i < nParameters; ++i) {
					++pos;
					tokcext(procedureParameters[i]);
				}
			}
			++pos; // advance to array of default arguments
			skip();
			++pos; // advance past withoutScreenRefresh boolean
		}
		else { // it is not a hat
			pos -= 4; // go back to script ([xpos, ypos, [blocks...]])
			skip(); // skip the script
			continue;
		}
		--pos;
		skip(); // advance to past hat block

		// if it is a hat block, then allocate enough space for all the Blocks and Values that will make it up
		Block *blockBuffer;
		Value *valueBuffer;
		const unsigned scriptPos = pos;
		allocateScript(&blockBuffer, &valueBuffer, nStackBlocks);

		// parse it into Blocks and Values
		pos = scriptPos; // return to the top of the script
		Block *blocks = blockBuffer; Value *values = valueBuffer;
		parseStack(&blocks, &values, nStackBlocks, NULL);

		// create a ThreadLink for it, and add it to the list
		ThreadLink *newThreadLink;
		if(hatType != PROCEDURE) {
			tmpThread.thread = createThreadContext(blockBuffer);
			dynarray_push_back(threads, &tmpThread);
			newThreadLink = dynarray_back(threads);
		}

		// finally, organize based on hat type for easy lookup by the runtime

		switch(hatType) {
		case GREEN_FLAG:
			dynarray_push_back(greenFlagThreads, &newThreadLink);
			break;
		case PROCEDURE:
			dynarray_push_back(procedures, &blockBuffer);
			if(nParameters != 0) {
				while(nParameters != 0)
					free(procedureParameters[--nParameters]);
				free(procedureParameters);
			}
			break;
		}
	} while(--nScriptsToGo != 0);

	// finalize dynamic data structures
	ThreadLink *threadsFinalized;
	dynarray_finalize(threads, (void**)&threadsFinalized);

	procedureMphf = generateMphf((char**)procedureNames->d, dynarray_len(procedures), CMPH_FCH);
	procedureHashTable = malloc(dynarray_len(procedures) * sizeof(Block*));
	nProcedureArgsHashTable = malloc(dynarray_len(procedures) * sizeof(uint16));
	for(unsigned i = 0; i < dynarray_len(procedures); ++i) {
		const char *const key = *((char**)dynarray_eltptr(procedureNames, i));
		uint32 phash = hash(key, strlen(key), procedureMphf);
		procedureHashTable[phash] = *((Block**)dynarray_eltptr(procedures, i));
		nProcedureArgsHashTable[phash] = *((uint16*)dynarray_eltptr(nProcedureArgs, i));
	}

	// clean up
	for(unsigned i = dynarray_len(procedureNames); i != 0;)
		free(((char**)procedureNames->d)[--i]);
	dynarray_free(procedureNames);
	dynarray_free(procedures);
	dynarray_free(nProcedureArgs);
	return threadsFinalized;
}

static bool attemptToParseSpriteProperty(SpriteContext *sprite) {
	if(tokceq("variables")) {
		puts("variables");
		sprite->variables = parseVariables();
		return true;
	}
	else if(tokceq("lists")) {
		puts("lists");
		sprite->lists = parseLists();
		return true;
	}
	else if(tokceq("scripts")) {
		puts("scripts");
		sprite->threads = parseScripts(sprite);
		sprite->procedures = procedureHashTable;
		sprite->nProcedureArgs = nProcedureArgsHashTable;
		sprite->proceduresMphf = procedureMphf;
		return true;
	}
	return false;
}

/* Takes a pointer to the JSON in memory, and returns an array of all the blocks (as in
	 blocks of Blocks and Values) that need to be freed when the player is stopped. */
void** loadProject(const char *const projectJson, const size_t jsonLength, ufastest *const nData) {
	// tokenize
	json = projectJson;
	tokenizeJson(jsonLength);

	// parse
	blockMphf = loadBlockHashFunc();
	noop_hash = cmph_search(blockMphf, "noop", 4);

	charCd = iconv_open("UTF-8", "UTF-32LE"); // LE for little-endian
	if(charCd == (iconv_t)-1) puts("[ERROR]Could not create encoding conversion descriptor.");
	dynarray_new(charBuffer, sizeof(char));

	dynarray_new(greenFlagThreads, sizeof(ThreadLink*));

	SpriteContext *stageContext = malloc(sizeof(SpriteContext));
	initializeSpriteContext(stageContext);

	ufastest i = tokens[0].size;
	pos = 1;
	do {
		if(!attemptToParseSpriteProperty(stageContext)) {
			if(tokceq("children")) {
				puts("children");
				skip();
			}
			else // key isn't significant
				skip();
		}
	} while(--i != 0);

	// load into runtime
	ThreadLink **greenFlagThreadsFinalized;
	unsigned nGreenFlagThreads = dynarray_len(greenFlagThreads);
	dynarray_finalize(greenFlagThreads, (void**)&greenFlagThreadsFinalized);
	setGreenFlagThreads(greenFlagThreadsFinalized, nGreenFlagThreads);

	// free resources
	free(tokens);
	cmph_destroy(blockMphf);
	dynarray_free(charBuffer);

	*nData = 0;
	return malloc(1);
}
