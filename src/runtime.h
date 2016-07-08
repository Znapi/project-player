#pragma once

struct BroadcastThreads {
	struct ThreadList *threadList;
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

extern void setVolume(const double newVolume);
extern void setTempo(const double newTempo);

extern void setStage(struct SpriteContext *const stage);
extern void setSprites(struct SpriteLink *const sprites);

extern bool stepThreads(void);

extern void setGreenFlagThreads(struct ThreadLink *const *const threadContexts, const uint16 amount);
extern void freeGreenFlagThreads(void);
extern void restartGreenFlagThreads(void);

extern void setBroadcastsHashTable(struct BroadcastThreads *const hashTable);
extern void freeBroadcastHashTable(void);
