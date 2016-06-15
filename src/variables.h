#pragma once

extern Variable* createVariable(Variable **variables, const char *name, Value value);
extern void freeVariables(Variable **variables);
extern void setVariable(Variable **variables, const char *name, const Value *const newValue);
extern Value getVariable(Variable **variables, const char *name);

extern List* createList(List **lists, const char *name, ListElement *elements, uint32 length);
extern void freeLists(List **lists);
extern List* getListPtr(List **lists, const char *name);

extern Value listGetFirst(const List *list);
extern Value listGetLast(const List *list);
extern Value listGet(const List *list, uint32 index);

extern void listAppend(List *list, const Value *const value);
extern void listPrepend(List *list, const Value *const value);
extern void listInsert(List *list, const Value *const value, uint32 index);

extern void listSetFirst(List *list, const Value *const newValue);
extern void listSetLast(List *list, const Value *const newValue);
extern void listSet(List *list, const Value *const newValue, uint32 index);

extern void listDeleteFirst(List *list);
extern void listDeleteLast(List *list);
extern void listDelete(List *list, uint32 index);
extern void listDeleteAll(List *list);

extern bool listContainsFloating(const List *list, const double floating);
extern bool listContainsBoolean(const List *list, const bool boolean);
extern bool listContainsString(const List *list, const char *const string);
