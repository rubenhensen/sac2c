/*
 *
 * $Log$
 * Revision 1.6  2004/11/23 22:28:38  ktr
 * renaming done.
 *
 * Revision 1.5  2004/11/23 17:45:07  ktr
 * COMPILES!
 *
 * Revision 1.4  2004/11/23 17:38:25  jhb
 * compile
 *
 * Revision 1.3  2004/11/19 15:42:41  ktr
 * Support for F_alloc_or_reshape added.
 *
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

#include "staticreuse.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "DupTree.h"
#include "free.h"

/**
 * INFO structure
 */
struct INFO {
};

/** <!--********************************************************************-->
 *
 * @fn node *EMSRdoStaticReuse( node *syntax_tree)
 *
 * @brief starting point of Static reuse inference
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMSRdoStaticReuse (node *syntax_tree)
{
    DBUG_ENTER ("EMSRdoStaticReuse");

    DBUG_PRINT ("EMSR", ("Starting static reuse inference"));

    TRAVpush (TR_emsr);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

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

    if ((PRF_PRF (arg_node) == F_alloc_or_reuse)
        || (PRF_PRF (arg_node) == F_alloc_or_reshape)) {
        node *rcexprs = PRF_EXPRS3 (arg_node);

        while (rcexprs != NULL) {
            node *rc = EXPRS_EXPR (rcexprs);

            if (!AVIS_ISALIAS (ID_AVIS (rc))) {
                /*
                 * STATIC REUSE!!!
                 */
                if (PRF_PRF (arg_node) == F_alloc_or_reuse) {
                    node *new_node = TCmakePrf1 (F_reuse, DUPdoDupNode (rc));
                    arg_node = FREEdoFreeNode (arg_node);
                    arg_node = new_node;

                    /*
                     * a = reuse( b, 1);
                     *
                     * Mark b as ALIASed such that it will not be statically freed
                     */
                    AVIS_ISALIAS (ID_AVIS (PRF_ARG1 (arg_node))) = TRUE;
                }

                if (PRF_PRF (arg_node) == F_alloc_or_reshape) {
                    PRF_PRF (arg_node) = F_reshape;

                    /*
                     * a = reshape( dim, shape, b);
                     *
                     * Mark b as ALIASed such that it will not be statically freed
                     */
                    AVIS_ISALIAS (ID_AVIS (PRF_ARG3 (arg_node))) = TRUE;
                }
                break;
            }
            rcexprs = EXPRS_NEXT (rcexprs);
        }
    }

    DBUG_RETURN (arg_node);
}

/*@}*/
