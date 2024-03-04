/******************************************************************************
 *
 * @defgroup RCI Reference Counting Inference
 *
 * Annotes reference counting instructions throughout the syntax tree.
 *
 * This phase injects an rc initialisation as first argument into the following
 * primitive functions:
 *   F_alloc, F_alloc_or_reuse, F_reshape_VxA, F_alloc_or_reshape, F_reuse,
 *   F_alloc_or_resize, F_resize, and F_suballoc
 *
 ******************************************************************************/
#include "cuda_utils.h"
#include "DataFlowMask.h"
#include "free.h"
#include "memory.h"
#include "NumLookUpTable.h"
#include "str.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "tree_utils.h"

#define DBUG_PREFIX "RCI"
#include "debug.h"

#include "referencecounting.h"

/******************************************************************************
 *
 * @enum rc_countmode
 *
 * @brief Enumeration of the different counting modes for N_id nodes.
 *
 ******************************************************************************/
typedef enum {
    rc_prfuse,
    rc_apuse
} rc_countmode;

/******************************************************************************
 *
 * @struct INFO
 *
 * @param INFO_MODE
 * @param INFO_ENV
 * @param INFO_ENV2
 * @param INFO_POSTASSIGN
 * @param INFO_FUNDEF
 * @param INFO_MASKBASE
 * @param INFO_WITHMASK
 * @param INFO_WITHVECNEEDED
 * @param INFO_ASSIGN
 * @param INFO_MUSTCOUNT
 * @param INFO_INWITHS
 *
 ******************************************************************************/
struct INFO {
    rc_countmode mode;
    nlut_t *env;
    nlut_t *env2;
    node *postassign;
    node *fundef;
    dfmask_base_t *maskbase;
    dfmask_t *withmask;
    bool withvecneeded;
    node *assign;
    bool mustcount;
    bool inwiths;
};

