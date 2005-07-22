/*
 *
 * $Log$
 * Revision 1.1  2005/07/22 15:09:38  sah
 * Initial revision
 *
 *
 */

#include "add_function_body.h"
#include "deserialize.h"
#include "serialize.h"
#include "internal_lib.h"
#include "tree_basic.h"
#include "modulemanager.h"
#include "namespaces.h"
#include "traverse.h"

#include <string.h>

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
#define INFO_AFB_RETURN(n) ((n)->ret)
#define INFO_AFB_SSACOUNTER(n) ((n)->ssacounter)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_AFB_RETURN (result) = NULL;
    INFO_AFB_SSACOUNTER (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

node *
AFBdoAddFunctionBody (node *fundef)
{
    node *body;
    info *info;

    DBUG_ENTER ("AFBdoAddFunctionBody");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "AFBdoAddFunctionBody is intended to be used on fundef nodes!");

    DBUG_ASSERT ((FUNDEF_BODY (fundef) == NULL),
                 "cannot fetch a body if one already exists");

    DBUG_PRINT ("AFB", ("Adding function body to `%s'.", CTIitemName (fundef)));

    body = DSloadFunctionBody (fundef);

    DBUG_PRINT ("AFB", ("Operation %s", (body == NULL) ? "failed" : "completed"));

    FUNDEF_BODY (fundef) = body;

    info = MakeInfo ();

    TRAVpush (TR_afb);

    TRAVdo (fundef, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (fundef);
}

/*
 * Helper functions for deserialize traversal
 */

static node *
LookUpSSACounter (node *cntchain, node *arg)
{
    node *result = NULL;
    DBUG_ENTER ("LookUpSSACounter");

    while ((cntchain != NULL) && (result == NULL)) {
        if (ILIBstringCompare (SSACNT_BASEID (cntchain), AVIS_NAME (ARG_AVIS (arg)))) {
            result = cntchain;
        }

        cntchain = SSACNT_NEXT (cntchain);
    }

    DBUG_RETURN (result);
}

/*
 * traversal functions
 */

node *
AFBfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AFBfundef");

    arg_node = TRAVcont (arg_node, arg_info);

    FUNDEF_RETURN (arg_node) = INFO_AFB_RETURN (arg_info);

    DBUG_RETURN (arg_node);
}

node *
AFBreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AFBreturn");

    INFO_AFB_RETURN (arg_info) = arg_node;

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
AFBblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AFBblock");

    if (INFO_AFB_SSACOUNTER (arg_info) == NULL) {
        /* we are the first block underneath the Fundef node */
        INFO_AFB_SSACOUNTER (arg_info) = BLOCK_SSACOUNTER (arg_node);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
AFBarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AFBarg");

    AVIS_SSACOUNT (ARG_AVIS (arg_node))
      = LookUpSSACounter (INFO_AFB_SSACOUNTER (arg_info), arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}
