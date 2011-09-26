
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
#include "system.h"
#include "DupTree.h"

#define LOWER_BOUND 0
#define UPPER_BOUND 1
#define MAX_ENTRIES 8
#define MAXLINE 32

/*
 * This file stores three matrices :
 *   - A constraint matrix derive from the withloop bounds;
 *   - A write fas representing the withloop write access matrix;
 *   - A read fas representing a read access matrix.
 */
static const char *outfile = ".polyhedral.out";

/*
 * This file strores the output from the polyhedral utility.
 * There are two possible results:
 *   1) '1' means that the write and read intersects;
 *   2) '0' means that the write and read does not intersect.
 */
static const char *infile = ".result.out";

/******************************************************************************
 *
 * This structure can be used to store one or more summation
 * of variables. Each variable is of type 'cuda_index_t' and
 * the number of variables is stored in 'nr_entries'. The
 * summation can represent either the lower/upper bound of a
 * withloop index or a subscript expressions of one dimension
 * of an array access. In the former case, the number of entires
 * is always two (lower bound and upper bound). In the latter,
 * it depends on the dimensionality of the accessed array.
 *
 */

typedef enum { RO_any, RO_eq, RO_lt, RO_gt, RO_le, RO_ge } relational_op_t;

typedef enum { LO_any, LO_and, LO_or, LO_not } logical_op_t;

typedef struct INDEX_EXPRS_T {
    int nr_entries;
    logical_op_t lop;
    relational_op_t rops[MAX_ENTRIES]; /* Flag indicating whether the exprssion represent
                                        * an equality or not. If it not an equality, it's
                                        * automatically assumed to be >= 0 */
    cuda_index_t *exprs[MAX_ENTRIES];
} index_exprs_t;

#define IE_NR_ENTRIES(n) ((n)->nr_entries)
#define IE_LOP(n) ((n)->lop)
#define IE_ROPS(n, i) ((n->rops)[i])
#define IE_EXPRS(n, i) ((n->exprs)[i])

static index_exprs_t *
CreateIndexExprs (int nr)
{
    int i;
    index_exprs_t *res;

    DBUG_ENTER ();

    res = (index_exprs_t *)MEMmalloc (sizeof (index_exprs_t));

    IE_NR_ENTRIES (res) = nr;
    IE_LOP (res) = LO_any;

    for (i = 0; i < MAX_ENTRIES; i++) {
        IE_EXPRS (res, i) = NULL;
        IE_ROPS (res, i) = RO_any;
    }

    DBUG_RETURN (res);
}

static void
FreeIndexExprs (index_exprs_t *ie)
{
    int i;
    cuda_index_t *exprs;

    DBUG_ENTER ();

    for (i = 0; i < IE_NR_ENTRIES (ie); i++) {
        exprs = IE_EXPRS (ie, i);
        exprs = TBfreeCudaIndex (exprs);
    }

    ie = MEMfree (ie);

    DBUG_RETURN ();
}

/******************************************************************************/
#define MAX_FUNAP_DEPTH 64

typedef struct FUNAP_LIST {
    int count;
    node *aps[MAX_FUNAP_DEPTH];
} funap_list_t;

#define FAP_LIST_COUNT(n) (n->count)
#define FAP_LIST_FUNAP(n, i) ((n->aps)[i])

static funap_list_t *
CreateFunapList ()
{
    funap_list_t *res;
    int i;

    DBUG_ENTER ();

    res = (funap_list_t *)MEMmalloc (sizeof (funap_list_t));

    FAP_LIST_COUNT (res) = 0;

    for (i = 0; i < MAX_FUNAP_DEPTH; i++) {
        FAP_LIST_FUNAP (res, i) = NULL;
    }

    DBUG_RETURN (res);
}

static funap_list_t *
FreeFunapList (funap_list_t *list)
{
    DBUG_ENTER ();

    list = MEMfree (list);
    list = NULL;

    DBUG_RETURN (list);
}

static node *
GetFunap (funap_list_t *list, int i)
{
    node *res;

    DBUG_ENTER ();

    res = FAP_LIST_FUNAP (list, i);

    DBUG_RETURN (res);
}

static funap_list_t *
InsertFunap (funap_list_t *list, node *ap)
{
    DBUG_ENTER ();

    if (FAP_LIST_COUNT (list) < MAX_FUNAP_DEPTH) {
        FAP_LIST_FUNAP (list, FAP_LIST_COUNT (list)) = ap;
        FAP_LIST_COUNT (list)++;
    }

    DBUG_RETURN (list);
}

