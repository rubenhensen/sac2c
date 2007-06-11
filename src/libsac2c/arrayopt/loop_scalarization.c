/*
 *
 * $Id$
 */

/**
 *
 * @file loop_scalarization.c
 *
 * Loop scalarization (LS) is the successor of array elimination (AE).
 * It is based on the observation that most simple cases of AE are covered
 * by the other optimizations, most notably Constant Folding.
 * The only case left is the case where small arrays are passed through
 * and modified by loop functions, i.e., we have a situation of the form:
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
 * Since we generally do not want to modify the entire loop body,
 * we apply a simpler transformation and rely on other optimizations
 * to achieve our final goal.
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
 * CF transformes the loop above into:
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
 * So far, we apply the LS iff x is used as array argument in selections
 * within the loop body only. If it is used in any other argument
 * position LS is NOT applied!
 * One may wonder, whether this transformation could be extended
 * for slightly more general situations. For example, we may consider
 * a situation where x occurs as an argument of F_add_SxA_, such as:
 *
 * <pre>
 *   ...
 *   x = [2,3];
 *   ...
 *   do {
 *         x = _add_SxA_( 1, x);
 *      }
 *
 *   ... x ...
 *
 * </pre>
 *
 * In order to avoid the intermediate vectors x, we would need
 * to scalarize F_add_SxA_. Although this could be done, it seems
 * to us that
 * (a) this is a rare case
 * (b) this prf scalarization could be handled in a more general setting
 *     which would then enable LS as described above.
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
 *         i = _add_SxA_( 1, i);
 *         a[7,3] = a[i];
 *      }
 *
 *   ... i,a ...
 *
 * </pre>
 *
 * If we would apply an extended version of LS here, we would
 * obtain (after the cycle):
 *
 * <pre>
 *   ...
 *   i = [2,3];
 *   i_0 = 2;
5B
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
 */

#include "dbug.h"

#include "types.h"
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

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    bool entry;
    node *reccall;
    node *extcall;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_ENTRY(n) (n->entry)
#define INFO_EXTCALL(n) (n->extcall)
#define INFO_RECCALL(n) (n->reccall)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_ENTRY (result) = TRUE;
    INFO_EXTCALL (result) = NULL;
    INFO_RECCALL (result) = NULL;

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

/** <!--*******************************************************************-->
 *
 * @fn node *AdjustLoopSignature( node *arg, shape * shp, info *arg_info)
 *
 * This function translates the non-scalar N_arg node "arg" into a
 * sequence of scalar N_arg nodes with fresh names whose lenth is
 * identical to the product of the shape "shp" and returns these.
 * Furthermore, an assignment of the form
 *    arg = _reshape_( shp, [sarg1, ...., sargn] );
 * is created and prepanded to the function body utilising INFO_FUNDEF.
 * A vardec for arg is being created and inserted into the function
 * body as well. This vardec reuses the N_avis of the arg given and the
 * arg given is being freed!
 *
 *****************************************************************************/
static void *CreateArg (constant *idx, void *accu, void *scalar_type);
static node *
AdjustLoopSignature (node *arg, shape *shp, info *arg_info)
{
    node *avis, *vardec;
    node *new_args, *old_args;
    node *assign;
    ntype *scalar_type;

    DBUG_ENTER ("AdjustLoopSignature");

    /**
     * replace arg by vardec:
     */
    avis = ARG_AVIS (arg);
    old_args = ARG_NEXT (arg);
    vardec = TBmakeVardec (avis, NULL);
    arg = FREEdoFreeNode (arg);

    /**
     * insert vardec:
     */

    /**
     * create the new arguments arg1, ...., argn
     */
    scalar_type
      = TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (avis))), SHcreateShape (0));
    new_args = (node *)COcreateAllIndicesAndFold (shp, CreateArg, NULL, scalar_type);
    scalar_type = TYfreeType (scalar_type);

    /**
     * create assignment
     *   arg = _reshape_( shp, [arg1, ...., argn] )
     */
    assign
      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                 TBmakeArray (TYcopyType (TYgetScalar (AVIS_TYPE (avis))),
                                              SHcopyShape (shp),
                                              TCmakeExprsFromArgs (new_args))),
                      NULL);
    AVIS_SSAASSIGN (avis) = assign;
    /**
     * and insert it:
     */

    DBUG_RETURN (TCappendArgs (new_args, old_args));
}

void *
CreateArg (constant *idx, void *accu, void *scalar_type)
{
    accu = TBmakeArg (TBmakeAvis (TRAVtmpVar (), TYcopyType ((ntype *)scalar_type)),
                      (node *)accu);

    return (accu);
}

/** <!--*******************************************************************-->
 *
 * @fn void AdjustRecursiveCall( node *recarg, shape * shp, info *arg_info)
 *
 *
 *****************************************************************************/
static void
AdjustRecursiveCall (node *recarg, shape *shp, info *arg_info)
{
    DBUG_ENTER ("AdjustRecursiveCall");
    DBUG_VOID_RETURN;
}

