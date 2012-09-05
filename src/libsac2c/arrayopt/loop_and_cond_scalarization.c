/*
 *
 * $Id$
 */

/**
 *
 * @file loop_and_cond_scalarization.c
 *
 * Loop and cond scalarization (LACS) is the successor of
 * array elimination (AE) and loop scalarization (LS).
 *
 * LACS extends LS to operate on CONDFUNs as well as on LOOPFUNS.
 *
 * This implementation may invalidate a lot of what follows here
 * in the way of motivation and documentation. This is the way
 * LACS works now:
 *
 * We scalarize ALL small array arguments, regardless of type. This may
 * be overkill, but we shall see. Some optimizations, such as AWLFI,
 * want to perform algebra on array shape elements. For example,
 * Livermore Loop 09 is a hand-unrolled sparse vector-matrix multiply.
 * The SAC abstract version of the code actually uses a sparse
 * matrix multiply (not unrolled), but it still has to determine
 * that shape(vec)[0] == shape(mat)[0]. This is trivial if
 * the shapes are scalarized, and not possible otherwise.
 *
 * Our implementation is as follows:
 *
 *   narr = [ s0, s1, s2];
 *   z = lacfun( narr);
 *
 * and the lacfun header is:
 *
 *   int lacfun( int[3] nid)
 *
 *
 * We rewrite this as:
 *
 *   z = lacfun( s0, s1, s2, narr);
 *
 * and the header as:
 *
 *   int lacfun( int s0, int s1, int s2, int[3] nid { scalars: [ s0, s1, s2]}
 *
 * where AVIS_SCALARS is an N_array of scalarized values, similar to
 * AVIS_SHAPE.
 *
 * Other optimizations can make use of AVIS_SCALARS as follows.
 * Assume we have this code in the LACFUN;
 *
 *  planes = _sel_VxA_( [0], nid);
 *
 * CF can replace this, in exactly the same way as if nid was an N_array:
 *
 *  planes = s0;
 *
 * If PMarray is extended to detect AVIS_SCALARS, then many
 * optimizations will be able to take advantage of LACS with NO
 * code changes.
 *
 * ----------- all comments below here need review ----------FIXME
 *
 * LS and LACS are based on the observation that most simple cases
 * of AE are covered by other optimizations, most notably Constant Folding.
 * The only case left (almost) is the case where small arrays are passed
 * into a LACFUN and are modified by loop or cond functions,
 * i.e., we have a situation of the form:
 *
 * <pre>
 *   ...
 *   x = foo();
 *   ...
 *   do {
 *         x = [ op_0( x[[0]]), ..., op_n( x[[n]])];
 *      }
 *
 *   ... x ...
 *
 * </pre>
 *
 * Since x is not used as a vector apart from elementary selections on it,
 * we strive to avoid the repeated creation of intermediate vectors that
 * are taken apart within the next iteration.
 * The overall goal is to achieve a code of the form:
 *
 * <pre>
 *   ...
 *   x = foo();
 *   x_0 = x[[0]];
 *   ...
 *   x_n = x[[n]];
 *   ...
 *   do{
 *       x_0 = op_0( x_0);
 *         ...
 *       x_n = op_n( x_n);
 *     }
 *   x = [x_0, ..., x_n];
 *   ... x ...
 *
 * </pre>
 *
 * Since we generally do not want to modify the entire LACFUN body,
 * we apply a simpler transformation and rely on other optimizations
 * to achieve our final goal.
 *
 * More precisely, we generate the following code:
 *
 * <pre>
 *   ...
 *   x = foo();
 *   x_0 = x[[0]];        ** new! **
 *   ...                  ** new! **
 *   x_n = x[[n]];        ** new! **
 *   ...
 *   do{
 *       x = [x_0, ..., x_n]; ** new! **
 *
 *       x = [ op_0( x[[0]]), ..., op_n( x[[n]])];
 *
 *       x_0 = x[[0]];        ** new! **
 *       ...                  ** new! **
 *       x_n = x[[n]];        ** new! **
 *     }
 *   ... x ...
 *
 * </pre>
 *
 * This transformation suffices iff Constant Folding (CF) and
 * Loop Invariant Removal (LIR) are turned on.
 * CF transforms the loop above into:
 *
 * <pre>
 *   ...
 *   x = foo();
 *   x_0 = x[[0]];
 *   ...
 *   x_n = x[[n]];
 *   ...
 *   do{
 *       tmp_0 = op_0( x_0);
 *         ...
 *       tmp_n = op_n( x_n);
 *       x = [ tmp_0, .., tmp_n];
 *       x_0 = tmp_0;
 *         ...
 *       x_n = tmp_n;
 *     }
 *   ... x ...
 *
 * </pre>
 *
 * Now, x is (*hope*) moved behind the loop by LIR as it is not referenced
 * within the loop anymore. This leads to:
 *
 * <pre>
 *   ...
 *   x = foo();
 *   x_0 = x[[0]];
 *   ...
 *   x_n = x[[n]];
 *   ...
 *   do{
 *       tmp_0 = op_0( x_0);
 *         ...
 *       tmp_n = op_n( x_n);
 *       x_0 = tmp_0;
 *         ...
 *       x_n = tmp_n;
 *     }
 *   x = [ tmp_0, .., tmp_n];
 *   ... x ...
 *
 * </pre>
 *
 * which is equivalent to our goal!
 *
 *
 * Some remarks on the extent of the optimization:
 * -----------------------------------------------
 *
 * 1. The optimization now also applies to CONDFUNs, as well as
 *    to LOOPFUNs. There are two rationales for this. Most
 *    important is that AWLF almost always requires access
 *    to index vectors, WL generators, and extrema as N_array
 *    nodes. Also, LOOPFUNs called from CONDFUNs would not
 *    otherwise benefit from the above transformations if
 *    the small array x is passed from the CONDFUN caller
 *    to the LOOPFUN.
 *
 * 2. LACS is applied to ALL small integer vectors. If this
 *    works out OK from a performance standpoint, then we
 *    can trivially extend LS to operate on all small arrays.
 *    This should benefit certain codes that operate on such
 *    arrays, such as the complex-arithmetic version of Mandelbrot,
 *    which works with two-element sets of doubles.
 *
 * 3. Implementation of the extension in (2) should be accompanied
 *    by a similar extension of PFRUNR, to unroll all (most?) small array
 *    operations.
 *
 * 4. The IVE stuff noted below does present a potential problem,
 *    but I have not done any analysis of that area.
 *    A scalarized version of vect2offset is certainly required,
 *    with semantics perhaps along these lines:
 *
 *      offset = scalars2offset( iv, N,
 *             shp0, shp1,...shp(N-1),
 *             iv0,  iv1,...,iv(N-1));
 *
 * 5. The only reason we need IV around is so that optimizations
 *    such as AWLF can trace back the origin of the indices to
 *    a WL generator, etc. A post-optimization traversal
 *    would convert replace the scalars2offset by an equivalent
 *    primitive that lacks IV, OR perhaps even use a sacprelude
 *    loop to do the work, assuming that LUR will eliminate
 *    the index operations on the shape vector and index vector.
 *
 * FIXME: Extensions now being made invalidate comments immediately
 * below here!
 *
 * One may wonder, whether this transformation could be extended
 * for slightly more general situations. For example, we may consider
 * a situation where x occurs as an argument of F_add_SxV_, such as:
 *
 * <pre>
 *   ...
 *   x = [2,3];
 *   ...
 *   do {
 *         x = _add_SxV_( 1, x);
 *      }
 *
 *   ... x ...
 *
 * </pre>
 *
 * In order to avoid the intermediate vectors x, we would need
 * to scalarize F_add_SxV_. Although this could be done, it seems
 * to us that
 * (a) this is a rare case
 * (b) this prf scalarization could be handled in a more general setting
 *     which would then enable LACS as described above.
 *
 * Another extension that could come to our mind would be to allow
 * occurrences of x in index vector positions since index vectors will
 * be eliminated by Index Vector Elimination (IVE) anyways.
 * However, this - in general - may lead to problems.
 * For example, consider the following scenario:
 *
 * <pre>
 *   ...
 *   i = [2,3];
 *   ...
 *   do {
 *         i = _add_SxV_( 1, i);
 *         a[7,3] = a[i];
 *      }
 *
 *   ... i,a ...
 *
 * </pre>
 *
 * If we would apply an extended version of LACS here, we would
 * obtain (after the cycle):
 *
 * <pre>
 *   ...
 *   i = [2,3];
 *   i_0 = 2;
 *   i_1 = 3;
 *   ...
 *   do {
 *         i_0 = i_0 + 1;
 *         i_1 = i_1 + 1;
 *         i = [ i_0, i_1];
 *         a[7,3] = a[i];
 *      }
 *
 *   ... i,a ...
 *
 * </pre>
 *
 * Since the increment of i has been scalarized, IVE cannot transform
 * i into a single scalar index anymore. As a net result, we will
 * obtain a superfluous multiplication and a superfluous addition.
 * Note here, that the drawback increases as the length of i increases.
 *
 *
 *
 * Implementation Strategy:
 * ------------------------
 *
 *  The overall idea is as follows:
 *  During the traversal of any LACFUN, we collect pointers to places
 *  which are subject to modification (if any) and we infer which arguments
 *  are used as array arguments in selections only.
 *  After traversing the body of a Do-fun, we traverse its arguments
 *  to see which ones are suitable candidates, i.e., which ones are used
 *  as arrays within selections only AND are AKS with element count
 *  less than or equal to "maxae", AKA "global.minarray" (default 4).
 *
 *  For each of these, we generate 3 "portions" of code:
 *   1) the new formal arguments for changing the signature and the
 *      array assignment in the beginning of the Do-fun,
 *   2) the new actual arguments for the recursive call and the scalarization
 *      of the array used in the recursive call, and
 *   3) the new actual arguments for the external call and the respective
 *      scalarization of the old argument.
 *  While all modifications/ extensions of the Do-fun itself are inserted
 *  directly after creation, the changes / extensions of the external
 *  call are done later, after leaving the Do-fun itself.
 *
 * Implementation Details:
 * -----------------------
 *
 * Prior to the modifications that happen in LACSarg while traversing Do-funs,
 * we have to collect the following information:
 *
 *   INFO_EXTCALL : the N_ap node of the external call to the current Do-fun
 *   INFO_RECCALL : the N_ap node of the recursive call to the current Do-fun
 *   INFO_FUNDEF  : the N_fundef of the current Do-fun
 *   INFO_PRECONDASSIGN : the N_assign node which precedes the N_assign that
 *                        hosts the N_cond of the current Do-fun
 *
 * As can be seen in LACSarg, we have extraced the actual code modifications
 * by means of three local functions:
 *  - AdjustLoopSignature,
 *  - AdjustRecursiveCall, and
 *  - ExtendExternalCall
 * All these directly modify the formal/actual parameters of the Do-fun and
 * its two calls. Furthermore, AdjustLoopSignature inserts its vardecs and
 * assignments via INFO_FUNDEF. Similarily, AdjustRecursiveCall uses
 * INFO_FUNDEF and INFO_PRECONDASSIGN do directly insert the generated
 * vardecs and assignments, respectively.
 *
 * Although ExtendExternalCall creates code very similar to that of
 * AdjustRecursiveCall, it does not insert the code directly but stores
 * the vardecs and assignments in INFO_VARDECS and in INFO_PREASSIGNS.
 * While the vardecs are inserted in LACSap (utilizing INFO_FUNDEF again -
 * now pointing to the N_fundef of the external function), the assignments
 * are inserted in LACSassign (which is traversed bottom up in order to
 * avoid a superfluous traversal of the freshly generated assignments).
 *
 *
 * We also make a final pass across the LACFUN loop signature,
 * replacing AVIS_SHAPE N_id nodes by their equivalent N_array
 * nodes. Strictly speaking, this only has to be done for CONDFUNs,
 * because of the need to preserve SSA behavior.
 *
 * Consider this example:
 *
 *   int condfun( int shp0, int shp1, int[.,.] mat { shape: shp}...
 *
 * We are * unable to write:
 *
 *   shp = [ shp0, shp1];
 *
 * and then use shp as an AVIS_SHAPE for mat, because the assignment
 * of the N_array has to appear in both legs of the cond, AND
 * they must have distinct names. So, that's nasty, but we CAN
 * just write:  AVIS_SHAPE( mat) = [ shp0, shp1];
 *
 */

