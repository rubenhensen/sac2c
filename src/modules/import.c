/*
 *
 * $Log$
 * Revision 1.1  1994/12/16 14:39:17  sbs
 * Initial revision
 *
 *
 */

/*
 * This file contains .....
 */

#include "tree.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"

#include "traverse.h"

#include "import.h"

/*
 *
 *  functionname  : Import
 *  arguments     : 1) syntax tree
 *  description   : Recursively scans and parses modul.dec's
 *  global vars   : syntax_tree, act_tab, imp_tab
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
Import (node *arg_node)
{

    DBUG_ENTER ("Import");

    act_tab = imp_tab;

    DBUG_RETURN (Trav (arg_node, NULL));
}

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IMmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IMmodul");
    DBUG_RETURN (arg_node);
}
