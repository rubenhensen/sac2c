#include "restore_objects.h"

#define DBUG_PREFIX "RESO"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "str.h"
#include "memory.h"
#include "new_types.h"

/*
 * INFO structure
 */
struct INFO {
    bool mdelete;
    bool dospmd;
};

/*
 * INFO macros
 */
#define INFO_DELETE(n) ((n)->mdelete)
#define INFO_DOSPMD(n) ((n)->dospmd)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

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

/*
 * static helper functions
 */
/** <!-- ****************************************************************** -->
 * @fn node *ResetAvisSubst( node *vardecs)
 *
 * @brief Resets the AVIS_SUBST field of all AVIS sons of the given VARDEC
 *        chain to NULL.
 *
 * @param vardecs a chain of N_vardec nodes
 *
 * @return chain of initialised N_vardec nodes
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

/** <!-- ****************************************************************** -->
 * @fn node *DeleteSubstVardecs( node *vardecs)
 *
 * @brief Removes all vardecs that have a AVIS son with AVIS_SUBST set to
 *        something other than NULL from the vardec chain.
 *
 * @param vardecs a chain of N_vardec nodes
 *
 * @return chain of N_vardec nodes with the mentioned vardecs removed.
 ******************************************************************************/
static node *
DeleteSubstVardecs (node *vardecs)
{
    DBUG_ENTER ();

    if (vardecs != NULL) {
        VARDEC_NEXT (vardecs) = DeleteSubstVardecs (VARDEC_NEXT (vardecs));

        if (AVIS_SUBST (VARDEC_AVIS (vardecs)) != NULL) {
            vardecs = FREEdoFreeNode (vardecs);
        }
    }

    DBUG_RETURN (vardecs);
}

/** <!-- ****************************************************************** -->
 * @fn node *StripArtificialArgs( node *args)
 *
 * @brief Removes all args that are marked as artificial from the given
 *        chain.
 *
 * @param args a chain of N_arg nodes.
 *
 * @return Cleaned chain of N_arg nodes.
 ******************************************************************************/
static node *
StripArtificialArgs (node *args)
{
    DBUG_ENTER ();

    if (args != NULL) {
        ARG_NEXT (args) = StripArtificialArgs (ARG_NEXT (args));

        if (ARG_ISARTIFICIAL (args)) {
            args = FREEdoFreeNode (args);
        }
    }

    DBUG_RETURN (args);
}

/** <!-- ****************************************************************** -->
 * @fn node *StripArtificialArgExprs( node *form_args, node *act_args)
 *
 * @brief Removes all expressions form the chain of actual args that have
 *        a corresponding artificial argument in the chain of formal args.
 *
 * @param form_args formal args (N_arg chain)
 * @param act_args actual args (N_exprs chain)
 *
 * @return Cleaned actual args (N_exprs chain)
 ******************************************************************************/
static node *
StripArtificialArgExprs (node *form_args, node *act_args)
{
    DBUG_ENTER ();

    if (form_args != NULL) {
        if (ARG_ISARTIFICIAL (form_args)) {
            act_args = FREEdoFreeNode (act_args);
        }

        act_args = StripArtificialArgExprs (ARG_NEXT (form_args), act_args);
    }

    DBUG_RETURN (act_args);
}

/** <!-- ****************************************************************** -->
 * @fn node *DeleteLHSforRHSobjects( node *lhs, node *rhs)
 *
 * @brief Removes all LHS ids form the chain that have
 *        a corresponding RHS object.
 *
 * @param lhs left hand side of assignment (N_ids chain)
 * @param rhs right hand side expressions (N_exprs chain)
 *
 * @return New left hand side (N_ids chain)
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

            /* Remove lhs ids */
            lhs = FREEdoFreeNode (lhs);
            if (prevlhs != NULL) {
                IDS_NEXT (prevlhs) = lhs;
            } else {
                lhs_out = lhs;
            }

        } else {
            /* Iterate lhs ids */
            prevlhs = lhs;
            lhs = IDS_NEXT (lhs);
        }

        /* Iterate rhs expr */
        rhs = EXPRS_NEXT (rhs);
    }

    DBUG_RETURN (lhs_out);
}