static funap_list_t *
RemoveFunap (funap_list_t *list)
{
    DBUG_ENTER ();

    if (FAP_LIST_COUNT (list) > 0) {
        FAP_LIST_COUNT (list)--;
    }

    DBUG_RETURN (list);
}

/******************************************************************************/

/* Two different traverse modes */
typedef enum { TR_normal, TR_collect } travmode_t;

/*
 * INFO structure
 */
struct INFO {
    travmode_t mode;
    index_exprs_t *ie;
    int dim;
    lut_t *lut;
    node *rc;
    dfmask_t *mask; /* Mask for external ids and withloop ids */
    nlut_t
      *nlut; /* Used to provide mapping between withloop ids and its nesting level. */
    int nest_level;
    int coefficient;
    int nr_extids;
    bool is_affine;
    node *ivids; /* borrow from ReuseWithArray.c ;-) */
    int writedim;
    node *fundef;
    node *apargs;
    int laclevel;
    funap_list_t *fap_list;
    node *condvar;
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
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_APARGS(n) ((n)->apargs)
#define INFO_LACLEVEL(n) ((n)->laclevel)
#define INFO_FAP_LIST(n) ((n)->fap_list)
#define INFO_CONDVAR(n) ((n)->condvar)

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
    INFO_FUNDEF (result) = NULL;
    INFO_APARGS (result) = NULL;
    INFO_LACLEVEL (result) = 0;
    INFO_FAP_LIST (result) = NULL;
    INFO_CONDVAR (result) = NULL;

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
 * Static helper functions
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

static node *
AnnotateCopyPart (node *with, node *rc)
{
    node *part = WITH_PART (with);

    DBUG_ENTER ();

    while (part != NULL) {
        if (IsNoopPart (part, rc)) {
            PART_ISCOPY (part) = TRUE;
        }

        part = PART_NEXT (part);
    }

    DBUG_RETURN (with);
}

static node *
FindFunargFromAparg (node *apargs, node *funargs, node *aparg)
{
    node *res = NULL;

    DBUG_ENTER ();

    while (apargs != NULL) {
        if (ID_AVIS (EXPRS_EXPR (apargs)) == aparg) {
            res = ARG_AVIS (funargs);
            break;
        }

        apargs = EXPRS_NEXT (apargs);
        funargs = ARG_NEXT (funargs);
    }

    DBUG_RETURN (res);
}

static node *
FindApargFromFunarg (node *funargs, node *apargs, node *funarg)
{
    node *res = NULL;

    DBUG_ENTER ();

    while (funargs != NULL) {
        if (ARG_AVIS (funargs) == funarg) {
            res = ID_AVIS (EXPRS_EXPR (apargs));
            break;
        }

        apargs = EXPRS_NEXT (apargs);
        funargs = ARG_NEXT (funargs);
    }

    DBUG_RETURN (res);
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
            if (INFO_LACLEVEL (arg_info) > 0) {
                node *ap, *ap_arg;
                ap = GetFunap (INFO_FAP_LIST (arg_info), INFO_LACLEVEL (arg_info) - 1);

                /* Find the conditional variable in the calling context */
                ap_arg = FindApargFromFunarg (FUNDEF_ARGS (AP_FUNDEF (ap)), AP_ARGS (ap),
                                              avis);

                /* For variables passed as arguments, continue traversal
                 * in the calling context */
                INFO_LACLEVEL (arg_info)--;
                ActOnId (ap_arg, arg_info);
                INFO_LACLEVEL (arg_info)++;
            } else {
                DBUG_ASSERT (INFO_LACLEVEL (arg_info) == 0, "Wrong lac level!");

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
#if 0
        /* For an id defined outside the withloop, do we continue 
         * traversing its rhs ids??? */
        AddIndex( IDX_EXTID, INFO_COEFFICIENT( arg_info), avis, 
                  0, INFO_DIM( arg_info), arg_info);  

	/* If this external id has not been come across before */
	if( !DFMtestMaskEntry( INFO_MASK( arg_info), NULL, avis)) {
	  DFMsetMaskEntrySet( INFO_MASK( arg_info), NULL, avis);
	  INFO_NR_EXTIDS( arg_info)++;
	  NLUTsetNum( INFO_NLUT( arg_info), avis, INFO_NR_EXTIDS( arg_info));
	}
#endif
                ASSIGN_STMT (ssa_assign) = TRAVopt (ASSIGN_STMT (ssa_assign), arg_info);
            }
        }
        /* Otherwise, we start backtracking to collect data access information */
        else {
            ASSIGN_STMT (ssa_assign) = TRAVopt (ASSIGN_STMT (ssa_assign), arg_info);
        }
    }

    DBUG_RETURN ();
}

