/*****************************************************************************
 *
 *
 * file:   annotate_cuda_withloop.c
 *
 * prefix: ACUWL
 *
 * description:
 *
 *
 *****************************************************************************/

#include "annotate_cuda_withloop.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"
#include "dbug.h"
#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "DataFlowMask.h"
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
#include "namespaces.h"

/*
 * INFO structure
 */
struct INFO {
    node *outerwl;
    bool cudarizable;
};

/*
 * INFO macros
 */
#define INFO_OUTERWL(n) (n->outerwl)
#define INFO_CUDARIZABLE(n) (n->cudarizable)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_OUTERWL (result) = NULL;
    INFO_CUDARIZABLE (result) = TRUE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *ACUWLdoAnnotateCUDAWL( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACUWLdoAnnotateCUDAWL (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ("ACUWLdoAnnotateCUDAWL");

    info = MakeInfo ();
    TRAVpush (TR_acuwl);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *ACUWLfundef( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACUWLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLfundef");

    if (!FUNDEF_ISSTICKY (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *ACUWLassign( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACUWLassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL)
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACUWLlet( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
/*
node *ACUWLlet( node *arg_node, info *arg_info)
{
  DBUG_ENTER("ACUWLlet");


  // If we are not in a Withloop yet
  if( INFO_OUTERWL( arg_info) == NULL) {
    LET_EXPR( arg_node) = TRAVdo( LET_EXPR(arg_node), arg_info);
  }
  // if we are already in a Withloop
  else {
    LET_IDS( arg_node) = TRAVopt( LET_IDS( arg_node), arg_info);
  }

  LET_EXPR( arg_node) = TRAVdo( LET_EXPR(arg_node), arg_info);

  LET_IDS( arg_node) = TRAVopt( LET_IDS( arg_node), arg_info);

  DBUG_RETURN( arg_node);
}
*/

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACUWLids( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
/*
node *ACUWLids( node *arg_node, info *arg_info)
{
  node *ids;
  int dim;

  DBUG_ENTER("ACUWLids");

  ids = arg_node;

  if( INFO_OUTERWL( arg_info) != NULL) {
    while( ids != NULL) {
      dim = TYgetDim( AVIS_TYPE( IDS_AVIS( ids)));
      if( dim > 0) {
        INFO_CUDARIZABLE( arg_info) = FALSE;
      }
      ids = IDS_NEXT( ids);
    }
  }
  DBUG_RETURN( arg_node);
}
*/

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACUWLcode( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACUWLcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLcode");

    if (INFO_OUTERWL (arg_info) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
        if (CODE_NEXT (arg_node) != NULL) {
            CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACUWLwith( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACUWLwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLwith");

    if (INFO_OUTERWL (arg_info) == NULL) {
        INFO_OUTERWL (arg_info) = arg_node;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_OUTERWL (arg_info) = NULL;
        WITH_CUDARIZABLE (arg_node) = INFO_CUDARIZABLE (arg_info);
        INFO_CUDARIZABLE (arg_info) = TRUE;
    } else {
        INFO_CUDARIZABLE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACUWLap( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACUWLap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLap");

    /* Found an AP within a Withloop. Not cudarizable */
    if (INFO_OUTERWL (arg_info) != NULL) {
        if (!STReq (NSgetModule (FUNDEF_NS (AP_FUNDEF (arg_node))), "Math")) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
        }
    } else {
        AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACUWLap( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACUWLprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLprf");

    switch (PRF_PRF (arg_node)) {
    case F_accu:
    case F_max_SxS:
        INFO_CUDARIZABLE (arg_info) = FALSE;
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
        break;
    default:
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
