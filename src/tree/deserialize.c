/*
 *
 * $Log$
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
CombineFunctionHeadAndBody (node *fundef, node *body)
{
    funtab *store_tab;
    info *info;

    DBUG_ENTER ("CombineFunctionHeadAndBody");

    info = MakeInfo ();

    store_tab = act_tab;
    act_tab = ds_tab;

    Trav (fundef, info);

    act_tab = store_tab;

    info = FreeInfo (info);

    FUNDEF_BODY (fundef) = body;

    DBUG_RETURN (fundef);
}

static node *
LookUpSSACounter (node *cntchain, node *avis)
{
    DBUG_ENTER ("LookUpSSACounter");

    while ((cntchain != NULL) && (AVIS_SSACOUNT (avis) == NULL)) {
        if (!strcmp (SSACNT_BASEID (cntchain),
                     VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (avis)))) {
            AVIS_SSACOUNT (avis) = cntchain;
        }
    }

    DBUG_RETURN (avis);
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

    ARG_AVIS (arg_node)
      = LookUpSSACounter (INFO_DS_SSACOUNTER (arg_info), ARG_AVIS (arg_node));

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
