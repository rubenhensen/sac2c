/* $Id$ */

#include "restore_objects.h"
#include "dbug.h"
#include "tree_basic.h"
#include "traverse.h"
#include "free.h"

/*
 * static helper functions
 */
node *
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

node *
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
            DBUG_ASSERT ((ARG_OBJDEF (AVIS_DECL (avis)) != NULL),
                         "found artificial arg without objdef pointer!");

            arg_node = FREEdoFreeNode (arg_node);
            arg_node = TBmakeGlobobj (ARG_OBJDEF (AVIS_DECL (avis)));
        }
    }

    DBUG_RETURN (arg_node);
}

node *
RESOap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RESOap");

    AP_ARGS (arg_node)
      = StripArtificialArgExprs (FUNDEF_ARGS (AP_FUNDEF (arg_node)), AP_ARGS (arg_node));

    arg_node = TRAVcont (arg_node, arg_info);

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
    DBUG_ENTER ("RESOdoRestoreObjects");

    TRAVpush (TR_reso);

    syntax_tree = TRAVdo (syntax_tree, NULL);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}
