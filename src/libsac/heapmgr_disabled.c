/*
 *
 * $Log$
 * Revision 1.3  1999/09/17 14:33:34  cg
 * New version of SAC heap manager:
 *  - no special API functions for top arena.
 *  - coalascing is always done deferred.
 *  - no doubly linked free lists any more.
 *
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

void
SAC_HM_Setup (size_byte_t initial_arena_of_arenas_size,
              size_byte_t initial_top_arena_size)
{
    return;
}

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
