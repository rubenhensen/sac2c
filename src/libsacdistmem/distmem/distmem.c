/*****************************************************************************
 *
 * file:   distmem.c
 *
 * prefix: SAC_DISTMEM_ (no tracing) / SAC_DISTMEM_TR_ (tracing)
 *
 * description:
 *
 *****************************************************************************/

#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include <inttypes.h>

#include "config.h"

/* By default, we do not use tracing. */
#if !defined(COMPILE_TRACE) && !defined(COMPILE_PROFILE)
#define COMPILE_PLAIN 1
#endif /* !defined(COMPILE_TRACE) */

#if COMPILE_TRACE
#define SAC_DO_TRACE 1
#define SAC_DO_TRACE_DISTMEM 1
#elif COMPILE_PROFILE
#define SAC_DO_PROFILE 1
#define SAC_DO_PROFILE_DISTMEM 1
#endif
#define SAC_DO_DISTMEM 1

#include "sac.h"

#undef SAC_DO_TRACE
#undef SAC_DO_TRACE_DISTMEM
#undef SAC_DO_PROFILE
#undef SAC_DO_PROFILE_DISTMEM
#undef SAC_DO_DISTMEM

#include "distmem_commlib.h"

/*
 * Silence empty source file warning.
 */
static UNUSED int SAC_DISTMEM_COMMLIB_dummy;

#if COMPILE_PLAIN
/*
 * If we compile for distmem_trace.o or distmem_profile.o, we don't need the global
 * variables since these always remain in distmem.o (without tracing and profiling).
 */

/*
 * Rank of this node
 * For tracing purposes we also need this when the distributed memory backend
 * is disabled. Therefore, it is also included into libsacdistmem.nodistmem
 */
size_t SAC_DISTMEM_rank = SAC_DISTMEM_RANK_UNDEFINED;
#endif /* COMPILE_PLAIN */

#ifdef COMPILE_DISTMEM

#if ENABLE_DISTMEM

/*
 * Protects memory starting at page_ptr
 * against read and write accesses.
 */
#define SAC_DISTMEM_PROT_NONE(page_ptr, size)                                            \
    if (mprotect (page_ptr, size, PROT_NONE) == -1) {                                    \
        SAC_RuntimeError ("Failed to protect memory.");                                  \
    }

/*
 * Protects a single memory page starting at page_ptr
 * against read and write accesses.
 */
#define SAC_DISTMEM_PROT_PAGE_NONE(page_ptr)                                             \
    SAC_DISTMEM_PROT_NONE (page_ptr, SAC_DISTMEM_pagesz);

/*
 * Unprotects a single memory page starting at page_ptr
 * (allows reads and writes).
 */
#define SAC_DISTMEM_PROT_PAGE_WRITE(page_ptr)                                            \
    if (mprotect (page_ptr, SAC_DISTMEM_pagesz, PROT_WRITE) == -1) {                     \
        SAC_RuntimeError ("Failed to unprotect memory.");                                \
    }

#if COMPILE_TRACE || COMPILE_PROFILE

#define SAC_DISTMEM_TR_RECORD_SEGFAULT() SAC_DISTMEM_TR_num_segfaults++;

#else /* COMPILE_TRACE || COMPILE_PROFILE */

/* Dummy definitions */

#define SAC_DISTMEM_TR_RECORD_SEGFAULT()

#endif /* COMPILE_TRACE || COMPILE_PROFILE */

#if COMPILE_PLAIN
/*
 * If we compile for distmem_trace.o or distmem_profile.o, we don't need the global
 * variables since these always remain in distmem.o (without tracing and profiling).
 */

/******************************************
 * Global variables
 *******************************************/

/* Rank of this node */
size_t SAC_DISTMEM_rank;

/* Number of nodes */
size_t SAC_DISTMEM_size;

/* System page size */
size_t SAC_DISTMEM_pagesz;

/* Segment size (applies to local shared segment and local
 * caches for segments owned by other nodes) */
