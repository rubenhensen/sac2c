/**
 * @file
 * @brief This file specifies a set of functions used by profile.c to profile memory
 *        associated with a CUDA device
 */
#include "config.h"
#include "profile_cuda.h"

#include <stdio.h>

#include "libsac/profile/profile_print.h"
#include "runtime/essentials_h/bool.h"

#define __PF_TIMER_FORMAT "%8.2f"
#define __PF_TIMER_PERCENTAGE(timer1, timer2)                                            \
    ((timer2.tv_sec + timer2.tv_usec / 1000000.0) == 0                                   \
       ? 0.0                                                                             \
       : (timer1 / 1000.0) * 100.0                                                       \
           / (timer2.tv_sec + (timer2.tv_usec / 1000000.0)))

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

#if ENABLE_CUDA

#include <cuda_runtime_api.h>

/* global timer struct for CUDA event timer */
cuda_timer_t *cuda_timer_knl = NULL;
cuda_timer_t *cuda_timer_htod = NULL;
cuda_timer_t *cuda_timer_dtoh = NULL;
cuda_timer_t *cuda_timer_hmal = NULL;
cuda_timer_t *cuda_timer_dmal = NULL;
cuda_timer_t *cuda_timer_hfree = NULL;
cuda_timer_t *cuda_timer_dfree = NULL;

/* counter for different CUDA constructs */
static size_t cuda_timer_knl_count = 0;
static size_t cuda_timer_htod_count = 0;
static size_t cuda_timer_dtoh_count = 0;
static size_t cuda_timer_hmal_count = 0;
static size_t cuda_timer_dmal_count = 0;
static size_t cuda_timer_hfree_count = 0;
static size_t cuda_timer_dfree_count = 0;

size_t
SAC_PF_TIMER_CUDA_Get_Counter (cuda_count_t type)
{
    size_t ret = 0;

    switch (type) {
    case pf_cuda_knl:
        ret = cuda_timer_knl_count;
        break;
    case pf_cuda_htod:
        ret = cuda_timer_htod_count;
        break;
    case pf_cuda_dtoh:
        ret = cuda_timer_dtoh_count;
        break;
    case pf_cuda_hmal:
        ret = cuda_timer_hmal_count;
        break;
    case pf_cuda_dmal:
        ret = cuda_timer_dmal_count;
        break;
    case pf_cuda_hfree:
        ret = cuda_timer_hfree_count;
        break;
    case pf_cuda_dfree:
        ret = cuda_timer_dfree_count;
        break;
    default:
        break;
    }

    return ret;
}

void
SAC_PF_TIMER_CUDA_Inc_Counter (cuda_count_t type)
{
    switch (type) {
    case pf_cuda_knl:
        cuda_timer_knl_count++;
        break;
    case pf_cuda_htod:
        cuda_timer_htod_count++;
        break;
    case pf_cuda_dtoh:
        cuda_timer_dtoh_count++;
        break;
    case pf_cuda_hmal:
        cuda_timer_hmal_count++;
        break;
    case pf_cuda_dmal:
        cuda_timer_dmal_count++;
        break;
    case pf_cuda_hfree:
        cuda_timer_hfree_count++;
        break;
    case pf_cuda_dfree:
        cuda_timer_dfree_count++;
        break;
    default:
        break;
    }
}

cuda_timer_t *
SAC_PF_TIMER_CUDA_Add (cuda_count_t type, cuda_timer_t *timer)
{
    if (timer == NULL) { // initilization
        timer = malloc (sizeof (cuda_timer_t));
        timer->next = NULL;
        timer->parent = timer; // we are the parent
    } else {
        timer->next = malloc (sizeof (cuda_timer_t));
        timer->next->next = NULL;
        timer->next->parent = timer->parent;
        timer = timer->next;
    }

    SAC_PF_TIMER_CUDA_Inc_Counter (type);

    cudaEventCreate (&timer->start);
    cudaEventCreate (&timer->stop);

    return timer;
}

void
SAC_PF_TIMER_CUDA_Destroy (cuda_timer_t *t)
{
    cuda_timer_t *timer;
    cuda_timer_t *next;

    if (t != NULL) {
        timer = t->parent;

        while (timer != NULL) {
            next = timer->next;
            cudaEventDestroy (timer->start);
            cudaEventDestroy (timer->stop);
            free (timer);
            timer = next;
        }
    }
}

float
SAC_PF_TIMER_CUDA_Sum (cuda_timer_t *t)
{
    float elapsed = 0, sum = 0;
    cuda_timer_t *timer;

    if (t != NULL) {
        timer = t->parent;

        while (timer != NULL) {
            cudaEventSynchronize (timer->stop);
            cudaEventElapsedTime (&elapsed, timer->start, timer->stop);
            sum += elapsed;
            timer = timer->next;
        }
    }

    return sum;
}

/**
 * @brief Function for printing timing information with a percentage of
 *        total time.
 *
 * @param title Title or label for field
 * @param space Filler to modify the spacing of the string
 * @param time1 Measured time as a time-structure
 * @param time2 Global total measured time, used to compute percentage
 */
void
SAC_PF_TIMER_CUDA_PrintTimePercentage (const char *title, const char *space, cuda_count_t type, const float time1,
                            const struct timeval *time2)
{
    fprintf (stderr,
             "%-40s:%s  %8.2f msec %8.2f %%\n   - count: %-27zu\n",
             title, space, time1,
             __PF_TIMER_PERCENTAGE (time1, (*time2)), SAC_PF_TIMER_CUDA_Get_Counter (type));
}

#endif /* ENABLE_CUDA */

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
