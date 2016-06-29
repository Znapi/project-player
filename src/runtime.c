/**
	Runtime
	  runtime.c
**/

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <cmph.h>

#include "ut/uthash.h"
#include "ut/utarray.h"
#include "ut/dynarray.h"

#include "types/primitives.h"
#include "types/value.h"
#include "types/block.h"
#include "types/thread.h"
#include "types/sprite.h"

#include "runtime.h"

#include "variables.h"

#include "strpool.h"
#include "value.h"

static ThreadContext *activeThread;
static SpriteContext *activeSprite;

static ThreadLink runningThreads = {{0}, NULL, NULL, NULL}; // the first item of the list is a stub that points to the first real item

static clock_t dtime;
static const clock_t workTime = (clock_t).75f * 1000 / 30; // work only for 75% of the alloted frame time. taken from Flash version.
static bool doRedraw, doYield;

static SpriteContext *stage;

void setStage(SpriteContext *const stageContext) {
	stage = stageContext;
}

static dynarray askResponse;

void initializeAskPrompt(void) {
	dynarray_init(&askResponse, sizeof(char));
	dynarray_ensure_size(&askResponse, 1024); // should be good enough
	dynarray_extend_back(&askResponse); // push a single null terminator
}

/**
	Counters

	Counters allow a block functions to store a float or integer between invocations
	(e.g. bf_repeat counts iterations).
	This allows control structure blocks to be treated like any other block.
**/

/* alloc is not quite the right term, but I couldn't think of a better one */
static bool allocTmpData(const Block *const block) {
	if(activeThread->tmp.slotsUsed == 0) {
		activeThread->tmp.owners[activeThread->tmp.slotsUsed] = block;
		++activeThread->tmp.slotsUsed;
		return true;
	}
	else if(activeThread->tmp.owners[activeThread->tmp.slotsUsed-1] != block) {
		activeThread->tmp.owners[activeThread->tmp.slotsUsed] = block;
		++activeThread->tmp.slotsUsed;
		return true;
	}
	else
		return false;
}

static void freeTmpData(void) {
	--activeThread->tmp.slotsUsed;
	activeThread->tmp.owners[activeThread->tmp.slotsUsed] = NULL;
}

static union TmpData* getTmpDataPointer(void) {
	return activeThread->tmp.data+(activeThread->tmp.slotsUsed-1);
}

/* get the unsigned integer value of the current counter */
static inline uint32 ugetTmpData(void) {
	return activeThread->tmp.data[activeThread->tmp.slotsUsed-1].u;
}

/* set the unsigned integer value of the current counter */
static inline void usetTmpData(const uint32 newValue) {
	activeThread->tmp.data[activeThread->tmp.slotsUsed-1].u = newValue;
}

/* get the single precision floating point value from the counter */
static inline float fgetTmpData(void) {
	return activeThread->tmp.data[activeThread->tmp.slotsUsed-1].f;
}

static inline void fsetTmpData(const float newValue) {
	activeThread->tmp.data[activeThread->tmp.slotsUsed-1].f = newValue;
}

static inline void* pgetTmpData(void) {
	return activeThread->tmp.data[activeThread->tmp.slotsUsed-1].p;
}

static inline void psetTmpData(void *const newValue) {
	activeThread->tmp.data[activeThread->tmp.slotsUsed-1].p = newValue;
}

/**
	Stack Frame Handling

	Originally, the plan was to pre-calculate stack jumps during the parsing stage, but that
	required lots of tracking of stacks during the parsing that I decided to ditch for just
	using stack frames. Stack frames also allow for local variables...if they ever come
	about. Maybe after I have a working version I will add pre-calculating stack jumps again.
**/

/* this procedure should not be used by anything other than enterSubstack/Procedure */
static inline void pushStackFrame(const Block *const returnStack) {
	activeThread->frame.nextBlock = returnStack;
	dynarray_push_back(activeThread->blockStack, &activeThread->frame);
}

