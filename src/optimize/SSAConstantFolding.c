/*
 *
 * $Log$
 * Revision 1.31  2002/09/03 18:47:43  dkr
 * - TAGGED_ARRAYS: constants propagation for N_ap activated again
 * - SSACFid(): support for dynamic types added
 *
 * Revision 1.29  2002/07/29 12:12:53  sbs
 * PRF_IF macro extended by z.
 *
 * Revision 1.28  2002/07/12 19:37:24  dkr
 * TAGGED_ARRAYS: constants propagation for N_ap deactivated
 *
 * Revision 1.27  2002/06/21 14:02:05  dkr
 * no changes done
 *
 * Revision 1.26  2002/04/08 19:58:14  dkr
 * debug code removed
 *
 * Revision 1.24  2001/12/14 16:37:54  dkr
 * bug in SSACFExpr2StructConstant() fixed
 *
 * Revision 1.23  2001/12/11 15:57:12  dkr
 * SSACFDim(): GetDim() used instead of GetShapeDim()
 *
 * Revision 1.22  2001/12/11 15:52:39  dkr
 * GetDim() replaced by GetShapeDim()
 *
 * Revision 1.21  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 1.20  2001/06/01 11:35:01  nmw
 * handling for infinite loops improoved
 *
 * Revision 1.19  2001/06/01 10:00:34  nmw
 * insert N_empty node in empty blocks
 *
 * [...]
 *
 * Revision 1.1  2001/03/20 16:16:54  nmw
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   SSAConstantFolding.c
 *
 * prefix: SSACF
 *
 * description:
 *   this module implementes constant folding on code in ssa form. for
 *   constant expressions we compute primitive functions at compile time
 *   so we need not to compute them at runtime. this simplyfies the code
 *   and allows further optimizations.
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
 *****************************************************************************/

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "SSAConstantFolding.h"
#include "constants.h"
#include "optimize.h"
#include "SSATransform.h"
#include "Inline.h"
#include "compare_tree.h"

/*
 * constant identifiers should be substituted by its constant value
 * also in the arg-chain of N_ap and N_prf nodes (not for applications
 * of special fundefs! there must always be identifers).
 */
#define SUBST_ID_WITH_CONSTANT_IN_AP_ARGS TRUE

#define SUBST_NONE 0
#define SUBST_SCALAR 1
#define SUBST_SCALAR_AND_ARRAY 2

/* prf-function name for debug output */
#ifndef DBUG_OFF
#define PRF_IF(n, s, x, y, z) x
static char *prf_string[] = {
#include "prf_node_info.mac"
};
#undef PRF_IF
#endif

/* maximum of supported args for primitive functions */
#define PRF_MAX_ARGS 3

/* macros used in SSACFFoldPrfExpr to handle arguments selections */
#define ONE_CONST_ARG(arg) (arg[0] != NULL)

#define TWO_CONST_ARG(arg) ((arg[0] != NULL) && (arg[1] != NULL))

#define ONE_CONST_ARG_OF_TWO(arg, arg_expr)                                              \
    ((((arg[0] == NULL) && (arg[1] != NULL)) || ((arg[0] != NULL) && (arg[1] == NULL)))  \
     && (arg_expr[0] != NULL) && (arg_expr[1] != NULL))

#define FIRST_CONST_ARG_OF_TWO(arg, arg_expr) ((arg[0] != NULL) && (arg_expr[1] != NULL))

#define THREE_CONST_ARG(arg) ((arg[0] != NULL) && (arg[1] != NULL) && (arg[2] != NULL))

#define ONE_ARG(arg_expr) (arg_expr[0] != NULL)

#define TWO_ARG(arg_expr) ((arg_expr[0] != NULL) && (arg_expr[1] != NULL))

#define THREE_ARG(arg)                                                                   \
    ((arg_expr[0] != NULL) && (arg_expr[1] != NULL) && (arg_expr[2] != NULL))

#define SECOND_CONST_ARG_OF_THREE(arg, arg_expr)                                         \
    ((arg_expr[0] != NULL) && (arg[1] != NULL) && (arg_expr[2] != NULL))

/* structural constant StructCO should be integrated in constants in future */
/* special constant version used for structural constants */
typedef struct {
    simpletype simpletype; /* basetype of struct constant */
    char *name;            /* only used for T_user !! */
    char *name_mod;        /* name of modul belonging to 'name' */
    constant *hidden_co;   /* pointer to constant of pointers */
} struc_constant;

/* access macros for structural constant type */
#define SCO_BASETYPE(n) (n->simpletype)
#define SCO_NAME(n) (n->name)
#define SCO_MOD(n) (n->name_mod)
#define SCO_HIDDENCO(n) (n->hidden_co)

/* functions to handle StructCO constants */
static struc_constant *SSACFExpr2StructConstant (node *expr);
static struc_constant *SSACFArray2StructConstant (node *expr);
static struc_constant *SSACFScalar2StructConstant (node *expr);
static node *SSACFDupStructConstant2Expr (struc_constant *struc_co);
static struc_constant *SCOFreeStructConstant (struc_constant *struc_co);

/* local used helper functions */
static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *SSACFids (ids *arg_ids, node *arg_info);
static node *SSACFPropagateConstants2Args (node *arg_chain, node *param_chain);
static ids *SSACFSetSSAASSIGN (ids *chain, node *assign);
static node **SSACFGetPrfArgs (node **array, node *prf_arg_chain, int max_args);
static constant **SSACFArgs2Const (constant **co_array, node **arg_expr, int max_args);
static shape *SSACFGetShapeOfExpr (node *expr);
static node *RemovePhiCopyTargetAttributes (node *vardecs, bool thenpart);

/*
 * primitive functions for non full-constant expressions like:
 *   dimension constant
 *   shape constant
 *   structural constant
 *
 * they have to be implemented seperatly as long as there is no constant type
 * that can handle all these cases internally
 */

static constant *SSACFDim (node *expr);
static constant *SSACFShape (node *expr);

/* implements: sel, reshape, take, drop */
static node *SSACFStructOpWrapper (prf op, constant *idx, node *expr);

/* implements: arithmetical opt. for add, sub, mul, div, and, or */
static node *SSACFArithmOpWrapper (prf op, constant **arg_co, node **arg_expr);

/*
 * some primitive functions that allows special optimizations in more
 * generic cases
 */
static node *SSACFEq (node *expr1, node *expr2);
static node *SSACFSub (node *expr1, node *expr2);
static node *SSACFModarray (node *a, constant *idx, node *elem);
static node *SSACFSel (node *idx_expr, node *array_expr);

/*
 * functions for internal use only
 */

/******************************************************************************
 *
 * function:
 *   node *SSACFPropagateConstants2Args(node *arg_chain, node *const_arg_chain)
 *
 * description:
 *   to propagate constant expressions from the calling context into a special
 *   function, this functions does a parallel traversal of the function args
 *   (stored in arg_chain) and the calling parameters (stored param_chain)
 *
 *****************************************************************************/

static node *
SSACFPropagateConstants2Args (node *arg_chain, node *param_chain)
{
    node *arg;

    DBUG_ENTER ("SSACFPropagateConstants2Args");

    arg = arg_chain;
    while (arg != NULL) {
        DBUG_ASSERT ((param_chain != NULL),
                     "different arg chains in fun definition/fun application");

        if (AVIS_SSACONST (ARG_AVIS (arg)) == NULL) {
            /* arg not marked as constant - try to make new constant */
            AVIS_SSACONST (ARG_AVIS (arg)) = COAST2Constant (EXPRS_EXPR (param_chain));
        }

        /* traverse to next element in both chains */
        arg = ARG_NEXT (arg);
        param_chain = EXPRS_NEXT (param_chain);
    }

    DBUG_RETURN (arg_chain);
}

/******************************************************************************
 *
 * function:
 *   ids *SSACFSetSSAASSIGN(ids *chain, node *assign)
 *
 * description:
 *   sets the AVIS_SSAASSIGN to the correct assignment.
 *   the backrefs may be wrong after Inlining, because DupTree is not able
 *   to correct them when dubbing an assignment chain only.
 *
 *****************************************************************************/

