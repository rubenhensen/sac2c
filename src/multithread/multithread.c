/*
 *
 * $Log$
 * Revision 1.2  2000/01/21 13:10:12  jhs
 * Added infrastructure for new mt support
 *
 * Revision 1.1  2000/01/21 11:11:38  jhs
 * Initial revision
 *
 *
 */

/******************************************************************************
 *
 * file:        multithread.c
 *
 * prefix:      MUTH
 *
 * description:
 *   This file initiates and guides the compilation process of the new
 *   multithread-support.
 *   ... The entire process is still under development ...
 *
 ******************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "globals.h"
#include "free.h"
#include "Error.h"

/******************************************************************************
 *
 * function:
 *   node *BuildMultiThread( node *syntax_tree)
 *
 * description:
 *   This function starts the process of building the *new* support for
 *   multithread. Throughout this process arg_info points to an N_info which
 *   is generated here.
 *
 ******************************************************************************/
node *
BuildMultiThread (node *syntax_tree)
{
    node *arg_info;

    DBUG_ENTER ("BuildMultiThread");

    arg_info = MakeInfo ();

    /* act_tab = muth_tab; */

    /* syntax_tree = Trav(syntax_tree, arg_info); */
    NOTE (("*** nothing implemented yet ***"));

    FREE (arg_info);

    DBUG_RETURN (syntax_tree);
}
