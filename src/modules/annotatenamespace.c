/*
 *
 * $Log$
 * Revision 1.1  2004/10/22 08:49:55  sah
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include "annotatenamespace.h"
#include "traverse.h"
#include "tree_basic.h"

node *
ANSUse (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSUse");

    if (USE_NEXT (arg_node) != NULL) {
        USE_NEXT (arg_node) = Trav (USE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSImport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSImport");

    if (IMPORT_NEXT (arg_node) != NULL) {
        IMPORT_NEXT (arg_node) = Trav (IMPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSExport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSExport");

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = Trav (EXPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSProvide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSProvide");

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = Trav (PROVIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSFundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSFundef");

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSTypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSTypedef");

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSObjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSObjdef");

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSAp (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSAp");

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSArg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSArg");

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSVardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSVardec");

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSModul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSModul");

    DBUG_RETURN (arg_node);
}
