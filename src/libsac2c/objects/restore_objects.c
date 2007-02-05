/* $Id$ */

#include "restore_objects.h"
#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "str.h"
#include "memory.h"
/*
 * INFO structure
 */
struct INFO {
    bool delete;
    bool dospmd;
};

/*
 * INFO macros
 */
#define INFO_DELETE(n) ((n)->delete)
#define INFO_DOSPMD(n) ((n)->dospmd)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_DELETE (result) = FALSE;
    INFO_DOSPMD (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

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
    DBUG_ENTER ("ResetAvisSubst");

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
    DBUG_ENTER ("DeleteSubstVardecs");

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
    DBUG_ENTER ("StripArtificialArgs");

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
    DBUG_ENTER ("StripArtificialArgExprs");

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

    DBUG_ENTER ("DeleteLHSforRHSobjects");

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

    DBUG_ENTER ("DeleteRHSobjects");

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

static node *
MarkArtificialArgs (node *fundef_args, node *ap_args)
{
    DBUG_ENTER ("MarkArtificialArgs");
    if (fundef_args != NULL) {
        node *avis = ID_AVIS (EXPRS_EXPR (ap_args));
        if (NODE_TYPE (AVIS_DECL (avis)) == N_arg) {
            if (ARG_ISARTIFICIAL (AVIS_DECL (avis))) {
                DBUG_PRINT ("RESO", ("Marking %s", AVIS_NAME (avis)));
                ARG_ISARTIFICIAL (fundef_args) = TRUE;
                ARG_OBJDEF (fundef_args) = ARG_OBJDEF (AVIS_DECL (avis));
            }
        }
        ARG_NEXT (fundef_args)
          = MarkArtificialArgs (ARG_NEXT (fundef_args), EXPRS_NEXT (ap_args));
    }
    DBUG_RETURN (fundef_args);
}

/*
 * traversal functions
 */
node *
RESOid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("RESOid");

    avis = ID_AVIS (arg_node);

    if (NODE_TYPE (AVIS_DECL (avis)) == N_arg) {
        if (ARG_ISARTIFICIAL (AVIS_DECL (avis))) {
            /*
             * found a reference to an objdef-argument, so
             * replace it.
             */
            DBUG_ASSERT ((ARG_OBJDEF (AVIS_DECL (avis)) != NULL),
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
    DBUG_ENTER ("RESOap");

    if (FUNDEF_ISSPMDFUN (AP_FUNDEF (arg_node))) {
        FUNDEF_ARGS (AP_FUNDEF (arg_node))
          = MarkArtificialArgs (FUNDEF_ARGS (AP_FUNDEF (arg_node)), AP_ARGS (arg_node));
    }

    AP_ARGS (arg_node)
      = StripArtificialArgExprs (FUNDEF_ARGS (AP_FUNDEF (arg_node)), AP_ARGS (arg_node));

    /*
     * unwrap function if neccessary
     * be aware that functions may get wrapped multiple times
     * if they are imported more than once!
     */
    while (FUNDEF_ISOBJECTWRAPPER (AP_FUNDEF (arg_node))) {
        DBUG_ASSERT ((FUNDEF_IMPL (AP_FUNDEF (arg_node)) != NULL),
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
RESOlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RESOlet");

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

    DBUG_RETURN (arg_node);
}

node *
RESOassign (node *arg_node, info *arg_info)
{
    bool delete;

    DBUG_ENTER ("RESOassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    delete = INFO_DELETE (arg_info);
    INFO_DELETE (arg_info) = FALSE;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (delete) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
RESOblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RESOblock");

    BLOCK_VARDEC (arg_node) = ResetAvisSubst (BLOCK_VARDEC (arg_node));

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_INSTR (arg_node) == NULL) {
        BLOCK_INSTR (arg_node) = TBmakeEmpty ();
    }

    BLOCK_VARDEC (arg_node) = DeleteSubstVardecs (BLOCK_VARDEC (arg_node));

    DBUG_RETURN (arg_node);
}

node *
RESOfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RESOfundef");

    if (FUNDEF_ISSPMDFUN (arg_node) && !INFO_DOSPMD (arg_info)) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
        DBUG_RETURN (arg_node);
    }

    /*
     * prcocess all bodies first
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL && !FUNDEF_ISSPMDFUN (arg_node)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     * then clean up the signatures
     */
    FUNDEF_ARGS (arg_node) = StripArtificialArgs (FUNDEF_ARGS (arg_node));

    /*
     * finally delete object wrapper functions
     */
    if (FUNDEF_ISOBJECTWRAPPER (arg_node)) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
RESOmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RESOmodule");

    /*
     * we have to traverse the fundefs first, as the bodies have
     * to be transformed prior to transforming the signatures!
     */
    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RESOpropagate (node *arg_node, info *arg_info)
{
    node *arg;

    DBUG_ENTER ("RESOpropagate");

    if (PROPAGATE_NEXT (arg_node) != NULL) {
        PROPAGATE_NEXT (arg_node) = TRAVdo (PROPAGATE_NEXT (arg_node), arg_info);
    }

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

    DBUG_ENTER ("RESOdoRestoreObjects");

    info = MakeInfo ();
    TRAVpush (TR_reso);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
