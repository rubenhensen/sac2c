/*
 * $Id$
 */

/**
 * @defgroup alloc Memory Allocation
 *
 * Introduces explicit instructions for allocating memory.
 * This effectively converts functional SAC programs into imperative programs
 * that perform state changes.
 *
 * @ingroup mm
 *
 * @{
 */

/**
 * @file alloc.c
 *
 * Prefix: EMAL
 */

#include "alloc.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "DupTree.h"
#include "new_types.h"
#include "type_utils.h"
#include "print.h"
#include "free.h"
#include "shape.h"
#include "string.h"
#include "dbug.h"

/**
 * Controls wheter AKS-Information should be used when possible
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
    node *reuse;
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
#define INFO_ALLOCLIST(n) ((n)->alloclist)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_WITHOPS(n) ((n)->withops)
#define INFO_INDEXVECTOR(n) ((n)->indexvector)
#define INFO_MUSTFILL(n) ((n)->mustfill)
#define INFO_WITHOPMODE(n) ((n)->withopmode)

/**
 * @name INFO functions
 *
 * @{
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ALLOCLIST (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_WITHOPS (result) = NULL;
    INFO_INDEXVECTOR (result) = NULL;
    INFO_MUSTFILL (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * @}
 */

/** <!--******************************************************************-->
 *
 * @name Functions for manipulating alloclist structures
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 *  @fn alloclist_struct *MakeALS( alloclist_struct *als, node *avis,
                                   node *dim, node *shape)
 *
 *  @brief Creates an ALLOCLIST_STRUCT for allocation.
 *
 *  @param als   An alloclist that will be appended to the new element.
 *  @param avis  N_avis node of the variable memory must be allocated for.
 *  @param dim   A suitable description of the rank of the array to
 *               be stored.
 *  @param shape A suitable description of the shape of the array to
 *               be stored.
 *
 ***************************************************************************/
static alloclist_struct *
MakeALS (alloclist_struct *als, node *avis, node *dim, node *shape)
{
    alloclist_struct *res;

    DBUG_ENTER ("MakeALS");

    res = MEMmalloc (sizeof (alloclist_struct));

    res->avis = avis;
    res->dim = dim;
    res->shape = shape;
    res->reshape = NULL;
    res->reuse = NULL;
    res->next = als;

    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * @fn alloclist_struct *FreeALS( alloclist_struct *als)
 *
 *  @brief Frees all elements of the given ALLOCLIST
 *
 *  @param als The alloclist to be freed
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

        als = MEMfree (als);
    }

    DBUG_RETURN (als);
}

/** <!--******************************************************************-->
 *
 * @fn alloclist_struct *Ids2ALS( node *ids)
 *
 *  @brief Converts a list of N_ids nodes to an alloclist.
 *
 *  @param ids N_ids chain to be converted.
 *
 ***************************************************************************/
static alloclist_struct *
Ids2ALS (node *ids)
{
    alloclist_struct *res;

    if (ids == NULL) {
        res = NULL;
    } else {
        res = MakeALS (Ids2ALS (IDS_NEXT (ids)), IDS_AVIS (ids), NULL, NULL);
    }

    return (res);
}

/** <!--******************************************************************-->
 *
 * @fn bool AlloclistContains( alloclist_struct *als, node *avis)
 *
 *  @brief Checks wheter the given alloclist contains an entry for the
 *         designated element.
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
 * @fn node *MakeAllocAssignment( alloclist_struct *als, node *next_node)
 *
 *  @brief Provides an allocation assignment corresponding to an alloclist
 *         element
 *
 *  @param als The alloclist
 *  @param next_node An N_assign node that will be appended to the
 *         allocation assignment.
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

    if (als->reuse != NULL) {
        alloc = TCmakePrf1 (F_reuse, als->reuse);
        als->reuse = NULL;
    } else {
        DBUG_ASSERT (als->dim != NULL, "alloc requires a dim expression!");
        DBUG_ASSERT (als->shape != NULL, "alloc requires a shape expression!");

        /*
         * Annotate exact dim/shape information if available
         */
#ifdef USEAKS
        if (TUdimKnown (AVIS_TYPE (als->avis))) {
            als->dim = FREEdoFreeTree (als->dim);
            als->dim = TBmakeNum (TYgetDim (AVIS_TYPE (als->avis)));
        }

        if (TUshapeKnown (AVIS_TYPE (als->avis))) {
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
    }
    als->dim = NULL;
    als->shape = NULL;

    alloc = TBmakeAssign (TBmakeLet (ids, alloc), next_node);
    AVIS_SSAASSIGN (IDS_AVIS (ids)) = alloc;

    DBUG_RETURN (alloc);
}

/** <!--******************************************************************-->
 *
 * @fn node *MakeDimArg( node *arg)
 *
 *  @brief returns MakeNum( 0) for primitive data types,
 *         the frame dim of a given N_array node and dim( a) for N_id nodes.
 *
 *  @param arg A N_num, N_float, N_char, N_bool, N_double, N_array, or N_id
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
        arg = TBmakeNum (SHgetDim (ARRAY_FRAMESHAPE (arg)));
        break;

    case N_id:
        arg = TCmakePrf1 (F_dim_A, DUPdoDupNode (arg));
        break;

    default:
        DBUG_EXECUTE ("EMAL", PRTdoPrintNode (arg););
        DBUG_ASSERT ((0), "Invalid argument");
    }

    DBUG_RETURN (arg);
}

/** <!--******************************************************************-->
 *
 * @fn node *MakeShapeArg( node *arg)
 *
 *  @brief returns [] for primitive data types,
 *         the frame shape for N_array nodes and shape( a) for N_id nodes.
 *
 *  @param arg A N_num, N_float, N_char, N_bool, N_double, N_array, or N_id
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
        arg = SHshape2Array (ARRAY_FRAMESHAPE (arg));
        break;

    case N_id:
        arg = TCmakePrf1 (F_shape_A, DUPdoDupNode (arg));
        break;

    default:
        DBUG_EXECUTE ("EMAL", PRTdoPrintNode (arg););
        DBUG_ASSERT ((0), "Invalid argument");
    }

    DBUG_RETURN (arg);
}

/** <!--******************************************************************-->
 *
 * @fn node *MakeSizeArg( node *arg)
 *
 *  @brief returns 1 for primitive data types,
 *         the product of the frame shape for N_array nodes and
 *         shape( a)[0] for N_id nodes representing VECTORS.
 *
 *  @param arg A N_num, N_float, N_char, N_bool, N_double, N_array, or N_id
 *             nodes representing VECTORS.
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
        arg = TBmakeNum (SHgetUnrLen (ARRAY_FRAMESHAPE (arg)));
        break;

    case N_id:
        arg = TCmakePrf2 (F_sel_VxA, TBmakeNum (0),
                          TCmakePrf1 (F_shape_A, DUPdoDupNode (arg)));
        break;

    default:
        DBUG_EXECUTE ("EMAL", PRTdoPrintNode (arg););
        DBUG_ASSERT ((0), "Invalid argument");
    }

    DBUG_RETURN (arg);
}

/** <!-- ****************************************************************** -->
 * @brief Adds a suballoc/fill combination for the withops. Furthermore,
 *        shape and dimensionality information is added to the alloclist
 *        if possible.
 *        Lifted from EMALcode.
 *
 * @param withops   withops of this with loop
 * @param idxs      wl idxs for with3, NULL otherwise
 * @param cexprs    result expressions
 * @param arg_info  info node used to access INFO_INDEXVECTOR, INFO_ALLOCLIST.
 *
 * @return N_assign change to be prepended to the body of this part
 ******************************************************************************/
