
/**
 *
 * @defgroup rwr Region-Aware With-Loop Reuse Candidate Inference
 *
 * @ingroup mm
 *
 * @{
 */

/**
 *
 * @file reusewithregion.c
 *
 * Prefix: RWR
 */
#include "reusewithregion.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "print.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "new_types.h"
#include "int_matrix.h"
#include "NumLookUpTable.h"
#include "LookUpTable.h"
#include "types.h"
#include "DataFlowMask.h"
#include "constants.h"

#define LOWER_BOUND 0
#define UPPER_BOUND 1

#define MAX_ENTRIES 8

typedef struct INDEX_EXPRS_T {
    int nr_entries;
    cuda_index_t *exprs[MAX_ENTRIES];
} index_exprs_t;

#define IE_NR_ENTRIES(n) ((n)->nr_entries)
#define IE_EXPRS(n, i) ((n->exprs)[i])

static index_exprs_t *
CreateIndexExprs (int nr)
{
    int i;
    index_exprs_t *res;

    DBUG_ENTER ();

    res = (index_exprs_t *)MEMmalloc (sizeof (index_exprs_t));

    IE_NR_ENTRIES (res) = nr;

    for (i = 0; i < MAX_ENTRIES; i++) {
        IE_EXPRS (res, i) = NULL;
    }

    DBUG_RETURN (res);
}

/* Two different traverse modes */
typedef enum { TR_normal, TR_collect, TR_compute } travmode_t;

/*
 * INFO structure
 */
struct INFO {
    travmode_t mode;
    index_exprs_t *ie;
    int dim;
    lut_t *lut;
    node *rc;
    dfmask_t
      *mask; /* Used to quickly test if an id with NULL ssaassign is a withloop ids */
    nlut_t
      *nlut; /* Used to provide mapping between withloop ids and its nesting level. */
    int nest_level;
    int coefficient;
    int nr_extids;
    bool is_affine;
    node *ivids; /* borrow from ReuseWithArray.c ;-) */
    int writedim;
};

/*
 * INFO macros
 */
