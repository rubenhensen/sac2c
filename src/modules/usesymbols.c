/*
 *
 * $Log$
 * Revision 1.2  2004/10/22 14:48:16  sah
 * fixed some typeos
 *
 * Revision 1.1  2004/10/22 13:50:44  sah
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include "usesymbols.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "tree_basic.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundefs;
};

/*
 * INFO macros
 */
#define INFO_USS_FUNDEFS(info) (info->fundefs)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_USS_FUNDEFS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/*
 * Traversal functions
 */

node *
USSAp (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSAp");

    DBUG_RETURN (arg_node);
}

node *
USSModul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSModul");

    INFO_USS_FUNDEFS (arg_info) = MODUL_FUNS (arg_node);

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

void
DoUseSymbols (node *modul)
{
    funtab *store_tab;
    info *info;

    DBUG_ENTER ("DoUseSymbols");

    info = MakeInfo ();

    store_tab = act_tab;
    act_tab = uss_tab;

    modul = Trav (modul, info);

    act_tab = store_tab;

    info = FreeInfo (info);

    DBUG_VOID_RETURN;
}
