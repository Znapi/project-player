/**
	Runtime
			runtime.c

	Interprets the blocks and manages the threads.

	The interpreter is still simple like in the Flash version, and in fact even simpler.
	It looks up the opcode string of a block in a dictionary (hashtable	specifically),
	getting a function pointer, and calls that function. The function accomplishes what
	the block is supposed to do, then returns the a pointer to the next block in the
	stack, or NULL if it is the end of the stack.

	Unlike the Flash version, control blocks aren't that special. They are treated like
	any other block, and have a function just like any other block. States aren't used
	like in the Flash version. Instead, a little extra work is done when making Block
	stacks, and control blocks contain pointers to the block sequences they can move
	the program flow to.

	Also unlike the Flash version, Scratch blocks are evaluated top-down. This reduces
	the amount of recursion done. The way blocks are stored in the JSON actually is
	helpful here. Arrays of blocks stored internally are basically the same arrays of
	blocks in the JSON but reversed.

	Thread management is pretty much a port from Flash version. If you know how	it
	works there, then you already know how it works here.

	Currently the goal is to mimic the Flash version because it works. Once this
	version is feature-complete, then changes can be made to make it more efficient.
**/

#include <stdio.h>
#include <math.h>
#include <time.h>

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

static ThreadContext *activeContext;
static clock_t dtime;

static Variable *stageVariables;
static List *stageLists;

/* An *item* in the linked list of ThreadContexts, despite being called a "...List" */
struct ThreadContextList {
	struct ThreadContext *context;
	struct ThreadContextList *next;
};
typedef struct ThreadContextList ThreadContextList;

static ThreadContextList contexts = {NULL, NULL}; // stub item that points to the first actual item of the linked list of contexts.

static const clock_t workTime = (clock_t).75f * 1000 / 30; // work only for 75% of the alloted frame time. taken from Flash version.
static bool doRedraw, doYield;

/**
	 Counters

	 Counters allow a block functions to store a float or integer between invocations
	 (e.g. bf_repeat counts iterations).
	 This allows control structure blocks to be treated like any other block.
**/

/* alloc is not quite the right term, but I couldn't think of a better one */
static bool allocCounter(const Block *const block) {
	if(activeContext->counters.slotsUsed == 0) {
		activeContext->counters.owners[activeContext->counters.slotsUsed] = block;
		++activeContext->counters.slotsUsed;
		return true;
	}
	else if(activeContext->counters.owners[activeContext->counters.slotsUsed-1] != block) {
		activeContext->counters.owners[activeContext->counters.slotsUsed] = block;
		++activeContext->counters.slotsUsed;
		return true;
	}
	else
		return false;
}

static void freeCounter(void) {
	--activeContext->counters.slotsUsed;
	activeContext->counters.owners[activeContext->counters.slotsUsed] = NULL;
}

/* get the unsigned integer value of the current counter */
static inline uint32 ugetCounter(void) {
	return activeContext->counters.counters[activeContext->counters.slotsUsed-1].u;
}

/* set the unsigned integer value of the current counter */
static inline void usetCounter(const uint32 newValue) {
	activeContext->counters.counters[activeContext->counters.slotsUsed-1].u = newValue;
}

/* get the single precision floating point value from the counter */
static inline float fgetCounter(void) {
	return activeContext->counters.counters[activeContext->counters.slotsUsed-1].f;
}

static inline void fsetCounter(const float newValue) {
	activeContext->counters.counters[activeContext->counters.slotsUsed-1].f = newValue;
}

/**
	 Stack Frame Handling

	 Originally, the plan was to pre-calculate stack jumps during the parsing stage, but
	 that required lots of tracking of stacks that I decided to ditch for just using
	 stack frames. Stack frames also allow for local variables...if they ever come about.
	 Maybe after I have a working version I will add pre-calcilating stack jumps again.
**/

static inline void pushStackFrame(const Block *nextStack) {
	dynarray_push_back(activeContext->nextStacks, &nextStack);
}

static inline void popStackFrame(void) {
	activeContext->nextBlock = *((Block**)dynarray_back_unsafe(activeContext->nextStacks)); // `unsafe` means don't check if there even is a back
	dynarray_pop_back(activeContext->nextStacks);
}

