/* $Id$ */

#include "object_analysis.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "new_types.h"
#include "DupTree.h"
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
#define INFO_OBJECTS(n) ((n)->objects)
#define INFO_CHANGES(n) ((n)->changes)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_OBJECTS (result) = NULL;
    INFO_CHANGES (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 * Local helper function
 */

static node *
CollectObjects (node *fundef, info *info)
{
    DBUG_ENTER ("CollectObjects");

    TCaddLinksToLinks (&INFO_OBJECTS (info), FUNDEF_OBJECTS (fundef));

    DBUG_RETURN (fundef);
}

static node *
ProjectObjects (node *fundef, info *info)
{
    DBUG_ENTER ("ProjectObjects");

    if (FUNDEF_OBJECTS (fundef) != NULL) {
        FUNDEF_OBJECTS (fundef) = FREEdoFreeTree (FUNDEF_OBJECTS (fundef));
    }
    if (INFO_OBJECTS (info) != NULL) {
        FUNDEF_OBJECTS (fundef) = DUPdoDupTree (INFO_OBJECTS (info));
    }

    DBUG_RETURN (fundef);
}

static void
UnifyOverloadedFunctions (node *funs, info *info)
{
    DBUG_ENTER ("UnifyOverloadedFunctions");

    while (funs != NULL) {
        if (FUNDEF_ISWRAPPERFUN (funs)) {
            if (TYisFun (FUNDEF_WRAPPERTYPE (funs))) {
                INFO_OBJECTS (info) = FUNDEF_OBJECTS (funs);

                FUNDEF_WRAPPERTYPE (funs)
                  = TYmapFunctionInstances (FUNDEF_WRAPPERTYPE (funs), CollectObjects,
                                            info);

                FUNDEF_WRAPPERTYPE (funs)
                  = TYmapFunctionInstances (FUNDEF_WRAPPERTYPE (funs), ProjectObjects,
                                            info);

                FUNDEF_OBJECTS (funs) = INFO_OBJECTS (info);
                INFO_OBJECTS (info) = NULL;
            } else {
                if (FUNDEF_OBJECTS (funs) != NULL) {
                    FUNDEF_OBJECTS (funs) = FREEdoFreeTree (FUNDEF_OBJECTS (funs));
                }
                if (FUNDEF_OBJECTS (FUNDEF_IMPL (funs)) != NULL) {
                    FUNDEF_OBJECTS (funs)
                      = DUPdoDupTree (FUNDEF_OBJECTS (FUNDEF_IMPL (funs)));
                }
            }
        }
        funs = FUNDEF_NEXT (funs);
    }

    DBUG_VOID_RETURN;
}

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
        INFO_CHANGES (arg_info) = 0;

        DBUG_PRINT ("OAN", ("!!! starting new iteration"));
        /*
         * we have to trust the programmer for FUNDECS,
         * so we only traverse FUNS
         */

        if (MODULE_FUNS (arg_node) != NULL) {
            MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
        }

        DBUG_PRINT ("OAN",
                    ("!!! last iteration added %d objects.", INFO_CHANGES (arg_info)));

        /*
         * propagate the infered objects to the wrappers and ensure
         * that all instances are marked for the same set of objects.
         * This is important as otherwise the signature of a wrapper
         * and its instances would not match once the objects have
         * been resolved. Furthermore, it has to be done after each
         * iteration as otherwise the object usage is not propagated
         * over wrapper calls!
         */
        DBUG_PRINT ("OAN", ("unifying dependencies of overloaded instances..."));

        UnifyOverloadedFunctions (MODULE_FUNS (arg_node), arg_info);

        DBUG_PRINT ("OAN", ("unifying completed."));
    } while (INFO_CHANGES (arg_info) != 0);

    DBUG_RETURN (arg_node);
}

node *
OANglobobj (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OANglobobj");

    DBUG_ASSERT ((GLOBOBJ_OBJDEF (arg_node) != NULL),
                 "found a global id without objdef!");

    DBUG_PRINT ("OAN", (">>> adding object %s", CTIitemName (GLOBOBJ_OBJDEF (arg_node))));

    INFO_CHANGES (arg_info)
      += TCaddLinkToLinks (&INFO_OBJECTS (arg_info), GLOBOBJ_OBJDEF (arg_node));

    DBUG_RETURN (arg_node);
}

node *
OANap (node *arg_node, info *arg_info)
{
    int newdeps;

    DBUG_ENTER ("OANap");

    DBUG_PRINT ("OAN",
                (">>> adding dependencies of %s", CTIitemName (AP_FUNDEF (arg_node))));

    newdeps = TCaddLinksToLinks (&INFO_OBJECTS (arg_info),
                                 FUNDEF_OBJECTS (AP_FUNDEF (arg_node)));

    DBUG_PRINT ("OAN", (">>> %d dependencies added", newdeps));

    INFO_CHANGES (arg_info) += newdeps;

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
OANfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OANfundef");

    DBUG_ASSERT ((INFO_OBJECTS (arg_info) == NULL),
                 "entering fundef with objects left over ?!?");

    /*
     * only process local functions, all others do have
     * correct annotations already!
     */
    if (FUNDEF_ISLOCAL (arg_node)) {
        DBUG_PRINT ("OAN", ("entering fundef %s", CTIitemName (arg_node)));

        INFO_OBJECTS (arg_info) = FUNDEF_OBJECTS (arg_node);

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }

        FUNDEF_OBJECTS (arg_node) = INFO_OBJECTS (arg_info);
        INFO_OBJECTS (arg_info) = NULL;

        DBUG_PRINT ("OAN", ("leaving fundef %s", CTIitemName (arg_node)));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}