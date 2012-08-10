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
 * However, this - in general - may leed to problems.
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
 *  This optimization treats the LaC funs as if they were inline!
 *  To enable that, use INFO_LEVEL which reflects the current level
 *  of the function. Thus we can avoid traversing top-level LaC-funs.
 *
 *  The overall idea is as follows:
 *  During the traversal of any Do-Fun, we collect pointers to places
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
 * Furthermore, we tag all N_avis nodes that are used in other positions than
 * the second arg of F_sel_VxA_VxA_ as AVIS_ISUSED (see LACSid / LACSprf). Thus,
 * all arguments that are used within selections only are exactly those
 * that have NOT been tagged;-)
 *
 * As can be seen in LACSarg, we have extraced the actual code modifications
 * by means of three local functions:
 *  - AdjustLoopSignature,
 *  - AdjustRecursiveCall, and
 *  - AdjustExternalCall
 * All these directly modify the formal/actual parameters of the Do-fun and
 * its two calls. Furthermore, AdjustLoopSignature inserts its vardecs and
 * assignments via INFO_FUNDEF. Similarily, AdjustRecursiveCall uses
 * INFO_FUNDEF and INFO_PRECONDASSIGN do directly insert the generated
 * vardecs and assignments, respectively.
 * Although AdjustExternalCall creates code very similar to that of
 * AdjustRecursiveCall, it does not insert the code directly but stores
 * the vardecs and assignments in INFO_EXTVARDECS and in INFO_EXTASSIGNS.
 * While the vardecs are inserted in LACSap (utilizing INFO_FUNDEF again -
 * now pointing to the N_fundef of the external function), the assignments
 * are inserted in LACSassign (which is traversed bottom up in order to
 * avoid a superfluous traversal of the freshly generated assignments).
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

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    int level;
    node *reccall;
    node *extcall;
    node *assigns;
    node *vardecs;
    node *lastassign;
    node *precond;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LEVEL(n) ((n)->level)
#define INFO_EXTCALL(n) ((n)->extcall)
#define INFO_EXTASSIGNS(n) ((n)->assigns)
#define INFO_EXTVARDECS(n) ((n)->vardecs)
#define INFO_RECCALL(n) ((n)->reccall)
#define INFO_LASTASSIGN(n) ((n)->lastassign)
#define INFO_PRECONDASSIGN(n) ((n)->precond)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LEVEL (result) = 0;
    INFO_EXTCALL (result) = NULL;
    INFO_EXTASSIGNS (result) = NULL;
    INFO_EXTVARDECS (result) = NULL;
    INFO_RECCALL (result) = NULL;
    INFO_LASTASSIGN (result) = NULL;
    INFO_PRECONDASSIGN (result) = NULL;

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
 * fold function for creating Vardecs.
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
    node *scal_avis, *array_avis;
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
 * @fn node *AdjustRecursiveCall( node *exprs, shape * shp, info *arg_info)
 *
 * This function replaces the non scalar recursive argument exprs_expr
 * by an exprs
 * chain of new scalar identifiers a1, ..., an of the same element type and
 * returns these. The topmost N-exprs of exprs is freed, its successors are
 * appended to the freshly created exprs chain!
 * Furthermore, it creates a sequence of assignments
 *    a1 = exprs[ 0*shp];
 *       ...
 *    an = exprs[ shp-1];
 * which are directly inserted in the body of the function using
 * INFO_PRECONDASSIGN.
 * The according vardecs are inserted within the body as well using INFO_FUNDEF.
 *
 *****************************************************************************/