#define INFO_MODE(n) ((n)->mode)
#define INFO_IE(n) ((n)->ie)
#define INFO_DIM(n) ((n)->dim)
#define INFO_LUT(n) ((n)->lut)
#define INFO_RC(n) ((n)->rc)
#define INFO_MASK(n) ((n)->mask)
#define INFO_NLUT(n) ((n)->nlut)
#define INFO_NEST_LEVEL(n) ((n)->nest_level)
#define INFO_COEFFICIENT(n) ((n)->coefficient)
#define INFO_NR_EXTIDS(n) ((n)->nr_extids)
#define INFO_IS_AFFINE(n) ((n)->is_affine)
#define INFO_IVIDS(n) ((n)->ivids)
#define INFO_WRITEDIM(n) ((n)->writedim)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_MODE (result) = TR_normal;
    INFO_IE (result) = NULL;
    INFO_DIM (result) = 0;
    INFO_LUT (result) = NULL;
    INFO_RC (result) = NULL;
    INFO_MASK (result) = NULL;
    INFO_NLUT (result) = NULL;
    INFO_NEST_LEVEL (result) = 0;
    INFO_COEFFICIENT (result) = 1;
    INFO_NR_EXTIDS (result) = 0;
    INFO_IS_AFFINE (result) = TRUE;
    INFO_IVIDS (result) = NULL;
    INFO_WRITEDIM (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/
static node *
IdentifyNoopArray (node *with)
{
    node *res = NULL;
    node *code = WITH_CODE (with);
    node *ivavis = IDS_AVIS (WITH_VEC (with));

    DBUG_ENTER ();

    while (code != NULL) {
        node *cass = AVIS_SSAASSIGN (ID_AVIS (CODE_CEXPR (code)));

        if ((cass != NULL) && (NODE_TYPE (ASSIGN_RHS (cass)) == N_prf)
            && (PRF_PRF (ASSIGN_RHS (cass)) == F_sel_VxA)
            && (NODE_TYPE (PRF_ARG1 (ASSIGN_RHS (cass))) == N_id)
            && (NODE_TYPE (PRF_ARG2 (ASSIGN_RHS (cass))) == N_id)
            && (ID_AVIS (PRF_ARG1 (ASSIGN_RHS (cass))) == ivavis)) {
            res = TBmakeId (ID_AVIS (PRF_ARG2 (ASSIGN_RHS (cass))));
            break;
        }

        code = CODE_NEXT (code);
    }
    DBUG_RETURN (res);
}

static bool
IsNoopPart (node *part, node *rc)
{
    bool res = FALSE;
    node *code = PART_CODE (part);
    node *ivavis = IDS_AVIS (PART_VEC (part));
    node *cass;

    DBUG_ENTER ();

    cass = AVIS_SSAASSIGN (ID_AVIS (CODE_CEXPR (code)));

    if ((cass != NULL) && (NODE_TYPE (ASSIGN_RHS (cass)) == N_prf)
        && (PRF_PRF (ASSIGN_RHS (cass)) == F_sel_VxA)
        && (NODE_TYPE (PRF_ARG1 (ASSIGN_RHS (cass))) == N_id)
        && (NODE_TYPE (PRF_ARG2 (ASSIGN_RHS (cass))) == N_id)
        && (ID_AVIS (PRF_ARG1 (ASSIGN_RHS (cass))) == ivavis)
        && (ID_AVIS (PRF_ARG2 (ASSIGN_RHS (cass))) == ID_AVIS (rc))) {
        res = TRUE;
    }

    DBUG_RETURN (res);
}

static node *
IdentifyOtherPart (node *with, node *rc)
{
    node *hotpart = NULL;
    node *part = WITH_PART (with);

    DBUG_ENTER ();

    while (part != NULL) {
        if (!IsNoopPart (part, rc)) {
            if (hotpart == NULL) {
                hotpart = part;
            } else {
                hotpart = NULL;
                break;
            }
        }

        part = PART_NEXT (part);
    }

    /*
     * The hot part must have a known GENWIDTH
     */
    if ((hotpart != NULL)
        && ((NODE_TYPE (PART_GENERATOR (hotpart)) != N_generator)
            || (GENERATOR_GENWIDTH (PART_GENERATOR (hotpart)) == NULL))) {
        hotpart = NULL;
    }

    DBUG_RETURN (hotpart);
}

/******************************************************************************
 *
 * Offset-aware With-Loop reuse candidate inference (rwr_tab)
 *
 * prefix: RWR
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *RWRdoRegionAwareReuseCandidateInference( node *with)
 *
 * @brief
 *
 * @param with-loop to search for reusable arrays
 *
 * @return list of reuse candidates
 *
 *****************************************************************************/
node *
RWRdoRegionAwareReuseCandidateInference (node *with, node *fundef)
{
    node *cand = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (with) == N_with, "Illegal node type!");

    if (((NODE_TYPE (WITH_WITHOP (with)) == N_genarray)
         || (NODE_TYPE (WITH_WITHOP (with)) == N_modarray))
        && (WITHOP_NEXT (WITH_WITHOP (with)) == NULL)) {
        node *hotpart = NULL;

        cand = IdentifyNoopArray (with);

        if (cand != NULL) {
            hotpart = IdentifyOtherPart (with, cand);

            if (hotpart != NULL) {
                node *oldnext;
                info *arg_info;
                dfmask_base_t *maskbase;

                arg_info = MakeInfo ();

                INFO_LUT (arg_info) = LUTgenerateLut ();
                INFO_NLUT (arg_info)
                  = NLUTgenerateNlut (FUNDEF_ARGS (fundef), FUNDEF_VARDECS (fundef));
                maskbase = DFMgenMaskBase (FUNDEF_ARGS (fundef), FUNDEF_VARDECS (fundef));
                INFO_MASK (arg_info) = DFMgenMaskClear (maskbase);

                INFO_RC (arg_info) = cand;
                cand = NULL;

                oldnext = PART_NEXT (hotpart);
                PART_NEXT (hotpart) = NULL;

                /* Start traversing the hot partition */
                TRAVpush (TR_rwr);
                hotpart = TRAVdo (hotpart, arg_info);

                /*
                 * After the first traversal, it can be possible that
                 * we find out the reuse candidate is not actually
                 * resable. This can be due to the non-affine indices
                 * of the accesses to this array
                 */
                if (INFO_RC (arg_info) != NULL) {
                    INFO_MODE (arg_info) = TR_compute;
                    hotpart = TRAVdo (hotpart, arg_info);
                }
                TRAVpop ();

                PART_NEXT (hotpart) = oldnext;

                if (INFO_RC (arg_info) != NULL) {
                    cand = TBmakeExprs (INFO_RC (arg_info), NULL);
                    INFO_RC (arg_info) = NULL;
                }
                INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
                INFO_NLUT (arg_info) = NLUTremoveNlut (INFO_NLUT (arg_info));
                INFO_MASK (arg_info) = DFMremoveMask (INFO_MASK (arg_info));
                maskbase = DFMremoveMaskBase (maskbase);

                arg_info = FreeInfo (arg_info);
            } else {
                cand = FREEdoFreeTree (cand);
            }
        }
    }

    DBUG_RETURN (cand);
}

