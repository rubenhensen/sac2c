/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup msa Mark SubAlloc template
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * =============================================================================
 * can be called on N_module               |   -----   | y   |       |
 * can be called on N_fundef               |   -----   | y   |       |
 * expects LaC funs                        |   -----   | n   |       |
 * follows N_ap to LaC funs                |   -----   | n   |       |
 * =============================================================================
 * deals with GLF properly                 |    yes    | y   |       |
 * =============================================================================
 * is aware of potential SAA annotations   |    yes    | y   |       |
 * utilises SAA annotations                |   -----   | n   |       |
 * =============================================================================
 * tolerates flattened N_array             |    yes    | y   |       |
 * tolerates flattened Generators          |    yes    | y   |       |
 * tolerates flattened operation parts     |    yes    | y   |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    | y   |       |
 * =============================================================================
 * tolerates multi-operator WLs            |    yes    | y   |       |
 * =============================================================================
 * </pre>
 *
 * Mark suballocs memory from suballoc
 *
 * @ingroup tt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file marksuballoc.c
 *
 * Prefix: MSA
 *
 *****************************************************************************/
#include "marksuballoc.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"
/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool isSuballoc;
};

#define INFO_IS_SUBALLOC(n) (n->isSuballoc)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_IS_SUBALLOC (result) = FALSE;

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
 * @fn node *MSAdoMarkSubAlloc( node *syntax_tree)
 *
 *****************************************************************************/
node *
MSAdoMarkSubAlloc (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("MSAdoMarkSuballoc");

    info = MakeInfo ();

    DBUG_PRINT ("MSA", ("Starting mark suballoc traversal."));

    TRAVpush (TR_msa);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("MSA", ("Mark suballoc traversal complete."));

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
 *
 * @fn node *MSAlet(node *arg_node, info *arg_info)
 *
 * @brief marks lhs if it is a suballoc
 *
 *****************************************************************************/
node *
MSAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MSAlet");

    INFO_IS_SUBALLOC (arg_info) = FALSE;
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_IS_SUBALLOC (arg_info)) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }
    INFO_IS_SUBALLOC (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MSAids(node *arg_node, info *arg_info)
 *
 * @brief marks avis of lhs
 *
 *****************************************************************************/
node *
MSAids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MSAids");

    if (INFO_IS_SUBALLOC (arg_info)) {
        AVIS_TYPE (IDS_AVIS (arg_node))
          = TYsetUnique (AVIS_TYPE (IDS_AVIS (arg_node)), TRUE);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MSAprf(node *arg_node, info *arg_info)
 *
 * @brief is this a suballoc? -> INFO_IS_SUBALLOC
 *
 *****************************************************************************/
node *
MSAprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MSAprf");

    if (PRF_PRF (arg_node) == F_suballoc) {
        INFO_IS_SUBALLOC (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
