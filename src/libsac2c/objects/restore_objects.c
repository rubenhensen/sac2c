#include "free.h"
#include "memory.h"
#include "new_types.h"
#include "print.h"
#include "str.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"

#define DBUG_PREFIX "RESO"
#include "debug.h"

#include "restore_objects.h"

/******************************************************************************
 *
 * @struct INFO
 *
 * @param INFO_LHS
 * @param INFO_DELETE
 * @param INFO_DOSPMD
 *
 ******************************************************************************/
struct INFO {
    node *lhs;
    bool delete;
    bool dospmd;
};

#define INFO_LHS(n) ((n)->lhs)
#define INFO_DELETE(n) ((n)->delete)
#define INFO_DOSPMD(n) ((n)->dospmd)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LHS (result) = NULL;
    INFO_DELETE (result) = FALSE;
    INFO_DOSPMD (result) = FALSE;

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
 * @fn node *ResetAvisSubst (node *vardecs)
 *
 * @brief Resets the AVIS_SUBST field of all N_avis sons of the given N_vardec
 * chain to NULL.
 *
 * @param vardecs a chain of N_vardec nodes.
 *
 * @return chain of initialised N_vardec nodes.
 *
 ******************************************************************************/
static node *
ResetAvisSubst (node *vardecs)
{
    DBUG_ENTER ();

    if (vardecs != NULL) {
        AVIS_SUBST (VARDEC_AVIS (vardecs)) = NULL;
        VARDEC_NEXT (vardecs) = ResetAvisSubst (VARDEC_NEXT (vardecs));
    }

    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * @fn node *DeleteSubstVardecs (node *vardecs)
 *
 * @brief Removes all vardecs that have a N_avis son with AVIS_SUBST set to
 * something other than NULL from the N_vardec chain.
 *
 * @param vardecs a chain of N_vardec nodes.
 *
 * @return Chain of N_vardec nodes with the mentioned vardecs removed.
 *
 ******************************************************************************/
static node *
DeleteSubstVardecs (node *vardecs)
{
    node *subst;

    DBUG_ENTER ();

    if (vardecs != NULL) {
        VARDEC_NEXT (vardecs) = DeleteSubstVardecs (VARDEC_NEXT (vardecs));

        subst = AVIS_SUBST (VARDEC_AVIS (vardecs));
        if (subst != NULL) {
            DBUG_PRINT ("freeing vardec %s with subst %s",
                        VARDEC_NAME (vardecs), OBJDEF_NAME (subst));
            vardecs = FREEdoFreeNode (vardecs);
        }
    }

    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * @fn node *StripArtificialArgs (node *args)
 *
 * @brief Removes all args that are marked as artificial from the given chain.
 *
 * @param args a chain of N_arg nodes.
 *
 * @return Cleaned chain of N_arg nodes.
 *
 ******************************************************************************/
static node *
StripArtificialArgs (node *args)
{
    DBUG_ENTER ();

    if (args != NULL) {
        ARG_NEXT (args) = StripArtificialArgs (ARG_NEXT (args));

        if (ARG_ISARTIFICIAL (args)) {
            DBUG_PRINT ("stripping artificial arg %s", ARG_NAME (args));
            args = FREEdoFreeNode (args);
        }
    }

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * @fn node *StripArtificialArgExprs (node *form_args, node *act_args)
 *
 * @brief Removes all expressions form the chain of actual args that have
 * a corresponding artificial argument in the chain of formal args.
 *
 * @param form_args Formal arguments (N_arg chain).
 * @param act_args Actual arguments (N_exprs chain).
 *
 * @return Cleaned actual args (N_exprs chain).
 *
 ******************************************************************************/
static node *
StripArtificialArgExprs (node *form_args, node *act_args)
{
    DBUG_ENTER ();

    while (form_args != NULL) {
        if (ARG_ISARTIFICIAL (form_args)) {
            DBUG_PRINT ("stripping actual arg %s with artificial arg %s",
                        ID_NAME (EXPRS_EXPR (act_args)), ARG_NAME (form_args));
            act_args = FREEdoFreeNode (act_args);
        }

        form_args = ARG_NEXT (form_args);
    }

    DBUG_RETURN (act_args);
}

/******************************************************************************
 *
 * @fn node *DeleteLHSforRHSobjects (node *lhs, node *rhs)
 *
 * @brief Removes all LHS ids form the chain that have a corresponding
 * RHS object.
 *
 * @param lhs left hand side of assignment (N_ids chain).
 * @param rhs right hand side expressions (N_exprs chain).
 *
 * @return New left hand side (N_ids chain).
 *
 ******************************************************************************/
node *
DeleteLHSforRHSobjects (node *lhs, node *rhs)
{
    node *prevlhs = NULL;
    node *lhs_out;

    DBUG_ENTER ();

    lhs_out = lhs;

    while (rhs != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (rhs)) == N_globobj) {
            // Remove lhs ids
            lhs = FREEdoFreeNode (lhs);
            if (prevlhs != NULL) {
                IDS_NEXT (prevlhs) = lhs;
            } else {
                lhs_out = lhs;
            }
        } else {
            // Iterate lhs ids
            prevlhs = lhs;
            lhs = IDS_NEXT (lhs);
        }

        // Iterate rhs expr
        rhs = EXPRS_NEXT (rhs);
    }

    DBUG_RETURN (lhs_out);
}

/******************************************************************************
 *
 * @fn node *DeleteRHSobjects (node *rhs)
 *
 * @brief Removes all RHS objects from the chain.
 *
 * @param lhs left hand side of assignment (N_ids chain).
 * @param rhs right hand side expressions (N_exprs chain).
 *
 * @return New left hand side (N_ids chain).
 *
 ******************************************************************************/
node *
DeleteRHSobjects (node *rhs)
{
    node *prevrhs = NULL;
    node *rhs_out = rhs;

    DBUG_ENTER ();

    while (rhs != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (rhs)) == N_globobj) {
            // Remove rhs object
            rhs = FREEdoFreeNode (rhs);
            if (prevrhs != NULL) {
                EXPRS_NEXT (prevrhs) = rhs;
            } else {
                rhs_out = rhs;
            }
        } else {
            // Iterate rhs
            prevrhs = rhs;
            rhs = EXPRS_NEXT (rhs);
        }
    }

    DBUG_RETURN (rhs_out);
}

