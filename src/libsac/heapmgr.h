/*
 *
 * $Log$
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

#ifndef LIBSAC_HEAPMGR_H
#define LIBSAC_HEAPMGR_H

#define SAC_DO_PHM 1
#include "sac_heapmgr.h"

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

extern void SAC_HM_CheckAllocPattern (size_unit_t diag, int arena_num);
extern void SAC_HM_CheckFreePattern (size_unit_t diag, int arena_num);
extern void SAC_HM_CheckAllocPatternAnyChunk (SAC_HM_header_t *addr);

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
    SAC_HM_CheckFreePattern (SMALLCHUNK_DIAG (freep), arena_num)

#define DIAG_CHECK_ALLOCPATTERN_SMALLCHUNK(freep, arena_num)                             \
    SAC_HM_CheckAllocPattern (SMALLCHUNK_DIAG (freep), arena_num)

#define DIAG_CHECK_FREEPATTERN_LARGECHUNK(freep, arena_num)                              \
    SAC_HM_CheckFreePattern (LARGECHUNK_DIAG (freep), arena_num)

#define DIAG_CHECK_ALLOCPATTERN_LARGECHUNK(freep, arena_num)                             \
    SAC_HM_CheckAllocPattern (LARGECHUNK_DIAG (freep), arena_num)

#define DIAG_CHECK_ALLOCPATTERN_ANYCHUNK(addr) SAC_HM_CheckAllocPatternAnyChunk (addr)

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

#endif /* LIBSAC_HEAPMGR_H */
