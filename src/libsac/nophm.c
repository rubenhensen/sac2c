/*
 *
 * $Log$
 * Revision 3.2  2000/12/29 14:26:48  cg
 * When compiling for multithreaded execution, any function gets one additional
 * parameter to hold the thread ID, which is needed for heap management.
 * Thread-specific data is only used by malloc().
 *
 * Revision 3.1  2000/11/20 18:02:45  sacbase
 * new release made
 *
 * Revision 1.2  2000/05/24 09:32:35  cg
 * Added dummy definition of SAC_HM_ShowDiagnostics().
 * Heap manager diagnostics are now printed after termination through
 * runtime error.
 *
 * Revision 1.1  2000/01/17 16:49:15  cg
 * Initial revision
 *
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

#include <stdlib.h>

#include "sac_message.h"

#define SAC_DO_MULTITHREAD 0
#include "sac_mt.h"
#undef SAC_DO_MULTITHREAD

#define SAC_DO_PHM 1
#include "sac_heapmgr.h"
#undef SAC_DO_PHM

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

void
SAC_HM_ShowDiagnostics ()
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
