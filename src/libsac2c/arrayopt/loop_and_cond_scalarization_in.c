/**
 *
 * @file loop_and_cond_scalarization_in.c
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
 * 2. LACS is applied to ALL small  vectors.
 *    This should benefit certain codes that operate on such
 *    arrays, such as the complex-arithmetic version of Mandelbrot,
 *    which works with complex numbers represented as two-element vectors
 *    of doubles.
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
 *      offset = scalars2offset( iv,
 *             shp0, shp1,...shp(N-1),
 *             iv0,  iv1,...,iv(N-1));
 *
 *    The only reason we need iv around is so that optimizations
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
 *  - LFUscalarizeArray
 * All these directly modify the formal/actual parameters of the Do-fun and
 * its two calls. Furthermore, AdjustLoopSignature inserts its vardecs and
 * assignments via INFO_FUNDEF. Similarily, AdjustRecursiveCall uses
 * INFO_FUNDEF and INFO_PRECONDASSIGN do directly insert the generated
 * vardecs and assignments, respectively.
 *
 * Although LFUscalarizeArray creates code very similar to that of
 * AdjustRecursiveCall, it does not insert the code directly but stores
 * the vardecs and assignments in INFO_VARDECS and in INFO_PREASSIGNS.
 * While the vardecs are inserted in LACSap (utilizing INFO_FUNDEF again -
 * now pointing to the N_fundef of the external function), the assignments
 * are inserted in LACSassign (which is traversed bottom up in order to
 * avoid a superfluous traversal of the freshly generated assignments).
 *
 */

#define DBUG_PREFIX "LACSI"
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
    node *newlacfunargs;
    node *preassignslacfun;
    size_t argnum;
    bool inap;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_RECCALL(n) ((n)->reccall)
#define INFO_EXTARGS(n) ((n)->extargs)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_AP(n) ((n)->ap)
#define INFO_NEWLACFUNARGS(n) ((n)->newlacfunargs)
#define INFO_PREASSIGNSLACFUN(n) ((n)->preassignslacfun)
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
    INFO_NEWLACFUNARGS (result) = NULL;
    INFO_PREASSIGNSLACFUN (result) = NULL;
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
 * @fn bool LACSIargHasAvisScalars( int argnum, node *ap))
 *
 * @brief Predicate for determining if argnumTH function call
 *        argument has already been scalarized.
 *
 *****************************************************************************/
static bool
LACSIargHasAvisScalars (size_t argnum, node *ap)
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
    new_args
      = (node *)COcreateAllIndicesAndFold (shp, CreateArg, NULL, scalar_type, FALSE);

    DBUG_RETURN (new_args);
}

/** <!--*******************************************************************-->
 *
 * @fn node *ScalarizeArguments( node *arg_node, info *arg_info)
 *
 * @brief: Scalarize the arguments in this N_ap call.
 *
 * @return: Updated N_ap, with side effects on the called LACFUN.
 *
 *****************************************************************************/
