/*
 *
 * $Log$
 * Revision 1.2  2004/10/25 11:58:47  sah
 * major code cleanup
 *
 * Revision 1.1  2004/09/23 21:12:03  sah
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include "deserialize.h"
#include <strings.h>
#include "internal_lib.h"
#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "modulemanager.h"
#include "serialize.h"

/*
 * INFO structure
 */
struct INFO {
    node *ret;
    node *ssacounter;
};

/*
 * INFO macros
 */
#define INFO_DS_RETURN(n) n->ret
#define INFO_DS_SSACOUNTER(n) n->ssacounter

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_DS_RETURN (result) = NULL;
    INFO_DS_SSACOUNTER (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

node *
LoadFunctionBody (node *fundef, node *modnode)
{
    node *result;
    module_t *module;
    serfun_p serfun;

    DBUG_ENTER ("LoadFunctionBody");

    module = LoadModule (FUNDEF_MOD (fundef));

    serfun = GetDeSerializeFunction (GenerateSerFunName (SET_funbody, fundef), module);

    result = serfun ();

    module = UnLoadModule (module);

    DBUG_RETURN (result);
}

node *
AddFunctionBodyToHead (node *fundef, node *module)
{
    funtab *store_tab;
    info *info;
    node *body;

    DBUG_ENTER ("CombineFunctionHeadAndBody");

    info = MakeInfo ();
    body = LoadFunctionBody (fundef, module);

    FUNDEF_BODY (fundef) = body;

    store_tab = act_tab;
    act_tab = ds_tab;

    Trav (fundef, info);

    act_tab = store_tab;

    info = FreeInfo (info);

    DBUG_RETURN (fundef);
}

static node *
LookUpSSACounter (node *cntchain, node *arg)
{
    node *result = NULL;
    DBUG_ENTER ("LookUpSSACounter");

    while ((cntchain != NULL) && (result == NULL)) {
        if (!strcmp (SSACNT_BASEID (cntchain), ARG_NAME (arg))) {
            result = cntchain;
        }

        cntchain = SSACNT_NEXT (cntchain);
    }

    DBUG_RETURN (result);
}

node *
DSReturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSReturn");

    INFO_DS_RETURN (arg_info) = arg_node;

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSBlock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSBlock");

    if (INFO_DS_SSACOUNTER (arg_info) == NULL) {
        /* we are the first block underneath the Fundef node */
        INFO_DS_SSACOUNTER (arg_info) = BLOCK_SSACOUNTER (arg_node);
    }

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSArg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSArg");

    AVIS_SSACOUNT (ARG_AVIS (arg_node))
      = LookUpSSACounter (INFO_DS_SSACOUNTER (arg_info), arg_node);

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSVardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSVardec");

    /* nothing to be done here */

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}