#define DBUG_PREFIX "LACS"
#include "debug.h"

#include "types.h"
#include "DupTree.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "constants.h"
#include "globals.h"
#include "memory.h"
#include "shape.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "LookUpTable.h"
#include "pattern_match.h"
#include "lacfun_utilities.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *reccall;
    node *extargs;
    node *vardecs;
    node *preassigns;
    node *ap;
    int argnum;
    bool inap;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_RECCALL(n) ((n)->reccall)
#define INFO_EXTARGS(n) ((n)->extargs)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_AP(n) ((n)->ap)
#define INFO_ARGNUM(n) ((n)->argnum)
#define INFO_INAP(n) ((n)->inap)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_RECCALL (result) = NULL;
    INFO_EXTARGS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_AP (result) = NULL;
    INFO_ARGNUM (result) = 0;
    INFO_INAP (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

struct ca_info {
    node *exprs;
    node *avis;
    node *vardecs;
};

/** <!--*******************************************************************-->
 *
 * @fn void *CreateArg( constant *idx, void *accu, void *scalar_type)
 *
 * fold function for creating N_arg nodes
 *
 *****************************************************************************/
static void *
CreateArg (constant *idx, void *accu, void *scalar_type)
{
    accu = TBmakeArg (TBmakeAvis (TRAVtmpVar (), TYcopyType ((ntype *)scalar_type)),
                      (node *)accu);
    DBUG_PRINT ("Created arg: %s", ARG_NAME (accu));

    return (accu);
}

/** <!--*******************************************************************-->
 *
 * @fn bool LACShasAvisScalars( int argnum, node *ap))
 *
 * fold function for creating Vardecs.
 *****************************************************************************/
static bool
LACShasAvisScalars (int argnum, node *ap)
{
    node *arg;
    bool z;

    DBUG_ENTER ();

    arg = TCgetNthArg (argnum, FUNDEF_ARGS (AP_FUNDEF (ap)));
    z = (NULL != AVIS_SCALARS (ARG_AVIS (arg)));

    DBUG_RETURN (z);
}

/** <!--*******************************************************************-->
 *
 * @fn void *CreateVardecs( constant *idx, void *accu, void *scalar_type)
 *
 * fold function for creating Vardecs.
 *****************************************************************************/
static void *
CreateVardecs (constant *idx, void *accu, void *scalar_type)
{
    accu = TBmakeVardec (TBmakeAvis (TRAVtmpVar (), TYcopyType ((ntype *)scalar_type)),
                         (node *)accu);
    DBUG_PRINT ("Created vardec: %s", VARDEC_NAME (accu));

    return (accu);
}

/** <!--*******************************************************************-->
 *
 * @fn void *CreateAssigns( constant *idx, void *accu, void *local_info)
 *
 * fold function for creating assignment chains.
 *
 *****************************************************************************/
static void *
CreateAssigns (constant *idx, void *accu, void *local_info)
{
    node *scal_avis;
    node *array_avis;
    node *avis;
    struct ca_info *l_info;

    l_info = (struct ca_info *)local_info;

    scal_avis = ID_AVIS (EXPRS_EXPR (l_info->exprs));
    array_avis = l_info->avis;

    /**
     * create a temp variable to hold the index:
     */
    avis = TBmakeAvis (TRAVtmpVar (),
                       TYmakeAKV (TYmakeSimpleType (T_int), COcopyConstant (idx)));
    DBUG_PRINT ("Created avis: %s", AVIS_NAME (avis));
    l_info->vardecs = TBmakeVardec (avis, l_info->vardecs);

    /**
     * create the selection:
     */
    accu = TBmakeAssign (TBmakeLet (TBmakeIds (scal_avis, NULL),
                                    TCmakePrf2 (F_sel_VxA, TBmakeId (avis),
                                                TBmakeId (array_avis))),
                         (node *)accu);
    AVIS_SSAASSIGN (scal_avis) = (node *)accu;

    /**
     * create the assignment of the constant index:
     */
    accu = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), COconstant2AST (idx)),
                         (node *)accu);
    AVIS_SSAASSIGN (avis) = (node *)accu;

    l_info->exprs = EXPRS_NEXT (l_info->exprs);

    return (accu);
}

