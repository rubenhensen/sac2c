/*
 *
 * $Log$
 * Revision 1.9  2001/05/02 08:02:11  nmw
 * arithmetical constant folding implemented
 *
 * Revision 1.8  2001/04/30 12:04:57  nmw
 * structual operations implemented
 *
 * Revision 1.7  2001/04/26 13:30:44  nmw
 * inline flag INFO_SSACF_INLFUNDEF used instead of ST_inlinefun
 *
 * Revision 1.6  2001/04/18 12:55:42  nmw
 * debug output for OPT traversal added
 *
 * Revision 1.5  2001/04/18 10:06:17  dkr
 * signature of InlineSingleApplication() modified
 *
 * Revision 1.4  2001/04/02 11:08:20  nmw
 * handling for multiple used special functions added
 *
 * Revision 1.3  2001/03/29 16:31:21  nmw
 * Constant Folding for Loops and Conditionals implemented
 *
 * Revision 1.2  2001/03/22 14:30:18  nmw
 * constant folding for primitive ari ops implemented
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
 *   this module does constant folding on code in ssa form.
 *
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

/*
 * constant identifiers should be substituted by its constant value
 * also in the arg-chain of N_ap and N_prf nodes.
 */
#define SUBST_ID_WITH_CONSTANT_IN_AP_ARGS TRUE

#define SUBST_NONE 0
#define SUBST_SCALAR 1
#define SUBST_SCALAR_AND_ARRAY 2

/* Prf-function name for debug output */
#ifndef DBUG_OFF
#define PRF_IF(n, s, x, y) x
static char *prf_string[] = {
#include "prf_node_info.mac"
};
#undef PRF_IF
#endif

#define ONE_CONST_ARG(arg1, arg2)                                                        \
    (((arg1 == NULL) && (arg2 != NULL)) || ((arg1 != NULL) && (arg2 == NULL)))

/* special constant version used for structural constants */
typedef struct {
    simpletype simpletype;
    char *name;     /* only used for T_user !! */
    char *name_mod; /* name of modul belonging to 'name' */
    constant *hidden_co;
} struc_constant;

/* access macros for structural constant type */
#define SCO_BASETYPE(n) (n->simpletype)
#define SCO_NAME(n) (n->name)
#define SCO_MOD(n) (n->name_mod)
#define SCO_HIDDENCO(n) (n->hidden_co)

/* local used helper functions */
static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *SSACFids (ids *arg_ids, node *arg_info);
static node *SSACFPropagateConstants2Args (node *arg_chain, node *param_chain);
static struc_constant *SSACFExpr2StructConstant (node *expr);
static node *SSACFDupStructConstant2Expr (struc_constant *struc_co);
static struc_constant *SCOFreeStructConstant (struc_constant *struc_co);
static shape *SSACFGetShapeOfExpr (node *expr);

/*
 * primitive functions for non full-constant expressions like:
 *   dimension constant
 *   shape constant
 *   structural constant
 *
 * they have to be implemented seperatly as long as there is no constant type
 * that can handle all these cases internally
 *
 */

static constant *SSACFDim (node *expr);
static constant *SSACFShape (node *expr);

/* implements: psi, reshape, take, drop */
static node *SSACFStructOpWrapper (prf op, constant *idx, node *expr);

/* implements arithmetical for add, sub, mul, div, and, or */
static node *SSACFArithmOpWrapper (prf op, constant *arg1, node *arg1_expr,
                                   constant *arg2, node *arg2_expr);

/* missing:
static node *SSACFModarray ( node *a, constant *idx, node *elem);
static node *SSACFCat      ( constant *dim, node *a, node *b);
static node *SSACFRotate   ( constant *dim, constant *num, node *a);
*/

/* functions for internal use only */
/******************************************************************************
 *
 * function:
 *   node *SSACFPropagateConstants2Args(node *arg_chain, node *const_arg_chain)
 *
 *
 * description:
 *   to propagate constant expressions from the calling context into a special
 *   function, this functions does a parallel traversal of the function args
 *   (stored in arg_chain) and the calling parameters (stored param_chain).
 *
 *
 *
 ******************************************************************************/
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

        /* traverse both chains */
        arg = ARG_NEXT (arg);
        param_chain = EXPRS_NEXT (param_chain);
    }

    DBUG_RETURN (arg_chain);
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
 ******************************************************************************/
