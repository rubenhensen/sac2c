/*
 *
 * $Log$
 * Revision 1.2  1998/05/07 08:17:51  cg
 * SAC header files converted to new naming conventions.
 *
 * Revision 1.1  1998/03/19 16:53:00  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_malloc.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides facilities for dynamically allocating and de-allocating
 *   memory.
 *
 *   Currently, the standard functions malloc() and free() are used for
 *   heap management. By the global switch MALLOCCHECK the libsac function
 *   SAC_MallocCheck() may be used instead of malloc(). This function
 *   in turn uses malloc() for storage allocation, but additionally checks
 *   for success and terminates program execution with an error message
 *   in the case of insufficient memory available.
 *
 *****************************************************************************/

#ifndef SAC_MALLOC_H

#define SAC_MALLOC_H

#if SAC_DO_MALLOCCHECK

extern void *SAC_MallocCheck (unsigned int);

#define SAC_MALLOC(size) SAC_MallocCheck (size)

#else /* SAC_DO_MALLOCCHECK */

extern void *malloc (unsigned int);

#define SAC_MALLOC(size) malloc (size)

#endif /* SAC_DO_MALLOCCHECK */

extern void free (void *);

#define SAC_FREE(pointer) free (pointer)

#endif /* SAC_MALLOC_H */