/******************************************************************************
 *
 * @fn node *MarkArtificialArgs (node *fundef_args, node *ap_args)
 *
 * @brief Whoever wrote me, didn't comment me. However, it seems as if this
 * function propagates the ISARTIFICIAL property from actual to formal
 * arguments.
 *
 * @param fundef_args formal arguments.
 * @param ap_args actual arguments.
 *
 * @return the updated formal arguments.
 *
 ******************************************************************************/
static node *
MarkArtificialArgs (node *fundef_args, node *ap_args)
{
    node *avis;

    DBUG_ENTER ();

    if (fundef_args != NULL) {
        avis = ID_AVIS (EXPRS_EXPR (ap_args));
        if (NODE_TYPE (AVIS_DECL (avis)) == N_arg) {
            if (ARG_ISARTIFICIAL (AVIS_DECL (avis))) {
                DBUG_PRINT ("Marking %s", AVIS_NAME (avis));
                ARG_ISARTIFICIAL (fundef_args) = TRUE;
                ARG_OBJDEF (fundef_args) = ARG_OBJDEF (AVIS_DECL (avis));
            }
        }

        ARG_NEXT (fundef_args) = MarkArtificialArgs (ARG_NEXT (fundef_args),
                                                     EXPRS_NEXT (ap_args));
    }

    DBUG_RETURN (fundef_args);
}

/******************************************************************************
 *
 * @fn bool SignaturesIdenticalModuloArtificials (node *fun1, node *fun2)
 *
 * @brief Checks whether the signature of fun1 and fun2 are identical after
 * all artificial args and rets have been removed.
 *
 * @param fun1 first function to compare.
 * @param fun2 second function to compare.
 *
 * @return TRUE iff the signatures match.
 *
 ******************************************************************************/
static bool
SignaturesIdenticalModuloArtificials (node *fun1, node *fun2)
{
    node *rets1, *rets2, *args1, *args2;
    bool res = TRUE;

    DBUG_ENTER ();

    // First check return types
    rets1 = FUNDEF_RETS (fun1);
    rets2 = FUNDEF_RETS (fun2);

    while (res && (rets1 != NULL) && (rets2 != NULL)) {
        if (RET_ISARTIFICIAL (rets1)) {
            rets1 = RET_NEXT (rets1);
        } else if (RET_ISARTIFICIAL (rets2)) {
            rets2 = RET_NEXT (rets2);
        } else {
            res = TYeqTypes (RET_TYPE (rets1), RET_TYPE (rets2));
            rets1 = RET_NEXT (rets1);
            rets2 = RET_NEXT (rets2);
        }
    }

    res = res && (rets1 == NULL) && (rets2 == NULL);

    // Same for argument types
    args1 = FUNDEF_ARGS (fun1);
    args2 = FUNDEF_ARGS (fun2);

    while (res && (args1 != NULL) && (args2 != NULL)) {
        if (ARG_ISARTIFICIAL (args1)) {
            args1 = ARG_NEXT (args1);
        } else if (ARG_ISARTIFICIAL (args2)) {
            args2 = ARG_NEXT (args2);
        } else {
            res = TYeqTypes (ARG_NTYPE (args1), ARG_NTYPE (args2));
            args1 = ARG_NEXT (args1);
            args2 = ARG_NEXT (args2);
        }
    }

    res = res && (args1 == NULL) && (args2 == NULL);

    DBUG_RETURN (res);
}

