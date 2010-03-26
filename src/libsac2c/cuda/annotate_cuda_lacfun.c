/*****************************************************************************
 *
 *
 * file:   annotate_cuda_loop.c
 *
 * prefix: ACULAC
 *
 * description:
 *
 *
 *****************************************************************************/

#include "annotate_cuda_lacfun.h"

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

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool inlacfun;
    bool cudarizable;
    bool fromap;
    node *lastassign;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INLACFUN(n) (n->inlacfun)
#define INFO_CUDARIZABLE(n) (n->cudarizable)
#define INFO_FROMAP(n) (n->fromap)
#define INFO_LASTASSIGN(n) (n->lastassign)
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
    INFO_INLACFUN (result) = FALSE;
    INFO_CUDARIZABLE (result) = FALSE;
    INFO_FROMAP (result) = FALSE;
    INFO_LASTASSIGN (result) = NULL;

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
 * @brief node *ACULACdoAnnotateCUDALacfun( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACULACdoAnnotateCUDALacfun (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ACULACdoAnnotateCUDALacfun");

    info = MakeInfo ();
    TRAVpush (TR_aculac);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACULACfundef( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACULACfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACULACfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    /* If the function is not a lac-fun, we traverse as normal */
    if (!FUNDEF_ISLACFUN (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        if (INFO_FROMAP (arg_info)) {
            INFO_INLACFUN (arg_info) = TRUE;
            INFO_CUDARIZABLE (arg_info) = TRUE;
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            FUNDEF_ISCUDALACFUN (arg_node) = INFO_CUDARIZABLE (arg_info);
        } else {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACULACassign( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACULACassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACULACassign");

    INFO_LASTASSIGN (arg_info) = arg_node;
    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);
    INFO_LASTASSIGN (arg_info) = NULL;

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACULACap( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACULACap (node *arg_node, info *arg_info)
{
    node *ap_assign, *old_fundef, *ap_fundef = NULL;
    bool old_cudarizable, old_inlacfun;

    DBUG_ENTER ("ACULACap");

    ap_fundef = AP_FUNDEF (arg_node);

    /* If the application is to a lac-fun */
    if (ap_fundef != NULL && FUNDEF_ISLACFUN (ap_fundef)) {
        if (ap_fundef != INFO_FUNDEF (arg_info)) {
            /* We only look at the lac fun  if it can potentially be executed
             * in single thread in CUDA */
            ap_assign = INFO_LASTASSIGN (arg_info);

            /* If the mode of the ap N_assign is CUDA_DEVICE_SINGLE or this
             * N_ap is in a lac-fun, we traverse its body */
            if (ASSIGN_EXECMODE (ap_assign) == CUDA_DEVICE_SINGLE
                || INFO_INLACFUN (arg_info)) { /* Ugly condition!!! */
                /* Stack info */
                old_fundef = INFO_FUNDEF (arg_info);
                old_cudarizable = INFO_CUDARIZABLE (arg_info);
                old_inlacfun = INFO_INLACFUN (arg_info);

                printf ("Traversing Loop fun: %s\n", FUNDEF_NAME (ap_fundef));

                /* traverse the lac-fun fundef */
                INFO_FROMAP (arg_info) = TRUE;
                ap_fundef = TRAVopt (ap_fundef, arg_info);
                INFO_FROMAP (arg_info) = FALSE;

                INFO_CUDARIZABLE (arg_info)
                  = old_cudarizable && FUNDEF_ISCUDALACFUN (ap_fundef);

                /* If the lac fun is not cudarizbale, we tag the
                 * application of this lac fun as CUDA_HOST_SINGLE */
                if (!FUNDEF_ISCUDALACFUN (ap_fundef)) {
                    ASSIGN_EXECMODE (ap_assign) = CUDA_HOST_SINGLE;
                }

                /* Pop info */
                INFO_INLACFUN (arg_info) = old_inlacfun;
                INFO_FUNDEF (arg_info) = old_fundef;
            }
        }
    }
    /* If the normal function application is within a lacfun,
     * the current lacfun cannot be cudarized. */
    else if (INFO_INLACFUN (arg_info)) {
        INFO_CUDARIZABLE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACULACwith( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACULACwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACULACwith");

    /* A withloop within a do-fun will prevent this do-fun from being
     * cudarized, since not memory can be dynamically allocated within
     * a cuda kernel function. */
    if (INFO_INLACFUN (arg_info)) {
        INFO_CUDARIZABLE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACULACwith( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACULACwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACULACwith2");

    /* A withloop within a do-fun will prevent this do-fun from being
     * cudarized, since not memory can be dynamically allocated within
     * a cuda kernel function. */
    if (INFO_INLACFUN (arg_info)) {
        INFO_CUDARIZABLE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}
