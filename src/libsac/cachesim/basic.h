/*****************************************************************************
 *
 * file:   basic.h
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

#ifndef _SAC_CS_BASIC_H_
#define _SAC_CS_BASIC_H_

#include "cachesim.h"

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

extern char SAC_CS_separator[];
extern char SAC_CS_separator_2[];

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

extern tFunRWAccess
  SAC_CS_read_access_table[MAX_CACHELEVEL + 2],
  SAC_CS_write_access_table[MAX_CACHELEVEL + 2];
/* SAC_CS_xxx_access_table[0] is unused,
 * SAC_CS_xxx_access_table[MAX_CACHELEVEL+1]
 *   for dummy/MainMem */
/* END: */

#define MAX_TAG_LENGTH 512

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

#endif /* _SAC_CS_BASIC_H_ */

