/*
 *
 * $Log$
 * Revision 1.1  2000/01/03 17:33:17  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:  wrapper.c
 *
 * prefix: SAC_HM
 *
 * description:
 *
 *   This file contains wrapper functions for allocation and de-allocation
 *   routines. These are required where the core routines for large and
 *   small chunk arenas cannot be called directly from the compiled SAC code.
 *
 *
 *****************************************************************************/

#include "heapmgr.h"

/******************************************************************************
 *
 * function:
 *   void *SAC_HM_MallocAnyChunk_st(size_byte_t size)
 *
 * description:
 *
 *   Wrapper function for allocation of a statically unknown amount of memory.
 *
 *   In contrast to malloc() however, this function assumes that single-threaded
 *   execution is guaranteed upon memory allocation.
 *
 *   If the entire heap manager is compiled for single-threaded operation only,
 *   this function simply calls malloc().
 *
 ******************************************************************************/

#ifdef MT

void *
SAC_HM_MallocAnyChunk_st (size_byte_t size)
{
    size_unit_t units;

    DIAG_INC (SAC_HM_call_malloc);

    if (size <= ARENA_4_MAXCS_BYTES) {
        /* Now, it's arena 1, 2, 3, or 4. */
        if (size <= ARENA_2_MAXCS_BYTES) {
            /* Now, it's arena 1 or 2. */
            if (size <= ARENA_1_MAXCS_BYTES) {
                return (SAC_HM_MallocSmallChunk (2, &(SAC_HM_arenas[0][1])));
            } else {
                return (SAC_HM_MallocSmallChunk (4, &(SAC_HM_arenas[0][2])));
            }
        } else {
            /* Now, it's arena 3 or 4. */
            if (size <= ARENA_3_MAXCS_BYTES) {
                return (SAC_HM_MallocSmallChunk (8, &(SAC_HM_arenas[0][3])));
            } else {
                return (SAC_HM_MallocSmallChunk (16, &(SAC_HM_arenas[0][4])));
            }
        }
    } else {
        units = ((size - 1) / UNIT_SIZE) + 3;

        if (units < ARENA_7_MINCS) {
            /* Now, it's arena 5 or 6. */
            if (units < ARENA_6_MINCS) {
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][5])));
            } else {
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][6])));
            }
        } else {
            /* Now, it's arena 7 or 8. */
            if (units < ARENA_8_MINCS) {
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][7])));
            } else {
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][8])));
            }
        }
    }
}

#else /* MT */

void *
SAC_HM_MallocAnyChunk_st (size_byte_t size)
{
    return (malloc (size));
}

#endif /* MT */

/******************************************************************************
 *
 * function:
 *   void *SAC_HM_MallocAnyChunk_mt(size_byte_t size, unsigned int thread_id)
 *
 * description:
 *
 *   Wrapper function for allocation of a statically unknown amount of memory.
 *
 *   In contrast to malloc() however, this function assumes that multi-threaded
 *   execution is guaranteed upon memory allocation and the thread ID of the
 *   calling thread is available..
 *
 *   If the entire heap manager is compiled for single-threaded operation only,
 *   this function simply calls malloc().
 *
 ******************************************************************************/

#ifdef MT

void *
SAC_HM_MallocAnyChunk_mt (size_byte_t size, unsigned int thread_id)
{
    size_unit_t units;
    void *mem;

    if (size <= ARENA_4_MAXCS_BYTES) {
        /* Now, it's arena 1, 2, 3, or 4. */
        if (size <= ARENA_2_MAXCS_BYTES) {
            /* Now, it's arena 1 or 2. */
            if (size <= ARENA_1_MAXCS_BYTES) {
                return (SAC_HM_MallocSmallChunk (2, &(SAC_HM_arenas[thread_id][1])));
            } else {
                return (SAC_HM_MallocSmallChunk (4, &(SAC_HM_arenas[thread_id][2])));
            }
        } else {
            /* Now, it's arena 3 or 4. */
            if (size <= ARENA_3_MAXCS_BYTES) {
                return (SAC_HM_MallocSmallChunk (8, &(SAC_HM_arenas[thread_id][3])));
            } else {
                return (SAC_HM_MallocSmallChunk (16, &(SAC_HM_arenas[thread_id][4])));
            }
        }
    } else {
        units = ((size - 1) / UNIT_SIZE) + 3;

        if (units < ARENA_7_MINCS) {
            /* Now, it's arena 5 or 6. */
            if (units < ARENA_6_MINCS) {
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[thread_id][5])));
            } else {
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[thread_id][6])));
            }
        } else {
            /* Now, it's arena 7 or 8. */
            if (units < ARENA_8_MINCS) {
                return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[thread_id][7])));
            } else {
                pthread_mutex_lock (&SAC_HM_top_arena_lock);
                mem = SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][TOP_ARENA]));
                pthread_mutex_unlock (&SAC_HM_top_arena_lock);
                return (mem);
            }
        }
    }
}

