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
#include <stdio.h>

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
	struct Link *previousItem = listHead;
	/*puts("---->");
	previousItem = listHead;
	while(previousItem != NULL) {
		printf("\t%p %p  \"%s\"\n", previousItem, previousItem->str, previousItem->str);
		previousItem = previousItem->next;
	}
	puts("\t----");
	previousItem = listHead;*/

	if(listHead == NULL) {
		listHead = malloc(sizeof(struct Link));
		listHead->next = NULL;
	}
	else {
		listHead = malloc(sizeof(struct Link));
		listHead->next = previousItem;
	}
	if(listHead == NULL) {
		puts("[ERROR]Could not allocate link in strpool.");
		return NULL;
	}

	listHead->str = malloc(length);
	if(listHead->str == NULL)
		puts("[ERROR]Could not allocate string in strpool.");

	/*previousItem = listHead;
	printf("\t%p %p  <no string>\n", previousItem, previousItem->str);
	previousItem = listHead->next;
	while(previousItem != NULL) {
		printf("\t%p %p  \"%s\"\n", previousItem, previousItem->str, previousItem->str);
		previousItem = previousItem->next;
	}
	puts("<----");*/
	//printf("ALLOC: %p %p\n", listHead, listHead->str); // used in manually verifying that the right amount of frees was being done
	
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