uintptr_t SAC_DISTMEM_segsz;

/* Pointer to start of local shared segment */
void *SAC_DISTMEM_shared_seg_ptr;

/* Pointer to start of local caches */
void *SAC_DISTMEM_cache_ptr;

/* Pointers to the local shared segment (at index SAC_DISTMEM_rank)
 * and to the local caches of segments owned by other nodes
 * (at other indices) */
void **SAC_DISTMEM_local_seg_ptrs;

/* Flag that indicates whether writes to distributed arrays are
 * currently allowed. */
bool SAC_DISTMEM_are_dist_writes_allowed = FALSE;

/* Flag that indicates whether writes into the local dsm cache are
 * currently allowed. */
bool SAC_DISTMEM_are_cache_writes_allowed = FALSE;

/* Current execution mode. */
SAC_DISTMEM_exec_mode_t SAC_DISTMEM_exec_mode = SAC_DISTMEM_exec_mode_sync;

/* Minimum number of array elements per node such that an array gets distributed */
size_t SAC_DISTMEM_min_elems_per_node;

/******************************************
 * Global variables used for
 * tracing/profiling
 *******************************************/

/* Number of distributed arrays */
unsigned long SAC_DISTMEM_TR_num_arrays = 0;

/* Number of invalidated pages */
unsigned long SAC_DISTMEM_TR_num_inval_pages = 0;

/* Number of segfaults = page fetches */
unsigned long SAC_DISTMEM_TR_num_segfaults = 0;

/* Number of pointer calculations */
unsigned long SAC_DISTMEM_TR_num_ptr_calcs = 0;

/* Number of barriers */
unsigned long SAC_DISTMEM_TR_num_barriers = 0;

#endif /* COMPILE_PLAIN */

/* Handles seg faults. Copies remote data into local cache. */
static void
SegvHandler (int sig, siginfo_t *si, void *unused)
{
    /* The segfault occured outside of the cache memory area
     * (i.e. was not caused by the dsm system). */
    if ((uintptr_t)si->si_addr < (uintptr_t)SAC_DISTMEM_cache_ptr) {
        SAC_RuntimeError ("DSM segfault at %p", si->si_addr);
    }

    /* Calculate the rank of the owner of the requested memory address. */
    size_t owner_rank
      = ((uintptr_t)si->si_addr - (uintptr_t)SAC_DISTMEM_cache_ptr) / SAC_DISTMEM_segsz;
    /* The local cache segments form a coherent memory area but there is no
     * cache segment for this node. Therefore, add 1 in case the result is
     * greater or equal than the rank of this node.
     */
    if (owner_rank >= SAC_DISTMEM_rank) {
        owner_rank++;
    }

    /* The segfault occured outside of the cache memory area
     * (i.e. was not caused by the dsm system). */
    if (owner_rank >= SAC_DISTMEM_size) {
        SAC_RuntimeError ("DSM segfault at %p", si->si_addr);
    }

    /* Record the segfault only here because we need to be sure
     * that the segfault was caused by the dsm system
     * (i.e. occured within the cache memory area). */
    SAC_DISTMEM_TR_RECORD_SEGFAULT ();

    /* Calculate the offset within the owner's segment. */
    uintptr_t remote_offset
      = ((uintptr_t)si->si_addr - (uintptr_t)SAC_DISTMEM_cache_ptr) % SAC_DISTMEM_segsz;
    /* Calculate the index of the page within the owner's segment. */
    size_t remote_page_index = remote_offset / SAC_DISTMEM_pagesz;

    /* Pointer to the page on which the segfault occured. */
    void *local_page_ptr
      = (void *)((uintptr_t)si->si_addr
                 - (uintptr_t)si->si_addr % (uintptr_t)SAC_DISTMEM_pagesz);

    /* Make the page writable so that it can be loaded from its owner. */
    SAC_DISTMEM_PROT_PAGE_WRITE (local_page_ptr);

    SAC_TR_DISTMEM_PRINT ("Fetching page %zd from node %zd to %p", remote_page_index,
                          owner_rank, local_page_ptr);

    /* Load the page from its owner node. */
    SAC_DISTMEM_COMMLIB_LOAD_PAGE (local_page_ptr, owner_rank, remote_page_index);

    /*
    // TODO: implement this
          MPI_SAFE(MPI_Win_lock(MPI_LOCK_SHARED, node, 0, win));
          MPI_SAFE(MPI_Get(page, pagesize, MPI_BYTE, node, remote_page * pagesize,
    pagesize, MPI_BYTE, win)); MPI_SAFE(MPI_Win_unlock(node, win));

          ARMCI_SAFE(ARMCI_Get((void*)((uintptr_t)getseg(node) + remote_page * pagesize),
    page, pagesize, node));

          const gaspi_queue_id_t queue_id = 0;
          uintptr_t local_offset = (uintptr_t)page - (uintptr_t)cache;
          GPI_SAFE(gaspi_read(get_segid(rank, SEGID_LOCAL_CACHE), local_offset, node,
    get_segid(node, SEGID_SHARED_SEG), remote_page * pagesize, pagesize, queue_id,
    GASPI_BLOCK)); GPI_SAFE(gaspi_wait(queue_id, GASPI_BLOCK));
    */
}

