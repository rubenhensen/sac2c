/*
 *
 * $Log$
 * Revision 2.2  2000/02/23 10:07:56  dkr
 * cond-, do- and while-dummy-functions are now traversed, too.
 *
 * Revision 2.1  1999/02/23 12:41:57  sacbase
 * new release made
 *
 * Revision 1.10  1999/01/07 10:50:49  cg
 * Added function ANAnwithop() and ANAfoldfun() which infer the dependence
 * of a function on its fold operations both for the old and for the new
 * with-loop.
 *
 * Revision 1.9  1998/12/02 16:32:42  cg
 * Now, generic object creation functions for imported global objects
 * have status ST_objinitfun rather than ST_imported. As a consequence,
 * functions with this status may have no body. This fact had to be
 * considered.
 *
 * Revision 1.8  1996/01/26 15:32:21  cg
 * function status ST_classfun now supported
 *
 * Revision 1.7  1995/12/01  17:20:31  cg
 * objinitfuns are now analysed as well.
 *
 * Revision 1.6  1995/11/01  16:29:55  cg
 * bug fixed in function ANAap. Now, functions are stored for all
 * functions, not only inline functions, which is necessary for finding
 * all needed global objects.
 *
 * Revision 1.5  1995/10/31  09:00:31  cg
 * Information about needed types and function is only stored
 * for inline functions.
 *
 * Revision 1.4  1995/10/20  16:55:01  cg
 * Now, indirectly needed global objects are concerned as well.
 * These are those global objects which do not occur in a function's
 * body but are needed by another function which is applied in the
 * body of the first one.
 *
 * Revision 1.3  1995/10/20  13:51:10  cg
 * calls to function InsertNode are extended to 3 parameters
 *
 * Revision 1.2  1995/10/20  09:29:02  cg
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
 *                  objects and writing SIBs.
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
 *  functionname  : FindAllNeededObjects
 *  arguments     : 1) fundef node
 *  description   : This function takes care of those global objects which
 *                  are only indirectly needed by the given function, i.e.
 *                  global objects which are needed by applied functions.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : StoreNeededNodes, StoreUnresolvedNodes
 *  macros        : ---
 *
 *  remarks       : Since only fundef nodes are traversed, the universal
 *                  traversal mechanism is not used to spare one table
 *
 */

node *
FindAllNeededObjects (node *arg_node)
{
    nodelist *tmp;

    DBUG_ENTER ("FindAllNeededObjects");

#if 0
  if ((FUNDEF_STATUS(arg_node)==ST_regular)
      || ((FUNDEF_STATUS(arg_node)==ST_objinitfun)
          && (FUNDEF_BODY(arg_node)!=NULL))) {
#endif
    if ((FUNDEF_STATUS (arg_node) != ST_imported) && (FUNDEF_BODY (arg_node) != NULL)) {
        /*
         *  For each not imported function the list of called functions
         *  is traversed.
         */

        DBUG_PRINT ("ANA", ("Collecting global objects for '%s`", ItemName (arg_node)));

        tmp = FUNDEF_NEEDFUNS (arg_node);
        while (tmp != NULL) {
            /*
             *  First, the global objects needed by the called function are
             *  added to this function's list of needed global objects.
             */

            StoreNeededNodes (FUNDEF_NEEDOBJS (NODELIST_NODE (tmp)), arg_node,
                              ST_artificial);

            if ((FUNDEF_STATUS (NODELIST_NODE (tmp)) == ST_regular)
                || ((FUNDEF_STATUS (NODELIST_NODE (tmp)) == ST_objinitfun)
                    && (FUNDEF_BODY (NODELIST_NODE (tmp)) != NULL))) {
                /*
                 *  If the called function is not imported, its called functions
                 *  are added to this function's list of needed functions.
                 *  Only those functions are concerned which still have attribute
                 *  ST_unresolved which means that they have not been treated
                 *  by this algorithm yet.
                 */

                StoreUnresolvedNodes (FUNDEF_NEEDFUNS (NODELIST_NODE (tmp)), arg_node,
                                      ST_artificial);
            }

            NODELIST_ATTRIB (tmp) = ST_resolved;

            tmp = NODELIST_NEXT (tmp);
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = FindAllNeededObjects (FUNDEF_NEXT (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ANAmodul
 *  arguments     : 1) N_modul node of syntax tree
 *                  2) arg_info unused
 *  description   : If the module has functions, these are traversed.
 *  global vars   : ---
 *  internal funs : FindAllNeededObjects
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

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = FindAllNeededObjects (MODUL_FUNS (arg_node));
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
 *  external funs : Trav, TidyUpNodelist
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
ANAfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ANAfundef");

#if 0
  if ((FUNDEF_STATUS( arg_node) == ST_regular) ||
     ((FUNDEF_STATUS( arg_node) == ST_objinitfun) && (FUNDEF_BODY( arg_node) != NULL))) {
#endif
    if ((FUNDEF_STATUS (arg_node) != ST_imported) && (FUNDEF_BODY (arg_node) != NULL)) {
        Trav (FUNDEF_BODY (arg_node), arg_node);
        FUNDEF_NEEDTYPES (arg_node) = TidyUpNodelist (FUNDEF_NEEDTYPES (arg_node));
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
 *  external funs : StoreNeededNode, Trav
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
ANAvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ANAvardec");

    if (FUNDEF_INLINE (arg_info)) {
        if (VARDEC_TYPEDEF (arg_node) != NULL) {
            StoreNeededNode (VARDEC_TYPEDEF (arg_node), arg_info, ST_regular);
        }

        if (VARDEC_NEXT (arg_node) != NULL) {
            VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
        }
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
 *  external funs : StoreNeededNode
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
        StoreNeededNode (ID_OBJDEF (arg_node), arg_info, ST_regular);
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
 *  external funs : StoreNeededNode, Trav
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
ANAap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ANAap");

    StoreNeededNode (AP_FUNDEF (arg_node), arg_info, ST_regular);

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ANAnwithop
 *  arguments     : 1) N_Nwithop node
 *                  2) fundef node which contains this with-loop
 *  description   : The flattened fold-operation is always required
 *                  by the function containing a fold with-loop
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : StoreNeededNode
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
ANAnwithop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ANAnwithop");

    if ((NWITHOP_TYPE (arg_node) == WO_foldfun)
        || (NWITHOP_TYPE (arg_node) == WO_foldprf)) {
        StoreNeededNode (NWITHOP_FUNDEF (arg_node), arg_info, ST_regular);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ANAfoldfun
 *  arguments     : 1) N_foldfun node
 *                  2) fundef node which contains this with-loop
 *  description   : The fold-operation is always required
 *                  by the function containing a fold with-loop
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : StoreNeededNode
 *  macros        : ---
 *
 *  remarks       : This function is only required to maintain compatibility
 *                  with old with-loop.
 *
 */

node *
ANAfoldfun (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ANAfoldfun");

    StoreNeededNode (FOLDFUN_FUNDEF (arg_node), arg_info, ST_regular);

    DBUG_RETURN (arg_node);
}
