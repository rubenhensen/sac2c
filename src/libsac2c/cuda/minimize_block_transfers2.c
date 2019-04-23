/**
 * @file
 * @defgroup mbtran2 Minimize Block Transfers
 * @ingroup cuda
 *
 * @brief Minimize the number of host<->device transfers in a
 *        sequential block of instructions.
 *
 * This modules tries to eliminate <host2device>/<device2host> instructions
 * in a sequential block of code. Two difference cases expose the opportunities
 * for elimination:
 *
 * 1.
 *   ~~~~
 *   a_host = device2host( b_dev);
 *   ...
 *   ...
 *   a_dev = host2device( a_host);
 *   ~~~~
 *
 *   The second memory transfer, i.e. a_dev = host2device( a_host)
 *   can be eliminated. Any reference to a_dev after it will be
 *   replaced by b_dev.
 *
 * 2.
 *   ~~~~
 *   b_dev = host2device( a_host);
 *   ...
 *   ...
 *   c_dev = host2device( a_host);
 *   ~~~~
 *
 *   The second memory transfer, i.e. c_dev = host2device( a_host)
 *   can be eliminated. Any reference to c_dev after it will be
 *   replaced by b_dev.
 *
 * @{
 */
#include "minimize_block_transfers2.h"

#include "new_types.h"
#include "tree_compound.h"
#include "free.h"
#include "globals.h"
#include "traverse.h"
#include "tree_basic.h"
#include "LookUpTable.h"
#include "memory.h"

#define DBUG_PREFIX "MBTRAN2"
#include "debug.h"

#include "deadcoderemoval.h"
#include "cuda_utils.h"
#include "SSACSE.h"
#include "DupTree.h"

/**
 * @name INFO structure
 * @{
 */
struct INFO {
    node *current_block;
    node *lastassign;
};

#define INFO_CURRENT_BLOCK(n) (n->current_block)
#define INFO_LASTASSIGN(n) (n->lastassign)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_CURRENT_BLOCK (result) = NULL;
    INFO_LASTASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** @} */

/**
 * @name Entry functions
 * @{
 */

/**
 * @brief Invoke the CUDA block transfer minimisation traversal
 * @param syntax_tree
 * @return syntax tree
 */
node *
MBTRAN2doMinimizeBlockTransfers (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_mbtran2);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("invoking CSE");
    syntax_tree = CSEdoCommonSubexpressionElimination (syntax_tree);

    /* We rely on Dead Code Removal to remove the
     * unused <host2device>/<device2host> */
    DBUG_PRINT ("invoking DCR");
    syntax_tree = DCRdoDeadCodeRemoval (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/** @} */

/**
 * @name Traversal functions
 * @{
 */

/**
 * @brief Store current N_block in info struct and traverse the N_assigns
 *
 * @param arg_node N_block
 * @param arg_info info structure
 * @return N_block
 */
node *
MBTRAN2block (node *arg_node, info *arg_info)
{
    node *old_block;

    DBUG_ENTER ();

    old_block = INFO_CURRENT_BLOCK (arg_info);
    INFO_CURRENT_BLOCK (arg_info) = arg_node;

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    INFO_CURRENT_BLOCK (arg_info) = old_block;

    DBUG_RETURN (arg_node);
}

/**
 * @brief Store the current N_assign in traverse statements top-down
 *
 * @param arg_node N_assign
 * @param arg_info info structure
 * @return N_assign
 */
node *
MBTRAN2assign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    // ASSIGN_CONTAINING_BLOCK( arg_node) = INFO_CURRENT_BLOCK( arg_info);
    INFO_LASTASSIGN (arg_info) = arg_node;

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    INFO_LASTASSIGN (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/**
 * @brief Check if `F_host2device` argument is assigned to via `F_device2host`,
 *              delete the current N_prf and replace the RHS of the assign.
 *
 * @param arg_node N_prf
 * @param arg_info info structure
 * @return N_prf
 */
node *
MBTRAN2prf (node *arg_node, info *arg_info)
{
    node *ssaassign;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_host2device:
        DBUG_PRINT ("Checking H2D to elimitating preceeding D2H");
        ssaassign = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (arg_node)));
        /*
        if( ISDEVICE2HOST( ssaassign) &&
            ( ASSIGN_CONTAINING_BLOCK( ssaassign) ==
              ASSIGN_CONTAINING_BLOCK( INFO_LASTASSIGN( arg_info)))) {
              ( */
        if (ISDEVICE2HOST (ssaassign)) {
            DBUG_PRINT ("...eliminating H2D and replacing LHS");
            node *dev_id = PRF_ARG1 (ASSIGN_RHS (ssaassign));
            node *dev_avis = ID_AVIS (dev_id);
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = TBmakeId (dev_avis);
            global.optcounters.cuda_min_trans++;
        }
        break;
    default:
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/** @} */
/** @} */
#undef DBUG_PREFIX