static ids *
SSACFSetSSAASSIGN (ids *chain, node *assign)
{
    DBUG_ENTER ("SSACFSetSSAASSIGN");

    if (chain != NULL) {
        /* set current assign as defining assignement */
        if (AVIS_SSAPHITARGET (IDS_AVIS (chain)) == PHIT_NONE) {
            AVIS_SSAASSIGN (IDS_AVIS (chain)) = assign;
        }

        /* check correct setting of SSAASSIGN() attribute */
        if (AVIS_SSAASSIGN (IDS_AVIS (chain)) != assign) {
            DBUG_PRINT ("WARN",
                        ("mismatch SSAASSIGN link for %s (phit: %d) - 1:%p, 2:%p - %p",
                         IDS_NAME (chain), AVIS_SSAPHITARGET (IDS_AVIS (chain)),
                         AVIS_SSAASSIGN (IDS_AVIS (chain)),
                         AVIS_SSAASSIGN2 (IDS_AVIS (chain)), assign));
        }

        /* traverse to next ids */
        IDS_NEXT (chain) = SSACFSetSSAASSIGN (IDS_NEXT (chain), assign);
    }

    DBUG_RETURN (chain);
}

/******************************************************************************
 *
 * function:
 *   node **SSACFGetPrfArgs(node **array, node *prf_arg_chain, int max_args)
 *
 * description:
 *   fills an pointer array of len max_args with the args from an exprs chain
 *   or NULL if there are no more args.
 *
 *****************************************************************************/

static node **
SSACFGetPrfArgs (node **array, node *prf_arg_chain, int max_args)
{
    node *expr;
    int i;

    DBUG_ENTER ("SSACFGetPrfArgs");

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
 *   constant **SSACFArgs2Const(constant **co_array,
 *                              node **arg_expr,
 *                              int max_args)
 *
 * description:
 *   converts all expr nodes to constant node and store them in an array of
 *   constants (co_array).
 *
 *****************************************************************************/

static constant **
SSACFArgs2Const (constant **co_array, node **arg_expr, int max_args)
{
    int i;

    DBUG_ENTER ("SSACFArgs2Const");

    for (i = 0; i < max_args; i++) {
        if (arg_expr[i] != NULL) {
            co_array[i] = COAST2Constant (arg_expr[i]);
        } else {
            co_array[i] = NULL;
        }
    }

    DBUG_RETURN (co_array);
}

/******************************************************************************
 *
 * function:
 *   constant *SSACFDim(node *expr)
 *
 * description:
 *   tries computes the dimension of an identifier and returns it as
 *   constant for later usage or NULL if the dimension is not known.
 *
 *****************************************************************************/

static constant *
SSACFDim (node *expr)
{
    constant *result;
    int dim;

    DBUG_ENTER ("SSACFDim");

    if (NODE_TYPE (expr) == N_id) {
        dim = GetDim (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (expr))));
        if (dim >= 0) {
            result = COMakeConstantFromInt (dim);
        } else {
            result = NULL;
        }
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SSACFShape(node *expr)
 *
 * description:
 *   computes the shape of a given identifier. returns the shape as expression
 *   or NULL if no constant shape can be infered.
 *   for userdefined types the result is the shape in simpletype elements.
 *
 *****************************************************************************/

static constant *
SSACFShape (node *expr)
{
    constant *result;
    int dim;
    shape *cshape;
    int *int_vec;

    DBUG_ENTER ("SSACFDim");

    if (NODE_TYPE (expr) == N_id) {
        dim = GetShapeDim (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (expr))));
        if (KNOWN_SHAPE (dim)) {
            /* store known shape as constant (int vector of len dim) */
            cshape
              = SHOldTypes2Shape (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (expr))));
            int_vec = SHShape2IntVec (cshape);
            cshape = SHFreeShape (cshape);

            result = COMakeConstant (T_int, SHCreateShape (1, dim), int_vec);
        } else {
            result = NULL;
        }
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SSACFStructOpWrapper(prf op, constant *idx, node *arg_expr)
 *
 * description:
 *   computes structural ops on array expressions with constant index vector.
 *   the array elements are stored as pointer in a constant of type T_hidden.
 *   the structural operation is performed on this hidden constant and
 *   the resulting elements are copied in a new result array.
 *
 *****************************************************************************/

static node *
SSACFStructOpWrapper (prf op, constant *idx, node *expr)
{
    struc_constant *struc_co;
    node *result;
    constant *old_hidden_co;

    DBUG_ENTER ("SSACFStructOpWrapper");

    /* tries to convert expr(especially arrays) into a structual constant */
    struc_co = SSACFExpr2StructConstant (expr);

    /* given expressession could be converted to struc_constant */
    if (struc_co != NULL) {
        /* save internal hidden input constant */
        old_hidden_co = SCO_HIDDENCO (struc_co);

        /* perform struc-op on hidden constant */
        switch (op) {
        case F_sel:
            SCO_HIDDENCO (struc_co) = COSel (idx, SCO_HIDDENCO (struc_co));
            break;

        case F_reshape:
            SCO_HIDDENCO (struc_co) = COReshape (idx, SCO_HIDDENCO (struc_co));
            break;

        case F_take:
            SCO_HIDDENCO (struc_co) = COTake (idx, SCO_HIDDENCO (struc_co));
            break;

        case F_drop:
            SCO_HIDDENCO (struc_co) = CODrop (idx, SCO_HIDDENCO (struc_co));
            break;

        default:
            DBUG_ASSERT ((FALSE), "primitive function on arrays not implemented");
        }

        /* return modified array */
        result = SSACFDupStructConstant2Expr (struc_co);

        DBUG_PRINT ("SSACF", ("op %s computed on structural constant", prf_string[op]));

        /* free tmp. struct constant */
        struc_co = SCOFreeStructConstant (struc_co);

        /* free internal input constant */
        old_hidden_co = COFreeConstant (old_hidden_co);
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   struc_constant *SSACFExpr2StructConstant(node *expr)
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

static struc_constant *
SSACFExpr2StructConstant (node *expr)
{
    struc_constant *struc_co;
    int dim;

    DBUG_ENTER ("SSACFExpr2StructConstant");

    struc_co = NULL;

    if (NODE_TYPE (expr) == N_array) {
        /* expression is an array */
        struc_co = SSACFArray2StructConstant (expr);
    } else if ((NODE_TYPE (expr) == N_id) && (AVIS_SSAASSIGN (ID_AVIS (expr)) != NULL)) {
        /* expression is an identifier */
        dim = GetShapeDim (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (expr))));
        if (dim == SCALAR) {
            /* id is a defined scalar */
            struc_co = SSACFScalar2StructConstant (expr);
        } else if (dim > SCALAR) {
            /* id is a defined array */
            struc_co = SSACFArray2StructConstant (expr);
        }
    }

    DBUG_RETURN (struc_co);
}

/******************************************************************************
 *
 * function:
 *   struc_constant *SSACFArray2StructConstant(node *expr)
 *
 * description:
 *   converts an N_array node (or a N_id of a defined array) from AST to
 *   a structural constant. To convert an array to a structural constant
 *   all array elements must be scalars!
 *
 *****************************************************************************/

static struc_constant *
SSACFArray2StructConstant (node *expr)
{
    struc_constant *struc_co;
    node *array;
    types *atype;
    shape *ashape;
    node **node_vec;
    node *tmp;
    bool valid_const;
    int elem_count;
    int i;

    DBUG_ENTER ("SSACFArray2StructConstant");

    DBUG_ASSERT (((NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_id)),
                 "SSACFArray2StructConstant supports only N_array and N_id nodes");

    atype = NULL;

    if (NODE_TYPE (expr) == N_array) {
        /* explicit array as N_array node */
        array = expr;
        /* shape of the given array */
        DBUG_ASSERT ((ARRAY_TYPE (array) != NULL), "unknown array type");
        atype = ARRAY_TYPE (array);

    } else if ((NODE_TYPE (expr) == N_id) && (AVIS_SSAASSIGN (ID_AVIS (expr)) != NULL)
               && (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (expr)))))
                   == N_array)) {
        /* indirect array via defined vardec */

        array = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (expr))));

        /* shape of the given array */
        atype = VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (expr)));
    } else {
        /* unsupported node type */
        array = NULL;
    }

    /* build an abstract structural constant of type (void*) T_hidden */
    if (array != NULL) {
        /* alloc hidden vector */
        ashape = SHOldTypes2Shape (atype);
        elem_count = SHGetUnrLen (ashape);
        node_vec = (node **)Malloc (elem_count * sizeof (node *));

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
        DBUG_ASSERT ((tmp == NULL), "array contains too much elements");

        /* create struc_constant */
        struc_co = (struc_constant *)Malloc (sizeof (struc_constant));
        SCO_BASETYPE (struc_co) = GetBasetype (atype);
        SCO_NAME (struc_co) = TYPES_NAME (atype);
        SCO_MOD (struc_co) = TYPES_MOD (atype);
        SCO_HIDDENCO (struc_co) = COMakeConstant (T_hidden, ashape, node_vec);

        /* remove invalid structural arrays */
        if (!valid_const) {
            struc_co = SCOFreeStructConstant (struc_co);
        }
    } else {
        /* no array with known elements */
        struc_co = NULL;
    }

    DBUG_RETURN (struc_co);
}

