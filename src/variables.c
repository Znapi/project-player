/**
	Scratch Variables and Lists
	  variables.c

	This module handles the variables and lists in a Scratch project, and provides an
	interface for interacting with the data. Variables and lists are accessed by name
	through a hash table. The hash tables are implemented by Troy D. Hanson, and the
	implementation can be found in ut/uthash.h.

	Variables are stored by the Variable structure, which contains their value and the
	metadata that is part of the hash table. Lists are stored in the List structure,
	which also contains metadata, but the list length and pointers to the first and last
	elements of a linked list of ListElements. ListElements contain metadata for building
	the linked list and the value of the element.
	TODO: implement lists as dynamic arrays rather than linked lists.

	Interface

	The interface provides procedures that are convenient for the block functions to call in
	order to manipulate variables and lists. For that reason, the interface greatly
	resembles the Data blocks in Scratch used for manipulating variables and lists.

	The most important behavior to know when using this module's interface is that data
	passed to it, such as Values and strings for variable names, copied before it is stored
	(if it is stored). This is convenient when most data being passed is in temporary
	memory, like most of the data generated while evaluating Scratch blocks.

	Data returned (except for the hash tables) by the interface is also always copies of the
	actual data, because the actual data is only to be directly modified by this module.
**/

#include <stdio.h>

#include "types/primitives.h"
#include "types/value.h"
#include "types/variables.h"

#include "ut/uthash.h"
#include "value.h"
#include "variables.h"

#include "strpool.h"

const Value defaultValue = {{.floating = 0.0}, FLOATING};

/** Variables **/

/* Takes an already allocated Variable, initializes it, and adds it to the given hash table of Variables. */
void variable_init(Variable **variables, Variable *const variable, const char *const name, const Value *const value) {
	variable->name = extractString(name);
	if(value == NULL)
		variable->value = defaultValue;
	else
		variable->value = extractSimplifiedValue(value);
	HASH_ADD_STR(*variables, name, variable);
}

/* Same as variable_add except it allocates the new variable */
void variable_new(Variable **variables, const char *const name, const Value *const value) {
	Variable *const newVar = malloc(sizeof(Variable));
	if(newVar == NULL) {
		printf("[ERROR]Could not allocate new variable \"%s\"\n", name);
		return;
	}
	variable_init(variables, malloc(sizeof(Variable)), name, value);
}

void freeVariables(Variable **variables) {
	Variable *current, *tmp;
	HASH_ITER(hh, *variables, current, tmp) {
		HASH_DEL(*variables, current);
		if(current->value.type == STRING)
			free(current->value.data.string);
		free(current);
	}
}

/* returns true if the variable does not exist in the hash table */
bool setVariable(Variable **variables, const char *const name, const Value *const newValue) {
	Variable *var;
	HASH_FIND_STR(*variables, name, var);
	if(var == NULL)
		return true;
	var->value = extractSimplifiedValue(newValue);
	return false;
}

bool getVariable(Variable **variables, const char *const name, Value *const returnValue) {
	Variable *var;
	HASH_FIND_STR(*variables, name, var);
	if(var == NULL) {
		*returnValue = defaultValue;
		return true;
	}
	*returnValue = var->value;
	return false;
}

/* Lists */

void list_init(List **lists, List *const list, const char *const name) {
	list->name = extractString(name);
	list->length = 0;
	list->first = list->last = NULL;
	HASH_ADD_STR(*lists, name, list);
}

List* list_new(List **lists, const char *const name) {
	List *const list = malloc(sizeof(List));
	if(list == NULL) {
		printf("[ERROR]Could not allocate list \"%s\"\n", name);
		return NULL;
	}
	list_init(lists, list, name);
	return list;
}

static inline void freeListElement(ListElement *listElement) {
	if(listElement->value.type == STRING)
		free(listElement->value.data.string);
	free(listElement);
}

void freeLists(List **lists) {
	List *current, *tmp;
	ListElement *currente, *nexte;
	HASH_ITER(hh, *lists, current, tmp) { // delete each list from the hash table
		HASH_DEL(*lists, current);

		nexte = current->first;
		while(nexte != NULL) { // free each list element
			currente = nexte;
			nexte = currente->next;
			freeListElement(currente);
		}

		//printf("FREE:   %p\n", current);
		free(current); // free each list
	}
}

bool getListPtr(List **lists, const char *const name, List **const returnList) {
	List *list;
	HASH_FIND_STR(*lists, name, list);
	if(list == NULL)
		return true;
	*returnList = list;
	return false;
}

Value listGetFirst(const List *const list) {
	if(list->first == NULL)
		return defaultValue;
	else
		return list->first->value;
}

Value listGetLast(const List *const list) {
	if(list->last == NULL)
		return defaultValue;
	else
		return list->last->value;
}

