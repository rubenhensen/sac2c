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
#ifndef COMPILE_TRACE
#define COMPILE_TRACE 0
#endif /* !defined(COMPILE_TRACE) */

#if COMPILE_TRACE
#define SAC_DO_TRACE 1
#define SAC_DO_TRACE_DISTMEM 1
#else /* COMPILE_TRACE */
#define SAC_DO_TRACE 0
#define SAC_DO_TRACE_DISTMEM 0
#endif /* COMPILE_TRACE */
#define SAC_DO_DISTMEM 1

#include "sac.h"

#undef SAC_DO_TRACE
#undef SAC_DO_TRACE_DISTMEM
#undef SAC_DO_DISTMEM

#include "distmem_commlib.h"

/*
 * Silence empty source file warning.
 */
static UNUSED int SAC_DISTMEM_COMMLIB_dummy;

#if !COMPILE_TRACE
/*
 * If we compile for distmem_trace.o, we don't need the global variables since
 * these always remain in distmem.o (without tracing).
 */

/*
 * Rank of this node
 * For tracing purposes we also need this when the distributed memory backend
 * is disabled. Therefore, it is also included into libsacdistmem.nodistmem
 */
size_t SAC_DISTMEM_rank = SAC_DISTMEM_RANK_UNDEFINED;
#endif

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

#if COMPILE_TRACE

#define SAC_DISTMEM_TR_RECORD_SEGFAULT() SAC_DISTMEM_TR_num_segfaults++;

#else /* COMPILE_TRACE */

/* Dummy definitions */

#define SAC_DISTMEM_TR_RECORD_SEGFAULT()

#endif /* COMPILE_TRACE*/

#if !COMPILE_TRACE
/*
 * If we compile for distmem_trace.o, we don't need the global variables since
 * these always remain in distmem.o (without tracing).
 */

/* Rank of this node */
size_t SAC_DISTMEM_rank;

/* Number of nodes */
size_t SAC_DISTMEM_size;

/* System page size */
size_t SAC_DISTMEM_pagesz;

/* Segment size (applies to local shared segment and local
 * caches for segments owned by other nodes) */
size_t SAC_DISTMEM_segsz;

/* Pointer to start of local shared segment */
void *SAC_DISTMEM_shared_seg_ptr;

/* Pointer to start of local caches */
void *SAC_DISTMEM_cache_ptr;

/* Pointers to the local shared segment (at index SAC_DISTMEM_rank)
 * and to the local caches of segments owned by other nodes
 * (at other indices) */
void **SAC_DISTMEM_local_seg_ptrs;

/* Offset to free memory within shared segment */
size_t SAC_DISTMEM_seg_free_offs = 0;

#endif /* !COMPILE_TRACE */

#if COMPILE_TRACE
/* Variables used for tracing */

static unsigned long SAC_DISTMEM_TR_num_inval_pages = 0;

static unsigned long SAC_DISTMEM_TR_num_segfaults = 0;

/* Used in the header file so this cannot be static. */
unsigned long SAC_DISTMEM_TR_num_ptr_calcs = 0;

#endif /* COMPILE_TRACE*/

