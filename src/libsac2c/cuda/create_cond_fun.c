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
#include "namespaces.h"
#include "new_types.h"
#include "free.h"

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
                    node *out_mem, node **condfun_p)
{
    info *arg_info;
    node *dup_assigns;
    node *ap_assign;
    node *fundef_args, *fundef_rets, *fundef_body;
    char *fundef_name;
    namespace_t *fundef_ns;
    node *cond_ass, *phi_ass, *return_mem, *return_ass, *return_node;
    dfmask_t *in_mask;

    DBUG_ENTER ("CCFdoCreateCondFun");

    arg_info = MakeInfo ();
    TRAVpush (TR_ccf);

    INFO_DUPLUT (arg_info) = LUTgenerateLut ();

    /* Add the prediacte variable to the in mask */
    in_mask = INFDFMSdoInferInDfmAssignChain (assigns, fundef);
    DFMsetMaskEntrySet (in_mask, NULL, predicate);

    /* Put all args into the loop up table */
    fundef_args = DFMUdfm2Args (in_mask, INFO_DUPLUT (arg_info));

    assigns = TRAVdo (assigns, arg_info);

    /* Create actual conditional function */
    fundef_rets = TBmakeRet (TYcopyType (AVIS_TYPE (out_mem)), NULL);
    fundef_name = TRAVtmpVarName ("condfun");
    fundef_ns = NSdupNamespace (FUNDEF_NS (fundef));
    fundef_body = TBmakeBlock (NULL, NULL);

    *condfun_p = TBmakeFundef (fundef_name, fundef_ns, fundef_rets, fundef_args,
                               fundef_body, *condfun_p);

    FUNDEF_ISCONDFUN (*condfun_p) = TRUE;

    dup_assigns = DUPdoDupTreeLutSsa (assigns, INFO_DUPLUT (arg_info), *condfun_p);

    cond_ass
      = TBmakeAssign (TBmakeCond (TBmakeId (
                                    LUTsearchInLutPp (INFO_DUPLUT (arg_info), predicate)),
                                  TBmakeBlock (dup_assigns, NULL),
                                  TBmakeBlock (TBmakeEmpty (), NULL)),
                      NULL);

    return_mem = TBmakeAvis (TRAVtmpVarName ("shmem"), TYcopyType (AVIS_TYPE (out_mem)));

    INFO_VARDECS (arg_info) = TBmakeVardec (return_mem, INFO_VARDECS (arg_info));

    phi_ass
      = TBmakeAssign (TBmakeLet (TBmakeIds (return_mem, NULL),
                                 TBmakeFuncond (TBmakeId (LUTsearchInLutPp (INFO_DUPLUT (
                                                                              arg_info),
                                                                            predicate)),
                                                TBmakeId (LUTsearchInLutPp (INFO_DUPLUT (
                                                                              arg_info),
                                                                            out_mem)),
                                                TBmakeId (LUTsearchInLutPp (INFO_DUPLUT (
                                                                              arg_info),
                                                                            in_mem)))),
                      NULL);
    AVIS_SSAASSIGN (return_mem) = phi_ass;

    return_node = TBmakeReturn (TBmakeExprs (TBmakeId (return_mem), NULL));
    return_ass = TBmakeAssign (return_node, NULL);

    /* Chain up the assigns */
    ASSIGN_NEXT (phi_ass) = return_ass;
    ASSIGN_NEXT (cond_ass) = phi_ass;

    FUNDEF_INSTR (*condfun_p) = cond_ass;
    FUNDEF_VARDEC (*condfun_p) = INFO_VARDECS (arg_info);
    INFO_VARDECS (arg_info) = NULL;
    FUNDEF_RETURN (*condfun_p) = return_node;

    /* Create function application in the calling context */
    ap_assign
      = TBmakeAssign (TBmakeLet (TBmakeIds (out_mem, NULL),
                                 TBmakeAp (*condfun_p, DFMUdfm2ApArgs (in_mask, NULL))),
                      NULL);

    AVIS_SSAASSIGN (out_mem) = ap_assign;

    INFO_DUPLUT (arg_info) = LUTremoveLut (INFO_DUPLUT (arg_info));

    assigns = FREEdoFreeTree (assigns);

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

    if (LUTsearchInLutPp (INFO_DUPLUT (arg_info), IDS_AVIS (arg_node))
        == IDS_AVIS (arg_node)) {
        dup_avis = DUPdoDupNode (IDS_AVIS (arg_node));

        AVIS_SSAASSIGN (dup_avis) = NULL;

        INFO_DUPLUT (arg_info)
          = LUTinsertIntoLutP (INFO_DUPLUT (arg_info), IDS_AVIS (arg_node), dup_avis);

        INFO_VARDECS (arg_info) = TBmakeVardec (dup_avis, INFO_VARDECS (arg_info));
    }

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
    node *dup_avis;

    DBUG_ENTER ("CCFids");

    if (LUTsearchInLutPp (INFO_DUPLUT (arg_info), ID_AVIS (arg_node))
        == ID_AVIS (arg_node)) {

        dup_avis = DUPdoDupNode (ID_AVIS (arg_node));
        AVIS_SSAASSIGN (dup_avis) = NULL;

        INFO_DUPLUT (arg_info)
          = LUTinsertIntoLutP (INFO_DUPLUT (arg_info), ID_AVIS (arg_node), dup_avis);
    }
    DBUG_RETURN (arg_node);
}
