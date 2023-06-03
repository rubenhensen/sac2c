/** <!--********************************************************************-->
 *
 * @defgroup lao Loop Allocation Optimization
 *
 * @ingroup mm
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file loopreuseopt.c
 *
 * Prefix: EMLR
 *
 *****************************************************************************/
#include "loopallocopt.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "EMLR"
#include "debug.h"

#include "print.h"
#include "new_types.h"
#include "user_types.h"
#include "DataFlowMask.h"
#include "filterrc.h"
#include "aliasanalysis.h"
#include "DupTree.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "aliasanalysis.h"
#include "type_utils.h"

/*
 * CONTEXT enumeration: lr_context_t
 */
typedef enum { LAO_undef, LAO_backtrace } lao_context_t;

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    lao_context_t context;
    node *preassign;
    bool prepend;
    node *fundef;
    node *apargs;
    node *doarg;  /* N_avis */
    node *extarg; /* N_avis */
    node *new_extargs;
    node *new_recargs;
    node *new_doargs;
    node *extvardecs;
    dfmask_t *reusemask;
};

#define INFO_CONTEXT(n) (n->context)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_PREPEND(n) (n->prepend)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_APARGS(n) (n->apargs)
#define INFO_DOARG(n) (n->doarg)
#define INFO_EXTARG(n) (n->extarg)
#define INFO_NEW_EXTARGS(n) (n->new_extargs)
#define INFO_NEW_RECARGS(n) (n->new_recargs)
#define INFO_NEW_DOARGS(n) (n->new_doargs)
#define INFO_EXTVARDECS(n) (n->extvardecs)
#define INFO_REUSEMASK(n) (n->reusemask)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_CONTEXT (result) = LAO_undef;
    INFO_PREASSIGN (result) = NULL;
    INFO_PREPEND (result) = FALSE;
    INFO_FUNDEF (result) = fundef;
    INFO_APARGS (result) = NULL;
    INFO_DOARG (result) = NULL;
    INFO_EXTARG (result) = NULL;
    INFO_NEW_EXTARGS (result) = NULL;
    INFO_NEW_RECARGS (result) = NULL;
    INFO_NEW_DOARGS (result) = NULL;
    INFO_EXTVARDECS (result) = NULL;
    INFO_REUSEMASK (result) = NULL;

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

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMLAOdoLoopAllocationOptimization( node *arg_node)
 *
 * @brief starting point of Loop Allocation Optimization traversal
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
EMLAOdoLoopAllocationOptimization (node *arg_node)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Starting Loop Allocation Optimization...");

    TRAVpush (TR_emlao);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_PRINT ("Loop Allocation Optimization Traversal complete.");

    arg_node = EMAAdoAliasAnalysis (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @node *EMLAOap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMLAOap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Traverse into loop functions
     */
    if ((FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        node *old_apargs;

        old_apargs = INFO_APARGS (arg_info);
        INFO_APARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        INFO_APARGS (arg_info) = old_apargs;

        if (INFO_NEW_EXTARGS (arg_info) != NULL) {
            AP_ARGS (arg_node)
              = TCappendExprs (INFO_NEW_EXTARGS (arg_info), AP_ARGS (arg_node));
            INFO_NEW_EXTARGS (arg_info) = NULL;
        }
        INFO_PREPEND (arg_info) = TRUE;
    }

    /* Recursive function call */
    if (AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info)) {
        node *doargs;
        node *recargs;
        node *extargs;

        doargs = FUNDEF_ARGS (AP_FUNDEF (arg_node));
        recargs = AP_ARGS (arg_node);
        extargs = INFO_APARGS (arg_info);

        while (doargs != NULL) {
            /*
             * We only look at arguments at the recursive call
             * that are arrays and is not loop invariant
             */
            if (ARG_AVIS (doargs) != ID_AVIS (EXPRS_EXPR (recargs))
                && !TUisScalar (AVIS_TYPE (ARG_AVIS (doargs)))
                && !AVIS_ISALIAS (ARG_AVIS (doargs))) {
                DBUG_PRINT ("start back tracing...\n");
                /*
                 * Track which fundef arg we are currently examing
                 * and the corresponding argument at the external
                 * callsite
                 */
                INFO_DOARG (arg_info) = ARG_AVIS (doargs);
                INFO_EXTARG (arg_info) = ID_AVIS (EXPRS_EXPR (extargs));

                /* Start backtracing... */
                INFO_CONTEXT (arg_info) = LAO_backtrace;
                AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (recargs)))
                  = TRAVopt (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (recargs))), arg_info);
                INFO_CONTEXT (arg_info) = LAO_undef;
            }

            doargs = ARG_NEXT (doargs);
            recargs = EXPRS_NEXT (recargs);
            extargs = EXPRS_NEXT (extargs);
        }

        INFO_DOARG (arg_info) = NULL;
        INFO_EXTARG (arg_info) = NULL;

        if (INFO_NEW_RECARGS (arg_info) != NULL) {
            DBUG_ASSERT ((INFO_NEW_DOARGS (arg_info) != NULL),
                         "New function arguments are null!");
            AP_ARGS (arg_node)
              = TCappendExprs (INFO_NEW_RECARGS (arg_info), AP_ARGS (arg_node));
            FUNDEF_ARGS (INFO_FUNDEF (arg_info))
              = TCappendArgs (INFO_NEW_DOARGS (arg_info),
                              FUNDEF_ARGS (INFO_FUNDEF (arg_info)));
            INFO_NEW_RECARGS (arg_info) = NULL;
            INFO_NEW_DOARGS (arg_info) = NULL;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLAOassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMLAOassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_CONTEXT (arg_info) == LAO_backtrace) {
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    } else {
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

        if (INFO_PREASSIGN (arg_info) != NULL && INFO_PREPEND (arg_info)) {
            node *old_assign;
            old_assign = arg_node;

            arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);

            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TCappendVardec (INFO_EXTVARDECS (arg_info),
                                FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            INFO_PREASSIGN (arg_info) = NULL;
            INFO_EXTVARDECS (arg_info) = NULL;
            INFO_PREPEND (arg_info) = FALSE;

            ASSIGN_NEXT (old_assign) = TRAVopt (ASSIGN_NEXT (old_assign), arg_info);
        } else {
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLAOarg( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMLAOarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_REUSEMASK (arg_info) == NULL) {
        AVIS_ISALIAS (ARG_AVIS (arg_node)) = FALSE;
    } else {
        AVIS_ISALIAS (ARG_AVIS (arg_node))
          = !DFMtestMaskEntry (INFO_REUSEMASK (arg_info), ARG_AVIS (arg_node));
    }

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLAOfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMLAOfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((!FUNDEF_ISLOOPFUN (arg_node)) || (arg_info != NULL)) {

        DBUG_PRINT ("Traversing function %s", FUNDEF_NAME (arg_node));

        if (FUNDEF_BODY (arg_node) != NULL) {
            node *old_args = NULL;
            dfmask_base_t *maskbase = NULL;
            info *info;
            info = MakeInfo (arg_node);

            if (arg_info != NULL) {
                INFO_APARGS (info) = INFO_APARGS (arg_info);
            }

            /*
             * For loop functions, set all arguments to be non-aliased.
             * The reason we can do this is because if an argument is still
             * determined to be aliased after the AA, we can be sure that
             * it happens inside the loop body and is not caused in the
             * calling context. For aliases caused in the calling context,
             * we can always eliminate that by making a copy of the argument
             * before passing it into the loop function.
             */
            if (FUNDEF_ISLOOPFUN (arg_node)) {
                maskbase
                  = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));

                FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), info);
                arg_node = EMAAdoAliasAnalysis (arg_node);

                INFO_REUSEMASK (info) = DFMgenMaskClear (maskbase);
                old_args = FUNDEF_ARGS (arg_node);
            }

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            if (arg_info != NULL) {
                INFO_NEW_EXTARGS (arg_info) = INFO_NEW_EXTARGS (info);
                INFO_PREASSIGN (arg_info) = INFO_PREASSIGN (info);
                INFO_EXTVARDECS (arg_info) = INFO_EXTVARDECS (info);
            }

            if (FUNDEF_ISLOOPFUN (arg_node)) {
                old_args = TRAVopt (old_args, info);
                old_args = NULL;
                INFO_REUSEMASK (info) = DFMremoveMask (INFO_REUSEMASK (info));
                maskbase = DFMremoveMaskBase (maskbase);
            }

            info = FreeInfo (info);
        }
    }

    if (arg_info == NULL) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLAOwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMLAOwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_CONTEXT (arg_info) == LAO_backtrace) {
        WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLAOwith2( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMLAOwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_CONTEXT (arg_info) == LAO_backtrace) {
        WITH2_WITHOP (arg_node) = TRAVopt (WITH2_WITHOP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLAOmodarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMLAOmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT ((INFO_CONTEXT (arg_info) == LAO_backtrace), "Wrong traverse mode!");

    AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node)))
      = TRAVopt (AVIS_SSAASSIGN (ID_AVIS (MODARRAY_MEM (arg_node))), arg_info);

    DBUG_RETURN (arg_node);
}