static constant *
SSACFDim (node *expr)
{
    constant *result;
    int dim;
    DBUG_ENTER ("SSACFDim");

    if (NODE_TYPE (expr) == N_id) {
        dim = GetDim (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (expr))));
        if (dim != UNKNOWN_SHAPE) {
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
 *
 ******************************************************************************/
static constant *
SSACFShape (node *expr)
{
    constant *result;
    int dim;
    shape *cshape;
    int *int_vec;

    DBUG_ENTER ("SSACFDim");

    if (NODE_TYPE (expr) == N_id) {
        dim = GetDim (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (expr))));
        if (dim != UNKNOWN_SHAPE) {
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
 *   node *SSACFStructOpWrapper(prf op, constant *idx, node *expr)
 *
 * description:
 *   computes structural ops on array expressions with constant index vector.
 *
 ******************************************************************************/
static node *
SSACFStructOpWrapper (prf op, constant *idx, node *expr)
{
    struc_constant *struc_co;
    node *result;

    DBUG_ENTER ("SSACFStructOpWrapper");

    struc_co = SSACFExpr2StructConstant (expr);

    /* given expressession could be converted to struc_constant */
    if (struc_co != NULL) {
        /* perform struc-op on hidden constant */
        switch (op) {
        case F_psi:
            SCO_HIDDENCO (struc_co) = COPsi (idx, SCO_HIDDENCO (struc_co));
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

        DBUG_PRINT ("SSACF",
                    ("SSACFStructOpWrapper: op computed on structural constant"));
        struc_co = SCOFreeStructConstant (struc_co);
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
 *   builds an constant of type T_hidden from an array in the AST.
 *   this allows to operate on structural constants like full constants.
 *
 *   this should later be integrated in the constants module.
 *
 ******************************************************************************/
static struc_constant *
SSACFExpr2StructConstant (node *expr)
{
    struc_constant *struc_co;
    node *array;
    types *atype;
    shape *ashape;
    node **node_vec;
    int elem_count;
    int i;
    node *tmp;

    DBUG_ENTER ("SSACFExpr2StructConstant");

    if (NODE_TYPE (expr) == N_array) {
        /* is this expression an array */
        array = expr;

        /* shape of the given array */
        DBUG_ASSERT ((ARRAY_TYPE (array) != NULL), "unknown array type");
        atype = ARRAY_TYPE (array);

    } else if (NODE_TYPE (expr) == N_id) {
        /* is id a defined array */
        if ((AVIS_SSAASSIGN (ID_AVIS (expr)) != NULL)
            && (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (expr)))))
                == N_array)) {
            array = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (expr))));

            /* shape of the given array */
            atype = VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (expr)));

        } else {
            array = NULL;
        }
    } else {
        array = NULL;
    }

    /* build an abstract structural constant of type (void*) T_hidden */
    if (array != NULL) {
        /* alloc hidden vector */
        ashape = SHOldTypes2Shape (atype);
        elem_count = SHGetUnrLen (ashape);
        node_vec = (node **)MALLOC (elem_count * sizeof (node *));

        /* copy element pointers from array to vector */
        tmp = ARRAY_AELEMS (array);
        for (i = 0; i < elem_count; i++) {
            DBUG_ASSERT ((tmp != NULL), "array contains to few elements");
            node_vec[i] = EXPRS_EXPR (tmp);
            tmp = EXPRS_NEXT (tmp);
        }
        DBUG_ASSERT ((tmp == NULL), "array contains to much elements");

        /* create struc_constant */
        struc_co = (struc_constant *)MALLOC (sizeof (struc_constant));
        SCO_BASETYPE (struc_co) = TYPES_BASETYPE (atype);
        SCO_NAME (struc_co) = TYPES_NAME (atype);
        SCO_MOD (struc_co) = TYPES_MOD (atype);
        SCO_HIDDENCO (struc_co) = COMakeConstant (T_hidden, ashape, node_vec);

    } else {
        /* no array with known elements */
        struc_co = NULL;
    }

    DBUG_RETURN (struc_co);
}

