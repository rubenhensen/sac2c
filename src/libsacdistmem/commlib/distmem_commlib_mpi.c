/*****************************************************************************
 *
 * file:   distmem_commlib_mpi.c
 *
 * prefix: SAC_DISTMEM_COMMLIB
 *
 * description:
 *
 *****************************************************************************/

#include "config.h"

/* By default, we do not use tracing. */
#ifndef COMPILE_TRACE
#define COMPILE_TRACE 0
#endif /* !defined(COMPILE_TRACE) */

/* Silence empty source file warning */
static UNUSED int SAC_DISTMEM_COMMLIB_MPI_dummy;

#ifdef COMPILE_DISTMEM_MPI

// TODO: This is temporary until we actually check the availability.
#define ENABLE_DISTMEM_MPI 1

#if ENABLE_DISTMEM_MPI

#include <mpi.h>

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

#define MPI_SAFE(fncall)                                                                 \
    {                                                                                    \
        int retval;                                                                      \
        int len;                                                                         \
        char *str = NULL;                                                                \
        if ((retval = fncall) != MPI_SUCCESS) {                                          \
            MPI_Error_string (retval, str, &len);                                        \
            SAC_RuntimeError ("Error calling: %s\n"                                      \
                              " at: %s:%i\n"                                             \
                              " error: %s \n",                                           \
                              #fncall, __FILE__, __LINE__, str);                         \
            SAC_DISTMEM_Exit ();                                                         \
        }                                                                                \
    }

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Init (int argc, char *argv[])
{
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Init (int argc, char *argv[])
{
#endif /* COMPILE_TRACE */
    MPI_SAFE (MPI_Init (&argc, &argv));
    MPI_SAFE (MPI_Comm_size (MPI_COMM_WORLD, &SAC_DISTMEM_size));
    MPI_SAFE (MPI_Comm_rank (MPI_COMM_WORLD, &SAC_DISTMEM_rank));
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Barrier ()
{
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Barrier ()
{
#endif /* COMPILE_TRACE */
    MPI_SAFE (MPI_Barrier (MPI_COMM_WORLD));
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Exit ()
{
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Exit ()
{
#endif /* COMPILE_TRACE */
    MPI_Finalize ();
}

#endif /* ENABLE_DISTMEM_MPI */

#endif /* defined(COMPILE_DISTMEM_MPI) */
