/** <!--********************************************************************-->
 *
 * @defgroup lr Loop Reuse Optimization
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
 * The idea:
 * ---------
 *
 * This traversal tries to improve loops that iteratively modify arrays.
 * The simplest example is:
 *
 *    i = 0;
 *    do {
 *      a[iv] = 29;
 *      i++;
 *    } while (i<20);
 *
 * Without LRO, the loop function body would contain code similar to this:
 *
 *   _emal_2615__emec_2599_a = _alloc_or_reuse_( 1, 1, [ 100 ], a);
 *   _emrb_3052_mem = _alloc_( 1, 0, [:int]);
 *   _emrb_3053_val = _fill_( _isreused_( _emal_2615__emec_2599_a, a), _emrb_3052_mem);
 *   _emec_2599_a = _MAIN::main__Loop_0__ReuseCond_4( _emrb_3053_val, _emal_2615__emec_2599_a, a) ;
 *   a__SSA0_1 = _fill_( _idx_modarray_AxSxS_( _emec_2599_a, i, _flat_11), _emec_2599_a);
 *
 * Since an alias of 'a' may be used below the loop, the loop body first checks
 * whether 'a' can be reused. depending on whether it can be reused, either the non-
 * modified elements of 'a' need to be copied into the new 'a' (_emec_2599_a) or
 * not. This need for distinction leads to the cond-function main__Loop_0__ReuseCond_4.
 * Thereafter, the actual in-place modification is performed (_idx_modarray_AxSxS_).
 *
 * The observation that gives rise to this optimisation is: at most the very first
 * iteration will require allocating and copying. Thereafter, this new array is
 * "unique" (RC==1) and will be reused all 19 iterations thereafter.
 * The idea of this optimisation is to ensure that 'a' will be unique in the
 * very first iteration as well. This is achieved by modifying the code of
 * the outside call to the loop function into somthing like this:
 *
 *   _emlr_3020_a = _alloc_or_reuse_( _dim_A_( a), _shape_A_( a), a);
 *   _emlr_3021_a = _fill_( _copy_( a), _emlr_3020_a);
 *   a__SSA0_1 = _MAIN::main__Loop_0( _emlr_3021_a, i) ;
 *
 * This, combined with an annotation of '_emlr_3021_a' being non-aliased,
 * suffices for the remaining mem-phases to end up with a loop body like this:
 *
 *   _emal_2615__emec_2599_a = a;
 *   _emec_2599_a = _fill_( _noop_( a), _emal_2615__emec_2599_a);
 *   a__SSA0_1 = _fill_( _idx_modarray_AxSxS_( _emec_2599_a, i, _flat_11), _emec_2599_a);
 *
 * IE, we now statically reuse 'a' within all 20 iterations, no RC checks or
 * alternative code variants needed.
 *
 *
 *
 *
 * The algorithm:
 * --------------
 *
 * In general, we look for loop funs whose parameters are being arguments to
 * _alloc_or_reuse_. However, not all of those qualify. Consider a case similar
 * to the one above. It would look like this:
 *
 *   <t> loop( a, i)
 *   {
 *       ...
 *       b = _alloc_or_reuse_ (a);
 *       ...
 *       if (p) {
 *          rec = loop (b, j);
 *       } else {
 *       }
 *       res = (p? rec: b);
 *       return res;
 *   }
 *
 *   Notice here, that the argument 'a' which we want to make unique also
 *   serves as first argument in the recursive call. This is essential as
 *   it guarantees that in the next iteration, it will be unique again!
 *
 *   In essence this means we are looking for parameters that are being
 *   used in _alloc_or_reuse_ *and* that are being fed a guaranteed unique
 *   argument in the recursive call.
 *   In order to decide whether 'b' is unique there, we need to have
 *   aliasing analysis. Unfortunately, AA has not yet been run and even
 *   if it was, that would not help, since standard AA always assumes that
 *   all function parameters are aliases and, thus would infer 'b' to
 *   be aliased.
 *   What needs to be done is to start out with an assumption that all
 *   those parameters that we intend to make unique are marked as
 *   non-aliased, then run an AA and check whether the corresponding
 *   recursive call arguments are non-aliased. If any one of them turns
 *   out to be aliased, we have to restart again, now marking the
 *   corresponding parameter as aliased. We repeat this exercise
 *   until a fixedpoint is reached.
 *
 *   The case described in issue #2323 is an excellent example of a case
 *   where successive refinements of the reuse-set happen. Essentially,
 *   the example there looks like this:
 *
 *   <t> loop( a, b, i)
 *   {
 *       ...
 *       c = _alloc_or_reuse_ (b);
 *       j = _alloc_or_reuse_ (i);
 *       ...
 *       if (p) {
 *          rec = loop (c, a, j);
 *       } else {
 *       }
 *       res = (p? rec: a);
 *       return res;
 *   }
 *
 *   In the initial round, we assume {a,b,i} to be non-aliased. While
 *   traversing the body, we find that {b,i} are possible reuse-set
 *   candidates. When looking at the recursive call AA sees that {a,b,i}
 *   all are non-aliased so we conclude that {b,i} could be the the reuse-set
 *   since neither 'a' nor 'j' are aliased.
 *   Now, we update the alias anotation of the parameters [NB: this was missing
 *   prior to bug #2323 being fixed :-)]; assuming that only {b,i} are
 *   non-aliased. Upon our second iteration, we now find out that 'a' in the
 *   recursive call is classifies as aliased which results in 'b' being taken
 *   out of the reuse-set, leaving us with 'i' only.
 *   Again, we change the alias annotation of 'b' to aliased and rerun our
 *   inference, just to find that the fixpoint has been reached.
 *
 *   Note here that it might be considered crazy that we actually lift
 *   scalar values such as 'i' here. However, that is ok as the entire
 *   mem phase treats all values equaly and eventualy strips out all
 *   potential overheads here!
 *
 *
 *
 *
 *
 * Implementation:
 * ---------------
 *
 * This traversal actually houses 2 traversals! EMLR and EMLRO!
 * EMLR is responsible for identifying loop functions. Once done,
 * it starts an EMLRO traversal on the body of that loop function.
 * The sole function of the EMLRO traversal is to compute the
 * ARG_ISALIAS flags of the loop function, i.e., the fixpoint iteration
 * explained above!
 * Once that is done, EMLR takes over again and it performs the code
 * transformation for making the actual parameters of the external
 * loop fun call unique. It uses the ARG_ISALIAS flags set by EMLRO
 * to identify those arguments that need modification.
 *
 *
 *****************************************************************************/
