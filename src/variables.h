#pragma once

extern void variable_init(Variable **variables, Variable *const variable, const char *const name, const Value *const value);
extern void variable_new(Variable **variables, const char *const name, const Value *const value);
extern void freeVariables(Variable **variables);
extern bool setVariable(Variable **variables, const char *name, const Value *const newValue);
extern bool getVariable(Variable **variables, const char *const name, Value *const returnValue);

extern void list_init(List **lists, List *list, const char *const name);
extern List* list_new(List **lists, const char *const name);
extern void freeLists(List **lists);
extern bool getListPtr(List **lists, const char *const name, List **const returnList);

extern Value listGetFirst(const List *const list);
extern Value listGetLast(const List *const list);
extern Value listGet(const List *const list, const uint32 index);

extern void listAppend(List *list, const Value *const value);
extern void listPrepend(List *list, const Value *const value);
extern void listInsert(List *list, const Value *const value, const uint32 index);

extern void listSetFirst(List *list, const Value *const newValue);
extern void listSetLast(List *list, const Value *const newValue);
extern void listSet(List *list, const Value *const newValue, const uint32 index);

extern void listDeleteFirst(List *list);
extern void listDeleteLast(List *list);
extern void listDelete(List *list, const uint32 index);
extern void listDeleteAll(List *list);

extern bool listContainsFloating(const List *const list, const double floating);
extern bool listContainsBoolean(const List *const list, const bool boolean);
extern bool listContainsString(const List *const list, const char *const string);
