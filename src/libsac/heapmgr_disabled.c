/*
 *
 * $Log$
 * Revision 1.2  1999/08/02 10:22:53  cg
 * Added dummy function definitions for new heap manager API functions
 * for pre-splitting and deferred coalascing.
 *
 * Revision 1.1  1999/07/08 12:28:56  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:
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

#include "sac_message.h"

#include "heapmgr.h"

SAC_HM_arena_t SAC_HM_arenas[NUM_ARENAS];

void *
SAC_HM_MallocSmallChunk (size_unit_t units, SAC_HM_arena_t *arena)
{
    return (malloc (units * UNIT_SIZE));
}

void *
SAC_HM_MallocSmallChunkPresplit (size_unit_t units, SAC_HM_arena_t *arena, int presplit)
{
    return (malloc (units * UNIT_SIZE));
}

void *
SAC_HM_MallocLargeChunk (size_unit_t units, SAC_HM_arena_t *arena)
{
    return (malloc (units * UNIT_SIZE));
}

void *
SAC_HM_MallocLargeChunkNoCoalasce (size_unit_t units, SAC_HM_arena_t *arena)
{
    return (malloc (units * UNIT_SIZE));
}

void *
SAC_HM_MallocLargeChunkDeferredCoalasce (size_unit_t units, SAC_HM_arena_t *arena)
{
    return (malloc (units * UNIT_SIZE));
}

void *
SAC_HM_MallocTopArena (size_unit_t units, SAC_HM_arena_t *arena)
{
    return (malloc (units * UNIT_SIZE));
}

void *
SAC_HM_MallocTopArenaDeferredCoalasce (size_unit_t units, SAC_HM_arena_t *arena)
{
    return (malloc (units * UNIT_SIZE));
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
SAC_HM_FreeLargeChunkNoCoalasce (SAC_HM_header_t *addr, SAC_HM_arena_t *arena)
{
    free (addr);
}

void
SAC_HM_FreeTopArena (SAC_HM_header_t *addr, SAC_HM_arena_t *arena)
{
    free (addr);
}

void *
SAC_HM_MallocCheck (unsigned long int size)
{
    void *tmp;

    tmp = malloc (size);

    if (tmp == NULL) {
        SAC_RuntimeError ("Unable to allocate %lu bytes of memory", size);
    }

    return (tmp);
}
