#pragma once

struct SpriteLink {
	struct SpriteContext context;
	UT_hash_handle hh;
};

struct BroadcastThreads {
	dynarray *threads; // dynamic array of pointers to ThreadLinks
	char *msg;
	struct BroadcastThreads **nullifyOnRestart;
	UT_hash_handle hh;
};

struct ProcedureLink {
	Block *script;
	uint16 nParameters;
	char *label;
	UT_hash_handle hh;
};

extern const blockfunc opsTable[];

extern void initializeAskPrompt(void);

extern void setStage(struct SpriteContext *const stage);
extern void setSprites(struct SpriteLink *const sprites);

extern struct ThreadContext createThreadContext(const struct Block *const block);
extern void freeThreadContext(struct ThreadContext *const context);

extern bool stepThreads(void);

extern void setGreenFlagThreads(struct ThreadLink *const *const threadContexts, const uint16 amount);
extern void freeGreenFlagThreads(void);
extern void restartGreenFlagThreads(void);

extern void setBroadcastsHashTable(struct BroadcastThreads *const hashTable);
extern void freeBroadcastHashTable(void);
