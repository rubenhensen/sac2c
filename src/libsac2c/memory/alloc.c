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
#include "constants.h"
#include "string.h"
#include "new_typecheck.h"
#include "globals.h"

#define DBUG_PREFIX "EMAL"
#include "debug.h"

#include "cuda_utils.h"
#include "ctinfo.h"

/**
 * Controls wheter AKS-Information should be used when possible
 */
#define USEAKS

/**
 * Enumeration of the different traversal modes for WITHOPs
 */
typedef enum { EA_memname, EA_shape } ea_withopmode;

/**
 * Enumeration of the different traversal modes for RANGESs
 */
typedef enum { EA_body, EA_index } ea_rangemode;

/**
 * Enumeration of the different allocation/fill modes for ALLOCLIST
 */
typedef enum { EA_nofill, EA_fill, EA_fillnoop } ea_fillmode;

/**
 * Structure used for ALLOCLIST.
 */
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
    bool inwiths;
    ea_withopmode withopmode;
    ea_rangemode rangemode;
    ea_fillmode mustfill;
};

/**
 * INFO macros
 */
#define INFO_ALLOCLIST(n) ((n)->alloclist)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_WITHOPS(n) ((n)->withops)
#define INFO_INDEXVECTOR(n) ((n)->indexvector)
#define INFO_INWITHS(n) ((n)->inwiths)
#define INFO_WITHOPMODE(n) ((n)->withopmode)
#define INFO_RANGEMODE(n) ((n)->rangemode)
#define INFO_MUSTFILL(n) ((n)->mustfill)

/**
 * @name INFO functions
 *
 * @{
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ALLOCLIST (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_WITHOPS (result) = NULL;
    INFO_INDEXVECTOR (result) = NULL;
    INFO_MUSTFILL (result) = EA_nofill;
    INFO_INWITHS (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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

    DBUG_ENTER ();

    res = (alloclist_struct *)MEMmalloc (sizeof (alloclist_struct));

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
    DBUG_ENTER ();

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

#if 0
/** <!--******************************************************************-->
 *
 * @fn bool AlloclistContains( alloclist_struct *als, node *avis)
 *
 *  @brief Checks wheter the given alloclist contains an entry for the
 *         designated element.
 *
 ***************************************************************************/
static bool AlloclistContains( alloclist_struct *als, node *avis)
{
  bool res;

  DBUG_ENTER ();

  if ( als == NULL) {
    res = FALSE;
  }
  else {
    if (als->avis == avis) {
      res = TRUE;
    }
    else {
      res = AlloclistContains( als->next, avis);
    }
  }

  DBUG_RETURN (res);
}
#endif

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

    DBUG_ENTER ();

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
    DBUG_ENTER ();

    switch (NODE_TYPE (arg)) {
    case N_numbyte:
    case N_numshort:
    case N_numint:
    case N_numlong:
    case N_numlonglong:
    case N_numubyte:
    case N_numushort:
    case N_numuint:
    case N_numulong:
    case N_numulonglong:
    case N_num:
    case N_float:
    /* FIXME  Not sure it should be 0...  */
    case N_floatvec:

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

    case N_typedef:
    case N_type:
        DBUG_UNREACHABLE ("typedef");
        break;

    default:
        DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg));
        DBUG_UNREACHABLE ("Invalid argument");
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
    DBUG_ENTER ();

    switch (NODE_TYPE (arg)) {
    case N_numbyte:
    case N_numshort:
    case N_numint:
    case N_numlong:
    case N_numlonglong:
    case N_numubyte:
    case N_numushort:
    case N_numuint:
    case N_numulong:
    case N_numulonglong:
    case N_num:
    case N_float:
    /* FIXME Still not sure that it should be 0...  */
    case N_floatvec:
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
        DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg));
        DBUG_UNREACHABLE ("Invalid argument");
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
    DBUG_ENTER ();

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
        DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg));
        DBUG_UNREACHABLE ("Invalid argument");
    }

    DBUG_RETURN (arg);
}

