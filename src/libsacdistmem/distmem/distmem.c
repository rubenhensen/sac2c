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

#include "config.h"

/* By default, we do not use tracing. */
#ifndef COMPILE_TRACE
#define COMPILE_TRACE 0
#endif /* !defined(COMPILE_TRACE) */

/*
 * Silence empty source file warning.
 */
static UNUSED int SAC_DISTMEM_COMMLIB_dummy;

#ifdef COMPILE_DISTMEM

#if ENABLE_DISTMEM

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
int SAC_DISTMEM_rank;

/* Number of nodes */
int SAC_DISTMEM_size;

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

// static unsigned long SAC_DISTMEM_TR_num_inval_pages = 0;

static unsigned long SAC_DISTMEM_TR_num_segfaults = 0;

// static unsigned long SAC_DISTMEM_TR_num_ptr_calcs = 0;

#endif /* COMPILE_TRACE*/

// TODO: Add methods for write protection. This may be useful in detecting errors
// because after loading data nodes should not write into cache segments.

/* Handles seg faults. Copies remote data into local cache. */
static void
SAC_DISTMEM_segv_handler (int sig, siginfo_t *si, void *unused)
{
    /* The segfault occured outside of the cache memory area
     * (i.e. was not caused by the dsm system). */
    if ((uintptr_t)si->si_addr < (uintptr_t)SAC_DISTMEM_cache_ptr) {
        SAC_RuntimeError ("Segfault at %p", si->si_addr);
    }

    /* Calculate the rank of the owner of the requested memory address. */
    int owner_rank
      = (uintptr_t)si->si_addr - (uintptr_t)SAC_DISTMEM_cache_ptr / SAC_DISTMEM_segsz;
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
        SAC_RuntimeError ("Segfault at %p", si->si_addr);
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
    int i;

    /* Query system page size */
    int pagesz = sysconf (_SC_PAGE_SIZE);
    if (pagesz == -1) {
        SAC_RuntimeError ("Querying system page size failed.");
    }
    SAC_DISTMEM_pagesz = pagesz;

    /* The memory to be used for the dsm system has to be a multiple of the
     * system page size. */
    size_t maxmem = maxmem_mb * 1024 * 1024 / SAC_DISTMEM_pagesz * SAC_DISTMEM_pagesz;

    SAC_TR_DISTMEM_PRINT (("Setting up communication library.\n"));

    SAC_DISTMEM_COMMLIB_SETUP (maxmem);

    SAC_TR_DISTMEM_PRINT (("Rank: %d, size: %d\n", SAC_DISTMEM_rank, SAC_DISTMEM_size));

    /* Initalize the offset to the free memory in the shared segment. */
    SAC_DISTMEM_seg_free_offs = 0;

    /* Register seg fault signal handler. */
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset (&sa.sa_mask);
    sa.sa_sigaction = SAC_DISTMEM_segv_handler;
    if (sigaction (SIGSEGV, &sa, NULL) == -1) {
        SAC_RuntimeError ("Registering segfault handler for dsm system failed.");
    }

    SAC_DISTMEM_INVAL_CACHE ();

#if COMPILE_TRACE
    SAC_TR_DISTMEM_PRINT (("Allocated memory: %zd MB per segment, %zd MB in total",
                           SAC_DISTMEM_segsz / 1024 / 1024,
                           SAC_DISTMEM_segsz * SAC_DISTMEM_size / 1024 / 1024));
#endif

    /* Init pointers to local memory segment per node. */
    SAC_DISTMEM_local_seg_ptrs = malloc (SAC_DISTMEM_size * sizeof (void *));
    for (i = 0; i < SAC_DISTMEM_rank; i++) {
        SAC_DISTMEM_local_seg_ptrs[i]
          = (void *)((uintptr_t)SAC_DISTMEM_cache_ptr + SAC_DISTMEM_segsz * i);
    }
    SAC_DISTMEM_local_seg_ptrs[SAC_DISTMEM_rank] = SAC_DISTMEM_shared_seg_ptr;
    for (i = SAC_DISTMEM_rank + 1; i < SAC_DISTMEM_size; i++) {
        SAC_DISTMEM_local_seg_ptrs[i]
          = (void *)((uintptr_t)SAC_DISTMEM_cache_ptr + SAC_DISTMEM_segsz * (i - 1));
    }
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_InvalCache (void)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_InvalCache (void)
#endif /* COMPILE_TRACE */
{
    /* Invalidate the entire cache. */
    SAC_DISTMEM_PROT_NONE (SAC_DISTMEM_cache_ptr,
                           (SAC_DISTMEM_size - 1) * SAC_DISTMEM_segsz);
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_Barrier (void)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_Barrier (void)
#endif /* COMPILE_TRACE */
{
    SAC_TR_DISTMEM_PRINT (("%d before barrier.\n", SAC_DISTMEM_rank));

    SAC_DISTMEM_COMMLIB_BARRIER ();

    SAC_TR_DISTMEM_PRINT (("%d after barrier.\n", SAC_DISTMEM_rank));
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_Exit (void)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_Exit (void)
#endif /* COMPILE_TRACE */
{
    SAC_TR_DISTMEM_PRINT (("%d exiting communication library.\n", SAC_DISTMEM_rank));

    SAC_DISTMEM_COMMLIB_EXIT ();
}

#if 0

/* Returns the node where the element resides */
/* Static so that compiler knows it is not used in other source file and can be inlined. */
/* This also works if the struct is not passed by reference. */
static inline int *elem_pointer(dsm_arr_t dsm_arr, size_t elem_index, int *recalc_index) {
#ifdef FLAG_STATS
		if (record_stats == 1) {
			num_ptr_calcs++;
		}
#endif

#ifdef FLAG_NO_DIV
		int *pointer = (int *)((uintptr_t)local_seg_ptrs[elem_index / dsm_arr.elems_first_nodes] + dsm_arr.offset) + elem_index % dsm_arr.elems_first_nodes;
		// At which index do we have to recalculate the pointer?
		*recalc_index = elem_index + (dsm_arr.elems_first_nodes - elem_index % dsm_arr.elems_first_nodes);
#else
		/* quot = node, rem = offset within node's array portion */
		ldiv_t q = ldiv(elem_index, dsm_arr.elems_first_nodes);
		int *pointer = (int *)((uintptr_t)local_seg_ptrs[q.quot] + dsm_arr.offset) + q.rem;
		// At which index do we have to recalculate the pointer?
		*recalc_index = elem_index + (dsm_arr.elems_first_nodes - q.rem);
#endif
	
	PRINT_DEBUG("At %d: pointer to elem %zd: %p\n", rank, elem_index, pointer);

	return pointer;
}
#endif

#if 0
/* Invalidates the cache for one array. */
void invalidate_array_cache(dsm_arr_t dsm_arr) {
#if !defined(USE_SEQ) && !defined(USE_SEQIX)
		int i;

		for (i = 0; i < size; i++) {
			if (i == rank) {
				continue;
			}

			uintptr_t start = (uintptr_t)local_seg_ptrs[i] + dsm_arr.offset;
			uintptr_t end = (uintptr_t)start + dsm_arr.element_size * dsm_arr.elems_first_nodes;
			void *page_start = (void*) (start - start % pagesize);
			size_t num_pages = end / pagesize - start / pagesize + 1;

			PRINT_DEBUG("At %d: Invalidating array cache from node %d, %zd pages from %p\n", rank, i, num_pages, page_start);
		
			/* Protect local cache */
			if (mprotect(page_start, pagesize * num_pages, PROT_NONE) == -1) {
				fprintf(stderr, "error mprotect: %d, codes: %d %d %d %d\n", errno, EACCES, EFAULT, EINVAL, ENOMEM);
				handle_error("mprotect");
			}

#ifdef FLAG_STATS
				if (record_stats == 1) {
					num_inval_pages += num_pages;
				}
#endif
		}
#endif
}
#endif

#endif /* ENABLE_DISTMEM */

#endif /* defined(COMPILE_DISTMEM) */
