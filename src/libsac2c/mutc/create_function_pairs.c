/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup cfp Create Function Pairs
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * ============================================================================
 * can be called on N_module               |   -----   |  y  |       |
 * can be called on N_fundef               |   -----   |  y  |       |
 * expects LaC funs                        |   -----   |  n  |       |
 * follows N_ap to LaC funs                |   -----   |  n  |       |
 * ============================================================================
 * deals with GLF properly                 |    yes    |  ut |       |
 * ============================================================================
 * is aware of potential SAA annotations   |    yes    |  y  |       |
 * utilises SAA annotations                |   -----   |  n  |       |
 * ============================================================================
 * tolerates flattened N_array             |    yes    |  y  |       |
 * tolerates flattened Generators          |    yes    |  y  |       |
 * tolerates flattened operation parts     |    yes    |  y  |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    |  y  |       |
 * ============================================================================
 * tolerates multi-operator WLs            |    yes    |  y  |       |
 * ============================================================================
 * ut = UnTested
 * </pre>
 *
 * This traversal duplicates all non thread functions and creates a
 * 'C' version of them.
 *
 * When given a function will return that function with next set to
 * the duplicated function if one was created.  This continues down
 * the chain of functions.
 *
 * @ingroup tt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file create_function_pairs.c
 *
 * Prefix: CFP
 *
 *****************************************************************************/
#include "create_function_pairs.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "DupTree.h"

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *CFPdoCreateFunctionPairs( node *syntax_tree)
 *
 *****************************************************************************/
node *
CFPdoCreateFunctionPairs (node *syntax_tree)
{
    DBUG_ENTER ("CFPdoCreateFunctionPairs");

    DBUG_ASSERT (((NODE_TYPE (syntax_tree) == N_module)
                  || (NODE_TYPE (syntax_tree) == N_fundef)),
                 "CFP is only designed to work on modules and fundefs");

    DBUG_ASSERT (((global.filetype == F_modimp) || (global.filetype == F_classimp)),
                 "CFP is intended for use on classes and modules only");

    DBUG_PRINT ("CFP", ("Create Function Pairs traversal."));

    TRAVpush (TR_cfp);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_PRINT ("CFP", ("Create Function Pairs complete."));

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *CFPfundef(node *arg_node, info *arg_info)
 *
 * @brief Create a thread function duplicate of this function if this
 *        function is a 'C' function.
 *
 *****************************************************************************/
node *
CFPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFPfundef");

    arg_node = TRAVcont (arg_node, arg_info);

    if ((!FUNDEF_ISTHREADFUN (arg_node)) && (!FUNDEF_ISEXTERN (arg_node))) {
        node *fundef_thread;

        fundef_thread = DUPdoDupNode (arg_node);
        FUNDEF_ISTHREADFUN (arg_node) = TRUE;
        FUNDEF_NEXT (fundef_thread) = FUNDEF_NEXT (arg_node);
        FUNDEF_NEXT (arg_node) = fundef_thread;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
