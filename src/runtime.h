#pragma once

extern ThreadContext createThreadContext(const Block *const block);
extern void freeThreadContext(const ThreadContext context);

extern bool stepThreads(void);

extern void setGreenFlagThreads(ThreadLink *const *const threadContexts, const uint16 amount);
extern void freeGreenFlagThreads(void);
extern void restartGreenFlagThreads(void);
