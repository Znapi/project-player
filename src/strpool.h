#pragma once

extern char* extractString(const char *const src, size_t *const len);

extern char* strpool_alloc(const size_t length);
extern void strpool_empty(void);
