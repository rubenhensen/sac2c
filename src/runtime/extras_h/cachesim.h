/*
 * $Id$
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

#ifndef _SAC_CACHESIM_H
#define _SAC_CACHESIM_H

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

#ifndef SAC_SIMD_COMPILATION

#define SAC_CS_NONE 0
#define SAC_CS_FILE 1
#define SAC_CS_SIMPLE 2
#define SAC_CS_ADVANCED 3

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

/******************************************************************************
 *
 * function:
 *   void SAC_CS_CheckArguments(...)
 *
 * description:
 *
 *   checks the command line arguments of the running SAC application for
 *   additional cache specifications that overwrite the default set when
 *   the application was compiled.
 *
 ******************************************************************************/

SAC_C_EXTERN
void SAC_CS_CheckArguments (int argc, char *argv[], tProfilingLevel *profilinglevel,
                            int *cs_global, char **cshost, char **csfile, char **csdir,
                            unsigned long int *cachesize1, int *cachelinesize1,
                            int *associativity1, tWritePolicy *writepolicy1,
                            unsigned long int *cachesize2, int *cachelinesize2,
                            int *associativity2, tWritePolicy *writepolicy2,
                            unsigned long int *cachesize3, int *cachelinesize3,
                            int *associativity3, tWritePolicy *writepolicy3);

/******************************************************************************
 *
 * function:
 *   void SAC_CS_Initialize( ...)
 *
 * description:
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

SAC_C_EXTERN
void SAC_CS_Initialize (int nr_of_cpu, tProfilingLevel profilinglevel, int cs_global,
                        char *cshost, char *csfile, char *csdir,
                        unsigned long int cachesize1, int cachelinesize1,
                        int associativity1, tWritePolicy writepolicy1,
                        unsigned long int cachesize2, int cachelinesize2,
                        int associativity2, tWritePolicy writepolicy2,
                        unsigned long int cachesize3, int cachelinesize3,
                        int associativity3, tWritePolicy writepolicy3);

/******************************************************************************
 *
 * function:
 *   void SAC_CS_Finalize( void)
 *
 * description:
 *   Frees all the memory which has been allocated during the run.
 *
 *****************************************************************************/

SAC_C_EXTERN void (*SAC_CS_Finalize) (void);

/******************************************************************************
 *
 * function:
 *   void SAC_CS_RegisterArray( void* baseaddress, int size)
 *
 * description:
 *   Prepares an 1-dimensional array for a detailed profilinglevel analysis.
 *   The array will be identified by its baseaddress. Size has to be given in
 *   byte.
 *
 *****************************************************************************/

SAC_C_EXTERN void (*SAC_CS_RegisterArray) (void * /*baseaddress*/, int /*size*/);

/******************************************************************************
 *
 * function:
 *   void SAC_CS_UnregisterArray( void* baseaddress)
 *
 * description:
 *   Opposite to SAC_CS_RegisterArray. Frees all memory which was used for the
 *   detailed profilinglevel analysis.
 *
 *****************************************************************************/

SAC_C_EXTERN void (*SAC_CS_UnregisterArray) (void * /*baseaddress*/);

/******************************************************************************
 *
 * function:
 *   void SAC_CS_ReadAccess( void* baseaddress, void* elemaddress)
 *
 * description:
 *   To simulate the cache every readaccess to an arrayelement has to execute
 *   this function. To identify the array this function gets the baseaddress
 *   of the affected array. The accessed element is given as a full address
 *   (not only the offset to the baseaddress).
 *
 *****************************************************************************/

SAC_C_EXTERN void (*SAC_CS_ReadAccess) (void * /*baseaddress*/, void * /*elemaddress*/);

/******************************************************************************
 *
 * function:
 *   void SAC_CS_WriteAccess( void* baseaddress, void* elemaddress)
 *
 * description:
 *   To simulate the cache every writeaccess to an arrayelement has to execute
 *   this function. To identify the array this function gets the baseaddress
 *   of the affected array. The accessed element is given as a full address
 *   (not only the offset to the baseaddress).
 *
 *****************************************************************************/

SAC_C_EXTERN void (*SAC_CS_WriteAccess) (void * /*baseaddress*/, void * /*elemaddress*/);

/******************************************************************************
 *
 * function:
 *   void SAC_CS_Start( char* tag)
 *
 * description:
 *   Starts the analysis and offers the possibility to mark it by a
 *   userdefined tag.
 *
 *****************************************************************************/

SAC_C_EXTERN void (*SAC_CS_Start) (char * /*tag*/);

/******************************************************************************
 *
 * function:
 *   void SAC_CS_Stop( void)
 *
 * description:
 *   Stops the analysis and prints its tag (defined in SAC_CS_Start) and
 *   their results.
 *   simple & detailed profilinglevel:
 *     hit- and missrate for each cachelevel
 *   detailed profilinglevel only:
 *     classification of misses as coldstart, self- or crossinterference
 *
 *****************************************************************************/

SAC_C_EXTERN void (*SAC_CS_Stop) (void);

#endif /* ! SAC_SIMD_COMPILATION */

/*****************************************************************************
 *
 *  Macro definitions
 *
 *****************************************************************************/

#if (SAC_DO_CACHESIM)