#else /* MT */

void *
SAC_HM_MallocAnyChunk_mt (size_byte_t size, unsigned int thread_id)
{
    return (malloc (size));
}

#endif /* MT */

/******************************************************************************
 *
 * function:
 *   void *SAC_HM_MallocSmallChunk_at(size_unit_t units, int arena_num)
 *
 * description:
 *
 *   Wrapper function for SAC_HM_MallocSmallChunk().
 *
 *   This function is used whenever the amount of memory to be allocated is
 *   statically available, the state with respect to multi-threaded execution
 *   however is unknown. This might even be a module implementation which will
 *   later be used in a purely sequential program.
 *
 *   If the entire heap manager is compiled for single-threaded operation only,
 *   this function simply calls SAC_HM_MallocSmallChunk().
 *
 ******************************************************************************/

#ifdef MT

void *
SAC_HM_MallocSmallChunk_at (size_unit_t units, int arena_num)
{
    unsigned int thread_id;

    if (SAC_MT_not_yet_parallel) {
        return (SAC_HM_MallocSmallChunk (units, &(SAC_HM_arenas[0][arena_num])));
    } else {
        thread_id = *((unsigned int *)pthread_getspecific (SAC_MT_threadid_key));
        return (SAC_HM_MallocSmallChunk (units, &(SAC_HM_arenas[thread_id][arena_num])));
    }
}

#else /* MT */

void *
SAC_HM_MallocSmallChunk_at (size_unit_t units, int arena_num)
{
    return (SAC_HM_MallocSmallChunk (units, &(SAC_HM_arenas[0][arena_num])));
}

#endif /* MT */

/******************************************************************************
 *
 * function:
 *   void *SAC_HM_MallocLargeChunk_at(size_unit_t units, int arena_num)
 *
 * description:
 *
 *   Wrapper function for SAC_HM_MallocLargeChunk().
 *
 *   This function is used whenever the amount of memory to be allocated is
 *   statically available, the state with respect to multi-threaded execution
 *   however is unknown. This might even be a module implementation which will
 *   later be used in a purely sequential program.
 *
 *   If the entire heap manager is compiled for single-threaded operation only,
 *   this function simply calls SAC_HM_MallocLargeChunk().
 *
 ******************************************************************************/

#ifdef MT

void *
SAC_HM_MallocLargeChunk_at (size_unit_t units, int arena_num)
{
    unsigned int thread_id;

    if (SAC_MT_not_yet_parallel) {
        return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][arena_num])));
    } else {
        thread_id = *((unsigned int *)pthread_getspecific (SAC_MT_threadid_key));
        return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[thread_id][arena_num])));
    }
}

#else /* MT */

void *
SAC_HM_MallocLargeChunk_at (size_unit_t units, int arena_num)
{
    return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][arena_num])));
}

#endif /* MT */

/******************************************************************************
 *
 * function:
 *   void *SAC_HM_MallocTopArena_at(size_unit_t units)
 *
 * description:
 *
 *   Wrapper function for SAC_HM_MallocLargeChunk().
 *
 *   This function is used whenever the amount of memory to be allocated is
 *   statically available, the state with respect to multi-threaded execution
 *   however is unknown. This might even be a module implementation which will
 *   later be used in a purely sequential program.
 *
 *   If the entire heap manager is compiled for single-threaded operation only,
 *   this function simply calls SAC_HM_MallocLargeChunk().
 *
 ******************************************************************************/

#ifdef MT

void *
SAC_HM_MallocTopArena_at (size_unit_t units)
{
    void *mem;

    if (SAC_MT_not_yet_parallel) {
        return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][TOP_ARENA])));
    } else {
        pthread_mutex_lock (&SAC_HM_top_arena_lock);
        mem = SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][TOP_ARENA]));
        pthread_mutex_unlock (&SAC_HM_top_arena_lock);
        return (mem);
    }
}

#else /* MT */

void *
SAC_HM_MallocTopArena_at (size_unit_t units)
{
    return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][TOP_ARENA])));
}

#endif /* MT */

/******************************************************************************
 *
 * function:
 *   void *SAC_HM_MallocTopArena_mt(size_unit_t units)
 *
 * description:
 *
 *   Wrapper function for SAC_HM_MallocLargeChunk().
 *
 *   This function is used whenever the amount of memory to be allocated is
 *   statically available and the execution state is multi-threaded.
 *   However, we still need a function abstraction here for the compilation
 *   of module implementations where it is unsure whether the module
 *   might later be used with a purely sequential program.
 *
 *   If the entire heap manager is compiled for single-threaded operation only,
 *   this function simply calls SAC_HM_MallocLargeChunk().
 *
 ******************************************************************************/

