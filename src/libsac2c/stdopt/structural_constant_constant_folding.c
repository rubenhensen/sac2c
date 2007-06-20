/*
 * $Id: structural_constant_constant_folding.c 15337 2007-06-11 19:49:07Z cg $
 */

/** <!--********************************************************************-->
 *
 * @defgroup cf  Structural Constant Constant Folding
 *
 * This module implements Constant Folding on Structural Constants.
 *
 *   each computed constant expression is stored in the AVIS_SSACONST(avis)
 *   attribute of the assigned identifier for later access.
 *
 *   when traversing into a special fundef we propagate constant information
 *   for all args (in loops only the loop-invariant ones) by storing the
 *   AVIS_SSACONST() in the corresponding args. constant results are propagted
 *   back in the calling context by inserting a assignment to the constant
 *   value. The removal of unused function args and result values is done
 *   later by the dead code removal.
 *
 *     structural constant, with full constant iv (array with ids as values):
 *       reshape, sel, take, drop, modarray
 *
 *     shape constant (array with known shape, scalar id):
 *       shape, sub
 *
 *     dim constants (expression with known dimension):
 *       dim, eq
 *
 *
 *  special sel-modarray optimization:
 *    looking up in a modarray chain for setting the sel referenced value
 *
 *  not yet implemented: cat, rotate
 *
 *  @ingroup opt
 *
 *  @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file structural_constant_constant_folding.c
 *
 * Prefix: SCCF
 *
 *****************************************************************************/
#include "structural_constant_constant_folding.h"

#include "dbug.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "new_types.h"
#include "type_utils.h"
#include "new_typecheck.h"
#include "globals.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "constants.h"
#include "shape.h"
#include "ctinfo.h"
#include "compare_tree.h"
#include "namespaces.h"
#include "remove_vardecs.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool remassign;
    bool onefundef;
    node *fundef;
    node *preassign;
};

#define INFO_REMASSIGN(n) (n->remassign)
#define INFO_ONEFUNDEF(n) (n->onefundef)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_PREASSIGN(n) (n->preassign)

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/* structural constant (SCO) should be integrated in constants.[ch] in future */
/* special constant version used for structural constants */
struct STRUCT_CONSTANT {
    simpletype simpletype;      /* basetype of struct constant */
    char *name;                 /* only used for T_user !! */
    const namespace_t *name_ns; /* namespace belonging to 'name' */
    shape *shape;               /* shape of struct constant */
    constant *hidden_co;        /* pointer to constant of pointers */
    node *tmpast;               /* pointer to temporary ast representation */
};

/* access macros for structural constant type */
#define SCO_BASETYPE(n) (n->simpletype)
#define SCO_NAME(n) (n->name)
#define SCO_NS(n) (n->name_ns)
#define SCO_SHAPE(n) (n->shape)
#define SCO_HIDDENCO(n) (n->hidden_co)
#define SCO_ELEMDIM(n) (SHgetDim (SCO_SHAPE (n)) - COgetDim (SCO_HIDDENCO (n)))
#define SCO_TMPAST(n) (n->tmpast)

/* local used helper functions */

/*
 * primitive functions for non full-constant expressions like:
 *   dimension constant
 *   shape constant
 *   structural constant
 *
 * they have to be implemented seperatly as long as there is no constant type
 * that can handle all these cases internally
 */

static node *StructOpSel (constant *idx, node *expr);
static node *CFStructOpReshape (constant *idx, node *expr);
static node *StructOpTake (constant *idx, node *expr);
static node *StructOpDrop (constant *idx, node *expr);

/******************************************************************************
 *
 * function:
 *   struct_constant *SCCFarray2StructConstant(node *array)
 *
 * description:
 *   converts an N_array node from AST to a structural constant.
 *   To convert an array to a structural constant, all array elements must be
 *   scalars!
 *
 *****************************************************************************/
