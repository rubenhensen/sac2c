/*****************************************************************************
 *
 * file:   malloc.c
 *
 * prefix: none
 *
 * description:
 *
 *   This file provides the necessary compatibility layer for calls to the
 *   SAC Private Heap Manager from outside compiled SAC code, i.e. from
 *   external modules and classes or generally from the outsode world in case
 *   of the SAC-from-C scenario.
 *
 *   More precisely, we provide custom implementations of the following
 *   functions from the standard dynamic heap management API:
 *    - malloc
 *    - free
 *    - calloc
 *    - realloc
 *    - valloc
 *    - memalign
 *    - posix_memalign
 *
 *
 *****************************************************************************/

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>

#include "heapmgr.h"

#ifdef __cplusplus
#define __throw throw ()
#else
#define __throw
#endif

/******************************************************************************
 *
 * global variable:
 *   static int not_yet_initialized
 *
 * description:
 *
 *   This is a global flag that allows to keep track whether the
 *   internal data structures of the heap manager have already been
 *   initialized or not.
 *
 *
 ******************************************************************************/

SAC_C_EXTERN unsigned int SAC_HM_CurrentThreadId (void);
static int not_yet_initialized = 1;

/******************************************************************************
 *
 * function:
 *   void *malloc(size_byte_t size)
 *
 * description:
 *
 *   Interface function for allocation of memory chunks.
 *
 *   Note that often the more specific heap manager API functions are called
 *   directly.
 *
 ******************************************************************************/

void *
malloc (size_t sz) __throw
{
    const SAC_HM_size_byte_t size = (SAC_HM_size_byte_t)sz;
    SAC_HM_size_unit_t units;
    void *mem;
#if SAC_MT_MODE > 0
    /* unsigned int thread_id, *thread_id_ptr; */
    unsigned int thread_id;
    const int multi_threaded = !SAC_MT_globally_single;
#else  /* MT */
    const unsigned int thread_id = 0;
#endif /* MT */

    DIAG_INC_LOCK (SAC_HM_call_malloc);

    if (not_yet_initialized) {
        SAC_HM_SetupMaster ();
    }

#if SAC_MT_MODE > 0
    if (multi_threaded) {
        /*
         * OpenMP mt solution and Pthread solution
         * share the same code of phm
         * except that OpenMP and Pthread use different
         * method to get thread id
         *
        thread_id_ptr = (unsigned int *) pthread_getspecific(SAC_MT_threadid_key);
        if (thread_id_ptr == NULL) {
          thread_id = 0;
        }
        else {
          thread_id = *thread_id_ptr;
        }
        */
        if (size <= SAC_HM_ARENA_7_MAXCS_BYTES) {
            thread_id = SAC_HM_CurrentThreadId ();
        } else {
            /* the allocation will go to the top arena, which is shared */
            thread_id = (unsigned)0xb19b00b5; /* invalid id, negative int */
        }
    } else {
        thread_id = 0;
    }
#endif /* MT */

    if (size <= SAC_HM_ARENA_4_MAXCS_BYTES) {
        /* Now, it's arena 1, 2, 3, or 4. */
        if (size <= SAC_HM_ARENA_2_MAXCS_BYTES) {
            /* Now, it's arena 1 or 2. */
            if (size <= SAC_HM_ARENA_1_MAXCS_BYTES) {
                DIAG_INC (SAC_HM_arenas[thread_id][1].cnt_alloc_var_size);
                return (SAC_HM_MallocSmallChunk (2, &(SAC_HM_arenas[thread_id][1])));
            } else {
                DIAG_INC (SAC_HM_arenas[thread_id][2].cnt_alloc_var_size);
                return (SAC_HM_MallocSmallChunk (4, &(SAC_HM_arenas[thread_id][2])));
            }
        } else {
            /* Now, it's arena 3 or 4. */
            if (size <= SAC_HM_ARENA_3_MAXCS_BYTES) {
                DIAG_INC (SAC_HM_arenas[thread_id][3].cnt_alloc_var_size);
                return (SAC_HM_MallocSmallChunk (8, &(SAC_HM_arenas[thread_id][3])));
            } else {
                DIAG_INC (SAC_HM_arenas[thread_id][4].cnt_alloc_var_size);
                return (SAC_HM_MallocSmallChunk (16, &(SAC_HM_arenas[thread_id][4])));
            }
        }
    } else {
        units = ((size - 1) / SAC_HM_UNIT_SIZE) + 3;

        if (units < SAC_HM_ARENA_7_MINCS) {
            assert ((int)thread_id >= 0);
            /* Now, it's arena 5 or 6. */
            if (units < SAC_HM_ARENA_6_MINCS) {
                DIAG_INC (SAC_HM_arenas[thread_id][5].cnt_alloc_var_size);
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[thread_id][5])));
            } else {
                DIAG_INC (SAC_HM_arenas[thread_id][6].cnt_alloc_var_size);
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[thread_id][6])));
            }
        } else {
            /* Now, it's arena 7 or 8. */
            if (units < SAC_HM_ARENA_8_MINCS) {
                assert ((int)thread_id >= 0);
                DIAG_INC (SAC_HM_arenas[thread_id][7].cnt_alloc_var_size);
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[thread_id][7])));
            } else {
#if SAC_MT_MODE > 0
                if (multi_threaded) {
                    SAC_MT_ACQUIRE_LOCK (SAC_HM_top_arena_lock);
                    DIAG_INC (SAC_HM_acquire_top_arena_lock);
                    DIAG_INC (SAC_HM_arenas[0][SAC_HM_TOP_ARENA].cnt_alloc_var_size);
                    mem = SAC_HM_MallocLargeChunk (units,
                                                   &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]));
                    SAC_MT_RELEASE_LOCK (SAC_HM_top_arena_lock);
                } else {
                    DIAG_INC (SAC_HM_arenas[0][SAC_HM_TOP_ARENA].cnt_alloc_var_size);
                    mem = SAC_HM_MallocLargeChunk (units,
                                                   &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]));
                }
