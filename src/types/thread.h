#pragma once

#include <time.h>

/* individual threads */

struct BlockStackFrame {
	uint16 level; // level of nesting in script, not total thread
	const struct Block *nextBlock;
};

struct ThreadContext {
	dynarray stack; // thread's stack / argument pool

	const struct Block *topBlock;
	struct BlockStackFrame frame;
	dynarray blockStack; // dynarray of BlockStackFrames

	clock_t lastTime;

	dynarray tmp; // dynarray of struct TmpDatas

	struct Value *parameters; // custom block parameters (just a pointer into the parametersStack)
	dynarray parametersStack; // dynarray of Values.
	dynarray nParametersStack; // dynarray of Values.
};
typedef struct ThreadContext ThreadContext;

/* doubly linked list of threads */
/* used for creating linked list of running threads */
struct ThreadLink {
	struct ThreadContext thread;
	struct SpriteContext *sprite;
	struct ThreadLink *next, *prev;
};
typedef struct ThreadLink ThreadLink;

/* doubly linked list of arrays of pointers ThreadLinks */
/* used for making collections of all threads for a certain hat type, where each array of
	 threads is for a different sprite */
struct ThreadList {
	struct ThreadLink **array;
	uint16 nThreads;
	struct ThreadList *prev, *next;
};
typedef struct ThreadList ThreadList;