#define INFO_MODE(n) ((n)->mode)
#define INFO_ENV(n) ((n)->env)
#define INFO_ENV2(n) ((n)->env2)
#define INFO_POSTASSIGN(n) ((n)->postassign)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_MASKBASE(n) ((n)->maskbase)
#define INFO_WITHMASK(n) ((n)->withmask)
#define INFO_WITHVECNEEDED(n) ((n)->withvecneeded)
#define INFO_ASSIGN(n) ((n)->assign)
#define INFO_MUSTCOUNT(n) ((n)->mustcount)
#define INFO_INWITHS(n) ((n)->inwiths)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_MODE (result) = rc_prfuse;
    INFO_ENV (result) = NULL;
    INFO_ENV2 (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_MASKBASE (result) = NULL;
    INFO_WITHMASK (result) = NULL;
    INFO_ASSIGN (result) = NULL;
    INFO_MUSTCOUNT (result) = FALSE;
    INFO_WITHVECNEEDED (result) = FALSE;
    INFO_INWITHS (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * @fn node *RCIdoReferenceCounting (node *arg_node)
 *
 * @brief Starting function of EM based reference counting inference.
 *
 * @return Modified syntax tree containing explicit memory management
 * instructions.
 *
 ******************************************************************************/
node *
RCIdoReferenceCounting (node *arg_node)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Starting reference counting inference...");

    TRAVpush (TR_rci);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_PRINT ("Reference counting inference complete.");

    DBUG_RETURN (arg_node);
}

static void
IncNum (nlut_t *env, node *avis, int count)
{
    DBUG_ENTER ();

    /**
     * If unused argument removal (UAR) marked this avis as not in use, it
     * should be skipped. In the optimisation phase applications of functions
     * have been edited such that a temporary dummy value is passed for this
     * value. In the precompile phase this dummy value will be removed, and the
     * corresponding argument will be removed from the function definition.
     * Therefore no reference counting should be applied.
     */
    if (!AVIS_ISDUMMY (avis)) {
        NLUTincNum (env, avis, count);
    }

    DBUG_RETURN ();
}

static node *
AdjustRC (node *avis, int count, node *arg_node)
{
    node *prf;

    DBUG_ENTER ();

    if (!AVIS_ISDUMMY (avis) && count != 0) {
        if (count > 0) {
            prf = TCmakePrf2 (F_inc_rc, TBmakeId (avis), TBmakeNum (count));
        } else {
            prf = TCmakePrf2 (F_dec_rc, TBmakeId (avis), TBmakeNum (-count));
        }
        arg_node = TBmakeAssign (TBmakeLet (NULL, prf), arg_node);
    }

    DBUG_RETURN (arg_node);
}

static node *
MakeRCAssignments (nlut_t *nlut)
{
    node *avis, *res = NULL;
    int count;

    DBUG_ENTER ();

    avis = NLUTgetNonZeroAvis (nlut);
    while (avis != NULL) {
        count = NLUTgetNum (nlut, avis);
        NLUTsetNum (nlut, avis, 0);

        res = AdjustRC (avis, count, res);

        avis = NLUTgetNonZeroAvis (NULL);
    }

    DBUG_RETURN (res);
}

static bool
ArgIsInout (node *arg, node *rets)
{
    bool res;

    DBUG_ENTER ();

    res = ARG_HASLINKSIGNINFO (arg)
        && rets != NULL
        && ((RET_HASLINKSIGNINFO (rets) && RET_LINKSIGN (rets) == ARG_LINKSIGN (arg))
           || ArgIsInout (arg, RET_NEXT (rets)));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *RCIfundef (node *arg_node, info *arg_info)
 *
 * @brief Traverses a fundef node by traversing the functions block.
 * After that, adjust_rc operations for the arguments are inserted.
 *
 ******************************************************************************/
node *
RCIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!FUNDEF_ISCONDFUN (arg_node) || arg_info != NULL) {
        DBUG_PRINT ("Inferencing reference counters in function %s...",
                    FUNDEF_NAME (arg_node));

        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info;

            info = MakeInfo ();
            INFO_FUNDEF (info) = arg_node;
            INFO_MASKBASE (info) = DFMgenMaskBase (FUNDEF_ARGS (arg_node),
                                                   FUNDEF_VARDECS (arg_node));
            INFO_ENV (info) = NLUTgenerateNlut (FUNDEF_ARGS (arg_node),
                                                FUNDEF_VARDECS (arg_node));

            if (FUNDEF_ISCONDFUN (arg_node)) {
                // Use environment of applying context
                node *extlet, *retexprs, *ids;

                extlet = ASSIGN_STMT (INFO_ASSIGN (arg_info));
                retexprs = RETURN_EXPRS (FUNDEF_RETURN (arg_node));
                ids = LET_IDS (extlet);

                while (ids != NULL) {
                    NLUTsetNum (INFO_ENV (info),
                                ID_AVIS (EXPRS_EXPR (retexprs)),
                                NLUTgetNum (INFO_ENV (arg_info), IDS_AVIS (ids)));
                    NLUTsetNum (INFO_ENV (arg_info), IDS_AVIS (ids), 0);

                    ids = IDS_NEXT (ids);
                    retexprs = EXPRS_NEXT (retexprs);
                }
            }

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            if (FUNDEF_ISCONDFUN (arg_node)) {
                // Transcribe environment back into applying context
                node *extlet, *argexprs, *args;

                extlet = ASSIGN_STMT (INFO_ASSIGN (arg_info));
                args = FUNDEF_ARGS (arg_node);
                argexprs = AP_ARGS (LET_EXPR (extlet));

                while (args != NULL) {
                    IncNum (INFO_ENV (arg_info),
                            ID_AVIS (EXPRS_EXPR (argexprs)),
                            NLUTgetNum (INFO_ENV (info), ARG_AVIS (args)));

                    args = ARG_NEXT (args);
                    argexprs = EXPRS_NEXT (argexprs);
                }
            } else {
                // Create RC statements at the beginning of the function
                node *arg;

                arg = FUNDEF_ARGS (arg_node);
                while (arg != NULL) {
                    if (ARG_ISUSEDINBODY (arg)) {
                        IncNum (INFO_ENV (info), ARG_AVIS (arg), -1);
                    }

                    arg = ARG_NEXT (arg);
                }
                BLOCK_ASSIGNS (FUNDEF_BODY (arg_node)) =
                    TCappendAssign (MakeRCAssignments (INFO_ENV (info)),
                                    BLOCK_ASSIGNS (FUNDEF_BODY (arg_node)));
            }

            INFO_ENV (info) = NLUTremoveNlut (INFO_ENV (info));
            INFO_MASKBASE (info) = DFMremoveMaskBase (INFO_MASKBASE (info));
            info = FreeInfo (info);
        }

        DBUG_PRINT ("Reference counting inference in function %s complete.",
                    FUNDEF_NAME (arg_node));
    }

    // Traverse other fundefs if this is a regular fundef traversal
    if (arg_info == NULL) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIassign (node *arg_node, info *arg_info)
 *
 * @brief Traverses bottom-up, the RHS and subsequently appends USE and DEFs.
 *
 ******************************************************************************/