static inline ListElement* findListElement(const List *const list, const uint32 index) {
	uint32 i = 0;
	ListElement *element = list->first;
	//printf("%p %i\n", element, i);
	while(i < index && element != NULL) {
		++i;
		element = element->next;
		//printf("%p %i\n", element, i);
	}
	return element;
}

Value listGet(const List *const list, const uint32 index) {
	ListElement *element = findListElement(list, index);
	if(element == NULL)
		return defaultValue;
	else
		return element->value;
}

void listAppend(List *list, const Value *const value) {
	ListElement *newElement = malloc(sizeof(ListElement));
	//if(newElement == NULL)
	//puts("FALILED");
	newElement->value = extractSimplifiedValue(value);
	newElement->next = NULL;
	if(list->length == 0)
		list->first = newElement;
	else
		list->last->next = newElement;
	list->last = newElement;
	if(list->length != UINT32_MAX)
		++list->length;
}

void listPrepend(List *list, const Value *const value) {
	ListElement *newElement = malloc(sizeof(ListElement));
	//if(newElement == NULL)
	//puts("FALILED");
	newElement->value = extractSimplifiedValue(value);
	newElement->next = list->first;
	list->first = newElement;
	if(list->length != UINT32_MAX)
		++list->length;
}

void listInsert(List *list, const Value *const value, const uint32 index) {
	ListElement *previous = findListElement(list, index-1); // index-1 is why there is a special case for index=0
	if(previous == NULL) // don't add the element if it is out of bounds of the list
		return;
	ListElement *newElement = malloc(sizeof(ListElement));
	newElement->value = extractSimplifiedValue(value);
	newElement->next = previous->next;
	previous->next = newElement;
	if(list->length != UINT32_MAX)
		++list->length;
}

void listSetFirst(List *list, const Value *const newValue) {
	ListElement *element = list->first;
	if(element == NULL)
		return;
	else
		element->value = extractSimplifiedValue(newValue);
}

void listSetLast(List *list, const Value *const newValue) {
	ListElement *element = list->last;
	if(element == NULL)
		return;
	else
		element->value = extractSimplifiedValue(newValue);
}

void listSet(List *list, const Value *const newValue, const uint32 index) {
	ListElement *element = findListElement(list, index);
	if(element == NULL)
		return;
	else
		element->value = extractSimplifiedValue(newValue);
}

void listDeleteFirst(List *list) {
	if(list->first != NULL) {
		ListElement *first = list->first;
		list->first = list->first->next;
		freeListElement(first);
		if(list->first == NULL) // if the last element was freed, then list->last needs to be NULL
			list->last = NULL;
		if(list->length != UINT32_MAX)
			--list->length;
	}
}

void listDeleteLast(List *list) {
	if(list->first != NULL) {
		if(list->length == 1) { // if there is no elements before it
			freeListElement(list->last);
			list->first = list->last = NULL;
		}
		else {
			ListElement *previous = findListElement(list, list->length-2);
			ListElement *last = list->last;
			previous->next = NULL;
			list->last = previous;
			freeListElement(last);
		}
		if(list->length != UINT32_MAX)
			--list->length;
	}
}

void listDelete(List *list, const uint32 index) {
	if(list->first != NULL) {
		if(index == 1)
			listDeleteFirst(list);
		else {
			ListElement *previous = findListElement(list, index-2);
			if(previous == NULL)
				return;
			ListElement *element = previous->next;
			if(element == NULL)
				return;
			previous->next = element->next;
			if(previous->next == NULL)
				list->last = previous;
			freeListElement(element);
			if(list->length != UINT32_MAX)
				--list->length;
		}
	}
}

void listDeleteAll(List *list) {
	ListElement *current, *next = list->first;
	while(next != NULL) {
		current = next;
		next = current->next;
		freeListElement(current);
	}
	list->first = list->last = NULL;
	list->length = 0;
}

bool listContainsFloating(const List *const list, const double floating) {
	uint32 i;
	ListElement *element = list->first;

	for(i = 0; i < list->length; ++i) {
		if(element->value.type == FLOATING) { // based on the fact that before a value is stored, it is simplified to a float if possible, so the only strings will be ones that can't be numbers
			if(element->value.data.floating == floating)
				return true;
		}
		element = element->next;
	}
	return false;
}

bool listContainsBoolean(const List *const list, const bool boolean) {
	uint32 i;
	ListElement *element = list->first;

	for(i = 0; i < list->length; ++i) {
		if(element->value.type == BOOLEAN) {
			if(element->value.data.boolean == boolean)
				return true;
		}
		element = element->next;
	}
	return false;
}

bool listContainsString(const List *const list, const char *const string) {
	uint32 i;
	ListElement *element = list->first;

	for(i = 0; i < list->length; ++i) {
		if(element->value.type == STRING) {
			if(strcasecmp(element->value.data.string, string))
				return true;
		}
		element = element->next;
	}
	return false;
}