/* this procedure should only be used by the interpreter */
static inline void popStackFrame(void) {
	if(activeThread->frame.level == 0) { // if we are inside a procedure, need to pop parameters as well
		dynarray_pop_back_n(activeThread->parametersStack, *(uint16*)dynarray_back(activeThread->nParametersStack));
		dynarray_pop_back(activeThread->nParametersStack);
		if(dynarray_len(activeThread->nParametersStack) != 0)
			activeThread->parameters = (Value*)dynarray_eltptr(activeThread->parametersStack,
																												 dynarray_len(activeThread->parametersStack) - *(uint16*)dynarray_back(activeThread->nParametersStack));
	}
	activeThread->frame = *((struct BlockStackFrame*)dynarray_back_unchecked(activeThread->blockStack));
	dynarray_pop_back(activeThread->blockStack);
}

/* convenience procedures for block functions */
static void enterSubstack(const Block *const returnStack) {
	pushStackFrame(returnStack);
	++activeThread->frame.level;
}

static void enterProcedure(const Block *const returnStack) {
	pushStackFrame(returnStack);
	activeThread->frame.level = 0;
}

/**
	Thread Control

	This part also resembles Interpreter.as in scratch-flash.

	In order for the interpreter to execute blocks in a thread, it needs to keep track of
	various pieces of data specific to a single execution of a block stack (the same custom
	block can be executed by multiple threads, and the same block stack can be executed by
	multiple clones). These are contained in a ThreadContext. See types/thread.h to see
	exactly what these pieces of data are.

	The interpreter also needs to know what sprite ("sprite" includes clones and the stage)
	the blocks are being run under, so properties, such as x and y positions and variables,
	can be made to the proper sprite. These properties are not specific to a single
	execution of a block stack, unlike the data that a ThreadContext stores. Instead,
	properties of one sprite modifiable by the interpreter are contained in a SpriteContext.

	Finally, a ThreadContext and a reference to its matching SpriteContext are stored in a
	ThreadLink. This makes it easy for the runtime to keep them associated when passing
	around the full context needed for a thread, without storing a reference to the
	SpriteContext in the ThreadContext itself.

	ThreadLinks also serve another purpose than giving ThreadContexts a way to
	reference their matching SpriteContext, which could be done by the ThreadContext
	itself. ThreadLinks are links in for creating linked lists of threads. This way,
	ThreadLinks can be stored however by another part of the runtime, and all that needs to
	be done to add them to the list of running threads is to change a couple of pointers.

	It is useful to make it easy to find all of the threads for one sprite, so ThreadLinks
	that share the same SpriteContext are all contained in an array. The SpriteContext
	contains a reference to this array. They can easily be iterated over, and they can
	be easily copied in order to make the threads for a newly created clone.

	Starting threads requires knowing where the ThreadLink for the thread is in memory. So
	that the runtime doesn't have to search each time the threads for a certain hat block
	need to be started, for each type of hat block a collection of references to the threads
	they start are created in the loading stage and added to/removed from by making/
	destroying clones. In the case of green flag scripts and stage-specific hats, however,
	these collections can simply be constant-sized arrays, because they will never need to
	be modified by the runtime.
**/

ThreadContext createThreadContext(const Block *const block) {
	ThreadContext new = {
		malloc(16*sizeof(Value)),

		block,
		{0, NULL},
		NULL,

		0,

		{malloc(16*sizeof(union TmpData)), malloc(16*sizeof(Block *)), 0},

		NULL,
		NULL,
		NULL,
	};
	dynarray_new(new.blockStack, sizeof(struct BlockStackFrame));
	dynarray_new(new.parametersStack, sizeof(Value));
	dynarray_new(new.nParametersStack, sizeof(uint16));
	return new;
}

void freeThreadContext(const ThreadContext *const context) {
	free(context->stack);
	dynarray_free(context->blockStack);
	dynarray_free(context->parametersStack);
	dynarray_free(context->nParametersStack);
	free(context->tmp.data);
	free(context->tmp.owners);
}

static void resetThreadContext(ThreadContext *const context) {
	context->frame.level = 0;
	context->frame.nextBlock = NULL;
	dynarray_clear(context->blockStack);
	context->tmp.slotsUsed = 0;
	context->parameters = NULL;
	dynarray_clear(context->parametersStack);
	dynarray_clear(context->nParametersStack);
}

