/*****************************************************************************
 *
 * file:   sac_heapmgr.h
 *
 * prefix: SAC_HM
 *
 * description:
 *
 *
 *****************************************************************************/

#ifndef _SAC_HEAPMGR_H
#define _SAC_HEAPMGR_H

#include <assert.h>

#if SAC_BACKEND_CUDA
#include "override_cuda_runtime.h"
#endif

#include "runtime/essentials_h/rt_misc.h" // SAC_MAX
#include "runtime/mt_h/rt_mt.h" // SAC_MT_DECLARE_LOCK
#include "runtime/essentials_h/cuda_transfer_methods.h"

#if SAC_MUTC_MACROS
#define SAC_DO_PHM 0
#endif

/*
 * Basic type definitions.
 */

typedef unsigned long int SAC_HM_size_byte_t;
typedef long int SAC_HM_size_unit_t;

#ifndef SAC_COMPILE_SACLIB
#if !SAC_MUTC_MACROS

#define MUTC 1
#ifndef SAC_BACKEND_MUTC
#include <sys/types.h> /* typedef unsigned int size_t;  */
#else
#include "stddef.h"
#endif
#undef MUTC

#endif
#endif /* SAC_COMPILE_SACLIB */

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

/*
 * Declaration of conventional heap management functions
 * used from within compiled SAC programs.
 */
#if !SAC_MUTC_MACROS && !SAC_CUDA_MACROS
SAC_C_EXTERN void *malloc (size_t size);
SAC_C_EXTERN void free (void *addr);
#endif

/*
 * Macro redefinitions for compatibility reasons with standard library.
 */

#ifndef SAC_MALLOC
#define SAC_MALLOC(x) malloc (x)
#endif

#ifndef SAC_FREE
#define SAC_FREE(x) free (x)
#endif

/* calloc() = like malloc(), but initializes the memory to zero */
#ifndef SAC_CALLOC
#define SAC_CALLOC(nmemb, size) calloc (nmemb, size)
#endif

#if SAC_DO_PHM

/*
 * Basic settings.
 */

#define SAC_HM_NUM_ARENAS 9
#define SAC_HM_NUM_SMALLCHUNK_ARENAS 5
#define SAC_HM_ARENA_OF_ARENAS 0
#define SAC_HM_TOP_ARENA (SAC_HM_NUM_ARENAS - 1)

#define SAC_HM_ALIGNMENT sizeof (double)

/*
 * Type definitions of internal heap management data structures.
 */

typedef union header_t {
    struct header_data1_t {
        SAC_HM_size_unit_t size;
        struct arena_t *arena;
    } data1;
    struct header_data2_t {
        union header_t *prevfree;
        union header_t *nextfree;
    } data2;
    struct header_data3_t {
        SAC_HM_size_unit_t prevsize;
        SAC_HM_size_unit_t diag;
    } data3;
    char align[(((SAC_MAX (SAC_MAX (sizeof (struct header_data1_t),
                                    sizeof (struct header_data2_t)),
                           sizeof (struct header_data3_t))
                  - 1)
                 / SAC_HM_ALIGNMENT)
                + 1)
               * SAC_HM_ALIGNMENT];
} SAC_HM_header_t;

/*
 * Access macros for administration blocks
 */

#define SAC_HM_LARGECHUNK_SIZE(header) (((header) + 1)->data1.size)
#define SAC_HM_LARGECHUNK_PREVSIZE(header) (((header) + 0)->data3.prevsize)
#define SAC_HM_LARGECHUNK_ARENA(header) (((header) + 1)->data1.arena)
#define SAC_HM_LARGECHUNK_NEXTFREE(header) (((header) + 2)->data2.nextfree)

/* smallchunk_size is valid only for the special wilderness chunk! */
#define SAC_HM_SMALLCHUNK_SIZE(header) (((header) + 0)->data1.size)
#define SAC_HM_SMALLCHUNK_ARENA(header) (((header) + 0)->data1.arena)
#define SAC_HM_SMALLCHUNK_NEXTFREE(header) (((header) + 1)->data2.nextfree)

#define SAC_HM_LARGECHUNK_DIAG(header) (((header) + 0)->data3.diag)
#define SAC_HM_SMALLCHUNK_DIAG(header) (((header) + 0)->data1.size)

/*
 * Memory is always administrated in chunks of UNIT_SIZE bytes.
 */

#define SAC_HM_UNIT_SIZE (sizeof (SAC_HM_header_t))

#define SAC_HM_BYTES_2_UNITS(size) ((((size)-1) / SAC_HM_UNIT_SIZE) + 1)

/*
 * Type definition for arena data structure
 */

typedef struct arena_t {
    int num;
    SAC_HM_header_t freelist[3];
    SAC_HM_header_t *wilderness;
    SAC_HM_size_unit_t binsize;        /* in units */
    SAC_HM_size_unit_t min_chunk_size; /* in units */

    /* The list of chunks that are no longer used and ready to be freed.
     * All updates to the pointer must be atomic; it is accessed asynchronously
     * in different threads!!
     * This is only used when SAC_DO_HM_XTHR_FREE is enabled, but the field
     * is always in the structure to preserve binary compatibility
     * of the API between different PHM variants (MT, XT). */
    volatile SAC_HM_header_t *unused_list;

    /* diagnostic info */
    unsigned long int size; /* in bytes */
    unsigned long int cnt_bins;
    unsigned long int cnt_alloc;
    unsigned long int cnt_alloc_var_size;
    unsigned long int cnt_after_freelist;
    unsigned long int cnt_after_splitting;
    unsigned long int cnt_after_wilderness;
    unsigned long int cnt_after_coalascing;
    unsigned long int cnt_after_coalascing_wilderness;
    unsigned long int cnt_after_extension;
    unsigned long int cnt_free;
    unsigned long int cnt_free_var_size;
    unsigned long int cnt_coalascing;
    unsigned long int cnt_coalascing_wilderness;
} SAC_HM_arena_t;

