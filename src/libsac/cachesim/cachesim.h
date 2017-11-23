/*****************************************************************************
 *
 * file:   cachesim.h
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

#ifndef _SAC_CS_CACHESIM_H_
#define _SAC_CS_CACHESIM_H_

#define ULINT unsigned long int

#define CNT_T unsigned long long int
/*
 * Note:
 *
 * The long long data type is not covered by the ANSI standard.
 * However, it is supported both under Solaris as well as Linux.
 *
 * So, we use it here because the regular long int is only 32 bits
 * on both systems which is insufficient for several examples.
 */

#define MAX_SHADOWARRAYS 100
#define MAX_CACHELEVEL 3

typedef enum eWritePolicy {
    SAC_CS_default,
    SAC_CS_fetch_on_write,
    SAC_CS_write_validate,
    SAC_CS_write_around
} tWritePolicy;

typedef enum eProfilingLevel {
    SAC_CS_none,
    SAC_CS_file,
    SAC_CS_simple,
    SAC_CS_advanced,
    SAC_CS_piped_simple,
    SAC_CS_piped_advanced
} tProfilingLevel;

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

#endif /* _SAC_CS_CACHESIM_H_ */

