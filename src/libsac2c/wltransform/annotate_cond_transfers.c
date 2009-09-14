/*****************************************************************************
 *
 *
 * file:   annotate_cond_transfers.c
 *
 * prefix: AMTRAN
 *
 * description:
 *   This module decides which <host2device> and <device2host>
 *   can be moved out of the enclosing do-fun. Since host<->device transfers
 *   are expensive operations to perform in CUDA programs, and transfers within
 *   loop make it even more severe, eliminating transfers within loops as
 *   much as possible is crucial to program performance. For detailed explanation
 *   of what transfers can be moved out and what cannot, please see commets
 *   in the code.
 *
 *****************************************************************************/

#include "annotate_cond_transfers.h"

#include <stdlib.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "dbug.h"
#include "traverse.h"
#include "memory.h"
#include "NumLookUpTable.h"
#include "cuda_utils.h"
#include "new_types.h"

enum traverse_mode { trav_collect, trav_annotate };

/*
 * INFO structure
 */
struct INFO {
    bool incondfun;
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

/*
 * INFO macros
 */

#define INFO_NLUT(n) (n->nlut)
#define INFO_TRAVMODE(n) (n->travmode)
#define INFO_FUNARGNUM(n) (n->funargnum)
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_INCONDFUN(n) (n->incondfun)
#define INFO_RECURSIVE_APARGS(n) (n->recursive_apargs)
#define INFO_INRECURSIVEAPARGS(n) (n->inrecursiveapargs)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LETIDS(n) (n->letids)
#define INFO_INFUNCOND(n) (n->infuncond)

/*
 * INFO functions
 */
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
    INFO_INCONDFUN (result) = FALSE;
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
 *
 * @fn
 *
 * @brief node *AMTRANdoAnnotateMemoryTransfers( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANdoAnnotateCondTransfers (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ("ACTRANdoAnnotateCondTransfers");

    info = MakeInfo ();
    TRAVpush (TR_actran);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *AMTRANfundef( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACTRANfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    /* We only traverse cond-fun. */
    if (FUNDEF_ISCONDFUN (arg_node)) {
        INFO_INCONDFUN (arg_info) = TRUE;
        /* First traversal, collect variable usage information */
        INFO_NLUT (arg_info)
          = NLUTgenerateNlut (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
        INFO_TRAVMODE (arg_info) = trav_collect;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        /* Second traversal, annotate <host2device> and <device2host> */
        INFO_TRAVMODE (arg_info) = trav_annotate;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_NLUT (arg_info) = NLUTremoveNlut (INFO_NLUT (arg_info));
        INFO_INCONDFUN (arg_info) = FALSE;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *ACTRANassign( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACTRANassign");

    INFO_LASTASSIGN (arg_info) = arg_node;
    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *AMTRANlet( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACTRANlet");

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *AMTRANfuncond( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANfuncond (node *arg_node, info *arg_info)
{
    node *then_id, *else_id;
    ntype *then_type, *else_type;
    bool cond;
    node *then_ssaassign, *else_ssaassign;

    DBUG_ENTER ("ACTRANfuncond");

    /* N_funcond can only appear after the N_cond node in cond-fun
     * The N_funcond selects value from either branch of the N_cond
     * as the return value of the cond-fun.
     */
    /*
      if( INFO_INCONDFUN( arg_info)) {
        // Set flag to TRUE so that ACTRANid knows the current N_id
        // appears in a N_funcond node
        INFO_INFUNCOND( arg_info) = TRUE;
        FUNCOND_IF( arg_node) = TRAVdo( FUNCOND_IF( arg_node), arg_info);
        FUNCOND_THEN( arg_node) = TRAVdo( FUNCOND_THEN( arg_node), arg_info);
        FUNCOND_ELSE( arg_node) = TRAVdo( FUNCOND_ELSE( arg_node), arg_info);
        INFO_INFUNCOND( arg_info) = FALSE;
      }
    */
    if (INFO_INCONDFUN (arg_info)) {
        if (INFO_TRAVMODE (arg_info) == trav_annotate) {
            then_id = FUNCOND_THEN (arg_node);
            else_id = FUNCOND_ELSE (arg_node);
            DBUG_ASSERT ((NODE_TYPE (then_id) == N_id && NODE_TYPE (else_id) == N_id),
                         "N_funcond has non N_id node in either THEN or ELSE!");
            then_type = AVIS_TYPE (ID_AVIS (then_id));
            else_type = AVIS_TYPE (ID_AVIS (else_id));
            cond = !TYisAUD (then_type) && !TYisAUD (else_type)
                   && TYeqTypes (then_type, else_type);
            then_ssaassign = AVIS_SSAASSIGN (ID_AVIS (then_id));
            else_ssaassign = AVIS_SSAASSIGN (ID_AVIS (else_id));
            if (ISDEVICE2HOST (then_ssaassign) && !cond) {
                ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (then_ssaassign) = TRUE;
            }
            if (ISDEVICE2HOST (else_ssaassign) && !cond) {
                ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (else_ssaassign) = TRUE;
            }
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *AMTRANap( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACTRANap");

    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *ACTRANid( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACTRANid");

    if (INFO_INCONDFUN (arg_info)) {
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
            // if( !INFO_INFUNCOND( arg_info)) {
            NLUTincNum (INFO_NLUT (arg_info), ID_AVIS (arg_node), 1);
            //}
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *AMTRANprf( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACTRANprf (node *arg_node, info *arg_info)
{
    node *id;
    DBUG_ENTER ("ACTRANprf");

    if (INFO_INCONDFUN (arg_info)) {
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
                /* We only look at <host2device> whose host variable
                 * is passed in as an argument of the do-fun*/
                if (NODE_TYPE (ID_DECL (id)) == N_arg) {

                    /* If the reference count of the host variable is not 0,
                     * we annotates the transfer to be not allowed to be moved out.
                     */
                    if (NLUTgetNum (INFO_NLUT (arg_info), ID_AVIS (id)) != 0) {
                        ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_LASTASSIGN (arg_info))
                          = TRUE;
                    }
                }
                /* If the host variable is not passed as an argument to the do-fun,
                 * the <host2device> CANNOT be moved out of the do-fun.
                 */
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
                /* If the reference count of the host variable is not 0,
                 * we annotates the transfer to be not allowed to be moved out.
                 */
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
