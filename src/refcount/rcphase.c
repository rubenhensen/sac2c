/*
 *
 * $Log$
 * Revision 1.11  2005/07/03 16:59:18  ktr
 * Switched to phase.h
 *
 * Revision 1.10  2004/12/16 14:37:30  ktr
 * added InplaceComputation
 *
 * Revision 1.9  2004/11/28 18:14:21  ktr
 * Changed name of starting function to EMRdoRefCountPhase
 *
 * Revision 1.8  2004/11/26 23:56:06  jhb
 * fixed DBUG_Enter Statement of EMRdoRefcounting
 *
 * Revision 1.7  2004/11/26 23:49:23  jhb
 * same function name header and c-file
 *
 * Revision 1.6  2004/11/26 23:34:25  ktr
 * EMRCdoRefcounting changed to EMRdoRefcounting
 *
 * Revision 1.5  2004/11/23 22:22:51  ktr
 * COMPILES!!!
 *
 * Revision 1.4  2004/11/23 20:23:28  jhb
 * compile
 *
 * Revision 1.3  2004/11/19 15:42:41  ktr
 * Support for F_alloc_or_reshape added.
 *
 * Revision 1.2  2004/10/12 10:24:55  ktr
 * added Filter reuse candidates traversal.
 *
 * Revision 1.1  2004/10/11 14:45:07  ktr
 * Initial revision
 *
 */

/**
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
