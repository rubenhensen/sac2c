/* Public stuff */
#include "sacinterface.h"

/* SAC config */
#include "config.h"

#if ENABLE_MT
/* the code is only loaded into libsac.mt.pth and libsac.mt.lpel */
#if defined(PTH) || defined(LPEL)

#define SAC_DO_MULTITHREAD 1
#define SAC_DO_MT_BEEHIVE 1
#define SAC_DO_PHM 1

#include "sac.h"

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

#if 0
  /* FIXME: PHM should be handled orthogonally from SAC_InitRuntimeSystem()
   *
   * PHM (Private heap manager) in sac4c setting is disabled because there are assumption
   * in the generated code about PHM and single-threadeness of a program.
   */
  SAC_HM_Setup( SAC_MT_global_threads + SAC_MT_hm_aux_threads);
#endif
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
    unsigned orphans = SAC_MT_BeesGrandTotal ();
    if (orphans) {
        SAC_RuntimeWarning ("SAC_FreeRuntimeSystem: There are %u bees still alive!",
                            orphans);
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