static int
GetColumn (cuda_index_t *idx, int cols, info *arg_info)
{
    int col;

    DBUG_ENTER ();

    switch (CUIDX_TYPE (idx)) {
    case IDX_CONSTANT:
        /* constants are always in the last column */
        col = cols - 1;
        break;
    case IDX_WITHIDS:
        col = NLUTgetNum (INFO_NLUT (arg_info), CUIDX_ID (idx));
        break;
    case IDX_EXTID:
        col = INFO_NEST_LEVEL (arg_info)
              + NLUTgetNum (INFO_NLUT (arg_info), CUIDX_ID (idx));
        break;
    default:
        DBUG_ASSERT ((0), "Unknown index type found!");
        break;
    }

    DBUG_RETURN (col);
}

static IntMatrix
InitConstraints (IntMatrix constraints, bool compute_bound, index_exprs_t *cond_ie,
                 int nr_bounds, int cond_nr, info *arg_info)
{
    node *ivids, *ids;
    index_exprs_t *ie;
    cuda_index_t *lb, *ub, *cond;
    int i, cols, x, y;

    DBUG_ENTER ();

    cols = MatrixCols (constraints);

    if (compute_bound) {
        ivids = INFO_IVIDS (arg_info);

        i = 0;
        while (ivids != NULL) {
            ids = SET_MEMBER (ivids);
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
                    x = GetColumn (lb, cols, arg_info);
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
                    x = GetColumn (ub, cols, arg_info);
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
    }

    if (cond_ie != NULL) {
        y = nr_bounds + cond_nr;

        MatrixSetEntry (constraints, 0, y, 1);

        cond = IE_EXPRS (cond_ie, cond_nr);
        while (cond != NULL) {
            x = GetColumn (cond, cols, arg_info);
            MatrixSetEntry (constraints, x, y,
                            MatrixGetEntry (constraints, x, y)
                              + CUIDX_COEFFICIENT (cond));
            cond = CUIDX_NEXT (cond);
        }
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
    ie = INFO_IE (arg_info);

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

static bool
CheckIntersection (IntMatrix constraints, IntMatrix write_fas, IntMatrix read_fas)
{
    bool res;
    FILE *matrix_file, *res_file;
    char buffer[MAXLINE];

    DBUG_ENTER ();

    printf ("hahah, checking intersection...\n");

    matrix_file = fopen (outfile, "w");
    MatrixToFile (constraints, matrix_file);
    MatrixToFile (write_fas, matrix_file);
    MatrixToFile (read_fas, matrix_file);
    fclose (matrix_file);

    SYScall ("$SAC2CBASE/src/tools/cuda/polyhedral < %s > %s\n", outfile, infile);

    res_file = fopen (infile, "r");
    res = atoi (fgets (buffer, MAXLINE, res_file)) == 0 ? FALSE : TRUE;
    fclose (res_file);

    // SYScall("rm -f %s %s\n", outfile, infile);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Region-aware With-Loop reuse candidate inference (rwr)
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
                node *oldpartnext, *oldcodenext, *hotcode;
                info *arg_info;
                dfmask_base_t *maskbase;

                arg_info = MakeInfo ();

                INFO_LUT (arg_info) = LUTgenerateLut ();
                INFO_NLUT (arg_info)
                  = NLUTgenerateNlut (FUNDEF_ARGS (fundef), FUNDEF_VARDECS (fundef));
                maskbase = DFMgenMaskBase (FUNDEF_ARGS (fundef), FUNDEF_VARDECS (fundef));
                INFO_MASK (arg_info) = DFMgenMaskClear (maskbase);
                INFO_FAP_LIST (arg_info) = CreateFunapList ();

                INFO_RC (arg_info) = cand;
                cand = NULL;

                hotcode = PART_CODE (hotpart);
                oldcodenext = CODE_NEXT (hotcode);
                CODE_NEXT (hotcode) = NULL;

                oldpartnext = PART_NEXT (hotpart);
                PART_NEXT (hotpart) = NULL;

                /* Start traversing the hot partition */
                TRAVpush (TR_rwr);
                hotpart = TRAVdo (hotpart, arg_info);
                TRAVpop ();

                CODE_NEXT (hotcode) = oldcodenext;
                PART_NEXT (hotpart) = oldpartnext;

                if (INFO_RC (arg_info) != NULL) {
                    with = AnnotateCopyPart (with, INFO_RC (arg_info));
                    cand = TBmakeExprs (INFO_RC (arg_info), NULL);
                    INFO_RC (arg_info) = NULL;
                }
                INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
                INFO_NLUT (arg_info) = NLUTremoveNlut (INFO_NLUT (arg_info));
                INFO_MASK (arg_info) = DFMremoveMask (INFO_MASK (arg_info));
                maskbase = DFMremoveMaskBase (maskbase);
                INFO_FAP_LIST (arg_info) = FreeFunapList (INFO_FAP_LIST (arg_info));

                arg_info = FreeInfo (arg_info);
            } else {
                cand = FREEdoFreeTree (cand);
            }
        }
    }

    DBUG_RETURN (cand);
}

