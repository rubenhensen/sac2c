/*
 *
 * $Log$
 * Revision 1.10  2000/02/04 16:50:20  cg
 * Added MSCA (memory size cache adjustment)
 *
 * Revision 1.9  2000/01/17 19:46:26  cg
 * Adjusted target architecture discrimination to new project wide standard.
 *
 * Revision 1.8  2000/01/17 17:58:45  cg
 * Added support for optimized allocation of refernce counters.
 *
 * Revision 1.7  2000/01/17 16:25:58  cg
 * Completely revised version:
 * Added multi-threaded heap management.
 *
 * Revision 1.6  1999/09/22 13:27:03  cg
 * Converted memory allocation macro to command postion (syntactically)
 * rather than expression position. This allows for more flexibility
 * in code generation as alloc and free operations may now be implemented
 * in a similar way.
 *
 * Revision 1.5  1999/09/17 14:33:34  cg
 * New version of SAC heap manager:
 *  - no special API functions for top arena.
 *  - coalascing is always done deferred.
 *  - no doubly linked free lists any more.
 *
 * Revision 1.4  1999/07/29 07:35:41  cg
 * Two new performance related features added to SAC private heap
 * management:
 *   - pre-splitting for arenas with fixed size chunks.
 *   - deferred coalascing for arenas with variable chunk sizes.
 *
 * Revision 1.3  1999/07/16 09:39:55  cg
 * Minor beautifications
 *
 * Revision 1.2  1999/07/09 07:34:40  cg
 * Some bugs fixed.
 *
 *
 * Revision 1.1  1998/07/08 16:54:34  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_heapmgr.h
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

#ifndef SAC_HEAPMGR_H
#define SAC_HEAPMGR_H

/*
 * Basic type definitions.
 */

typedef unsigned int SAC_HM_size_byte_t;
typedef int SAC_HM_size_unit_t;

#ifndef SAC_COMPILE_SACLIB

#if defined(SAC_FOR_SOLARIS_SPARC)
#include <sys/types.h>
/* typedef unsigned int size_t;  */

#elif defined(SAC_FOR_LINUX_X86)
#include <sys/types.h>
/* typedef unsigned int size_t;  */

#elif defined(SAC_FOR_OSF_ALPHA)
#include <sys/types.h>
/* typedef unsigned int size_t;  */

#else
typedef UNKNOWN_OS size_t;

#endif

#endif /* SAC_COMPILE_SACLIB */

/*
 * Declaration of conventional heap management functions
 * used from within compiled SAC programs.
 */

extern void *malloc (size_t size);
extern void free (void *addr);

/*
 * Macro redefinitions for compatibility reasons with standard library.
 */

#ifndef SAC_MALLOC
#define SAC_MALLOC(x) malloc (x)
#endif

#ifndef SAC_FREE
#define SAC_FREE(x) free (x)
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

#ifndef SAC_MIN
#define SAC_MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef SAC_MAX
#define SAC_MAX(a, b) ((a) < (b) ? (b) : (a))
#endif

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
    unsigned long int size;            /* in bytes */
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
          mincs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                \
    }

#define SAC_HM_ARENA_OF_LARGE_CHUNKS(n, binsize, mincs)                                  \
    {                                                                                    \
        n, SAC_HM_LARGE_ARENA_FREELIST_BASE (n), SAC_HM_arenas[0][n].freelist, binsize,  \
          mincs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                \
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
#define SAC_HM_ADDR_SIZE(addr) ((((SAC_HM_header_t *)addr) - 1)->data1.size)
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
    SAC_HM_single_threaded,
    SAC_HM_multi_threaded,
    SAC_HM_any_threaded
} SAC_HM_thread_status_t;

#define SAC_HM_DEFINE_THREAD_STATUS(status)                                              \
    static const SAC_HM_thread_status_t SAC_HM_thread_status = status;

/*
 * Mutex locks for multi-threaded execution.
 */

SAC_MT_DECLARE_LOCK (SAC_HM_top_arena_lock);
SAC_MT_DECLARE_LOCK (SAC_HM_diag_counter_lock);

/*
 * Declaration of SAC heap management global variables.
 */

extern int SAC_HM_not_yet_initialized;
extern unsigned long int SAC_HM_acquire_top_arena_lock;

/*
 * Declaration of SAC heap management functions.
 */

extern void SAC_HM_SetupMaster ();
extern void SAC_HM_SetupWorkers (unsigned int num_threads);

extern void *SAC_HM_MallocSmallChunk (SAC_HM_size_unit_t units, SAC_HM_arena_t *arena);
extern void *SAC_HM_MallocLargeChunk (SAC_HM_size_unit_t units, SAC_HM_arena_t *arena);

