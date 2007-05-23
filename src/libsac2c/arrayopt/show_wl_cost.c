/*
 * $Id: show_wl_cost.c dpa $
 */

/** <!--********************************************************************-->
 *
 * @defgroup swlf Symbolic With-Loop Folding
 *
 * This group includes all the files needed for symbolic with loop fold
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file show_wl_cost.c
 *
 * Prefix: SHWLC
 *
 *****************************************************************************/

#include "show_wl_cost.h"

#include "phase.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "globals.h"
#include "dbug.h"
#include "print.h"

/** <!--********************************************************************-->
 *
 * @fn node *SHWLCprintPreFun( node *arg_node, info *arg_info)
 *
 * Printing prefun that prints informations about WL-Cost.
 *
 *****************************************************************************/
node *
SHWLCprintPreFun (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SHWLCprintPreFun");

    switch (NODE_TYPE (arg_node)) {
    case N_with:
        fprintf (global.outfile, "/* WL-Cost=%d */", WITH_COST (arg_node));
        break;
    case N_avis:
        fprintf (global.outfile, "/* WL_NEEDCOUNT=%d */", AVIS_WL_NEEDCOUNT (arg_node));
        break;
    case N_ids:
        fprintf (global.outfile, "/* WL_NEEDCOUNT=%d */",
                 AVIS_WL_NEEDCOUNT (IDS_AVIS (arg_node)));
        break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SHWLactivate( node *syntax_tree)
 *
 * Subphase start function that activates printing of WL-Cost information
 *
 *****************************************************************************/
node *
SHWLCactivate (node *syntax_tree)
{
    DBUG_ENTER ("SHWLCactivate");

    TRAVsetPreFun (TR_prt, SHWLCprintPreFun);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *SHWLCdeactivate( node *syntax_tree)
 *
 * Subphase start function that activates printing of WL-Cost information
 *
 *****************************************************************************/

node *
SHWLCdeactivate (node *syntax_tree)
{
    DBUG_ENTER ("SHWLCdeactivate");

    TRAVsetPreFun (TR_prt, NULL);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Symbolic With-Loop Folding -->
 *****************************************************************************/
