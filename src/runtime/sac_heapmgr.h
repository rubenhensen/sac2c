/*
 *
 * $Log$
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

typedef unsigned int size_byte_t;
typedef int size_unit_t;

/*
 * Declaration of conventional heap management functions.
 */

extern void *malloc (size_byte_t size);
extern void *calloc (size_byte_t nelem, size_byte_t elsize);
extern void *memalign (size_byte_t alignment, size_byte_t size);
extern void *realloc (void *ptr, size_byte_t size);
extern void *valloc (size_byte_t size);
extern void free (void *addr);

/*
 * Macro redefinitions for compatibility reasons with standard library.
 */

#define SAC_MALLOC(x) SAC_HM_MALLOC (x)
#define SAC_FREE(x) SAC_HM_FREE (x)

#if SAC_DO_PHM

/*
 * Basic settings.
 */

#if SAC_DO_CHECK_HEAP
#define DIAG
#endif

#define NUM_ARENAS 9
#define NUM_SMALLCHUNK_ARENAS 5

#define ALIGNMENT sizeof (double)

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
        size_unit_t size;
        struct arena_t *arena;
    } data1;
    struct header_data2_t {
        union header_t *prevfree;
        union header_t *nextfree;
    } data2;
    struct header_data3_t {
        size_unit_t prevsize;
        size_unit_t diag;
    } data3;
    char align[(((SAC_MAX (SAC_MAX (sizeof (struct header_data1_t),
                                    sizeof (struct header_data2_t)),
                           sizeof (struct header_data3_t))
                  - 1)
                 / ALIGNMENT)
                + 1)
               * ALIGNMENT];
} SAC_HM_header_t;

/*
 * Memory is always administrated in chunks of UNIT_SIZE bytes.
 */

#define UNIT_SIZE (sizeof (SAC_HM_header_t))

typedef struct arena_t {
    int num;
    SAC_HM_header_t freelist[3];
    size_unit_t arena_size;     /* in units */
    size_unit_t min_chunk_size; /* in units */
    SAC_HM_header_t *wilderness;
    void (*freefun) (SAC_HM_header_t *addr, struct arena_t *arena);
#ifdef DIAG
    unsigned long int cnt_alloc;
    unsigned long int cnt_free;
    unsigned long int cnt_split;
    unsigned long int cnt_coalasce;
    unsigned long int size; /* in bytes */
    unsigned long int bins;
#endif
} SAC_HM_arena_t;

/*
 * Access macros.
 */

#define SAC_HM_ADDR_2_ARENA(addr) ((((SAC_HM_header_t *)addr) - 1)->data1.arena)

/*
 * Configuration of arenas.
 */

extern SAC_HM_arena_t SAC_HM_arenas[NUM_ARENAS];

/*
 * Minimum chunk sizes (MINCS) for arenas
 */

#define ARENA_1_MINCS 2
#define ARENA_2_MINCS 3
#define ARENA_3_MINCS 5
#define ARENA_4_MINCS 9
#define ARENA_5_MINCS 17
#define ARENA_6_MINCS 129
#define ARENA_7_MINCS 1025
#define ARENA_8_MINCS 8193

/*
 * Maximum chunk sizes (MAXCS) for arenas
 */

#define ARENA_1_MAXCS_BYTES ((ARENA_2_MINCS - 2) * UNIT_SIZE)
#define ARENA_2_MAXCS_BYTES ((ARENA_3_MINCS - 2) * UNIT_SIZE)
#define ARENA_3_MAXCS_BYTES ((ARENA_4_MINCS - 2) * UNIT_SIZE)
#define ARENA_4_MAXCS_BYTES ((ARENA_5_MINCS - 2) * UNIT_SIZE)

#define ARENA_5_MAXCS_BYTES ((ARENA_6_MINCS - 3) * UNIT_SIZE)
#define ARENA_6_MAXCS_BYTES ((ARENA_7_MINCS - 3) * UNIT_SIZE)
#define ARENA_7_MAXCS_BYTES ((ARENA_8_MINCS - 3) * UNIT_SIZE)

/*
 * Declaration of specific heap management functions.
 */