/** <!--*******************************************************************-->
 *
 * @fn void AdjustExternalCall( node *extarg, shape * shp, info *arg_info)
 *
 *
 *****************************************************************************/
static void
AdjustExternalCall (node *extarg, shape *shp, info *arg_info)
{
    DBUG_ENTER ("AdjustExternalCall");
    DBUG_VOID_RETURN;
}

/** <!--*******************************************************************-->
 *
 * @fn node *LSfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LSfundef (node *arg_node, info *arg_info)
{
    node *fundef;

    DBUG_ENTER ("LSfundef");

    if (!(INFO_ENTRY (arg_info)
          && (FUNDEF_ISDOFUN (arg_node) || FUNDEF_ISCONDFUN (arg_node)))) {
        if (FUNDEF_ISDOFUN (arg_node)) {
            fundef = INFO_FUNDEF (arg_info);
            INFO_FUNDEF (arg_info) = arg_node;
        } else {
            /**
             * although this should be redundant I put it here to make re-engineering
             * easier ;-))
             */
            INFO_FUNDEF (arg_info) = NULL;
        }

        /**
         *
         * First, we mark all AVISs that are not used as 2nd arg to F_Sel
         * as ISUSED:
         */
        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVcont (FUNDEF_BODY (arg_node), arg_info);
        }

        if (FUNDEF_ISDOFUN (arg_node)) {
            /**
             * We should now have both calls in INFO_EXTCALL and
             * INFO_RECCALL, respectively!
             * Therefore, we should be able to do our magic while traversing
             * the args!
             */
            if (FUNDEF_ARGS (arg_node) != NULL) {
                INFO_EXTCALL (arg_info) = AP_ARGS (INFO_EXTCALL (arg_info));
                INFO_RECCALL (arg_info) = AP_ARGS (INFO_RECCALL (arg_info));
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
            }
            INFO_EXTCALL (arg_info) = NULL;
            INFO_RECCALL (arg_info) = NULL;

            INFO_FUNDEF (arg_info) = fundef;
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LSarg( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LSarg (node *arg_node, info *arg_info)
{
    node *extarg, *recarg;
    shape *shp;

    DBUG_ENTER ("LSarg");

    extarg = INFO_EXTCALL (arg_info);
    recarg = INFO_RECCALL (arg_info);

    if (ARG_NEXT (arg_node) != NULL) {
        INFO_EXTCALL (arg_info) = ARG_NEXT (INFO_EXTCALL (arg_info));
        INFO_RECCALL (arg_info) = ARG_NEXT (INFO_RECCALL (arg_info));
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    if (TUshapeKnown (AVIS_TYPE (ARG_AVIS (arg_node)))) {
        shp = TYgetShape (AVIS_TYPE (ARG_AVIS (arg_node)));
        if ((SHgetUnrLen (shp) <= global.minarray)
            && !AVIS_ISUSED (ARG_AVIS (arg_node))) {
            shp = TYgetShape (AVIS_TYPE (ARG_AVIS (arg_node)));
            /**
             * First we create new arguments and we insert the array construction
             * at the beginning of the function body:
             */
            arg_node = AdjustLoopSignature (arg_node, shp, arg_info);
            /**
             * Then, we modify the recursive call to reflect the change
             * in the signature:
             */
            AdjustRecursiveCall (recarg, shp, arg_info);
            /**
             * Eventually, we compute INFO_EXTVARDECS and INFO_EXTASSIGNS
             * for later insertion into the calling context:
             */
            AdjustExternalCall (extarg, shp, arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LSap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LSap (node *arg_node, info *arg_info)
{
    node *external;

    DBUG_ENTER ("LSap");

    if (AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info)) {
        INFO_RECCALL (arg_info) = arg_node;
    } else {
        if (AP_ARGS (arg_node) != NULL) {
            AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
        }
        if (FUNDEF_ISDOFUN (AP_FUNDEF (arg_node))
            || FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
            INFO_ENTRY (arg_info) = FALSE;
            external = INFO_EXTCALL (arg_info);
            INFO_EXTCALL (arg_info) = arg_node;

            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            INFO_EXTCALL (arg_info) = external;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LSprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LSprf (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("LSprf");

    if (PRF_PRF (arg_node) == F_sel) {
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LSid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LSid (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("LSid");

    AVIS_ISUSED (ID_AVIS (arg_node)) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *LSdoLoopScalarization( node *syntax_tree)
 *
 *   @brief call this function for eliminating arrays within loops
 *   @param part of the AST (usually the entire tree) IVE is to be applied on.
 *   @return modified AST.
 *
 *****************************************************************************/
node *
LSdoLoopScalarization (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("LSdoLoopScalarization");

    DBUG_PRINT ("OPT", ("Starting Loop Scalarization ...."));

    TRAVpush (TR_ls);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);

    info = FreeInfo (info);
    TRAVpop ();

    DBUG_PRINT ("OPT", ("Loop Scalarization done!"));

    DBUG_RETURN (syntax_tree);
}
