/*
 * Identifies distributable arrays for the distributed memory backend.
 * Arrays are distributable unless they are written to in the body of a with-loop
 * even though they are not the result variables of the with-loop.
 * TODO: Variables that are written to in side effect functions are also not
 * distributable.
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
    bool traversing_with_loop;
    bool traversing_lhs;
    node *results_of_with_loop;
};

/*
 * INFO macros
 */
#define INFO_TRAVERSING_WITH_LOOP(n) ((n)->traversing_with_loop)
#define INFO_TRAVERSING_LHS(n) ((n)->traversing_lhs)
#define INFO_RESULTS_OF_WITH_LOOP(n) ((n)->results_of_with_loop)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_TRAVERSING_WITH_LOOP (result) = FALSE;
    INFO_TRAVERSING_LHS (result) = FALSE;

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

    if (INFO_TRAVERSING_WITH_LOOP (arg_info)) {
        /* We are traversing the body of a genarray or modarray with-loop.
         * All variables on the left-hand side are non-distributable
         * unless they are the target variables of the with-loop. */

        /* Traverse the left-hand side. */
        INFO_TRAVERSING_LHS (arg_info) = TRUE;
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
        INFO_TRAVERSING_LHS (arg_info) = FALSE;

        /* Traverse the right-hand side. */
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    } else if (NODE_TYPE (LET_EXPR (arg_node)) == N_with2
               && (NODE_TYPE (WITH2_WITHOP (LET_EXPR (arg_node))) == N_genarray
                   || NODE_TYPE (WITH2_WITHOP (LET_EXPR (arg_node))) == N_modarray)) {
        /* We are not in the body of a with-loop and have encountered
         * a genarray or modarray with-loop. */

        INFO_TRAVERSING_WITH_LOOP (arg_info) = TRUE;
        INFO_RESULTS_OF_WITH_LOOP (arg_info) = LET_IDS (arg_node);

        /* Traverse the right-hand side. */
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

        INFO_TRAVERSING_WITH_LOOP (arg_info) = FALSE;
        INFO_RESULTS_OF_WITH_LOOP (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
DMIDAids (node *arg_node, info *arg_info)
{
    node *ids;
    bool is_result_of_with_loop;

    DBUG_ENTER ();

    if (INFO_TRAVERSING_LHS (arg_info)) {
        /* This is the left-hand side of an expression in the body of a
         * genarray or modarray with-loop.
         * All variables on the left-hand side are non-distributable
         * unless they are the result variables of the with-loop. */

        ids = INFO_RESULTS_OF_WITH_LOOP (arg_info);
        is_result_of_with_loop = FALSE;

        do {
            if (arg_node == ids) {
                is_result_of_with_loop = TRUE;
                break;
            }
        } while ((ids = IDS_NEXT (ids)) != NULL);

        if (!is_result_of_with_loop) {
            AVIS_DISTMEMISDISTRIBUTABLE (IDS_AVIS (arg_node)) = FALSE;
        }

        /* Use TRAVopt because there may not be another argument. */
        IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);
    }

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
