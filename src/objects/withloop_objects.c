/* $Id$ */

#include "withloop_objects.h"

#include "ctinfo.h"
#include "dbug.h"
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
    node *lastexprs;
    node *objiter;

    /* Append to the last exprs to match the extract() ops */
    lastexprs = withexprs;
    while (EXPRS_NEXT (lastexprs) != NULL) {
        lastexprs = EXPRS_NEXT (lastexprs);
    }

    objiter = objects;
    while (objiter != NULL) {
        node *avis;

        avis = ID_AVIS (SET_MEMBER (objiter));
        printf ("adding expr for %s\n", AVIS_NAME (avis));
        EXPRS_NEXT (lastexprs) = TBmakeExprs (TBmakeId (avis), NULL);
        lastexprs = EXPRS_NEXT (lastexprs);
        objiter = SET_NEXT (objiter);
    }

    return withexprs;
}

static node *
AddObjectsToWithOps (node *withops, node *objects)
{
    node *lastop;
    node *objiter;

    /* Append to the last op, so first find the last one */
    lastop = withops;
    while (WITHOP_NEXT (lastop) != NULL) {
        lastop = WITHOP_NEXT (lastop);
    }

    objiter = objects;
    while (objiter != NULL) {
        node *avis;

        L_WITHOP_NEXT (lastop, TBmakeExtract ());
        lastop = WITHOP_NEXT (lastop);
        avis = ID_AVIS (SET_MEMBER (objiter));
        printf ("adding extract() for %s\n", AVIS_NAME (avis));
        /*EXTRACT_OBJECT( lastop) = TBmakeId( avis);*/
        objiter = SET_NEXT (objiter);
    }

    return withops;
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
    node *last_ids;
    node *objiter;

    /* Append to the last exprs to match the extract() ops */
    last_ids = lhs_ids;
    while (IDS_NEXT (last_ids) != NULL) {
        last_ids = IDS_NEXT (last_ids);
    }

    objiter = objects;
    while (objiter != NULL) {
        node *avis;

        avis = ID_AVIS (SET_MEMBER (objiter));
        printf ("adding lhs expr for %s\n", AVIS_NAME (avis));
        IDS_NEXT (last_ids) = TBmakeIds (avis, NULL);
        last_ids = IDS_NEXT (last_ids);
        objiter = SET_NEXT (objiter);
    }
    return lhs_ids;
}

/*
 * Traversal functions
 */

node *
WOAid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WOAid");

    if (INFO_INWITHLOOP (arg_info) == TRUE) {

        node *avis = ID_AVIS (arg_node);
        ntype *type = AVIS_TYPE (avis);

        if (TYisArray (type) && TUisUniqueUserType (TYgetScalar (type))) {

            /* This is an object referenced from within a with-loop. Add it
             * to the list of objects that will be attached to the with-loop. */
            TCSetAdd (&INFO_OBJECTS (arg_info), arg_node);
            CTIwarnLine (NODE_LINE (arg_node),
                         "Unique var %s of type %s referenced from with-loop. This is "
                         "currently unsupported!",
                         AVIS_NAME (avis),
                         UTgetName (TYgetUserType (TYgetScalar (type))));
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
