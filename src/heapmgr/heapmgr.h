/*
 *
 * $Log$
 * Revision 1.2  2000/01/17 16:25:58  cg
 * Moved some declarations to sac_heapmgr.h
 *
 * Revision 1.1  2000/01/03 17:33:17  cg
 * Initial revision
 *
 *
 */

/*
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
 * file:   heapmgr.h
 *
 * prefix: SAC_HM
 *
 * description:
 *
 *   This file contains internal declarations for the SAC
 *   heap manager, i.e. declarations which are exclusively needed for
 *   compilation of the heap manager, but not for compilation of SAC
 *   code that makes usage of the heap manager. These public declarations
 *   may be found in src/runtime/sac_heapmgr.h.
 *
 *   This file also hosts all general documentation of the heap manager.
 *
 */

/*****************************************************************************
 *
 *   The SAC Private Heap Manager.
 *
 *****************************************************************************
 *
 *   SAC uses a private heap management facility for performance reasons.
 *   The main reason for this is the lousy runtime performance of the Solaris
 *   implementations of malloc() and free() when it comes to multi-threaded
 *   program execution. Furthermore, the private heap manager allows for a
 *   richer interface that exploits static knowledge of the source code for
 *   optimizing memory allocation and de-allocation tasks.
 *
 *
 * arenas:
 *
 *   The heap is organized in so-called arenas. An arena is an organizational
 *   entity for requested heap chunks of a certain size range. Each arena is
 *   associated with a minimum and maximum chunk size, a bin size, an allocation
 *   and de-allocation function, and a free list. A characteristic of this free
 *   list is that it is never empty; it always has a first dummy entry with size 0.
 *   This allows for a more efficient implementation of the free list handling.
 *
 *   There are 4 different variations of arenas:
 *    - small chunk arenas,
 *    - large chunk arenas,
 *    - top arena,
 *    - arena of arenas.
 *
 *   In a small chunk arena, each chunk of memory has the same size which is
 *   characteristic for the arena. This allows fast allocation/de-allocation
 *   strategies. However, it is only feasible for rather small memory requests
 *   because it implies significant internal memory fragmentation.
 *
 *   In large chunk arenas, each allocated chunk of memory is associated with
 *   its size, i.e. different chunk sizes within the range of the given
 *   arena may occur.
 *
 *   The top arena basically is a large chunk arena. It satisfies the
 *   class of largest memory requests. The top arena, however, is the only
 *   arena which may arbitrarily be extended by requesting more memory from the
 *   operating system.
 *
 *   Large and small chunk arenas are organized as subsequently allocated bins of
 *   contiguous memory. These bins are allocated in the so-called arena of arenas.
 *   The bins of the arena of arenas itself are allocated within the top arena.
 *   Note that bins are never de-allocated. The reason is that it is too costly
 *   to identify completely empty bins.
 *
 *
 * bins:
 *
 *   Except for the top arena, arenas are organized in bins of contiguous
 *   memory. The first time, a memory request for a certain arena occurs,
 *   a bin is allocated within the arena of arenas. Each time a new request
 *   cannot be satisfied within the already allocated bins of the arena,
 *   a new bin is allocated.
 *
 *
 * initialization:
 *
 *   When the heap manager is initialized upon program startup, a certain
 *   configurable amount of memory is requested from the operating system
 *   and the following memory layout is established:
 *
 *     |                  |
 *     +------------------+
 *     |                  |
 *     |  arena of arenas |
 *     |                  |
 *     +------------------+
 *     |                  |
 *     |  top arena       |
 *     |                  |
 *     +------------------+
 *     |                  |
 *
 *   The default sizes are 1MB each for the first bin of the arena of arenas
 *   and the top arena.
 *
 *
 * administration data layout:
 *
 *   Memory is always allocated in so-called units (currently sizeof(double)).
 *   This assures proper alignment as required by malloc().
 *
 *   Each separately indentifiable chunk of memory (allocated or de-allocated)
 *   is associated with some administrative information:
 *
 *   Small chunks:
 *
 *     |                  |
 *     +==================+
 *     | size/diag        |
 *     +------------------+
 *     | arena_ptr        |
 *     +==================+
 *     | nextfree_ptr     |     <-- returned pointer
 *     +------------------+
 *     |                  |
 *     +==================+
 *     |                  |
 *
 *   The size field conatains the size of the memory chunk in units. This
 *   information does exclusively exist for those chunks that are not of the
 *   fixed chunk size of the small chunk arena. For example, each time a new
 *   bin is allocated it is kept as a whole. Subsequently small chunks of the
 *   fixed chunk size are split from this. These small fixed size chunks are
 *   never coalasced again and don't have size information themselves.
 *   The variable sized initial chunk in a small chunk arena is called a
 *   wilderness chunk.
 *
 *   The arena_ptr is a pointer back to the arena data structure which contains
 *   all the characteristic information about the arena. This pointer allows
 *   a quick determination of the free list and the de-allocation function
 *   upon a de-allocation request for a specific memory chunk (free).
 *
 *   The nextfree_ptr points to the next entry of the free list. This is only
 *   available when the chunk is currently un-allocated. Otherwise the memory
 *   cell belongs to the memory returned by the allocator. This technique helps
 *   to minimize the administrative overhead in terms of memory consumption.
 *
 *   Split chunks (of the fixed chunk size) use the size field for storage of a
 *   diagnostic pattern iff diagnostic heap management is activated. This pattern
 *   allows to detect certain forms of corruptions of the internal heap manager
 *   data structures caused by programs that write over the allocated amount of
 *   memory.
 *
 *   Large chunks:
 *
 *     |                  |
 *     +==================+
 *     | prevsize         |
 *     +------------------+
 *     | diag             |
 *     +==================+
 *     | size             |
 *     +------------------+
 *     | arena_ptr        |
 *     +==================+
 *     | nextfree_ptr     |     <-- returned pointer
 *     +------------------+
 *     |                  |
 *     +==================+
 *     |                  |
 *
 *   In contrast to the field entries of a small chunk administration block, a large
 *   chunk administration block always contains the diagnostic pattern as well as
 *   the chunk size in units.
 *
 *   Additionally the size of the preceding chunk (in units) is stored. A negative
 *   number in this field means that the preceding chunk is currently allocated.
 *   This information is needed for efficient coalascing of neighboring un-allocated
 *   memory chunks.
 *
 *
 *
 *****************************************************************************/

