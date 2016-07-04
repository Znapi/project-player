#pragma once

enum SpriteScope {
	STAGE,
	SPRITE,
	CLONE
};

struct SpriteContext {
	const char *name;
	enum SpriteScope scope;

	struct ThreadLink *threads; // array of thread contexts
	uint16 nThreads;

	struct Variable *variables; // a hash table of Scratch variables
	struct List *lists; // a hash table of Scratch lists

	struct ProcedureLink *procedureHashTable; // table of pointers to procedures to be accessed with hashes
	struct ThreadList *broadcastHashTable; // table of pointers to arrays of broadcast threads
	struct ThreadList whenClonedThreads; // TODO: don't need a  ThreadList for this

	double xpos, ypos, direction,
		size, volume, tempo;
	struct {
		double color, brightness, ghost,
			pixelate, mosaic,
			fisheye, whirl;
	} effects;
};
typedef struct SpriteContext SpriteContext;
