/*
 *
 * $Log$
 * Revision 1.4  2004/10/28 17:20:09  sah
 * now deserialisation has an internal state
 * ,
 *
 * Revision 1.3  2004/10/26 09:33:56  sah
 * ongoing implementation
 *
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
#include "modulemanager.h"
#include "deserialize.h"

/*
 * INFO structure
 */
struct INFO {
    node *module;
};

/*
 * INFO macros
 */
#define INFO_USS_MODULE(info) (info->module)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_USS_MODULE (result) = NULL;

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
    module_t *module;

    DBUG_ENTER ("USSAp");

    if (strcmp (AP_MOD (arg_node), MODUL_NAME (INFO_USS_MODULE (arg_info)))) {
        module = LoadModule (AP_MOD (arg_node));

        AddSymbolToAst (AP_NAME (arg_node), module);
    }

    DBUG_RETURN (arg_node);
}

node *
USSModul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSModul");

    INFO_USS_MODULE (arg_info) = arg_node;

    InitDeserialize (arg_node);

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    FinishDeserialize (arg_node);

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
