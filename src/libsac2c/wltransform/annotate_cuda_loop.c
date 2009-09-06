/*****************************************************************************
 *
 *
 * file:   annotate_cuda_loop.c
 *
 * prefix: ACUL
 *
 * description:
 *
 *
 *****************************************************************************/

#include "annotate_cuda_loop.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"
#include "dbug.h"
#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "types.h"

enum traverse_mode { trav_normalfun, trav_dofun };

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool indofun;
    bool cudarizable;
    enum traverse_mode travmode;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INDOFUN(n) (n->indofun)
#define INFO_CUDARIZABLE(n) (n->cudarizable)
#define INFO_TRAVMODE(n) (n->travmode)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_INDOFUN (result) = FALSE;
    INFO_CUDARIZABLE (result) = FALSE;
    INFO_TRAVMODE (result) = trav_normalfun;

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
 * @brief node *ACUWLdoAnnotateCUDAWL( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACULdoAnnotateCUDALoop (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ACULdoAnnotateCUDALoop");

    info = MakeInfo ();
    TRAVpush (TR_acul);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACULfundef( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACULfundef (node *arg_node, info *arg_info)
{
    bool old_indofun;

    DBUG_ENTER ("ACULfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    /* If the function is not a do-fun, we traverse as normal */
    if (!FUNDEF_ISDOFUN (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        /* If the travers mode is dofun (which means this do-fun is reached via
         * the link of the corresponding N_ap, not the normal order of function
         * traversal), we traverse the do-fun; otherwise we traverse the next
         * function.
         */
        if (INFO_TRAVMODE (arg_info) == trav_dofun) {
            old_indofun = INFO_INDOFUN (arg_info);
            INFO_INDOFUN (arg_info) = TRUE;
            INFO_CUDARIZABLE (arg_info) = TRUE;
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            FUNDEF_ISCUDARIZABLE (arg_node) = INFO_CUDARIZABLE (arg_info);
            INFO_INDOFUN (arg_info) = old_indofun;
        } else {
            /* Traverse mode is not dofun, we skip the do fun */
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACULap( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACULap (node *arg_node, info *arg_info)
{
    node *old_fundef;
    bool old_cudarizable;
    enum traverse_mode old_mode;

    DBUG_ENTER ("ACULap");

    /* If the application is to a do-fun */
    if (FUNDEF_ISDOFUN (AP_FUNDEF (arg_node))) {
        /* If this is a recursive application of the enclosing do-fun */
        if (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info)) {

            /* Stack info */
            old_fundef = INFO_FUNDEF (arg_info);
            old_mode = INFO_TRAVMODE (arg_info);
            old_cudarizable = INFO_CUDARIZABLE (arg_info);

            /* traverse the do-fun fundef */
            INFO_TRAVMODE (arg_info) = trav_dofun;
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            INFO_CUDARIZABLE (arg_info)
              = old_cudarizable && FUNDEF_ISCUDARIZABLE (AP_FUNDEF (arg_node));

            /* Pop info */
            INFO_TRAVMODE (arg_info) = old_mode;
            INFO_FUNDEF (arg_info) = old_fundef;
        }
    }
    /* If the non-do-fun application is within a do-loop,
     * the current do-loop cannot be cudarized.
     */
    else if (INFO_INDOFUN (arg_info)) {
        INFO_CUDARIZABLE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACULgenarray( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACULgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACULgenarray");

    /* A genarray within a do-fun will prevent this do-fun from being
     * cudarized, since not memory can be dynamically allocated within
     * a cuda kernel function.
     */
    if (INFO_INDOFUN (arg_info)) {
        INFO_CUDARIZABLE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}
