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
#include "type_utils.h"
#include "shape.h"

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

static void
InitCudaBlockSizes ()
{
    DBUG_ENTER ("InitCudaBlockSizes");

    if (STReq (global.config.cuda_arch, "10") || STReq (global.config.cuda_arch, "11")) {
        global.optimal_threads = 256;
        global.optimal_blocks = 3;
        global.cuda_1d_block_large = 256;
        global.cuda_1d_block_small = 64;
        global.cuda_blocking_factor = 16;
        global.cuda_2d_block_x = 16;
        global.cuda_2d_block_y = 16;
    } else if (STReq (global.config.cuda_arch, "12")
               || STReq (global.config.cuda_arch, "13")) {
        global.optimal_threads = 256;
        global.optimal_blocks = 4;
        global.cuda_1d_block_large = 256;
        global.cuda_1d_block_small = 64;
        global.cuda_blocking_factor = 16;
        global.cuda_2d_block_x = 16;
        global.cuda_2d_block_y = 16;
    } else if (STReq (global.config.cuda_arch, "20")) {
        global.optimal_threads = 512;
        global.optimal_blocks = 3;
        global.cuda_1d_block_large = 512;
        global.cuda_1d_block_small = 64;
        global.cuda_blocking_factor = 32;
        global.cuda_2d_block_x = 32;
        global.cuda_2d_block_y = 16;
    } else {
        DBUG_ASSERT (FALSE, "Unknown CUDA architecture");
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravPart(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
ATravPart (node *arg_node, info *arg_info)
{
    int dim;

    DBUG_ENTER ("ATravPart");

    dim = TCcountIds (PART_IDS (arg_node));

    if (dim == 1) {
        PART_THREADBLOCKSHAPE (arg_node)
          = TBmakeArray (TYmakeSimpleType (T_int), SHcreateShape (1, dim),
                         TBmakeExprs (TBmakeNum (global.cuda_1d_block_large), NULL));
    } else if (dim == 2) {
        PART_THREADBLOCKSHAPE (arg_node)
          = TBmakeArray (TYmakeSimpleType (T_int), SHcreateShape (1, dim),
                         TBmakeExprs (TBmakeNum (global.cuda_2d_block_y),
                                      TBmakeExprs (TBmakeNum (global.cuda_2d_block_x),
                                                   NULL)));
    } else {
        /* For all other dimensionalities, we do not create TB shape info */
        PART_THREADBLOCKSHAPE (arg_node) = NULL;
    }

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

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

    InitCudaBlockSizes ();

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
        // if ( !FUNDEF_ISSTICKY( arg_node)) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
        //}
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }
    /* If the fundef is lac function, we check whether the traversal
     * is initiated from the calling site. */
    else {
        if (INFO_FROM_AP (arg_info)) {
            old_fundef = INFO_FUNDEF (arg_info);
            INFO_FUNDEF (arg_info) = arg_node;
            /* if the traversal is initiated from the calling site,
             * we traverse the body the function */
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
    ntype *ty;

    DBUG_ENTER ("ACUWLwith");

    ty = IDS_NTYPE (INFO_LETIDS (arg_info));

    INFO_CUDARIZABLE (arg_info) = TRUE;

    /* If the N_with is a top level withloop */
    if (!INFO_INWL (arg_info)) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        INFO_INWL (arg_info) = TRUE;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_INWL (arg_info) = FALSE;

        /* We only cudarize AKS N_with */
        WITH_CUDARIZABLE (arg_node)
          = (TYisAKS (ty) || TYisAKD (ty)) && INFO_CUDARIZABLE (arg_info);

        if (WITH_CUDARIZABLE (arg_node)) {
            anontrav_t atrav[2] = {{N_part, &ATravPart}, {0, NULL}};

            TRAVpushAnonymous (atrav, &TRAVsons);
            WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), NULL);
            TRAVpop ();
        }
    } else {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

        /* Since we only try to cudarize outermost N_with, any
         * inner N_with is tagged as not cudarizbale */
        WITH_CUDARIZABLE (arg_node) = FALSE;

        INFO_CUDARIZABLE (arg_info)
          = (TYisAKS (ty) || TYisAKD (ty)) && INFO_CUDARIZABLE (arg_info);
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
        if (!global.optimize.doscuf) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
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

    type = ID_NTYPE (arg_node);

    if (INFO_INWL (arg_info)) {
        /* We do not cudarize any N_with which contains arrays
         * other than AKS arrays */
        if (!TUisScalar (type) && !TYisAKV (type) && !TYisAKS (type) && !TYisAKD (type)) {
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
        if (!FUNDEF_ISLACFUN (fundef)) {
            ns = FUNDEF_NS (AP_FUNDEF (arg_node)); /* ns could be NULL */
            if (ns == NULL || !STReq (NSgetModule (ns), "Math")) {
                INFO_CUDARIZABLE (arg_info) = FALSE;
            }
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
