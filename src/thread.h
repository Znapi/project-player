#pragma once

#include "types/thread.h"

extern void threadContext_init(ThreadContext *const context, const struct Block *const topBlock);
extern void threadContext_done(ThreadContext *const context);
extern void threadContext_reset(ThreadContext *const context);

extern void threadList_init(ThreadList *const threadList, const uint16 nThreads);
extern ThreadList* threadList_new(const uint16 nThreads);
static inline void threadList_done(ThreadList *const threadList) {
	free(threadList->array);
}
static inline void threadList_free(ThreadList *const threadList) {
	threadList_done(threadList);
	free(threadList);
}

extern void threadList_push(ThreadList **const head, ThreadList *const element);
extern void threadList_remove(ThreadList **const head, ThreadList *const element);

extern void threadList_copyArray(ThreadList *const newList, const ThreadList *const oldList, ThreadLink *const newThreads, const ThreadLink *const oldThreads);
extern ThreadList* threadList_copy(ThreadList *const oldList, ThreadLink *const newThreads, const ThreadLink *const oldThreads);

#define THREADLINK_FOR_EACH(/* ThreadLink *threadLink, ThreadLink *next, ThreadLink stubLink */ threadLink, stubLink) \
	for((threadLink) = (stubLink).next; (threadLink) != NULL; (threadLink) = (threadLink)->next)

/* deletion safe version */
#define THREADLINK_FOR_EACH_SAFE(/* ThreadLink *threadLink, ThreadLink *next, ThreadLink *tmp, ThreadLink stubLink */ threadLink, tmp, stubLink) \
	for((threadLink) = (stubLink).next, (tmp) = (threadLink == NULL) ? NULL : (threadLink)->next; \
			(threadLink) != NULL;																							\
			(threadLink) = (tmp), (tmp) = (threadLink == NULL) ? NULL : (threadLink)->next)

#define THREADLIST_ITER(/* ThreadList *head, ThreadList *threadList */ head, threadList)	\
	for((threadList) = (head); (threadList) != NULL; (threadList) = (threadList)->next)

/* TODO: put this documentation somewhere else
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