#define isThreadStopped(t) ((t).frame.nextBlock == NULL) // && (t).frame.level == 0 && dynarray_len((t).blockStack) == 0)

static void startThread(ThreadLink *const link) {
	resetThreadContext(&link->thread);
	link->thread.frame.nextBlock = link->thread.topBlock;
	if(link->prev != NULL) // if the thread is already started, don't attempt to readd it to the list
		return;

	if(runningThreads.next == NULL) {
		link->next = NULL;
	}
	else {
		link->next = runningThreads.next; // link the given to the first item in the list of running threads
		runningThreads.next->prev = link;
	}
	link->prev = &runningThreads;
	runningThreads.next = link; // add it to the list of running threads
}

// start threads using an array of pointers to the threads
static void startThreadsInArray(ThreadLink *const *const threads, uint16 nThreads) {
	do {
		startThread(threads[--nThreads]);
	} while(nThreads != 0);
}

static ThreadLink* stopThread(ThreadLink *const stopped) {
	ThreadLink *next = stopped->next;
	if(next == NULL) {
		stopped->prev->next = NULL;
	}
	else {
		stopped->prev->next = next;
		next->prev = stopped->prev;
	}
	stopped->prev = stopped->next = NULL;
	return next;
}

static void stopAllThreads(void) {
	ThreadLink *current = &runningThreads,
		*next;
	while(current != NULL) {
		next = current->next;
		current->next = current->prev =  NULL;
		current = next;
	}
}

static void stopThreadsForSprite(const bool stopCurrentThread) {
	ThreadLink *current = &runningThreads;
	while(current != NULL) {
		current = current->next;
		if(current->sprite == activeSprite) {
			if(stopCurrentThread || &current->thread != activeThread)
				stopThread(current);
		}
	}
}

static ThreadLink *const *greenFlagThreads;
static uint16 nGreenFlagThreads;

void setGreenFlagThreads(ThreadLink *const *const threads, const uint16 nThreads) {
	greenFlagThreads = threads;
	nGreenFlagThreads = nThreads;
}

void freeGreenFlagThreads(void) {
	free((void*)greenFlagThreads);
}

void restartGreenFlagThreads(void) {
	startThreadsInArray(greenFlagThreads, nGreenFlagThreads);
}

static struct BroadcastThreads *broadcastsHashTable = NULL;

void setBroadcastsHashTable(struct BroadcastThreads *const hashTable) {
	broadcastsHashTable = hashTable;
}

void freeBroadcastsHashTable(void) {
	struct BroadcastThreads *current, *next;
	HASH_ITER(hh, broadcastsHashTable, current, next) {
		dynarray_free(current->threads);
		HASH_DEL(broadcastsHashTable, current);
		free(current);
	}
}

/* returns a boolean saying whether or not the current thread was restarted */
static bool startBroadcastThreads(const char *const msg, const size_t msgLen, struct BroadcastThreads **const nullifyOnRestart) {
	struct BroadcastThreads *broadcastThreadsLink;
	HASH_FIND(hh, broadcastsHashTable, msg, msgLen,  broadcastThreadsLink);
	if(broadcastThreadsLink == NULL) {
		if(nullifyOnRestart != NULL)
			*nullifyOnRestart = NULL;
		return false;
	}
	if(broadcastThreadsLink->nullifyOnRestart != NULL)
		*broadcastThreadsLink->nullifyOnRestart = NULL;
	broadcastThreadsLink->nullifyOnRestart = nullifyOnRestart;
	if(nullifyOnRestart != NULL)
		*nullifyOnRestart = broadcastThreadsLink;

	bool r = false;
	dynarray *threads = broadcastThreadsLink->threads;
	for(uint16 i = 0; i < dynarray_len(threads); ++i) {
		ThreadLink **thread = (ThreadLink**)dynarray_eltptr(threads, i);
		if(&(*thread)->thread == activeThread)
			r = true;
		startThread(*thread);
	}
	return r;
}

/**
	Procedure Lookups

	Custom block procedures are stored as scripts just like all the other scripts. A hash
	table is used to lookup the location of the scripts in memory. The procedure name is
	hashed, and a pointer to the top block of the procedure is found.

	All procedure arguments for a thread are stored in a dynamic array. The argument count
	for each procedure is found in hash tables, and is kept so the right amount of arguments
	are freed at the end of the procedure.
**/

