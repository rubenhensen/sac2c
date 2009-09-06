
#include "prepare_kernel_generation.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"
#include "dbug.h"
#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "DataFlowMask.h"
#include "NameTuplesUtils.h"
#include "scheduling.h"
#include "wl_bounds.h"
#include "new_types.h"
#include "user_types.h"
#include "shape.h"
#include "LookUpTable.h"
#include "convert.h"
#include "math_utils.h"
#include "types.h"
#include "deadcoderemoval.h"
#include "NumLookUpTable.h"
#include "cuda_utils.h"

/*
 * INFO structure
 */
struct INFO {
    bool incudadoloop;
    node *with;
};

/*
 * INFO macros
 */
#define INFO_INCUDADOLOOP(n) (n->incudadoloop)
#define INFO_WITH(n) (n->with)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_INCUDADOLOOP (result) = FALSE;
    INFO_WITH (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

node *
PKNLGdoPrepareKernelGeneration (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("PKNLGdoPrepareKernelGeneration");

    info = MakeInfo ();
    TRAVpush (TR_pknlg);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

node *
PKNLGdo (node *arg_node, info *arg_info)
{
    bool old_incudadoloop;

    DBUG_ENTER ("PKNLGdo");

    if (DO_ISCUDARIZABLE (arg_node)) {
        old_incudadoloop = INFO_INCUDADOLOOP (arg_info);
        INFO_INCUDADOLOOP (arg_info) = TRUE;
        DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);
        INFO_INCUDADOLOOP (arg_info) = old_incudadoloop;
    } else {
        DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
PKNLGwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PKNLGwith");

    if (INFO_INCUDADOLOOP (arg_info)) {
        WITH_CUDARIZABLE (arg_node) = FALSE;
        INFO_WITH (arg_info) = arg_node;
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        INFO_WITH (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
PKNLGpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PKNLGpart");

    if (!WITH_CUDARIZABLE (INFO_WITH (arg_info))) {
        PART_CUDARIZABLE (arg_node) = FALSE;
        PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
