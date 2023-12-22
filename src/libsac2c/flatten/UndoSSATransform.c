/*****************************************************************************
 *
 * file:   UndoSSATransform.c
 *
 * prefix: USSA
 *
 * description:
 *
 *****************************************************************************/

#include "types.h"
#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "USSA"
#include "debug.h"

#include "traverse.h"
#include "free.h"
#include "UndoSSATransform.h"
#include "print.h"
#include "globals.h"

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
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_REMASSIGN (result) = FALSE;
    INFO_THENASS (result) = NULL;
    INFO_APPENDELSE (result) = FALSE;
    INFO_ELSEASS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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

    DBUG_ENTER ();

    /* process args */
    tmp = FUNDEF_ARGS (fundef);
    while (tmp != NULL) {
        AVIS_SUBST (ARG_AVIS (tmp)) = NULL;
        tmp = ARG_NEXT (tmp);
    }

    /* process vardecs */
    if (FUNDEF_BODY (fundef) != NULL) {
        tmp = BLOCK_VARDECS (FUNDEF_BODY (fundef));
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

    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        if (FUNDEF_ISLACFUN (arg_node)) {
            arg_node = UssaInitAvisFlags (arg_node);

            INFO_FUNDEF (arg_info) = arg_node;
            INFO_LHS (arg_info) = NULL;
            INFO_REMASSIGN (arg_info) = FALSE;
            INFO_THENASS (arg_info) = NULL;
            INFO_ELSEASS (arg_info) = NULL;

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        } else {
            FUNDEF_VARDECS (arg_node) = TRAVopt (FUNDEF_VARDECS (arg_node), arg_info);
        }
    }

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *USSATblock(node *arg_node, info *arg_info)
 *
 * description:
 *  traverses BLOCK_ASSIGNS and VARDECs
 *
 ******************************************************************************/