/** <!-- ****************************************************************** -->
 * @fn node *DeleteRHSobjects( node *rhs)
 *
 * @brief Removes all RHS objects from the chain.
 *
 * @param lhs left hand side of assignment (N_ids chain)
 * @param rhs right hand side expressions (N_exprs chain)
 *
 * @return New left hand side (N_ids chain)
 ******************************************************************************/
node *
DeleteRHSobjects (node *rhs)
{
    node *prevrhs = NULL;
    node *rhs_out = rhs;

    DBUG_ENTER ();

    while (rhs != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (rhs)) == N_globobj) {

            /* Remove rhs object */
            rhs = FREEdoFreeNode (rhs);
            if (prevrhs != NULL) {
                EXPRS_NEXT (prevrhs) = rhs;
            } else {
                rhs_out = rhs;
            }

        } else {
            /* Iterate rhs */
            prevrhs = rhs;
            rhs = EXPRS_NEXT (rhs);
        }
    }

    DBUG_RETURN (rhs_out);
}

/** <!-- ****************************************************************** -->
 * @fn node * MarkArtificialArgs( node *fundef_args, node *ap_args)
 *
 * @brief Whoever wrote me, didn't comment me. However, it seems as if this
 *        function propagates the ISARTIFICIAL property from actual to
 *        formal arguments.
 *
 * @param fundef_args formal arguments
 * @param ap_args     actual arguments
 *
 * @return the updated formal arguments.
 ******************************************************************************/
static node *
MarkArtificialArgs (node *fundef_args, node *ap_args)
{
    DBUG_ENTER ();
    if (fundef_args != NULL) {
        node *avis = ID_AVIS (EXPRS_EXPR (ap_args));
        if (NODE_TYPE (AVIS_DECL (avis)) == N_arg) {
            if (ARG_ISARTIFICIAL (AVIS_DECL (avis))) {
                DBUG_PRINT ("Marking %s", AVIS_NAME (avis));
                ARG_ISARTIFICIAL (fundef_args) = TRUE;
                ARG_OBJDEF (fundef_args) = ARG_OBJDEF (AVIS_DECL (avis));
            }
        }
        ARG_NEXT (fundef_args)
          = MarkArtificialArgs (ARG_NEXT (fundef_args), EXPRS_NEXT (ap_args));
    }
    DBUG_RETURN (fundef_args);
}

/** <!-- ****************************************************************** -->
 * @fn bool SignaturesIdenticalModuloArtificials( node *fun1, node *fun2)
 *
 * @brief Checks whether the signature of fun1 and fun2 are identical after
 *        all artificial args and rets have been removed.
 *
 * @param fun1 first function to compare
 * @param fun2 second function to compare
 *
 * @return TRUE iff the signatures match
 ******************************************************************************/
static bool
SignaturesIdenticalModuloArtificials (node *fun1, node *fun2)
{
    bool result = TRUE;
    node *rets1, *rets2, *args1, *args2;

    DBUG_ENTER ();

    /* first check return types */
    rets1 = FUNDEF_RETS (fun1);
    rets2 = FUNDEF_RETS (fun2);

    while (result && (rets1 != NULL) && (rets2 != NULL)) {
        if (RET_ISARTIFICIAL (rets1)) {
            rets1 = RET_NEXT (rets1);
        } else if (RET_ISARTIFICIAL (rets2)) {
            rets2 = RET_NEXT (rets2);
        } else {
            result = TYeqTypes (RET_TYPE (rets1), RET_TYPE (rets2));
            rets1 = RET_NEXT (rets1);
            rets2 = RET_NEXT (rets2);
        }
    }
    result = result && (rets1 == NULL) && (rets2 == NULL);

    /* same for argument types */
    args1 = FUNDEF_ARGS (fun1);
    args2 = FUNDEF_ARGS (fun2);

    while (result && (args1 != NULL) && (args2 != NULL)) {
        if (ARG_ISARTIFICIAL (args1)) {
            args1 = ARG_NEXT (args1);
        } else if (ARG_ISARTIFICIAL (args2)) {
            args2 = ARG_NEXT (args2);
        } else {
            result = TYeqTypes (ARG_NTYPE (args1), ARG_NTYPE (args2));
            args1 = ARG_NEXT (args1);
            args2 = ARG_NEXT (args2);
        }
    }
    result = result && (args1 == NULL) && (args2 == NULL);

    DBUG_RETURN (result);
}

