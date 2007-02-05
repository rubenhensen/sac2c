/*****************************************************************************
 *
 * $Id$
 *
 * file:   UndoSSATransform.c
 *
 * prefix: USSA
 *
 * description:
 *
 *****************************************************************************/
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "UndoSSATransform.h"
#include "print.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *lhs;
    bool remassign;
    bool appendelse;
    node *thenass;
    node *elseass;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LHS(n) (n->lhs)
#define INFO_REMASSIGN(n) (n->remassign)
#define INFO_APPENDELSE(n) (n->appendelse)
#define INFO_THENASS(n) (n->thenass)
#define INFO_ELSEASS(n) (n->elseass)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_REMASSIGN (result) = FALSE;
    INFO_THENASS (result) = NULL;
    INFO_ELSEASS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   node *USSAInitAvisFlags(node *fundef)
 *
 * description:
 *   inits flags AVIS_SUBST needed for this module in all vardec and args.
 *
 ******************************************************************************/
static node *
UssaInitAvisFlags (node *fundef)
{
    node *tmp;

    DBUG_ENTER ("UssaInitAvisFlags");

    /* process args */
    tmp = FUNDEF_ARGS (fundef);
    while (tmp != NULL) {
        AVIS_SUBST (ARG_AVIS (tmp)) = NULL;
        tmp = ARG_NEXT (tmp);
    }

    /* process vardecs */
    if (FUNDEF_BODY (fundef) != NULL) {
        tmp = BLOCK_VARDEC (FUNDEF_BODY (fundef));
        while (tmp != NULL) {
            AVIS_SUBST (VARDEC_AVIS (tmp)) = NULL;
            tmp = VARDEC_NEXT (tmp);
        }
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *    node* USSATfundef(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
USSATfundef (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("USSATfundef");

    if ((FUNDEF_ISLACFUN (arg_node)) && (FUNDEF_BODY (arg_node) != NULL)) {

        arg_node = UssaInitAvisFlags (arg_node);

        INFO_FUNDEF (arg_info) = arg_node;
        INFO_LHS (arg_info) = NULL;
        INFO_REMASSIGN (arg_info) = FALSE;
        INFO_THENASS (arg_info) = NULL;
        INFO_ELSEASS (arg_info) = NULL;

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        if (FUNDEF_VARDEC (arg_node) != NULL) {
            FUNDEF_VARDEC (arg_node) = TRAVdo (FUNDEF_VARDEC (arg_node), arg_info);
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *USSATblock(node *arg_node, info *arg_info)
 *
 * description:
 *  traverses BLOCK_INSTR and VARDECs
 *
 ******************************************************************************/
node *
USSATblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSATblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSATvardec(node *arg_node, info *arg_info)
 *
 * description:
 *   remove unnecessary vardecs
 *
 ******************************************************************************/
node *
USSATvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSATvardec");

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    if ((VARDEC_AVIS (arg_node) == NULL)
        || (AVIS_SUBST (VARDEC_AVIS (arg_node)) != NULL)) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSATid(node *arg_node, info *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
USSATid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSATid");

    if (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        DBUG_PRINT ("USSA", ("rename id %s(" F_PTR ") in %s(" F_PTR ")",
                             AVIS_NAME (ID_AVIS (arg_node)), ID_AVIS (arg_node),
                             AVIS_NAME (AVIS_SUBST (ID_AVIS (arg_node))),
                             AVIS_SUBST (ID_AVIS (arg_node))));

        /* restore rename back to undo vardec */
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSATids(node *arg_ids, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
USSATids (node *arg_ids, info *arg_info)
{
    DBUG_ENTER ("USSATids");

    if (AVIS_SUBST (IDS_AVIS (arg_ids)) != NULL) {
        DBUG_PRINT ("USSA", ("rename ids %s(" F_PTR ") in %s(" F_PTR ")",
                             AVIS_NAME (IDS_AVIS (arg_ids)), IDS_AVIS (arg_ids),
                             AVIS_NAME (AVIS_SUBST (IDS_AVIS (arg_ids))),
                             AVIS_SUBST (IDS_AVIS (arg_ids))));

        /* restore rename back to undo vardec */
        IDS_AVIS (arg_ids) = AVIS_SUBST (IDS_AVIS (arg_ids));
    }

    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TRAVdo (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *    node* USSATassign(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses instruction and removes tagged assignments.
 *
 ******************************************************************************/
node *
USSATassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSATassign");

    /*
     * Bottom-up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_REMASSIGN (arg_info) = FALSE;

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if ((INFO_REMASSIGN (arg_info))) {
        arg_node = FREEdoFreeNode (arg_node);
        INFO_REMASSIGN (arg_info) = FALSE;
    }

    if ((INFO_APPENDELSE (arg_info))) {
        arg_node = TCappendAssign (INFO_ELSEASS (arg_info), arg_node);
        INFO_ELSEASS (arg_info) = NULL;
        INFO_APPENDELSE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSATlet(node *arg_node, info *arg_info)
 *
 * description:
 *   starts traversal in ids chain and rhs expression.
 *
 ******************************************************************************/
node *
USSATlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSATlet");

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSATcond(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
USSATcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSATcond");

    /*
     * After all informations from the funconds have been annotated,
     * perform renaming in the THENASS, ELSEASS assigment chains
     */
    if (INFO_THENASS (arg_info) != NULL) {
        INFO_THENASS (arg_info) = TRAVdo (INFO_THENASS (arg_info), arg_info);
    }

    if (INFO_ELSEASS (arg_info) != NULL) {
        INFO_ELSEASS (arg_info) = TRAVdo (INFO_ELSEASS (arg_info), arg_info);
    }

    /*
     * perform renaming in the branches of the conditional
     */
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    /*
     * Attach THENASS to the THEN-branch
     */
    if (INFO_THENASS (arg_info) != NULL) {
        DBUG_ASSERT (FUNDEF_ISCONDFUN (INFO_FUNDEF (arg_info)),
                     "Then branch of loop function must not be extended!");

        BLOCK_INSTR (COND_THEN (arg_node))
          = TCappendAssign (BLOCK_INSTR (COND_THEN (arg_node)), INFO_THENASS (arg_info));
        INFO_THENASS (arg_info) = NULL;
    }

    /*
     * In cond-functions only:
     * Attach ELSEASS to the ELSE-branch
     */
    if (FUNDEF_ISCONDFUN (INFO_FUNDEF (arg_info))) {
        if (INFO_ELSEASS (arg_info) != NULL) {

            BLOCK_INSTR (COND_ELSE (arg_node))
              = TCappendAssign (BLOCK_INSTR (COND_ELSE (arg_node)),
                                INFO_ELSEASS (arg_info));
            INFO_ELSEASS (arg_info) = NULL;
        }
    } else {
        INFO_APPENDELSE (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   bool IdGivenByFillOperation( node *id)
 *
 * description:
 *
 *****************************************************************************/
static bool
IdGivenByFillOperation (node *idavis)
{
    bool res = FALSE;

    DBUG_ENTER ("IdGivenByFillOperation");

    if (AVIS_SSAASSIGN (idavis) != NULL) {
        node *ids, *expr;

        ids = ASSIGN_LHS (AVIS_SSAASSIGN (idavis));
        expr = ASSIGN_RHS (AVIS_SSAASSIGN (idavis));

        switch (NODE_TYPE (expr)) {
        case N_prf: {
            res = (PRF_PRF (expr) == F_fill);
        } break;

        case N_with:
        case N_with2: {
            node *ops;

            if (NODE_TYPE (expr) == N_with) {
                ops = WITH_WITHOP (expr);
            } else {
                ops = WITH2_WITHOP (expr);
            }

            while (IDS_AVIS (ids) != idavis) {
                ids = IDS_NEXT (ids);
                ops = WITHOP_NEXT (ops);
            }

            res = (((NODE_TYPE (ops) == N_genarray) || (NODE_TYPE (ops) == N_modarray))
                   && (WITHOP_MEM (ops) != NULL));
        } break;

        case N_ap: {
            node *rets = FUNDEF_RETS (AP_FUNDEF (expr));

            while ((IDS_AVIS (ids) != idavis) && (rets != NULL)) {
                ids = IDS_NEXT (ids);
                rets = RET_NEXT (rets);
            }

            if ((rets != NULL) && (RET_HASLINKSIGNINFO (rets))) {
                node *args = FUNDEF_ARGS (AP_FUNDEF (expr));
                while (args != NULL) {
                    if (ARG_HASLINKSIGNINFO (args)
                        && (ARG_LINKSIGN (args) == RET_LINKSIGN (rets))) {
                        res = TRUE;
                    }
                    args = ARG_NEXT (args);
                }
            }
        } break;

        default:
            break;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *USSATfuncond(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
USSATfuncond (node *arg_node, info *arg_info)
{
    node *lhsavis;
    node *rhsavis;

    DBUG_ENTER ("USSATfuncond");

    lhsavis = IDS_AVIS (INFO_LHS (arg_info));

    /*
     * Ex.: r = funcond( c, t, e);
     *
     * Try to replace the lhs occurences of t and e with r.
     *
     * This can only be performed iff
     *
     *  - t,e are not function arguments
     *    They must thus be defined in the conditional itself
     *
     *  - t,e have not already been replaced
     *    This may happen in the presence of multiple funconds
     *
     *  - t,e are not given by fill-operations (after memory management)
     *    The renaming scheme applied by PREC:MMV for good reasons relies on the
     *    presence of a subsequent copy assignment
     *
     *  - t,e are not given by genarray oder modarray with-loops
     *    for the same reason they must not be given by fill operations.
     */

    /*
     * Try to trigger replacement of t with r
     */
    rhsavis = ID_AVIS (FUNCOND_THEN (arg_node));
    if ((NODE_TYPE (AVIS_DECL (rhsavis)) == N_arg) || (IdGivenByFillOperation (rhsavis))
        || (AVIS_SUBST (rhsavis) != NULL)) {

        /*
         * At least one criterium is not met => insert copy assignment
         */
        INFO_THENASS (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL), TBmakeId (rhsavis)),
                          INFO_THENASS (arg_info));
    } else {
        AVIS_SUBST (rhsavis) = lhsavis;
    }

    /*
     * Try to trigger replacement of e with r
     */
    rhsavis = ID_AVIS (FUNCOND_ELSE (arg_node));
    if ((NODE_TYPE (AVIS_DECL (rhsavis)) == N_arg) || (IdGivenByFillOperation (rhsavis))
        || (AVIS_SUBST (rhsavis) != NULL)) {

        /*
         * At least one criterium is not met => insert copy assignment
         */
        INFO_ELSEASS (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL), TBmakeId (rhsavis)),
                          INFO_ELSEASS (arg_info));
    } else {
        AVIS_SUBST (rhsavis) = lhsavis;
    }

    INFO_REMASSIGN (arg_info) = TRUE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* USSATdoUndoSsaTransform(node* module)
 *
 * description:
 *   Starts traversal of AST to restore original artificial identifier.
 *
 ******************************************************************************/
node *
USSATdoUndoSsaTransform (node *module)
{
    info *arg_info;

    DBUG_ENTER ("USSATdoUndoSSATransform");

    /* ast is no longer in ssaform */
    global.valid_ssaform = FALSE;

    arg_info = MakeInfo ();

    TRAVpush (TR_ussat);
    module = TRAVdo (module, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (module);
}
