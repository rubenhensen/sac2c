/*
 *
 * $Id$
 *
 */

#include "add_function_body.h"
#include "deserialize.h"
#include "serialize.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "modulemanager.h"
#include "namespaces.h"
#include "traverse.h"
#include "dbug.h"
#include "ctinfo.h"

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

    result = MEMmalloc (sizeof (info));

    INFO_AFB_RETURN (result) = NULL;
    INFO_AFB_SSACOUNTER (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

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

/**
 * @brief Looks up the SSACounter for the given arg in the given chain.
 *        This is done by comparing the args name to the basename stored
 *        in the SSACounter. As args are always the first occurence of
 *        a name and thus never are renamed, this test suffices.
 *
 * @param cntchain chain of ssacounter nodes
 * @param arg the arg node
 *
 * @return matching ssacounter node or NULL
 */
static node *
LookUpSSACounter (node *cntchain, node *arg)
{
    node *result = NULL;
    DBUG_ENTER ("LookUpSSACounter");

    while ((cntchain != NULL) && (result == NULL)) {
        if (STReq (SSACNT_BASEID (cntchain), AVIS_NAME (ARG_AVIS (arg)))) {
            result = cntchain;
        }

        cntchain = SSACNT_NEXT (cntchain);
    }

    DBUG_RETURN (result);
}

/*
 * traversal functions
 */

/**
 * @brief continues traversal within the body of the fundef in order to
 *        find the return node which is needed to set FUNDEF_RETURN
 *        correctly. Furthermore, the SSACounters for the args are corrected.
 *
 * @param arg_node a fundef node
 * @param arg_info info structure
 *
 * @return corrected fundef node
 */
node *
AFBfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AFBfundef");

    /*
     * infer INFO_AFB_RETURN and INFO_AFB_SSACOUNTER
     */
    FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

    /*
     * correct args ssa counters
     */
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    /*
     * correct FUNDEF_RETURN
     */
    FUNDEF_RETURN (arg_node) = INFO_AFB_RETURN (arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief Sets INFO_AFB_RETURN to the current node.
 *
 * @param arg_node return node
 * @param arg_info info structure
 *
 * @return non-modified return node
 */
node *
AFBreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AFBreturn");

    INFO_AFB_RETURN (arg_info) = arg_node;

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief Saves this blocks SSACounter in INFO_AFB_SSACOUNTER for
 *        further reference.
 *
 * @param arg_node block node
 * @param arg_info info structure
 *
 * @return unmodified block node
 */
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

/**
 * @brief restores the args SSACounter based on the information found
 *        in INFO_AFB_SSACOUNTER.
 *
 * @param arg_node an arg node
 * @param arg_info info structure
 *
 * @return corrected arg node
 */
node *
AFBarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AFBarg");

    AVIS_SSACOUNT (ARG_AVIS (arg_node))
      = LookUpSSACounter (INFO_AFB_SSACOUNTER (arg_info), arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ***************************************************************** -->
 * @brief in case of a LACFUN, this funtion ensures that the body
 *        of that lacfun is deserialized as well, as it is a part
 *        of this functions body.
 *
 * @param arg_node N_ap node
 * @param arg_info info structure
 *
 * @return unmodified N_ap node
 */
node *
AFBap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AFBap");

    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
        && (FUNDEF_BODY (AP_FUNDEF (arg_node)) == NULL)) {

        AP_FUNDEF (arg_node) = AFBdoAddFunctionBody (AP_FUNDEF (arg_node));
    }

    DBUG_RETURN (arg_node);
}
