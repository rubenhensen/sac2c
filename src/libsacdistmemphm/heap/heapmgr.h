/*****************************************************************************
 *
 * file:   heapmgr.h
 *
 * prefix: SAC_DISTMEM_HM
 *
 * description:
 *
 *   This file contains internal declarations for the SAC distributed memory
 *   heap manager, i.e. declarations which are exclusively needed for
 *   compilation of the distributed memory heap manager, but not for compilation of SAC
 *   code that makes usage of the heap manager. These public declarations
 *   may be found in src/runtime/distmemphm_h.
 *
 *   This file also hosts all general documentation of the distributed memory heap
 * manager.
 *
 */

/*****************************************************************************
 *
 *   The SAC Distributed Memory Private Heap Manager.
 *
 *****************************************************************************
 *
 *   The SAC distributed memory backend requires a private heap management facility
 *   because the dsm segment size is fixed at program startup.
 *   This heap manager is only used for memory allocation in the DSM segment. The
 *traditional heap manager may or may not be used for memory allocation outside of the DSM
 *segment. Only distributed arrays (but not their descriptors!), out parameters of
 *function applications with side effects and (in the future) intermediate results of
 *distributed fold with-loops are allocated in the DSM segment.
 *
 *   The distributed memory heap manager is based on the traditional SAC heap manager
 *   but has been simplified in some respects:
 *    - It does not have to deal with different backends since it is specific for the
 *distributed memory backend.
 *    - It does not have to deal with multi-threading because distributed arrays can only
 *be allocated in single threaded execution. (Also at this point multithreading cannot be
 *      used with the distributed memory backend.)
 *
 *   Also, the distributed memory heap manager is not responsible for the initial
 *   allocation of the DSM segment. This is handled by the libsacdistmem because it
 *   depends on the underlying communication library and we want to avoid having
 *   a variant of libsacdistmemphm for each communication library.
 *
 *
 *
 * Arenas:
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
 *   class of largest memory requests. Other than with the traditional SAC
 *   heap manager, the distributed memory heap manager does not allow the top
 *   arena to be extended. Instead, an out of memory runtime error is raised.
 *
 *   Large and small chunk arenas are organized as subsequently allocated bins of
 *   contiguous memory. These bins are allocated in the so-called arena of arenas.
 *   The bins of the arena of arenas itself are allocated within the top arena.
 *   Note that bins are never de-allocated. The reason is that it is too costly
 *   to identify completely empty bins.
 *
 *
 * Bins:
 *
 *   Except for the top arena, arenas are organized in bins of contiguous
 *   memory. The first time, a memory request for a certain arena occurs,
 *   a bin is allocated within the arena of arenas. Each time a new request
 *   cannot be satisfied within the already allocated bins of the arena,
 *   a new bin is allocated.
 *
 *
 * Initialization:
 *
 *   When the distributed memory heap manager is initialized upon program startup, it is
 *passed a reference to the DSM memory segment along with its size. The following memory
 *layout is established.
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
 * Administration data layout:
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
 *  Descriptor Allocation Optimization (DAO)
 *
 *   This optimization of the traditional SAC heap manager is also used by the distributed
 *   memory heap manager. Descriptors of distributed arrays are not allocated in the DSM
 *segment, but when broadcasting AKD or AKS variables, their descriptors are also
 *allocated in the DSM segment.
 *
 *
 *  Memory Size Cache Adjustment (MSCA)
 *
 *   MSCA is yet another heap management trick which speeds up program execution
 *   in many situations. Whenever array sizes are close to multiples of the
 *   cache size, there is a significant propability that access to these arrays
 *   results in severe cross interference cache thrashing.
 *
 *   As a more or less ad-hoc solution to this problem, MSCA artificially increases
 *   memory size requests before allocation actually takes place. This simple
 *   measure manipulates the relative positions of arrays in memory and may cause
 *   significant performance improvements, in particular on machines with low
 *   associative cache memories.
 *
 *   Nevertheless, a more sophisticated solution to the problem of cross
 *   interference conflicts would be desirable. Such a solution would be
 *   Array Placements (APL). Unfortunately, APL for the time being is only
 *   rudimentarily implemented in the SAC compiler.
 *
 *****************************************************************************/

#ifndef _SAC_DISTMEM_HEAPMGR_H_
#define _SAC_DISTMEM_HEAPMGR_H_

#include "config.h"

