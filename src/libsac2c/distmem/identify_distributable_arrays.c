/*
 * Identifies distributable arrays for the distributed memory backend.
 * Arrays are distributable if they are the result of a
 * genarray/modarray with-loop.
 * TODO: Possibly this is not sufficient and arguments to functions/
 * results of function calls also have to be distributable.
 */

#include "identify_distributable_arrays.h"

#include "traverse.h"
#include "memory.h"
#include "tree_basic.h"

#define DBUG_PREFIX "DMIDA"
#include "debug.h"

/*
 * INFO structure
 */
struct INFO {
    bool is_result_of_with;
};

/*
 * INFO macros
 */
#define INFO_IS_RESULT_OF_WITH(n) ((n)->is_result_of_with)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_IS_RESULT_OF_WITH (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * Traversal functions
 */
node *
DMIDAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_with2
        && (NODE_TYPE (WITH2_WITHOP (LET_EXPR (arg_node))) == N_genarray
            || NODE_TYPE (WITH2_WITHOP (LET_EXPR (arg_node))) == N_modarray)) {

        /* A variable is distributable if the result of a genarray/modarray with-loop is
         * assigned to it. */
        INFO_IS_RESULT_OF_WITH (arg_info) = TRUE;
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
        INFO_IS_RESULT_OF_WITH (arg_info) = FALSE;
    }

    /* There is no need to traverse the expression.
     * There may be an inner with-loop but the distributed memory
     * backend only supports one level of parallelism. */

    DBUG_RETURN (arg_node);
}

node *
DMIDAids (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    /* Use TRAVopt because there may not be another argument. */
    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    avis = IDS_AVIS (arg_node);
    AVIS_DISTMEMISDISTRIBUTABLE (avis) = TRUE;

    DBUG_RETURN (arg_node);
}

node *
DMIDAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISEXPORTED (arg_node)) {
        /* If the function is exported, its argumens are distributable. */
        /* Traverse the arguments if there are any. */
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
    }

    /* Traverse the remaining function definitions if there are any. */
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
DMIDAarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* We only get here if the function is exported. In that case,
     * the argument is distributable. */
    AVIS_DISTMEMISDISTRIBUTABLE (ARG_AVIS (arg_node)) = TRUE;

    /* Examine the remaining arguments if there are any. */
    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
DMIDAap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/*
 * Traversal start function
 */
node *
DMIDAdoIdentifyDistributableArrays (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    TRAVpush (TR_dmida);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);
    info = FreeInfo (info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
