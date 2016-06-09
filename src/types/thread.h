#pragma once

//#include <time.h>              // not all modules need the functionality provided in time.h
#include <sys/_types/_clock_t.h> // instead, only define the type clock_t, which is all that is needed from time.h to compile this

union Counter {
	uint32 u;
	float f;
};

struct ThreadContext {
	struct Value *stack; // A pointer to this thread's stack / argument pool
	const struct Block *nextBlock; // pointer to next block[]

	dynarray *nextBlocks; // stack of blocks to advance to when current stack runs out

	clock_t lastTime;

	struct {
		union Counter *counters;
		const struct Block **owners;
		ufastest slotsUsed;
	} counters;

	struct SpriteContext *const spriteCtx; // where sprite/stage attributes/variables can be accessed

	//struct Variable **parameters; // custom block parameters
	//struct Variable **parametersOfPrevStacks; // if only Scratch had a proper scoping system...can't wait to see how Scratch 3 will handle it
};
typedef struct ThreadContext ThreadContext;

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
