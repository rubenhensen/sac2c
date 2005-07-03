/*
 *
 * $Log$
 * Revision 1.1  2005/07/03 17:01:29  ktr
 * Initial revision
 *
 */
#include "codesimplification.h"

#include "phase.h"
#include "globals.h"
#include "traverse.h"
#include "tree_basic.h"
#include "dbug.h"

/** <!--*******************************************************************-->
 *
 * @fn node *CSdoCodeSimplification( node *syntax_tree)
 *
 * @brief
 *
 * @param syntax_tree The whole syntax tree.
 *
 * @return The transformed syntax tree.
 *
 ****************************************************************************/
node *
CSdoCodeSimplification (node *syntax_tree)
{
    DBUG_ENTER ("CSdoCodeSimplification");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "Code simplification not started with N_module node");

    /*
     * While2Do
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_w2d, syntax_tree);

    /*
     * Handle conditional expressions
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_hce, syntax_tree);

    /*
     * Handle MOPS nodes
     *
     * Before applying the actual flattening of code, we eliminate some
     * special constructs such as N_mop nodes.( Later, this should be the place
     * to eliminate N_dot nodes as well but, at the time being, this happens
     * right after scan-parse.
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_hm, syntax_tree);

    /*
     * Flattening
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_flat, syntax_tree);

    DBUG_RETURN (syntax_tree);
}
