/*
 *
 * $Id$
 *
 */

/**<!--*********************************************************************-->
 *
 * @file runtime_specialization.c
 *
 * @brief Traversal for adding wrapper entry functions.
 *
 * @author tvd
 *
 *****************************************************************************/

#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "dbug.h"
#include "traverse.h"
#include "DupTree.h"
#include "globals.h"
#include "new_types.h"
#include "namespaces.h"
#include "specialization_oracle_static_shape_knowledge.h"
#include "strip_external_signatures.h"

struct INFO {
    node *module;
};

#define INFO_MODULE(n) ((n)->module)

static info *
MakeInfo (info *arg_info)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *arg_info)
{
    DBUG_ENTER ("FreeInfo");

    arg_info = MEMfree (arg_info);

    DBUG_RETURN (arg_info);
}

/** <!--********************************************************************-->
 *
 * @fn CreateWrapperEntry ( node *arg_node)
 *
 * @brief  Create a wrapper entry function for a wrapper function.
 *
 * @param arg_node  the fundef for which we're creating an entry function.
 *
 * @return  The updated fundef.
 *
 *****************************************************************************/
static node *
CreateWrapperEntry (node *arg_node, info *arg_info)
{
    node *result;
    node *body;
    node *block;
    node *ids;
    node *assigns;
    node *vardecs = NULL;
    node *exprs = NULL;

    DBUG_ENTER ("CreateWrapperEntry");

    body = FUNDEF_BODY (arg_node);
    FUNDEF_BODY (arg_node) = NULL;

    /*
     * Create a copy of the function header.
     */
    result = DUPdoDupNode (arg_node);

    /*
     * Put the function in the correct namespace.
     */
    FUNDEF_NS (result) = NSfreeNamespace (FUNDEF_NS (result));
    FUNDEF_NS (result) = NSbuildView (FUNDEF_NS (arg_node));

    /*
     * Reset the functions state.
     */
    FUNDEF_WASIMPORTED (result) = FALSE;
    FUNDEF_WASUSED (result) = FALSE;
    FUNDEF_ISLOCAL (result) = TRUE;

    /*
     * Set the body of the copy to the body of the original wrapper.
     */
    FUNDEF_BODY (result) = body;

    /*
     * Create the chain of assignments that is part of the function body.
     * The chain is created starting at the last element.
     */

    ids = TCcreateIdsFromRets (FUNDEF_RETS (arg_node), &vardecs);

    /* The return statement. */
    assigns = TBmakeAssign (TBmakeReturn (TCcreateExprsFromIds (ids)), NULL);

    node *prf_args = TCcreateExprsFromArgs (FUNDEF_ARGS (arg_node));

    /* A call to the original wrapper function. */
    assigns = TBmakeAssign (TBmakeLet (ids, TBmakeAp (result, prf_args)), assigns);

    /*
     * This primitive function creates the necessary code for encoding each
     * argument's shape information at runtime.
     */
    assigns = TBmakeAssign (TBmakeLet (NULL, TBmakePrf (F_we_shape_encode,
                                                        TCcreateExprsFromArgs (
                                                          FUNDEF_ARGS (arg_node)))),
                            assigns);

    /* Information about the function: name + module name. */
    exprs = TBmakeExprs (TCmakeStrCopy (FUNDEF_NAME (arg_node)),
                         TBmakeExprs (TCmakeStrCopy (NSgetModule (FUNDEF_NS (arg_node))),
                                      NULL));

    /*
     * This primitive function creates two static strings containing the function
     * name and the module name.
     *
     * The information is passed to the primitive function through 'exprs'.
     */
    assigns
      = TBmakeAssign (TBmakeLet (NULL, TBmakePrf (F_we_modfun_info, exprs)), assigns);

    block = TBmakeBlock (assigns, vardecs);

    /*
     * Set the body of the wrapper to the newly created body. The switch is needed
     * to 'fool' dispatch. Now a call to the original wrapper is in fact a call to
     * a wrapper entry.
     */
    FUNDEF_BODY (arg_node) = block;

    FUNDEF_ISWRAPPERFUN (result) = FALSE;
    FUNDEF_ISWRAPPERFUN (arg_node) = FALSE;
    FUNDEF_ISWRAPPERENTRYFUN (arg_node) = TRUE;
    FUNDEF_ISINDIRECTWRAPPERFUN (result) = TRUE;

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn RTSPECfundef (node *arg_node, info *arg_info)
 *
 * @brief  Walk through the fundef chain and add a wrapper entry function if a
 * local wrapper function is found.
 *
 * @param  arg_node  The current node of the syntax tree.
 * @param  arg_info  Info object, unused.
 *
 * @return  The updated fundef node.
 *
 *****************************************************************************/
node *
RTSPECfundef (node *arg_node, info *arg_info)
{
    node *result;
    result = arg_node;
    bool wrapper_entry_created = FALSE;

    DBUG_ENTER ("RTSPECfundef");

    if (FUNDEF_ISLOCAL (arg_node) && FUNDEF_ISWRAPPERFUN (arg_node)
        && NSequals (FUNDEF_NS (arg_node), global.modulenamespace)
        && FUNDEF_ISEXPORTED (arg_node)) {
        DBUG_PRINT ("RTSPEC", ("Creating a wrapper entry function for: %s",
                               FUNDEF_NAME (arg_node)));

        result = CreateWrapperEntry (arg_node, arg_info);
        wrapper_entry_created = TRUE;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        /*
         * Traverse all the functions in the fundef chain.
         */
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    if (wrapper_entry_created) {
        FUNDEF_NEXT (result) = FUNDEF_NEXT (arg_node);
        FUNDEF_NEXT (arg_node) = result;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn RTSPECdoCreateWrapperEntries ( node *arg_node)
 *
 * @brief Start the traversal for adding wrapper entry functions.
 *
 * @param arg_node  the syntax tree.
 *
 * @return  the updated and extended syntax tree.
 *
 * ***************************************************************************/
node *
RTSPECmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RTSPECmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        INFO_MODULE (arg_info) = arg_node;
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn RTSPECdoCreateWrapperEntries ( node *arg_node)
 *
 * @brief Start the traversal for adding wrapper entry functions.
 *
 * @param arg_node  the syntax tree.
 *
 * @return  the updated and extended syntax tree.
 *
 * ***************************************************************************/
node *
RTSPECdoCreateWrapperEntries (node *arg_node)
{
    DBUG_ENTER ("RTSPECdoCreateWrapperEntries");

    info *info = NULL;

    info = MakeInfo (info);

    TRAVpush (TR_rtspec);

    arg_node = TRAVdo (arg_node, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}
