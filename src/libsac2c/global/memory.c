/*
 * $Id$
 */

#include "memory.h"
#undef MEMmalloc

#include <stdlib.h>
#include <string.h>

#define DBUG_PREFIX "MEM_ALLOC"
#include "debug.h"

#include "check_mem.h"
#include "ctinfo.h"
#include "globals.h"
#include "DupTree.h"
#include "free.h"
#include "convert.h"
#include "str.h"

/******************************************************************************
 *
 * function:
 *   void *MEMmalloc( int size)
 *   void *MEMfree( void *address)
 *
 * description:
 *   These functions for memory allocation and de-allocation are wrappers
 *   for the standard functions malloc() and free().
 *
 *   They allow to implement some additional functionality, e.g. accounting
 *   of currently allocated memory.
 *
 ******************************************************************************/

#ifdef SHOW_MALLOC

/*
 * These types are only used to compute malloc_align_step.
 *
 * CAUTION:
 *
 * We need malloc_align_step in check_mem.c as well. Rather than using a single
 * global variable we use two static global variables, which are initialised
 * exactly in the same way, and of course need to be.
 */

typedef union {
    long int l;
    double d;
} malloc_align_type;

typedef struct {
    int size;
    malloc_align_type align;
} malloc_header_type;

static int malloc_align_step = sizeof (malloc_header_type) - sizeof (malloc_align_type);

void *
MEMmalloc (int size)
{
    void *orig_ptr;
    void *shifted_ptr;

    DBUG_ENTER ();

    DBUG_ASSERT (size >= 0, "MEMmalloc called with negative size!");
    DBUG_ASSERT (malloc_align_step > 0, "malloc_align_step not set");

    if (size > 0) {

        /*
         * Since some UNIX system (e.g. ALPHA) do return NULL for size 0 as well
         * we do complain for ((NULL == tmp) && (size > 0)) only!!
         */
        orig_ptr = malloc (size + malloc_align_step);

        if (orig_ptr == NULL) {
            CTIabortOutOfMemory (size);
        }

        shifted_ptr = CHKMregisterMem (size, orig_ptr);

        if (global.current_allocated_mem + size < global.current_allocated_mem) {

            DBUG_ASSERT (0, "counter for allocated memory: overflow detected");
        }
        global.current_allocated_mem += size;
        if (global.max_allocated_mem < global.current_allocated_mem) {
            global.max_allocated_mem = global.current_allocated_mem;
        }

        DBUG_PRINT ("Alloc memory: %d Bytes at adress: " F_PTR, size, shifted_ptr);

        DBUG_PRINT_TAG ("MEM_TOTAL", "Currently allocated memory: %u",
                        global.current_allocated_mem);

#ifdef CLEANMEM
        /*
         * Initialize memory
         */
        shifted_ptr = memset (shifted_ptr, 0, size);
#endif

    } else {
        shifted_ptr = NULL;
    }

    DBUG_RETURN (shifted_ptr);
}

void *
MEMmallocAt (int size, char *file, int line)
{
    void *pointer;

    DBUG_ENTER ();

    if (size > 0) {
        pointer = MEMmalloc (size);

        CHKMsetLocation (pointer, file, line);
    } else {
        pointer = NULL;
    }

    DBUG_RETURN (pointer);
}

#ifdef NOFREE

void *
MEMfree (void *address)
{
    DBUG_ENTER ();

    address = NULL;

    DBUG_RETURN (address);
}

#else /* NOFREE */

void *
MEMfree (void *shifted_ptr)
{
    void *orig_ptr = NULL;
    int size;

    DBUG_ENTER ();

    DBUG_ASSERT (malloc_align_step > 0, "malloc_align_step not set");

    if (shifted_ptr != NULL) {
        size = CHKMgetSize (shifted_ptr);

        DBUG_ASSERT (size >= 0, "illegal size found!");
        DBUG_PRINT ("Free memory: %d Bytes at adress: " F_PTR, size, shifted_ptr);

        if (global.current_allocated_mem < global.current_allocated_mem - size) {
            DBUG_ASSERT (0, "counter for allocated memory: overflow detected");
        }
        global.current_allocated_mem -= size;

        DBUG_PRINT_TAG ("MEM_TOTAL", "Currently allocated memory: %u",
                        global.current_allocated_mem);

#ifdef CLEANMEM
        /*
         * this code overwrites the memory prior to freeing it. This
         * is very useful when watching a memory address in gdb, as
         * one gets notified as soon as it is freed. Needs SHOW_MALLOC
         * to get the size of the freed memory chunk.
         */
        shifted_ptr = memset (shifted_ptr, 0, size);
#endif /* CLEANMEM */

        orig_ptr = CHKMunregisterMem (shifted_ptr);
        free (orig_ptr);
        orig_ptr = NULL;
    }

    DBUG_RETURN (orig_ptr);
}

#endif /* NOFREE */

/******************************************************************************
 *
 * function:
 *   void MEMdbugMemoryLeakCheck( void)
 *
 * description:
 *   computes and prints memory usage w/o memory used for the actual
 *   syntax tree.
 *
 ******************************************************************************/

void
MEMdbugMemoryLeakCheck (void)
{
    node *ast_dup;
    int mem_before;

    DBUG_ENTER ();

    mem_before = global.current_allocated_mem;
    CTInote ("*** Currently allocated memory (Bytes):   %s",
             CVintBytes2String (global.current_allocated_mem));
    ast_dup = DUPdoDupTree (global.syntax_tree);
    CTInote ("*** Size of the syntax tree (Bytes):      %s",
             CVintBytes2String (global.current_allocated_mem - mem_before));
    CTInote ("*** Other memory allocated/ Leak (Bytes): %s",
             CVintBytes2String (2 * mem_before - global.current_allocated_mem));
    FREEdoFreeTree (ast_dup);
    CTInote ("*** FreeTree / DupTree leak (Bytes):      %s",
             CVintBytes2String (global.current_allocated_mem - mem_before));

    DBUG_RETURN ();
}

#else /*SHOW_MALLOC */

void *
MEMmalloc (int size)
{
    void *ptr;

    DBUG_ENTER ();

    DBUG_ASSERT (size >= 0, "MEMmalloc called with negative size!");

    if (size > 0) {
        /*
         * Since some UNIX system (e.g. ALPHA) do return NULL for size 0 as well
         * we do complain for ((NULL == tmp) && (size > 0)) only!!
         */
        ptr = malloc (size);

        if (ptr == NULL) {
            CTIabortOutOfMemory (size);
        }
    } else {
        ptr = NULL;
    }

    DBUG_RETURN (ptr);
}

void *
MEMfree (void *address)
{
    DBUG_ENTER ();

    if (address != NULL) {
        free (address);
        address = NULL;
    }

    DBUG_RETURN (address);
}

#endif /* SHOW_MALLOC */

/******************************************************************************
 *
 * Function:
 *   void *MEMcopy( int size, void *mem)
 *
 * Description:
 *   Allocates memory and returns a pointer to the copy of 'mem'.
 *
 ******************************************************************************/

void *
MEMcopy (int size, void *mem)
{
    void *result;

    DBUG_ENTER ();

    result = MEMmalloc (size);

    result = memcpy (result, mem, size);

    DBUG_RETURN (result);
}

#undef DBUG_PREFIX