/******************************************************************************
 *
 * function:
 *   static node *SSACFDupStructConstant2Expr(struc_constant *struc_co)
 *
 * description:
 *   builds an array of the given strucural constant and duplicate
 *   elements in it. therfore the original array must not be freed before
 *   the target array is build up from the elements of the original.
 *
 ******************************************************************************/
static node *
SSACFDupStructConstant2Expr (struc_constant *struc_co)
{
    node *expr;
    node *aelems;
    int i;
    int elems_count;
    node **node_vec;

    DBUG_ENTER ("SSACFDupStructConstant2Expr");

    /* build up elems chain */
    node_vec = (node **)COGetDataVec (SCO_HIDDENCO (struc_co));

    if (COGetDim (SCO_HIDDENCO (struc_co)) == 0) {
        /* result is a scalar */
        expr = DupNode (node_vec[0]);
    } else {
        /* result is a new array */
        elems_count = SHGetUnrLen (COGetShape (SCO_HIDDENCO (struc_co)));

        aelems = NULL;
        for (i = elems_count - 1; i > 0; i--) {
            aelems = MakeExprs (DupNode (node_vec[i]), aelems);
        }

        /* build array node */
        expr = MakeArray (aelems);
        ARRAY_TYPE (expr)
          = MakeTypes (SCO_BASETYPE (struc_co), COGetDim (SCO_HIDDENCO (struc_co)),
                       SHShape2OldShpseg (COGetShape (SCO_HIDDENCO (struc_co))),
                       StringCopy (SCO_NAME (struc_co)),
                       StringCopy (SCO_NAME (struc_co)));
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
 ******************************************************************************/
static struc_constant *
SCOFreeStructConstant (struc_constant *struc_co)
{
    DBUG_ENTER ("SCOFreeStructConstant");

    DBUG_ASSERT ((struc_co != NULL), "SCOFreeStructConstant: NULL pointer");

    /* free substructure */
    SCO_HIDDENCO (struc_co) = COFreeConstant (SCO_HIDDENCO (struc_co));

    /* free structure */
    FREE (struc_co);

    DBUG_RETURN ((struc_constant *)NULL);
}

/******************************************************************************
 *
 * function:
 *   node *SSACFArithmOpWrapper(prf op,
 *                              constant *arg1,
 *                              node *arg1_expr,
 *                              constant *arg2,
 *                              node *arg2_expr)
 *
 * description:
 * implements arithmetical operations for add, sub, mul, div, and, or on one
 * constant arg and one other expression.
 *
 ******************************************************************************/
static node *
SSACFArithmOpWrapper (prf op, constant *arg1, node *arg1_expr, constant *arg2,
                      node *arg2_expr)
{
    node *result;
    node *expr;
    constant *co;
    bool swap;
    constant *tmp_co;
    shape *target_shp;

    DBUG_ENTER ("SSACFArithmOpWrapper");

    /* get constant and expression */
    if (arg1 != NULL) {
        swap = FALSE;
        co = arg1;
        expr = arg2_expr;
    } else {
        swap = TRUE;
        co = arg2;
        expr = arg1_expr;
    }
    DBUG_ASSERT ((co != NULL), "no constant arg found");

    result = NULL;
    tmp_co = NULL;

    switch (op) {
    case F_add:
        if (COIsZero (co)) { /* x+0 -> x  or 0+x -> x */
            result = DupTree (expr);
        }
        break;

    case F_sub:
        if (swap && COIsZero (co)) { /* x-0 -> x */
            result = DupTree (expr);
        }
        break;

    case F_mul:
        if (COIsZero (co)) { /* x*0 -> 0 or 0*x -> 0 */
            target_shp = SSACFGetShapeOfExpr (expr);
            if (target_shp != NULL) {
                /* Create ZeroConstant of same type and shape as expression */
                tmp_co = COMakeZero (COGetType (co), target_shp);
            }
        } else if (COIsOne (co)) { /* x*1 -> x or 1*x -> x */
            result = DupTree (expr);
        }
        break;

    case F_div:
        if (COIsZero (co)) {
            if (swap) { /* x/0 -> err */
                WARN (expr->lineno, ("Division by zero expected"));
            } else { /* 0/x -> 0 */
                target_shp = SSACFGetShapeOfExpr (expr);
                if (target_shp != NULL) {
                    /* Create ZeroConstant of same type and shape as expression */
                    tmp_co = COMakeZero (COGetType (co), target_shp);
                }
            }
        } else if (swap && COIsOne (co)) { /* x/1 -> x */
            result = DupTree (expr);
        }
        break;

    case F_and:
        if (COIsTrue (co)) { /* x&&true -> x or true&&x -> x */
            result = DupTree (expr);
        } else if (COIsFalse (co)) { /* x&&false->false or false&&x->false */
            target_shp = SSACFGetShapeOfExpr (expr);
            if (target_shp != NULL) {
                /* Create False constant of same shape as expression */
                tmp_co = COMakeFalse (target_shp);
            }
        }
        break;

    case F_or:
        if (COIsFalse (co)) { /* x||false->x or false||x -> x */
            result = DupTree (expr);
        } else if (COIsFalse (co)) { /* x||true->true or true&&x->true */
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
        DBUG_PRINT ("SSACF", ("arithmetic constant folding done."));
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
 ******************************************************************************/
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

/* traversal functions */
/******************************************************************************
 *
 * function:
 *   node* SSACFfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses args and block in this order.
 *   the args are only traversed in do/while special functions to remove
 *   propagated constants from loop dependend arguments.
 *
 ******************************************************************************/
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
 *   traverses vardecs and instructions in this order.
 *
 *
 ******************************************************************************/
node *
SSACFblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFblock");

    /*
    if (BLOCK_VARDEC(arg_node) != NULL) {
      BLOCK_VARDEC(arg_node) = Trav(BLOCK_VARDEC(arg_node), arg_info);
    }
    */

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFarg(node *arg_node, node *arg_info)
 *
 * description:
 *   checks if only loop invariant arguments are constant (if special do-loop
 *   or while-loop fundef).
 *
 *
 ******************************************************************************/
node *
SSACFarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFarg");

    /* constants for non loop invarinat args are useless */
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
 *   node* SSACFvardec(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFvardec");

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
 *   from constant conditionals)
 *
 *
 ******************************************************************************/
node *
SSACFassign (node *arg_node, node *arg_info)
{
    bool remove_assignment;
    node *tmp;

    DBUG_ENTER ("SSACFassign");

    /* init flags for possible code removal/movement */
    INFO_SSACF_REMASSIGN (arg_info) = FALSE;
    INFO_SSACF_POSTASSIGN (arg_info) = NULL;
    INFO_SSACF_ASSIGN (arg_info) = arg_node;
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }
    INFO_SSACF_ASSIGN (arg_info) = NULL;

    /* save removal flag for bottom-up traversal */
    remove_assignment = INFO_SSACF_REMASSIGN (arg_info);

    /* integrate post assignments after current assignment */
    ASSIGN_NEXT (arg_node)
      = AppendAssign (INFO_SSACF_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
    INFO_SSACF_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (remove_assignment) {
        /* skip this assignment and free it */
        DBUG_PRINT ("SSACF", ("remove dead assignment"));
        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);

        ASSIGN_NEXT (tmp) = NULL;
        FreeNode (tmp);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFcond(node *arg_node, node *arg_info)
 *
 * description:
 *   checks for constant conditional - removes corresponding of the
 *   conditional.
 *   traverses conditional and optional then-part, else-part
 *
 ******************************************************************************/
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
                        ("found condition with condition == true, select then part"));
            /* select then part */
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
                        ("found condition with condition == false, select else part"));
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
            && ((FUNDEF_STATUS (INFO_SSACF_FUNDEF (arg_info)) == ST_dofun)
                || (FUNDEF_STATUS (INFO_SSACF_FUNDEF (arg_info)) == ST_whilefun))) {
            WARN (NODE_LINE (arg_node),
                  ("infinite loop detected, program may not terminate"));
            /* ex special function cannot be inlined */
            FUNDEF_STATUS (INFO_SSACF_FUNDEF (arg_info)) = ST_regular;
        } else {
            /* ex special function can be simply inlined */
            INFO_SSACF_INLFUNDEF (arg_info) = TRUE;
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
 *   do NOT substitute identifiers in return statement with their values!
 *
 *
 ******************************************************************************/
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
 ******************************************************************************/
node *
SSACFlet (node *arg_node, node *arg_info)
{
    constant *new_co;
    DBUG_ENTER ("SSACFlet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "let without expression");

    /* remove tag SSAPHITARGET if conditional has been removed */
    if ((FUNDEF_STATUS (INFO_SSACF_FUNDEF (arg_info)) == ST_regular)
        || (FUNDEF_STATUS (INFO_SSACF_FUNDEF (arg_info)) == ST_inlinefun)) {
        AVIS_SSAPHITARGET (IDS_AVIS (LET_IDS (arg_node))) = PHIT_NONE;
    }

    /*
     * left side is not marked as constant -> compute expression
     * if there is a special function application with mutiple results
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
             * traverse ids chain and result chain of the concering return
             * statement. for each constant identifier add a separate
             * assignent and substitute the result identifier in the
             * function application with a dummy identifier (that will be
             * removed by the dead code removal)
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

                /* remove this assignment */
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
             * maybe check for two equal constants?
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
 ******************************************************************************/
node *
SSACFap (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("SSACFap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    /* substitute scalar constants in arguments (if no special function)*/
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

        /* propagate constant args to called special function */
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
        FREE (new_arg_info);

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
 *   ( only when INFO_SSACF_INSCONST flag is set)
 *   in EXPRS chain of N_ap ARGS ( if SUBST_ID_WITH_CONSTANT_IN_AP_ARGS == TRUE)
 *      EXPRS chain of N_prf ARGS (if SUBST_ID_WITH_CONSTANT_IN_AP_ARGS == TRUE)
 *      EXPRS chain of N_array AELEMS
 *
 ******************************************************************************/
node *
SSACFid (node *arg_node, node *arg_info)
{
    node *new_node;
    DBUG_ENTER ("SSACFid");

    /* check for constant scalar identifier */
    if ((((TYPES_DIM (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (arg_node))))
           == SCALAR)
          && (INFO_SSACF_INSCONST (arg_info) >= SUBST_SCALAR))
         || ((TYPES_DIM (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (ID_AVIS (arg_node))))
              > SCALAR)
             && (INFO_SSACF_INSCONST (arg_info)) == SUBST_SCALAR_AND_ARRAY))
        && (AVIS_SSACONST (ID_AVIS (arg_node)) != NULL)) {
        DBUG_PRINT ("SSACF",
                    ("substitue identifier %s through its value",
                     VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (arg_node)))));

        /* substitute identifier with its value */
        new_node = COConstant2AST (AVIS_SSACONST (ID_AVIS (arg_node)));
        arg_node = FreeTree (arg_node);
        arg_node = new_node;
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
 *   the function application with its value.
 *
 *
 ******************************************************************************/
node *
SSACFprf (node *arg_node, node *arg_info)
{
    node *new_node;
    constant *new_co;
    constant *arg1;
    constant *arg2;
    node *arg1_expr;
    node *arg2_expr;

    DBUG_ENTER ("SSACFprf");

    DBUG_PRINT ("SSACF", ("evaluating prf %s", prf_string[PRF_PRF (arg_node)]));

    /* substitute constant identifiers in arguments */
    INFO_SSACF_INSCONST (arg_info) = SUBST_ID_WITH_CONSTANT_IN_AP_ARGS;
    if (PRF_ARGS (arg_node) != NULL) {
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
    }
    INFO_SSACF_INSCONST (arg_info) = FALSE;

    /* init local variables */
    new_node = NULL;
    new_co = NULL;

    /* try to convert args to constants */
    if (EXPRS_EXPR (PRF_ARGS (arg_node)) != NULL) {
        arg1 = COAST2Constant (EXPRS_EXPR (PRF_ARGS (arg_node)));
        arg1_expr = EXPRS_EXPR (PRF_ARGS (arg_node));
    } else {
        arg1 = NULL;
        arg1_expr = NULL;
    }

    if ((EXPRS_NEXT (PRF_ARGS (arg_node)) != NULL)
        && (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))) != NULL)) {
        arg2 = COAST2Constant (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))));
        arg2_expr = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node)));
    } else {
        arg2 = NULL;
        arg2_expr = NULL;
    }

    /* do constant folding on primitive functions */
    switch (PRF_PRF (arg_node)) {
        /* single-argument functions */
    case F_toi:
    case F_toi_A:
        if (arg1 != NULL) {
            new_co = COToi (arg1);
        }
        break;

    case F_tof:
    case F_tof_A:
        if (arg1 != NULL) {
            new_co = COTof (arg1);
        }
        break;

    case F_tod:
    case F_tod_A:
        if (arg1 != NULL) {
            new_co = COTod (arg1);
        }
        break;

    case F_abs:
        if (arg1 != NULL) {
            new_co = COAbs (arg1);
        }
        break;

    case F_not:
        if (arg1 != NULL) {
            new_co = CONot (arg1);
        }
        break;

        /* two-argument functions */
    case F_min:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COMin (arg1, arg2);
        }
        break;

    case F_max:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COMax (arg1, arg2);
        }
        break;

    case F_add:
    case F_add_AxS:
    case F_add_SxA:
    case F_add_AxA:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COAdd (arg1, arg2);
        }
        if (ONE_CONST_ARG (arg1, arg2)) {
            new_node = SSACFArithmOpWrapper (F_add, arg1, arg1_expr, arg2, arg2_expr);
        }
        break;

    case F_sub:
    case F_sub_AxS:
    case F_sub_SxA:
    case F_sub_AxA:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COSub (arg1, arg2);
        }
        if (ONE_CONST_ARG (arg1, arg2)) {
            new_node = SSACFArithmOpWrapper (F_sub, arg1, arg1_expr, arg2, arg2_expr);
        }
        break;

    case F_mul:
    case F_mul_AxS:
    case F_mul_SxA:
    case F_mul_AxA:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COMul (arg1, arg2);
        }
        if (ONE_CONST_ARG (arg1, arg2)) {
            new_node = SSACFArithmOpWrapper (F_mul, arg1, arg1_expr, arg2, arg2_expr);
        }
        break;

    case F_div:
    case F_div_SxA:
    case F_div_AxS:
    case F_div_AxA:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = CODiv (arg1, arg2);
        }
        if (ONE_CONST_ARG (arg1, arg2)) {
            new_node = SSACFArithmOpWrapper (F_div, arg1, arg1_expr, arg2, arg2_expr);
        }
        break;

    case F_mod:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COMod (arg1, arg2);
        }
        break;

    case F_and:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COAnd (arg1, arg2);
        }
        if (ONE_CONST_ARG (arg1, arg2)) {
            new_node = SSACFArithmOpWrapper (F_and, arg1, arg1_expr, arg2, arg2_expr);
        }
        break;

    case F_or:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COOr (arg1, arg2);
        }
        if (ONE_CONST_ARG (arg1, arg2)) {
            new_node = SSACFArithmOpWrapper (F_or, arg1, arg1_expr, arg2, arg2_expr);
        }
        break;

    case F_le:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COLe (arg1, arg2);
        }
        break;

    case F_lt:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COLt (arg1, arg2);
        }
        break;

    case F_eq:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COEq (arg1, arg2);
        }
        break;

    case F_ge:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COGe (arg1, arg2);
        }
        break;

    case F_gt:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COGt (arg1, arg2);
        }
        break;

    case F_neq:
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = CONeq (arg1, arg2);
        }
        break;

    case F_dim:
        if (arg1 != NULL) {
            /* for pure constant arg */
            new_co = CODim (arg1);
        } else if (arg1_expr != NULL) {
            /* for some non full constant expression args */
            new_co = SSACFDim (arg1_expr);
        }
        break;

    case F_shape:
        /* for pure constant arg */
        if (arg1 != NULL) {
            new_co = COShape (arg1);
        } else if (arg1_expr != NULL) {
            /* for some non full constant expression args */
            new_co = SSACFShape (arg1_expr);
        }
        break;

    case F_reshape:
        /* for pure constant arg */
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COReshape (arg1, arg2);
        } else if ((arg1 != NULL) && (arg2_expr != NULL)) {
            /* for some non constant expression and constant index vector */
            new_node = SSACFStructOpWrapper (F_reshape, arg1, arg2_expr);
        }
        break;

    case F_idx_psi:
        /* not implemented yet */
        break;

    case F_psi:
        /* for pure constant args */
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COPsi (arg1, arg2);
        } else if ((arg1 != NULL) && (arg2_expr != NULL)) {
            /* for some non constant expression and constant index vector */
            new_node = SSACFStructOpWrapper (F_psi, arg1, arg2_expr);
        }
        break;

    case F_idx_modarray:
        /* not implemeted yet */
        break;

    case F_modarray:
        /* not implemented yet */
        break;

    case F_take:
        /* for pure constant args */
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = COTake (arg1, arg2);
        } else if ((arg1 != NULL) && (arg2_expr != NULL)) {
            /* for some non constant expression and constant index vector */
            new_node = SSACFStructOpWrapper (F_take, arg1, arg2_expr);
        }
        break;

    case F_drop:
        /* for pure constant args */
        if ((arg1 != NULL) && (arg2 != NULL)) {
            new_co = CODrop (arg1, arg2);
        } else if ((arg1 != NULL) && (arg2_expr != NULL)) {
            /* for some non constant expression and constant index vector */
            new_node = SSACFStructOpWrapper (F_drop, arg1, arg2_expr);
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
                              prf_string[PRF_PRF (arg_node)]));
    }

    /* free used constant data */
    if (arg1 != NULL) {
        arg1 = COFreeConstant (arg1);
    }

    if (arg2 != NULL) {
        arg2 = COFreeConstant (arg2);
    }

    /*
     * if we got a new computed expression instead of the primitive function
     * we substitute the fun_ap with the new expression
     */
    if ((new_co != NULL) || (new_node != NULL)) {
        if (new_co != NULL) {
            /* create new node with constant value instead of prf node */
            new_node = COConstant2AST (new_co);
            new_co = COFreeConstant (new_co);
        } else {
            /* some constant expression of non full constant args have been computed */
        }

        /* free this primitive function */
        FreeTree (arg_node);
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
 ******************************************************************************/
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

        /* rename this identifier */
        IDS_AVIS (arg_ids) = VARDEC_AVIS (new_vardec);
        IDS_VARDEC (arg_ids) = new_vardec;
#ifndef NO_ID_NAME
        /* for compatiblity only
         * there is no real need for name string in ids structure because
         * you can get it from vardec without redundancy.
         */
        FREE (IDS_NAME (arg_ids));
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
 ******************************************************************************/
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
 ******************************************************************************/
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
 *   node* SSAConstantFolding(node* fundef, node* modul)
 *
 * description:
 *   starts the DeadCodeRemoval for the given fundef. This fundef must not be
 *   a special fundef (these will be traversed in their order of application).
 *
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
    if ((FUNDEF_STATUS (fundef) != ST_condfun) && (FUNDEF_STATUS (fundef) != ST_dofun)
        && (FUNDEF_STATUS (fundef) != ST_whilefun)) {
        arg_info = MakeInfo ();

        INFO_SSACF_MODUL (arg_info) = modul;

        old_tab = act_tab;
        act_tab = ssacf_tab;

        fundef = Trav (fundef, arg_info);

        act_tab = old_tab;

        FREE (arg_info);
    }

    DBUG_RETURN (fundef);
}