#ifndef HEAPMGR_H
#define HEAPMGR_H

#ifdef MT

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199506L
#endif

#ifndef _REENTRANT
#define _REENTRANT
#endif

#define SAC_DO_MULTITHREAD 1
#define SAC_DO_THREADS_STATIC 1
#else
#define SAC_DO_MULTITHREAD 0
#endif /*  MT  */

#include "sac_mt.h"

#undef SAC_DO_MULTITHREAD
#undef SAC_DO_THREADS_STATIC

#define SAC_DO_PHM 1
#define SAC_COMPILE_SACLIB
#include "sac_heapmgr.h"
#undef SAC_COMPILE_LIB

/*
 * Macro definition of sbrk() system call
 */

#define SBRK(size) sbrk ((intptr_t) (size))

/*
 * Initialization of some basic values/sizes.
 */

#define KB 1024
#define MB (KB * KB)
#define SBRK_CHUNK (MB)

#define DIAG_FREEPATTERN -123456
#define DIAG_ALLOCPATTERN 123456

#ifndef NULL
#define NULL ((void *)0)
#endif

/*
 * Declarations of internal heap manager global variables and functions
 */

extern void SAC_HM_OutOfMemory (SAC_HM_size_byte_t request);
extern SAC_HM_header_t *SAC_HM_AllocateNewBinInArenaOfArenas (SAC_HM_size_unit_t units,
                                                              SAC_HM_arena_t *arena);
extern SAC_HM_header_t *SAC_HM_ExtendTopArenaWilderness (SAC_HM_size_unit_t units);

extern SAC_HM_arena_t SAC_HM_arenas[][SAC_HM_NUM_ARENAS + 2];

#ifdef DIAG
extern unsigned long int SAC_HM_call_sbrk;
extern unsigned long int SAC_HM_call_malloc;
extern unsigned long int SAC_HM_call_realloc;
extern unsigned long int SAC_HM_call_calloc;
extern unsigned long int SAC_HM_call_valloc;
extern unsigned long int SAC_HM_call_memalign;
extern unsigned long int SAC_HM_heapsize;

extern void SAC_HM_ClearDiagCounters (SAC_HM_arena_t *arena);
extern void SAC_HM_AddDiagCounters (SAC_HM_arena_t *arena, SAC_HM_arena_t *add_arena);

extern void SAC_HM_CheckAllocDiagPattern (SAC_HM_size_unit_t diag, int arena_num);
extern void SAC_HM_CheckFreeDiagPattern (SAC_HM_size_unit_t diag, int arena_num);
extern void SAC_HM_CheckDiagPatternAnyChunk (SAC_HM_header_t *addr);

