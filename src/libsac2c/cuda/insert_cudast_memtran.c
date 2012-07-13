/** <!--********************************************************************-->
 *
 * @defgroup
 *
 *
 * @{ASSIGN_STMT( arg_node)
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file cuda_type_conversion.c
 *
 * Prefix: ICSMEM
 *
 *****************************************************************************/
#include "insert_cudast_memtran.h"

/*
 * Other includes go here
 */
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
#include "new_types.h"
#include "LookUpTable.h"
#include "math_utils.h"
#include "types.h"
#include "type_utils.h"
#include "cuda_utils.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "remove_dfms.h"
#include "infer_dfms.h"
#include "constants.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    bool incudast;
    node *preassigns;
    node *postassigns;
    bool fromap;
    bool trav_ids;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INCUDAST(n) (n->incudast)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_POSTASSIGNS(n) (n->postassigns)
#define INFO_FROMAP(n) (n->fromap)
#define INFO_TRAV_IDS(n) (n->trav_ids)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_INCUDAST (result) = FALSE;
    INFO_PREASSIGNS (result) = NULL;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_FROMAP (result) = FALSE;
    INFO_TRAV_IDS (result) = TRUE;

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
 * @fn node *ICSMEMdoInsertCudastMemtran( node *syntax_tree)
 *
 *****************************************************************************/
