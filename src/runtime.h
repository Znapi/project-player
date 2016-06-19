#pragma once

extern struct ThreadContext createThreadContext(const struct Block *const block);
extern void freeThreadContext(const struct ThreadContext *const context);

extern bool stepThreads(void);

extern void setGreenFlagThreads(struct ThreadLink *const *const threadContexts, const uint16 amount);
extern void freeGreenFlagThreads(void);
extern void restartGreenFlagThreads(void);
