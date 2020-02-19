/**
 * @file
 * @brief This file specifies a set of functions used by profile.c to profile memory
 *        associated with a CUDA device
 */
#include "libsac/profile/profile_cuda.h"

#include <stdio.h>

#include "libsac/profile/profile_print.h"
#include "runtime/essentials_h/bool.h"

/* Global Variables */

/* these counter are for all memory operations */
static size_t SAC_PF_MEM_CUDA_alloc_memsize = 0; /**< Holds the total size of memory allocated in bytes */
static size_t SAC_PF_MEM_CUDA_free_memsize = 0; /**< Holds the total size of memory freed in bytes */
static size_t SAC_PF_MEM_CUDA_max_alloc = 0; /**< Holds max memory allocated size in bytes */

/* these counters deal with arrays */
static size_t SAC_PF_MEM_CUDA_alloc_memcnt = 0; /**< Holds the count of calls to malloc/calloc/realloc */
static size_t SAC_PF_MEM_CUDA_free_memcnt = 0; /**< Holds the count of calls to free */

/* these counters deal with descriptors */
static size_t SAC_PF_MEM_CUDA_alloc_descnt = 0; /**< Holds the count of descriptor allocations */
static size_t SAC_PF_MEM_CUDA_free_descnt = 0; /**< Holds the count of descriptor frees */

/**
 * @brief Increments the alloc counter.
 *
 * @param size Size in bytes
 */
void
SAC_PF_MEM_CUDA_AllocMemcnt (size_t size)
{
    SAC_PF_MEM_CUDA_alloc_memcnt += 1;
    SAC_PF_MEM_CUDA_alloc_memsize += size;
}

/**
 * @brief Increments the free counter.
 *
 * @param size Size in bytes
 */
void
SAC_PF_MEM_CUDA_FreeMemcnt (size_t size)
{
    SAC_PF_MEM_CUDA_free_memcnt += 1;
    SAC_PF_MEM_CUDA_free_memsize += size;
}

/**
 * @brief Increments the descriptor alloc counter.
 *
 * @param size Size in bytes
 */
void
SAC_PF_MEM_CUDA_AllocDescnt (size_t size)
{
    SAC_PF_MEM_CUDA_alloc_descnt += 1;
    SAC_PF_MEM_CUDA_alloc_memsize += size;
}

/**
 * @brief Increments the descriptor free counter.
 *
 * @param size Size in bytes
 */
void
SAC_PF_MEM_CUDA_FreeDescnt (size_t size)
{
    SAC_PF_MEM_CUDA_free_descnt += 1;
    SAC_PF_MEM_CUDA_free_memsize += size;
}

/**
 * @brief Adds to the global maximum memory allocated value.
 *
 * @param size Size in bytes
 */
void
SAC_PF_MEM_CUDA_AddToMax (size_t size)
{
    if (size > SAC_PF_MEM_CUDA_max_alloc)
        SAC_PF_MEM_CUDA_max_alloc = size;
}

/**
 * @brief Call to print CUDA memory profiling statistics.
 */
void
SAC_PF_MEM_CUDA_PrintStats ()
{
    SAC_PF_PrintHeader ("CUDA Memory Profile");

    fprintf (stderr, "\n*** %-72s\n", "Memory operation counters:");
    SAC_PF_PrintSection ("For Arrays");
    SAC_PF_PrintCount ("no. calls to (m)alloc", "", SAC_PF_MEM_CUDA_alloc_memcnt);
    SAC_PF_PrintCount ("no. calls to free", "", SAC_PF_MEM_CUDA_free_memcnt);
    SAC_PF_PrintCount ("no. reuses of memory", "", 0);
    SAC_PF_PrintSection ("For Descriptors");
    SAC_PF_PrintCount ("no. calls to (m)alloc", "", SAC_PF_MEM_CUDA_alloc_descnt);
    SAC_PF_PrintCount ("no. calls to free", "", SAC_PF_MEM_CUDA_free_descnt);

    fprintf (stderr, "\n*** %-72s\n", "Memory usage counters:");
    SAC_PF_PrintSize ("total size of memory allocated", "",
                      BYTES_TO_KBYTES (SAC_PF_MEM_CUDA_alloc_memsize), "Kbytes");
    SAC_PF_PrintSize ("total size of memory freed", "",
                      BYTES_TO_KBYTES (SAC_PF_MEM_CUDA_free_memsize), "Kbytes");
    SAC_PF_PrintSize ("size of largest allocation", "",
                      BYTES_TO_KBYTES (SAC_PF_MEM_CUDA_max_alloc), "Kbytes");
}
