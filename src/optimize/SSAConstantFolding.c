/*
 *
 * $Log$
 * Revision 1.85  2005/04/20 19:15:29  ktr
 * removed CFarg, CFlet. Codebrushing.
 *
 * Revision 1.84  2005/03/04 21:21:42  cg
 * Useless conditional eliminated.
 * Integration of silently duplicated LaC funs at the end of the
 * fundef chain added.
 *
 * Revision 1.83  2005/02/14 15:51:48  mwe
 * CFids moved to cvp
 *
 * Revision 1.82  2005/02/01 17:40:26  mwe
 * evaluations of primitive operations done by typechecker via typeupgrade removed from
 * CFfoldPrfExpr
 *
 * Revision 1.81  2005/01/31 15:28:04  mwe
 * reimplement folding of conditionals
 *
 * Revision 1.80  2005/01/26 10:25:40  mwe
 * AVIS_SSACONST removed and replaced by usage of akv types
 * traversals changed: now constant values are infered when type is akv
 *
 * Revision 1.79  2005/01/11 12:58:15  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 1.78  2004/12/08 18:00:42  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 1.77  2004/11/27 02:55:25  ktr
 * YO!
 *
 * Revision 1.76  2004/11/26 19:46:41  khf
 * SacDevCamp04: COMPILES!!!
 *
 * Revision 1.75  2004/11/16 14:39:17  mwe
 * ntype-support for ID_TYPE added
 *
 * Revision 1.74  2004/11/10 18:27:29  mwe
 * code for type upgrade added
 * use ntype-structure instead of type-structure
 * new code deactivated by MWE_NTYPE_READY
 *
 * Revision 1.73  2004/10/15 11:39:04  ktr
 * Reactived constant propagation.
 *
 * Revision 1.72  2004/10/07 12:38:00  ktr
 * Replaced the old With-Loop Scalarization with a new implementation.
 *
 * Revision 1.71  2004/09/27 08:37:57  ktr
 * yet another silly bug fixed.
 *
 * Revision 1.70  2004/09/26 13:00:20  ktr
 * bugfix
 *
 * Revision 1.69  2004/09/26 12:08:43  ktr
 * Constant index scalars are now known inside the with-loop body as well.
 *
 * Revision 1.68  2004/09/25 14:35:52  ktr
 * Whenever a generator is known to cover just one index, the withid is assumed
 * to be constant inside thate corresponding code.
 *
 * Revision 1.67  2004/09/24 17:05:46  ktr
 * Bug #60: Deactivated propagation of constant PRF-Arguments as this was not
 * always allowed.
 *
 * Revision 1.66  2004/09/22 22:14:09  ktr
 * SSACFShapeSel is now called correctly.
 *
 * Revision 1.65  2004/09/22 12:00:19  ktr
 * F_idx_shape_sel and F_shape_sel are now evaluated as well
 *
 * Revision 1.64  2004/09/22 10:06:58  ktr
 * Shape structures obtained via COGetShape are not freed any longer.
 *
 * Revision 1.63  2004/09/21 17:32:20  ktr
 * sel( iv, shape(A)) is now compiled into shape_sel(iv, A);
 * However, shape_sel itself is not yet treated by the CF.
 *
 * Revision 1.62  2004/09/21 16:07:21  ktr
 * Replaced bloated StructOpWrapper with seperate functions for
 * Sel, Reshape, idx_sel, Take, Drop
 *
 * Revision 1.61  2004/08/25 20:22:03  sbs
 * bad bug in SSACFCatVxV fixed!
 * result structural constant was NEVER allocated.
 * Now, it is reused and the constant is taken care of
 * by vec2_hidden_co.
 *
 * Revision 1.60  2004/08/25 16:28:19  ktr
 * ...now it even works :)
 *
 * Revision 1.59  2004/08/25 16:07:29  ktr
 * cat_VxV of an empty vector now yields the other vector.
 *
 * Revision 1.58  2004/07/23 13:59:13  ktr
 * AP arguments are no longer replaced by constants as this is now done
 * by ConstVarPropagation.
 *
 * Revision 1.57  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.56  2004/06/03 09:03:31  khf
 * Added support for prf F_idx_sel
 *
 * Revision 1.55  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 1.54  2004/03/05 19:14:27  mwe
 * representation of conditional changed
 * using N_funcond node instead of phi
 *
 * Revision 1.53  2004/03/02 09:17:07  khf
 * setting of AVIS_SSACONST in SSACFlet modified
 *
 * Revision 1.52  2004/02/06 14:19:33  mwe
 * remove usage of PHIASSIGN and ASSIGN2
 * implement usage of primitive phi function instead
 *
 * Revision 1.51  2003/11/28 10:25:21  sbs
 * L_VARDEC_OR_ARG_TYPE(IDS_VARDEC( ids), expr) used instead of IDS_TYPE( ids) = expr
 *
 * Revision 1.50  2003/11/25 14:30:23  sbs
 * type improvement after CF (newTC only) added.
 *
 * Revision 1.49  2003/11/06 15:24:10  ktr
 * SSACFStructOpWrapper now annotates correct shape information
 *
 * Revision 1.48  2003/09/26 10:25:16  sbs
 * new optimization for F_modarray added: if the index vector is an
 * empty vector, simply the element value constituts the result!
 *
 * Revision 1.47  2003/09/16 18:15:26  ktr
 * Index vectors are now treated as structural constants.
 *
 * Revision 1.46  2003/07/29 07:31:07  ktr
 * Added support for structural CF of cat_VxV and A[idx1][idx2] == A[idx1++idx2]
 *
 * Revision 1.45  2003/06/15 22:04:59  ktr
 * result is now initialized
 *
 * Revision 1.44  2003/06/13 09:26:15  ktr
 * Fixed bugs about missing calls of SHCopyShape
 *
 * Revision 1.43  2003/06/11 21:47:29  ktr
 * Added support for multidimensional arrays.
 *
 * Revision 1.42  2003/05/23 16:24:59  ktr
 * A multidimensional array is created if an array is found that contains
 * arrays itself.
 *
 * Revision 1.41  2003/04/07 14:22:01  sbs
 * F_drop_SxV and F_take_SxV mapped on the general versions (which have been extended
 * accordingly 8-)
 *
 * COCat used for F_cat_VxV now.
 *
 * Revision 1.40  2003/03/18 16:30:34  sah
 * added new prf cat_VxV, take_SxV, drop_SxV
 *
 * Revision 1.39  2002/10/09 21:58:22  dkr
 * optimization for 'reshape(shape(a),a)' added
 *
 * Revision 1.38  2002/10/09 12:44:04  dkr
 * structural constants exported now
 * (someone should move this stuff to constants.[ch] ...)
 *
 * Revision 1.37  2002/09/17 15:07:05  dkr
 * SSACFlet(): support for prfs with multiple return values added
 *
 * Revision 1.36  2002/09/13 22:13:44  dkr
 * detects (. % 0) now -> division by zero
 *
 * Revision 1.35  2002/09/11 23:07:59  dkr
 * rf_node_info.mac modified.
 *
 * Revision 1.34  2002/09/09 19:16:09  dkr
 * prf_string removed (mdb_prf used instead)
 *
 * Revision 1.33  2002/09/09 17:47:07  dkr
 * F_{add,sub,mul,div} replaced by F_{add,sub,mul,div}_SxS
 *
 * Revision 1.32  2002/09/05 20:51:23  dkr
 * SSACFGetShapeOfExpr(): DBUG_ASSERTs about unknown shapes removed
 *
 * Revision 1.31  2002/09/03 18:47:43  dkr
 * - new backend: constants propagation for N_ap activated again
 * - SSACFid(): support for dynamic types added
 *
 * Revision 1.29  2002/07/29 12:12:53  sbs
 * PRF_IF macro extended by z.
 *
 * Revision 1.28  2002/07/12 19:37:24  dkr
 * new backend: constants propagation for N_ap deactivated
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
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "globals.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "constants.h"
#include "shape.h"
#include "ctinfo.h"
#include "optimize.h"
#include "compare_tree.h"
#include "SSAConstantFolding.h"

/*
 * INFO struct
 */
