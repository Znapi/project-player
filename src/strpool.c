/**
	String Pool
			strpool.h

	This string "pool" is for allocating strings to be freed all at once.
	The name is not the best.

	Currently, this is used only for strings allocated during evaluation of
	a single block's arguments.
**/

#include "types/primitives.h"

#include <string.h>
#include <stdlib.h>
//#include <stdio.h>

#include "strpool.h"

/* makes a copy of a string, but not in the pool */
char* extractString(const char *str) {
	size_t length = strlen(str);
	char *newString = malloc(length+1);
	memcpy(newString, str, length);
	newString[length] = '\0';
	return newString;
}

struct Link {
	char *str;
	struct Link *next;
};
static struct Link *listHead = NULL;

/* allocates a string that doesn't make the caller responsible for freeing it, it's pointer is recorded and is freed automatically */
char* strpool_alloc(uint32 length) {
	if(listHead == NULL) {
		listHead = malloc(sizeof(struct Link));
		listHead->next = NULL;
	}
	else {
		struct Link *previousItem = listHead;
		listHead = malloc(sizeof(struct Link));
		listHead->next = previousItem;
	}
	//printf("ALLOC: %p %p\n", ptr_list_head, str); // used in manually verifying that the right amount of frees was being done

	listHead->str = malloc(length);
	return listHead->str;
}

/* frees all strings that have been allocated with allocString */
void strpool_empty(void) {
	struct Link *next, *current = listHead;
	while(current != NULL) {
		next = current->next;
		//printf("FREE:  %p %p\n", current, current->str);
		free(current->str);
		free(current);
		current = next;
	}
	listHead = NULL;
}