/** <!--********************************************************************-->
 *
 * @fn node *RWRwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RWRwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* We do not traverse WITH_CODE here as the code will be traversed
     * when the partition is traversed. Do so avoids traversing the code
     * twice. */
    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    DBUG_RETURN (arg_node);
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
    node *ids_iter, *ids, *lb, *ub;

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
        ids_iter = ids;
        while (ids_iter != NULL) {
            DFMsetMaskEntrySet (INFO_MASK (arg_info), NULL, IDS_AVIS (ids_iter));

            /* store the nest level of the withloop ids */
            NLUTsetNum (INFO_NLUT (arg_info), IDS_AVIS (ids_iter),
                        INFO_NEST_LEVEL (arg_info) + i);

            /* Create a new index_exprs struct for each withloop ids */
            INFO_IE (arg_info) = CreateIndexExprs (UPPER_BOUND + 1);

            IE_LOP (INFO_IE (arg_info)) = LO_and;
            IE_ROPS (INFO_IE (arg_info), LOWER_BOUND) = RO_ge;
            IE_ROPS (INFO_IE (arg_info), UPPER_BOUND) = RO_gt;

            /* Start collecting lower bound expressions */
            if (COisConstant (EXPRS_EXPR (lb))) {
                IE_EXPRS (INFO_IE (arg_info), LOWER_BOUND)
                  = TBmakeCudaIndex (IDX_CONSTANT,
                                     -COconst2Int (COaST2Constant (EXPRS_EXPR (lb))),
                                     NULL, 0, IE_EXPRS (INFO_IE (arg_info), LOWER_BOUND));
            } else {
                INFO_DIM (arg_info) = LOWER_BOUND;
                INFO_COEFFICIENT (arg_info) = -1;
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

            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (ids_iter),
                                   INFO_IE (arg_info));
            INFO_IE (arg_info) = NULL;

            ids_iter = IDS_NEXT (ids_iter);
            lb = EXPRS_NEXT (lb);
            ub = EXPRS_NEXT (ub);
            i++;
        }
        INFO_MODE (arg_info) = TR_normal;

        if (INFO_NEST_LEVEL (arg_info) == 0) {
            INFO_WRITEDIM (arg_info) = dim;
        }

        /* Push withids */
        INFO_IVIDS (arg_info)
          = TCappendSet (INFO_IVIDS (arg_info), TBmakeSet (ids, NULL));

        INFO_NEST_LEVEL (arg_info) += dim;
        PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);
        INFO_NEST_LEVEL (arg_info) -= dim;

        /* Pop withids */
        INFO_IVIDS (arg_info) = TCdropSet (-1, INFO_IVIDS (arg_info));

        if (PART_NEXT (arg_node) == NULL) {
            index_exprs_t *ie;
            /* last partition of the withloop, we cleanup LUT */
            while (ids != NULL) {
                ie = LUTsearchInLutPp (INFO_LUT (arg_info), IDS_AVIS (ids));
                FreeIndexExprs (ie);
                ids = IDS_NEXT (ids);
            }
        } else {
            PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);
        }
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
    } else {
        DBUG_ASSERT (0, "Wrong traverse mode!");
    }

    DBUG_RETURN (arg_node);
}

