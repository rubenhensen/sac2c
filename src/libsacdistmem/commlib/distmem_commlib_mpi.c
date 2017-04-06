/*****************************************************************************
 *
 * file:   distmem_commlib_mpi.c
 *
 * prefix: SAC_DISTMEM_COMMLIB
 *
 * description: This is the MPI specific implementation of distmem_commlib.h.
 *
 * We make use of MPI-3 one-sided communication facilities.
 * See https://cvw.cac.cornell.edu/MPIoneSided/ for a tutorial.
 *
 *****************************************************************************/

#include "fun-attrs.h"

/* By default, we do not use tracing. */
#ifndef COMPILE_TRACE
#define COMPILE_TRACE 0
#endif /* !defined(COMPILE_TRACE) */

/* Silence empty source file warning */
static UNUSED int SAC_DISTMEM_COMMLIB_MPI_dummy;

#ifdef COMPILE_DISTMEM_MPI

#if ENABLE_DISTMEM_MPI

#include <mpi.h>
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
#define SAC_DO_DISTMEM_MPI 1

#include "sac.h"

#undef SAC_DO_TRACE
#undef SAC_DO_TRACE_DISTMEM
#undef SAC_DO_DISTMEM
#undef SAC_DO_DISTMEM_MPI

#include "distmem_commlib.h"

/*
 * Takes care of error handling for MPI functions that
 * return a success value.
 */
#define MPI_SAFE(fncall)                                                                 \
    {                                                                                    \
        int retval, err_cls;                                                             \
        int err_str_len, err_cls_str_len;                                                \
        char *err_str = NULL;                                                            \
        char *err_cls_str = NULL;                                                        \
        if ((retval = fncall) != MPI_SUCCESS) {                                          \
            MPI_Error_class (retval, &err_cls);                                          \
            MPI_Error_string (retval, err_str, &err_str_len);                            \
            MPI_Error_string (err_cls, err_cls_str, &err_cls_str_len);                   \
            SAC_RuntimeError ("Error during MPI call from: %s\n"                         \
                              " at: %s:%i\n"                                             \
                              " error class: %d (%s) \n",                                \
                              " error: %d (%s)", #fncall, __FILE__, __LINE__, err_cls,   \
                              err_cls_str, retval, err_str);                             \
        }                                                                                \
    }

static MPI_Errhandler mpi_err_handler;
static MPI_Comm comm = MPI_COMM_WORLD;
static MPI_Win win;

/*
 * Custom MPI error handler.
 */
static void
handle_mpi_error (MPI_Comm *comm, int *err_code, ...)
{
    int err_cls;
    int err_str_len, err_cls_str_len;
    char *err_str = NULL;
    char *err_cls_str = NULL;

    MPI_Error_class (*err_code, &err_cls);
    MPI_Error_string (*err_code, err_str, &err_str_len);
    MPI_Error_string (err_cls, err_cls_str, &err_cls_str_len);
    SAC_RuntimeError ("MPI error: \n"
                      " error class: %d (%s) \n",
                      " error: %d (%s) \n", err_cls, err_cls_str, *err_code, err_str);
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Init (int argc, char *argv[])
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Init (int argc, char *argv[])
#endif /* COMPILE_TRACE */
{
    MPI_SAFE (MPI_Init (&argc, &argv));

    int size;
    MPI_SAFE (MPI_Comm_size (comm, &size));
    SAC_DISTMEM_size = size;
    int rank;
    MPI_SAFE (MPI_Comm_rank (comm, &rank));
    SAC_DISTMEM_rank = rank;

    MPI_SAFE (MPI_Comm_create_errhandler (handle_mpi_error, &mpi_err_handler));
    MPI_SAFE (MPI_Comm_set_errhandler (comm, mpi_err_handler));
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Setup (size_t maxmem, bool alloc_cache_outside_dsm)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Setup (size_t maxmem, bool alloc_cache_outside_dsm)
#endif /* COMPILE_TRACE */
{
    /* When MPI is used as a communication library, the cache is always allocated
     * outside of the DSM segment. We can, therefore, use the maximum amount of memory
     * for the shared segment. */
    SAC_DISTMEM_segsz = maxmem;

    /*
     * Divide by and multiply with the page size because the segment size has to be a
     * multiple of the page size.
     */
    SAC_DISTMEM_segsz = SAC_DISTMEM_segsz / SAC_DISTMEM_pagesz * SAC_DISTMEM_pagesz;

    if ((SAC_DISTMEM_shared_seg_ptr
         = mmap (NULL, SAC_DISTMEM_segsz, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0))
        == (void *)-1) {
        SAC_RuntimeError ("Error during mmap of shared segment: %d", errno);
    }
    MPI_SAFE (MPI_Win_create (SAC_DISTMEM_shared_seg_ptr, SAC_DISTMEM_segsz, 1,
                              MPI_INFO_NULL, comm, &win));

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
    MPI_SAFE (MPI_Barrier (comm));
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Exit (int exit_code)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Exit (int exit_code)
#endif /* COMPILE_TRACE */
{
    MPI_SAFE (MPI_Win_free (&win));
    MPI_SAFE (MPI_Finalize ());
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Abort (int exit_code)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Abort (int exit_code)
#endif /* COMPILE_TRACE */
{
    MPI_SAFE (MPI_Abort (comm, exit_code));
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
    SAC_TR_DISTMEM_PRINT ("MPI get from node %d, offset: %zd.", owner_rank,
                          remote_page_index * SAC_DISTMEM_pagesz);
    MPI_SAFE (MPI_Win_lock (MPI_LOCK_SHARED, owner_rank, 0, win));
    MPI_SAFE (MPI_Get (local_page_ptr, SAC_DISTMEM_pagesz, MPI_BYTE, owner_rank,
                       remote_page_index * SAC_DISTMEM_pagesz, SAC_DISTMEM_pagesz,
                       MPI_BYTE, win));
    MPI_SAFE (MPI_Win_unlock (owner_rank, win));
}

#endif /* ENABLE_DISTMEM_MPI */

#endif /* defined(COMPILE_DISTMEM_MPI) */
