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

#define NUM_ARENAS 9
#define NUM_SMALLCHUNK_ARENAS 5

#define ALIGNMENT sizeof (double)

/*
 * Type definitions of internal heap management data structures.
 */

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

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
    } data3;
    char
      align[(((MAX (MAX (sizeof (struct header_data1_t), sizeof (struct header_data2_t)),
                    sizeof (struct header_data3_t))
               - 1)
              / ALIGNMENT)
             + 1)
            * ALIGNMENT];
} SAC_HM_header_t;

#define UNIT_SIZE (sizeof (SAC_HM_header_t))

typedef struct arena_t {
    SAC_HM_header_t base[3];
    size_unit_t arena_size;     /* in units */
    size_unit_t min_chunk_size; /* in units */
    SAC_HM_header_t *freelist;
    SAC_HM_header_t *wilderness;
    void (*freefun) (SAC_HM_header_t *addr, struct arena_t *arena);
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

extern void SAC_HM_Init (size_unit_t initial_arena_of_arenas_size,
                         size_unit_t initial_top_arena_size);

extern void *SAC_HM_MallocSmallChunk (size_unit_t units, SAC_HM_arena_t *arena);
extern void *SAC_HM_MallocLargeChunk (size_unit_t units, SAC_HM_arena_t *arena);
extern void *SAC_HM_MallocTopArena (size_unit_t units, SAC_HM_arena_t *arena);

extern void SAC_HM_FreeSmallChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena);
extern void SAC_HM_FreeLargeChunk (SAC_HM_header_t *addr, SAC_HM_arena_t *arena);
extern void SAC_HM_FreeTopArena (SAC_HM_header_t *addr, SAC_HM_arena_t *arena);

/*
 * Definition of helper macros.
 */

/*
 * Definition of general macros.
 */

#define SAC_HM_INIT() SAC_HM_Init (SAC_SET_SMALL_CHUNK_MEM, SAC_SET_TOP_CHUNK_MEM)

/*
 * Definition of memory allocation macros.
 */

#define SAC_HM_MALLOC(size) malloc (size)

#define SAC_HM_UNITS(size) ((((size)-1) / UNIT_SIZE) + 3)

#define SAC_HM_MALLOC_FIXED_SIZE(size)                                                   \
    (((size) <= ARENA_4_MAXCS_BYTES)                                                     \
       ? (((size) <= ARENA_2_MAXCS_BYTES)                                                \
            ? (((size) <= ARENA_1_MAXCS_BYTES)                                           \
                 ? (SAC_HM_MallocSmallChunk (2, &(SAC_HM_arenas[1])))                    \
                 : (SAC_HM_MallocSmallChunk (4, &(SAC_HM_arenas[2]))))                   \
            : (((size) <= ARENA_3_MAXCS_BYTES)                                           \
                 ? (SAC_HM_MallocSmallChunk (8, &(SAC_HM_arenas[3])))                    \
                 : (SAC_HM_MallocSmallChunk (16, &(SAC_HM_arenas[4])))))                 \
       : ((SAC_HM_UNITS (size) < ARENA_7_MINCS)                                          \
            ? ((SAC_HM_UNITS (size) < ARENA_6_MINCS)                                     \
                 ? (SAC_HM_MallocLargeChunk (SAC_HM_UNITS (size), &(SAC_HM_arenas[5])))  \
                 : (SAC_HM_MallocLargeChunk (SAC_HM_UNITS (size), &(SAC_HM_arenas[6])))) \
            : ((SAC_HM_UNITS (size) < ARENA_8_MINCS)                                     \
                 ? (SAC_HM_MallocLargeChunk (SAC_HM_UNITS (size), &(SAC_HM_arenas[7])))  \
                 : (SAC_HM_MallocTopArena (SAC_HM_UNITS (size), &(SAC_HM_arenas[8]))))))

/*
 * Definition of memory de-allocation macros.
 */

#define SAC_HM_FREE(addr)                                                                \
    {                                                                                    \
        SAC_HM_arena_t *arena;                                                           \
        arena = SAC_HM_ADDR_2_ARENA (addr);                                              \
        arena->freefun ((SAC_HM_header_t *)addr, arena);                                 \
    }

#define SAC_HM_FREE_FIXED_SIZE(addr, size)                                               \
    {                                                                                    \
        if (size <= ARENA_4_MAXCS_BYTES) {                                               \
            if (size <= ARENA_2_MAXCS_BYTES) {                                           \
                if (size <= ARENA_1_MAXCS_BYTES) {                                       \
                    SAC_HM_FreeSmallChunk (addr, &(SAC_HM_arenas[1]));                   \
                } else {                                                                 \
                    SAC_HM_FreeSmallChunk (addr, &(SAC_HM_arenas[2]));                   \
                }                                                                        \
            } else {                                                                     \
                if (size <= ARENA_3_MAXCS_BYTES) {                                       \
                    SAC_HM_FreeSmallChunk (addr, &(SAC_HM_arenas[3]));                   \
                } else {                                                                 \
                    SAC_HM_FreeSmallChunk (addr, &(SAC_HM_arenas[4]));                   \
                }                                                                        \
            }                                                                            \
        } else {                                                                         \
            if (size < ARENA_7_MINCS) {                                                  \
                if (size < ARENA_6_MINCS) {                                              \
                    SAC_HM_FreeLargeChunk (addr, &(SAC_HM_arenas[5]));                   \
                } else {                                                                 \
                    SAC_HM_FreeLargeChunk (addr, &(SAC_HM_arenas[6]));                   \
                }                                                                        \
            } else {                                                                     \
                if (size < ARENA_8_MINCS) {                                              \
                    SAC_HM_FreeLargeChunk (addr, &(SAC_HM_arenas[7]));                   \
                } else {                                                                 \
                    SAC_HM_FreeTopArena (addr, &(SAC_HM_arenas[8]));                     \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#else /* SAC_DO_PHM */

#if SAC_DO_CHECK_MALLOC

extern void *SAC_HM_MallocCheck (unsigned int);

#define SAC_HM_INIT()
#define SAC_HM_MALLOC(size) SAC_HM_MallocCheck (size)
#define SAC_HM_MALLOC_FIXED_SIZE(size) SAC_HM_MallocCheck (size)
#define SAC_HM_FREE(addr) free (addr)
#define SAC_HM_FREE_FIXED_SIZE(addr, size) free (addr)

#else /* SAC_DO_CHECK_MALLOC */

#define SAC_HM_INIT()
#define SAC_HM_MALLOC(size) malloc (size)
#define SAC_HM_MALLOC_FIXED_SIZE(size) malloc (size)
#define SAC_HM_FREE(addr) free (addr)
#define SAC_HM_FREE_FIXED_SIZE(addr, size) free (addr)

#endif /* SAC_DO_CHECK_MALLOC */

#endif /* SAC_DO_PHM */

#endif /* SAC_HEAPMGR_H */