node *
RCIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    INFO_ASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    ASSIGN_NEXT (arg_node) = TCappendAssign (INFO_POSTASSIGN (arg_info),
                                             ASSIGN_NEXT (arg_node));
    INFO_POSTASSIGN (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIlet (node *arg_node, info *arg_info)
 *
 * @brief Traverses the RHS and subsequently adds LHS identifiers to DEFLIST.
 *
 ******************************************************************************/
node *
RCIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_MUSTCOUNT (arg_info) = TRUE;

    INFO_MODE (arg_info) = rc_apuse;
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_MUSTCOUNT (arg_info)) {
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIreturn (node *arg_node, info *arg_info)
 *
 * @brief Traverses the returned identifiers each environment will be increased
 * by one.
 *
 ******************************************************************************/
node *
RCIreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!FUNDEF_ISCONDFUN (INFO_FUNDEF (arg_info))) {
        INFO_MODE (arg_info) = rc_apuse;
        RETURN_EXPRS (arg_node) = TRAVopt (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIid (node *arg_node, info *arg_info)
 *
 * @brief Traverses RHS identifiers.
 *
 ******************************************************************************/
node *
RCIid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    IncNum (INFO_ENV (arg_info), ID_AVIS (arg_node), 1);

    if (INFO_MODE (arg_info) == rc_prfuse) {
        INFO_POSTASSIGN (arg_info) =
            AdjustRC (ID_AVIS (arg_node), -1, INFO_POSTASSIGN (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIids (node *arg_node, info *arg_info)
 *
 * @brief Traverses LHS identifiers.
 *
 ******************************************************************************/
node *
RCIids (node *arg_node, info *arg_info)
{
    int count;

    DBUG_ENTER ();

    count = NLUTgetNum (INFO_ENV (arg_info), IDS_AVIS (arg_node));
    NLUTsetNum (INFO_ENV (arg_info), IDS_AVIS (arg_node), 0);

    INFO_POSTASSIGN (arg_info) =
        AdjustRC (IDS_AVIS (arg_node), count - 1, INFO_POSTASSIGN (arg_info));

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIap (node *arg_node, info *arg_info)
 *
 * @brief Adds one to each of the argument's environments. By definition, a
 * function application consumes a reference from each of its arguments.
 *
 ******************************************************************************/
node *
RCIap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        /**
         * CONDFUNs are traversed in order of appearance.
         * Make sure all results are actually consumed first!
         */
        node *ids = ASSIGN_LHS (INFO_ASSIGN (arg_info));

        while (ids != NULL) {
            if (NLUTgetNum (INFO_ENV (arg_info), IDS_AVIS (ids)) == 0) {
                NLUTsetNum (INFO_ENV (arg_info), IDS_AVIS (ids), 1);

                INFO_POSTASSIGN (arg_info) =
                    AdjustRC (IDS_AVIS (ids), -1, INFO_POSTASSIGN (arg_info));
            }
            ids = IDS_NEXT (ids);
        }

        // Traverse condfun
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

        INFO_MUSTCOUNT (arg_info) = FALSE;
    } else {
        node *apargs, *funargs;
        /**
         * Some parameters of external functions must be externally refcounted
         * Furthermore, reference arguments are not refcounted at all.
         * For ... arguments, the reference counting is determined by the
         * refcountdots pragma.
         */
        funargs = FUNDEF_ARGS (AP_FUNDEF (arg_node));
        apargs = AP_ARGS (arg_node);

        while (apargs != NULL) {

            if (funargs == NULL) {
                if (FUNDEF_REFCOUNTDOTS (AP_FUNDEF (arg_node))) {
                    INFO_MODE (arg_info) = rc_apuse;
                } else {
                    INFO_MODE (arg_info) = rc_prfuse;
                }
            } else if (!ArgIsInout (funargs, FUNDEF_RETS (AP_FUNDEF (arg_node)))
                       && !ARG_ISREFCOUNTED (funargs)
                       && !ARG_WASREFERENCE (funargs)) {
                INFO_MODE (arg_info) = rc_prfuse;
            } else if (FUNDEF_ISCUDALACFUN (AP_FUNDEF (arg_node))
                       && INFO_FUNDEF (arg_info) != AP_FUNDEF (arg_node)) {
                /**
                 * Because cuda lac fun does not do refcount on its own,
                 * the arguments need to be treated as prf arguments.
                 */
                INFO_MODE (arg_info) = rc_prfuse;
            } else {
                INFO_MODE (arg_info) = rc_apuse;
            }

            EXPRS_EXPR (apargs) = TRAVdo (EXPRS_EXPR (apargs), arg_info);

            apargs = EXPRS_NEXT (apargs);
            if (funargs != NULL) {
                funargs = ARG_NEXT (funargs);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIprf (node *arg_node, info *arg_info)
 *
 * @brief Traverses a prf's arguments.
 *
 ******************************************************************************/
node *
RCIprf (node *arg_node, info *arg_info)
{
    node *lhs, *args;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    /**
     * alloc (dim, shp)
     * - Initialize rc with 1
     */
    case F_alloc:
    case F_alloc_or_reuse:
    case F_reshape_VxA:
    case F_alloc_or_reshape:
    case F_reuse:
    case F_alloc_or_resize:
    case F_resize:
        if (INFO_ASSIGN (arg_info) != NULL) {
            PRF_ARGS (arg_node) = TBmakeExprs (TBmakeNum (1),
                                               PRF_ARGS (arg_node));
        }
        /**
         * Traverse shape expression and reuse candidates
         * without corrupting the shape descriptor.
         */
        INFO_ASSIGN (arg_info) = NULL;
        INFO_MODE (arg_info) = rc_prfuse;
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        break;

    /**
     * type_conv (type, a);
     * - type must not be traversed as it is a N_type node
     * - a must be counted like a funap use of a
     */
    case F_type_error:
    case F_guard_error:
    case F_dispatch_error:
    case F_type_conv:
    case F_type_fix:
        INFO_MODE (arg_info) = rc_apuse;
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        break;

    /**
     * fill (expr, a);
     * - expr must be traversed
     * - a must be counted like a funap use of a
     */
    case F_fill:
        INFO_MODE (arg_info) = rc_prfuse;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
        INFO_MODE (arg_info) = rc_apuse;
        PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);
        break;

    /**
     * wl_break (v, m, iv)
     * - Traverse value v only
     */
    case F_wl_break:
        INFO_MODE (arg_info) = rc_prfuse;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
        break;

    /**
     * wl_assign (v, m, iv, idx)
     * - Traverse value v and idx
     */
    case F_wl_assign:
        INFO_MODE (arg_info) = rc_prfuse;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
        PRF_ARG4 (arg_node) = TRAVdo (PRF_ARG4 (arg_node), arg_info);
        break;

    /**
     * cond_wl_assign (cond, shmemidx, shmem, devidx, devmem)
     */
    case F_cond_wl_assign:
        INFO_MODE (arg_info) = rc_prfuse;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
        PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);
        PRF_ARG3 (arg_node) = TRAVdo (PRF_ARG3 (arg_node), arg_info);
        PRF_ARG4 (arg_node) = TRAVdo (PRF_ARG4 (arg_node), arg_info);
        if (ID_SSAASSIGN (PRF_ARG6 (arg_node)) != NULL) {
            PRF_ARG6 (arg_node) = TRAVdo (PRF_ARG6 (arg_node), arg_info);
        }
        break;

    /**
     * prop_obj_out (a)
     * - a must be counted like a funap use of a
     */
    case F_prop_obj_out:
        INFO_MODE (arg_info) = rc_apuse;
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        break;

    /**
     * x1', .., xn' = guard (x1, .., xn, t1, .., tn, p1, .., pm)
     * - Traverse xi as app since they are aliased into xi'
     * - Types ti must not be traversed as they are N_type nodes
     * - Traverse pj as prf
     */
    case F_guard:
        lhs = ASSIGN_LHS (INFO_ASSIGN (arg_info));
        args = PRF_ARGS (arg_node);

        // Traverse xi as app
        INFO_MODE (arg_info) = rc_apuse;
        while (lhs != NULL) {
            EXPRS_EXPR (args) = TRAVdo (EXPRS_EXPR (args), arg_info);
            lhs = IDS_NEXT (lhs);
            args = EXPRS_NEXT (args);
        }

        // Skip ti as they are N_type nodes
        args = TCgetNthExprs (PRF_NUMVARIABLERETS (arg_node), args);

        // Traverse remaining pj as prf
        INFO_MODE (arg_info) = rc_prfuse;
        while (args != NULL) {
            EXPRS_EXPR (args) = TRAVdo (EXPRS_EXPR (args), arg_info);
            args = EXPRS_NEXT (args);
        }

        break;

    /**
     * v = unshare (a, iv1, .., ivn)
     * - Traverse a like app since a is aliased into v
     * - Do not traverse iv.
     */
    case F_unshare:
        INFO_MODE (arg_info) = rc_apuse;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
        INFO_MODE (arg_info) = rc_prfuse;
        break;

    /**
     * a_mem = suballoc (mem, idx)
     * - Initialize rc with 1
     */
    case F_suballoc:
        if (INFO_ASSIGN (arg_info) != NULL) {
            PRF_ARGS (arg_node) = TBmakeExprs (TBmakeNum (1),
                                               PRF_ARGS (arg_node));
        }
        /**
         * Do not visit the memory variable (to pretend its only
         * referenced once) but the index!
         */
        INFO_MODE (arg_info) = rc_prfuse;
        PRF_EXPRS3 (arg_node) = TRAVopt (PRF_EXPRS3 (arg_node), arg_info);
        break;

    case F_syncout:
    case F_syncin:
        DBUG_ASSERT (TCcountExprs (PRF_ARGS (arg_node)) == 1,
                     "_sync{out,in}_ should have 1 argument in this phase");
        INFO_MODE (arg_info) = rc_apuse;
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
        break;

    case F_enclose:
        INFO_MODE (arg_info) = rc_apuse;
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
        break;

    case F_disclose:
        INFO_MODE (arg_info) = rc_prfuse;
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
        lhs = ASSIGN_LHS (INFO_ASSIGN (arg_info));
        INFO_POSTASSIGN (arg_info) = AdjustRC (IDS_AVIS (lhs), 1,
                                               INFO_POSTASSIGN (arg_info));
        break;

    case F_prefetch2device:
    case F_prefetch2host:
        INFO_MODE (arg_info) = rc_apuse;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
        INFO_MODE (arg_info) = rc_prfuse;
        break;

    /**
     * The first argument is an alias, as such we do no reference counting
     * of this N_id, the second argument does need to be reference counted.
     */
    case F_host2device_end:
    case F_device2host_end:
        INFO_MODE (arg_info) = rc_apuse;
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
        INFO_MODE (arg_info) = rc_prfuse;
        PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);
        break;

    /**
     * Do not visit any of the arguments
     */
    case F_accu:
    case F_UAR_dummy_type:
    case F_prop_obj_in:
    case F_noop:
        break;

    default:
        INFO_MODE (arg_info) = rc_prfuse;
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIarray (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
RCIarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_MODE (arg_info) = rc_prfuse;

    ARRAY_AELEMS (arg_node) = TRAVopt (ARRAY_AELEMS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIwiths (node *arg_node, info *arg_info)
 *
 * @brief Traverse all with-loops, but keep the current with-loop's post
 * assignments from ending up inside the next with-loop.
 *
 ******************************************************************************/
node *
RCIwiths (node *arg_node, info *arg_info)
{
    node *postassigns;

    DBUG_ENTER ();

    WITHS_WITH (arg_node) = TRAVdo (WITHS_WITH (arg_node), arg_info);

    postassigns = INFO_POSTASSIGN (arg_info);
    INFO_POSTASSIGN (arg_info) = NULL;

    INFO_INWITHS (arg_info) = TRUE;
    WITHS_NEXT (arg_node) = TRAVopt (WITHS_NEXT (arg_node), arg_info);

    INFO_POSTASSIGN (arg_info) = TCappendAssign (postassigns,
                                                 INFO_POSTASSIGN (arg_info));
    INFO_INWITHS (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIwith (node *arg_node, info *arg_info)
 *
 * @brief Traverses a withloop and thereby allocates memory for the index
 * variables and the result.
 *
 ******************************************************************************/
node *
RCIwith (node *arg_node, info *arg_info)
{
    node *avis;
    bool old_inwiths;

    DBUG_ENTER ();

    INFO_WITHMASK (arg_info) = DFMgenMaskClear (INFO_MASKBASE (arg_info));

    if (WITH_CODE (arg_node) != NULL) {
        /**
         * Nested with-loops have to be fully traversed, regardless of being
         * inside a hybrid with-loop. We thus set the corresponding flag to
         * false, and restore it later.
         */
        old_inwiths = INFO_INWITHS (arg_info);
        INFO_INWITHS (arg_info) = FALSE;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_MUSTCOUNT (arg_info) = TRUE;
        INFO_INWITHS (arg_info) = old_inwiths;
    }

    // Consume all variables used inside the with-loop
    avis = DFMgetMaskEntryAvisSet (INFO_WITHMASK (arg_info));
    while (avis != NULL) {
        // Add one to the environment and create a dec_rc
        if (!CUisShmemTypeNew (AVIS_TYPE (avis))) {
            IncNum (INFO_ENV (arg_info), avis, 1);
            INFO_POSTASSIGN (arg_info) = AdjustRC (avis, -1,
                                                   INFO_POSTASSIGN (arg_info));
        }

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    INFO_WITHMASK (arg_info) = DFMremoveMask (INFO_WITHMASK (arg_info));

    /**
     * In AKD-IV-Withloops, the IV is always needed except in the case
     * of CUDA WLs, where the IV is anyway lifted out.
     */
    INFO_WITHVECNEEDED (arg_info) = !WITH_CUDARIZABLE (arg_node);
    WITH_WITHID (arg_node) = TRAVdo (WITH_WITHID (arg_node), arg_info);

    INFO_MODE (arg_info) = rc_prfuse;
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    // For all with-loops in WITHS nodes but the first, we skip the withops
    if (!INFO_INWITHS (arg_info)) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIwith2 (node *arg_node, info *arg_info)
 *
 * @brief Traverses a withloop and thereby allocates memory for the index
 * variables and the result.
 *
 ******************************************************************************/
node *
RCIwith2 (node *arg_node, info *arg_info)
{
    node *avis;
    bool old_inwiths;

    DBUG_ENTER ();

    INFO_WITHMASK (arg_info) = DFMgenMaskClear (INFO_MASKBASE (arg_info));

    if (WITH2_CODE (arg_node) != NULL) {
        /**
         * Nested with-loops have to be fully traversed, regardless of being
         * inside a hybrid with-loop. We thus set the corresponding flag to
         * false, and restore it later.
         */
        old_inwiths = INFO_INWITHS (arg_info);
        INFO_INWITHS (arg_info) = FALSE;
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        INFO_MUSTCOUNT (arg_info) = TRUE;
        INFO_INWITHS (arg_info) = old_inwiths;
    }

    // Consume all Variables used inside the with-loop
    avis = DFMgetMaskEntryAvisSet (INFO_WITHMASK (arg_info));
    while (avis != NULL) {
        // Add one to the environment and create a dec_rc
        IncNum (INFO_ENV (arg_info), avis, 1);
        INFO_POSTASSIGN (arg_info) = AdjustRC (avis, -1,
                                               INFO_POSTASSIGN (arg_info));

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    INFO_WITHMASK (arg_info) = DFMremoveMask (INFO_WITHMASK (arg_info));

    // In with2-loops (AKS-IV), the index vector is not always required
    INFO_WITHVECNEEDED (arg_info) = FALSE;
    WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);

    INFO_MODE (arg_info) = rc_prfuse;
    WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);

    // For all with-loops in WITHS nodes but the first, we skip the withops
    if (!INFO_INWITHS (arg_info)) {
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIwith3 (node *arg_node, info *arg_info)
 *
 * @brief Traverses a with3-loop and counts references in its body.
 *
 ******************************************************************************/
node *
RCIwith3 (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    INFO_WITHMASK (arg_info) = DFMgenMaskClear (INFO_MASKBASE (arg_info));

    if (WITH3_RANGES (arg_node) != NULL) {
        WITH3_RANGES (arg_node) = TRAVdo (WITH3_RANGES (arg_node), arg_info);
        INFO_MUSTCOUNT (arg_info) = TRUE;
    }

    // Consume all variables used inside the with-loop
    avis = DFMgetMaskEntryAvisSet (INFO_WITHMASK (arg_info));
    while (avis != NULL) {
        // Add one to the environment and create a dec_rc
        IncNum (INFO_ENV (arg_info), avis, 1);
        INFO_POSTASSIGN (arg_info) =
            AdjustRC (avis, -1, INFO_POSTASSIGN (arg_info));

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    INFO_WITHMASK (arg_info) = DFMremoveMask (INFO_WITHMASK (arg_info));

    INFO_MODE (arg_info) = rc_prfuse;
    WITH3_OPERATIONS (arg_node) = TRAVdo (WITH3_OPERATIONS (arg_node),
                                          arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIcode (node *arg_node, info *arg_info)
 *
 * @brief Traverses a with-loop's code and inserts ADJUST_RCs at the
 * beginning of the code block.
 *
 ******************************************************************************/
node *
RCIcode (node *arg_node, info *arg_info)
{
    node *avis;
    dfmask_t *withmask;
    nlut_t *old_env;

    DBUG_ENTER ();

    withmask = INFO_WITHMASK (arg_info);
    INFO_WITHMASK (arg_info) = NULL;

    old_env = INFO_ENV (arg_info);
    INFO_ENV (arg_info) = NLUTgenerateNlutFromNlut (old_env);

    // Traverse CEXPRS like funaps
    INFO_MODE (arg_info) = rc_apuse;
    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    // Mark the variable as used in the outer context
    avis = NLUTgetNonZeroAvis (INFO_ENV (arg_info));
    while (avis != NULL) {
        DFMsetMaskEntrySet (withmask, avis);
        avis = NLUTgetNonZeroAvis (NULL);
    }

    // Prepend block with INC_RC statements
    BLOCK_ASSIGNS (CODE_CBLOCK (arg_node))
      = TCappendAssign (MakeRCAssignments (INFO_ENV (arg_info)),
                        BLOCK_ASSIGNS (CODE_CBLOCK (arg_node)));

    INFO_WITHMASK (arg_info) = withmask;
    INFO_ENV (arg_info) = NLUTremoveNlut (INFO_ENV (arg_info));
    INFO_ENV (arg_info) = old_env;

    // Count the references in next code
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIrange (node *arg_node, info *arg_info)
 *
 * @brief Traverses a with3-loop's range and inserts ADJUST_RCs at the
 * beginning of the body block.
 *
 ******************************************************************************/
node *
RCIrange (node *arg_node, info *arg_info)
{
    node *avis;
    dfmask_t *withmask;
    nlut_t *old_env;

    DBUG_ENTER ();

    withmask = INFO_WITHMASK (arg_info);
    INFO_WITHMASK (arg_info) = NULL;

    old_env = INFO_ENV (arg_info);
    INFO_ENV (arg_info) = NLUTgenerateNlutFromNlut (old_env);

    // Traverse CEXPRS like funaps
    INFO_MODE (arg_info) = rc_apuse;
    RANGE_RESULTS (arg_node) = TRAVdo (RANGE_RESULTS (arg_node), arg_info);
    RANGE_BODY (arg_node) = TRAVdo (RANGE_BODY (arg_node), arg_info);

    // Mark the variable as used in the outer context
    avis = NLUTgetNonZeroAvis (INFO_ENV (arg_info));
    while (avis != NULL) {
        DFMsetMaskEntrySet (withmask, avis);
        avis = NLUTgetNonZeroAvis (NULL);
    }

    // Prepend block with INC_RC statements
    BLOCK_ASSIGNS (RANGE_BODY (arg_node))
      = TCappendAssign (MakeRCAssignments (INFO_ENV (arg_info)),
                        BLOCK_ASSIGNS (RANGE_BODY (arg_node)));

    INFO_WITHMASK (arg_info) = withmask;
    INFO_ENV (arg_info) = NLUTremoveNlut (INFO_ENV (arg_info));
    INFO_ENV (arg_info) = old_env;

    // Count the references in next code
    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    // Finally count the lowerbound, upperbound and chunksize in prf mode
    INFO_MODE (arg_info) = rc_prfuse;
    RANGE_LOWERBOUND (arg_node) = TRAVdo (RANGE_LOWERBOUND (arg_node), arg_info);
    RANGE_UPPERBOUND (arg_node) = TRAVdo (RANGE_UPPERBOUND (arg_node), arg_info);
    RANGE_CHUNKSIZE (arg_node) = TRAVopt (RANGE_CHUNKSIZE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIwithid (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
RCIwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_MODE (arg_info) = rc_prfuse;
    WITHID_IDS (arg_node) = TRAVopt (WITHID_IDS (arg_node), arg_info);
    WITHID_IDXS (arg_node) = TRAVopt (WITHID_IDXS (arg_node), arg_info);

    if (INFO_WITHVECNEEDED (arg_info)) {
        WITHID_VEC (arg_node) = TRAVdo (WITHID_VEC (arg_node), arg_info);
    }

    WITHID_VECNEEDED (arg_node) =
        NLUTgetNum (INFO_ENV (arg_info), ID_AVIS (WITHID_VEC (arg_node))) > 0;

    if (!WITHID_VECNEEDED (arg_node)) {
        DBUG_PRINT ("Index vector %s will not be built!\n",
                    ID_NAME (WITHID_VEC (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIgenarray (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
RCIgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /**
     * genarray (shp, def, mem)
     * - shp, def must be refcounted like a prf use
     * - mem must be refcounted like a funap use
     */
    INFO_MODE (arg_info) = rc_prfuse;
    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    GENARRAY_DEFAULT (arg_node) = TRAVopt (GENARRAY_DEFAULT (arg_node), arg_info);

    GENARRAY_DEFSHAPEEXPR (arg_node) =
        TRAVopt (GENARRAY_DEFSHAPEEXPR (arg_node), arg_info);

    INFO_MODE (arg_info) = rc_apuse;
    GENARRAY_MEM (arg_node) = TRAVdo (GENARRAY_MEM (arg_node), arg_info);

    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCImodarray (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
RCImodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /**
     * modarray (A, mem)
     * - A must be refcounted like a prf use
     * - mem must be refcounted like a funap use
     */
    INFO_MODE (arg_info) = rc_prfuse;
    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);
    INFO_MODE (arg_info) = rc_apuse;
    MODARRAY_MEM (arg_node) = TRAVdo (MODARRAY_MEM (arg_node), arg_info);

    MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIfold (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
RCIfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /**
     * fold (op, n)
     * - op is not a variable
     * - n must be refcounted like a funap use
     *
     * fold (op, de, in)
     * - op is not a variable
     * - de is not used at run time
     * - in must be refcounted like a funap use
     */
    INFO_MODE (arg_info) = rc_apuse;
    if (FOLD_INITIAL (arg_node) == NULL) {
        FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);
    } else {
        FOLD_INITIAL (arg_node) = TRAVopt (FOLD_INITIAL (arg_node), arg_info);
    }

    if (FOLD_PARTIALMEM (arg_node) != NULL) {
        INFO_MODE (arg_info) = rc_apuse;
        FOLD_PARTIALMEM (arg_node) = TRAVdo (FOLD_PARTIALMEM (arg_node), arg_info);
    }

    FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIpropagate (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
RCIpropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /**
     * propagate (n)
     * - n must be refcounted like a funap use
     */
    INFO_MODE (arg_info) = rc_apuse;
    PROPAGATE_DEFAULT (arg_node) = TRAVdo (PROPAGATE_DEFAULT (arg_node), arg_info);

    PROPAGATE_NEXT (arg_node) = TRAVopt (PROPAGATE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIfuncond (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
RCIfuncond (node *arg_node, info *arg_info)
{
    node *lhs;
    int n;

    DBUG_ENTER ();

    if (INFO_ENV2 (arg_info) == NULL) {
        INFO_ENV2 (arg_info) = NLUTduplicateNlut (INFO_ENV (arg_info));
    }

    lhs = ASSIGN_LHS (INFO_ASSIGN (arg_info));
    n = NLUTgetNum (INFO_ENV (arg_info), IDS_AVIS (lhs));
    NLUTsetNum (INFO_ENV (arg_info), IDS_AVIS (lhs), 0);
    NLUTsetNum (INFO_ENV2 (arg_info), IDS_AVIS (lhs), 0);

    IncNum (INFO_ENV (arg_info), ID_AVIS (FUNCOND_THEN (arg_node)), n);
    IncNum (INFO_ENV2 (arg_info), ID_AVIS (FUNCOND_ELSE (arg_node)), n);

    INFO_MUSTCOUNT (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RCIcond (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
RCIcond (node *arg_node, info *arg_info)
{
    nlut_t *nzlut, *env;
    node *avis;

    DBUG_ENTER ();

    if (INFO_ENV2 (arg_info) == NULL) {
        INFO_ENV2 (arg_info) = NLUTduplicateNlut (INFO_ENV (arg_info));
    }

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    INFO_MODE (arg_info) = rc_prfuse;
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THENASSIGNS (arg_node) = TCappendAssign (INFO_POSTASSIGN (arg_info),
                                                  COND_THENASSIGNS (arg_node));
    INFO_POSTASSIGN (arg_info) = NULL;

    env = INFO_ENV (arg_info);
    INFO_ENV (arg_info) = INFO_ENV2 (arg_info);

    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    INFO_MODE (arg_info) = rc_prfuse;
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_ELSEASSIGNS (arg_node) = TCappendAssign (INFO_POSTASSIGN (arg_info),
                                                  COND_ELSEASSIGNS (arg_node));
    INFO_POSTASSIGN (arg_info) = NULL;

    INFO_ENV (arg_info) = env;

    env = NLUTgenerateNlutFromNlut (env);

    nzlut = NLUTaddNluts (INFO_ENV (arg_info), INFO_ENV2 (arg_info));

    avis = NLUTgetNonZeroAvis (nzlut);
    while (avis != NULL) {
        int t, e, m;

        t = NLUTgetNum (INFO_ENV (arg_info), avis);
        e = NLUTgetNum (INFO_ENV2 (arg_info), avis);

        if ((t == 0) || (e == 0)) {
            m = e > t ? e : t;
        } else {
            m = e < t ? e : t;
        }

        NLUTsetNum (INFO_ENV (arg_info), avis, t - m);
        NLUTsetNum (INFO_ENV2 (arg_info), avis, e - m);
        NLUTsetNum (env, avis, m);

        avis = NLUTgetNonZeroAvis (NULL);
    }

    nzlut = NLUTremoveNlut (nzlut);

    COND_THENASSIGNS (arg_node) =
        TCappendAssign (MakeRCAssignments (INFO_ENV (arg_info)),
                        COND_THENASSIGNS (arg_node));
    COND_ELSEASSIGNS (arg_node) =
        TCappendAssign (MakeRCAssignments (INFO_ENV2 (arg_info)),
                        COND_ELSEASSIGNS (arg_node));

    INFO_ENV2 (arg_info) = NLUTremoveNlut (INFO_ENV2 (arg_info));
    INFO_ENV (arg_info) = NLUTremoveNlut (INFO_ENV (arg_info));
    INFO_ENV (arg_info) = env;

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
