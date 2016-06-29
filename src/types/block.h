#pragma once

typedef ubyte blockhash; // for asserting that a type is specifically a block hash
typedef const struct Block* (*blockfunc)(const struct Block *block, struct Value * const reportSlot, const struct Value arg[]); // block function pointer

// Internal representation of a block
struct Block {
	blockfunc func;
	ubyte level; // the level this block or constant argument is on
	union {
		struct Block *next; // if this is a stack block, this links to the next Block, NULL if it is the end of the stack
		const struct Value *value; // if this is a constant argument, this is a pointer to the Value of this argument
		struct Block **substacks; // if this is a control flow block (e.g. if/else), then this points to an array of pointers to possible substacks to jump to
	} p;
};
typedef struct Block Block;