/** <!--*******************************************************************-->
 *
 * @fn node *GenerateNewRecursiveCallArguments( node *args)
 *
 * @brief: exprs is an N_arg chain of the new parameters for the lacfun.
 *         From it, we generate an N_exprs chain which will be used
 *         to prefix the recursive call to the lacfun.
 *
 *****************************************************************************/
static node *
GenerateNewRecursiveCallArguments (node *args)
{
    node *z = NULL;
    node *avis;
    node *expr;

    DBUG_ENTER ();

    while (NULL != args) {
        avis = ARG_AVIS (args);
        expr = TBmakeExprs (TBmakeId (avis), NULL);
        z = TCappendExprs (z, expr);
        args = ARG_NEXT (args);
    }

    DBUG_RETURN (z);
}

/** <!--*******************************************************************-->
 *
 * @fn node * ExtendExternalCall( node *avis, info *arg_info)
 *
 * This function generates an exprs chain of new scalar identifiers a1, ...,
 * and of the same element type as avis, and returns these.
 *
 *    a1 = exprs[ shp-shp];
 *       ...
 *    an = exprs[ shp-1];
 *
 * which are stored in INFO_PREASSIGNS( arg_info) for later insertion.
 * The according vardecs are stored in INFO_VARDECS( arg_info)
 *
 *****************************************************************************/