static node *
ScalarizeArguments (node *arg_node, info *arg_info)
{
    node *lacfundef;
    node *nass;
    node *newass;

    DBUG_ENTER ();

    lacfundef = AP_FUNDEF (arg_node);

    // Extend the external call argument list.
    if (NULL != INFO_EXTARGS (arg_info)) {
        AP_ARGS (arg_node) = TCappendExprs (INFO_EXTARGS (arg_info), AP_ARGS (arg_node));
        INFO_EXTARGS (arg_info) = NULL;
    }

    // Extend the recursive call argument list.
    if (FUNDEF_ISLOOPFUN (lacfundef)) {
        FUNDEF_LOOPRECURSIVEAP (lacfundef)
          = LET_EXPR (ASSIGN_STMT (LFUfindRecursiveCallAssign (lacfundef)));
    }

    if (NULL != INFO_RECCALL (arg_info)) {
        AP_ARGS (FUNDEF_LOOPRECURSIVEAP (lacfundef))
          = TCappendExprs (INFO_RECCALL (arg_info),
                           AP_ARGS (FUNDEF_LOOPRECURSIVEAP (lacfundef)));
        INFO_RECCALL (arg_info) = NULL;
    }

    // Prefix the new lacfun arguments
    if (NULL != INFO_NEWLACFUNARGS (arg_info)) {
        FUNDEF_ARGS (lacfundef)
          = TCappendArgs (INFO_NEWLACFUNARGS (arg_info), FUNDEF_ARGS (lacfundef));
        INFO_NEWLACFUNARGS (arg_info) = NULL;
    }

    // Insert preassigns for recursive call before N_cond
    if (NULL != INFO_PREASSIGNSLACFUN (arg_info)) {
        nass = LFUfindAssignBeforeCond (lacfundef);
        newass = ASSIGN_NEXT (nass);
        ASSIGN_NEXT (nass) = INFO_PREASSIGNSLACFUN (arg_info);
        nass = TCappendAssign (nass, newass);
        INFO_PREASSIGNSLACFUN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LACSImodule(node *arg_node, info *arg_info)
 *
 * description:
 *   prunes the syntax tree by only going into function defintions
 *
 *****************************************************************************/
node *
LACSImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSIfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LACSIfundef (node *arg_node, info *arg_info)
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
 * @fn node *LACSIid( node *arg_node, info *arg_info)
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
LACSIid (node *arg_node, info *arg_info)
{
    shape *shp;
    shape *ravelshp;
    node *newexprs;
    node *newlacfunexprs = NULL;
    node *rca;
    node *lacfundef;
    node *newargs;
    node *avis;
    ntype *scalar_type;
    node *arg;
    node *recursivearg = NULL;
    node *exprs;
    int len;

    DBUG_ENTER ();

    if (NULL != INFO_AP (arg_info)) { // Do nothing if not called from LACSap
        avis = ID_AVIS (arg_node);
        DBUG_PRINT ("inspecting call value: %s", AVIS_NAME (avis));

        lacfundef = (NULL != INFO_AP (arg_info)) ? AP_FUNDEF (INFO_AP (arg_info)) : NULL;
        rca = (NULL != lacfundef) ? FUNDEF_LOOPRECURSIVEAP (lacfundef) : NULL;
        rca = (NULL != rca) ? AP_ARGS (rca) : NULL;
        arg = TCgetNthArg (INFO_ARGNUM (arg_info), FUNDEF_ARGS (lacfundef));

        /* Does this LACFUN argument meet our criteria for LACS? */
        if (TUshapeKnown (AVIS_TYPE (avis))
            && (!LACSIargHasAvisScalars (INFO_ARGNUM (arg_info), INFO_AP (arg_info))) &&
            // checking outside call here is fruitless. We
            // must check loop-invariance, too. ( !TYisAKV( AVIS_TYPE( avis)))   &&
            (TYgetDim (AVIS_TYPE (avis)) > 0)) {

            shp = TYgetShape (AVIS_TYPE (avis));
            ravelshp = SHcreateShape (1, SHgetUnrLen (shp));
            len = SHgetUnrLen (shp);
            if ((len > 0) && (len <= global.minarray)) {
                DBUG_PRINT ("Scalarizing lacfun arg: %s", AVIS_NAME (ARG_AVIS (arg)));
                global.optcounters.lacsi_expr += 1;

                /**
                 * First we create new external function call arguments and the required
                 * selections from the N_id. New vardecs and assigns will be
                 * dealt with in LACSfundef and LACSassign, respectively.
                 * INFO_EXTARGS will be handled when we return to LACSap.
                 * That will complete changes to the calling function.
                 *
                 * NB. We use the ravel of shp.
                 *
                 */

                DBUG_ASSERT (0 != SHgetDim (shp), "Why scalarize a scalar?");
                newexprs = LFUscalarizeArray (avis, &INFO_PREASSIGNS (arg_info),
                                              &INFO_VARDECS (arg_info), ravelshp);

                if (FUNDEF_ISLOOPFUN (lacfundef)) {
                    recursivearg
                      = TCgetNthExprs (INFO_ARGNUM (arg_info),
                                       AP_ARGS (FUNDEF_LOOPRECURSIVEAP (lacfundef)));
                    newlacfunexprs
                      = LFUscalarizeArray (ID_AVIS (EXPRS_EXPR (recursivearg)),
                                           &INFO_PREASSIGNSLACFUN (arg_info),
                                           &FUNDEF_VARDECS (lacfundef), ravelshp);
                }
                INFO_EXTARGS (arg_info)
                  = TCappendExprs (INFO_EXTARGS (arg_info), newexprs);

                // Now, extend the lacfun signature. First, the new scalar arguments.
                newargs = ExtendLacfunSignature (arg_node, arg_info);

                // And now, the AVIS_SCALARS N_array for the corresponding
                // lacfun's N_arg entry.
                arg = TCgetNthArg (INFO_ARGNUM (arg_info), FUNDEF_ARGS (lacfundef));
                scalar_type = TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (avis))),
                                         SHcreateShape (0));
                exprs = TCcreateExprsFromArgs (newargs);
                exprs = TBmakeArray (scalar_type, SHcopyShape (shp), exprs);
                AVIS_SCALARS (ARG_AVIS (arg)) = exprs;

                /**
                 * Lastly, we prefix the recursive call to reflect the
                 * new arguments in the signature. We could do this now,
                 * but if we want to extend LACS to work on non-LIR variables,
                 * it will be easier to collect all the parameters and do
                 * it in LACSap.
                 */
                if (FUNDEF_ISLOOPFUN (lacfundef)) {
                    INFO_RECCALL (arg_info)
                      = TCappendExprs (INFO_RECCALL (arg_info), newlacfunexprs);
                }

                // We have to do the prefixing on the LACFUN argument list
                // after we handle all N_id nodes.
                INFO_NEWLACFUNARGS (arg_info)
                  = TCappendArgs (INFO_NEWLACFUNARGS (arg_info), newargs);

            } else {
                DBUG_PRINT ("not scalarized: %s", AVIS_NAME (ID_AVIS (arg_node)));
            }
        } else {
            DBUG_PRINT ("arg: %s - shape unknown or scalar",
                        AVIS_NAME (ID_AVIS ((arg_node))));
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSIassign( node *arg_node, info *arg_info)
 *
 * @note We do this part bottom-up, because that keeps the
 *       preassigns out of the way.
 *
 *****************************************************************************/
node *
LACSIassign (node *arg_node, info *arg_info)
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
 * @fn node *LACSIexprs( node *arg_node, info *arg_info)
 *
 * @brief We should be traversing the N_ap argument list here.
 *        We increment the argument index, then continue.
 *
 *****************************************************************************/
node *
LACSIexprs (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);

    if (NULL != INFO_AP (arg_info)) { // Do nothing if not called from LACSap
        INFO_ARGNUM (arg_info) = INFO_ARGNUM (arg_info) + 1;
    }

    EXPRS_NEXT (arg_node) = TRAVopt (EXPRS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSIap( node *arg_node, info *arg_info)
 *
 * INFO_ARGNUM keeps track of which AP_ARG element we are looking at.
 *
 *****************************************************************************/
node *
LACSIap (node *arg_node, info *arg_info)
{
    node *lacfundef;

    DBUG_ENTER ();

    lacfundef = AP_FUNDEF (arg_node);
    /* non-recursive LACFUN calls only */
    if (FUNDEF_ISLACFUN (lacfundef) && (lacfundef != INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("Found LACFUN: %s call from: %s", FUNDEF_NAME (lacfundef),
                    FUNDEF_NAME (INFO_FUNDEF (arg_info)));

        DBUG_ASSERT (NULL == INFO_EXTARGS (arg_info), "INFO_EXTARGS not NULL");
        DBUG_ASSERT (NULL == INFO_RECCALL (arg_info), "INFO_RECCALL not NULL");
        DBUG_ASSERT (NULL == INFO_NEWLACFUNARGS (arg_info),
                     "INFO_NEWLACFUNARGS not NULL");

        INFO_AP (arg_info) = arg_node;
        INFO_INAP (arg_info) = TRUE;
        INFO_ARGNUM (arg_info) = 0;
        AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

        arg_node = ScalarizeArguments (arg_node, arg_info);
        FUNDEF_RETURN (lacfundef) = LFUfindFundefReturn (lacfundef);
        INFO_INAP (arg_info) = FALSE;
        INFO_AP (arg_info) = NULL;
        INFO_ARGNUM (arg_info) = 0;
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSIprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LACSIprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *LACSIdoLoopAndCondScalarization( node *arg_node)
 *
 *   @brief This traversal eliminates arrays within loops and conditionals
 *          for one function or module.
 *   @param arg_node
 *   @return modified AST.
 *
 *****************************************************************************/
node *
LACSIdoLoopAndCondScalarization (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_lacsi);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
