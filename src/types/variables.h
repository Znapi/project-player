#pragma once

#include "../ut/uthash.h" // don't worry about unused functionality, it is all macros, so it only affects the pre-processing stage

struct Variable {
	struct Value value;
	const char *name;
	UT_hash_handle hh;
};
typedef struct Variable Variable;

struct ListElement {
	struct Value value;
	struct ListElement *next;
};
typedef struct ListElement ListElement;

struct List {
	struct ListElement *first; // a linked list of list elements
	struct ListElement *last;
	uint32 length;
	const char *name;
	UT_hash_handle hh;
};
typedef struct List List;
// TODO: switch lists to dynarrays