#ifdef MT

void *
SAC_HM_MallocTopArena_mt (size_unit_t units)
{
    void *mem;

    pthread_mutex_lock (&SAC_HM_top_arena_lock);
    mem = SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][TOP_ARENA]));
    pthread_mutex_unlock (&SAC_HM_top_arena_lock);
    return (mem);
}

#else /* MT */

void *
SAC_HM_MallocTopArena_mt (size_unit_t units)
{
    return (SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][TOP_ARENA])));
}

#endif /* MT */

/******************************************************************************
 *
 * function:
 *   void SAC_HM_FreeTopArena_mt(SAC_HM_header_t *addr)
 *
 * description:
 *
 *   This function de-allocates memory chunks in the top arena.
 *   Concerning functionality, it is equivalent to SAC_HM_FreeLargeChunk().
 *   This function, however, is used during multi-threaded execution;
 *   access to the top arena´s data structures is protected by a
 *   mutex lock.
 *
 *   This function is also considered a wrapper function; the function
 *   being wrapped is simply inlined because it consists of a few lines
 *   of code only.
 *
 ******************************************************************************/

#ifdef MT

void
SAC_HM_FreeTopArena_mt (SAC_HM_header_t *addr)
{
    SAC_HM_arena_t *arena = &(SAC_HM_arenas[0][TOP_ARENA]);
    SAC_HM_header_t *freep = addr - 2;

    pthread_mutex_lock (&SAC_HM_top_arena_lock);

    DIAG_CHECK_ALLOCPATTERN_LARGECHUNK (freep, arena->num);
    DIAG_SET_FREEPATTERN_LARGECHUNK (freep);
    DIAG_INC (arena->cnt_free);

    SAC_HM_LARGECHUNK_PREVSIZE (freep + SAC_HM_LARGECHUNK_SIZE (freep))
      = SAC_HM_LARGECHUNK_SIZE (freep);

    SAC_HM_LARGECHUNK_NEXTFREE (freep) = SAC_HM_LARGECHUNK_NEXTFREE (arena->freelist);
    SAC_HM_LARGECHUNK_NEXTFREE (arena->freelist) = freep;

    pthread_mutex_unlock (&SAC_HM_top_arena_lock);
}

#else /* MT */

void
SAC_HM_FreeTopArena_mt (SAC_HM_header_t *addr)
{
    SAC_HM_FreeLargeChunk (addr, &(SAC_HM_arenas[0][TOP_ARENA]));
}

#endif /* MT */

/******************************************************************************
 *
 * function:
 *   void SAC_HM_FreeTopArena_at(SAC_HM_header_t *addr)
 *
 * description:
 *
 *   This function de-allocates memory chunks in the top arena.
 *   Concerning functionality, it is equivalent to SAC_HM_FreeLargeChunk().
 *   This function, however, is used whenever execution might be either
 *   multi-threaded or single-threaded.
 *
 *   In this case, the actual execution state is determined dynamically
 *   and access to the top arena´s data structures is protexted by a
 *   mutex lock if necessary.
 *
 *   This function is also considered a wrapper function; the function
 *   being wrapped is simply inlined because it consists of a few lines
 *   of code only.
 *
 ******************************************************************************/

#ifdef MT

void
SAC_HM_FreeTopArena_at (SAC_HM_header_t *addr)
{
    SAC_HM_arena_t *arena = &(SAC_HM_arenas[0][TOP_ARENA]);
    SAC_HM_header_t *freep = addr - 2;
    const int multi_threaded = !SAC_MT_not_yet_parallel;

    if (multi_threaded) {
        pthread_mutex_lock (&SAC_HM_top_arena_lock);
    }

    DIAG_CHECK_ALLOCPATTERN_LARGECHUNK (freep, arena->num);
    DIAG_SET_FREEPATTERN_LARGECHUNK (freep);
    DIAG_INC (arena->cnt_free);

    SAC_HM_LARGECHUNK_PREVSIZE (freep + SAC_HM_LARGECHUNK_SIZE (freep))
      = SAC_HM_LARGECHUNK_SIZE (freep);

    SAC_HM_LARGECHUNK_NEXTFREE (freep) = SAC_HM_LARGECHUNK_NEXTFREE (arena->freelist);
    SAC_HM_LARGECHUNK_NEXTFREE (arena->freelist) = freep;

    if (multi_threaded) {
        pthread_mutex_unlock (&SAC_HM_top_arena_lock);
    }
}

#else /* MT */

void
SAC_HM_FreeTopArena_at (SAC_HM_header_t *addr)
{
    SAC_HM_FreeLargeChunk (addr, &(SAC_HM_arenas[0][TOP_ARENA]));
}

#endif /* MT */
