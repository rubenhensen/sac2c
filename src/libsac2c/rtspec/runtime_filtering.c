/** <--*********************************************************************-->
 *
 * @file  runtime_filtering.c
 *
 * @brief Traversal that filters unneeded functions and inserst the function
 * targeted for runtime optimization into the FUNSPEC chain.
 *
 * @author tvd
 *
 ****************************************************************************/

#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "namespaces.h"

#define DBUG_PREFIX "RTFILTER"
#include "debug.h"

#include "traverse.h"
#include "specialize.h"
#include "globals.h"
#include "ctinfo.h"
#include "new_types.h"
#include "DupTree.h"
#include "type_utils.h"
#include "str.h"

struct INFO {
    int args_found;

    node *module;
    node *args;
};

#define INFO_MODULE(n) ((n)->module)
#define INFO_ARGS(n) ((n)->args)
#define INFO_ARGSFOUND(n) ((n)->args_found)

static info *
MakeInfo (info *arg_info)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ARGSFOUND (result) = 0;
    INFO_MODULE (result) = NULL;
    INFO_ARGS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *arg_info)
{
    DBUG_ENTER ();

    arg_info = MEMfree (arg_info);

    DBUG_RETURN (arg_info);
}

/** <!--*******************************************************************-->
 *
 * @fn RTFILTERarg( node *arg_node, info *arg_info)
 *
 * @brief Traverse all the arguments of a FUNDEF and check if the function
 *        targeted for specialization has been found.
 *
 * If the targeted function is found, a flag in the info structure is set to
 * TRUE so the FUNDEF traversal can take care of specialization.
 *
 * @param arg_node  The ARG node.
 * @param arg_info  The info structure.
 *
 * @return The unaltered ARG node.
 *
 ****************************************************************************/
node *
RTFILTERarg (node *arg_node, info *arg_info)
{
    ntype *local, *global;

#ifndef DBUG_OFF
    char *tmp_str_a, *tmp_str_b;
#endif

    local = ARG_NTYPE (arg_node);
    global = ARG_NTYPE (INFO_ARGS (arg_info));

    DBUG_ENTER ();

    DBUG_PRINT (">>>>>> Checking argument ...");

    DBUG_ASSERT (!(TUcontainsUser (local) && TUcontainsUser (global)),
                 "User-defined are currently not supported!");

    DBUG_EXECUTE (tmp_str_a = TYtype2String (local, 0, 0));
    DBUG_EXECUTE (tmp_str_b = TYtype2String (global, 0, 0));
    DBUG_PRINT ("Searching for: %s, found: %s", tmp_str_a, tmp_str_b);

    /*
     * Check if the base types match.
     */
    if (TYgetSimpleType (TYgetScalar (local)) == TYgetSimpleType (TYgetScalar (global))) {
        /*
         * Count the number of matching arguments.
         */
        INFO_ARGSFOUND (arg_info)++;

        if (ARG_NEXT (INFO_ARGS (arg_info)) != NULL && ARG_NEXT (arg_node) != NULL) {
            INFO_ARGS (arg_info) = ARG_NEXT (INFO_ARGS (arg_info));
            ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn RTFILTERfundef( node *arg_node, info *arg_info)
 *
 * @brief Traverse all the FUNDEF nodes and subsequently traverse all the
 * fundefs arguments. If the arguments have the correct types (designating the
 * function targeted for specialization) the fundef is copied and updated. The
 * updated FUNDEF is added to the FUNSPEC chain for optimization.
 *
 * If the FUNDEF does not have the correct signature, the flag WASIMPORTED is
 * set to false, the compiler will then automagically clean them up.
 *
 * @param arg_node  The FUNDEF node.
 * @param arg_info  The info structure.
 *
 * @return The unaltered FUNDEF node or the next in the chain.
 *
 ****************************************************************************/
node *
RTFILTERfundef (node *arg_node, info *arg_info)
{
    node *funspec = NULL;

    DBUG_ENTER ();
    DBUG_PRINT (">>>> Checking function ...");

    /* We're only interested in the function we're specializing. */
    if (STReq (FUNDEF_NAME (arg_node), global.rt_fun_name)) {
        DBUG_PRINT (">>>> Function found ...");

        INFO_ARGS (arg_info) = global.rt_args;

        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);

        /* Do we have the correct function? */
        if (INFO_ARGSFOUND (arg_info) == global.rt_num_args) {
            DBUG_PRINT ("Arguments match: creating FUNSPEC node.");

            /* Create a new fundef with the correct signature for specialization. */
            funspec
              = TBmakeFundef (STRcpy (FUNDEF_NAME (arg_node)),
                              NSdupNamespace (MODULE_NAMESPACE (INFO_MODULE (arg_info))),
                              DUPdoDupTree (FUNDEF_RETS (arg_node)), global.rt_args, NULL,
                              funspec);

            /* Designate the fundef for specialization. */
            MODULE_FUNSPECS (INFO_MODULE (arg_info)) = funspec;
        }

    } else {
        FUNDEF_WASIMPORTED (arg_node) = FALSE;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        INFO_ARGSFOUND (arg_info) = 0;
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn RTFILTERmodule
 *
 * @brief The filtering starts at module level as a function specialization has
 * to be added to the FUNSPEC chain of the module.
 *
 * @param arg_node  The MODULE node.
 * @param arg_info  The info structure.
 *
 * @return The original MODULE, possibly with a new FUNDEF in its FUNSPEC chain.
 *
 ****************************************************************************/
node *
RTFILTERmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (MODULE_FUNS (arg_node) != NULL) {
        DBUG_PRINT (">> Checking module ...");
        INFO_MODULE (arg_info) = arg_node;
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn RTdoFilter
 *
 * @brief  Entry of the traversal that filteres all unneeded FUNDEFS from the
 * fundef chain. That way only the function we're interested in will be
 * specialized.
 *
 * @param syntax_tree  The abstract syntax tree.
 *
 * @return  The updated syntax tree.
 *
 ****************************************************************************/
node *
RTdoFilter (node *syntax_tree)
{
    DBUG_ENTER ();

    info *info = NULL;

    info = MakeInfo (info);

    TRAVpush (TR_rtfilter);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
