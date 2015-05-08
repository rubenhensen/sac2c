/*****************************************************************************
 *
 * file:   distmem_commlib.h
 *
 * prefix: SAC_DISTMEM_COMMLIB_ (no tracing) / SAC_DISTMEM_COMMLIB_TR_ (tracing)
 *
 * description: This header file declares the communication library specific
 *              interface of libsacdistmem. For each communication library
 *              an implementation has to be provided. This header file is only
 *              intended for internal use from within libsacdistmem. From
 *              outside use the communication library independend interface
 *              provided by distmem.h. Do not call the functions declared by
 *              this header file directly but use the provided macros to
 *              abstract from the trace settings.
 *
 *****************************************************************************/

#ifndef _SAC_DISTMEM_COMMLIB_H_
#define _SAC_DISTMEM_COMMLIB_H_

/*****************************************************************************/

/******************************************
 * No tracing declarations
 *******************************************/

/*
 * Initializes the communication library.
 *
 * Has to be called before any other operation. The
 * communication library cannot be used after this
 * operation but other operations (e.g. parameter handling)
 * are permitted then.
 */
void SAC_DISTMEM_COMMLIB_Init (int argc, char *argv[]);

/*
 * Sets up the communication library so that it can be used.
 *
 * Before this operation is called the following global
 * variables have to be initialized:
 *   - SAC_DISTMEM_pagesz
 *
 * This operation initializes the following global variables:
 *   - SAC_DISTMEM_rank
 *   - SAC_DISTMEM_size
 *   - SAC_DISTMEM_segsz
 *   - SAC_DISTMEM_shared_seg_ptr
 *   - SAC_DISTMEM_cache_ptr
 */
void SAC_DISTMEM_COMMLIB_Setup (size_t maxmem);

/*
 * Performs a barrier operation.
 */
void SAC_DISTMEM_COMMLIB_Barrier (void);

/*
 * Loads the memory page with index remote_page_index within the shared segment of
 * node owner_rank into the local address local_page_ptr.
 */
void SAC_DISTMEM_COMMLIB_LoadPage (void *local_page_ptr, int owner_rank,
                                   size_t remote_page_index);

/*
 * Shuts down the communication library.
 * Call exit() afterwards but do not rely on
 * it being executed.
 */
void SAC_DISTMEM_COMMLIB_Exit (void);

/******************************************
 * Tracing declarations
 *******************************************/

void SAC_DISTMEM_COMMLIB_TR_Init (int argc, char *argv[]);

void SAC_DISTMEM_COMMLIB_TR_Setup (size_t maxmem);

void SAC_DISTMEM_COMMLIB_TR_Barrier (void);

void SAC_DISTMEM_COMMLIB_TR_LoadPage (void *local_page_ptr, int owner_rank,
                                      size_t remote_page_index);

void SAC_DISTMEM_COMMLIB_TR_Exit (void);

/******************************************
 * Macros to abstract from different
 * function names when tracing is
 * activated/not activated.
 *******************************************/

#if COMPILE_TRACE

#define SAC_DISTMEM_COMMLIB_INIT(argc, argv) SAC_DISTMEM_COMMLIB_TR_Init (argc, argv);

#define SAC_DISTMEM_COMMLIB_SETUP(maxmem) SAC_DISTMEM_COMMLIB_TR_Setup (maxmem);

#define SAC_DISTMEM_COMMLIB_BARRIER() SAC_DISTMEM_COMMLIB_TR_Barrier ();

#define SAC_DISTMEM_COMMLIB_LOAD_PAGE(local_page_ptr, owner_rank, remote_page_index)     \
    SAC_DISTMEM_COMMLIB_TR_LoadPage (local_page_ptr, owner_rank, remote_page_index);

#define SAC_DISTMEM_COMMLIB_EXIT() SAC_DISTMEM_COMMLIB_TR_Exit ();

#else /* COMPILE_TRACE */

#define SAC_DISTMEM_COMMLIB_INIT(argc, argv) SAC_DISTMEM_COMMLIB_Init (argc, argv);

#define SAC_DISTMEM_COMMLIB_SETUP(maxmem) SAC_DISTMEM_COMMLIB_Setup (maxmem);

#define SAC_DISTMEM_COMMLIB_BARRIER() SAC_DISTMEM_COMMLIB_Barrier ();

#define SAC_DISTMEM_COMMLIB_LOAD_PAGE(local_page_ptr, owner_rank, remote_page_index)     \
    SAC_DISTMEM_COMMLIB_LoadPage (local_page_ptr, owner_rank, remote_page_index);

#define SAC_DISTMEM_COMMLIB_EXIT() SAC_DISTMEM_COMMLIB_Exit ();

#endif /* COMPILE_TRACE */

#endif /* _SAC_DISTMEM_COMMLIB_H */
