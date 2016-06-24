/**
	 dynarray.h -- Inlining implementation of dynamic arrays.

	 dynarray.h is an implementation of dynamic arrays in a single header
	 file using inlining functions.

	 It is just a modified, stripped down version of Hanson's utarray.h plus
	 a few extra procedures. Changes include:

	 		* Renaming to dynarray
	 		* Removing ICDs, specifically constructors and deconstructors, with
				the size variable moved into the dynarray structure
			* Changing most procedures from macros to inlining functions.
			* Adding dynarray_front_unchecked, dynarray_back_unchecked,
				dynarray_ensure_size, dynarray_extract, dynarray_finalize,
				and dynarray_pop_back_n.
**/

/*
	Copyright (c) 2008-2014, Troy D. Hanson   http://troydhanson.github.com/uthash/
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
	IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
	TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
	PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
	OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DYNARRAY_H
#define DYNARRAY_H

#define DYNARRAY_VERSION 1.0.0

#include <stddef.h>  /* size_t */
#include <string.h>  /* memset, etc */
#include <stdlib.h>  /* exit */

#ifndef oom
#define oom() exit(-1)
#endif

typedef struct dynarray {
	unsigned i,n; // i: index of next available slot, n: num slots
	size_t sz; // size of each element/slot
	char *d; // n slots of size sz
} dynarray;

static inline void dynarray_init(dynarray *a, size_t sz) {
	memset(a, 0, sizeof(dynarray));
	a->sz = sz;
}

static inline void dynarray_done(dynarray *a) {
	if (a->n)
		free(a->d);
	a->n = 0;
}

// this one's a macro to prevent needing to do &dynarray to call
//static inline void dynarray_new(dynarray *a, size_t sz) {
#define dynarray_new(a, sz) {	\
		(a) = (dynarray*)malloc(sizeof(dynarray));					\
		dynarray_init(a, sz);																\
	}

static inline void dynarray_free(dynarray *a) {
	dynarray_done(a);
	free(a);
}

static inline void dynarray_reserve(dynarray *a, size_t by) {
	if ((a->i + by) > a->n) {
		char *tmp;
		while((a->i + by) > a->n) { a->n = (a->n) ? (2*a->n) : 8; }
		tmp = (char*)realloc(a->d, a->n*a->sz);
		if (tmp == NULL) oom();
		a->d = tmp;
	}
}

static inline void dynarray_ensure_size(dynarray *a, size_t s) {
	if (s > a->n) {
		char *tmp;
		while(s > a->n) { a->n = (a->n) ? (2*a->n) : 8; }
		tmp = (char*)realloc(a->d, a->n*a->sz);
		if (tmp == NULL) oom();
		a->d = tmp;
	}
}

//static inline unsigned dynarray_len(dynarray *a) { return a->i; }
#define dynarray_len(a) ((a)->i)

static inline void dynarray_clear(dynarray *a) { a->i = 0; }

static inline void dynarray_sort(dynarray *a, int (*cmp)(const void*, const void*)) { qsort(a->d, a->i, a->sz, cmp); }

static inline void* dynarray_find(dynarray *a, void *v, int (*cmp)(const void*, const void*)) { return bsearch(v, a->d, a->i, a->sz, cmp); }

static inline void* _dynarray_eltptr(dynarray *a, unsigned j) { return a->d + a->sz*j; };
static inline void* dynarray_eltptr(dynarray *a, unsigned j) { return (j < a->i) ? _dynarray_eltptr(a, j) : NULL; }
static inline int dynarray_eltidx(dynarray *a, void *e) { return ((char*)e >= a->d) ? (((char*)e - (char*)a->d) / (size_t)a->sz) : -1; }

static inline void* dynarray_front_unchecked(dynarray *a) { return _dynarray_eltptr(a, 0); }
static inline void* dynarray_back_unchecked(dynarray *a) { return _dynarray_eltptr(a, a->i-1); }