#else  /* MT */
                DIAG_INC (SAC_HM_arenas[0][SAC_HM_TOP_ARENA].cnt_alloc_var_size);
                mem = SAC_HM_MallocLargeChunk (units,
                                               &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]));
#endif /* MT */
                // fprintf(stderr, "malloc: top arena alloc: %p, size %u\n", mem, sz);
                return (mem);
            }
        }
    }
}

/******************************************************************************
 *
 * function:
 *   void free(void *addr)
 *
 * description:
 *
 *   Interface function for de-allocation of memory chunks.
 *
 *   Note that often the more specific heap manager API functions are called
 *   directly.
 *
 ******************************************************************************/

#define ARRAY_PLACEMENT

void
free (void *addr)
{
    SAC_HM_arena_t *arena;

    if (addr != NULL) {

        arena = SAC_HM_ADDR_ARENA (addr);

        if (arena != NULL) {
#ifdef ARRAY_PLACEMENT
            if ((long int)arena & (long int)1) {
                addr = (SAC_HM_header_t *)((long int)arena & (~(long int)1));
                arena = SAC_HM_ADDR_ARENA (addr);
            }
#endif
            DIAG_CHECK_ALLOCPATTERN_ANYCHUNK ((SAC_HM_header_t *)addr);
            DIAG_INC (arena->cnt_free_var_size);

            if (arena->num < SAC_HM_NUM_SMALLCHUNK_ARENAS) {
                SAC_HM_FreeSmallChunk ((SAC_HM_header_t *)addr, arena);
                return;
            }

#if SAC_MT_MODE > 0
            if (arena->num < SAC_HM_TOP_ARENA) {
                SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr, arena);
            } else {
                SAC_HM_FreeTopArena_at ((SAC_HM_header_t *)addr);
            }
#else  /* MT */
            SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr, arena);
#endif /* MT */
        }
    }
}

/*****************************************************************************
 *
 * function:
 *   void *calloc(size_t nelem, size_t elsize)
 *
 * description:
 *
 *   SAC heap manager specific implementation of calloc().
 *
 *****************************************************************************/

void *
calloc (size_t nelem, size_t elsize)
{
    void *res;

    DIAG_INC_LOCK (SAC_HM_call_calloc);
    DIAG_DEC_LOCK (SAC_HM_call_malloc);

    res = malloc (nelem * elsize);

    res = memset (res, 0, nelem * elsize);

    return (res);
}

/******************************************************************************
 *
 * function:
 *   void *realloc(void *ptr, size_t size)
 *
 * description:
 *
 *   SAC heap manager specific implementation of realloc().
 *
 ******************************************************************************/

void *
realloc (void *ptr, size_t size)
{
    void *mem;
    SAC_HM_size_unit_t old_size_units;
    SAC_HM_arena_t *arena;

    DIAG_INC_LOCK (SAC_HM_call_realloc);

    if (ptr == NULL) {
        DIAG_DEC_LOCK (SAC_HM_call_malloc);
        return (malloc (size));
    }

    if (size == 0) {
        free (ptr);
        return (NULL);
    }

    if (not_yet_initialized) {
        SAC_HM_SetupMaster ();
    }

    arena = SAC_HM_ADDR_ARENA (ptr);

    if (arena->num < SAC_HM_NUM_SMALLCHUNK_ARENAS) {
        old_size_units = arena->min_chunk_size;
        if (size <= (size_t)old_size_units) {
            /*
             * The given memory location is in a small chunk arena and the requested
             * new memory size is less than the old one.
             *  -> do nothing
             */
            return (ptr);
        }
    } else {
        old_size_units = SAC_HM_LARGECHUNK_SIZE (((SAC_HM_header_t *)ptr) - 2);
    }

    DIAG_DEC_LOCK (SAC_HM_call_malloc);
    mem = malloc (size);

    mem = memcpy (mem, ptr, SAC_MIN (old_size_units * SAC_HM_UNIT_SIZE, size));

    free (ptr);

    return (mem);
}

