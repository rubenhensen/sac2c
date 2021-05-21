/**
 * @file
 * @defgroup amtran Annotate Memory Transfers
 * @ingroup cuda
 *
 * @brief Annotate the memory transfers that are allowed to be
 *        lifted from a do-fun.
 *
 * This module decides which <host2device> and <device2host> can be
 * lifted out of the enclosing do-fun. Since host<->device transfers
 * are expensive operations to perform in CUDA programs, and transfers
 * within loop make it even more severe, eliminating transfers within
 * loops as much as possible is crucial to program performance. For
 * detailed explanation of what transfers can be moved out and what
 * cannot, please see commets in the code.
 *
 * @{
 */
#include "annotate_memory_transfers.h"

#include <stdlib.h>
#include "tree_basic.h"
#include "tree_compound.h"

#define DBUG_PREFIX "AMTRAN"
#include "debug.h"

#include "traverse.h"
#include "memory.h"
#include "NumLookUpTable.h"
#include "cuda_utils.h"
#include "pattern_match.h"

/* Two traverse mode:
 *   tarv_collect:     we collect variable usage information.
 *   trav_consolidate: we annoate which memory transfer can be lifted out.
 *   trav_annoate:     we annoate which memory transfer can be lifted out.
 */
enum traverse_mode { trav_collect, trav_consolidate, trav_annotate };

/**
 * @name INFO structure
 * @{
 */
struct INFO {
    bool indofun;
    nlut_t *nlut;
    enum traverse_mode travmode;
    int funargnum;
    node *lastassign;
    node *recursive_apargs;
    bool inrecursiveapargs;
    bool infuncond;
    node *fundef;
    node *letids;
};

#define INFO_NLUT(n) (n->nlut)
#define INFO_TRAVMODE(n) (n->travmode)
#define INFO_FUNARGNUM(n) (n->funargnum)
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_INDOFUN(n) (n->indofun)
#define INFO_RECURSIVE_APARGS(n) (n->recursive_apargs)
#define INFO_INRECURSIVEAPARGS(n) (n->inrecursiveapargs)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LETIDS(n) (n->letids)
#define INFO_INFUNCOND(n) (n->infuncond)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_NLUT (result) = NULL;
    INFO_TRAVMODE (result) = trav_collect;
    INFO_FUNARGNUM (result) = 0;
    INFO_LASTASSIGN (result) = NULL;
    INFO_INDOFUN (result) = FALSE;
    INFO_RECURSIVE_APARGS (result) = NULL;
    INFO_INRECURSIVEAPARGS (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_INFUNCOND (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** @} */

/**
 * @brief Find fundef arguments in application arguments
 *
 * @param fundef_args
 * @param ap_args
 * @param id
 * @return matching fundef arguments
 */
static node *
GetFundefArgFromApArg (node *fundef_args, node *ap_args, node *id)
{
    DBUG_ENTER ();

    while (fundef_args != NULL) {
        if (EXPRS_EXPR (ap_args) == id)
            break;
        fundef_args = ARG_NEXT (fundef_args);
        ap_args = EXPRS_NEXT (ap_args);
    }

    DBUG_ASSERT (fundef_args != NULL, "No matching N_fundef arg found!");

    DBUG_RETURN (fundef_args);
}

/**
 * @name Entry functions
 * @{
 */

/**
 * @brief
 *
 * @param arg_node N_fundef
 * @param arg_info info structure
 * @return N_fundef
 */
node *
AMTRANdoAnnotateMemoryTransfers (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_amtran);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** @} */

/**
 * @name Traversal functions
 * @{
 */

/**
 * @brief
 *
 * @param arg_node N_fundef
 * @param arg_info info structure
 * @return N_fundef
 */
node *
AMTRANfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    /* We only traverse do-fun. */
    if (FUNDEF_ISLOOPFUN (arg_node)) {
        DBUG_PRINT ("(LOOP) Looking at %s...", FUNDEF_NAME (arg_node));
        INFO_INDOFUN (arg_info) = TRUE;
        INFO_NLUT (arg_info)
          = NLUTgenerateNlut (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));

        /* First traversal: collect variable usage information */
        INFO_TRAVMODE (arg_info) = trav_collect;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        /* Second traversal: */
        INFO_TRAVMODE (arg_info) = trav_consolidate;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        /* Third traversal: annotate host2device and device2host */
        INFO_TRAVMODE (arg_info) = trav_annotate;
        INFO_FUNARGNUM (arg_info) = 0;
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        INFO_NLUT (arg_info) = NLUTremoveNlut (INFO_NLUT (arg_info));
        INFO_RECURSIVE_APARGS (arg_info) = NULL;
        INFO_INDOFUN (arg_info) = FALSE;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_fundef
 * @param arg_info info structure
 * @return N_fundef
 */
