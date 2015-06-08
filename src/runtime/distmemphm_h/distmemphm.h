/*****************************************************************************
 *
 * file:   distmemphm.h
 *
 * prefix: SAC_DISTMEM_HM
 *
 * description: heap manager for the distributed memory backend
 *
 *
 *****************************************************************************/

#ifndef _SAC_DISTMEMPHM_H

#if SAC_DO_DISTMEM

#include <assert.h>

/*
 * Basic type definitions.
 */

typedef unsigned long int SAC_DISTMEM_HM_size_byte_t;
typedef long int SAC_DISTMEM_HM_size_unit_t;

/*
 * Basic settings.
 */

#define SAC_DISTMEM_HM_NUM_ARENAS 9
#define SAC_DISTMEM_HM_NUM_SMALLCHUNK_ARENAS 5
#define SAC_DISTMEM_HM_ARENA_OF_ARENAS 0
#define SAC_DISTMEM_HM_TOP_ARENA (SAC_DISTMEM_HM_NUM_ARENAS - 1)

#define SAC_DISTMEM_HM_ALIGNMENT sizeof (double)

/*
 * Type definitions of internal heap management data structures.
 */

typedef union distmem_hm_header_t {
    struct distmem_hm_header_data1_t {
        SAC_DISTMEM_HM_size_unit_t size;
        struct distmem_hm_arena_t *arena;
    } data1;
    struct distmem_hm_header_data2_t {
        union distmem_hm_header_t *prevfree;
        union distmem_hm_header_t *nextfree;
    } data2;
    struct distmem_hm_header_data3_t {
        SAC_DISTMEM_HM_size_unit_t prevsize;
        SAC_DISTMEM_HM_size_unit_t diag;
    } data3;
    char align[(((SAC_MAX (SAC_MAX (sizeof (struct distmem_hm_header_data1_t),
                                    sizeof (struct distmem_hm_header_data2_t)),
                           sizeof (struct distmem_hm_header_data3_t))
                  - 1)
                 / SAC_DISTMEM_HM_ALIGNMENT)
                + 1)
               * SAC_DISTMEM_HM_ALIGNMENT];
} SAC_DISTMEM_HM_header_t;

/*
 * Access macros for administration blocks
 */

#define SAC_DISTMEM_HM_LARGECHUNK_SIZE(header) (((header) + 1)->data1.size)
#define SAC_DISTMEM_HM_LARGECHUNK_PREVSIZE(header) (((header) + 0)->data3.prevsize)
#define SAC_DISTMEM_HM_LARGECHUNK_ARENA(header) (((header) + 1)->data1.arena)
#define SAC_DISTMEM_HM_LARGECHUNK_NEXTFREE(header) (((header) + 2)->data2.nextfree)

/* smallchunk_size is valid only for the special wilderness chunk! */
#define SAC_DISTMEM_HM_SMALLCHUNK_SIZE(header) (((header) + 0)->data1.size)
#define SAC_DISTMEM_HM_SMALLCHUNK_ARENA(header) (((header) + 0)->data1.arena)
#define SAC_DISTMEM_HM_SMALLCHUNK_NEXTFREE(header) (((header) + 1)->data2.nextfree)

#define SAC_DISTMEM_HM_LARGECHUNK_DIAG(header) (((header) + 0)->data3.diag)
#define SAC_DISTMEM_HM_SMALLCHUNK_DIAG(header) (((header) + 0)->data1.size)

/*
 * Memory is always administrated in chunks of UNIT_SIZE bytes.
 */

#define SAC_DISTMEM_HM_UNIT_SIZE (sizeof (SAC_DISTMEM_HM_header_t))

#define SAC_DISTMEM_HM_BYTES_2_UNITS(size) ((((size)-1) / SAC_DISTMEM_HM_UNIT_SIZE) + 1)

/*
 * Type definition for arena data structure
 */

typedef struct distmem_hm_arena_t {
    int num;
    SAC_DISTMEM_HM_header_t freelist[3];
    SAC_DISTMEM_HM_header_t *wilderness;
    SAC_DISTMEM_HM_size_unit_t binsize;        /* in units */
    SAC_DISTMEM_HM_size_unit_t min_chunk_size; /* in units */

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
} SAC_DISTMEM_HM_arena_t;

/*
 * Static initialization of arenas
 */