static void
AddIndex (unsigned int type, int coefficient, node *idx, int looplevel, int dim,
          info *arg_info)
{
    DBUG_ENTER ();

    IE_EXPRS (INFO_IE (arg_info), dim)
      = TBmakeCudaIndex (type, coefficient, idx, looplevel,
                         IE_EXPRS (INFO_IE (arg_info), dim));

    DBUG_RETURN ();
}

static void
ActOnId (node *avis, info *arg_info)
{
    node *ssa_assign;

    DBUG_ENTER ();

    /* This function checks the id (avis) and performs the following
     * actions:
     *   - If the ssa assign of this id is NULL:
     *       - If it's a function argument, we add it as an external
     *         variable to the indices;
     *       - If it's a WL ids, we either add a ThreadIdx or a LoopIdx
     *         depending on whether it belongs to outermost cuda WL or
     *         inner WLs. In this case, we also need to update the coefficient
     *         matrix, i.e. set the appropriate cell to 1.
     *   - If the ssa assign of this id is NOT NULL:
     *       - If the defining assignment is outside of the current cuda
     *         WL, we add it as an external variable to the indices;
     *       - Otherwise, we start backtracking with a "collect" mode.
     */

    ssa_assign = AVIS_SSAASSIGN (avis);
    /*
     * There are two cases where SSAASSIGN can be:
     *  - id is a function argument,
     *  - id is withids.
     */
    if (ssa_assign == NULL) {
        if (NODE_TYPE (AVIS_DECL (avis)) == N_arg) {
            AddIndex (IDX_EXTID, INFO_COEFFICIENT (arg_info), avis, 0,
                      INFO_DIM (arg_info), arg_info);

            /* If this external id has not been come across before */
            if (!DFMtestMaskEntry (INFO_MASK (arg_info), NULL, avis)) {
                DFMsetMaskEntrySet (INFO_MASK (arg_info), NULL, avis);
                INFO_NR_EXTIDS (arg_info)++;
                NLUTsetNum (INFO_NLUT (arg_info), avis, INFO_NR_EXTIDS (arg_info));
            }
        }
        /* This is a withloop ids. We search for the exact ids */
        else if (DFMtestMaskEntry (INFO_MASK (arg_info), NULL, avis)) {
            AddIndex (IDX_WITHIDS, INFO_COEFFICIENT (arg_info), avis, 0,
                      INFO_DIM (arg_info), arg_info);
        } else {
            DBUG_ASSERT ((0), "None N_arg or a withids node with NULL ssaassign!");
        }
    } else {
        /* If this id is defined by an assignment outside the current cuda WL */
        if (ASSIGN_LEVEL (ssa_assign) == 0) {
            constant *cnst = COaST2Constant (ASSIGN_RHS (ssa_assign));
            if (cnst != NULL) {
                AddIndex (IDX_CONSTANT, COconst2Int (cnst), NULL, 0, INFO_DIM (arg_info),
                          arg_info);
            } else {
                /* For an id defined outside the withloop, do we continue
                 * traversing its rhs ids??? */
                AddIndex (IDX_EXTID, INFO_COEFFICIENT (arg_info), avis, 0,
                          INFO_DIM (arg_info), arg_info);

                /* If this external id has not been come across before */
                if (!DFMtestMaskEntry (INFO_MASK (arg_info), NULL, avis)) {
                    DFMsetMaskEntrySet (INFO_MASK (arg_info), NULL, avis);
                    INFO_NR_EXTIDS (arg_info)++;
                    NLUTsetNum (INFO_NLUT (arg_info), avis, INFO_NR_EXTIDS (arg_info));
                }
            }
        }
        /* Otherwise, we start backtracking to collect data access information */
        else {
            ASSIGN_STMT (ssa_assign) = TRAVopt (ASSIGN_STMT (ssa_assign), arg_info);
        }
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn node *RWRpart( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RWRpart (node *arg_node, info *arg_info)
{
    int dim, i;
    node *ids, *lb, *ub;

    DBUG_ENTER ();

    ids = PART_IDS (arg_node);
    lb = PART_BOUND1 (arg_node);
    ub = PART_BOUND2 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (lb) == N_array), "Lower bound is not an N_array!");
    DBUG_ASSERT ((NODE_TYPE (ub) == N_array), "Uower bound is not an N_array!");

    dim = TCcountIds (ids);
    lb = ARRAY_AELEMS (lb);
    ub = ARRAY_AELEMS (ub);

    if (INFO_MODE (arg_info) == TR_normal) {
        INFO_MODE (arg_info) = TR_collect;

        /*
         * In the number lookup table, the number corresponding to a
         * withloop ids is its nesting level, i.e. starts from 1 to
         * to innermost level. The reseaon we do not start from 0 is
         * because later when we want to create a matrix to represent
         * the loop bound contstraints, the first loop index should
         * apprear in the second column of the matrix. The first column
         * of the matrix is used to indicate whether the constraint
         * is equality or inequality. So this makes it easier to locate
         * the correct column to put the loop index later on, without
         * the need to add on to it beforehand
         */
        i = 1;
        while (ids != NULL) {
            DFMsetMaskEntrySet (INFO_MASK (arg_info), NULL, IDS_AVIS (ids));

            /* store the nest level of the withloop ids */
            NLUTsetNum (INFO_NLUT (arg_info), IDS_AVIS (ids),
                        INFO_NEST_LEVEL (arg_info) + i);

            /* Create a new index_exprs struct for each withloop ids */
            INFO_IE (arg_info) = CreateIndexExprs (UPPER_BOUND + 1);

            /* Start collecting lower bound expressions */
            if (COisConstant (EXPRS_EXPR (lb))) {
                IE_EXPRS (INFO_IE (arg_info), LOWER_BOUND)
                  = TBmakeCudaIndex (IDX_CONSTANT,
                                     COconst2Int (COaST2Constant (EXPRS_EXPR (lb))), NULL,
                                     0, IE_EXPRS (INFO_IE (arg_info), LOWER_BOUND));
            } else {
                INFO_DIM (arg_info) = LOWER_BOUND;
                INFO_COEFFICIENT (arg_info) = 1;
                ActOnId (ID_AVIS (EXPRS_EXPR (lb)), arg_info);
            }

            /* Start collecting upper bound expressions */
            if (COisConstant (EXPRS_EXPR (ub))) {
                IE_EXPRS (INFO_IE (arg_info), UPPER_BOUND)
                  = TBmakeCudaIndex (IDX_CONSTANT,
                                     COconst2Int (COaST2Constant (EXPRS_EXPR (ub))), NULL,
                                     0, IE_EXPRS (INFO_IE (arg_info), UPPER_BOUND));
            } else {
                INFO_DIM (arg_info) = UPPER_BOUND;
                INFO_COEFFICIENT (arg_info) = 1;
                ActOnId (ID_AVIS (EXPRS_EXPR (ub)), arg_info);
            }

            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (ids),
                                                     INFO_IE (arg_info));
            INFO_IE (arg_info) = NULL;

            ids = IDS_NEXT (ids);
            lb = EXPRS_NEXT (lb);
            ub = EXPRS_NEXT (ub);
            i++;
        }
        INFO_MODE (arg_info) = TR_normal;

        INFO_NEST_LEVEL (arg_info) += dim;
        PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);
        INFO_NEST_LEVEL (arg_info) -= dim;
        PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    } else if (INFO_MODE (arg_info) == TR_compute) {
        if (INFO_NEST_LEVEL (arg_info) == 0) {
            INFO_WRITEDIM (arg_info) = dim;
        }

        INFO_IVIDS (arg_info)
          = TCappendSet (INFO_IVIDS (arg_info), TBmakeSet (ids, NULL));
        INFO_NEST_LEVEL (arg_info) += dim;
        PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);
        INFO_NEST_LEVEL (arg_info) -= dim;

        INFO_IVIDS (arg_info) = TCdropSet (-1, INFO_IVIDS (arg_info));
        PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);
    } else {
        DBUG_ASSERT ((0), "Wrong traverse mode!");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RWRassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RWRassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_MODE (arg_info) == TR_normal) {
        ASSIGN_LEVEL (arg_node) = INFO_NEST_LEVEL (arg_info);
        ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    } else if (INFO_MODE (arg_info) == TR_collect) {
        ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    } else if (INFO_MODE (arg_info) == TR_compute) {
        ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        DBUG_ASSERT (0, "Wrong traverse mode!");
    }

    DBUG_RETURN (arg_node);
}

