/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup wlsc WLSCheck
 *
 * @brief checks whether all criteria necessary for applying With-Loop
 *        Scalarization are met.
 *
 * Example:
 *
 * <pre>
 * A = with (iv)
 *       ( lb_1 <= iv < ub_1) {
 *          block_1
 *       }: res_1,
 *       ...
 *       ( lb_n <= iv < ub_n) {
 *          block_n
 *       }: res_n,
 *     genarray( shp);
 * </pre>
 *
 * The following criteria must be met in order to apply With-Loop
 * Scalarization to the above with-loop:
 *
 * - The outer with-loop must be either genarray or modarray
 *
 * - res_1 ... res_n must all be AKS and must have equal shape
 *
 * - res_1 ... res_n must not be scalars
 *
 * - lb_i, ub_i must either be AKV N_id nodes OR N_array nodes.
 *
 * - if wls_aggressive was specified or size( res_i) < maxwls,
 *    => perform With-Loop Scalarization
 *
 * - In all other cases, block_i must either be empty or must contain a
 *   perfectly nested with-loop:
 *
 *   - The first (and only) assignment inside each block must have a
 *     with-loop on the right hand side.
 *
 *   <pre>
 *     (lb_i <= iv < ub_i) {
 *       res_i = with (jv)
 *                 ( lj_1 <= jv < uj_1): expr( iv, jv),
 *                 ( lj_m <= jv < uj_m): expr( iv, jv),
 *               genarray( shp_i);
 *     }: res_i,
 *       ...
 *     (lb_j <= iv < ub_j) {
 *       res_j = with (kv)
 *                 ( lk_1 <= kv < uk_1): expr( iv, kv),
 *                 ( lk_m <= kv < uk_m): expr( iv, kv),
 *               genarray( shp_j);
 *     }: res_j,
 *   </pre>
 *
 *   - Inner with-loops must either be genarray or modarray with-loops
 *
 *   - Inner generators must not depend on iv
 *
 *   - All inner index vectors must have equal length
 *
 * @ingroup wls
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file wlscheck.c
 *
 * Prefix: WLSC
 *
 *****************************************************************************/
#include "wls.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "new_types.h"
#include "print.h"
#include "shape.h"
#include "str.h"
#include "memory.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool possible;
    bool innertrav;
    node *cexpr;
    node *innerwithid;
    node *outerwithid;
};

#define INFO_POSSIBLE(n) (n->possible)
#define INFO_INNERTRAV(n) (n->innertrav)
#define INFO_CEXPR(n) (n->cexpr)
#define INFO_INNERWITHID(n) (n->innerwithid)
#define INFO_OUTERWITHID(n) (n->outerwithid)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_POSSIBLE (result) = TRUE;
    INFO_INNERTRAV (result) = FALSE;
    INFO_CEXPR (result) = NULL;
    INFO_INNERWITHID (result) = NULL;
    INFO_OUTERWITHID (result) = NULL;

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
 * @fn int WLSCdoCheck( node *with)
 *
 * @brief starting function of the WLSCheck traversal.
 *
 * @param with with-loop be checked.
 *
 * @return Returns the number of inner dimensions that can be scalarized
 *         in particular: WLSCheck( w) == 0 means, WLS is not applicable.
 *
 *****************************************************************************/