static node *
ExtendExternalCall (node *avis, info *arg_info)
{
    node *old_exprs;
    node *new_exprs;
    ntype *scalar_type;
    node *new_vardecs;
    node *new_assigns;
    struct ca_info local_info;
    struct ca_info *local_info_ptr = &local_info;
    shape *shp;

    DBUG_ENTER ();

    shp = SHcopyShape (TYgetShape (AVIS_TYPE (avis)));
    /**
     * create the vardecs:
     */
    scalar_type
      = TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (avis))), SHcreateShape (0));
    new_vardecs
      = (node *)COcreateAllIndicesAndFold (shp, CreateVardecs, NULL, scalar_type);

    /**
     * create the exprs:
     */
    new_exprs = TCcreateExprsFromVardecs (new_vardecs);

    /**
     * create the assignments:
     */
    local_info_ptr->exprs = new_exprs;
    local_info_ptr->avis = avis;
    local_info_ptr->vardecs = NULL;

    new_assigns
      = (node *)COcreateAllIndicesAndFold (shp, CreateAssigns, NULL, local_info_ptr);
    new_vardecs = TCappendVardec (new_vardecs, local_info_ptr->vardecs);

    /**
     * insert vardecs and assignments into arg_info:
     */
    INFO_VARDECS (arg_info) = TCappendVardec (new_vardecs, INFO_VARDECS (arg_info));
    INFO_PREASSIGNS (arg_info) = TCappendAssign (new_assigns, INFO_PREASSIGNS (arg_info));

    DBUG_RETURN (new_exprs);
}

