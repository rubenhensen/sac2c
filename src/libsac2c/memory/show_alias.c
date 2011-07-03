/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup mm Memory Management
 *
 * This group includes all the files needed for introducing explicit
 * memory management instructions and optimizing that representation.
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file show_alias.c
 *
 * Prefix: SHAL
 *
 *****************************************************************************/

#include "show_alias.h"

#include "phase.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "globals.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "print.h"

/** <!--********************************************************************-->
 *
 * @fn node *SHALprintPreFun( node *arg_node, info *arg_info)
 *
 * Printing prefun that prints informations about aliasing if the compilation
 * is aborted during memory management.
 *
 *****************************************************************************/

node *
SHALprintPreFun (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (NODE_TYPE (arg_node)) {
    case N_arg:
        if (ARG_ISALIASING (arg_node)) {
            fprintf (global.outfile, " /* ALIAS */");
        }
        if (AVIS_ISALIAS (ARG_AVIS (arg_node))) {
            fprintf (global.outfile, " /* alias */");
        }
        break;
    case N_ret:
        if (RET_ISALIASING (arg_node)) {
            fprintf (global.outfile, " /* ALIAS */");
        }
        break;
    case N_vardec:
        if (AVIS_ISALIAS (VARDEC_AVIS (arg_node))) {
            INDENT;
            fprintf (global.outfile, " /* alias */\n");
        }
        break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SHALactivate( node *syntax_tree)
 *
 * Subphase start frunction that activates printing of alias information
 *
 *****************************************************************************/

node *
SHALactivate (node *syntax_tree)
{
    DBUG_ENTER ();

    TRAVsetPreFun (TR_prt, SHALprintPreFun);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *SHALdeactivate( node *syntax_tree)
 *
 * Subphase start frunction that activates printing of alias information
 *
 *****************************************************************************/

node *
SHALdeactivate (node *syntax_tree)
{
    DBUG_ENTER ();

    TRAVsetPreFun (TR_prt, NULL);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Memory Management -->
 *****************************************************************************/

#undef DBUG_PREFIX
