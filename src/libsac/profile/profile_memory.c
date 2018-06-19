/**
 * @brief This file specifies a set of functions used by profile.c to profile memory
 *        operations, specifically count and size(s).
 */

#include <stdio.h>

#include "libsac/profile/profile_print.h"
#include "libsac/profile/profile_memory.h"

/* Global Variables */

/* these counter are for all memory operations */
static unsigned long SAC_PF_MEM_alloc_memsize = 0; /**< Holds the total size of memory allocated in bits */
static unsigned long SAC_PF_MEM_free_memsize = 0; /**< Holds the total size of memory freed in bits */

/* these counters deal with arrays */
static unsigned long SAC_PF_MEM_alloc_memcnt = 0; /**< Holds the count of calls to malloc/calloc/realloc */
static unsigned long SAC_PF_MEM_free_memcnt = 0; /**< Holds the count of calls to free */
static unsigned long SAC_PF_MEM_reuse_memcnt = 0; /**< Holds the count of dynamic reuses */

/* these counters deal with descriptors */
static unsigned long SAC_PF_MEM_alloc_descnt = 0; /**< Holds the count of descriptor allocations */
static unsigned long SAC_PF_MEM_free_descnt = 0; /**< Holds the count of descriptor frees */

/**
 * @brief Increments the alloc counter.
 *
 * @param size Size in units
 * @param typesize Size of type in bits
 */
void
SAC_PF_MEM_AllocMemcnt (int size, int typesize)
{
    SAC_PF_MEM_alloc_memcnt += 1;
    SAC_PF_MEM_alloc_memsize += size * typesize;
}

/**
 * @brief Increments the free counter.
 *
 * @param size Size in units
 * @param typesize Size of type in bits
 */
void
SAC_PF_MEM_FreeMemcnt (int size, int typesize)
{
    SAC_PF_MEM_free_memcnt += 1;
    SAC_PF_MEM_free_memsize += size * typesize;
}

/**
 * @brief Increments the descriptor alloc counter.
 *
 * @param size Size in units
 * @param typesize Size of type in bits
 */
void
SAC_PF_MEM_AllocDescnt (int size, int typesize)
{
    SAC_PF_MEM_alloc_descnt += 1;
    SAC_PF_MEM_alloc_memsize += size * typesize;
}

/**
 * @brief Increments the descriptor free counter.
 *
 * @param size Size in units
 * @param typesize Size of type in bits
 */
void
SAC_PF_MEM_FreeDescnt (int size, int typesize)
{
    SAC_PF_MEM_free_descnt += 1;
    SAC_PF_MEM_free_memsize += size * typesize;
}

/**
 * @brief Increments the reuse counter.
 */
void
SAC_PF_MEM_ReuseMemcnt ()
{
    SAC_PF_MEM_reuse_memcnt += 1;
}

/**
 * @brief Call to print memory profiling statistics.
 */
void
SAC_PF_MEM_PrintStats ()
{
    SAC_PF_PrintHeader ("Memory Profile");

    fprintf (stderr, "\n*** %-72s\n", "Memory operation counters:");
    SAC_PF_PrintSection ("For Arrays");
    SAC_PF_PrintCount ("no. calls to (m)alloc", "", SAC_PF_MEM_alloc_memcnt);
    SAC_PF_PrintCount ("no. calls to free", "", SAC_PF_MEM_free_memcnt);
    SAC_PF_PrintCount ("no. reuses of memory", "", SAC_PF_MEM_reuse_memcnt);
    SAC_PF_PrintSection ("For Descriptors");
    SAC_PF_PrintCount ("no. calls to (m)alloc", "", SAC_PF_MEM_alloc_descnt);
    SAC_PF_PrintCount ("no. calls to free", "", SAC_PF_MEM_free_descnt);

    fprintf (stderr, "\n*** %-72s\n", "Memory usage counters:");
    SAC_PF_PrintSize ("total size of memory allocated", "", SAC_PF_MEM_alloc_memsize/8, "bytes");
    SAC_PF_PrintSize ("total size of memory freed", "", SAC_PF_MEM_free_memsize/8, "bytes");
}