extern void SAC_HM_FreeSmallChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena);
extern void SAC_HM_FreeLargeChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena);

extern void *SAC_HM_MallocAnyChunk_at (SAC_HM_size_byte_t size);
extern void *SAC_HM_MallocAnyChunk_st (SAC_HM_size_byte_t size);
extern void *SAC_HM_MallocAnyChunk_mt (SAC_HM_size_byte_t size, unsigned int thread_id);

extern void *SAC_HM_MallocSmallChunk_at (SAC_HM_size_unit_t units, int arena_num);
extern void *SAC_HM_MallocLargeChunk_at (SAC_HM_size_unit_t units, int arena_num);
extern void *SAC_HM_MallocTopArena_at (SAC_HM_size_unit_t units);
extern void *SAC_HM_MallocTopArena_mt (SAC_HM_size_unit_t units);

extern void SAC_HM_FreeTopArena_mt (SAC_HM_header_t *addr);
extern void SAC_HM_FreeTopArena_at (SAC_HM_header_t *addr);

extern void SAC_HM_ShowDiagnostics ();
extern void SAC_HM_CheckAllocPatternAnyChunk (SAC_HM_header_t *addr);

/*
 * Definition of general macros.
 */

#if SAC_DO_MULTITHREAD
#define SAC_HM_SETUP()                                                                   \
    {                                                                                    \
        if (SAC_HM_not_yet_initialized) {                                                \
            SAC_HM_SetupMaster ();                                                       \
            SAC_HM_not_yet_initialized = 0;                                              \
        }                                                                                \
        SAC_HM_SetupWorkers (SAC_MT_THREADS ());                                         \
    }
#else /* SAC_DO_MULTITHREAD */
#define SAC_HM_SETUP()                                                                   \
    {                                                                                    \
        if (SAC_HM_not_yet_initialized) {                                                \
            SAC_HM_SetupMaster ();                                                       \
            SAC_HM_not_yet_initialized = 0;                                              \
        }                                                                                \
    }
#endif /* SAC_DO_MULTITHREAD */

#if SAC_DO_MULTITHREAD
#define SAC_HM_DEFINE_INITIAL_THREAD_STATUS()                                            \
    SAC_HM_DEFINE_THREAD_STATUS (SAC_HM_any_threaded)
#else /* SAC_DO_MULTITHREAD */
#define SAC_HM_DEFINE_INITIAL_THREAD_STATUS()                                            \
    SAC_HM_DEFINE_THREAD_STATUS (SAC_HM_single_threaded)
#endif /* SAC_DO_MULTITHREAD */

#if SAC_DO_COMPILE_MODULE
#define SAC_HM_DEFINE()                                                                  \
    extern SAC_HM_arena_t SAC_HM_arenas[][SAC_HM_NUM_ARENAS + 2];                        \
    SAC_HM_DEFINE_INITIAL_THREAD_STATUS ();                                              \
    static const unsigned int SAC_MT_mythread = 0;
#else
#define SAC_HM_DEFINE()                                                                  \
    SAC_HM_arena_t SAC_HM_arenas[SAC_SET_THREADS_MAX][SAC_HM_NUM_ARENAS + 2]             \
      = SAC_HM_SETUP_ARENAS ();                                                          \
    SAC_HM_DEFINE_INITIAL_THREAD_STATUS ();                                              \
    static const unsigned int SAC_MT_mythread = 0;                                       \
    const SAC_HM_size_byte_t SAC_HM_initial_master_arena_of_arenas_size                  \
      = SAC_SET_INITIAL_MASTER_HEAPSIZE;                                                 \
    const SAC_HM_size_byte_t SAC_HM_initial_worker_arena_of_arenas_size                  \
      = SAC_SET_INITIAL_WORKER_HEAPSIZE;                                                 \
    const SAC_HM_size_byte_t SAC_HM_initial_top_arena_size                               \
      = SAC_SET_INITIAL_UNIFIED_HEAPSIZE;                                                \
    const unsigned int SAC_HM_max_worker_threads = SAC_SET_THREADS_MAX - 1;
#endif
/*
 * The above definition of SAC_MT_mythread is only done to assure that
 * this variable is always declared. Whenever it is really used to determine
 * the thread ID, this global variable is shadowed by a local identifier.
 * Unfortunately, the heap manager inserts code that references this
 * variable but which is guaranteed to be executed only if SAC_MT_mythread
 * is set correctly. However, to suit the C compiler SAC_MT_mythread must
 * nevertheless exist.
 */