/*
 * Static initialization of master thread arenas
 */

#define SAC_HM_SMALL_ARENA_FREELIST_BASE(n)                                              \
    {                                                                                    \
        {{0, &(SAC_HM_arenas[0][n])}}, {{0, NULL}},                                      \
        {                                                                                \
            {                                                                            \
                0, NULL                                                                  \
            }                                                                            \
        }                                                                                \
    }

#define SAC_HM_LARGE_ARENA_FREELIST_BASE(n)                                              \
    {                                                                                    \
        {{-1, NULL}}, {{0, &(SAC_HM_arenas[0][n])}},                                     \
        {                                                                                \
            {                                                                            \
                0, NULL                                                                  \
            }                                                                            \
        }                                                                                \
    }

#define SAC_HM_ARENA_OF_SMALL_CHUNKS(n, binsize, mincs)                                  \
    {                                                                                    \
        n, SAC_HM_SMALL_ARENA_FREELIST_BASE (n), SAC_HM_arenas[0][n].freelist, binsize,  \
          mincs, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                          \
    }

#define SAC_HM_ARENA_OF_LARGE_CHUNKS(n, binsize, mincs)                                  \
    {                                                                                    \
        n, SAC_HM_LARGE_ARENA_FREELIST_BASE (n), SAC_HM_arenas[0][n].freelist, binsize,  \
          mincs, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                          \
    }

#define SAC_HM_SETUP_ARENAS()                                                            \
    {                                                                                    \
        {                                                                                \
            SAC_HM_ARENA_OF_SMALL_CHUNKS (0, SAC_HM_ARENA_0_BINSIZE,                     \
                                          SAC_HM_ARENA_0_MINCS),                         \
              SAC_HM_ARENA_OF_SMALL_CHUNKS (1, SAC_HM_ARENA_1_BINSIZE,                   \
                                            SAC_HM_ARENA_2_MINCS - 1),                   \
              SAC_HM_ARENA_OF_SMALL_CHUNKS (2, SAC_HM_ARENA_2_BINSIZE,                   \
                                            SAC_HM_ARENA_3_MINCS - 1),                   \
              SAC_HM_ARENA_OF_SMALL_CHUNKS (3, SAC_HM_ARENA_3_BINSIZE,                   \
                                            SAC_HM_ARENA_4_MINCS - 1),                   \
              SAC_HM_ARENA_OF_SMALL_CHUNKS (4, SAC_HM_ARENA_4_BINSIZE,                   \
                                            SAC_HM_ARENA_5_MINCS - 1),                   \
              SAC_HM_ARENA_OF_LARGE_CHUNKS (5, SAC_HM_ARENA_5_BINSIZE,                   \
                                            SAC_HM_ARENA_5_MINCS),                       \
              SAC_HM_ARENA_OF_LARGE_CHUNKS (6, SAC_HM_ARENA_6_BINSIZE,                   \
                                            SAC_HM_ARENA_6_MINCS),                       \
              SAC_HM_ARENA_OF_LARGE_CHUNKS (7, SAC_HM_ARENA_7_BINSIZE,                   \
                                            SAC_HM_ARENA_7_MINCS),                       \
              SAC_HM_ARENA_OF_LARGE_CHUNKS (8, SAC_HM_ARENA_8_BINSIZE,                   \
                                            SAC_HM_ARENA_8_MINCS)                        \
        }                                                                                \
    }

/*
 * Access macros for chunk administration blocks
 * relative to memory location returned by malloc(), etc.
 */

#define SAC_HM_ADDR_ARENA(addr) ((((SAC_HM_header_t *)addr) - 1)->data1.arena)
#define SAC_HM_ADDR_SIZE(addr)                                                           \
    ((((SAC_HM_header_t *)addr) - 1)->data1.size) /* it's a lie! */
#define SAC_HM_ADDR_NEXTFREE(addr) (((SAC_HM_header_t *)addr)->data2.nextfree)

/*
 * Minimum chunk sizes (MINCS) for arenas
 */

#define SAC_HM_ARENA_0_MINCS 1
#define SAC_HM_ARENA_1_MINCS 2
#define SAC_HM_ARENA_2_MINCS 3
#define SAC_HM_ARENA_3_MINCS 5
#define SAC_HM_ARENA_4_MINCS 9
#define SAC_HM_ARENA_5_MINCS 17
#define SAC_HM_ARENA_6_MINCS 129
#define SAC_HM_ARENA_7_MINCS 1025
#define SAC_HM_ARENA_8_MINCS 8193

/*
 * Maximum chunk sizes (MAXCS) for arenas
 */

#define SAC_HM_ARENA_1_MAXCS_BYTES ((SAC_HM_ARENA_2_MINCS - 2) * SAC_HM_UNIT_SIZE)
#define SAC_HM_ARENA_2_MAXCS_BYTES ((SAC_HM_ARENA_3_MINCS - 2) * SAC_HM_UNIT_SIZE)
#define SAC_HM_ARENA_3_MAXCS_BYTES ((SAC_HM_ARENA_4_MINCS - 2) * SAC_HM_UNIT_SIZE)
#define SAC_HM_ARENA_4_MAXCS_BYTES ((SAC_HM_ARENA_5_MINCS - 2) * SAC_HM_UNIT_SIZE)

#define SAC_HM_ARENA_5_MAXCS_BYTES ((SAC_HM_ARENA_6_MINCS - 3) * SAC_HM_UNIT_SIZE)
#define SAC_HM_ARENA_6_MAXCS_BYTES ((SAC_HM_ARENA_7_MINCS - 3) * SAC_HM_UNIT_SIZE)
#define SAC_HM_ARENA_7_MAXCS_BYTES ((SAC_HM_ARENA_8_MINCS - 3) * SAC_HM_UNIT_SIZE)

/*
 * Bin sizes for arenas
 */

