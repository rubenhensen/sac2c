/*
 * Identifies sub-allocated arrays for the distributed memory backend and marks them
 * as DSM arrays (allocated in DSM array but not distributed).
 * This is done by finding all suballocs and marking the avis of the lhs that it
 * is DSM.
 */

#include "identify_suballocated_arrays.h"

#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"

#define DBUG_PREFIX "DMISAA"
#include "debug.h"

/*
 * INFO structure
 */
struct INFO {
    bool isSuballoc;
};

/*
 * INFO macros
 */
#define INFO_IS_SUBALLOC(n) (n->isSuballoc)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_IS_SUBALLOC (result) = FALSE;

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
DMISAAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_IS_SUBALLOC (arg_info) = FALSE;
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_IS_SUBALLOC (arg_info)) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    INFO_IS_SUBALLOC (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

node *
DMISAAids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_IS_SUBALLOC (arg_info)) {
        /* Mark the variable as suballocated for the
         * distributed memory backend. */
        AVIS_DISTMEMSUBALLOC (IDS_AVIS (arg_node)) = TRUE;
        AVIS_TYPE (IDS_AVIS (arg_node))
          = TYsetDistributed (AVIS_TYPE (IDS_AVIS (arg_node)), distmem_dis_dsm);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
DMISAAprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PRF_PRF (arg_node) == F_suballoc) {
        INFO_IS_SUBALLOC (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/*
 * Traversal start function
 */
node *
DMISAAdoIdentifySubAllocatedArrays (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    TRAVpush (TR_dmisaa);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);
    info = FreeInfo (info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