#define SAC_DISTMEM_HM_SMALL_ARENA_FREELIST_BASE(n)                                      \
    {                                                                                    \
        {{0, &(SAC_DISTMEM_HM_arenas[n])}}, {{0, NULL}},                                 \
        {                                                                                \
            {                                                                            \
                0, NULL                                                                  \
            }                                                                            \
        }                                                                                \
    }

#define SAC_DISTMEM_HM_LARGE_ARENA_FREELIST_BASE(n)                                      \
    {                                                                                    \
        {{-1, NULL}}, {{0, &(SAC_DISTMEM_HM_arenas[n])}},                                \
        {                                                                                \
            {                                                                            \
                0, NULL                                                                  \
            }                                                                            \
        }                                                                                \
    }

#define SAC_DISTMEM_HM_ARENA_OF_SMALL_CHUNKS(n, binsize, mincs)                          \
    {                                                                                    \
        n, SAC_DISTMEM_HM_SMALL_ARENA_FREELIST_BASE (n),                                 \
          SAC_DISTMEM_HM_arenas[n].freelist, binsize, mincs, 0, 0, 0, 0, 0, 0, 0, 0, 0,  \
          0, 0, 0, 0, 0                                                                  \
    }

#define SAC_DISTMEM_HM_ARENA_OF_LARGE_CHUNKS(n, binsize, mincs)                          \
    {                                                                                    \
        n, SAC_DISTMEM_HM_LARGE_ARENA_FREELIST_BASE (n),                                 \
          SAC_DISTMEM_HM_arenas[n].freelist, binsize, mincs, 0, 0, 0, 0, 0, 0, 0, 0, 0,  \
          0, 0, 0, 0, 0                                                                  \
    }

#define SAC_DISTMEM_HM_SETUP_ARENAS()                                                    \
    {                                                                                    \
        SAC_DISTMEM_HM_ARENA_OF_SMALL_CHUNKS (0, SAC_DISTMEM_HM_ARENA_0_BINSIZE,         \
                                              SAC_DISTMEM_HM_ARENA_0_MINCS),             \
          SAC_DISTMEM_HM_ARENA_OF_SMALL_CHUNKS (1, SAC_DISTMEM_HM_ARENA_1_BINSIZE,       \
                                                SAC_DISTMEM_HM_ARENA_2_MINCS - 1),       \
          SAC_DISTMEM_HM_ARENA_OF_SMALL_CHUNKS (2, SAC_DISTMEM_HM_ARENA_2_BINSIZE,       \
                                                SAC_DISTMEM_HM_ARENA_3_MINCS - 1),       \
          SAC_DISTMEM_HM_ARENA_OF_SMALL_CHUNKS (3, SAC_DISTMEM_HM_ARENA_3_BINSIZE,       \
                                                SAC_DISTMEM_HM_ARENA_4_MINCS - 1),       \
          SAC_DISTMEM_HM_ARENA_OF_SMALL_CHUNKS (4, SAC_DISTMEM_HM_ARENA_4_BINSIZE,       \
                                                SAC_DISTMEM_HM_ARENA_5_MINCS - 1),       \
          SAC_DISTMEM_HM_ARENA_OF_LARGE_CHUNKS (5, SAC_DISTMEM_HM_ARENA_5_BINSIZE,       \
                                                SAC_DISTMEM_HM_ARENA_5_MINCS),           \
          SAC_DISTMEM_HM_ARENA_OF_LARGE_CHUNKS (6, SAC_DISTMEM_HM_ARENA_6_BINSIZE,       \
                                                SAC_DISTMEM_HM_ARENA_6_MINCS),           \
          SAC_DISTMEM_HM_ARENA_OF_LARGE_CHUNKS (7, SAC_DISTMEM_HM_ARENA_7_BINSIZE,       \
                                                SAC_DISTMEM_HM_ARENA_7_MINCS),           \
          SAC_DISTMEM_HM_ARENA_OF_LARGE_CHUNKS (8, SAC_DISTMEM_HM_ARENA_8_BINSIZE,       \
                                                SAC_DISTMEM_HM_ARENA_8_MINCS)            \
    }

/*
 * Access macros for chunk administration blocks
 * relative to memory location returned by malloc(), etc.
 */

#define SAC_DISTMEM_HM_ADDR_ARENA(addr)                                                  \
    ((((SAC_DISTMEM_HM_header_t *)addr) - 1)->data1.arena)

