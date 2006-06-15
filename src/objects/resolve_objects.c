/* $Id$ */

#include "resolve_objects.h"

#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "new_types.h"
#include "internal_lib.h"
#include "free.h"

/*
 * static helper functions
 */

node *
AppendObjdefsToArgs (node *args, node *objlist)
{
    node *avis;

    DBUG_ENTER ("AppendObjdefsToArgs");

    if (objlist != NULL) {
        args = AppendObjdefsToArgs (args, SET_NEXT (objlist));

        avis = TBmakeAvis (ILIBtmpVarName (OBJDEF_NAME (SET_MEMBER (objlist))),
                           TYcopyType (OBJDEF_TYPE (SET_MEMBER (objlist))));
        AVIS_DECLTYPE (avis) = TYcopyType (AVIS_TYPE (avis));

        OBJDEF_ARGAVIS (SET_MEMBER (objlist)) = avis;

        args = TBmakeArg (avis, args);
        ARG_ISARTIFICIAL (args) = TRUE;
        ARG_ISREFERENCE (args) = TRUE;
        ARG_OBJDEF (args) = SET_MEMBER (objlist);
    }

    DBUG_RETURN (args);
}

node *
AppendObjdefsToArgExprs (node *exprs, node *objlist)
{
    DBUG_ENTER ("AppendObjdefsToArgExprs");

    if (objlist != NULL) {
        exprs = AppendObjdefsToArgExprs (exprs, SET_NEXT (objlist));

        DBUG_ASSERT ((OBJDEF_ARGAVIS (SET_MEMBER (objlist)) != NULL),
                     "found objdef required for fun-ap but without argarvis!");

        exprs = TBmakeExprs (TBmakeId (OBJDEF_ARGAVIS (SET_MEMBER (objlist))), exprs);
    }

    DBUG_RETURN (exprs);
}

node *
CleanUpObjlist (node *list)
{
    DBUG_ENTER ("CleanUpObjlist");

    if (list != NULL) {
        SET_NEXT (list) = CleanUpObjlist (SET_NEXT (list));

        OBJDEF_ARGAVIS (SET_MEMBER (list)) = NULL;
    }

    DBUG_RETURN (list);
}

/*
 * traversal functions
 */
node *
RSOmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSOmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSOfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSOfundef");

    DBUG_PRINT ("RSO", ("processing fundef %s...", CTIitemName (arg_node)));

    FUNDEF_ARGS (arg_node)
      = AppendObjdefsToArgs (FUNDEF_ARGS (arg_node), FUNDEF_OBJECTS (arg_node));

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_OBJECTS (arg_node) = CleanUpObjlist (FUNDEF_OBJECTS (arg_node));

    DBUG_PRINT ("RSO", ("leaving fundef %s...", CTIitemName (arg_node)));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSOglobobj (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("RSOglobobj");

    DBUG_ASSERT ((OBJDEF_ARGAVIS (GLOBOBJ_OBJDEF (arg_node)) != NULL),
                 "found a globobj with no matching arg!");

    DBUG_PRINT ("RSO", (">>> replacing global object %s by local arg %s",
                        CTIitemName (GLOBOBJ_OBJDEF (arg_node)),
                        AVIS_NAME (OBJDEF_ARGAVIS (GLOBOBJ_OBJDEF (arg_node)))));

    new_node = TBmakeId (OBJDEF_ARGAVIS (GLOBOBJ_OBJDEF (arg_node)));
    arg_node = FREEdoFreeNode (arg_node);

    DBUG_RETURN (new_node);
}

node *
RSOap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSOap");

    DBUG_PRINT ("RSO",
                (">>> updating call to function %s", CTIitemName (AP_FUNDEF (arg_node))));

    AP_ARGS (arg_node) = AppendObjdefsToArgExprs (AP_ARGS (arg_node),
                                                  FUNDEF_OBJECTS (AP_FUNDEF (arg_node)));

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
RSOdoResolveObjects (node *syntax_tree)
{
    DBUG_ENTER ("RSOdoResolveObjects");

    TRAVpush (TR_rso);

    syntax_tree = TRAVdo (syntax_tree, NULL);

    TRAVpop (TR_rso);

    DBUG_RETURN (syntax_tree);
}
