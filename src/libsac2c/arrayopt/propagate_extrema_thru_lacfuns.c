/*
 * $Id$
 */

/**<!--******************************************************************-->
 *
 * @file propagate_extrema_thru_lacfuns.c
 *
 * This traversal performs two functions on LACFUN arguments
 * and results:
 *
 *  a. It propagates argument AVIS_MIN and AVIS_MAX values into a
 *     LACFUN, when the corresponding formal parameters within
 *     the LACFUN do not yet have those extrema.
 *
 *  b. It propagates extrema out of LACFUN results into the
 *     calling environment.
 *     Or will, eventually... It does not do so, at present.
 *
 *  The traversal does not concern itself with scalarization, duplicated
 *  arguments, etc.  These tasks are left to LS and EDFA.
 *
 * Overview of traversal:
 *
 *  We traverse all functions. Each invocation of a LACFUN is
 *  dealt with immediately, as follows:
 *
 *    0. Mark the LACFUN in INFO_LACFUN.
 *    1. Invoke a traversal of FUNDEF_LOCALFUNS, skipping
 *       any function except the LACFUN.
 *    2. Upon finding the LACFUN:
 *
 *       a. Find the argument elements that have extrema in the
 *          calling environment, but not in in the called
 *          environment.
 *
 *       b. Add new vardecs to the LACFUN, defining the new extrema names.
 *
 *       c. Add F_noteminval/F_notemaxval to the LACFUN, to attach
 *          the extrema to their arguments.
 *
 *       d. Rewrite the LACFUN header to include the new extrema.
 *
 *       e. Rewrite the LACFUN (Loopfun) recursive call.
 *
 *       f. As a side effect, return code to amend the
 *          LACFUN N_ap external call, to include the new extrema.
 *
 *    3. Upon return to the LACFUN caller, amend the LACFUN N_ap
 *       external call.
 *
 *    FIXME: At least for now, we restrict things to loop-invariant
 *    variables in LOOPFUNs, as that makes analysis easier.
 *    Eventually, we want to include extrema deduced by LUR on
 *    loop-carried variables.
 *
 * [FIXME - do same sort of thing on LACFUN results.]
 *
 *
 **************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "node_basic.h"
#include "new_types.h"

#define DBUG_PREFIX "PETL"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "free.h"
#include "globals.h"
#include "constants.h"
#include "shape.h"
#include "tree_compound.h"
#include "LookUpTable.h"
#include "DupTree.h"
#include "indexvectorutils.h"
#include "propagate_extrema_thru_lacfuns.h"
#include "eliminate_duplicate_fundef_args.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool onefundef;
    lut_t *lutrenames;
    node *newargs;
    node *newouterapargs;
    node *newrecursiveapargs;
    node *outerfunap;
    node *lacfun;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_ONEFUNDEF(n) (n->onefundef)
#define INFO_LUTRENAMES(n) (n->lutrenames)
#define INFO_NEWARGS(n) (n->newargs)
#define INFO_NEWOUTERAPARGS(n) (n->newouterapargs)
#define INFO_NEWRECURSIVEAPARGS(n) (n->newrecursiveapargs)
#define INFO_OUTERFUNAP(n) (n->outerfunap)
#define INFO_LACFUN(n) (n->lacfun)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();
    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_ONEFUNDEF (result) = TRUE;
    INFO_LUTRENAMES (result) = NULL;
    INFO_NEWARGS (result) = NULL;
    INFO_NEWOUTERAPARGS (result) = NULL;
    INFO_NEWRECURSIVEAPARGS (result) = NULL;
    INFO_OUTERFUNAP (result) = NULL;
    INFO_LACFUN (result) = NULL;

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
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/**<!--***********************************************************************-->
 *
 * @fn node *EnhanceLacfunHeader( node *arg_node, info *arg_info)
 *
 * @brief Add N_arg nodes for new extrema.
 *
 * @param N_fundef lacfun arg_node to be traversed
 *
 * @result: updated N_fundef node
 *
 * @implementation:
 *
 *        If we have:
 *
 *           z = Loop_1( AhasMin, B, ChasMinAndMax, DhasMax, A);
 *
 *        and:
 *
 *           int Loop_1( int A, int B, int C, int D, int E)
 *
 *        then we introduce extrema for A and D, giving us:
 *
 *           int Loop_1( Amin, Cmin, Cmax, Dmax,
 *                       int A, int B, int C, int D, int E)
 *
 *        These new N_arg nodes are not added to the LACFUN until
 *        we have completed renaming of its calls.
 *
 *        At the same time, we build N_ap argument chains for
 *        the outer call to the LACFUN and the recursive (if any) call.
 *        They are dealt with later on.
 *
 ******************************************************************************/
