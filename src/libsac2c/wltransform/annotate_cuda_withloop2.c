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
 * @file annotate_cuda_withloop2.c
 *
 * Prefix: ACUWL
 *
 *****************************************************************************/
#include "annotate_cuda_withloop2.h"

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
    node *fundef;
    bool from_ap;
};

#define INFO_INWL(n) (n->inwl)
#define INFO_CUDARIZABLE(n) (n->cudarizable)
#define INFO_LETIDS(n) (n->letids)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_FROM_AP(n) (n->from_ap)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_INWL (result) = FALSE;
    INFO_CUDARIZABLE (result) = TRUE;
    INFO_LETIDS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_FROM_AP (result) = FALSE;

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
    node *old_fundef;

    DBUG_ENTER ("ACUWLfundef");

    /* During the main traversal, we only look at non-lac functions */
    if (!FUNDEF_ISLACFUN (arg_node)) {
        /* Need to find out why the function must not be sticky */
        if (!FUNDEF_ISSTICKY (arg_node)) {
            INFO_FUNDEF (arg_info) = arg_node;
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
            INFO_FUNDEF (arg_info) = NULL;
        }
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        if (INFO_FROM_AP (arg_info)) {
            old_fundef = INFO_FUNDEF (arg_info);
            INFO_FUNDEF (arg_info) = arg_node;
            /* Traversal of lac functions is initiated from the calling site */
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
            INFO_FUNDEF (arg_info) = old_fundef;
        } else {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

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
    DBUG_ENTER ("ACUWLwith");

    INFO_CUDARIZABLE (arg_info) = TRUE;

    if (!INFO_INWL (arg_info)) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        INFO_INWL (arg_info) = TRUE;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_INWL (arg_info) = FALSE;

        /* We only cudarize AKS N_with */
        WITH_CUDARIZABLE (arg_node)
          = TYisAKS (AVIS_TYPE (IDS_AVIS (INFO_LETIDS (arg_info))))
            && INFO_CUDARIZABLE (arg_info);
    } else {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

        /*
            // We only cudarize AKS N_with
            WITH_CUDARIZABLE( arg_node) =
              TYisAKS( AVIS_TYPE( IDS_AVIS( INFO_LETIDS( arg_info)))) &&
              INFO_CUDARIZABLE( arg_info);

            // The inner N_with makes the outer N_with uncudarizable
            INFO_CUDARIZABLE( arg_info) = FALSE;
        */

        /* Since we only try to cudarize outermost N_with, any
         * inner N_with is tagged as not cudarizbale */
        WITH_CUDARIZABLE (arg_node) = FALSE;
        INFO_CUDARIZABLE (arg_info)
          = TYisAKS (AVIS_TYPE (IDS_AVIS (INFO_LETIDS (arg_info))))
            && INFO_CUDARIZABLE (arg_info);
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

    /* An outermost fold N_with is currently not cudarizable;
     * however, if the fold is within an N_with, we do not signal
     * uncudarizeable to the enclosing N_with. */
    if (!INFO_INWL (arg_info)) {
        INFO_CUDARIZABLE (arg_info) = FALSE;
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
 * @fn node *ACUWLid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLid (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ("ACUWLid");

    type = AVIS_TYPE (ID_AVIS (arg_node));

    if (INFO_INWL (arg_info)) {
        /* We do not cudarize any N_with which contains arrays
         * other than AKS arrays */
        if (!TYisScalar (type) && !TYisAKV (type) && !TYisAKS (type)) {
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
    node *fundef;
    namespace_t *ns;
    bool traverse_lac_fun, old_from_ap;

    DBUG_ENTER ("ACUWLap");

    fundef = AP_FUNDEF (arg_node);

    /* For us to traverse a function from calling site, it must be a
     * condictional function or a loop function and must not be the
     * recursive function call in the loop function. */
    traverse_lac_fun = (FUNDEF_ISLACFUN (fundef) && fundef != INFO_FUNDEF (arg_info));

    old_from_ap = INFO_FROM_AP (arg_info);
    INFO_FROM_AP (arg_info) = TRUE;

    /* If we are in an N_with, we need to distinguish between two different
     * N_ap: 1) Invocation to lac functions and 2) invocation to normal
     * functions. For the former, we traversal into the corresponding N_fundef
     * and for the later we need to check whether it prevents the current
     * N_with from being cudarized. */

    /*
      if( INFO_INWL( arg_info)) {
        if( traverse_lac_fun) {
           AP_FUNDEF( arg_node) = TRAVdo( AP_FUNDEF( arg_node), arg_info);
        }
        else {
          // The only function application allowed in a cudarizbale N_with
          // is mathematical functions. However, the check below is ugly
          // and a better way needs to be found.
          ns = FUNDEF_NS( AP_FUNDEF( arg_node));
          if( ns == NULL || !STReq( NSgetModule( ns), "Math")) {
            INFO_CUDARIZABLE( arg_info) = FALSE;
          }
        }
      }
      else {
        if( traverse_lac_fun) {
          AP_FUNDEF( arg_node) = TRAVdo( AP_FUNDEF( arg_node), arg_info);
        }
      }
    */

    if (traverse_lac_fun) {
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    } else if (INFO_INWL (arg_info)) {
        /* The only function application allowed in a cudarizbale N_with
         * is mathematical functions. However, the check below is ugly
         * and a better way needs to be found. */
        ns = FUNDEF_NS (AP_FUNDEF (arg_node)); /* ns could be NULL */
        if (ns == NULL || !STReq (NSgetModule (ns), "Math")) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
        }
    }

    /* We need to traverse N_ap arguments because they might
     * contain Non-AKS arrays */
    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    INFO_FROM_AP (arg_info) = old_from_ap;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