/** <!--*******************************************************************-->
 *
 * @fn node *ExtendLacfunSignature( node *arg_node, info *arg_info)
 *
 * This function prefixes the lacfuns's signature with scalarized
 * elements of the N_id arg_node.
 *
 *****************************************************************************/
static node *
ExtendLacfunSignature (node *arg_node, info *arg_info)
{
    node *avis;
    node *vardecs;
    node *new_args;
    ntype *scalar_type;
    shape *shp;

    DBUG_ENTER ();

    avis = ID_AVIS (arg_node);
    shp = SHcopyShape (TYgetShape (AVIS_TYPE (avis)));

    /**
     * create the new arguments arg1, ...., argn
     */
    scalar_type
      = TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (avis))), SHcreateShape (0));
    new_args = (node *)COcreateAllIndicesAndFold (shp, CreateArg, NULL, scalar_type);

    DBUG_RETURN (new_args);
}

/******************************************************************************
 *
 * function: node *CorrectArgAvisShape( arg_node, arg_info)
 *
 * description: Examine FUNDEF_ARG, and replace any
 *  AVIS_SHAPE(arg) elements by their N_array equivalents, if arg is AKD.
 *
 * argument: N_fundef node.
 *
 * result: Updated fundef.
 *
 *****************************************************************************/