static inline void* dynarray_front(dynarray *a) { return (a->i) ? dynarray_front_unchecked(a) : NULL; }
static inline void* dynarray_back(dynarray *a) { return (a->i) ? dynarray_back_unchecked(a) : NULL; }
static inline void* dynarray_next(dynarray *a, void *e) { return (e == NULL) ? dynarray_front(a) : ((a->i > (dynarray_eltidx(a,e)+1)) ? _dynarray_eltptr(a, dynarray_eltidx(a, e) + 1) : NULL); }
static inline void* dynarray_prev(dynarray *a, void *e) { return (e == NULL) ? dynarray_back(a) : ((dynarray_eltidx(a, e) > 0) ? _dynarray_eltptr(a, dynarray_eltidx(a, e) - 1) : NULL); }

static inline void dynarray_push_back(dynarray *a, void *p) {
	dynarray_reserve(a, 1);
	memcpy(_dynarray_eltptr(a, a->i++), p, a->sz);
}

static inline unsigned dynarray_pop_back(dynarray *a) { return a->i--; }
static inline void dynarray_pop_back_n(dynarray *a, unsigned n) { a->i -= n; } // unchecked

static inline void dynarray_extend_back(dynarray *a) {
	dynarray_reserve(a, 1);
	memset(_dynarray_eltptr(a, a->i), 0, a->sz);
	++a->i;
}

static inline void dynarray_resize(dynarray *dst, size_t num) {
	if (dst->i < (size_t)num) {
		dynarray_reserve(dst, num-dst->i);
		memset(_dynarray_eltptr(dst, dst->i), 0, dst->sz*(num-dst->i));
	}
	dst->i = num;
}

static inline void dynarray_insert(dynarray *a, void *p, size_t j) {
	if (j > a->i) dynarray_resize(a, j);
	dynarray_reserve(a, 1);
	if (j < a->i) {
		memmove(_dynarray_eltptr(a, j+1), _dynarray_eltptr(a, j),
						(a->i - j)*(a->sz));
	}
	memcpy(_dynarray_eltptr(a, j), p, a->sz);
	++a->i;
}

static inline void dynarray_inserta(dynarray *a, dynarray *w, size_t j) {
	if (dynarray_len(w) == 0) return;
	if (j > a->i) dynarray_resize(a, j);
	dynarray_reserve(a, dynarray_len(w));
	if (j < a->i) {
		memmove(_dynarray_eltptr(a, j + dynarray_len(w)),
						_dynarray_eltptr(a, j),
						(a->i - j)*(a->sz));
	}
	memcpy(_dynarray_eltptr(a, j), _dynarray_eltptr(w, 0),
				 dynarray_len(w)*(a->sz));
	a->i += dynarray_len(w);
}

static inline void dynarray_concat(dynarray *dst, dynarray *src) { dynarray_inserta(dst, src, dynarray_len(dst)); }

static inline void dynarray_erase(dynarray *a, size_t pos, size_t len) {
	if (a->i > (pos+len)) {
		memmove(_dynarray_eltptr(a, pos), _dynarray_eltptr(a, pos+len),
						((a->i)-(pos+len))*(a->sz));
	}
	a->i -= len;
}

static inline void dynarray_renew(dynarray *a, size_t u) {
	if (a) dynarray_clear(a);
	else dynarray_new(a,u);
}

/* Places the actual array in a minimally sized contiguous allocation (using realloc) and frees the dynarray (the metadata, `a`).
	 The addresses of elements is not guarenteed to stay the same.
	 `f` is a pointer that this macro points to the new allocation.
	 `a` is a pointer to the dynarray and becomes invalid. */
static inline void dynarray_finalize(dynarray *a, void **f) {
	*f = realloc(a->d, a->sz*a->i);
	if(*f == NULL) oom();
	free(a);
}

/* Leaves the dynarray untouched, but copies all the elements to a new, minimally sized, contiguous allocation (allocated using malloc).
	 `f` is a pointer that this macro points to the new allocation.
	 `a` is a pointer to the dynarray and will still be perfectly valid. */
static inline void dynarray_extract(dynarray *a, void **f) {
	*f = malloc(a->sz*a->i);
	if(*f == NULL) oom();
	memcpy(*f, a->d, a->sz*a->i);
}

#endif /* DYNARRAY_H */