#define SAC_HM_ARENA_0_BINSIZE 131072
#define SAC_HM_ARENA_1_BINSIZE 512
#define SAC_HM_ARENA_2_BINSIZE 512
#define SAC_HM_ARENA_3_BINSIZE 256
#define SAC_HM_ARENA_4_BINSIZE 512
#define SAC_HM_ARENA_5_BINSIZE 2048
#define SAC_HM_ARENA_6_BINSIZE 8196
#define SAC_HM_ARENA_7_BINSIZE 32768
#define SAC_HM_ARENA_8_BINSIZE 0

/*
 * Thread status
 *
 *   The purpose of the thread status is to keep track of the current execution
 *   state for allocation and de-allocation purposes. This trick allows to leave
 *   the code generator almost untouched when integrating multi-threaded heap
 *   management into sac2c.
 */

typedef enum {
    SAC_HM_single_threaded, /* ST functions */
    SAC_HM_multi_threaded,  /* MT, XT, SPMD functions */
    SAC_HM_any_threaded
} SAC_HM_thread_status_t;

#define SAC_HM_DEFINE_THREAD_STATUS(status)                                              \
    static const SAC_HM_thread_status_t SAC_HM_thread_status = status;

/*
 * Mutex locks for multi-threaded execution.
 */

SAC_MT_DECLARE_LOCK (SAC_HM_top_arena_lock)
SAC_MT_DECLARE_LOCK (SAC_HM_diag_counter_lock)

/*
 * Declaration of SAC heap management global variables.
 */

SAC_C_EXTERN int SAC_HM_not_yet_initialized;
SAC_C_EXTERN unsigned long int SAC_HM_acquire_top_arena_lock;

/*
 * Declaration of SAC heap management functions.
 */

SAC_C_EXTERN void SAC_HM_Setup (unsigned int threads);

SAC_C_EXTERN void *SAC_HM_MallocSmallChunk (SAC_HM_size_unit_t units,
                                            SAC_HM_arena_t *arena);
SAC_C_EXTERN void *SAC_HM_MallocLargeChunk (SAC_HM_size_unit_t units,
                                            SAC_HM_arena_t *arena);

SAC_C_EXTERN void SAC_HM_FreeSmallChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena);
SAC_C_EXTERN void SAC_HM_FreeLargeChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena);

SAC_C_EXTERN void *SAC_HM_MallocAnyChunk_at (SAC_HM_size_byte_t size,
                                             unsigned int thread_id);
SAC_C_EXTERN void *SAC_HM_MallocAnyChunk_st (SAC_HM_size_byte_t size);
SAC_C_EXTERN void *SAC_HM_MallocAnyChunk_mt (SAC_HM_size_byte_t size,
                                             unsigned int thread_id);

SAC_C_EXTERN void *SAC_HM_MallocSmallChunk_at (SAC_HM_size_unit_t units, int arena_num);
SAC_C_EXTERN void *SAC_HM_MallocLargeChunk_at (SAC_HM_size_unit_t units, int arena_num);
SAC_C_EXTERN void *SAC_HM_MallocTopArena_at (SAC_HM_size_unit_t units);
SAC_C_EXTERN void *SAC_HM_MallocTopArena_mt (SAC_HM_size_unit_t units);

SAC_C_EXTERN void *SAC_HM_MallocDesc (SAC_HM_header_t *addr, SAC_HM_size_byte_t size,
                                      SAC_HM_size_byte_t desc_size);

SAC_C_EXTERN void SAC_HM_FreeTopArena_mt (SAC_HM_header_t *addr);
SAC_C_EXTERN void SAC_HM_FreeTopArena_at (SAC_HM_header_t *addr);

SAC_C_EXTERN void SAC_HM_FreeDesc (SAC_HM_header_t *addr);

SAC_C_EXTERN void SAC_HM_ShowDiagnostics (void);
SAC_C_EXTERN void SAC_HM_CheckAllocPatternAnyChunk (SAC_HM_header_t *addr);

SAC_C_EXTERN void *SAC_HM_PlaceArray (void *alloc, void *base, long int offset,
                                      long int cache_size);

/* Discovering threads */
/* Invalid Thread ID */
#define SAC_HM_THREADID_INVALID (0xDeadBeef)

SAC_C_EXTERN unsigned int SAC_HM_CurrentThreadId (void);
SAC_C_EXTERN int SAC_HM_DiscoversThreads (void);

/*
 * Definition of general macros.
 */

#define SAC_HM_SETUP()                                                                   \
    {                                                                                    \
        SAC_HM_Setup (SAC_MT_GLOBAL_THREADS () + SAC_HM_RTSPEC_THREADS ());              \
    }

/* Pleasing both the C99 and C++ compiler is tiresome. */
#ifdef __cplusplus
#define SAC_EXTERN_IF_CPP extern
#else
#define SAC_EXTERN_IF_CPP
#endif

#define SAC_HM_DEFINE__IMPL()                                                            \
    SAC_HM_arena_t SAC_HM_arenas[SAC_SET_THREADS_MAX][SAC_HM_NUM_ARENAS + 2]             \
      = SAC_HM_SETUP_ARENAS ();                                                          \
    SAC_EXTERN_IF_CPP const SAC_HM_size_byte_t                                           \
      SAC_HM_initial_master_arena_of_arenas_size                                         \
      = SAC_SET_INITIAL_MASTER_HEAPSIZE;                                                 \
    SAC_EXTERN_IF_CPP const SAC_HM_size_byte_t                                           \
      SAC_HM_initial_worker_arena_of_arenas_size                                         \
      = SAC_SET_INITIAL_WORKER_HEAPSIZE;                                                 \
    SAC_EXTERN_IF_CPP const SAC_HM_size_byte_t SAC_HM_initial_top_arena_size             \
      = SAC_SET_INITIAL_UNIFIED_HEAPSIZE;                                                \
    SAC_EXTERN_IF_CPP const unsigned int SAC_HM_max_worker_threads                       \
      = SAC_SET_THREADS_MAX - 1;