node *
AMTRANarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* N_arg->linksign is reused here to assign each argument
     * of a do-fun a sequential number starting from 0. */
    ARG_LINKSIGN (arg_node) = INFO_FUNARGNUM (arg_info);
    INFO_FUNARGNUM (arg_info) += 1;

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_fundef
 * @param arg_info info structure
 * @return N_fundef
 */
node *
AMTRANassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_TRAVMODE (arg_info) == trav_consolidate) {
        /* Bottom-up traversal */
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    } else {
        INFO_LASTASSIGN (arg_info) = arg_node;
        ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_fundef
 * @param arg_info info structure
 * @return N_fundef
 */
node *
AMTRANlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_TRAVMODE (arg_info) == trav_consolidate) {
        /* If we have assignment such as A = B; and the use count
         * of A is 0, we set the use count of B also to 0 for the
         * following reason:
         *
         * e.g.  a = device2host( a_dev);
         *       ....
         *       b = a;
         *       .... (no use of b)
         *
         * In this case, if we do nothing, then the use count of a will
         * be greated than 0 and this may prevent lifting the device2host
         * However, since the rest of the program other than the recursive
         * loop fun application contains no reference to b, this device2host
         * may be lifted out as well. That's why we need to set 'a' and 'b'
         * use count to 0;
         */
        if (NODE_TYPE (LET_EXPR (arg_node)) == N_id
            && NLUTgetNum (INFO_NLUT (arg_info), IDS_AVIS (LET_IDS (arg_node))) == 0) {
            NLUTsetNum (INFO_NLUT (arg_info), ID_AVIS (LET_EXPR (arg_node)), 0);
        }
    } else {
        INFO_LETIDS (arg_info) = LET_IDS (arg_node);
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_fundef
 * @param arg_info info structure
 * @return N_fundef
 */
node *
AMTRANfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* N_funcond can only appear after the recurive do-fun application(?).
     * The N_funcond selects either the value returned from the
     * recursive do-fun application or a value defined in the do-fun body
     * as the return value of the enclosing do-fun. */
    if (INFO_INDOFUN (arg_info)) {
        /* Set flag to TRUE so that AMTRANid knows the current N_id
         * appears in a N_funcond node */
        INFO_INFUNCOND (arg_info) = TRUE;
        FUNCOND_IF (arg_node) = TRAVdo (FUNCOND_IF (arg_node), arg_info);
        FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);
        FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);
        INFO_INFUNCOND (arg_info) = FALSE;
    }
    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_fundef
 * @param arg_info info structure
 * @return N_fundef
 */
node *
AMTRANap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("inspecting N_ap of %s...", FUNDEF_NAME (AP_FUNDEF (arg_node)));

    if (INFO_INDOFUN (arg_info)) {
        /* If the N_ap is a recursive do-fun application
         * and the traverse mode is collect. */
        if (INFO_FUNDEF (arg_info) == AP_FUNDEF (arg_node)
            && INFO_TRAVMODE (arg_info) == trav_collect) {
            DBUG_PRINT ("(mode: collect), at recursive N_ap");
            /* The arguments of the recursive do-fun application
             * need to be stored and will be used in the annotate
             * traversal. */
            INFO_RECURSIVE_APARGS (arg_info) = AP_ARGS (arg_node);

            /* We indicate to AMTRANid that the current N_id it traverses
             * is actually an argument of the recursive do-fun application.
             * Therefore, it can decide whether to increment the reference
             * count of the variable or not, i.e. if the N_id is an argument
             * of the recursive do-fun application, we do NOT increment
             * it's reference count. */
            INFO_INRECURSIVEAPARGS (arg_info) = TRUE;
            AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
            INFO_INRECURSIVEAPARGS (arg_info) = FALSE;
        } else if (INFO_FUNDEF (arg_info) == AP_FUNDEF (arg_node)
                   && INFO_TRAVMODE (arg_info) == trav_annotate) {
            DBUG_PRINT ("(mode: annotate), at recursive N_ap");
            INFO_INRECURSIVEAPARGS (arg_info) = TRUE;
            AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
            INFO_INRECURSIVEAPARGS (arg_info) = FALSE;
        } else {
            AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
        }
    }
    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_fundef
 * @param arg_info info structure
 * @return N_fundef
 */
