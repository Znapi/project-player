#pragma once

// attempted conversions
extern bool tryToFloating(const Value *const value, double *ret);

// forced conversions
extern int64 toInteger(const Value *const value);
extern double toFloating(const Value *const value);
extern uint32 toString(const Value *const value, char **string);
extern bool toBoolean(const Value *const value);

// parsing
extern Value strnToValue(const char *const string, const ubyte length);

// copying
extern Value extractValue(const Value *const value);
extern Value extractSimplifiedValue(const Value *const value);