/******************************************************************************
 *
 * function:
 *   struc_constant *SSACFScalar2StructConstant(node *expr)
 *
 * description:
 *   converts an scalar node to a structual constant (e.g. N_num, ... or N_id)
 *
 ******************************************************************************/

static struc_constant *
SSACFScalar2StructConstant (node *expr)
{
    struc_constant *struc_co;
    shape *cshape;
    types *ctype;
    node **elem;
    nodetype nt;

    DBUG_ENTER ("SSACFScalar2StructConstant");

    nt = NODE_TYPE (expr);

    if ((nt == N_num) || (nt == N_float) || (nt == N_double) || (nt == N_bool)
        || ((nt == N_id)
            && (GetShapeDim (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (expr))))
                == SCALAR))) {
        /* create structural constant of scalar */
        ctype = VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (expr)));

        /* alloc hidden vector */
        cshape = SHMakeShape (0);
        elem = (node **)Malloc (sizeof (node *));

        /* copy element pointers from array to vector */
        *elem = expr;

        /* create struc_constant */
        struc_co = (struc_constant *)Malloc (sizeof (struc_constant));
        SCO_BASETYPE (struc_co) = TYPES_BASETYPE (ctype);
        SCO_NAME (struc_co) = TYPES_NAME (ctype);
        SCO_MOD (struc_co) = TYPES_MOD (ctype);
        SCO_HIDDENCO (struc_co) = COMakeConstant (T_hidden, cshape, elem);

    } else {
        struc_co = NULL;
    }

    DBUG_RETURN (struc_co);
}

/******************************************************************************
 *
 * function:
 *   node *SSACFDupStructConstant2Expr(struc_constant *struc_co)
 *
 * description:
 *   builds an array of the given strucural constant and duplicate
 *   elements in it. therfore the original array must not be freed before
 *   the target array is build up from the elements of the original array.
 *
 *****************************************************************************/

static node *
SSACFDupStructConstant2Expr (struc_constant *struc_co)
{
    node *expr;
    node *aelems;
    int i;
    int elems_count;
    node **node_vec;

    DBUG_ENTER ("SSACFDupStructConstant2Expr");

    /* build up elements chain */
    node_vec = (node **)COGetDataVec (SCO_HIDDENCO (struc_co));

    if (COGetDim (SCO_HIDDENCO (struc_co)) == 0) {
        /* result is a scalar */
        expr = DupNode (node_vec[0]);
    } else {
        /* result is a new array */
        elems_count = SHGetUnrLen (COGetShape (SCO_HIDDENCO (struc_co)));

        aelems = NULL;
        for (i = elems_count - 1; i >= 0; i--) {
            aelems = MakeExprs (DupNode (node_vec[i]), aelems);
        }

        /* build array node */
        expr = MakeArray (aelems);
        ARRAY_TYPE (expr)
          = MakeTypes (SCO_BASETYPE (struc_co), COGetDim (SCO_HIDDENCO (struc_co)),
                       SHShape2OldShpseg (COGetShape (SCO_HIDDENCO (struc_co))),
                       StringCopy (SCO_NAME (struc_co)), SCO_MOD (struc_co));
    }
    DBUG_RETURN (expr);
}

/******************************************************************************
 *
 * function:
 *   struc_constant *SCOFreeStructConstant(struc_constant *struc_co)
 *
 * description:
 *   frees the struc_constant data structure and the internal constant element.
 *
 *****************************************************************************/

static struc_constant *
SCOFreeStructConstant (struc_constant *struc_co)
{
    DBUG_ENTER ("SCOFreeStructConstant");

    DBUG_ASSERT ((struc_co != NULL), "SCOFreeStructConstant: NULL pointer");

    /* free substructure */
    SCO_HIDDENCO (struc_co) = COFreeConstant (SCO_HIDDENCO (struc_co));

    /* free structure */
    struc_co = Free (struc_co);

    DBUG_RETURN ((struc_constant *)NULL);
}

/******************************************************************************
 *
 * function:
 *   node *SSACFArithmOpWrapper(prf op,
 *                              constant **arg_co,
 *                              node **arg_expr)
 *
 * description:
 * implements arithmetical optimizations for add, sub, mul, div, and, or on one
 * constant arg and one other expression.
 *
 *****************************************************************************/

static node *
SSACFArithmOpWrapper (prf op, constant **arg_co, node **arg_expr)
{
    node *result;
    node *expr;
    constant *co;
    bool swap;
    constant *tmp_co;
    shape *target_shp;

    DBUG_ENTER ("SSACFArithmOpWrapper");

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
    case F_add:
        if (COIsZero (co, TRUE)) { /* x+0 -> x  or 0+x -> x */
            result = DupTree (expr);
        }
        break;

    case F_sub:
        if (swap && COIsZero (co, TRUE)) { /* x-0 -> x */
            result = DupTree (expr);
        }
        break;

    case F_mul:
        if (COIsZero (co, TRUE)) { /* x*0 -> 0 or 0*x -> 0 */
            target_shp = SSACFGetShapeOfExpr (expr);
            if (target_shp != NULL) {
                /* Create ZeroConstant of same type and shape as expression */
                tmp_co = COMakeZero (COGetType (co), target_shp);
            }
        } else if (COIsOne (co, TRUE)) { /* x*1 -> x or 1*x -> x */
            result = DupTree (expr);
        }
        break;

    case F_div:
        if (swap && COIsZero (co, FALSE)) {
            /* any 0 in divisor, x/0 -> err */
            ABORT (expr->lineno, ("Division by zero expected"));
        } else if (swap && COIsOne (co, TRUE)) { /* x/1 -> x */
            result = DupTree (expr);
        } else if ((!swap) && COIsZero (co, TRUE)) { /* 0/x -> 0 */
            target_shp = SSACFGetShapeOfExpr (expr);
            if (target_shp != NULL) {
                /* Create ZeroConstant of same type and shape as expression */
                tmp_co = COMakeZero (COGetType (co), target_shp);
                WARN (expr->lineno, ("expression 0/x replaced by 0"));
            }
        }
        break;

    case F_and:
        if (COIsTrue (co, TRUE)) { /* x&&true -> x or true&&x -> x */
            result = DupTree (expr);
        } else if (COIsFalse (co, TRUE)) { /* x&&false->false or false&&x->false */
            target_shp = SSACFGetShapeOfExpr (expr);
            if (target_shp != NULL) {
                /* Create False constant of same shape as expression */
                tmp_co = COMakeFalse (target_shp);
            }
        }
        break;

    case F_or:
        if (COIsFalse (co, TRUE)) { /* x||false->x or false||x -> x */
            result = DupTree (expr);
        } else if (COIsFalse (co, TRUE)) { /* x||true->true or true&&x->true */
            target_shp = SSACFGetShapeOfExpr (expr);
            if (target_shp != NULL) {
                /* Create True constant of same shape as expression */
                tmp_co = COMakeFalse (target_shp);
            }
        }
        break;

    default:
        DBUG_ASSERT ((FALSE), "unsupported operation for arithmetical constant folding");
    }

    /* convert computed constant to exporession */
    if (tmp_co != NULL) {
        result = COConstant2AST (tmp_co);
        tmp_co = COFreeConstant (tmp_co);
    }