struct INFO {
    bool remassign;
    node *fundef;
    bool insconst;
    node *postassign;
    node *assign;
    node *withid;
};

/*
 * INFO macros
 */
#define INFO_CF_REMASSIGN(n) (n->remassign)
#define INFO_CF_FUNDEF(n) (n->fundef)
#define INFO_CF_INSCONST(n) (n->insconst)
#define INFO_CF_POSTASSIGN(n) (n->postassign)
#define INFO_CF_ASSIGN(n) (n->assign)
#define INFO_CF_WITHID(n) (n->withid)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_CF_REMASSIGN (result) = FALSE;
    INFO_CF_FUNDEF (result) = NULL;
    INFO_CF_INSCONST (result) = FALSE;
    INFO_CF_POSTASSIGN (result) = NULL;
    INFO_CF_ASSIGN (result) = NULL;
    INFO_CF_WITHID (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 * constant identifiers should be substituted by its constant value
 * also in the arg-chain of N_ap and N_prf nodes (not for applications
 * of special fundefs! there must always be identifers).
 */
#define SUBST_ID_WITH_CONSTANT_IN_AP_ARGS TRUE

#define SUBST_NONE 0
#define SUBST_SCALAR 1
#define SUBST_SCALAR_AND_ARRAY 2

/* maximum of supported args for primitive functions */
#define PRF_MAX_ARGS 3

/* macros used in CFfoldPrfExpr to handle arguments selections */
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

/* structural constant (SCO) should be integrated in constants.[ch] in future */
/* special constant version used for structural constants */
struct STRUCT_CONSTANT {
    simpletype simpletype; /* basetype of struct constant */
    char *name;            /* only used for T_user !! */
    char *name_mod;        /* name of modul belonging to 'name' */
    shape *shape;          /* shape of struct constant */
    constant *hidden_co;   /* pointer to constant of pointers */
};

/* access macros for structural constant type */
#define SCO_BASETYPE(n) (n->simpletype)
#define SCO_NAME(n) (n->name)
#define SCO_MOD(n) (n->name_mod)
#define SCO_SHAPE(n) (n->shape)
#define SCO_HIDDENCO(n) (n->hidden_co)
#define SCO_ELEMDIM(n) (SHgetDim (SCO_SHAPE (n)) - COgetDim (SCO_HIDDENCO (n)))

/* local used helper functions */
static node *SetSsaAssign (node *chain, node *assign);
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

static constant *Dim (node *expr);
static constant *Shape (node *expr);

static node *StructOpSel (constant *idx, node *expr);
static node *StructOpReshape (constant *idx, node *expr);
static node *StructOpIdxSel (constant *idx, node *expr);
static node *StructOpTake (constant *idx, node *expr);
static node *StructOpDrop (constant *idx, node *expr);

/* implements: arithmetical opt. for add, sub, mul, div, and, or */
static node *ArithmOpWrapper (prf op, constant **arg_co, node **arg_expr);

/*
 * some primitive functions that allows special optimizations in more
 * generic cases
 */
static node *Eq (node *expr1, node *expr2);
static node *Sub (node *expr1, node *expr2);
static node *Modarray (node *a, constant *idx, node *elem);
static node *Sel (node *idx_expr, node *array_expr);

/*
 * functions to handle SCOs
 */

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
    int dim;

    DBUG_ENTER ("SCOExpr2StructConstant");

    struc_co = NULL;

    if (NODE_TYPE (expr) == N_array) {
        /* expression is an array */
        struc_co = CFscoArray2StructConstant (expr);
    } else {
        if (NODE_TYPE (expr) == N_id) {
            if (AVIS_SSAASSIGN (ID_AVIS (expr)) != NULL) {
                /* expression is an identifier */
                if ((TYisAKD (AVIS_TYPE (ID_AVIS (expr))))
                    || (TYisAKS (AVIS_TYPE (ID_AVIS (expr))))
                    || (TYisAKV (AVIS_TYPE (ID_AVIS (expr))))) {
                    dim = TYgetDim (AVIS_TYPE (ID_AVIS (expr)));
                } else {
                    dim = -1;
                }

                if (dim == SCALAR) {
                    /* id is a defined scalar */
                    struc_co = CFscoScalar2StructConstant (expr);
                } else if (dim > SCALAR) {
                    /* id is a defined array */
                    struc_co = CFscoArray2StructConstant (expr);
                }
            } else {
                if (AVIS_WITHID (ID_AVIS (expr))) {
                    /* expression is given by a withid */
                    if ((TYisAKD (AVIS_TYPE (ID_AVIS (expr))))
                        || (TYisAKS (AVIS_TYPE (ID_AVIS (expr))))
                        || (TYisAKV (AVIS_TYPE (ID_AVIS (expr))))) {
                        dim = TYgetDim (AVIS_TYPE (ID_AVIS (expr)));
                    } else {
                        dim = -1;
                    }

                    if (dim == SCALAR) {
                        /* id is a defined scalar */
                        struc_co = CFscoScalar2StructConstant (expr);
                    } else if (dim > SCALAR) {
                        /* id is a withid vector */
                        struc_co = CFscoWithidVec2StructConstant (expr);
                    }
                }
            }
        }
    }

    DBUG_RETURN (struc_co);
}

