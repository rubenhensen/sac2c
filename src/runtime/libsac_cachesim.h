/*
 * $Log$
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
 * prefix:
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

#endif /* LIBSAC_CACHESIM_H */