#ifndef DBUG_OFF
    if (result != NULL) {
        DBUG_PRINT ("SSACF",
                    ("arithmetic constant folding done for %s.", prf_string[op]));
    }
#endif

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   shape *SSACFGetShapeOfExpr(node *expr)
 *
 * description:
 *   try to calculate the shape of the given expression. this can be a
 *   identifier or an array node. returns NULL if no shape can be computed.
 *
 *****************************************************************************/

static shape *
SSACFGetShapeOfExpr (node *expr)
{
    shape *shp;
    DBUG_ENTER ("SSACFGetShapeOfExpr");
    DBUG_ASSERT ((expr != NULL), "SSACFGetShapeOfExpr called with NULL pointer");

    switch (NODE_TYPE (expr)) {
    case N_id:
        /* get shape from type info */
        shp = SHOldTypes2Shape (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (expr))));
        DBUG_ASSERT ((shp != NULL), "identifier with unknown shape");
        break;

    case N_array:
        /* get shape from array type attribute */
        DBUG_ASSERT ((ARRAY_TYPE (expr) != NULL), "array type attribute is missing");
        shp = SHOldTypes2Shape (ARRAY_TYPE (expr));
        DBUG_ASSERT ((shp != NULL), "array with unknown shape");
        break;

    default:
        shp = NULL;
    }

    DBUG_RETURN (shp);
}

/******************************************************************************
 *
 * function:
 *   simpletype SSACFGetBasetypeOfExpr(node *expr)
 *
 * description:
 *   try to get the basetype of the given expression. this can be a
 *   identifier or an array node. returns NULL if no type can be computed.
 *
 *****************************************************************************/

static simpletype
SSACFGetBasetypeOfExpr (node *expr)
{
    simpletype stype;
    DBUG_ENTER ("SSACFGetBasetypeOfExpr");
    DBUG_ASSERT ((expr != NULL), "SSACFGetBasetypeOfExpr called with NULL pointer");

    switch (NODE_TYPE (expr)) {
    case N_id:
        /* get basetype from type info */
        stype = GetBasetype (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (expr))));
        break;

    case N_array:
        /* get shape from array type attribute */
        DBUG_ASSERT ((ARRAY_TYPE (expr) != NULL), "array type attribute is missing");
        stype = GetBasetype (ARRAY_TYPE (expr));
        break;

    default:
        stype = T_unknown;
    }

    DBUG_RETURN (stype);
}

/******************************************************************************
 *
 * function:
 *   node *SSACFEq(node *expr1, node *expr2)
 *
 * description:
 *   implements the F_eq primitive function for two expressions via a cmptree.
 *
 *****************************************************************************/