/******************************************************************************
 *
 * function:
 *   struct_constant *CFscoWithidVec2StructConstant(node *expr)
 *
 * description:
 *   converts a vector defined in a NWithId-Node into a structural constant.
 *
 *****************************************************************************/
struct_constant *
CFscoWithidVec2StructConstant (node *expr)
{
    struct_constant *struc_co;
    node *withid;
    node *scalars;
    node *exprs;
    int elem_count;
    node *tmp;
    ntype *vtype;
    node **node_vec;
    shape *vshape;
    int i;

    DBUG_ENTER ("CFscoWithidVec2StructConstant");

    DBUG_ASSERT ((NODE_TYPE (expr) == N_id),
                 "CFscoWithidVec2StructConstant supports only N_id nodes");

    withid = AVIS_WITHID (ID_AVIS (expr));
    scalars = WITHID_IDS (withid);
    exprs = TCids2Exprs (scalars);
    elem_count = TCcountExprs (scalars);

    /* create structural constant of vector */
    vtype = AVIS_TYPE (ID_AVIS (expr));

    /* alloc hidden vector */
    vshape = SHcreateShape (1, elem_count);
    node_vec = (node **)ILIBmalloc (elem_count * sizeof (node *));

    /* copy element pointers from array to vector */
    tmp = exprs;
    for (i = 0; i < elem_count; i++) {
        node_vec[i] = EXPRS_EXPR (tmp);
        tmp = EXPRS_NEXT (tmp);
    }

    /* create struct_constant */
    struc_co = ILIBmalloc (sizeof (struct_constant));
    SCO_BASETYPE (struc_co) = T_int;
    if (TYisUser (vtype)) {
        SCO_NAME (struc_co) = TYgetName (vtype);
        SCO_MOD (struc_co) = TYgetMod (vtype);
    } else {
        SCO_NAME (struc_co) = NULL;
        SCO_MOD (struc_co) = NULL;
    }
    SCO_SHAPE (struc_co) = SHcopyShape (vshape);

    SCO_HIDDENCO (struc_co) = COmakeConstant (T_hidden, vshape, node_vec);

    DBUG_RETURN (struc_co);
}

/******************************************************************************
 *
 * function:
 *   struct_constant *CFscoArray2StructConstant(node *expr)
 *
 * description:
 *   converts an N_array node (or a N_id of a defined array) from AST to
 *   a structural constant. To convert an array to a structural constant
 *   all array elements must be scalars!
 *
 *****************************************************************************/

struct_constant *
CFscoArray2StructConstant (node *expr)
{
    struct_constant *struc_co;
    node *array;
    ntype *atype;
    shape *realshape;
    shape *ashape;
    node **node_vec;
    node *tmp;
    bool valid_const;
    int elem_count;
    int i;

    DBUG_ENTER ("CFscoArray2StructConstant");

    DBUG_ASSERT (((NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_id)),
                 "CFscoArray2StructConstant supports only N_array and N_id nodes");

    atype = NULL;

    if (NODE_TYPE (expr) == N_array) {
        /* explicit array as N_array node */
        array = expr;
        /* shape of the given array */
        atype = NTCnewTypeCheck_Expr (array);
    } else if ((NODE_TYPE (expr) == N_id) && (AVIS_SSAASSIGN (ID_AVIS (expr)) != NULL)
               && (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (expr)))))
                   == N_array)) {
        /* indirect array via defined vardec */

        array = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (expr))));

        /* shape of the given array */
        atype = AVIS_TYPE (ID_AVIS (expr));
    } else {
        /* unsupported node type */
        array = NULL;
    }

    /* build an abstract structural constant of type (void*) T_hidden */
    if ((array != NULL) && (TYisAKS (atype) || TYisAKV (atype))) {
        /* alloc hidden vector */
        realshape = SHcopyShape (TYgetShape (atype));
        ashape = SHcopyShape (ARRAY_SHAPE (array));

        elem_count = SHgetUnrLen (ashape);
        node_vec = (node **)ILIBmalloc (elem_count * sizeof (node *));

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
        struc_co = (struct_constant *)ILIBmalloc (sizeof (struct_constant));
        SCO_BASETYPE (struc_co) = TYgetSimpleType (TYgetScalar (atype));
        if (TYisUser (atype)) {
            SCO_NAME (struc_co) = TYgetName (atype);
            SCO_MOD (struc_co) = TYgetMod (atype);
        } else {
            SCO_NAME (struc_co) = NULL;
            SCO_MOD (struc_co) = NULL;
        }
        SCO_SHAPE (struc_co) = realshape;

        SCO_HIDDENCO (struc_co) = COmakeConstant (T_hidden, ashape, node_vec);

        /* remove invalid structural arrays */
        if (!valid_const) {
            struc_co = CFscoFreeStructConstant (struc_co);
        }
    } else {
        /* no array with known elements */
        struc_co = NULL;
    }

    if (NODE_TYPE (expr) == N_array) {
        atype = TYfreeType (atype);
    }

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