/*
 * traversal functions
 */
node *
RESOid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    avis = ID_AVIS (arg_node);

    if (NODE_TYPE (AVIS_DECL (avis)) == N_arg) {
        if (ARG_ISARTIFICIAL (AVIS_DECL (avis))) {
            /*
             * found a reference to an objdef-argument, so
             * replace it.
             */
            DBUG_ASSERT (ARG_OBJDEF (AVIS_DECL (avis)) != NULL,
                         "found artificial arg without objdef pointer!");

            arg_node = FREEdoFreeNode (arg_node);
            arg_node = TBmakeGlobobj (ARG_OBJDEF (AVIS_DECL (avis)));
        }
    } else if (AVIS_SUBST (avis) != NULL) {
        /*
         * found an alias to an objdef-argument, so replace it.
         */
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TBmakeGlobobj (AVIS_SUBST (avis));
    }

    DBUG_RETURN (arg_node);
}

node *
RESOap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISSPMDFUN (AP_FUNDEF (arg_node))) {
        FUNDEF_ARGS (AP_FUNDEF (arg_node))
          = MarkArtificialArgs (FUNDEF_ARGS (AP_FUNDEF (arg_node)), AP_ARGS (arg_node));
    }

    AP_ARGS (arg_node)
      = StripArtificialArgExprs (FUNDEF_ARGS (AP_FUNDEF (arg_node)), AP_ARGS (arg_node));

    /*
     * unwrap function if necessary and possible.
     * be aware that functions may get wrapped multiple times
     * if they are imported more than once!
     * furthermore, we can only unwrap the object wrapper if it
     * has not been specialized and the signatures still match.
     */
    while (FUNDEF_ISOBJECTWRAPPER (AP_FUNDEF (arg_node))
           && SignaturesIdenticalModuloArtificials (AP_FUNDEF (arg_node),
                                                    FUNDEF_IMPL (AP_FUNDEF (arg_node)))) {
        DBUG_ASSERT (FUNDEF_IMPL (AP_FUNDEF (arg_node)) != NULL,
                     "found object wrapper with FUNDEF_IMPL not set!");
        AP_FUNDEF (arg_node) = FUNDEF_IMPL (AP_FUNDEF (arg_node));
    }

    if (FUNDEF_ISSPMDFUN (AP_FUNDEF (arg_node))) {
        INFO_DOSPMD (arg_info) = TRUE;
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        INFO_DOSPMD (arg_info) = FALSE;
    }
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
RESOprf (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_afterguard:
        avis = ID_AVIS (PRF_ARG1 (arg_node));

        if ((NODE_TYPE (AVIS_DECL (avis)) == N_arg)
            && (ARG_ISARTIFICIAL (AVIS_DECL (avis)))) {
            INFO_DELETE (arg_info) = TRUE;
        } else {
            arg_node = TRAVsons (arg_node, arg_info);
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
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    /*
     * detect assignments of the form
     *
     * <ids> = <globobj>
     *
     * and trigger the substitution if
     * <ids>. Furthermore, we can mark the
     * assignment to be deleted.
     */
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_globobj) {
        DBUG_ASSERT (((AVIS_SUBST (IDS_AVIS (LET_IDS (arg_node))) == NULL)
                      || (AVIS_SUBST (IDS_AVIS (LET_IDS (arg_node)))
                          == GLOBOBJ_OBJDEF (LET_EXPR (arg_node)))),
                     "found an ids node that is a potential alias for two objects!");

        AVIS_SUBST (IDS_AVIS (LET_IDS (arg_node))) = GLOBOBJ_OBJDEF (LET_EXPR (arg_node));

        INFO_DELETE (arg_info) = TRUE;
    }

    /*
     * detect assignments of the form
     *
     * <ids> = with ... : <globobj> ...
     *
     * and delete both, as it is an identity assignment.
     */
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_with
        || NODE_TYPE (LET_EXPR (arg_node)) == N_with2) {
        node *with_code;

        if (NODE_TYPE (LET_EXPR (arg_node)) == N_with) {
            with_code = WITH_CODE (LET_EXPR (arg_node));
        } else {
            with_code = WITH2_CODE (LET_EXPR (arg_node));
        }

        LET_IDS (arg_node)
          = DeleteLHSforRHSobjects (LET_IDS (arg_node), CODE_CEXPRS (with_code));

        /* Delete RHS for every with generator! */
        while (with_code != NULL) {
            CODE_CEXPRS (with_code) = DeleteRHSobjects (CODE_CEXPRS (with_code));
            with_code = CODE_NEXT (with_code);
        }
    }

    /*
     * detect assignments of the form
     *
     * <ids> = F_prop_obj_in( iv, <globobj> );
     *   or
     * <ids> = F_prop_obj_out( <globobj> );
     *
     * and delete lhs where rhs is a globobj, as it is
     * and identity assignment.
     */
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_prf
        && (PRF_PRF (LET_EXPR (arg_node)) == F_prop_obj_in
            || PRF_PRF (LET_EXPR (arg_node)) == F_prop_obj_out)) {
        node *prf_args;
        node *exprs;

        prf_args = PRF_ARGS (LET_EXPR (arg_node));

        if (PRF_PRF (LET_EXPR (arg_node)) == F_prop_obj_in) {
            exprs = EXPRS_NEXT (prf_args);
        } else {
            exprs = prf_args;
        }

        LET_IDS (arg_node) = DeleteLHSforRHSobjects (LET_IDS (arg_node), exprs);
        exprs = DeleteRHSobjects (exprs);

        if (PRF_PRF (LET_EXPR (arg_node)) == F_prop_obj_in) {
            EXPRS_NEXT (prf_args) = exprs;
        } else {
            PRF_ARGS (LET_EXPR (arg_node)) = exprs;
        }
    }

    /*
     * detect assignments of the form
     *
     * <ids> = F_afterguard( <globobj>, ...);
     *
     * and delete lhs as it is
     * and identity assignment.
     */
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_prf
        && (PRF_PRF (LET_EXPR (arg_node)) == F_afterguard)
        && (NODE_TYPE (PRF_ARG1 (LET_EXPR (arg_node))) == N_globobj)) {
        AVIS_SUBST (IDS_AVIS (LET_IDS (arg_node)))
          = GLOBOBJ_OBJDEF (PRF_ARG1 (LET_EXPR (arg_node)));

        INFO_DELETE (arg_info) = TRUE;
    }
    DBUG_RETURN (arg_node);
}

