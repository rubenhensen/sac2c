/*
 *
 * $Log$
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
#define NEW_INFO

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "internal_lib.h"
#include "DupTree.h"
#include "ssa.h"
#include "new_types.h"
#include "alloc.h"
#include "print.h"

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
typedef enum { ea_memname, ea_shape } ea_withopmode;

/** <!--******************************************************************-->
 *
 *  Structure used for ALLOCLIST.
 *
 ***************************************************************************/
typedef struct ALLOCLIST_STRUCT {
    node *avis;
    node *dim;
    node *shape;
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
#define INFO_EMAL_ALLOCLIST(n) (n->alloclist)
#define INFO_EMAL_FUNDEF(n) (n->fundef)
#define INFO_EMAL_WITHOPS(n) (n->withops)
#define INFO_EMAL_INDEXVECTOR(n) (n->indexvector)
#define INFO_EMAL_MUSTFILL(n) (n->mustfill)
#define INFO_EMAL_WITHOPMODE(n) (n->withopmode)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

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

    info = Free (info);

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

    res = Malloc (sizeof (alloclist_struct));

    res->avis = avis;
    res->dim = dim;
    res->shape = shape;
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
            als->dim = FreeTree (als->dim);
        }

        if (als->shape != NULL) {
            als->shape = FreeTree (als->shape);
        }

        if (als->next != NULL) {
            als->next = FreeALS (als->next);
        }

        als = Free (als);
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
Ids2ALS (ids *i)
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
    ids *ids;

    DBUG_ENTER ("MakeAllocAssignment");

    ids = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (als->avis))), NULL,
                   ST_regular);
    IDS_AVIS (ids) = als->avis;
    IDS_VARDEC (ids) = AVIS_VARDECORARG (als->avis);

    DBUG_ASSERT (als->dim != NULL, "alloc requires a dim expression!");
    DBUG_ASSERT (als->shape != NULL, "alloc requires a shape expression!");

    /*
     * Annotate exact dim/shape information if available
     */
#ifdef USEAKS
    if ((TYIsAKD (AVIS_TYPE (als->avis))) || (TYIsAKS (AVIS_TYPE (als->avis)))
        || (TYIsAKV (AVIS_TYPE (als->avis)))) {
        als->dim = FreeTree (als->dim);
        als->dim = MakeNum (TYGetDim (AVIS_TYPE (als->avis)));
    }

    if ((TYIsAKS (AVIS_TYPE (als->avis))) || (TYIsAKV (AVIS_TYPE (als->avis)))) {
        als->shape = FreeTree (als->shape);
        als->shape = SHShape2Array (TYGetShape (AVIS_TYPE (als->avis)));
    }
