/*
 *
 * $Log$
 * Revision 1.4  1999/09/17 14:33:34  cg
 * New version of SAC heap manager:
 *  - no special API functions for top arena.
 *  - coalascing is always done deferred.
 *  - no doubly linked free lists any more.
 *
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
 * file:  heapmgr.h
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

#ifndef HEAPMGR_H
#define HEAPMGR_H

#define SAC_DO_PHM 1
#include "sac_heapmgr.h"

/*
 * Initialization of some basic values/sizes.
 */

#define KB 1024
#define MB (KB * KB)
#define SBRK_CHUNK (MB)

#define ARENA_OF_ARENAS 0
#define TOP_ARENA (NUM_ARENAS - 1)

#define DIAG_FREEPATTERN -123456
#define DIAG_ALLOCPATTERN 123456

#define LARGECHUNK_SIZE(header) (((header) + 1)->data1.size)
#define LARGECHUNK_PREVSIZE(header) (((header) + 0)->data3.prevsize)
#define LARGECHUNK_ARENA(header) (((header) + 1)->data1.arena)
#define LARGECHUNK_PREVFREE(header) (((header) + 2)->data2.prevfree)
#define LARGECHUNK_NEXTFREE(header) (((header) + 2)->data2.nextfree)

#define SMALLCHUNK_SIZE(header) (((header) + 0)->data1.size)
#define SMALLCHUNK_ARENA(header) (((header) + 0)->data1.arena)
#define SMALLCHUNK_PREVFREE(header) (((header) + 1)->data2.prevfree)
#define SMALLCHUNK_NEXTFREE(header) (((header) + 1)->data2.nextfree)

#define LARGECHUNK_DIAG(header) (((header) + 0)->data3.diag)
#define SMALLCHUNK_DIAG(header) (((header) + 0)->data1.size)

#ifndef NULL
#define NULL ((void *)0)
#endif

extern size_byte_t SAC_HM_pagesize;

#ifdef DIAG
extern unsigned long int SAC_HM_call_sbrk;
extern unsigned long int SAC_HM_call_malloc;
extern unsigned long int SAC_HM_call_free;
extern unsigned long int SAC_HM_call_realloc;
extern unsigned long int SAC_HM_call_calloc;
extern unsigned long int SAC_HM_call_valloc;
extern unsigned long int SAC_HM_call_memalign;
extern unsigned long int SAC_HM_heapsize;
#endif

#ifdef DIAG

extern void SAC_HM_CheckAllocDiagPattern (size_unit_t diag, int arena_num);
extern void SAC_HM_CheckFreeDiagPattern (size_unit_t diag, int arena_num);
extern void SAC_HM_CheckDiagPatternAnyChunk (SAC_HM_header_t *addr);

#define DIAG_INC(cnt) (cnt)++
#define DIAG_DEC(cnt) (cnt)--
#define DIAG_ADD(cnt, val) (cnt) += (val)
#define DIAG_SET(cnt, val) (cnt) = (val)

#define DIAG_SET_FREEPATTERN_SMALLCHUNK(freep) SMALLCHUNK_DIAG (freep) = DIAG_FREEPATTERN
#define DIAG_SET_ALLOCPATTERN_SMALLCHUNK(freep)                                          \
    SMALLCHUNK_DIAG (freep) = DIAG_ALLOCPATTERN

#define DIAG_SET_FREEPATTERN_LARGECHUNK(freep) LARGECHUNK_DIAG (freep) = DIAG_FREEPATTERN
#define DIAG_SET_ALLOCPATTERN_LARGECHUNK(freep)                                          \
    LARGECHUNK_DIAG (freep) = DIAG_ALLOCPATTERN

#define DIAG_CHECK_FREEPATTERN_SMALLCHUNK(freep, arena_num)                              \
    SAC_HM_CheckFreeDiagPattern (SMALLCHUNK_DIAG (freep), arena_num)

#define DIAG_CHECK_ALLOCPATTERN_SMALLCHUNK(freep, arena_num)                             \
    SAC_HM_CheckAllocDiagPattern (SMALLCHUNK_DIAG (freep), arena_num)

#define DIAG_CHECK_FREEPATTERN_LARGECHUNK(freep, arena_num)                              \
    SAC_HM_CheckFreeDiagPattern (LARGECHUNK_DIAG (freep), arena_num)

#define DIAG_CHECK_ALLOCPATTERN_LARGECHUNK(freep, arena_num)                             \
    SAC_HM_CheckAllocDiagPattern (LARGECHUNK_DIAG (freep), arena_num)

#define DIAG_CHECK_ALLOCPATTERN_ANYCHUNK(addr) SAC_HM_CheckDiagPatternAnyChunk (addr)

#else /* DIAG */

#define DIAG_INC(cnt)
#define DIAG_DEC(cnt)
#define DIAG_ADD(cnt, val)
#define DIAG_SET(cnt, val)

#define DIAG_SET_FREEPATTERN_SMALLCHUNK(freep)
#define DIAG_SET_ALLOCPATTERN_SMALLCHUNK(freep)

#define DIAG_SET_FREEPATTERN_LARGECHUNK(freep)
#define DIAG_SET_ALLOCPATTERN_LARGECHUNK(freep)

#define DIAG_CHECK_FREEPATTERN_SMALLCHUNK(freep, arena_num)
#define DIAG_CHECK_ALLOCPATTERN_SMALLCHUNK(freep, arena_num)

#define DIAG_CHECK_FREEPATTERN_LARGECHUNK(freep, arena_num)
#define DIAG_CHECK_ALLOCPATTERN_LARGECHUNK(freep, arena_num)

#define DIAG_CHECK_ALLOCPATTERN_ANYCHUNK(addr)

#endif /* DIAG */

#endif /* HEAPMGR_H */
