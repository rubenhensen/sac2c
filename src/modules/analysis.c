/*
 *
 * $Log$
 * Revision 3.7  2004/07/17 19:50:26  sah
 * switch to INFO structure
 * PHASE I
 *
 * Revision 3.6  2003/11/18 17:37:23  dkr
 * ANAnwithop(): call of TravSons() added
 *
 * Revision 3.5  2002/10/18 13:37:37  sbs
 * access to ID_ATTRIB replaced by FLAG inspection.
 *
 * Revision 3.4  2001/03/05 16:42:04  dkr
 * no macros NWITH???_IS_FOLD used
 *
 * Revision 3.3  2001/02/14 17:50:57  dkr
 * redundant VARDEC_TYPEDEF replaced by VARDEC_TDEF
 *
 * Revision 3.2  2000/11/24 14:51:13  nmw
 * analysis ignores fundefs marked as ST_ignore
 *
 * Revision 3.1  2000/11/20 18:00:47  sacbase
 * new release made
 *
 * Revision 2.5  2000/10/31 18:10:58  cg
 * Added support for new function tag ST_exported.
 *
 * Revision 2.4  2000/05/30 12:35:36  dkr
 * functions for old with-loop removed
 *
 * Revision 2.3  2000/02/23 20:16:34  cg
 * Node status ST_imported replaced by ST_imported_mod and
 * ST_imported_class in order to allow distinction between enteties
 * that are imported from a module and those that are imported from a
 * class.
 *
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
 */

#define NEW_INFO

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
};

/*
 * INFO macros
 */
#define INFO_ANA_FUNDEF(n) (n->fundef)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_ANA_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

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
 *
 */

node *
Analysis (node *syntaxtree)
{
    info *info;

    DBUG_ENTER ("Analysis");

    act_tab = analy_tab;
    info = MakeInfo ();

    syntaxtree = Trav (syntaxtree, info);

    info = FreeInfo (info);

    DBUG_RETURN (syntaxtree);
}

/*
 *
 *  functionname  : FindAllNeededObjects
 *  arguments     : 1) fundef node
 *  description   : This function takes care of those global objects which
 *                  are only indirectly needed by the given function, i.e.
 *                  global objects which are needed by applied functions.
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

    if ((FUNDEF_STATUS (arg_node) != ST_imported_mod)
        && (FUNDEF_STATUS (arg_node) != ST_imported_class)
        && (FUNDEF_STATUS (arg_node) != ST_ignore) && (FUNDEF_BODY (arg_node) != NULL)) {
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

            if ((FUNDEF_STATUS (NODELIST_NODE (tmp)) != ST_imported_mod)
                && (FUNDEF_STATUS (NODELIST_NODE (tmp)) != ST_imported_class)
                && (FUNDEF_STATUS (NODELIST_NODE (tmp)) != ST_ignore)
                && (FUNDEF_BODY (NODELIST_NODE (tmp)) != NULL)) {
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
 *
 */

node *
ANAmodul (node *arg_node, info *arg_info)
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
 *
 */

node *
ANAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANAfundef");

    if ((FUNDEF_STATUS (arg_node) != ST_imported_mod)
        && (FUNDEF_STATUS (arg_node) != ST_imported_class)
        && (FUNDEF_STATUS (arg_node) != ST_ignore) && (FUNDEF_BODY (arg_node) != NULL)) {
        /* store fundef in INFO structure */
        node *oldfundef = INFO_ANA_FUNDEF (arg_info);
        INFO_ANA_FUNDEF (arg_info) = arg_node;

        Trav (FUNDEF_BODY (arg_node), arg_info);

        /* reset value (should be NULL) */
        INFO_ANA_FUNDEF (arg_info) = oldfundef;
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
 *
 */

node *
ANAvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANAvardec");

    if (FUNDEF_INLINE (INFO_ANA_FUNDEF (arg_info))) {
        if (VARDEC_TDEF (arg_node) != NULL) {
            StoreNeededNode (VARDEC_TDEF (arg_node), INFO_ANA_FUNDEF (arg_info),
                             ST_regular);
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
 *
 */

node *
ANAid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANAid");

    if (GET_FLAG (ID, arg_node, IS_GLOBAL)) {
        StoreNeededNode (ID_OBJDEF (arg_node), INFO_ANA_FUNDEF (arg_info), ST_regular);
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
 *
 */

node *
ANAap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANAap");

    StoreNeededNode (AP_FUNDEF (arg_node), INFO_ANA_FUNDEF (arg_info), ST_regular);

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *ANAnwithop( node *arg_node, info *arg_info)
 *
 * Description:
 *   The flattened fold-operation is always required by the function containing
 *   a fold with-loop.
 *
 ******************************************************************************/

node *
ANAnwithop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANAnwithop");

    if (NWITHOP_IS_FOLD (arg_node)) {
        StoreNeededNode (NWITHOP_FUNDEF (arg_node), INFO_ANA_FUNDEF (arg_info),
                         ST_regular);
    }

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}