#endif

    alloc = MakePrf2 (F_alloc, als->dim, als->shape);

    als->dim = NULL;
    als->shape = NULL;

    alloc = MakeAssign (MakeLet (alloc, ids), next_node);
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
        arg = MakeNum (0);
        break;

    case N_array:
        arg = MakeNum (SHGetDim (ARRAY_SHAPE (arg)));
        break;

    case N_id:
        arg = MakePrf1 (F_dim, DupNode (arg));
        break;

    default:
        DBUG_EXECUTE ("EMAL", PrintNode (arg););
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
        arg = CreateZeroVector (0, T_int);
        break;

    case N_array:
        arg = SHShape2Array (ARRAY_SHAPE (arg));
        break;

    case N_id:
        arg = MakePrf1 (F_shape, DupNode (arg));
        break;

    default:
        DBUG_EXECUTE ("EMAL", PrintNode (arg););
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
        arg = MakeNum (1);
        break;

    case N_array:
        arg = MakeNum (SHGetUnrLen (ARRAY_SHAPE (arg)));
        break;

    case N_id:
        arg = MakePrf2 (F_sel, MakeNum (0), MakePrf1 (F_shape, DupNode (arg)));
        break;

    default:
        DBUG_EXECUTE ("EMAL", PrintNode (arg););
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
        als->dim = MakeNum (1);
        als->shape = MakeShapeArg (arg_node);
    } else {
        if (ARRAY_AELEMS (arg_node) != NULL) {
            /*
             * [ a, ... ]
             * alloc( outer_dim + dim(a), shape( [a, ...]))
             */
            if (NODE_TYPE (ARRAY_AELEMS (arg_node)) == N_id) {
                als->dim = MakePrf2 (F_add_SxS, MakeDimArg (arg_node),
                                     MakeDimArg (EXPRS_EXPR (ARRAY_AELEMS (arg_node))));
            } else {
                als->dim = MakeDimArg (arg_node);
            }

            als->shape = MakePrf1 (F_shape, DupTree (arg_node));
        } else {
            /*
             * []: empty array
             * alloc_or_reuse( 1, outer_dim, shape( []))
             *
             * The dimension of the right-hand-side is unknown
             *   -> A has to be a AKD array!
             */
            DBUG_ASSERT (/* AKD not allowed here! */
                         TYIsAKS (AVIS_TYPE (als->avis))
                           || TYIsAKV (AVIS_TYPE (als->avis)),
                         "assignment  A = [];  found, where A has unknown shape!");

            als->dim = MakeNum (TYGetDim (AVIS_TYPE (als->avis)));

            als->shape = MakePrf1 (F_shape, DupTree (arg_node));
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
     * Bottom-up travseral
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    /*
     * Traverse RHS of assignment
     */
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
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
    ids *lhs, *arg1, *arg2;
    alloclist_struct *als;
    node *withops, *indexvector, *cexprs, *assign;
    node *memavis, *valavis, *cexavis;

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
    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
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
    cexprs = NCODE_CEXPRS (arg_node);
    assign = NULL;

    while (withops != NULL) {
        DBUG_ASSERT (als != NULL, "ALLOCLIST must have an element for each WITHOP");
        DBUG_ASSERT (cexprs != NULL, "With-Loop must have as many results as ALLOCLIST");

        cexavis = ID_AVIS (EXPRS_EXPR (cexprs));

        /*
         * Set shape of genarray-wls if not already done and possible
         */
        if (NWITHOP_TYPE (withops) == WO_genarray) {

            if (als->dim == NULL) {
                if (TYIsAKD (AVIS_TYPE (cexavis)) || TYIsAKS (AVIS_TYPE (cexavis))
                    || TYIsAKV (AVIS_TYPE (cexavis))) {
                    als->dim = MakePrf2 (F_add_SxS, MakeSizeArg (NWITHOP_SHAPE (withops)),
                                         MakeNum (TYGetDim (AVIS_TYPE (cexavis))));
                }
            }
            if (als->shape == NULL) {
                if (TYIsAKS (AVIS_TYPE (cexavis)) || TYIsAKV (AVIS_TYPE (cexavis))) {
                    als->shape
                      = MakePrf2 (F_cat_VxV, DupNode (NWITHOP_SHAPE (withops)),
                                  SHShape2Array (TYGetShape (AVIS_TYPE (cexavis))));
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
        if ((NWITHOP_TYPE (withops) == WO_genarray)
            || (NWITHOP_TYPE (withops) == WO_modarray)) {

            if ((TYIsAKD (AVIS_TYPE (cexavis)) || TYIsAKS (AVIS_TYPE (cexavis))
                 || TYIsAKV (AVIS_TYPE (cexavis)))
                && (TYGetDim (AVIS_TYPE (cexavis)) == 0)) {
                /*
                 * Create a new value variable
                 * Ex: a_val
                 */
                FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info))
                  = MakeVardec (TmpVarName ("val"),
                                DupOneTypes (VARDEC_TYPE (AVIS_VARDECORARG (cexavis))),
                                FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));

                valavis = VARDEC_AVIS (FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));
                AVIS_TYPE (valavis) = TYCopyType (AVIS_TYPE (cexavis));

                /*
                 * Create wl-assign operation
                 *
                 * Ex:
                 *   ...
                 *   a_val = wl_assign( a, A, iv);
                 * }: a;
                 */
                lhs = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (valavis))),
                               NULL, ST_regular);
                IDS_AVIS (lhs) = valavis;
                IDS_VARDEC (lhs) = AVIS_VARDECORARG (valavis);

                arg1 = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (cexavis))),
                                NULL, ST_regular);
                IDS_AVIS (arg1) = cexavis;
                IDS_VARDEC (arg1) = AVIS_VARDECORARG (cexavis);

                arg2 = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (als->avis))),
                                NULL, ST_regular);
                IDS_AVIS (arg2) = als->avis;
                IDS_VARDEC (arg2) = AVIS_VARDECORARG (als->avis);

                assign = MakeAssign (MakeLet (MakePrf3 (F_wl_assign, MakeIdFromIds (arg1),
                                                        MakeIdFromIds (arg2),
                                                        DupTree (INFO_EMAL_INDEXVECTOR (
                                                          arg_info))),
                                              lhs),
                                     assign);
                AVIS_SSAASSIGN (IDS_AVIS (lhs)) = assign;

                /*
                 * Substitute cexpr
                 *
                 * Ex:
                 *   ...
                 *   a_val = wl_assign( a, A, iv);
                 * }: a_val;
                 */
                EXPRS_EXPR (cexprs) = FreeTree (EXPRS_EXPR (cexprs));
                EXPRS_EXPR (cexprs) = MakeIdFromIds (DupOneIds (lhs));
            } else {
                /*
                 * Create a new memory variable
                 * Ex: a_mem
                 */
                FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info))
                  = MakeVardec (TmpVarName ("mem"),
                                DupOneTypes (VARDEC_TYPE (AVIS_VARDECORARG (cexavis))),
                                FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));

                memavis = VARDEC_AVIS (FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));
                AVIS_TYPE (memavis) = TYCopyType (AVIS_TYPE (cexavis));

                /*
                 * Create a new value variable
                 * Ex: a_val
                 */
                FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info))
                  = MakeVardec (TmpVarName ("val"),
                                DupOneTypes (VARDEC_TYPE (AVIS_VARDECORARG (cexavis))),
                                FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));

                valavis = VARDEC_AVIS (FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));
                AVIS_TYPE (valavis) = TYCopyType (AVIS_TYPE (cexavis));

                /*
                 * Create fill operation
                 *
                 * Ex:
                 *   ...
                 *   a_val = fill( copy( a), a_mem);
                 * }: a;
                 */
                lhs = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (valavis))),
                               NULL, ST_regular);
                IDS_AVIS (lhs) = valavis;
                IDS_VARDEC (lhs) = AVIS_VARDECORARG (valavis);

                arg1 = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (cexavis))),
                                NULL, ST_regular);
                IDS_AVIS (arg1) = cexavis;
                IDS_VARDEC (arg1) = AVIS_VARDECORARG (cexavis);

                arg2 = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (memavis))),
                                NULL, ST_regular);
                IDS_AVIS (arg2) = memavis;
                IDS_VARDEC (arg2) = AVIS_VARDECORARG (memavis);

                assign = MakeAssign (MakeLet (MakePrf2 (F_fill,
                                                        MakePrf1 (F_copy,
                                                                  MakeIdFromIds (arg1)),
                                                        MakeIdFromIds (arg2)),
                                              lhs),
                                     assign);
                AVIS_SSAASSIGN (IDS_AVIS (lhs)) = assign;

                /*
                 * Substitute cexpr
                 *
                 * Ex:
                 *   ...
                 *   a_val = fill( copy( a), a_mem);
                 * }: a_val;
                 */
                EXPRS_EXPR (cexprs) = FreeTree (EXPRS_EXPR (cexprs));
                EXPRS_EXPR (cexprs) = MakeIdFromIds (DupOneIds (lhs));

                /*
                 * Create suballoc assignment
                 *
                 * Ex:
                 *   ...
                 *   a_mem = suballoc( A, iv);
                 *   a_val = fill( copy( a), a_mem);
                 * }: a_val;
                 */
                lhs = DupOneIds (arg2);

                arg1 = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (als->avis))),
                                NULL, ST_regular);
                IDS_AVIS (arg1) = als->avis;
                IDS_VARDEC (arg1) = AVIS_VARDECORARG (als->avis);

                assign = MakeAssign (MakeLet (MakePrf2 (F_suballoc, MakeIdFromIds (arg1),
                                                        DupTree (INFO_EMAL_INDEXVECTOR (
                                                          arg_info))),
                                              lhs),
                                     assign);
                AVIS_SSAASSIGN (IDS_AVIS (lhs)) = assign;
            }
        }
        als = als->next;
        withops = NWITHOP_NEXT (withops);
        cexprs = EXPRS_NEXT (cexprs);
    }

    if (assign != NULL) {
        BLOCK_INSTR (NCODE_CBLOCK (arg_node))
          = AppendAssign (BLOCK_INSTR (NCODE_CBLOCK (arg_node)), assign);
    }

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
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
node *
EMALconst (node *arg_node, info *arg_info)
{
    alloclist_struct *als;

    DBUG_ENTER ("EMALconst");

    als = INFO_EMAL_ALLOCLIST (arg_info);

    if (als != NULL) {
        als->dim = MakeNum (0);
        als->shape = CreateZeroVector (0, T_int);

        /*
         * Signal EMALlet to wrap this RHS in a fill operation
         */
        INFO_EMAL_MUSTFILL (arg_info) = TRUE;
    }

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
        FUNDEF_BODY (fundef) = Trav (FUNDEF_BODY (fundef), arg_info);
    }

    /*
     * Traverse other fundefs
     */
    if (FUNDEF_NEXT (fundef) != NULL) {
        FUNDEF_NEXT (fundef) = Trav (FUNDEF_NEXT (fundef), arg_info);
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
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info),
                     IDS_AVIS (ID_IDS (ICM_ARG1 (arg_node))), MakeNum (0),
                     CreateZeroVector (0, T_int));
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
    ids *ids;
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
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

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
            ids = MakeIds (TmpVarName (IDS_NAME (LET_IDS (arg_node))), NULL, ST_regular);

            FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info))
              = MakeVardec (IDS_NAME (ids),
                            DupOneTypes (VARDEC_TYPE (IDS_VARDEC (LET_IDS (arg_node)))),
                            FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));

            avis = VARDEC_AVIS (FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));
            AVIS_TYPE (avis) = TYCopyType (AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node))));

            IDS_AVIS (ids) = avis;
            IDS_VARDEC (ids) = AVIS_VARDECORARG (avis);

            LET_EXPR (arg_node)
              = MakePrf (F_fill, MakeExprs (LET_EXPR (arg_node), Ids2Exprs (ids)));

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
        als->dim = MakeNum (0);
        als->shape = CreateZeroVector (0, T_int);
        break;

    case F_shape:
        /*
         * shape( A );
         * alloc(1, shape( shape( A )))
         */
        als->dim = MakeNum (1);
        als->shape = MakePrf1 (F_shape, DupTree (arg_node));
        break;

    case F_reshape:
        /*
         * reshape( sh, A );
         * alloc( shape( sh )[0], sh );
         */
        als->dim = MakeSizeArg (PRF_ARG1 (arg_node));
        als->shape = MakePrf1 (F_shape, DupTree (arg_node));
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
        als->dim = MakeNum (1);
        als->shape = MakePrf1 (F_shape, DupTree (arg_node));
        break;

    case F_sel:
        /*
         * sel( iv, A );
         * alloc( dim(A) - shape(iv)[0], shape( sel( iv, A)))
         */
        als->dim = MakePrf2 (F_sub_SxS, MakePrf1 (F_dim, DupNode (PRF_ARG2 (arg_node))),
                             MakeSizeArg (PRF_ARG1 (arg_node)));

        als->shape = MakePrf1 (F_shape, DupNode (arg_node));
        break;

    case F_idx_sel:
        /*
         * idx_sel can only occur when the result shape is known!!!
         *
         * a = idx_sel( idx, A);
         * alloc( dim( a), shape( idx_sel( idx, A)));
         */
        DBUG_ASSERT (/* No AKDs allowed here! */
                     TYIsAKS (AVIS_TYPE (als->avis)) || TYIsAKV (AVIS_TYPE (als->avis)),
                     "idx_sel with unknown result shape found!");
        als->dim = MakeNum (TYGetDim (AVIS_TYPE (als->avis)));
        als->shape = MakePrf1 (F_shape, DupNode (arg_node));
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
        als->dim = MakeNum (0);
        als->shape = CreateZeroVector (0, T_int);
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
        als->dim = MakeNum (0);
        als->shape = CreateZeroVector (0, T_int);
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
        if ((CountExprs (PRF_ARGS (arg_node)) < 2)
            || (NODE_TYPE (PRF_ARG2 (arg_node)) != N_id)
            || ((TYIsAKD (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node))))
                 || TYIsAKS (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node))))
                 || TYIsAKV (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node)))))
                && (TYGetDim (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node)))) == 0))) {
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
        DBUG_EXECUTE ("EMAL", PrintNode (arg_node););
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
    ids *i;

    DBUG_ENTER ("EMALwith");

    /*
     * ALLOCLIST is needed to traverse NCODEs and will be rescued there
     */

    /*
     * Annoate destination memory by traversing WITHOPS
     */
    INFO_EMAL_WITHOPMODE (arg_info) = ea_memname;
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    /*
     * In order to build a proper suballoc/fill combinations in each Code-Block
     * it is necessary to know which result variables refer to
     * genarray/modarray - withops
     * Furthermore, a code template for the index vector is needed
     */
    INFO_EMAL_WITHOPS (arg_info) = NWITH_WITHOP (arg_node);
    INFO_EMAL_INDEXVECTOR (arg_info) = MakeIdFromIds (DupOneIds (NWITH_VEC (arg_node)));

    /*
     * Traverse codes
     */
    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    /*
     * Free INDEXVECTOR code template
     */
    INFO_EMAL_INDEXVECTOR (arg_info) = FreeTree (INFO_EMAL_INDEXVECTOR (arg_info));

    /*
     * Rebuild ALLOCLIST by traversing WITHOPS
     */
    INFO_EMAL_WITHOPMODE (arg_info) = ea_shape;
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    /*
     * Allocate memory for the index vector
     */
    if (NWITH_VEC (arg_node) != NULL) {
        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info), IDS_AVIS (NWITH_VEC (arg_node)),
                     MakeNum (1), MakeShapeArg (NWITH_BOUND1 (arg_node)));
    }

    /*
     * Allocate memory for the index variables
     */
    i = NWITH_IDS (arg_node);
    while (i != NULL) {
        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info), IDS_AVIS (i), MakeNum (0),
                     CreateZeroVector (0, T_int));

        i = IDS_NEXT (i);
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
    ids *i;

    DBUG_ENTER ("EMALwith2");

    /*
     * Annoate destination memory by traversing WITHOPS
     */
    INFO_EMAL_WITHOPMODE (arg_info) = ea_memname;
    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    /*
     * In order to build a proper suballoc/fill combinations in each Code-Block
     * it is necessary to know which result variables refer to
     * genarray/modarray - withops
     * Furthermore, a code template for the index vector is needed
     */
    INFO_EMAL_WITHOPS (arg_info) = NWITH2_WITHOP (arg_node);
    INFO_EMAL_INDEXVECTOR (arg_info) = MakeIdFromIds (DupOneIds (NWITH2_VEC (arg_node)));

    /*
     * Traverse codes
     */
    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    /*
     * Free INDEXVECTOR code template
     */
    INFO_EMAL_INDEXVECTOR (arg_info) = FreeTree (INFO_EMAL_INDEXVECTOR (arg_info));

    /*
     * Rebuild ALLOCLIST by traversing WITHOPS
     */
    INFO_EMAL_WITHOPMODE (arg_info) = ea_shape;
    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    /*
     * Allocate memory for the index vector
     *
     * In Nwith2, shape of the index vector is always known!
     */
    if (NWITH2_VEC (arg_node) != NULL) {
        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info), IDS_AVIS (NWITH2_VEC (arg_node)),
                     MakeNum (1),
                     SHShape2Array (TYGetShape (
                       AVIS_TYPE (IDS_AVIS (NWITHID_VEC (NWITH2_WITHID (arg_node)))))));
    }

    /*
     * Allocate memory for the index variables
     */
    i = NWITH2_IDS (arg_node);
    while (i != NULL) {
        INFO_EMAL_ALLOCLIST (arg_info)
          = MakeALS (INFO_EMAL_ALLOCLIST (arg_info), IDS_AVIS (i), MakeNum (0),
                     CreateZeroVector (0, T_int));

        i = IDS_NEXT (i);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMALwithop
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
EMALwithop (node *arg_node, info *arg_info)
{
    node *wlavis;
    alloclist_struct *als;

    DBUG_ENTER ("EMALwithop");

    DBUG_ASSERT (INFO_EMAL_ALLOCLIST (arg_info) != NULL,
                 "ALLOCLIST must contain an entry for each WITHOP!");

    als = INFO_EMAL_ALLOCLIST (arg_info);
    INFO_EMAL_ALLOCLIST (arg_info) = als->next;
    als->next = NULL;

    if (NWITHOP_NEXT (arg_node) != NULL) {
        NWITHOP_NEXT (arg_node) = Trav (NWITHOP_NEXT (arg_node), arg_info);
    }

    if (INFO_EMAL_WITHOPMODE (arg_info) == ea_memname) {
        switch (NWITHOP_TYPE (arg_node)) {
        case WO_modarray:
        case WO_genarray:
            /*
             * Create new identifier for new memory
             */
            FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info))
              = MakeVardec (TmpVarName (VARDEC_NAME (AVIS_VARDECORARG (als->avis))),
                            DupOneTypes (VARDEC_TYPE (AVIS_VARDECORARG (als->avis))),
                            FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));

            wlavis = VARDEC_AVIS (FUNDEF_VARDEC (INFO_EMAL_FUNDEF (arg_info)));
            AVIS_TYPE (wlavis) = TYCopyType (AVIS_TYPE (als->avis));
            als->avis = wlavis;

            /*
             * Annotate which memory is to be used
             */
            NWITHOP_MEM (arg_node)
              = MakeId (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (als->avis))), NULL,
                        ST_regular);
            ID_AVIS (NWITHOP_MEM (arg_node)) = als->avis;
            ID_VARDEC (NWITHOP_MEM (arg_node)) = AVIS_VARDECORARG (als->avis);
            break;

        case WO_foldfun:
        case WO_foldprf:
            break;
        case WO_unknown:
            DBUG_ASSERT ((0), "Unknown Withloop-Type found");
        }
        als->next = INFO_EMAL_ALLOCLIST (arg_info);
        INFO_EMAL_ALLOCLIST (arg_info) = als;
    } else {
        DBUG_ASSERT (INFO_EMAL_WITHOPMODE (arg_info) == ea_shape,
                     "Unknown Withop traversal mode");
        switch (NWITHOP_TYPE (arg_node)) {
        case WO_foldprf:
        case WO_foldfun:
            /*
             * fold-wl:
             * fold wls allocate memory on their own
             */
            als = FreeALS (als);
            break;

        case WO_genarray:
            /*
             * If shape information has not yet been gathered it must be
             * inferred using the default element
             */
            if (als->dim == NULL) {
                DBUG_ASSERT (NWITHOP_DEFAULT (arg_node) != NULL,
                             "Default element required!");
                als->dim = MakePrf2 (F_add_SxS, MakeSizeArg (NWITHOP_SHAPE (arg_node)),
                                     MakeDimArg (NWITHOP_DEFAULT (arg_node)));
            }

            if (als->shape == NULL) {
                DBUG_ASSERT (NWITHOP_DEFAULT (arg_node) != NULL,
                             "Default element required!");
                als->shape
                  = MakePrf1 (F_shape,
                              MakePrf2 (F_genarray, DupNode (NWITHOP_SHAPE (arg_node)),
                                        DupNode (NWITHOP_DEFAULT (arg_node))));
            }

            /*
             * genarray-wl:
             * Allocation must remain in ALLOCLIST
             */
            als->next = INFO_EMAL_ALLOCLIST (arg_info);
            INFO_EMAL_ALLOCLIST (arg_info) = als;
            break;

        case WO_modarray:
            /*
             * modarray-wl:
             * dim, shape are the same as in the modified array
             */
            als->dim = MakePrf1 (F_dim, DupNode (NWITHOP_ARRAY (arg_node)));
            als->shape = MakePrf1 (F_shape, DupNode (NWITHOP_ARRAY (arg_node)));

            /*
             * modarray-wl:
             * Allocation must remain in ALLOCLIST
             */
            als->next = INFO_EMAL_ALLOCLIST (arg_info);
            INFO_EMAL_ALLOCLIST (arg_info) = als;
            break;

        case WO_unknown:
            DBUG_ASSERT ((0), "Unknown Withloop-Type found");
        }
    }

    DBUG_RETURN (arg_node);
}
/**
 * @}
 */

/** <!--******************************************************************-->
 *
 * @fn EMAllocateFill
 *
 *  @brief Starting function of transformation SAC -> SAC-MemVal.
 *
 *  @param syntax_tree
 *
 *  @return modified syntax tree containing explicit alloc and fill prfs
 *
 ***************************************************************************/
node *
EMAllocateFill (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("EMALAllocateFill");

    info = MakeInfo ();

    act_tab = emalloc_tab;
    syntax_tree = Trav (syntax_tree, info);

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/**
 * @}
 */