#if SAC_DO_COMPILE_MODULE
#define SAC_HM_DEFINE()                                                                  \
    SAC_C_EXTERN SAC_HM_arena_t SAC_HM_arenas[][SAC_HM_NUM_ARENAS + 2];

#else /* SAC_DO_COMPILE_MODULE */

#define SAC_HM_DEFINE() SAC_HM_DEFINE__IMPL ()

#endif /* SAC_DO_COMPILE_MODULE */

#if SAC_DO_CHECK_HEAP
#define SAC_HM_PRINT() SAC_HM_ShowDiagnostics ();
#define SAC_HM_INC_DIAG_COUNTER(counter) (counter)++;
#else
#define SAC_HM_PRINT()
#define SAC_HM_INC_DIAG_COUNTER(counter)
#endif

/* FIXME: move to the SAC_DO_CHECK_HEAP #if-block above once the worst bugs get
 * hunted down. */
#define SAC_HM_ASSERT(x) assert (x)

/*
 * Definition of memory allocation macros.
 */

#if !defined(SAC_DO_CUDA_ALLOC) || SAC_DO_CUDA_ALLOC == SAC_CA_system                    \
  || SAC_DO_CUDA_ALLOC == SAC_CA_cureg
#define SAC_HM_MALLOC(var, size, basetype)                                               \
    {                                                                                    \
        switch (SAC_HM_thread_status) {                                                  \
        case SAC_HM_single_threaded:                                                     \
            SAC_HM_ASSERT (SAC_MT_globally_single                                        \
                           && "An ST/SEQ call in the MT/XT context!! (1)");              \
            var = (basetype *)SAC_HM_MallocAnyChunk_st (size);                           \
            break;                                                                       \
        case SAC_HM_multi_threaded:                                                      \
            var = (basetype *)SAC_HM_MallocAnyChunk_mt (size, SAC_MT_SELF_THREAD_ID ()); \
            break;                                                                       \
        case SAC_HM_any_threaded:                                                        \
            var = (basetype *)SAC_HM_MallocAnyChunk_at (size, SAC_MT_SELF_THREAD_ID ()); \
            break;                                                                       \
        }                                                                                \
    }
#elif SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define SAC_HM_MALLOC(var, size, basetype)                                               \
    {                                                                                    \
        SAC_TR_GPU_PRINT ("Allocating CUDA host allocated area.");                       \
        cudaHostAlloc ((void **)&var, sizeof (basetype) * size, cudaHostAllocPortable);  \
        SAC_GET_CUDA_MALLOC_ERROR ();                                                    \
    }
#else /* managed */
#define SAC_HM_MALLOC(var, size, basetype)                                               \
    {                                                                                    \
        SAC_TR_GPU_PRINT ("Allocating CUDA managed memory area.");                       \
        cudaMallocManaged ((void **)&var, sizeof (basetype) * size,                      \
                           cudaMemAttachGlobal);                                         \
        SAC_GET_CUDA_MALLOC_ERROR ();                                                    \
    }
#endif

#if !defined(SAC_DO_CUDA_ALLOC) || SAC_DO_CUDA_ALLOC == SAC_CA_system                    \
  || SAC_DO_CUDA_ALLOC == SAC_CA_cureg
#define SAC_HM_MALLOC_AS_SCALAR(var, size, basetype)                                     \
    {                                                                                    \
        switch (SAC_HM_thread_status) {                                                  \
        case SAC_HM_single_threaded:                                                     \
            SAC_HM_ASSERT (SAC_MT_globally_single                                        \
                           && "An ST/SEQ call in the MT/XT context!! (2)");              \
            var = (basetype)SAC_HM_MallocAnyChunk_st (size);                             \
            break;                                                                       \
        case SAC_HM_multi_threaded:                                                      \
            var = (basetype)SAC_HM_MallocAnyChunk_mt (size, SAC_MT_SELF_THREAD_ID ());   \
            break;                                                                       \
        case SAC_HM_any_threaded:                                                        \
            var = (basetype)SAC_HM_MallocAnyChunk_at (size, SAC_MT_SELF_THREAD_ID ());   \
            break;                                                                       \
        }                                                                                \
    }
#elif SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define SAC_HM_MALLOC_AS_SCALAR(var, size, basetype)                                     \
    {                                                                                    \
        SAC_TR_GPU_PRINT ("Allocating CUDA host allocated area.");                       \
        cudaHostAlloc ((void **)&var, size, cudaHostAllocPortable);                      \
        SAC_GET_CUDA_MALLOC_ERROR ();                                                    \
    }
#else /* managed */
#define SAC_HM_MALLOC_AS_SCALAR(var, size, basetype)                                     \
    {                                                                                    \
        SAC_TR_GPU_PRINT ("Allocating CUDA managed memory area.");                       \
        cudaMallocManaged ((void **)&var, size, cudaMemAttachGlobal);                    \
        SAC_GET_CUDA_MALLOC_ERROR ();                                                    \
    }
#endif

#if SAC_DO_MULTITHREAD

#define SAC_HM_MALLOC_SMALL_CHUNK(var, units, arena_num, basetype)                       \
    {                                                                                    \
        SAC_HM_ASSERT ((arena_num >= 0 && arena_num < 5)                                 \
                       && "That is not a small arena!");                                 \
        switch (SAC_HM_thread_status) {                                                  \
        case SAC_HM_single_threaded:                                                     \
            SAC_HM_ASSERT (SAC_MT_globally_single                                        \
                           && "An ST/SEQ small-arena call in the MT/XT context!!");      \
            var = (basetype *)SAC_HM_MallocSmallChunk (units,                            \
                                                       &(SAC_HM_arenas[0][arena_num]));  \
            break;                                                                       \
        case SAC_HM_multi_threaded:                                                      \
            var = (basetype *)                                                           \
              SAC_HM_MallocSmallChunk (units, &(SAC_HM_arenas[SAC_MT_SELF_THREAD_ID ()]  \
                                                             [arena_num]));              \
            break;                                                                       \
        case SAC_HM_any_threaded:                                                        \
            /* var = SAC_HM_MallocSmallChunk_at(units, arena_num); */                    \
            var = (basetype *)                                                           \
              SAC_HM_MallocSmallChunk (units, &(SAC_HM_arenas[SAC_MT_SELF_THREAD_ID ()]  \
                                                             [arena_num]));              \
            break;                                                                       \
        }                                                                                \
    }

