/*
 *
 * $Log$
 * Revision 1.1  2000/01/03 17:33:17  cg
 * Initial revision
 *
 *
 */

/*
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

#include "sac_message.h"
#include "heapmgr.h"

void
SAC_HM_Setup_mt (unsigned int num_threads,
                 size_byte_t initial_master_arena_of_arenas_size,
                 size_byte_t initial_worker_arena_of_arenas_size,
                 size_byte_t initial_top_arena_size)
{
    return;
}

void
SAC_HM_Setup (size_byte_t initial_master_arena_of_arenas_size,
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
SAC_HM_MallocLargeChunk (size_unit_t units, SAC_HM_arena_t *arena)
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
SAC_HM_MallocAnyChunk_st (size_byte_t size)
{
    return (malloc (size));
}

void *
SAC_HM_MallocAnyChunk_mt (size_byte_t size, unsigned int thread_id)
{
    return (malloc (size));
}

void *
SAC_HM_MallocSmallChunk_at (size_unit_t units, int arena_num)
{
    return (malloc (units * UNIT_SIZE));
}

void *
SAC_HM_MallocLargeChunk_at (size_unit_t units, int arena_num)
{
    return (malloc (units * UNIT_SIZE));
}

void *
SAC_HM_MallocTopArena_at (size_unit_t units)
{
    return (malloc (units * UNIT_SIZE));
}

void *
SAC_HM_MallocTopArena_mt (size_unit_t units)
{
    return (malloc (units * UNIT_SIZE));
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