/** <!--********************************************************************-->
 *
 * @fn static size_t DetMaxDim0SharePerNode( size_t dim0_size)
 *
 *   @brief   Determines the maximum share of the array's first dimension per node.
 *
 *   @param dim0_size     size of the array's first dimension
 *   @return              maximum share of the array's first dimension per node
 *
 ******************************************************************************/

static size_t
DetMaxDim0SharePerNode (size_t dim0_size)
{
    if (dim0_size < SAC_DISTMEM_size * (SAC_DISTMEM_size - 1)
        && dim0_size % SAC_DISTMEM_size != 0) {
        /* The distribution method does not work because then some nodes would not receive
         * any elements. */
        SAC_RuntimeError ("The first dimension of the array is too small (%zd) for the "
                          "distribution method (requires at least nodes * (nodes - 1) = "
                          "%zd).",
                          dim0_size, SAC_DISTMEM_size * (SAC_DISTMEM_size - 1));
    }

    return (dim0_size + SAC_DISTMEM_size - 1) / SAC_DISTMEM_size;
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_Init (int argc, char *argv[])
#elif COMPILE_PROFILE
void
SAC_DISTMEM_PR_Init (int argc, char *argv[])
#else /* COMPILE_PLAIN */
void
SAC_DISTMEM_Init (int argc, char *argv[])
#endif
{
    SAC_DISTMEM_COMMLIB_INIT (argc, argv);
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_Setup (size_t maxmem_mb, size_t min_elems_per_node)
#elif COMPILE_PROFILE
void
SAC_DISTMEM_PR_Setup (size_t maxmem_mb, size_t min_elems_per_node)
#else /* COMPILE_PLAIN */
void
SAC_DISTMEM_Setup (size_t maxmem_mb, size_t min_elems_per_node)
#endif
{
    size_t i;

    SAC_DISTMEM_min_elems_per_node = min_elems_per_node;

    /* Query system page size */
    int pagesz = sysconf (_SC_PAGE_SIZE);
    if (pagesz == -1) {
        SAC_RuntimeError ("Querying system page size failed.");
    }
    SAC_DISTMEM_pagesz = pagesz;

    /* The memory to be used for the dsm system has to be a multiple of the
     * system page size. */
    size_t maxmem = maxmem_mb * 1024 * 1024 / SAC_DISTMEM_pagesz * SAC_DISTMEM_pagesz;

    SAC_TR_DISTMEM_PRINT ("Setting up communication library (min. elements/node: %zd).",
                          SAC_DISTMEM_min_elems_per_node);

    SAC_DISTMEM_COMMLIB_SETUP (maxmem);

    SAC_TR_DISTMEM_PRINT ("Size: %d", SAC_DISTMEM_size);

    /* Register seg fault signal handler. */
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset (&sa.sa_mask);
    sa.sa_sigaction = SegvHandler;
    if (sigaction (SIGSEGV, &sa, NULL) == -1) {
        SAC_RuntimeError ("Registering segfault handler for dsm system failed.");
    }

    SAC_DISTMEM_INVAL_ENTIRE_CACHE ();

#if COMPILE_TRACE
    SAC_TR_DISTMEM_PRINT (
      "Allocated memory: %zd MB per segment, %zd MB in total, seg size: %" PRIuPTR
      " B, page size: %zd B",
      SAC_DISTMEM_segsz / 1024 / 1024, SAC_DISTMEM_segsz * SAC_DISTMEM_size / 1024 / 1024,
      SAC_DISTMEM_segsz, SAC_DISTMEM_pagesz);
#endif

    /* Init pointers to local memory segment per node. */
    SAC_DISTMEM_local_seg_ptrs = malloc (SAC_DISTMEM_size * sizeof (void *));
    for (i = 0; i < SAC_DISTMEM_rank; i++) {
        SAC_DISTMEM_local_seg_ptrs[i]
          = (void *)((uintptr_t)SAC_DISTMEM_cache_ptr + SAC_DISTMEM_segsz * i);
        SAC_TR_DISTMEM_PRINT ("   Segment of %zd: %p", i, SAC_DISTMEM_local_seg_ptrs[i]);
    }
    SAC_DISTMEM_local_seg_ptrs[SAC_DISTMEM_rank] = SAC_DISTMEM_shared_seg_ptr;
    SAC_TR_DISTMEM_PRINT ("   Segment of %zd: %p", SAC_DISTMEM_rank,
                          SAC_DISTMEM_local_seg_ptrs[SAC_DISTMEM_rank]);
    for (i = SAC_DISTMEM_rank + 1; i < SAC_DISTMEM_size; i++) {
        SAC_DISTMEM_local_seg_ptrs[i]
          = (void *)((uintptr_t)SAC_DISTMEM_cache_ptr + SAC_DISTMEM_segsz * (i - 1));
        SAC_TR_DISTMEM_PRINT ("   Segment of %zd: %p", i, SAC_DISTMEM_local_seg_ptrs[i]);
    }

    /* Setup of private heap manager. */
    SAC_TR_DISTMEM_PRINT ("Initializing setup of heap manager.");
    SAC_DISTMEM_HM_Setup ();

    SAC_DISTMEM_BARRIER ();
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_InvalEntireCache (void)
#elif COMPILE_PROFILE
void
SAC_DISTMEM_PR_InvalEntireCache (void)
#else /* COMPILE_PLAIN */
void
SAC_DISTMEM_InvalEntireCache (void)
#endif
{
    uintptr_t size = (SAC_DISTMEM_size - 1) * SAC_DISTMEM_segsz;
    SAC_TR_DISTMEM_PRINT ("Invalidating entire cache (%" PRIuPTR " B from %p).", size,
                          SAC_DISTMEM_cache_ptr);
    SAC_DISTMEM_PROT_NONE (SAC_DISTMEM_cache_ptr, size);
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_InvalCache (uintptr_t arr_offset, size_t b)
#elif COMPILE_PROFILE
void
SAC_DISTMEM_PR_InvalCache (uintptr_t arr_offset, size_t b)
#else /* COMPILE_PLAIN */
void
SAC_DISTMEM_InvalCache (uintptr_t arr_offset, size_t b)
#endif
{
    size_t i;

    for (i = 0; i < SAC_DISTMEM_size; i++) {
        if (i == SAC_DISTMEM_rank) {
            continue;
        }

        uintptr_t start = (uintptr_t)SAC_DISTMEM_local_seg_ptrs[i] + arr_offset;
        uintptr_t end = (uintptr_t)start + b;
        void *page_start = (void *)(start - start % SAC_DISTMEM_pagesz);
        size_t num_pages = end / SAC_DISTMEM_pagesz - start / SAC_DISTMEM_pagesz + 1;

        SAC_TR_DISTMEM_PRINT ("Invalidating %zd B = %zd cache pages of node %i from %p.",
                              b, num_pages, i, page_start);
        SAC_DISTMEM_PROT_NONE (page_start, SAC_DISTMEM_pagesz * num_pages);

#if COMPILE_TRACE || COMPILE_PROFILE
        SAC_DISTMEM_TR_num_inval_pages += num_pages;
#endif
    }
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_Barrier (void)
#elif COMPILE_PROFILE
void
SAC_DISTMEM_PR_Barrier (void)
#else /* COMPILE_PLAIN */
void
SAC_DISTMEM_Barrier (void)
#endif
{
    if (SAC_DISTMEM_exec_mode != SAC_DISTMEM_exec_mode_side_effects) {
#if COMPILE_TRACE || COMPILE_PROFILE
        SAC_DISTMEM_TR_num_barriers++;
#endif

        SAC_TR_DISTMEM_PRINT ("Waiting at barrier.");

        SAC_DISTMEM_COMMLIB_BARRIER ();
    } else {
        SAC_TR_DISTMEM_PRINT ("Ignoring barrier in side effects execution mode.");
    }
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_Exit (int exit_code)
#elif COMPILE_PROFILE
void
SAC_DISTMEM_PR_Exit (int exit_code)
#else /* COMPILE_PLAIN */
void
SAC_DISTMEM_Exit (int exit_code)
#endif
{
    /* Print heap manager diagnostics; */
    SAC_DISTMEM_HM_ShowDiagnostics ();

    SAC_TR_DISTMEM_PRINT ("Exiting communication library with exit code %d.", exit_code);

    SAC_TR_DISTMEM_PRINT ("   Distributed arrays: %lu", SAC_DISTMEM_TR_num_arrays);
    SAC_TR_DISTMEM_PRINT ("   Invalidated pages: %lu", SAC_DISTMEM_TR_num_inval_pages);
    SAC_TR_DISTMEM_PRINT ("   Seg faults: %lu", SAC_DISTMEM_TR_num_segfaults);
    SAC_TR_DISTMEM_PRINT ("   Pointer calculations: %lu", SAC_DISTMEM_TR_num_ptr_calcs);
    SAC_TR_DISTMEM_PRINT ("   Barriers: %lu", SAC_DISTMEM_TR_num_barriers);

    SAC_DISTMEM_BARRIER ();
    SAC_DISTMEM_COMMLIB_EXIT (exit_code);
}

#if COMPILE_TRACE
bool
SAC_DISTMEM_TR_DetDoDistrArr (size_t total_elems, size_t dim0_size)
#elif COMPILE_PROFILE
bool
SAC_DISTMEM_PR_DetDoDistrArr (size_t total_elems, size_t dim0_size)
#else /* COMPILE_PLAIN */
bool
SAC_DISTMEM_DetDoDistrArr (size_t total_elems, size_t dim0_size)
#endif
{
    bool do_dist = TRUE;

    if (SAC_DISTMEM_exec_mode != SAC_DISTMEM_exec_mode_sync) {
        SAC_TR_DISTMEM_PRINT ("Array is not distributed because program is not in "
                              "synchronous execution mode.");
        return FALSE;
    }

    /* It can happen that dim0 is 0! */
    if (dim0_size == 0
        || (dim0_size < SAC_DISTMEM_size * (SAC_DISTMEM_size - 1)
            && dim0_size % SAC_DISTMEM_size != 0)) {
        /* The distribution method does not work because then some nodes would not receive
         * any elements. */
        do_dist = FALSE;
    } else {
        /* The last node owns the least elements. */
        size_t min_elems
          = total_elems
            - (SAC_DISTMEM_size - 1)
                * SAC_DISTMEM_DET_MAX_ELEMS_PER_NODE (total_elems, dim0_size);

        if (min_elems < SAC_DISTMEM_min_elems_per_node) {
            do_dist = FALSE;
        }
    }

    SAC_TR_DISTMEM_PRINT ("Distribute array of size %zd (size of dim0: %zd)? %d",
                          total_elems, dim0_size, do_dist);

    return do_dist;
}

#if COMPILE_TRACE
size_t
SAC_DISTMEM_TR_DetMaxElemsPerNode (size_t total_elems, size_t dim0_size)
#elif COMPILE_PROFILE
size_t
SAC_DISTMEM_PR_DetMaxElemsPerNode (size_t total_elems, size_t dim0_size)
#else /* COMPILE_PLAIN */
size_t
SAC_DISTMEM_DetMaxElemsPerNode (size_t total_elems, size_t dim0_size)
#endif
{
    size_t max_dim0 = DetMaxDim0SharePerNode (dim0_size);
    size_t max_elems = max_dim0 * total_elems / dim0_size;

    SAC_TR_DISTMEM_PRINT ("Maximum number of elements/dim0 share per node is %zd/%zd for "
                          "array of size/dim0 %zd/%zd.",
                          max_elems, max_dim0, total_elems, dim0_size);

    return max_elems;
}

#if COMPILE_TRACE
size_t
SAC_DISTMEM_TR_DetDim0Start (size_t dim0_size, size_t start_range, size_t stop_range)
#elif COMPILE_PROFILE
size_t
SAC_DISTMEM_PR_DetDim0Start (size_t dim0_size, size_t start_range, size_t stop_range)
#else /* COMPILE_PLAIN */
size_t
SAC_DISTMEM_DetDim0Start (size_t dim0_size, size_t start_range, size_t stop_range)
#endif
{
    size_t max_dim0 = DetMaxDim0SharePerNode (dim0_size);
    size_t start_owned = max_dim0 * SAC_DISTMEM_rank;

    size_t start = SAC_MAX (start_owned, start_range);

    SAC_TR_DISTMEM_PRINT ("Starts at dim0 = %zd (size of dim0: %zd)", start, dim0_size);

    return start;
}

#if COMPILE_TRACE
size_t
SAC_DISTMEM_TR_DetDim0Stop (size_t dim0_size, size_t start_range, size_t stop_range)
#elif COMPILE_PROFILE
size_t
SAC_DISTMEM_PR_DetDim0Stop (size_t dim0_size, size_t start_range, size_t stop_range)
#else /* COMPILE_PLAIN */
size_t
SAC_DISTMEM_DetDim0Stop (size_t dim0_size, size_t start_range, size_t stop_range)
#endif
{
    size_t max_dim0 = DetMaxDim0SharePerNode (dim0_size);
    size_t start_owned = max_dim0 * SAC_DISTMEM_rank;
    size_t stop_owned = SAC_MIN (start_owned + max_dim0, dim0_size);

    size_t stop = SAC_MIN (stop_owned, stop_range);

    SAC_TR_DISTMEM_PRINT ("Stops at dim0 = %zd (size of dim0: %zd)", stop, dim0_size);

    return stop;
}

#if COMPILE_TRACE
/*
 * This must be a C function because otherwise
 * we get a operation on SAC_DISTMEM_TR_num_ptr_calcs may be
 * undefined [-Wsequence-point] warning.
 */
void
SAC_DISTMEM_TR_IncNumPtrCalcs (void)
{
    SAC_DISTMEM_TR_num_ptr_calcs++;
}

#endif /* COMPILE_TRACE */

#endif /* ENABLE_DISTMEM */

#elif COMPILE_PLAIN /* defined(COMPILE_DISTMEM) */

/* Dummy function for SAC_RuntimeError when the distributed memory backend is not used. */
void
SAC_DISTMEM_Exit (int exit_code)
{
}

#endif /* defined(COMPILE_DISTMEM) */
