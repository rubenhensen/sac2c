/*
 *
 * $Log$
 * Revision 1.2  2004/11/26 20:27:30  jhb
 * ccompile
 *
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
#include "free.h"
#include "internal_lib.h"

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

    result = ILIBmalloc (sizeof (info));

    INFO_OAN_OBJECTS (result) = NULL;
    INFO_OAN_CHANGES (result) = 0;

    DBUG_RETURN (result);
}

info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 * Local helper function
 */

/*
 * start of traversal
 */
node *
OANdoObjectAnalysis (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("OANdoObjectAnalysis");

    info = MakeInfo ();

    TRAVpush (TR_oan);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/*
 * Traversal functions
 */
node *
OANmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OANModule");

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

        if (MODULE_FUNS (arg_node) != NULL) {
            MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
        }

        DBUG_PRINT ("OAN",
                    ("last iteration added %d objects.", INFO_OAN_CHANGES (arg_info)));

    } while (INFO_OAN_CHANGES (arg_info) != 0);

    DBUG_RETURN (arg_node);
}

node *
OANglobobj (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OANglobobj");

    DBUG_ASSERT ((GLOBOBJ_OBJDEF (arg_node) != NULL),
                 "found a global id without objdef!");

    INFO_OAN_CHANGES (arg_info)
      += TCaddLinkToLinks (&INFO_OAN_OBJECTS (arg_info), GLOBOBJ_OBJDEF (arg_node));

    DBUG_RETURN (arg_node);
}

node *
OANap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OANap");

    INFO_OAN_CHANGES (arg_info)
      += TCaddLinksToLinks (&INFO_OAN_OBJECTS (arg_info),
                            FUNDEF_OBJECTS (AP_FUNDEF (arg_node)));

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
OANfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OANfundef");

    DBUG_ASSERT ((INFO_OAN_OBJECTS (arg_info) == NULL),
                 "entering fundef with objects left over ?!?");

    /*
     * only process local functions, all others do have
     * correct annotations already!
     */
    if (FUNDEF_ISLOCAL (arg_node)) {
        INFO_OAN_OBJECTS (arg_info) = FUNDEF_OBJECTS (arg_node);

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }

        FUNDEF_OBJECTS (arg_node) = INFO_OAN_OBJECTS (arg_info);
        INFO_OAN_OBJECTS (arg_info) = NULL;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