#if (SAC_DO_CACHESIM_FILE)
#define SAC_CS_LEVEL SAC_CS_file

#elif (SAC_DO_CACHESIM_IMDE)

#if (SAC_DO_CACHESIM_ADV)
#define SAC_CS_LEVEL SAC_CS_advanced
#else
#define SAC_CS_LEVEL SAC_CS_simple
#endif

#elif (SAC_DO_CACHESIM_PIPE)

#if (SAC_DO_CACHESIM_ADV)
#define SAC_CS_LEVEL SAC_CS_piped_advanced
#else
#define SAC_CS_LEVEL SAC_CS_piped_simple
#endif

#else

#define SAC_CS_LEVEL SAC_CS_none

#endif

#define SAC_CS_SETUP()                                                                   \
    {                                                                                    \
        tProfilingLevel profilinglevel = SAC_CS_LEVEL;                                   \
        int cs_global = SAC_DO_CACHESIM_GLOBAL;                                          \
        char *cshost = SAC_SET_CACHESIM_HOST;                                            \
        char *csfile = SAC_SET_CACHESIM_FILE;                                            \
        char *csdir = SAC_SET_CACHESIM_DIR;                                              \
                                                                                         \
        unsigned long int cachesize1 = SAC_SET_CACHE_1_SIZE;                             \
        int cachelinesize1 = SAC_SET_CACHE_1_LINE;                                       \
        int associativity1 = SAC_SET_CACHE_1_ASSOC;                                      \
        tWritePolicy writepolicy1 = SAC_SET_CACHE_1_WRITEPOL;                            \
                                                                                         \
        unsigned long int cachesize2 = SAC_SET_CACHE_2_SIZE;                             \
        int cachelinesize2 = SAC_SET_CACHE_2_LINE;                                       \
        int associativity2 = SAC_SET_CACHE_2_ASSOC;                                      \
        tWritePolicy writepolicy2 = SAC_SET_CACHE_2_WRITEPOL;                            \
                                                                                         \
        unsigned long int cachesize3 = SAC_SET_CACHE_3_SIZE;                             \
        int cachelinesize3 = SAC_SET_CACHE_3_LINE;                                       \
        int associativity3 = SAC_SET_CACHE_3_ASSOC;                                      \
        tWritePolicy writepolicy3 = SAC_SET_CACHE_3_WRITEPOL;                            \
                                                                                         \
        SAC_CS_CheckArguments (__argc, __argv, &profilinglevel, &cs_global, &cshost,     \
                               &csfile, &csdir, &cachesize1, &cachelinesize1,            \
                               &associativity1, &writepolicy1, &cachesize2,              \
                               &cachelinesize2, &associativity2, &writepolicy2,          \
                               &cachesize3, &cachelinesize3, &associativity3,            \
                               &writepolicy3);                                           \
                                                                                         \
        SAC_CS_Initialize (SAC_MT_THREADS (), profilinglevel, cs_global, cshost, csfile, \
                           csdir, cachesize1, cachelinesize1, associativity1,            \
                           writepolicy1, cachesize2, cachelinesize2, associativity2,     \
                           writepolicy2, cachesize3, cachelinesize3, associativity3,     \
                           writepolicy3);                                                \
                                                                                         \
        SAC_CS_START ("#");                                                              \
    }

#define SAC_CS_FINALIZE()                                                                \
    {                                                                                    \
        SAC_CS_STOP ("#");                                                               \
        SAC_CS_Finalize ();                                                              \
    }

#define SAC_CS_READ_ARRAY(var_NT, pos)                                                   \
    SAC_CS_ReadAccess (SAC_ND_A_FIELD (var_NT), SAC_ND_A_FIELD (var_NT) + (pos)),

#define SAC_CS_WRITE_ARRAY(var_NT, pos)                                                  \
    SAC_CS_WriteAccess (SAC_ND_A_FIELD (var_NT), SAC_ND_A_FIELD (var_NT) + (pos)),

#define SAC_CS_REGISTER_ARRAY(var_NT)                                                    \
  SAC_CS_RegisterArray( SAC_ND_A_FIELD( var_NT),
                        SAC_ND_A_SIZE( var_NT) * sizeof( *(SAC_ND_A_FIELD( var_NT))));

#define SAC_CS_UNREGISTER_ARRAY(var_NT) SAC_CS_UnregisterArray (SAC_ND_A_FIELD (var_NT));

#define SAC_CS_START(tag) SAC_CS_Start (tag);
#define SAC_CS_STOP(tag) SAC_CS_Stop ();

#else /* SAC_DO_CACHESIM */

#define SAC_CS_LEVEL SAC_CS_none
#define SAC_CS_SETUP()
#define SAC_CS_FINALIZE()
#define SAC_CS_READ_ARRAY(var_NT, pos)
#define SAC_CS_WRITE_ARRAY(var_NT, pos)
#define SAC_CS_REGISTER_ARRAY(var_NT)
#define SAC_CS_UNREGISTER_ARRAY(var_NT)
#define SAC_CS_START(tag)
#define SAC_CS_STOP(tag)

#endif /* SAC_DO_CACHESIM */

#endif /* _SAC_CACHESIM_H */
