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

#ifndef LIBSAC_CACHESIM_H
#define LIBSAC_CACHESIM_H

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

extern CNT_T SAC_CS_rhit[MAX_CACHELEVEL + 1], SAC_CS_rinvalid[MAX_CACHELEVEL + 1],
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

#define MAX_TAG_LENGTH 512

#endif /* LIBSAC_CACHESIM_H */
