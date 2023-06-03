/** <!--********************************************************************-->
 *
 * @file create_lac_fun.c
 *
 * prefix: CLACF
 *
 * description:
 *
 *****************************************************************************/

#include "create_lac_fun.h"
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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

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
 * @fn node *CLACFdoCreateLacFun( node *syntax_tree)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CLACFdoCreateLacFun (bool condfun, /* If true, we create cond fun, otherwise loop fun */
                     node *fundef, node *assigns,
                     node *predicate,  /* Used when create cond fun */
                     node *iterator,   /* Used when create do fun */
                     node *loop_bound, /* Used when create do fun */
                     node *in_mem, node *out_mem, node **lacfun_p)
{
    info *arg_info;
    node *dup_assigns;
    node *ap_assign;
    node *fundef_args, *fundef_rets, *fundef_body;
    char *fundef_name;
    namespace_t *fundef_ns;
    node *cond_ass, *phi_ass, *return_mem, *return_ass, *return_node;
    dfmask_t *in_mask;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    TRAVpush (TR_clacf);

    if (condfun) {
        INFO_DUPLUT (arg_info) = LUTgenerateLut ();

        /* Add the prediacte variable to the in mask */
        in_mask = INFDFMSdoInferInDfmAssignChain (assigns, fundef);
        DFMsetMaskEntrySet (in_mask, predicate);

        /* Put all args into the loop up table */
        fundef_args = DFMUdfm2Args (in_mask, INFO_DUPLUT (arg_info));

        assigns = TRAVdo (assigns, arg_info);

        /* Create actual conditional function */
        fundef_rets = TBmakeRet (TYcopyType (AVIS_TYPE (out_mem)), NULL);
        fundef_name = TRAVtmpVarName ("condfun");
        fundef_ns = NSdupNamespace (FUNDEF_NS (fundef));
        fundef_body = TBmakeBlock (NULL, NULL);

        *lacfun_p = TBmakeFundef (fundef_name, fundef_ns, fundef_rets, fundef_args,
                                  fundef_body, *lacfun_p);

        FUNDEF_ISCONDFUN (*lacfun_p) = TRUE;

        dup_assigns = DUPdoDupTreeLutSsa (assigns, INFO_DUPLUT (arg_info), *lacfun_p);

        cond_ass
          = TBmakeAssign (TBmakeCond (TBmakeId (LUTsearchInLutPp (INFO_DUPLUT (arg_info),
                                                                  predicate)),
                                      TBmakeBlock (dup_assigns, NULL),
                                      TBmakeBlock (NULL, NULL)),
                          NULL);

        return_mem
          = TBmakeAvis (TRAVtmpVarName ("shmem"), TYcopyType (AVIS_TYPE (out_mem)));

        INFO_VARDECS (arg_info) = TBmakeVardec (return_mem, INFO_VARDECS (arg_info));

        phi_ass
          = TBmakeAssign (TBmakeLet (TBmakeIds (return_mem, NULL),
                                     TBmakeFuncond (TBmakeId (
                                                      LUTsearchInLutPp (INFO_DUPLUT (
                                                                          arg_info),
                                                                        predicate)),
                                                    TBmakeId (
                                                      LUTsearchInLutPp (INFO_DUPLUT (
                                                                          arg_info),
                                                                        out_mem)),
                                                    TBmakeId (
                                                      LUTsearchInLutPp (INFO_DUPLUT (
                                                                          arg_info),
                                                                        in_mem)))),
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
        ap_assign = TBmakeAssign (TBmakeLet (TBmakeIds (out_mem, NULL),
                                             TBmakeAp (*lacfun_p,
                                                       DFMUdfm2ApArgs (in_mask, NULL))),
                                  NULL);

        AVIS_SSAASSIGN (out_mem) = ap_assign;

        INFO_DUPLUT (arg_info) = LUTremoveLut (INFO_DUPLUT (arg_info));

        assigns = FREEdoFreeTree (assigns);
    } else {
        node *new_iterator, *comp_val, *comp_predicate;
        node *inc_ass, *sub_ass, *comp_ass, *recursive_ap;
        node *recursive_ret;

        INFO_DUPLUT (arg_info) = LUTgenerateLut ();

        /* Add the prediacte variable to the in mask */
        in_mask = INFDFMSdoInferInDfmAssignChain (assigns, fundef);
        DFMsetMaskEntrySet (in_mask, iterator);
        DFMsetMaskEntrySet (in_mask, loop_bound);
        DFMsetMaskEntrySet (in_mask, in_mem);

        /* Put all args into the look up table */
        fundef_args = DFMUdfm2Args (in_mask, INFO_DUPLUT (arg_info));

        assigns = TRAVdo (assigns, arg_info);

        dup_assigns = DUPdoDupTreeLutSsa (assigns, INFO_DUPLUT (arg_info), *lacfun_p);

        /* Three assignments to work out loop termination condition */
        new_iterator = TBmakeAvis (TRAVtmpVarName ("iterator"),
                                   TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

        INFO_VARDECS (arg_info) = TBmakeVardec (new_iterator, INFO_VARDECS (arg_info));
        inc_ass = TBmakeAssign (
          TBmakeLet (TBmakeIds (new_iterator, NULL),
                     TBmakePrf (F_add_SxS,
                                TBmakeExprs (TBmakeId (
                                               LUTsearchInLutPp (INFO_DUPLUT (arg_info),
                                                                 iterator)),
                                             TBmakeExprs (TBmakeNum (1), NULL)))),
          NULL);
        AVIS_SSAASSIGN (new_iterator) = inc_ass;

        comp_val = TBmakeAvis (TRAVtmpVarName ("comp_val"),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        INFO_VARDECS (arg_info) = TBmakeVardec (comp_val, INFO_VARDECS (arg_info));
        sub_ass = TBmakeAssign (
          TBmakeLet (TBmakeIds (comp_val, NULL),
                     TBmakePrf (F_sub_SxS,
                                TBmakeExprs (TBmakeId (new_iterator),
                                             TBmakeExprs (TBmakeId (
                                                            LUTsearchInLutPp (INFO_DUPLUT (
                                                                                arg_info),
                                                                              loop_bound)),
                                                          NULL)))),
          NULL);
        AVIS_SSAASSIGN (new_iterator) = sub_ass;

        comp_predicate
          = TBmakeAvis (TRAVtmpVarName ("iterator"),
                        TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

        INFO_VARDECS (arg_info) = TBmakeVardec (comp_predicate, INFO_VARDECS (arg_info));
        comp_ass
          = TBmakeAssign (TBmakeLet (TBmakeIds (comp_predicate, NULL),
                                     TBmakePrf (F_lt_SxS,
                                                TBmakeExprs (TBmakeId (comp_val),
                                                             TBmakeExprs (TBmakeNum (0),
                                                                          NULL)))),
                          NULL);
        AVIS_SSAASSIGN (new_iterator) = comp_ass;

        ASSIGN_NEXT (sub_ass) = comp_ass;
        ASSIGN_NEXT (inc_ass) = sub_ass;
        dup_assigns = TCappendAssign (dup_assigns, inc_ass);
        /************************************************************/

        /* Create actual loop function */
        fundef_rets = TBmakeRet (TYcopyType (AVIS_TYPE (out_mem)), NULL);
        fundef_name = TRAVtmpVarName ("loopfun");
        fundef_ns = NSdupNamespace (FUNDEF_NS (fundef));
        fundef_body = TBmakeBlock (NULL, NULL);

        *lacfun_p = TBmakeFundef (fundef_name, fundef_ns, fundef_rets, fundef_args,
                                  fundef_body, *lacfun_p);

        FUNDEF_ISLOOPFUN (*lacfun_p) = TRUE;

        /* Compute the argument list of the recursive function call */
        node *recursive_args = NULL;
        while (fundef_args != NULL) {
            if (ARG_AVIS (fundef_args)
                == LUTsearchInLutPp (INFO_DUPLUT (arg_info), in_mem)) {
                if (recursive_args == NULL) {
                    recursive_args
                      = TBmakeExprs (TBmakeId (LUTsearchInLutPp (INFO_DUPLUT (arg_info),
                                                                 out_mem)),
                                     NULL);
                } else {
                    recursive_args
                      = TCappendExprs (recursive_args,
                                       TBmakeExprs (TBmakeId (
                                                      LUTsearchInLutPp (INFO_DUPLUT (
                                                                          arg_info),
                                                                        out_mem)),
                                                    NULL));
                }
            } else if (ARG_AVIS (fundef_args)
                       == LUTsearchInLutPp (INFO_DUPLUT (arg_info), iterator)) {
                if (recursive_args == NULL) {
                    recursive_args = TBmakeExprs (TBmakeId (new_iterator), NULL);
                } else {
                    recursive_args
                      = TCappendExprs (recursive_args,
                                       TBmakeExprs (TBmakeId (new_iterator), NULL));
                }
            } else {
                if (recursive_args == NULL) {
                    recursive_args
                      = TBmakeExprs (TBmakeId (ARG_AVIS (fundef_args)), NULL);
                } else {
                    recursive_args
                      = TCappendExprs (recursive_args,
                                       TBmakeExprs (TBmakeId (ARG_AVIS (fundef_args)),
                                                    NULL));
                }
            }

            fundef_args = ARG_NEXT (fundef_args);
        }
        /************************************************************/

        recursive_ret
          = TBmakeAvis (TRAVtmpVarName ("shmem"), TYcopyType (AVIS_TYPE (out_mem)));
        INFO_VARDECS (arg_info) = TBmakeVardec (recursive_ret, INFO_VARDECS (arg_info));
        recursive_ap = TBmakeAp (*lacfun_p, recursive_args);
        AP_ISRECURSIVEDOFUNCALL (recursive_ap) = TRUE;
        FUNDEF_LOOPRECURSIVEAP (*lacfun_p) = recursive_ap;
        recursive_ap
          = TBmakeAssign (TBmakeLet (TBmakeIds (recursive_ret, NULL), recursive_ap),
                          NULL);
        AVIS_SSAASSIGN (recursive_ret) = recursive_ap;

        /* Conditional containing the recursive call */
        cond_ass = TBmakeAssign (TBmakeCond (TBmakeId (comp_predicate),
                                             TBmakeBlock (recursive_ap, NULL),
                                             TBmakeBlock (NULL, NULL)),
                                 NULL);

        return_mem
          = TBmakeAvis (TRAVtmpVarName ("shmem"), TYcopyType (AVIS_TYPE (out_mem)));
        INFO_VARDECS (arg_info) = TBmakeVardec (return_mem, INFO_VARDECS (arg_info));

        phi_ass
          = TBmakeAssign (TBmakeLet (TBmakeIds (return_mem, NULL),
                                     TBmakeFuncond (TBmakeId (comp_predicate),
                                                    TBmakeId (recursive_ret),
                                                    TBmakeId (
                                                      LUTsearchInLutPp (INFO_DUPLUT (
                                                                          arg_info),
                                                                        out_mem)))),
                          NULL);
        AVIS_SSAASSIGN (return_mem) = phi_ass;

        return_node = TBmakeReturn (TBmakeExprs (TBmakeId (return_mem), NULL));
        return_ass = TBmakeAssign (return_node, NULL);

        /* Chain up the assigns */
        ASSIGN_NEXT (phi_ass) = return_ass;
        ASSIGN_NEXT (cond_ass) = phi_ass;

        dup_assigns = TCappendAssign (dup_assigns, cond_ass);

        FUNDEF_ASSIGNS (*lacfun_p) = dup_assigns;
        FUNDEF_VARDECS (*lacfun_p) = INFO_VARDECS (arg_info);
        INFO_VARDECS (arg_info) = NULL;
        FUNDEF_RETURN (*lacfun_p) = return_node;

        /* Create function application in the calling context */
        ap_assign = TBmakeAssign (TBmakeLet (TBmakeIds (out_mem, NULL),
                                             TBmakeAp (*lacfun_p,
                                                       DFMUdfm2ApArgs (in_mask, NULL))),
                                  NULL);

        AVIS_SSAASSIGN (out_mem) = ap_assign;

        INFO_DUPLUT (arg_info) = LUTremoveLut (INFO_DUPLUT (arg_info));

        assigns = FREEdoFreeTree (assigns);
    }

    TRAVpop ();
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (ap_assign);
}

/** <!--********************************************************************-->
 *
 * @fn node *CLACFassign( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CLACFassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CLACFids( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CLACFids (node *arg_node, info *arg_info)
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
 * @fn node *CLACFid( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CLACFid (node *arg_node, info *arg_info)
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
