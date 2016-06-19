#pragma once

struct SpriteContext {
	struct ThreadLink *threads; // array of thread contexts
	uint16 nThreads;

	struct Variable *variables; // a hash table of Scratch variables
	struct List *lists; // a hash table of Scratch lists

	struct Block *const *procedures; // table of pointers to procedures to be accessed with hashes
	const uint16 *nProcedureArgs;
	cmph_t *proceduresMphf;

	int16 xpos, ypos, direction;
	uint16 layer, costumeNumber, size,
		volume, tempo;
	struct {
		int16 color, brightness, ghost,
			pixelate, mosaic,
			fisheye, whirl;
	} effects;
};
typedef struct SpriteContext SpriteContext;
