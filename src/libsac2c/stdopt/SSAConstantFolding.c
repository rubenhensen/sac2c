/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup cf Constant Folding
 *
 *   this module implementes constant folding on code in ssa form. for
 *   constant expressions we compute primitive functions at compile time
 *   so we need not to compute them at runtime. this simplyfies the code
 *   and allows further optimizations.
 *
 * TODO: this comment needs to be adjusted to the changes that have been made
 *       during post-Marielyst brushing/bugfixing!!!!
 *
 *   IMPORTANT: Making CF dependent on AKV types rather than constant arguments
 *   is a VERY important design decision. Only this way we prevent CF
 *   to be called with arguments that may violate the prf's domain restrictions
 *   (cf. bug 61).
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
 *   at this time the following primitive operations are implemented:
 *     for full constants (scalar value, arrays with known values):
 *       toi, tof, tod, abs, not, dim, shape, min, max, add, sub, mul, div,
 *       mod, and, le, lt, eq, ge, neq, reshape, sel, take, drop, modarray
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
 *  arithmetic optimizations:
 *    add (x+0->x, 0+x->x),
 *    sub (x-0->x),
 *    mul (x*0->0, 0*x->0, x*1->x, 1*x->x),
 *    div (x/0->err, 0/x->0, x/1->x),
 *    and (x&&1->x, 1&&x->x, x&&0->0, 0&&x->0),
 *    or  (x||1->1, 1||x->1, x||0->x, 0||x->x)
 *
 *  special sel-modarray optimization:
 *    looking up in a modarray chain for setting the sel referenced value
 *
 *  not yet implemented: cat, rotate
 *
 *  As of 2006-12-22, most of the appropriate SxS optimizations have been
 *  extended to SxA and AxS. rbe
 *
 * TODO:
 *  1. Could extend F_reshape(x,y) and F_take(x,y) to work on any arrays where we can
 *prove that all(x==shape(y)). Perhaps SAA can do this handily.
 *
 *  @ingroup opt
 *
 *  @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file SSAConstantFolding.c
 *
 * Prefix: SSACF
 *
 *****************************************************************************/
#include "SSAConstantFolding.h"

#include "dbug.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
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

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_REMASSIGN (result) = FALSE;
    INFO_ONEFUNDEF (result) = TRUE;
    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}
/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node* CFdoConstantFolding(node* fundef)
 *
 *****************************************************************************/