#define SAC_HM_MALLOC_LARGE_CHUNK(var, units, arena_num, basetype)                       \
    {                                                                                    \
        SAC_HM_ASSERT ((arena_num >= 5 && arena_num < SAC_HM_TOP_ARENA)                  \
                       && "That is not a large arena");                                  \
        switch (SAC_HM_thread_status) {                                                  \
        case SAC_HM_single_threaded:                                                     \
            SAC_HM_ASSERT (SAC_MT_globally_single                                        \
                           && "An ST/SEQ large-arena call in the MT/XT context!!");      \
            var = (basetype *)SAC_HM_MallocLargeChunk (units,                            \
                                                       &(SAC_HM_arenas[0][arena_num]));  \
            break;                                                                       \
        case SAC_HM_multi_threaded:                                                      \
            var = (basetype *)                                                           \
              SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[SAC_MT_SELF_THREAD_ID ()]  \
                                                             [arena_num]));              \
            break;                                                                       \
        case SAC_HM_any_threaded:                                                        \
            /* var = SAC_HM_MallocLargeChunk_at(units, arena_num);  */                   \
            var = (basetype *)                                                           \
              SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[SAC_MT_SELF_THREAD_ID ()]  \
                                                             [arena_num]));              \
            break;                                                                       \
        }                                                                                \
    }

#define SAC_HM_MALLOC_TOP_ARENA(var, units, basetype)                                    \
    {                                                                                    \
        switch (SAC_HM_thread_status) {                                                  \
        case SAC_HM_single_threaded:                                                     \
            SAC_HM_ASSERT (SAC_MT_globally_single                                        \
                           && "An ST/SEQ top-arena call in the MT/XT context!!");        \
            var = (basetype *)                                                           \
              SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]));    \
            break;                                                                       \
        case SAC_HM_multi_threaded:                                                      \
            SAC_MT_ACQUIRE_LOCK (SAC_HM_top_arena_lock);                                 \
            SAC_HM_INC_DIAG_COUNTER (SAC_HM_acquire_top_arena_lock);                     \
            var = (basetype *)                                                           \
              SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]));    \
            SAC_MT_RELEASE_LOCK (SAC_HM_top_arena_lock);                                 \
            break;                                                                       \
        case SAC_HM_any_threaded:                                                        \
            var = (basetype *)SAC_HM_MallocTopArena_at (units);                          \
            break;                                                                       \
        }                                                                                \
    }

#else /* SAC_DO_MULTITHREAD */

#define SAC_HM_MALLOC_SMALL_CHUNK(var, units, arena_num, basetype)                       \
    var = (basetype *)SAC_HM_MallocSmallChunk (units, &(SAC_HM_arenas[0][arena_num]));

#define SAC_HM_MALLOC_LARGE_CHUNK(var, units, arena_num, basetype)                       \
    var = (basetype *)SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][arena_num]));

#define SAC_HM_MALLOC_TOP_ARENA(var, units, basetype)                                    \
    var = (basetype *)SAC_HM_MallocLargeChunk (units,                                    \
                                               &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]));

#endif /* SAC_DO_MULTITHREAD */

#if 0
/*
 * Although for small examples the following allocator code in expression
 * position yields the same assembler code results as the current version
 * in command position, the new form has proved to be more efficient in
 * large applications for a still unknown reason.
 *
 * The new form, however, is also more handy for introducing combined
 * allocation facilities for data structures and their associated descriptors.
 */

#define SAC_HM_MALLOC_FIXED_SIZE(size)                                                   \
    (((size) <= ARENA_4_MAXCS_BYTES)                                                     \
       ? (((size) <= ARENA_2_MAXCS_BYTES)                                                \
            ? (((size) <= ARENA_1_MAXCS_BYTES)                                           \
                 ? (SAC_HM_MallocSmallChunkPresplit (2, &(SAC_HM_arenas[1]), 16))        \
                 : (SAC_HM_MallocSmallChunkPresplit (4, &(SAC_HM_arenas[2]), 16)))       \
            : (((size) <= ARENA_3_MAXCS_BYTES)                                           \
                 ? (SAC_HM_MallocSmallChunk (8, &(SAC_HM_arenas[3])))                    \
                 : (SAC_HM_MallocSmallChunk (16, &(SAC_HM_arenas[4])))))                 \
       : ((SAC_HM_UNITS (size) < ARENA_7_MINCS)                                          \
            ? ((SAC_HM_UNITS (size) < ARENA_6_MINCS)                                     \
                 ? (SAC_HM_MallocLargeChunk (SAC_HM_UNITS (size), &(SAC_HM_arenas[5])))  \
                 : (SAC_HM_MallocLargeChunk (SAC_HM_UNITS (size), &(SAC_HM_arenas[6])))) \
            : ((SAC_HM_UNITS (size) < ARENA_8_MINCS)                                     \
                 ? (SAC_HM_MallocLargeChunk (SAC_HM_UNITS (size), &(SAC_HM_arenas[7])))  \
                 : (SAC_HM_MallocLargeChunk (SAC_HM_UNITS (size),                        \
                                             &(SAC_HM_arenas[8]))))))

#endif /*  0  */

/* APS: arena preselection, enabled by default */
#if SAC_DO_APS

#if SAC_DO_MSCA

