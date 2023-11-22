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

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

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
#include "shape.h"

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
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_DUPLUT (result) = NULL;
    INFO_VARDECS (result) = NULL;

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
CCFdoCreateCondFun (node *fundef, node *then_assigns, node *else_assigns,
                    node *predicate, /* Used when create cond fun */
                    node *in_mem, node *then_out_mem, node *else_out_mem, node **lacfun_p)
{
    info *arg_info;
    node *then_dup_assigns = NULL, *else_dup_assigns = NULL;
    node *ap_assign;
    node *fundef_args, *fundef_rets, *fundef_body;
    char *fundef_name;
    namespace_t *fundef_ns;
    node *cond_ass, *phi_ass, *return_mem, *return_ass, *return_node;
    dfmask_t *in_mask;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    TRAVpush (TR_ccf);

    INFO_DUPLUT (arg_info) = LUTgenerateLut ();

    /* Add the prediacte variable to the in mask */
    in_mask = INFDFMSdoInferInDfmAssignChain (then_assigns, fundef);
    DFMsetMaskEntrySet (in_mask, predicate);

    if (else_assigns != NULL) {
        DFMsetMaskOr (in_mask, INFDFMSdoInferInDfmAssignChain (else_assigns, fundef));
    }

    /* Put all args into the loop up table */
    fundef_args = DFMUdfm2Args (in_mask, INFO_DUPLUT (arg_info));

    then_assigns = TRAVopt (then_assigns, arg_info);
    else_assigns = TRAVopt (else_assigns, arg_info);

    /* Create actual conditional function */
    fundef_rets = TBmakeRet (TYcopyType (AVIS_TYPE (in_mem)), NULL);
    fundef_name = TRAVtmpVarName ("condfun");
    fundef_ns = NSdupNamespace (FUNDEF_NS (fundef));
    fundef_body = TBmakeBlock (NULL, NULL);

    *lacfun_p = TBmakeFundef (fundef_name, fundef_ns, fundef_rets, fundef_args,
                              fundef_body, *lacfun_p);

    FUNDEF_ISCONDFUN (*lacfun_p) = TRUE;

    then_dup_assigns
      = DUPdoDupTreeLutSsa (then_assigns, INFO_DUPLUT (arg_info), *lacfun_p);

    if (else_assigns != NULL) {
        else_dup_assigns
          = DUPdoDupTreeLutSsa (else_assigns, INFO_DUPLUT (arg_info), *lacfun_p);
    }

    cond_ass
      = TBmakeAssign (TBmakeCond (TBmakeId (
                                    (node *)LUTsearchInLutPp (INFO_DUPLUT (arg_info),
                                                              predicate)),
                                  TBmakeBlock (then_dup_assigns, NULL),
                                  TBmakeBlock (else_dup_assigns, NULL)),
                      NULL);

    return_mem = TBmakeAvis (TRAVtmpVarName ("shmem"), TYcopyType (AVIS_TYPE (in_mem)));

    INFO_VARDECS (arg_info) = TBmakeVardec (return_mem, INFO_VARDECS (arg_info));

    phi_ass = TBmakeAssign (
      TBmakeLet (TBmakeIds (return_mem, NULL),
                 TBmakeFuncond (TBmakeId (
                                  (node *)LUTsearchInLutPp (INFO_DUPLUT (arg_info),
                                                            predicate)),
                                TBmakeId (
                                  (node *)LUTsearchInLutPp (INFO_DUPLUT (arg_info),
                                                            then_out_mem)),
                                TBmakeId (
                                  (node *)LUTsearchInLutPp (INFO_DUPLUT (arg_info),
                                                            (else_assigns == NULL
                                                               ? in_mem
                                                               : else_out_mem))))),
      NULL);
    AVIS_SSAASSIGN (return_mem) = phi_ass;

    return_node = TBmakeReturn (TBmakeExprs (TBmakeId (return_mem), NULL));
    return_ass = TBmakeAssign (return_node, NULL);

    /* Chain up the assigns */
    ASSIGN_NEXT (phi_ass) = return_ass;
    ASSIGN_NEXT (cond_ass) = phi_ass;

    FUNDEF_ASSIGNS (*lacfun_p) = cond_ass;
    FUNDEF_VARDECS (*lacfun_p) = INFO_VARDECS (arg_info);
    INFO_VARDECS (arg_info) = NULL;
    FUNDEF_RETURN (*lacfun_p) = return_node;

    /* Create function application in the calling context */
    ap_assign
      = TBmakeAssign (TBmakeLet (TBmakeIds (then_out_mem, NULL),
                                 TBmakeAp (*lacfun_p, DFMUdfm2ApArgs (in_mask, NULL))),
                      NULL);

    AVIS_SSAASSIGN (then_out_mem) = ap_assign;

    INFO_DUPLUT (arg_info) = LUTremoveLut (INFO_DUPLUT (arg_info));

    then_assigns = FREEdoFreeTree (then_assigns);
    else_assigns = FREEoptFreeTree(else_assigns);

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
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    if (LUTsearchInLutPp (INFO_DUPLUT (arg_info), ID_AVIS (arg_node))
        == ID_AVIS (arg_node)) {

        dup_avis = DUPdoDupNode (ID_AVIS (arg_node));
        AVIS_SSAASSIGN (dup_avis) = NULL;

        INFO_DUPLUT (arg_info)
          = LUTinsertIntoLutP (INFO_DUPLUT (arg_info), ID_AVIS (arg_node), dup_avis);
    }
    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