static struct_constant *
SCCFarray2StructConstant (node *array)
{
    struct_constant *struc_co;
    ntype *atype;
    shape *realshape;
    shape *ashape;
    node **node_vec;
    node *tmp;
    bool valid_const;
    int elem_count;
    int i;

    DBUG_ENTER ("SCCFarray2StructConstant");

    DBUG_ASSERT ((array != NULL) && (NODE_TYPE (array) == N_array),
                 "SCCFarray2StructConstant supports only N_array nodes");

    /* shape of the given array */
    atype = NTCnewTypeCheck_Expr (array);

    /* build an abstract structural constant of type (void*) T_hidden */
    if (TUshapeKnown (atype)) {
        /* alloc hidden vector */
        realshape = SHcopyShape (TYgetShape (atype));
        ashape = SHcopyShape (ARRAY_SHAPE (array));

        elem_count = SHgetUnrLen (ashape);
        node_vec = (node **)MEMmalloc (elem_count * sizeof (node *));

        /* copy element pointers from array to vector */
        valid_const = TRUE;
        tmp = ARRAY_AELEMS (array);
        for (i = 0; i < elem_count; i++) {
            if (tmp == NULL) {
                /* array contains too few elements - there must be non scalar elements */
                valid_const = FALSE;
            } else {
                node_vec[i] = EXPRS_EXPR (tmp);
                tmp = EXPRS_NEXT (tmp);
            }
        }
        DBUG_ASSERT ((tmp == NULL), "array contains too many elements");

        /* create struct_constant */
        struc_co = (struct_constant *)MEMmalloc (sizeof (struct_constant));
        SCO_BASETYPE (struc_co) = TYgetSimpleType (TYgetScalar (atype));
        if (TYisUser (atype)) {
            SCO_NAME (struc_co) = TYgetName (atype);
            SCO_NS (struc_co) = TYgetNamespace (atype);
        } else {
            SCO_NAME (struc_co) = NULL;
            SCO_NS (struc_co) = NULL;
        }
        SCO_SHAPE (struc_co) = realshape;

        SCO_HIDDENCO (struc_co) = COmakeConstant (T_hidden, ashape, node_vec);
        SCO_TMPAST (struc_co) = NULL;

        /* remove invalid structural arrays */
        if (!valid_const) {
            struc_co = SCCFfreeStructConstant (struc_co);
        }
    } else {
        /* no array with known elements */
        struc_co = NULL;
    }

    atype = TYfreeType (atype);

    DBUG_RETURN (struc_co);
}

/******************************************************************************
 *
 * function:
 *   struct_constant *SCCFscalar2StructConstant(node *expr)
 *
 * description:
 *   converts an scalar node to a structual constant (e.g. N_num, ... or N_id)
 *
 ******************************************************************************/
static struct_constant *
SCCFscalar2StructConstant (node *expr)
{
    struct_constant *struc_co = NULL;
    shape *cshape;
    ntype *ctype;
    node **elem;
    nodetype nt;

    DBUG_ENTER ("SCCFscalar2StructConstant");

    nt = NODE_TYPE (expr);

    switch (nt) {
    case N_num:
    case N_float:
    case N_double:
    case N_bool:
    case N_char:
        /* determin type */
        ctype = NTCnewTypeCheck_Expr (expr);

        /* alloc hidden vector */
        cshape = SHmakeShape (0);
        elem = (node **)MEMmalloc (sizeof (node *));

        /* copy element pointers from array to vector */
        *elem = expr;

        /* create struct_constant */
        struc_co = (struct_constant *)MEMmalloc (sizeof (struct_constant));
        SCO_BASETYPE (struc_co) = TYgetSimpleType (TYgetScalar (ctype));
        SCO_NAME (struc_co) = NULL;
        SCO_NS (struc_co) = NULL;

        SCO_SHAPE (struc_co) = SHcopyShape (cshape);
        SCO_HIDDENCO (struc_co) = COmakeConstant (T_hidden, cshape, elem);
        SCO_TMPAST (struc_co) = NULL;

        ctype = TYfreeType (ctype);
        break;

    case N_id:
        if ((TUdimKnown (ID_NTYPE (expr))) && (TYgetDim (ID_NTYPE (expr)) == 0)) {
            /* create structural constant as scalar */
            ctype = ID_NTYPE (expr);

            /* alloc hidden vector */
            cshape = SHmakeShape (0);
            elem = (node **)MEMmalloc (sizeof (node *));

            /* copy element pointers from array to vector */
            *elem = expr;

            /* create struct_constant */
            struc_co = (struct_constant *)MEMmalloc (sizeof (struct_constant));
            SCO_BASETYPE (struc_co) = TYgetSimpleType (TYgetScalar (ctype));
            if (TYisUser (ctype)) {
                SCO_NAME (struc_co) = TYgetName (ctype);
                SCO_NS (struc_co) = TYgetNamespace (ctype);
            } else {
                SCO_NAME (struc_co) = NULL;
                SCO_NS (struc_co) = NULL;
            }
            SCO_SHAPE (struc_co) = SHcopyShape (cshape);
            SCO_HIDDENCO (struc_co) = COmakeConstant (T_hidden, cshape, elem);
            SCO_TMPAST (struc_co) = NULL;
        }
        break;

    default:
        break;
    }

    DBUG_RETURN (struc_co);
}

