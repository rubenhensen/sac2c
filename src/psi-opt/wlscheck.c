/*
 *
 * $Log$
 * Revision 1.1  2004/10/07 12:36:00  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup wlsc WLSCheck
 * @ingroup wls
 *
 * @brief checks whether all criteria necessary for applying With-Loop
 *        Scalarization are met.
 *
 * <pre>
 *
 * Example:
 *
 * A = with (iv)
 *       ( lb_1 <= iv < ub_1) {
 *          block_1
 *       }: res_1,
 *       ...
 *       ( lb_n <= iv < ub_n) {
 *          block_n
 *       }: res_n,
 *     genarray( shp);
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
 * - if wls_aggressive was specified or size( res_i) < maxwls,
 *    => perform With-Loop Scalarization
 *
 * - In all other cases, block_i must either be empty or must contain a
 *   perfectly nested with-loop:
 *
 *   - The first (and only) assignment inside each block must have a
 *     with-loop on the right hand side.
 *
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
 *
 *   - Inner with-loops must either be genarray or modarray with-loops
 *
 *   - Inner generators must not depend on iv
 *
 *   - All inner index vectors must have equal length
 *
 * </pre>
 *
 * @{
 */

/**
 *
 * @file wlscheck.c
 *
 * Contains a traversal to check whether all criteria necessary for applying
 * With-Loop Scalarization are met.
 */

#define NEW_INFO

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "new_types.h"
#include "print.h"
#include "optimize.h"
#include "shape.h"
#include "wls.h"

/**
 * INFO structure
 */
struct INFO {
    bool possible;
    bool innertrav;
    node *cexpr;
    node *innerwithid;
    node *outerwithid;
};

/**
 * INFO macros
 */