int
WLSCdoCheck (node *with)
{
    info *info;
    int res;

    DBUG_ENTER ("WLSCdoCheck");

    DBUG_ASSERT (NODE_TYPE (with) == N_with, "First parameter must be a with-loop");

    info = MakeInfo ();

    DBUG_PRINT ("WLS", ("Checking whether with-loop can be scalarized..."));

    TRAVpush (TR_wlsc);
    with = TRAVdo (with, info);
    TRAVpop ();

    if (INFO_POSSIBLE (info)) {

        if (INFO_INNERWITHID (info) == NULL) {
            /*
             * If there is no inner with-loop, the number of scalarizable
             * dimensions is only given by the CEXPRs
             */
            res = TYgetDim (ID_NTYPE (INFO_CEXPR (info)));
        } else {
            /*
             * In all other cases, use the number of inner index scalars
             */
            res = TCcountIds (WITHID_IDS (INFO_INNERWITHID (info)));
        }
    } else {
        res = 0;
    }

    if (res > 0) {
        DBUG_PRINT ("WLS", ("With-loop can be scalarized."));
    } else {
        DBUG_PRINT ("WLS", ("With-loop cannot be scalarized."));
    }

    info = FreeInfo (info);

    DBUG_RETURN (res);
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
 * @fn node *WLSCblock(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @param return
 *
 *****************************************************************************/
node *
WLSCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSCblock");

    /*
     * To enhance applicability of WLS, we allow empty code blocks
     * This means, the CEXPR is defined outside the block
     */
    if (NODE_TYPE (BLOCK_INSTR (arg_node)) != N_empty) {
        /*
         * The block's first assignment must be INFO_CEXPR
         */
        if (BLOCK_INSTR (arg_node) != AVIS_SSAASSIGN (ID_AVIS (INFO_CEXPR (arg_info)))) {
            INFO_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("WLS", ("CEXPR is not first assignment in block!!!"));
        }

        /*
         * The assignment must have a with-loop as RHS
         */
        if (INFO_POSSIBLE (arg_info)) {
            if (NODE_TYPE (ASSIGN_RHS (BLOCK_INSTR (arg_node))) != N_with) {
                INFO_POSSIBLE (arg_info) = FALSE;
                DBUG_PRINT ("WLS", ("CEXPR is not given by a with-loop!"));
            }
        }

        /*
         * Then we continue the check inside of that with-loop
         */
        if (INFO_POSSIBLE (arg_info)) {
            ASSIGN_RHS (BLOCK_INSTR (arg_node))
              = TRAVdo (ASSIGN_RHS (BLOCK_INSTR (arg_node)), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSCcode(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSCcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSCcode");

    DBUG_ASSERT (INFO_INNERTRAV (arg_info) == FALSE,
                 "WLSCcode must only be called in outer code traversal");

    /*
     * Remember the current CEXPR
     */
    INFO_CEXPR (arg_info) = CODE_CEXPR (arg_node);

    /*
     * The CEXPR must be AKS or better
     */
    if ((!TYisAKS (ID_NTYPE (CODE_CEXPR (arg_node))))
        && (!TYisAKV (ID_NTYPE (CODE_CEXPR (arg_node))))) {
        INFO_POSSIBLE (arg_info) = FALSE;
        DBUG_PRINT ("WLS", ("CEXPR is neither AKS nor AKV!!!"));
    }

    /*
     * The CEXPR must not be scalar
     */
    if (INFO_POSSIBLE (arg_info)) {
        if (TYgetDim (ID_NTYPE (CODE_CEXPR (arg_node))) == 0) {
            INFO_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("WLS", ("With-loop already has got scalar elements."));
        }
    }

    /*
     * Its type must match the type of the previous CEXPRs
     */
    if ((INFO_POSSIBLE (arg_info)) && (INFO_CEXPR (arg_info) != NULL)) {
        if (!TYeqTypes (ID_NTYPE (INFO_CEXPR (arg_info)),
                        ID_NTYPE (CODE_CEXPR (arg_node)))) {
            INFO_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("WLS", ("Inner CEXPRS have different types!!!"));
        }
    }

    if ((INFO_POSSIBLE (arg_info)) && (!global.wls_aggressive)
        && (SHgetUnrLen (TYgetShape (ID_NTYPE (CODE_CEXPR (arg_node))))
            > global.maxwls)) {

        /*
         * Traverse into CODE_CBLOCK
         */
        if (CODE_CBLOCK (arg_node) != NULL) {
            INFO_INNERTRAV (arg_info) = TRUE;
            CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
            INFO_INNERTRAV (arg_info) = FALSE;
        } else {
            /*
             * No CBLOCK means there is no inner with-loop
             */
            INFO_POSSIBLE (arg_info) = FALSE;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSCdefault(node *arg_node, info *arg_info)
 *
 * @brief rules out default generators
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSCdefault (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSCdefault");

    INFO_POSSIBLE (arg_info) = FALSE;
    DBUG_PRINT ("WLS", ("Default generators cannot be merged"));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSCgenerator(node *arg_node, info *arg_info)
 *
 * @brief rules out non-AKV N_id bounds
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSCgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSCgenerator");

    /*
     * WLS cannot be performed if the bound vectors are neither full not
     * structural constants
     */
    if ((NODE_TYPE (GENERATOR_BOUND1 (arg_node)) == N_id)
        && (!TYisAKV (ID_NTYPE (GENERATOR_BOUND1 (arg_node))))) {
        INFO_POSSIBLE (arg_info) = FALSE;
        DBUG_PRINT ("WLS", ("Bounds vector is neither AKV N_id nor N_array"));
    }

    if ((NODE_TYPE (GENERATOR_BOUND2 (arg_node)) == N_id)
        && (!TYisAKV (ID_NTYPE (GENERATOR_BOUND2 (arg_node))))) {
        INFO_POSSIBLE (arg_info) = FALSE;
        DBUG_PRINT ("WLS", ("Bounds vector is neither AKV N_id nor N_array"));
    }

    if ((GENERATOR_STEP (arg_node) != NULL)
        && (NODE_TYPE (GENERATOR_STEP (arg_node)) == N_id)
        && (!TYisAKV (ID_NTYPE (GENERATOR_STEP (arg_node))))) {
        INFO_POSSIBLE (arg_info) = FALSE;
        DBUG_PRINT ("WLS", ("Bounds vector is neither AKV N_id nor N_array"));
    }

    if ((GENERATOR_WIDTH (arg_node) != NULL)
        && (NODE_TYPE (GENERATOR_WIDTH (arg_node)) == N_id)
        && (!TYisAKV (ID_NTYPE (GENERATOR_WIDTH (arg_node))))) {
        INFO_POSSIBLE (arg_info) = FALSE;
        DBUG_PRINT ("WLS", ("Bounds vector is neither AKV N_id nor N_array"));
    }

    /*
     * Inner generators must not have dependencies to the outer withloop
     */
    if (INFO_INNERTRAV (arg_info)) {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSCid(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSCid (node *arg_node, info *arg_info)
{
    node *ids;

    DBUG_ENTER ("WLSCid");

    ids = WITHID_VEC (INFO_OUTERWITHID (arg_info));
    if (ID_AVIS (arg_node) == IDS_AVIS (ids)) {
        INFO_POSSIBLE (arg_info) = FALSE;
        DBUG_PRINT ("WLS", ("Dependecy of with-loop constructs detected!!!"));
    }

    ids = WITHID_IDS (INFO_OUTERWITHID (arg_info));
    while (ids != NULL) {
        if (ID_AVIS (arg_node) == IDS_AVIS (ids)) {
            INFO_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("WLS", ("Dependecy of with-loop constructs detected!!!"));
        }
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSCpart(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSCpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSCpart");

    if (!INFO_INNERTRAV (arg_info)) {
        /*
         * Outer part traversal
         */
        INFO_OUTERWITHID (arg_info) = PART_WITHID (arg_node);

        /*
         * Traverse into the part's code
         */
        PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);

        /*
         * Traverse into the generator
         */
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

        /*
         * Traverse next part
         */
        if (INFO_POSSIBLE (arg_info)) {
            if (PART_NEXT (arg_node) != NULL) {
                PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
            }
        }
    } else {
        /*
         * Inner part traversal
         */

        /*
         * Traverse generators
         */
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

        /*
         * Traverse next part
         */
        if (INFO_POSSIBLE (arg_info)) {
            if (PART_NEXT (arg_node) != NULL) {
                PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSCwith(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSCwith");

    if (!INFO_INNERTRAV (arg_info)) {
        /*
         * Traversal of outer with-loop
         */

        /*
         * Outer with-loop must not be a fold with-loop
         */
        if ((WITH_TYPE (arg_node) != N_genarray)
            && (WITH_TYPE (arg_node) != N_modarray)) {
            INFO_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("WLS", ("Outer with-loop is no genarray/modarray with-loop!"));
        }

        /*
         * Outer with-loop must not be a multi-operator with-loop
         */
        if (WITHOP_NEXT (WITH_WITHOP (arg_node)) != NULL) {
            INFO_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("WLS", ("Outer with-loop is multi-operator with-loop!"));
        }

        /*
         * Its parts must form a full partition of index space
         */
        if (INFO_POSSIBLE (arg_info)) {
            if (WITH_PARTS (arg_node) < 0) {
                INFO_POSSIBLE (arg_info) = FALSE;
                DBUG_PRINT ("WLS", ("Outer with-loop has no full partition!"));
            }
        }

        /*
         * Traverse into parts for further checking
         */
        if (INFO_POSSIBLE (arg_info)) {
            WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        }
    } else {
        /*
         * Traversal of inner with-loop
         */

        /*
         * Traverse withop
         */
        if (INFO_POSSIBLE (arg_info)) {
            WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        }

        /*
         * Traverse withid
         */
        if (INFO_POSSIBLE (arg_info)) {
            WITH_WITHID (arg_node) = TRAVdo (WITH_WITHID (arg_node), arg_info);
        }

        /*
         * Traverse parts
         */
        if (INFO_POSSIBLE (arg_info)) {
            WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSCwithid(node *arg_node, info *arg_info)
 *
 * @brief ensures all the inner withids cover the same number of dimensions.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSCwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSCwithid");

    if (INFO_INNERTRAV (arg_info)) {
        /*
         * Inner withid traversal
         */
        if (INFO_INNERWITHID (arg_info) == NULL) {
            INFO_INNERWITHID (arg_info) = arg_node;
        } else {
            if (TCcountIds (WITHID_IDS (INFO_INNERWITHID (arg_info)))
                != TCcountIds (WITHID_IDS (arg_node))) {
                INFO_POSSIBLE (arg_info) = FALSE;
                DBUG_PRINT ("WLS", ("Inner with-loops' index vectors differ in length"));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSCgenarray(node *arg_node, info *arg_info)
 *
 * @brief ensures the inner shape has no depencies to the outer generator.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSCgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSCgenarray");

    /*
     * The inner shape must not depend on the outer WITHID
     */
    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSCwithop(node *arg_node, info *arg_info)
 *
 * @brief ensures the inner shape has no depencies to the outer generator.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSCmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSCmodarray");

    /*
     * The modified array must not depend on the outer WITHID
     */
    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSCfold(node *arg_node, info *arg_info)
 *
 * @brief ensures the inner shape has no depencies to the outer generator.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSCfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSCfold");

    INFO_POSSIBLE (arg_info) = FALSE;
    DBUG_PRINT ("WLS", ("Inner with-loop is no genarray/modarray with-loop!!!"));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSCpropagate(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSCpropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSCpropagate");

    INFO_POSSIBLE (arg_info) = FALSE;
    DBUG_PRINT ("WLS", ("Inner with-loop is no genarray/modarray with-loop!!!"));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- WLSC -->
 *****************************************************************************/