// TODO: Add methods for write protection. This may be useful in detecting errors
// because after loading data nodes should not write into cache segments.

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
      = (uintptr_t)si->si_addr % (uintptr_t)SAC_DISTMEM_cache_ptr / SAC_DISTMEM_segsz;
    /* Calculate the index of the page within the owner's segment. */
    size_t remote_page_index = remote_offset / SAC_DISTMEM_pagesz;

    /* Pointer to the page on which the segfault occured. */
    void *local_page_ptr
      = (void *)((uintptr_t)si->si_addr
                 - (uintptr_t)si->si_addr % (uintptr_t)SAC_DISTMEM_pagesz);

    /* Make the page writable so that it can be loaded from its owner. */
    SAC_DISTMEM_PROT_PAGE_WRITE (local_page_ptr);

    SAC_TR_DISTMEM_PRINT ("Fetching page %zd from %zd to %p", remote_page_index,
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
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_Init (int argc, char *argv[])
#endif /* COMPILE_TRACE */
{
    SAC_DISTMEM_COMMLIB_INIT (argc, argv);
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_Setup (size_t maxmem_mb)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_Setup (size_t maxmem_mb)
#endif /* COMPILE_TRACE */
{
    size_t i;

    /* Query system page size */
    int pagesz = sysconf (_SC_PAGE_SIZE);
    if (pagesz == -1) {
        SAC_RuntimeError ("Querying system page size failed.");
    }
    SAC_DISTMEM_pagesz = pagesz;

    /* The memory to be used for the dsm system has to be a multiple of the
     * system page size. */
    size_t maxmem = maxmem_mb * 1024 * 1024 / SAC_DISTMEM_pagesz * SAC_DISTMEM_pagesz;

    SAC_TR_DISTMEM_PRINT ("Setting up communication library.");

    SAC_DISTMEM_COMMLIB_SETUP (maxmem);

    SAC_TR_DISTMEM_PRINT ("Size: %d", SAC_DISTMEM_size);

    /* Initalize the offset to the free memory in the shared segment. */
    SAC_DISTMEM_seg_free_offs = 0;

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
        SAC_TR_DISTMEM_PRINT ("\tSegment of %zd: %p\n", i, SAC_DISTMEM_local_seg_ptrs[i]);
    }
    SAC_DISTMEM_local_seg_ptrs[SAC_DISTMEM_rank] = SAC_DISTMEM_shared_seg_ptr;
    SAC_TR_DISTMEM_PRINT ("\tSegment of %zd: %p\n", SAC_DISTMEM_rank,
                          SAC_DISTMEM_local_seg_ptrs[SAC_DISTMEM_rank]);
    for (i = SAC_DISTMEM_rank + 1; i < SAC_DISTMEM_size; i++) {
        SAC_DISTMEM_local_seg_ptrs[i]
          = (void *)((uintptr_t)SAC_DISTMEM_cache_ptr + SAC_DISTMEM_segsz * (i - 1));
        SAC_TR_DISTMEM_PRINT ("\tSegment of %zd: %p\n", i, SAC_DISTMEM_local_seg_ptrs[i]);
    }
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_InvalEntireCache (void)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_InvalEntireCache (void)
#endif /* COMPILE_TRACE */
{
    uintptr_t size = (SAC_DISTMEM_size - 1) * SAC_DISTMEM_segsz;
    SAC_TR_DISTMEM_PRINT ("Invalidating entire cache (%" PRIuPTR " B from %p).", size,
                          SAC_DISTMEM_cache_ptr);
    SAC_DISTMEM_PROT_NONE (SAC_DISTMEM_cache_ptr, size);
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_InvalCache (uintptr_t arr_offset, size_t b)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_InvalCache (uintptr_t arr_offset, size_t b)
#endif /* COMPILE_TRACE */
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

#if COMPILE_TRACE
        SAC_DISTMEM_TR_num_inval_pages += num_pages;
#endif
    }
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_Barrier (void)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_Barrier (void)
#endif /* COMPILE_TRACE */
{
    SAC_TR_DISTMEM_PRINT ("Before barrier.");

    SAC_DISTMEM_COMMLIB_BARRIER ();

    SAC_TR_DISTMEM_PRINT ("After barrier.");
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_Exit (void)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_Exit (void)
#endif /* COMPILE_TRACE */
{
    SAC_TR_DISTMEM_PRINT ("Exiting communication library.");

    SAC_TR_DISTMEM_PRINT ("\t Invalidated pages: %lu", SAC_DISTMEM_TR_num_inval_pages);
    SAC_TR_DISTMEM_PRINT ("\t Seg faults: %lu", SAC_DISTMEM_TR_num_segfaults);
    SAC_TR_DISTMEM_PRINT ("\t Pointer calculations: %lu", SAC_DISTMEM_TR_num_ptr_calcs);

    SAC_DISTMEM_COMMLIB_EXIT ();
}

#if COMPILE_TRACE
bool
SAC_DISTMEM_TR_DetDoDistrArr (size_t total_elems, size_t dim0_size)
#else  /* COMPILE_TRACE */
bool
SAC_DISTMEM_DetDoDistrArr (size_t total_elems, size_t dim0_size)
#endif /* COMPILE_TRACE */
{
    bool do_dist = TRUE;

    if (dim0_size < SAC_DISTMEM_size * (SAC_DISTMEM_size - 1)
        && dim0_size % SAC_DISTMEM_size != 0) {
        /* The distribution method does not work because then some nodes would not receive
         * any elements. */
        do_dist = FALSE;
    } else {
        /* The last node owns the least elements. */
        size_t min_elems
          = total_elems
            - (SAC_DISTMEM_size - 1)
                * SAC_DISTMEM_DET_MAX_ELEMS_PER_NODE (total_elems, dim0_size);

        if (min_elems < SAC_DISTMEM_MIN_ELEMS_PER_NODE) {
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
#else  /* COMPILE_TRACE */
size_t
SAC_DISTMEM_DetMaxElemsPerNode (size_t total_elems, size_t dim0_size)
#endif /* COMPILE_TRACE */
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
#else  /* COMPILE_TRACE */
size_t
SAC_DISTMEM_DetDim0Start (size_t dim0_size, size_t start_range, size_t stop_range)
#endif /* COMPILE_TRACE */
{
    size_t max_dim0 = DetMaxDim0SharePerNode (dim0_size);
    size_t start_owned = max_dim0 * SAC_DISTMEM_rank;
    size_t stop_owned = SAC_MIN (start_owned + max_dim0, dim0_size);

    size_t start = SAC_MAX (start_owned, start_range);

    SAC_TR_DISTMEM_PRINT ("Starts at dim0 = %zd (size of dim0: %zd)", start, dim0_size);

    return start;
}

#if COMPILE_TRACE
size_t
SAC_DISTMEM_TR_DetDim0Stop (size_t dim0_size, size_t start_range, size_t stop_range)
#else  /* COMPILE_TRACE */
size_t
SAC_DISTMEM_DetDim0Stop (size_t dim0_size, size_t start_range, size_t stop_range)
#endif /* COMPILE_TRACE */
{
    size_t max_dim0 = DetMaxDim0SharePerNode (dim0_size);
    size_t start_owned = max_dim0 * SAC_DISTMEM_rank;
    size_t stop_owned = SAC_MIN (start_owned + max_dim0, dim0_size);

    size_t stop = SAC_MIN (stop_owned, stop_range);

    SAC_TR_DISTMEM_PRINT ("Stops at dim0 = %zd (size of dim0: %zd)", stop, dim0_size);

    return stop;
}

#if COMPILE_TRACE
void *
SAC_DISTMEM_TR_Malloc (size_t b, uintptr_t *offset)
#else  /* COMPILE_TRACE */
void *
SAC_DISTMEM_Malloc (size_t b, uintptr_t *offset)
#endif /* COMPILE_TRACE */
{
    *offset = SAC_DISTMEM_seg_free_offs;
    SAC_DISTMEM_seg_free_offs += b;

    if (SAC_DISTMEM_seg_free_offs > SAC_DISTMEM_segsz) {
        SAC_RuntimeError ("Out of memory: DSM segment size exceeded.");
    }

    SAC_TR_DISTMEM_PRINT ("Allocated %zd B at offset %" PRIuPTR " in shared segment.", b,
                          *offset);
    return (void *)(((uintptr_t)SAC_DISTMEM_shared_seg_ptr) + *offset);
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_IncNumPtrCalcs (void)
{
    SAC_DISTMEM_TR_num_ptr_calcs++;
}
#endif /* COMPILE_TRACE */

#endif /* ENABLE_DISTMEM */

#endif /* defined(COMPILE_DISTMEM) */
