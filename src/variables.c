/**
	Scratch Variables and Lists
	  variables.c

	This module handles the variables and lists in a Scratch project, and provides an
	interface for interacting with the data. Variables and lists are accessed by name
	through a hash table. The hash tables are implemented by Troy D. Hanson, and the
	implementation can be found in ut/uthash.h.

	Variables are stored by the Variable structure, which contains their value and the
	metadata that is part of the hash table. Lists are stored in the List structure, which
	also contains metadata, and the contents are stored as a dynarray of Values.

	Interface

	The interface provides procedures that are convenient for the block functions to call in
	order to manipulate variables and lists. For that reason, the interface greatly
	resembles the Data blocks in Scratch used for manipulating variables and lists.

	The most important behavior to know when using this module's interface is that data
	passed to it, such as Values and strings for variable names, copied before it is stored
	(if it is stored). This is convenient when most data being passed is in temporary
	memory, like most of the data generated while evaluating Scratch blocks.
**/

#include <stdio.h>
#include "ut/uthash.h"
#include "ut/utarray.h"

#include "types/primitives.h"
#include "types/value.h"
#include "types/variables.h"

#include "value.h"
#include "variables.h"

#include "strpool.h"

const Value defaultValue = {{.floating = 0.0}, FLOATING};

static void value_copy(Value *dst, Value *src) {
	if(src->type == STRING) {
		dst->type = STRING;
		size_t l = (strlen(src->data.string)+1)*sizeof(char);
		dst->data.string = malloc(l);
		memcpy(dst->data.string, src->data.string, l);
	}
	else
		memcpy(dst, src, sizeof(Value));
}

static void value_dtor(Value *v) {
	if(v->type == STRING)
		free(v->data.string);
}

/** Variables **/

/* Takes an already allocated Variable, initializes it, and adds it to the given hash table of Variables. */
void variable_init(Variable **variables, Variable *const variable, const char *const name, const size_t nameLen, const Value *const value) {
	variable->name = extractString(name, (size_t*)&nameLen);
	if(value == NULL)
		variable->value = defaultValue;
	else
		variable->value = extractSimplifiedValue(value);
	HASH_ADD_KEYPTR(hh, *variables, variable->name, nameLen, variable);
}

/* Same as variable_add except it allocates the new variable */
void variable_new(Variable **variables, const char *const name, const size_t nameLen, const Value *const value) {
	Variable *const newVar = malloc(sizeof(Variable));
	if(newVar == NULL) {
		printf("[ERROR]Could not allocate new variable \"%s\"\n", name);
		return;
	}
	variable_init(variables, malloc(sizeof(Variable)), name, nameLen, value);
}

void freeVariables(Variable **variables) {
	Variable *current, *tmp;
	HASH_ITER(hh, *variables, current, tmp) {
		HASH_DEL(*variables, current);
		value_dtor(&current->value);
		free(current);
	}
}

Variable* copyVariables(const Variable *const *const variables) {
	Variable *newVars = NULL;
	const Variable *src = *variables;
	for(uint16 i = HASH_COUNT(*variables); i != 0; --i) {
		Variable *new = malloc(sizeof(Variable));
		size_t len = 0;
		new->name = extractString(src->name, &len);
		new->value = extractValue(&src->value);
		HASH_ADD_KEYPTR(hh, newVars, new->name, len, new);
		src = src->hh.next;
	}
	return newVars;
}

