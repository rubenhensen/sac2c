/*
 *
 * $Log$
 * Revision 1.2  1995/10/20 09:29:02  cg
 * first working revision
 *
 * Revision 1.1  1995/10/19  11:04:05  cg
 * Initial revision
 *
 *
 *
 */

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "traverse.h"

/*
 *
 *  functionname  : Analysis
 *  arguments     : 1) syntax tree
 *  description   : starts traversal mechanism for function analysis
 *                  For each not imported function, it is analysed
 *                  which types, functions and global objects are needed
 *                  in its body.
 *                  This information is needed later for handling global
 *                  objects and writing SIBs
 *  global vars   : act_tab, analy_tab
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
Analysis (node *syntaxtree)
{
    DBUG_ENTER ("Analysis");

    act_tab = analy_tab;

    DBUG_RETURN (Trav (syntaxtree, NULL));
}

/*
 *
 *  functionname  : ANAmodul
 *  arguments     : 1) N_modul node of syntax tree
 *                  2) arg_info unused
 *  description   : If the module has functions, these are traversed.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
ANAmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ANAmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ANAfundef
 *  arguments     : 1) pointer to N_fundef node
 *                  2) arg_info unused
 *  description   : If the function is not imported, its body is traversed.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
ANAfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ANAfundef");

    if (FUNDEF_STATUS (arg_node) == ST_regular) {
        Trav (FUNDEF_BODY (arg_node), arg_node);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ANAvardec
 *  arguments     : 1) vardec node
 *                  2) fundef node which contains this vardec
 *  description   : If information about the type of the declared variable
 *                  is available, then the respective typedef node is
 *                  added to this functions's list of needed types.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : InsertNode, Trav
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
ANAvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ANAvardec");

    if (VARDEC_TYPEDEF (arg_node) != NULL) {
        FUNDEF_NEEDTYPES (arg_info) = InsertNode (VARDEC_TYPEDEF (arg_node), arg_info);
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ANAid
 *  arguments     : 1) id node
 *                  2) fundef node which contains this vardec
 *  description   : If the id node represents a global object, then the
 *                  respective objdef node is added to the function's
 *                  list of needed global objects.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : InsertNode
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
ANAid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ANAid");

    if (ID_ATTRIB (arg_node) == ST_global) {
        FUNDEF_NEEDOBJS (arg_info) = InsertNode (ID_OBJDEF (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ANAap
 *  arguments     : 1) ap node
 *                  2) fundef node which contains this vardec
 *  description   : The applied user-defined function is added to the
 *                  defined function's list of needed functions.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : InsertNode, Trav
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
ANAap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ANAap");

    FUNDEF_NEEDFUNS (arg_info) = InsertNode (AP_FUNDEF (arg_node), arg_info);

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
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