static index_exprs_t *
ComputeElseCondition (index_exprs_t *ie)
{
    int i;
    cuda_index_t *idx_exprs;
    index_exprs_t *new_ie;

    DBUG_ENTER ();

    /* Creare empty IE */
    new_ie = CreateIndexExprs (0);

    for (i = 0; i < IE_NR_ENTRIES (ie); i++) {
        switch (IE_ROPS (ie, i)) {
        case RO_any:
            break;
        case RO_eq:
            break;
        case RO_lt:
            break;
        case RO_gt:
            break;
        case RO_le:
            break;
        case RO_ge:
            IE_LOP (new_ie) = LO_or;
            /*
             * If we have a condition of the form:
             *   a1n1+a2n2+...amnm == 0
             * we generate two inequalities for the
             * 'else' branch:
             *   1) a1n1+a2n2+...amnm - 1 >= 0
             *   2) a1n1+a2n2+...amnm + 1 <= 0
             * The reason to add '-1' and '+1' in the inequalities is because
             * we need to ensure the lhs is '>=' (or '<=') zero instead of i'>'
             * ('<') zero to meet the polylib requirements. For the second case,
             * we also convert '<=' to '>=' by negating each term on the lhs.
             * Be careful that the above two inequalities are ORed together so
             * when creating constraints, they should not apprear in the same matrix.
             */
            idx_exprs = DUPCudaIndex (IE_EXPRS (ie, i));
            idx_exprs = TBmakeCudaIndex (IDX_CONSTANT, 1, NULL, 0, idx_exprs);
            IE_ROPS (new_ie, IE_NR_ENTRIES (new_ie)) = RO_ge;
            IE_EXPRS (new_ie, IE_NR_ENTRIES (new_ie)) = idx_exprs;
            /* Negate all lhs terms */
            while (idx_exprs != NULL) {
                CUIDX_COEFFICIENT (idx_exprs) = -CUIDX_COEFFICIENT (idx_exprs);
                idx_exprs = CUIDX_NEXT (idx_exprs);
            }
            IE_NR_ENTRIES (new_ie)++;
            break;
        default:
            break;
        }
    }

    /* The orignial 'then' condition expression can be freed */
    FreeIndexExprs (ie);

    DBUG_RETURN (new_ie);
}

