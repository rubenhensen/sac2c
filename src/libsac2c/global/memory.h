#ifndef _SAC_MEMORY_H_
#define _SAC_MEMORY_H_

#include "types.h"
#include "hash_table.h"

/*
 * mallocinfo_t is a struct we use to store information about a malloc
 */
typedef struct mallocinfo_t {
    compiler_phase_t phase;
    int size;
    bool isfreed;
    bool isreachable;
    bool isnode;
    nodetype type;
    char *file;
    int line;
    int occurrence;
    struct mallocinfo_t *next;
} mallocinfo_t;

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

/*
 * description:
 *   When debugging is enabled, _MEMmalloc is called instead of malloc
 *   _MEMmalloc stores all available information about the malloc in a hash-table
 *   This allows us to better understand the behaviour of the compiler
 *
 *   should be used instead of malloc
 */
extern void *_MEMmalloc (int size, char *file, int line);
#define MEMmalloc(size) _MEMmalloc (size, __FILE__, __LINE__)
#define MEMmallocAt(size, file, line) _MEMmalloc (size, file, line)

/*
 * description:
 *   _MEMfree is used to free a pointer and, if available, remove the
 *   corresponding mallocinfo_t struct from the associative hash-table
 *
 *   should be used instead of free
 */
extern void *_MEMfree (void *address);
#define __MEMfree(x) MEMfree (x)
#ifdef __cplusplus
#define MEMfree(x) (__typeof(x)) _MEMfree ((void *)x)
#else
#define MEMfree(x) _MEMfree (x)
#endif

/*
 * description:
 *   _MEMrealloc is used to reallocate memory and store info about this in the
 *   associative hash-table
 *
 *   should be used instead of realloc
 */
extern void *_MEMrealloc (void *, int);
#define __MEMrealloc(ptr, size) _MEMrealloc (ptr, size)

/*
 * MEMcopy allocates new memory via MEMmalloc and copies the memory from void*
 * mem to this new malloc
 */
extern void *MEMcopy (int size, void *mem);

/*
 * Also used when traversing the tree to check for space leaks, that is why we
 * declare it extern
 */

#endif /* _SAC_MEMORY_H_ */
