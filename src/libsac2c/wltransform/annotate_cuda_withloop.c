/*****************************************************************************
 *
 * @defgroup
 *
 *
 * description:
 *   This module annotates N_with which can be executed on the
 *   CUDA device in parallel. Currently, cudarizable N_withs are
 *   limited as follows:
 *     1) A N_with must not contain any inner N_withs.
 *     2) Fold N_with is currently not supported.
 *     3) Function applications are not allowed in a cudarizable
 *        N_with except primitive mathematical functions.
 *
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file annotate_cuda_withloop.c
 *
 * Prefix: ACUWL
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
#include "DupTree.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool inwl;
    bool cudarizable;
    node *letids;
    /* bool init_from_ap; */
    node *fundef;
    bool keep_inner;
};

#define INFO_INWL(n) (n->inwl)
#define INFO_CUDARIZABLE(n) (n->cudarizable)
#define INFO_LETIDS(n) (n->letids)
/* #define INFO_INIT_FROM_AP(n)   (n->init_from_ap) */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_KEEP_INNER(n) (n->keep_inner)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_INWL (result) = FALSE;
    INFO_CUDARIZABLE (result) = TRUE;
    INFO_LETIDS (result) = NULL;
    /* INFO_INIT_FROM_AP(result) = FALSE; */
    INFO_FUNDEF (result) = NULL;
    INFO_KEEP_INNER (result) = FALSE;

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
 * @fn node *ACUWLdoAnnotateCUDAWL( node *syntax_tree)
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
 * @fn node *ACUWLfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ACUWLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLfundef");

    /* Need to find out why the function must not be sticky  */
    if (!FUNDEF_ISSTICKY (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLlet");

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLwith (node *arg_node, info *arg_info)
{
    bool old_cudarizable;
    node *avis;

    DBUG_ENTER ("ACUWLwith");

    if (!INFO_INWL (arg_info)) {
        INFO_INWL (arg_info) = TRUE;
        INFO_CUDARIZABLE (arg_info) = TRUE;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_INWL (arg_info) = FALSE;
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        /* We only cudarize AKS N_with */
        avis = IDS_AVIS (INFO_LETIDS (arg_info));
        WITH_CUDARIZABLE (arg_node)
          = TYisAKS (AVIS_TYPE (avis)) && INFO_CUDARIZABLE (arg_info);
    } else {
        /* The inner N_with makes the outer N_with uncudarizable */
        old_cudarizable = INFO_CUDARIZABLE (arg_info);
        INFO_CUDARIZABLE (arg_info) = TRUE;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        if (INFO_KEEP_INNER (arg_info)) {
            WITH_CUDARIZABLE (arg_node) = FALSE;
            INFO_KEEP_INNER (arg_info) = FALSE;
            INFO_CUDARIZABLE (arg_info) = old_cudarizable;
        } else {
            /* We only cudarize AKS N_with */
            avis = IDS_AVIS (INFO_LETIDS (arg_info));
            WITH_CUDARIZABLE (arg_node)
              = TYisAKS (AVIS_TYPE (avis)) && INFO_CUDARIZABLE (arg_info);

            INFO_CUDARIZABLE (arg_info) = FALSE;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLfold( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLfold");

    /* An inner fold N_with doesn't stop the outer
     * N_with from being cudarzied. */
    if (!INFO_INWL (arg_info)) {
        printf ("Setting fold to FALSE\n");
        INFO_CUDARIZABLE (arg_info) = FALSE;
    } else {
        if (INFO_CUDARIZABLE (arg_info)) {
            INFO_KEEP_INNER (arg_info) = TRUE;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLbreak( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLbreak (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLbreak");

    /* Currently, we do not support break N_with */
    INFO_CUDARIZABLE (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLpropagate( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLpropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLpropagate");

    /* Currently, we do not support propagate N_with */
    INFO_CUDARIZABLE (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
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
 * @fn node *ACUWLid( node *arg_node, info *arg_info)
 *
 * @brief
 *
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
 * @fn node *ACUWLap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLap (node *arg_node, info *arg_info)
{
    namespace_t *ns;

    DBUG_ENTER ("ACUWLap");

    /* Found a function application within a N_with, not cudarizable. Also,
     * since we are in a phase after f2l, no LaC functions exist, so we
     * do not need to worry about them. */
    if (INFO_INWL (arg_info)) {
        /* The only function application allowed in a cudarizbale N_with
         * is mathematical functions. However, the check below is ugly
         * and a better way needs to be found. */
        ns = FUNDEF_NS (AP_FUNDEF (arg_node)); /* ns could be NULL */
        if (ns == NULL || !STReq (NSgetModule (ns), "Math")) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
        }
        AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
