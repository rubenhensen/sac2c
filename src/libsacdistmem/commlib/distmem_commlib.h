/*****************************************************************************
 *
 * file:   distmem_commlib.h
 *
 * prefix: SAC_DISTMEM_COMMLIB_ (no tracing) / SAC_DISTMEM_COMMLIB_TR_ (tracing)
 *
 * description:
 *
 *****************************************************************************/

/* No tracing */

void SAC_DISTMEM_COMMLIB_Init (int argc, char *argv[]);

void SAC_DISTMEM_COMMLIB_Barrier ();

void SAC_DISTMEM_COMMLIB_Exit ();

/* Tracing */

void SAC_DISTMEM_COMMLIB_TR_Init (int argc, char *argv[]);

void SAC_DISTMEM_COMMLIB_TR_Barrier ();

void SAC_DISTMEM_COMMLIB_TR_Exit ();

/* Macros to hide function calls */
#if COMPILE_TRACE

#define SAC_DISTMEM_COMMLIB_INIT(argc, argv) SAC_DISTMEM_COMMLIB_TR_Init (argc, argv);

#define SAC_DISTMEM_COMMLIB_BARRIER() SAC_DISTMEM_COMMLIB_TR_Barrier ();

#define SAC_DISTMEM_COMMLIB_EXIT() SAC_DISTMEM_COMMLIB_TR_Exit ();

#else /* COMPILE_TRACE */

#define SAC_DISTMEM_COMMLIB_INIT(argc, argv) SAC_DISTMEM_COMMLIB_Init (argc, argv);

#define SAC_DISTMEM_COMMLIB_BARRIER() SAC_DISTMEM_COMMLIB_Barrier ();

#define SAC_DISTMEM_COMMLIB_EXIT() SAC_DISTMEM_COMMLIB_Exit ();

#endif /* COMPILE_TRACE */