struct_constant *
CFscoScalar2StructConstant (node *expr)
{
    struct_constant *struc_co;
    shape *cshape;
    ntype *ctype;
    node **elem;
    nodetype nt;

    DBUG_ENTER ("CFscoScalar2StructConstant");

    nt = NODE_TYPE (expr);

    if ((nt == N_num) || (nt == N_float) || (nt == N_double) || (nt == N_bool)
        || ((nt == N_id) && (TYgetDim (AVIS_TYPE (ID_AVIS (expr))) == SCALAR))) {
        /* create structural constant as scalar */
        ctype = AVIS_TYPE (ID_AVIS (expr));

        /* alloc hidden vector */
        cshape = SHmakeShape (0);
        elem = (node **)ILIBmalloc (sizeof (node *));

        /* copy element pointers from array to vector */
        *elem = expr;

        /* create struct_constant */
        struc_co = (struct_constant *)ILIBmalloc (sizeof (struct_constant));
        SCO_BASETYPE (struc_co) = TYgetSimpleType (TYgetScalar (ctype));
        if (TYisUser (ctype)) {
            SCO_NAME (struc_co) = TYgetName (ctype);
            SCO_MOD (struc_co) = TYgetMod (ctype);
        } else {
            SCO_NAME (struc_co) = NULL;
            SCO_MOD (struc_co) = NULL;
        }
        SCO_SHAPE (struc_co) = SHcopyShape (cshape);
        SCO_HIDDENCO (struc_co) = COmakeConstant (T_hidden, cshape, elem);

    } else {
        struc_co = NULL;
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
        expr = TBmakeArray (SHcopyShape (COgetShape (SCO_HIDDENCO (struc_co))), aelems);
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

    /* free shape */
    SCO_SHAPE (struc_co) = SHfreeShape (SCO_SHAPE (struc_co));

    /* free substructure */
    SCO_HIDDENCO (struc_co) = COfreeConstant (SCO_HIDDENCO (struc_co));

    /* free structure */
    struc_co = ILIBfree (struc_co);

    DBUG_RETURN ((struct_constant *)NULL);
}

/*
 * functions for internal use only
 */

/******************************************************************************
 *
 * function:
 *   node *SetSsaAssign(node *chain, node *assign)
 *
 * description:
 *   sets the AVIS_SSAASSIGN to the correct assignment.
 *   the backrefs may be wrong after Inlining, because DupTree is not able
 *   to correct them when dubbing an assignment chain only.
 *
 *****************************************************************************/

static node *
SetSsaAssign (node *chain, node *assign)
{
    DBUG_ENTER ("CFSetSSAASSIGN");

    if (chain != NULL) {
        /* set current assign as defining assignement */
        AVIS_SSAASSIGN (IDS_AVIS (chain)) = assign;

        /* check correct setting of SSAASSIGN() attribute */
        if (AVIS_SSAASSIGN (IDS_AVIS (chain)) != assign) {
            DBUG_PRINT ("WARN",
                        ("mismatch SSAASSIGN link for %s - 1:%p - %p", IDS_NAME (chain),
                         AVIS_SSAASSIGN (IDS_AVIS (chain)), assign));
        }

        /* traverse to next ids */
        IDS_NEXT (chain) = SetSsaAssign (IDS_NEXT (chain), assign);
    }

    DBUG_RETURN (chain);
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

static constant *
Dim (node *expr)
{
    constant *result;
    int dim;

    DBUG_ENTER ("Dim");

    if (NODE_TYPE (expr) == N_id) {
        if ((TYisAKD (AVIS_TYPE (ID_AVIS (expr))))
            || (TYisAKS (AVIS_TYPE (ID_AVIS (expr))))
            || (TYisAKV (AVIS_TYPE (ID_AVIS (expr))))) {
            dim = TYgetDim (AVIS_TYPE (ID_AVIS (expr)));
            result = COmakeConstantFromInt (dim);
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
 *   node *Shape(node *expr)
 *
 * description:
 *   computes the shape of a given identifier. returns the shape as expression
 *   or NULL if no constant shape can be infered.
 *   for userdefined types the result is the shape in simpletype elements.
 *
 *****************************************************************************/

static constant *
Shape (node *expr)
{
    constant *result;
    int dim;
    shape *cshape;
    int *int_vec;

    DBUG_ENTER ("Dim");

    if (NODE_TYPE (expr) == N_id) {
        if ((TYisAKD (AVIS_TYPE (ID_AVIS (expr))))
            || (TYisAKS (AVIS_TYPE (ID_AVIS (expr))))
            || (TYisAKV (AVIS_TYPE (ID_AVIS (expr))))) {

            dim = TYgetDim (AVIS_TYPE (ID_AVIS (expr)));
            /* store known shape as constant (int vector of len dim) */

            /*cshape = SHoldTypes2Shape(VARDEC_OR_ARG_TYPE(AVIS_DECL(ID_AVIS(expr)))); */
            cshape = TYgetShape (AVIS_TYPE (ID_AVIS (expr)));
            int_vec = SHshape2IntVec (cshape);

            result = COmakeConstant (T_int, SHcreateShape (1, dim), int_vec);
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
 *   node *StructOpIdxSel(constant *idx, node *arg_expr)
 *
 * description:
 *   computes structural idxsel on array expressions with
 *   constant index vector.
 *
 *****************************************************************************/

static node *
StructOpIdxSel (constant *idx, node *expr)
{
    struct_constant *struc_co;
    node *result;
    constant *old_hidden_co;

    DBUG_ENTER ("StructOpIdxSel");

    /*
     * tries to convert expr(especially arrays) into a structual constant
     */
    struc_co = CFscoExpr2StructConstant (expr);

    /*
     * given expression could be converted to struct_constant
     */
    if (struc_co != NULL) {
        /*
         * save internal hidden input constant
         */
        old_hidden_co = SCO_HIDDENCO (struc_co);

        /*
         * perform struc-op on hidden constant
         */
        SCO_HIDDENCO (struc_co) = COidxSel (idx, SCO_HIDDENCO (struc_co));

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
    } else {
        result = NULL;
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
 *   computes structural take on array expressions with constant index vector.
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
                tmp_co = COmakeFalse (target_shp);
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
    ntype *etype = NULL;
    shape *shp = NULL;

    DBUG_ENTER ("GetShapeOfExpr");

    DBUG_ASSERT ((expr != NULL), "GetShapeOfExpr called with NULL pointer");

    etype = NTCnewTypeCheck_Expr (expr);

    if (TYisAKS (etype) || TYisAKV (etype)) {
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
 *   implement Modarray on gerneric structural constant arrays with given
 *   full constant index vector. This works like CFStructOpWrapper() but
 *   has been moved to a separate function because of different function
 *   signature.
 *
 ******************************************************************************/

static node *
Modarray (node *a, constant *idx, node *elem)
{
    node *result;
    struct_constant *struc_a;
    struct_constant *struc_elem;
    constant *old_hidden_co;
    node *newarray;

    DBUG_ENTER ("Modarray");

    /**
     * if the index is an empty vector, we simply replace the entire
     * expression by the elem value!
     */
    if (COisEmptyVect (idx)) {
        result = DUPdoDupTree (elem);
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

            if (SCO_ELEMDIM (struc_a) != SCO_ELEMDIM (struc_elem)) {
                newarray = TCmakeFlatArray (TBmakeExprs (elem, NULL));
                struc_elem = CFscoFreeStructConstant (struc_elem);
                struc_elem = CFscoArray2StructConstant (newarray);
            }

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
            } else
                result = NULL;
        } else {
            result = NULL;
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
 *   node *ShapeSel(constant *idx, node *array_expr)
 *
 * description:
 *   selects element idx from the shape vector of array_expr if
 *   its shape is known
 *
 *****************************************************************************/
static node *
ShapeSel (constant *idx, node *array_expr)
{
    node *res = NULL;

    int shape_elem;

    DBUG_ENTER ("ShapeSel");

    if (TYisAKS (AVIS_TYPE (ID_AVIS (array_expr)))
        || TYisAKV (AVIS_TYPE (ID_AVIS (array_expr)))) {

        shape_elem = ((int *)COgetDataVec (idx))[0];

        res = TBmakeNum (
          SHgetExtent (TYgetShape (AVIS_TYPE (ID_AVIS (array_expr))), shape_elem));
    }

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
    node *prf_shape;
    node *concat;
    node *mod_arr_expr;
    node *mod_idx_expr;
    node *mod_elem_expr;
    constant *idx_co;
    constant *mod_idx_co;
    constant *old_hidden_co;
    constant *zero_co;
    struct_constant *idx_struc;

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

        case F_shape:
            /*
             * sel( [i], shape( A)) can be optimized using F_shape_sel.
             *
             * => _shape_sel_( [i], A)
             */
            prf_shape = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (array_expr)));

            idx_struc = CFscoExpr2StructConstant (idx_expr);
            if (idx_struc != NULL) {
                /*
                 * save internal hidden input constant
                 */
                old_hidden_co = SCO_HIDDENCO (idx_struc);

                zero_co = COmakeConstantFromInt (0);
                SCO_HIDDENCO (idx_struc) = COidxSel (zero_co, SCO_HIDDENCO (idx_struc));
                zero_co = COfreeConstant (zero_co);

                /*
                 * free internal input constant
                 */
                old_hidden_co = COfreeConstant (old_hidden_co);

                result
                  = TCmakePrf2 (F_idx_shape_sel, CFscoDupStructConstant2Expr (idx_struc),
                                DUPdoDupNode (PRF_ARG1 (prf_shape)));

                /*
                 * free scruct constant
                 */
                idx_struc = CFscoFreeStructConstant (idx_struc);
            } else {
                result = TCmakePrf2 (F_shape_sel, DUPdoDupNode (idx_expr),
                                     DUPdoDupNode (PRF_ARG1 (prf_shape)));
            }
            break;

        default:
            break;
        }
    }

    DBUG_RETURN (result);
}

/*
 * traversal functions for CF traversal
 */

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

    INFO_CF_FUNDEF (arg_info) = arg_node;

    if ((FUNDEF_ARGS (arg_node) != NULL) && (FUNDEF_ISDOFUN (arg_node))) {
        /* traverse args of fundef */
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
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

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

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
    bool remove_assignment;
    node *tmp;
    node *old_assign;

    DBUG_ENTER ("CFassign");

    /* stack current assignment */
    old_assign = INFO_CF_ASSIGN (arg_info);

    /* init flags for possible code removal/movement */
    INFO_CF_REMASSIGN (arg_info) = FALSE;
    INFO_CF_POSTASSIGN (arg_info) = NULL;
    INFO_CF_ASSIGN (arg_info) = arg_node;
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    /* restore old assignment */
    INFO_CF_ASSIGN (arg_info) = old_assign;

    /* save removal flag for modifications in bottom-up traversal */
    remove_assignment = INFO_CF_REMASSIGN (arg_info);

    /* integrate post assignments after current assignment */
    ASSIGN_NEXT (arg_node)
      = TCappendAssign (INFO_CF_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
    INFO_CF_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (remove_assignment) {
        /* skip this assignment and free it */
        DBUG_PRINT ("CF", ("remove dead assignment"));

        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);

        tmp = FREEdoFreeNode (tmp);
    }

    DBUG_RETURN (arg_node);
}

/***********************************************************************
 *
 *  function:
 *    node *CFfuncond(node *arg_node, node* arg_info)
 *
 *  description:
 *    Check if the conditional predicate is constant.
 *    If it is constant, than resolve all funcond nodes according
 *    to the predicate and set the inline flag.
 *
 **********************************************************************/

node *
CFfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFfuncond");

    DBUG_ASSERT ((FUNCOND_IF (arg_node) != NULL), "missing condition in conditional");
    DBUG_ASSERT ((FUNCOND_THEN (arg_node) != NULL), "missing then part in conditional");
    DBUG_ASSERT ((FUNCOND_ELSE (arg_node) != NULL), "missing else part in conditional");

    /*
     * traverse condition to analyse for constant expression
     * and substitute constants with their values to get
     * a simple N_bool node for the condition (if constant)
     */
    if ((N_id == NODE_TYPE (FUNCOND_IF (arg_node)))
        && (TYisAKV (AVIS_TYPE (ID_AVIS (FUNCOND_IF (arg_node)))))) {
        node *tmp;

        tmp = COconstant2AST (TYgetValue (AVIS_TYPE (ID_AVIS (FUNCOND_IF (arg_node)))));
        FUNCOND_IF (arg_node) = FREEdoFreeNode (FUNCOND_IF (arg_node));
        FUNCOND_IF (arg_node) = tmp;
    }

    /* check for constant condition */
    if (NODE_TYPE (FUNCOND_IF (arg_node)) == N_bool) {
        node *tmp;

        if (BOOL_VAL (FUNCOND_IF (arg_node)) == TRUE) {

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

    DBUG_ASSERT ((COND_COND (arg_node) != NULL), "missing condition in conditional");
    DBUG_ASSERT ((COND_THEN (arg_node) != NULL), "missing then part in conditional");
    DBUG_ASSERT ((COND_ELSE (arg_node) != NULL), "missing else part in conditional");

    /*
     * traverse condition to analyse for constant expression
     * and substitute constants with their values to get
     * a simple N_bool node for the condition (if constant)
     */

    if ((N_id == NODE_TYPE (COND_COND (arg_node)))
        && (TYisAKV (AVIS_TYPE (ID_AVIS (COND_COND (arg_node)))))) {
        node *tmp;

        tmp = COconstant2AST (TYgetValue (AVIS_TYPE (ID_AVIS (COND_COND (arg_node)))));
        COND_COND (arg_node) = FREEdoFreeNode (COND_COND (arg_node));
        COND_COND (arg_node) = tmp;
    }

    /* check for constant condition */
    if (NODE_TYPE (COND_COND (arg_node)) == N_bool) {
        if (BOOL_VAL (COND_COND (arg_node)) == TRUE) {
            DBUG_PRINT ("CF", ("found condition with condition==true, select then part"));

            /* select then-part for later insertion in assignment chain */
            INFO_CF_POSTASSIGN (arg_info) = BLOCK_INSTR (COND_THEN (arg_node));

            if (NODE_TYPE (INFO_CF_POSTASSIGN (arg_info)) == N_empty) {
                /* empty code block must not be moved */
                INFO_CF_POSTASSIGN (arg_info) = NULL;
            } else {
                /*
                 * delete pointer to codeblock to preserve assignments from
                 * being freed
                 */
                BLOCK_INSTR (COND_THEN (arg_node)) = NULL;
            }
        } else {
            /* select else part */
            DBUG_PRINT ("CF",
                        ("found condition with condition==false, select else part"));

            /* select else-part for later insertion in assignment chain */
            INFO_CF_POSTASSIGN (arg_info) = BLOCK_INSTR (COND_ELSE (arg_node));

            if (NODE_TYPE (INFO_CF_POSTASSIGN (arg_info)) == N_empty) {
                /* empty code block must not be moved */
                INFO_CF_POSTASSIGN (arg_info) = NULL;
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
        INFO_CF_REMASSIGN (arg_info) = TRUE;

        /*
         * because there can be only one conditional in a special function
         * now this special function contains no conditional and therefore
         * is no special function anymore. this function can now be inlined
         * without any problems.
         * if this is a do- or while function and the condition is evaluated
         * to true we have an endless loop and will rise a warning message.
         */
        if ((BOOL_VAL (COND_COND (arg_node)) == TRUE)
            && (FUNDEF_ISDOFUN (INFO_CF_FUNDEF (arg_info)))) {
            CTIwarnLine (NODE_LINE (arg_node),
                         "Infinite loop detected, program may not terminate");
        }

        FUNDEF_ISCONDFUN (INFO_CF_FUNDEF (arg_info)) = FALSE;
        FUNDEF_ISINLINE (INFO_CF_FUNDEF (arg_info)) = TRUE;
    } else {
        /*
         * no constant condition:
         * do constant folding in conditional
         * traverse then-part
         */
        if (COND_THEN (arg_node) != NULL) {
            COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
        }

        /* traverse else-part */
        if (COND_ELSE (arg_node) != NULL) {
            COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFreturn(node *arg_node, info *arg_info)
 *
 * description:
 *   do NOT substitute identifiers in return statement with their value!
 *
 *****************************************************************************/

node *
CFreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFreturn");

    /* do NOT substitue constant identifiers with their value */
    INFO_CF_INSCONST (arg_info) = SUBST_NONE;
    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFap(node *arg_node, info *arg_info)
 *
 * description:
 *   propagate constants and traverse in special function
 *
 *****************************************************************************/

node *
CFap (node *arg_node, info *arg_info)
{
    info *new_arg_info;

    DBUG_ENTER ("CFap");

    /* traverse special fundef without recursion */
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_CF_FUNDEF (arg_info))) {
        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("CF", ("traversal of special fundef %s finished\n",
                           FUNDEF_NAME (AP_FUNDEF (arg_node))));
        new_arg_info = FreeInfo (new_arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFid(node *arg_node, info *arg_info)
 *
 * description:
 *   substitute identifers with their computed constant
 *   (only when INFO_CF_INSCONST flag is set)
 *   in EXPRS chain of N_ap ARGS  (if SUBST_ID_WITH_CONSTANT_IN_AP_ARGS == TRUE)
 *      EXPRS chain of N_prf ARGS (if SUBST_ID_WITH_CONSTANT_IN_AP_ARGS == TRUE)
 *      EXPRS chain of N_array AELEMS
 *
 *****************************************************************************/
/*
 * MWE
 * still necessary
 * replace SSACONST by const from akv-type
 */
node *
CFid (node *arg_node, info *arg_info)
{
    node *new_node;
    constant *const_node;
    int dim;

    DBUG_ENTER ("CFid");

    /* check for constant scalar identifier */
    if (TYisAKV (AVIS_TYPE (ID_AVIS (arg_node)))) {
        const_node = TYgetValue (AVIS_TYPE (ID_AVIS (arg_node)));
        dim = COgetDim (const_node);

        if (((dim == SCALAR) && (INFO_CF_INSCONST (arg_info) >= SUBST_SCALAR))
            || ((dim > SCALAR)
                && (INFO_CF_INSCONST (arg_info) == SUBST_SCALAR_AND_ARRAY))) {
            DBUG_PRINT ("CF", ("substitue identifier %s through its value",
                               VARDEC_OR_ARG_NAME (AVIS_DECL (ID_AVIS (arg_node)))));

            /* substitute identifier with its value */
            new_node = COconstant2AST (const_node);
            arg_node = FREEdoFreeTree (arg_node);
            arg_node = new_node;
        }
        /*const_node = COfreeConstant( const_node);*/
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
    node *oldelems, *tmp;
    shape *shp = NULL, *newshp;

    DBUG_ENTER ("CFarray");

    /* substitute constant identifiers in array elements */
    INFO_CF_INSCONST (arg_info) = TRUE;
    if (ARRAY_AELEMS (arg_node) != NULL) {
        ARRAY_AELEMS (arg_node) = TRAVdo (ARRAY_AELEMS (arg_node), arg_info);

        /* Test whether subarrays can be copied in */
        /* Therefore all elemens need to be id nodes defined by N_array nodes.
           Furthermore, they must all add the same dimensionality to
           the dimension of their children */
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
            /* Merge subarrays into this arrays */
            oldelems = ARRAY_AELEMS (arg_node);
            tmp = oldelems;
            while (tmp != NULL) {
                newelems
                  = TCappendExprs (newelems, DUPdoDupTree (ARRAY_AELEMS (ASSIGN_RHS (
                                               ID_SSAASSIGN (EXPRS_EXPR (tmp))))));
                tmp = EXPRS_NEXT (tmp);
            }
            newshp = SHappendShapes (ARRAY_SHAPE (arg_node), shp);

            FREEdoFreeTree (arg_node);

            arg_node = TBmakeArray (newshp, newelems);
        }
    }
    INFO_CF_INSCONST (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
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

    /*
     * ktr: There is no reason to do this as we have CVP now
     *      CFfoldPrfExpr will look at the constant anyways
     *
     * substitute constant identifiers in prf. arguments
     */
    INFO_CF_INSCONST (arg_info) = SUBST_ID_WITH_CONSTANT_IN_AP_ARGS;
    if (PRF_ARGS (arg_node) != NULL) {
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
    }
    INFO_CF_INSCONST (arg_info) = FALSE;

    /* look up arguments */
    arg_expr = GetPrfArgs (arg_expr, PRF_ARGS (arg_node), PRF_MAX_ARGS);

    /* try some constant folding */
    new_node = CFfoldPrfExpr (PRF_PRF (arg_node), arg_expr);

    if (new_node != NULL) {
        /* free this primitive function and substitute it with new node */
        arg_node = FREEdoFreeTree (arg_node);
        arg_node = new_node;

        /* increment constant folding counter */
        cf_expr++;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CFwith( node *arg_node, info *arg_info)
 *
 * description:
 *   traverses parts and withops
 *
 *****************************************************************************/

node *
CFwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFwith");

    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    if (WITH_WITHOP (arg_node) != NULL) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CFpart( node *arg_node, info *arg_info)
 *
 * description:
 *   traverses withid, generators and code
 *   if the current generator covers only one element and the current
 *   code is only used once, withid can become constant in the code
 *
 *****************************************************************************/

node *
CFpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFpart");

    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);

    /*
     * If this part's code is only used by this part,
     * withid may become constant
     */
    if (CODE_USED (PART_CODE (arg_node)) == 1) {
        INFO_CF_WITHID (arg_info) = PART_WITHID (arg_node);
    }
    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFgenerator(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses parameter of generator to substitute constant arrays
 *   with their array representation to allow constant folding on known
 *   shape information.
 *   Furthermore, index elements which are known to be constant in this
 *   generator (e.g. [0,0] <= iv = [i,j] < [1000,1] => j == 0) are
 *   augmented with a constant.
 *
 *****************************************************************************/

node *
CFgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFgenerator");

    INFO_CF_INSCONST (arg_info) = SUBST_SCALAR_AND_ARRAY;
    DBUG_PRINT ("CF", ("substitute constant generator parameters"));

    if (GENERATOR_BOUND1 (arg_node) != NULL) {
        GENERATOR_BOUND1 (arg_node) = TRAVdo (GENERATOR_BOUND1 (arg_node), arg_info);
    }
    if (GENERATOR_BOUND2 (arg_node) != NULL) {
        GENERATOR_BOUND2 (arg_node) = TRAVdo (GENERATOR_BOUND2 (arg_node), arg_info);
    }
    if (GENERATOR_STEP (arg_node) != NULL) {
        GENERATOR_STEP (arg_node) = TRAVdo (GENERATOR_STEP (arg_node), arg_info);
    }
    if (GENERATOR_WIDTH (arg_node) != NULL) {
        GENERATOR_WIDTH (arg_node) = TRAVdo (GENERATOR_WIDTH (arg_node), arg_info);
    }

    if (INFO_CF_WITHID (arg_info) != NULL) {
        /*
         * If the upper and lower bounds vectors are known and the index space
         * covers just one element, the index vector is constant
         */
        if ((GENERATOR_BOUND1 (arg_node) != NULL)
            && (GENERATOR_BOUND2 (arg_node) != NULL)) {
#if 0
      constant *lower, *upper, *diff;
      shape *diffshp;
      node *_ids;

      /*MWE
       * change to usage of akv
       * types are modified: move to type_upgrade
       */ 
      lower = COaST2Constant( GENERATOR_BOUND1( arg_node));
      upper = COaST2Constant( GENERATOR_BOUND2( arg_node));

      if (( lower != NULL) && ( upper != NULL)) {
        diff     = COsub( upper, lower);
        diffshp  = COconstant2Shape( diff);
        diff     = COfreeConstant( diff);
        
        /*
         * Check whether the whole index vector is constant
         */
        if  ( SHgetUnrLen( diffshp) == 1) {
          _ids = WITHID_VEC( INFO_CF_WITHID( arg_info));
          if ( _ids != NULL) {
            AVIS_SSACONST( IDS_AVIS( _ids)) = COcopyConstant( lower);
          }
        }
        diffshp = SHfreeShape( diffshp);
      }

      if (lower != NULL) {
        lower = COfreeConstant( lower);
      }
      if (upper != NULL) {
        upper = COfreeConstant( upper);
      }

#endif
            if ((NODE_TYPE (GENERATOR_BOUND1 (arg_node)) == N_array)
                && (NODE_TYPE (GENERATOR_BOUND2 (arg_node)) == N_array)) {
                /*
                 * Check whether the index scalars are constant
                 */
                node *_ids;
                node *lb, *ub;
#if 0
        constant *lbc, *ubc;
#endif
                lb = ARRAY_AELEMS (GENERATOR_BOUND1 (arg_node));
                ub = ARRAY_AELEMS (GENERATOR_BOUND2 (arg_node));
                _ids = WITHID_IDS (INFO_CF_WITHID (arg_info));

#if 0
	/* MWE
	 * change to akv
	 * types are modified: move to type upgrade
	 */
        while (_ids != NULL) {
          lbc = COaST2Constant( EXPRS_EXPR( lb));
          ubc = COaST2Constant( EXPRS_EXPR( ub));

          if ( ( lbc != NULL) && ( ubc != NULL)) {
            diff = COsub( ubc, lbc);
            if ( COisOne( diff, TRUE)) {
              AVIS_SSACONST( IDS_AVIS( _ids)) = COcopyConstant( lbc);
            }
            diff = COfreeConstant( diff);
          }

          if ( lbc != NULL) {
            lbc = COfreeConstant( lbc);
          }

          if ( ubc != NULL) {
            ubc = COfreeConstant( ubc);
          }
          
          lb = EXPRS_NEXT( lb);
          ub = EXPRS_NEXT( ub);
          _ids = IDS_NEXT( _ids);
        }
#endif
            }
        }
        INFO_CF_WITHID (arg_info) = NULL;
    }

    INFO_CF_INSCONST (arg_info) = SUBST_NONE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CFcode(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses CODE_CBLOCK and not CEXPRS
 *
 *****************************************************************************/
node *
CFcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CFNcode");

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CFfoldPrfExpr(prf op, node **arg_expr)
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
CFfoldPrfExpr (prf op, node **arg_expr)
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
        break;

    case F_tof_S:
    case F_tof_A:
        break;

    case F_tod_S:
    case F_tod_A:
        break;

    case F_abs:
        break;

    case F_not:
        break;

    case F_dim:
        if
            ONE_CONST_ARG (arg_co)
            {
            }
        else if
            ONE_ARG (arg_expr)
            {
                /* for some non full constant expression */
                new_co = Dim (arg_expr[0]);
            }
        break;

    case F_shape:
        if
            ONE_CONST_ARG (arg_co)
            {
            }
        else if
            ONE_ARG (arg_expr)
            {
                /* for some non full constant expression */
                new_co = Shape (arg_expr[0]);
            }
        break;

        /* two-argument functions */
    case F_min:
        break;

    case F_max:
        break;

    case F_add_SxS:
    case F_add_AxS:
    case F_add_SxA:
    case F_add_AxA:
        if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = ArithmOpWrapper (F_add_SxS, arg_co, arg_expr);
            }
        break;

    case F_sub_SxS:
    case F_sub_AxS:
    case F_sub_SxA:
    case F_sub_AxA:
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

    case F_mul_SxS:
    case F_mul_AxS:
    case F_mul_SxA:
    case F_mul_AxA:
        if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = ArithmOpWrapper (F_mul_SxS, arg_co, arg_expr);
            }
        break;

    case F_div_SxS:
    case F_div_SxA:
    case F_div_AxS:
    case F_div_AxA:
        if
            TWO_CONST_ARG (arg_co)
            {
                if (COisZero (arg_co[1], FALSE)) { /* any 0 in divisor, x/0 -> err */
                    CTIabortLine (NODE_LINE (arg_expr[1]), "Division by zero expected");
                }
            }
        if
            ONE_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                new_node = ArithmOpWrapper (F_div_SxS, arg_co, arg_expr);
            }
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
                /*	shape *shp = SHoldTypes2Shape( TYtype2OldType( ID_NTYPE(
                 * arg_expr[1])));*/
                shape *shp = SHcopyShape (TYgetShape (ID_NTYPE (arg_expr[1])));
                if (shp != NULL) {
                    if (SHcompareWithCArray (shp, COgetDataVec (arg_co[0]),
                                             SHgetExtent (COgetShape (arg_co[0]), 0))) {
                        new_node = DUPdoDupNode (arg_expr[1]);
                    }
                    shp = SHfreeShape (shp);
                }
            }
        }
        break;

    case F_idx_shape_sel:
    case F_shape_sel:
        if
            FIRST_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                /* for some none constant expression and constant index vector */
                new_node = ShapeSel (arg_co[0], arg_expr[1]);
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

    case F_idx_sel:
        if
            TWO_CONST_ARG (arg_co)
            {
                /* for pure constant args */
                new_co = COidxSel (arg_co[0], arg_co[1]);
            }
        else if
            FIRST_CONST_ARG_OF_TWO (arg_co, arg_expr)
            {
                /* for some non constant expression and constant index skalar */
                new_node = StructOpIdxSel (arg_co[0], arg_expr[1]);
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

        /* three-argument functions */
    case F_modarray:
        if (SECOND_CONST_ARG_OF_THREE (arg_co, arg_expr)) {
            new_node = Modarray (arg_expr[0], arg_co[1], arg_expr[2]);
        }
        break;

    case F_cat_VxV:
        if
            TWO_CONST_ARG (arg_co)
            {
            }
        else {
            new_node = CatVxV (arg_expr[0], arg_expr[1]);
        }
        break;

    case F_cat:
        /* not implemented yet */
        break;

    case F_rotate:
        /* not implemented yet */
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
 *   node* SSAConstantFolding(node* fundef, node* modul)
 *
 * description:
 *   starts the DeadCodeRemoval for the given fundef. This fundef must not be
 *   a special fundef (these will be traversed in their order of application).
 *
 ******************************************************************************/

node *
CFdoConstantFolding (node *fundef, node *module)
{
    info *arg_info;

    DBUG_ENTER ("CFdoConstantFolding");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "CFdoConstantFolding called for non-fundef node");

    DBUG_PRINT ("OPT",
                ("starting constant folding (ssa) in function %s", FUNDEF_NAME (fundef)));

    /* do not start traversal in special functions */
    if (!(FUNDEF_ISLACFUN (fundef))) {
        arg_info = MakeInfo ();

        TRAVpush (TR_cf);
        fundef = TRAVdo (fundef, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (fundef);
}
