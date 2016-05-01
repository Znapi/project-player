#pragma once

extern Variable* createVariable(Variable **variables, const char *name, Value value);
extern void freeVariables(Variable **variables);
extern void setVariable(Variable **variables, const char *name, Value newValue);
extern Value getVariable(Variable **variables, const char *name);

extern List* createList(List **lists, const char *name, ListElement *elements, uint32 length);
extern void freeLists(List **lists);
extern List* getListPtr(List **lists, const char *name);

extern Value getFirstListElement(const List *list);
extern Value getLastListElement(const List *list);
extern Value getListElement(const List *list, uint32 index);

extern void listAppend(List *list, Value value);
extern void listPrepend(List *list, Value value);
extern void listInsert(List *list, Value value, uint32 index);

extern void listSetFirst(List *list, Value newValue);
extern void listSetLast(List *list, Value newValue);
extern void listSet(List *list, Value newValue, uint32 index);

extern void listDeleteFirst(List *list);
extern void listDeleteLast(List *list);
extern void listDelete(List *list, uint32 index);

extern bool listContainsFloating(const List *list, double floating);
extern bool listContainsBoolean(const List *list, bool boolean);
extern bool listContainsString(const List *list, const char *string);
