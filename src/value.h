#pragma once

#define value_free(v) { if(v.type == STRING) free(v.data.string); }

// attempted conversions
extern bool tryToFloating(const Value *const value, double *ret);

// forced conversions
extern int64 toInteger(const Value *const value);
extern double toFloating(const Value *const value);
extern size_t toString(const Value *const value, char **string);
extern bool toBoolean(const Value *const value);

// parsing
extern Value strnToValue(const char *const string, const size_t length);

// copying
extern Value extractValue(const Value *const value);
extern Value extractSimplifiedValue(const Value *const value);
