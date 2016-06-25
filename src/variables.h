#pragma once

typedef struct Variable Variable;
typedef struct List List;

extern void variable_init(Variable **variables, Variable *const variable, const char *const name, const uint16 nameLen, const Value *const value);
extern void variable_new(Variable **variables, const char *const name, const uint16 nameLen, const Value *const value);
extern void freeVariables(Variable **variables);
extern Variable* copyVariables(const Variable *const *const variables);
extern bool setVariable(Variable **variables, const char *name, const Value *const newValue);
extern bool getVariable(Variable **variables, const char *const name, Value *const returnValue);

extern void list_init(List **lists, List *list, const char *const name, const uint16 nameLen);
extern UT_array* list_new(List **lists, const char *const name, const uint16 nameLen);
extern void freeLists(List **lists);
extern List *copyLists(const List *const *const Lists);
extern bool getListContents(List **lists, const char *const name, UT_array **const returnContents);

extern Value listGetFirst(const UT_array *const list);
extern Value listGetLast(const UT_array *const list);
extern Value listGet(const UT_array *const list, const uint32 index);

extern void listAppend(UT_array *list, const Value *const value);
extern void listPrepend(UT_array *list, const Value *const value);
extern void listInsert(UT_array *list, const Value *const value, const uint32 index);

extern void listSetFirst(UT_array *list, const Value *const newValue);
extern void listSetLast(UT_array *list, const Value *const newValue);
extern void listSet(UT_array *list, const Value *const newValue, const uint32 index);

extern void listDeleteFirst(UT_array *list);
extern void listDeleteLast(UT_array *list);
extern void listDelete(UT_array *list, const uint32 index);
extern void listDeleteAll(UT_array *list);

extern bool listContainsFloating(const UT_array *const list, const double floating);
extern bool listContainsBoolean(const UT_array *const list, const bool boolean);
extern bool listContainsString(const UT_array *const list, const char *const string);
