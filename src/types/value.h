#pragma once

enum Type {FLOATING, STRING, BOOLEAN};

struct Value {
	union {
		uint32 integer; // used by procedure parameter
		double floating; // double precision floating point for projects needing high precision floating point numbers
		char *string; // pointer to a UTF-8 null-terminated string
		bool boolean; // boolean using C99 special boolean handling
	} data;
	enum Type type;
};
typedef struct Value Value;