node *
CFdoConstantFolding (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("CFdoConstantFolding");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "CFdoConstantFolding called for non-fundef node");

    /* do not start traversal in special functions */
    arg_info = MakeInfo ();

    TRAVpush (TR_cf);
    fundef = TRAVdo (fundef, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn node* CFdoConstantFoldingModule(node* syntax_tree)
 *
 *****************************************************************************/

node *
CFdoConstantFoldingModule (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("CFdoConstantFoldingModule");

    /* do not start traversal in special functions */
    arg_info = MakeInfo ();

    INFO_ONEFUNDEF (arg_info) = FALSE;

    TRAVpush (TR_cf);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/* maximum of supported args for primitive functions */
#define PRF_MAX_ARGS 3

/* macros used in CFfoldPrfExpr to handle arguments selections */
#define ONE_CONST_ARG(arg) (arg[0] != NULL)

#define TWO_CONST_ARG(arg) ((arg[0] != NULL) && (arg[1] != NULL))

#define ONE_CONST_ARG_OF_TWO(arg, arg_expr)                                              \
    ((((arg[0] == NULL) && (arg[1] != NULL)) || ((arg[0] != NULL) && (arg[1] == NULL)))  \
     && (arg_expr[0] != NULL) && (arg_expr[1] != NULL))

#define FIRST_CONST_ARG_OF_TWO(arg, arg_expr) ((arg[0] != NULL) && (arg_expr[1] != NULL))

#define SECOND_CONST_ARG_OF_TWO(arg, arg_expr) ((arg[1] != NULL) && (arg_expr[0] != NULL))

#define ONE_ARG(arg_expr) (arg_expr[0] != NULL)

#define TWO_ARG(arg_expr) ((arg_expr[0] != NULL) && (arg_expr[1] != NULL))

#define SECOND_CONST_ARG_OF_THREE(arg, arg_expr)                                         \
    ((arg_expr[0] != NULL) && (arg[1] != NULL) && (arg_expr[2] != NULL))

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
static node **GetPrfArgs (node **array, node *prf_arg_chain, int max_args);
static constant **Args2Const (constant **co_array, node **arg_expr, int max_args);
static shape *GetShapeOfExpr (node *expr);

/*
 * primitive functions for non full-constant expressions like:
 *   dimension constant
 *   shape constant
 *   structural constant
 *
 * they have to be implemented seperatly as long as there is no constant type
 * that can handle all these cases internally
 */

static node *Dim (node *expr);
static node *Shape (node *expr, info *arg_info);

static node *StructOpSel (constant *idx, node *expr);
static node *StructOpReshape (constant *idx, node *expr);
static node *StructOpTake (constant *idx, node *expr);
static node *StructOpDrop (constant *idx, node *expr);

/* implements: arithmetical opt. for add, sub, mul, div, and, or */
static node *ArithmOpWrapper (prf op, constant **arg_co, node **arg_expr);

/*
 * some primitive functions that allows special optimizations in more
 * generic cases
 */
static node *Eq (node *expr1, node *expr2);
static node *Add (node *expr1, node *expr2);
static node *Sub (node *expr1, node *expr2);
static node *Modarray (node *a, constant *idx, node *elem);
static node *Sel (node *idx_expr, node *array_expr);

/*
 * functions to handle SCOs
 */

/******************************************************************************
 *
 * function:
 *   struct_constant *CFscoArray2StructConstant(node *array)
 *
 * description:
 *   converts an N_array node from AST to a structural constant.
 *   To convert an array to a structural constant all array elements must be
 *   scalars!
 *
 *****************************************************************************/
static struct_constant *
CFscoArray2StructConstant (node *array)
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

    DBUG_ENTER ("CFscoArray2StructConstant");

    DBUG_ASSERT ((array != NULL) && (NODE_TYPE (array) == N_array),
                 "CFscoArray2StructConstant supports only N_array nodes");

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
            struc_co = CFscoFreeStructConstant (struc_co);
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
 *   struct_constant *CFscoScalar2StructConstant(node *expr)
 *
 * description:
 *   converts an scalar node to a structual constant (e.g. N_num, ... or N_id)
 *
 ******************************************************************************/
static struct_constant *
CFscoScalar2StructConstant (node *expr)
{
    struct_constant *struc_co = NULL;
    shape *cshape;
    ntype *ctype;
    node **elem;
    nodetype nt;

    DBUG_ENTER ("CFscoScalar2StructConstant");

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
 *   struct_constant *SCOExpr2StructConstant(node *expr)
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
CFscoExpr2StructConstant (node *expr)
{
    struct_constant *struc_co;

    DBUG_ENTER ("SCOExpr2StructConstant");

    struc_co = NULL;

    switch (NODE_TYPE (expr)) {
    case N_array:
        /* expression is an array */
        struc_co = CFscoArray2StructConstant (expr);
        break;

    case N_bool:
    case N_char:
    case N_float:
    case N_double:
    case N_num:
        struc_co = CFscoScalar2StructConstant (expr);
        break;

    case N_id:
        if ((TUdimKnown (ID_NTYPE (expr))) && (TYgetDim (ID_NTYPE (expr)) == 0)) {
            struc_co = CFscoScalar2StructConstant (expr);
        } else {
            if (TYisAKV (ID_NTYPE (expr))) {
                node *array = COconstant2AST (TYgetValue (ID_NTYPE (expr)));
                struc_co = CFscoExpr2StructConstant (array);
                SCO_TMPAST (struc_co) = array;
            } else {
                node *ass = AVIS_SSAASSIGN (ID_AVIS (expr));
                if (ass != NULL) {
                    struc_co = CFscoExpr2StructConstant (ASSIGN_RHS (ass));
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
 *   node *CFscoDupStructConstant2Expr(struct_constant *struc_co)
 *
 * description:
 *   builds an array of the given strucural constant and duplicate
 *   elements in it. therfore the original array must not be freed before
 *   the target array is build up from the elements of the original array.
 *
 *****************************************************************************/

node *
CFscoDupStructConstant2Expr (struct_constant *struc_co)
{
    node *expr;
    node *aelems;
    int i;
    int elems_count;
    node **node_vec;

    DBUG_ENTER ("CFscoDupStructConstant2Expr");

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
 *   struct_constant *CFscoFreeStructConstant(struct_constant *struc_co)
 *
 * description:
 *   frees the struct_constant data structure and the internal constant element.
 *
 *****************************************************************************/

struct_constant *
CFscoFreeStructConstant (struct_constant *struc_co)
{
    DBUG_ENTER ("CFscoFreeStructConstant");

    DBUG_ASSERT ((struc_co != NULL), "CDscoFreeStructConstant: NULL pointer");

    DBUG_ASSERT ((SCO_SHAPE (struc_co) != NULL),
                 "CFscoFreeStructConstant: SCO_SHAPE is NULL");

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

/** <!--********************************************************************-->
 *
 * @fn bool IsFullyConstantNode( node *arg_node)
 *
 *****************************************************************************/
static bool
IsFullyConstantNode (node *arg_node)
{
    bool res;

    DBUG_ENTER ("IsFullyConstantNode");

    switch (NODE_TYPE (arg_node)) {
    case N_bool:
    case N_char:
    case N_num:
    case N_float:
    case N_double:
        res = TRUE;
        break;

    case N_array: {
        node *elems = ARRAY_AELEMS (arg_node);
        res = TRUE;
        while (res && (elems != NULL)) {
            res = res && IsFullyConstantNode (EXPRS_EXPR (elems));
            elems = EXPRS_NEXT (elems);
        }
    } break;

    default:
        res = FALSE;
        break;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node **GetPrfArgs( node **array, node *prf_arg_chain, int max_args)
 *
 * description:
 *   fills an pointer array of len max_args with the args from an exprs chain
 *   or NULL if there are no more args.
 *
 *****************************************************************************/

static node **
GetPrfArgs (node **array, node *prf_arg_chain, int max_args)
{
    node *expr;
    int i;

    DBUG_ENTER ("CFGetPrfArgs");

    for (i = 0; i < max_args; i++) {
        if (prf_arg_chain != NULL) {
            expr = EXPRS_EXPR (prf_arg_chain);
            prf_arg_chain = EXPRS_NEXT (prf_arg_chain);
        } else {
            expr = NULL;
        }
        array[i] = expr;
    }

    DBUG_RETURN (array);
}

/******************************************************************************
 *
 * function:
 *   constant **Args2Const(constant **co_array,
 *                              node **arg_expr,
 *                              int max_args)
 *
 * description:
 *   converts all expr nodes to constant node and store them in an array of
 *   constants (co_array).
 *
 *****************************************************************************/

static constant **
Args2Const (constant **co_array, node **arg_expr, int max_args)
{
    int i;

    DBUG_ENTER ("CFArgs2Const");

    for (i = 0; i < max_args; i++) {
        if (arg_expr[i] != NULL) {
            co_array[i] = COaST2Constant (arg_expr[i]);
        } else {
            co_array[i] = NULL;
        }
    }

    DBUG_RETURN (co_array);
}

/******************************************************************************
 *
 * function:
 *   constant *Dim(node *expr)
 *
 * description:
 *   tries computes the dimension of an identifier and returns it as
 *   constant for later usage or NULL if the dimension is not known.
 *
 *****************************************************************************/

static node *
Dim (node *expr)
{
    node *res = NULL;

    DBUG_ENTER ("Dim");

    DBUG_ASSERT (NODE_TYPE (expr) == N_id, "Dim can only be applied to N_id nodes!");

    if (TUdimKnown (ID_NTYPE (expr))) {
        res = TBmakeNum (TYgetDim (ID_NTYPE (expr)));
    } else {
        node *dim = AVIS_DIM (ID_AVIS (expr));
        if (dim != NULL) {
            res = DUPdoDupNode (dim);
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *Shape(node *expr)
 *
 * description:
 *   computes the shape of a given identifier. returns the shape as expression
 *   or NULL if no constant shape can be infered.
 *   for userdefined types the result is the shape in simpletype elements.
 *
 *****************************************************************************/

static node *
Shape (node *expr, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("Shape");

    DBUG_ASSERT (NODE_TYPE (expr) == N_id, "Shape can only be applied to N_id nodes.");

    if (TUshapeKnown (ID_NTYPE (expr))) {
        res = SHshape2Array (TYgetShape (ID_NTYPE (expr)));
    } else {
        node *shape = AVIS_SHAPE (ID_AVIS (expr));
        if (shape != NULL) {
            res = DUPdoDupNode (shape);
        } else {
            if (TUdimKnown (ID_NTYPE (expr))) {
                int i;
                for (i = TYgetDim (ID_NTYPE (expr)) - 1; i >= 0; i--) {
                    node *avis = TBmakeAvis (ILIBtmpVarName (ID_NAME (expr)),
                                             TYmakeAKS (TYmakeSimpleType (T_int),
                                                        SHmakeShape (0)));

                    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                      = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

                    INFO_PREASSIGN (arg_info)
                      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                                 TCmakePrf2 (F_idx_shape_sel,
                                                             TBmakeNum (i),
                                                             DUPdoDupNode (expr))),
                                      INFO_PREASSIGN (arg_info));
                    AVIS_SSAASSIGN (avis) = INFO_PREASSIGN (arg_info);

                    res = TBmakeExprs (TBmakeId (avis), res);
                }
                res = TCmakeIntVector (res);
            }
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *IdxShapeSel(constant *idx, node *array_expr, info *arg_info)
 *
 *****************************************************************************/
static node *
IdxShapeSel (constant *idx, node *expr, info *arg_info)
{
    node *res = NULL;
    int shape_elem;

    DBUG_ENTER ("IdxShapeSel");

    shape_elem = ((int *)COgetDataVec (idx))[0];

    if (TUshapeKnown (ID_NTYPE (expr))) {
        res = TBmakeNum (SHgetExtent (TYgetShape (ID_NTYPE (expr)), shape_elem));
    } else {
        node *shape = AVIS_SHAPE (ID_AVIS (expr));

        if ((shape != NULL) && (NODE_TYPE (shape) == N_array)) {
            node *shpel = TCgetNthExpr (shape_elem + 1, ARRAY_AELEMS (shape));
            res = DUPdoDupNode (shpel);
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *StructOpSel(constant *idx, node *arg_expr)
 *
 * description:
 *   computes structural sel on array expressions with constant index vector.
 *
 *****************************************************************************/
static node *
StructOpSel (constant *idx, node *expr)
{
    struct_constant *struc_co;
    constant *old_hidden_co;

    node *result;

    int idxlen;
    int structdim;

    shape *tmp_shape;

    constant *take_vec;
    constant *tmp_idx;

    DBUG_ENTER ("StructOpSel");

    result = NULL;

    /*
     * Try to convert expr(especially arrays) into a structual constant
     */
    struc_co = CFscoExpr2StructConstant (expr);

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

            result = TCmakePrf2 (F_sel, COconstant2AST (tmp_idx),
                                 CFscoDupStructConstant2Expr (struc_co));
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
            result = CFscoDupStructConstant2Expr (struc_co);
        }

        /*
         * free tmp. struct constant
         */
        struc_co = CFscoFreeStructConstant (struc_co);

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
 *   node *StructOpReshape(constant *idx, node *arg_expr)
 *
 * description:
 *   computes structural reshape on array expressions with
 *   constant index vector.
 *
 *****************************************************************************/
static node *
StructOpReshape (constant *idx, node *expr)
{
    struct_constant *struc_co;
    constant *old_hidden_co;

    node *result;

    int structdim;

    shape *idx_shape;
    shape *idx_shape_postfix;
    shape *elem_shape;

    constant *drop_vec;
    constant *tmp_idx;

    DBUG_ENTER ("CFStructOpReshape");

    result = NULL;

    /*
     * Try to convert expr(especially arrays) into a structual constant
     */
    struc_co = CFscoExpr2StructConstant (expr);

    if (struc_co != NULL) {

        structdim = COgetDim (SCO_HIDDENCO (struc_co));

        elem_shape = SHdropFromShape (structdim, SCO_SHAPE (struc_co));
        idx_shape = COconstant2Shape (idx);
        idx_shape_postfix = SHtakeFromShape (-1 * SHgetDim (elem_shape), idx_shape);
        idx_shape = SHfreeShape (idx_shape);

        /*
         * If the idx_shape_postfix equals the element shape,
         * the reshape operation can be performed
         */
        if (SHcompareShapes (elem_shape, idx_shape_postfix)) {
            /*
             * save internal hidden input constant
             */
            old_hidden_co = SCO_HIDDENCO (struc_co);

            drop_vec = COmakeConstantFromInt (-1 * SHgetDim (elem_shape));
            tmp_idx = COdrop (drop_vec, idx);
            drop_vec = COfreeConstant (drop_vec);

            idx_shape = COconstant2Shape (tmp_idx);
            SCO_HIDDENCO (struc_co) = COreshape (tmp_idx, SCO_HIDDENCO (struc_co));
            tmp_idx = COfreeConstant (tmp_idx);

            SCO_SHAPE (struc_co) = SHfreeShape (SCO_SHAPE (struc_co));
            SCO_SHAPE (struc_co) = SHappendShapes (idx_shape, elem_shape);
            idx_shape = SHfreeShape (idx_shape);

            result = CFscoDupStructConstant2Expr (struc_co);

            /*
             * free internal hidden input constant
             */
            old_hidden_co = COfreeConstant (old_hidden_co);
        }
        elem_shape = SHfreeShape (elem_shape);
        idx_shape_postfix = SHfreeShape (idx_shape_postfix);

        /*
         * free tmp. struct constant
         */
        struc_co = CFscoFreeStructConstant (struc_co);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *StructOpTake(constant *idx, node *arg_expr)
 *
 * description:
 *   computes structural take on array expressions with constant index vector.
 *
 *****************************************************************************/

static node *
StructOpTake (constant *idx, node *expr)
{
    struct_constant *struc_co;
    constant *old_hidden_co;

    node *result;

    int idxlen;
    int structdim;

    shape *sco_shape;
    shape *dropped_shape;

    DBUG_ENTER ("StructOpTake");

    result = NULL;

    /*
     * Try to convert expr(especially arrays) into a structual constant
     */
    struc_co = CFscoExpr2StructConstant (expr);

    /*
     * given expression could be converted to struct_constant
     */
    if (struc_co != NULL) {

        idxlen = SHgetUnrLen (COgetShape (idx));
        structdim = COgetDim (SCO_HIDDENCO (struc_co));

        if (idxlen <= structdim) {
            /*
             * save internal hidden input constant
             */
            old_hidden_co = SCO_HIDDENCO (struc_co);

            SCO_HIDDENCO (struc_co) = COtake (idx, SCO_HIDDENCO (struc_co));

            sco_shape = SCO_SHAPE (struc_co);
            dropped_shape = SHdropFromShape (structdim, sco_shape);
            sco_shape = SHfreeShape (sco_shape);
            SCO_SHAPE (struc_co)
              = SHappendShapes (COgetShape (SCO_HIDDENCO (struc_co)), dropped_shape);
            dropped_shape = SHfreeShape (dropped_shape);

            /*
             * return modified array
             */
            result = CFscoDupStructConstant2Expr (struc_co);

            /*
             * free tmp. struct constant
             */
            struc_co = CFscoFreeStructConstant (struc_co);

            /*
             * free internal input constant
             */
            old_hidden_co = COfreeConstant (old_hidden_co);
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *StructOpDrop(constant *idx, node *arg_expr)
 *
 * description:
 *   computes structural drop on array expressions with constant index vector.
 *
 *****************************************************************************/

static node *
StructOpDrop (constant *idx, node *expr)
{
    struct_constant *struc_co;
    constant *old_hidden_co;

    node *result;

    int idxlen;
    int structdim;

    shape *sco_shape;
    shape *dropped_shape;

    DBUG_ENTER ("StructOpDrop");

    result = NULL;

    /*
     * Try to convert expr(especially arrays) into a structual constant
     */
    struc_co = CFscoExpr2StructConstant (expr);

    /*
     * given expression could be converted to struct_constant
     */
    if (struc_co != NULL) {

        idxlen = SHgetUnrLen (COgetShape (idx));
        structdim = COgetDim (SCO_HIDDENCO (struc_co));

        if (idxlen <= structdim) {
            /*
             * save internal hidden input constant
             */
            old_hidden_co = SCO_HIDDENCO (struc_co);

            SCO_HIDDENCO (struc_co) = COdrop (idx, SCO_HIDDENCO (struc_co));

            sco_shape = SCO_SHAPE (struc_co);
            dropped_shape = SHdropFromShape (structdim, sco_shape);
            sco_shape = SHfreeShape (sco_shape);
            SCO_SHAPE (struc_co)
              = SHappendShapes (COgetShape (SCO_HIDDENCO (struc_co)), dropped_shape);
            dropped_shape = SHfreeShape (dropped_shape);

            /*
             * return modified array
             */
            result = CFscoDupStructConstant2Expr (struc_co);

            /*
             * free tmp. struct constant
             */
            struc_co = CFscoFreeStructConstant (struc_co);

            /*
             * free internal input constant
             */
            old_hidden_co = COfreeConstant (old_hidden_co);
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *ArithmOpWrapper( prf op, constant **arg_co, node **arg_expr)
 *
 * description:
 * implements arithmetical optimizations for add, sub, mul, div, and, or on one
 * constant arg and one other expression.
 *
 *****************************************************************************/

static node *
ArithmOpWrapper (prf op, constant **arg_co, node **arg_expr)
{
    node *result;
    node *expr;
    constant *co;
    bool swap;
    constant *tmp_co;
    shape *target_shp;

    DBUG_ENTER ("ArithmOpWrapper");

    /* get constant and expression, maybe we have to swap the arguments */
    if (arg_co[0] != NULL) {
        swap = FALSE;
        co = arg_co[0];
        expr = arg_expr[1];
    } else {
        swap = TRUE;
        co = arg_co[1];
        expr = arg_expr[0];
    }
    DBUG_ASSERT ((co != NULL), "no constant arg found");

    result = NULL;
    tmp_co = NULL;

    switch (op) {
    case F_add_SxS:
        if (COisZero (co, TRUE)) { /* x+0 -> x  or 0+x -> x */
            result = DUPdoDupTree (expr);
        }
        break;

    case F_sub_SxS:
        if (swap && COisZero (co, TRUE)) { /* x-0 -> x */
            result = DUPdoDupTree (expr);
        }
        break;

    case F_mul_SxS:
        if (COisZero (co, TRUE)) { /* x*0 -> 0 or 0*x -> 0 */
            target_shp = GetShapeOfExpr (expr);
            if (target_shp != NULL) {
                /* Create ZeroConstant of same type and shape as expression */
                tmp_co = COmakeZero (COgetType (co), target_shp);
            }
        } else if (COisOne (co, TRUE)) { /* x*1 -> x or 1*x -> x */
            result = DUPdoDupTree (expr);
        }
        break;

    case F_div_SxS:
        if (swap && COisZero (co, FALSE)) {
            /* any 0 in divisor, x/0 -> err */
            CTIabortLine (NODE_LINE (expr), "Division by zero expected");
        } else if (swap && COisOne (co, TRUE)) { /* x/1 -> x */
            result = DUPdoDupTree (expr);
        } else if ((!swap) && COisZero (co, TRUE)) { /* 0/x -> 0 */
            target_shp = GetShapeOfExpr (expr);
            if (target_shp != NULL) {
                /* Create ZeroConstant of same type and shape as expression */
                tmp_co = COmakeZero (COgetType (co), target_shp);
                CTIwarnLine (NODE_LINE (expr), "Expression 0/x replaced by 0");
            }
        }
        break;

    case F_and:
        if (COisTrue (co, TRUE)) { /* x&&true -> x or true&&x -> x */
            result = DUPdoDupTree (expr);
        } else if (COisFalse (co, TRUE)) { /* x&&false->false or false&&x->false */
            target_shp = GetShapeOfExpr (expr);
            if (target_shp != NULL) {
                /* Create False constant of same shape as expression */
                tmp_co = COmakeFalse (target_shp);
            }
        }
        break;

    case F_or:
        if (COisFalse (co, TRUE)) { /* x||false->x or false||x -> x */
            result = DUPdoDupTree (expr);
        } else if (COisFalse (co, TRUE)) { /* x||true->true or true&&x->true */
            target_shp = GetShapeOfExpr (expr);
            if (target_shp != NULL) {
                /* Create True constant of same shape as expression */
                tmp_co = COmakeTrue (target_shp);
            }
        }
        break;

    default:
        DBUG_ASSERT ((FALSE), "unsupported operation for arithmetical constant folding");
    }

    /* convert computed constant to exporession */
    if (tmp_co != NULL) {
        result = COconstant2AST (tmp_co);
        tmp_co = COfreeConstant (tmp_co);
    }

    if (result != NULL) {
        DBUG_PRINT ("CF",
                    ("arithmetic constant folding done for %s.", global.mdb_prf[op]));
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   shape *GetShapeOfExpr(node *expr)
 *
 * description:
 *   try to calculate the shape of the given expression. this can be a
 *   identifier or an array node. returns NULL if no shape can be computed.
 *
 *****************************************************************************/

static shape *
GetShapeOfExpr (node *expr)
{
    ntype *etype;
    shape *shp = NULL;

    DBUG_ENTER ("GetShapeOfExpr");

    DBUG_ASSERT ((expr != NULL), "GetShapeOfExpr called with NULL pointer");

    etype = NTCnewTypeCheck_Expr (expr);

    if (TUshapeKnown (etype)) {
        shp = SHcopyShape (TYgetShape (etype));
    }

    etype = TYfreeType (etype);

    DBUG_RETURN (shp);
}

/******************************************************************************
 *
 * function:
 *   simpletype GetBasetypeOfExpr(node *expr)
 *
 * description:
 *   try to get the basetype of the given expression. this can be a
 *   identifier or an array node. returns NULL if no type can be computed.
 *
 *****************************************************************************/

static simpletype
GetBasetypeOfExpr (node *expr)
{
    simpletype stype;
    ntype *etype;

    DBUG_ENTER ("GetBasetypeOfExpr");
    DBUG_ASSERT ((expr != NULL), "GetBasetypeOfExpr called with NULL pointer");

    etype = NTCnewTypeCheck_Expr (expr);

    stype = TYgetSimpleType (TYgetScalar (etype));

    etype = TYfreeType (etype);

    DBUG_RETURN (stype);
}

/******************************************************************************
 *
 * function:
 *   node *Eq(node *expr1, node *expr2)
 *
 * description:
 *   implements the F_eq primitive function for two expressions via a cmptree.
 *
 *****************************************************************************/

static node *
Eq (node *expr1, node *expr2)
{
    node *result;

    DBUG_ENTER ("Eq");

    if (CMPTdoCompareTree (expr1, expr2) == CMPT_EQ) {
        result = TBmakeBool (TRUE);
    } else {
        result = NULL; /* no concrete answer for equal operation possible */
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *Add(node *expr1, node *expr2)
 *
 * description:
 *   implements special optimization for x + _esd_neg_(x) -> 0
 *                                       _esd_neg_(x) + x -> 0
 *
 *****************************************************************************/

static node *
Add (node *expr1, node *expr2)
{
    node *result;
    constant *tmp_co;
    shape *target_shp;

    DBUG_ENTER ("Add");

    result = NULL;

    if ((NODE_TYPE (expr1) == N_id) && (NODE_TYPE (expr2) == N_id)
        && (TUshapeKnown (AVIS_TYPE (ID_AVIS (expr1))))
        && (TUshapeKnown (AVIS_TYPE (ID_AVIS (expr2))))) {
        node *ass1 = AVIS_SSAASSIGN (ID_AVIS (expr1));
        node *ass2 = AVIS_SSAASSIGN (ID_AVIS (expr2));

        if (((ass1 != NULL) && (NODE_TYPE (ASSIGN_RHS (ass1)) == N_prf)
             && (PRF_PRF (ASSIGN_RHS (ass1)) == F_esd_neg)
             && (ID_AVIS (PRF_ARG1 (ASSIGN_RHS (ass1))) == ID_AVIS (expr2)))
            || ((ass2 != NULL) && (NODE_TYPE (ASSIGN_RHS (ass2)) == N_prf)
                && (PRF_PRF (ASSIGN_RHS (ass2)) == F_esd_neg)
                && (ID_AVIS (PRF_ARG1 (ASSIGN_RHS (ass2))) == ID_AVIS (expr1)))) {
            target_shp = GetShapeOfExpr (expr1);
            if (target_shp != NULL) {
                /* Create ZeroConstant of same type and shape as expression */
                tmp_co = COmakeZero (GetBasetypeOfExpr (expr1), target_shp);
                result = COconstant2AST (tmp_co);
                tmp_co = COfreeConstant (tmp_co);
            }
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *Sub(node *expr1, node *expr2)
 *
 * description:
 *   implements special optimization for x - x -> 0
 *
 *****************************************************************************/

static node *
Sub (node *expr1, node *expr2)
{
    node *result;
    constant *tmp_co;
    shape *target_shp;

    DBUG_ENTER ("Sub");

    result = NULL;

    if (CMPTdoCompareTree (expr1, expr2) == CMPT_EQ) {
        target_shp = GetShapeOfExpr (expr1);
        if (target_shp != NULL) {
            /* Create ZeroConstant of same type and shape as expression */
            tmp_co = COmakeZero (GetBasetypeOfExpr (expr1), target_shp);
            result = COconstant2AST (tmp_co);
            tmp_co = COfreeConstant (tmp_co);
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *Modarray( node *a, constant *idx, node *elem)
 *
 * description:
 *   implement Modarray on generic structural constant arrays with given
 *   full constant index vector. This works like CFStructOpWrapper() but
 *   has been moved to a separate function because of different function
 *   signature.
 *
 ******************************************************************************/

static node *
Modarray (node *a, constant *idx, node *elem)
{
    node *result = NULL;
    struct_constant *struc_a;
    struct_constant *struc_elem;
    constant *old_hidden_co;
    ntype *atype;

    DBUG_ENTER ("Modarray");

    /**
     * if the index is an empty vector, we simply replace the entire
     * expression by the elem value!
     * Well, not quite!!! This is only valid, iff
     *      shape( elem) ==  shape( a)
     * If we do not know this, then the only thing we can do is
     * to replace the modarray by
     *      _type_conv_( type(a), elem)
     * iff a is AKS!
     * cf bug246 !!!
     */
    if (COisEmptyVect (idx)) {
        DBUG_ASSERT ((NODE_TYPE (a) == N_id),
                     "non id found in array-arg position of F_modarray");
        DBUG_ASSERT ((NODE_TYPE (elem) == N_id),
                     "non id found in elem-arg position of F_modarray");
        if (AVIS_SHAPE (ID_AVIS (a)) != NULL) {
            if (CMPTdoCompareTree (AVIS_SHAPE (ID_AVIS (a)), AVIS_SHAPE (ID_AVIS (elem)))
                == CMPT_EQ) {
                result = DUPdoDupTree (elem);
            }
        }
        atype = AVIS_TYPE (ID_AVIS (a));
        if ((result == NULL) && TUshapeKnown (atype)) {
            result = TCmakePrf2 (F_type_conv, TBmakeType (TYeliminateAKV (atype)),
                                 DUPdoDupTree (elem));
        }
    } else {
        /**
         * as we are not dealing with the degenerate case (idx == []),
         * we need a and elem to be structural constants in order to be
         * able to do anything!
         */
        struc_a = CFscoExpr2StructConstant (a);
        struc_elem = CFscoExpr2StructConstant (elem);

        /* given expressession could be converted to struct_constant */
        if ((struc_a != NULL) && (struc_elem != NULL)) {
            if (SCO_ELEMDIM (struc_a) == SCO_ELEMDIM (struc_elem)) {
                /* save internal hidden constant */
                old_hidden_co = SCO_HIDDENCO (struc_a);

                /* perform modarray operation on structural constant */
                SCO_HIDDENCO (struc_a)
                  = COmodarray (SCO_HIDDENCO (struc_a), idx, SCO_HIDDENCO (struc_elem));

                /* return modified array */
                result = CFscoDupStructConstant2Expr (struc_a);

                DBUG_PRINT ("CF", ("op computed on structural constant"));

                /* free internal constant */
                old_hidden_co = COfreeConstant (old_hidden_co);
            }
        }

        /* free struct constants */
        if (struc_a != NULL) {
            struc_a = CFscoFreeStructConstant (struc_a);
        }

        if (struc_elem != NULL) {
            struc_elem = CFscoFreeStructConstant (struc_elem);
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *CatVxV(node *vec1, node *vec2)
 *
 * description:
 *   tries to concatenate the given vectors as struct constants
 *
 *****************************************************************************/

static node *
CatVxV (node *vec1, node *vec2)
{
    node *result;
    struct_constant *sc_vec1;
    struct_constant *sc_vec2;

    DBUG_ENTER ("CatVxV");

    result = NULL;

    sc_vec1 = CFscoExpr2StructConstant (vec1);
    sc_vec2 = CFscoExpr2StructConstant (vec2);

    if ((sc_vec1 != NULL) && (sc_vec2 != NULL)) {
        constant *vec2_hidden_co;

        /*
         * if both vectors are structural constant we can concatenate then
         */
        vec2_hidden_co = SCO_HIDDENCO (sc_vec2);
        SCO_HIDDENCO (sc_vec2) = COcat (SCO_HIDDENCO (sc_vec1), SCO_HIDDENCO (sc_vec2));

        result = CFscoDupStructConstant2Expr (sc_vec2);

        vec2_hidden_co = COfreeConstant (vec2_hidden_co);
    } else if (sc_vec1 != NULL) {
        /*
         * if vec1 is a structural constant of shape [0],
         * the result is a copy of vec2
         */
        if (SHgetUnrLen (COgetShape (SCO_HIDDENCO (sc_vec1))) == 0) {
            result = DUPdoDupNode (vec2);
        }
    } else if (sc_vec2 != NULL) {
        /*
         * if vec2 is a structural constant of shape [0],
         * the result is a copy of vec1
         */
        if (SHgetUnrLen (COgetShape (SCO_HIDDENCO (sc_vec2))) == 0) {
            result = DUPdoDupNode (vec1);
        }
    }

    if (sc_vec1 != NULL)
        sc_vec1 = CFscoFreeStructConstant (sc_vec1);

    if (sc_vec2 != NULL)
        sc_vec2 = CFscoFreeStructConstant (sc_vec2);

    DBUG_RETURN (result);
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
 *      b = modarray(a, iv, value)
 *      x = sel(iv, b)    ->   x = value;
 *
 *   2. iv is an expression with known constant value:
 *      b = modarray(a, [5], val5);
 *      c = modarray(b, [3], val3);
 *      d = modarray(c, [2], val2);
 *      x = sel([5], d)   ->  x = val5;
 *
 *   maybe this allows to eliminate some arrays at all.
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

        case F_modarray:
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
                    /* no further analysis possible, because of non constant idx expr */
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

        case F_sel:
            prf_sel = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (array_expr)));
            concat = CatVxV (EXPRS_EXPR (PRF_ARGS (prf_sel)), idx_expr);

            if (concat != NULL) {
                result = TCmakePrf2 (F_sel, concat,
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

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node* CFfundef(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses args and block in this order.
 *   the args are only traversed in loop special functions to remove
 *   propagated constants from loop dependend arguments.
 *
 *****************************************************************************/

node *
CFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        if (FUNDEF_ISLACINLINE (arg_node)) {
            RMVdoRemoveVardecsOneFundef (arg_node);
        }
    }

    if (!INFO_ONEFUNDEF (arg_info)) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFblock(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses instructions only
 *
 *****************************************************************************/

node *
CFblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFblock");

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_INSTR (arg_node) == NULL) {
        /* insert at least the N_empty node in an empty block */
        BLOCK_INSTR (arg_node) = TBmakeEmpty ();
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFassign(node *arg_node, info *arg_info)
 *
 * description:
 *   top-down traversal of assignments. in bottom-up return traversal remove
 *   marked assignment-nodes from chain and insert moved assignments (e.g.
 *   from constant, inlined conditionals)
 *
 *****************************************************************************/

node *
CFassign (node *arg_node, info *arg_info)
{
    bool remassign;
    node *preassign;

    DBUG_ENTER ("CFassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /* save removal flag for modifications in bottom-up traversal */
    remassign = INFO_REMASSIGN (arg_info);
    preassign = INFO_PREASSIGN (arg_info);
    INFO_REMASSIGN (arg_info) = FALSE;
    INFO_PREASSIGN (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (remassign) {
        /* skip this assignment and free it */
        DBUG_PRINT ("CF", ("remove dead assignment"));
        arg_node = FREEdoFreeNode (arg_node);
    }

    if (preassign != NULL) {
        arg_node = TCappendAssign (preassign, arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 *  function:
 *    node *CFfuncond(node *arg_node, node* arg_info)
 *
 *  description:
 *    Check if the conditional predicate is constant.
 *    If it is constant, than resolve all funcond nodes according
 *    to the predicate and set the inline flag.
 *
 *****************************************************************************/
node *
CFfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFfuncond");

    /* check for constant condition */
    if (TYisAKV (ID_NTYPE (FUNCOND_IF (arg_node)))
        && ((!FUNDEF_ISDOFUN (INFO_FUNDEF (arg_info)))
            || (!COisTrue (TYgetValue (ID_NTYPE (FUNCOND_IF (arg_node))), TRUE)))) {
        node *tmp;
        if (COisTrue (TYgetValue (ID_NTYPE (FUNCOND_IF (arg_node))), TRUE)) {
            tmp = FUNCOND_THEN (arg_node);
            FUNCOND_THEN (arg_node) = NULL;
        } else {
            tmp = FUNCOND_ELSE (arg_node);
            FUNCOND_ELSE (arg_node) = NULL;
        }
        arg_node = FREEdoFreeTree (arg_node);
        arg_node = tmp;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFcond(node *arg_node, info *arg_info)
 *
 * description:
 *   checks for constant conditional - removes corresponding counterpart
 *   of the conditional.
 *
 *   traverses conditional and optional then-part, else-part
 *
 *****************************************************************************/

node *
CFcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFcond");

    /* check for constant condition */
    if (TYisAKV (ID_NTYPE (COND_COND (arg_node)))
        && ((!FUNDEF_ISDOFUN (INFO_FUNDEF (arg_info)))
            || (!COisTrue (TYgetValue (ID_NTYPE (COND_COND (arg_node))), TRUE)))) {
        if (COisTrue (TYgetValue (ID_NTYPE (COND_COND (arg_node))), TRUE)) {

            /*  traverse then-part */
            if (COND_THEN (arg_node) != NULL) {
                COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
            }

            /* select then-part for later insertion in assignment chain */
            INFO_PREASSIGN (arg_info) = BLOCK_INSTR (COND_THEN (arg_node));

            if (NODE_TYPE (INFO_PREASSIGN (arg_info)) == N_empty) {
                /* empty code block must not be moved */
                INFO_PREASSIGN (arg_info) = NULL;
            } else {
                /*
                 * delete pointer to codeblock to preserve assignments from
                 * being freed
                 */
                BLOCK_INSTR (COND_THEN (arg_node)) = NULL;
            }

            /*
             * if this is a do- or while function and the condition is evaluated
             * to true we have an endless loop and will rise a warning message.
             */
            if (FUNDEF_ISDOFUN (INFO_FUNDEF (arg_info))) {
                CTIwarnLine (NODE_LINE (arg_node),
                             "Infinite loop detected, program may not terminate");
            }
        } else {
            /*  traverse else-part */
            if (COND_ELSE (arg_node) != NULL) {
                COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
            }

            /* select else-part for later insertion in assignment chain */
            INFO_PREASSIGN (arg_info) = BLOCK_INSTR (COND_ELSE (arg_node));

            if (NODE_TYPE (INFO_PREASSIGN (arg_info)) == N_empty) {
                /* empty code block must not be moved */
                INFO_PREASSIGN (arg_info) = NULL;
            } else {
                /*
                 * delete pointer to codeblock to preserve assignments from
                 * being freed
                 */
                BLOCK_INSTR (COND_ELSE (arg_node)) = NULL;
            }
        }

        /*
         * mark this assignment for removal, the selected code part will
         * be inserted behind this conditional assignment and traversed
         * for constant folding.
         */
        INFO_REMASSIGN (arg_info) = TRUE;

        FUNDEF_ISDOFUN (INFO_FUNDEF (arg_info)) = FALSE;
        FUNDEF_ISCONDFUN (INFO_FUNDEF (arg_info)) = FALSE;
        FUNDEF_ISLACINLINE (INFO_FUNDEF (arg_info)) = TRUE;
    } else {
        /*
         * no constant condition:
         * do constant folding in conditional
         */
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFlet(node *arg_node, info *arg_info)
 *
 * description:
 *
 *****************************************************************************/

node *
CFlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFlet");

    /*
     * Try to replace the rhs with a constant (given by the lhs type) if
     * - RHS node IS NOT an N_funcond node (this would violate fun-form
     * - RHS is not yet constant
     */
    if ((NODE_TYPE (LET_EXPR (arg_node)) != N_funcond)
        && (!IsFullyConstantNode (LET_EXPR (arg_node)))) {

        /*
         * Traverse into LHS
         * This yields an assignment for each ids node with constant type
         */
        if (LET_IDS (arg_node) != NULL) {
            LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
        }

        /*
         * If ALL ids nodes are constant, the current assignment can be eliminated
         */
        if (TCcountIds (LET_IDS (arg_node))
            == TCcountAssigns (INFO_PREASSIGN (arg_info))) {

            /*
             * Set all AVIS_SSAASSIGN links
             */
            node *preass = INFO_PREASSIGN (arg_info);
            while (preass != NULL) {
                AVIS_SSAASSIGN (IDS_AVIS (ASSIGN_LHS (preass))) = preass;
                preass = ASSIGN_NEXT (preass);
            }

            global.optcounters.cf_expr += TCcountIds (LET_IDS (arg_node));
            INFO_REMASSIGN (arg_info) = TRUE;
        } else {
            if (INFO_PREASSIGN (arg_info) != NULL) {
                INFO_PREASSIGN (arg_info) = FREEdoFreeTree (INFO_PREASSIGN (arg_info));
            }
        }
    }

    /*
     * Traverse rhs only if it has not been replaced by constants
     */
    if (INFO_PREASSIGN (arg_info) == NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CFids (node *arg_node, info *arg_info)
{
    node *dim, *shape;

    DBUG_ENTER ("CFids");

    dim = AVIS_DIM (IDS_AVIS (arg_node));
    shape = AVIS_SHAPE (IDS_AVIS (arg_node));

    if (TYisAKV (IDS_NTYPE (arg_node))) {
        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (DUPdoDupNode (arg_node),
                                     COconstant2AST (TYgetValue (IDS_NTYPE (arg_node)))),
                          INFO_PREASSIGN (arg_info));
        /*
         * Do not yet set AVIS_SSAASSIGN to the new assignment
         * this is done in CFlet iff it turns out the assignment chain is
         * in fact required
         */
    }
#if 0
  /*
   * This has been commented out since when throwing away a WL on the RHS,
   * we MUST ensure that the references to shape variables defined in that
   * WL are removed. 
   * I see some solutions:
   * 1) Move shape attributes to IDS node (probably more DT like) 
   * 2) Traverse WL to remove all SV references first
   */
  else if ( ( dim != NULL) && ( shape != NULL)) {
    if ( ( NODE_TYPE( dim) == N_num) && ( NUM_VAL( dim) == 1) &&
         ( NODE_TYPE( shape) == N_array) && 
         ( NODE_TYPE( EXPRS_EXPR( ARRAY_AELEMS( shape))) == N_num) &&
         ( NUM_VAL( EXPRS_EXPR( ARRAY_AELEMS( shape))) == 0)) {
      ntype *ty = TYmakeAKS( TYcopyType( TYgetScalar( IDS_NTYPE( arg_node))),
                             SHmakeShape(0));

      INFO_PREASSIGN( arg_info) =
        TBmakeAssign( TBmakeLet( DUPdoDupNode( arg_node),
                                 TCmakeVector( ty, NULL)),
                      INFO_PREASSIGN( arg_info));
      
    }
  }
#endif

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFarray(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses array elements to propagate constant identifiers
 *
 ******************************************************************************/

node *
CFarray (node *arg_node, info *arg_info)
{
    node *newelems = NULL;
    node *oldelems, *tmp, *first_inner_array;
    shape *shp = NULL, *newshp;
    ntype *basetype;
    ntype *atype;

    DBUG_ENTER ("CFarray");

    /*
     * Test whether whole array can be replaced with an array constant
     */
    atype = NTCnewTypeCheck_Expr (arg_node);

    if (TYisAKV (atype)) {
        /*
         * replace it
         */
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = COconstant2AST (TYgetValue (atype));
    } else {
        /*
         * Try to merge subarrays in
         */
        if (ARRAY_AELEMS (arg_node) != NULL) {
            /*
             * All elements need to be id nodes defined by N_array nodes.
             * Furthermore, they must all add the same dimensionality to
             * the dimension of their children
             */
            tmp = ARRAY_AELEMS (arg_node);
            while (tmp != NULL) {
                if ((NODE_TYPE (EXPRS_EXPR (tmp)) != N_id)
                    || (ID_SSAASSIGN (EXPRS_EXPR (tmp)) == NULL)
                    || (NODE_TYPE (ASSIGN_RHS (ID_SSAASSIGN (EXPRS_EXPR (tmp))))
                        != N_array)) {
                    break;
                }
                oldelems = ASSIGN_RHS (ID_SSAASSIGN (EXPRS_EXPR (tmp)));

                if (shp == NULL)
                    shp = ARRAY_SHAPE (oldelems);
                else if (!SHcompareShapes (shp, ARRAY_SHAPE (oldelems)))
                    break;

                tmp = EXPRS_NEXT (tmp);
            }
            if (tmp == NULL) {
                /*
                 * Merge subarrays into this arrays
                 */
                oldelems = ARRAY_AELEMS (arg_node);
                DBUG_ASSERT (oldelems != NULL,
                             "Trying to merge subarrays into an empty array!");
                first_inner_array = ASSIGN_RHS (ID_SSAASSIGN (EXPRS_EXPR (oldelems)));
                tmp = oldelems;
                while (tmp != NULL) {
                    newelems
                      = TCappendExprs (newelems, DUPdoDupTree (ARRAY_AELEMS (ASSIGN_RHS (
                                                   ID_SSAASSIGN (EXPRS_EXPR (tmp))))));
                    tmp = EXPRS_NEXT (tmp);
                }

                basetype = TYcopyType (ARRAY_ELEMTYPE (first_inner_array));
                newshp = SHappendShapes (ARRAY_SHAPE (arg_node), shp);

                arg_node = FREEdoFreeNode (arg_node);

                arg_node = TBmakeArray (basetype, newshp, newelems);
            }
        }
    }

    atype = TYfreeType (atype);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CFfoldPrfExpr(prf op, node **arg_expr, info *arg_info)
 *
 * description:
 *   try to compute the primitive function for the given args.
 *   args must be a (static) node* array with len PRF_MAX_ARGS.
 *
 * returns:
 *   a computed result node or NULL if no computing is possible.
 *
 *****************************************************************************/
static node *
CFfoldPrfExpr (prf op, node **arg_expr, info *arg_info)
{
    node *new_node;
    constant *new_co;
    constant *arg_co_mem[PRF_MAX_ARGS];
    constant **arg_co = &arg_co_mem[0];
    int i;

    DBUG_ENTER ("CFfoldPrfExpr");

    /* init local variables */
    new_node = NULL;
    new_co = NULL;

    /* fill static arrays with converted constants */
    arg_co = Args2Const (arg_co, arg_expr, PRF_MAX_ARGS);

    /* do constant folding for selected primitive function */
    switch (op) {
        /* one-argument functions */
    case F_toi_S:
    case F_toi_A:
        if (T_int == TYgetSimpleType (TYgetScalar (ID_NTYPE (arg_expr[0])))) {
            new_node = DUPdoDupTree (arg_expr[0]);
        }
        break;

    case F_tod_S:
    case F_tod_A:
        if (T_double == TYgetSimpleType (TYgetScalar (ID_NTYPE (arg_expr[0])))) {
            new_node = DUPdoDupTree (arg_expr[0]);
        }
        break;

    case F_tof_S:
    case F_tof_A:
        if (T_float == TYgetSimpleType (TYgetScalar (ID_NTYPE (arg_expr[0])))) {
            new_node = DUPdoDupTree (arg_expr[0]);
        }
        break;

    case F_abs:
        break;

    case F_not:
        break;

    case F_dim:
        if
            ONE_ARG (arg_expr)
            {
                /* for some non full constant expression */
                new_node = Dim (arg_expr[0]);
            }
        break;

    case F_shape:
        if
            ONE_ARG (arg_expr)
            {
                /* for some non full constant expression */
                new_node = Shape (arg_expr[0], arg_info);
            }
        break;

    case F_idx_shape_sel:
        if ((arg_co[0] != NULL) && (arg_expr[1] != NULL)) {
            new_node = IdxShapeSel (arg_co[0], arg_expr[1], arg_info);
        }
        break;

        /* two-argument functions */
    case F_min:
        break;

    case F_max:
        break;

    case F_add_SxS:
        if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = ArithmOpWrapper (F_add_SxS, arg_co, arg_expr);
            }
        else if
            TWO_ARG (arg_expr)
            {
                new_node = Add (arg_expr[0], arg_expr[1]);
            }
        break;
    case F_add_AxA:
        if
            TWO_ARG (arg_expr)
            {
                new_node = Add (arg_expr[0], arg_expr[1]);
            }
        break;
    case F_add_AxS:
        if
            SECOND_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                if (COisZero (arg_co[1], FALSE)) { /* A + 0 */
                    new_node = DUPdoDupTree (arg_expr[0]);
                }
            }
        break;

    case F_add_SxA:
        if
            FIRST_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                if (COisZero (arg_co[0], FALSE)) { /* 0 + A */
                    new_node = DUPdoDupTree (arg_expr[1]);
                }
            }
        break;

    case F_sub_SxS:
        if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = ArithmOpWrapper (F_sub_SxS, arg_co, arg_expr);
            }
        else if
            TWO_ARG (arg_expr)
            {
                new_node = Sub (arg_expr[0], arg_expr[1]);
            }
        break;

    case F_sub_AxA:
        if
            TWO_ARG (arg_expr)
            {
                new_node = Sub (arg_expr[0], arg_expr[1]);
            }
        break;

    case F_sub_AxS:
        if
            SECOND_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                if (COisZero (arg_co[1], FALSE)) { /* A - 0 */
                    new_node = DUPdoDupTree (arg_expr[0]);
                }
            }
    case F_sub_SxA:
        break;

    case F_mul_SxS:
        if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = ArithmOpWrapper (F_mul_SxS, arg_co, arg_expr);
            }
        break;
    case F_mul_AxS:
        if
            SECOND_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                if (COisOne (arg_co[1], FALSE)) { /* A * 1 */
                    new_node = DUPdoDupTree (arg_expr[0]);
                }
            }
        break;

    case F_mul_SxA:
        if
            FIRST_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                if (COisOne (arg_co[0], FALSE)) { /* 1 * A */
                    new_node = DUPdoDupTree (arg_expr[1]);
                }
            }
        break;

    case F_mul_AxA:
        break;

    case F_div_SxS:
        if
            TWO_CONST_ARG (arg_co)
            {
                if (COisZero (arg_co[1], FALSE)) { /* any 0 in divisor, x/0 -> err */
                    CTIabortLine (NODE_LINE (arg_expr[1]), "Division by zero expected");
                }
            }
        else if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = ArithmOpWrapper (F_div_SxS, arg_co, arg_expr);
            }
        break;

    case F_div_AxS:
        if
            SECOND_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                if (COisOne (arg_co[1], FALSE)) { /* A / 1 */
                    new_node = DUPdoDupTree (arg_expr[0]);
                }
            }
        break;

    case F_div_SxA:
    case F_div_AxA:
        break;

    case F_mod:
        if
            TWO_CONST_ARG (arg_co)
            {
                if (COisZero (arg_co[1], FALSE)) { /* any 0 in divisor, x/0 -> err */
                    CTIabortLine (NODE_LINE (arg_expr[1]), "Division by zero expected");
                }
            }
        break;

    case F_and:
        if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = ArithmOpWrapper (F_and, arg_co, arg_expr);
            }
        break;

    case F_or:
        if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = ArithmOpWrapper (F_or, arg_co, arg_expr);
            }
        break;

    case F_le:
        break;

    case F_lt:
        break;

    case F_eq:
        if
            TWO_ARG (arg_expr)
            {
                /* for two expressions (does a treecmp) */
                new_node = Eq (arg_expr[0], arg_expr[1]);
            }
        break;

    case F_ge:
        break;

    case F_gt:
        break;

    case F_neq:
        break;

    case F_reshape:
        if
            TWO_CONST_ARG (arg_co)
            {
            }
        else if ((arg_co[0] != NULL) && (arg_expr[1] != NULL)) {
            /* for some non constant expression and constant index vector */
            new_node = StructOpReshape (arg_co[0], arg_expr[1]);
            if ((new_node == NULL) && (NODE_TYPE (arg_expr[1]) == N_id)) {
                /* reshape( shp, a)  ->  a    iff (shp == shape(a)) */
                if (TUshapeKnown (ID_NTYPE (arg_expr[1]))) {
                    if (SHcompareWithCArray (TYgetShape (ID_NTYPE (arg_expr[1])),
                                             COgetDataVec (arg_co[0]),
                                             SHgetExtent (COgetShape (arg_co[0]), 0))) {
                        new_node = DUPdoDupNode (arg_expr[1]);
                    }
                }
            }
        } else if (((arg_expr[0] != NULL) && (arg_expr[1] != NULL))
                   && ((AVIS_SHAPE (ID_AVIS (arg_expr[1])) != NULL)
                       && ((CMPTdoCompareTree (arg_expr[0],
                                               AVIS_SHAPE (ID_AVIS (arg_expr[1])))
                            == CMPT_EQ)
                           || ((AVIS_SSAASSIGN (ID_AVIS (arg_expr[0])) != NULL)
                               && (CMPTdoCompareTree (ASSIGN_RHS (AVIS_SSAASSIGN (
                                                        ID_AVIS (arg_expr[0]))),
                                                      AVIS_SHAPE (ID_AVIS (arg_expr[1])))
                                   == CMPT_EQ))))) {
            new_node = DUPdoDupNode (arg_expr[1]);
        }
        break;

    case F_sel:
        if
            FIRST_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                /* for some non constant expression and constant index vector */
                new_node = StructOpSel (arg_co[0], arg_expr[1]);
            }

        if ((new_co == NULL) && (new_node == NULL) && (TWO_ARG (arg_expr))) {
            /* for some expressions concerning sel-modarray combinations or
               sel-sel combinations */
            new_node = Sel (arg_expr[0], arg_expr[1]);
        }
        break;

    case F_take_SxV:
    case F_take:
        if
            TWO_CONST_ARG (arg_co)
            {
                /* for pure constant args */
                new_co = COtake (arg_co[0], arg_co[1]);
            }
        else if
            FIRST_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                /* for some non constant expression and constant index vector */
                new_node = StructOpTake (arg_co[0], arg_expr[1]);
            }
        break;

    case F_drop_SxV:
    case F_drop:
        if
            TWO_CONST_ARG (arg_co)
            {
                /* for pure constant args */
                new_co = COdrop (arg_co[0], arg_co[1]);
            }
        else if
            FIRST_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                /* for some non constant expression and constant index vector */
                new_node = StructOpDrop (arg_co[0], arg_expr[1]);
            }
        break;

    case F_cat_VxV:
        if
            TWO_CONST_ARG (arg_co)
            {
            }
        else if (FIRST_CONST_ARG_OF_TWO (arg_co, arg_expr)
                 && (SHgetUnrLen (COgetShape (arg_co[0])) == 0)) {

            new_node = DUPdoDupNode (arg_expr[1]);
        } else if (SECOND_CONST_ARG_OF_TWO (arg_co, arg_expr)
                   && (SHgetUnrLen (COgetShape (arg_co[1])) == 0)) {

            new_node = DUPdoDupNode (arg_expr[0]);
        } else {
            new_node = CatVxV (arg_expr[0], arg_expr[1]);
        }
        break;

        /* three-argument functions */
    case F_modarray:
        if (SECOND_CONST_ARG_OF_THREE (arg_co, arg_expr)) {
            new_node = Modarray (arg_expr[0], arg_co[1], arg_expr[2]);
        }
        break;

#ifdef HADROTATE
        if
            we had a rotate primitive, this would do the trick

              /* Rotate by zero is trivial */
              if FIRST_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                if (COisZero (arg_co[0], FALSE)) { /* rotate([0], A) */
                    new_node = DUPdoDupTree (arg_expr[1]);
                }
            }
        break;
#endif

    /* not implemented yet */
    case F_rotate:
    case F_cat:
        break;

    default:
        DBUG_PRINT ("CF", ("no implementation in SSAConstantFolding for prf %s",
                           global.mdb_prf[op]));
    }

    /* free used constant data */
    for (i = 0; i < PRF_MAX_ARGS; i++) {
        if (arg_co[i] != NULL) {
            arg_co[i] = COfreeConstant (arg_co[i]);
        }
    }

    /*
     * if we got a new computed expression instead of the primitive function
     * we create a new expression with the result
     */
    if ((new_co != NULL) || (new_node != NULL)) {
        if (new_co != NULL) {
            /* create new node with constant value instead of prf node */
            new_node = COconstant2AST (new_co);
            new_co = COfreeConstant (new_co);
        } else {
            /* some constant expression of non full constant args have been computed */
        }
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFprf(node *arg_node, info *arg_info)
 *
 * description:
 *   evaluates primitive function with constant paramters and substitutes
 *   the function application by its value.
 *
 *****************************************************************************/

node *
CFprf (node *arg_node, info *arg_info)
{
    node *new_node;
    node *arg_expr_mem[PRF_MAX_ARGS];
    node **arg_expr = &arg_expr_mem[0];

    DBUG_ENTER ("CFprf");

    DBUG_PRINT ("CF", ("evaluating prf %s", global.mdb_prf[PRF_PRF (arg_node)]));

    /* look up arguments */
    arg_expr = GetPrfArgs (arg_expr, PRF_ARGS (arg_node), PRF_MAX_ARGS);

    /* try some constant folding */
    new_node = CFfoldPrfExpr (PRF_PRF (arg_node), arg_expr, arg_info);

    if (new_node != NULL) {
        /* free this primitive function and substitute it with new node */
        arg_node = FREEdoFreeTree (arg_node);
        arg_node = new_node;

        /* increment constant folding counter */
        global.optcounters.cf_expr++;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CFwith (node *arg_node, info *arg_info)
{
    node *vecassign = NULL;

    DBUG_ENTER ("CFwith");

    /*
     * Create a fake assignment for the index vector in case the variables
     * need to be looked up.
     */
    if (WITH_IDS (arg_node) != NULL) {
        vecassign
          = TBmakeAssign (TBmakeLet (DUPdoDupNode (WITH_VEC (arg_node)),
                                     TCmakeIntVector (TCids2Exprs (WITH_IDS (arg_node)))),
                          NULL);
        AVIS_SSAASSIGN (IDS_AVIS (WITH_VEC (arg_node))) = vecassign;
    }

    /*
     * Codes are traversed via the Parts to allow for exploiting generators
     * of width one.
     */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /*
     * Remove the fake assignment after the traversal
     */
    if (vecassign != NULL) {
        AVIS_SSAASSIGN (IDS_AVIS (WITH_VEC (arg_node))) = NULL;
        vecassign = FREEdoFreeTree (vecassign);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFcode( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CFcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFcode");

    /*
     * Do not traverse CODE_NEXT since codes are traversed through the Parts
     */
    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFpart( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CFpart (node *arg_node, info *arg_info)
{
    ntype *temp;
    node *n;

    DBUG_ENTER ("CFpart");

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    /*
     * Try to temporarily upgrade the types of the index variables to AKV
     * in case some width of the generator is known to be one
     */
    if ((CODE_USED (PART_CODE (arg_node)) == 1)
        && (NODE_TYPE (PART_GENERATOR (arg_node)) == N_generator)
        && (GENERATOR_GENWIDTH (PART_GENERATOR (arg_node)) != NULL)) {
        node *gen = PART_GENERATOR (arg_node);
        ntype *gwtype;
        ntype *lbtype;

        /*
         * Try to upgrade the type of the index vector
         */
        gwtype = NTCnewTypeCheck_Expr (GENERATOR_GENWIDTH (gen));
        lbtype = NTCnewTypeCheck_Expr (GENERATOR_BOUND1 (gen));

        if ((TYisAKV (gwtype)) && (COisOne (TYgetValue (gwtype), TRUE))
            && (TYisAKV (lbtype))) {
            IDS_NTYPE (PART_VEC (arg_node))
              = TYfreeType (IDS_NTYPE (PART_VEC (arg_node)));
            IDS_NTYPE (PART_VEC (arg_node)) = TYcopyType (lbtype);
        }

        gwtype = TYfreeType (gwtype);
        lbtype = TYfreeType (lbtype);

        /*
         * Try to upgrade the types of the index scalars
         */
        if ((NODE_TYPE (GENERATOR_GENWIDTH (gen)) == N_array)
            && (NODE_TYPE (GENERATOR_BOUND1 (gen)) == N_array)) {
            node *lbe = ARRAY_AELEMS (GENERATOR_BOUND1 (gen));
            node *gwe = ARRAY_AELEMS (GENERATOR_GENWIDTH (gen));
            n = PART_IDS (arg_node);

            while (n != NULL) {
                gwtype = NTCnewTypeCheck_Expr (EXPRS_EXPR (gwe));
                lbtype = NTCnewTypeCheck_Expr (EXPRS_EXPR (lbe));

                if ((TYisAKV (gwtype)) && (COisOne (TYgetValue (gwtype), TRUE))
                    && (TYisAKV (lbtype))) {
                    IDS_NTYPE (n) = TYfreeType (IDS_NTYPE (n));
                    IDS_NTYPE (n) = TYcopyType (lbtype);
                }

                gwtype = TYfreeType (gwtype);
                lbtype = TYfreeType (lbtype);

                n = IDS_NEXT (n);
                lbe = EXPRS_NEXT (lbe);
                gwe = EXPRS_NEXT (gwe);
            }
        }
    }

    /*
     * Traverse this parts code if it has not yet been traversed.
     * Mark the code as completely traversed afterwards by inverting CODE_USED
     */
    if (CODE_USED (PART_CODE (arg_node)) > 0) {
        PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
        CODE_USED (PART_CODE (arg_node)) *= -1;
    }

    /*
     * Revert types of index variables to AKS
     */
    temp = IDS_NTYPE (PART_VEC (arg_node));
    IDS_NTYPE (PART_VEC (arg_node)) = TYeliminateAKV (temp);
    temp = TYfreeType (temp);

    n = PART_IDS (arg_node);
    while (n != NULL) {
        temp = IDS_NTYPE (n);
        IDS_NTYPE (n) = TYeliminateAKV (temp);
        temp = TYfreeType (temp);

        n = IDS_NEXT (n);
    }

    /*
     * Traverse PART_NEXT
     */
    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

    /*
     * Revert CODE_USED to correct state
     */
    CODE_USED (PART_CODE (arg_node)) = abs (CODE_USED (PART_CODE (arg_node)));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Constant folding -->
 *****************************************************************************/