/**
	 Interpreter

	 This part is extremely similar to Interpreter.as in scratch-flash.
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

		if(next.level == level) { // it must be a constant argument
			stack[stackPos] = *(next.p.value);
			++stackPos;
		}
		else if(next.level > level) { // need to recurse
			const Block *new = interpret(block + blockPos, stack + stackPos + 1, stack + stackPos, level + 1);
			if(new == NULL)
				return NULL;
			++stackPos;
			blockPos += new - (block + blockPos);
		}
		else/* next.level < level */{ // it must be a block
			Block *move = (*opsTable[next.hash])(block + blockPos, reportSlot, stack);
			if(level == 1)
				return move;
			else
				return &(block[blockPos]);
		}

		++blockPos;
	} while(next.level >= level);
	// if we have exited the loop, some kind of error occured
	puts("ERROR OCCURED WHILE EVALUTAING");
	return NULL;
}

/** Thread Control **/

#define stopThread(context) {context->nextBlock = NULL;}

ThreadContext createThreadContext(SpriteContext *const spriteContext, const Block *const block) {
	ThreadContext new = {
		malloc(16*sizeof(Value)),
		block,
		NULL,
		0,
		{malloc(16*sizeof(union Counter)), malloc(16*sizeof(Block *)), 0},
		spriteContext
	};
	dynarray_new(new.nextStacks, sizeof(Block*));
	return new;
}

void freeThreadContext(const ThreadContext context) {
	free(context.stack);
	dynarray_free(context.nextStacks);
	free(context.counters.counters);
	free(context.counters.owners);
}

void startThread(ThreadContext *const context) {
	ThreadContextList *new = malloc(sizeof(ThreadContextList)); // allocate new list item
	new->context = context; // initialize it
	new->next = contexts.next; // link it to the first item in the list
	contexts.next = new; // set it to the first item
}

static void stopAllThreads(void) {
	ThreadContextList *current = contexts.next,
		*last = current;
	while(current != NULL) {
		stopThread(current->context);
		last = current;
		current = current->next;
		free(last);
	}
	contexts.next = NULL;
}

/* Steps the active thread until a yield point is reached or there are no more blocks.
   Returns a boolean to tell whether or not the thread should be stopped. */
static bool stepActiveThread(void) {
	clock_t newTime;
	doYield = false;
	while(!doYield) {
		newTime = clock();
		dtime = (float)newTime - activeContext->lastTime;
		activeContext->lastTime = newTime;

		activeContext->nextBlock = interpret(activeContext->nextBlock, activeContext->stack, NULL, 1);
		strpool_empty(); // free strings allocated to during evaluation

		while(activeContext->nextBlock == NULL) {
			if(dynarray_len(activeContext->nextStacks) != 0)
				popStackFrame();
			else
				return true;
		}
	}
	return false;
}

bool stepThreads(void) {
	doRedraw = false;
	ThreadContextList *last = &contexts;
	ThreadContextList *current = contexts.next;
	clock_t startTime = clock(),
		currentTime = startTime;
	do {
		// step each thread
		while(current != NULL) {
			printf("--thread\n");
			activeContext = current->context; // set the active context
			// step the thread
			if(stepActiveThread()) { // if the thread should be killed
				current = current->next; // advance to the next context
				free(last->next); // free the context of the stopped thread
				last->next = current; // make linked skip over freed context
			}
			else {
				last = current; // record the current context
				current = current->next; // advance to the next context
			}
		}
		// get the current time, check if a redraw needs to be done, and repeat
		currentTime = clock();
	} while(currentTime - startTime < workTime && doRedraw == false);
	return false;
}

// an array of thread contexts that should be used to start new threads when the green flag is clicked
static ThreadContext *contextsForGreenFlag;
// an array to the first blocks after the green flag hats
static const Block **stacksForGreenFlag;
static uint16 nContextsForGreenFlag;

void setContextsForGreenFlags(ThreadContext *const threadContexts, uint16 nContexts) {
	contextsForGreenFlag = threadContexts;
	nContextsForGreenFlag = nContexts;
	stacksForGreenFlag = malloc(sizeof(Block*)*nContextsForGreenFlag);
	if(stacksForGreenFlag == NULL)
		printf("[FATAL]Out of memory\n\tCould not allocate array of pointers to green flag scripts.\n");
	for(uint16 i = 0; i < nContextsForGreenFlag; ++i) {
		stacksForGreenFlag[i] = contextsForGreenFlag[i].nextBlock;
	}
}

void freeContextsForGreenFlagArray(void) {
	free(contextsForGreenFlag);
	free(stacksForGreenFlag);
}

void restartThreadsForGreenFlag(void) {
	for(uint16 i = 0; i < nContextsForGreenFlag; ++i) {
		contextsForGreenFlag[i].nextBlock = stacksForGreenFlag[i];
		contextsForGreenFlag[i].counters.slotsUsed = 0;
		startThread(contextsForGreenFlag+i); // don't worry about it already being started, because all threads should have been stopped before this function was called
	}
}
