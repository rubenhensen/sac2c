
/**
 *
 * @defgroup pra Polyhedral based WITH-loop reuse candidate analysis
 *
 * @ingroup mm
 *
 * @{
 */

/**
 *
 * @file polyhedral_reuse_analysis.c
 *
 * Prefix: PRA
 */
#include "polyhedral_reuse_analysis.h"

#define DBUG_PREFIX "PRA"
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
#include "identify_noop_branch.h"
#include "filemgr.h"
#include "reusewithoffset.h"
#include "sacdirs.h"

#define LOWER_BOUND 0
#define UPPER_BOUND 1
#define MAX_ENTRIES 8
#define MAXLINE 32

/* Statistic counter, records the number of polyhedral files generated */
static int count = 0;

/*
 * This file stores three matrices :
 *   - A constraint matrix derived from the withloop bounds and
 *     predicates of conditionals;
 *   - A write access matrix representing the transformation from
 *     withloop iteration space to the write data space;
 *   - A read access matrix representing the transformation from
 *     withloop iteration space to the read data space.
 */
static const char *outfile = "polyhedral";

/*
 * This file strores the output from the polyhedral utility.
 * There are two possible outcomes:
 *   1) '1' means that the write and read intersects;
 *   2) '0' means that the write and read does not intersect.
 */
static const char *infile = "result";

/******************************************************************************
 *
 * The index_exprs_t struct is used to store one or more equalities or
 * inequalities. The exact number is stored in 'nr_entries'. Note that
 * if the inequalities represent the withloop lower/upper bounds, the
 * number will always be exactly 2. For conditional predicates, the
 * number can vary depending on the exact conditions considered. All
 * equalities/inequalities are compared against zeros on the rhs. Each
 * lhs expression represents the summation of one or more variables chained
 * together with the cuda_index_t structs. The exact relational operator
 * or each equality/inequality is stored in the 'rops' array. Moreover,
 * the logical relationship between all equalities/inequalties are stored
 * in 'lop'. Note that since we use a single variable to store this
 * relationship, this means we are not allowed to have equalities/inequalities
 * with mixed logical relationship (TODO: this needs to be generalised
 * in the future). The reason we need to specify the logical relationship
 * is because when generating constraint matrix, equalities/inequalities
 * with 'AND" relationship must be appear in the same matrix while those
 * with 'OR" relationship can only be appear one at a time, following the
 * withloop bound constraints.
 * Note that this struct can also be used to represent subscript expression
 * of an array access. In this case, the number of entires depends on the
 * dimensionality of the array. Also, neither 'lop' nor 'rops' are relevant
 * here can should be set to 'LO_any' and 'RO_any' repectively (means we do
 * not care).
 */

typedef enum { RO_any, RO_eq, RO_lt, RO_gt, RO_le, RO_ge } relational_op_t;

typedef enum { LO_any, LO_and, LO_or, LO_not } logical_op_t;

typedef struct INDEX_EXPRS_T {
    unsigned int nr_entries;
    logical_op_t lop;
    relational_op_t rops[MAX_ENTRIES];
    cuda_index_t *exprs[MAX_ENTRIES];
} index_exprs_t;

#define IE_NR_ENTRIES(n) (n->nr_entries)
#define IE_LOP(n) (n->lop)
#define IE_ROPS(n, i) ((n->rops)[i])
#define IE_EXPRS(n, i) ((n->exprs)[i])

static index_exprs_t *
CreateIndexExprs (unsigned int nr)
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
    unsigned int i;
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

/*
 * This struct stores a list of function applications. Currently, all
 * applications are conditional function applications. The number of
 * stored application is stored in 'count'.
 */
typedef struct FUNAP_LIST {
    int count;
    node *aps[MAX_FUNAP_DEPTH];
} funap_list_t;

#define FAP_LIST_COUNT(n) (n->count)
#define FAP_LIST_FUNAP(n, i) ((n->aps)[i])

static funap_list_t *
CreateFunapList (void)
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

/* Two different traversal modes */
typedef enum { TR_normal, TR_collect } travmode_t;

/*
 * INFO structure
 */