#include "loopreuseopt.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "EMLR"
#include "debug.h"

#include "print.h"
#include "shape.h"
#include "new_types.h"
#include "user_types.h"
#include "DataFlowMask.h"
#include "filterrc.h"
#include "aliasanalysis.h"
#include "DupTree.h"
#include "str.h"
#include "memory.h"

/*
 * CONTEXT enumeration: lr_context_t
 */
typedef enum { LR_undef, LR_allocorreuse, LR_condargs, LR_doargs, LR_recap } lr_context_t;

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    lr_context_t context;
    node *preassign;
    node *fundef;
    node *apargs;
    dfmask_t *apmask;
    dfmask_t *reusemask;
};

#define INFO_CONTEXT(n) (n->context)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_REUSEMASK(n) (n->reusemask)
#define INFO_APARGS(n) (n->apargs)
#define INFO_APMASK(n) (n->apmask)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_CONTEXT (result) = LR_undef;
    INFO_PREASSIGN (result) = NULL;
    INFO_FUNDEF (result) = fundef;
    INFO_REUSEMASK (result) = NULL;
    INFO_APARGS (result) = NULL;
    INFO_APMASK (result) = NULL;

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
 * @fn node *EMLRdoLoopReuseOptimization( node *arg_node)
 *
 * @brief starting point of Loop Reuse Optimization traversal
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
EMLRdoLoopReuseOptimization (node *arg_node)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Starting Loop Reuse Optimization...");

    TRAVpush (TR_emlr);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_PRINT ("Loop Reuse Traversal complete.");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/