extern void SAC_HM_Setup (size_byte_t initial_arena_of_arenas_size,
                          size_byte_t initial_top_arena_size);

extern void *SAC_HM_MallocSmallChunk (size_unit_t units, SAC_HM_arena_t *arena);
extern void *SAC_HM_MallocSmallChunkPresplit (size_unit_t units, SAC_HM_arena_t *arena,
                                              int presplit);
extern void *SAC_HM_MallocLargeChunk (size_unit_t units, SAC_HM_arena_t *arena);
extern void *SAC_HM_MallocLargeChunkNoCoalasce (size_unit_t units, SAC_HM_arena_t *arena);

extern void SAC_HM_FreeSmallChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena);
extern void SAC_HM_FreeLargeChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena);

#if SAC_DO_CHECK_HEAP
extern void SAC_HM_ShowDiagnostics ();
extern void SAC_HM_CheckAllocPatternAnyChunk (SAC_HM_header_t *addr);
#endif

/*
 * Definition of helper macros.
 */

/*
 * Definition of general macros.
 */

#define SAC_HM_SETUP()                                                                   \
    SAC_HM_Setup (SAC_SET_INITIAL_HEAPSIZE >= 2 ? 1 : 0,                                 \
                  SAC_SET_INITIAL_HEAPSIZE >= 2 ? SAC_SET_INITIAL_HEAPSIZE - 1           \
                                                : SAC_SET_INITIAL_HEAPSIZE)

#if SAC_DO_CHECK_HEAP
#define SAC_HM_PRINT() SAC_HM_ShowDiagnostics ()
#else
#define SAC_HM_PRINT()
#endif

/*
 * Definition of memory allocation macros.
 */

#define SAC_HM_MALLOC(size) malloc (size)

#define SAC_HM_UNITS(size) ((((size)-1) / UNIT_SIZE) + 3)

#if 0
/*
 * Although for small examples the following allocator code in expression
 * position yields the same assembler code results as the current version
 * in command position, the new form has proved to be more efficient in
 * large applications for a still unknown reason.
 * 
 * The new form, however, is also more handy for introducing combined 
 * allocation facilities for data structures and their associated data
 * structures.
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
#endif

#if 1
#define SAC_HM_MALLOC_FIXED_SIZE(var, size)                                              \
    {                                                                                    \
        if ((size) <= ARENA_4_MAXCS_BYTES) {                                             \
            if ((size) <= ARENA_2_MAXCS_BYTES) {                                         \
                if ((size) <= ARENA_1_MAXCS_BYTES) {                                     \
                    var = SAC_HM_MallocSmallChunkPresplit (2, &(SAC_HM_arenas[1]), 16);  \
                } else {                                                                 \
                    var = SAC_HM_MallocSmallChunkPresplit (4, &(SAC_HM_arenas[2]), 16);  \
                }                                                                        \
            } else {                                                                     \
                if ((size) <= ARENA_3_MAXCS_BYTES) {                                     \
                    var = SAC_HM_MallocSmallChunk (8, &(SAC_HM_arenas[3]));              \
                } else {                                                                 \
                    var = SAC_HM_MallocSmallChunk (16, &(SAC_HM_arenas[4]));             \
                }                                                                        \
            }                                                                            \
        } else {                                                                         \
            if (SAC_HM_UNITS (size) < ARENA_7_MINCS) {                                   \
                if (SAC_HM_UNITS (size) < ARENA_6_MINCS) {                               \
                    var = SAC_HM_MallocLargeChunk (SAC_HM_UNITS (size),                  \
                                                   &(SAC_HM_arenas[5]));                 \
                } else {                                                                 \
                    var = SAC_HM_MallocLargeChunk (SAC_HM_UNITS (size),                  \
                                                   &(SAC_HM_arenas[6]));                 \
                }                                                                        \
            } else {                                                                     \
                if (SAC_HM_UNITS (size) < ARENA_8_MINCS) {                               \
                    var = SAC_HM_MallocLargeChunk (SAC_HM_UNITS (size),                  \
                                                   &(SAC_HM_arenas[7]));                 \
                } else {                                                                 \
                    var = SAC_HM_MallocLargeChunk (SAC_HM_UNITS (size),                  \
                                                   &(SAC_HM_arenas[8]));                 \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }
#else
#define SAC_HM_MALLOC_FIXED_SIZE(var, size) var = SAC_HM_MALLOC (size)
#endif

/*
 * Definition of memory de-allocation macros.
 */

