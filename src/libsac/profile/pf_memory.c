/**
 * This file specifies a set of functions used by profile.c to profile memory
 * operations, specifically count and size(s).
 */

#include <stdio.h>

#include "pf_memory.h"

#include "libsac/essentials/profile.h"

/* Global Variables */
static unsigned long SAC_PF_MEM_alloc_memsize = 0; /**< Holds the total size
                                                    * of memory allocated
                                                    */
static unsigned long SAC_PF_MEM_free_memsize = 0; /**< Holds the total size of
                                                   * memory freed
                                                   */
static unsigned long SAC_PF_MEM_alloc_memcnt = 0; /**< Holds the count of calls to malloc/calloc/realloc */
static unsigned long SAC_PF_MEM_free_memcnt = 0; /**< Holds the count of calls to free */
static unsigned long SAC_PF_MEM_reuse_memcnt = 0; /**< Holds the count of dynamic reuses */

/**
 * @brief Increments the alloc counter.
 */
void
SAC_PF_MEM_AllocMemcnt (int size)
{
    SAC_PF_MEM_alloc_memcnt += 1;
    SAC_PF_MEM_alloc_memsize += size;
}

/**
 * @brief Increments the free counter.
 */
void
SAC_PF_MEM_FreeMemcnt (int size)
{
    SAC_PF_MEM_free_memcnt += 1;
    SAC_PF_MEM_free_memsize += size;
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

    fprintf (stderr, "%%%%%% %-72s\n", "Memory operation counters:");
    SAC_PF_PrintCount ("Calls to (m)alloc", "", SAC_PF_MEM_alloc_memcnt);
    SAC_PF_PrintCount ("Calls to free", "", SAC_PF_MEM_free_memcnt);
    SAC_PF_PrintCount ("Number of reuses", "", SAC_PF_MEM_reuse_memcnt);

    fprintf (stderr, "\n%%%%%% %-72s\n", "Memory usage counters:");
    SAC_PF_PrintCount ("Total size of memory allocated", "", SAC_PF_MEM_alloc_memsize);
    SAC_PF_PrintCount ("Total size of memory freed", "", SAC_PF_MEM_free_memsize);
    SAC_PF_PrintCount ("--> Difference", "", SAC_PF_MEM_alloc_memsize - SAC_PF_MEM_free_memsize);
}
