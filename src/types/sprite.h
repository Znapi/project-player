#pragma once

/**
	Sprites

	All of the data that makes up a project is organized into sprites. The stage and clones
	also count as sprites, to make things simpler.

	Other than the data inside the struct SpriteContext, each sprite also owns it's array of
	ThreadLinks, variables, lists, and ThreadList of "when cloned" threads. Each sprite must
	have a unique copy of this data, and it must be freed with the sprite.

	Only the Stage and parent sprites get a unique `name` and `procedureHashTable`, and it
	is freed with the sprite. Clones simply share the same name as their parent, and must
	not free `name` or `procedureHashTable`.
**/

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
	struct ThreadList whenClonedThreads; // TODO: don't need a  ThreadList for this
	struct ThreadList **broadcastThreadLists; // array of pointers to ThreadLists for broadcast threads
	uint16 nBroadcastThreadLists;

	double xpos, ypos, direction, size;
	struct {
		double color, brightness, ghost,
			pixelate, mosaic,
			fisheye, whirl;
	} effects;

	const struct Costume *costumes;
	const struct Costume *currentCostume;
};
typedef struct SpriteContext SpriteContext;

struct SpriteLink {
	struct SpriteContext context;
	UT_hash_handle hh;
};
