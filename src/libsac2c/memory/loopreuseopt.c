/** <!--********************************************************************-->
 *
 * @defgroup lr Loop Reuse Optimization
 *
 * @ingroup mm
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file loopreuseopt.c
 *
 * Prefix: EMLR
 *
 *****************************************************************************/
#include "loopreuseopt.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "EMLR"
#include "debug.h"

#include "print.h"
#include "shape.h"
#include "new_types.h"
#include "user_types.h"
#include "DataFlowMask.h"
#include "filterrc.h"
#include "aliasanalysis.h"
#include "DupTree.h"
#include "str.h"
#include "memory.h"

/*
 * CONTEXT enumeration: lr_context_t
 */
typedef enum { LR_undef, LR_allocorreuse, LR_condargs, LR_doargs, LR_recap } lr_context_t;

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    lr_context_t context;
    node *preassign;
    node *fundef;
    node *apargs;
    dfmask_t *apmask;
    dfmask_t *reusemask;
};

#define INFO_CONTEXT(n) (n->context)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_REUSEMASK(n) (n->reusemask)
#define INFO_APARGS(n) (n->apargs)
#define INFO_APMASK(n) (n->apmask)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_CONTEXT (result) = LR_undef;
    INFO_PREASSIGN (result) = NULL;
    INFO_FUNDEF (result) = fundef;
    INFO_REUSEMASK (result) = NULL;
    INFO_APARGS (result) = NULL;
    INFO_APMASK (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMLRdoLoopReuseOptimization( node *arg_node)
 *
 * @brief starting point of Loop Reuse Optimization traversal
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
EMLRdoLoopReuseOptimization (node *arg_node)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Starting Loop Reuse Optimization...");

    TRAVpush (TR_emlr);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_PRINT ("Loop Reuse Traversal complete.");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/
static bool
DFMequalMasks (dfmask_t *mask1, dfmask_t *mask2)
{
    dfmask_t *d1, *d2;
    int sum;

    DBUG_ENTER ();

    d1 = DFMgenMaskMinus (mask1, mask2);
    d2 = DFMgenMaskMinus (mask2, mask1);

    sum = DFMtestMask (d1) + DFMtestMask (d2);

    d1 = DFMremoveMask (d1);
    d2 = DFMremoveMask (d2);

    DBUG_RETURN (sum == 0);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @node *EMLRap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMLRap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Traverse into special functions
     */
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    /*
     * Optimize loops
     */
    if ((FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        node *doargs;
        node *apargs;

        /*
         * Perform loop optimization using seperate traversal
         */
        TRAVpush (TR_emlro);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), NULL);
        TRAVpop ();

        /*
         * Copy all arguments that can be statically reused inside the loop
         */
        doargs = FUNDEF_ARGS (AP_FUNDEF (arg_node));
        apargs = AP_ARGS (arg_node);

        while (doargs != NULL) {
            if (!AVIS_ISALIAS (ARG_AVIS (doargs))) {
                /*
                 * Insert copy instructions
                 *
                 * b     = do( a);
                 *
                 * is converted into
                 *
                 * a_mem = alloc_or_reuse( dim(a), shape(a), a);
                 * a_val = fill( copy( a), a_mem);
                 *
                 * b      = do( a_val);
                 */
                node *memavis, *valavis, *oldarg, *oldavis;

                oldarg = EXPRS_EXPR (apargs);
                oldavis = ID_AVIS (oldarg);

                /*
                 * Create a new memory variable
                 * Ex: a_mem
                 */
                memavis = TBmakeAvis (TRAVtmpVarName (ID_NAME (oldarg)),
                                      TYeliminateAKV (AVIS_TYPE (oldavis)));

                FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (memavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

                /*
                 * Create a new value variable
                 * Ex: a_val
                 */
                valavis = TBmakeAvis (TRAVtmpVarName (ID_NAME (oldarg)),
                                      TYcopyType (AVIS_TYPE (oldavis)));

                FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (valavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

                /*
                 * Create fill operation
                 *
                 * Ex:
                 *   ...
                 *   a_val = fill( copy( a), a_mem);
                 */
                INFO_PREASSIGN (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (valavis, NULL),
                                             TCmakePrf2 (F_fill,
                                                         TCmakePrf1 (F_copy,
                                                                     DUPdoDupNode (
                                                                       oldarg)),
                                                         TBmakeId (memavis))),
                                  INFO_PREASSIGN (arg_info));
                AVIS_SSAASSIGN (valavis) = INFO_PREASSIGN (arg_info);

                /*
                 * Substitute application argument
                 *
                 * Ex:
                 *   ...
                 *   a_val = fill( copy( a), a_mem);
                 *   b     = do( a_val);
                 */
                EXPRS_EXPR (apargs) = TBmakeId (valavis);

                /*
                 * Create allor_or_reuse assignment
                 *
                 * Ex:
                 *   a_mem = allor_or_reuse( dim( a), shape( a), a);
                 *   a_val = fill( copy( a), a_mem);
                 *   b     = do( a_val);
                 */
                INFO_PREASSIGN (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (memavis, NULL),
                                             TCmakePrf3 (F_alloc_or_reuse,
                                                         TCmakePrf1 (F_dim_A,
                                                                     DUPdoDupNode (
                                                                       oldarg)),
                                                         TCmakePrf1 (F_shape_A,
                                                                     DUPdoDupNode (
                                                                       oldarg)),
                                                         oldarg)),
                                  INFO_PREASSIGN (arg_info));
                AVIS_SSAASSIGN (memavis) = INFO_PREASSIGN (arg_info);
            }

            doargs = ARG_NEXT (doargs);
            apargs = EXPRS_NEXT (apargs);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLRassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMLRassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Bottom-up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL) {
        INFO_PREASSIGN (arg_info) = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);

        arg_node = INFO_PREASSIGN (arg_info);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLRfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMLRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((!FUNDEF_ISLACFUN (arg_node)) || (arg_info != NULL)) {

        DBUG_PRINT ("Traversing function %s", FUNDEF_NAME (arg_node));
        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info;
            info = MakeInfo (arg_node);

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            info = FreeInfo (info);
        }
    }

    if (arg_info == NULL) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Loop optimization traversal functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @node *EMLROap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMLROap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        INFO_APARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    if ((FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info))) {
        /*
         * Clean up reusable arguments
         */
        if (FUNDEF_ARGS (INFO_FUNDEF (arg_info)) != NULL) {
            INFO_CONTEXT (arg_info) = LR_recap;
            INFO_APARGS (arg_info) = AP_ARGS (arg_node);

            FUNDEF_ARGS (INFO_FUNDEF (arg_info))
              = TRAVdo (FUNDEF_ARGS (INFO_FUNDEF (arg_info)), arg_info);

            INFO_CONTEXT (arg_info) = LR_undef;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLROarg( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMLROarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_CONTEXT (arg_info)) {
    case LR_doargs:
        if (INFO_REUSEMASK (arg_info) == NULL) {
            AVIS_ISALIAS (ARG_AVIS (arg_node)) = FALSE;
        } else {
            AVIS_ISALIAS (ARG_AVIS (arg_node))
              = !DFMtestMaskEntry (INFO_REUSEMASK (arg_info), NULL, ARG_AVIS (arg_node));
        }
        break;

    case LR_condargs:
        if (DFMtestMaskEntry (INFO_REUSEMASK (arg_info), NULL, ARG_AVIS (arg_node))) {
            DFMsetMaskEntrySet (INFO_APMASK (arg_info), NULL,
                                ID_AVIS (EXPRS_EXPR (INFO_APARGS (arg_info))));
        }

        INFO_APARGS (arg_info) = EXPRS_NEXT (INFO_APARGS (arg_info));
        break;

    case LR_recap:
        if (AVIS_ISALIAS (ID_AVIS (EXPRS_EXPR (INFO_APARGS (arg_info))))) {
            DFMsetMaskEntryClear (INFO_REUSEMASK (arg_info), NULL, ARG_AVIS (arg_node));
        }

        INFO_APARGS (arg_info) = EXPRS_NEXT (INFO_APARGS (arg_info));
        break;

    default:
        DBUG_UNREACHABLE ("Illegal context!");
        break;
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLROfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMLROfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (FUNDEF_ISLACFUN (arg_node),
                 "EMLROfundef is only applicable for LAC-functions");

    if (FUNDEF_ISLOOPFUN (arg_node)) {
        info *info;
        node *fundef_next;
        dfmask_base_t *maskbase;
        dfmask_t *oldmask;

        DBUG_PRINT ("Optimizing %s", FUNDEF_NAME (arg_node));

        /*
         * Traversal of DOFUN
         */

        info = MakeInfo (arg_node);
        maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));

        /*
         * rescue FUNDEF_NEXT
         */
        fundef_next = FUNDEF_NEXT (arg_node);

        /*
         * Filter reuse candidates
         */
        FUNDEF_NEXT (arg_node) = NULL;
        arg_node = FRCdoFilterReuseCandidates (arg_node);

        /*
         * Initialize arguments' AVIS_ALIAS with values from REUSEMASK
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            INFO_CONTEXT (info) = LR_doargs;
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            INFO_CONTEXT (info) = LR_undef;
        }

        /*
         * Perform alias analysis
         */
        arg_node = EMAAdoAliasAnalysis (arg_node);

        INFO_REUSEMASK (info) = DFMgenMaskClear (maskbase);
        oldmask = DFMgenMaskClear (maskbase);

        while (TRUE) {
            DFMsetMaskCopy (oldmask, INFO_REUSEMASK (info));

            /*
             * Traverse body in order to determine REUSEMASK
             */
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            if (DFMequalMasks (oldmask, INFO_REUSEMASK (info))) {
                break;
            }

            /*
             * Perform alias analysis
             */
            arg_node = EMAAdoAliasAnalysis (arg_node);
        }

        /*
         * Print statically resusable variables
         */
        DBUG_PRINT ("The following variables can be statically reused:");
        DBUG_EXECUTE (DFMprintMask (global.outfile, "%s\n", INFO_REUSEMASK (info)));

        /*
         * Initialize arguments' AVIS_ALIAS with values from REUSEMASK
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            INFO_CONTEXT (info) = LR_doargs;
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            INFO_CONTEXT (info) = LR_undef;
        }

        /*
         * Restore FUNDEF_NEXT
         */
        FUNDEF_NEXT (arg_node) = fundef_next;

        oldmask = DFMremoveMask (oldmask);
        INFO_REUSEMASK (info) = DFMremoveMask (INFO_REUSEMASK (info));
        maskbase = DFMremoveMaskBase (maskbase);
        info = FreeInfo (info);
    } else {
        info *info;
        dfmask_base_t *maskbase;

        /*
         * Traversal of inner CONDFUN
         */
        info = MakeInfo (arg_node);
        maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));
        INFO_REUSEMASK (info) = DFMgenMaskClear (maskbase);

        /*
         * Traverse function body
         */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

        /*
         * Transcribe REUSEMASK
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            INFO_APMASK (info) = INFO_REUSEMASK (arg_info);
            INFO_APARGS (info) = INFO_APARGS (arg_info);
            INFO_APARGS (arg_info) = NULL;
            INFO_CONTEXT (info) = LR_condargs;
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            INFO_CONTEXT (info) = LR_undef;
        }

        INFO_REUSEMASK (info) = DFMremoveMask (INFO_REUSEMASK (info));
        maskbase = DFMremoveMaskBase (maskbase);
        info = FreeInfo (info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLROid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMLROid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_CONTEXT (arg_info)) {
    case LR_allocorreuse:
        if (!AVIS_ISALIAS (ID_AVIS (arg_node))) {
            DFMsetMaskEntrySet (INFO_REUSEMASK (arg_info), NULL, ID_AVIS (arg_node));
        }
        break;

    case LR_undef:
        break;

    default:
        DBUG_UNREACHABLE ("Illegal context!");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLROprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMLROprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PRF_PRF (arg_node) == F_alloc_or_reuse) {
        if (PRF_EXPRS3 (arg_node) != NULL) {
            INFO_CONTEXT (arg_info) = LR_allocorreuse;
            PRF_EXPRS3 (arg_node) = TRAVdo (PRF_EXPRS3 (arg_node), arg_info);
            INFO_CONTEXT (arg_info) = LR_undef;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Loop Reuse Optimization -->
 *****************************************************************************/

#undef DBUG_PREFIX
