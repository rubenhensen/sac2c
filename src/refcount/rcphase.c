/**
 * $Id$
 *
 * @defgroup rcp Reference Counting
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file rcphase.c
 *
 *
 */
#include "rcphase.h"

#include "phase.h"
#include "globals.h"
#include "traverse.h"
#include "tree_basic.h"
#include "dbug.h"

/** <!--*******************************************************************-->
 *
 * @fn node *EMRdoRefCountPhase( node *syntax_tree)
 *
 * @brief
 *
 * @param syntax_tree The whole syntax tree.
 *
 * @return The transformed syntax tree.
 *
 ****************************************************************************/
node *
EMRdoRefCountPhase (node *syntax_tree)
{
    DBUG_ENTER ("EMRdoRefCountPhase");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "RCphase not started with N_module node");

    /*
     * Reference counting
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rc, syntax_tree);

    /*
     * Refcount minimization
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rcm, syntax_tree);

    /*
     * Reference counting optimizations
     */
    if (global.optimize.dorco) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_rco, syntax_tree);
    }

    /*
     * Reuse elimination
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_re, syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/*@}*/ /* defgroup rcp */