/******************************************************************************
 *
 * function:
 *   void *memalign(size_t alignment, size_t size)
 *
 * description:
 *
 *   SAC heap manager specific implementation of memalign().
 *
 ******************************************************************************/

void *
memalign (size_t alignment, size_t size)
{
    void *mem;
    size_t misalign, size_needed;
    SAC_HM_header_t *freep, *prefixp;
    SAC_HM_arena_t *arena;
    SAC_HM_size_unit_t offset_units;

    DIAG_INC_LOCK (SAC_HM_call_memalign);

    if (alignment <= SAC_HM_UNIT_SIZE) {
        /* automatic alignment */
        DIAG_DEC_LOCK (SAC_HM_call_malloc);
        return (malloc (size));
    }

    /* worst case allocation for a posteriori alignment */
    size_needed = SAC_MAX (size + alignment + (SAC_HM_UNIT_SIZE + SAC_HM_UNIT_SIZE),
                           SAC_HM_ARENA_5_MINCS * SAC_HM_UNIT_SIZE);

    DIAG_DEC_LOCK (SAC_HM_call_malloc);
    mem = malloc (size_needed);

    misalign = ((size_t)mem) % alignment;

    if (misalign == 0) {
        /* Memory is already correctly aligned. */
        return (mem);
    }

    offset_units = (alignment - misalign) / SAC_HM_UNIT_SIZE;

    if (offset_units < 2) {
        /*
         * Offset is too small to host administration info for free prefix.
         */
        offset_units += alignment / SAC_HM_UNIT_SIZE;
    }

    prefixp = ((SAC_HM_header_t *)mem) - 2;
    arena = SAC_HM_LARGECHUNK_ARENA (prefixp);
    freep = prefixp + offset_units;

    /*
     * Setup memory location to be returned.
     */

    SAC_HM_LARGECHUNK_SIZE (freep) = SAC_HM_LARGECHUNK_SIZE (prefixp) - offset_units;
    SAC_HM_LARGECHUNK_ARENA (freep) = arena;
    SAC_HM_LARGECHUNK_PREVSIZE (freep)
      = -1; /* preceeding chunk is allocated; was 'offset_units' */
    SAC_HM_LARGECHUNK_DIAG (freep) = DIAG_ALLOCPATTERN;

    /*
     * Setup memory location returned by malloc();
     * this will now be de-allocated again using free().
     */

    SAC_HM_LARGECHUNK_SIZE (prefixp) = offset_units;

    free (prefixp + 2);

#if defined(__GNUC__) && (__GNUC__ >= 12)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wuse-after-free"
    /*
     * XXX(hans) the following line causes a use-after-free warning in GCC,
     * this is a false positive, likely caused by the static analysis unable to
     * fully resolve the underlying union type, thus misunderstand what effect
     * the pointer offsets have.
     */
#endif
    return ((void *)(freep + 2));
#if defined(__GNUC__) && (__GNUC__ >= 12)
#  pragma GCC diagnostic pop
#endif
}

/******************************************************************************
 *
 * function:
 *   int posix_memalign( void **memptr, size_t alignment, size_t size)
 *
 * description:
 *
 *   SAC heap manager specific implementation of posix_memalign().
 *
 ******************************************************************************/

int
posix_memalign (void **memptr, size_t alignment, size_t size)
{
    DIAG_INC_LOCK (SAC_HM_call_posix_memalign);
    DIAG_DEC_LOCK (SAC_HM_call_memalign);

    if ((alignment % sizeof (void *) != 0) || ((alignment & (alignment - 1)) != 0)) {
        return (EINVAL);
    }

    *memptr = memalign (alignment, size);

    if (*memptr == NULL) {
        return (ENOMEM);
    } else {
        return (0);
    }
}

/******************************************************************************
 *
 * function:
 *   void *valloc(size_t size)
 *
 * description:
 *
 *   SAC heap manager specific implementation of valloc().
 *
 ******************************************************************************/

void *
valloc (size_t size)
{
    DIAG_INC_LOCK (SAC_HM_call_valloc);
    DIAG_DEC_LOCK (SAC_HM_call_memalign);

    return (memalign (getpagesize (), size));
}

/******************************************************************************
 *
 * function:
 *   void SAC_HM_SetInitialized(void)
 *
 * description:
 *
 * This function also enforces linking with the standard heap
 * mangement API compatibility mayer because this function is called from
 * SAC_HM_SetupMaster() in setup.c.
 *
 ******************************************************************************/

void
SAC_HM_SetInitialized (void)
{
    not_yet_initialized = 0;
}

/******************************************************************************
 *
 * function:
 *   int SAC_HM_GetInitialized(void)
 *
 * description:
 *
 * This function also enforces linking with the standard heap
 * mangement API compatibility mayer because this function is called from
 * SAC_HM_SetupMaster() in setup.c.
 *
 ******************************************************************************/

int
SAC_HM_GetInitialized (void)
{
    return (not_yet_initialized);
}