/*
 * Definition of macros for diagnostic heap management
 */

#define DIAG_INC(cnt) (cnt)++
#define DIAG_DEC(cnt) (cnt)--
#define DIAG_ADD(cnt, val) (cnt) += (val)
#define DIAG_SET(cnt, val) (cnt) = (val)

#ifdef MT
#define DIAG_INC_LOCK(cnt)                                                               \
    {                                                                                    \
        SAC_MT_ACQUIRE_LOCK (SAC_HM_diag_counter_lock);                                  \
        DIAG_INC (cnt);                                                                  \
        SAC_MT_RELEASE_LOCK (SAC_HM_diag_counter_lock);                                  \
    }

#define DIAG_DEC_LOCK(cnt)                                                               \
    {                                                                                    \
        SAC_MT_ACQUIRE_LOCK (SAC_HM_diag_counter_lock);                                  \
        DIAG_DEC (cnt);                                                                  \
        SAC_MT_RELEASE_LOCK (SAC_HM_diag_counter_lock);                                  \
    }

#define DIAG_ADD_LOCK(cnt, val)                                                          \
    {                                                                                    \
        SAC_MT_ACQUIRE_LOCK (SAC_HM_diag_counter_lock);                                  \
        DIAG_ADD (cnt, val);                                                             \
        SAC_MT_RELEASE_LOCK (SAC_HM_diag_counter_lock);                                  \
    }

#define DIAG_SET_LOCK(cnt, val)                                                          \
    {                                                                                    \
        SAC_MT_ACQUIRE_LOCK (SAC_HM_diag_counter_lock);                                  \
        DIAG_SET (cnt, val);                                                             \
        SAC_MT_RELEASE_LOCK (SAC_HM_diag_counter_lock);                                  \
    }

#else /* MT */
#define DIAG_INC_LOCK(cnt) DIAG_INC (cnt)
#define DIAG_DEC_LOCK(cnt) DIAG_DEC (cnt)
#define DIAG_ADD_LOCK(cnt, val) DIAG_ADD (cnt, val)
#define DIAG_SET_LOCK(cnt, val) DIAG_SET (cnt, val)
#endif /* MT */

#define DIAG_SET_FREEPATTERN_SMALLCHUNK(freep)                                           \
    SAC_HM_SMALLCHUNK_DIAG (freep) = DIAG_FREEPATTERN
#define DIAG_SET_ALLOCPATTERN_SMALLCHUNK(freep)                                          \
    SAC_HM_SMALLCHUNK_DIAG (freep) = DIAG_ALLOCPATTERN

#define DIAG_SET_FREEPATTERN_LARGECHUNK(freep)                                           \
    SAC_HM_LARGECHUNK_DIAG (freep) = DIAG_FREEPATTERN
#define DIAG_SET_ALLOCPATTERN_LARGECHUNK(freep)                                          \
    SAC_HM_LARGECHUNK_DIAG (freep) = DIAG_ALLOCPATTERN

#define DIAG_CHECK_FREEPATTERN_SMALLCHUNK(freep, arena_num)                              \
    SAC_HM_CheckFreeDiagPattern (SAC_HM_SMALLCHUNK_DIAG (freep), arena_num)

#define DIAG_CHECK_ALLOCPATTERN_SMALLCHUNK(freep, arena_num)                             \
    SAC_HM_CheckAllocDiagPattern (SAC_HM_SMALLCHUNK_DIAG (freep), arena_num)

#define DIAG_CHECK_FREEPATTERN_LARGECHUNK(freep, arena_num)                              \
    SAC_HM_CheckFreeDiagPattern (SAC_HM_LARGECHUNK_DIAG (freep), arena_num)

#define DIAG_CHECK_ALLOCPATTERN_LARGECHUNK(freep, arena_num)                             \
    SAC_HM_CheckAllocDiagPattern (SAC_HM_LARGECHUNK_DIAG (freep), arena_num)

#define DIAG_CHECK_ALLOCPATTERN_ANYCHUNK(addr) SAC_HM_CheckDiagPatternAnyChunk (addr)

#else /* DIAG */

#define DIAG_INC(cnt)
#define DIAG_DEC(cnt)
#define DIAG_ADD(cnt, val)
#define DIAG_SET(cnt, val)

#define DIAG_INC_LOCK(cnt)
#define DIAG_DEC_LOCK(cnt)
#define DIAG_ADD_LOCK(cnt, val)
#define DIAG_SET_LOCK(cnt, val)

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