node *
USSATblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt(BLOCK_ASSIGNS (arg_node), arg_info);

    BLOCK_VARDECS (arg_node) = TRAVopt(BLOCK_VARDECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSATvardec(node *arg_node, info *arg_info)
 *
 * description:
 *   remove unnecessary vardecs
 *   kill SSAASSIGN links
 *
 ******************************************************************************/

node *
USSATvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    VARDEC_AVIS (arg_node) = TRAVdo (VARDEC_AVIS (arg_node), arg_info);

    VARDEC_NEXT (arg_node) = TRAVopt(VARDEC_NEXT (arg_node), arg_info);

    if (AVIS_SUBST (VARDEC_AVIS (arg_node)) != NULL) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSATavis(node *arg_node, info *arg_info)
 *
 * description:
 *   remove unnecessary aviss
 *
 ******************************************************************************/

node *
USSATavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_SSAASSIGN (arg_node) = NULL;

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
    DBUG_ENTER ();

    if (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        DBUG_PRINT ("rename id %s(" F_PTR ") in %s(" F_PTR ")",
                    AVIS_NAME (ID_AVIS (arg_node)), (void *)ID_AVIS (arg_node),
                    AVIS_NAME (AVIS_SUBST (ID_AVIS (arg_node))),
                    (void *)AVIS_SUBST (ID_AVIS (arg_node)));

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
    DBUG_ENTER ();

    if (AVIS_SUBST (IDS_AVIS (arg_ids)) != NULL) {
        DBUG_PRINT ("rename ids %s(" F_PTR ") in %s(" F_PTR ")",
                    AVIS_NAME (IDS_AVIS (arg_ids)), (void *)IDS_AVIS (arg_ids),
                    AVIS_NAME (AVIS_SUBST (IDS_AVIS (arg_ids))),
                    (void *)AVIS_SUBST (IDS_AVIS (arg_ids)));

        /* restore rename back to undo vardec */
        IDS_AVIS (arg_ids) = AVIS_SUBST (IDS_AVIS (arg_ids));
    }

    IDS_NEXT (arg_ids) = TRAVopt(IDS_NEXT (arg_ids), arg_info);

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
    DBUG_ENTER ();

    /*
     * Bottom-up traversal
     */
    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    INFO_REMASSIGN (arg_info) = FALSE;

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

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
    DBUG_ENTER ();

    LET_IDS (arg_node) = TRAVopt(LET_IDS (arg_node), arg_info);

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
    DBUG_ENTER ();

    /*
     * After all informations from the funconds have been annotated,
     * perform renaming in the THENASS, ELSEASS assigment chains
     */
    INFO_THENASS (arg_info) = TRAVopt(INFO_THENASS (arg_info), arg_info);

    INFO_ELSEASS (arg_info) = TRAVopt(INFO_ELSEASS (arg_info), arg_info);

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

        BLOCK_ASSIGNS (COND_THEN (arg_node))
          = TCappendAssign (BLOCK_ASSIGNS (COND_THEN (arg_node)),
                            INFO_THENASS (arg_info));
        INFO_THENASS (arg_info) = NULL;
    }

    /*
     * In cond-functions only:
     * Attach ELSEASS to the ELSE-branch
     */
    if (FUNDEF_ISCONDFUN (INFO_FUNDEF (arg_info))) {
        if (INFO_ELSEASS (arg_info) != NULL) {

            BLOCK_ASSIGNS (COND_ELSE (arg_node))
              = TCappendAssign (BLOCK_ASSIGNS (COND_ELSE (arg_node)),
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
 * @fn bool IdGivenByFillOperation( node *id)
 *
 ******************************************************************************/
static bool
IdGivenByFillOperation (node *idavis)
{
    bool res = FALSE;

    DBUG_ENTER ();

    if (AVIS_SSAASSIGN (idavis) != NULL) {
        node *ids, *expr;

        ids = ASSIGN_LHS (AVIS_SSAASSIGN (idavis));
        expr = ASSIGN_RHS (AVIS_SSAASSIGN (idavis));

        switch (NODE_TYPE (expr)) {
        case N_prf:
            res = PRF_PRF (expr) == F_fill || PRF_PRF (expr) == F_guard;
            break;

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

            res = NODE_TYPE (ops) == N_genarray
                || NODE_TYPE (ops) == N_modarray
                || NODE_TYPE (ops) == N_break;
            res = res && (WITHOP_MEM (ops) != NULL);
        } break;

        case N_ap:
            if (FUNDEF_ISLOOPFUN (AP_FUNDEF (expr))) {
                res = TRUE;
            } else {
                node *rets = FUNDEF_RETS (AP_FUNDEF (expr));

                while ((IDS_AVIS (ids) != idavis) && (rets != NULL)) {
                    ids = IDS_NEXT (ids);
                    rets = RET_NEXT (rets);
                }

                if ((rets != NULL) && (RET_HASLINKSIGNINFO (rets))) {
                    node *args = FUNDEF_ARGS (AP_FUNDEF (expr));
                    while (args != NULL) {
                        if (ARG_HASLINKSIGNINFO (args)
                            && ARG_LINKSIGN (args) == RET_LINKSIGN (rets)) {
                            res = TRUE;
                        }
                        args = ARG_NEXT (args);
                    }
                }
            }
            break;

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

    DBUG_ENTER ();

    lhsavis = IDS_AVIS (INFO_LHS (arg_info));

    /*
     * Ex.: r = funcond( c, t, e);
     *
     * Try to replace the lhs occurences of t and e with r.
     *
     * This can only be performed iff
     *
     *  - the types of t,e are in fact subtypes of the type of r.
     *    Due to -noOPT we may face a situation like this:
     *    int res = ( bool{true} p ? int x : int[2] y);
     *    Here, type(e) !<= type(r) (cf. bug 441)
     *
     *  - t,e are not function arguments (RET_HASLINKSIGNINFO!!)
     *    They must thus be defined in the conditional itself
     *
     *  - t,e have not already been replaced
     *    This may happen in the presence of multiple funconds
     *
     *  - t,e are not given by fill-operations (after memory management)
     *    The renaming scheme applied by PREC:MMV for good reasons relies on the
     *    presence of a subsequent copy assignment
     *
     *  - t,e are not be given by a guard for the same reasons
     *    see IdGivenByFillOperation
     *
     *  - t,e are not given by genarray oder modarray with-loops
     *    for the same reason they must not be given by fill operations.
     *
     */

    /*
     * Try to trigger replacement of t with r
     */
    rhsavis = ID_AVIS (FUNCOND_THEN (arg_node));

    if ((NODE_TYPE (AVIS_DECL (rhsavis)) == N_arg)
        || (IdGivenByFillOperation (rhsavis) && FUNDEF_ISCONDFUN (INFO_FUNDEF (arg_info)))
        || !TYleTypes (AVIS_TYPE (rhsavis), AVIS_TYPE (lhsavis))
        || (AVIS_SUBST (rhsavis) != NULL)) {

        /*
         * At least one criterium is not met => insert copy assignment
         */
        if (TYleTypes (AVIS_TYPE (rhsavis), AVIS_TYPE (lhsavis))) {
            INFO_THENASS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL), TBmakeId (rhsavis)),
                              INFO_THENASS (arg_info));
        } else {
            INFO_THENASS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL),
                                         TCmakePrf1 (F_type_error,
                                                     TBmakeType (
                                                       TYmakeBottomType (STRcpy (
                                                         "The typechecker guaranteed the "
                                                         "first branch of"
                                                         " the conditional never to be "
                                                         "executed"))))),
                              INFO_THENASS (arg_info));
        }
    } else {
        AVIS_SUBST (rhsavis) = lhsavis;
    }

    /*
     * Try to trigger replacement of e with r
     */
    rhsavis = ID_AVIS (FUNCOND_ELSE (arg_node));

    if ((NODE_TYPE (AVIS_DECL (rhsavis)) == N_arg)
        || (IdGivenByFillOperation (rhsavis) && FUNDEF_ISCONDFUN (INFO_FUNDEF (arg_info)))
        || !TYleTypes (AVIS_TYPE (rhsavis), AVIS_TYPE (lhsavis))
        || (AVIS_SUBST (rhsavis) != NULL)) {

        /*
         * At least one criterium is not met => insert copy assignment
         */
        if (TYleTypes (AVIS_TYPE (rhsavis), AVIS_TYPE (lhsavis))) {
            INFO_ELSEASS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL), TBmakeId (rhsavis)),
                              INFO_ELSEASS (arg_info));
        } else {
            INFO_ELSEASS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL),
                                         TCmakePrf1 (F_type_error,
                                                     TBmakeType (
                                                       TYmakeBottomType (STRcpy (
                                                         "The typechecker guaranteed the "
                                                         "second branch of"
                                                         " the conditional never to be "
                                                         "executed"))))),
                              INFO_ELSEASS (arg_info));
        }
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

    DBUG_ENTER ();

    /* ast is no longer in ssaform */
    global.valid_ssaform = FALSE;

    arg_info = MakeInfo ();

    TRAVpush (TR_ussat);
    module = TRAVdo (module, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (module);
}

#undef DBUG_PREFIX