static node *
CorrectArgAvisShape (node *arg_node, info *arg_info)
{
    node *args;
    node *avis;
    pattern *pat;
    node *arr;

    DBUG_ENTER ();

    pat = PMarray (1, PMAgetNode (&arr), 1, PMskip (0));
    args = FUNDEF_ARGS (arg_node);
    while (args != NULL) {
        avis = ARG_AVIS (args);
        DBUG_PRINT ("Looking at %s", AVIS_NAME (avis));
        if ((NULL != AVIS_SHAPE (avis)) && (N_id == NODE_TYPE (AVIS_SHAPE (avis)))) {
            DBUG_PRINT ("Found AVIS_SHAPE N_id %s",
                        AVIS_NAME (ID_AVIS (AVIS_SHAPE (avis))));
            arr = NULL;
            if (PMmatchFlat (pat, AVIS_SHAPE (avis))) {
                /* AVIS_SHAPE is an N_array. Replace AVIS_SHAPE N_id by its value */
                DBUG_PRINT ("Replacing AVIS_SHAPE N_id %s",
                            AVIS_NAME (ID_AVIS (AVIS_SHAPE (avis))));
                AVIS_SHAPE (avis) = FREEdoFreeNode (AVIS_SHAPE (avis));
                AVIS_SHAPE (avis) = DUPdoDupNode (arr);
            } else {
                DBUG_PRINT ("AVIS_SHAPE not defined by N_array %s",
                            AVIS_NAME (ID_AVIS (AVIS_SHAPE (avis))));
            }
        }
        args = ARG_NEXT (args);
    }

    pat = PMfree (pat);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LACSmodule(node *arg_node, info *arg_info)
 *
 * description:
 *   prunes the syntax tree by only going into function defintions
 *
 *****************************************************************************/
node *
LACSmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LACSfundef (node *arg_node, info *arg_info)
{
    node *old_fundef;

    DBUG_ENTER ();

    DBUG_PRINT ("Starting to traverse %s %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));
    old_fundef = INFO_FUNDEF (arg_info);
    INFO_FUNDEF (arg_info) = arg_node;

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_RETURN (arg_node) = LFUfindFundefReturn (arg_node);
    if (NULL != INFO_VARDECS (arg_info)) {
        FUNDEF_VARDECS (arg_node)
          = TCappendVardec (INFO_VARDECS (arg_info), FUNDEF_VARDECS (arg_node));
        INFO_VARDECS (arg_info) = NULL;
    }

    INFO_FUNDEF (arg_info) = old_fundef;
    DBUG_PRINT ("leaving function %s", FUNDEF_NAME (arg_node));

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSid( node *arg_node, info *arg_info)
 *
 * @brief Most of the action happens here. We want to look
 *        at one of the external function call arguments in an N_ap.
 *        If we are not called from the N_ap node, do nothing.
 *
 * @note At present, we require the N_id to be loop-invariant.
 *       This restriction could be eased if the recursive call
 *       was preceded by selections on the N_id, as is done
 *       in the external call. However, I want to get this part
 *       working first.
 *
 *****************************************************************************/
node *
LACSid (node *arg_node, info *arg_info)
{
    shape *shp;
    node *newexprs;
    node *rca;
    node *lacfundef;
    node *newargs;
    node *avis;
    ntype *scalar_type;
    node *arg;

    DBUG_ENTER ();

    if (INFO_INAP (arg_info)) { // Do nothing if not called from LACSap
        avis = ID_AVIS (arg_node);
        DBUG_PRINT ("inspecting call value: %s", AVIS_NAME (avis));

        lacfundef = (NULL != INFO_AP (arg_info)) ? AP_FUNDEF (INFO_AP (arg_info)) : NULL;
        rca = (NULL != lacfundef) ? FUNDEF_LOOPRECURSIVEAP (lacfundef) : NULL;
        rca = (NULL != rca) ? AP_ARGS (rca) : NULL;
        arg = TCgetNthArg (INFO_ARGNUM (arg_info), FUNDEF_ARGS (lacfundef));

        /* Does this LACFUN argument meet our criteria for LACS? */
        if (TUshapeKnown (AVIS_TYPE (avis))
            && (!LACShasAvisScalars (INFO_ARGNUM (arg_info), INFO_AP (arg_info)))
            && (LFUisLoopFunInvariant (lacfundef, ARG_AVIS (arg), rca))
            && (TYgetDim (AVIS_TYPE (avis)) > 0)) {

            shp = SHcopyShape (TYgetShape (AVIS_TYPE (avis)));
            if ((SHgetUnrLen (shp) <= global.minarray)) {
                DBUG_PRINT ("replacing arg: %s!", AVIS_NAME (avis));
                global.optcounters.lacs_expr += 1;

                /**
                 * First we create new external function call arguments and the required
                 * selections from the N_id. New vardecs and assigns will be
                 * dealt with in LACSfundef and LACSassign, respectively.
                 * INFO_EXTARGS will be handled when we return to LACSap.
                 * That will complete changes to the calling function.
                 */
                newexprs = ExtendExternalCall (avis, arg_info);
                INFO_EXTARGS (arg_info)
                  = TCappendExprs (INFO_EXTARGS (arg_info), newexprs);

                // Now, extend the lacfun signature. First, the new scalar arguments.
                newargs = ExtendLacfunSignature (arg_node, arg_info);

                // And now, the AVIS_SCALARS N_array for the corresponding
                // lacfun's N_arg entry.
                arg = TCgetNthArg (INFO_ARGNUM (arg_info), FUNDEF_ARGS (lacfundef));
                scalar_type = TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (avis))),
                                         SHcreateShape (0));
                AVIS_SCALARS (ARG_AVIS (arg))
                  = TBmakeArray (scalar_type, SHcopyShape (shp),
                                 TCcreateExprsFromArgs (newargs));

                /**
                 * Lastly, we prefix the recursive call to reflect the
                 * new arguments in the signature. We could do this now,
                 * if we want to extend LACS to work on non-LIR variables,
                 * it will be easier to collect all the parameters and do
                 * it in LACSap.
                 */
                if (FUNDEF_ISLOOPFUN (lacfundef)) {
                    INFO_RECCALL (arg_info)
                      = TCappendExprs (INFO_RECCALL (arg_info),
                                       GenerateNewRecursiveCallArguments (newargs));
                }

                // We have to do the prefixing after we generate AVIS_SCALARS
                // and handle the recursive call.
                FUNDEF_ARGS (lacfundef) = TCappendArgs (newargs, FUNDEF_ARGS (lacfundef));

            } else {
                DBUG_PRINT ("not scalarized: %s", AVIS_NAME (ID_AVIS (arg_node)));
            }
            shp = SHfreeShape (shp);
        } else {
            DBUG_PRINT ("arg: %s - shape unknown or scalar",
                        AVIS_NAME (ID_AVIS ((arg_node))));
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSassign( node *arg_node, info *arg_info)
 *
 * @note We do this part bottom-up, because that keeps the
 *       preassigns out of the way.
 *
 *****************************************************************************/
node *
LACSassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (NULL != INFO_PREASSIGNS (arg_info)) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSarg( node *arg_node, info *arg_info)
 *
 * @brief We keep track of the argument index, then continue.
 *
 *****************************************************************************/
node *
LACSarg (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    INFO_ARGNUM (arg_info) = INFO_ARGNUM (arg_info) + 1;
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSap( node *arg_node, info *arg_info)
 *
 * INFO_ARGNUM keeps track of which AP_ARG element we are looking at.
 *
 *****************************************************************************/
node *
LACSap (node *arg_node, info *arg_info)
{
    node *lacfundef;
    node *reccall;

    DBUG_ENTER ();

    lacfundef = AP_FUNDEF (arg_node);
    /* non-recursive LACFUN calls only */
    if (FUNDEF_ISLACFUN (lacfundef) && (lacfundef != INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("Found LACFUN: %s call from: %s", FUNDEF_NAME (lacfundef),
                    FUNDEF_NAME (INFO_FUNDEF (arg_info)));

        DBUG_ASSERT (NULL == INFO_VARDECS (arg_info), "INFO_VARDECS not NULL");
        DBUG_ASSERT (NULL == INFO_EXTARGS (arg_info), "INFO_EXTARGS not NULL");
        DBUG_ASSERT (NULL == INFO_RECCALL (arg_info), "INFO_RECCALL not NULL");

        INFO_AP (arg_info) = arg_node;
        INFO_INAP (arg_info) = TRUE;
        INFO_ARGNUM (arg_info) = 0;
        AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
        INFO_INAP (arg_info) = FALSE;
        INFO_AP (arg_info) = NULL;
        INFO_ARGNUM (arg_info) = 0;

        // Extend the external call argument list.
        if (NULL != INFO_EXTARGS (arg_info)) {
            AP_ARGS (arg_node)
              = TCappendExprs (INFO_EXTARGS (arg_info), AP_ARGS (arg_node));
            INFO_EXTARGS (arg_info) = NULL;
        }

        // Extend the recursive call argument list.
        if (NULL != INFO_RECCALL (arg_info)) {
            AP_ARGS (FUNDEF_LOOPRECURSIVEAP (lacfundef))
              = TCappendExprs (INFO_RECCALL (arg_info),
                               AP_ARGS (FUNDEF_LOOPRECURSIVEAP (lacfundef)));
            INFO_RECCALL (arg_info) = NULL;
        }

        FUNDEF_RETURN (lacfundef) = LFUfindFundefReturn (lacfundef);
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LACSprf (node *arg_node, info *arg_info)
{
    int DEADCODE;

    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *LACSdoLoopAndCondScalarization( node *arg_node)
 *
 *   @brief This traversal eliminates arrays within loops and conditionals
 *          for one function or module.
 *   @param arg_node
 *   @return modified AST.
 *
 *****************************************************************************/
node *
LACSdoLoopAndCondScalarization (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_lacs);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
