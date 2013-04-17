/*****************************************************************************
 *
 * file:   nophm.c
 *
 * prefix: SAC_HM
 *
 * description:
 *
 *   This file contains dummy definitions of all public functions of the SAC
 *   heap manager. These may be needed if a compiled SAC program, e.g. from
 *   a module implementation, contains calls to these functions but shall
 *   not be linked with the heap manager library (-noPHM). In this case, these
 *   calls are linked with the functions defined here, which map them either
 *   to malloc() or free().
 *
 *****************************************************************************/

#include <stdlib.h>

#define SAC_DO_MULTITHREAD 0
#define SAC_DO_PHM 1

#include "sac.h"

#undef SAC_DO_PHM
#undef SAC_DO_MULTITHREAD

SAC_HM_arena_t SAC_HM_arenas[1][SAC_HM_NUM_ARENAS + 2];

void *
SAC_HM_MallocSmallChunk (SAC_HM_size_unit_t units, SAC_HM_arena_t *arena)
{
    return (malloc (units * SAC_HM_UNIT_SIZE));
}

void *
SAC_HM_MallocLargeChunk (SAC_HM_size_unit_t units, SAC_HM_arena_t *arena)
{
    return (malloc (units * SAC_HM_UNIT_SIZE));
}

void
SAC_HM_FreeSmallChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena)
{
    free (addr);
}

void
SAC_HM_FreeLargeChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena)
{
    free (addr);
}

void
SAC_HM_FreeTopArena_mt (SAC_HM_header_t *addr)
{
    free (addr);
}

void
SAC_HM_FreeTopArena_at (SAC_HM_header_t *addr)
{
    free (addr);
}

void *
SAC_HM_MallocAnyChunk_st (SAC_HM_size_byte_t size)
{
    return (malloc (size));
}

void *
SAC_HM_MallocAnyChunk_mt (SAC_HM_size_byte_t size, unsigned int thread_id)
{
    return (malloc (size));
}

void *
SAC_HM_MallocAnyChunk_at (SAC_HM_size_byte_t size, unsigned int thread_id)
{
    return (malloc (size));
}

void *
SAC_HM_MallocSmallChunk_at (SAC_HM_size_unit_t units, int arena_num)
{
    return (malloc (units * SAC_HM_UNIT_SIZE));
}

void *
SAC_HM_MallocLargeChunk_at (SAC_HM_size_unit_t units, int arena_num)
{
    return (malloc (units * SAC_HM_UNIT_SIZE));
}

void *
SAC_HM_MallocTopArena_at (SAC_HM_size_unit_t units)
{
    return (malloc (units * SAC_HM_UNIT_SIZE));
}

void *
SAC_HM_MallocTopArena_mt (SAC_HM_size_unit_t units)
{
    return (malloc (units * SAC_HM_UNIT_SIZE));
}

void *
SAC_HM_MallocDesc (SAC_HM_header_t *addr, SAC_HM_size_byte_t size,
                   SAC_HM_size_byte_t desc_size)
{
    return (malloc (desc_size));
}

void
SAC_HM_FreeDesc (SAC_HM_header_t *addr)
{
    free (addr);
}

void
SAC_HM_ShowDiagnostics (void)
{
}

void *
SAC_HM_MallocCheck (SAC_HM_size_byte_t size)
{
    void *tmp;

    tmp = malloc (size);

    if (tmp == NULL) {
        SAC_RuntimeError ("Unable to allocate %lu bytes of memory",
                          (unsigned long int)size);
    }

    return (tmp);
}

unsigned int
SAC_HM_CurrentThreadId (void)
{
    /* This should never be called as no PHM is installed and the MT should assign ID
     * itself. */
    SAC_RuntimeError ("SAC_HM_CurrentThreadId: in -nophm this should not be called!");
    return 0xdeadbeef;
}

int
SAC_HM_DiscoversThreads (void)
{
    /* This informs the MT layer that it has to assign threads IDs to its threads.
     * The downside is that no other threads than those created by the SAC runtime
     * are allowed in the environment. */
    return 0;
}