static const struct ProcedureLink *getProcedure(const char *const label, const size_t labelLen) {
	struct ProcedureLink *procLink;
	HASH_FIND(hh, activeSprite->procedureHashTable, label, labelLen, procLink);
	return procLink;
}

/**
	Interpreter

	The interpreter is still simple like in the Flash version.

	The function pointer for each block is pre-computed by looking up the opcode string of a
	each block in a hashtable, getting a function pointer. The interpreter calls that
	function, which I call a block function (it is called a primitive in scratch-flash). The
	block function performs some instructions, and then it returns the a pointer to the next
	block in the stack, or NULL if the block is at the end of the stack.

	Unlike the Flash version, Scratch blocks aren't evaluated strictly left to right, so
	that recursion can be done by the interpreter, not the block functions themselves.
	This also means that the interpreter has to maintain its own stack of evaluated
	arguments, rather than relying on recursion.

	Also unlike the Flash version, control blocks aren't that special. They are treated like
	any other block, and have a block function just like any other block.
**/

/** Include the runtime library and hash value table **/
#include "runtime_lib.c"
#include "blockhash/opstable.c"

/* basically `evalCmd` in the Flash version */
static const Block* interpret(const Block block[], Value stack[], Value *const reportSlot, const ufastest level) {
	Block next;
	ufastest blockPos = 0, stackPos = 0;

	do {
		next = block[blockPos];

		if(next.level == level) {
			if(next.func == NULL) // constant argument
				stack[stackPos] = *(next.p.value);
			else // block argument (without any arguments itself)
				(*next.func)(block + blockPos, stack+stackPos, NULL);
			++stackPos;
		}
		else if(next.level > level) { // need to recurse
			const Block *new = interpret(block + blockPos, stack + stackPos + 1, stack + stackPos, level + 1);
			++stackPos;
			blockPos += new - (block + blockPos);
		}
		else /* next.level < level */ { // it must be a block
			const Block *move = (*next.func)(block + blockPos, reportSlot, stack);
			if(level == 1)
				return move;
			else
				return &(block[blockPos]);
		}

		++blockPos;
	} while(next.level >= level);
	// if we have exited the loop, some kind of error occured
	puts("[ERROR]Interpreter exited loop because next.level < level!");
	return NULL;
}

/* Steps the active thread until a yield point is reached or there are no more blocks.
   Returns a boolean to tell whether or not the thread should be stopped. */
static bool stepActiveThread(void) {
	clock_t newTime;
	doYield = false;
	while(!doYield) {
		newTime = clock();
		dtime = (float)newTime - activeThread->lastTime;
		activeThread->lastTime = newTime;

		activeThread->frame.nextBlock = interpret(activeThread->frame.nextBlock, activeThread->stack, NULL, 1);
		strpool_empty(); // free strings allocated to during evaluation

		while(activeThread->frame.nextBlock == NULL) {
			if(dynarray_len(activeThread->blockStack) != 0)
				popStackFrame();
			else
				return true;
		}
	}
	return false;
}

/* Steps all threads that are in the list of running threads as of being called.
	 Returns a boolean telling whether or not there are still threads left. */
bool stepThreads(void) {
	doRedraw = false;
	ThreadLink *current;
	clock_t startTime = clock(),
		currentTime = startTime;
	do {
		current = runningThreads.next;

		// step each thread
		while(current != NULL) {
			//printf("--thread\n");
			activeThread = &current->thread; // set the active context
			activeSprite = current->sprite;

			// step the thread
			if(stepActiveThread()) { // if the thread should be killed
				if(current->prev == NULL) // if the chain has already been broken, meaning all threads were stopped
					return false;
				else {
					current = stopThread(current);
					if(runningThreads.next == NULL) {
						return false;
					}
				}
			}
			else {
				current = current->next; // advance to the next context
			}
		}
		// get the current time, check if a redraw needs to be done, and repeat TODO
		currentTime = clock();
	} while(currentTime - startTime < workTime && doRedraw == false);
	return true;
}