struct INFO {
    travmode_t mode;
    index_exprs_t *ie;
    int dim; /* Which dimension are we currently traversing? */
    lut_t
      *lut; /* Proveide two mappings: 1) part_id->index_exprs_t(lower/upper bound); 2)
               predicate_id->index_exprs_t( predicate expressions) */
    node *rc;
    dfmask_t *mask; /* Mask for external N_ids and withloop ids */
    nlut_t *nlut; /* Used to provide mapping between withloop ids and its nesting level */
    unsigned int nest_level;
    int coefficient;
    unsigned int nr_extids; /* Total number external N_ids found during TR_normal traversal */
    bool is_affine;
    node *ivids;  /* borrow from ReuseWithArray.c ;-) */
    unsigned int writedim; /* This is essentially the dimensionality of the horpart */
    int laclevel; /* Nesting level of the funap. This can be used to get the actual N_ap
                     node from the fap_list */
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
#define INFO_LACLEVEL(n) ((n)->laclevel)
#define INFO_FAP_LIST(n) ((n)->fap_list)
#define INFO_CONDVAR(n) ((n)->condvar)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

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

/** <!--********************************************************************-->
 *
 * @fn node
 *
 * @brief
 *
 *
 *****************************************************************************/
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

/** <!--********************************************************************-->
 *
 * @fn node
 *
 * @brief
 *
 *
 *****************************************************************************/
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

/** <!--********************************************************************-->
 *
 * @fn node
 *
 * @brief
 *
 *
 *****************************************************************************/
static void
AddIndex (unsigned int type, int coefficient, node *idx, size_t looplevel, int dim,
          info *arg_info)
{
    DBUG_ENTER ();

    IE_EXPRS (INFO_IE (arg_info), dim)
      = TBmakeCudaIndex (type, coefficient, idx, looplevel,
                         IE_EXPRS (INFO_IE (arg_info), dim));

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn node
 *
 * @brief
 *
 *
 *****************************************************************************/
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
                if (!DFMtestMaskEntry (INFO_MASK (arg_info), avis)) {
                    DFMsetMaskEntrySet (INFO_MASK (arg_info), avis);
                    INFO_NR_EXTIDS (arg_info)++;
                    NLUTsetNum (INFO_NLUT (arg_info), avis, INFO_NR_EXTIDS (arg_info));
                }
            }
        }
        /* This is a withloop ids. We search for the exact ids */
        else if (DFMtestMaskEntry (INFO_MASK (arg_info), avis)) {
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
	if( !DFMtestMaskEntry( INFO_MASK( arg_info), avis)) {
	  DFMsetMaskEntrySet( INFO_MASK( arg_info), avis);
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

/** <!--********************************************************************-->
 *
 * @fn node
 *
 * @brief
 *
 *
 *****************************************************************************/
static unsigned int
GetColumn (cuda_index_t *idx, unsigned int cols, info *arg_info)
{
    unsigned int col = 0;

    DBUG_ENTER ();

    switch (CUIDX_TYPE (idx)) {
    case IDX_CONSTANT:
        /* constants are always in the last column */
        col = cols - 1;
        break;
    case IDX_WITHIDS:
        col = (unsigned int)NLUTgetNum (INFO_NLUT (arg_info), CUIDX_ID (idx));
        break;
    case IDX_EXTID:
        col = INFO_NEST_LEVEL (arg_info)
              + (unsigned int)NLUTgetNum (INFO_NLUT (arg_info), CUIDX_ID (idx));
        break;
    default:
        DBUG_ASSERT ((0), "Unknown index type found!");
        break;
    }

    DBUG_RETURN (col);
}

/** <!--********************************************************************-->
 *
 * @fn node
 *
 * @brief
 *
 *
 *****************************************************************************/
static IntMatrix
InitConstraints (IntMatrix constraints, bool compute_bound, index_exprs_t *cond_ie,
                 unsigned int nr_bounds, unsigned int cond_nr, info *arg_info)
{
    node *ivids, *ids;
    index_exprs_t *ie;
    cuda_index_t *lb, *ub, *cond;
    unsigned int i, cols, x, y;

    DBUG_ENTER ();

    cols = MatrixCols (constraints);

    if (compute_bound) {
        ivids = INFO_IVIDS (arg_info);

        i = 0;
        while (ivids != NULL) {
            ids = SET_MEMBER (ivids);
            while (ids != NULL) {
                ie = (index_exprs_t *)LUTsearchInLutPp (INFO_LUT (arg_info),
                                                        IDS_AVIS (ids));
                DBUG_ASSERT (((node *)ie != IDS_AVIS (ids)),
                             "Found withloop ids with null IE!");

                lb = IE_EXPRS (ie, LOWER_BOUND);
                ub = IE_EXPRS (ie, UPPER_BOUND);

                /* Initialize the constraint for lower bound */
                x = (unsigned int)NLUTgetNum (INFO_NLUT (arg_info), IDS_AVIS (ids));
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
                x = (unsigned int)NLUTgetNum (INFO_NLUT (arg_info), IDS_AVIS (ids));
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

/** <!--********************************************************************-->
 *
 * @fn IntMatrix InitWriteFas( IntMatrix fas, int write_dim, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
static IntMatrix
InitWriteFas (IntMatrix fas, unsigned int write_dim, info *arg_info)
{
    unsigned int i, j, rows, cols;

    DBUG_ENTER ();

    rows = MatrixRows (fas);
    cols = MatrixCols (fas);

    /*
     * Continue with the example given in the call site:
     *
     * part1: [i,j]
     * part2: [k]
     * Array: A[j,k]
     * extid: e
     *
     *                i  j  k  e  cnst
     *   part_dim0    0  0  0  0   0
     *   part_dim1    0  0  0  0   0
     *   extid        0  0  0  0   0
     *   cnst         0  0  0  0   0
     *
     * Initializing write fas is straightforward. First initilize
     * each corresponding part_dim to 1 to give:
     *
     *                i  j  k  e  cnst
     *   part_dim0    1  0  0  0   0
     *   part_dim1    0  1  0  0   0
     *   extid        0  0  0  0   0
     *   cnst         0  0  0  0   0
     */
    i = 0;
    while (i < write_dim) {
        MatrixSetEntry (fas, i, i, 1);
        i++;
    }

    /*
     * Then initilize each extid and constant to 1 to give:
     *
     *                i  j  k  e  cnst
     *   part_dim0    1  0  0  0   0
     *   part_dim1    0  1  0  0   0
     *   extid        0  0  0  1   0
     *   cnst         0  0  0  0   1
     */
    j = 0;
    while (j < rows - write_dim) {
        MatrixSetEntry (fas, cols - (rows - write_dim - j), j + write_dim, 1);
        j++;
    }

    DBUG_RETURN (fas);
}

/** <!--********************************************************************-->
 *
 * @fn node
 *
 * @brief
 *
 *
 *****************************************************************************/
static IntMatrix
InitReadFas (IntMatrix fas, unsigned int read_dim, node *arr, info *arg_info)
{
    index_exprs_t *ie;
    cuda_index_t *indices; 
    unsigned int i, j, rows, cols, x =0, y = 0;

    DBUG_ENTER ();

    rows = MatrixRows (fas);
    cols = MatrixCols (fas);
    ie = INFO_IE (arg_info);

    /*
     * Continue with the example given in the call site:
     *
     * part1: [i,j]
     * part2: [k]
     * Array: A[j,k]
     * extid: e
     *
     *                i  j  k  e  cnst
     *   arr_dim0     0  0  0  0   0
     *   arr_dim1     0  0  0  0   0
     *   extid        0  0  0  0   0
     *   cnst         0  0  0  0   0
     *
     * For each array dimension, we dissect its subscript. Depending
     * on the type of the variable encountered, we update the corresponding
     * entry in the matrix. For the give example, we get:
     *
     *                i  j  k  e  cnst
     *   part_dim0    0  1  0  0   0
     *   part_dim1    0  0  1  0   0
     *   extid        0  0  0  0   0
     *   cnst         0  0  0  0   0
     */

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
                x = (unsigned int)NLUTgetNum (INFO_NLUT (arg_info), CUIDX_ID (indices)) - 1;
                break;
            case IDX_EXTID:
                x = INFO_NEST_LEVEL (arg_info)
                    + (unsigned int)NLUTgetNum (INFO_NLUT (arg_info), CUIDX_ID (indices)) - 1;
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

    /*
     * Finally, we need to fill in the corresponding entries for the
     * entid and constant to give;
     *
     *                i  j  k  e  cnst
     *   part_dim0    0  1  0  0   0
     *   part_dim1    0  0  1  0   0
     *   extid        0  0  0  1   0
     *   cnst         0  0  0  0   1
     */
    j = 0;
    while (j < rows - read_dim) {
        MatrixSetEntry (fas, cols - (rows - read_dim - j), j + read_dim, 1);
        j++;
    }

    DBUG_RETURN (fas);
}

/** <!--********************************************************************-->
 *
 * @fn node
 *
 * @brief
 *
 *
 *****************************************************************************/
static bool
CheckIntersection (IntMatrix constraints, IntMatrix write_fas, IntMatrix read_fas)
{
    bool res;
    FILE *matrix_file, *res_file;
    char buffer[MAXLINE];
    char polyhedral_filename[MAXLINE];
    char result_filename[MAXLINE];

    DBUG_ENTER ();

    count++;
    sprintf (polyhedral_filename, "%s%d.out", outfile, count);
    sprintf (result_filename, "%s%d.out", infile, count);

    matrix_file = FMGRwriteOpen (polyhedral_filename, "w");
    MatrixToFile (constraints, matrix_file);
    MatrixToFile (write_fas, matrix_file);
    MatrixToFile (read_fas, matrix_file);
    FMGRclose (matrix_file);

    SYScall (DLL_DIR "/sacprapolyhedral < %s > %s\n", polyhedral_filename,
             result_filename);

    res_file = FMGRreadOpen ("%s", result_filename);
    res = atoi (fgets (buffer, MAXLINE, res_file)) == 0 ? FALSE : TRUE;
    FMGRclose (res_file);

    // SYScall("rm -f *.out\n");

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief
 *
 *****************************************************************************/
static index_exprs_t *
ComputeElseCondition (index_exprs_t *ie)
{
    unsigned int i;
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

    DBUG_RETURN (new_ie);
}

/******************************************************************************
 *
 *
 * prefix: PRA
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *PRAdoPolyhedralReuseAnalysis( node *with, node *fundef)
 *
 * @brief
 *
 * @param with-loop to search for reusable arrays
 *
 * @return list of reuse candidates
 *
 *****************************************************************************/
node *
PRAdoPolyhedralReuseAnalysis (node *with, node *fundef)
{
    node *cand = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (with) == N_with, "Illegal node type!");

    if (((NODE_TYPE (WITH_WITHOP (with)) == N_genarray)
         || (NODE_TYPE (WITH_WITHOP (with)) == N_modarray))
        && (WITHOP_NEXT (WITH_WITHOP (with)) == NULL)) {
        node *hotpart = NULL;

        cand = RWOidentifyNoopArray (with);

        if (cand != NULL) {
            hotpart = RWOidentifyOtherPart (with, cand);

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

                /* Separte the partition and its code from the
                 * rest of the partitions/codes */
                hotcode = PART_CODE (hotpart);
                oldcodenext = CODE_NEXT (hotcode);
                CODE_NEXT (hotcode) = NULL;
                oldpartnext = PART_NEXT (hotpart);
                PART_NEXT (hotpart) = NULL;

                /* Start traversing the hot partition */
                TRAVpush (TR_pra);
                hotpart = TRAVdo (hotpart, arg_info);
                TRAVpop ();

                /* Restore the partition and code chain */
                CODE_NEXT (hotcode) = oldcodenext;
                PART_NEXT (hotpart) = oldpartnext;

                /*
                 * If at this point reuse candidate is not NULL,
                 * then it can actually be reused. We annotate
                 * copy partitions and noop conditional branch.
                 */
                if (INFO_RC (arg_info) != NULL) {
                    with = RWOannotateCopyPart (with, INFO_RC (arg_info));
                    cand = TBmakeExprs (INFO_RC (arg_info), NULL);
                    INFO_RC (arg_info) = NULL;
                    hotpart = INBdoIdentifyNoopBranch (hotpart);
                    WITH_HASRC (with) = TRUE;
                }

                /* Clean up */
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
 * @fn node *PRAwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
PRAwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * We do not traverse WITH_CODE here as the code will be traversed
     * when the partition is traversed. This avoids traversing the code
     * twice.
     */
    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRApart( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
PRApart (node *arg_node, info *arg_info)
{
    unsigned int dim, i;
    node *ids_iter, *ids, *lb, *ub;

    DBUG_ENTER ();

    ids = PART_IDS (arg_node);
    lb = PART_BOUND1 (arg_node);
    ub = PART_BOUND2 (arg_node);

    DBUG_ASSERT ((NODE_TYPE (lb) == N_array), "Lower bound is not an N_array!");
    DBUG_ASSERT ((NODE_TYPE (ub) == N_array), "Uower bound is not an N_array!");

    dim = (unsigned int)TCcountIds (ids);
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
            DFMsetMaskEntrySet (INFO_MASK (arg_info), IDS_AVIS (ids_iter));

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
                                     -COconst2IntAndFree (COaST2Constant (EXPRS_EXPR (lb))),
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
                                     COconst2IntAndFree (COaST2Constant (EXPRS_EXPR (ub))), NULL,
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
                ie = (index_exprs_t *)LUTsearchInLutPp (INFO_LUT (arg_info),
                                                        IDS_AVIS (ids));
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
 * @fn node *PRAassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
PRAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_MODE (arg_info) == TR_normal) {
        ASSIGN_LEVEL (arg_node) = INFO_NEST_LEVEL (arg_info);
        ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    } else if (INFO_MODE (arg_info) == TR_collect) {
        ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    } else {
        DBUG_UNREACHABLE ("Wrong traverse mode!");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRAcond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
PRAcond (node *arg_node, info *arg_info)
{
    node *old_condvar, *condvar, *ext_condvar, *ap;
    index_exprs_t *then_ie, *else_ie;

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

        /* Find the conditional variable in the calling context */
        ext_condvar = FindApargFromFunarg (FUNDEF_ARGS (AP_FUNDEF (ap)), AP_ARGS (ap),
                                           ARG_AVIS (ID_DECL (condvar)));
        DBUG_ASSERT ((ext_condvar != NULL), "External conditional variable is NULL!");

        /* Construct the predicate expressions and
         * see if they are affince */
        INFO_MODE (arg_info) = TR_collect;
        INFO_LACLEVEL (arg_info)--;
        INFO_IS_AFFINE (arg_info) = TRUE;
        ActOnId (ext_condvar, arg_info);
        INFO_LACLEVEL (arg_info)++;
        INFO_MODE (arg_info) = TR_normal;

        if (INFO_IS_AFFINE (arg_info)) {
            old_condvar = INFO_CONDVAR (arg_info);
            INFO_CONDVAR (arg_info) = condvar;

            then_ie = INFO_IE (arg_info);
            INFO_IE (arg_info) = NULL;

            /* Traverse 'then' branch */
            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), ID_AVIS (condvar), then_ie);

            COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);

            /* Traverse 'else' branch */
            else_ie = ComputeElseCondition (then_ie);
            INFO_LUT (arg_info)
              = LUTupdateLutP (INFO_LUT (arg_info), ID_AVIS (condvar), else_ie, NULL);

            COND_ELSE (arg_node) = TRAVopt (COND_ELSE (arg_node), arg_info);

            FreeIndexExprs (then_ie);
            FreeIndexExprs (else_ie);
            INFO_CONDVAR (arg_info) = old_condvar;
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
 * @fn node *PRAfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
PRAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (FUNDEF_ISCONDFUN (arg_node),
                 "Only conditional function can be traversed!");

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRAap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
PRAap (node *arg_node, info *arg_info)
{
    node *rc, *old_rc;

    DBUG_ENTER ();

    if (INFO_MODE (arg_info) == TR_normal) {
        if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node)) && INFO_RC (arg_info) != NULL) {
            /* Find the corresponding fundef argument of the reuse
             * candidate in the calling context */
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
        /* If we encounter a function application during TR_collect,
         * the current expression we are collecting is not affine */
        INFO_IS_AFFINE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRAprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
PRAprf (node *arg_node, info *arg_info)
{
    node *operand1, *operand2;
    int old_coefficient;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_sel_VxA:
    case F_idx_sel:
        if (INFO_MODE (arg_info) == TR_normal) {
            IntMatrix constraints, write_fas = NULL, read_fas = NULL;
            node *iv = PRF_ARG1 (arg_node);
            node *arr = PRF_ARG2 (arg_node);
            node *iv_ssaassign, *ids, *avis;
            unsigned int dim;
            bool valid = TRUE;

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

                /* It is only valid if the array subscript expression in
                 * each dimension is affine. */
                if (valid) {
                    unsigned int i, extids, nr_conds = 0;
                    unsigned int level,write_dim, read_dim;
                    bool intersected = FALSE;
                    index_exprs_t *ie = NULL;

                    /* Dimension of the accessed array */
                    read_dim = TYgetDim (ID_NTYPE (arr));
                    /* Dimension of the outermost enclising partition */
                    write_dim = INFO_WRITEDIM (arg_info);
                    /* Nesting level */
                    level = INFO_NEST_LEVEL (arg_info);
                    /* Number of external N_ids */
                    extids = INFO_NR_EXTIDS (arg_info);

                    /* For write fas and read fas, the number of columns is the same,
                     * namely the sum of the nesting level and the number of external
                     * N_ids (we need to add 1 to account for the 'constant dimension'.
                     * For write fas, the number of rows is the sum of the dimension
                     * of the outermost partition dimension and the number of external
                     * N_ids (we also need to add 1 to account for the 'constant
                     * dimension'). For read fas the number of rows is the sum of the
                     * dimension of the accessed array and the number of external N_ids
                     * (again 1 is added to account for the 'constant dimension'). Suppose
                     * a 2D array access (e.g. A[j,k]) is enclosed in two partitions, one
                     * [i,j] and the other [k] (essentially a 1D withloop enclosed inside
                     * a 2D withloop). Also assume there is one external N_id, e. The
                     * write matrix would look like (uninitialized):
                     *
                     *                i  j  k  e  cnst
                     *   part_dim0    0  0  0  0   0
                     *   part_dim1    0  0  0  0   0
                     *   extid        0  0  0  0   0
                     *   cnst         0  0  0  0   0
                     *
                     * The read matrix would look like (uninitialized):
                     *
                     *                i  j  k  e  cnst
                     *   arr_dim0     0  0  0  0   0
                     *   arr_dim1     0  0  0  0   0
                     *   extid        0  0  0  0   0
                     *   cnst         0  0  0  0   0
                     *
                     * Explaination on how these two matrice are initilized can be
                     * found in functions InitWrtieFas and InitReadFas.
                     */
                    write_fas = NewMatrix (level + extids + 1, write_dim + extids + 1);
                    read_fas = NewMatrix (level + extids + 1, read_dim + extids + 1);
                    write_fas = InitWriteFas (write_fas, write_dim, arg_info);
                    read_fas = InitReadFas (read_fas, read_dim, arr, arg_info);

                    /*
                     * We only need to construct the contstraint matrix if the write fas
                     * and read fas are not equal. If they are equal, this implies
                     * an inplace select/update. Therefore, there cannot be intersection.
                     */
                    if (!MatrixEqual (write_fas, read_fas)) {
                        logical_op_t cond_relation = LO_and;
                        /*
                         * If the array access is within a conditional (indicated by a
                         * non-NULL INFO_CONVAR), nr_conds stores the number of
                         * equalities/inequalities of this conditional.
                         */
                        if (INFO_CONDVAR (arg_info) != NULL) {
                            ie = (index_exprs_t *)
                              LUTsearchInLutPp (INFO_LUT (arg_info),
                                                ID_AVIS (INFO_CONDVAR (arg_info)));
                            DBUG_ASSERT (((node *)ie
                                          != ID_AVIS (INFO_CONDVAR (arg_info))),
                                         "Found condvar with null IE!");
                            nr_conds = IE_NR_ENTRIES (ie);
                            cond_relation = IE_LOP (ie);
                        }

                        /*
                         * For each equality or inequality, we construct a constraint
                         * matrix and compute the intersection with the images trasformed
                         * by write and read access matrix. Only all of them produce no
                         * intersection can we conclude that there is not intersection.
                         */
                        if (cond_relation == LO_and) {
                            /*
                             * If the logical relationship between all
                             * equlities/inequalities is 'AND', we only need to construct
                             * ONE contraint matrix, contains equlities and inequalities
                             * stemed from both withloop bounds and conditional
                             * predicates.
                             */
                            /* Columns */ /* Rows */
                            constraints
                              = NewMatrix (level + extids + 2, level * 2 + nr_conds);
                            constraints = InitConstraints (constraints, TRUE, NULL,
                                                           level * 2, 0, arg_info);
                            for (i = 0; i < nr_conds; i++) {
                                constraints = InitConstraints (constraints, FALSE, ie,
                                                               level * 2, i, arg_info);
                            }
                            intersected
                              = CheckIntersection (constraints, write_fas, read_fas);
                            FreeMatrix (constraints);
                        } else if (cond_relation == LO_or) {
                            /*
                             * If the logical relationship between all
                             * equlities/inequalities is 'OR', we need to construct one
                             * contraint matrix for each equalities/inequalities stemed
                             * from the conditional predicate. The equalities and
                             * inequalities stemed from the withloop bounds reamin the
                             * same. Only when all of them produce no intersection can we
                             * conclude that there is not intersection.
                             */
                            constraints = NewMatrix (level + extids + 2, level * 2 + 1);
                            constraints = InitConstraints (constraints, TRUE, NULL,
                                                           level * 2, 0, arg_info);
                            for (i = 0; i < nr_conds; i++) {
                                constraints = InitConstraints (constraints, FALSE, ie,
                                                               level * 2, i, arg_info);
                                intersected
                                  = (intersected
                                     || CheckIntersection (constraints, write_fas,
                                                           read_fas));
                                MatrixClearRow (constraints, level * 2);
                            }
                        } else if (cond_relation == LO_not) {
                            DBUG_UNREACHABLE (
                              "Logical operator 'not' is not supported yet!");
                        } else {
                            DBUG_UNREACHABLE ("Unknow logical operator found!");
                        }

                        if (intersected) {
                            INFO_RC (arg_info) = FREEdoFreeNode (INFO_RC (arg_info));
                            INFO_RC (arg_info) = NULL;
                        }
                    } else {
                        /* Equal write fas and read fas, in-place select/update */
                        PRF_ISINPLACESELECT (arg_node) = TRUE;
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
                            * COconst2IntAndFree (COaST2Constant (operand1)),
                          NULL, 0, INFO_DIM (arg_info), arg_info);
            } else {
                ActOnId (ID_AVIS (operand1), arg_info);
            }

            if (COisConstant (operand2)) {
                AddIndex (IDX_CONSTANT,
                          INFO_COEFFICIENT (arg_info)
                            * COconst2IntAndFree (COaST2Constant (operand2)),
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
                            * COconst2IntAndFree (COaST2Constant (operand1)),
                          NULL, 0, INFO_DIM (arg_info), arg_info);
            } else {
                ActOnId (ID_AVIS (operand1), arg_info);
            }

            if (COisConstant (operand2)) {
                old_coefficient = INFO_COEFFICIENT (arg_info);
                INFO_COEFFICIENT (arg_info) *= -1;
                AddIndex (IDX_CONSTANT,
                          INFO_COEFFICIENT (arg_info)
                            * COconst2IntAndFree (COaST2Constant (operand2)),
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
                INFO_COEFFICIENT (arg_info) *= COconst2IntAndFree (COaST2Constant (operand2));
                ActOnId (ID_AVIS (operand1), arg_info);
                INFO_COEFFICIENT (arg_info) = old_coefficient;
            } else if (COisConstant (operand1) && !COisConstant (operand2)) {
                old_coefficient = INFO_COEFFICIENT (arg_info);
                INFO_COEFFICIENT (arg_info) *= COconst2IntAndFree (COaST2Constant (operand1));
                ActOnId (ID_AVIS (operand2), arg_info);
                INFO_COEFFICIENT (arg_info) = old_coefficient;
            } else {
                AddIndex (IDX_CONSTANT,
                          INFO_COEFFICIENT (arg_info)
                            * COconst2IntAndFree (COaST2Constant (operand1))
                            * COconst2IntAndFree (COaST2Constant (operand2)),
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
                            * COconst2IntAndFree (COaST2Constant (operand1)),
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
                AddIndex (IDX_CONSTANT, COconst2IntAndFree (COaST2Constant (operand1)), NULL, 0,
                          INFO_DIM (arg_info), arg_info);
            } else {
                INFO_COEFFICIENT (arg_info) = 1;
                ActOnId (ID_AVIS (operand1), arg_info);
            }

            if (COisConstant (operand2)) {
                AddIndex (IDX_CONSTANT, -COconst2IntAndFree (COaST2Constant (operand2)), NULL, 0,
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
