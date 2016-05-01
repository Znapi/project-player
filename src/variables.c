#include "types/primitives.h"
#include "types/value.h"
#include "types/sprite.h"

#include "variables.h"
#include "ut/uthash.h"

/* Variables */

Variable* createVariable(Variable **variables, const char *const name, Value value) {
	Variable *newVar = malloc(sizeof(Variable));
	newVar->name = name;
	newVar->value = value;
	HASH_ADD_STR(*variables, name, newVar);
	return newVar;
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

void setVariable(Variable **variables, const char *const name, Value newValue) {
	Variable *var;
	HASH_FIND_STR(*variables, name, var);
	if(var == NULL)
		createVariable(variables, name, newValue);
	else {
		var->name = name;
		var->value = newValue;
	}
}

Value getVariable(Variable **variables, const char *name) {
	Variable *var;
	HASH_FIND_STR(*variables, name, var);
	if(var == NULL) {
		const Value defaultValue = {{.floating = 0.0}, FLOATING};
		createVariable(variables, name, defaultValue);
		return defaultValue;
	}
	else
		return var->value;
}

/* Lists */

List* createList(List **lists, const char *name, ListElement *elements, uint32 length) {
	List *list = malloc(sizeof(List));;
	if(list == NULL)
		return NULL;
	list->name = name;
	list->length = length;
	if(elements == NULL)
		list->first = list->last = NULL;
	else {
		list->first = elements;
		uint32 i;
		ListElement *c = elements;
		for(i = 0; i < length; ++i) // find the last element
			c = elements->next;
		list->last = c;
	}
	HASH_ADD_STR(*lists, name, list);
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

List* getListPtr(List **lists, const char *name) {
	List *list;
	HASH_FIND_STR(*lists, name, list);
	if(list == NULL) {
		createList(lists, name, NULL, 0);
		return getListPtr(lists, name);
	}
	else
		return list;
}

const Value zeroValue = {{0.0}, FLOATING};

Value getFirstListElement(const List *list) {
	if(list->first == NULL)
		return zeroValue;
	else
		return list->first->value;
}

Value getLastListElement(const List *list) {
	if(list->last == NULL)
		return zeroValue;
	else
		return list->last->value;
}

static inline ListElement* findListElement(const List *list, uint32 index) {
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

Value getListElement(const List *list, uint32 index) {
	ListElement *element = findListElement(list, index);
	if(element == NULL)
		return zeroValue;
	else
		return element->value;
}

void listAppend(List *list, Value value) {
	ListElement *newElement = malloc(sizeof(ListElement));
	//if(newElement == NULL)
	//puts("FALILED");
	newElement->value = value;
	newElement->next = NULL;
	if(list->length == 0)
		list->first = newElement;
	else
		list->last->next = newElement;
	list->last = newElement;
	if(list->length != UINT32_MAX)
		++list->length;
}

void listPrepend(List *list, Value value) {
	ListElement *newElement = malloc(sizeof(ListElement));
	//if(newElement == NULL)
	//puts("FALILED");
	newElement->value = value;
	newElement->next = list->first;
	list->first = newElement;
	if(list->length != UINT32_MAX)
		++list->length;
}

void listInsert(List *list, Value value, uint32 index) {
	if(index == 0) { // could leave this out, and leave it up to the caller to choose listPrepend when index=0
		listPrepend(list, value);
		return;
	}

	ListElement *previous = findListElement(list, index-1); // index-1 is why there is a special case for index=0
	if(previous == NULL) // don't add the element if it is out of bounds of the list
		return;
	ListElement *newElement = malloc(sizeof(ListElement));
	newElement->value = value;
	newElement->next = previous->next;
	previous->next = newElement;
	if(list->length != UINT32_MAX)
		++list->length;
}

void listSetFirst(List *list, Value newValue) {
	ListElement *element = list->first;
	if(element == NULL)
		return;
	else
		element->value = newValue;
}

void listSetLast(List *list, Value newValue) {
	ListElement *element = list->last;
	if(element == NULL)
		return;
	else
		element->value = newValue;
}

void listSet(List *list, Value newValue, uint32 index) {
	ListElement *element = findListElement(list, index);
	if(element == NULL)
		return;
	else
		element->value = newValue;
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

void listDelete(List *list, uint32 index) {
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

bool listContainsFloating(const List *list, double floating) {
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

bool listContainsBoolean(const List *list, bool boolean) {
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

bool listContainsString(const List *list, const char *string) {
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