#if SAC_DO_CHECK_HEAP
#define SAC_HM_PRINT() SAC_HM_ShowDiagnostics ()
#define SAC_HM_INC_DIAG_COUNTER(counter) (counter)++
#else
#define SAC_HM_PRINT()
#define SAC_HM_INC_DIAG_COUNTER(counter)
#endif

/*
 * Definition of memory allocation macros.
 */

#define SAC_HM_MALLOC(var, size)                                                         \
    {                                                                                    \
        switch (SAC_HM_thread_status) {                                                  \
        case SAC_HM_single_threaded:                                                     \
            var = SAC_HM_MallocAnyChunk_st (size);                                       \
            break;                                                                       \
        case SAC_HM_multi_threaded:                                                      \
            var = SAC_HM_MallocAnyChunk_mt (size, SAC_MT_MYTHREAD ());                   \
            break;                                                                       \
        case SAC_HM_any_threaded:                                                        \
            var = SAC_HM_MallocAnyChunk_at (size);                                       \
            break;                                                                       \
        }                                                                                \
    }

#if 0
/*
 * Although for small examples the following allocator code in expression
 * position yields the same assembler code results as the current version
 * in command position, the new form has proved to be more efficient in
 * large applications for a still unknown reason.
 * 
 * The new form, however, is also more handy for introducing combined 
 * allocation facilities for data structures and their associated 
 * reference counters.
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

#if SAC_DO_MULTITHREAD

#define SAC_HM_MALLOC_SMALL_CHUNK(var, units, arena_num)                                 \
    {                                                                                    \
        switch (SAC_HM_thread_status) {                                                  \
        case SAC_HM_single_threaded:                                                     \
            var = SAC_HM_MallocSmallChunk (units, &(SAC_HM_arenas[0][arena_num]));       \
            break;                                                                       \
        case SAC_HM_multi_threaded:                                                      \
            var = SAC_HM_MallocSmallChunk (units, &(SAC_HM_arenas[SAC_MT_MYTHREAD ()]    \
                                                                 [arena_num]));          \
            break;                                                                       \
        case SAC_HM_any_threaded:                                                        \
            var = SAC_HM_MallocSmallChunk_at (units, arena_num);                         \
            break;                                                                       \
        }                                                                                \
    }

#define SAC_HM_MALLOC_LARGE_CHUNK(var, units, arena_num)                                 \
    {                                                                                    \
        switch (SAC_HM_thread_status) {                                                  \
        case SAC_HM_single_threaded:                                                     \
            var = SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][arena_num]));       \
            break;                                                                       \
        case SAC_HM_multi_threaded:                                                      \
            var = SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[SAC_MT_MYTHREAD ()]    \
                                                                 [arena_num]));          \
            break;                                                                       \
        case SAC_HM_any_threaded:                                                        \
            var = SAC_HM_MallocLargeChunk_at (units, arena_num);                         \
            break;                                                                       \
        }                                                                                \
    }

#define SAC_HM_MALLOC_TOP_ARENA(var, units)                                              \
    {                                                                                    \
        switch (SAC_HM_thread_status) {                                                  \
        case SAC_HM_single_threaded:                                                     \
            var                                                                          \
              = SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]));  \
            break;                                                                       \
        case SAC_HM_multi_threaded:                                                      \
            SAC_MT_ACQUIRE_LOCK (SAC_HM_top_arena_lock);                                 \
            SAC_HM_INC_DIAG_COUNTER (SAC_HM_acquire_top_arena_lock);                     \
            var                                                                          \
              = SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]));  \
            SAC_MT_RELEASE_LOCK (SAC_HM_top_arena_lock);                                 \
            break;                                                                       \
        case SAC_HM_any_threaded:                                                        \
            var = SAC_HM_MallocTopArena_at (units);                                      \
            break;                                                                       \
        }                                                                                \
    }

#else /* SAC_DO_MULTITHREAD */

#define SAC_HM_MALLOC_SMALL_CHUNK(var, units, arena_num)                                 \
    var = SAC_HM_MallocSmallChunk (units, &(SAC_HM_arenas[0][arena_num]));

#define SAC_HM_MALLOC_LARGE_CHUNK(var, units, arena_num)                                 \
    var = SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][arena_num]));

#define SAC_HM_MALLOC_TOP_ARENA(var, units)                                              \
    var = SAC_HM_MallocLargeChunk (units, &(SAC_HM_arenas[0][SAC_HM_TOP_ARENA]));

#endif /* SAC_DO_MULTITHREAD */

#if SAC_DO_APS

#if SAC_DO_MSCA

