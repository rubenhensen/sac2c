/*
 *
 * $Log$
 * Revision 1.1  2004/11/20 17:19:36  sah
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include "objanalysis.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"

/*
 * INFO structure
 */
struct INFO {
    node *objects;
    int changes;
};

/*
 * INFO macros
 */
#define INFO_OAN_OBJECTS(n) ((n)->objects)
#define INFO_OAN_CHANGES(n) ((n)->changes)

/*
 * INFO functions
 */
info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_OAN_OBJECTS (result) = NULL;
    INFO_OAN_CHANGES (result) = 0;

    DBUG_RETURN (result);
}

info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/*
 * Local helper function
 */

/*
 * start of traversal
 */
void
ObjectAnalysis (node *syntax_tree)
{
    info *info;
    funtab *store_tab;

    DBUG_ENTER ("ObjectAnalysis");

    info = MakeInfo ();

    store_tab = act_tab;
    act_tab = oan_tab;

    syntax_tree = Trav (syntax_tree, info);

    act_tab = store_tab;

    info = FreeInfo (info);

    DBUG_VOID_RETURN;
}

/*
 * Traversal functions
 */
node *
OANModul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OANModul");

    /*
     * iterate until the set of objects does not
     * change any more
     */
    do {
        /*
         * reset changes counter
         */
        INFO_OAN_CHANGES (arg_info) = 0;

        /*
         * we have to trust the programmer for FUNDECS,
         * so we only traverse FUNS
         */

        if (MODUL_FUNS (arg_node) != NULL) {
            MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
        }

        DBUG_PRINT ("OAN",
                    ("last iteration added %d objects.", INFO_OAN_CHANGES (arg_info)));

    } while (INFO_OAN_CHANGES (arg_info) != 0);

    DBUG_RETURN (arg_node);
}

node *
OANId (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OANId");

    if (GET_FLAG (ID, arg_node, IS_GLOBAL)) {
        /*
         * we found a global object, so add the objdef to the list
         */

        DBUG_ASSERT ((ID_OBJDEF (arg_node) != NULL), "found a global id without objdef!");

        INFO_OAN_CHANGES (arg_info)
          += AddLinkToLinks (&INFO_OAN_OBJECTS (arg_info), ID_OBJDEF (arg_node));
    }

    DBUG_RETURN (arg_node);
}

node *
OANAp (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OANAp");

    INFO_OAN_CHANGES (arg_info)
      += AddLinksToLinks (&INFO_OAN_OBJECTS (arg_info),
                          FUNDEF_OBJECTS (AP_FUNDEF (arg_node)));

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
OANFundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OANFundef");

    DBUG_ASSERT ((INFO_OAN_OBJECTS (arg_info) == NULL),
                 "entering fundef with objects left over ?!?");

    /*
     * only process local functions, all others do have
     * correct annotations already!
     */
    if (GET_FLAG (FUNDEF, arg_node, IS_LOCAL)) {
        INFO_OAN_OBJECTS (arg_info) = FUNDEF_OBJECTS (arg_node);

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        }

        FUNDEF_OBJECTS (arg_node) = INFO_OAN_OBJECTS (arg_info);
        INFO_OAN_OBJECTS (arg_info) = NULL;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
