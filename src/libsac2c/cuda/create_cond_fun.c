/** <!--********************************************************************-->
 *
 * @file create_cond_fun.c
 *
 * prefix: CCF
 *
 * description:
 *
 *****************************************************************************/

#include "create_cond_fun.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "dbug.h"
#include "memory.h"
#include "tree_compound.h"
#include "infer_dfms.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "LookUpTable.h"
#include "DupTree.h"

/*
 * INFO structure
 */
struct INFO {
    lut_t *duplut;
    node *vardecs;
};

#define INFO_DUPLUT(n) (n->duplut)
#define INFO_VARDECS(n) (n->vardecs)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_DUPLUT (result) = NULL;
    INFO_VARDECS (result) = NULL;

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
 * @fn node *CCFdoCreateCondFun( node *syntax_tree)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CCFdoCreateCondFun (node *fundef, node *assigns, node *predicate, node *in_mem,
                    node *out_mem)
{
    info *arg_info;
    node *ap_assign;
    dfmask_t *in_mask;

    DBUG_ENTER ("CCFdoCreateCondFun");

    arg_info = MakeInfo ();

    TRAVpush (TR_ccf);

    INFO_DUPLUT (arg_info) = LUTgenerateLut ();
    assigns = TRAVdo (assigns, arg_info);

    in_mask = INFDFMSdoInferInDfmAssignChain (assigns, fundef);
    DFMsetMaskEntrySet (in_mask, NULL, predicate);
    ap_assign
      = TBmakeAssign (TBmakeLet (TBmakeIds (out_mem, NULL),
                                 TBmakeAp (fundef, DFMUdfm2ApArgs (in_mask, NULL))),
                      NULL);

    TRAVpop ();
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (ap_assign);
}

/** <!--********************************************************************-->
 *
 * @fn node *CCFassign( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CCFassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CCFassign");

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CCFids( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CCFids (node *arg_node, info *arg_info)
{
    node *dup_avis;

    DBUG_ENTER ("CCFids");

    dup_avis = DUPdoDupNode (IDS_AVIS (arg_node));

    INFO_DUPLUT (arg_info)
      = LUTinsertIntoLutP (INFO_DUPLUT (arg_info), IDS_AVIS (arg_node), dup_avis);

    INFO_VARDECS (arg_info) = TBmakeVardec (dup_avis, INFO_VARDECS (arg_info));

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CCFid( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CCFid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CCFids");

    INFO_DUPLUT (arg_info)
      = LUTinsertIntoLutP (INFO_DUPLUT (arg_info), ID_AVIS (arg_node),
                           DUPdoDupNode (ID_AVIS (arg_node)));

    DBUG_RETURN (arg_node);
}