#define SAC_HM_MALLOC_FIXED_SIZE(var, size)                                              \
    {                                                                                    \
        if ((size) <= SAC_HM_ARENA_4_MAXCS_BYTES) {                                      \
            if ((size) <= SAC_HM_ARENA_2_MAXCS_BYTES) {                                  \
                if ((size) <= SAC_HM_ARENA_1_MAXCS_BYTES) {                              \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 2, 1);                               \
                } else {                                                                 \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 4, 2);                               \
                }                                                                        \
            } else {                                                                     \
                if ((size) <= SAC_HM_ARENA_3_MAXCS_BYTES) {                              \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 8, 3);                               \
                } else {                                                                 \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 16, 4);                              \
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
                    SAC_HM_MALLOC_LARGE_CHUNK (var, _units, 5);                          \
                } else {                                                                 \
                    SAC_HM_MALLOC_LARGE_CHUNK (var, _units, 6);                          \
                }                                                                        \
            } else {                                                                     \
                if (_units < SAC_HM_ARENA_8_MINCS) {                                     \
                    SAC_HM_MALLOC_LARGE_CHUNK (var, _units, 7);                          \
                } else {                                                                 \
                    SAC_HM_MALLOC_TOP_ARENA (var, _units);                               \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#else /* SAC_DO_MSCA */

#define SAC_HM_MALLOC_FIXED_SIZE(var, size)                                              \
    {                                                                                    \
        if ((size) <= SAC_HM_ARENA_4_MAXCS_BYTES) {                                      \
            if ((size) <= SAC_HM_ARENA_2_MAXCS_BYTES) {                                  \
                if ((size) <= SAC_HM_ARENA_1_MAXCS_BYTES) {                              \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 2, 1);                               \
                } else {                                                                 \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 4, 2);                               \
                }                                                                        \
            } else {                                                                     \
                if ((size) <= SAC_HM_ARENA_3_MAXCS_BYTES) {                              \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 8, 3);                               \
                } else {                                                                 \
                    SAC_HM_MALLOC_SMALL_CHUNK (var, 16, 4);                              \
                }                                                                        \
            }                                                                            \
        } else {                                                                         \
            const SAC_HM_size_unit_t units = SAC_HM_BYTES_2_UNITS (size) + 2;            \
            if (units < SAC_HM_ARENA_7_MINCS) {                                          \
                if (units < SAC_HM_ARENA_6_MINCS) {                                      \
                    SAC_HM_MALLOC_LARGE_CHUNK (var, units, 5);                           \
                } else {                                                                 \
                    SAC_HM_MALLOC_LARGE_CHUNK (var, units, 6);                           \
                }                                                                        \
            } else {                                                                     \
                if (units < SAC_HM_ARENA_8_MINCS) {                                      \
                    SAC_HM_MALLOC_LARGE_CHUNK (var, units, 7);                           \
                } else {                                                                 \
                    SAC_HM_MALLOC_TOP_ARENA (var, units);                                \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#endif /* SAC_DO_MSCA */

#else /* SAC_DO_APS */

#define SAC_HM_MALLOC_FIXED_SIZE(var, size) SAC_HM_MALLOC (var, size)

#endif /* SAC_DO_APS */

#if SAC_DO_RCAO

#define SAC_HM_MALLOC_FIXED_SIZE_WITH_RC(var, var_rc, size)                              \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (var, ((size) + (2 * SAC_HM_UNIT_SIZE)));               \
        var_rc = (int *)(((SAC_HM_header_t *)var) + (SAC_HM_BYTES_2_UNITS (size) + 1));  \
        SAC_HM_ADDR_ARENA (var_rc) = NULL;                                               \
    }

#define SAC_HM_MALLOC_WITH_RC(var, var_rc, size)                                         \
    {                                                                                    \
        SAC_HM_MALLOC (var, ((size) + (2 * SAC_HM_UNIT_SIZE)));                          \
        var_rc = (int *)(((SAC_HM_header_t *)var) + (SAC_HM_BYTES_2_UNITS (size) + 1));  \
        SAC_HM_ADDR_ARENA (var_rc) = NULL;                                               \
    }

#else /* SAC_DO_RCAO */

#define SAC_HM_MALLOC_FIXED_SIZE_WITH_RC(var, var_rc, size)                              \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (var, (size));                                          \
        SAC_HM_MALLOC_FIXED_SIZE (var_rc, sizeof (int));                                 \
    }

#define SAC_HM_MALLOC_WITH_RC(var, var_rc, size)                                         \
    {                                                                                    \
        SAC_HM_MALLOC (var, size);                                                       \
        SAC_HM_MALLOC_FIXED_SIZE (var_rc, sizeof (int));                                 \
    }

#endif /* SAC_DO_RCAO */

/*
 * Definition of memory de-allocation macros.
 */

#define SAC_HM_FREE(addr) free (addr);

#if SAC_DO_APS

/*
 * Note, that due to RCAO the size of the allocated chunk might be 2 untits
 * larger than actually required by the size of the data object to be stored.
 */
#define SAC_HM_FREE_FIXED_SIZE(addr, size)                                               \
    {                                                                                    \
        if ((size) == sizeof (int)) {                                                    \
            if (SAC_HM_ADDR_ARENA (addr) != NULL) {                                      \
                SAC_HM_FreeSmallChunk ((SAC_HM_header_t *)addr,                          \
                                       SAC_HM_ADDR_ARENA (addr));                        \
            }                                                                            \
        } else {                                                                         \
            if (((size) + 2 * SAC_HM_UNIT_SIZE) <= SAC_HM_ARENA_4_MAXCS_BYTES) {         \
                SAC_HM_FreeSmallChunk ((SAC_HM_header_t *)addr,                          \
                                       SAC_HM_ADDR_ARENA (addr));                        \
            } else {                                                                     \
                if (size <= SAC_HM_ARENA_4_MAXCS_BYTES) {                                \
                    if (SAC_HM_ADDR_ARENA (addr)->num == 4) {                            \
                        SAC_HM_FreeSmallChunk ((SAC_HM_header_t *)addr,                  \
                                               SAC_HM_ADDR_ARENA (addr));                \
                    } else {                                                             \
                        SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr,                  \
                                               SAC_HM_ADDR_ARENA (addr));                \
                    }                                                                    \
                } else {                                                                 \
                    if (SAC_HM_BYTES_2_UNITS (size) + (2 + 2) < SAC_HM_ARENA_8_MINCS) {  \
                        SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr,                  \
                                               SAC_HM_ADDR_ARENA (addr));                \
                    } else {                                                             \
                        if (SAC_HM_BYTES_2_UNITS (size) + 2 < SAC_HM_ARENA_8_MINCS) {    \
                            if (SAC_HM_ADDR_ARENA (addr)->num == 7) {                    \
                                SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr,          \
                                                       SAC_HM_ADDR_ARENA (addr));        \
                            } else {                                                     \
                                switch (SAC_HM_thread_status) {                          \
                                case SAC_HM_single_threaded:                             \
                                    SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr,      \
                                                           &(SAC_HM_arenas               \
                                                               [0][SAC_HM_TOP_ARENA]));  \
                                    break;                                               \
                                case SAC_HM_multi_threaded:                              \
                                    SAC_HM_FreeTopArena_mt ((SAC_HM_header_t *)addr);    \
                                    break;                                               \
                                case SAC_HM_any_threaded:                                \
                                    SAC_HM_FreeTopArena_at ((SAC_HM_header_t *)addr);    \
                                    break;                                               \
                                }                                                        \
                            }                                                            \
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
                    }                                                                    \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#else /* SAC_DO_APS */

#define SAC_HM_FREE_FIXED_SIZE(addr, size) SAC_HM_FREE (addr)

#endif /* SAC_DO_APS */

#define SAC_DO_INLINE_FREE 0

#if SAC_DO_INLINE_FREE
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

#define SAC_HM_MALLOC_FIXED_SIZE_WITH_RC(var, var_rc, size)                              \
    {                                                                                    \
        SAC_HM_MALLOC (var, (size));                                                     \
        SAC_HM_MALLOC (var_rc, sizeof (int));                                            \
    }

#define SAC_HM_MALLOC_WITH_RC(var, var_rc, size)                                         \
    {                                                                                    \
        SAC_HM_MALLOC (var, size);                                                       \
        SAC_HM_MALLOC (var_rc, sizeof (int));                                            \
    }

#define SAC_HM_MALLOC_FIXED_SIZE(var, size) SAC_HM_MALLOC (var, size)
#define SAC_HM_FREE_FIXED_SIZE(addr, size) SAC_HM_FREE (addr)

#define SAC_HM_DEFINE_THREAD_STATUS(status)

#if SAC_DO_CHECK_MALLOC

extern void *SAC_HM_MallocCheck (unsigned int);

#define SAC_HM_MALLOC(var, size) var = SAC_HM_MallocCheck (size)
#define SAC_HM_FREE(addr) free (addr)

#else /* SAC_DO_CHECK_MALLOC */

#define SAC_HM_MALLOC(var, size) var = malloc (size)
#define SAC_HM_FREE(addr) free (addr)

#endif /* SAC_DO_CHECK_MALLOC */

#endif /* SAC_DO_PHM */

#endif /* SAC_HEAPMGR_H */
