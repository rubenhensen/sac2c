/*
 *
 * $Log$
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

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "refcounting.h"
#include "rcopt.h"
#include "reuseelimination.h"
#include <string.h>

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

    DBUG_PRINT ("EMM", ("Performing reference counting (rc)"));

    /*
     * Reference counting
     */
    syntax_tree = EMRCdoRefCounting (syntax_tree);
    if ((global.break_after == PH_refcnt)
        && (0 == strcmp (global.break_specifier, "rc"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Performing reference counting optmizations (rco)"));

    /*
     * Reference counting optimizations
     */
    if (global.optimize.dorco) {
        syntax_tree = EMRCOdoRefCountOpt (syntax_tree);
    }
    if ((global.break_after == PH_refcnt)
        && (0 == strcmp (global.break_specifier, "rco"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Removing reuse operations (re)"));

    /*
     * Reuse elimination
     */
    syntax_tree = EMREdoReuseElimination (syntax_tree);
    if ((global.break_after == PH_refcnt)
        && (0 == strcmp (global.break_specifier, "re"))) {
        goto DONE;
    }

DONE:
    DBUG_RETURN (syntax_tree);
}

/*@}*/ /* defgroup rcp */
