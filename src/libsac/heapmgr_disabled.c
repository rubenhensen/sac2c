/*
 *
 * $Log$
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
 *
 *
 *
 *
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
SAC_HM_MallocLargeChunk (size_unit_t units, SAC_HM_arena_t *arena)
{
    return (malloc (units * UNIT_SIZE));
}

void *
SAC_HM_MallocTopArena (size_unit_t units, SAC_HM_arena_t *arena)
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
