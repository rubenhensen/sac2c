/*
 *
 * $Log$
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
#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "refcounting.h"
#include "rcopt.h"
#include "reuseelimination.h"

/** <!--*******************************************************************-->
 *
 * @fn node *RCphase( node *syntax_tree)
 *
 * @brief
 *
 * @param syntax_tree The whole syntax tree.
 *
 * @return The transformed syntax tree.
 *
 ****************************************************************************/
node *
RCphase (node *syntax_tree)
{
    DBUG_ENTER ("RCphase");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_modul),
                 "RCphase not started with N_modul node");

    /*
     * Reference counting
     */
    syntax_tree = EMRefCount (syntax_tree);
    if ((break_after == PH_refcnt) && (0 == strcmp (break_specifier, "rc"))) {
        goto DONE;
    }

    /*
     * Reference counting optimizations
     */
    if (optimize & OPT_RCO) {
        syntax_tree = EMRCORefCountOpt (syntax_tree);
    }
    if ((break_after == PH_refcnt) && (0 == strcmp (break_specifier, "rco"))) {
        goto DONE;
    }

    /*
     * Reuse elimination
     */
    syntax_tree = EMREReuseElimination (syntax_tree);
    if ((break_after == PH_refcnt) && (0 == strcmp (break_specifier, "re"))) {
        goto DONE;
    }

DONE:
    DBUG_RETURN (syntax_tree);
}

/*@}*/ /* defgroup rcp */