/*
 * Minimum chunk sizes (MINCS) for arenas
 */

#define SAC_DISTMEM_HM_ARENA_0_MINCS 1
#define SAC_DISTMEM_HM_ARENA_1_MINCS 2
#define SAC_DISTMEM_HM_ARENA_2_MINCS 3
#define SAC_DISTMEM_HM_ARENA_3_MINCS 5
#define SAC_DISTMEM_HM_ARENA_4_MINCS 9
#define SAC_DISTMEM_HM_ARENA_5_MINCS 17
#define SAC_DISTMEM_HM_ARENA_6_MINCS 129
#define SAC_DISTMEM_HM_ARENA_7_MINCS 1025
#define SAC_DISTMEM_HM_ARENA_8_MINCS 8193

/*
 * Maximum chunk sizes (MAXCS) for arenas
 */

#define SAC_DISTMEM_HM_ARENA_1_MAXCS_BYTES                                               \
    ((SAC_DISTMEM_HM_ARENA_2_MINCS - 2) * SAC_DISTMEM_HM_UNIT_SIZE)
#define SAC_DISTMEM_HM_ARENA_2_MAXCS_BYTES                                               \
    ((SAC_DISTMEM_HM_ARENA_3_MINCS - 2) * SAC_DISTMEM_HM_UNIT_SIZE)
#define SAC_DISTMEM_HM_ARENA_3_MAXCS_BYTES                                               \
    ((SAC_DISTMEM_HM_ARENA_4_MINCS - 2) * SAC_DISTMEM_HM_UNIT_SIZE)
#define SAC_DISTMEM_HM_ARENA_4_MAXCS_BYTES                                               \
    ((SAC_DISTMEM_HM_ARENA_5_MINCS - 2) * SAC_DISTMEM_HM_UNIT_SIZE)
#define SAC_DISTMEM_HM_ARENA_5_MAXCS_BYTES                                               \
    ((SAC_DISTMEM_HM_ARENA_6_MINCS - 3) * SAC_DISTMEM_HM_UNIT_SIZE)
#define SAC_DISTMEM_HM_ARENA_6_MAXCS_BYTES                                               \
    ((SAC_DISTMEM_HM_ARENA_7_MINCS - 3) * SAC_DISTMEM_HM_UNIT_SIZE)
#define SAC_DISTMEM_HM_ARENA_7_MAXCS_BYTES                                               \
    ((SAC_DISTMEM_HM_ARENA_8_MINCS - 3) * SAC_DISTMEM_HM_UNIT_SIZE)

/*
 * Bin sizes for arenas
 */

#define SAC_DISTMEM_HM_ARENA_0_BINSIZE 131072
#define SAC_DISTMEM_HM_ARENA_1_BINSIZE 512
#define SAC_DISTMEM_HM_ARENA_2_BINSIZE 512
#define SAC_DISTMEM_HM_ARENA_3_BINSIZE 256
#define SAC_DISTMEM_HM_ARENA_4_BINSIZE 512
#define SAC_DISTMEM_HM_ARENA_5_BINSIZE 2048
#define SAC_DISTMEM_HM_ARENA_6_BINSIZE 8196
#define SAC_DISTMEM_HM_ARENA_7_BINSIZE 32768
#define SAC_DISTMEM_HM_ARENA_8_BINSIZE 0

/*
 * Declaration of SAC distributed memory heap management global variables
 */

SAC_C_EXTERN SAC_DISTMEM_HM_arena_t SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_NUM_ARENAS + 2];

/*
 * Declaration of SAC distributed memory heap management functions
 */

SAC_C_EXTERN void SAC_DISTMEM_HM_Setup (void);

SAC_C_EXTERN void *SAC_DISTMEM_HM_MallocSmallChunk (SAC_DISTMEM_HM_size_unit_t units,
                                                    SAC_DISTMEM_HM_arena_t *arena);
SAC_C_EXTERN void *SAC_DISTMEM_HM_MallocLargeChunk (SAC_DISTMEM_HM_size_unit_t units,
                                                    SAC_DISTMEM_HM_arena_t *arena);

SAC_C_EXTERN void SAC_DISTMEM_HM_FreeSmallChunk (SAC_DISTMEM_HM_header_t *addr,
                                                 SAC_DISTMEM_HM_arena_t *arena);
