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
    info *info;

    DBUG_ENTER ("IVEXIdoIndexVectorExtremaInsertion");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "IVEXIdoIndexVectorExtremaInsertion must only be called on entire "
                 "modules!");

    info = MakeInfo ();

    DBUG_PRINT ("IVEXI", ("Starting index vector extrema insertion traversal."));

    TRAVpush (TR_ivexi);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    DBUG_PRINT ("IVEXI", ("Index vector extrema insertion complete."));

    info = FreeInfo (info);

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
 *
 * description:
 *
 ******************************************************************************/
node *
IVEXImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXImodule");
    DBUG_RETURN (arg_node);
}

node *
IVEXIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIfundef");
    DBUG_RETURN (arg_node);
}

node *
IVEXIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIblock");
    DBUG_RETURN (arg_node);
}

node *
IVEXIwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIwith");
    DBUG_RETURN (arg_node);
}

node *
IVEXIgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIgenerator");
    DBUG_RETURN (arg_node);
}