#define SAC_HM_MALLOC_FIXED_SIZE(var, size, basetype)                                    \
    {                                                                                    \
        if ((size) <= SAC_HM_ARENA_4_MAXCS_BYTES) {                                      \
            if ((size) <= SAC_HM_ARENA_2_MAXCS_BYTES) {                                  \
                if ((size) <= SAC_HM_ARENA_1_MAXCS_BYTES) {                              \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 2, 1, basetype)                      \
                } else {                                                                 \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 4, 2, basetype)                      \
                }                                                                        \
            } else {                                                                     \
                if ((size) <= SAC_HM_ARENA_3_MAXCS_BYTES) {                              \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 8, 3, basetype)                      \
                } else {                                                                 \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 16, 4, basetype)                     \
                }                                                                        \
            }                                                                            \
        } else {                                                                         \
            const SAC_HM_size_byte_t _real_size_1                                        \
              = size                                                                     \
                + SAC_HM_CACHE_ADJUST (size, SAC_SET_CACHE_3_SIZE,                       \
                                       SAC_SET_CACHE_3_MSCA_FACTOR);                     \
            const SAC_HM_size_byte_t _real_size_2                                        \
              = _real_size_1                                                             \
                + SAC_HM_CACHE_ADJUST (_real_size_1, SAC_SET_CACHE_2_SIZE,               \
                                       SAC_SET_CACHE_2_MSCA_FACTOR);                     \
            const SAC_HM_size_byte_t _real_size_3                                        \
              = _real_size_2                                                             \
                + SAC_HM_CACHE_ADJUST (_real_size_2, SAC_SET_CACHE_1_SIZE,               \
                                       SAC_SET_CACHE_1_MSCA_FACTOR);                     \
            const SAC_HM_size_unit_t _units = SAC_HM_BYTES_2_UNITS (_real_size_3) + 2;   \
                                                                                         \
            if (_units < SAC_HM_ARENA_7_MINCS) {                                         \
                if (_units < SAC_HM_ARENA_6_MINCS) {                                     \
                    SAC_HM_MALLOC_LARGE_CHUNK (var, _units, 5, basetype)                 \
                } else {                                                                 \
                    SAC_HM_MALLOC_LARGE_CHUNK (var, _units, 6, basetype)                 \
                }                                                                        \
            } else {                                                                     \
                if (_units < SAC_HM_ARENA_8_MINCS) {                                     \
                    SAC_HM_MALLOC_LARGE_CHUNK (var, _units, 7, basetype)                 \
                } else {                                                                 \
                    SAC_HM_MALLOC_TOP_ARENA (var, _units, basetype)                      \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#else /* SAC_DO_MSCA */

#define SAC_HM_MALLOC_FIXED_SIZE(var, size, basetype)                                    \
    {                                                                                    \
        if ((size) <= SAC_HM_ARENA_4_MAXCS_BYTES) {                                      \
            if ((size) <= SAC_HM_ARENA_2_MAXCS_BYTES) {                                  \
                if ((size) <= SAC_HM_ARENA_1_MAXCS_BYTES) {                              \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 2, 1, basetype)                      \
                } else {                                                                 \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 4, 2, basetype)                      \
                }                                                                        \
            } else {                                                                     \
                if ((size) <= SAC_HM_ARENA_3_MAXCS_BYTES) {                              \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 8, 3, basetype)                      \
                } else {                                                                 \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 16, 4, basetype)                     \
                }                                                                        \
            }                                                                            \
        } else {                                                                         \
            const SAC_HM_size_unit_t units = SAC_HM_BYTES_2_UNITS (size) + 2;            \
            if (units < SAC_HM_ARENA_7_MINCS) {                                          \
                if (units < SAC_HM_ARENA_6_MINCS) {                                      \
                    SAC_HM_MALLOC_LARGE_CHUNK (var, units, 5, basetype)                  \
                } else {                                                                 \
                    SAC_HM_MALLOC_LARGE_CHUNK (var, units, 6, basetype)                  \
                }                                                                        \
            } else {                                                                     \
                if (units < SAC_HM_ARENA_8_MINCS) {                                      \
                    SAC_HM_MALLOC_LARGE_CHUNK (var, units, 7, basetype)                  \
                } else {                                                                 \
                    SAC_HM_MALLOC_TOP_ARENA (var, units, basetype)                       \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#endif /* SAC_DO_MSCA */

#else /* SAC_DO_APS */

#define SAC_HM_MALLOC_FIXED_SIZE(var, size, basetype) SAC_HM_MALLOC (var, size, basetype)

#endif /* SAC_DO_APS */

#if SAC_DO_DAO

#if 0
/*
 * The below macro should realy use macros to find out the type of the
 * descriptor rather than assume that it is SAC_array_descriptor_t if
 * it is ever revived in the future
 *
 * CAJ 110318
 */
#define SAC_HM_MALLOC_FIXED_SIZE_WITH_DESC(var, var_desc, size, dim)                     \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (var, ((size) + BYTE_SIZE_OF_DESC (dim)                 \
                                        + 2 * SAC_HM_UNIT_SIZE))                         \
                                                                                         \
        var_desc = (SAC_array_descriptor_t) (((SAC_HM_header_t *)var)                    \
                                             + (SAC_HM_BYTES_2_UNITS (size) + 1));       \
        SAC_HM_ADDR_ARENA (var_desc) = NULL;                                             \
    }
#else
#define SAC_HM_MALLOC_FIXED_SIZE_WITH_DESC(var, var_desc, size, dim, basetype,           \
                                           descbasetype)                                 \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (var,                                                   \
                                  ((size) + BYTE_SIZE_OF_DESC (dim)                      \
                                   + 2 * SAC_HM_UNIT_SIZE),                              \
                                  basetype)                                              \
                                                                                         \
        var_desc = (descbasetype *)SAC_HM_MallocDesc ((SAC_HM_header_t *)var, size,      \
                                                      BYTE_SIZE_OF_DESC (dim));          \
    }
#endif

#else /* SAC_DO_DAO */

#define SAC_HM_MALLOC_FIXED_SIZE_WITH_DESC(var, var_desc, size, dim, basetype,           \
                                           descbasetype)                                 \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (var, (size), basetype)                                 \
        SAC_HM_MALLOC_FIXED_SIZE (var_desc, BYTE_SIZE_OF_DESC (dim), descbasetype)       \
    }