static node *
AdjustRecursiveCall (node *exprs, shape *shp, info *arg_info)
{
    node *old_exprs, *new_exprs;
    node *avis;
    ntype *scalar_type;
    node *new_vardecs, *new_assigns;
    struct ca_info local_info;
    struct ca_info *local_info_ptr = &local_info;

    DBUG_ENTER ();

    /**
     * eliminate topmost exprs/expr:
     */
    avis = ID_AVIS (EXPRS_EXPR (exprs));
    old_exprs = EXPRS_NEXT (exprs);
    exprs = FREEdoFreeNode (exprs);

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
     * insert vardecs:
     */
    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TCappendVardec (new_vardecs, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

    /**
     * insert assignments:
     */

    ASSIGN_NEXT (INFO_PRECONDASSIGN (arg_info))
      = TCappendAssign (new_assigns, ASSIGN_NEXT (INFO_PRECONDASSIGN (arg_info)));

    DBUG_RETURN (TCappendExprs (new_exprs, old_exprs));
}

/** <!--*******************************************************************-->
 *
 * @fn node *AdjustLoopSignature( node *arg, shape * shp, info *arg_info)
 *
 * This function translates the non-scalar N_arg node "arg" into a
 * sequence of scalar N_arg nodes with fresh names whose length is
 * identical to the product of the shape "shp" and returns these.
 *
 * Furthermore, an assignment of the form
 *
 *    arg = _reshape_( shp, [sarg1, ...., sargn] );
 *
 * is created and prepended to the function body utilising INFO_FUNDEF.
 * A vardec for arg is created and inserted into the function
 * body as well. This vardec reuses the N_avis of the arg given and the
 * arg given is being freed!
 *
 * If the function is a CONDFUN, two args are generated,
 * one for each leg of the N_cond.
 *
 *****************************************************************************/
static node *
AdjustLoopSignature (node *arg, shape *shp, info *arg_info)
{
    node *avis, *vardec, *block;
    node *new_args, *old_args;
    node *assign;
    node *assignelse;
    ntype *scalar_type;
    node *thenelse;
    lut_t *lut;

    DBUG_ENTER ();

    /**
     * replace arg by vardec:
     */
    avis = ARG_AVIS (arg);
    old_args = ARG_NEXT (arg);
    vardec = TBmakeVardec (avis, NULL);
    DBUG_PRINT ("Created vardec: %s", AVIS_NAME (avis));

    ARG_AVIS (arg) = NULL;
    arg = FREEdoFreeNode (arg);

    /**
     * insert vardec:
     */
    block = FUNDEF_BODY (INFO_FUNDEF (arg_info));
    BLOCK_VARDECS (block) = TCappendVardec (BLOCK_VARDECS (block), vardec);

    /**
     * create the new arguments arg1, ...., argn
     */
    scalar_type
      = TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (avis))), SHcreateShape (0));
    new_args = (node *)COcreateAllIndicesAndFold (shp, CreateArg, NULL, scalar_type);

    /**
     * create assignment
     *   arg = _reshape_( shp, [arg1, ...., argn] )
     */
    assign = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                      TBmakeArray (scalar_type, SHcopyShape (shp),
                                                   TCcreateExprsFromArgs (new_args))),
                           NULL);
    AVIS_SSAASSIGN (avis) = assign;

    /*
     * and insert the new assign:
     */

    if (FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))) {
        /* LOOPFUN */
        BLOCK_ASSIGNS (block) = TCappendAssign (assign, BLOCK_ASSIGNS (block));
    } else {
        /* CONDFUN is harder */
        DBUG_ASSERT (FUNDEF_ISCONDFUN (INFO_FUNDEF (arg_info)), "Expected CONDFUN");

        thenelse = BLOCK_ASSIGNS (COND_THEN (ASSIGN_STMT (BLOCK_ASSIGNS (block))));
        BLOCK_ASSIGNS (COND_THEN (ASSIGN_STMT (BLOCK_ASSIGNS (block))))
          = TCappendAssign (assign, thenelse);

        /* This handily gives us a shiny new vardec and avis! */
        assignelse = DUPdoDupNodeSsa (assign, INFO_FUNDEF (arg_info));

        /* We have to rename uses of assignelse in the COND_ELSE block */
        lut = LUTgenerateLut ();
        LUTinsertIntoLutP (lut, IDS_AVIS (LET_IDS (ASSIGN_STMT (assign))),
                           IDS_AVIS (LET_IDS (ASSIGN_STMT (assignelse))));

        thenelse = BLOCK_ASSIGNS (COND_ELSE (ASSIGN_STMT (BLOCK_ASSIGNS (block))));
        BLOCK_ASSIGNS (COND_ELSE (ASSIGN_STMT (BLOCK_ASSIGNS (block))))
          = TCappendAssign (assignelse, thenelse);

        BLOCK_ASSIGNS (COND_ELSE (ASSIGN_STMT (BLOCK_ASSIGNS (block))))
          = DUPdoDupTreeLut (BLOCK_ASSIGNS (
                               COND_ELSE (ASSIGN_STMT (BLOCK_ASSIGNS (block)))),
                             lut);
        lut = LUTremoveLut (lut);
    }

    DBUG_RETURN (TCappendArgs (new_args, old_args));
}