/******************************************************************************
 *
 * function:
 *   struct_constant *SCCFexpr2StructConstant(node *expr)
 *
 * description:
 *   builds an constant of type T_hidden from an array or scalar in the AST.
 *   this allows to operate on structural constants like full constants.
 *
 *   this should later be integrated in a more powerful constants module.
 *
 *   be careful:
 *     the created structural constant contain pointers to elements of the
 *     array, so you MUST NEVER FREE the original expression before you
 *     have dupped the structural constant into a array!
 *
 *****************************************************************************/

struct_constant *
SCCFexpr2StructConstant (node *expr)
{
    struct_constant *struc_co;

    DBUG_ENTER ("SCCFexpr2StructConstant");

    struc_co = NULL;

    switch (NODE_TYPE (expr)) {
    case N_array:
        /* expression is an array */
        struc_co = SCCFarray2StructConstant (expr);
        break;

    case N_bool:
    case N_char:
    case N_float:
    case N_double:
    case N_num:
        struc_co = SCCFscalar2StructConstant (expr);
        break;

    case N_id:
        if ((TUdimKnown (ID_NTYPE (expr))) && /* AKS Scalar */
            (TYgetDim (ID_NTYPE (expr)) == 0)) {
            struc_co = SCCFscalar2StructConstant (expr);
        } else {
            if (TYisAKV (ID_NTYPE (expr))) { /* Value known */
                node *array = COconstant2AST (TYgetValue (ID_NTYPE (expr)));
                struc_co = SCCFexpr2StructConstant (array);
                SCO_TMPAST (struc_co) = array;
            } else {
                node *ass = AVIS_SSAASSIGN (ID_AVIS (expr));
                if (ass != NULL) {
                    struc_co = SCCFexpr2StructConstant (ASSIGN_RHS (ass));
                }
            }
        }
        break;
    default:
        break;
    }

    DBUG_RETURN (struc_co);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFdupStructConstant2Expr(struct_constant *struc_co)
 *
 * description:
 *   builds an array of the given strucural constant and duplicate
 *   elements in it. therfore the original array must not be freed before
 *   the target array is build up from the elements of the original array.
 *
 *****************************************************************************/

node *
SCCFdupStructConstant2Expr (struct_constant *struc_co)
{
    node *expr;
    node *aelems;
    int i;
    int elems_count;
    node **node_vec;

    DBUG_ENTER ("SCCFdupStructConstant2Expr");

    /* build up elements chain */
    node_vec = (node **)COgetDataVec (SCO_HIDDENCO (struc_co));

    if (COgetDim (SCO_HIDDENCO (struc_co)) == 0) {
        /* result is a single node */
        expr = DUPdoDupNode (node_vec[0]);
    } else {
        /* result is a new array */
        elems_count = SHgetUnrLen (COgetShape (SCO_HIDDENCO (struc_co)));

        aelems = NULL;
        for (i = elems_count - 1; i >= 0; i--) {
            aelems = TBmakeExprs (DUPdoDupNode (node_vec[i]), aelems);
        }

        /* build array node */
        expr = TBmakeArray (TYmakeAKS (TYmakeSimpleType (SCO_BASETYPE (struc_co)),
                                       SHmakeShape (0)),
                            SHcopyShape (COgetShape (SCO_HIDDENCO (struc_co))), aelems);
    }
    DBUG_RETURN (expr);
}

/******************************************************************************
 *
 * function:
 *   struct_constant *SCCFfreeStructConstant(struct_constant *struc_co)
 *
 * description:
 *   frees the struct_constant data structure and the internal constant element.
 *
 *****************************************************************************/

struct_constant *
SCCFfreeStructConstant (struct_constant *struc_co)
{
    DBUG_ENTER ("SCCFfreeStructConstant");

    DBUG_ASSERT ((struc_co != NULL), "SCCFfreeStructConstant: NULL pointer");

    DBUG_ASSERT ((SCO_SHAPE (struc_co) != NULL),
                 "SCCFfreeStructConstant: SCO_SHAPE is NULL");

    /* free temporary ast representation */
    if (SCO_TMPAST (struc_co) != NULL) {
        SCO_TMPAST (struc_co) = FREEdoFreeTree (SCO_TMPAST (struc_co));
    }

    /* free shape */
    SCO_SHAPE (struc_co) = SHfreeShape (SCO_SHAPE (struc_co));

    /* free substructure */
    SCO_HIDDENCO (struc_co) = COfreeConstant (SCO_HIDDENCO (struc_co));

    /* free structure */
    struc_co = MEMfree (struc_co);

    DBUG_RETURN ((struct_constant *)NULL);
}

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *StructOpSel(constant *idx, node *arg_expr)
 *
 * description:
 *   computes sel on array expressions with constant structural constant
 *   arguments, as follows:
 *
 *      z = sel (structconstant idx, structconstant X)
 *
 *   If (shape(idx) == frame_dim(X)),  it replaces the sel() by:
 *
 *      z = X[idx]
 *
 *   If (shape(idx) > frame_dim(X)), it replaces the sel() by:
 *
 *      tmp = X[take([frame_dim(X)], idx)];   NB. Compile time
 *      z   = tmp[drop([frame_dim(X)], idx)]; NB. run time
 *
 *   If (shape(idx) < dim(X)), it does the appropriate partial selection:
 *
 *      z = X[idx];
 *
 *
 *****************************************************************************/

static node *
StructOpSel (constant *idx, node *expr)
{
    struct_constant *struc_co;
    constant *old_hidden_co;

    node *result = NULL;

    int idxlen;
    int structdim;

    shape *tmp_shape;

    constant *take_vec;
    constant *tmp_idx;

    DBUG_ENTER ("StructOpSel");

    /* Try to convert expr(especially arrays) into a structural constant */
    struc_co = SCCFexpr2StructConstant (expr);

    if (struc_co != NULL) {
        /* save internal hidden input constant */
        old_hidden_co = SCO_HIDDENCO (struc_co);

        idxlen = SHgetUnrLen (COgetShape (idx));

        structdim = COgetDim (SCO_HIDDENCO (struc_co));

        if (structdim < idxlen) {
            /*
             * Selection vector has more elements than there are array dimensions
             *
             * 1. Perform partial selection
             */
            take_vec = COmakeConstantFromInt (structdim);
            tmp_idx = COtake (take_vec, idx);

            SCO_HIDDENCO (struc_co) = COsel (tmp_idx, SCO_HIDDENCO (struc_co));
            tmp_idx = COfreeConstant (tmp_idx);

            /*
             * 2. return selection on the remaining array element
             */
            tmp_idx = COdrop (take_vec, idx);
            take_vec = COfreeConstant (take_vec);

            result = TCmakePrf2 (F_sel_VxA, COconstant2AST (tmp_idx),
                                 SCCFdupStructConstant2Expr (struc_co));
            tmp_idx = COfreeConstant (tmp_idx);
        } else {
            /*
             * Perform selection
             */
            SCO_HIDDENCO (struc_co) = COsel (idx, SCO_HIDDENCO (struc_co));

            if (structdim > idxlen) {
                /*
                 * Selection vector is too short, selection yields subarray
                 */
                tmp_shape = SCO_SHAPE (struc_co);
                SCO_SHAPE (struc_co) = SHdropFromShape (idxlen, tmp_shape);
                SHfreeShape (tmp_shape);
            }
            result = SCCFdupStructConstant2Expr (struc_co);
        }

        /*
         * free tmp. struct constant
         */
        struc_co = SCCFfreeStructConstant (struc_co);

        /*
         * free internal input constant
         */
        old_hidden_co = COfreeConstant (old_hidden_co);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *CFStructOpReshape(constant *shp, node *arg_expr)
 *
 * description:
 *   computes structural reshape on array expressions with
 *   constant index vector:
 *
 *     z = reshape(shp, X);
 *
 * In the following, the frame shape is the shape of X, and
 * the cell shape is the shape of any of the elements of X.
 *
 * Conditions:
 *   ( dim(cell) <= shape(shp))
 *   ( take((-dim(cell), shp)) <==> cell_shape)
 *   ( prod(drop(-dim(cell), shp) = prod(frame_shape)))
 *   The last of these is performed by TC, so is ignored here.
 *
 *****************************************************************************/
static node *
CFStructOpReshape (constant *shp, node *arg2)
{
    struct_constant *struc_co;
    constant *old_hidden_co;

    node *res = NULL;

    int frame_rank;

    shape *shp_shape;
    shape *shp_shape_postfix;
    shape *cell_shape;
    int cell_dim;
    int shp_dim;

    constant *drop_vec;
    constant *tmp_idx;

    DBUG_ENTER ("CFStructOpReshape");

    /* Try to convert expr(especially arrays) into a structual constant */
    struc_co = SCCFexpr2StructConstant (arg2);

    if (struc_co != NULL) {

        frame_rank = COgetDim (SCO_HIDDENCO (struc_co));
        cell_shape = SHdropFromShape (frame_rank, SCO_SHAPE (struc_co));
        cell_dim = SHgetDim (cell_shape);
        shp_shape = COconstant2Shape (shp);
        shp_dim = SHgetDim (shp_shape);

        if (cell_dim <= shp_dim) { /* Avoid overtake in next line */
            shp_shape_postfix = SHtakeFromShape (-1 * cell_dim, shp_shape);
            shp_shape = SHfreeShape (shp_shape);

            /*
             * If the shp_shape_postfix equals the element shape,
             * the reshape operation can be performed
             */
            if (SHcompareShapes (cell_shape, shp_shape_postfix)) {

                /* save internal hidden input constant */
                old_hidden_co = SCO_HIDDENCO (struc_co);

                drop_vec = COmakeConstantFromInt (-1 * SHgetDim (cell_shape));
                tmp_idx = COdrop (drop_vec, shp);
                drop_vec = COfreeConstant (drop_vec);

                shp_shape = COconstant2Shape (tmp_idx);
                SCO_HIDDENCO (struc_co) = COreshape (tmp_idx, SCO_HIDDENCO (struc_co));
                tmp_idx = COfreeConstant (tmp_idx);

                SCO_SHAPE (struc_co) = SHfreeShape (SCO_SHAPE (struc_co));
                SCO_SHAPE (struc_co) = SHappendShapes (shp_shape, cell_shape);
                shp_shape = SHfreeShape (shp_shape);

                res = SCCFdupStructConstant2Expr (struc_co);

                /* free internal hidden input constant */
                old_hidden_co = COfreeConstant (old_hidden_co);
            }
        }
        cell_shape = SHfreeShape (cell_shape);
        shp_shape_postfix = SHfreeShape (shp_shape_postfix);

        /*
         * free tmp. struct constant
         */
        struc_co = SCCFfreeStructConstant (struc_co);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_reshape(node *arg_node, info *arg_info)
 *
 * description:
 *   Eliminates structural constant reshape expression
 *      reshape( shp, arr)

 *   when it can determine that and that shp is a constant.
 *      shape(arr) <==>  shp
 *
 *
 *****************************************************************************/
node *
SCCFprf_reshape (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *arg1;

    DBUG_ENTER ("SCCFprf_reshape");
    arg1 = COaST2Constant (PRF_ARG1 (arg_node));

    /* CF handles const/const case */
    if ((NULL != arg1)) {
        /* constant shp, non-constant arr */
        res = CFStructOpReshape (arg1, PRF_ARG2 (arg_node));
    }

    arg1 = COfreeConstant (arg1);
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *StructOpTake(constant *arg1, node *arg2)
 *
 * description:
 *   Attempts to eliminate under-take on array expressions when
 *   arg1 is constant and arg2 is structural constant.
 *
 *****************************************************************************/

static node *
StructOpTake (constant *arg1, node *arg2)
{
    struct_constant *struc_co;
    constant *old_hidden_co;
    node *res = NULL;
    int idxlen;
    int structdim;
    shape *sco_shape;
    shape *dropped_shape;

    DBUG_ENTER ("StructOpTake");

    /* Try to convert arg2 (especially arrays) into a structural constant */
    struc_co = SCCFexpr2StructConstant (arg2);

    /* given expression was converted to struct_constant */
    if (struc_co != NULL) {

        idxlen = SHgetUnrLen (COgetShape (arg1));
        structdim = COgetDim (SCO_HIDDENCO (struc_co));

        if (idxlen <= structdim) {
            /* save internal hidden input constant */
            old_hidden_co = SCO_HIDDENCO (struc_co);

            SCO_HIDDENCO (struc_co) = COtake (arg1, SCO_HIDDENCO (struc_co));

            sco_shape = SCO_SHAPE (struc_co);
            dropped_shape = SHdropFromShape (structdim, sco_shape);
            sco_shape = SHfreeShape (sco_shape);
            SCO_SHAPE (struc_co)
              = SHappendShapes (COgetShape (SCO_HIDDENCO (struc_co)), dropped_shape);
            dropped_shape = SHfreeShape (dropped_shape);

            /* return modified array */
            res = SCCFdupStructConstant2Expr (struc_co);

            /* free tmp. struct constant */
            struc_co = SCCFfreeStructConstant (struc_co);

            /* free internal input constant */
            old_hidden_co = COfreeConstant (old_hidden_co);
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_take(node *arg_node, info *arg_info)
 *
 * description:
 *   computes structural undertake on array expressions with constant arg1.
 *
 *
 *****************************************************************************/
node *
SCCFprf_take (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *arg1;
    constant *arg2;
    constant *tmp;

    DBUG_ENTER ("SCCFprf_take");
    arg1 = COaST2Constant (PRF_ARG1 (arg_node));
    arg2 = COaST2Constant (PRF_ARG2 (arg_node));

    if (NULL != arg1) { /* If arg1 isn't constant, we're stuck */
        if (NULL != arg2) {
            /* Both args are constants. Piece of cake. */
            tmp = COtake (arg1, arg2);
            res = COconstant2AST (tmp);
            tmp = COfreeConstant (tmp);
        } else {
            /* See if we can make arg2 a structural constant and do it that way. */
            res = StructOpTake (arg1, PRF_ARG2 (arg_node));
        }
    }
    arg1 = COfreeConstant (arg1);
    arg2 = COfreeConstant (arg2);
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *StructOpDrop(constant *arg1, node *arg2)
 *
 * description:
 *   computes structural drop on array expressions with constant arg1.
 *
 *****************************************************************************/

static node *
StructOpDrop (constant *arg1, node *arg2)
{
    struct_constant *struc_co;
    constant *old_hidden_co;

    node *res = NULL;
    int idxlen;
    int structdim;
    shape *sco_shape;
    shape *dropped_shape;

    DBUG_ENTER ("StructOpDrop");

    /* Try to convert expr(especially arrays) into a structural constant */
    struc_co = SCCFexpr2StructConstant (arg2);

    /* given expression could be converted to struct_constant */
    if (struc_co != NULL) {

        idxlen = SHgetUnrLen (COgetShape (arg1));
        structdim = COgetDim (SCO_HIDDENCO (struc_co));

        if (idxlen <= structdim) {
            /*
             * save internal hidden input constant
             */
            old_hidden_co = SCO_HIDDENCO (struc_co);

            SCO_HIDDENCO (struc_co) = COdrop (arg1, SCO_HIDDENCO (struc_co));

            sco_shape = SCO_SHAPE (struc_co);
            dropped_shape = SHdropFromShape (structdim, sco_shape);
            sco_shape = SHfreeShape (sco_shape);
            SCO_SHAPE (struc_co)
              = SHappendShapes (COgetShape (SCO_HIDDENCO (struc_co)), dropped_shape);
            dropped_shape = SHfreeShape (dropped_shape);

            /* return modified array */
            res = SCCFdupStructConstant2Expr (struc_co);

            /* free tmp. struct constant */
            struc_co = SCCFfreeStructConstant (struc_co);

            /* free internal input constant */
            old_hidden_co = COfreeConstant (old_hidden_co);
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_drop(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements drop for structural constants
 *
 *
 *****************************************************************************/
node *
SCCFprf_drop (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *arg1;
    constant *arg2;
    constant *tmp;

    DBUG_ENTER ("SCCFprf_drop");

    arg1 = COaST2Constant (PRF_ARG1 (arg_node));
    arg2 = COaST2Constant (PRF_ARG2 (arg_node));

    if (NULL != arg1) { /* If arg1 isn't constant, we're stuck */
        if (NULL != arg2) {
            /* Both args are constants. Piece of cake. */
            tmp = COdrop (arg1, arg2);
            res = COconstant2AST (tmp);
            tmp = COfreeConstant (tmp);
        } else {
            /* See if we can make arg2 a structural constant and do it that way. */
            res = StructOpDrop (arg1, PRF_ARG2 (arg_node));
        }
    }
    arg1 = COfreeConstant (arg1);
    arg2 = COfreeConstant (arg2);
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *StructOpModarray( node *arg1, constant *arg2, node *arg3)
 *
 * description:
 *   implement modarray on structural constant arrays with
 *   constant index vector arg2.
 *
 ******************************************************************************/

static node *
StructOpModarray (node *arg1, constant *arg2, node *arg3)
{
    node *res = NULL;
    struct_constant *struc_a;
    struct_constant *struc_arg3;
    constant *old_hidden_co;
    ntype *atype;

    DBUG_ENTER ("StructOpModarray");

    /**
     * if the index is an empty vector, we simply replace the entire
     * expression by the arg3 value!
     * Well, not quite!!! This is only valid, iff
     *      shape( arg3) ==  shape(arg1)
     * If we do not know this, then the only thing we can do is
     * to replace the modarray by
     *      _type_conv_( type(arg1), arg3)
     * iff a is AKS! * cf bug246 !!! */

    if (COisEmptyVect (arg2)) {
        DBUG_ASSERT ((NODE_TYPE (arg1) == N_id),
                     "non id found in array-arg position of F_modarray");
        DBUG_ASSERT ((NODE_TYPE (arg3) == N_id),
                     "non id found in arg3-arg position of F_modarray");
        if (AVIS_SHAPE (ID_AVIS (arg1)) != NULL) {
            if (CMPTdoCompareTree (AVIS_SHAPE (ID_AVIS (arg1)),
                                   AVIS_SHAPE (ID_AVIS (arg3)))
                == CMPT_EQ) {
                res = DUPdoDupTree (arg3);
            }
        }
        atype = AVIS_TYPE (ID_AVIS (arg1));
        if ((res == NULL) && TUshapeKnown (atype)) {
            res = TCmakePrf2 (F_type_conv, TBmakeType (TYeliminateAKV (atype)),
                              DUPdoDupTree (arg3));
        }
    } else {
        /**
         * as we are not dealing with the degenerate case (arg2 == []),
         * we need arg1 and arg3 to be structural constants in order to be
         * able to do anything!
         */
        struc_a = SCCFexpr2StructConstant (arg1);
        struc_arg3 = SCCFexpr2StructConstant (arg3);

        /* given expressession could be converted to struct_constant */
        if ((struc_a != NULL) && (struc_arg3 != NULL)) {
            if (SCO_ELEMDIM (struc_a) == SCO_ELEMDIM (struc_arg3)) {
                /* save internal hidden constant */
                old_hidden_co = SCO_HIDDENCO (struc_a);

                /* perform modarray operation on structural constant */
                SCO_HIDDENCO (struc_a)
                  = COmodarray (SCO_HIDDENCO (struc_a), arg2, SCO_HIDDENCO (struc_arg3));

                /* return modified array */
                res = SCCFdupStructConstant2Expr (struc_a);

                DBUG_PRINT ("CF", ("op computed on structural constant"));

                /* free internal constant */
                old_hidden_co = COfreeConstant (old_hidden_co);
            }
        }

        /* free struct constants */
        if (struc_a != NULL) {
            struc_a = SCCFfreeStructConstant (struc_a);
        }

        if (struc_arg3 != NULL) {
            struc_arg3 = SCCFfreeStructConstant (struc_arg3);
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_modarray(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements modarray for structural constants
 *
 *****************************************************************************/
node *
SCCFprf_modarray (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *arg2;

    DBUG_ENTER ("SCCFprf_modarray");
    arg2 = COaST2Constant (PRF_ARG2 (arg_node));
    if (NULL != arg2) {
        res = StructOpModarray (PRF_ARG1 (arg_node), arg2, PRF_ARG3 (arg_node));
    }

    arg2 = COfreeConstant (arg2);
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *StructOpCat(node *arg1, node *arg2)
 *
 * description:
 *   tries to concatenate the given vectors as struct constants
 *   Handles:
 *      vec   ++ vec
 *      vec   ++ empty
 *      empty ++ vec
 *
 *****************************************************************************/

static node *
StructOpCat (node *arg1, node *arg2)
{
    node *res = NULL;
    struct_constant *sc_vec1;
    struct_constant *sc_vec2;

    DBUG_ENTER ("StructOpCat");

    sc_vec1 = SCCFexpr2StructConstant (arg1);
    sc_vec2 = SCCFexpr2StructConstant (arg2);

    if ((sc_vec1 != NULL) && (sc_vec2 != NULL)) {
        constant *vec2_hidden_co;

        /* if both vectors are structural constants, we can catenate them */
        vec2_hidden_co = SCO_HIDDENCO (sc_vec2);
        SCO_HIDDENCO (sc_vec2) = COcat (SCO_HIDDENCO (sc_vec1), SCO_HIDDENCO (sc_vec2));

        res = SCCFdupStructConstant2Expr (sc_vec2);

        vec2_hidden_co = COfreeConstant (vec2_hidden_co);
    } else if (sc_vec1 != NULL) {
        /* arg1 is empty vector, so result is arg2 */
        if (SHgetUnrLen (COgetShape (SCO_HIDDENCO (sc_vec1))) == 0) {
            res = DUPdoDupNode (arg2);
        }
    } else if (sc_vec2 != NULL) {
        /* arg2 is empty vector, so result is arg1 */
        if (SHgetUnrLen (COgetShape (SCO_HIDDENCO (sc_vec2))) == 0) {
            res = DUPdoDupNode (arg1);
        }
    }

    if (sc_vec1 != NULL)
        sc_vec1 = SCCFfreeStructConstant (sc_vec1);

    if (sc_vec2 != NULL)
        sc_vec2 = SCCFfreeStructConstant (sc_vec2);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_cat_VxV(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements vector catenate for structural constants
 *
 *****************************************************************************/
node *
SCCFprf_cat_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *arg1;
    constant *arg2;

    DBUG_ENTER ("SCCFprf_cat_VxV");
    arg1 = COaST2Constant (PRF_ARG1 (arg_node));
    arg2 = COaST2Constant (PRF_ARG2 (arg_node));

    /* V++empty or empty++V */
    if ((NULL != arg1) && (0 == SHgetUnrLen (COgetShape (arg1)))) {
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    } else if ((NULL != arg2) && (0 == SHgetUnrLen (COgetShape (arg2)))) {
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    } else {
        /* Try structural constants */
        res = StructOpCat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node));
    }

    arg1 = COfreeConstant (arg1);
    arg2 = COfreeConstant (arg2);
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *Sel(node *idx_expr, node *array_expr)
 *
 * description:
 *   tries a special sel-modarray optimization for the following cases:
 *
 *   1. iv is an unknown expression:
 *      b = modarray(arr, iv, value)
 *      x = sel(iv, b)    ->   x = value;
 *
 *   2. iv is an expression with known constant value:
 *      b = modarray(arr, [5], val5);
 *      c = modarray(b, [3], val3);
 *      d = modarray(c, [2], val2);
 *      x = sel([5], d)   ->  x = val5;
 *
 *   maybe this allows elimination of some arrays.
 *   HOWEVER, we still need to ensure that iv is a valid index for arr.
 *   As of 2007-06-18, this check is NOT done!
 *
 *****************************************************************************/

static node *
Sel (node *idx_expr, node *array_expr)
{
    node *result;
    node *prf_mod;
    node *prf_sel;
    node *concat;
    node *mod_arr_expr;
    node *mod_idx_expr;
    node *mod_elem_expr;
    constant *idx_co;
    constant *mod_idx_co;

    DBUG_ENTER ("Sel");

    result = NULL;

    /*
     * checks if array_expr is an array identifier defined by a primitive
     * modarray operation
     */
    if ((NODE_TYPE (array_expr) == N_id)
        && (AVIS_SSAASSIGN (ID_AVIS (array_expr)) != NULL)
        && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (array_expr)))) == N_prf)) {

        switch (PRF_PRF (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (array_expr))))) {

        case F_modarray_AxVxS:
            prf_mod = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (array_expr)));

            /* get parameter of modarray */
            DBUG_ASSERT ((PRF_ARGS (prf_mod) != NULL), "missing 1. arg for modarray");
            mod_arr_expr = EXPRS_EXPR (PRF_ARGS (prf_mod));

            DBUG_ASSERT ((EXPRS_NEXT (PRF_ARGS (prf_mod)) != NULL),
                         "missing 2. arg for modarray");
            mod_idx_expr = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (prf_mod)));

            DBUG_ASSERT ((EXPRS_NEXT (EXPRS_NEXT (PRF_ARGS (prf_mod))) != NULL),
                         "missing 3. arg for modarray");
            mod_elem_expr = EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (PRF_ARGS (prf_mod))));

            /* try to build up constants from index vectors */
            idx_co = COaST2Constant (idx_expr);
            mod_idx_co = COaST2Constant (mod_idx_expr);

            if ((CMPTdoCompareTree (idx_expr, mod_idx_expr) == CMPT_EQ)
                || ((idx_co != NULL) && (mod_idx_co != NULL)
                    && (COcompareConstants (idx_co, mod_idx_co)))) {
                /*
                 * idx vectors in sel and modarray are equal
                 * - replace sel() with element
                 */
                result = DUPdoDupTree (mod_elem_expr);

                DBUG_PRINT ("CF", ("sel-modarray optimization done"));

            } else {
                /* index vector does not match, but if both are constant, we can try
                 * to look up futher in a modarray chain to find a matching one.
                 * to avoid wrong decisions we need constant vectors in both idx
                 * expressions.
                 */
                if ((idx_co != NULL) && (mod_idx_co != NULL)) {
                    result = Sel (idx_expr, mod_arr_expr);
                } else {
                    /* no further analysis possible, because of non-constant idx expr */
                    result = NULL;
                }
            }

            /* free local constants */
            if (idx_co != NULL) {
                idx_co = COfreeConstant (idx_co);
            }
            if (mod_idx_co != NULL) {
                mod_idx_co = COfreeConstant (mod_idx_co);
            }

            break;

        case F_sel_VxA:
            prf_sel = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (array_expr)));
            concat = StructOpCat (EXPRS_EXPR (PRF_ARGS (prf_sel)), idx_expr);

            if (concat != NULL) {
                result = TCmakePrf2 (F_sel_VxA, concat,
                                     DUPdoDupTree (
                                       EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (prf_sel)))));
            }
            break;

        default:
            break;
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_sel(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements sel(arg1, arg2) for structural constants
 *
 *
 *****************************************************************************/
node *
SCCFprf_sel (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *arg1;

    DBUG_ENTER ("SCCFprf_sel");

    arg1 = COaST2Constant (PRF_ARG1 (arg_node));

    if (NULL != arg1) { /* z = sel (constant, arg2) */
        res = StructOpSel (arg1, PRF_ARG2 (arg_node));
    } else {
        res = Sel (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node));
    }
    arg1 = COfreeConstant (arg1);
    DBUG_RETURN (res);
}