node *
RESOassign (node *arg_node, info *arg_info)
{
    bool xdelete;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    xdelete = INFO_DELETE (arg_info);
    INFO_DELETE (arg_info) = FALSE;

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    if (xdelete) {
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
        FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);
        DBUG_RETURN (arg_node);
    }

    /*
     * process all bodies first
     */
    FUNDEF_BODY (arg_node) = TRAVopt(FUNDEF_BODY (arg_node), arg_info);

    if (FUNDEF_NEXT (arg_node) != NULL && !FUNDEF_ISSPMDFUN (arg_node)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     * then clean up the signatures
     */
    FUNDEF_ARGS (arg_node) = StripArtificialArgs (FUNDEF_ARGS (arg_node));

    /*
     * finally, we can remove those object wrappers where the signature of the
     * object wrapper still matches that of the wrapped function (modulo
     * artificial args), i.e., those, that have not been specialized.
     */
    if (FUNDEF_ISOBJECTWRAPPER (arg_node)
        && SignaturesIdenticalModuloArtificials (arg_node, FUNDEF_IMPL (arg_node))) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
RESOmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * we have to traverse the fundefs first, as the bodies have
     * to be transformed prior to transforming the signatures!
     */
    MODULE_FUNS (arg_node) = TRAVopt(MODULE_FUNS (arg_node), arg_info);

    MODULE_FUNDECS (arg_node) = TRAVopt(MODULE_FUNDECS (arg_node), arg_info);

    MODULE_FUNSPECS (arg_node) = TRAVopt(MODULE_FUNSPECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RESOpropagate (node *arg_node, info *arg_info)
{
    node *arg;

    DBUG_ENTER ();

    PROPAGATE_NEXT (arg_node) = TRAVopt(PROPAGATE_NEXT (arg_node), arg_info);

    arg = AVIS_DECL (ID_AVIS (PROPAGATE_DEFAULT (arg_node)));

    if (NODE_TYPE (arg) == N_arg && ARG_ISARTIFICIAL (arg)) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/*
 * traversal start function
 */

node *
RESOdoRestoreObjects (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_reso);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
