/*
 * $Log$
 * Revision 1.3  1999/02/17 17:14:22  her
 * new parameter for Initialize: profilelevel
 * english comments
 *
 */

/*****************************************************************************
 *
 * file:   sac_cachesim.h
 *
 * prefix: SAC_CS_
 *
 * description:
 *
 *   Deciding if a code-optimization has been successfull or not is often hard
 *   because the hardware caches behave like a black box. There is no way to
 *   get information about hit- and missrates after a run of a special program.
 *
 *   The SAC_CacheSimulation shall help here. It simulates up to three
 *   cachelevels for a single CPU. Because the most used and bigest
 *   datastructures in SAC are arrays the simulated accesses are limmeted to
 *   arrayelements. The SAC_CacheSimulation analyses the accesses and tells the
 *   user about the number of hits and misses. Furthermore the misses are
 *   classified as coldstartmiss, self- or crossinterferencemiss.
 *
 *****************************************************************************/

#ifndef SAC_CACHESIM_H
#define SAC_CACHESIM_H

#define _POSIX_C_SOURCE 199506L

#define ULINT unsigned long int
#define MAX_SHADOWARRAYS 100
#define MAX_CACHELEVEL 3

typedef enum eWritePolicy {
    Default,
    fetch_on_write,
    write_validate,
    write_around
} tWritePolicy;

typedef enum eProfilingLevel { simple, detailed } tProfilingLevel;

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

/******************************************************************************
 *
 * function:
 *   void SAC_CS_Initialize(...)
 *
 * description:
 *
 *   Initiates all neccesary structures according to the specified cache-
 *   parameters.
 *   About specifying the cacheparameters:
 *     profilinglevel decides between a simple or more detailed analysis
 *     cachesize in kilobyte (1kbyte=1024byte)
 *     cachesize==0 means that this cachelevel is not installed
 *     cachelinesize in byte
 *     associativity==1                       -> direct mapped cache
 *     associativity==cachesize/cachelinesize -> full associative cache
 *     writepolicy specifies on of the three writemisspolicies
 *
 *****************************************************************************/
extern void SAC_CS_Initialize (int nr_of_cpu, tProfilingLevel profilinglevel,
                               ULINT cachesize1, int cachelinesize1, int associativity1,
                               tWritePolicy writepolicy1, ULINT cachesize2,
                               int cachelinesize2, int associativity2,
                               tWritePolicy writepolicy2, ULINT cachesize3,
                               int cachelinesize3, int associativity3,
                               tWritePolicy writepolicy3);

/******************************************************************************
 *
 * function:
 *   void SAC_CS_Finalize(void)
 *
 * description:
 *   Frees all the memory which has been allocated during the run.
 *
 *****************************************************************************/
extern void SAC_CS_Finalize (void);

/******************************************************************************
 *
 * function:
 *   void SAC_CS_RegisterArray(void* baseaddress, int size)
 *
 * description:
 *   Prepares an 1-dimensional array for a detailed profilinglevel analysis.
 *   The array will be identified by its baseaddress. Size has to be given in
 *   byte.
 *
 *****************************************************************************/
extern void SAC_CS_RegisterArray (void *baseaddress, int size);

/******************************************************************************
 *
 * function:
 *   void SAC_CS_UnregisterArray(void* baseaddress)
 *
 * description:
 *   Opposite to SAC_CS_RegisterArray. Frees all memory which was used for the
 *   detailed profilinglevel analysis.
 *
 *****************************************************************************/
extern void SAC_CS_UnregisterArray (void *baseaddress);

/******************************************************************************
 *
 * function:
 *   void SAC_CS_ReadAccess(void* baseaddress, void* elemaddress)
 *
 * description:
 *   To simulate the cache every readaccess to an arrayelement has to execute
 *   this function. To identify the array this function gets the baseaddress
 *   of the affected array. The accessed element is given as a full address
 *   (not only the offset to the baseaddress).
 *
 *****************************************************************************/
extern void (*SAC_CS_ReadAccess) (void * /*baseaddress*/, void * /*elemaddress*/);

/******************************************************************************
 *
 * function:
 *   void SAC_CS_WriteAccess(void* baseaddress, void* elemaddress)
 *
 * description:
 *   To simulate the cache every writeaccess to an arrayelement has to execute
 *   this function. To identify the array this function gets the baseaddress
 *   of the affected array. The accessed element is given as a full address
 *   (not only the offset to the baseaddress).
 *
 *****************************************************************************/
extern void (*SAC_CS_WriteAccess) (void * /*baseaddress*/, void * /*elemaddress*/);

/******************************************************************************
 *
 * function:
 *   void SAC_CS_WriteAccess(void* baseaddress, void* elemaddress)
 *
 * description:
 *   Prints the Results of the analysis.
 *   simple & detailed profilinglevel:
 *     hit- and missrate for each cachelevel
 *   detailed profilinglevel only:
 *     classification of misses as coldstart, self- or crossinterference
 *
 *****************************************************************************/
extern void SAC_CS_ShowResults (void);

#endif
