/*
 * $Log$
 * Revision 1.1  2005/09/14 21:26:36  sah
 * Initial revision
 *
 *
 */

#include "index_optimize.h"

#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "free.h"
#include "internal_lib.h"

/**
 * INFO structure
 */
struct INFO {
};

/**
 * INFO macros
 */

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/**
 * optimizer functions
 */
static node *
ReplaceByWithOffset (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ReplaceByWithOffset");

    DBUG_RETURN (arg_node);
}

static node *
LiftOffsetOut (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LiftOffsetOut");

    DBUG_RETURN (arg_node);
}

static node *
ReplaceByIdx2Offset (node *arg_node, info *arg_info)
{
    node *result;
    node *ivassign;
    node *idxs;

    DBUG_ENTER ("ReplaceByIdx2Offset");

    DBUG_ASSERT ((NODE_TYPE (PRF_ARG2 (arg_node)) == N_id),
                 "ReplaceByIdx2Offset called with iv being non N_id node");
    DBUG_ASSERT ((AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node))) != NULL),
                 "ReplaceByIdx2Offset with AVIS_SSAASSIGN of iv being NULL");

    ivassign = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node)));

    DBUG_ASSERT ((NODE_TYPE (ASSIGN_RHS (ivassign)) == N_array),
                 "ReplaceByIdx2Offset with non N_array AVIS_SSAASSIGN");

    idxs = ARRAY_AELEMS (ASSIGN_RHS (ivassign));

    result = TBmakePrf (F_idxs2offset, TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                                                    DUPdoDupTree (idxs)));

    arg_node = FREEdoFreeNode (arg_node);

    DBUG_RETURN (result);
}

static node *
OptimizeComputation (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OptimizeComputation");

    DBUG_RETURN (arg_node);
}

node *
IVEOprf (node *arg_node, info *arg_info)
{
    node *ivarg;
    node *ivassign;

    DBUG_ENTER ("IVEOprf");

    if (PRF_PRF (arg_node) == F_vect2offset) {
        ivarg = PRF_ARG2 (arg_node);
        ivassign = AVIS_SSAASSIGN (ID_AVIS (ivarg));

        if (ivassign != NULL) {
            if (NODE_TYPE (ASSIGN_RHS (ivassign)) == N_array) {
                /*
                 * this index vector is defined as a array of
                 * scalars.
                 */
                arg_node = ReplaceByIdx2Offset (arg_node, arg_info);
            } else if (NODE_TYPE (ASSIGN_RHS (ivassign)) == N_prf) {
                /*
                 * this index vector is defined as a computation
                 * on (maybe) other index vectors or constants
                 */
                arg_node = OptimizeComputation (arg_node, arg_info);
            }
        } else {
            if (NODE_TYPE (AVIS_DECL (ID_AVIS (ivarg))) == N_arg) {
                /*
                 * the index vector is an argument of this function
                 */
                arg_node = LiftOffsetOut (arg_node, arg_info);
            } else {
                /*
                 * this id has no defining assignment and is no argument.
                 * the only possible reason for this is, that this id is a
                 * withloop index vector.
                 */
                arg_node = ReplaceByWithOffset (arg_node, arg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

node *
IVEOdoIndexVectorEliminationOptimisation (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IVEIdoIndexVectorEliminationOptimisation");

    TRAVpush (TR_iveo);

    info = MakeInfo ();

    syntax_tree = TRAVdo (syntax_tree, info);

    info = FreeInfo (info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}