/* returns true if the variable does not exist in the hash table */
bool setVariable(Variable **variables, const char *const name, const Value *const newValue) {
	Variable *var;
	HASH_FIND_STR(*variables, name, var);
	if(var == NULL)
		return true;
	value_dtor(&var->value);
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

static UT_icd value_icd = {sizeof(Value), NULL, (ctor_f*)value_copy, (dtor_f*)value_dtor};

void list_init(List **lists, List *const list, const char *const name, size_t nameLen) {
	list->name = extractString(name, &nameLen);
	utarray_init(&list->contents, &value_icd);
	HASH_ADD_KEYPTR(hh, *lists, list->name, nameLen, list);
}

UT_array* list_new(List **lists, const char *const name, const size_t nameLen) {
	List *const list = malloc(sizeof(List));
	if(list == NULL) {
		printf("[ERROR]Could not allocate list \"%s\"\n", name);
		return NULL;
	}
	list_init(lists, list, name, nameLen);
	return &list->contents;
}

void freeLists(List **lists) {
	List *list, *tmp;
	HASH_ITER(hh, *lists, list, tmp) { // delete each list from the hash table
		utarray_done(&list->contents);
		//printf("FREE:   %p\n", current);
		HASH_DEL(*lists, list);
		free(list); // free each list
	}
}

List* copyLists(const List *const *const lists) {
	List *newLists = NULL;
	const List *src = *lists;
	for(uint16 i = HASH_COUNT(*lists); i != 0; --i) {
		List *new = malloc(sizeof(List));
		size_t nameLen = 0;
		new->name = extractString(src->name, &nameLen);
		utarray_init(&new->contents, &value_icd);
		utarray_inserta(&new->contents, &src->contents, 0);
		HASH_ADD_KEYPTR(hh, newLists, new->name, nameLen, new);
		src = src->hh.next;
	}
	return newLists;
}

bool getListContents(List **lists, const char *const name, UT_array **const returnContents) {
	List *list;
	HASH_FIND_STR(*lists, name, list);
	if(list == NULL)
		return true;
	*returnContents = &list->contents;
	return false;
}

Value listGetFirst(const UT_array *const list) {
	if(utarray_len(list) == 0)
		return defaultValue;
	else
		return *(Value*)utarray_front(list);
}

Value listGetLast(const UT_array *const list) {
	if(utarray_len(list) == 0)
		return defaultValue;
	else
		return *(Value*)utarray_back(list);
}

Value listGet(const UT_array *const list, const uint32 index) {
	if(utarray_len(list) > index)
		return *(Value*)utarray_eltptr(list, index);
	else
		return defaultValue;
}

void listAppend(UT_array *list, const Value *const value) {
	utarray_push_back(list, (void*)value);
}

void listPrepend(UT_array *list, const Value *const value) {
	utarray_insert(list, (void*)value, 0);
}

void listInsert(UT_array *list, const Value *const value, const uint32 index) {
	utarray_insert(list, (void*)value, index);
}

void listSetFirst(UT_array *list, const Value *const newValue) {
	Value *elt = (Value*)utarray_front(list);
	value_dtor(elt);
	*elt = *newValue;
}

void listSetLast(UT_array *list, const Value *const newValue) {
	Value *elt = (Value*)utarray_back(list);
	value_dtor(elt);
	*elt = *newValue;
}

void listSet(UT_array *list, const Value *const newValue, const uint32 index) {
	Value *elt = (Value*)utarray_eltptr(list, index);
	value_dtor(elt);
	*elt = *newValue;
}

void listDeleteFirst(UT_array *list) {
	utarray_erase(list, 0, 1);
}

void listDeleteLast(UT_array *list) {
	utarray_pop_back(list);
}

void listDelete(UT_array *list, const uint32 index) {
	utarray_erase(list, index, 1);
}

void listDeleteAll(UT_array *list) {
	utarray_clear(list);
}

bool listContainsFloating(const UT_array *const list, const double floating) {
	for(uint32 i = 0; i < utarray_len(list); ++i) {
		Value *v = (Value*)utarray_eltptr(list, i);
		if(v->type == FLOATING) { // based on the fact that before a value is stored, it is simplified to a float if possible, so the only strings will be ones that can't be numbers
			if(v->data.floating == floating)
				return true;
		}
	}
	return false;
}

bool listContainsBoolean(const UT_array *const list, const bool boolean) {
	for(uint32 i = 0; i < utarray_len(list); ++i) {
		Value *v = (Value*)utarray_eltptr(list, i);
		if(v->type == BOOLEAN) {
			if(v->data.boolean == boolean)
				return true;
		}
	}
	return false;
}

bool listContainsString(const UT_array *const list, const char *const string) {
	for(uint32 i = 0; i < utarray_len(list); ++i) {
		Value *v = (Value*)utarray_eltptr(list, i);
		if(v->type == STRING) {
			if(strcmp(v->data.string, string) == 0)
				return true;
		}
	}
	return false;
}
