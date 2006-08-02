/* $Id$ */

/*
 * The task of the withloop opbject analysis traversal is to find with-loops
 * that reference objects in their body, and make sure the objects are
 * returned from the bodies to the surrounding code, as follows:
 *
 * BEFORE:
 * -------
 *
 * a = with ( iv )  {
 *       ...
 *       stdout = print ( stdout, f ( iv ) );
 *       ...
 *     } : b
 *     genarray ( shp (b), ... );
 *
 * AFTER:
 * ------
 *
 * a, stdout = with ( iv ) {
 *       ...
 *       stdout = print ( stdout, f ( iv ) );
 *       ...
 *     } : b, stdout
 *     genarray ( shp ( b), ...)
 *     extract ( stdout);
 */

#include "withloop_objects.h"

#include "ctinfo.h"
#include "dbug.h"
#include "DupTree.h"
#include "globals.h"
#include "internal_lib.h"
#include "new_types.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "type_utils.h"
#include "user_types.h"

/*
 * INFO structure
 */

struct INFO {
    bool inwithloop;
    node *objects;
};

/*
 * INFO macros
 */

#define INFO_INWITHLOOP(n) ((n)->inwithloop)
#define INFO_OBJECTS(n) ((n)->objects)

/*
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_INWITHLOOP (result) = FALSE;
    INFO_OBJECTS (result) = NULL;

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
 * Local helper functions
 */

static node *
AddObjectsToWithExprs (node *withexprs, node *objects)
{
    node *exprs;
    node *object;
    node *avis;

    DBUG_ENTER ("AddObjectsToWithExprs");

    /* Append to the last exprs to match the extract() ops */
    exprs = withexprs;
    while (EXPRS_NEXT (exprs) != NULL) {
        exprs = EXPRS_NEXT (exprs);
    }

    object = objects;
    while (object != NULL) {
        avis = ID_AVIS (EXPRS_EXPR (object));
        EXPRS_NEXT (exprs) = TBmakeExprs (TBmakeId (avis), NULL);
        exprs = EXPRS_NEXT (exprs);
        object = EXPRS_NEXT (object);
    }

    DBUG_RETURN (withexprs);
}

static node *
AddObjectsToWithOps (node *withops, node *objects)
{
    node *withop;
    node *object;

    DBUG_ENTER ("AddObjectsToWithOps");

    /* Fast-forward to the last withop */
    withop = withops;
    while (WITHOP_NEXT (withop) != NULL) {
        withop = WITHOP_NEXT (withop);
    }

    /* Append an extract() withop for every object */
    object = objects;
    while (object != NULL) {
        /* Use the original object as default element */
        L_WITHOP_NEXT (withop, TBmakeExtract (DUPdoDupTree (EXPRS_EXPR (object))));
        withop = WITHOP_NEXT (withop);
        object = EXPRS_NEXT (object);
    }

    DBUG_RETURN (withops);
}

static node *
AddObjectsToWithLoop (node *withnode, node *objects)
{
    node *withexprs;

    DBUG_ENTER ("AddObjectsToWithLoop");

    /* Add the objects the the withloop's exprs */
    withexprs = CODE_CEXPRS (WITH_CODE (withnode));
    CODE_CEXPRS (WITH_CODE (withnode)) = AddObjectsToWithExprs (withexprs, objects);

    /* Add a extract withop for each object */
    WITH_WITHOP (withnode) = AddObjectsToWithOps (WITH_WITHOP (withnode), objects);

    DBUG_RETURN (withnode);
}

static node *
AddObjectsToLHS (node *lhs_ids, node *objects)
{
    node *ids;
    node *object;
    node *avis;

    DBUG_ENTER ("AddObjectsToLHS");

    /* Append to the last exprs to match the extract() ops */
    ids = lhs_ids;
    while (IDS_NEXT (ids) != NULL) {
        ids = IDS_NEXT (ids);
    }

    object = objects;
    while (object != NULL) {
        avis = ID_AVIS (EXPRS_EXPR (object));
        IDS_NEXT (ids) = TBmakeIds (avis, NULL);
        ids = IDS_NEXT (ids);
        object = EXPRS_NEXT (object);
    }

    DBUG_RETURN (lhs_ids);
}

/*
 * Traversal functions
 */

node *
WOAid (node *arg_node, info *arg_info)
{
    node *avis;
    ntype *type;

    DBUG_ENTER ("WOAid");

    if (INFO_INWITHLOOP (arg_info) == TRUE) {

        avis = ID_AVIS (arg_node);
        type = AVIS_TYPE (avis);

        if (TYisArray (type) && TUisUniqueUserType (TYgetScalar (type))) {
            /*
             * This is an object referenced from within a with-loop. Add it
             * to the list of objects that will be attached to the with-loop.
             */
            INFO_OBJECTS (arg_info) = TBmakeExprs (arg_node, INFO_OBJECTS (arg_info));
        }
    }

    arg_node = TRAVcont (arg_node, arg_info);
    DBUG_RETURN (arg_node);
}

node *
WOAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WOAlet");

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

        if (NODE_TYPE (LET_EXPR (arg_node)) == N_with) {
            /*
             * This is a let with a with-loop RHS. Add the with-loop's objects,
             * which should now be in arg_info, to the LHS expressions.
             */
            if (INFO_OBJECTS (arg_info) != NULL) {
                LET_IDS (arg_node)
                  = AddObjectsToLHS (LET_IDS (arg_node), INFO_OBJECTS (arg_info));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

node *
WOAwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WOAwith");

    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    if (WITH_CODE (arg_node) != NULL) {
        /* Traverse the with-loop's code body to find object references. */
        INFO_INWITHLOOP (arg_info) = TRUE;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_INWITHLOOP (arg_info) = FALSE;

        /* Attach the referenced objects to the with-loop. */
        arg_node = AddObjectsToWithLoop (arg_node, INFO_OBJECTS (arg_info));
    }

    if (WITH_WITHOP (arg_node) != NULL) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 * Traversal start function
 */

node *
WOAdoWithloopObjectAnalysis (node *syntax_tree)
{
    info *arg_info;
    DBUG_ENTER ("WOAdoWithloopObjectAnalysis");

    if (global.wlo == TRUE) {
        TRAVpush (TR_woa);

        arg_info = MakeInfo ();

        syntax_tree = TRAVdo (syntax_tree, arg_info);

        FreeInfo (arg_info);

        TRAVpop ();
    }

    DBUG_RETURN (syntax_tree);
}