node *
AMTRANid (node *arg_node, info *arg_info)
{
    int nlut_num;

    DBUG_ENTER ();

    DBUG_PRINT ("inspecting N_id of %s...", ID_NAME (arg_node));

    if (INFO_INDOFUN (arg_info)) {
        if (INFO_TRAVMODE (arg_info) == trav_collect) {
            /* If the N_id is:
             * (1) In the argument list of the recursive do-fun
             *     application or
             * (2) In the N_funcond node
             * we do NOT increment its reference count. Otherwise,
             * we increments its reference count by one. This is
             * because of the following reasons:
             * a) For <host2device>, if the only reference to its
             *    host variable is an argument in the recursive
             *    application of the enclosing do-fun, it can still
             *    be moved out of the do-fun and the the arguments
             *    in N_ap and N_fundef can be replaced by the device
             *    variable.
             *    e.g.
             *
             *    loop_fun( ..., a_host, ...) {
             *      ... (No reference of a_host)
             *      b_dev = host2device( a_host);
             *      ... (No reference of a_host)
             *      loop_fun( ..., a_host ,...);
             *    }
             *
             *    ==>
             *
             *    b_dev = host2device( a_host);
             *    loop_fun( ..., b_dev , ...) {
             *      ... (No reference of a_host)
             *      ... (No reference of a_host)
             *      loop_fun( ..., b_dev ,...);
             *    }
             *
             *
             * b) For <device2host>, if the only reference to its
             *    host variable is in the N_funcond, it can be
             *    moved out of the do-fun
             *    e.g.
             *
             *    float[*] loop_fun( ...) {
             *      ...
             *      a_host = device2host( a_dev);
             *      ... (No reference of a_host)
             *      b_host = loop_fun( ... );
             *      c_host = cond ? b_host : a_host;
             *      return c_host;
             *    }
             *
             *    ==>
             *
             *    float_dev[*] loop_fun( ...) {
             *      ...
             *      ... (No reference of a_host)
             *      b_dev = loop_fun( ... );
             *      c_dev = cond ? b_dev : a_dev;
             *      return c_dev;
             *    }
             *    a_host = host2device( a_dev);
             */
            if (!INFO_INRECURSIVEAPARGS (arg_info) && !INFO_INFUNCOND (arg_info)) {
                DBUG_PRINT ("(mode: collect), adding %s to NLUT", ID_NAME (arg_node));
                NLUTincNum (INFO_NLUT (arg_info), ID_AVIS (arg_node), 1);
            }
        } else if (INFO_TRAVMODE (arg_info) == trav_annotate) {
            if (INFO_INRECURSIVEAPARGS (arg_info)
                && ISDEVICE2HOST (ID_SSAASSIGN (arg_node))) {
                node *fundef_args = FUNDEF_ARGS (INFO_FUNDEF (arg_info));
                node *ap_args = INFO_RECURSIVE_APARGS (arg_info);
                node *arg = GetFundefArgFromApArg (fundef_args, ap_args, arg_node);
                /* If the N_arg at correpsonding position cannot be
                 * replaced by its cuda counterpart, this devicetohost
                 * cannot be lifted */
                nlut_num = NLUTgetNum (INFO_NLUT (arg_info), ARG_AVIS (arg));
                if (nlut_num != 0) {
                    DBUG_PRINT ("(mode: annotate), N_avis %s found %d time, can not move done D2H", ID_NAME (arg_node), nlut_num);
                    ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (ID_SSAASSIGN (arg_node)) = TRUE;
                }
            }
        } else {
            /* Do nothing */
        }
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_fundef
 * @param arg_info info structure
 * @return N_fundef
 */
node *
AMTRANprf (node *arg_node, info *arg_info)
{
    node *id;
    DBUG_ENTER ();


    if (INFO_INDOFUN (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_host2device:
            DBUG_PRINT ("inspecting N_prf `F_host2device`");
            /* Ensure that each <host2device> is initially
             * tagged as can be moved out. */
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
                ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_LASTASSIGN (arg_info)) = FALSE;
            }
            /* If we are in trav_annotate traverse mode */
            if (INFO_TRAVMODE (arg_info) == trav_annotate) {
                DBUG_PRINT ("(mode: annoate), checking N_prf argument refcount");
                id = PRF_ARG1 (arg_node);

                /* We only look at <host2device> whose host N_id
                 * is passed in as an argument of the do-fun*/
                if (NODE_TYPE (ID_DECL (id)) == N_arg) {
                    /* We obtain the N_id in the recursive application argument list
                     * at the same position as that of the host variable in the N_fundef
                     * argument list. */
                    node *ap_arg = CUnthApArg (INFO_RECURSIVE_APARGS (arg_info),
                                               ARG_LINKSIGN (ID_DECL (id)));

                    /* If the reference count of the host N_id is not 0,
                     * we annotates the transfer to be not allowed to be moved out. */
                    if (NLUTgetNum (INFO_NLUT (arg_info), ID_AVIS (id)) != 0) {
                        DBUG_PRINT (" cannot move-out h2d of %s", ID_NAME (id));
                        ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_LASTASSIGN (arg_info))
                          = TRUE;
                    } else {
                        /* If the declaration of the N_ap argument is not the same as
                         * as that of the host variable (this can happen if the argument
                         * is a locally defined variable), the <host2device> can only be
                         * moved out if the N_ap argument is defined by a <device2host>
                         * i.e. its SSA assign is <device2host>: e.g.
                         *
                         *    loop_fun( *, a_host, *, *) {
                         *      ... (No reference of a_host)
                         *      b_dev = host2device( a_host);
                         *      ... (No reference of a_host)
                         *      c_host = device2host( c_dev);
                         *      loop_fun( *, c_host , *, *);
                         *    }
                         *
                         *    ==>
                         *
                         *    b_dev = host2device( a_host);
                         *    loop_fun( ..., b_dev , ...) {
                         *      ... (No reference of a_host)
                         *      ... (No reference of a_host)
                         *      c_host = device2host( c_dev);
                         *      loop_fun( ..., c_dev ,...);
                         *    }
                         */

                        pattern *pat;
                        pat = PMprf (1, PMAisPrf (F_device2host), 1, PMvar (0, 0));

                        if (ID_DECL (ap_arg) != ID_DECL (id) &&
                            /* This following two conditions are added to fix the bug
                             * occurs when compiling the wave kernel. The bug has to do
                             * with the fact that when an argument is passed to the loop
                             * recursive at a different position than its position in the
                             * loop function signature. However, this is a quick fix and
                             * needs to be investigated more carefully. */
                            (NODE_TYPE (ID_DECL (ap_arg)) != N_arg
                             || NLUTgetNum (INFO_NLUT (arg_info), ID_AVIS (ap_arg)) != 0)
                            && !PMmatchFlat (pat, ap_arg)) {
                            ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_LASTASSIGN (arg_info))
                              = TRUE;
                        }

                        pat = PMfree (pat);
                    }

                    /* If the N_ap argument is defined by <device2host> and the N_fundef
                     * argument at the same position must be a host variable, then the
                     * <device2host> CANNOT be move out of the do-fun.
                     *    e.g.
                     *
                     *    loop_fun( *, a_host, *, *) {
                     *      ... (Some reference of a_host)
                     *      b_dev = host2device( a_host); (CANNOT be moved out)
                     *      ... (Some reference of a_host)
                     *      c_host = device2host( c_dev); (CANNOT be moved out either)
                     *      loop_fun( *, c_host , *, *);
                     *    }
                     */
                    if (ISDEVICE2HOST (AVIS_SSAASSIGN (ID_AVIS (ap_arg)))) {
                        if (ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_LASTASSIGN (arg_info))) {
                            ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (
                              AVIS_SSAASSIGN (ID_AVIS (ap_arg)))
                              = TRUE;
                        } else {
                            /* If the host2device is allowed to be moved up and the
                             * ssaassign of the corresponding recursive ap arg is
                             * device2host, set the ap avis to te avis of the device
                             * variable in device2host. This is a fix to the bug found in
                             * kp1_trapezoidal_float.sac In that program, a host2device is
                             * allowed to be moved up and the corresponding device2host is
                             * not, this causes that the fundef has a dev arguemnt but the
                             * recursive ap still has a host arguemnt at the same
                             * position*/
                            ID_AVIS (ap_arg) = ID_AVIS (
                              PRF_ARG1 (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (ap_arg)))));
                        }
                    }
                }
                /* If the host variable is not passed as an argument to the do-fun,
                 * the <host2device> CANNOT be moved out of the do-fun. */
                else {
                    ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_LASTASSIGN (arg_info)) = TRUE;
                }
            }
            break;
        case F_device2host:
            DBUG_PRINT ("inspecting N_prf `F_device2host`");
            /* Ensure that each device2host is initially
             * tagged as can be moved out */
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
                ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (INFO_LASTASSIGN (arg_info)) = FALSE;
            }
            if (INFO_TRAVMODE (arg_info) == trav_annotate) {
                DBUG_PRINT ("(mode: annoate), checking N_prf argument refcount");
                /* If the reference count of the host N_id is not 0,
                 * we annotates the transfer to be not allowed to be moved out. */
                if (NLUTgetNum (INFO_NLUT (arg_info), IDS_AVIS (INFO_LETIDS (arg_info)))
                    != 0) {
                    ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (INFO_LASTASSIGN (arg_info)) = TRUE;
                }
            }
            break;
        default:
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

/** @} */
/** @} */
#undef DBUG_PREFIX
