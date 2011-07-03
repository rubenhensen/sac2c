/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup Move Syncs
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * ============================================================================
 * can be called on N_module               |   -----   | y   |       |
 * can be called on N_fundef               |   -----   | y   |       |
 * expects LaC funs                        |   -----   | *   |       |
 * follows N_ap to LaC funs                |   -----   | n   |       |
 * ============================================================================
 * deals with GLF properly                 |    yes    | y   |       |
 * ============================================================================
 * is aware of potential SAA annotations   |    yes    | y   |       |
 * utilises SAA annotations                |   -----   | n   |       |
 * ============================================================================
 * tolerates flattened N_array             |    yes    | y   |       |
 * tolerates flattened Generators          |    yes    | y   |       |
 * tolerates flattened operation parts     |    yes    | y   |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    | y   |       |
 * ============================================================================
 * tolerates multi-operator WLs            |    yes    | y** |       |
 * ============================================================================
 * *  Does not move syncs into or out of LaC funs
 * ** Untested
 * </pre>
 *
 * This traversal moves _syncin_ prf to just before the first use of there
 * lhs.
 *
 * @ingroup tt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file move_syncs.c
 *
 * Prefix: MS
 *
 *****************************************************************************/
#include "move_syncs.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "MS"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "pattern_match.h"
#include "move_assigns.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
};

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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
 * @fn node *MSdoMoveSyncs( node *syntax_tree)
 *
 *****************************************************************************/
node *
MSdoMoveSyncs (node *syntax_tree)
{
    info *info;
    pattern *pat;
    pattern *stop_pat;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_PRINT ("Starting move syncs traversal.");

    pat = PMprf (1, PMAisPrf (F_syncin), 0);

    stop_pat = PMfalse (0, 0);

    syntax_tree = MAdoMoveAssigns (syntax_tree, pat, FALSE, 0, stop_pat);

    DBUG_PRINT ("Move syncs traversal complete.");

    pat = PMfree (pat);
    stop_pat = PMfree (stop_pat);

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
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

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