static bool
ASTisScalar (node *ast)
{
    bool res;
    ntype *atype;

    DBUG_ENTER ();

    atype = NTCnewTypeCheck_Expr (ast);
    res = (TYgetDim (atype) == 0);
    atype = TYfreeType (atype);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @brief Adds a suballoc/fill combination for the withops. Furthermore,
 *        shape and dimensionality information is added to the alloclist
 *        if possible.
 *        Lifted from EMALcode.
 *
 * @param withops   withops of this with loop
 * @param with3     amending a with3 loop
 * @param idxs      wl idxs for with3, NULL otherwise
 * @param chunksize chunksize for with3, NULL otherwise
 * @param cexprs    result expressions
 * @param arg_info  info node used to access INFO_INDEXVECTOR, INFO_ALLOCLIST.
 *
 * @return N_assign change to be prepended to the body of this part
 ******************************************************************************/
static node *
AmendWithLoopCode (node *withops, bool with3, node *idxs, node *chunksize, node *cexprs,
                   info *arg_info)
{
    node *valavis, *cexavis, *indexvector;
    node *wlidx = NULL;
    node *memavis = NULL;
    ntype *crestype = NULL;
    node *assign = NULL;
    int dim;
    alloclist_struct *als;

    DBUG_ENTER ();

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
         *
         * NOTE: for with3 loops with a chunksize, the result
         *       shape is not outershape ++ innershape, as the outer-most
         *       dimension of the inner shape is a partition of the outer
         *       shape. Thus, we have to drop the first element in these
         *       cases.
         */
        if (NODE_TYPE (withops) == N_genarray) {
            if (als->dim == NULL) {
                if (TUdimKnown (AVIS_TYPE (cexavis))) {
                    dim = TYgetDim (AVIS_TYPE (cexavis));
                    if (with3) {
                        if (chunksize != NULL) {
                            /*
                             * as computing in current dim drop 1 from the dim
                             * as we have a chunk size we are computing a group of values
                             * so add one to the dim to hold the set of results AKA: no
                             * change
                             */
                            dim += 1 - 1;
                        } else {
                            dim += -1;
                        }
                    }

                    als->dim
                      = TCmakePrf2 (F_add_SxS, MakeSizeArg (GENARRAY_SHAPE (withops)),
                                    TBmakeNum (dim));
                }
            }
            if (als->shape == NULL) {
                if (TUshapeKnown (AVIS_TYPE (cexavis))) {
                    shape *shape = TYgetShape (AVIS_TYPE (cexavis));

                    if (chunksize != NULL) {
                        shape = SHdropFromShape (1, shape);
                    }

                    als->shape
                      = TCmakePrf2 (F_cat_VxV, DUPdoDupNode (GENARRAY_SHAPE (withops)),
                                    SHshape2Array (shape));

                    if (chunksize != NULL) {
                        shape = SHfreeShape (shape);
                    }
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
         *
         *   For a with3, the encoding is different. The shape
         *   of the actual default element is
         *
         *   SHAPEEXPR ++ shape( default)
         *
         *   and to make things worse, the shape of the cexpr, if chunksize
         *   is not NULL, is
         *
         *   [CHUNKSIZE] ++ [SHAPEEXPR] ++ default
         *
         *   we have to take all this into account when computing
         *   crestype.
         */

        /*
         * N_genarray
         */
        if ((NODE_TYPE (withops) == N_genarray) && (GENARRAY_DEFAULT (withops) != NULL)) {
            DBUG_ASSERT (NODE_TYPE (GENARRAY_DEFAULT (withops)) == N_id,
                         "found a non flattened default expression!");

            /*
             * use more special type of default or cexpression
             */
            if (TUleShapeInfo (AVIS_TYPE (cexavis),
                               ID_NTYPE (GENARRAY_DEFAULT (withops)))) {
                crestype = TYcopyType (AVIS_TYPE (cexavis));
            } else {
                /*
                 * we have to compute the actual default type
                 */
                if (GENARRAY_DEFSHAPEEXPR (withops) == NULL) {
                    /*
                     * no shape expression, this is a simple with/with2
                     */
                    crestype = TYcopyType (ID_NTYPE (GENARRAY_DEFAULT (withops)));
                } else {
                    ntype *deftype = ID_NTYPE (GENARRAY_DEFAULT (withops));
                    node *sexpr = GENARRAY_DEFSHAPEEXPR (withops);

                    /*
                     * a shape expression, this is a with3
                     *
                     * To build an AKS type, we need an AKS default, constant
                     * SHAPEEXPR and constant CHUNKSIZE, if present
                     */
                    if (TUshapeKnown (deftype)
                        && ((chunksize == NULL) || (NODE_TYPE (chunksize) == N_num))
                        && COisConstant (sexpr)) {
                        node *exprs = ARRAY_AELEMS (sexpr);
                        int pos = 0;
                        shape *lshape;

                        lshape = SHmakeShape (TCcountExprs (exprs)
                                              + ((chunksize == NULL) ? 0 : 1));

                        while (exprs != NULL) {
                            lshape
                              = SHsetExtent (lshape, pos, NUM_VAL (EXPRS_EXPR (exprs)));
                            exprs = EXPRS_NEXT (exprs);
                            pos++;
                        }

                        if (chunksize != NULL) {
                            lshape = SHsetExtent (lshape, pos, NUM_VAL (chunksize));
                            pos++;
                        }

                        crestype
                          = TYmakeAKS (TYcopyType (TYgetScalar (deftype)),
                                       SHappendShapes (lshape, TYgetShape (deftype)));

                        lshape = SHfreeShape (lshape);
                    } else if (TUdimKnown (deftype)) {
                        /*
                         * We can build an AKD
                         */
                        /* This warning with TCcountExprs is due to TYmakeAKD taking 'int dots' 
                         * and TYgetDim int returns from Shape related function, COgetDim and SHgetDim,
                         * so leaving this warning for now until new datatype is introduced
                         */
                        crestype = TYmakeAKD (TYcopyType (TYgetScalar (deftype)),
                                              TCcountExprs (ARRAY_AELEMS (sexpr))
                                                + ((chunksize == NULL) ? 0 : 1)
                                                + TYgetDim (deftype),
                                              SHmakeShape (0));
                    } else {
                        /*
                         * if it is AUD, we can just use the original crestype
                         */
                        crestype = TYcopyType (AVIS_TYPE (cexavis));
                    }
                }
            }

            /*
             * N_modarray:
             */
        } else if ((NODE_TYPE (withops) == N_modarray)
                   && (!TUshapeKnown (AVIS_TYPE (cexavis)))) {
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
                    DBUG_PRINT ("subvar %s is Scalar", AVIS_NAME (als->avis));

                    crestype
                      = TYmakeAKS (TYcopyType (TYgetScalar (restype)), SHmakeShape (0));
                } else {
                    /*
                     * non scalar!
                     */
                    if (TUshapeKnown (restype)) {
                        DBUG_PRINT ("subvar of %s is AKS", AVIS_NAME (als->avis));

                        crestype
                          = TYmakeAKS (TYcopyType (TYgetScalar (restype)),
                                       SHdropFromShape (SHgetExtent (TYgetShape (ivtype),
                                                                     0),
                                                        TYgetShape (restype)));
                    } else {
                        DBUG_PRINT ("subvar of %s is AKD", AVIS_NAME (als->avis));

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
            node *args = NULL;

            if (TUdimKnown (crestype) && (TYgetDim (crestype) == 0)) {
                node *prfap;

                /*
                 * Create a new value variable
                 * Ex: a_val
                 */
                valavis = TBmakeAvis (TRAVtmpVarName ("val"), TYcopyType (crestype));
                DBUG_PRINT ("Created new scalar cell value variable %s",
                            AVIS_NAME (valavis));

                FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (valavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

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
                DBUG_PRINT ("Created new non-scalar cell value variable %s",
                            AVIS_NAME (memavis));

                FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (memavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

                /*
                 * Create a new value variable
                 * Ex: a_val
                 */
                valavis = TBmakeAvis (TRAVtmpVarName ("val"), TYcopyType (crestype));

                FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (valavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

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
                 * if the shape of a_mem is not statically known, we will
                 * have to compute it at runtime.
                 *
                 * For N_genarray withloops, this can be done using the
                 * default element. Thus, we annotate a alloc-style shape
                 * expression here if it is needed.
                 *
                 * For N_modarray withloops, the shape is still set by some
                 * backend magic. However, this won't work with the mutc
                 * backend -> TODO MUTC
                 *
                 * Ex:
                 * {
                 *   ...
                 *   a_mem = suballoc( A, idx, cdim, \[ shape \]);
                 *   a_val = fill( copy( a), a_mem);
                 * }: a_val;
                 */
                if ((NODE_TYPE (withops) == N_genarray)
                    && !TUshapeKnown (AVIS_TYPE (memavis))) {
                    node *genshape = NULL;

                    DBUG_ASSERT (GENARRAY_DEFAULT (withops) != NULL,
                                 "default element required!");

                    /*
                     * first, add in the default shape expression
                     */
                    if (GENARRAY_DEFSHAPEEXPR (withops) != NULL) {
                        DBUG_ASSERT (NODE_TYPE (GENARRAY_DEFSHAPEEXPR (withops))
                                       == N_array,
                                     "default-shape expression needs to be a vector!");
                        genshape
                          = DUPdoDupTree (ARRAY_AELEMS (GENARRAY_DEFSHAPEEXPR (withops)));
                    }
                    /*
                     * add outermost dimension of chunked
                     */
                    if (chunksize != NULL) {
                        genshape = TBmakeExprs (DUPdoDupNode (chunksize), genshape);
                    }

                    if ((genshape != NULL) && ASTisScalar (GENARRAY_DEFAULT (withops))) {
                        args = TBmakeExprs (TCmakeIntVector (genshape), NULL);
                    } else if (genshape != NULL) {
                        args = TBmakeExprs (TCmakePrf1 (F_shape_A,
                                                        TCmakePrf2 (F_genarray,
                                                                    TCmakeIntVector (
                                                                      genshape),
                                                                    DUPdoDupNode (
                                                                      GENARRAY_DEFAULT (
                                                                        withops)))),
                                            NULL);
                    } else {
                        args = TBmakeExprs (TCmakePrf1 (F_shape_A,
                                                        DUPdoDupNode (
                                                          GENARRAY_DEFAULT (withops))),
                                            NULL);
                    }
                }
                args = TBmakeExprs (TBmakeId (als->avis),
                                    TBmakeExprs (TBmakeId (wlidx),
                                                 TBmakeExprs ((chunksize != NULL)
                                                                ? TBmakeNum (0)
                                                                : TBmakeNum (1),
                                                              args)));

                assign = TBmakeAssign (TBmakeLet (TBmakeIds (memavis, NULL),
                                                  TBmakePrf (F_suballoc, args)),
                                       assign);

                AVIS_SSAASSIGN (memavis) = assign;
            }
        }

        if ((NODE_TYPE (withops) == N_fold) && FOLD_ISPARTIALFOLD (withops)) {
            node *prf, *avis, *tmp_exprs;
            avis = cexavis;

            while (AVIS_SSAASSIGN (avis) != NULL) {
                prf = ASSIGN_RHS (AVIS_SSAASSIGN (avis));
                DBUG_ASSERT ((NODE_TYPE (prf) == N_prf
                              && PRF_PRF (prf) == F_cond_wl_assign),
                             "Result of partial fold is not defined by F_cond_wl_assign");
                tmp_exprs = TBmakeExprs (DUPdoDupNode (FOLD_PARTIALMEM (withops)),
                                         EXPRS_EXPRS5 (PRF_ARGS (prf)));
                EXPRS_NEXT (EXPRS_EXPRS4 (PRF_ARGS (prf))) = tmp_exprs;
                avis = ID_AVIS (PRF_ARG6 (prf));
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
 *  allocate memory which must be filled. When we encounter a loop function
 *  we interate through all its arguments looking for any N_avis which was
 *  added with the EMR loop optimisation phase - when we find one, we add it
 *  to the ALLOCLIST.
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
    node * args;

    DBUG_ENTER ();

    /* we do not allocate the LHS */
    INFO_ALLOCLIST (arg_info) = FreeALS (INFO_ALLOCLIST (arg_info));

    if (FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node))) {
        /*
         * With EMR loop optimisation, we lift WL array allocations out of
         * the loop function body. Giving us:
         *
         * type[SIZE] emr_lift;
         *
         * LHS = loopfun (a, b, ..., emr_lift);
         *
         * We need to transform it into:
         *
         * type[SIZE] emr_lift;
         *
         * emr_lift = alloc (dim (a), shape (a));
         * LHS = loopfun (a, b, ..., emr_lift);
         */
        args = AP_ARGS (arg_node);

        while (args != NULL) {
            if (AVIS_ISALLOCLIFT (ID_AVIS (EXPRS_EXPR (args)))) {
                INFO_ALLOCLIST (arg_info)
                    = MakeALS (INFO_ALLOCLIST (arg_info), ID_AVIS (EXPRS_EXPR (args)),
                               MakeDimArg (EXPRS_EXPR (args)), MakeShapeArg (EXPRS_EXPR (args)));
                AVIS_ISALLOCLIFT (ID_AVIS (EXPRS_EXPR (args))) = FALSE;
            }
            args = EXPRS_NEXT (args);
        }
    }

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

    DBUG_ENTER ();

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
    INFO_MUSTFILL (arg_info) = EA_fill;

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

    DBUG_ENTER ();

    /*
     * Bottom-up traversal
     */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /*
     * Traverse RHS of assignment
     */
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

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

    DBUG_ENTER ();

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
    assign = AmendWithLoopCode (INFO_WITHOPS (arg_info), FALSE, NULL, NULL,
                                CODE_CEXPRS (arg_node), arg_info);

    if (assign != NULL) {
        BLOCK_ASSIGNS (CODE_CBLOCK (arg_node))
          = TCappendAssign (BLOCK_ASSIGNS (CODE_CBLOCK (arg_node)), assign);
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
        DBUG_ENTER ();                                                                   \
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
            INFO_MUSTFILL (arg_info) = EA_fill;                                          \
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
EMALCONST (floatvec)
EMALCONST (double)
EMALCONST (num)
EMALCONST (numbyte)
EMALCONST (numshort)
EMALCONST (numint)
EMALCONST (numlong)
EMALCONST (numlonglong)
EMALCONST (numubyte)
EMALCONST (numushort)
EMALCONST (numuint)
EMALCONST (numulong)
EMALCONST (numulonglong)

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
        switch (INFO_MUSTFILL (arg_info)) {
        case EA_fill:
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

            /* propogate page-locked flag */
            AVIS_ISCUDAPINNED (avis) = AVIS_ISCUDAPINNED (IDS_AVIS (LET_IDS (arg_node)));

            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            LET_EXPR (arg_node)
              = TCmakePrf2 (F_fill, LET_EXPR (arg_node), TBmakeId (avis));

            /*
             * Set the avis of the freshly allocated variable
             */
            INFO_ALLOCLIST (arg_info)->avis = avis;
            break;
        case EA_fillnoop:
            /*
             * b = prf (a);
             *
             * is transferred into:
             *
             * a = alloc (...)
             * b = fill (noop (a), a);
             */
            avis = ID_AVIS (PRF_ARG1 (LET_EXPR (arg_node)));

            /* we intentionally update the N_avis type to match the LHS type */
            AVIS_TYPE (avis) = TYeliminateAKV (AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node))));

            /* we remove the prf operation, replace with fill */
            LET_EXPR (arg_node) = FREEdoFreeTree (LET_EXPR (arg_node));
            LET_EXPR (arg_node)
              = TCmakePrf2 (F_fill, TCmakePrf1 (F_noop, TBmakeId (avis)),
                            TBmakeId (avis));

            /* make sure to allocate N_avis */
            INFO_ALLOCLIST (arg_info)->avis = avis;
        case EA_nofill:
            /* do nothing */
            break;
        default:
            DBUG_UNREACHABLE ("Invalid EMAL fill mode!");
        }
        INFO_MUSTFILL (arg_info) = EA_nofill;
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

    DBUG_ENTER ();

    als = INFO_ALLOCLIST (arg_info);

    /*
     * Signal EMALlet to wrap this prf in a fill-operation
     */
    INFO_MUSTFILL (arg_info) = EA_fill;

#if 0
  /* to be done */
  alloc_funtab[PRF_PRF( arg_node)]( arg_node, arg_info);
#endif

    switch (PRF_PRF (arg_node)) {
    case F_dim_A:
    case F_size_A:
        /*
         * dim( A );   -or-   size( A );
         * alloc( 0, [] );
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_isDist_A:
        /*
         * _isDist_A_ always yields a scalar
         *
         * _isDist_A_( A);
         * alloc( 0, []);
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_bool);
        break;

    case F_firstElems_A:
        /*
         * _firstElems_A_ always yields a scalar
         *
         * _firstElems_A_( A);
         * alloc( 0, []);
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_ulong);
        break;

    case F_localFrom_A:
        /*
         * _localFrom_A_ always yields a scalar
         *
         * _localFrom_A_( A);
         * alloc( 0, []);
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_ulong);
        break;

    case F_localCount_A:
        /*
         * _localCount_A_ always yields a scalar
         *
         * _localCount_A_( A);
         * alloc( 0, []);
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_ulong);
        break;

    case F_offs_A:
        /*
         * _offs_A_ always yields a scalar
         *
         * _offs_A_( A);
         * alloc( 0, []);
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_ulong);
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

    case F_sel_VxIA:
        // DBUG_UNREACHABLE ("IMPLEMENT THIS");
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
        /* If the type of the array is shared memory, we do not
         * attempt to reuse it. Instead, we create a normal F_alloc.
         * This F_alloc will eventually be removed from the cuda
         * kernel s*/
        if (CUisShmemTypeNew (ID_NTYPE (PRF_ARG1 (arg_node)))) {
            als->dim = MakeDimArg (PRF_ARG1 (arg_node));
            als->shape = MakeShapeArg (PRF_ARG1 (arg_node));
        } else {
            als->reuse = DUPdoDupNode (PRF_ARG1 (arg_node));
        }
        break;

    case F_hideValue_SxA:
    case F_hideShape_SxA:
    case F_hideDim_SxA:
        als->dim = MakeDimArg (PRF_ARG2 (arg_node));
        als->shape = MakeShapeArg (PRF_ARG2 (arg_node));
        arg_node = TCmakePrf1 (F_copy, DUPdoDupNode (PRF_ARG2 (arg_node)));
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

    case F_add_SxS:
    case F_sub_SxS:
    case F_mul_SxS:
    case F_div_SxS:
    case F_and_SxS:
    case F_or_SxS:
    case F_mod_SxS:
    case F_aplmod_SxS:
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
    case F_tobool_S:
    case F_tob_S:
    case F_tos_S:
    case F_toi_S:
    case F_tol_S:
    case F_toll_S:
    case F_toub_S:
    case F_tous_S:
    case F_toui_S:
    case F_toul_S:
    case F_toull_S:
    case F_toc_S:
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
    case F_aplmod_SxV:
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
    case F_aplmod_VxS:
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

    /* SIMD stuff  */
    case F_add_SMxSM:
    case F_sub_SMxSM:
    case F_mul_SMxSM:
    case F_div_SMxSM:
        /*
         * The first argument is a stupid scalar, so the second
         * or the third (as they are identical shape-wise) argument
         * is a right shape for the allocation.
         */
        als->dim = MakeDimArg (PRF_ARG2 (arg_node));
        als->shape = MakeShapeArg (PRF_ARG2 (arg_node));
        break;

    case F_simd_sel_SxS:
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_simd_modarray:
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_simd_sel_VxA: {
        node *simd_length;

        als->dim = MakeDimArg (TBmakeNum (1));

        /* 1-d vector, with the value defined in the first agument.  */
        simd_length = DUPdoDupTree (PRF_ARG1 (arg_node));
        als->shape = TCmakeIntVector (TBmakeExprs (simd_length, NULL));
        break;
    }

    case F_add_VxV:
    case F_sub_VxV:
    case F_mul_VxV:
    case F_div_VxV:
    case F_and_VxV:
    case F_or_VxV:
    case F_mod_VxV:
    case F_aplmod_VxV:
    case F_min_VxV:
    case F_max_VxV:
    case F_le_VxV:
    case F_lt_VxV:
    case F_eq_VxV:
    case F_neq_VxV:
    case F_ge_VxV:
    case F_gt_VxV:
        /* XXX Why does it matter if the type of the argument is id or not
         * it still has a shape, right?
         */
        if (NODE_TYPE (PRF_ARG2 (arg_node)) != N_id) {
            als->dim = MakeDimArg (PRF_ARG1 (arg_node));
            als->shape = MakeShapeArg (PRF_ARG1 (arg_node));
        } else {
            als->dim = MakeDimArg (PRF_ARG2 (arg_node));
            als->shape = MakeShapeArg (PRF_ARG2 (arg_node));
        }
        break;

    case F_mask_SxSxS:
    case F_mask_VxSxS:
    case F_mask_VxSxV:
    case F_mask_VxVxS:
    case F_mask_VxVxV:
        als->dim = MakeDimArg (PRF_ARG1 (arg_node));
        als->shape = MakeShapeArg (PRF_ARG1 (arg_node));
        break;

    case F_mask_SxVxS:
    case F_mask_SxVxV:
        als->dim = MakeDimArg (PRF_ARG2 (arg_node));
        als->shape = MakeShapeArg (PRF_ARG2 (arg_node));
        break;

    case F_mask_SxSxV:
        als->dim = MakeDimArg (PRF_ARG3 (arg_node));
        als->shape = MakeShapeArg (PRF_ARG3 (arg_node));
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
    case F_syncin:
    case F_syncout:
    case F_type_error:
    case F_type_conv:
    case F_dispatch_error:
    case F_unshare:
        /*
         * a,... = accu( iv, n, ...) (and prop_obj)
         * v = unshare( a, iv1,...,ivn)   (v is aliases a)
         * v,... = _type_error_( ...)
         * require a special treatment as
         * none of its return value must be allocated
         */
        INFO_ALLOCLIST (arg_info) = FreeALS (INFO_ALLOCLIST (arg_info));
        INFO_MUSTFILL (arg_info) = EA_nofill;
        break;

    case F_guard:
        /*
         * X' = guard( X, p);
         */
        als->dim = MakeDimArg (PRF_ARG1 (arg_node));
        als->shape = MakeShapeArg (PRF_ARG1 (arg_node));
        break;

    case F_afterguard:
        /*
         * v = afterguard(a,p1,...,pn)
         * - consumes p1...pn
         * - v is an alias of a_i
         */
        INFO_ALLOCLIST (arg_info) = FreeALS (INFO_ALLOCLIST (arg_info));
        INFO_MUSTFILL (arg_info) = EA_nofill;
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

    case F_val_le_val_SxS:
        als->dim = MakeDimArg (PRF_ARG1 (arg_node));
        als->shape = MakeShapeArg (PRF_ARG1 (arg_node));
        break;

    case F_same_shape_AxA:
    case F_shape_matches_dim_VxA:
    case F_non_neg_val_S:
    case F_non_neg_val_V:
    case F_val_lt_shape_VxA:
    case F_val_le_val_VxV:
    case F_val_lt_val_SxS:
    case F_prod_matches_prod_shape_VxA:
        /*
         * p = constraint(a1,..,an)
         * - a_i are consumed
         * - p is a boolean result
         */
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_cudamemprefetch:
        if (AVIS_ISALLOCLIFT (ID_AVIS (PRF_ARG1 (arg_node))))
        {
            /* rhs EMR lifted avis is never previously assigned to,
             * we need to explicitly allocate it.
             */
            als->avis = ID_AVIS (PRF_ARG1 (arg_node));
            als->dim = MakeDimArg (PRF_ARG1 (arg_node));
            als->shape = MakeShapeArg (PRF_ARG1 (arg_node));
        } else {
            /* we perform no allocation of the lhs */
            INFO_ALLOCLIST (arg_info) = FreeALS (INFO_ALLOCLIST (arg_info));
        }
        INFO_MUSTFILL (arg_info) = EA_nofill;
        break;

    case F_host2device:
        /*
         * CUDA MLTRAN identifies what N_avis needs to be transferred to the
         * CUDA device. Ordinarily its selections are sane, except when handling
         * EMR loop optimisation lifted N_avis. These are not assigned to in the
         * calling context, meaning that EMAL does not handle these (no alloc
         * materialises). We need to handle this separately, and do so by using
         * the alternative fill/alloc insertion in EMALlet (see EA_fillnoop). This
         * elides the host2device prf, instead using fill (noop (...), ...).
         */
        INFO_MUSTFILL (arg_info) = AVIS_ISALLOCLIFT (ID_AVIS (PRF_ARG1 (arg_node)))
                                     ? EA_fillnoop
                                     : INFO_MUSTFILL (arg_info);
        /* Fall-through */
    case F_device2host:
        als->dim = MakeDimArg (PRF_ARG1 (arg_node));
        als->shape = MakeShapeArg (PRF_ARG1 (arg_node));
        break;

    case F_cuda_threadIdx_x:
    case F_cuda_threadIdx_y:
    case F_cuda_threadIdx_z:
    case F_cuda_blockIdx_x:
    case F_cuda_blockIdx_y:
    case F_cuda_blockDim_x:
    case F_cuda_blockDim_y:
    case F_cuda_blockDim_z:
    case F_cuda_gridDim_x:
    case F_cuda_gridDim_y:
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_cond:
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;

    case F_cond_wl_assign:
        INFO_ALLOCLIST (arg_info) = FreeALS (INFO_ALLOCLIST (arg_info));
        INFO_MUSTFILL (arg_info) = EA_nofill;
        break;

    case F_shmem_boundary_load:
        als->dim = MakeDimArg (PRF_ARG2 (arg_node));
        als->shape = MakeShapeArg (PRF_ARG2 (arg_node));
        break;

    case F_shmem_boundary_check:
        als->dim = TBmakeNum (0);
        als->shape = TCcreateZeroVector (0, T_int);
        break;
    case F_syncthreads:
        als->dim = MakeDimArg (PRF_ARG1 (arg_node));
        als->shape = MakeShapeArg (PRF_ARG1 (arg_node));
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
    case F_run_mt_genarray:
    case F_run_mt_modarray:
    case F_run_mt_fold:

        DBUG_UNREACHABLE ("invalid prf found!");
        break;

    case F_sync:
        /*
         * require a special treatment as
         * its return value must not  be allocated
         */
        INFO_ALLOCLIST (arg_info) = FreeALS (INFO_ALLOCLIST (arg_info));
        INFO_MUSTFILL (arg_info) = EA_nofill;
        break;

    /*
     * Both enclose and disclose do not really create new structures.
     */
    case F_enclose:
    case F_disclose:
        INFO_ALLOCLIST (arg_info) = FreeALS (INFO_ALLOCLIST (arg_info));
        INFO_MUSTFILL (arg_info) = EA_nofill;
        break;

    default:
        DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node));
        DBUG_UNREACHABLE ("unknown prf found!");
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

    DBUG_ENTER ();

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
    /*
     * Only allocate the index vector once for every distributed with-loop.
     */
    if (!INFO_INWITHS (arg_info)) {
        INFO_ALLOCLIST (arg_info)
          = MakeALS (INFO_ALLOCLIST (arg_info), IDS_AVIS (WITH_VEC (arg_node)),
                     TBmakeNum (1), MakeShapeArg (WITH_BOUND1 (arg_node)));
    }

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
    /*
     if( PART_NEXT( WITH_PART( arg_node)) != NULL) {
     node *nextpart = PART_NEXT( WITH_PART( arg_node));
     PART_WITHID( nextpart) = FREEdoFreeNode( PART_WITHID( nextpart));
     PART_WITHID( nextpart) = DUPdoDupNode( WITH_WITHID( arg_node));
     }
     */
    node *nextpart = PART_NEXT (WITH_PART (arg_node));
    while (nextpart != NULL) {
        PART_WITHID (nextpart) = FREEdoFreeNode (PART_WITHID (nextpart));
        PART_WITHID (nextpart) = DUPdoDupNode (WITH_WITHID (arg_node));
        nextpart = PART_NEXT (nextpart);
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
    node *expr;

    DBUG_ENTER ();

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
        /*
         * Only allocate the index vector once for every distributed with-loop.
         */
        if (!INFO_INWITHS (arg_info)) {
            INFO_ALLOCLIST (arg_info)
              = MakeALS (INFO_ALLOCLIST (arg_info), IDS_AVIS (WITH2_VEC (arg_node)),
                         TBmakeNum (1),
                         SHshape2Array (TYgetShape (
                           AVIS_TYPE (IDS_AVIS (WITHID_VEC (WITH2_WITHID (arg_node)))))));
        }

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

    DBUG_ENTER ();

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
        /*
         * Only allocate the index variables once for every distributed with-loop.
         */
        if (!INFO_INWITHS (arg_info)) {
            INFO_ALLOCLIST (arg_info)
              = MakeALS (INFO_ALLOCLIST (arg_info), IDS_AVIS (ids), TBmakeNum (0),
                         TCcreateZeroVector (0, T_int));
        }

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
        /*
         * Only allocate the offset scalars once for every distributed with-loop.
         */
        if (!INFO_INWITHS (arg_info)) {
            INFO_ALLOCLIST (arg_info)
              = MakeALS (INFO_ALLOCLIST (arg_info), IDS_AVIS (ids), TBmakeNum (0),
                         TCcreateZeroVector (0, T_int));
        }

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
 * @fn node *EMALwiths( node *arg_node, info *arg_info)
 *
 *  @brief Traverse first with-loop, then set flag in info so that for the
 *         next with-loops we only look at the code nodes.
 *
 *  @param arg_node with-loop
 *  @param arg_info
 *
 *  @return with-loop
 *
 ***************************************************************************/
node *
EMALwiths (node *arg_node, info *arg_info)
{
    alloclist_struct *als, *end, *ids;

    DBUG_ENTER ();

    if (WITHS_NEXT (arg_node) != NULL) {
        /* save the start of the allocation list for the LHS of the let. */
        ids = INFO_ALLOCLIST (arg_info);

        /* get allocation list for this with-loop */
        WITHS_WITH (arg_node) = TRAVdo (WITHS_WITH (arg_node), arg_info);

        /*
         * for the next with-loop, we want to reset the allocation list to its
         * initial state, that is, with only the element corresponding to the LHS of
         * the let.
         */
        als = INFO_ALLOCLIST (arg_info);
        INFO_ALLOCLIST (arg_info) = ids;
        INFO_INWITHS (arg_info) = TRUE;

        WITHS_NEXT (arg_node) = TRAVdo (WITHS_NEXT (arg_node), arg_info);

        /* to restore the ALS, we go through the resulting list until we find the
         * first ids element. We then append the saved allocation list, which also
         * contains the ids elements in its tail */
        if (INFO_ALLOCLIST (arg_info) != ids) {
            for (end = INFO_ALLOCLIST (arg_info); end->next != ids; end = end->next)
                ;
            end->next = als;
        } else {
            INFO_ALLOCLIST (arg_info) = als;
        }
    } else {
        /* get allocation list for this with-loop */
        WITHS_WITH (arg_node) = TRAVdo (WITHS_WITH (arg_node), arg_info);

        INFO_INWITHS (arg_info) = FALSE;
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

    DBUG_ENTER ();

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
        /* We don't want to allocate a distributed array for each with-loop version.
         * We keep the allocations in a table, and reuse them as possible.
         */
        if (!INFO_INWITHS (arg_info)) {
            /*
             * Create new identifier for new memory
             */
            wlavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (als->avis)),
                                 TYeliminateAKV (AVIS_TYPE (als->avis)));

            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TBmakeVardec (wlavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            als->avis = wlavis;
        } else {
            wlavis = als->avis;
        }

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
         *
         * for with/with2, the result shape is
         *
         *   GENARRAY_SHAPE ++ shape( GENARRAY_DEFAULT)
         *
         * however, for with3 the shape is
         *
         *   GENARRAY_SHAPE ++ GENARRAY_SHAPEEXPR ++ shape( GENARRAY_DEFAULT)
         *
         * with the additional constraint that shape( GENARRAY_SHAPE) is [1]
         * and that GENARRAY_SHAPEEXPR is an N_array node.
         *
         */
        if (als->dim == NULL) {
            DBUG_ASSERT (GENARRAY_DEFAULT (arg_node) != NULL,
                         "Default element required!");
            if (GENARRAY_DEFSHAPEEXPR (arg_node) == NULL) {
                /* with / with2 */
                als->dim = TCmakePrf2 (F_add_SxS, MakeSizeArg (GENARRAY_SHAPE (arg_node)),
                                       MakeDimArg (GENARRAY_DEFAULT (arg_node)));
            } else {
                /* with3 */
                DBUG_ASSERT (TCcountExprs (ARRAY_AELEMS (GENARRAY_SHAPE (arg_node))) == 1,
                             "Illegal shape length in with3 genarray.");

                als->dim
                  = TCmakePrf2 (F_add_SxS, TBmakeNum (1),
                                TCmakePrf2 (F_add_SxS,
                                            MakeSizeArg (
                                              GENARRAY_DEFSHAPEEXPR (arg_node)),
                                            MakeDimArg (GENARRAY_DEFAULT (arg_node))));
            }
        }

        if (als->shape == NULL) {
            DBUG_ASSERT (GENARRAY_DEFAULT (arg_node) != NULL,
                         "Default element required!");

            if (GENARRAY_DEFSHAPEEXPR (arg_node) == NULL) {
                /* with/with2 */
                als->shape
                  = TCmakePrf1 (F_shape_A,
                                TCmakePrf2 (F_genarray,
                                            DUPdoDupNode (GENARRAY_SHAPE (arg_node)),
                                            DUPdoDupNode (GENARRAY_DEFAULT (arg_node))));
            } else {
                /* with3 */
                DBUG_ASSERT (NODE_TYPE (GENARRAY_SHAPE (arg_node)) == N_array,
                             "Illegal shape in genarray of with3");
                DBUG_ASSERT (NODE_TYPE (GENARRAY_DEFSHAPEEXPR (arg_node)) == N_array,
                             "Illegal defshapeexpr in genarray of with3");

                als->shape
                  = TCmakePrf1 (F_shape_A,
                                TCmakePrf2 (F_genarray,
                                            TCmakeIntVector (
                                              TCappendExprs (DUPdoDupTree (ARRAY_AELEMS (
                                                               GENARRAY_SHAPE (
                                                                 arg_node))),
                                                             DUPdoDupTree (ARRAY_AELEMS (
                                                               GENARRAY_DEFSHAPEEXPR (
                                                                 arg_node))))),
                                            DUPdoDupNode (GENARRAY_DEFAULT (arg_node))));
            }
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

    DBUG_ENTER ();

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

        /* We don't want to allocate a distributed array for each with-loop version.
         * We keep the allocations in a table, and reuse them as possible.
         */
        if (!INFO_INWITHS (arg_info)) {
            /*
             * Create new identifier for new memory
             */
            wlavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (als->avis)),
                                 TYeliminateAKV (AVIS_TYPE (als->avis)));

            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TBmakeVardec (wlavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            als->avis = wlavis;
        } else {
            wlavis = als->avis;
        }

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

    DBUG_ENTER ();

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

        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TBmakeVardec (wlavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

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

    DBUG_ENTER ();

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
    node *wlavis;

    DBUG_ENTER ();

    DBUG_ASSERT (INFO_ALLOCLIST (arg_info) != NULL,
                 "ALLOCLIST must contain an entry for each WITHOP!");

    /*
     * Pop first element from alloclist for traversal of next WITHOP
     */
    als = INFO_ALLOCLIST (arg_info);
    INFO_ALLOCLIST (arg_info) = als->next;
    als->next = NULL;

    FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);

    if (FOLD_ISPARTIALFOLD (arg_node)) {
        if (INFO_WITHOPMODE (arg_info) == EA_memname) {
            /*
             * Create new identifier for new memory
             */
            wlavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (als->avis)),
                                 TYeliminateAKV (AVIS_TYPE (als->avis)));

            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TBmakeVardec (wlavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            als->avis = wlavis;

            /*
             * Annotate which memory is to be used
             */
            FOLD_PARTIALMEM (arg_node) = TBmakeId (wlavis);

            /*
             * Restore first element of alloclist as it is needed in EMALcode
             * to preserve correspondence between the result values and the withops
             */
            als->next = INFO_ALLOCLIST (arg_info);
            INFO_ALLOCLIST (arg_info) = als;
        } else {
            DBUG_ASSERT (INFO_WITHOPMODE (arg_info) == EA_shape,
                         "Unknown Withop traversal mode");

            als->dim = TBmakeNum (TYgetDim (AVIS_TYPE (als->avis)));
            als->shape = SHshape2Array (TYgetShape (AVIS_TYPE (als->avis)));

            als->next = INFO_ALLOCLIST (arg_info);
            INFO_ALLOCLIST (arg_info) = als;
        }
    } else {
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
    }
    DBUG_RETURN (arg_node);
}

node *
EMALwith3 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

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
    INFO_RANGEMODE (arg_info) = EA_body;
    WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);

    /*
     * Collect shape/dim information of the with3 results
     */
    INFO_WITHOPMODE (arg_info) = EA_shape;
    WITH3_OPERATIONS (arg_node) = TRAVdo (WITH3_OPERATIONS (arg_node), arg_info);

    /*
     * Collect shape/dim information for the range index
     */
    INFO_RANGEMODE (arg_info) = EA_index;
    WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
EMALrange (node *arg_node, info *arg_info)
{
    alloclist_struct *als;
    node *withops, *assign, *ids;
    ea_rangemode mode;

    DBUG_ENTER ();

    if (INFO_RANGEMODE (arg_info) == EA_body) {
        /*
         * Rescue ALLOCLIST and WITHOPS and RANGEMODE
         */
        als = INFO_ALLOCLIST (arg_info);
        INFO_ALLOCLIST (arg_info) = NULL;
        withops = INFO_WITHOPS (arg_info);
        INFO_WITHOPS (arg_info) = NULL;
        mode = INFO_RANGEMODE (arg_info);

        /*
         * Traverse block
         */
        RANGE_BODY (arg_node) = TRAVdo (RANGE_BODY (arg_node), arg_info);

        /*
         * Restore ALLOCLIST, WITHOPS and RANGEMODE
         */
        INFO_ALLOCLIST (arg_info) = als;
        INFO_WITHOPS (arg_info) = withops;
        INFO_RANGEMODE (arg_info) = mode;

        /*
         * Shape information for each result array must be set and
         * suballoc/fill(copy,...) must be inserted. As a prerequisite,
         * we have to construct a code template for the
         * index vector.
         */
        INFO_INDEXVECTOR (arg_info)
          = TCmakeIntVector (TCids2Exprs (RANGE_INDEX (arg_node)));
        assign = AmendWithLoopCode (INFO_WITHOPS (arg_info), TRUE, RANGE_IDXS (arg_node),
                                    RANGE_CHUNKSIZE (arg_node), RANGE_RESULTS (arg_node),
                                    arg_info);
        /*
         * the template is not needed anymore, so lets free it
         */
        INFO_INDEXVECTOR (arg_info) = FREEdoFreeTree (INFO_INDEXVECTOR (arg_info));

        if (assign != NULL) {
            BLOCK_ASSIGNS (RANGE_BODY (arg_node))
              = TCappendAssign (BLOCK_ASSIGNS (RANGE_BODY (arg_node)), assign);
        }
    } else {
        DBUG_ASSERT (INFO_RANGEMODE (arg_info) == EA_index, "unknown EA_range mode");
        /*
         * allocate memory for the index and replace the ids by an id.
         * NOTE: we do not allocate memory for the IDXS here, as they
         * other than for withids have an actual computation associated
         * with them (i.e., a defining site), which will trigger their
         * allocation in the range body.
         */
        ids = RANGE_INDEX (arg_node);
        INFO_ALLOCLIST (arg_info)
          = MakeALS (INFO_ALLOCLIST (arg_info), IDS_AVIS (ids), TBmakeNum (0),
                     TCcreateZeroVector (0, T_int));
        RANGE_INDEX (arg_node) = TBmakeId (IDS_AVIS (ids));
        ids = FREEdoFreeNode (ids);
    }

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

    DBUG_ENTER ();

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

#undef DBUG_PREFIX
