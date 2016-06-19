#pragma once

//#include <time.h>              // not all modules need the functionality provided in time.h
#include <sys/_types/_clock_t.h> // instead, only define the type clock_t, which is all that is needed from time.h to compile this

union Counter {
	uint32 u;
	float f;
};

struct BlockStackFrame {
	uint16 level; // level of nesting in script, not total thread
	const struct Block *nextBlock;
};

struct ThreadContext {
	struct Value *stack; // A pointer to this thread's stack / argument pool TODO: make this a dynarray

	const struct Block *const topBlock;
	struct BlockStackFrame frame;
	dynarray *blockStack; // dynarray of BlockStackFrames TODO: embed dynarray into ThreadContext

	clock_t lastTime;

	struct {
		union Counter *counters; // TODO: make this a dynarray
		const struct Block **owners;
		ufastest slotsUsed;
	} counters;

	struct Value *parameters; // custom block parameters
	dynarray *parametersStack; // dynarray of Values.
};
typedef struct ThreadContext ThreadContext;

struct ThreadLink {
	struct ThreadContext thread;
	struct SpriteContext *sprite;
	struct ThreadLink *next;
};
typedef struct ThreadLink ThreadLink;

/*
	Evaluating Blocks
	=================

	Here is an example of what a thread's stack looks like while evaluating a stack block and it's children:

			0a["", 1a["", 2a["", 3a], 2b_], 1b["", 2c_, 2d["", 3b_]], 1c_]
			3a_, 2a, 2b_, 1a, 2c_, 3b, 2d, 1b, 1c_
			3a 2a 2b 1a 2c 3b 2d 1b 1c

			|  |  |3a|
			|  |2a|  |
			|  |2a|2b|
			|1a|
			|1a|  |2c|
			|1a|  |2c|  |3b|
			|1a|  |2c|2d|
			|1a|1b|
			|1a|1b|1c|

	Here the example is extended for proof of concept:

			0a[1a["", 2a["", 3a["", 4a], 3b_], 2b["", 3c_, 3d["", 4b_]], 2c_], 1b_, 1c["", 2d["", 3e_, 3f_, 3g_], 2e["", 3h]]

			...
			|  |2a|2b|2c|
			|1a|
			|1a|1b|
			|1a|1b|  |  |3e|
			|1a|1b|  |  |3e|3f|
			|1a|1b|  |  |3e|3f|3g|
			|1a|1b|  |2d|
			|1a|1b|  |2d|  |3h|
			|1a|1b|  |2d|2e|
			|1a|1b|1c|
*/