static info *
CreateExternalAssigns (info *arg_info)
{
    node *extavis, *memavis, *valavis;

    DBUG_ENTER ();

    /*
     * Insert copy instructions
     *
     * b     = do( a);
     *
     * is converted into
     *
     * a_mem = alloc( dim(a), shape(a));
     * a_val = fill( copy( a), a_mem);
     *
     * b      = do( a_val, a);
     *
     * Here a_val is a duplicate of a and it
     * will be reused in the loop body.
     *
     */

    /*
     * Obtain old argument 'a'
     */
    extavis = INFO_EXTARG (arg_info);

    /*
     * Create a new memory variable
     * Ex: a_mem
     */
    memavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (extavis)),
                          TYeliminateAKV (AVIS_TYPE (extavis)));

    INFO_EXTVARDECS (arg_info) = TBmakeVardec (memavis, INFO_EXTVARDECS (arg_info));

    /*
     * Create a new value variable
     * Ex: a_val
     */
    valavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (extavis)),
                          TYeliminateAKV (AVIS_TYPE (extavis)));

    INFO_EXTVARDECS (arg_info) = TBmakeVardec (valavis, INFO_EXTVARDECS (arg_info));

    /*
     * Create fill operation
     *
     * Ex:
     *   ...
     *   a_val = fill( copy( a), a_mem);
     */
    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (valavis, NULL),
                                 TCmakePrf2 (F_fill,
                                             TCmakePrf1 (F_noop, TBmakeId (extavis)),
                                             TBmakeId (memavis))),
                      INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (valavis) = INFO_PREASSIGN (arg_info);

    /*
     * Prepend application argument
     *
     * Ex:
     *   ...
     *   a_val = fill( copy( a), a_mem);
     *   b     = do( a_val, a);
     */
    INFO_NEW_EXTARGS (arg_info)
      = TBmakeExprs (TBmakeId (valavis), INFO_NEW_EXTARGS (arg_info));

    /*
     * Create alloc assignment
     *
     * Ex:
     *   a_mem = alloc( dim( a), shape( a), a);
     *   a_val = fill( copy( a), a_mem);
     *   b     = do( a_val, a);
     */
    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (memavis, NULL),
                                 TCmakePrf2 (F_alloc,
                                             TCmakePrf1 (F_dim_A, TBmakeId (extavis)),
                                             TCmakePrf1 (F_shape_A, TBmakeId (extavis)))),
                      INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (memavis) = INFO_PREASSIGN (arg_info);

    DBUG_RETURN (arg_info);
}

