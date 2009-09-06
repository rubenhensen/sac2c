/*****************************************************************************
 *
 *
 * file:   annotate_cuda_withloop.c
 *
 * prefix: ACUWL
 *
 * description:
 *   This module annotates N_withs which can be executed on the
 *   CUDA device in parallel. Currently, cudarizable N_withs are
 *   limited as follows:
 *     1) A N_with must not contain any inner N_withs. However,
 *        in the future, even outer the innermost N_withs should
 *        be cudarized
 *     2) Fold N_with is currently not supported.
 *     3) Function applications are not allowed in a cudarizable
 *        N_with except primitive mathematical functions.
 *
 *
 *****************************************************************************/

#include "annotate_cuda_withloop.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "globals.h"
#include "dbug.h"
#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "namespaces.h"
#include "new_types.h"

/*
 * INFO structure
 */
struct INFO {
    bool inwl;
    bool cudarizable;
};

/*
 * INFO macros
 */
#define INFO_INWL(n) (n->inwl)
#define INFO_CUDARIZABLE(n) (n->cudarizable)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_INWL (result) = FALSE;
    INFO_CUDARIZABLE (result) = TRUE;

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
ACUWLdoAnnotateCUDAWL (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ("ACUWLdoAnnotateCUDAWL");

    info = MakeInfo ();
    TRAVpush (TR_acuwl);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *ACUWLfundef( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACUWLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLfundef");

    /* Need to find out why the function must not be sticky */
    if (!FUNDEF_ISSTICKY (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACUWLwith( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACUWLwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLwith");

    if (!INFO_INWL (arg_info)) {
        INFO_INWL (arg_info) = TRUE;
        INFO_CUDARIZABLE (arg_info) = TRUE;
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_INWL (arg_info) = FALSE;
        WITH_CUDARIZABLE (arg_node) = INFO_CUDARIZABLE (arg_info);
    } else {
        /* If we are already in a N_with, the current N_with
         * is tagged as not cudarizable as its an inner N_with */
        WITH_CUDARIZABLE (arg_node) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACUWLfold( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACUWLfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLfold");

    /* Currently, we do not support cudaraizable fold */
    INFO_CUDARIZABLE (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACUWLcode( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACUWLcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLcode");

    if (INFO_INWL (arg_info)) {
        CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
        CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACUWLid( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACUWLid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("ACUWLid");

    avis = ID_AVIS (arg_node);

    if (INFO_INWL (arg_info)) {
        /* We do not cudarize AUD N_with */
        if (TYisAUD (AVIS_TYPE (avis)) || TYisAUDGZ (AVIS_TYPE (avis))) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief ACUWLap( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
ACUWLap (node *arg_node, info *arg_info)
{
    namespace_t *ns;

    DBUG_ENTER ("ACUWLap");

    /* Found a function application within a N_with, not cudarizable. Also,
     * since we are in a phase after f2l, no LaC functions exist, so we
     * do not need to worry about them.
     */
    if (INFO_INWL (arg_info)) {
        /* The only function application allowed in a cudarizbale N_with
         * is mathematical functions. However, the check below is ugly
         * and a better way needs to be found.
         */
        ns = FUNDEF_NS (AP_FUNDEF (arg_node)); /* ns could be NULL */
        if (ns == NULL || !STReq (NSgetModule (ns), "Math")) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
        }
    } else {
        AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
