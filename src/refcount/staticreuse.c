/*
 *
 * $Log$
 * Revision 1.2  2004/10/22 15:38:19  ktr
 * Ongoing implementation.
 *
 * Revision 1.1  2004/10/21 16:18:17  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup strf Static reuse inference
 * @ingroup rcsr
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file staticreuse.c
 *
 *
 */
#define NEW_INFO

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "DupTree.h"

/**
 * INFO structure
 */
struct INFO {
};

/** <!--********************************************************************-->
 *
 * @fn node *EMSRStaticReuse( node *syntax_tree)
 *
 * @brief starting point of Static reuse inference
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMSRStaticReuse (node *syntax_tree)
{
    DBUG_ENTER ("EMSRStaticReuse");

    DBUG_PRINT ("EMSR", ("Starting static reuse inference"));

    act_tab = emsr_tab;

    syntax_tree = Trav (syntax_tree, NULL);

    DBUG_PRINT ("EMSR", ("Static reuse inference complete"));

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Static reuse inference traversal (emsrf_tab)
 *
 * prefix: EMSR
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMSRprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMSRprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMSRprf");

    if (PRF_PRF (arg_node) == F_alloc_or_reuse) {
        node *rcexprs = PRF_EXPRS3 (arg_node);

        while (rcexprs != NULL) {
            node *rc = EXPRS_EXPR (rcexprs);

            if (!AVIS_ALIAS (ID_AVIS (rc))) {
                /*
                 * STATIC REUSE!!!
                 */
                node *new_node = MakePrf1 (F_reuse, DupNode (rc));
                arg_node = FreeNode (arg_node);
                arg_node = new_node;

                /*
                 * a = reuse( b, 1);
                 *
                 * Mark b as ALIASed such that it will not be statically freed
                 */
                AVIS_ALIAS (ID_AVIS (PRF_ARG1 (arg_node))) = TRUE;
                break;
            }
            rcexprs = EXPRS_NEXT (rcexprs);
        }
    }

    DBUG_RETURN (arg_node);
}

/*@}*/