/** <!--*******************************************************************-->
 *
 * @fn node * AdjustExternalCall( node *exprs, shape * shp, info *arg_info)
 *
 * This function replaces the non-scalar external argument exprs_expr
 * by an exprs chain of new scalar identifiers a1, ..., an
 * o the same element type and returns these.
 * The topmost N-exprs of exprs is freed, its successors are
 * appended to the freshly created exprs chain!
 * Furthermore, it creates a sequence of assignments
 *
 *    a1 = exprs[ 0*shp];
 *       ...
 *    an = exprs[ shp-1];
 *
 * which are stored in INFO_EXTASSIGNS( arg_info) for later insertion.
 * The according vardecs are stored in INFO_EXTVARDECS( arg_info)
 *
 *
 *****************************************************************************/
static node *
AdjustExternalCall (node *exprs, shape *shp, info *arg_info)
{
    node *old_exprs, *new_exprs;
    node *avis;
    ntype *scalar_type;
    node *new_vardecs, *new_assigns;
    struct ca_info local_info;
    struct ca_info *local_info_ptr = &local_info;

    DBUG_ENTER ();

    /**
     * eliminate topmost exprs/expr:
     */
    avis = ID_AVIS (EXPRS_EXPR (exprs));
    old_exprs = EXPRS_NEXT (exprs);
    exprs = FREEdoFreeNode (exprs);

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
    INFO_EXTVARDECS (arg_info) = TCappendVardec (new_vardecs, INFO_EXTVARDECS (arg_info));
    INFO_EXTASSIGNS (arg_info) = TCappendAssign (new_assigns, INFO_EXTASSIGNS (arg_info));

    DBUG_RETURN (TCappendExprs (new_exprs, old_exprs));
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
    node *fundef, *args;
    node *extap, *recap;

    DBUG_ENTER ();

    DBUG_PRINT ("Starting to traverse %s %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));
    if (!((INFO_LEVEL (arg_info) == 0)
          && (FUNDEF_ISLOOPFUN (arg_node) || FUNDEF_ISCONDFUN (arg_node)))) {

        fundef = INFO_FUNDEF (arg_info);
        INFO_FUNDEF (arg_info) = arg_node;

        /**
         *
         * First, we clear all AVIS_ISUSED for all arguments:
         * It is one of the mysteries of the universe that
         * a TRUE value of AVIS_ISUSED means: Do NOT attempt to
         * scalarize this array.
         */
        args = FUNDEF_ARGS (arg_node);
        while (args != NULL) {
            AVIS_ISUSED (ARG_AVIS (args)) = FALSE;
            args = ARG_NEXT (args);
        }

        /**
         * Then, mark all AVISs that are NOT used as 2nd arg to F_Sel
         * as ISUSED:
         */
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        if (FUNDEF_ISLACFUN (arg_node)) {
            /**
             * We should now have the external call to the LACFUN in INFO_EXTCALL
             * and should have the recursive call to a LOOPFUN in INFO_RECCALL.
             * Therefore, we should be able to do our magic while traversing
             * the LACFUN args.
             */
            if (FUNDEF_ARGS (arg_node) != NULL) {
                extap = INFO_EXTCALL (arg_info);
                recap = INFO_RECCALL (arg_info);
                INFO_EXTCALL (arg_info) = AP_ARGS (extap);
                INFO_RECCALL (arg_info) = (NULL != recap) ? AP_ARGS (recap) : NULL;
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
                AP_ARGS (extap) = INFO_EXTCALL (arg_info);
                if (NULL != recap) {
                    AP_ARGS (recap) = INFO_RECCALL (arg_info);
                }
            }
            INFO_EXTCALL (arg_info) = NULL;
            INFO_RECCALL (arg_info) = NULL;
        }

        INFO_FUNDEF (arg_info) = fundef;
    }
    DBUG_PRINT ("leaving function %s", FUNDEF_NAME (arg_node));

    if (INFO_LEVEL (arg_info) == 0) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSarg( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @note Conditionals on INFO_RECCALL are because of LOOPFUN vs. CONDFUN
 *
 *****************************************************************************/
node *
LACSarg (node *arg_node, info *arg_info)
{
    node *mem_extcall, *mem_reccall;
    shape *shp;

    DBUG_ENTER ();

    if (ARG_NEXT (arg_node) != NULL) {
        mem_extcall = INFO_EXTCALL (arg_info);
        mem_reccall = INFO_RECCALL (arg_info);
        INFO_EXTCALL (arg_info) = EXPRS_NEXT (INFO_EXTCALL (arg_info));
        if (NULL != INFO_RECCALL (arg_info)) {
            INFO_RECCALL (arg_info) = EXPRS_NEXT (INFO_RECCALL (arg_info));
        }

        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);

        EXPRS_NEXT (mem_extcall) = INFO_EXTCALL (arg_info);
        if (NULL != mem_reccall) {
            EXPRS_NEXT (mem_reccall) = INFO_RECCALL (arg_info);
        }
        INFO_EXTCALL (arg_info) = mem_extcall;
        INFO_RECCALL (arg_info) = mem_reccall;
    }

    DBUG_PRINT ("inspecting arg: %s", ARG_NAME (arg_node));

    /* Traverse AVIS_SHAPE, etc., to mark extrema and SAA info as AVIS_ISUSED */
    ARG_AVIS (arg_node) = TRAVdo (ARG_AVIS (arg_node), arg_info);

    if (TUshapeKnown (AVIS_TYPE (ARG_AVIS (arg_node)))
        && (TYgetDim (AVIS_TYPE (ARG_AVIS (arg_node))) > 0)) {
        shp = SHcopyShape (TYgetShape (AVIS_TYPE (ARG_AVIS (arg_node))));
        if ((SHgetUnrLen (shp) <= global.minarray)
            && !AVIS_ISUSED (ARG_AVIS (arg_node))) {
            DBUG_PRINT ("replacing arg: %s!", ARG_NAME (arg_node));
            global.optcounters.lacs_expr += 1;
            /**
             * First we create new arguments and we insert the array construction
             * at the beginning of the function body:
             */
            arg_node = AdjustLoopSignature (arg_node, shp, arg_info);
            /**
             * Then, we modify the recursive call to reflect the change
             * in the signature:
             */
            if (FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))) {
                INFO_RECCALL (arg_info)
                  = AdjustRecursiveCall (INFO_RECCALL (arg_info), shp, arg_info);
            }
            /**
             * Eventually, we compute INFO_EXTVARDECS and INFO_EXTASSIGNS
             * for later insertion into the calling context:
             */
            INFO_EXTCALL (arg_info)
              = AdjustExternalCall (INFO_EXTCALL (arg_info), shp, arg_info);
        } else {
            DBUG_PRINT ("  %s!",
                        AVIS_ISUSED (ARG_AVIS (arg_node)) ? "vector use!" : "too large!");
        }
        shp = SHfreeShape (shp);
    } else {
        DBUG_PRINT ("arg: %s - shape unknown or scalar", ARG_NAME (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LACSassign (node *arg_node, info *arg_info)
{
    node *lastassign = NULL;
    node *precondassign;

    DBUG_ENTER ();

    if (ASSIGN_NEXT (arg_node) != NULL) {
        lastassign = INFO_LASTASSIGN (arg_info);
        INFO_LASTASSIGN (arg_info) = arg_node;

        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);

        INFO_LASTASSIGN (arg_info) = lastassign;
    }

    if (NODE_TYPE (ASSIGN_STMT (arg_node)) == N_cond) {
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
        INFO_PRECONDASSIGN (arg_info) = lastassign;
    } else {
        precondassign = INFO_PRECONDASSIGN (arg_info);
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
        INFO_PRECONDASSIGN (arg_info) = precondassign;
    }

    if (INFO_EXTASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_EXTASSIGNS (arg_info), arg_node);
        INFO_EXTASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LACSap (node *arg_node, info *arg_info)
{
    node *external, *recursive;

    DBUG_ENTER ();

    if ((AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info))
        && FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))) {
        INFO_RECCALL (arg_info) = arg_node;
    } else {
        AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

        if (FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node))
            || FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
            INFO_LEVEL (arg_info)++;
            external = INFO_EXTCALL (arg_info);
            recursive = INFO_RECCALL (arg_info);
            INFO_EXTCALL (arg_info) = arg_node;

            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            if (INFO_EXTVARDECS (arg_info) != NULL) {
                FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                  = TCappendVardec (INFO_EXTVARDECS (arg_info),
                                    FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));
                INFO_EXTVARDECS (arg_info) = NULL;
            }

            INFO_EXTCALL (arg_info) = external;
            INFO_RECCALL (arg_info) = recursive;
            INFO_LEVEL (arg_info)--;
        }
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

    DBUG_ENTER ();

    if ((PRF_PRF (arg_node) == F_sel_VxA) || (PRF_PRF (arg_node) == F_idx_sel)) {
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSid( node *arg_node, info *arg_info)
 *
 * Basically, we scalarize just about everything now.
 *
 *****************************************************************************/
node *
LACSid (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    //  DBUG_PRINT ( "%s marked as scalarizable!", ID_NAME( arg_node));
    // AVIS_ISUSED( ID_AVIS( arg_node)) = TRUE;
    //  DBUG_PRINT ( "%s marked as non-scalarizable", ID_NAME( arg_node));

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
