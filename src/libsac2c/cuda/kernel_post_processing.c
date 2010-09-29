
#include "kernel_post_processing.h"

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
    bool remove_ids;
    node *lhs;
    nlut_t *nlut;
    node *with3ids;
    node *preassigns;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_REMOVE_ASSIGN(n) (n->remove_assign)
#define INFO_REMOVE_IDS(n) (n->remove_ids)
#define INFO_LHS(n) (n->lhs)
#define INFO_NLUT(n) (n->nlut)
#define INFO_WITH3IDS(n) (n->with3ids)
#define INFO_PREASSIGNS(n) (n->preassigns)

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
    INFO_REMOVE_IDS (result) = FALSE;
    INFO_LHS (result) = NULL;
    INFO_NLUT (result) = NULL;
    INFO_WITH3IDS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static node *
RemoveUnusedVardecs (node *vardecs, info *arg_info)
{
    DBUG_ENTER ("RemoveUnusedVardecs");

    if (VARDEC_NEXT (vardecs) != NULL) {
        VARDEC_NEXT (vardecs) = RemoveUnusedVardecs (VARDEC_NEXT (vardecs), arg_info);
    }

    if (NLUTgetNum (INFO_NLUT (arg_info), VARDEC_AVIS (vardecs)) == 0) {
        printf ("Vardec %s is being removed\n", VARDEC_NAME (vardecs));
        vardecs = FREEdoFreeNode (vardecs);
    }

    DBUG_RETURN (vardecs);
}