/** <!--********************************************************************-->
 *
 * @fn node *RWRcond( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RWRcond (node *arg_node, info *arg_info)
{
    node *old_condvar, *condvar, *ext_condvar, *ap;

    DBUG_ENTER ();

    if (INFO_MODE (arg_info) == TR_normal) {
        condvar = COND_COND (arg_node);
        DBUG_ASSERT (NODE_TYPE (condvar) == N_id, "Conditional variable is not N_id!");

        DBUG_ASSERT (NODE_TYPE (ID_DECL (condvar)) == N_arg,
                     "Conditional variable is not an argument!");

        /* The reason we need to minus 1 is because an ap at
         * level n is stored in the (n-1)th position in the
         * funap list. */
        ap = GetFunap (INFO_FAP_LIST (arg_info), INFO_LACLEVEL (arg_info) - 1);

        printf ("++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf ("Conditional function %s\n", FUNDEF_NAME (AP_FUNDEF (ap)));
        printf ("I'm going to look for external var for %s\n",
                AVIS_NAME (ARG_AVIS (ID_DECL (condvar))));
        printf ("++++++++++++++++++++++++++++++++++++++++++++++++++\n");

        /* Find the conditional variable in the calling context */
        ext_condvar = FindApargFromFunarg (FUNDEF_ARGS (AP_FUNDEF (ap)), AP_ARGS (ap),
                                           ARG_AVIS (ID_DECL (condvar)));
        DBUG_ASSERT ((ext_condvar != NULL), "External conditional variable is NULL!");

        INFO_MODE (arg_info) = TR_collect;
        INFO_LACLEVEL (arg_info)--;
        INFO_IS_AFFINE (arg_info) = TRUE;
        ActOnId (ext_condvar, arg_info);
        INFO_LACLEVEL (arg_info)++;
        INFO_MODE (arg_info) = TR_normal;

        if (INFO_IS_AFFINE (arg_info)) {
            old_condvar = INFO_CONDVAR (arg_info);
            INFO_CONDVAR (arg_info) = condvar;

            /* Traverse 'then' branch */
            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), ID_AVIS (condvar),
                                   INFO_IE (arg_info));

            COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);

            /* Traverse 'else' branch */
            /*
                  INFO_IE( arg_info) = ComputeElseCondition( INFO_IE( arg_info));
                  INFO_LUT( arg_info) =
                    LUTinsertIntoLutP( INFO_LUT( arg_info),
                                       ID_AVIS( condvar),
                                       INFO_IE( arg_info));

                  COND_ELSE( arg_node) = TRAVopt( COND_ELSE( arg_node), arg_info);

                  INFO_CONDVAR( arg_info) = old_condvar;
            */
        } else {
            INFO_RC (arg_info) = FREEdoFreeNode (INFO_RC (arg_info));
            INFO_RC (arg_info) = NULL;
            FreeIndexExprs (INFO_IE (arg_info));
            INFO_IE (arg_info) = NULL;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RWRap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RWRap (node *arg_node, info *arg_info)
{
    node *rc, *old_rc;

    DBUG_ENTER ();

    if (INFO_MODE (arg_info) == TR_normal) {
        if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node)) && INFO_RC (arg_info) != NULL) {
            rc = FindFunargFromAparg (AP_ARGS (arg_node),
                                      FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                                      ID_AVIS (INFO_RC (arg_info)));
            if (rc != NULL) {
                /* Push info */
                INFO_FAP_LIST (arg_info)
                  = InsertFunap (INFO_FAP_LIST (arg_info), arg_node);
                old_rc = INFO_RC (arg_info);
                INFO_RC (arg_info) = TBmakeId (rc);

                INFO_LACLEVEL (arg_info)++;
                AP_FUNDEF (arg_node) = TRAVopt (AP_FUNDEF (arg_node), arg_info);
                INFO_LACLEVEL (arg_info)--;

                /* Pop info */
                INFO_FAP_LIST (arg_info) = RemoveFunap (INFO_FAP_LIST (arg_info));
                INFO_RC (arg_info) = FREEdoFreeNode (INFO_RC (arg_info));
                INFO_RC (arg_info) = old_rc;
            }
        }
    } else {
        INFO_IS_AFFINE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
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

    switch (PRF_PRF (arg_node)) {
    case F_sel_VxA:
        if (INFO_MODE (arg_info) == TR_normal) {
            IntMatrix constraints, write_fas, read_fas;
            node *iv = PRF_ARG1 (arg_node);
            node *arr = PRF_ARG2 (arg_node);
            node *iv_ssaassign, *ids, *avis;
            int dim, valid = TRUE;

            DBUG_ASSERT (NODE_TYPE (iv) == N_id,
                         "Non-id node found in the first argument of F_sel_VxA!");
            DBUG_ASSERT (NODE_TYPE (arr) == N_id,
                         "Non-id node found in the second argument of F_sel_VxA!");

            iv_ssaassign = ID_SSAASSIGN (iv);

            if (INFO_RC (arg_info) != NULL
                && ID_AVIS (arr) == ID_AVIS (INFO_RC (arg_info)) && iv_ssaassign != NULL
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
                        valid = FALSE;
                        break;
                    }

                    INFO_DIM (arg_info)++;
                    ids = EXPRS_NEXT (ids);
                }
                INFO_MODE (arg_info) = TR_normal;
                INFO_DIM (arg_info) = 0;

                if (valid) {
                    int i, read_dim, write_dim, level, extids, nr_conds = 0;
                    bool intersected = FALSE;
                    index_exprs_t *ie = NULL;

                    /* If the array access is within a conditional, nr_conds stores
                     * the number of equalitiy or inequality implied by the conditional
                     */
                    if (INFO_CONDVAR (arg_info) != NULL) {
                        ie = LUTsearchInLutPp (INFO_LUT (arg_info),
                                               ID_AVIS (INFO_CONDVAR (arg_info)));
                        DBUG_ASSERT (((node *)ie != ID_AVIS (INFO_CONDVAR (arg_info))),
                                     "Found condvar with null IE!");
                        nr_conds = IE_NR_ENTRIES (ie);
                    }

                    read_dim = TYgetDim (ID_NTYPE (arr));
                    write_dim = INFO_WRITEDIM (arg_info);
                    level = INFO_NEST_LEVEL (arg_info);
                    extids = INFO_NR_EXTIDS (arg_info);

                    write_fas = NewMatrix (level + extids + 1, write_dim + extids + 1);
                    read_fas = NewMatrix (level + extids + 1, read_dim + extids + 1);
                    write_fas = InitWriteFas (write_fas, write_dim, arg_info);
                    read_fas = InitReadFas (read_fas, read_dim, arr, arg_info);

                    /* For each equality or inequality, we construct a constraint matrix
                     * and compute the intersection with the images trasformed by write
                     * and read access matrix. Only all of them produce no intersection
                     * can we conclude that there is not intersection. */
                    if (IE_LOP (ie) == LO_and) {
                        constraints
                          = NewMatrix (level + extids + 2, level * 2 + nr_conds);
                        for (i = 0; i < nr_conds; i++) {
                            if (i == 0) {
                                printf ("i == 0\n");
                                constraints = InitConstraints (constraints, TRUE, ie,
                                                               level * 2, i, arg_info);
                            } else {
                                printf ("i != 0\n");
                                constraints = InitConstraints (constraints, FALSE, ie,
                                                               level * 2, i, arg_info);
                            }
                        }
                        intersected
                          = CheckIntersection (constraints, write_fas, read_fas);
                        FreeMatrix (constraints);
                    } else if (IE_LOP (ie) == LO_or) {
                        for (i = 0; i < nr_conds; i++) {
                            constraints = NewMatrix (level + extids + 2, level * 2 + 1);
                            constraints = InitConstraints (constraints, TRUE, ie,
                                                           level * 2, i, arg_info);
                            intersected
                              = (intersected
                                 || CheckIntersection (constraints, write_fas, read_fas));
                            FreeMatrix (constraints);
                        }
                    } else {
                    }

                    if (intersected) {
                        INFO_RC (arg_info) = FREEdoFreeNode (INFO_RC (arg_info));
                        INFO_RC (arg_info) = NULL;
                    }
                }

                /* Cleaning up */
                FreeMatrix (write_fas);
                FreeMatrix (read_fas);
                FreeIndexExprs (INFO_IE (arg_info));
                INFO_IE (arg_info) = NULL;
            }
        }
        break;
    case F_add_SxS:
        if (INFO_MODE (arg_info) == TR_collect) {
            operand1 = PRF_ARG1 (arg_node);
            operand2 = PRF_ARG2 (arg_node);

            if (COisConstant (operand1)) {
                AddIndex (IDX_CONSTANT,
                          INFO_COEFFICIENT (arg_info)
                            * COconst2Int (COaST2Constant (operand1)),
                          NULL, 0, INFO_DIM (arg_info), arg_info);
            } else {
                ActOnId (ID_AVIS (operand1), arg_info);
            }

            if (COisConstant (operand2)) {
                AddIndex (IDX_CONSTANT,
                          INFO_COEFFICIENT (arg_info)
                            * COconst2Int (COaST2Constant (operand2)),
                          NULL, 0, INFO_DIM (arg_info), arg_info);
            } else {
                ActOnId (ID_AVIS (operand2), arg_info);
            }
        }
        break;
    case F_sub_SxS:
        if (INFO_MODE (arg_info) == TR_collect) {
            operand1 = PRF_ARG1 (arg_node);
            operand2 = PRF_ARG2 (arg_node);

            if (COisConstant (operand1)) {
                AddIndex (IDX_CONSTANT,
                          INFO_COEFFICIENT (arg_info)
                            * COconst2Int (COaST2Constant (operand1)),
                          NULL, 0, INFO_DIM (arg_info), arg_info);
            } else {
                ActOnId (ID_AVIS (operand1), arg_info);
            }

            if (COisConstant (operand2)) {
                old_coefficient = INFO_COEFFICIENT (arg_info);
                INFO_COEFFICIENT (arg_info) *= -1;
                AddIndex (IDX_CONSTANT,
                          INFO_COEFFICIENT (arg_info)
                            * COconst2Int (COaST2Constant (operand2)),
                          NULL, 0, INFO_DIM (arg_info), arg_info);
                INFO_COEFFICIENT (arg_info) = old_coefficient;
            } else {
                old_coefficient = INFO_COEFFICIENT (arg_info);
                INFO_COEFFICIENT (arg_info) *= -1;
                ActOnId (ID_AVIS (operand2), arg_info);
                INFO_COEFFICIENT (arg_info) = old_coefficient;
            }
        }
        break;
    case F_mul_SxS:
        if (INFO_MODE (arg_info) == TR_collect) {
            operand1 = PRF_ARG1 (arg_node);
            operand2 = PRF_ARG2 (arg_node);

            if (!COisConstant (operand1) && !COisConstant (operand2)) {
                /* Currently, multiplication of two non-constants
                 * are treated as non-affine */
                INFO_IS_AFFINE (arg_info) = FALSE;
            } else if (!COisConstant (operand1) && COisConstant (operand2)) {
                old_coefficient = INFO_COEFFICIENT (arg_info);
                INFO_COEFFICIENT (arg_info) *= COconst2Int (COaST2Constant (operand2));
                ActOnId (ID_AVIS (operand1), arg_info);
                INFO_COEFFICIENT (arg_info) = old_coefficient;
            } else if (COisConstant (operand1) && !COisConstant (operand2)) {
                old_coefficient = INFO_COEFFICIENT (arg_info);
                INFO_COEFFICIENT (arg_info) *= COconst2Int (COaST2Constant (operand1));
                ActOnId (ID_AVIS (operand2), arg_info);
                INFO_COEFFICIENT (arg_info) = old_coefficient;
            } else {
                AddIndex (IDX_CONSTANT,
                          INFO_COEFFICIENT (arg_info)
                            * COconst2Int (COaST2Constant (operand1))
                            * COconst2Int (COaST2Constant (operand2)),
                          NULL, 0, INFO_DIM (arg_info), arg_info);
            }
        }
        break;
    case F_neg_S:
        if (INFO_MODE (arg_info) == TR_collect) {
            operand1 = PRF_ARG1 (arg_node);

            if (COisConstant (operand1)) {
                old_coefficient = INFO_COEFFICIENT (arg_info);
                INFO_COEFFICIENT (arg_info) *= -1;
                AddIndex (IDX_CONSTANT,
                          INFO_COEFFICIENT (arg_info)
                            * COconst2Int (COaST2Constant (operand1)),
                          NULL, 0, INFO_DIM (arg_info), arg_info);
                INFO_COEFFICIENT (arg_info) = old_coefficient;
            } else {
                old_coefficient = INFO_COEFFICIENT (arg_info);
                INFO_COEFFICIENT (arg_info) *= -1;
                ActOnId (ID_AVIS (operand1), arg_info);
                INFO_COEFFICIENT (arg_info) = old_coefficient;
            }
        }
        break;
    case F_eq_SxS:
        if (INFO_MODE (arg_info) == TR_collect) {
            cuda_index_t *idx;
            operand1 = PRF_ARG1 (arg_node);
            operand2 = PRF_ARG2 (arg_node);
            INFO_IE (arg_info) = CreateIndexExprs (2);
            IE_LOP (INFO_IE (arg_info)) = LO_and;
            IE_ROPS (INFO_IE (arg_info), 0) = RO_ge;
            IE_ROPS (INFO_IE (arg_info), 0) = RO_ge;

            INFO_DIM (arg_info) = 0;
            if (COisConstant (operand1)) {
                AddIndex (IDX_CONSTANT, COconst2Int (COaST2Constant (operand1)), NULL, 0,
                          INFO_DIM (arg_info), arg_info);
            } else {
                INFO_COEFFICIENT (arg_info) = 1;
                ActOnId (ID_AVIS (operand1), arg_info);
            }

            if (COisConstant (operand2)) {
                AddIndex (IDX_CONSTANT, -COconst2Int (COaST2Constant (operand2)), NULL, 0,
                          INFO_DIM (arg_info), arg_info);
            } else {
                INFO_COEFFICIENT (arg_info) = -1;
                ActOnId (ID_AVIS (operand2), arg_info);
            }

            idx = IE_EXPRS (INFO_IE (arg_info), 0);
            IE_EXPRS (INFO_IE (arg_info), 1) = DUPCudaIndex (idx);
            while (idx != NULL) {
                CUIDX_COEFFICIENT (idx) = -CUIDX_COEFFICIENT (idx);
                idx = CUIDX_NEXT (idx);
            }
        }
        break;
    case F_lt_SxS:
        if (INFO_MODE (arg_info) == TR_collect) {
        }
        break;
    case F_gt_SxS:
        if (INFO_MODE (arg_info) == TR_collect) {
        }
        break;
    case F_le_SxS:
        if (INFO_MODE (arg_info) == TR_collect) {
        }
        break;
    case F_ge_SxS:
        if (INFO_MODE (arg_info) == TR_collect) {
        }
        break;
    default:
        if (INFO_MODE (arg_info) == TR_collect) {
            /* all other primitives are counted as non-affine */
            INFO_IS_AFFINE (arg_info) = FALSE;
        }
        break;
    }

    DBUG_RETURN (arg_node);
}
/* @} */