#endif /* SAC_DO_DAO */

/*
 * Definition of memory de-allocation macros.
 */

#if !defined(SAC_DO_CUDA_ALLOC) || SAC_DO_CUDA_ALLOC == SAC_CA_system                    \
  || SAC_DO_CUDA_ALLOC == SAC_CA_cureg
#define SAC_HM_FREE(addr) free (addr);
#elif SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define SAC_HM_FREE(addr)                                                                \
    do {                                                                                 \
        SAC_TR_GPU_PRINT ("Freeing CUDA host allocated area.");                          \
        cudaFreeHost (addr);                                                             \
        SAC_GET_CUDA_FREE_ERROR ();                                                      \
    } while (0);
#else /* managed */
#define SAC_HM_FREE(addr)                                                                \
    do {                                                                                 \
        SAC_TR_GPU_PRINT ("Freeing CUDA managed memory area.");                          \
        cudaFree (addr);                                                                 \
        SAC_GET_CUDA_FREE_ERROR ();                                                      \
    } while (0);
#endif

#if SAC_DO_APS

/*
 * Note, that due to DAO the size of the allocated chunk might be larger than
 * actually required by the size of the data object to be stored.
 */

#define SAC_HM_FREE_DESC(addr)                                                           \
    {                                                                                    \
        SAC_HM_FreeDesc ((SAC_HM_header_t *)addr);                                       \
    }

#define SAC_HM_FREE_FIXED_SIZE(addr, size)                                               \
    {                                                                                    \
        if (((size) + 2 * SAC_HM_UNIT_SIZE) <= SAC_HM_ARENA_4_MAXCS_BYTES) {             \
            SAC_HM_FreeSmallChunk ((SAC_HM_header_t *)addr, SAC_HM_ADDR_ARENA (addr));   \
        } else {                                                                         \
            if (size <= SAC_HM_ARENA_4_MAXCS_BYTES) {                                    \
                if (SAC_HM_ADDR_ARENA (addr)->num == 4) {                                \
                    SAC_HM_FreeSmallChunk ((SAC_HM_header_t *)addr,                      \
                                           SAC_HM_ADDR_ARENA (addr));                    \
                } else {                                                                 \
                    SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr,                      \
                                           SAC_HM_ADDR_ARENA (addr));                    \
                }                                                                        \
            } else {                                                                     \
                if (SAC_HM_BYTES_2_UNITS (size) + (2 + 2) < SAC_HM_ARENA_8_MINCS) {      \
                    SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr,                      \
                                           SAC_HM_ADDR_ARENA (addr));                    \
                } else {                                                                 \
                    if (SAC_HM_BYTES_2_UNITS (size) + 2 < SAC_HM_ARENA_8_MINCS) {        \
                        if (SAC_HM_ADDR_ARENA (addr)->num == 7) {                        \
                            SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr,              \
                                                   SAC_HM_ADDR_ARENA (addr));            \
                        } else {                                                         \
                            switch (SAC_HM_thread_status) {                              \
                            case SAC_HM_single_threaded:                                 \
                                SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr,          \
                                                       &(SAC_HM_arenas                   \
                                                           [0][SAC_HM_TOP_ARENA]));      \
                                break;                                                   \
                            case SAC_HM_multi_threaded:                                  \
                                SAC_HM_FreeTopArena_mt ((SAC_HM_header_t *)addr);        \
                                break;                                                   \
                            case SAC_HM_any_threaded:                                    \
                                SAC_HM_FreeTopArena_at ((SAC_HM_header_t *)addr);        \
                                break;                                                   \
                            }                                                            \
                        }                                                                \
                    } else {                                                             \
                        switch (SAC_HM_thread_status) {                                  \
                        case SAC_HM_single_threaded:                                     \
                            SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr,              \
                                                   &(SAC_HM_arenas[0]                    \
                                                                  [SAC_HM_TOP_ARENA]));  \
                            break;                                                       \
                        case SAC_HM_multi_threaded:                                      \
                            SAC_HM_FreeTopArena_mt ((SAC_HM_header_t *)addr);            \
                            break;                                                       \
                        case SAC_HM_any_threaded:                                        \
                            SAC_HM_FreeTopArena_at ((SAC_HM_header_t *)addr);            \
                            break;                                                       \
                        }                                                                \
                    }                                                                    \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#else /* SAC_DO_APS */

#define SAC_HM_FREE_DESC(addr)                                                           \
    {                                                                                    \
        if (SAC_HM_ADDR_ARENA (addr) != NULL) {                                          \
            SAC_HM_FREE (addr)                                                           \
        }                                                                                \
    }

#define SAC_HM_FREE_FIXED_SIZE(addr, size) SAC_HM_FREE (addr)

#endif /* SAC_DO_APS */

#define SAC_DO_INLINE_FREE 0

#if SAC_DO_INLINE_FREE

#if SAC_DO_HM_XTHR_FREE
#error SAC_DO_INLINE_FREE is incompatible with SAC_DO_HM_XTHR_FREE !
/* and SAC_DO_INLINE_FREE is a folly anyways */
#endif

#define SAC_HM_FreeLargeChunk(addr, arena)                                               \
    {                                                                                    \
        SAC_HM_header_t *freep;                                                          \
                                                                                         \
        freep = (addr)-2;                                                                \
                                                                                         \
        SAC_HM_LARGECHUNK_PREVSIZE (freep + SAC_HM_LARGECHUNK_SIZE (freep))              \
          = SAC_HM_LARGECHUNK_SIZE (freep);                                              \
                                                                                         \
        SAC_HM_LARGECHUNK_NEXTFREE (freep)                                               \
          = SAC_HM_LARGECHUNK_NEXTFREE ((arena)->freelist);                              \
        SAC_HM_LARGECHUNK_NEXTFREE ((arena)->freelist) = freep;                          \
    }