static IntMatrix
InitConstraints (IntMatrix constraints, info *arg_info)
{
    node *ivids, *ids;
    index_exprs_t *ie;
    cuda_index_t *lb, *ub;
    int i, rows, cols, x, y;

    DBUG_ENTER ();

    ivids = INFO_IVIDS (arg_info);
    rows = MatrixRows (constraints);
    cols = MatrixCols (constraints);

    i = 0;
    while ((ids = SET_MEMBER (ivids)) != NULL) {
        while (ids != NULL) {
            ie = LUTsearchInLutPp (INFO_LUT (arg_info), IDS_AVIS (ids));
            DBUG_ASSERT (((node *)ie != IDS_AVIS (ids)),
                         "Found withloop ids with null IE!");

            lb = IE_EXPRS (ie, LOWER_BOUND);
            ub = IE_EXPRS (ie, UPPER_BOUND);

            /* Initialize the constraint for lower bound */
            x = NLUTgetNum (INFO_NLUT (arg_info), IDS_AVIS (ids));
            y = i * 2;
            MatrixSetEntry (constraints, x, y, 1);
            /* All constraints are inequality */
            MatrixSetEntry (constraints, 0, y, 1);

            while (lb != NULL) {
                switch (CUIDX_TYPE (lb)) {
                case IDX_CONSTANT:
                    /* constants are always in the last column */
                    x = cols - 1;
                    break;
                case IDX_WITHIDS:
                    x = NLUTgetNum (INFO_NLUT (arg_info), CUIDX_ID (lb));
                    break;
                case IDX_EXTID:
                    x = INFO_NEST_LEVEL (arg_info)
                        + NLUTgetNum (INFO_NLUT (arg_info), CUIDX_ID (lb));
                    break;
                default:
                    DBUG_ASSERT ((0), "Unknown index type found!");
                    break;
                }
                MatrixSetEntry (constraints, x, y,
                                MatrixGetEntry (constraints, x, y)
                                  + CUIDX_COEFFICIENT (lb));
                lb = CUIDX_NEXT (lb);
            }

            /* Initialize the constraint for upper bound */
            x = NLUTgetNum (INFO_NLUT (arg_info), IDS_AVIS (ids));
            y = i * 2 + 1;
            MatrixSetEntry (constraints, x, y, -1);
            /* All constraints are inequality */
            MatrixSetEntry (constraints, 0, y, 1);

            while (ub != NULL) {
                switch (CUIDX_TYPE (ub)) {
                case IDX_CONSTANT:
                    /* constants are always in the last column */
                    x = cols - 1;
                    break;
                case IDX_WITHIDS:
                    x = NLUTgetNum (INFO_NLUT (arg_info), CUIDX_ID (ub));
                    break;
                case IDX_EXTID:
                    x = INFO_NEST_LEVEL (arg_info)
                        + NLUTgetNum (INFO_NLUT (arg_info), CUIDX_ID (ub));
                    break;
                default:
                    DBUG_ASSERT ((0), "Unknown index type found!");
                    break;
                }
                MatrixSetEntry (constraints, x, y,
                                MatrixGetEntry (constraints, x, y)
                                  + CUIDX_COEFFICIENT (ub));
                ub = CUIDX_NEXT (ub);
            }

            /*
             * For the constant in upper bound, we need to substract 1
             * from it to convert the '<' specified in SAC source to
             * '<=' required by polilib
             */
            MatrixSetEntry (constraints, cols - 1, y,
                            MatrixGetEntry (constraints, cols - 1, y) - 1);

            ids = IDS_NEXT (ids);
            i++;
        }
        ivids = SET_NEXT (ivids);
    }

    DBUG_RETURN (constraints);
}