#if SAC_DO_CHECK_HEAP
#define SAC_HM_FREE(addr)                                                                \
    {                                                                                    \
        SAC_HM_arena_t *arena;                                                           \
        SAC_HM_CheckAllocPatternAnyChunk ((SAC_HM_header_t *)addr);                      \
        arena = SAC_HM_ADDR_2_ARENA (addr);                                              \
        arena->freefun ((SAC_HM_header_t *)addr, arena);                                 \
    }
#else
#define SAC_HM_FREE(addr)                                                                \
    {                                                                                    \
        SAC_HM_arena_t *arena;                                                           \
        arena = SAC_HM_ADDR_2_ARENA (addr);                                              \
        arena->freefun ((SAC_HM_header_t *)addr, arena);                                 \
    }
#endif

#if 1
#define SAC_HM_FREE_FIXED_SIZE(addr, size)                                               \
    {                                                                                    \
        if ((size) <= ARENA_4_MAXCS_BYTES) {                                             \
            if ((size) <= ARENA_2_MAXCS_BYTES) {                                         \
                if ((size) <= ARENA_1_MAXCS_BYTES) {                                     \
                    SAC_HM_FreeSmallChunk ((SAC_HM_header_t *)addr,                      \
                                           &(SAC_HM_arenas[1]));                         \
                } else {                                                                 \
                    SAC_HM_FreeSmallChunk ((SAC_HM_header_t *)addr,                      \
                                           &(SAC_HM_arenas[2]));                         \
                }                                                                        \
            } else {                                                                     \
                if ((size) <= ARENA_3_MAXCS_BYTES) {                                     \
                    SAC_HM_FreeSmallChunk ((SAC_HM_header_t *)addr,                      \
                                           &(SAC_HM_arenas[3]));                         \
                } else {                                                                 \
                    SAC_HM_FreeSmallChunk ((SAC_HM_header_t *)addr,                      \
                                           &(SAC_HM_arenas[4]));                         \
                }                                                                        \
            }                                                                            \
        } else {                                                                         \
            if (SAC_HM_UNITS (size) < ARENA_7_MINCS) {                                   \
                if (SAC_HM_UNITS (size) < ARENA_6_MINCS) {                               \
                    SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr,                      \
                                           &(SAC_HM_arenas[5]));                         \
                } else {                                                                 \
                    SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr,                      \
                                           &(SAC_HM_arenas[6]));                         \
                }                                                                        \
            } else {                                                                     \
                if (SAC_HM_UNITS (size) < ARENA_8_MINCS) {                               \
                    SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr,                      \
                                           &(SAC_HM_arenas[7]));                         \
                } else {                                                                 \
                    SAC_HM_FreeLargeChunk ((SAC_HM_header_t *)addr,                      \
                                           &(SAC_HM_arenas[8]));                         \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }
#else
#define SAC_HM_FREE_FIXED_SIZE(addr, size) SAC_HM_FREE (addr)
#endif

#else /* SAC_DO_PHM */

#define SAC_HM_SETUP()
#define SAC_HM_PRINT()
#define SAC_HM_MALLOC_FIXED_SIZE(var, size) var = SAC_HM_MALLOC (size)
#define SAC_HM_FREE_FIXED_SIZE(addr, size) SAC_HM_FREE (addr)

#if SAC_DO_CHECK_MALLOC

extern void *SAC_HM_MallocCheck (unsigned int);

#define SAC_HM_MALLOC(size) SAC_HM_MallocCheck (size)
#define SAC_HM_FREE(addr) free (addr)

#else /* SAC_DO_CHECK_MALLOC */

#define SAC_HM_MALLOC(size) malloc (size)
#define SAC_HM_FREE(addr) free (addr)

#endif /* SAC_DO_CHECK_MALLOC */

#endif /* SAC_DO_PHM */

#endif /* SAC_HEAPMGR_H */