node *
ICSMEMdoInsertCudastMemtran (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_icsmem);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

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
 * @fn node *ICSMEMfundef( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/

/*
node *ICSMEMfundef( node *arg_node, info *arg_info)
{
  node *old_fundef;

  DBUG_ENTER( "ICSMEMfundef");

  if( !FUNDEF_ISLACFUN( arg_node)) {
    INFO_FUNDEF( arg_info) = arg_node;
    FUNDEF_BODY( arg_node) = TRAVopt( FUNDEF_BODY( arg_node), arg_info);
    INFO_FUNDEF( arg_info) = NULL;
    FUNDEF_NEXT( arg_node) = TRAVopt( FUNDEF_NEXT( arg_node), arg_info);
  }
  else {
    if( INFO_FROMAP( arg_info)) {
      old_fundef = INFO_FUNDEF( arg_info);
      INFO_FUNDEF( arg_info) = arg_node;
      FUNDEF_BODY( arg_node) = TRAVopt( FUNDEF_BODY( arg_node), arg_info);
      INFO_FUNDEF( arg_info) = old_fundef;
    }
    else {
      FUNDEF_NEXT( arg_node) = TRAVopt( FUNDEF_NEXT( arg_node), arg_info);
    }
  }

  DBUG_RETURN( arg_node);
}
*/

node *
ICSMEMfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    INFO_FUNDEF (arg_info) = NULL;

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCCassign( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
ICSMEMassign (node *arg_node, info *arg_info)
{
    node *next;

    DBUG_ENTER ();

    /*
      ASSIGN_NEXT( arg_node) = TRAVopt( ASSIGN_NEXT( arg_node), arg_info);

      ASSIGN_STMT( arg_node) = TRAVopt( ASSIGN_STMT( arg_node), arg_info);

      if( INFO_POSTASSIGNS( arg_info) != NULL) {
        assigns = TCappendAssign( INFO_POSTASSIGNS( arg_info), ASSIGN_NEXT( arg_node));
        arg_node = TCappendAssign( arg_node, assigns);
        INFO_POSTASSIGNS( arg_info) = NULL;
      }

      if( INFO_PREASSIGNS( arg_info) != NULL) {
        arg_node = TCappendAssign( INFO_PREASSIGNS( arg_info), arg_node);
        INFO_PREASSIGNS( arg_info) = NULL;
      }
    */

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    next = ASSIGN_NEXT (arg_node);
    ASSIGN_NEXT (arg_node) = NULL;

    if (INFO_POSTASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (arg_node, INFO_POSTASSIGNS (arg_info));
        INFO_POSTASSIGNS (arg_info) = NULL;
    }

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    node *last_assign = arg_node;
    while (ASSIGN_NEXT (last_assign) != NULL) {
        last_assign = ASSIGN_NEXT (last_assign);
    }

    ASSIGN_NEXT (last_assign) = next;
    ASSIGN_NEXT (last_assign) = TRAVopt (ASSIGN_NEXT (last_assign), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICSMEMlet( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
ICSMEMlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    if (INFO_TRAV_IDS (arg_info)) {
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
    } else {
        INFO_TRAV_IDS (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICSMEMcudast( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
ICSMEMcudast (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_INCUDAST (arg_info) = TRUE;
    CUDAST_REGION (arg_node) = TRAVopt (CUDAST_REGION (arg_node), arg_info);
    INFO_INCUDAST (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICSMEMap( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
/*
node *ICSMEMap( node *arg_node, info *arg_info)
{
  bool old_fromap;
  node *fundef;

  DBUG_ENTER( "ICSMEMap");

  fundef = AP_FUNDEF( arg_node);

  if( INFO_INCUDAST( arg_info)) {
    if( fundef != INFO_FUNDEF( arg_info)) {
      DBUG_ASSERT( fundef != NULL, "Found N_ap with empty N_fundef in N_cudast!");
      DBUG_ASSERT( FUNDEF_ISLACFUN( fundef), "Non lac N_ap found in N_cudast!");
      old_fromap = INFO_FROMAP( arg_info);
      INFO_FROMAP( arg_info) = TRUE;
      AP_FUNDEF( arg_node) = TRAVdo( AP_FUNDEF( arg_node), arg_info);
      INFO_FROMAP( arg_info) = old_fromap;
    }
    INFO_TRAV_IDS( arg_info) = FALSE;
  }

  DBUG_RETURN( arg_node);
}
*/

node *
ICSMEMap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* If the application is lac fun, we don't need to create
     * host2device for it in this phase. It will be dealt with
     * in later phase when data transfers are lifted out of
     * loops and conditionals. */
    if (AP_FUNDEF (arg_node) != NULL && FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))) {
        INFO_TRAV_IDS (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICSMEMfuncond( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
ICSMEMfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_TRAV_IDS (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICSMEMids( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
ICSMEMids (node *arg_node, info *arg_info)
{
    node *avis, *new_avis, *fundef;
    ntype *dev_type, *scalar_type;
    simpletype sty;

    DBUG_ENTER ();

    fundef = INFO_FUNDEF (arg_info);
    avis = IDS_AVIS (arg_node);

    if ((FUNDEF_ISCUDALACFUN (fundef) || INFO_INCUDAST (arg_info))
        && !TUisScalar (AVIS_TYPE (avis))) {
        DBUG_ASSERT (TYisAKS (AVIS_TYPE (avis)),
                     "Non AKS N_ids found in CUDA LAC fun or CUDAST!");

        dev_type = TYcopyType (AVIS_TYPE (avis));
        scalar_type = TYgetScalar (dev_type);
        sty = CUh2dSimpleTypeConversion (TYgetSimpleType (scalar_type));
        scalar_type = TYsetSimpleType (scalar_type, sty);
        new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);

        INFO_POSTASSIGNS (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                     TBmakePrf (F_device2host,
                                                TBmakeExprs (TBmakeId (new_avis), NULL))),
                          INFO_POSTASSIGNS (arg_info));

        AVIS_SSAASSIGN (new_avis) = AVIS_SSAASSIGN (avis);
        AVIS_SSAASSIGN (avis) = INFO_POSTASSIGNS (arg_info);

        IDS_AVIS (arg_node) = new_avis;

        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)),
                            TBmakeVardec (new_avis, NULL));
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICSMEMid( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
ICSMEMid (node *arg_node, info *arg_info)
{
    node *avis, *new_avis, *fundef;
    ntype *dev_type, *scalar_type;
    simpletype sty;

    DBUG_ENTER ();

    fundef = INFO_FUNDEF (arg_info);
    avis = ID_AVIS (arg_node);

    /* This condition needs to be further restricted */
    if ((FUNDEF_ISCUDALACFUN (fundef) || INFO_INCUDAST (arg_info))
        && !TUisScalar (AVIS_TYPE (avis))) {
        if (TYisAKV (AVIS_TYPE (avis))) {
            new_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (avis)));

            AVIS_ISCUDALOCAL (new_avis) = TRUE;

            INFO_PREASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL),
                                         COconstant2AST (
                                           TYgetValue (ID_NTYPE (arg_node)))),
                              INFO_PREASSIGNS (arg_info));

            AVIS_SSAASSIGN (new_avis) = INFO_PREASSIGNS (arg_info);
            ID_AVIS (arg_node) = new_avis;

            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)),
                                TBmakeVardec (new_avis, NULL));
        } else {
            DBUG_ASSERT (TYisAKS (AVIS_TYPE (avis)),
                         "Non AKS N_id found in CUDA LAC fun or CUDAST!");

            dev_type = TYcopyType (AVIS_TYPE (avis));
            scalar_type = TYgetScalar (dev_type);
            sty = CUh2dSimpleTypeConversion (TYgetSimpleType (scalar_type));
            scalar_type = TYsetSimpleType (scalar_type, sty);
            new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);

            INFO_PREASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL),
                                         TBmakePrf (F_host2device,
                                                    TBmakeExprs (TBmakeId (avis), NULL))),
                              INFO_PREASSIGNS (arg_info));

            AVIS_SSAASSIGN (new_avis) = INFO_PREASSIGNS (arg_info);
            ID_AVIS (arg_node) = new_avis;
            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)),
                                TBmakeVardec (new_avis, NULL));
        }
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