static IntMatrix
InitWriteFas (IntMatrix fas, int write_dim, info *arg_info)
{
    int i, j, rows, cols;

    DBUG_ENTER ();

    rows = MatrixRows (fas);
    cols = MatrixCols (fas);

    i = 0;
    while (i < write_dim) {
        MatrixSetEntry (fas, i, i, 1);
        i++;
    }

    j = 0;
    while (j < rows - write_dim) {
        MatrixSetEntry (fas, cols - (rows - write_dim - j), j + write_dim, 1);
        j++;
    }

    DBUG_RETURN (fas);
}

static IntMatrix
InitReadFas (IntMatrix fas, int read_dim, node *arr, info *arg_info)
{
    index_exprs_t *ie;
    cuda_index_t *indices;
    int i, j, rows, cols, x, y;

    DBUG_ENTER ();

    rows = MatrixRows (fas);
    cols = MatrixCols (fas);

    ie = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arr));
    DBUG_ASSERT (((node *)ie != ID_AVIS (arr)), "Found reuse array with null IE!");

    i = 0;
    while (i < IE_NR_ENTRIES (ie)) {
        indices = IE_EXPRS (ie, i);

        y = i;
        while (indices != NULL) {
            switch (CUIDX_TYPE (indices)) {
            case IDX_CONSTANT:
                /* constants are always in the last column */
                x = cols - 1;
                break;
            case IDX_WITHIDS:
                x = NLUTgetNum (INFO_NLUT (arg_info), CUIDX_ID (indices)) - 1;
                break;
            case IDX_EXTID:
                x = INFO_NEST_LEVEL (arg_info)
                    + NLUTgetNum (INFO_NLUT (arg_info), CUIDX_ID (indices)) - 1;
                break;
            default:
                DBUG_ASSERT ((0), "Unknown index type found!");
                break;
            }
            MatrixSetEntry (fas, x, y,
                            MatrixGetEntry (fas, x, y) + CUIDX_COEFFICIENT (indices));

            indices = CUIDX_NEXT (indices);
        }
        i++;
    }

    j = 0;
    while (j < rows - read_dim) {
        MatrixSetEntry (fas, cols - (rows - read_dim - j), j + read_dim, 1);
        j++;
    }

    DBUG_RETURN (fas);
}

