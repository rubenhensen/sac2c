/*
 *
 * $Log$
 * Revision 1.35  2005/06/19 11:15:33  sah
 * modified insertion of suballoc
 *
 * Revision 1.34  2005/06/16 08:04:05  sbs
 * F_dispatch_error treated in the same way as F_type_error
 *
 * Revision 1.33  2005/03/17 19:37:43  sbs
 * EMALwith now can handle two parts as well.
 *
 * Revision 1.32  2005/03/13 15:39:15  ktr
 * Marielyst N_fold bug fixed
 *
 * Revision 1.31  2004/12/13 18:54:49  ktr
 * Withids contain N_id/N_exprs of N_id after explicit allocation now.
 *
 * Revision 1.30  2004/11/28 18:14:21  ktr
 * added traversal functions for bool, num, float, double, char
 *
 * Revision 1.29  2004/11/24 13:59:04  ktr
 * MakeLet permuted
 *
 * Revision 1.28  2004/11/23 22:12:35  ktr
 * renaming done.
 *
 * Revision 1.27  2004/11/23 17:32:58  ktr
 * COMPILES!!!
 *
 * Revision 1.26  2004/11/19 15:42:41  ktr
 * Support for F_alloc_or_reshape added.
 *
 * Revision 1.25  2004/11/09 22:16:06  ktr
 * Support for F_copy added.
 *
 * Revision 1.24  2004/09/21 17:27:59  ktr
 * Added support for F_idx_shape_sel, F_shape_sel
 *
 * Revision 1.23  2004/09/20 10:59:41  ktr
 * Minor bugfix...
 *
 * Revision 1.22  2004/09/20 09:45:55  ktr
 * Brushed use of new types while dealing with bug #56.
 *
 * Revision 1.21  2004/09/15 17:14:13  ktr
 * Ids2ALS is now static
 *
 * Revision 1.20  2004/09/15 17:07:14  ktr
 * code brushing after code audit with Karsten
 *
 * Revision 1.19  2004/09/06 13:53:56  ktr
 * Reordered alloc sequence.
 *
 * Revision 1.18  2004/08/27 08:18:23  ktr
 * No allocation is now done for F_type_error.
 * This caused bug #48 in alloc.c.
 *
 * Revision 1.17  2004/08/05 16:08:00  ktr
 * Scalar with-loops are now treated as they always were. By using the
 * F_wl_assign abstraction we can now explicitly refcount this case.
 *
 * Revision 1.16  2004/08/02 16:14:47  ktr
 * Memory allocation for with-loops is correct now.
 *
 * Revision 1.15  2004/07/31 21:26:11  ktr
 * corrected WL-shape descriptor for scalar withloops.
 *
 * Revision 1.14  2004/07/29 12:08:16  ktr
 * MakeDimArg, MakeShapeArg and MakeSizeArg added.
 * Constants must now only be filled if they are used on a RHS of an assignment
 *
 * Revision 1.13  2004/07/27 17:21:12  ktr
 * minor changes
 *
 * Revision 1.12  2004/07/26 18:45:53  ktr
 * - corrected some misused MakePrf instructions.
 * - changed representation of ND_CREATE__ARRAY__SHAPE
 *
 * Revision 1.11  2004/07/23 15:55:24  ktr
 * No NULL Assignment chain is appended to an empty block
 *
 * Revision 1.10  2004/07/23 11:51:58  ktr
 * changed IDX2OFFSET into IDXS2OFFSET
 *
 * Revision 1.9  2004/07/21 16:57:09  ktr
 * blah
 *
 * Revision 1.8  2004/07/20 13:47:08  ktr
 * Removed need for RedoSSATransform-.
 * Especially this means that only regular identifiers are built that must
 * be marked artificial before precompile.
 *
 * Revision 1.7  2004/07/19 14:53:38  ktr
 * Genarray-WL results are now markes ST_artificial such that they are
 * removed by precompile. This nicely reflects that genarray-wls are nothing
 * but fancy fill operations.
 *
 * Revision 1.6  2004/07/19 12:38:47  ktr
 * INFO structure is now initialized correctly.
 *
 * Revision 1.5  2004/07/17 10:26:04  ktr
 * EMAL now uses an INFO structure.
 *
 * Revision 1.4  2004/07/16 12:52:05  ktr
 * support for F_accu added.
 *
 * Revision 1.3  2004/07/16 12:07:14  ktr
 * EMAL now traverses into N_ap and N_funcond, too.
 *
 * Revision 1.2  2004/07/15 13:39:23  ktr
 * renamed EMALAllocateFill into EMAllocateFill
 *
 * Revision 1.1  2004/07/14 15:26:36  ktr
 * Initial revision
 *
 *
 */

/**
 * @defgroup emm Explicit Memory Management
 *
 * This group includes all the files needed by explicit memory management
 *
 * @{
 */

/**
 *
 * @file alloc.c
 *
 * SAC -> SAC-MemVal.
 *
 * much more text needed
 *
 */

#include "alloc.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "internal_lib.h"
#include "DupTree.h"
#include "ssa.h"
#include "new_types.h"
#include "print.h"
#include "free.h"
#include "shape.h"
#include "string.h"

/*
 * The following switch controls wheter AKS-Information should be used
 * when possible
 */
#define USEAKS

/** <!--******************************************************************-->
 *
 *  Enumeration of the different traversal modes for WITHOPs
 *
 ***************************************************************************/
typedef enum { EA_memname, EA_shape } ea_withopmode;

/** <!--******************************************************************-->
 *
 *  Structure used for ALLOCLIST.
 *
 ***************************************************************************/
typedef struct ALLOCLIST_STRUCT {
    node *avis;
    node *dim;
    node *shape;
    node *reshape;
    struct ALLOCLIST_STRUCT *next;
} alloclist_struct;

/**
 * INFO structure
 */
struct INFO {
    alloclist_struct *alloclist;
    node *fundef;
    node *withops;
    node *indexvector;
    bool mustfill;
    ea_withopmode withopmode;
};

/**
 * INFO macros
 */
