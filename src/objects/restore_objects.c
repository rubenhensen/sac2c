/* $Id$ */

#include "restore_objects.h"
#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "internal_lib.h"
/*
 * INFO structure
 */
struct INFO {
    bool delete;
};

/*
 * INFO macros
 */
#define INFO_DELETE(n) ((n)->delete)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_DELETE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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

    /*
     * prcocess all bodies first
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
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