/** <!--********************************************************************-->
 *
 * @fn node *RWRprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RWRprf (node *arg_node, info *arg_info)
{
    node *operand1, *operand2;
    int old_coefficient;

    DBUG_ENTER ();

    /* If we are in cuda withloop */
    if (INFO_NEST_LEVEL (arg_info) > 0) {
        switch (PRF_PRF (arg_node)) {
        case F_sel_VxA:
            if (INFO_MODE (arg_info) == TR_normal) {
                node *iv = PRF_ARG1 (arg_node);
                node *arr = PRF_ARG2 (arg_node);
                node *iv_ssaassign, *ids, *avis;
                int dim;

                DBUG_ASSERT (NODE_TYPE (iv) == N_id,
                             "Non-id node found in the first argument of F_sel_VxA!");
                DBUG_ASSERT (NODE_TYPE (arr) == N_id,
                             "Non-id node found in the second argument of F_sel_VxA!");

                iv_ssaassign = ID_SSAASSIGN (iv);

                if (ID_AVIS (arr) == ID_AVIS (INFO_RC (arg_info)) && iv_ssaassign != NULL
                    && NODE_TYPE (ASSIGN_RHS (iv_ssaassign)) == N_array) {
                    dim = TYgetDim (ID_NTYPE (arr));

                    /* Create a new index_exprs struct for the reuse candidate */
                    INFO_IE (arg_info) = CreateIndexExprs (dim);

                    ids = ARRAY_AELEMS (ASSIGN_RHS (iv_ssaassign));
                    INFO_DIM (arg_info) = 0;

                    INFO_MODE (arg_info) = TR_collect;
                    /* Loop through each index */
                    while (ids != NULL) {
                        avis = ID_AVIS (EXPRS_EXPR (ids));

                        /*
                         * Whether the index in the dimension is affine and if
                         * it it what is the constant coefficient
                         */
                        INFO_IS_AFFINE (arg_info) = TRUE;
                        INFO_COEFFICIENT (arg_info) = 1;

                        /* All the magic happens here ;-) */
                        ActOnId (avis, arg_info);

                        /*
                         * If any of the index is non-affine, stop examing the rest
                         * of the indices and assume this access has no reuse
                         */
                        if (!INFO_IS_AFFINE (arg_info)) {
                            /* clean up code needs to be here!!! */
                            break;
                        }

                        INFO_DIM (arg_info)++;
                        ids = EXPRS_NEXT (ids);
                    }
                    INFO_MODE (arg_info) = TR_normal;
                    INFO_DIM (arg_info) = 0;

                    if (INFO_IS_AFFINE (arg_info)) {
                        INFO_LUT (arg_info)
                          = LUTinsertIntoLutP (INFO_LUT (arg_info), ID_AVIS (arr),
                                               INFO_IE (arg_info));
                    } else {
                        /* This array cannot be a reuse candidate */
                    }
                }
            } else if (INFO_MODE (arg_info) == TR_compute) {
                IntMatrix constraints, write_fas, read_fas;
                node *arr = PRF_ARG2 (arg_node);
                int read_dim, write_dim, level, extids;

                if (INFO_RC (arg_info) != NULL
                    && ID_AVIS (arr) == ID_AVIS (INFO_RC (arg_info))) {
                    read_dim = TYgetDim (ID_NTYPE (arr));
                    write_dim = INFO_WRITEDIM (arg_info);
                    level = INFO_NEST_LEVEL (arg_info);
                    extids = INFO_NR_EXTIDS (arg_info);

                    constraints = NewMatrix (level + extids + 2, level * 2);
                    write_fas = NewMatrix (level + extids + 1, write_dim + extids + 1);
                    read_fas = NewMatrix (level + extids + 1, read_dim + extids + 1);

                    constraints = InitConstraints (constraints, arg_info);
                    write_fas = InitWriteFas (write_fas, write_dim, arg_info);
                    read_fas = InitReadFas (read_fas, read_dim, arr, arg_info);

                    printf ("Constraint matrix:\n");
                    MatrixDisplay (constraints, stdout);
                    printf ("Write fas matrix:\n");
                    MatrixDisplay (constraints, stdout);
                    printf ("Read fas matrix:\n");
                    MatrixDisplay (constraints, stdout);
                }
            }
            break;
        case F_add_SxS:
            if (INFO_MODE (arg_info) == TR_collect) {
                operand1 = PRF_ARG1 (arg_node);
                operand2 = PRF_ARG2 (arg_node);

                if (NODE_TYPE (operand1) == N_num) {
                    AddIndex (IDX_CONSTANT,
                              INFO_COEFFICIENT (arg_info) * NUM_VAL (operand1), NULL, 0,
                              INFO_DIM (arg_info), arg_info);
                } else if (NODE_TYPE (operand1) == N_id) {
                    ActOnId (ID_AVIS (operand1), arg_info);
                } else {
                    DBUG_ASSERT (0, "Unknown type of node found in operands!");
                }

                if (NODE_TYPE (operand2) == N_num) {
                    AddIndex (IDX_CONSTANT,
                              INFO_COEFFICIENT (arg_info) * NUM_VAL (operand2), NULL, 0,
                              INFO_DIM (arg_info), arg_info);
                } else if (NODE_TYPE (operand2) == N_id) {
                    ActOnId (ID_AVIS (operand2), arg_info);
                } else {
                    DBUG_ASSERT (0, "Unknown type of node found in operands!");
                }
            }
            break;
        case F_sub_SxS:
            if (INFO_MODE (arg_info) == TR_collect) {
                operand1 = PRF_ARG1 (arg_node);
                operand2 = PRF_ARG2 (arg_node);

                if (NODE_TYPE (operand1) == N_num) {
                    AddIndex (IDX_CONSTANT,
                              INFO_COEFFICIENT (arg_info) * NUM_VAL (operand1), NULL, 0,
                              INFO_DIM (arg_info), arg_info);
                } else if (NODE_TYPE (operand1) == N_id) {
                    ActOnId (ID_AVIS (operand1), arg_info);
                } else {
                    DBUG_ASSERT (0, "Unknown type of node found in operands!");
                }

                if (NODE_TYPE (operand2) == N_num) {
                    old_coefficient = INFO_COEFFICIENT (arg_info);
                    INFO_COEFFICIENT (arg_info) *= -1;
                    AddIndex (IDX_CONSTANT,
                              INFO_COEFFICIENT (arg_info) * NUM_VAL (operand2), NULL, 0,
                              INFO_DIM (arg_info), arg_info);
                    INFO_COEFFICIENT (arg_info) = old_coefficient;
                } else if (NODE_TYPE (operand2) == N_id) {
                    old_coefficient = INFO_COEFFICIENT (arg_info);
                    INFO_COEFFICIENT (arg_info) *= -1;
                    ActOnId (ID_AVIS (operand2), arg_info);
                    INFO_COEFFICIENT (arg_info) = old_coefficient;
                } else {
                    DBUG_ASSERT (0, "Unknown type of node found in operands!");
                }
            }
            break;
        case F_mul_SxS:
            if (INFO_MODE (arg_info) == TR_collect) {
                operand1 = PRF_ARG1 (arg_node);
                operand2 = PRF_ARG2 (arg_node);

                if (NODE_TYPE (operand1) == N_id && NODE_TYPE (operand2) == N_id) {
                    /* Currently, multiplication of two ids are treated as non-affine */
                    INFO_IS_AFFINE (arg_info) = FALSE;
                } else if (NODE_TYPE (operand1) == N_id
                           && NODE_TYPE (operand2) == N_num) {
                    old_coefficient = INFO_COEFFICIENT (arg_info);
                    INFO_COEFFICIENT (arg_info) *= NUM_VAL (operand2);
                    ActOnId (ID_AVIS (operand1), arg_info);
                    INFO_COEFFICIENT (arg_info) = old_coefficient;
                } else if (NODE_TYPE (operand1) == N_num
                           && NODE_TYPE (operand2) == N_id) {
                    old_coefficient = INFO_COEFFICIENT (arg_info);
                    INFO_COEFFICIENT (arg_info) *= NUM_VAL (operand1);
                    ActOnId (ID_AVIS (operand2), arg_info);
                    INFO_COEFFICIENT (arg_info) = old_coefficient;
                } else {
                    DBUG_ASSERT (0, "Unknown type of node found in operands!");
                }
            }
            break;
        case F_neg_S:
            if (INFO_MODE (arg_info) == TR_collect) {
                operand1 = PRF_ARG1 (arg_node);

                if (NODE_TYPE (operand1) == N_num) {
                    old_coefficient = INFO_COEFFICIENT (arg_info);
                    INFO_COEFFICIENT (arg_info) *= -1;
                    AddIndex (IDX_CONSTANT,
                              INFO_COEFFICIENT (arg_info) * NUM_VAL (operand1), NULL, 0,
                              INFO_DIM (arg_info), arg_info);
                    INFO_COEFFICIENT (arg_info) = old_coefficient;
                } else if (NODE_TYPE (operand1) == N_id) {
                    old_coefficient = INFO_COEFFICIENT (arg_info);
                    INFO_COEFFICIENT (arg_info) *= -1;
                    ActOnId (ID_AVIS (operand1), arg_info);
                    INFO_COEFFICIENT (arg_info) = old_coefficient;
                } else {
                    DBUG_ASSERT (0, "Unknown type of node found in operands!");
                }
            }
            break;
        default:
            if (INFO_MODE (arg_info) == TR_collect) {
                /* all other primitives are counted as non-affine */
                INFO_IS_AFFINE (arg_info) = FALSE;
            }
            break;
        }
    }

    DBUG_RETURN (arg_node);
}
/* @} */
