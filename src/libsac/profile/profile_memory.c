/**
 * @file
 * @brief This file specifies a set of functions used by profile.c to profile memory
 *        operations, specifically count and size(s).
 */
#include "libsac/profile/profile_memory.h"

#include <stdio.h>

#include "libsac/profile/profile_print.h"
#include "libsac/profile/profile_cuda.h"
#include "runtime/essentials_h/bool.h"

/* Global Variables */

/* these counter are for all memory operations */
static size_t SAC_PF_MEM_alloc_memsize = 0; /**< Holds the total size of memory allocated in bytes */
static size_t SAC_PF_MEM_free_memsize = 0; /**< Holds the total size of memory freed in bytes */
static size_t SAC_PF_MEM_max_alloc = 0; /**< Holds max memory allocated size in bytes */

/* these counters deal with arrays */
static size_t SAC_PF_MEM_alloc_memcnt = 0; /**< Holds the count of calls to malloc/calloc/realloc */
static size_t SAC_PF_MEM_free_memcnt = 0; /**< Holds the count of calls to free */
static size_t SAC_PF_MEM_reuse_memcnt = 0; /**< Holds the count of dynamic reuses */

/* these counters deal with descriptors */
static size_t SAC_PF_MEM_alloc_descnt = 0; /**< Holds the count of descriptor allocations */
static size_t SAC_PF_MEM_free_descnt = 0; /**< Holds the count of descriptor frees */

/**
 * @brief Increments the alloc counter.
 *
 * @param size Size in bytes
 */
void
SAC_PF_MEM_AllocMemcnt (size_t size)
{
    SAC_PF_MEM_alloc_memcnt += 1;
    SAC_PF_MEM_alloc_memsize += size;
}

/**
 * @brief Increments the free counter.
 *
 * @param size Size in bytes
 */
void
SAC_PF_MEM_FreeMemcnt (size_t size)
{
    SAC_PF_MEM_free_memcnt += 1;
    SAC_PF_MEM_free_memsize += size;
}

/**
 * @brief Increments the descriptor alloc counter.
 *
 * @param size Size in bytes
 */
void
SAC_PF_MEM_AllocDescnt (size_t size)
{
    SAC_PF_MEM_alloc_descnt += 1;
    SAC_PF_MEM_alloc_memsize += size;
}

/**
 * @brief Increments the descriptor free counter.
 *
 * @param size Size in bytes
 */
void
SAC_PF_MEM_FreeDescnt (size_t size)
{
    SAC_PF_MEM_free_descnt += 1;
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
 * @brief Adds to the global maximum memory allocated value.
 *
 * @param size Size in bytes
 */
void
SAC_PF_MEM_AddToMax (size_t size)
{
    if (size > SAC_PF_MEM_max_alloc)
        SAC_PF_MEM_max_alloc = size;
}

/**
 * @brief Test if a record's fields are all zero.
 *
 * @param record the record to check
 */
inline bool
SAC_PF_MEM_IsRecordZero (SAC_PF_MEMORY_RECORD record)
{
    return !(record.alloc_mem_count || record.free_mem_count || record.reuse_mem_count
#ifdef SAC_BACKEND_CUDA
             || record.alloc_mem_cuda_count || record.free_mem_cuda_count
             || record.alloc_desc_cuda_count || record.free_desc_cuda_count
#endif /* SAC_BACKEND_CUDA */
             || record.alloc_desc_count || record.free_desc_count);
}

/**
 * @brief Print memory profiling statistics for current record.
 *
 * @param record the record to print
 */
inline void
SAC_PF_MEM_PrintRecordStats (SAC_PF_MEMORY_RECORD record)
{
    fprintf (stderr, "\n+++ %-72s\n", "System Memory:");
    SAC_PF_PrintSection ("For Arrays");
    SAC_PF_PrintCount ("   no. calls to (m)alloc", "", record.alloc_mem_count);
    SAC_PF_PrintCount ("   no. calls to free", "", record.free_mem_count);
    SAC_PF_PrintCount ("   no. reuses of memory", "", record.reuse_mem_count);
    SAC_PF_PrintSection ("For Descriptors");
    SAC_PF_PrintCount ("   no. calls to (m)alloc", "", record.alloc_desc_count);
    SAC_PF_PrintCount ("   no. calls to free", "", record.free_desc_count);
#ifdef SAC_BACKEND_CUDA
    fprintf (stderr, "\n+++ %-72s\n", "CUDA Memory:");
    SAC_PF_PrintSection ("For Arrays");
    SAC_PF_PrintCount ("   no. calls to (m)alloc", "", record.alloc_mem_cuda_count);
    SAC_PF_PrintCount ("   no. calls to free", "", record.free_mem_cuda_count);
    SAC_PF_PrintCount ("   no. reuses of memory", "", 0);
    SAC_PF_PrintSection ("For Descriptors");
    SAC_PF_PrintCount ("   no. calls to (m)alloc", "", record.alloc_desc_cuda_count);
    SAC_PF_PrintCount ("   no. calls to free", "", record.free_desc_cuda_count);
#endif /* SAC_BACKEND_CUDA */
}

/**
 * @brief Print statistics for given function.
 *
 * @param func_name Name of function
 * @param num_ap Number of applications of function
 * @param records The records that store the statistics
 */
void
SAC_PF_MEM_PrintFunStats (const char *func_name, unsigned num_ap,
                          const SAC_PF_MEMORY_RECORD *records)
{
    unsigned i;
    bool zero;

    /* lets check that there is actually something useful to print */
    for (i = 0, zero = true; i < num_ap; i++) {
        zero &= SAC_PF_MEM_IsRecordZero (records[i]);
    }

    /* if at least one record is non-zero */
    if (!zero) {
        SAC_PF_PrintHeader (func_name);
        for (i = 0; i < num_ap; i++) {
            if (!SAC_PF_MEM_IsRecordZero (records[i])) {
                if (num_ap > 1) {
                    fprintf (stderr, "--- Application %d\n", i);
                }
                SAC_PF_MEM_PrintRecordStats (records[i]);
            }
        }
    }
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
    SAC_PF_PrintSize ("total size of memory allocated", "",
                      BYTES_TO_KBYTES (SAC_PF_MEM_alloc_memsize), "Kbytes");
    SAC_PF_PrintSize ("total size of memory freed", "",
                      BYTES_TO_KBYTES (SAC_PF_MEM_free_memsize), "Kbytes");
    SAC_PF_PrintSize ("size of largest allocation", "",
                      BYTES_TO_KBYTES (SAC_PF_MEM_max_alloc), "Kbytes");

#ifdef SAC_BACKEND_CUDA
    SAC_PF_MEM_CUDA_PrintStats ();
#endif /* SAC_BACKEND_CUDA */
}