/** <!--********************************************************************-->
 *
 * @node *EMLAOprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMLAOprf (node *arg_node, info *arg_info)
{
    node *reuse_avis;

    DBUG_ENTER ();

    if (INFO_CONTEXT (arg_info) == LAO_backtrace) {
        switch (PRF_PRF (arg_node)) {
        case F_reuse:
            DBUG_PRINT ("starting backtracing from F_reuse...\n");
            AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (arg_node)))
              = TRAVopt (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (arg_node))), arg_info);
            break;
        case F_alloc:
            INFO_NEW_RECARGS (arg_info) = TBmakeExprs (TBmakeId (INFO_DOARG (arg_info)),
                                                       INFO_NEW_RECARGS (arg_info));

            reuse_avis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (INFO_DOARG (arg_info))),
                                     TYeliminateAKV (AVIS_TYPE (INFO_DOARG (arg_info))));

            INFO_NEW_DOARGS (arg_info)
              = TBmakeArg (reuse_avis, INFO_NEW_DOARGS (arg_info));

            DFMsetMaskEntryClear (INFO_REUSEMASK (arg_info), INFO_DOARG (arg_info));

            /* Change F_alloc to F_reuse */
            PRF_ARGS (arg_node) = FREEdoFreeTree (PRF_ARGS (arg_node));
            PRF_PRF (arg_node) = F_reuse;
            PRF_ARGS (arg_node) = TBmakeExprs (TBmakeId (reuse_avis), NULL);

            /* Three arg_info fields are updated by this function:
             *   - INFO_PREASSIGNS external assignments to create and copy
             *                     the reusable array.
             *   - INFO_EXTVARDECS external new variable declarations.
             *   - INFO_NEW_EXTARGS new external loop function application
             *                      arguments, i.e. the reusable array.
             */
            arg_info = CreateExternalAssigns (arg_info);
            break;
        default:
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}
 *****************************************************************************/
