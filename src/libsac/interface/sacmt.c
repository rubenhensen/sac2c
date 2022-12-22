#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

/* Public stuff */
#include "sacinterface.h"

/* SAC config */
#include "config.h"

#if SAC_MT_MODE > 0
/* the code is only loaded into libsac.mt.pth and libsac.mt.lpel */
#if defined(SAC_MT_LIB_pthread) || defined(SAC_MT_LIB_lpel)

#define SAC_DO_MULTITHREAD 1
#define SAC_DO_MT_BEEHIVE 1
#define SAC_DO_PHM 1

#include "libsac/mt/mt_beehive.h"   // SAC_MT_SetupAsLibraryInitial, ...
#include "libsac/essentials/message.h" // SAC_RuntimeWarning, ...
#include "libsac/mt/mt.h"  // SAC_MT_barrier_type, ...
#include "libsac/mt/mt_pth.h"  // SAC_MT_PTH_SetupStandalone

/** --------------------------------------------------------------------------------------
 *
 * SAC4C INTERFACE FUNCTIONS
 *
 * They are published in sacinterface.h
 */

/** <!--********************************************************************-->
 *
 * @fn void SAC_InitRuntimeSystem ( int argc, char *argv[],
 *                                  unsigned int num_threads,
 *                                  int num_schedulers, bool do_trace)
 *
 * @brief Runtime initialization. For sacinterface.h.
 *
 *****************************************************************************/

void
SAC_InitRuntimeSystem ( int argc, char *argv[], unsigned int num_threads,
                        int num_schedulers, unsigned int do_trace)
{
    SAChive *hive;

    SAC_MT_barrier_type = 0;       // using spin-locks
    SAC_MT_cpu_bind_strategy = 0;  // assuming hwloc off!
    SAC_MT_do_trace = do_trace;
    SAC_MT_SetupInitial (argc, argv, num_threads, 1024);
    SAC_MT_globally_single = 0;   // In a library we're never alone!

    SAC_MT_PTH_SetupStandalone (num_schedulers);
    SAC_MT_singleton_queen = NULL;  // we are not in standalone mode!
    
}

/** <!--********************************************************************-->
 *
 * @fn void SAC_FreeRuntimeSystem ();
 *
 * @brief Runtime freeing. For sacinterface.h.
 *
 *****************************************************************************/
void
SAC_FreeRuntimeSystem (void)
{
    SAChive *hive;

    hive = SAC_DetachHive ();
    SAC_ReleaseHive (hive);
    SAC_ReleaseQueen ();

    /* check there are no hives/bees left behind */
    unsigned orphan_hives = SAC_MT_cnt_hives;
    unsigned orphan_queens = SAC_MT_cnt_queen_bees;
    unsigned orphan_worker = SAC_MT_cnt_worker_bees;

    if (orphan_hives || orphan_queens || orphan_worker) {
        SAC_RuntimeWarning ("SAC_FreeRuntimeSystem: There are orphans left behind: %u "
                            "hives, %u queens, %u workers.",
                            orphan_hives, orphan_queens, orphan_worker);
    }
}

SAChive *
SAC_AllocHive (unsigned int num_bees, int num_schedulers, const int *places, void *thdata)
{
    return (SAChive *)SAC_MT_AllocHive (num_bees, num_schedulers, places, thdata);
}

void
SAC_ReleaseHive (SAChive *h)
{
    SAC_MT_ReleaseHive ((struct sac_hive_common_t *)h);
}

void
SAC_AttachHive (SAChive *h)
{
    SAC_MT_AttachHive ((struct sac_hive_common_t *)h);
}

SAChive *
SAC_DetachHive (void)
{
    return (SAChive *)SAC_MT_DetachHive ();
}

void
SAC_ReleaseQueen (void)
{
    SAC_MT_ReleaseQueen ();
}

#endif

#else
/** <!--********************************************************************-->
 *
 * @fn void SAC_InitRuntimeSystem ( int argc, char *argv[],
 *                                  unsigned int num_threads,
 *                                  int num_schedulers, bool do_trace)
 *
 * @brief Runtime initialization. For sacinterface.h.
 *
 *****************************************************************************/

void
SAC_InitRuntimeSystem ( int argc, char *argv[], unsigned int num_threads,
                        int num_schedulers, unsigned int do_trace)
{
}

/** <!--********************************************************************-->
 *
 * @fn void SAC_FreeRuntimeSystem ();
 *
 * @brief Runtime freeing. For sacinterface.h.
 *
 *****************************************************************************/
void
SAC_FreeRuntimeSystem (void)
{
}

#endif
