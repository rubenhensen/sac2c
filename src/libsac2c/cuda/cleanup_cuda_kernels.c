
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
#include "type_utils.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool remove_assign;
    node *lhs;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_REMOVE_ASSIGN(n) (n->remove_assign)
#define INFO_LHS(n) (n->lhs)

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
    INFO_LHS (result) = NULL;

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

    /* we only traverse cuda kernels */
    if (FUNDEF_ISCUDAGLOBALFUN (arg_node) || FUNDEF_ISCUDASTGLOBALFUN (arg_node)) {
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
CLKNLlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CLKNLlet");

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_id) {
        /* if we found a assignment of the form N_id = N_id
         * in cuda kernels and they are arrays, we repalce the RHS N_id by
         * primitive copy( N_id). */
        node *avis = ID_AVIS (LET_EXPR (arg_node));
        if (!CUisDeviceTypeNew (AVIS_TYPE (ID_AVIS (LET_EXPR (arg_node))))
            && TYgetDim (AVIS_TYPE (ID_AVIS (LET_EXPR (arg_node)))) > 0) {
            LET_EXPR (arg_node) = FREEdoFreeNode (LET_EXPR (arg_node));
            LET_EXPR (arg_node) = TCmakePrf1 (F_copy, TBmakeId (avis));
        } else if (AVIS_ISCUDALOCAL (IDS_AVIS (LET_IDS (arg_node)))
                   || AVIS_ISCUDALOCAL (ID_AVIS (LET_EXPR (arg_node)))) {
            AVIS_ISCUDALOCAL (IDS_AVIS (LET_IDS (arg_node))) = TRUE;
            AVIS_ISCUDALOCAL (ID_AVIS (LET_EXPR (arg_node))) = TRUE;
            LET_EXPR (arg_node) = FREEdoFreeNode (LET_EXPR (arg_node));
            LET_EXPR (arg_node) = TCmakePrf1 (F_copy, TBmakeId (avis));
        }
    }
    /* If an assignment of the form: A = [v1,v2...v3] occurs
     * in the kernel, we tag A as cuda local. Normally,
     * this case should be handled in insert wl memory
     * transfer. However, since insert wl structral bound
     * is run after that and it might introduce assignment
     * of the form mentioned above, we have to be careful.
     * This bug is discoverred in the compilation of livermore
     * loop21. */
    else if (NODE_TYPE (LET_EXPR (arg_node)) == N_array) {
        AVIS_ISCUDALOCAL (IDS_AVIS (LET_IDS (arg_node))) = TRUE;
        INFO_LHS (arg_info) = LET_IDS (arg_node);
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    } else {
        INFO_LHS (arg_info) = LET_IDS (arg_node);
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CLKNLprf (node *arg_node, info *arg_info)
{
    node *dim, *free_var, *array;
    int dim_num;
    ntype *type;

    DBUG_ENTER ("CLKNLprf");

    switch (PRF_PRF (arg_node)) {
    case F_alloc:
        dim = PRF_ARG2 (arg_node);
        if ((NODE_TYPE (dim) == N_num)) {
            dim_num = NUM_VAL (dim);
            if (dim_num > 0) {
                INFO_REMOVE_ASSIGN (arg_info) = TRUE;
                printf ("%s = F_alloc() removed in function %s\n",
                        IDS_NAME (INFO_LHS (arg_info)),
                        FUNDEF_NAME (INFO_FUNDEF (arg_info)));
            }
        } else if ((NODE_TYPE (dim) == N_prf)) {
            if (PRF_PRF (dim) == F_dim_A) {
                array = PRF_ARG1 (dim);
                DBUG_ASSERT ((NODE_TYPE (array) == N_id),
                             "Non N_id node found for arguemnt of F_dim_A!");
                DBUG_ASSERT (TYgetDim (AVIS_TYPE (ID_AVIS (array))) == 0,
                             "Non scalar found for F_dim_A as the second arguemnt of "
                             "F_alloc!");
            } else {
                DBUG_ASSERT ((0), "Wrong dim argument for F_alloc!");
            }
        } else {
            DBUG_ASSERT ((0), "Wrong dim argument for F_alloc!");
        }

        break;
    case F_free:
        free_var = PRF_ARG1 (arg_node);
        type = AVIS_TYPE (ID_AVIS (free_var));
        DBUG_ASSERT ((TYisAKV (type) || TYisAKS (type)),
                     "Non AKV and AKS node found in CUDA kernels!");
        dim_num = TYgetDim (type);
        if (dim_num > 0) {
            INFO_REMOVE_ASSIGN (arg_info) = TRUE;
            printf ("F_free( %s) removed in function %s\n", ID_NAME (free_var),
                    FUNDEF_NAME (INFO_FUNDEF (arg_info)));
        }
        break;
    case F_dec_rc:
        array = PRF_ARG1 (arg_node);
        if (!TUisScalar (AVIS_TYPE (ID_AVIS (array)))) {
            /* AVIS_ISCUDALOCAL( ID_AVIS( array)) */
            INFO_REMOVE_ASSIGN (arg_info) = TRUE;
            printf ("F_dec_rc( %s) removed in function %s\n", ID_NAME (array),
                    FUNDEF_NAME (INFO_FUNDEF (arg_info)));
        }
        break;
    case F_inc_rc:
        array = PRF_ARG1 (arg_node);
        if (!TUisScalar (AVIS_TYPE (ID_AVIS (array)))) {
            /* AVIS_ISCUDALOCAL( ID_AVIS( array)) */
            INFO_REMOVE_ASSIGN (arg_info) = TRUE;
            printf ("F_inc_rc( %s) removed in function %s\n", ID_NAME (array),
                    FUNDEF_NAME (INFO_FUNDEF (arg_info)));
        }
        break;
    case F_suballoc:
        /* If we have a suballoc, the lhs ids might be tagged as cuda
         * local in CLKNLlet if the rhs is an N_array. However, we
         * do not want that, therefore, we need to set it back to
         * FALSE here */
        AVIS_ISCUDALOCAL (IDS_AVIS (INFO_LHS (arg_info))) = FALSE;
        break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}