SAC_C_EXTERN void SAC_DISTMEM_HM_FreeLargeChunk (SAC_DISTMEM_HM_header_t *addr,
                                                 SAC_DISTMEM_HM_arena_t *arena);

SAC_C_EXTERN void *SAC_DISTMEM_HM_MallocAnyChunk (SAC_DISTMEM_HM_size_byte_t size);
SAC_C_EXTERN void SAC_DISTMEM_HM_FreeAnyChunk (void *addr);

SAC_C_EXTERN void SAC_DISTMEM_HM_ShowDiagnostics (void);

/*
 * Definition of memory allocation macros.
 */

#define SAC_DISTMEM_HM_MALLOC(var, size, basetype)                                       \
    {                                                                                    \
        SAC_TR_DISTMEM_PRINT ("SAC_DISTMEM_HM_MALLOC( %s, %d, %s)", #var, size,          \
                              #basetype);                                                \
        SAC_DISTMEM_CHECK_IS_DSM_ALLOC_ALLOWED ();                                       \
        var = (basetype *)SAC_DISTMEM_HM_MallocAnyChunk (size);                          \
    }

#define SAC_DISTMEM_HM_MALLOC_SMALL_CHUNK(var, units, arena_num, basetype)               \
    var = (basetype *)                                                                   \
      SAC_DISTMEM_HM_MallocSmallChunk (units, &(SAC_DISTMEM_HM_arenas[arena_num]));

#define SAC_DISTMEM_HM_MALLOC_LARGE_CHUNK(var, units, arena_num, basetype)               \
    var = (basetype *)                                                                   \
      SAC_DISTMEM_HM_MallocLargeChunk (units, &(SAC_DISTMEM_HM_arenas[arena_num]));

#define SAC_DISTMEM_HM_MALLOC_TOP_ARENA(var, units, basetype)                            \
    var = (basetype *)SAC_DISTMEM_HM_MallocLargeChunk (units,                            \
                                                       &(SAC_DISTMEM_HM_arenas           \
                                                           [SAC_DISTMEM_HM_TOP_ARENA]));

/* APS: arena preselection, enabled by default */
#if SAC_DO_APS

#if SAC_DO_MSCA

#define SAC_DISTMEM_HM_MALLOC_FIXED_SIZE(var, size, basetype)                            \
    {                                                                                    \
        SAC_TR_DISTMEM_PRINT ("SAC_DISTMEM_HM_MALLOC_FIXED_SIZE( %s, %d, %s)", #var,     \
                              size, #basetype);                                          \
        SAC_DISTMEM_CHECK_IS_DSM_ALLOC_ALLOWED ();                                       \
        if ((size) <= SAC_DISTMEM_HM_ARENA_4_MAXCS_BYTES) {                              \
            if ((size) <= SAC_DISTMEM_HM_ARENA_2_MAXCS_BYTES) {                          \
                if ((size) <= SAC_DISTMEM_HM_ARENA_1_MAXCS_BYTES) {                      \
                    SAC_DISTMEM_HM_MALLOC_SMALL_CHUNK (var, 2, 1, basetype)              \
                } else {                                                                 \
                    SAC_DISTMEM_HM_MALLOC_SMALL_CHUNK (var, 4, 2, basetype)              \
                }                                                                        \
            } else {                                                                     \
                if ((size) <= SAC_DISTMEM_HM_ARENA_3_MAXCS_BYTES) {                      \
                    SAC_DISTMEM_HM_MALLOC_SMALL_CHUNK (var, 8, 3, basetype)              \
                } else {                                                                 \
                    SAC_DISTMEM_HM_MALLOC_SMALL_CHUNK (var, 16, 4, basetype)             \
                }                                                                        \
            }                                                                            \
        } else {                                                                         \
            const SAC_DISTMEM_HM_size_byte_t _real_size_1                                \
              = size                                                                     \
                + SAC_DISTMEM_HM_CACHE_ADJUST (size, SAC_SET_CACHE_3_SIZE,               \
                                               SAC_SET_CACHE_3_MSCA_FACTOR);             \
            const SAC_DISTMEM_HM_size_byte_t _real_size_2                                \
              = _real_size_1                                                             \
                + SAC_DISTMEM_HM_CACHE_ADJUST (_real_size_1, SAC_SET_CACHE_2_SIZE,       \
                                               SAC_SET_CACHE_2_MSCA_FACTOR);             \
            const SAC_DISTMEM_HM_size_byte_t _real_size_3                                \
              = _real_size_2                                                             \
                + SAC_DISTMEM_HM_CACHE_ADJUST (_real_size_2, SAC_SET_CACHE_1_SIZE,       \
                                               SAC_SET_CACHE_1_MSCA_FACTOR);             \
            const SAC_DISTMEM_HM_size_unit_t _units                                      \
              = SAC_DISTMEM_HM_BYTES_2_UNITS (_real_size_3) + 2;                         \
                                                                                         \
            if (_units < SAC_DISTMEM_HM_ARENA_7_MINCS) {                                 \
                if (_units < SAC_DISTMEM_HM_ARENA_6_MINCS) {                             \
                    SAC_DISTMEM_HM_MALLOC_LARGE_CHUNK (var, _units, 5, basetype)         \
                } else {                                                                 \
                    SAC_DISTMEM_HM_MALLOC_LARGE_CHUNK (var, _units, 6, basetype)         \
                }                                                                        \
            } else {                                                                     \
                if (_units < SAC_DISTMEM_HM_ARENA_8_MINCS) {                             \
                    SAC_DISTMEM_HM_MALLOC_LARGE_CHUNK (var, _units, 7, basetype)         \
                } else {                                                                 \
                    SAC_DISTMEM_HM_MALLOC_TOP_ARENA (var, _units, basetype)              \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#else /* SAC_DO_MSCA */

#define SAC_DISTMEM_HM_MALLOC_FIXED_SIZE(var, size, basetype)                            \
    {                                                                                    \
        SAC_TR_DISTMEM_PRINT ("SAC_DISTMEM_HM_MALLOC_FIXED_SIZE( %s, %d, %s)", #var,     \
                              size, #basetype);                                          \
        SAC_DISTMEM_CHECK_IS_DSM_ALLOC_ALLOWED ();                                       \
        if ((size) <= SAC_DISTMEM_HM_ARENA_4_MAXCS_BYTES) {                              \
            if ((size) <= SAC_DISTMEM_HM_ARENA_2_MAXCS_BYTES) {                          \
                if ((size) <= SAC_DISTMEM_HM_ARENA_1_MAXCS_BYTES) {                      \
                    SAC_DISTMEM_HM_MALLOC_SMALL_CHUNK (var, 2, 1, basetype)              \
                } else {                                                                 \
                    SAC_DISTMEM_HM_MALLOC_SMALL_CHUNK (var, 4, 2, basetype)              \
                }                                                                        \
            } else {                                                                     \
                if ((size) <= SAC_DISTMEM_HM_ARENA_3_MAXCS_BYTES) {                      \
                    SAC_DISTMEM_HM_MALLOC_SMALL_CHUNK (var, 8, 3, basetype)              \
                } else {                                                                 \
                    SAC_DISTMEM_HM_MALLOC_SMALL_CHUNK (var, 16, 4, basetype)             \
                }                                                                        \
            }                                                                            \
        } else {                                                                         \
            const SAC_DISTMEM_HM_size_unit_t units                                       \
              = SAC_DISTMEM_HM_BYTES_2_UNITS (size) + 2;                                 \
            if (units < SAC_DISTMEM_HM_ARENA_7_MINCS) {                                  \
                if (units < SAC_DISTMEM_HM_ARENA_6_MINCS) {                              \
                    SAC_DISTMEM_HM_MALLOC_LARGE_CHUNK (var, units, 5, basetype)          \
                } else {                                                                 \
                    SAC_DISTMEM_HM_MALLOC_LARGE_CHUNK (var, units, 6, basetype)          \
                }                                                                        \
            } else {                                                                     \
                if (units < SAC_DISTMEM_HM_ARENA_8_MINCS) {                              \
                    SAC_DISTMEM_HM_MALLOC_LARGE_CHUNK (var, units, 7, basetype)          \
                } else {                                                                 \
                    SAC_DISTMEM_HM_MALLOC_TOP_ARENA (var, units, basetype)               \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#endif /* SAC_DO_MSCA */

#else /* SAC_DO_APS */

#define SAC_DISTMEM_HM_MALLOC_FIXED_SIZE(var, size, basetype)                            \
    SAC_DISTMEM_HM_MALLOC (var, size, basetype)

#endif /* SAC_DO_APS */

/*
 * Definition of memory de-allocation macros.
 */

#define SAC_DISTMEM_HM_FREE(addr) SAC_DISTMEM_HM_FreeAnyChunk (addr);

#if SAC_DO_APS

#define SAC_DISTMEM_HM_FREE_FIXED_SIZE(addr, size)                                       \
    {                                                                                    \
        if (((size) + 2 * SAC_DISTMEM_HM_UNIT_SIZE)                                      \
            <= SAC_DISTMEM_HM_ARENA_4_MAXCS_BYTES) {                                     \
            SAC_DISTMEM_HM_FreeSmallChunk ((SAC_DISTMEM_HM_header_t *)addr,              \
                                           SAC_DISTMEM_HM_ADDR_ARENA (addr));            \
        } else {                                                                         \
            if (size <= SAC_DISTMEM_HM_ARENA_4_MAXCS_BYTES) {                            \
                if (SAC_DISTMEM_HM_ADDR_ARENA (addr)->num == 4) {                        \
                    SAC_DISTMEM_HM_FreeSmallChunk ((SAC_DISTMEM_HM_header_t *)addr,      \
                                                   SAC_DISTMEM_HM_ADDR_ARENA (addr));    \
                } else {                                                                 \
                    SAC_DISTMEM_HM_FreeLargeChunk ((SAC_DISTMEM_HM_header_t *)addr,      \
                                                   SAC_DISTMEM_HM_ADDR_ARENA (addr));    \
                }                                                                        \
            } else {                                                                     \
                if (SAC_DISTMEM_HM_BYTES_2_UNITS (size) + (2 + 2)                        \
                    < SAC_DISTMEM_HM_ARENA_8_MINCS) {                                    \
                    SAC_DISTMEM_HM_FreeLargeChunk ((SAC_DISTMEM_HM_header_t *)addr,      \
                                                   SAC_DISTMEM_HM_ADDR_ARENA (addr));    \
                } else {                                                                 \
                    if (SAC_DISTMEM_HM_BYTES_2_UNITS (size) + 2                          \
                        < SAC_DISTMEM_HM_ARENA_8_MINCS) {                                \
                        if (SAC_DISTMEM_HM_ADDR_ARENA (addr)->num == 7) {                \
                            SAC_DISTMEM_HM_FreeLargeChunk ((SAC_DISTMEM_HM_header_t *)   \
                                                             addr,                       \
                                                           SAC_DISTMEM_HM_ADDR_ARENA (   \
                                                             addr));                     \
                        } else {                                                         \
                            SAC_DISTMEM_HM_FreeLargeChunk (                              \
                              (SAC_DISTMEM_HM_header_t *)addr,                           \
                              &(SAC_DISTMEM_HM_arenas[SAC_DISTMEM_HM_TOP_ARENA]));       \
                        }                                                                \
                    } else {                                                             \
                        SAC_DISTMEM_HM_FreeLargeChunk ((SAC_DISTMEM_HM_header_t *)addr,  \
                                                       &(SAC_DISTMEM_HM_arenas           \
                                                           [SAC_DISTMEM_HM_TOP_ARENA])); \
                    }                                                                    \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#else /* SAC_DO_APS */

#define SAC_DISTMEM_HM_FREE_FIXED_SIZE(addr, size) SAC_DISTMEM_HM_FREE (addr)

#endif /* SAC_DO_APS */

#if SAC_DO_MSCA

#define SAC_DISTMEM_HM_CACHE_ADJUST(size, cache_size, factor)                            \
    ((cache_size) <= 0                                                                   \
       ? 0                                                                               \
       : ((size) % (cache_size) < (cache_size) * (factor)                                \
            ? (SAC_DISTMEM_HM_size_byte_t) ((cache_size) * (factor))                     \
                - (size) % (cache_size)                                                  \
            : ((cache_size) - (size) % (cache_size) < (cache_size) * (factor)            \
                 ? (SAC_DISTMEM_HM_size_byte_t) ((cache_size) * (factor)) + (cache_size) \
                     - (size) % (cache_size)                                             \
                 : 0)))

#else /* SAC_DO_MSCA */

#define SAC_DISTMEM_HM_CACHE_ADJUST(size, cache_size, factor) 0

#endif /* SAC_DO_MSCA */

#endif /* SAC_DO_DISTMEM */

#endif /* _SAC_DISTMEMPHM_H */