static node *
SSACFEq (node *expr1, node *expr2)
{
    node *result;

    DBUG_ENTER ("SSACFEq");

    if (CompareTree (expr1, expr2) == CMPT_EQ) {
        result = MakeBool (TRUE);
    } else {
        result = NULL; /* no concrete answer for equal operation possible */
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SSACFSub(node *expr1, node *expr2)
 *
 * description:
 *   implements special optimization for x - x -> 0
 *
 *****************************************************************************/

static node *
SSACFSub (node *expr1, node *expr2)
{
    node *result;
    constant *tmp_co;
    shape *target_shp;

    DBUG_ENTER ("SSACFSub");

    result = NULL;

    if (CompareTree (expr1, expr2) == CMPT_EQ) {
        target_shp = SSACFGetShapeOfExpr (expr1);
        if (target_shp != NULL) {
            /* Create ZeroConstant of same type and shape as expression */
            tmp_co = COMakeZero (SSACFGetBasetypeOfExpr (expr1), target_shp);
            result = COConstant2AST (tmp_co);
            tmp_co = COFreeConstant (tmp_co);
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SSACFModarray( node *a, constant *idx, node *elem)
 *
 * description:
 *   implement Modarray on gerneric structural constant arrays with given
 *   full constant index vector. This works like SSACFStructOpWrapper() but
 *   has been moved to a separate function because of different function
 *   signature.
 *
 ******************************************************************************/

static node *
SSACFModarray (node *a, constant *idx, node *elem)
{
    node *result;
    struc_constant *struc_a;
    struc_constant *struc_elem;
    constant *old_hidden_co;

    DBUG_ENTER ("SSACFModarray");

    struc_a = SSACFExpr2StructConstant (a);
    struc_elem = SSACFExpr2StructConstant (elem);

    /* given expressession could be converted to struc_constant */
    if ((struc_a != NULL) && (struc_elem != NULL)) {
        /* save internal hidden constant */
        old_hidden_co = SCO_HIDDENCO (struc_a);

        /* perform modarray operation on structural constant */
        SCO_HIDDENCO (struc_a)
          = COModarray (SCO_HIDDENCO (struc_a), idx, SCO_HIDDENCO (struc_elem));

        /* return modified array */
        result = SSACFDupStructConstant2Expr (struc_a);

        DBUG_PRINT ("SSACF", ("op computed on structural constant"));

        /* free internal constant */
        old_hidden_co = COFreeConstant (old_hidden_co);
    } else {
        result = NULL;
    }

    /* free struct constants */
    if (struc_a != NULL) {
        struc_a = SCOFreeStructConstant (struc_a);
    }

    if (struc_elem != NULL) {
        struc_elem = SCOFreeStructConstant (struc_elem);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SSACFSel(node *idx_expr, node *array_expr)
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
SSACFSel (node *idx_expr, node *array_expr)
{
    node *result;
    node *prf_mod;
    node *mod_arr_expr;
    node *mod_idx_expr;
    node *mod_elem_expr;
    constant *idx_co;
    constant *mod_idx_co;

    DBUG_ENTER ("SSACFSel");

    result = NULL;

    /*
     * checks if array_expr is an array identifier defined by a primitive
     * modarray operation
     */
    if ((NODE_TYPE (array_expr) == N_id)
        && (AVIS_SSAASSIGN (ID_AVIS (array_expr)) != NULL)
        && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (array_expr)))) == N_prf)
        && (PRF_PRF (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (array_expr)))) == F_modarray)) {

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
        idx_co = COAST2Constant (idx_expr);
        mod_idx_co = COAST2Constant (mod_idx_expr);

        if ((CompareTree (idx_expr, mod_idx_expr) == CMPT_EQ)
            || ((idx_co != NULL) && (mod_idx_co != NULL)
                && (COCompareConstants (idx_co, mod_idx_co)))) {
            /*
             * idx vectors in sel and modarray are equal
             * - replace sel() with element
             */
            result = DupTree (mod_elem_expr);

            DBUG_PRINT ("SSACF", ("sel-modarray optimization done"));

        } else {
            /* index vector does not match, but if both are constant, we can try
             * to look up futher in a modarray chain to find a matching one.
             * to avoid wrong decisions we need constant vectors in both idx
             * expressions.
             */
            if ((idx_co != NULL) && (mod_idx_co != NULL)) {
                result = SSACFSel (idx_expr, mod_arr_expr);
            } else {
                /* no further analysis possible, because of non constant idx expr */
                result = NULL;
            }
        }

        /* free local constants */
        if (idx_co != NULL) {
            idx_co = COFreeConstant (idx_co);
        }
        if (mod_idx_co != NULL) {
            mod_idx_co = COFreeConstant (mod_idx_co);
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *RemovePhiCopyTargetAttributes(node* vardecs, bool thenpart)
 *
 * description:
 *   removes all phi copy target attributes from all vardecs and set
 *   the SSAASSIGN(2) atrtribute to the correct assign in thenpart (== TRUE)
 *   or elsepart (== FALSE)
 *
 *****************************************************************************/

static node *
RemovePhiCopyTargetAttributes (node *vardecs, bool thenpart)
{
    node *act_vardec;

    DBUG_ENTER ("RemovePhiCopyTargetAttributes");

    act_vardec = vardecs;
    while (act_vardec != NULL) {

        if (AVIS_SSAPHITARGET (VARDEC_AVIS (act_vardec)) != PHIT_NONE) {
            if (thenpart) {
                /* AVIS_SSAASSIGN() is correct */
                AVIS_SSAASSIGN2 (VARDEC_AVIS (act_vardec)) = NULL;
            } else {
                AVIS_SSAASSIGN (VARDEC_AVIS (act_vardec))
                  = AVIS_SSAASSIGN2 (VARDEC_AVIS (act_vardec));
                AVIS_SSAASSIGN2 (VARDEC_AVIS (act_vardec)) = NULL;
            }

            /* vardec is no longer phi-copy target */
            AVIS_SSAPHITARGET (VARDEC_AVIS (act_vardec)) = PHIT_NONE;
        }
        act_vardec = VARDEC_NEXT (act_vardec);
    }

    DBUG_RETURN (vardecs);
}

/*
 * traversal functions for SSACF traversal
 */

/******************************************************************************
 *
 * function:
 *   node* SSACFfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses args and block in this order.
 *   the args are only traversed in loop special functions to remove
 *   propagated constants from loop dependend arguments.
 *
 *****************************************************************************/

node *
SSACFfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFfundef");

    INFO_SSACF_FUNDEF (arg_info) = arg_node;

    if ((FUNDEF_ARGS (arg_node) != NULL) && (FUNDEF_IS_LOOPFUN (arg_node))) {
        /* traverse args of fundef */
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* save block for inserting vardecs during the traversal */
        INFO_SSACF_TOPBLOCK (arg_info) = FUNDEF_BODY (arg_node);

        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFblock(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses instructions only
 *
 *****************************************************************************/

node *
SSACFblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) == NULL) {
        /* insert at least the N_empty node in an empty block */
        BLOCK_INSTR (arg_node) = MakeEmpty ();
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFarg(node *arg_node, node *arg_info)
 *
 * description:
 *   checks if only loop invariant arguments are constant
 *   SSACFarg is only called for  special loop fundefs
 *
 *****************************************************************************/

node *
SSACFarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFarg");

    /* constants for non loop invariant args are useless */
    if ((!(AVIS_SSALPINV (ARG_AVIS (arg_node)))
         && (AVIS_SSACONST (ARG_AVIS (arg_node)) != NULL))) {
        /* free constant */
        AVIS_SSACONST (ARG_AVIS (arg_node))
          = COFreeConstant (AVIS_SSACONST (ARG_AVIS (arg_node)));
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFassign(node *arg_node, node *arg_info)
 *
 * description:
 *   top-down traversal of assignments. in bottom-up return traversal remove
 *   marked assignment-nodes from chain and insert moved assignments (e.g.
 *   from constant, inlined conditionals)
 *
 *****************************************************************************/

node *
SSACFassign (node *arg_node, node *arg_info)
{
    bool remove_assignment;
    node *tmp;
    node *old_assign;

    DBUG_ENTER ("SSACFassign");

    /* stack current assignment */
    old_assign = INFO_SSACF_ASSIGN (arg_info);

    /* init flags for possible code removal/movement */
    INFO_SSACF_REMASSIGN (arg_info) = FALSE;
    INFO_SSACF_POSTASSIGN (arg_info) = NULL;
    INFO_SSACF_ASSIGN (arg_info) = arg_node;
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    /* restore old assignment */
    INFO_SSACF_ASSIGN (arg_info) = old_assign;

    /* save removal flag for modifications in bottom-up traversal */
    remove_assignment = INFO_SSACF_REMASSIGN (arg_info);

    /* integrate post assignments after current assignment */
    ASSIGN_NEXT (arg_node)
      = AppendAssign (INFO_SSACF_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
    INFO_SSACF_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (remove_assignment) {
        /* skip this assignment and free it */
        DBUG_PRINT ("SSACF", ("remove dead assignment"));

        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);

        tmp = FreeNode (tmp);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFcond(node *arg_node, node *arg_info)
 *
 * description:
 *   checks for constant conditional - removes corresponding counterpart
 *   of the conditional.
 *
 *   traverses conditional and optional then-part, else-part
 *
 *****************************************************************************/

node *
SSACFcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFcond");

    DBUG_ASSERT ((COND_COND (arg_node) != NULL), "missing condition in conditional");
    DBUG_ASSERT ((COND_THEN (arg_node) != NULL), "missing then part in conditional");
    DBUG_ASSERT ((COND_ELSE (arg_node) != NULL), "missing else part in conditional");

    /*
     * traverse condition to analyse for constant expression
     * and substitute constants with their values to get
     * a simple N_bool node for the condition (if constant)
     */
    INFO_SSACF_INSCONST (arg_info) = SUBST_SCALAR;
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);
    INFO_SSACF_INSCONST (arg_info) = SUBST_NONE;

    /* check for constant condition */
    if (NODE_TYPE (COND_COND (arg_node)) == N_bool) {
        if (BOOL_VAL (COND_COND (arg_node)) == TRUE) {
            DBUG_PRINT ("SSACF",
                        ("found condition with condition==true, select then part"));

            /* select then-part for later insertion in assignment chain */
            INFO_SSACF_POSTASSIGN (arg_info) = BLOCK_INSTR (COND_THEN (arg_node));

            if (NODE_TYPE (INFO_SSACF_POSTASSIGN (arg_info)) == N_empty) {
                /* empty code block must not be moved */
                INFO_SSACF_POSTASSIGN (arg_info) = NULL;
            } else {
                /*
                 * delete pointer to codeblock to preserve assignments from
                 * being freed
                 */
                BLOCK_INSTR (COND_THEN (arg_node)) = NULL;
            }
        } else {
            /* select else part */
            DBUG_PRINT ("SSACF",
                        ("found condition with condition==false, select else part"));

            /* select else-part for later insertion in assignment chain */
            INFO_SSACF_POSTASSIGN (arg_info) = BLOCK_INSTR (COND_ELSE (arg_node));

            if (NODE_TYPE (INFO_SSACF_POSTASSIGN (arg_info)) == N_empty) {
                /* empty code block must not be moved */
                INFO_SSACF_POSTASSIGN (arg_info) = NULL;
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
        INFO_SSACF_REMASSIGN (arg_info) = TRUE;

        /*
         * because there can be only one conditional in a special function
         * now this special function contains no conditional and therefore
         * is no special function anymore. this function can now be inlined
         * without any problems.
         * if this is a do- or while function and the condition is evaluated
         * to true we have an endless loop and will rise a warning message.
         */
        if ((BOOL_VAL (COND_COND (arg_node)) == TRUE)
            && (FUNDEF_IS_LOOPFUN (INFO_SSACF_FUNDEF (arg_info)))) {
            WARN (NODE_LINE (arg_node),
                  ("infinite loop detected, program may not terminate"));

            /* ex special function cannot be inlined and is now a regular one */
            FUNDEF_STATUS (INFO_SSACF_FUNDEF (arg_info)) = ST_regular;
            FUNDEF_USED (INFO_SSACF_FUNDEF (arg_info)) = USED_INACTIVE;
            FUNDEF_VARDEC (INFO_SSACF_FUNDEF (arg_info))
              = RemovePhiCopyTargetAttributes (FUNDEF_VARDEC (
                                                 INFO_SSACF_FUNDEF (arg_info)),
                                               TRUE);
        } else {
            /* ex special function can be simply inlined in calling context */
            INFO_SSACF_INLFUNDEF (arg_info) = TRUE;
            FUNDEF_VARDEC (INFO_SSACF_FUNDEF (arg_info))
              = RemovePhiCopyTargetAttributes (FUNDEF_VARDEC (
                                                 INFO_SSACF_FUNDEF (arg_info)),
                                               BOOL_VAL (COND_COND (arg_node)));
        }

    } else {
        /*
         * no constant condition:
         * do constant folding in conditional
         * traverse then-part
         */
        if (COND_THEN (arg_node) != NULL) {
            COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
        }

        /* traverse else-part */
        if (COND_ELSE (arg_node) != NULL) {
            COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
        }
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFreturn(node *arg_node, node *arg_info)
 *
 * description:
 *   do NOT substitute identifiers in return statement with their value!
 *
 *****************************************************************************/

node *
SSACFreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFreturn");

    /* do NOT substitue constant identifiers with their value */
    INFO_SSACF_INSCONST (arg_info) = SUBST_NONE;
    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFlet(node *arg_node, node *arg_info)
 *
 * description:
 *   checks expression for constant value and sets corresponding AVIS_SSACONST
 *   attribute for later usage.
 *   if constant folding has eliminated the condtional in a special function
 *   this function can be inlined here, because it is no longer a special one.
 *
 *****************************************************************************/

node *
SSACFlet (node *arg_node, node *arg_info)
{
    constant *new_co;

    DBUG_ENTER ("SSACFlet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "let without expression");

    /* remove tag SSAPHITARGET if conditional has been removed */
    if ((INFO_SSACF_INLFUNDEF (arg_info) == TRUE)
        && (AVIS_SSAPHITARGET (IDS_AVIS (LET_IDS (arg_node))) != PHIT_NONE)) {
        /* phi-copy-target is now normal identifier */
        AVIS_SSAPHITARGET (IDS_AVIS (LET_IDS (arg_node))) = PHIT_NONE;
    }

    /*
     * left side is not marked as constant -> compute expression
     * if there is a special function application with multiple results
     * the constant results will be removed by dead code removal
     * so this conditions holds here, too. for a general function
     * application there is no constant propagation allowed.
     */
    if ((LET_IDS (arg_node) != NULL)
        && (AVIS_SSACONST (IDS_AVIS (LET_IDS (arg_node))) == NULL)) {

        /* traverse expression to calculate constants */
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

        if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
            /*
             * handling for constant back-propagation from special fundefs
             * traverse ids chain and result chain of the concerning return
             * statement. for each constant identifier add a separate
             * assignent and substitute the result identifier in the
             * function application with a dummy identifier (that will be
             * removed by the next dead code removal)
             */
            if (INFO_SSACF_RESULTS (arg_info) != NULL) {
                LET_IDS (arg_node) = TravIDS (LET_IDS (arg_node), arg_info);
            }

            /* called function can be inlined */
            if (INFO_SSACF_INLINEAP (arg_info)) {
                DBUG_PRINT ("SSACF", ("inline function %s in %s",
                                      FUNDEF_NAME (AP_FUNDEF (LET_EXPR (arg_node))),
                                      FUNDEF_NAME (INFO_SSACF_FUNDEF (arg_info))));

                /* inline special function */
                INFO_SSACF_POSTASSIGN (arg_info)
                  = AppendAssign (InlineSingleApplication (arg_node,
                                                           INFO_SSACF_FUNDEF (arg_info)),
                                  INFO_SSACF_POSTASSIGN (arg_info));

                FUNDEF_STATUS (AP_FUNDEF (LET_EXPR (arg_node))) = ST_regular;

                /* remove this assignment and reset the inline flag */
                INFO_SSACF_REMASSIGN (arg_info) = TRUE;

                INFO_SSACF_INLINEAP (arg_info) = FALSE;
            }

        } else {
            /* set AVIS_SSACONST attributes */
            DBUG_ASSERT ((IDS_NEXT (LET_IDS (arg_node)) == NULL),
                         "only one result allowed for non N_ap nodes");

            /*
             * do not set SSACONST in phi target variables due to two different
             * definitions of this variable in one conditional.
             * the phi-copy-target definition in loop then parts are never constant
             * because they are defined as result of the recursive call.
             * the assignments in the else part of a tail-end recursive loop can be
             * used without any problems.
             */
            if (AVIS_SSAPHITARGET (IDS_AVIS (LET_IDS (arg_node))) == PHIT_COND) {
                new_co = NULL;
            } else {
                new_co = COAST2Constant (LET_EXPR (arg_node));
            }

            if (new_co != NULL) {
                AVIS_SSACONST (IDS_AVIS (LET_IDS (arg_node))) = new_co;
                DBUG_PRINT ("SSACF", ("identifier %s marked as constant",
                                      VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (
                                        IDS_AVIS (LET_IDS (arg_node))))));
            } else {
                /* expression is not constant */
                DBUG_PRINT ("SSACF", ("identifier %s is not constant",
                                      VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (
                                        IDS_AVIS (LET_IDS (arg_node))))));
            }
        }
    } else {
        /* left side is already maked as constant - no further processing needed */
    }

    /* update defining assigns after inlining */
    LET_IDS (arg_node)
      = SSACFSetSSAASSIGN (LET_IDS (arg_node), INFO_SSACF_ASSIGN (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFap(node *arg_node, node *arg_info)
 *
 * description:
 *   propagate constants and traverse in special function
 *
 *****************************************************************************/

node *
SSACFap (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("SSACFap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    /* substitute scalar constants in arguments (if no special function) */
    if (FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node))) {
        INFO_SSACF_INSCONST (arg_info) = SUBST_NONE;
    } else {
        INFO_SSACF_INSCONST (arg_info)
          = SUBST_SCALAR && SUBST_ID_WITH_CONSTANT_IN_AP_ARGS;
    }

    /* traverse arg chain */
    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }
    INFO_SSACF_INSCONST (arg_info) = SUBST_NONE;

    /* traverse special fundef without recursion */
    if ((FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_SSACF_FUNDEF (arg_info))) {

        DBUG_PRINT ("SSACF", ("traverse in special fundef %s",
                              FUNDEF_NAME (AP_FUNDEF (arg_node))));
        INFO_SSACF_MODUL (arg_info)
          = CheckAndDupSpecialFundef (INFO_SSACF_MODUL (arg_info), AP_FUNDEF (arg_node),
                                      INFO_SSACF_ASSIGN (arg_info));

        DBUG_ASSERT ((FUNDEF_USED (AP_FUNDEF (arg_node)) == 1),
                     "more than one instance of special function used.");

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        INFO_SSACF_MODUL (new_arg_info) = INFO_SSACF_MODUL (arg_info);
        INFO_SSACF_INLFUNDEF (new_arg_info) = FALSE;

        /* propagate constant args into called special function */
        FUNDEF_ARGS (AP_FUNDEF (arg_node))
          = SSACFPropagateConstants2Args (FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                                          AP_ARGS (arg_node));

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        /* save exprs chain of return list for later propagating constants */
        INFO_SSACF_RESULTS (arg_info)
          = RETURN_EXPRS (FUNDEF_RETURN (AP_FUNDEF (arg_node)));

        /* can this special function be inlined? */
        if (INFO_SSACF_INLFUNDEF (new_arg_info) == TRUE) {
            INFO_SSACF_INLINEAP (arg_info) = TRUE;
        } else {
            INFO_SSACF_INLINEAP (arg_info) = FALSE;
        }

        DBUG_PRINT ("SSACF", ("traversal of special fundef %s finished\n",
                              FUNDEF_NAME (AP_FUNDEF (arg_node))));
        new_arg_info = FreeTree (new_arg_info);

    } else {
        /* no traversal into a normal fundef */
        DBUG_PRINT ("SSACF", ("do not traverse in normal fundef %s",
                              FUNDEF_NAME (AP_FUNDEF (arg_node))));
        INFO_SSACF_RESULTS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFid(node *arg_node, node *arg_info)
 *
 * description:
 *   substitute identifers with their computed constant
 *   (only when INFO_SSACF_INSCONST flag is set)
 *   in EXPRS chain of N_ap ARGS  (if SUBST_ID_WITH_CONSTANT_IN_AP_ARGS == TRUE)
 *      EXPRS chain of N_prf ARGS (if SUBST_ID_WITH_CONSTANT_IN_AP_ARGS == TRUE)
 *      EXPRS chain of N_array AELEMS
 *
 *****************************************************************************/

node *
SSACFid (node *arg_node, node *arg_info)
{
    node *new_node;
    int dim;

    DBUG_ENTER ("SSACFid");

    /* check for constant scalar identifier */
    if (AVIS_SSACONST (ID_AVIS (arg_node)) != NULL) {
        dim = COGetDim (AVIS_SSACONST (ID_AVIS (arg_node)));

        if (((dim == SCALAR) && (INFO_SSACF_INSCONST (arg_info) >= SUBST_SCALAR))
            || ((dim > SCALAR)
                && (INFO_SSACF_INSCONST (arg_info) == SUBST_SCALAR_AND_ARRAY))) {
            DBUG_PRINT ("SSACF",
                        ("substitue identifier %s through its value",
                         VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (arg_node)))));

            /* substitute identifier with its value */
            new_node = COConstant2AST (AVIS_SSACONST (ID_AVIS (arg_node)));
            arg_node = FreeTree (arg_node);
            arg_node = new_node;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFarray(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses array elements to propagate constant identifiers
 *
 ******************************************************************************/

node *
SSACFarray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFarray");

    /* substitute constant identifiers in array elements */
    INFO_SSACF_INSCONST (arg_info) = TRUE;
    if (ARRAY_AELEMS (arg_node) != NULL) {
        ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);
    }
    INFO_SSACF_INSCONST (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFprf(node *arg_node, node *arg_info)
 *
 * description:
 *   evaluates primitive function with constant paramters and substitutes
 *   the function application by its value.
 *
 *****************************************************************************/

node *
SSACFprf (node *arg_node, node *arg_info)
{
    node *new_node;
    node *arg_expr_mem[PRF_MAX_ARGS];
    node **arg_expr = &arg_expr_mem[0];

    DBUG_ENTER ("SSACFprf");

    DBUG_PRINT ("SSACF", ("evaluating prf %s", prf_string[PRF_PRF (arg_node)]));

    /* substitute constant identifiers in prf. arguments */
    INFO_SSACF_INSCONST (arg_info) = SUBST_ID_WITH_CONSTANT_IN_AP_ARGS;
    if (PRF_ARGS (arg_node) != NULL) {
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
    }
    INFO_SSACF_INSCONST (arg_info) = FALSE;

    /* look up arguments */
    arg_expr = SSACFGetPrfArgs (arg_expr, PRF_ARGS (arg_node), PRF_MAX_ARGS);

    /* try some constant folding */
    new_node = SSACFFoldPrfExpr (PRF_PRF (arg_node), arg_expr);

    if (new_node != NULL) {
        /* free this primitive function and substitute it with new node */
        arg_node = FreeTree (arg_node);
        arg_node = new_node;

        /* increment constant folding counter */
        cf_expr++;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACFids(node *arg_ids, node *arg_info)
 *
 * description:
 *   traverse ids chain and return exprs chain (stored in INFO_SSACF_RESULT)
 *   and look for constant results.
 *   each constant identifier will be set in an separate assignment (added to
 *   INFO_SSACF_POSTASSIGN) and substituted in the function application with
 *   a new dummy identifier that can be removed by constant folding later.
 *
 *****************************************************************************/

static ids *
SSACFids (ids *arg_ids, node *arg_info)
{
    constant *new_co;
    node *assign_let;
    node *new_vardec;

    DBUG_ENTER ("SSACFids");

    DBUG_ASSERT ((INFO_SSACF_RESULTS (arg_info) != NULL),
                 "different ids and result chains");

    new_co = COAST2Constant (EXPRS_EXPR (INFO_SSACF_RESULTS (arg_info)));

    if (new_co != NULL) {
        DBUG_PRINT ("SSACF",
                    ("identifier %s marked as constant",
                     VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))));

        AVIS_SSACONST (IDS_AVIS (arg_ids)) = new_co;

        /* create one let assign for constant definition */
        assign_let = MakeAssignLet (StringCopy (VARDEC_OR_ARG_NAME (
                                      AVIS_VARDECORARG (IDS_AVIS (arg_ids)))),
                                    AVIS_VARDECORARG (IDS_AVIS (arg_ids)),
                                    COConstant2AST (new_co));

        /* append new copy assignment to then-part block */
        INFO_SSACF_POSTASSIGN (arg_info)
          = AppendAssign (INFO_SSACF_POSTASSIGN (arg_info), assign_let);

        /* store definition assignment */
        AVIS_SSAASSIGN (IDS_AVIS (arg_ids)) = assign_let;

        DBUG_PRINT ("SSACF",
                    ("create constant assignment for %s",
                     VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))));

        /* create new dummy identifier */
        new_vardec = SSANewVardec (AVIS_VARDECORARG (IDS_AVIS (arg_ids)));
        BLOCK_VARDEC (INFO_SSACF_TOPBLOCK (arg_info))
          = AppendVardec (BLOCK_VARDEC (INFO_SSACF_TOPBLOCK (arg_info)), new_vardec);

        AVIS_SSAASSIGN (VARDEC_AVIS (new_vardec)) = INFO_SSACF_ASSIGN (arg_info);

        /* rename this identifier */
        IDS_AVIS (arg_ids) = VARDEC_AVIS (new_vardec);
        IDS_VARDEC (arg_ids) = new_vardec;
#ifndef NO_ID_NAME
        /* for compatiblity only
         * there is no real need for name string in ids structure because
         * you can get it from vardec without redundancy.
         */
        IDS_NAME (arg_ids) = Free (IDS_NAME (arg_ids));
        IDS_NAME (arg_ids) = StringCopy (VARDEC_NAME (new_vardec));
#endif
    }

    if (IDS_NEXT (arg_ids) != NULL) {
        INFO_SSACF_RESULTS (arg_info) = EXPRS_NEXT (INFO_SSACF_RESULTS (arg_info));
        IDS_NEXT (arg_ids) = TravIDS (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   ids *TravIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *   similar implementation of trav mechanism as used for nodes
 *   here used for ids.
 *
 *****************************************************************************/

static ids *
TravIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = SSACFids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFNgen(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses parameter of generator to substitute constant arrays
 *   with their array representation to allow constant folding on known
 *   shape information.
 *
 *****************************************************************************/

node *
SSACFNgen (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFNgen");

    INFO_SSACF_INSCONST (arg_info) = SUBST_SCALAR_AND_ARRAY;
    DBUG_PRINT ("SSACF", ("substitute constant generator parameters"));

    if (NGEN_BOUND1 (arg_node) != NULL) {
        NGEN_BOUND1 (arg_node) = Trav (NGEN_BOUND1 (arg_node), arg_info);
    }
    if (NGEN_BOUND2 (arg_node) != NULL) {
        NGEN_BOUND2 (arg_node) = Trav (NGEN_BOUND2 (arg_node), arg_info);
    }
    if (NGEN_STEP (arg_node) != NULL) {
        NGEN_STEP (arg_node) = Trav (NGEN_STEP (arg_node), arg_info);
    }
    if (NGEN_WIDTH (arg_node) != NULL) {
        NGEN_WIDTH (arg_node) = Trav (NGEN_WIDTH (arg_node), arg_info);
    }
    INFO_SSACF_INSCONST (arg_info) = SUBST_NONE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACFFoldPrfExpr(prf op, node **arg_expr)
 *
 * description:
 *   try to compute the primitive function for the given args.
 *   args must be a (static) node* array with len PRF_MAX_ARGS.
 *
 * returns:
 *   a computed result node or NULL if no computing is possible.
 *
 *****************************************************************************/

node *
SSACFFoldPrfExpr (prf op, node **arg_expr)
{
    node *new_node;
    constant *new_co;
    constant *arg_co_mem[PRF_MAX_ARGS];
    constant **arg_co = &arg_co_mem[0];
    int i;

    DBUG_ENTER ("SSACFFoldPrfExpr");

    /* init local variables */
    new_node = NULL;
    new_co = NULL;

    /* fill static arrays with converted constants */
    arg_co = SSACFArgs2Const (arg_co, arg_expr, PRF_MAX_ARGS);

    /* do constant folding for selected primitive function */
    switch (op) {
        /* one-argument functions */
    case F_toi:
    case F_toi_A:
        if
            ONE_CONST_ARG (arg_co)
            {
                new_co = COToi (arg_co[0]);
            }
        break;

    case F_tof:
    case F_tof_A:
        if
            ONE_CONST_ARG (arg_co)
            {
                new_co = COTof (arg_co[0]);
            }
        break;

    case F_tod:
    case F_tod_A:
        if
            ONE_CONST_ARG (arg_co)
            {
                new_co = COTod (arg_co[0]);
            }
        break;

    case F_abs:
        if
            ONE_CONST_ARG (arg_co)
            {
                new_co = COAbs (arg_co[0]);
            }
        break;

    case F_not:
        if
            ONE_CONST_ARG (arg_co)
            {
                new_co = CONot (arg_co[0]);
            }
        break;

    case F_dim:
        if
            ONE_CONST_ARG (arg_co)
            {
                /* for pure constant arg */
                new_co = CODim (arg_co[0]);
            }
        else if
            ONE_ARG (arg_expr)
            {
                /* for some non full constant expression */
                new_co = SSACFDim (arg_expr[0]);
            }
        break;

    case F_shape:
        if
            ONE_CONST_ARG (arg_co)
            {
                /* for pure constant arg */
                new_co = COShape (arg_co[0]);
            }
        else if
            ONE_ARG (arg_expr)
            {
                /* for some non full constant expression */
                new_co = SSACFShape (arg_expr[0]);
            }
        break;

        /* two-argument functions */
    case F_min:
        if
            TWO_CONST_ARG (arg_co)
            {
                new_co = COMin (arg_co[0], arg_co[1]);
            }
        break;

    case F_max:
        if
            TWO_CONST_ARG (arg_co)
            {
                new_co = COMax (arg_co[0], arg_co[1]);
            }
        break;

    case F_add:
    case F_add_AxS:
    case F_add_SxA:
    case F_add_AxA:
        if
            TWO_CONST_ARG (arg_co)
            {
                new_co = COAdd (arg_co[0], arg_co[1]);
            }
        if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = SSACFArithmOpWrapper (F_add, arg_co, arg_expr);
            }
        break;

    case F_sub:
    case F_sub_AxS:
    case F_sub_SxA:
    case F_sub_AxA:
        if
            TWO_CONST_ARG (arg_co)
            {
                new_co = COSub (arg_co[0], arg_co[1]);
            }
        else if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = SSACFArithmOpWrapper (F_sub, arg_co, arg_expr);
            }
        else if
            TWO_ARG (arg_expr)
            {
                new_node = SSACFSub (arg_expr[0], arg_expr[1]);
            }
        break;

    case F_mul:
    case F_mul_AxS:
    case F_mul_SxA:
    case F_mul_AxA:
        if
            TWO_CONST_ARG (arg_co)
            {
                new_co = COMul (arg_co[0], arg_co[1]);
            }
        if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = SSACFArithmOpWrapper (F_mul, arg_co, arg_expr);
            }
        break;

    case F_div:
    case F_div_SxA:
    case F_div_AxS:
    case F_div_AxA:
        if
            TWO_CONST_ARG (arg_co)
            {
                if (COIsZero (arg_co[1], FALSE)) { /* any 0 in divisor, x/0 -> err */
                    SYSABORT (("Division by zero expected"));
                } else {
                    new_co = CODiv (arg_co[0], arg_co[1]);
                }
            }
        if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = SSACFArithmOpWrapper (F_div, arg_co, arg_expr);
            }
        break;

    case F_mod:
        if
            TWO_CONST_ARG (arg_co)
            {
                new_co = COMod (arg_co[0], arg_co[1]);
            }
        break;

    case F_and:
        if
            TWO_CONST_ARG (arg_co)
            {
                new_co = COAnd (arg_co[0], arg_co[1]);
            }
        if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = SSACFArithmOpWrapper (F_and, arg_co, arg_expr);
            }
        break;

    case F_or:
        if
            TWO_CONST_ARG (arg_co)
            {
                new_co = COOr (arg_co[0], arg_co[1]);
            }
        if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = SSACFArithmOpWrapper (F_or, arg_co, arg_expr);
            }
        break;

    case F_le:
        if
            TWO_CONST_ARG (arg_co)
            {
                new_co = COLe (arg_co[0], arg_co[1]);
            }
        break;

    case F_lt:
        if
            TWO_CONST_ARG (arg_co)
            {
                new_co = COLt (arg_co[0], arg_co[1]);
            }
        break;

    case F_eq:
        if
            TWO_CONST_ARG (arg_co)
            {
                /* for pure constant arg */
                new_co = COEq (arg_co[0], arg_co[1]);
            }
        else if
            TWO_ARG (arg_expr)
            {
                /* for two expressions (does a treecmp) */
                new_node = SSACFEq (arg_expr[0], arg_expr[1]);
            }
        break;

    case F_ge:
        if
            TWO_CONST_ARG (arg_co)
            {
                new_co = COGe (arg_co[0], arg_co[1]);
            }
        break;

    case F_gt:
        if
            TWO_CONST_ARG (arg_co)
            {
                new_co = COGt (arg_co[0], arg_co[1]);
            }
        break;

    case F_neq:
        if
            TWO_CONST_ARG (arg_co)
            {
                new_co = CONeq (arg_co[0], arg_co[1]);
            }
        break;

    case F_reshape:
        if
            TWO_CONST_ARG (arg_co)
            {
                /* for pure constant arg */
                new_co = COReshape (arg_co[0], arg_co[1]);
            }
        else if ((arg_co[0] != NULL) && (arg_expr[1] != NULL)) {
            /* for some non constant expression and constant index vector */
            new_node = SSACFStructOpWrapper (F_reshape, arg_co[0], arg_expr[1]);
        }
        break;

    case F_sel:
        if
            TWO_CONST_ARG (arg_co)
            {
                /* for pure constant args */
                new_co = COSel (arg_co[0], arg_co[1]);
            }
        else if
            FIRST_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                /* for some non constant expression and constant index vector */
                new_node = SSACFStructOpWrapper (F_sel, arg_co[0], arg_expr[1]);
            }

        if ((new_co == NULL) && (new_node == NULL) && (TWO_ARG (arg_expr))) {
            /* for some expressions concerning sel-modarray combinations */
            new_node = SSACFSel (arg_expr[0], arg_expr[1]);
        }
        break;

    case F_take:
        if
            TWO_CONST_ARG (arg_co)
            {
                /* for pure constant args */
                new_co = COTake (arg_co[0], arg_co[1]);
            }
        else if
            FIRST_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                /* for some non constant expression and constant index vector */
                new_node = SSACFStructOpWrapper (F_take, arg_co[0], arg_expr[1]);
            }
        break;

    case F_drop:
        if
            TWO_CONST_ARG (arg_co)
            {
                /* for pure constant args */
                new_co = CODrop (arg_co[0], arg_co[1]);
            }
        else if
            FIRST_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                /* for some non constant expression and constant index vector */
                new_node = SSACFStructOpWrapper (F_drop, arg_co[0], arg_expr[1]);
            }
        break;

        /* three-argument functions */
    case F_modarray:
        if
            THREE_CONST_ARG (arg_co)
            {
                /* for pure constant args */
                new_co = COModarray (arg_co[0], arg_co[1], arg_co[2]);
            }
        else if (SECOND_CONST_ARG_OF_THREE (arg_co, arg_expr)) {
            new_node = SSACFModarray (arg_expr[0], arg_co[1], arg_expr[2]);
        }
        break;

    case F_cat:
        /* not implemeted yet */
        break;

    case F_rotate:
        /* not implemented yet */
        break;

    default:
        DBUG_PRINT ("SSACF", ("no implementation in SSAConstantFolding for  prf %s",
                              prf_string[op]));
    }

    /* free used constant data */
    for (i = 0; i < PRF_MAX_ARGS; i++) {
        if (arg_co[i] != NULL) {
            arg_co[i] = COFreeConstant (arg_co[i]);
        }
    }

    /*
     * if we got a new computed expression instead of the primitive function
     * we create a new expression with the result
     */
    if ((new_co != NULL) || (new_node != NULL)) {
        if (new_co != NULL) {
            /* create new node with constant value instead of prf node */
            new_node = COConstant2AST (new_co);
            new_co = COFreeConstant (new_co);
        } else {
            /* some constant expression of non full constant args have been computed */
        }
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSAConstantFolding(node* fundef, node* modul)
 *
 * description:
 *   starts the DeadCodeRemoval for the given fundef. This fundef must not be
 *   a special fundef (these will be traversed in their order of application).
 *
 ******************************************************************************/

node *
SSAConstantFolding (node *fundef, node *modul)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSAConstantFolding");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "SSAConstantFolding called for non-fundef node");

    DBUG_PRINT ("OPT",
                ("starting constant folding (ssa) in function %s", FUNDEF_NAME (fundef)));

    /* do not start traversal in special functions */
    if (!(FUNDEF_IS_LACFUN (fundef))) {
        arg_info = MakeInfo ();

        INFO_SSACF_MODUL (arg_info) = modul;

        old_tab = act_tab;
        act_tab = ssacf_tab;

        fundef = Trav (fundef, arg_info);

        act_tab = old_tab;

        arg_info = FreeTree (arg_info);
    }

    DBUG_RETURN (fundef);
}
