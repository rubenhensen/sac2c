/*****************************************************************************
 *
 * @defgroup Annotate the memory transfers that are allowed to be
 *           lifted from a do-fun.
 *
 *
 *   This module decides which <host2device> and <device2host> can be
 *   lifted out of the enclosing do-fun. Since host<->device transfers
 *   are expensive operations to perform in CUDA programs, and transfers
 *   within loop make it even more severe, eliminating transfers within
 *   loops as much as possible is crucial to program performance. For
 *   detailed explanation of what transfers can be moved out and what
 *   cannot, please see commets in the code.
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file annotate_memory_transfers.c
 *
 * Prefix: AMTRAN
 *
 *****************************************************************************/
#include "annotate_memory_transfers.h"

#include <stdlib.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "dbug.h"
#include "traverse.h"
#include "memory.h"
#include "NumLookUpTable.h"
#include "cuda_utils.h"
#include "pattern_match.h"

/* Two traverse mode:
 *   tarv_collect: we collect variable usage information.
 *   trav_annoate: we annoate which memory transfer can be lifted out.
 */
enum traverse_mode { trav_collect, trav_consolidate, trav_annotate };

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

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
    DBUG_ENTER ("FreeInfo");

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
 * @fn node *AMTRANdoAnnotateMemoryTransfers( node *syntax_tree)
 *
 *****************************************************************************/
node *
AMTRANdoAnnotateMemoryTransfers (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ("AMTRANdoAnnotateMemoryTransfers");

    info = MakeInfo ();
    TRAVpush (TR_amtran);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *AMTRANfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
AMTRANfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AMTRANfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    /* We only traverse do-fun. */
    if (FUNDEF_ISDOFUN (arg_node)) {
        INFO_INDOFUN (arg_info) = TRUE;
        /* First traversal, collect variable usage information */
        INFO_NLUT (arg_info)
          = NLUTgenerateNlut (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
        INFO_TRAVMODE (arg_info) = trav_collect;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        INFO_TRAVMODE (arg_info) = trav_consolidate;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        /* Second traversal, annotate <host2device> and <device2host> */
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

/** <!--********************************************************************-->
 *
 * @fn node *AMTRANarg( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
AMTRANarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AMTRANarg");

    /* N_arg->linksign is reused here to assign each argument
     * of a do-fun a sequential number starting from 0.
     */
    ARG_LINKSIGN (arg_node) = INFO_FUNARGNUM (arg_info);
    INFO_FUNARGNUM (arg_info) += 1;

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AMTRANassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
AMTRANassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AMTRANassign");

    if (INFO_TRAVMODE (arg_info) == trav_consolidate) {
        /* Bottom-up traversal */
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);
    } else {
        INFO_LASTASSIGN (arg_info) = arg_node;
        ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AMTRANlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
AMTRANlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AMTRANlet");

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
         * In this case, wif we do nothing, than use count of a will
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

/** <!--********************************************************************-->
 *
 * @fn node *AMTRANfuncond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
AMTRANfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AMTRANfuncond");

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

/** <!--********************************************************************-->
 *
 * @fn node *AMTRANap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
AMTRANap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AMTRANap");

    if (INFO_INDOFUN (arg_info)) {
        /* If the N_ap is a recursive do-fun application
         * and the traverse mode is collect. */
        if (INFO_FUNDEF (arg_info) == AP_FUNDEF (arg_node)
            && INFO_TRAVMODE (arg_info) == trav_collect) {
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
        } else {
            AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AMTRANid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
AMTRANid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AMTRANid");

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
                NLUTincNum (INFO_NLUT (arg_info), ID_AVIS (arg_node), 1);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AMTRANprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
AMTRANprf (node *arg_node, info *arg_info)
{
    node *id;
    DBUG_ENTER ("AMTRANprf");

    if (INFO_INDOFUN (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_host2device:
            /* Ensure that each <host2device> is initially
             * tagged as can be moved out. */
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
                ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_LASTASSIGN (arg_info)) = FALSE;
            }
            /* If we are in trav_annotate traverse mode */
            if (INFO_TRAVMODE (arg_info) == trav_annotate) {
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
                            //! ISDEVICE2HOST( AVIS_SSAASSIGN( ID_AVIS( ap_arg)))) {
                            !PMmatchFlat (pat, ap_arg)) {
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
                    /*
                                if( ISDEVICE2HOST( AVIS_SSAASSIGN( ID_AVIS( ap_arg))) &&
                                    ASSIGN_ISNOTALLOWEDTOBEMOVEDUP( INFO_LASTASSIGN(
                       arg_info))) { ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN( AVIS_SSAASSIGN(
                       ID_AVIS( ap_arg))) = TRUE;
                                }
                    */
                    pattern *pat;
                    pat = PMprf (1, PMAisPrf (F_device2host), 1, PMvar (0, 0));

                    if (PMmatchFlat (pat, ap_arg)
                        && ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_LASTASSIGN (arg_info))) {
                        node *d2h_assign = AVIS_SSAASSIGN (ID_AVIS (ap_arg));
                        node *rhs = ASSIGN_RHS (d2h_assign);
                        while (NODE_TYPE (rhs) != N_prf) {
                            DBUG_ASSERT (NODE_TYPE (rhs) == N_id, "Non-id node found!");
                            d2h_assign = AVIS_SSAASSIGN (ID_AVIS (rhs));
                            rhs = ASSIGN_RHS (d2h_assign);
                        }
                        ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (d2h_assign) = TRUE;
                    }

                    pat = PMfree (pat);
                }
                /* If the host variable is not passed as an argument to the do-fun,
                 * the <host2device> CANNOT be moved out of the do-fun. */
                else {
                    ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_LASTASSIGN (arg_info)) = TRUE;
                }
            }
            break;
        case F_device2host:
            /* Ensure that each <device2host> is initially
             * tagged as can be moved out */
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
                ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (INFO_LASTASSIGN (arg_info)) = FALSE;
            }
            if (INFO_TRAVMODE (arg_info) == trav_annotate) {
                /* If the reference count of the host N_id is not 0,
                 * we annotates the transfer to be not allowed to be moved out. */
                if (NLUTgetNum (INFO_NLUT (arg_info), IDS_AVIS (INFO_LETIDS (arg_info)))
                    != 0) {
                    ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (INFO_LASTASSIGN (arg_info)) = TRUE;
                }
            }
        default:
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
