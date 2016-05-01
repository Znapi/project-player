#pragma once

extern Value copyValue(const Value value);

// try conversions
//extern bool tryToBoolean(const char *str, bool *ret);
extern bool tryToFloating(const Value value, double *ret);

// forced conversions
extern int64 toInteger(const Value value);
extern double toFloating(const Value value);
extern uint32 toString(const Value value, char **string);
extern bool toBoolean(const Value value);

extern Value strnToValue(const char *const string, const ubyte length);
extern Value copySimplifiedValue(const Value value);
