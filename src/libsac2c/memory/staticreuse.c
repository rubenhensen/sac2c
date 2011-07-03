/*
 * $Id$
 */

/**
 * @defgroup strf Static reuse inference
 *
 * @ingroup mm
 *
 * @{
 */

/**
 * @file staticreuse.c
 *
 * Prefix: EMSR
 */

#include "staticreuse.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "EMSR"
#include "debug.h"

#include "print.h"
#include "DupTree.h"
#include "free.h"
#include "type_utils.h"

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
    DBUG_ENTER ();

    DBUG_PRINT ("Starting static reuse inference");

    TRAVpush (TR_emsr);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_PRINT ("Static reuse inference complete");

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
    DBUG_ENTER ();

    if ((PRF_PRF (arg_node) == F_alloc_or_reuse)
        || (PRF_PRF (arg_node) == F_alloc_or_reshape)
        || (PRF_PRF (arg_node) == F_alloc_or_resize)) {
        node *rcexprs = PRF_EXPRS3 (arg_node);

        while (rcexprs != NULL) {
            node *rc = EXPRS_EXPR (rcexprs);

            if (!AVIS_ISALIAS (ID_AVIS (rc))) {
                /*
                 * STATIC REUSE!!!
                 */
                if (PRF_PRF (arg_node) == F_alloc_or_reuse) {
                    /*
                     * a = reuse( b);
                     */
                    node *new_node = TCmakePrf1 (F_reuse, DUPdoDupNode (rc));
                    arg_node = FREEdoFreeNode (arg_node);
                    arg_node = new_node;
                } else if (PRF_PRF (arg_node) == F_alloc_or_reshape) {
                    /*
                     * a = reshape( dim, shape, b);
                     */
                    PRF_PRF (arg_node) = F_reshape_VxA;
                } else if (PRF_PRF (arg_node) == F_alloc_or_resize) {
                    /*
                     * a = resize( b, shp)
                     */
                    node *new_node
                      = TCmakePrf3 (F_resize, DUPdoDupNode (PRF_ARG1 (arg_node)),
                                    DUPdoDupNode (PRF_ARG2 (arg_node)),
                                    DUPdoDupNode (rc));
                    arg_node = FREEdoFreeNode (arg_node);
                    arg_node = new_node;
                }
                break;
            }
            rcexprs = EXPRS_NEXT (rcexprs);
        }

        if (PRF_PRF (arg_node) == F_alloc_or_reuse) {
            rcexprs = PRF_EXPRS3 (arg_node);
            if (TUisScalar (ID_NTYPE (EXPRS_EXPR (rcexprs)))) {
                PRF_PRF (arg_node) = F_alloc;
                PRF_EXPRS3 (arg_node) = FREEdoFreeTree (PRF_EXPRS3 (arg_node));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/*@}*/

#undef DBUG_PREFIX
