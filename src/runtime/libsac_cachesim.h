/*
 * $Log$
 * Revision 1.5  1999/07/05 11:57:31  her
 * changes to seperate read from writecounters
 *
 * Revision 1.4  1999/05/20 14:16:49  cg
 * added macro MAX_TAG_LENGTH
 *
 * Revision 1.3  1999/05/10 10:55:31  her
 * removed SAC_CS_CheckArguments; was already in sac_cachesim.h
 *
 * Revision 1.2  1999/05/03 11:52:38  her
 * added a forward declaration for SAC_CS_CheckArguments
 *
 * Revision 1.1  1999/04/06 13:41:24  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   libsac_cachesim.h
 *
 * prefix: SAC_CS
 *
 * description:
 *
 *   This header file contains definitions that are used internally within
 *   the implementation of the cache simulation part of the SAC library, but
 *   should be invisible outside and are thus not part of sac_cachesim.h
 *
 *
 *****************************************************************************/

#ifndef LIBSAC_CACHESIM_H
#define LIBSAC_CACHESIM_H

#define _POSIX_C_SOURCE 199506L

#define ULINT unsigned long int
#define MAX_SHADOWARRAYS 100
#define MAX_CACHELEVEL 3

typedef struct sCacheLevel { /* about simulated cache */
    int associativity;
    int cachelinesize;
    tWritePolicy writepolicy;
    ULINT cachesize;
    ULINT internsize; /* cachesize/assoc. */
    int cls_bits;
    ULINT cls_mask;
    int is_bits;
    ULINT is_mask;
    ULINT nr_cachelines;
    ULINT *data;
    /* about shadowarrays */
    char *shadowarrays[MAX_SHADOWARRAYS];
    ULINT shadowbases[MAX_SHADOWARRAYS];
    ULINT shadowalignedtop[MAX_SHADOWARRAYS];
    int shadowmaxindices[MAX_SHADOWARRAYS];
    int shadownrcols[MAX_SHADOWARRAYS];
} tCacheLevel;

typedef void (*tFunRWAccess) (void * /*baseaddress*/, void * /*elemaddress*/);
/* Pointer to a function which gets two void* as argument
 * and returns a void */

/* BEGIN: The folowing variables are declared in the
 * ´libsac_cachesim_basic.c´-file. Changes there have to be done here
 * too!!!  */
extern char SAC_CS_separator[];

extern tCacheLevel *SAC_CS_cachelevel[MAX_CACHELEVEL + 1];
/* SAC_CS_cachelevel[0] is unused */

extern ULINT SAC_CS_rhit[MAX_CACHELEVEL + 1], SAC_CS_rinvalid[MAX_CACHELEVEL + 1],
  SAC_CS_rmiss[MAX_CACHELEVEL + 1], SAC_CS_rcold[MAX_CACHELEVEL + 1],
  SAC_CS_rcross[MAX_CACHELEVEL + 1], SAC_CS_rself[MAX_CACHELEVEL + 1],
  SAC_CS_whit[MAX_CACHELEVEL + 1], SAC_CS_winvalid[MAX_CACHELEVEL + 1],
  SAC_CS_wmiss[MAX_CACHELEVEL + 1], SAC_CS_wcold[MAX_CACHELEVEL + 1],
  SAC_CS_wcross[MAX_CACHELEVEL + 1], SAC_CS_wself[MAX_CACHELEVEL + 1];
/* SAC_CS_xxx[0] is unused */

extern int SAC_CS_level;

extern tFunRWAccess SAC_CS_ReadAccess, SAC_CS_WriteAccess,
  SAC_CS_read_access_table[MAX_CACHELEVEL + 2],
  SAC_CS_write_access_table[MAX_CACHELEVEL + 2];
/* SAC_CS_xxx_access_table[0] is unused,
 * SAC_CS_xxx_access_table[MAX_CACHELEVEL+1]
 *   for dummy/MainMem */
/* END: */

#define MAX_TAG_LENGTH 70

#endif /* LIBSAC_CACHESIM_H */