#define SAC_HM_FreeSmallChunk(addr, arena)                                               \
    {                                                                                    \
        SAC_HM_header_t *freep;                                                          \
                                                                                         \
        freep = (addr)-1;                                                                \
                                                                                         \
        SAC_HM_SMALLCHUNK_NEXTFREE (freep)                                               \
          = SAC_HM_SMALLCHUNK_NEXTFREE ((arena)->freelist);                              \
        SAC_HM_SMALLCHUNK_NEXTFREE ((arena)->freelist) = freep;                          \
    }

#endif /* SAC_DO_INLINE_FREE */

#if SAC_DO_MSCA

#define SAC_HM_CACHE_ADJUST(size, cache_size, factor)                                    \
    ((cache_size) <= 0                                                                   \
       ? 0                                                                               \
       : ((size) % (cache_size) < (cache_size) * (factor)                                \
            ? (SAC_HM_size_byte_t) ((cache_size) * (factor)) - (size) % (cache_size)     \
            : ((cache_size) - (size) % (cache_size) < (cache_size) * (factor)            \
                 ? (SAC_HM_size_byte_t) ((cache_size) * (factor)) + (cache_size)         \
                     - (size) % (cache_size)                                             \
                 : 0)))

#else /* SAC_DO_MSCA */

#define SAC_HM_CACHE_ADJUST(size, cache_size, factor) 0

#endif /* SAC_DO_MSCA */

#else /* SAC_DO_PHM */

#define SAC_HM_SETUP()
#define SAC_HM_PRINT()
#define SAC_HM_DEFINE()

#if SAC_DO_CHECK_MALLOC
SAC_C_EXTERN void *SAC_HM_MallocCheck (unsigned int);
#define SAC_HM_MALLOC(var, size, basetype) var = (basetype *)SAC_HM_MallocCheck (size);
#define SAC_HM_MALLOC_AS_SCALAR(var, size, basetype)                                     \
    var = (basetype)SAC_HM_MallocCheck (size);
#else /* SAC_DO_CHECK_MALLOC */
#if !defined(SAC_DO_CUDA_ALLOC) || SAC_DO_CUDA_ALLOC == SAC_CA_system                    \
  || SAC_DO_CUDA_ALLOC == SAC_CA_cureg
#define SAC_HM_MALLOC(var, size, basetype) var = (basetype *)malloc (size);
#elif SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define SAC_HM_MALLOC(var, size, basetype)                                               \
    do {                                                                                 \
        SAC_TR_GPU_PRINT ("Allocating CUDA host allocated area.");                       \
        cudaHostAlloc ((void **)&var, size, cudaHostAllocPortable);                      \
        SAC_GET_CUDA_MALLOC_ERROR ();                                                    \
    } while (0);
#else /* managed */
#define SAC_HM_MALLOC(var, size, basetype)                                               \
    do {                                                                                 \
        SAC_TR_GPU_PRINT ("Allocating CUDA managed memory area.");                       \
        cudaMallocManaged ((void **)&var, size, cudaMemAttachGlobal);                    \
        SAC_GET_CUDA_MALLOC_ERROR ();                                                    \
    } while (0);
#endif

#if !defined(SAC_DO_CUDA_ALLOC) || SAC_DO_CUDA_ALLOC == SAC_CA_system                    \
  || SAC_DO_CUDA_ALLOC == SAC_CA_cureg
#define SAC_HM_MALLOC_AS_SCALAR(var, size, basetype) var = (basetype)malloc (size);
#elif SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define SAC_HM_MALLOC_AS_SCALAR(var, size, basetype)                                     \
    do {                                                                                 \
        SAC_TR_GPU_PRINT ("Allocating CUDA host allocated area.");                       \
        cudaHostAlloc ((void **)&var, size, cudaHostAllocPortable);                      \
        SAC_GET_CUDA_MALLOC_ERROR ();                                                    \
    } while (0);
#else /* managed */
#define SAC_HM_MALLOC_AS_SCALAR(var, size, basetype)                                     \
    do {                                                                                 \
        SAC_TR_GPU_PRINT ("Allocating CUDA managed memory area.");                       \
        cudaMallocManaged ((void **)&var, size, cudaMemAttachGlobal);                    \
        SAC_GET_CUDA_MALLOC_ERROR ();                                                    \
    } while (0);
#endif
#endif /* SAC_DO_CHECK_MALLOC */

#define SAC_HM_MALLOC_FIXED_SIZE(var, size, basetype) SAC_HM_MALLOC (var, size, basetype)

#define SAC_HM_MALLOC_FIXED_SIZE_WITH_DESC(var, var_desc, size, dim, basetype,           \
                                           descbasetype)                                 \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (var, (size), basetype)                                 \
        SAC_HM_MALLOC_FIXED_SIZE (var_desc, BYTE_SIZE_OF_DESC (dim), descbasetype)       \
    }

#if !defined(SAC_DO_CUDA_ALLOC) || SAC_DO_CUDA_ALLOC == SAC_CA_system                    \
  || SAC_DO_CUDA_ALLOC == SAC_CA_cureg
#define SAC_HM_FREE(addr) free (addr);
#elif SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define SAC_HM_FREE(addr)                                                                \
    do {                                                                                 \
        SAC_TR_GPU_PRINT ("Allocating CUDA host allocated area.");                       \
        cudaFreeHost (addr);                                                             \
    } while (0);
#else /* managed */
#define SAC_HM_FREE(addr)                                                                \
    do {                                                                                 \
        SAC_TR_GPU_PRINT ("Allocating CUDA managed memory area.");                       \
        cudaFree (addr);                                                                 \
    } while (0);
#endif

#define SAC_HM_FREE_DESC(addr) SAC_HM_FREE (addr)

#define SAC_HM_FREE_FIXED_SIZE(addr, size) SAC_HM_FREE (addr)

#define SAC_HM_DEFINE_THREAD_STATUS(status)

#if !SAC_MUTC_MACROS
SAC_C_EXTERN void SAC_HM_ShowDiagnostics (void);
#endif

#endif /* SAC_DO_PHM */

#endif /* _SAC_HEAPMGR_H */
