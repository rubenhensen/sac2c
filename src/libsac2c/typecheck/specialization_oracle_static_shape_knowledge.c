#include "specialization_oracle_static_shape_knowledge.h"

#include "tree_basic.h"
#include "tree_compound.h"
/*#include "str.h"*/
#include "memory.h"
/*
#include "shape.h"
#include "new_types.h"*/
#include "dbug.h"
#include "traverse.h"
#include "constants.h"
#include "shape.h"
/*#include "DupTree.h"
#include "free.h"
*/

/**
 *
 * @specialistation_oracle_static_shape_knowledge.c
 *
 * @brief
 *
 */

/**
 * INFO structure
 */
struct INFO {
};

/**
 * INFO macros
 */

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node N_const node
 * @param arg_info INFO structure
 *
 * @return N_const node
 *******************************************************************************/
node *
SOSSKconst (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKconst");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node N_id node
 * @param arg_info INFO structure
 *
 * @return N_id node
 *******************************************************************************/
node *
SOSSKid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKid");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node N_prf node
 * @param arg_info INFO structure
 *
 * @return N_prf node
 *******************************************************************************/
node *
SOSSKprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKprf");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node N_fundef node
 * @param arg_info INFO structure
 *
 * @return N_fundef node
 *******************************************************************************/
node *
SOSSKfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKfundef");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node N_let node
 * @param arg_info INFO structure
 *
 * @return N_let node
 *******************************************************************************/
node *
SOSSKlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKlet");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node N_with node
 * @param arg_info INFO structure
 *
 * @return N_with node
 *******************************************************************************/
node *
SOSSKwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKwith");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param syntax_tree N_module node
 *
 * @return transformed syntax tree
 *******************************************************************************/
node *
SOSSKdoSpecializationOracleSSK (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("SOSSKdoSpecializationOracleSSK");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "SOSSKdoSpecializationOracleSSK is intended to run on the entire tree");

    info = MakeInfo ();
    TRAVpush (TR_sossk);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