static node *
AmendWithLoopCode (node *withops, node *idxs, node *cexprs, info *arg_info)
{
    node *memavis, *valavis, *cexavis, *indexvector, *wlidx;
    ntype *crestype = NULL;
    node *assign = NULL;
    alloclist_struct *als;

    DBUG_ENTER ("AmendWithLoopCode");

    indexvector = INFO_INDEXVECTOR (arg_info);
    als = INFO_ALLOCLIST (arg_info);

    while (withops != NULL) {
        DBUG_ASSERT (als != NULL, "ALLOCLIST must have an element for each WITHOP");
        DBUG_ASSERT (cexprs != NULL, "With-Loop must have as many results as ALLOCLIST");

        cexavis = ID_AVIS (EXPRS_EXPR (cexprs));

        if ((NODE_TYPE (withops) == N_genarray) || (NODE_TYPE (withops) == N_modarray)) {
            if (idxs == NULL) {
                /* with / with2: use index annotated at withop */
                wlidx = WITHOP_IDX (withops);
            } else {
                /* with3: extract one idx from idxs annotated at range */
                wlidx = IDS_AVIS (idxs);
                idxs = IDS_NEXT (idxs);
            }
        } else {
            wlidx = NULL;
        }

        /*
         * Set shape of genarray-wls if not already done and possible
         */
        if (NODE_TYPE (withops) == N_genarray) {

            if (als->dim == NULL) {
                if (TUdimKnown (AVIS_TYPE (cexavis))) {
                    als->dim
                      = TCmakePrf2 (F_add_SxS, MakeSizeArg (GENARRAY_SHAPE (withops)),
                                    TBmakeNum (TYgetDim (AVIS_TYPE (cexavis))));
                }
            }
            if (als->shape == NULL) {
                if (TUshapeKnown (AVIS_TYPE (cexavis))) {
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
         * {
         *   ...
         * }: a;
         */

        /*
         * We first try to find out the most special type for the
         * elements computed in each iteration of the withloop
         * (the c-expression). The reason for this is twofold:
         *
         * - if we manage to find an AKS type, we can use wlassign
         *   instead of a subvar which is more efficient
         *
         * - to be able to allocate and build a correct descriptor
         *   we have to make sure, that the shape-class of the
         *   subvar is AKS if the default value or cexpr is
         *   (in the latter case, there usually is no default value
         *   at all). If the subvar is AKD/AUD, we need the default
         *   value to assign a correct descriptor.
         *   For modarrays, we use a special WL_MODARRAY_SUBSHAPE
         *   icm to create the descriptor of the subvar. This icm
         *   relies on the fact, that the shape-class of the subvar
         *   always is more special than the one of the result
         *   array.
         */

        /*
         * N_genarray
         */
        if ((NODE_TYPE (withops) == N_genarray) && (GENARRAY_DEFAULT (withops) != NULL)) {
            DBUG_ASSERT ((NODE_TYPE (GENARRAY_DEFAULT (withops)) == N_id),
                         "found a non flattened default expression!");

            /*
             * use more special type of default or cexpression
             */
            if (TYleTypes (AVIS_TYPE (cexavis), ID_NTYPE (GENARRAY_DEFAULT (withops)))) {
                crestype = TYcopyType (AVIS_TYPE (cexavis));
            } else {
                crestype = TYcopyType (ID_NTYPE (GENARRAY_DEFAULT (withops)));
            }
            /*
             * N_modarray:
             */
        } else if ((NODE_TYPE (withops) == N_modarray)
                   && (!TUshapeKnown (AVIS_TYPE (cexavis)))) {

            DBUG_ASSERT ((indexvector != NULL), "modarray for with3 not yet supported!");

            ntype *ivtype = ID_NTYPE (indexvector);
            ntype *restype = AVIS_TYPE (als->avis);
            /*
             * we need a AKS index vector in order to
             * do any computation on the cexpressions
             * shape.
             */
            if (TUshapeKnown (ivtype) && TUdimKnown (restype)) {
                if (TYgetDim (restype) == SHgetExtent (TYgetShape (ivtype), 0)) {
                    /*
                     * a scalar!
                     */
                    DBUG_PRINT ("EMAL", ("subvar %s is Scalar", AVIS_NAME (als->avis)));

                    crestype
                      = TYmakeAKS (TYcopyType (TYgetScalar (restype)), SHmakeShape (0));
                } else {
                    /*
                     * non scalar!
                     */
                    if (TUshapeKnown (restype)) {
                        DBUG_PRINT ("EMAL",
                                    ("subvar of %s is AKS", AVIS_NAME (als->avis)));

                        crestype
                          = TYmakeAKS (TYcopyType (TYgetScalar (restype)),
                                       SHdropFromShape (SHgetExtent (TYgetShape (ivtype),
                                                                     0),
                                                        TYgetShape (restype)));
                    } else {
                        DBUG_PRINT ("EMAL",
                                    ("subvar of %s is AKD", AVIS_NAME (als->avis)));

                        crestype = TYmakeAKD (TYcopyType (TYgetScalar (restype)),
                                              TYgetDim (restype) - TYgetDim (ivtype),
                                              SHmakeShape (0));
                    }
                }
            } else {
                /*
                 * restype is AUD or index is non AKS, so leave crestype as is
                 */
                crestype = TYcopyType (AVIS_TYPE (cexavis));
            }
        } else {
            /*
             * cexavis already is AKS, so we cannot do better!
             */
            crestype = TYcopyType (AVIS_TYPE (cexavis));
        }

        if ((NODE_TYPE (withops) == N_genarray) || (NODE_TYPE (withops) == N_modarray)
            || (NODE_TYPE (withops) == N_break)) {

            if (TUdimKnown (crestype) && (TYgetDim (crestype) == 0)) {
                node *prfap;

                /*
                 * Create a new value variable
                 * Ex: a_val
                 */
                valavis = TBmakeAvis (TRAVtmpVarName ("val"), TYcopyType (crestype));

                FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (valavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

                /*
                 * Create wl-assign operation
                 *
                 * Ex:
                 * {
                 *   ...
                 *   a_val = wl_assign( a, A, iv, idx);
                 * }: a_val;
                 *
                 * for Break withops, we use wl_break(a,A,iv);
                 */
                if (NODE_TYPE (withops) == N_break) {
                    prfap
                      = TCmakePrf3 (F_wl_break, TBmakeId (cexavis), TBmakeId (als->avis),
                                    DUPdoDupNode (INFO_INDEXVECTOR (arg_info)));

                } else {
                    prfap
                      = TCmakePrf4 (F_wl_assign, TBmakeId (cexavis), TBmakeId (als->avis),
                                    DUPdoDupNode (INFO_INDEXVECTOR (arg_info)),
                                    TBmakeId (wlidx));
                }

                assign
                  = TBmakeAssign (TBmakeLet (TBmakeIds (valavis, NULL), prfap), assign);

                AVIS_SSAASSIGN (valavis) = assign;

                /*
                 * Substitute cexpr
                 *
                 * Ex:
                 * {
                 *   ...
                 *   a_val = wl_assign( a, A, idx);
                 * }: a_val;
                 */
                EXPRS_EXPR (cexprs) = FREEdoFreeTree (EXPRS_EXPR (cexprs));
                EXPRS_EXPR (cexprs) = TBmakeId (valavis);
            } else {
                /*
                 * Create a new memory variable
                 * Ex: a_mem
                 */
                memavis = TBmakeAvis (TRAVtmpVarName ("mem"), TYeliminateAKV (crestype));

                FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (memavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

                /*
                 * Create a new value variable
                 * Ex: a_val
                 */
                valavis = TBmakeAvis (TRAVtmpVarName ("val"), TYcopyType (crestype));

                FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (valavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

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
                 *   a_mem = suballoc( A, idx);
                 *   a_val = fill( copy( a), a_mem);
                 * }: a_val;
                 */
                assign
                  = TBmakeAssign (TBmakeLet (TBmakeIds (memavis, NULL),
                                             TCmakePrf2 (F_suballoc, TBmakeId (als->avis),
                                                         TBmakeId (wlidx))),
                                  assign);
                AVIS_SSAASSIGN (memavis) = assign;
            }
        }
        als = als->next;
        withops = WITHOP_NEXT (withops);
        cexprs = EXPRS_NEXT (cexprs);
        crestype = TYfreeType (crestype);
    }

    DBUG_RETURN (assign);
}

/**
 * @}
 */

/**
 *
 * @name Traversal functions
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn node *EMALap( node *arg_node, info *arg_info)
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

    INFO_ALLOCLIST (arg_info) = FreeALS (INFO_ALLOCLIST (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn node *EMALarray( node *arg_node, info *arg_info)
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

    als = INFO_ALLOCLIST (arg_info);

    if (ARRAY_STRING (arg_node) != NULL) {
        /* array is a string */
        als->dim = TBmakeNum (1);
        als->shape = MakeShapeArg (arg_node);
    } else {
        if (ARRAY_AELEMS (arg_node) == NULL) {
            /*
             * [:type]
             * alloc( outer_dim + dim( type), outer_shape ++ shape( type))
             */
            shape *sh;

            DBUG_ASSERT (TUshapeKnown (ARRAY_ELEMTYPE (arg_node)),
                         "assignment  A = [:basetype];  found, "
                         "where basetype has non-constant shape!");

            sh = SHappendShapes (ARRAY_FRAMESHAPE (arg_node),
                                 TYgetShape (ARRAY_ELEMTYPE (arg_node)));

            als->dim = TBmakeNum (SHgetDim (sh));
            als->shape = SHshape2Array (sh);

            sh = SHfreeShape (sh);
        } else {
            /*
             * [ a, ... ]
             * alloc( outer_dim + dim(a), shape( [a, ...]))
             */
            if (NODE_TYPE (ARRAY_AELEMS (arg_node)) == N_id) {
                als->dim = TCmakePrf2 (F_add_SxS, MakeDimArg (arg_node),
                                       MakeDimArg (EXPRS_EXPR (ARRAY_AELEMS (arg_node))));
            } else {
                /*
                 * Elements MUST be constant scalars
                 */
                als->dim = MakeDimArg (arg_node);
            }

            als->shape = TCmakePrf1 (F_shape_A, DUPdoDupTree (arg_node));
        }
    }

    /*
     * Signal EMALlet to wrap this RHS in a fill-operation
     */
    INFO_MUSTFILL (arg_info) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn node *EMALassign( node *arg_node, info *arg_info)
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
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /*
     * Traverse RHS of assignment
     */
    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    /*
     * Allocate missing variables
     */
    als = INFO_ALLOCLIST (arg_info);
    while (als != NULL) {
        arg_node = MakeAllocAssignment (als, arg_node);
        als = als->next;
    }
    INFO_ALLOCLIST (arg_info) = FreeALS (INFO_ALLOCLIST (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn node *EMALcode( node *arg_node, info *arg_info)
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
    node *withops, *indexvector, *assign;

    DBUG_ENTER ("EMALcode");

    /*
     * Rescue ALLOCLIST, WITHOPS and INDEXVECTOR
     */
    als = INFO_ALLOCLIST (arg_info);
    INFO_ALLOCLIST (arg_info) = NULL;
    withops = INFO_WITHOPS (arg_info);
    INFO_WITHOPS (arg_info) = NULL;
    indexvector = INFO_INDEXVECTOR (arg_info);
    INFO_INDEXVECTOR (arg_info) = NULL;

    /*
     * Traverse block
     */
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    /*
     * Restore ALLOCLIST, WITHOPS and INDEXVECTOR
     */
    INFO_ALLOCLIST (arg_info) = als;
    INFO_WITHOPS (arg_info) = withops;
    INFO_INDEXVECTOR (arg_info) = indexvector;

    /*
     * Shape information for each result array must be set and
     * suballoc/fill(copy,...) must be inserted
     */
    assign = AmendWithLoopCode (INFO_WITHOPS (arg_info), NULL, CODE_CEXPRS (arg_node),
                                arg_info);

    if (assign != NULL) {
        BLOCK_INSTR (CODE_CBLOCK (arg_node))
          = TCappendAssign (BLOCK_INSTR (CODE_CBLOCK (arg_node)), assign);
    }

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * Macro for defining the traversal of nodes representing the primitive
 * data types.
 *
 ***************************************************************************/
#define EMALCONST(name)                                                                  \
    node *EMAL##name (node *arg_node, info *arg_info)                                    \
    {                                                                                    \
        alloclist_struct *als;                                                           \
                                                                                         \
        DBUG_ENTER ("EMAL" #name);                                                       \
                                                                                         \
        als = INFO_ALLOCLIST (arg_info);                                                 \
                                                                                         \
        if (als != NULL) {                                                               \
            als->dim = TBmakeNum (0);                                                    \
            als->shape = TCcreateZeroVector (0, T_int);                                  \
                                                                                         \
            /*                                                                           \
             * Signal EMALlet to wrap this RHS in a fill operation                       \
             */                                                                          \
            INFO_MUSTFILL (arg_info) = TRUE;                                             \
        }                                                                                \
                                                                                         \
        DBUG_RETURN (arg_node);                                                          \
    }

/** <!--******************************************************************-->
 *
 * @fn node *EMALbool( node *arg_node, info *arg_info)
 * @fn node *EMALchar( node *arg_node, info *arg_info)
 * @fn node *EMALfloat( node *arg_node, info *arg_info)
 * @fn node *EMALdouble( node *arg_node, info *arg_info)
 * @fn node *EMALnum( node *arg_node, info *arg_info)
 *
 ***************************************************************************/
EMALCONST (bool)
EMALCONST (char)
EMALCONST (float)
EMALCONST (double)
EMALCONST (num)

/** <!--******************************************************************-->
 *
 * @fn node *EMALfuncond( node *arg_node, info *arg_info)
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

    INFO_ALLOCLIST (arg_info) = FreeALS (INFO_ALLOCLIST (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn node *EMALfundef( node *fundef, info *arg_info)
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

    INFO_FUNDEF (arg_info) = fundef;

    /*
     * Traverse fundef body
     */
    FUNDEF_BODY (fundef) = TRAVopt (FUNDEF_BODY (fundef), arg_info);

    /*
     * Traverse other fundefs
     */
    FUNDEF_NEXT (fundef) = TRAVopt (FUNDEF_NEXT (fundef), arg_info);

    DBUG_RETURN (fundef);
}

/** <!--******************************************************************-->
 *
 * @fn node *EMALid( node *arg_node, info *arg_info)
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

    INFO_ALLOCLIST (arg_info) = FreeALS (INFO_ALLOCLIST (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn node *EMALlet( node *arg_node, info *arg_info)
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
    INFO_ALLOCLIST (arg_info) = Ids2ALS (LET_IDS (arg_node));

    /*
     * Traverse RHS
     */
    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

        /*
         * Wrap RHS in Fill-operation if necessary
         */
        if (INFO_MUSTFILL (arg_info)) {
            /*
             * a = b + c;
             *
             * is transformed into
             *
             * a' = alloc(...);
             * a = fill( b + c, a');
             */
            avis
              = TBmakeAvis (TRAVtmpVarName (IDS_NAME (LET_IDS (arg_node))),
                            TYeliminateAKV (AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)))));

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

            LET_EXPR (arg_node)
              = TCmakePrf2 (F_fill, LET_EXPR (arg_node), TBmakeId (avis));

            /*
             * Set the avis of the freshly allocated variable
             */
            INFO_ALLOCLIST (arg_info)->avis = avis;
        }
        INFO_MUSTFILL (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn node *EMALprf( node *arg_node, info *arg_info)
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

    als = INFO_ALLOCLIST (arg_info);

    /*
     * Signal EMALlet to wrap this prf in a fill-operation
     */
    INFO_MUSTFILL (arg_info) = TRUE;

#if 0
  /* to be done */
  alloc_funtab[PRF_PRF( arg_node)]( arg_node, arg_info);
#endif

    switch (PRF_PRF (arg_node)) {
    case F_dim_A:
        /*
         * dim( A );
         * alloc( 0, [] );
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_shape_A:
        /*
         * shape( A );
         * alloc(1, shape( shape( A )))
         */
        als->dim = TBmakeNum (1);
        als->shape = TCmakePrf1 (F_shape_A, DUPdoDupTree (arg_node));
        break;

    case F_reshape_VxA:
        /*
         * reshape( sh, A );
         * alloc_or_reshape( shape( sh )[0], sh, A );
         * copy( A);
         */
        als->dim = MakeSizeArg (PRF_ARG1 (arg_node));
        als->shape = TCmakePrf1 (F_shape_A, DUPdoDupTree (arg_node));

        /*
         * For reshaping to work, both RHS and LHS must be non-scalar
         * This caused bug #145
         */
        if (((!TUdimKnown (ID_NTYPE (PRF_ARG2 (arg_node))))
             || (TYgetDim (ID_NTYPE (PRF_ARG2 (arg_node))) > 0))
            && ((!TUdimKnown (AVIS_TYPE (als->avis)))
                || (TYgetDim (AVIS_TYPE (als->avis)) > 0))) {
            als->reshape = DUPdoDupNode (PRF_ARG2 (arg_node));
        }

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
        als->shape = TCmakePrf1 (F_shape_A, DUPdoDupTree (arg_node));
        break;

    case F_sel_VxA:
        /*
         * sel( iv, A );
         * alloc( dim(A) - shape(iv)[0], shape( sel( iv, A)))
         */
        als->dim = TCmakePrf2 (F_sub_SxS,
                               TCmakePrf1 (F_dim_A, DUPdoDupNode (PRF_ARG2 (arg_node))),
                               MakeSizeArg (PRF_ARG1 (arg_node)));

        als->shape = TCmakePrf1 (F_shape_A, DUPdoDupNode (arg_node));
        break;

    case F_idx_sel:
        /*
         * idx_sel can only occur when the result shape is known!!!
         *
         * a = idx_sel( idx, A);
         * alloc( dim( a), shape( idx_sel( idx, A)));
         */
        DBUG_ASSERT (/* No AKDs allowed here! */
                     TUshapeKnown (AVIS_TYPE (als->avis)),
                     "idx_sel with unknown result shape found!");
        als->dim = TBmakeNum (TYgetDim (AVIS_TYPE (als->avis)));
        als->shape = TCmakePrf1 (F_shape_A, DUPdoDupNode (arg_node));
        break;

    case F_modarray_AxVxS:
    case F_modarray_AxVxA:
    case F_idx_modarray_AxSxS:
    case F_idx_modarray_AxSxA:
        /*
         * modarray_AxVxS( A, iv, val);
         * modarray_AxVxA( A, iv, val);
         * idx_modarray_AxSxS( A, idx, val);
         * idx_modarray_AxSxA( A, idx, val);
         *
         * reuse( A);
         */
        als->reuse = DUPdoDupNode (PRF_ARG1 (arg_node));
        break;

    case F_idx_shape_sel:
        /*
         * shape_sel always yields a scalar
         *
         * a = shape_sel( idx, A)
         * alloc( 0, []);
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_idxs2offset:
    case F_vect2offset:
        /*
         * offset is always a scalar
         *
         * a = shape_sel( idx, A)
         * alloc( 0, []);
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_run_mt_genarray:
    case F_run_mt_modarray:
    case F_run_mt_fold:
        /*
         * boolean predicate
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_add_SxS:
    case F_sub_SxS:
    case F_mul_SxS:
    case F_div_SxS:
    case F_and_SxS:
    case F_or_SxS:
    case F_mod_SxS:
    case F_min_SxS:
    case F_max_SxS:
    case F_le_SxS:
    case F_lt_SxS:
    case F_eq_SxS:
    case F_neq_SxS:
    case F_ge_SxS:
    case F_gt_SxS:
    case F_neg_S:
    case F_abs_S:
    case F_tob_S:
    case F_toc_S:
    case F_toi_S:
    case F_tof_S:
    case F_tod_S:
    case F_not_S:
        /*
         * simple scalar operations
         * alloc( 0, [])
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_add_SxV:
    case F_sub_SxV:
    case F_mul_SxV:
    case F_div_SxV:
    case F_and_SxV:
    case F_or_SxV:
    case F_mod_SxV:
    case F_min_SxV:
    case F_max_SxV:
    case F_le_SxV:
    case F_lt_SxV:
    case F_eq_SxV:
    case F_neq_SxV:
    case F_ge_SxV:
    case F_gt_SxV:
        als->dim = MakeDimArg (PRF_ARG2 (arg_node));
        als->shape = MakeShapeArg (PRF_ARG2 (arg_node));
        break;

    case F_add_VxS:
    case F_sub_VxS:
    case F_mul_VxS:
    case F_div_VxS:
    case F_and_VxS:
    case F_or_VxS:
    case F_mod_VxS:
    case F_min_VxS:
    case F_max_VxS:
    case F_le_VxS:
    case F_lt_VxS:
    case F_eq_VxS:
    case F_neq_VxS:
    case F_ge_VxS:
    case F_gt_VxS:
    case F_not_V:
    case F_neg_V:
    case F_abs_V:
        als->dim = MakeDimArg (PRF_ARG1 (arg_node));
        als->shape = MakeShapeArg (PRF_ARG1 (arg_node));
        break;

    case F_add_VxV:
    case F_sub_VxV:
    case F_mul_VxV:
    case F_div_VxV:
    case F_and_VxV:
    case F_or_VxV:
    case F_mod_VxV:
    case F_min_VxV:
    case F_max_VxV:
    case F_le_VxV:
    case F_lt_VxV:
    case F_eq_VxV:
    case F_neq_VxV:
    case F_ge_VxV:
    case F_gt_VxV:
        if (NODE_TYPE (PRF_ARG2 (arg_node)) != N_id) {
            als->dim = MakeDimArg (PRF_ARG1 (arg_node));
            als->shape = MakeShapeArg (PRF_ARG1 (arg_node));
        } else {
            als->dim = MakeDimArg (PRF_ARG2 (arg_node));
            als->shape = MakeShapeArg (PRF_ARG2 (arg_node));
        }
        break;

    case F_copy:
        if ((TCcountExprs (PRF_ARGS (arg_node)) < 2)
            || (NODE_TYPE (PRF_ARG2 (arg_node)) != N_id)
            || (TUdimKnown (ID_NTYPE (PRF_ARG2 (arg_node)))
                && (TYgetDim (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node)))) == 0))) {
            als->dim = MakeDimArg (PRF_ARG1 (arg_node));
            als->shape = MakeShapeArg (PRF_ARG1 (arg_node));
        } else {
            als->dim = MakeDimArg (PRF_ARG2 (arg_node));
            als->shape = MakeShapeArg (PRF_ARG2 (arg_node));
        }
        break;

    case F_prop_obj_in:
    case F_prop_obj_out:
    case F_accu:
    case F_type_error:
    case F_type_conv:
    case F_dispatch_error:
        /*
         * a,... = accu( iv, n, ...) (and prop_obj)
         * v,... = _type_error_( ...)
         * require a special treatment as
         * none of its return value must be allocated
         */
        INFO_ALLOCLIST (arg_info) = FreeALS (INFO_ALLOCLIST (arg_info));

        INFO_MUSTFILL (arg_info) = FALSE;
        break;

    case F_guard:
        /*
         * v1,...,vn = guard(p,a1,...,an)
         * - consumes p
         * - each v_i is an alias of a_i
         */
        INFO_ALLOCLIST (arg_info) = FreeALS (INFO_ALLOCLIST (arg_info));

        INFO_MUSTFILL (arg_info) = FALSE;
        break;

    case F_afterguard:
        /*
         * v = afterguard(a,p1,...,pn)
         * - consumes p1...pn
         * - v is an alias of a_i
         */
        INFO_ALLOCLIST (arg_info) = FreeALS (INFO_ALLOCLIST (arg_info));

        INFO_MUSTFILL (arg_info) = FALSE;
        break;

    case F_type_constraint:
        /*
         * p = type_constraint( t, a)
         * - t is a type argument which has no runtime representation
         * - a is consumed
         * - p is a boolean scalarresult
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_same_shape_AxA:
    case F_shape_matches_dim_VxA:
    case F_non_neg_val_V:
    case F_val_lt_shape_VxA:
    case F_val_le_val_VxV:
    case F_prod_matches_prod_shape_VxA:
        /*
         * p = constraint(a1,..,an)
         * - a_i are consumed
         * - p is a boolean result
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
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
    case F_genarray:
        DBUG_ASSERT ((0), "invalid prf found!");
        break;

    default:
        DBUG_EXECUTE ("EMAL", PRTdoPrintNode (arg_node););
        DBUG_ASSERT (FALSE, "unknown prf found!");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn node *EMALwith( node *arg_node, info *arg_info)
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

    DBUG_ENTER ("EMALwith");

    /*
     * ALLOCLIST is needed to traverse CODEs and will be rescued there
     */

    /*
     * Annoate destination memory by traversing WITHOPS
     */
    INFO_WITHOPMODE (arg_info) = EA_memname;
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /*
     * In order to build a proper suballoc/fill combinations in each Code-Block
     * it is necessary to know which result variables refer to
     * genarray/modarray - withops
     * Furthermore, a code template for the index vector is needed
     */
    INFO_WITHOPS (arg_info) = WITH_WITHOP (arg_node);
    INFO_INDEXVECTOR (arg_info) = TBmakeId (IDS_AVIS (WITH_VEC (arg_node)));

    /*
     * Traverse codes
     */
    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);

    /*
     * Free INDEXVECTOR code template
     */
    INFO_INDEXVECTOR (arg_info) = FREEdoFreeTree (INFO_INDEXVECTOR (arg_info));

    /*
     * Rebuild ALLOCLIST by traversing WITHOPS
     */
    INFO_WITHOPMODE (arg_info) = EA_shape;
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /*
     * Allocate memory for the index vector
     * and replace WITH_VEC ids with id
     */
    INFO_ALLOCLIST (arg_info)
      = MakeALS (INFO_ALLOCLIST (arg_info), IDS_AVIS (WITH_VEC (arg_node)), TBmakeNum (1),
                 MakeShapeArg (WITH_BOUND1 (arg_node)));

    expr = TBmakeId (IDS_AVIS (WITH_VEC (arg_node)));
    WITH_VEC (arg_node) = FREEdoFreeTree (WITH_VEC (arg_node));
    WITH_VEC (arg_node) = expr;

    /*
     * Traverse first withid to allocate memory for offset scalars
     */
    WITH_WITHID (arg_node) = TRAVdo (WITH_WITHID (arg_node), arg_info);

    /*
     * Duplicate withid for the second part
     */
    if (PART_NEXT (WITH_PART (arg_node)) != NULL) {
        node *nextpart = PART_NEXT (WITH_PART (arg_node));
        PART_WITHID (nextpart) = FREEdoFreeNode (PART_WITHID (nextpart));
        PART_WITHID (nextpart) = DUPdoDupNode (WITH_WITHID (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn node *EMALwith2( node *arg_node, info *arg_info)
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
    DBUG_ENTER ("EMALwith2");

    /*
     * Annoate destination memory by traversing WITHOPS
     */
    INFO_WITHOPMODE (arg_info) = EA_memname;
    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    /*
     * In order to build a proper suballoc/fill combinations in each Code-Block
     * it is necessary to know which result variables refer to
     * genarray/modarray - withops
     * Furthermore, a code template for the index vector is needed
     */
    INFO_WITHOPS (arg_info) = WITH2_WITHOP (arg_node);
    INFO_INDEXVECTOR (arg_info) = TBmakeId (IDS_AVIS (WITH2_VEC (arg_node)));

    /*
     * Traverse codes
     */
    WITH2_CODE (arg_node) = TRAVopt (WITH2_CODE (arg_node), arg_info);

    /*
     * Free INDEXVECTOR code template
     */
    INFO_INDEXVECTOR (arg_info) = FREEdoFreeTree (INFO_INDEXVECTOR (arg_info));

    /*
     * Rebuild ALLOCLIST by traversing WITHOPS
     */
    INFO_WITHOPMODE (arg_info) = EA_shape;
    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    /*
     * Allocate memory for the index vector
     * and replace WITH2_VEC ids with id
     *
     * In Nwith2, shape of the index vector is always known!
     */
    if (WITH2_VEC (arg_node) != NULL) {
        node *expr;
        INFO_ALLOCLIST (arg_info)
          = MakeALS (INFO_ALLOCLIST (arg_info), IDS_AVIS (WITH2_VEC (arg_node)),
                     TBmakeNum (1),
                     SHshape2Array (TYgetShape (
                       AVIS_TYPE (IDS_AVIS (WITHID_VEC (WITH2_WITHID (arg_node)))))));

        expr = TBmakeId (IDS_AVIS (WITH2_VEC (arg_node)));
        WITH2_VEC (arg_node) = FREEdoFreeTree (WITH2_VEC (arg_node));
        WITH2_VEC (arg_node) = expr;
    }

    /*
     * Traverse withid to allocate memory for the index scalars
     */
    WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *EMALwithid( node *arg_node, info *arg_info)
 *
 ****************************************************************************/
node *
EMALwithid (node *arg_node, info *arg_info)
{
    node *expr, *ids;

    DBUG_ENTER ("EMALwithid");

    /*
     * Do not allocate memory for the index vector as its shape may not
     * be statically known.
     */

    /*
     * Allocate memory for the index variables
     * and replace WITH_IDS with exprs chain of id.
     */
    expr = NULL;
    ids = WITHID_IDS (arg_node);
    while (ids != NULL) {
        INFO_ALLOCLIST (arg_info)
          = MakeALS (INFO_ALLOCLIST (arg_info), IDS_AVIS (ids), TBmakeNum (0),
                     TCcreateZeroVector (0, T_int));

        expr = TCappendExprs (expr, TBmakeExprs (TBmakeId (IDS_AVIS (ids)), NULL));
        ids = IDS_NEXT (ids);
    }

    if (WITHID_IDS (arg_node) != NULL) {
        WITHID_IDS (arg_node) = FREEdoFreeTree (WITHID_IDS (arg_node));
        WITHID_IDS (arg_node) = expr;
    }

    /*
     * Allocate memory for the offset scalars
     * and replace WITHID_IDXS with exprs chain of id.
     */
    expr = NULL;
    ids = WITHID_IDXS (arg_node);
    while (ids != NULL) {
        INFO_ALLOCLIST (arg_info)
          = MakeALS (INFO_ALLOCLIST (arg_info), IDS_AVIS (ids), TBmakeNum (0),
                     TCcreateZeroVector (0, T_int));

        expr = TCappendExprs (expr, TBmakeExprs (TBmakeId (IDS_AVIS (ids)), NULL));
        ids = IDS_NEXT (ids);
    }

    if (WITHID_IDXS (arg_node) != NULL) {
        WITHID_IDXS (arg_node) = FREEdoFreeTree (WITHID_IDXS (arg_node));
        WITHID_IDXS (arg_node) = expr;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn node *EMALgenarray( node *arg_node, info *arg_info)
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

    DBUG_ASSERT (INFO_ALLOCLIST (arg_info) != NULL,
                 "ALLOCLIST must contain an entry for each WITHOP!");

    /*
     * Pop first element from alloclist for traversal of next WITHOP
     */
    als = INFO_ALLOCLIST (arg_info);
    INFO_ALLOCLIST (arg_info) = als->next;
    als->next = NULL;

    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    if (INFO_WITHOPMODE (arg_info) == EA_memname) {

        /*
         * Create new identifier for new memory
         */
        wlavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (als->avis)),
                             TYeliminateAKV (AVIS_TYPE (als->avis)));

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (wlavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

        als->avis = wlavis;

        /*
         * Annotate which memory is to be used
         */
        GENARRAY_MEM (arg_node) = TBmakeId (wlavis);

        /*
         * Restore first element of alloclist as it is needed in EMALcode
         * to preserve correspondence between the result values and the withops
         */
        als->next = INFO_ALLOCLIST (arg_info);
        INFO_ALLOCLIST (arg_info) = als;
    } else {
        DBUG_ASSERT (INFO_WITHOPMODE (arg_info) == EA_shape,
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
              = TCmakePrf1 (F_shape_A,
                            TCmakePrf2 (F_genarray,
                                        DUPdoDupNode (GENARRAY_SHAPE (arg_node)),
                                        DUPdoDupNode (GENARRAY_DEFAULT (arg_node))));
        }

        /*
         * genarray-wl:
         * Allocation must remain in ALLOCLIST
         */
        als->next = INFO_ALLOCLIST (arg_info);
        INFO_ALLOCLIST (arg_info) = als;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn node *EMALmodarray( node *arg_node, info *arg_info)
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

    DBUG_ASSERT (INFO_ALLOCLIST (arg_info) != NULL,
                 "ALLOCLIST must contain an entry for each WITHOP!");

    /*
     * Pop first element from alloclist for traversal of next WITHOP
     */
    als = INFO_ALLOCLIST (arg_info);
    INFO_ALLOCLIST (arg_info) = als->next;
    als->next = NULL;

    MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

    if (INFO_WITHOPMODE (arg_info) == EA_memname) {

        /*
         * Create new identifier for new memory
         */
        wlavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (als->avis)),
                             TYeliminateAKV (AVIS_TYPE (als->avis)));

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (wlavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

        als->avis = wlavis;

        /*
         * Annotate which memory is to be used
         */
        MODARRAY_MEM (arg_node) = TBmakeId (wlavis);

        /*
         * Restore first element of alloclist as it is needed in EMALcode
         * to preserve correspondence between the result values and the withops
         */
        als->next = INFO_ALLOCLIST (arg_info);
        INFO_ALLOCLIST (arg_info) = als;
    } else {
        DBUG_ASSERT (INFO_WITHOPMODE (arg_info) == EA_shape,
                     "Unknown Withop traversal mode");
        /*
         * modarray-wl:
         * dim, shape are the same as in the modified array
         */
        als->dim = TCmakePrf1 (F_dim_A, DUPdoDupNode (MODARRAY_ARRAY (arg_node)));
        als->shape = TCmakePrf1 (F_shape_A, DUPdoDupNode (MODARRAY_ARRAY (arg_node)));

        /*
         * modarray-wl:
         * Allocation must remain in ALLOCLIST
         */
        als->next = INFO_ALLOCLIST (arg_info);
        INFO_ALLOCLIST (arg_info) = als;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn node *EMALbreak( node *arg_node, info *arg_info)
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
EMALbreak (node *arg_node, info *arg_info)
{
    node *wlavis;
    alloclist_struct *als;

    DBUG_ENTER ("EMALbreak");

    DBUG_ASSERT (INFO_ALLOCLIST (arg_info) != NULL,
                 "ALLOCLIST must contain an entry for each WITHOP!");

    /*
     * Pop first element from alloclist for traversal of next WITHOP
     */
    als = INFO_ALLOCLIST (arg_info);
    INFO_ALLOCLIST (arg_info) = als->next;
    als->next = NULL;

    BREAK_NEXT (arg_node) = TRAVopt (BREAK_NEXT (arg_node), arg_info);

    if (INFO_WITHOPMODE (arg_info) == EA_memname) {

        /*
         * Create new identifier for new memory
         */
        wlavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (als->avis)),
                             TYeliminateAKV (AVIS_TYPE (als->avis)));

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (wlavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

        als->avis = wlavis;

        /*
         * Annotate which memory is to be used
         */
        BREAK_MEM (arg_node) = TBmakeId (wlavis);

        /*
         * Restore first element of alloclist as it is needed in EMALcode
         * to preserve correspondence between the result values and the withops
         */
        als->next = INFO_ALLOCLIST (arg_info);
        INFO_ALLOCLIST (arg_info) = als;
    } else {
        DBUG_ASSERT (INFO_WITHOPMODE (arg_info) == EA_shape,
                     "Unknown Withop traversal mode");
        /*
         * break-withop:
         * dim = 0, shape = []
         */
        als->dim = TBmakeNum (0);
        als->shape = TCmakeIntVector (NULL);

        /*
         * break:
         * Allocation must remain in ALLOCLIST
         */
        als->next = INFO_ALLOCLIST (arg_info);
        INFO_ALLOCLIST (arg_info) = als;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn node *EMALpropagate( node *arg_node, info *arg_info)
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
EMALpropagate (node *arg_node, info *arg_info)
{
    alloclist_struct *als;

    DBUG_ENTER ("EMALpropagate");

    DBUG_ASSERT (INFO_ALLOCLIST (arg_info) != NULL,
                 "ALLOCLIST must contain an entry for each WITHOP!");

    /*
     * Pop first element from alloclist for traversal of next WITHOP
     */
    als = INFO_ALLOCLIST (arg_info);
    INFO_ALLOCLIST (arg_info) = als->next;
    als->next = NULL;

    PROPAGATE_NEXT (arg_node) = TRAVopt (PROPAGATE_NEXT (arg_node), arg_info);

    if (INFO_WITHOPMODE (arg_info) == EA_memname) {
        /*
         * Restore first element of alloclist as it is needed in EMALcode
         * to preserve correspondence between the result values and the withops
         */
        als->next = INFO_ALLOCLIST (arg_info);
        INFO_ALLOCLIST (arg_info) = als;
    } else {
        DBUG_ASSERT (INFO_WITHOPMODE (arg_info) == EA_shape,
                     "Unknown Withop traversal mode");
        /*
         * propagate-withop:
         * dim = 0, shape = [] (object)
         * wrong! should not alloc at all!
         */
        als->dim = TBmakeNum (0);
        als->shape = TCmakeIntVector (NULL);

        /*
         * propagate:
         * Allocation is removed from ALLOCLIST, we don't want it alloc'ed
         */
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn node *EMALfold( node *arg_node, info *arg_info)
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

    DBUG_ASSERT (INFO_ALLOCLIST (arg_info) != NULL,
                 "ALLOCLIST must contain an entry for each WITHOP!");

    /*
     * Pop first element from alloclist for traversal of next WITHOP
     */
    als = INFO_ALLOCLIST (arg_info);
    INFO_ALLOCLIST (arg_info) = als->next;
    als->next = NULL;

    FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);

    if (INFO_WITHOPMODE (arg_info) == EA_memname) {
        /*
         * Restore first element of alloclist as it is still needed in EMALcode
         * to preserve correspondence between the result values and the withops
         */
        als->next = INFO_ALLOCLIST (arg_info);
        INFO_ALLOCLIST (arg_info) = als;
    } else {
        DBUG_ASSERT (INFO_WITHOPMODE (arg_info) == EA_shape,
                     "Unknown Withop traversal mode");
        /*
         * Ultimately, fold-wls allocate memory on their own and thus don't need
         * further storage
         */
        als = FreeALS (als);
    }

    DBUG_RETURN (arg_node);
}

node *
EMALwith3 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMALwith3");

    /*
     * Annoate destination memory by traversing WITHOPS
     */
    INFO_WITHOPMODE (arg_info) = EA_memname;
    WITH3_OPERATIONS (arg_node) = TRAVdo (WITH3_OPERATIONS (arg_node), arg_info);

    /*
     * In order to build a proper suballoc/fill combinations in each Range-Block
     * it is necessary to know which result variables refer to
     * genarray/modarray - withops
     * For a with3, the index vector is different for each range, so we
     * generate the code template there.
     */
    INFO_WITHOPS (arg_info) = WITH3_OPERATIONS (arg_node);
    INFO_INDEXVECTOR (arg_info) = NULL;

    /*
     * now traverse the code in all ranges
     */
    WITH3_RANGES (arg_node) = TRAVdo (WITH3_RANGES (arg_node), arg_info);

    /*
     * Collect shape/dim information of the with3 results
     */
    INFO_WITHOPMODE (arg_info) = EA_shape;
    WITH3_OPERATIONS (arg_node) = TRAVdo (WITH3_OPERATIONS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
EMALrange (node *arg_node, info *arg_info)
{
    alloclist_struct *als;
    node *withops, *assign, *ids;

    DBUG_ENTER ("EMALrange");

    /*
     * Rescue ALLOCLIST and WITHOPS
     */
    als = INFO_ALLOCLIST (arg_info);
    INFO_ALLOCLIST (arg_info) = NULL;
    withops = INFO_WITHOPS (arg_info);
    INFO_WITHOPS (arg_info) = NULL;

    /*
     * Traverse block
     */
    RANGE_BODY (arg_node) = TRAVdo (RANGE_BODY (arg_node), arg_info);

    /*
     * Restore ALLOCLIST, WITHOPS and INDEXVECTOR
     */
    INFO_ALLOCLIST (arg_info) = als;
    INFO_WITHOPS (arg_info) = withops;

    /*
     * Shape information for each result array must be set and
     * suballoc/fill(copy,...) must be inserted. As a prerequisite,
     * we have to construct a code template for the
     * index vector.
     */
    INFO_INDEXVECTOR (arg_info) = TCmakeIntVector (TCids2Exprs (RANGE_INDEX (arg_node)));
    assign = AmendWithLoopCode (INFO_WITHOPS (arg_info), RANGE_IDXS (arg_node),
                                RANGE_RESULTS (arg_node), arg_info);
    /*
     * the template and the special handle to the indices is not needed
     * anymore, so lets free both
     */
    INFO_INDEXVECTOR (arg_info) = FREEdoFreeTree (INFO_INDEXVECTOR (arg_info));
    RANGE_IDXS (arg_node) = FREEdoFreeTree (RANGE_IDXS (arg_node));

    if (assign != NULL) {
        BLOCK_INSTR (RANGE_BODY (arg_node))
          = TCappendAssign (BLOCK_INSTR (RANGE_BODY (arg_node)), assign);
    }

    /*
     * allocate memory for the index and replace the ids by an id.
     * NOTE: we do not allocate memory for the IDXS here, as they
     * other than for withids have an actual computation associated
     * with them (i.e., a defining site), which will trigger their
     * allocation in the range body.
     */
    ids = RANGE_INDEX (arg_node);
    INFO_ALLOCLIST (arg_info) = MakeALS (INFO_ALLOCLIST (arg_info), IDS_AVIS (ids),
                                         TBmakeNum (0), TCcreateZeroVector (0, T_int));
    RANGE_INDEX (arg_node) = TBmakeId (IDS_AVIS (ids));
    ids = FREEdoFreeNode (ids);

    /*
     * go to next range
     */
    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @}
 */

/**
 * @name Entry functions.
 *
 * @{
 */

/** <!--******************************************************************-->
 *
 * @fn node *EMALdoAlloc( node *syntax_tree)
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

/**
 * @}
 */
