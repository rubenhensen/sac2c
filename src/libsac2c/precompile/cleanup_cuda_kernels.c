
#include "cleanup_cuda_kernels.h"

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
    node *fundef;
    bool remove_assign;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_REMOVE_ASSIGN(n) (n->remove_assign)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_REMOVE_ASSIGN (result) = FALSE;

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
CLKNLdoCleanupCUDAKernels (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CLKNLdoCleanupCUDAKernels");

    info = MakeInfo ();
    TRAVpush (TR_clknl);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

node *
CLKNLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CLKNLfundef");

    if (FUNDEF_ISCUDAGLOBALFUN (arg_node)) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
CLKNLassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CLKNLassign");

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_REMOVE_ASSIGN (arg_info)) {
        arg_node = FREEdoFreeNode (arg_node);
        INFO_REMOVE_ASSIGN (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

node *
CLKNLprf (node *arg_node, info *arg_info)
{
    node *num_node, *free_var;
    int dim;
    ntype *type;

    DBUG_ENTER ("CLKNLprf");

    switch (PRF_PRF (arg_node)) {
    case F_alloc:
        num_node = PRF_ARG2 (arg_node);
        DBUG_ASSERT ((NODE_TYPE (num_node) == N_num),
                     "Second argument of F_alloc is not a constant!");
        dim = NUM_VAL (num_node);
        if (dim > 0) {
            INFO_REMOVE_ASSIGN (arg_info) = TRUE;
        }
        break;
    case F_free:
        free_var = PRF_ARG1 (arg_node);
        type = AVIS_TYPE (ID_AVIS (free_var));
        DBUG_ASSERT ((TYisAKV (type) || TYisAKS (type)),
                     "Non AKV and AKS node found in CUDA kernels!");
        dim = TYgetDim (type);
        if (dim > 0) {
            INFO_REMOVE_ASSIGN (arg_info) = TRUE;
        }
        break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}