static node *
EnhanceLacfunHeader (node *arg_node, info *arg_info)
{
    node *apargs;
    node *lacfunargs;
    node *newavis;
    node *callarg;
    node *minmax;
    node *argavis;
    node *rca;
    ntype *typ;
    node *outercall;
    node *reccall;

    DBUG_ENTER ();
    DBUG_PRINT ("Enhancing LACFUN %s header", FUNDEF_NAME (arg_node));

    INFO_NEWARGS (arg_info) = NULL;
    INFO_NEWOUTERAPARGS (arg_info) = NULL;
    INFO_NEWRECURSIVEAPARGS (arg_info) = NULL;

    apargs = AP_ARGS (INFO_OUTERFUNAP (arg_info)); /* outer call */
    lacfunargs = FUNDEF_ARGS (arg_node);           /* formal parameters */
    rca = FUNDEF_LOOPRECURSIVEAP (arg_node);       /* recursive call args */
    rca = (NULL != rca) ? AP_ARGS (rca) : NULL;

    /* Create avis and vardec nodes for imported extrema. */
    while (NULL != apargs) {
        callarg = EXPRS_EXPR (apargs);
        argavis = ID_AVIS (callarg);
        if (EDFAisLoopFunInvariant (arg_node, lacfunargs, rca)) {
            if ((NULL == AVIS_MIN (ARG_AVIS (lacfunargs)))
                && (NULL != AVIS_MIN (argavis))) {
                minmax = AVIS_MIN (ID_AVIS (callarg));
                typ = AVIS_TYPE (ID_AVIS (callarg));
                newavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (ID_AVIS (callarg))),
                                      TYcopyType (typ));
                AVIS_MIN (ARG_AVIS (lacfunargs)) = TBmakeId (newavis);
                DBUG_PRINT ("Adding AVIS_MIN(%s) for formal parameter %s",
                            AVIS_NAME (newavis), AVIS_NAME (ARG_AVIS (lacfunargs)));
                INFO_NEWARGS (arg_info) = TBmakeArg (newavis, INFO_NEWARGS (arg_info));
                outercall
                  = TBmakeExprs (TBmakeId (ID_AVIS (AVIS_MIN (ID_AVIS (callarg)))), NULL);
                INFO_NEWOUTERAPARGS (arg_info)
                  = TCappendExprs (INFO_NEWOUTERAPARGS (arg_info), outercall);
                reccall = TBmakeExprs (TBmakeId (newavis), NULL);
                INFO_NEWRECURSIVEAPARGS (arg_info)
                  = TCappendExprs (INFO_NEWRECURSIVEAPARGS (arg_info), reccall);
            }
            if ((NULL == AVIS_MAX (ARG_AVIS (lacfunargs)))
                && (NULL != AVIS_MAX (argavis))) {
                minmax = AVIS_MAX (ID_AVIS (callarg));
                typ = AVIS_TYPE (ID_AVIS (callarg));
                newavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (ID_AVIS (callarg))),
                                      TYcopyType (typ));
                AVIS_MAX (ARG_AVIS (lacfunargs)) = TBmakeId (newavis);
                DBUG_PRINT ("Adding AVIS_MAX(%s) for formal parameter %s",
                            AVIS_NAME (newavis), AVIS_NAME (ARG_AVIS (lacfunargs)));
                INFO_NEWARGS (arg_info) = TBmakeArg (newavis, INFO_NEWARGS (arg_info));
                outercall
                  = TBmakeExprs (TBmakeId (ID_AVIS (AVIS_MAX (ID_AVIS (callarg)))), NULL);
                INFO_NEWOUTERAPARGS (arg_info)
                  = TCappendExprs (INFO_NEWOUTERAPARGS (arg_info), outercall);
                reccall = TBmakeExprs (TBmakeId (newavis), NULL);
                INFO_NEWRECURSIVEAPARGS (arg_info)
                  = TCappendExprs (INFO_NEWRECURSIVEAPARGS (arg_info), reccall);
            }
        }
        apargs = EXPRS_NEXT (apargs);
        lacfunargs = ARG_NEXT (lacfunargs);
        rca = (NULL != rca) ? EXPRS_NEXT (rca) : NULL;
    }

    if ((NULL != INFO_NEWRECURSIVEAPARGS (arg_info)) && (FUNDEF_ISLOOPFUN (arg_node))) {
        DBUG_PRINT ("Replacing recursive call to LACFUN: %s", FUNDEF_NAME (arg_node));
        reccall = FUNDEF_LOOPRECURSIVEAP (arg_node);
        AP_ARGS (reccall)
          = TCappendExprs (INFO_NEWRECURSIVEAPARGS (arg_info), AP_ARGS (reccall));
        INFO_NEWRECURSIVEAPARGS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *EnhanceLacfunBody( node *arg_node, info *arg_info)
 *
 * @brief Add F_noteminval/maxval for affected arguments.
 *        Perform renames in code block for noteminval/maxval'd arguments.
 *
 * @param  arg_node: N_block in lacfun that will have notes
 *         prepended to it.
 *
 * @result: updated N_block node
 *
 * @implementation:
 *
 *        If we have:
 *
 *           z = Loop_1( AhasMin, B, ChasMinAndMax, DhasMax, A);
 *
 *        and:
 *
 *           int Loop_1( int A, int B, int C, int D, int E)
 *
 *        then we have already introduced extrema for A and D, giving us,
 *        essentially: (The extrema are not attached until we return
 *        from here, though.)
 *
 *           int Loop_1( Amin, Cmin, Cmax, Dmax,
 *                       int A, int B, int C, int D, int E)
 *
 *        Now, we add notes to attach the extrema to their owners:
 *           ...
 *             A'  = _noteminval( A,  Amin);
 *             C'  = _noteminval( C,  Cmin);
 *             C'' = _notemaxvam( C', Cmax);
 *             D'  = _notemaxval( D,  Dmax);
 *
 *        Placement of the above assigns is made slightly messier
 *        by the need to introduce noteminval/maxval in both legs of a CONDFUN,
 *        with different (SSA) lhs names.
 *
 ******************************************************************************/
static node *
EnhanceLacfunBody (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
#ifdef FIXME // to be coded
    node *apargs;
    node *lacfunargs;
    node *newargs = NULL;
    node *newavis;
    node *callarg;
    node *minmax;
    node *argavis;
    node *rca;
    ntype *typ;

    DBUG_PRINT ("Enhancing LACFUN %s body", FUNDEF_NAME (INFO_FUNDEF (arg_info)));
    DBUG_ASSERT (N_block == NODE_TYPE (arg_node), "Expected N_block");

    LUTremoveContentLut (INFO_LUTRENAMES (arg_info));
    apargs = AP_ARGS (INFO_OUTERFUNAP (arg_info));         /* outer call */
    lacfunargs = FUNDEF_ARGS (INFO_FUNDEF (arg_info));     /* lacfun formal parameters */
    rca = FUNDEF_LOOPRECURSIVEAP (INFO_FUNDEF (arg_info)); /* recursive call args */
    rca = (NULL != rca) ? AP_ARGS (rca) : NULL;

    /* Build new assigns */
    while (NULL != apargs) {
        callarg = EXPRS_EXPR (apargs);
        argavis = ID_AVIS (callarg);
        if (EDFAisLoopFunInvariant (INFO_FUNDEF (arg_info), lacfunargs, rca)) {
            if ((NULL == AVIS_MIN (ARG_AVIS (lacfunargs)))
                && (NULL != AVIS_MIN (argavis))) {
                minmax = AVIS_MIN (ID_AVIS (callarg));
                typ = AVIS_TYPE (ID_AVIS (callarg));
                newavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (ID_AVIS (callarg))),
                                      TYcopyType (typ));
                newargs = TBmakeArg (newavis, newargs);
            }
            if ((NULL == AVIS_MAX (ARG_AVIS (lacfunargs)))
                && (NULL != AVIS_MAX (argavis))) {
                minmax = AVIS_MAX (ID_AVIS (callarg));
                typ = AVIS_TYPE (ID_AVIS (callarg));
                newavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (ID_AVIS (callarg))),
                                      TYcopyType (typ));
                newargs = TBmakeArg (newavis, newargs);
            }
        }
        apargs = EXPRS_NEXT (apargs);
        lacfunargs = ARG_NEXT (lacfunargs);
    }
    /* Rename block references to old N_arg names */
    arg_node = DUPdoDupNodeLut (arg_node, INFO_LUTRENAMES (arg_info));

