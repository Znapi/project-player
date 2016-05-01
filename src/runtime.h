#pragma once

extern ThreadContext createThreadContext(SpriteContext *const spriteContext, const Block *const block);
extern void freeThreadContext(const ThreadContext context);

extern bool stepThreads(void);

extern void setContextsForGreenFlags(ThreadContext *const threadContexts, const uint16 amount);
extern void freeContextsForGreenFlagArray(void);
extern void restartThreadsForGreenFlag(void);
