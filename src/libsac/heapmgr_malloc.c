/*
 *
 * $Log$
 * Revision 1.3  1999/07/29 07:35:41  cg
 * Two new performance related features added to SAC private heap
 * management:
 *   - pre-splitting for arenas with fixed size chunks.
 *   - deferred coalascing for arenas with variable chunk sizes.
 *
 * Revision 1.2  1999/07/16 09:41:16  cg
 * Added facilities for heap management diagnostics.
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
 *
 *
 *
 *
 *
 *****************************************************************************/

#include "heapmgr.h"

void *
malloc (size_byte_t size)
{
    size_unit_t units;

    DIAG_INC (SAC_HM_call_malloc);

    if (size <= ARENA_4_MAXCS_BYTES) {
        /* Now, it's arena 1, 2, 3, or 4. */
        if (size <= ARENA_2_MAXCS_BYTES) {
            /* Now, it's arena 1 or 2. */
            if (size <= ARENA_1_MAXCS_BYTES) {
                return (SAC_HM_MallocSmallChunkPresplit (2, &(SAC_HM_arenas[1]), 16));
            } else {
                return (SAC_HM_MallocSmallChunkPresplit (4, &(SAC_HM_arenas[2]), 16));
            }
        } else {
            /* Now, it's arena 3 or 4. */
            if (size <= ARENA_3_MAXCS_BYTES) {
                return (SAC_HM_MallocSmallChunk (8, &(SAC_HM_arenas[3])));
            } else {
                return (SAC_HM_MallocSmallChunk (16, &(SAC_HM_arenas[4])));
            }
        }
    } else {
        units = ((size - 1) / UNIT_SIZE) + 3;

        if (units < ARENA_7_MINCS) {
            /* Now, it's arena 5 or 6. */
            if (units < ARENA_6_MINCS) {
                return (SAC_HM_MallocLargeChunkNoCoalasce (units, &(SAC_HM_arenas[5])));
            } else {
                return (SAC_HM_MallocLargeChunkNoCoalasce (units, &(SAC_HM_arenas[6])));
            }
        } else {
            /* Now, it's arena 7 or 8. */
            if (units < ARENA_8_MINCS) {
                return (SAC_HM_MallocLargeChunkNoCoalasce (units, &(SAC_HM_arenas[7])));
            } else {
                return (
                  SAC_HM_MallocTopArenaDeferredCoalasce (units, &(SAC_HM_arenas[8])));
            }
        }
    }
}

void
free (void *addr)
{
    SAC_HM_arena_t *arena;

    DIAG_INC (SAC_HM_call_free);

    if (addr != NULL) {
        DIAG_CHECK_ALLOCPATTERN_ANYCHUNK ((SAC_HM_header_t *)addr);
        arena = SAC_HM_ADDR_2_ARENA (addr);
        arena->freefun ((SAC_HM_header_t *)addr, arena);
    }
}