node *
RESOid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    avis = ID_AVIS (arg_node);

    if (NODE_TYPE (AVIS_DECL (avis)) == N_arg) {
        if (ARG_ISARTIFICIAL (AVIS_DECL (avis))) {
            // Found a reference to an objdef-argument, so replace it
            DBUG_ASSERT (ARG_OBJDEF (AVIS_DECL (avis)) != NULL,
                         "found artificial arg without objdef pointer!");

            arg_node = FREEdoFreeNode (arg_node);
            arg_node = TBmakeGlobobj (ARG_OBJDEF (AVIS_DECL (avis)));
        }
    } else if (AVIS_SUBST (avis) != NULL) {
        // Found an alias to an objdef-argument, so replace it
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TBmakeGlobobj (AVIS_SUBST (avis));
    }

    DBUG_RETURN (arg_node);
}

node *
RESOap (node *arg_node, info *arg_info)
{
    node *fundef;

    DBUG_ENTER ();

    fundef = AP_FUNDEF (arg_node);
    DBUG_PRINT ("looking at application of %s", FUNDEF_NAME (fundef));
    DBUG_EXECUTE (PRTdoPrintNode (arg_node));

    if (FUNDEF_ISSPMDFUN (fundef)) {
        FUNDEF_ARGS (fundef) = MarkArtificialArgs (FUNDEF_ARGS (fundef),
                                                   AP_ARGS (arg_node));
    }

    AP_ARGS (arg_node) = StripArtificialArgExprs (FUNDEF_ARGS (fundef),
                                                  AP_ARGS (arg_node));

    /**
     * Unwrap function if necessary and possible. Be aware that functions may
     * get wrapped multiple times if they are imported more than once!
     * Furthermore, we can only unwrap the object wrapper if it has not been
     * specialized and the signatures still match.
     */
    while (FUNDEF_ISOBJECTWRAPPER (fundef)
           && SignaturesIdenticalModuloArtificials (fundef,
                                                    FUNDEF_IMPL (fundef))) {
        DBUG_ASSERT (FUNDEF_IMPL (fundef) != NULL,
                     "found object wrapper with FUNDEF_IMPL not set");
        DBUG_PRINT ("unwrapping application of %s to %s",
                    FUNDEF_NAME (fundef), FUNDEF_NAME (FUNDEF_IMPL (fundef)));
        fundef = FUNDEF_IMPL (fundef);
    }

    if (FUNDEF_ISSPMDFUN (fundef)) {
        INFO_DOSPMD (arg_info) = TRUE;
        fundef = TRAVdo (fundef, arg_info);
        INFO_DOSPMD (arg_info) = FALSE;
    }

    AP_FUNDEF (arg_node) = fundef;

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
RESOprf (node *arg_node, info *arg_info)
{
    node *lhs, *args, *decl;

    DBUG_ENTER ();

    lhs = INFO_LHS (arg_info);
    args = PRF_ARGS (arg_node);

    switch (PRF_PRF (arg_node)) {
    /**
     * x1', .., xn' = guard (x1, .., xn, p1, .., pm)
     */
    case F_guard:
        INFO_DELETE (arg_info) = TRUE;
        while (lhs != NULL) {
            decl = ID_DECL (EXPRS_EXPR (args));
            if (NODE_TYPE (decl) != N_arg || !ARG_ISARTIFICIAL (decl)) {
                EXPRS_EXPR (args) = TRAVdo (EXPRS_EXPR (args), arg_info);
                INFO_DELETE (arg_info) = FALSE;
            }

            lhs = IDS_NEXT (lhs);
            args = EXPRS_NEXT (args);
        }

        break;

    default:
        arg_node = TRAVsons (arg_node, arg_info);
        break;
    }

    DBUG_RETURN (arg_node);
}

node *
RESOlet (node *arg_node, info *arg_info)
{
    node *lhs, *rhs, *expr;

    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    arg_node = TRAVcont (arg_node, arg_info);

    lhs = LET_IDS (arg_node);
    rhs = LET_EXPR (arg_node);

    switch (NODE_TYPE (rhs)) {
    /**
     * Detect assignments of the form
     *   <ids> = <globobj>
     * and trigger the substitution if <ids>.
     * Furthermore, we can mark the assignment to be deleted.
     */
    case N_globobj:
        DBUG_ASSERT (AVIS_SUBST (IDS_AVIS (lhs)) == NULL
                     || AVIS_SUBST (IDS_AVIS (lhs)) == GLOBOBJ_OBJDEF (rhs),
                     "found an N_ids that is a potential alias for two objects");
        AVIS_SUBST (IDS_AVIS (lhs)) = GLOBOBJ_OBJDEF (rhs);
        INFO_DELETE (arg_info) = TRUE;
        break;

    /**
     * Detect assignments of the form
     *   <ids> = with (...) : <globobj> ...
     * and delete both, as it is an identity assignment.
     */
    case N_with:
    case N_with2: {
        expr = NODE_TYPE (rhs) == N_with ? WITH_CODE (rhs) : WITH2_CODE (rhs);

        LET_IDS (arg_node) = DeleteLHSforRHSobjects (lhs, CODE_CEXPRS (expr));

        // Delete RHS for every with generator
        while (expr != NULL) {
            CODE_CEXPRS (expr) = DeleteRHSobjects (CODE_CEXPRS (expr));
            expr = CODE_NEXT (expr);
        }
    } break;

    case N_prf: {
        node *args = PRF_ARGS (rhs);
        switch (PRF_PRF (rhs)) {
        /**
         * Detect assignments of the form
         *   <ids> = _prop_obj_in_ (iv, <globobj>);
         * or
         *   <ids> = _prop_obj_out_ (<globobj>);
         * and delete lhs where rhs is a globobj,
         * as it is an identity assignment.
         */
        case F_prop_obj_in:
        case F_prop_obj_out:
            expr = PRF_PRF (rhs) == F_prop_obj_in ? EXPRS_NEXT (args) : args;

            LET_IDS (arg_node) = DeleteLHSforRHSobjects (lhs, expr);
            expr = DeleteRHSobjects (expr);

            if (PRF_PRF (rhs) == F_prop_obj_in) {
                EXPRS_NEXT (args) = expr;
            } else {
                PRF_ARGS (rhs) = expr;
            }
            break;

        /**
         * Detect assignments of the form
         *   x1', .., xn' = guard (x1, .., xn, p1, .., pn)
         * and delete lhs where rhs is a globobj,
         * as it is an identity assignment.
         */
        case F_guard:
            INFO_DELETE (arg_info) = TRUE;
            while (lhs != NULL) {
                expr = EXPRS_EXPR (args);
                if (NODE_TYPE (expr) == N_globobj) {
                    AVIS_SUBST (IDS_AVIS (lhs)) = GLOBOBJ_OBJDEF (expr);
                } else {
                    INFO_DELETE (arg_info) = FALSE;
                }

                lhs = IDS_NEXT (lhs);
                args = EXPRS_NEXT (args);
            }
            break;

        default:
            break;
        }
    } break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

node *
RESOassign (node *arg_node, info *arg_info)
{
    bool delete;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    delete = INFO_DELETE (arg_info);
    INFO_DELETE (arg_info) = FALSE;

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    if (delete) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
RESOblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_VARDECS (arg_node) = ResetAvisSubst (BLOCK_VARDECS (arg_node));

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    BLOCK_VARDECS (arg_node) = DeleteSubstVardecs (BLOCK_VARDECS (arg_node));

    DBUG_RETURN (arg_node);
}

node *
RESOfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISSPMDFUN (arg_node) && !INFO_DOSPMD (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        DBUG_RETURN (arg_node);
    }

    // Process all bodies first
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    if (!FUNDEF_ISSPMDFUN (arg_node)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    // Then clean up the signatures
    FUNDEF_ARGS (arg_node) = StripArtificialArgs (FUNDEF_ARGS (arg_node));

    /**
     * Finally, we can remove those object wrappers where the signature of the
     * object wrapper still matches that of the wrapped function (modulo
     * artificial args), i.e., those, that have not been specialized.
     */
    if (FUNDEF_ISOBJECTWRAPPER (arg_node)
        && SignaturesIdenticalModuloArtificials (arg_node,
                                                 FUNDEF_IMPL (arg_node))) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
RESOmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /**
     * We have to traverse the fundefs first, as the bodies have to be
     * transformed prior to transforming the signatures!
     */
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    MODULE_FUNDECS (arg_node) = TRAVopt (MODULE_FUNDECS (arg_node), arg_info);

    MODULE_FUNSPECS (arg_node) = TRAVopt (MODULE_FUNSPECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RESOpropagate (node *arg_node, info *arg_info)
{
    node *arg;

    DBUG_ENTER ();

    PROPAGATE_NEXT (arg_node) = TRAVopt (PROPAGATE_NEXT (arg_node), arg_info);

    arg = AVIS_DECL (ID_AVIS (PROPAGATE_DEFAULT (arg_node)));
    if (NODE_TYPE (arg) == N_arg && ARG_ISARTIFICIAL (arg)) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
RESOdoRestoreObjects (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_reso);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