#define INFO_WLS_POSSIBLE(n) (n->possible)
#define INFO_WLS_INNERTRAV(n) (n->innertrav)
#define INFO_WLS_CEXPR(n) (n->cexpr)
#define INFO_WLS_INNERWITHID(n) (n->innerwithid)
#define INFO_WLS_OUTERWITHID(n) (n->outerwithid)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_WLS_POSSIBLE (result) = TRUE;
    INFO_WLS_INNERTRAV (result) = FALSE;
    INFO_WLS_CEXPR (result) = NULL;
    INFO_WLS_INNERWITHID (result) = NULL;
    INFO_WLS_OUTERWITHID (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn int WLSCheck( node *with)
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
WLSCheck (node *with)
{
    funtab *old_tab;
    info *info;
    int res;

    DBUG_ENTER ("WLSCheck");

    DBUG_ASSERT (NODE_TYPE (with) == N_Nwith, "First parameter must be a with-loop");

    info = MakeInfo ();

    old_tab = act_tab;
    act_tab = wlsc_tab;

    DBUG_PRINT ("WLS", ("Checking whether with-loop can be scalarized..."));

    with = Trav (with, info);
    act_tab = old_tab;

    if (INFO_WLS_POSSIBLE (info)) {

        if (INFO_WLS_INNERWITHID (info) == NULL) {
            /*
             * If there is no inner with-loop, the number of scalarizable
             * dimensions is only given by the CEXPRs
             */
            res = TYGetDim (ID_NTYPE (INFO_WLS_CEXPR (info)));
        } else {
            /*
             * In all other cases, use the number of inner index scalars
             */
            res = CountIds (NWITHID_IDS (INFO_WLS_INNERWITHID (info)));
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

/******************************************************************************
 *
 * WLS check traversal (wlsc_tab)
 *
 * prefix: WLSC
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
         * The block's first assignment must be INFO_WLS_CEXPR
         */
        if (BLOCK_INSTR (arg_node)
            != AVIS_SSAASSIGN (ID_AVIS (INFO_WLS_CEXPR (arg_info)))) {
            INFO_WLS_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("WLS", ("CEXPR is not first assignment in block!!!"));
        }

        /*
         * The assignment must have a with-loop as RHS
         */
        if (INFO_WLS_POSSIBLE (arg_info)) {
            if (NODE_TYPE (ASSIGN_RHS (BLOCK_INSTR (arg_node))) != N_Nwith) {
                INFO_WLS_POSSIBLE (arg_info) = FALSE;
                DBUG_PRINT ("WLS", ("CEXPR is not given by a with-loop!"));
            }
        }

        /*
         * Then we continue the check inside of that with-loop
         */
        if (INFO_WLS_POSSIBLE (arg_info)) {
            ASSIGN_RHS (BLOCK_INSTR (arg_node))
              = Trav (ASSIGN_RHS (BLOCK_INSTR (arg_node)), arg_info);
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

    DBUG_ASSERT (INFO_WLS_INNERTRAV (arg_info) == FALSE,
                 "WLSCcode must only be called in outer code traversal");

    /*
     * Remember the current CEXPR
     */
    INFO_WLS_CEXPR (arg_info) = NCODE_CEXPR (arg_node);

    /*
     * The CEXPR must be AKS or better
     */
    if ((!TYIsAKS (ID_NTYPE (NCODE_CEXPR (arg_node))))
        && (!TYIsAKV (ID_NTYPE (NCODE_CEXPR (arg_node))))) {
        INFO_WLS_POSSIBLE (arg_info) = FALSE;
        DBUG_PRINT ("WLS", ("CEXPR is neither AKS nor AKV!!!"));
    }

    /*
     * The CEXPR must not be scalar
     */
    if (INFO_WLS_POSSIBLE (arg_info)) {
        if (TYGetDim (ID_NTYPE (NCODE_CEXPR (arg_node))) == 0) {
            INFO_WLS_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("WLS", ("With-loop already has got scalar elements."));
        }
    }

    /*
     * Its type must match the type of the previous CEXPRs
     */
    if ((INFO_WLS_POSSIBLE (arg_info)) && (INFO_WLS_CEXPR (arg_info) != NULL)) {
        if (!TYEqTypes (ID_NTYPE (INFO_WLS_CEXPR (arg_info)),
                        ID_NTYPE (NCODE_CEXPR (arg_node)))) {
            INFO_WLS_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("WLS", ("Inner CEXPRS have different types!!!"));
        }
    }

    if ((INFO_WLS_POSSIBLE (arg_info)) && (!wls_aggressive)
        && (SHGetUnrLen (TYGetShape (ID_NTYPE (NCODE_CEXPR (arg_node)))) > maxwls)) {

        /*
         * Traverse into NCODE_CBLOCK
         */
        if (NCODE_CBLOCK (arg_node) != NULL) {
            INFO_WLS_INNERTRAV (arg_info) = TRUE;
            NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
            INFO_WLS_INNERTRAV (arg_info) = FALSE;
        } else {
            /*
             * No CBLOCK means there is no inner with-loop
             */
            INFO_WLS_POSSIBLE (arg_info) = FALSE;
        }
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
    DBUG_ENTER ("WLSCid");

    if (AVIS_WITHID (ID_AVIS (arg_node)) == INFO_WLS_OUTERWITHID (arg_info)) {
        INFO_WLS_POSSIBLE (arg_info) = FALSE;
        DBUG_PRINT ("WLS", ("Dependecy of with-loop constructs detected!!!"));
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

    if (!INFO_WLS_INNERTRAV (arg_info)) {
        /*
         * Outer part traversal
         */

        /*
         * Traverse into withid in order to initialize AVIS_WITHID fields
         */
        NPART_WITHID (arg_node) = Trav (NPART_WITHID (arg_node), arg_info);
        INFO_WLS_OUTERWITHID (arg_info) = NPART_WITHID (arg_node);

        /*
         * Traverse into the part's code
         */
        NPART_CODE (arg_node) = Trav (NPART_CODE (arg_node), arg_info);

        /*
         * Traverse next part
         */
        if (INFO_WLS_POSSIBLE (arg_info)) {
            if (NPART_NEXT (arg_node) != NULL) {
                NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
            }
        }
    } else {
        /*
         * Inner part traversal
         */

        /*
         * Traverse generators
         */
        NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

        /*
         * Traverse next part
         */
        if (INFO_WLS_POSSIBLE (arg_info)) {
            if (NPART_NEXT (arg_node) != NULL) {
                NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
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

    if (!INFO_WLS_INNERTRAV (arg_info)) {
        /*
         * Traversal of outer with-loop
         */

        /*
         * Outer with-loop must not be a fold with-loop
         */
        if ((NWITH_TYPE (arg_node) != WO_genarray)
            && (NWITH_TYPE (arg_node) != WO_modarray)) {
            INFO_WLS_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("WLS", ("Outer with-loop is no genarray/modarray with-loop!"));
        }

        /*
         * Its parts must form a full partition of index space
         */
        if (INFO_WLS_POSSIBLE (arg_info)) {
            if (NWITH_PARTS (arg_node) < 0) {
                INFO_WLS_POSSIBLE (arg_info) = FALSE;
                DBUG_PRINT ("WLS", ("Outer with-loop has no full partition!"));
            }
        }

        /*
         * Traverse into parts for further checking
         */
        if (INFO_WLS_POSSIBLE (arg_info)) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }
    } else {
        /*
         * Traversal of inner with-loop
         */

        /*
         * Traverse withop
         */
        if (INFO_WLS_POSSIBLE (arg_info)) {
            NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
        }

        /*
         * Traverse withid
         */
        if (INFO_WLS_POSSIBLE (arg_info)) {
            NWITH_WITHID (arg_node) = Trav (NWITH_WITHID (arg_node), arg_info);
        }

        /*
         * Traverse parts
         */
        if (INFO_WLS_POSSIBLE (arg_info)) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
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

    if (!INFO_WLS_INNERTRAV (arg_info)) {
        /*
         * Outer withid traversal
         */
        ids *_ids;

        /*
         * The outer WL must be a AKD-WL
         * => index scalars must be present
         */
        INFO_WLS_POSSIBLE (arg_info)
          = (INFO_WLS_POSSIBLE (arg_info) && (NWITHID_IDS (arg_node) != NULL));

        /*
         * Initialize the AVIS_WITHID field of the withid variables
         */
        AVIS_WITHID (IDS_AVIS (NWITHID_VEC (arg_node))) = arg_node;

        _ids = NWITHID_IDS (arg_node);
        while (_ids != NULL) {
            AVIS_WITHID (IDS_AVIS (_ids)) = arg_node;
            _ids = IDS_NEXT (_ids);
        }
    } else {
        /*
         * Inner withid traversal
         */
        if (INFO_WLS_INNERWITHID (arg_info) == NULL) {
            INFO_WLS_INNERWITHID (arg_info) = arg_node;
        } else {
            if (CountIds (NWITHID_IDS (INFO_WLS_INNERWITHID (arg_info)))
                != CountIds (NWITHID_IDS (arg_node))) {
                INFO_WLS_POSSIBLE (arg_info) = FALSE;
                DBUG_PRINT ("WLS", ("Inner with-loops' index vectors differ in length"));
            }
        }
    }

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
WLSCwithop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSCwithop");

    switch (NWITHOP_TYPE (arg_node)) {

    case WO_genarray:
        /*
         * The inner shape must not depend on the outer WITHID
         */
        NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        break;

    case WO_modarray:
        /*
         * The modified array must not depend on the outer WITHID
         */
        NWITHOP_ARRAY (arg_node) = Trav (NWITHOP_ARRAY (arg_node), arg_info);
        break;

    default:
        INFO_WLS_POSSIBLE (arg_info) = FALSE;
        DBUG_PRINT ("WLS", ("Inner with-loop is no genarray/modarray with-loop!!!"));
        break;
    }

    DBUG_RETURN (arg_node);
}

/*@}*/ /* defgroup wlsc */
