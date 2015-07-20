/*****************************************************************************
 *
 * file:   distmem_commlib_armci.c
 *
 * prefix: SAC_DISTMEM_COMMLIB
 *
 * description: This is the ARMCI specific implementation of distmem_commlib.h.
 *
 *****************************************************************************/

#include "config.h"

/* By default, we do not use tracing. */
#ifndef COMPILE_TRACE
#define COMPILE_TRACE 0
#endif /* !defined(COMPILE_TRACE) */

/* Silence empty source file warning */
static UNUSED int SAC_DISTMEM_COMMLIB_ARMCI_dummy;

#ifdef COMPILE_DISTMEM_ARMCI

#if ENABLE_DISTMEM_ARMCI

/* For ARMCI */
#include "armci.h"
#include "message.h"
/* For mmap */
#include <sys/mman.h>
/* For errno */
#include <errno.h>

#if COMPILE_TRACE
#define SAC_DO_TRACE 1
#define SAC_DO_TRACE_DISTMEM 1
#else /* COMPILE_TRACE */
#define SAC_DO_TRACE 0
#define SAC_DO_TRACE_DISTMEM 0
#endif /* COMPILE_TRACE */
#define SAC_DO_DISTMEM 1
#define SAC_DO_DISTMEM_ARMCI 1

#include "sac.h"

#undef SAC_DO_TRACE
#undef SAC_DO_TRACE_DISTMEM
#undef SAC_DO_DISTMEM
#undef SAC_DO_DISTMEM_ARMCI

#include "distmem_commlib.h"

/*
 * Takes care of error handling for ARMCI functions that
 * return a success value.
 */
#define ARMCI_SAFE(fncall)                                                               \
    {                                                                                    \
        int retval;                                                                      \
        if ((retval = fncall) != 0) {                                                    \
            SAC_RuntimeError ("Error during ARMCI call from: %s\n"                       \
                              " at: %s:%i\n"                                             \
                              " error: %d \n",                                           \
                              #fncall, __FILE__, __LINE__, retval);                      \
        }                                                                                \
    }

void **armci_seg_ptrs;

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Init (int argc, char *argv[])
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Init (int argc, char *argv[])
#endif /* COMPILE_TRACE */
{
    /* Initialise Message Passing library. */
    armci_msg_init (&argc, &argv);

    /* Initialise ARMCI. */
    ARMCI_Init ();

    SAC_DISTMEM_rank = armci_msg_me ();
    SAC_DISTMEM_size = armci_msg_nproc ();
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Setup (size_t maxmem, bool alloc_cache_outside_dsm)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Setup (size_t maxmem, bool alloc_cache_outside_dsm)
#endif /* COMPILE_TRACE */
{
    /* When ARMCI is used as a communication library, the cache is always allocated
     * outside of the DSM segment. We can, therefore, use the maximum amount of memory
     * for the shared segment. */
    SAC_DISTMEM_segsz = maxmem;

    /*
     * Divide by and multiply with the page size because the segment size has to be a
     * multiple of the page size.
     */
    SAC_DISTMEM_segsz = SAC_DISTMEM_segsz / SAC_DISTMEM_pagesz * SAC_DISTMEM_pagesz;

    armci_seg_ptrs = (void **)malloc (SAC_DISTMEM_size * sizeof (int *));
    if (armci_seg_ptrs == NULL) {
        SAC_RuntimeError ("Error during malloc for armci_seg_ptrs: %d", errno);
    }

    ARMCI_SAFE (ARMCI_Malloc (armci_seg_ptrs, SAC_DISTMEM_segsz));
    SAC_DISTMEM_shared_seg_ptr = armci_seg_ptrs[SAC_DISTMEM_rank];

    if ((SAC_DISTMEM_cache_ptr = mmap (NULL, (SAC_DISTMEM_size - 1) * SAC_DISTMEM_segsz,
                                       PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0))
        == (void *)-1) {
        SAC_RuntimeError ("Error during mmap of cache: %d", errno);
    }
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Barrier ()
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Barrier ()
#endif /* COMPILE_TRACE */
{
    armci_msg_barrier ();
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Exit (int exit_code)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Exit (int exit_code)
#endif /* COMPILE_TRACE */
{
    ARMCI_Finalize ();
    armci_msg_finalize ();
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Abort (int exit_code)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Abort (int exit_code)
#endif /* COMPILE_TRACE */
{
    ARMCI_Error ("An ARMCI error has occured.", exit_code);
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_LoadPage (void *local_page_ptr, size_t owner_rank,
                                 size_t remote_page_index)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_LoadPage (void *local_page_ptr, size_t owner_rank,
                              size_t remote_page_index)
#endif /* COMPILE_TRACE */
{
    SAC_TR_DISTMEM_PRINT ("ARMCI get from node %d, offset: %zd.", owner_rank,
                          remote_page_index * SAC_DISTMEM_pagesz);
    ARMCI_SAFE (ARMCI_Get ((void *)((uintptr_t)armci_seg_ptrs[owner_rank]
                                    + remote_page_index * SAC_DISTMEM_pagesz),
                           local_page_ptr, SAC_DISTMEM_pagesz, owner_rank));
}

#endif /* ENABLE_DISTMEM_ARMCI */

#endif /* defined(COMPILE_DISTMEM_ARMCI) */
