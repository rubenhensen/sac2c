#ifndef _SAC_MEMORY_H_
#define _SAC_MEMORY_H_

#include "types.h"
#include "uthash.h"

/*
 * mallocinfo_t is a struct we use to store information about a malloc
 */
typedef struct mallocinfo_t {
    void *key;
    compiler_phase_t phase;
    int size;
    bool isfreed;
    bool isreachable;
    bool wasintree;
    bool isnode;
    nodetype type;
    char *file;
    const char *callingfunc;
    int line;
    int occurrence;
    struct mallocinfo_t *next;
    struct UT_hash_handle hh;
} mallocinfo_t;

extern mallocinfo_t *malloctable; // needed by check_mem

/*
 * mallocphaseinfo_t is a struct we use to store information about a phase
 * regarding memory managment
 */
typedef struct mallocphaseinfo_t {
    int nmallocd;
    int nfreed;
    int nleaked;
    int leakedsize;
    int notfreedsize;
    mallocinfo_t *notfreed;
    mallocinfo_t *leaked;
    compiler_phase_t phase;
} mallocphaseinfo_t;

extern mallocphaseinfo_t phasetable[]; // needed by check_mem

/*
 *   When debugging is enabled, _MEMmalloc is called instead of malloc
 *   _MEMmalloc stores all available information about the malloc in a hash-table
 *   This allows us to better understand the behaviour of the compiler.
 *
 *   Should be used instead of `malloc'.
 */
extern void *_MEMmalloc (int size, char *file, int line, const char *func);
#define MEMmalloc(size) _MEMmalloc (size, __FILE__, __LINE__, __func__)
#define MEMmallocAt(size, file, line) _MEMmalloc (size, file, line, __func__)

/*
 *   _MEMfree is used to free a pointer and, if available, remove the
 *   corresponding mallocinfo_t struct from the associative hash-table.
 *
 *   Should be used instead of free
 */
extern void *_MEMfree (void *address);

/*
 *   _MEMrealloc is used to reallocate memory and store info about this in the
 *   associative hash-table
 *
 *   should be used instead of realloc
 */
extern void *_MEMrealloc (void *, int);

/*
 *   MEMcopy copies SIZE bytes starting from MEM to the newly allocated memory
 *   location.
 */
extern void *MEMcopy (int size, void *mem);

#ifdef __cplusplus
#define MEMfree(x) (__typeof(x)) _MEMfree ((void *)x)
#define MEMrealloc(x, y) (__typeof(x)) _MEMrealloc ((void *)x, y)
#else
#define MEMfree(x) _MEMfree (x)
#define MEMrealloc(x, y) _MEMrealloc (x, y)
#endif

#endif /* _SAC_MEMORY_H_ */