#define SAC_DO_DISTMEM 1
#include "sac.h"
#undef SAC_DO_DISTMEM

/*
 * Initialization of some basic values/sizes.
 */

#define KB 1024
#define MB (KB * KB)

#define DIAG_FREEPATTERN -123456
#define DIAG_ALLOCPATTERN 123456

/*
 * Declarations of internal heap manager global variables and functions
 */

extern SAC_DISTMEM_HM_header_t *
SAC_DISTMEM_HM_AllocateNewBinInArenaOfArenas (SAC_DISTMEM_HM_size_unit_t units,
                                              SAC_DISTMEM_HM_arena_t *arena);

extern SAC_DISTMEM_HM_arena_t SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_NUM_ARENAS + 2];

#ifdef DIAG

extern void SAC_DISTMEM_HM_ClearDiagCounters (SAC_DISTMEM_HM_arena_t *arena);
extern void SAC_DISTMEM_HM_AddDiagCounters (SAC_DISTMEM_HM_arena_t *arena,
                                            SAC_DISTMEM_HM_arena_t *add_arena);

extern void SAC_DISTMEM_HM_CheckAllocDiagPattern (SAC_DISTMEM_HM_size_unit_t diag,
                                                  int arena_num);
extern void SAC_DISTMEM_HM_CheckFreeDiagPattern (SAC_DISTMEM_HM_size_unit_t diag,
                                                 int arena_num);
extern void SAC_DISTMEM_HM_CheckDiagPatternAnyChunk (SAC_DISTMEM_HM_header_t *addr);

static inline void
memset_words (void *s, unsigned c, size_t nw)
{
    unsigned *sw = (unsigned *)s;
    for (size_t i = 0; i < nw; ++i) {
        sw[i] = c;
    }
}

/*
 * Definition of macros for diagnostic heap management
 */

#define DIAG_INC(cnt) (cnt)++
#define DIAG_DEC(cnt) (cnt)--
#define DIAG_ADD(cnt, val) (cnt) += (val)
#define DIAG_SET(cnt, val) (cnt) = (val)

#define DIAG_SET_FREEPATTERN_SMALLCHUNK(freep)                                           \
    SAC_DISTMEM_HM_SMALLCHUNK_DIAG (freep) = DIAG_FREEPATTERN
#define DIAG_SET_ALLOCPATTERN_SMALLCHUNK(freep)                                          \
    SAC_DISTMEM_HM_SMALLCHUNK_DIAG (freep) = DIAG_ALLOCPATTERN

#define DIAG_SET_FREEPATTERN_LARGECHUNK(freep)                                           \
    SAC_DISTMEM_HM_LARGECHUNK_DIAG (freep) = DIAG_FREEPATTERN
#define DIAG_SET_ALLOCPATTERN_LARGECHUNK(freep)                                          \
    SAC_DISTMEM_HM_LARGECHUNK_DIAG (freep) = DIAG_ALLOCPATTERN

#define DIAG_CHECK_FREEPATTERN_SMALLCHUNK(freep, arena_num)                              \
    SAC_DISTMEM_HM_CheckFreeDiagPattern (SAC_DISTMEM_HM_SMALLCHUNK_DIAG (freep),         \
                                         arena_num)

#define DIAG_CHECK_ALLOCPATTERN_SMALLCHUNK(freep, arena_num)                             \
    SAC_DISTMEM_HM_CheckAllocDiagPattern (SAC_DISTMEM_HM_SMALLCHUNK_DIAG (freep),        \
                                          arena_num)

#define DIAG_CHECK_FREEPATTERN_LARGECHUNK(freep, arena_num)                              \
    SAC_DISTMEM_HM_CheckFreeDiagPattern (SAC_DISTMEM_HM_LARGECHUNK_DIAG (freep),         \
                                         arena_num)

#define DIAG_CHECK_ALLOCPATTERN_LARGECHUNK(freep, arena_num)                             \
    SAC_DISTMEM_HM_CheckAllocDiagPattern (SAC_DISTMEM_HM_LARGECHUNK_DIAG (freep),        \
                                          arena_num)

#define DIAG_CHECK_ALLOCPATTERN_ANYCHUNK(addr)                                           \
    SAC_DISTMEM_HM_CheckDiagPatternAnyChunk (addr)

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

#endif /* _SAC_DISTMEM_HEAPMGR_H_ */
