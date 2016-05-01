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

struct Str_Link {
	char *str;
	struct Str_Link *next;
};
static struct Str_Link *ptr_list_head = NULL;

/* allocates a string that doesn't make the caller responsible for freeing it, it's pointer is recorded and is freed automatically */
char* allocString(uint32 length) {
	char *str = malloc(length);

	if(ptr_list_head == NULL) {
		ptr_list_head = malloc(sizeof(struct Str_Link));
		ptr_list_head->next = NULL;
	}
	else {
		struct Str_Link *previousItem = ptr_list_head;
		ptr_list_head = malloc(sizeof(struct Str_Link));
		ptr_list_head->next = previousItem;
	}
	//printf("ALLOC: %p %p\n", ptr_list_head, str); // used in manually verifying that the right amount of frees was being done

	ptr_list_head->str = str;

	return str;
}

/* frees all strings that have been allocated with allocString */
void freeStrings(void) {
	struct Str_Link *current = ptr_list_head;

	if(current != NULL) {
		struct Str_Link *next = current->next;
		do {
			next = current->next;
			//printf("FREE:  %p %p\n", current, current->str);
			free(current->str);
			free(current);
			current = next;
		} while(current != NULL);
	}

	ptr_list_head = NULL;
}
