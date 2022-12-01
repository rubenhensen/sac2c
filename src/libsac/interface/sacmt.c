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

/** --------------------------------------------------------------------------------------
 *
 * SAC4C INTERFACE FUNCTIONS
 *
 * They are published in sacinterface.h
 */

/** <!--********************************************************************-->
 *
 * @fn void SAC_InitRuntimeSystem ( int argc, char *argv[], unsigned int num_threads);
 *
 * @brief Runtime initialization. For sacinterface.h.
 *
 *****************************************************************************/

void
SAC_InitRuntimeSystem (void)
{
    SAC_MT_SetupAsLibraryInitial ();
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
#endif
