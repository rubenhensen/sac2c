/*
 *
 * $Id$
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
 *  Descriptor Allocation Optimization (DAO):
 *
 *   Arrays in SAC are represented by two data object: the data field itself,
 *   which contains the array elements in row-major order, and the descriptor,
 *   which - depending on the array type - contains structural information such
 *   as the reference counter, the size of the data field, and the shape.
 *
 *   By default, data field and descriptor are allocated and de-allocated
 *   completely independent of each other. However, in ALMOST all cases
 *   allocation and de-allocation of data field and associated descriptor
 *   happen to be at the same time. This raises the opportunity of an
 *   optimization, which we have called descriptor allocation optimization.
 *
 *   The basic idea is to allocate just one chunk of memory which then
 *   accomodates both the data field and the descriptor. The rationale behind
 *   this approach is that allocation time cost is mostly independent to small
 *   increases in memory size, and the descriptor is typically much smaller
 *   than the data field. Moreover, de-allocation of the descriptor is avoided
 *   completely.
 *
 *   The major obstacle for this approach is the fact that data field and
 *   descriptor are in most cases, but not always allocated and de-allocated
 *   together. This is due to SAC's foreign language interface, which allows
 *   arrays to be allocated and/or de-allocated outside the SAC world. In
 *   this case, descriptors need to be created and removed when an array
 *   enters or leaves the SAC world.
 *
 *   Technically, DAO is realized by allocating memory for the size of the data
 *   field plus the size of the descriptor plus the size of two basic administration
 *   blocks. One administration block is required for storing internal information,
 *   basically we must be able to distinguish fully-allocated descriptors from
 *   DAO-fake-allocated ones. The other administration block assures that we have
 *   sufficient memory to properly align the descriptor.
 *
 *   The offset from the beginning of the data field to the descriptor part
 *   is then computed by determining how many basic blocks are needed for the
 *   data field, then add another basic block for administration, and take
 *   the following one as starting point for storing the descriptor.
 *
 *   DAO-fake-allocated descriptors are uniquely marked by having their arena
 *   pointer being set to NULL. This way, we can easily distinguish between
 *   real and fake-allocations once it comes to de-allocation of descriptors.
 *   However, as we for performance reasons we don't want to check the arena
 *   on all and any de-allocation, a specific de-allocation ICM for descriptors
 *   is available and MUST be used for this purpose.
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
 *   significant performance improvements, in particular on machnies with low
 *   associative cache memories.
 *
 *   Nevertheless, a more sophisticated solution to the problem of cross
 *   interference conflicts would be desirable. Such a solution would be
 *   Array Placements (APL). Unfortunately, APL for the time being is only
 *   rudimentarily implemented in the SAC compiler.
 *
 *
 *
 *****************************************************************************/

#ifndef _SAC_HEAPMGR_H_
#define _SAC_HEAPMGR_H_

#include "config.h"

#ifdef MT

#define SAC_DO_MULTITHREAD 1
#define SAC_DO_THREADS_STATIC 1

#ifdef PHM_XT
/* MT and PHM_XT */
/* The free()'ing thread may not be the same as the malloc()'ing thread. */
#define SAC_DO_HM_XTHR_FREE 1
/* There may be other threads in the process than those created by SAC runtime.
 * Discover and assign IDs autonomously. */
#define SAC_DO_HM_DISCOVER_THREADS 1
/* Initialize the library on its own, no dependencies on the other SAC runtime. */
#define SAC_DO_HM_AUTONOMOUS 1

#else /* PHM_XT */
/* MT but not PHM_XT */
#define SAC_DO_HM_XTHR_FREE 0
#define SAC_DO_HM_DISCOVER_THREADS 0
#define SAC_DO_HM_AUTONOMOUS 0

#endif /* PHM_XT */

#else /* MT */

#define SAC_DO_MULTITHREAD 0
#define SAC_DO_HM_XTHR_FREE 0
#define SAC_DO_HM_DISCOVER_THREADS 0
#define SAC_DO_HM_AUTONOMOUS 0

#endif /*  MT  */

#if SAC_DO_HM_DISCOVER_THREADS || SAC_DO_HM_AUTONOMOUS
/* Assumed number of threads: this is only used in sac4c XT variant.
 * Statically compiled in to enable static memory alloc. */
#define SAC_HM_ASSUME_THREADS_MAX 512

#endif /* SAC_DO_HM_DISCOVER_THREADS || SAC_DO_HM_AUTONOMOUS */

#define SAC_DO_PHM 1
#define SAC_COMPILE_SACLIB
#define SAC_DO_COMPILE_MODULE 1

#include "sac.h"

#undef SAC_COMPILE_SACLIB
#undef SAC_DO_MULTITHREAD
#undef SAC_DO_THREADS_STATIC
#undef SAC_DO_COMPILE_MODULE

/*
 * Macro definition of sbrk() system call
 */

#define SBRK(size) sbrk ((SBRK_T) (size))

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

extern void SAC_HM_SetInitialized (void);
extern int SAC_HM_GetInitialized (void);
extern void SAC_HM_SetupMaster (void);

#if SAC_DO_HM_XTHR_FREE

/* Internal helper function */
/* Atomically pop all elements from the unused list of the arena.
 * Returns pointer to the head of the list which may be traversed subsequently
 * using local operations only. */
static inline SAC_HM_header_t *
grab_arena_unused_list (SAC_HM_arena_t *arena)
{
    SAC_HM_header_t *their_list;
    do {
        /* get the current head of the list */
        their_list = (SAC_HM_header_t *)arena->unused_list;
        /* swap the head with NULL, thus atomically disconnecting the list
         * from the unused list of the arena */
    } while (!__sync_bool_compare_and_swap (&arena->unused_list, their_list, NULL));
    return their_list;
}

#endif

#ifdef DIAG
extern unsigned long int SAC_HM_call_sbrk;
extern unsigned long int SAC_HM_call_malloc;
extern unsigned long int SAC_HM_call_realloc;
extern unsigned long int SAC_HM_call_calloc;
extern unsigned long int SAC_HM_call_valloc;
extern unsigned long int SAC_HM_call_memalign;
extern unsigned long int SAC_HM_call_posix_memalign;
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

#endif /* _SAC_HEAPMGR_H_ */
