/*****************************************************************************
 *
 * file:   rt_cachesim.h
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

#ifndef _SAC_RT_CACHESIM_H
#define _SAC_RT_CACHESIM_H

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

#if SAC_MUTC_MACROS
#else

#define SAC_CS_NONE 0
#define SAC_CS_FILE 1
#define SAC_CS_SIMPLE 2
#define SAC_CS_ADVANCED 3

#endif

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
        SAC_CS_Initialize (SAC_MT_GLOBAL_THREADS (), profilinglevel, cs_global, cshost,  \
                           csfile, csdir, cachesize1, cachelinesize1, associativity1,    \
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

#endif /* _SAC_RT_CACHESIM_H */

