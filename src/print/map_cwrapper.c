#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "map_cwrapper.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "convert.h"
#include "filemgr.h"
#include "globals.h"
#include "free.h"
#include "resource.h"
#include "shape.h"

/******************************************************************************
 *
 * function:
 *   node *MCWmodul(node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses only in functions of module
 *
 ******************************************************************************/

node *
MCWmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("MCWmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        /* if there are some fundefs traverse them */

        INFO_MCW_MODUL (arg_info) = arg_node;

        /* the modul node is needed to hang the wrapperchain in N_module */
        Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MCWfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   Builds up N_cwrapper nodes for wrapper and nodelist with fundefs
 *
 ******************************************************************************/

node *
MCWfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("MCWfundef");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MCWarg(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses N_arg nodes
 *
 ******************************************************************************/

node *
MCWarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("MCWarg");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MapCWrapper(node *syntaxtree)
 *
 * description:
 *   Builds up a mappingtree by traversing syntaxtree looking for fundefs to
 *   export. N_cwrapper nodes are located as son of N_modul
 *
 *
 ******************************************************************************/

node *
MapCWrapper (node *syntax_tree)
{
    node *arg_info;

    DBUG_ENTER ("MapCWrapper");

    if (generatelibrary & GENERATELIBRARY_C) {
        arg_info = MakeInfo ();

        syntax_tree = Trav (syntax_tree, arg_info);

        FREE (arg_info);
    }

    DBUG_RETURN (syntax_tree);
}