static bool
DFMequalMasks (dfmask_t *mask1, dfmask_t *mask2)
{
    dfmask_t *d1, *d2;
    int sum;

    DBUG_ENTER ();

    d1 = DFMgenMaskMinus (mask1, mask2);
    d2 = DFMgenMaskMinus (mask2, mask1);

    sum = DFMtestMask (d1) + DFMtestMask (d2);

    d1 = DFMremoveMask (d1);
    d2 = DFMremoveMask (d2);

    DBUG_RETURN (sum == 0);
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
/** <!--********************************************************************-->
 *
 * @node *EMLRap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMLRap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Traverse into special functions
     */
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    /*
     * Optimize loops
     */
    if ((FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        node *doargs;
        node *apargs;

        /*
         * Perform loop optimization using seperate traversal
         */
        TRAVpush (TR_emlro);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), NULL);
        TRAVpop ();

        /*
         * Copy all arguments that can be statically reused inside the loop
         */
        doargs = FUNDEF_ARGS (AP_FUNDEF (arg_node));
        apargs = AP_ARGS (arg_node);

        while (doargs != NULL) {
            /*
             * We don't want to affect aliased N_avis or any N_avis that
             * is a result of the EMRL optimisation which lifts memory
             * out of loops (as this is redundent here).
             */
            if (!AVIS_ISALIAS (ARG_AVIS (doargs))
                && !AVIS_ISALLOCLIFT (ID_AVIS (EXPRS_EXPR (apargs)))) {
                /*
                 * Insert copy instructions
                 *
                 * b     = do( a);
                 *
                 * is converted into
                 *
                 * a_mem = alloc_or_reuse( dim(a), shape(a), a);
                 * a_val = fill( copy( a), a_mem);
                 *
                 * b      = do( a_val);
                 */
                node *memavis, *valavis, *oldarg, *oldavis;

                oldarg = EXPRS_EXPR (apargs);
                oldavis = ID_AVIS (oldarg);

                /*
                 * Create a new memory variable
                 * Ex: a_mem
                 */
                memavis = TBmakeAvis (TRAVtmpVarName (ID_NAME (oldarg)),
                                      TYeliminateAKV (AVIS_TYPE (oldavis)));

                FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (memavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

                /*
                 * Create a new value variable
                 * Ex: a_val
                 */
                valavis = TBmakeAvis (TRAVtmpVarName (ID_NAME (oldarg)),
                                      TYcopyType (AVIS_TYPE (oldavis)));

                FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (valavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

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
                                                         TCmakePrf1 (F_copy,
                                                                     DUPdoDupNode (
                                                                       oldarg)),
                                                         TBmakeId (memavis))),
                                  INFO_PREASSIGN (arg_info));
                AVIS_SSAASSIGN (valavis) = INFO_PREASSIGN (arg_info);

                /*
                 * Substitute application argument
                 *
                 * Ex:
                 *   ...
                 *   a_val = fill( copy( a), a_mem);
                 *   b     = do( a_val);
                 */
                EXPRS_EXPR (apargs) = TBmakeId (valavis);

                /*
                 * Create allor_or_reuse assignment
                 *
                 * Ex:
                 *   a_mem = allor_or_reuse( dim( a), shape( a), a);
                 *   a_val = fill( copy( a), a_mem);
                 *   b     = do( a_val);
                 */
                INFO_PREASSIGN (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (memavis, NULL),
                                             TCmakePrf3 (F_alloc_or_reuse,
                                                         TCmakePrf1 (F_dim_A,
                                                                     DUPdoDupNode (
                                                                       oldarg)),
                                                         TCmakePrf1 (F_shape_A,
                                                                     DUPdoDupNode (
                                                                       oldarg)),
                                                         oldarg)),
                                  INFO_PREASSIGN (arg_info));
                AVIS_SSAASSIGN (memavis) = INFO_PREASSIGN (arg_info);
            }

            doargs = ARG_NEXT (doargs);
            apargs = EXPRS_NEXT (apargs);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLRassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMLRassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Bottom-up traversal
     */
    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL) {
        INFO_PREASSIGN (arg_info) = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);

        arg_node = INFO_PREASSIGN (arg_info);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLRfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMLRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((!FUNDEF_ISLACFUN (arg_node)) || (arg_info != NULL)) {

        DBUG_PRINT ("Traversing function %s", FUNDEF_NAME (arg_node));
        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info;
            info = MakeInfo (arg_node);

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            info = FreeInfo (info);
        }
    }

    if (arg_info == NULL) {
        FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Loop optimization traversal functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @node *EMLROap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMLROap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        INFO_APARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    if ((FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info))) {
        /*
         * Clean up reusable arguments
         */
        DBUG_PRINT ("   checking recursive loop call");
        DBUG_PRINT ("   current reuse candidates are:");
        DBUG_EXECUTE (DFMprintMask (global.outfile, "%s, ",
                      INFO_REUSEMASK (arg_info)));
        if (FUNDEF_ARGS (INFO_FUNDEF (arg_info)) != NULL) {
            INFO_CONTEXT (arg_info) = LR_recap;
            INFO_APARGS (arg_info) = AP_ARGS (arg_node);

            FUNDEF_ARGS (INFO_FUNDEF (arg_info))
              = TRAVdo (FUNDEF_ARGS (INFO_FUNDEF (arg_info)), arg_info);

            INFO_CONTEXT (arg_info) = LR_undef;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLROarg( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMLROarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_CONTEXT (arg_info)) {
    case LR_doargs:
        if (INFO_REUSEMASK (arg_info) == NULL) {
            AVIS_ISALIAS (ARG_AVIS (arg_node)) = FALSE;
        } else {
            AVIS_ISALIAS (ARG_AVIS (arg_node))
              = !DFMtestMaskEntry (INFO_REUSEMASK (arg_info), ARG_AVIS (arg_node));
        }
        break;

    case LR_condargs:
        if (DFMtestMaskEntry (INFO_REUSEMASK (arg_info), ARG_AVIS (arg_node))) {
            DFMsetMaskEntrySet (INFO_APMASK (arg_info),
                                ID_AVIS (EXPRS_EXPR (INFO_APARGS (arg_info))));
        }

        INFO_APARGS (arg_info) = EXPRS_NEXT (INFO_APARGS (arg_info));
        break;

    case LR_recap:
        DBUG_PRINT ("      argument %s is %s",
                    ID_NAME (EXPRS_EXPR (INFO_APARGS (arg_info))),
                    AVIS_ISALIAS (ID_AVIS (EXPRS_EXPR (INFO_APARGS (arg_info))))?
                    "alias => elide" : "ok");
        if (AVIS_ISALIAS (ID_AVIS (EXPRS_EXPR (INFO_APARGS (arg_info))))) {
            DBUG_PRINT ("      eliding %s from reuse-mask", ARG_NAME (arg_node));
            DFMsetMaskEntryClear (INFO_REUSEMASK (arg_info), ARG_AVIS (arg_node));
        }

        INFO_APARGS (arg_info) = EXPRS_NEXT (INFO_APARGS (arg_info));
        break;

    default:
        DBUG_UNREACHABLE ("Illegal context!");
        break;
    }

    ARG_NEXT (arg_node) = TRAVopt(ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLROfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMLROfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
#ifndef DBUG_OFF
    int fix_iter;
#endif

    DBUG_ASSERT (FUNDEF_ISLACFUN (arg_node),
                 "EMLROfundef is only applicable for LAC-functions");

    if (FUNDEF_ISLOOPFUN (arg_node)) {
        info *info;
        node *fundef_next;
        dfmask_base_t *maskbase;
        dfmask_t *oldmask;

        DBUG_PRINT ("Optimizing %s", FUNDEF_NAME (arg_node));

        /*
         * Traversal of DOFUN
         */

        info = MakeInfo (arg_node);
        maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));

        /*
         * rescue FUNDEF_NEXT
         */
        fundef_next = FUNDEF_NEXT (arg_node);

        /*
         * Filter reuse candidates
         */
        FUNDEF_NEXT (arg_node) = NULL;
        arg_node = FRCdoFilterReuseCandidates (arg_node);

        /*
         * Initialize arguments' AVIS_ALIAS with FALSE
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            INFO_CONTEXT (info) = LR_doargs;
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            INFO_CONTEXT (info) = LR_undef;
        }

        /*
         * Perform alias analysis
         */
        arg_node = EMAAdoAliasAnalysis (arg_node);

        INFO_REUSEMASK (info) = DFMgenMaskClear (maskbase);
        oldmask = DFMgenMaskClear (maskbase);

#ifndef DBUG_OFF
        fix_iter = 0;
#endif
        DBUG_PRINT ("starting fixpoint iteration of loop body");
        while (TRUE) {
            DFMsetMaskCopy (oldmask, INFO_REUSEMASK (info));

#ifndef DBUG_OFF
        fix_iter += 1;
#endif

            DBUG_PRINT ("iteration %d:", fix_iter);
            /*
             * Traverse body in order to determine REUSEMASK
             */
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            if (DFMequalMasks (oldmask, INFO_REUSEMASK (info))) {
                break;
            }

            /*
             * Perform alias analysis
             */
            arg_node = EMAAdoAliasAnalysis (arg_node);

            /*
             * Update arguments' AVIS_ALIAS with values from REUSEMASK
             */
            if (FUNDEF_ARGS (arg_node) != NULL) {
                INFO_CONTEXT (info) = LR_doargs;
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
                INFO_CONTEXT (info) = LR_undef;
            }

        }

        /*
         * Print statically resusable variables
         */
        DBUG_PRINT ("Fixpoint reached!");
        DBUG_PRINT ("The following variables can be statically reused:");
        DBUG_EXECUTE (DFMprintMask (global.outfile, "%s, ", INFO_REUSEMASK (info)));

        /*
         * Initialize arguments' AVIS_ALIAS with values from REUSEMASK
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            INFO_CONTEXT (info) = LR_doargs;
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            INFO_CONTEXT (info) = LR_undef;
        }

        /*
         * Restore FUNDEF_NEXT
         */
        FUNDEF_NEXT (arg_node) = fundef_next;

        oldmask = DFMremoveMask (oldmask);
        INFO_REUSEMASK (info) = DFMremoveMask (INFO_REUSEMASK (info));
        maskbase = DFMremoveMaskBase (maskbase);
        info = FreeInfo (info);
    } else {
        info *info;
        dfmask_base_t *maskbase;

        /*
         * Traversal of inner CONDFUN
         */
        info = MakeInfo (arg_node);
        maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));
        INFO_REUSEMASK (info) = DFMgenMaskClear (maskbase);

        /*
         * Traverse function body
         */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

        /*
         * Transcribe REUSEMASK
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            INFO_APMASK (info) = INFO_REUSEMASK (arg_info);
            INFO_APARGS (info) = INFO_APARGS (arg_info);
            INFO_APARGS (arg_info) = NULL;
            INFO_CONTEXT (info) = LR_condargs;
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            INFO_CONTEXT (info) = LR_undef;
        }

        INFO_REUSEMASK (info) = DFMremoveMask (INFO_REUSEMASK (info));
        maskbase = DFMremoveMaskBase (maskbase);
        info = FreeInfo (info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLROid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMLROid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_CONTEXT (arg_info)) {
    case LR_allocorreuse:
        if (!AVIS_ISALIAS (ID_AVIS (arg_node))) {
            DFMsetMaskEntrySet (INFO_REUSEMASK (arg_info), ID_AVIS (arg_node));
        }
        break;

    case LR_undef:
        break;

    default:
        DBUG_UNREACHABLE ("Illegal context!");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLROprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMLROprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PRF_PRF (arg_node) == F_alloc_or_reuse) {
        if (PRF_EXPRS3 (arg_node) != NULL) {
            INFO_CONTEXT (arg_info) = LR_allocorreuse;
            PRF_EXPRS3 (arg_node) = TRAVdo (PRF_EXPRS3 (arg_node), arg_info);
            INFO_CONTEXT (arg_info) = LR_undef;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Loop Reuse Optimization -->
 *****************************************************************************/

#undef DBUG_PREFIX
