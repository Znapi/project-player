#include "types/primitives.h"
#include "types/value.h"

#include "strpool.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "value.h"

// Return a new Value from the given value, and make a copy of a string if any.
Value copyValue(const Value value) {
	if(value.type == STRING) {
		Value copy = value;
		copy.data.string = extractString(value.data.string);
		return copy;
	}
	else
		return value;
}

// 'unsuccecssful' for tryTo functions means that data was lost from a string representation

/* Tries to convert the given string to a boolean. Returns true if successful and stores the resulting boolean in ret, false otherwise. */
static bool strTryToBoolean(const char *str, bool *ret) {
	if(str[0] == 't') {
		if(strcmp(str+1, "rue") == 0) {
			*ret = true;
			return true;
		}
		else
			return false;
	}
	else if(str[0] == 'f') {
		if(strcmp(str+1, "alse") == 0) {
			*ret = false;
			return true;
		}
		else
			return false;
	}
	return false;
}

static bool strnTryToBoolean(const char *str, const ubyte len, bool *ret) {
	if(str[0] == 't') {
		if(strncmp(str+1, "rue", len-1) == 0) {
			*ret = true;
			return true;
		}
		else
			return false;
	}
	else if(str[0] == 'f') {
		if(strncmp(str+1, "alse", len-1) == 0) {
			*ret = false;
			return true;
		}
		else
			return false;
	}
	return false;
}

/* Tries to convert the given string to a double, and stores the resulting double in ret. Returns true if successful, false otherwise. */
static bool strTryToFloating(const char *str, double *ret) {
	char *endptr;
	*ret = strtod(str, &endptr); // do a conversion to a number
	if(endptr - str != strlen(str)) // if data was lost
		return false;
	else
		return true;
}

static bool strnTryToFloating(const char *str, const ubyte len, double *ret) {
	char *endptr;
	*ret = strtod(str, &endptr); // do a conversion to a number
	if(endptr - str != len) // if data was lost
		return false;
	else
		return true;
}

static union {
	bool b;
	double f;
} t;

/* Tries to convert the given Value to a double. Returns true if successful and stores the resulting double in ret, false otherwise. */
bool tryToFloating(const Value value, double *ret) {
	switch(value.type) {
	case FLOATING:
		*ret = value.data.floating;
		return true;
	case BOOLEAN:
		*ret = (double)value.data.boolean;
		return true;
	case STRING:
		if(strTryToFloating(value.data.string, &t.f)) {
			*ret = t.f;
			return true;
		}
		else if(strTryToBoolean(value.data.string, &t.b)) {
			*ret = (double)t.b;
			return true;
		}
		else
			return false;
	}
}

/* takes a Value, and will return a 64 bit integer representation */
int64 toInteger(const Value value) {
	switch(value.type) {

	case FLOATING:
		/*if(value.data.floating == NAN) return 0;
			else*/ return (int64)value.data.floating;

	case STRING:
		if(strTryToBoolean(value.data.string, &t.b))
			return t.b;
		else
			return (int64)strtoll(value.data.string, NULL, 0);

	case BOOLEAN:
		return value.data.boolean;
	}
}

double toFloating(const Value value) {
	switch(value.type) {

	case FLOATING:
		/*if(value.data.floating == NAN)
			return 0;
			else*/
			return value.data.floating;

	case STRING:
		if(strTryToBoolean(value.data.string, &t.b))
			return t.b;
		else
			return strtod(value.data.string, NULL);

	case BOOLEAN:
		return (double)value.data.boolean;
	}
}

/* takes a Value, creates a string with allocString(will be auto freed), and return the length of the string */
uint32 toString(const Value value, char **string) {
	uint32 size;
	char buf[64];
	switch(value.type) {

	case FLOATING:
		sprintf(buf, "%f", value.data.floating);
		size = strlen(buf)+1;
		*string = allocString(size);
		memcpy(*string, buf, size);
		return size-1;

	case STRING:
		size = strlen(value.data.string)+1;
		*string = allocString(size);
		memcpy(*string, value.data.string, size);
		return size-1;

	case BOOLEAN:
		if(value.data.boolean) {
			*string = allocString(5);
			memcpy(*string, "true", 5*sizeof(char));
			return 4;
		}
		else {
			*string = allocString(6);
			memcpy(*string, "false", 6*sizeof(char));
			return 5;
		}
	}
}

bool toBoolean(const Value value) {
	switch(value.type) {
	case FLOATING:
		return value.data.floating == 1.0;

	case STRING:
		if(strTryToBoolean(value.data.string, &t.b))
			return t.b;
		else
			return strtoll(value.data.string, NULL, 0) == 1;

	case BOOLEAN:
		return value.data.boolean;
	}
}

Value strnToValue(const char *const string, const ubyte length) {
	Value copy;
	if(strnTryToFloating(string, length, &t.f)) {
		copy.type = FLOATING;
		copy.data.floating = t.f;
	}
	else if(strnTryToBoolean(string, length, &t.b)) {
		copy.type = BOOLEAN;
		copy.data.boolean = t.b;
	}
	else {
		copy.type = STRING;
		copy.data.string = malloc(length+1);
		memcpy(copy.data.string, string, length);
		copy.data.string[length] = '\0';
	}
	return copy;
}

/*Value jsonPrimToValue(const char *const string, const ubyte length) {
	Value v;
	switch(string[0]) {
	case 't':
		v.type = BOOLEAN;
		v.data.boolean = true;
		break;
	case 'f':
		v.type = BOOLEAN;
		v.data.boolean = false;
		break;
	case 'n':
		v.type = FLOATING;
		v.data.floating = 0.0;
		break;
	default:
		strnTryToFloating(...
	}
	}*/

Value copySimplifiedValue(const Value value) {
	if(value.type == STRING) {
		Value copy;
		if(strTryToFloating(value.data.string, &t.f)) {
			copy.type = FLOATING;
			copy.data.floating = t.f;
		}
		else if(strTryToBoolean(value.data.string, &t.b)) {
			copy.type = BOOLEAN;
			copy.data.boolean = t.b;
		}
		else {
			copy.type = STRING;
			copy.data.string = extractString(value.data.string);
		}
		return copy;
	}
	else
		return value;
}
