/*
 * $Id: ivextrema.c 15815 2008-10-24 18:04:47Z rbe $
 */

/** <!--********************************************************************-->
 *
 * @defgroup ivexi Index Vector Extrema Insertion Traversal
 *
 * This traversal inserts maxima and minima for index vector variables
 * in with-loops. Later optimizations will propagate these maxima
 * and minima, and use then as input for other optimizations,
 * such as with-loop folding and guard removal.
 *
 * @ingroup ivexi
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file ivextrema.c
 *
 * Prefix: IVEXI
 *
 *****************************************************************************/
#include "ivextrema.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"
#include "shape.h"
#include "constants.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "pattern_match.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
};

/**
 * INFO macros
 */
// xxxxxxxxxxxxxxxxxxxxxxxxxFIXME  #define INFO_GENARRAYS(n)       ((n)->genarrays)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    //      xxxxxxxxxxxxxxxxxxxx FIXME INFO_GENARRAYS( result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}
/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *IVEXIdoInsertIndexVectorExtrema( node *arg_node)
 *
 *****************************************************************************/
node *
IVEXIdoInsertIndexVectorExtrema (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("IVEXIdoIndexVectorExtremaInsertion");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "IVEXIdoIndexVectorExtremaInsertion expected N_modules");

    arg_info = MakeInfo ();

    DBUG_PRINT ("IVEX", ("Starting index vector extrema insertion traversal."));

    TRAVpush (TR_ivexi);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DBUG_PRINT ("IVEX", ("Index vector extrema insertion complete."));

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
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

/******************************************************************************
 *
 * function:
 *   node *IVEXImodule( node *arg_node, info *arg_info)
 *   node *IVEXIfundef( node *arg_node, info *arg_info)
 *   node *IVEXIblock( node *arg_node, info *arg_info)
 *   node *IVEXIwith( node *arg_node, info *arg_info)
 *   node *IVEXIcode( node *arg_node, info *arg_info)
 *   node *IVEXIlet( node *arg_node, info *arg_info)
 *   node *IVEXIassign( node *arg_node, info *arg_info)
 *
 * description:
 *   Most of the following functions merely
 *   walk the tree, returning anything changed.
 *
 ******************************************************************************/
node *
IVEXImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXImodule");
    DBUG_PRINT ("IVEX", ("Found module"));
    if (NULL != MODULE_FUNS (arg_node)) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

node *
IVEXIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIfundef");
    DBUG_PRINT ("IVEX", ("Found fundef"));
    if (NULL != FUNDEF_BODY (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IVEXIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIblock");
    DBUG_PRINT ("IVEX", ("Found block"));
    if (NULL != BLOCK_INSTR (arg_node)) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

node *
IVEXIcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIcode");
    DBUG_PRINT ("IVEX", ("Found code"));
    if (NULL != CODE_CBLOCK (arg_node)) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    if (NULL != CODE_NEXT (arg_node)) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IVEXIwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIwith");
    DBUG_PRINT ("IVEX", ("Found WL"));
    if (NULL != WITH_PART (arg_node)) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

node *
IVEXIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIlet");
    DBUG_PRINT ("IVEX", ("Found let"));
    if (NULL != LET_EXPR (arg_node)) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

node *
IVEXIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIassign");
    DBUG_PRINT ("IVEX", ("Found assign"));
    if (NULL != ASSIGN_INSTR (arg_node)) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIpart( node *arg_node, info *arg_info)
 *
 * description:
 *   This does all the work. It grabs the lower and upper bounds of
 *   the WL partition index variable, and stuffs them into the index variable's
 *   MAXVAL and MINVAL.
 *
 *   TODO: 2008-12-09: Bodo and I feel that this code should also handle PART_IDS,
 *         but it does not do that today.
 *
 ******************************************************************************/
node *
IVEXIpart (node *arg_node, info *arg_info)
{
    node *ivavis;
    node *bound1 = NULL;
    node *bound2 = NULL;
    node *ae1;
    node *ae2;
    constant *b1fs = NULL;
    constant *b2fs = NULL;

    DBUG_ENTER ("IVEXIpart");
    DBUG_PRINT ("IVEX", ("Found WL partition"));
    ivavis = IDS_AVIS (WITHID_VEC (PART_WITHID (arg_node)));
    DBUG_ASSERT ((N_avis == NODE_TYPE (ivavis)),
                 "IVEXIpart expected N_avis in WL index vector");
    /* Updating the AVIS here is not strictly kosher from a functional standpoint,
     * but Bodo thinks the alternative (saving this data in an arg_info node
     * and making a second pass across the syntax tree) is not much better.
     */

    /* If the bounds are N_array nodes, we can do something useful here */
    if (PM (PMarray (&b1fs, &bound1, GENERATOR_BOUND1 (PART_GENERATOR (arg_node))))
        && PM (PMarray (&b2fs, &bound2, GENERATOR_BOUND2 (PART_GENERATOR (arg_node))))) {

        /* ast.xml claims these can be an N_num, too. We'll see. */

        /* TODO: We only handle single-element IV today.  */

        /* TODO: What if the N_array is empty, e.g.:  [:int] ??  */

        /* An N_array element can contain either an N_id or an N_num. The latter appears
         * in, e.g., [23].
         */
        if (NULL != ARRAY_AELEMS (bound1)) {
            ae1 = EXPRS_EXPR (ARRAY_AELEMS (bound1));
            if (N_id == NODE_TYPE (ae1)) {
                AVIS_MINVAL (ivavis) = ID_AVIS (ae1);
                DBUG_PRINT ("IVEX", ("Set AVIS_MINVAL for %s to %s", AVIS_NAME (ivavis),
                                     AVIS_NAME (ID_AVIS (ae1))));
            } else {
                DBUG_PRINT ("IVEX", ("FIXME: ignoring constant BOUND1"));
            }

            if (NULL != EXPRS_NEXT (ARRAY_AELEMS (bound1))) {
                DBUG_PRINT ("IVEX", ("FIXME: found non-single-element BOUND1."));
            }
        } else {
            DBUG_PRINT ("IVEX", ("FIXME: found NULL IV BOUND1"));
        }

        if (NULL != ARRAY_AELEMS (bound2)) {
            ae2 = EXPRS_EXPR (ARRAY_AELEMS (bound2));
            if (N_id == NODE_TYPE (ae2)) {
                AVIS_MAXVAL (ivavis) = ID_AVIS (ae2);
                DBUG_PRINT ("IVEX", ("Set AVIS_MAXVAL for %s to %s", AVIS_NAME (ivavis),
                                     AVIS_NAME (ID_AVIS (ae2))));
            } else {
                DBUG_PRINT ("IVEX", ("FIXME: ignoring constant BOUND2"));
            }

            if (NULL != EXPRS_NEXT (ARRAY_AELEMS (bound2))) {
                DBUG_PRINT ("IVEX", ("FIXME: found non-single-element BOUND2."));
            }
        } else {
            DBUG_PRINT ("IVEX", ("FIXME: found NULL IV BOUND2"));
        }

        if (NULL != WITHID_IDS (PART_WITHID (arg_node))) {
            DBUG_PRINT ("IVEX", ("FIXME: is not handling with WL WITH_IDS yet."));
        }
    }

    DBUG_RETURN (arg_node);
}
