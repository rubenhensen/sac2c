/*
 * $Id: ivextrema.c 15815 2008-10-24 18:04:47Z rbe $
 */

/** <!--********************************************************************-->
 *
 * @defgroup ivexi Index Vector Extrema Insertion Traversal
 *
 * This traversal inserts maxima and minima for index vector variables
 * in with loops. Later optimizations will propagate these maxima
 * and minima, and use then as input for other optimizations,
 * such as with-loop folding and guard removal.
 * Currently, we are looking for the following code pattern:
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

    DBUG_PRINT ("IVEXI", ("Starting index vector extrema insertion traversal."));

    TRAVpush (TR_ivexi);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DBUG_PRINT ("IVEXI", ("Index vector extrema insertion complete."));

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
IVEXIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIassign");
    DBUG_PRINT ("IVEX", ("Found assign"));
    if (NULL != ASSIGN_INSTR (arg_node)) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
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
 *   the WL partition index variable, and stuff them into the index variable's
 *   MAXVAL and MINVAL.
 *
 ******************************************************************************/
node *
IVEXIpart (node *arg_node, info *arg_info)
{
    node *partn;
    node *iv;
    node *avis;

    DBUG_ENTER ("IVEXIpart");
    DBUG_PRINT ("IVEX", ("Found WL partition"));
    if (NULL != WITH_PART (arg_node)) {
        DBUG_PRINT ("IVEX", ("GOTIT"));
        partn = WITH_PART (arg_node);
        iv = PART_WITHID (partn);
        if (N_id != NODE_TYPE (iv)) {
            DBUG_PRINT ("IVEX", ("iv node type not N_id"));
        } else {
            avis = ID_AVIS (iv);
            AVIS_MINVAL (avis) = GENERATOR_BOUND1 (PART_GENERATOR (partn));
            AVIS_MAXVAL (avis) = GENERATOR_BOUND2 (PART_GENERATOR (partn));
        }
    }
    DBUG_RETURN (arg_node);
}