#define INFO_EMAL_ALLOCLIST(n) ((n)->alloclist)
#define INFO_EMAL_FUNDEF(n) ((n)->fundef)
#define INFO_EMAL_WITHOPS(n) ((n)->withops)
#define INFO_EMAL_INDEXVECTOR(n) ((n)->indexvector)
#define INFO_EMAL_MUSTFILL(n) ((n)->mustfill)
#define INFO_EMAL_WITHOPMODE(n) ((n)->withopmode)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_EMAL_ALLOCLIST (result) = NULL;
    INFO_EMAL_FUNDEF (result) = NULL;
    INFO_EMAL_WITHOPS (result) = NULL;
    INFO_EMAL_INDEXVECTOR (result) = NULL;
    INFO_EMAL_MUSTFILL (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/**
 *
 *  ALLOCLIST FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn MakeALS
 *
 *  @brief creates an ALLOCLIST_STRUCT for allocation
 *
 *  @param ALLOCLIST
 *  @param avis
 *  @param dim
 *  @param shape
 *
 *  @return ALLOCLIST_STRUCT
 *
 ***************************************************************************/
static alloclist_struct *
MakeALS (alloclist_struct *als, node *avis, node *dim, node *shape)
{
    alloclist_struct *res;

    DBUG_ENTER ("MakeALS");

    res = ILIBmalloc (sizeof (alloclist_struct));

    res->avis = avis;
    res->dim = dim;
    res->shape = shape;
    res->reshape = NULL;
    res->next = als;

    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * @fn FreeALS
 *
 *  @brief frees all elements of the given ALLOCLIST
 *
 *  @param als ALLOCLIST
 *
 *  @return NULL
 *
 ***************************************************************************/
static alloclist_struct *
FreeALS (alloclist_struct *als)
{
    DBUG_ENTER ("FreeALS");

    if (als != NULL) {
        if (als->dim != NULL) {
            als->dim = FREEdoFreeTree (als->dim);
        }

        if (als->shape != NULL) {
            als->shape = FREEdoFreeTree (als->shape);
        }

        if (als->next != NULL) {
            als->next = FreeALS (als->next);
        }

        als = ILIBfree (als);
    }

    DBUG_RETURN (als);
}

/** <!--******************************************************************-->
 *
 * @fn Ids2ALS
 *
 *  @brief
 *
 *  @param ids
 *
 *  @return alloclist_struct
 *
 ***************************************************************************/
static alloclist_struct *
Ids2ALS (node *i)
{
    alloclist_struct *res;

    if (i == NULL) {
        res = NULL;
    } else {
        res = MakeALS (Ids2ALS (IDS_NEXT (i)), IDS_AVIS (i), NULL, NULL);
    }

    return (res);
}

/** <!--******************************************************************-->
 *
 * @fn AlloclistContains
 *
 *  @brief
 *
 *  @param ALLOCLIST
 *  @param avis
 *
 *  @return bool
 *
 ***************************************************************************/
static bool
AlloclistContains (alloclist_struct *als, node *avis)
{
    bool res;

    DBUG_ENTER ("AlloclistContains");

    if (als == NULL) {
        res = FALSE;
    } else {
        if (als->avis == avis) {
            res = TRUE;
        } else {
            res = AlloclistContains (als->next, avis);
        }
    }

    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * @fn MakeAllocAssignment
 *
 *  @brief converts an ALLOCLIST_STRUCT into an allocation assignment.
 *
 *  @param als
 *  @param next_node
 *
 *  @return An allocation assignment for the given als
 *
 ***************************************************************************/
static node *
MakeAllocAssignment (alloclist_struct *als, node *next_node)
{
    node *alloc;
    node *ids;

    DBUG_ENTER ("MakeAllocAssignment");

    ids = TBmakeIds (als->avis, NULL);

    DBUG_ASSERT (als->dim != NULL, "alloc requires a dim expression!");
    DBUG_ASSERT (als->shape != NULL, "alloc requires a shape expression!");

    /*
     * Annotate exact dim/shape information if available
     */
#ifdef USEAKS
    if ((TYisAKD (AVIS_TYPE (als->avis))) || (TYisAKS (AVIS_TYPE (als->avis)))
        || (TYisAKV (AVIS_TYPE (als->avis)))) {
        als->dim = FREEdoFreeTree (als->dim);
        als->dim = TBmakeNum (TYgetDim (AVIS_TYPE (als->avis)));
    }

    if ((TYisAKS (AVIS_TYPE (als->avis))) || (TYisAKV (AVIS_TYPE (als->avis)))) {
        als->shape = FREEdoFreeTree (als->shape);
        als->shape = SHshape2Array (TYgetShape (AVIS_TYPE (als->avis)));
    }
#endif

    if (als->reshape != NULL) {
        alloc = TCmakePrf3 (F_alloc_or_reshape, als->dim, als->shape, als->reshape);
        als->reshape = NULL;
    } else {
        alloc = TCmakePrf2 (F_alloc, als->dim, als->shape);
    }
    als->dim = NULL;
    als->shape = NULL;

    alloc = TBmakeAssign (TBmakeLet (ids, alloc), next_node);
    AVIS_SSAASSIGN (IDS_AVIS (ids)) = alloc;

    DBUG_RETURN (alloc);
}

/** <!--******************************************************************-->
 *
 * @fn MakeDimArg
 *
 *  @brief returns MakeNum( 0) for primitive data types and dim( a) for N_ids
 *
 *  @param arg
 *  @param
 *
 *  @return
 *
 ***************************************************************************/
static node *
MakeDimArg (node *arg)
{
    DBUG_ENTER ("MakeDimArg");

    switch (NODE_TYPE (arg)) {
    case N_num:
    case N_float:
    case N_double:
    case N_char:
    case N_bool:
        arg = TBmakeNum (0);
        break;

    case N_array:
        arg = TBmakeNum (SHgetDim (ARRAY_SHAPE (arg)));
        break;

    case N_id:
        arg = TCmakePrf1 (F_dim, DUPdoDupNode (arg));
        break;

    default:
        DBUG_EXECUTE ("EMAL", PRTdoPrintNode (arg););
        DBUG_ASSERT ((0), "Invalid argument");
    }

    DBUG_RETURN (arg);
}

/** <!--******************************************************************-->
 *
 * @fn MakeShapeArg
 *
 *  @brief returns [] for primitive data types and shape( a) for N_ids
 *
 *  @param arg
 *  @param
 *
 *  @return
 *
 ***************************************************************************/
static node *
MakeShapeArg (node *arg)
{
    DBUG_ENTER ("MakeShapeArg");

    switch (NODE_TYPE (arg)) {
    case N_num:
    case N_float:
    case N_double:
    case N_char:
    case N_bool:
        arg = TCcreateZeroVector (0, T_int);
        break;

    case N_array:
        arg = SHshape2Array (ARRAY_SHAPE (arg));
        break;

    case N_id:
        arg = TCmakePrf1 (F_shape, DUPdoDupNode (arg));
        break;

    default:
        DBUG_EXECUTE ("EMAL", PRTdoPrintNode (arg););
        DBUG_ASSERT ((0), "Invalid argument");
    }

    DBUG_RETURN (arg);
}

/** <!--******************************************************************-->
 *
 * @fn MakeSizeArg
 *
 *  @brief
 *
 *  @param arg
 *  @param
 *
 *  @return
 *
 ***************************************************************************/
static node *
MakeSizeArg (node *arg)
{
    DBUG_ENTER ("MakeSizeArg");

    switch (NODE_TYPE (arg)) {
    case N_num:
    case N_float:
    case N_double:
    case N_char:
    case N_bool:
        arg = TBmakeNum (1);
        break;

    case N_array:
        arg = TBmakeNum (SHgetUnrLen (ARRAY_SHAPE (arg)));
        break;

    case N_id:
        arg = TCmakePrf2 (F_sel, TBmakeNum (0), TCmakePrf1 (F_shape, DUPdoDupNode (arg)));
        break;

    default:
        DBUG_EXECUTE ("EMAL", PRTdoPrintNode (arg););
        DBUG_ASSERT ((0), "Invalid argument");
    }

    DBUG_RETURN (arg);
}

/**
 * @}
 */

/**
 *
 *  TRAVERSAL FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn EMALap
 *
 *  @brief removes all elements from ALLOCLIST.
 *
 *  Function applications return MemVals so we don't need to
 *  allocatate memory which must be filled.
 *
 *  @param arg_node the function application
 *  @param arg_info containing ALLOCLIST
 *
 *  @return arg_node (unmodified) and modified ALLOCLIST
 *
 ***************************************************************************/
node *
EMALap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMALap");

    INFO_EMAL_ALLOCLIST (arg_info) = FreeALS (INFO_EMAL_ALLOCLIST (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALarray
 *
 *  @brief updates ALLOCLIST with shape information about the given array.
 *
 *  shape information can be obtained from syntactic shape and element shape.
 *
 *  Ex. [[ a, b], [ c, d]]
 *  dim = 2 + dim(a)
 *  shape = shape( [[ a, b], [ c, d]])
 *
 *  shape calculation is extremely nasty here as for some reason
 *  CREATE_ARRAY_SHAPE requires the entire array.
 *
 *  @param arg_node an N_array node
 *  @param arg_info containing ALLOCLIST for this assignment
 *
 *  @return unmodified N_array node. ALLOCLIST has been updated, though.
 *
 ***************************************************************************/
node *
EMALarray (node *arg_node, info *arg_info)
{
    alloclist_struct *als;

    DBUG_ENTER ("EMALarray");

    als = INFO_EMAL_ALLOCLIST (arg_info);

    if (ARRAY_STRING (arg_node) != NULL) {
        /* array is a string */
        als->dim = TBmakeNum (1);
        als->shape = MakeShapeArg (arg_node);
    } else {
        if (ARRAY_AELEMS (arg_node) != NULL) {
            /*
             * [ a, ... ]
             * alloc( outer_dim + dim(a), shape( [a, ...]))
             */
            if (NODE_TYPE (ARRAY_AELEMS (arg_node)) == N_id) {
                als->dim = TCmakePrf2 (F_add_SxS, MakeDimArg (arg_node),
                                       MakeDimArg (EXPRS_EXPR (ARRAY_AELEMS (arg_node))));
            } else {
                als->dim = MakeDimArg (arg_node);
            }

            als->shape = TCmakePrf1 (F_shape, DUPdoDupTree (arg_node));
        } else {
            /*
             * []: empty array
             * alloc_or_reuse( 1, outer_dim, shape( []))
             *
             * The dimension of the right-hand-side is unknown
             *   -> A has to be a AKD array!
             */
            DBUG_ASSERT (/* AKD not allowed here! */
                         TYisAKS (AVIS_TYPE (als->avis))
                           || TYisAKV (AVIS_TYPE (als->avis)),
                         "assignment  A = [];  found, where A has unknown shape!");

            als->dim = TBmakeNum (TYgetDim (AVIS_TYPE (als->avis)));

            als->shape = TCmakePrf1 (F_shape, DUPdoDupTree (arg_node));
        }
    }

    /*
     * Signal EMALlet to wrap this RHS in a fill-operation
     */
    INFO_EMAL_MUSTFILL (arg_info) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALassign
 *
 *  @brief performs a bottom-up traversal and inserts alloc-statements.
 *
 *  Assignment get prepended with all the alloc-statements specified in
 *  ALLOCLIST.
 *
 *  @param arg_node N_assign node
 *  @param arg_info containing ALLOCLIST and COPYFILLLIST
 *
 *  @return assignmentchain with allocs and fills.
 *
 ***************************************************************************/
node *
EMALassign (node *arg_node, info *arg_info)
{
    alloclist_struct *als;

    DBUG_ENTER ("EMALassign");

    /*
     * Bottom-up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /*
     * Traverse RHS of assignment
     */
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    /*
     * Allocate missing variables
     */
    als = INFO_EMAL_ALLOCLIST (arg_info);
    while (als != NULL) {
        arg_node = MakeAllocAssignment (als, arg_node);
        als = als->next;
    }
    INFO_EMAL_ALLOCLIST (arg_info) = FreeALS (INFO_EMAL_ALLOCLIST (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALcode
 *
 *  @brief appends codeblocks with necessary suballoc/fill combinations
 *
 *  @param arg_node with-code
 *  @param arg_info
 *
 *  @return with-code appended with suballoc/fill combinations
 *
 ***************************************************************************/
node *
EMALcode (node *arg_node, info *arg_info)
{
    alloclist_struct *als;
    node *withops, *indexvector, *cexprs, *assign;
    node *memavis, *valavis, *cexavis;
    ntype *crestype;

    DBUG_ENTER ("EMALcode");

    /*
     * Rescue ALLOCLIST, WITHOPS and INDEXVECTOR
     */
    als = INFO_EMAL_ALLOCLIST (arg_info);
    INFO_EMAL_ALLOCLIST (arg_info) = NULL;
    withops = INFO_EMAL_WITHOPS (arg_info);
    INFO_EMAL_WITHOPS (arg_info) = NULL;
    indexvector = INFO_EMAL_INDEXVECTOR (arg_info);
    INFO_EMAL_INDEXVECTOR (arg_info) = NULL;

    /*
     * Traverse block
     */
    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    /*
     * Restore ALLOCLIST, WITHOPS and INDEXVECTOR
     */
    INFO_EMAL_ALLOCLIST (arg_info) = als;
    INFO_EMAL_WITHOPS (arg_info) = withops;
    INFO_EMAL_INDEXVECTOR (arg_info) = indexvector;

    /*
     * Shape information for each result array must be set and
     * suballoc/fill(copy,...) must be inserted
     */
    als = INFO_EMAL_ALLOCLIST (arg_info);
    withops = INFO_EMAL_WITHOPS (arg_info);
    cexprs = CODE_CEXPRS (arg_node);
    assign = NULL;

    while (withops != NULL) {
        DBUG_ASSERT (als != NULL, "ALLOCLIST must have an element for each WITHOP");
        DBUG_ASSERT (cexprs != NULL, "With-Loop must have as many results as ALLOCLIST");

        cexavis = ID_AVIS (EXPRS_EXPR (cexprs));

        /*
         * Set shape of genarray-wls if not already done and possible
         */
        if (NODE_TYPE (withops) == N_genarray) {

            if (als->dim == NULL) {
                if (TYisAKD (AVIS_TYPE (cexavis)) || TYisAKS (AVIS_TYPE (cexavis))
                    || TYisAKV (AVIS_TYPE (cexavis))) {
                    als->dim
                      = TCmakePrf2 (F_add_SxS, MakeSizeArg (GENARRAY_SHAPE (withops)),
                                    TBmakeNum (TYgetDim (AVIS_TYPE (cexavis))));
                }
            }
            if (als->shape == NULL) {
                if (TYisAKS (AVIS_TYPE (cexavis)) || TYisAKV (AVIS_TYPE (cexavis))) {
                    als->shape
                      = TCmakePrf2 (F_cat_VxV, DUPdoDupNode (GENARRAY_SHAPE (withops)),
                                    SHshape2Array (TYgetShape (AVIS_TYPE (cexavis))));
                }
            }
        }

        /*
         * Insert wl_assign prf for scalar with-loop results
         *
         * Insert suballoc/fill combinations for nonscalar with-loop results
         *
         * Ex:
         *   ...
         * }: a;
         */

        /*
         * when inserting suballocs, we have to make sure that the
         * shape class of the suballocated var matches the shape
         * class of the array the suballocation is performed
         * on. So we use the default expression here to find out the
         * shape class to use. If the result of the cexpr does not
         * fit within the default value, this will lead to a type
         * error during runtime!
         */
        if ((NODE_TYPE (withops) == N_genarray) && (GENARRAY_DEFAULT (withops) != NULL)) {
            DBUG_ASSERT ((NODE_TYPE (GENARRAY_DEFAULT (withops)) == N_id),
                         "found a non flattened default expression!");

            if (TYleTypes (AVIS_TYPE (cexavis),
                           AVIS_TYPE (ID_AVIS (GENARRAY_DEFAULT (withops))))) {
                crestype = AVIS_TYPE (cexavis);
            } else {
                crestype = AVIS_TYPE (ID_AVIS (GENARRAY_DEFAULT (withops)));
            }
        } else {
            crestype = AVIS_TYPE (cexavis);
        }

        if ((NODE_TYPE (withops) == N_genarray) || (NODE_TYPE (withops) == N_modarray)) {

            if ((TYisAKD (crestype) || TYisAKS (crestype) || TYisAKV (crestype))
                && (TYgetDim (crestype) == 0)) {
                /*
                 * Create a new value variable
                 * Ex: a_val
                 */
                valavis = TBmakeAvis (ILIBtmpVarName ("val"), TYcopyType (crestype));

                FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info))
                  = TBmakeVardec (valavis, FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));

                /*
                 * Create wl-assign operation
                 *
                 * Ex:
                 * {
                 *   ...
                 *   a_val = wl_assign( a, A, iv);
                 * }: a_val;
                 */
                assign
                  = TBmakeAssign (TBmakeLet (TBmakeIds (valavis, NULL),
                                             TCmakePrf3 (F_wl_assign, TBmakeId (cexavis),
                                                         TBmakeId (als->avis),
                                                         DUPdoDupNode (
                                                           INFO_EMAL_INDEXVECTOR (
                                                             arg_info)))),
                                  assign);
                AVIS_SSAASSIGN (valavis) = assign;

                /*
                 * Substitute cexpr
                 *
                 * Ex:
                 * {
                 *   ...
                 *   a_val = wl_assign( a, A, iv);
                 * }: a_val;
                 */
                EXPRS_EXPR (cexprs) = FREEdoFreeTree (EXPRS_EXPR (cexprs));
                EXPRS_EXPR (cexprs) = TBmakeId (valavis);
            } else {
                /*
                 * Create a new memory variable
                 * Ex: a_mem
                 */
                memavis = TBmakeAvis (ILIBtmpVarName ("mem"), TYeliminateAKV (crestype));

                FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info))
                  = TBmakeVardec (memavis, FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));

                /*
                 * Create a new value variable
                 * Ex: a_val
                 */
                valavis = TBmakeAvis (ILIBtmpVarName ("val"), TYcopyType (crestype));

                FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info))
                  = TBmakeVardec (valavis, FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));

                /*
                 * Create fill operation
                 *
                 * Ex:
                 * {
                 *   ...
                 *   a_val = fill( copy( a), a_mem);
                 * }: a;
                 */
                assign
                  = TBmakeAssign (TBmakeLet (TBmakeIds (valavis, NULL),
                                             TCmakePrf2 (F_fill,
                                                         TCmakePrf1 (F_copy,
                                                                     TBmakeId (cexavis)),
                                                         TBmakeId (memavis))),
                                  assign);
                AVIS_SSAASSIGN (valavis) = assign;

                /*
                 * Substitute cexpr
                 *
                 * Ex:
                 * {
                 *   ...
                 *   a_val = fill( copy( a), a_mem);
                 * }: a_val;
                 */
                EXPRS_EXPR (cexprs) = FREEdoFreeTree (EXPRS_EXPR (cexprs));
                EXPRS_EXPR (cexprs) = TBmakeId (valavis);

                /*
                 * Create suballoc assignment
                 *
                 * Ex:
                 * {
                 *   ...
                 *   a_mem = suballoc( A, iv);
                 *   a_val = fill( copy( a), a_mem);
                 * }: a_val;
                 */
                assign
                  = TBmakeAssign (TBmakeLet (TBmakeIds (memavis, NULL),
                                             TCmakePrf2 (F_suballoc, TBmakeId (als->avis),
                                                         DUPdoDupNode (
                                                           INFO_EMAL_INDEXVECTOR (
                                                             arg_info)))),
                                  assign);
                AVIS_SSAASSIGN (memavis) = assign;
            }
        }
        als = als->next;
        withops = WITHOP_NEXT (withops);
        cexprs = EXPRS_NEXT (cexprs);
    }

    if (assign != NULL) {
        BLOCK_INSTR (CODE_CBLOCK (arg_node))
          = TCappendAssign (BLOCK_INSTR (CODE_CBLOCK (arg_node)), assign);
    }

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALconst
 *
 *  @brief updates ALLOCLIST with shape information about the given constant
 *
 *  @param constant
 *  @param arg_info
 *
 *  @return constant
 *
 ***************************************************************************/
static node *
EMALconst (node *arg_node, info *arg_info)
{
    alloclist_struct *als;

    DBUG_ENTER ("EMALconst");

    als = INFO_EMAL_ALLOCLIST (arg_info);

    if (als != NULL) {
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);

        /*
         * Signal EMALlet to wrap this RHS in a fill operation
         */
        INFO_EMAL_MUSTFILL (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALbool
 *
 *  @brief
 *
 *  @param arg_node N_bool
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
EMALbool (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMALbool");

    arg_node = EMALconst (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALchar
 *
 *  @brief
 *
 *  @param arg_node N_char
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
EMALchar (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMALchar");

    arg_node = EMALconst (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALdouble
 *
 *  @brief
 *
 *  @param arg_node N_double
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
EMALdouble (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMALdouble");

    arg_node = EMALconst (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALfloat
 *
 *  @brief
 *
 *  @param arg_node N_float
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
EMALfloat (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMALfloat");

    arg_node = EMALconst (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALnum
 *
 *  @brief
 *
 *  @param arg_node N_num
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
EMALnum (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMALnum");

    arg_node = EMALconst (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALfuncond
 *
 *  @brief removes all elements from ALLOCLIST.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node (unmodified) and modified ALLOCLIST
 *
 ***************************************************************************/
node *
EMALfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMALfuncond");

    INFO_EMAL_ALLOCLIST (arg_info) = FreeALS (INFO_EMAL_ALLOCLIST (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALfundef
 *
 *  @brief traverses a fundef node by traversing the functions body.
 *         After that, SSA form is restored.
 *
 *  @param fundef
 *  @param arg_info
 *
 *  @return fundef
 *
 ***************************************************************************/
node *
EMALfundef (node *fundef, info *arg_info)
{
    DBUG_ENTER ("EMALfundef");

    INFO_EMAL_FUNDEF (arg_info) = fundef;

    /*
     * Traverse fundef body
     */
    if (FUNDEF_BODY (fundef) != NULL) {
        FUNDEF_BODY (fundef) = TRAVdo (FUNDEF_BODY (fundef), arg_info);
    }

    /*
     * Traverse other fundefs
     */
    if (FUNDEF_NEXT (fundef) != NULL) {
        FUNDEF_NEXT (fundef) = TRAVdo (FUNDEF_NEXT (fundef), arg_info);
    }

    DBUG_RETURN (fundef);
}

/** <!--******************************************************************-->
 *
 * @fn EMALicm
 *
 *  @brief allocates memory for the first argument of IVE-ICMs
 *
 *  @param arg_node
 *  @param arg_info containing ALLOCLIST
 *
 *  @return arg_node (unmodified) and modified ALLOCLIST
 *
 ***************************************************************************/
node *
EMALicm (node *arg_node, info *arg_info)
{
    char *name;

    DBUG_ENTER ("EMALicm");

    name = ICM_NAME (arg_node);
    if ((strstr (name, "USE_GENVAR_OFFSET") != NULL)
        || (strstr (name, "VECT2OFFSET") != NULL)
        || (strstr (name, "IDXS2OFFSET") != NULL)) {

        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info), ID_AVIS (ICM_ARG1 (arg_node)),
                     TBmakeNum (0), TCcreateZeroVector (0, T_int));
    } else {
        DBUG_ASSERT ((0), "Unknown ICM found during EMAL");
    }
    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALid
 *
 *  @brief removes all elements from ALLOCLIST.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node (unmodified) and modified ALLOCLIST
 *
 ***************************************************************************/
node *
EMALid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMALid");

    INFO_EMAL_ALLOCLIST (arg_info) = FreeALS (INFO_EMAL_ALLOCLIST (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALlet
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
EMALlet (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("EMALlet");

    /*
     * Put all LHS identifiers into ALLOCLIST
     */
    INFO_EMAL_ALLOCLIST (arg_info) = Ids2ALS (LET_IDS (arg_node));

    /*
     * Traverse RHS
     */
    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

        /*
         * Wrap RHS in Fill-operation if necessary
         */
        if (INFO_EMAL_MUSTFILL (arg_info)) {
            /*
             * a = b + c;
             *
             * is transformed into
             *
             * a' = alloc(...);
             * a = fill( b + c, a');
             */
            avis
              = TBmakeAvis (ILIBtmpVarName (IDS_NAME (LET_IDS (arg_node))),
                            TYeliminateAKV (AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)))));

            FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info))
              = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));

            LET_EXPR (arg_node)
              = TCmakePrf2 (F_fill, LET_EXPR (arg_node), TBmakeId (avis));

            /*
             * Set the avis of the freshly allocated variable
             */
            INFO_EMAL_ALLOCLIST (arg_info)->avis = avis;
        }
        INFO_EMAL_MUSTFILL (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALprf
 *
 *  @brief updates ALLOCLIST with shape information about the given prf
 *
 *  @param arg_node prf
 *  @param arg_info
 *
 *  @return prf
 *
 ***************************************************************************/
node *
EMALprf (node *arg_node, info *arg_info)
{
    node *new_node;
    alloclist_struct *als;

    DBUG_ENTER ("EMALprf");

    als = INFO_EMAL_ALLOCLIST (arg_info);

    /*
     * Signal EMALlet to wrap this prf in a fill-operation
     */
    INFO_EMAL_MUSTFILL (arg_info) = TRUE;

    switch (PRF_PRF (arg_node)) {
    case F_dim:
        /*
         * dim( A );
         * alloc( 0, [] );
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_shape:
        /*
         * shape( A );
         * alloc(1, shape( shape( A )))
         */
        als->dim = TBmakeNum (1);
        als->shape = TCmakePrf1 (F_shape, DUPdoDupTree (arg_node));
        break;

    case F_reshape:
        /*
         * reshape( sh, A );
         * alloc_or_reshape( shape( sh )[0], sh, A );
         * copy( A);
         */
        als->dim = MakeSizeArg (PRF_ARG1 (arg_node));
        als->shape = TCmakePrf1 (F_shape, DUPdoDupTree (arg_node));
        als->reshape = DUPdoDupNode (PRF_ARG2 (arg_node));

        new_node = TCmakePrf1 (F_copy, DUPdoDupNode (PRF_ARG2 (arg_node)));
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = new_node;
        break;

    case F_drop_SxV:
    case F_take_SxV:
    case F_cat_VxV:
        /*
         * drop( dv, V );
         * alloc( 1, shape( drop( dv, V )));
         * take( tv, V );
         * alloc( 1, shape( take( tv, V )));
         * cat( V1, V2 );
         * alloc( 1, shape( cat( V1, V2 )));
         */
        als->dim = TBmakeNum (1);
        als->shape = TCmakePrf1 (F_shape, DUPdoDupTree (arg_node));
        break;

    case F_sel:
        /*
         * sel( iv, A );
         * alloc( dim(A) - shape(iv)[0], shape( sel( iv, A)))
         */
        als->dim
          = TCmakePrf2 (F_sub_SxS, TCmakePrf1 (F_dim, DUPdoDupNode (PRF_ARG2 (arg_node))),
                        MakeSizeArg (PRF_ARG1 (arg_node)));

        als->shape = TCmakePrf1 (F_shape, DUPdoDupNode (arg_node));
        break;

    case F_idx_sel:
        /*
         * idx_sel can only occur when the result shape is known!!!
         *
         * a = idx_sel( idx, A);
         * alloc( dim( a), shape( idx_sel( idx, A)));
         */
        DBUG_ASSERT (/* No AKDs allowed here! */
                     TYisAKS (AVIS_TYPE (als->avis)) || TYisAKV (AVIS_TYPE (als->avis)),
                     "idx_sel with unknown result shape found!");
        als->dim = TBmakeNum (TYgetDim (AVIS_TYPE (als->avis)));
        als->shape = TCmakePrf1 (F_shape, DUPdoDupNode (arg_node));
        break;

    case F_modarray:
    case F_idx_modarray:
        /*
         * idx_modarray( A, idx, val);
         * alloc( dim( A ), shape ( A ));
         */
        als->dim = MakeDimArg (PRF_ARG1 (arg_node));
        als->shape = MakeShapeArg (PRF_ARG1 (arg_node));
        break;

    case F_idx_shape_sel:
    case F_shape_sel:
        /*
         * shape_sel always yields a scalar
         *
         * a = shape_sel( idx, A)
         * alloc( 0, []);
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_add_SxS:
    case F_sub_SxS:
    case F_mul_SxS:
    case F_div_SxS:
    case F_toi_S:
    case F_tof_S:
    case F_tod_S:
        /*
         * single scalar operations
         * alloc( 0, [])
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_mod:
    case F_min:
    case F_max:
    case F_neg:
    case F_not:
    case F_abs:
    case F_and:
    case F_or:
    case F_le:
    case F_lt:
    case F_eq:
    case F_neq:
    case F_ge:
    case F_gt:
    case F_toi_A:
    case F_tof_A:
    case F_tod_A:
    case F_add_AxS:
    case F_sub_AxS:
    case F_mul_AxS:
    case F_div_AxS:
    case F_add_AxA:
    case F_mul_AxA:
    case F_sub_AxA:
    case F_div_AxA:
    case F_add_SxA:
    case F_sub_SxA:
    case F_mul_SxA:
    case F_div_SxA:
    case F_copy:
        if ((TCcountExprs (PRF_ARGS (arg_node)) < 2)
            || (NODE_TYPE (PRF_ARG2 (arg_node)) != N_id)
            || ((TYisAKD (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node))))
                 || TYisAKS (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node))))
                 || TYisAKV (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node)))))
                && (TYgetDim (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node)))) == 0))) {
            als->dim = MakeDimArg (PRF_ARG1 (arg_node));
            als->shape = MakeShapeArg (PRF_ARG1 (arg_node));
        } else {
            als->dim = MakeDimArg (PRF_ARG2 (arg_node));
            als->shape = MakeShapeArg (PRF_ARG2 (arg_node));
        }
        break;

    case F_accu:
        /*
         * a,... = accu( iv, n, ...)
         * accu requires a special treatment as
         * none of its return values must be allocated
         */
        INFO_EMAL_ALLOCLIST (arg_info) = FreeALS (INFO_EMAL_ALLOCLIST (arg_info));

        INFO_EMAL_MUSTFILL (arg_info) = FALSE;
        break;

    case F_type_error:
    case F_dispatch_error:
        /*
         * v,... = _type_error_( ...)
         * _type_error_ requires a special treatment as
         * none of its return value must llocated
         */
        INFO_EMAL_ALLOCLIST (arg_info) = FreeALS (INFO_EMAL_ALLOCLIST (arg_info));

        INFO_EMAL_MUSTFILL (arg_info) = FALSE;
        break;

    case F_alloc:
    case F_suballoc:
    case F_fill:
    case F_wl_assign:
    case F_alloc_or_reuse:
    case F_inc_rc:
    case F_dec_rc:
    case F_free:
    case F_to_unq:
    case F_from_unq:
        DBUG_ASSERT ((0), "invalid prf found!");
        break;

    case F_take:
    case F_drop:
    case F_cat:
    case F_rotate:
    case F_genarray:
        DBUG_ASSERT ((0), "Non-instrinsic primitive functions not implemented!"
                          " Use array.lib instead!");
        break;

    default:
        DBUG_EXECUTE ("EMAL", PRTdoPrintNode (arg_node););
        DBUG_ASSERT ((0), "unknown prf found!");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALwith
 *
 *  @brief
 *
 *  @param arg_node with-loop
 *  @param arg_info
 *
 *  @return with-loop
 *
 ***************************************************************************/
node *
EMALwith (node *arg_node, info *arg_info)
{
    node *expr;
    node *ids;

    DBUG_ENTER ("EMALwith");

    /*
     * ALLOCLIST is needed to traverse CODEs and will be rescued there
     */

    /*
     * Annoate destination memory by traversing WITHOPS
     */
    INFO_EMAL_WITHOPMODE (arg_info) = EA_memname;
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /*
     * In order to build a proper suballoc/fill combinations in each Code-Block
     * it is necessary to know which result variables refer to
     * genarray/modarray - withops
     * Furthermore, a code template for the index vector is needed
     */
    INFO_EMAL_WITHOPS (arg_info) = WITH_WITHOP (arg_node);
    INFO_EMAL_INDEXVECTOR (arg_info) = TBmakeId (IDS_AVIS (WITH_VEC (arg_node)));

    /*
     * Traverse codes
     */
    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    /*
     * Free INDEXVECTOR code template
     */
    INFO_EMAL_INDEXVECTOR (arg_info) = FREEdoFreeTree (INFO_EMAL_INDEXVECTOR (arg_info));

    /*
     * Rebuild ALLOCLIST by traversing WITHOPS
     */
    INFO_EMAL_WITHOPMODE (arg_info) = EA_shape;
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /*
     * Allocate memory for the index vector
     * and replace WITH_VEC ids with id
     */
    if (WITH_VEC (arg_node) != NULL) {
        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info), IDS_AVIS (WITH_VEC (arg_node)),
                     TBmakeNum (1), MakeShapeArg (WITH_BOUND1 (arg_node)));

        expr = TBmakeId (IDS_AVIS (WITH_VEC (arg_node)));
        WITH_VEC (arg_node) = FREEdoFreeTree (WITH_VEC (arg_node));
        WITH_VEC (arg_node) = expr;
        if (PART_NEXT (WITH_PART (arg_node)) != NULL) {
            expr = TBmakeId (IDS_AVIS (PART_VEC (PART_NEXT (WITH_PART (arg_node)))));
            PART_VEC (PART_NEXT (WITH_PART (arg_node)))
              = FREEdoFreeTree (PART_VEC (PART_NEXT (WITH_PART (arg_node))));
            PART_VEC (PART_NEXT (WITH_PART (arg_node))) = expr;
        }
    }

    /*
     * Allocate memory for the index variables
     * and replace WITH_IDS with exprs chain of id.
     */
    expr = NULL;
    ids = WITH_IDS (arg_node);
    while (ids != NULL) {
        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info), IDS_AVIS (ids), TBmakeNum (0),
                     TCcreateZeroVector (0, T_int));

        expr = TCappendExprs (expr, TBmakeExprs (TBmakeId (IDS_AVIS (ids)), NULL));
        ids = IDS_NEXT (ids);
    }

    if (WITH_IDS (arg_node) != NULL) {
        WITH_IDS (arg_node) = FREEdoFreeTree (WITH_IDS (arg_node));
        WITH_IDS (arg_node) = expr;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALwith2
 *
 *  @brief
 *
 *  @param arg_node with-loop2
 *  @param arg_info
 *
 *  @return with-loop2
 *
 ***************************************************************************/
node *
EMALwith2 (node *arg_node, info *arg_info)
{
    node *expr;
    node *ids;

    DBUG_ENTER ("EMALwith2");

    /*
     * Annoate destination memory by traversing WITHOPS
     */
    INFO_EMAL_WITHOPMODE (arg_info) = EA_memname;
    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    /*
     * In order to build a proper suballoc/fill combinations in each Code-Block
     * it is necessary to know which result variables refer to
     * genarray/modarray - withops
     * Furthermore, a code template for the index vector is needed
     */
    INFO_EMAL_WITHOPS (arg_info) = WITH2_WITHOP (arg_node);
    INFO_EMAL_INDEXVECTOR (arg_info) = TBmakeId (IDS_AVIS (WITH2_VEC (arg_node)));

    /*
     * Traverse codes
     */
    if (WITH2_CODE (arg_node) != NULL) {
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    }

    /*
     * Free INDEXVECTOR code template
     */
    INFO_EMAL_INDEXVECTOR (arg_info) = FREEdoFreeTree (INFO_EMAL_INDEXVECTOR (arg_info));

    /*
     * Rebuild ALLOCLIST by traversing WITHOPS
     */
    INFO_EMAL_WITHOPMODE (arg_info) = EA_shape;
    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    /*
     * Allocate memory for the index vector
     * and replace WITH2_VEC ids with id
     *
     * In Nwith2, shape of the index vector is always known!
     */
    if (WITH2_VEC (arg_node) != NULL) {
        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info), IDS_AVIS (WITH2_VEC (arg_node)),
                     TBmakeNum (1),
                     SHshape2Array (TYgetShape (
                       AVIS_TYPE (IDS_AVIS (WITHID_VEC (WITH2_WITHID (arg_node)))))));

        expr = TBmakeId (IDS_AVIS (WITH2_VEC (arg_node)));
        WITH2_VEC (arg_node) = FREEdoFreeTree (WITH2_VEC (arg_node));
        WITH2_VEC (arg_node) = expr;
    }

    /*
     * Allocate memory for the index variables
     * and replace WITH_IDS with exprs chain of id.
     */
    expr = NULL;
    ids = WITH2_IDS (arg_node);
    while (ids != NULL) {
        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info), IDS_AVIS (ids), TBmakeNum (0),
                     TCcreateZeroVector (0, T_int));

        expr = TCappendExprs (expr, TBmakeExprs (TBmakeId (IDS_AVIS (ids)), NULL));
        ids = IDS_NEXT (ids);
    }

    if (WITH2_IDS (arg_node) != NULL) {
        WITH2_IDS (arg_node) = FREEdoFreeTree (WITH2_IDS (arg_node));
        WITH2_IDS (arg_node) = expr;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALgenarray
 *
 *  @brief
 *
 *  @param arg_node with-loop
 *  @param arg_info
 *
 *  @return with-loop
 *
 ***************************************************************************/
node *
EMALgenarray (node *arg_node, info *arg_info)
{
    node *wlavis;
    alloclist_struct *als;

    DBUG_ENTER ("EMALgenarray");

    DBUG_ASSERT (INFO_EMAL_ALLOCLIST (arg_info) != NULL,
                 "ALLOCLIST must contain an entry for each WITHOP!");

    /*
     * Pop first element from alloclist for traversal of next WITHOP
     */
    als = INFO_EMAL_ALLOCLIST (arg_info);
    INFO_EMAL_ALLOCLIST (arg_info) = als->next;
    als->next = NULL;

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    if (INFO_EMAL_WITHOPMODE (arg_info) == EA_memname) {

        /*
         * Create new identifier for new memory
         */
        wlavis = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (als->avis)),
                             TYeliminateAKV (AVIS_TYPE (als->avis)));

        FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info))
          = TBmakeVardec (wlavis, FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));

        als->avis = wlavis;

        /*
         * Annotate which memory is to be used
         */
        GENARRAY_MEM (arg_node) = TBmakeId (wlavis);

        /*
         * Restore first element of alloclist as it is needed in EMALcode
         * to preserve correspondence between the result values and the withops
         */
        als->next = INFO_EMAL_ALLOCLIST (arg_info);
        INFO_EMAL_ALLOCLIST (arg_info) = als;
    } else {
        DBUG_ASSERT (INFO_EMAL_WITHOPMODE (arg_info) == EA_shape,
                     "Unknown Withop traversal mode");

        /*
         * If shape information has not yet been gathered it must be
         * inferred using the default element
         */
        if (als->dim == NULL) {
            DBUG_ASSERT (GENARRAY_DEFAULT (arg_node) != NULL,
                         "Default element required!");
            als->dim = TCmakePrf2 (F_add_SxS, MakeSizeArg (GENARRAY_SHAPE (arg_node)),
                                   MakeDimArg (GENARRAY_DEFAULT (arg_node)));
        }

        if (als->shape == NULL) {
            DBUG_ASSERT (GENARRAY_DEFAULT (arg_node) != NULL,
                         "Default element required!");
            als->shape
              = TCmakePrf1 (F_shape,
                            TCmakePrf2 (F_genarray,
                                        DUPdoDupNode (GENARRAY_SHAPE (arg_node)),
                                        DUPdoDupNode (GENARRAY_DEFAULT (arg_node))));
        }

        /*
         * genarray-wl:
         * Allocation must remain in ALLOCLIST
         */
        als->next = INFO_EMAL_ALLOCLIST (arg_info);
        INFO_EMAL_ALLOCLIST (arg_info) = als;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALmodarray
 *
 *  @brief
 *
 *  @param arg_node with-loop
 *  @param arg_info
 *
 *  @return with-loop
 *
 ***************************************************************************/
node *
EMALmodarray (node *arg_node, info *arg_info)
{
    node *wlavis;
    alloclist_struct *als;

    DBUG_ENTER ("EMALwithop");

    DBUG_ASSERT (INFO_EMAL_ALLOCLIST (arg_info) != NULL,
                 "ALLOCLIST must contain an entry for each WITHOP!");

    /*
     * Pop first element from alloclist for traversal of next WITHOP
     */
    als = INFO_EMAL_ALLOCLIST (arg_info);
    INFO_EMAL_ALLOCLIST (arg_info) = als->next;
    als->next = NULL;

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    if (INFO_EMAL_WITHOPMODE (arg_info) == EA_memname) {

        /*
         * Create new identifier for new memory
         */
        wlavis = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (als->avis)),
                             TYeliminateAKV (AVIS_TYPE (als->avis)));

        FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info))
          = TBmakeVardec (wlavis, FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));

        als->avis = wlavis;

        /*
         * Annotate which memory is to be used
         */
        MODARRAY_MEM (arg_node) = TBmakeId (wlavis);

        /*
         * Restore first element of alloclist as it is needed in EMALcode
         * to preserve correspondence between the result values and the withops
         */
        als->next = INFO_EMAL_ALLOCLIST (arg_info);
        INFO_EMAL_ALLOCLIST (arg_info) = als;
    } else {
        DBUG_ASSERT (INFO_EMAL_WITHOPMODE (arg_info) == EA_shape,
                     "Unknown Withop traversal mode");
        /*
         * modarray-wl:
         * dim, shape are the same as in the modified array
         */
        als->dim = TCmakePrf1 (F_dim, DUPdoDupNode (MODARRAY_ARRAY (arg_node)));
        als->shape = TCmakePrf1 (F_shape, DUPdoDupNode (MODARRAY_ARRAY (arg_node)));

        /*
         * modarray-wl:
         * Allocation must remain in ALLOCLIST
         */
        als->next = INFO_EMAL_ALLOCLIST (arg_info);
        INFO_EMAL_ALLOCLIST (arg_info) = als;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALfold
 *
 *  @brief
 *
 *  @param arg_node with-loop
 *  @param arg_info
 *
 *  @return with-loop
 *
 ***************************************************************************/
node *
EMALfold (node *arg_node, info *arg_info)
{
    alloclist_struct *als;

    DBUG_ENTER ("EMALfold");

    DBUG_ASSERT (INFO_EMAL_ALLOCLIST (arg_info) != NULL,
                 "ALLOCLIST must contain an entry for each WITHOP!");

    /*
     * Pop first element from alloclist for traversal of next WITHOP
     */
    als = INFO_EMAL_ALLOCLIST (arg_info);
    INFO_EMAL_ALLOCLIST (arg_info) = als->next;
    als->next = NULL;

    if (FOLD_NEXT (arg_node) != NULL) {
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    if (INFO_EMAL_WITHOPMODE (arg_info) == EA_memname) {
        /*
         * Restore first element of alloclist as it is still needed in EMALcode
         * to preserve correspondence between the result values and the withops
         */
        als->next = INFO_EMAL_ALLOCLIST (arg_info);
        INFO_EMAL_ALLOCLIST (arg_info) = als;
    } else {
        DBUG_ASSERT (INFO_EMAL_WITHOPMODE (arg_info) == EA_shape,
                     "Unknown Withop traversal mode");
        /*
         * Ultimately, fold-wls allocate memory on their own and thus don't need
         * further storage
         */
        als = FreeALS (als);
    }

    DBUG_RETURN (arg_node);
}
/**
 * @}
 */

/** <!--******************************************************************-->
 *
 * @fn EMALdoAlloc
 *
 *  @brief Starting function of transformation SAC -> SAC-MemVal.
 *
 *  @param syntax_tree
 *
 *  @return modified syntax tree containing explicit alloc and fill prfs
 *
 ***************************************************************************/
node *
EMALdoAlloc (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("EMALdoAlloc");

    info = MakeInfo ();

    TRAVpush (TR_emal);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/**
 * @}
 */