#endif //  FIXME // to be coded
    DBUG_RETURN (arg_node);
}
/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/**<!--***********************************************************************-->
 *
 * @fn node *PETLdoPropagateExtremaThruLacfuns(node *arg_node)
 *
 * @brief starting point of traversal
 *
 * @param N_fundef to be traversed
 *
 * @result new N_fundef chain with extrema now propagated in and
 *         out of any LACFUNs.
 *
 ******************************************************************************/
node *
PETLdoPropagateExtremaThruLacfuns (node *arg_node)
{
    info *arg_info;
    DBUG_ENTER ();

    arg_info = MakeInfo ();
    INFO_ONEFUNDEF (arg_info) = (N_fundef == NODE_TYPE (arg_node));

    INFO_LUTRENAMES (arg_info) = LUTgenerateLut ();

    TRAVpush (TR_petl);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    INFO_LUTRENAMES (arg_info) = LUTremoveLut (INFO_LUTRENAMES (arg_info));
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *PETLmodule(node *arg_node, info *arg_info)
 *
 * @brief traverses the functions of the module
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
PETLmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *PETLfundef(node *arg_node, info *arg_info)
 *
 * @brief If INFO_LACFUN is non-NULL, we are being invoked
 *        from an N_ap. If this is not the LACFUN, do nothing.
 *        Otherwise, operate on this LACFUN to add extrema.
 *
 *        If INFO_LACFUN is NULL, we are searching for N_ap
 *        calls, so just traverse this function.
 *
 * @param arg_node fundef to be traversed
 * @param arg_info
 *
 * @result Updated N_fundef, if it contains an N_ap that calls
 *         a LACFUN. Updated FUNDEF_LOCALFUNS.
 *
 ******************************************************************************/
node *
PETLfundef (node *arg_node, info *arg_info)
{
    node *oldfundef;

    DBUG_ENTER ();

    oldfundef = INFO_FUNDEF (arg_info);
    INFO_FUNDEF (arg_info) = arg_node;
    if (NULL == INFO_LACFUN (arg_info)) { /* Vanilla traversal */
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    } else {
        if (arg_node == INFO_LACFUN (arg_info)) {
            DBUG_PRINT ("Looking at lacfun: %s", FUNDEF_NAME (arg_node));
            arg_node = EnhanceLacfunHeader (arg_node, arg_info);
            /* Traverse body to find where to insert F_noteminval/maxval */
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
            /* Prepend new N_arg elements to lacfun header. This
             * can not be done until we finish messing with the function body. */
            FUNDEF_ARGS (arg_node)
              = TCappendArgs (INFO_NEWARGS (arg_info), FUNDEF_ARGS (arg_node));
            INFO_NEWARGS (arg_info) = NULL;
        }
    }

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    INFO_FUNDEF (arg_info) = oldfundef;

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *PETLblock(node *arg_node, info *arg_info)
 *
 * @brief If we have added extrema, insert F_noteminval/maxval ops for
 *        LACFUNs.
 *
 *        Otherwise, merely traverse the entire block.
 *
 * @param arg_node N_block node to be traversed
 *
 * @param arg_info
 *
 * @result Updated N_block node
 *
 ******************************************************************************/
node *
PETLblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((NULL != INFO_NEWARGS (arg_info))) {
        if (FUNDEF_ISCONDFUN (INFO_FUNDEF (arg_info))) {
            arg_node = TRAVcont (arg_node, arg_info); /* We have to dig deeper */
        } else {                                      /* This is a loopfun */
            DBUG_ASSERT (FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info)), "Expected LOOPFUN");
            arg_node = EnhanceLacfunBody (arg_node, arg_info);
            arg_node = TRAVcont (arg_node, arg_info); /* Fix recursive and outer calls */
        }
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *PETLap(node *arg_node, info *arg_info)
 *
 * @brief If this is a non-recursive call to a LACFUN, traverse into it.
 *        When we return, the new arguments to the N_ap will have been
 *        built for us; prepend those to the LACFUN N_ap.
 *
 *        If this is a recursive call to a LACFUN, then we already
 *        have built the new arguments to this call; prepend them
 *        to the LACFUN N_ap.
 *
 * @param arg_node N_ap node to be traversed
 * @param arg_info
 *
 * @result arg_node
 *
 ******************************************************************************/
node *
PETLap (node *arg_node, info *arg_info)
{
    node *calledfn;

    DBUG_ENTER ();

    calledfn = AP_FUNDEF (arg_node);
    if ((FUNDEF_ISLACFUN (calledfn)) &&         /* Ignore non-lacfun call */
        (calledfn != INFO_FUNDEF (arg_info))) { /* Ignore recursive call */
        DBUG_PRINT ("Found LACFUN: %s call from: %s", FUNDEF_NAME (calledfn),
                    FUNDEF_NAME (INFO_FUNDEF (arg_info)));
        INFO_LACFUN (arg_info) = calledfn;     /* The called lacfun */
        INFO_OUTERFUNAP (arg_info) = arg_node; /* The calling N_ap */

        INFO_NEWOUTERAPARGS (arg_info) = NULL;
        /* This is really a traversal only into the LACFUN */
        FUNDEF_LOCALFUNS (INFO_FUNDEF (arg_info))
          = TRAVdo (FUNDEF_LOCALFUNS (INFO_FUNDEF (arg_info)), arg_info);
        INFO_LACFUN (arg_info) = NULL; /* Back to normal traversal */

        /* Append new outer call arguments if the LACFUN generated them for us */
        if (NULL != INFO_NEWOUTERAPARGS (arg_info)) {
            DBUG_PRINT ("Appending new arguments to call of %s from %s",
                        FUNDEF_NAME (calledfn), FUNDEF_NAME (INFO_FUNDEF (arg_info)));
            AP_ARGS (arg_node)
              = TCappendExprs (INFO_NEWOUTERAPARGS (arg_info), AP_ARGS (arg_node));
            INFO_NEWOUTERAPARGS (arg_info) = NULL;
        }
    }

    if ((FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info)))
        && (AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info)) && /* Recursive call */
        (NULL != INFO_NEWRECURSIVEAPARGS (arg_info))) {
        DBUG_PRINT ("Replacing recursive call to LACFUN: %s",
                    FUNDEF_NAME (INFO_FUNDEF (arg_info)));
        AP_ARGS (arg_node)
          = TCappendExprs (INFO_NEWRECURSIVEAPARGS (arg_info), AP_ARGS (arg_node));
        INFO_NEWRECURSIVEAPARGS (arg_info) = NULL;
        FUNDEF_LOOPRECURSIVEAP (INFO_FUNDEF (arg_info)) = arg_node;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PETLcond( node *arg_node, info *arg_info)
 *
 * @brief If we have addded extrema, then
 *        generate F_noteminval/maxval statements for the
 *        THEN and ELSE paths.
 *
 *        Otherwise, this is a normal traversal.
 *
 * @param arg_node N_cond node
 * @param arg_info
 *
 * @result arg_node, updated
 *
 *****************************************************************************/
node *
PETLcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((NULL != INFO_NEWARGS (arg_info))) {
        COND_THEN (arg_node) = EnhanceLacfunBody (COND_THEN (arg_node), arg_info);
        COND_ELSE (arg_node) = EnhanceLacfunBody (COND_ELSE (arg_node), arg_info);
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
