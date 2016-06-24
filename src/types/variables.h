#pragma once

#include "../ut/uthash.h" // don't worry about unused functionality, it is all macros, so it only affects the pre-processing stage

struct Variable {
	struct Value value;
	const char *name;
	UT_hash_handle hh;
};
typedef struct Variable Variable;

struct List {
	UT_array contents; // dynamic array of Values
	const char *name;
	UT_hash_handle hh;
};
typedef struct List List;
