/*****************************************************************************
 *
 * @defgroup Lift memory transfers in loops whenever possible
 *
 *
 *   This module implements the transformation of lifting memory transfers
 *   (<host2device>/<device2host>) out of a do-fun. Memory transfers that
 *   are allowed to be moved out were tagged in the previous phase, i.e.
 *   Annotate Memory Transfer (AMTRAN).
 *
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file minimize_loop_transfers.c
 *
 * Prefix: MCSTRAN
 *
 *****************************************************************************/
#include "minimize_cudast_transfers.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "remove_dfms.h"
#include "infer_dfms.h"
#include "NameTuplesUtils.h"
#include "scheduling.h"
#include "wl_bounds.h"
#include "new_types.h"
#include "user_types.h"
#include "shape.h"
#include "LookUpTable.h"
#include "convert.h"
#include "math_utils.h"
#include "types.h"
#include "deadcoderemoval.h"
#include "cuda_utils.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool incudast;
    bool lift;
    dfmask_t *in_mask;
    dfmask_t *out_mask;
    node *preassigns;
    node *postassigns;
    node *letids;
};

#define INFO_INCUDAST(n) (n->incudast)
#define INFO_LIFT(n) (n->lift)
#define INFO_IN_MASK(n) (n->in_mask)
#define INFO_OUT_MASK(n) (n->out_mask)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_POSTASSIGNS(n) (n->postassigns)
#define INFO_LETIDS(n) (n->letids)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_INCUDAST (result) = FALSE;
    INFO_LIFT (result) = FALSE;
    INFO_IN_MASK (result) = NULL;
    INFO_OUT_MASK (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_LETIDS (result) = NULL;

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
 * @fn node *MCSTRANdoMinimizeCudastTransfers( node *syntax_tree)
 *
 *****************************************************************************/
node *
MCSTRANdoMinimizeCudastTransfers (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ();

    /*
     * Infer dataflow masks
     */
    syntax_tree = INFDFMSdoInferDfms (syntax_tree, HIDE_LOCALS_NEVER);

    info = MakeInfo ();

    TRAVpush (TR_mcstran);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    syntax_tree = DCRdoDeadCodeRemoval (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *MCSTRANcudast( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
MCSTRANcudast (node *arg_node, info *arg_info)
{
    node *region;

    DBUG_ENTER ();

    region = CUDAST_REGION (arg_node);

    INFO_IN_MASK (arg_info) = BLOCK_IN_MASK (region);
    INFO_OUT_MASK (arg_info) = BLOCK_OUT_MASK (region);
    INFO_INCUDAST (arg_info) = TRUE;

    BLOCK_ASSIGNS (region) = TRAVopt (BLOCK_ASSIGNS (region), arg_info);

    INFO_INCUDAST (arg_info) = FALSE;
    INFO_IN_MASK (arg_info) = NULL;
    INFO_OUT_MASK (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MCSTRANassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
MCSTRANassign (node *arg_node, info *arg_info)
{
    node *assigns;

    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_INCUDAST (arg_info) && INFO_LIFT (arg_info)) {
        arg_node = FREEdoFreeNode (arg_node);
        INFO_LIFT (arg_info) = FALSE;
    }

    if (!INFO_INCUDAST (arg_info)) {
        if (INFO_POSTASSIGNS (arg_info) != NULL) {
            assigns
              = TCappendAssign (INFO_POSTASSIGNS (arg_info), ASSIGN_NEXT (arg_node));
            ASSIGN_NEXT (arg_node) = NULL;
            arg_node = TCappendAssign (arg_node, assigns);
            INFO_POSTASSIGNS (arg_info) = NULL;
            global.optcounters.cuda_min_trans++;
        }

        if (INFO_PREASSIGNS (arg_info) != NULL) {
            arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
            INFO_PREASSIGNS (arg_info) = NULL;
            global.optcounters.cuda_min_trans++;
        }
    }

    /*
      ASSIGN_STMT( arg_node) = TRAVopt( ASSIGN_STMT( arg_node), arg_info);




      next = ASSIGN_NEXT( arg_node);
      ASSIGN_NEXT( arg_node) = NULL;

      if ( INFO_POSTASSIGNS( arg_info) != NULL) {
        arg_node = TCappendAssign( arg_node, INFO_POSTASSIGNS( arg_info));
        INFO_POSTASSIGNS( arg_info) = NULL;
      }

      if( INFO_PREASSIGNS( arg_info) != NULL) {
        arg_node = TCappendAssign( INFO_PREASSIGNS( arg_info), arg_node);
        INFO_PREASSIGNS( arg_info) = NULL;
      }

      node *last_assign = arg_node;
      while( ASSIGN_NEXT( last_assign) != NULL) {
        last_assign = ASSIGN_NEXT( last_assign);
      }

      ASSIGN_NEXT( last_assign) = next;
      ASSIGN_NEXT( last_assign) = TRAVopt( ASSIGN_NEXT( last_assign), arg_info);
    */

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MCSTRANlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
MCSTRANlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    INFO_LETIDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MCSTRANprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
MCSTRANprf (node *arg_node, info *arg_info)
{
    node *lhs_avis, *rhs_avis;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_host2device:
        lhs_avis = IDS_AVIS (INFO_LETIDS (arg_info));
        rhs_avis = ID_AVIS (PRF_ARG1 (arg_node));
        if (INFO_INCUDAST (arg_info)
            && DFMtestMaskEntry (INFO_IN_MASK (arg_info), rhs_avis)) {
            INFO_PREASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (lhs_avis, NULL),
                                         TBmakePrf (F_host2device,
                                                    TBmakeExprs (TBmakeId (rhs_avis),
                                                                 NULL))),
                              INFO_PREASSIGNS (arg_info));

            AVIS_SSAASSIGN (lhs_avis) = INFO_PREASSIGNS (arg_info);
            INFO_LIFT (arg_info) = TRUE;
        }
        break;
    case F_device2host:
        lhs_avis = IDS_AVIS (INFO_LETIDS (arg_info));
        rhs_avis = ID_AVIS (PRF_ARG1 (arg_node));
        if (INFO_INCUDAST (arg_info)
            && DFMtestMaskEntry (INFO_OUT_MASK (arg_info), lhs_avis)) {
            INFO_POSTASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (lhs_avis, NULL),
                                         TBmakePrf (F_device2host,
                                                    TBmakeExprs (TBmakeId (rhs_avis),
                                                                 NULL))),
                              INFO_POSTASSIGNS (arg_info));

            AVIS_SSAASSIGN (lhs_avis) = INFO_POSTASSIGNS (arg_info);
            INFO_LIFT (arg_info) = TRUE;
        }
        break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
