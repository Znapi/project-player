/**
	Threads
	  thread.c

	This moudle implements general functions for creating and destroying ThreadContexts,
	ThreadLink, and ThreadLists. It also includes adding/deleting/iterating linked lists of
	ThreadLists and resetting ThreadContexts.

	Adding/deleting/interating for ThreadLinks is not implemented because those functions
	are specially implemented by the runtime as stop/start threads functions.

	ThreadContexts should never be stored alone, but should always be embedded in a
	ThreadLink. Hence, this module does not malloc or free ThreadContexts.
**/

#include "ut/dynarray.h"

#include "types/primitives.h"
#include "types/value.h"

#include "thread.h"

void threadContext_init(ThreadContext *const context, const struct Block *const topBlock) {
	context->topBlock = topBlock;
	dynarray_init(&context->stack, sizeof(Value));
	dynarray_init(&context->blockStack, sizeof(struct BlockStackFrame));
	dynarray_init(&context->tmp, sizeof(Value));
	dynarray_init(&context->parametersStack, sizeof(Value));
	dynarray_init(&context->nParametersStack, sizeof(uint16));
}

void threadContext_done(ThreadContext *const context) {
	dynarray_done(&context->stack);
	dynarray_done(&context->blockStack);
	dynarray_done(&context->tmp);
	dynarray_done(&context->parametersStack);
	dynarray_done(&context->nParametersStack);
}

void threadContext_reset(ThreadContext *const context) {
	dynarray_clear(&context->stack);
	context->frame.level = 0;
	context->frame.nextBlock = NULL;
	dynarray_clear(&context->blockStack);
	dynarray_clear(&context->tmp);
	context->parameters = NULL;
	dynarray_clear(&context->parametersStack);
	dynarray_clear(&context->nParametersStack);
}

void threadList_init(ThreadList *const threadList, const uint16 nThreads) {
	threadList->array = calloc(nThreads, sizeof(ThreadLink*));
	threadList->nThreads = nThreads;
}

ThreadList* threadList_new(const uint16 nThreads) {
	ThreadList *new = malloc(sizeof(ThreadList));
	threadList_init(new, nThreads);
	return new;
}

void threadList_push(ThreadList **const head, ThreadList *const element) {
	element->prev = NULL;
	if(*head == NULL) {
		*head = element;
		element->next = NULL;
	}
	else {
		(*head)->prev = element;
		element->next = *head;
		*head = element;
	}
}

void threadList_insert(ThreadList *const existingElement, ThreadList *const newElement) {
	if(existingElement->next != NULL) {
		existingElement->next->prev = newElement;
	}
	newElement->next = existingElement->next;

	newElement->prev = existingElement;
	existingElement->next = newElement;
}

void threadList_remove(ThreadList **const head, ThreadList *const element) {
	if(head != NULL) {
		if(*head == element) {
			*head = element->next;
			if(element->next != NULL) // TODO: fix control flow
				element->next->prev = NULL;
			return;
		}
	}

	if(element->prev != NULL)
		element->prev->next = element->next;
	if(element->next != NULL)
		element->next->prev = element->prev;
}

void threadList_copyArray(ThreadList *const newList, const ThreadList *const oldList, ThreadLink *const newThreads, const ThreadLink *const oldThreads) {
	if(newList->nThreads != 0) {
		newList->array = malloc(newList->nThreads*sizeof(ThreadLink*));
		for(uint16 i = 0; i < newList->nThreads; ++i) {
			newList->array[i] = newThreads + (long)(oldList->array[i] - oldThreads);
		}
	}
}

ThreadList* threadList_copy(ThreadList *const oldList, ThreadLink *const newThreads, const ThreadLink *const oldThreads) {
	ThreadList *new = malloc(sizeof(ThreadList));
	new->nThreads = oldList->nThreads;
	threadList_copyArray(new, oldList, newThreads, oldThreads);
	return new;
}