node *
KPPdoKernelPostProcessing (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("KPPdoKernelPostProcessing");

    info = MakeInfo ();
    TRAVpush (TR_kpp);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

node *
KPPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("KPPfundef");

    /* we only traverse cuda kernels */
    if (FUNDEF_ISCUDAGLOBALFUN (arg_node) || FUNDEF_ISCUDASTGLOBALFUN (arg_node)) {

        INFO_NLUT (arg_info)
          = NLUTgenerateNlut (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        FUNDEF_VARDEC (arg_node)
          = RemoveUnusedVardecs (FUNDEF_VARDEC (arg_node), arg_info);

        INFO_NLUT (arg_info) = NLUTremoveNlut (INFO_NLUT (arg_info));
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
KPPassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("KPPassign");

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_REMOVE_ASSIGN (arg_info)) {
        arg_node = FREEdoFreeNode (arg_node);
        INFO_REMOVE_ASSIGN (arg_info) = FALSE;
    }

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
KPPlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("KPPlet");

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_id) {
        /* if we found a assignment of the form N_id = N_id
         * in cuda kernels and they are arrays, we repalce the RHS N_id by
         * primitive copy( N_id). */
        node *avis = ID_AVIS (LET_EXPR (arg_node));
        if (!CUisDeviceTypeNew (ID_NTYPE (LET_EXPR (arg_node)))
            && TYgetDim (ID_NTYPE (LET_EXPR (arg_node))) > 0) {
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
    }

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    if (INFO_REMOVE_IDS (arg_info)) {
        /* This is only required for syncthread primitive */
        LET_IDS (arg_node) = FREEdoFreeNode (LET_IDS (arg_node));
        LET_IDS (arg_node) = NULL;
        INFO_REMOVE_IDS (arg_info) = FALSE;
    } else if (LET_IDS (arg_node) != NULL
               && NLUTgetNum (INFO_NLUT (arg_info), IDS_AVIS (LET_IDS (arg_node))) == 0) {
        /* Any assignment defining a id that is not used anywhere
         * can be removed. */
        INFO_REMOVE_ASSIGN (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

node *
KPPwith3 (node *arg_node, info *arg_info)
{
    node *old_with3ids, *preassign = NULL;
    node *rhs = NULL;

    DBUG_ENTER ("KPPwith3");

    old_with3ids = INFO_WITH3IDS (arg_info);
    INFO_WITH3IDS (arg_info) = INFO_LHS (arg_info);

    if (NODE_TYPE (WITH3_OPERATIONS (arg_node)) == N_fold) {
        if (FOLD_INITIAL (WITH3_OPERATIONS (arg_node)) != NULL) {
            rhs = DUPdoDupNode (FOLD_INITIAL (WITH3_OPERATIONS (arg_node)));
        } else if (FOLD_NEUTRAL (WITH3_OPERATIONS (arg_node)) != NULL) {
            rhs = DUPdoDupTree (FOLD_NEUTRAL (WITH3_OPERATIONS (arg_node)));
        } else {
            DBUG_ASSERT ((0), "Both neutral and initial are NULL!");
        }

        preassign
          = TBmakeAssign (TBmakeLet (DUPdoDupTree (INFO_LHS (arg_info)), rhs), NULL);
    }

    if (WITH3_RANGES (arg_node) != NULL) {
        WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);
        WITH3_OPERATIONS (arg_node) = TRAVopt (WITH3_OPERATIONS (arg_node), arg_info);
    } else {
        WITH3_OPERATIONS (arg_node) = TRAVopt (WITH3_OPERATIONS (arg_node), arg_info);
        INFO_REMOVE_ASSIGN (arg_info) = TRUE;
    }

    INFO_WITH3IDS (arg_info) = old_with3ids;

    INFO_PREASSIGNS (arg_info) = preassign;

    DBUG_RETURN (arg_node);
}

node *
KPPrange (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("KPPrange");

    RANGE_RESULTS (arg_node) = TRAVopt (RANGE_RESULTS (arg_node), arg_info);
    RANGE_INDEX (arg_node) = TRAVopt (RANGE_INDEX (arg_node), arg_info);
    RANGE_LOWERBOUND (arg_node) = TRAVopt (RANGE_LOWERBOUND (arg_node), arg_info);
    RANGE_UPPERBOUND (arg_node) = TRAVopt (RANGE_UPPERBOUND (arg_node), arg_info);
    RANGE_CHUNKSIZE (arg_node) = TRAVopt (RANGE_CHUNKSIZE (arg_node), arg_info);
    RANGE_BODY (arg_node) = TRAVopt (RANGE_BODY (arg_node), arg_info);
    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
KPPid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("KPPid");

    NLUTincNum (INFO_NLUT (arg_info), ID_AVIS (arg_node), 1);

    DBUG_RETURN (arg_node);
}

node *
KPPprf (node *arg_node, info *arg_info)
{
    node *dim, *free_var, *array;
    int dim_num;
    ntype *type;

    DBUG_ENTER ("KPPprf");

    switch (PRF_PRF (arg_node)) {
    case F_alloc:
        dim = PRF_ARG2 (arg_node);
        if (NODE_TYPE (dim) == N_num) {
            dim_num = NUM_VAL (dim);
            if (dim_num > 0) {
                INFO_REMOVE_ASSIGN (arg_info) = TRUE;
                DBUG_PRINT ("KPP", ("%s = F_alloc() removed in function %s\n",
                                    IDS_NAME (INFO_LHS (arg_info)),
                                    FUNDEF_NAME (INFO_FUNDEF (arg_info))));
            }
        } else if (NODE_TYPE (dim) == N_prf) {
            if (PRF_PRF (dim) == F_dim_A) {
                array = PRF_ARG1 (dim);
                DBUG_ASSERT ((NODE_TYPE (array) == N_id),
                             "Non N_id node found for arguemnt of F_dim_A!");
                DBUG_ASSERT (TYgetDim (ID_NTYPE (array)) == 0, "Non scalar found for "
                                                               "F_dim_A as the second "
                                                               "arguemnt of F_alloc!");
            } else {
                DBUG_ASSERT ((0), "Wrong dim argument for F_alloc!");
            }
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
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
            DBUG_PRINT ("KPP",
                        ("F_free( %s) removed in function %s\n", ID_NAME (free_var),
                         FUNDEF_NAME (INFO_FUNDEF (arg_info))));
        } else {
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
        }
        break;
    case F_dec_rc:
        array = PRF_ARG1 (arg_node);
        if (!TUisScalar (AVIS_TYPE (ID_AVIS (array)))) {
            /* AVIS_ISCUDALOCAL( ID_AVIS( array)) */
            INFO_REMOVE_ASSIGN (arg_info) = TRUE;
            DBUG_PRINT ("KPP", ("F_dec_rc( %s) removed in function %s\n", ID_NAME (array),
                                FUNDEF_NAME (INFO_FUNDEF (arg_info))));
        } else {
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
        }
        break;
    case F_inc_rc:
        array = PRF_ARG1 (arg_node);
        if (!TUisScalar (AVIS_TYPE (ID_AVIS (array)))) {
            /* AVIS_ISCUDALOCAL( ID_AVIS( array)) */
            INFO_REMOVE_ASSIGN (arg_info) = TRUE;
            DBUG_PRINT ("KPP", ("F_inc_rc( %s) removed in function %s\n", ID_NAME (array),
                                FUNDEF_NAME (INFO_FUNDEF (arg_info))));
        } else {
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
        }
        break;
    case F_suballoc:
        /* If we have a suballoc, the lhs ids might be tagged as cuda
         * local in KPPlet if the rhs is an N_array. However, we
         * do not want that, therefore, we need to set it back to
         * FALSE here */
        AVIS_ISCUDALOCAL (IDS_AVIS (INFO_LHS (arg_info))) = FALSE;
        NLUTincNum (INFO_NLUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)), 1);
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
        break;
    case F_syncthreads:
        PRF_ARGS (arg_node) = FREEdoFreeTree (PRF_ARGS (arg_node));
        PRF_ARGS (arg_node) = NULL;
        INFO_REMOVE_IDS (arg_info) = TRUE;
        break;
    case F_idx_modarray_AxSxS:
        /*
         * Note that this is a quick fix for the problem generated in
         * cuknl. In cuknl, if a modarray_AxSxS works on shared memory,
         * the first argument has an old avis in the calling context (
         * See function CUKNLid in create_cuda_kernels.c). This causes
         * problem when dvr is run when the avis in the calling context
         * is freed. So here we simple set the avis of the first argument
         * to the avis of the lhs ids. However, a better solution should
         * eventually replace this quick fix.
         */
        if (CUisShmemTypeNew (IDS_NTYPE (INFO_LHS (arg_info)))
            && CUisShmemTypeNew (ID_NTYPE (PRF_ARG1 (arg_node)))) {
            ID_AVIS (PRF_ARG1 (arg_node)) = IDS_AVIS (INFO_LHS (arg_info));
        }
    case F_syncin:
    case F_syncout:
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
        PRF_ARGS (arg_node)
          = TCappendExprs (PRF_ARGS (arg_node), TCids2Exprs (INFO_WITH3IDS (arg_info)));
        break;
    default:
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
        break;
    }

    DBUG_RETURN (arg_node);
}
