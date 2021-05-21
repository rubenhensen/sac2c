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
 * - lb_i, ub_i must either be AKV N_id nodes OR N_array nodes,
 *              or their predecessors must be one of those.
 *
 * - if -dowls_aggressive was specified or size( res_i) < maxwls,
 *    => perform With-Loop Scalarization
 *
 * - In all other cases, block_i must either be empty or must contain a
 *   perfectly nested with-loop:
 *
 *   - Leading _noteminval/_notemaxval nodes are ignored, as they
 *     are eliminated after optimization.
 *   - The first (and only) remaining assignment inside each block must have a
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
 *   - The strictures on block_i contents comes from this observation:
 *     If the outer wl's block contains anything but the inner wl,
 *     the execution of this code will inevitably be multiplied
 *     through WLS. Essentially, WLS pushes the code into the inner wl.
 *     This can of course have very adverse effects on
 *     performance. Consequently, WLS bails out by default,
 *     but with -wls-aggressive it is forced to do it no matter
 *     what the consequences may be.
 *
 *     The trouble is that it is generally undecidable whether the
 *     positive effect of WLS overcomes the negative effect of
 *     multiplying work. The latter depends very
 *     much on the code multiplied and likewise on the generator
 *     of the inner wl, i.e. how often the execution is repeated.
 *     Last, but not least, the C compiler may or may
 *     not be able to undo the transformation, as the code remains
 *     invariant to the for-loops generated for the inner wl.
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

#define DBUG_PREFIX "WLS"
#include "debug.h"

#include "new_types.h"
#include "print.h"
#include "shape.h"
#include "str.h"
#include "memory.h"
#include "pattern_match.h"
#include "constants.h"

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
    node *nassign; /* for DBUG_PRINT clarification */
};

#define INFO_POSSIBLE(n) (n->possible)
#define INFO_INNERTRAV(n) (n->innertrav)
#define INFO_CEXPR(n) (n->cexpr)
#define INFO_INNERWITHID(n) (n->innerwithid)
#define INFO_OUTERWITHID(n) (n->outerwithid)
#define INFO_NASSIGN(n) (n->nassign)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

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
    DBUG_ENTER ();

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
 * @fn size_t WLSCdoCheck( node *with, node *nassign)
 *
 * @brief starting function of the WLSCheck traversal.
 *
 * @param with with-loop be checked.
 *
 * @return Returns the number of inner dimensions that can be scalarized
 *         in particular: WLSCheck( w) == 0 means, WLS is not applicable.
 *
 *****************************************************************************/
size_t
WLSCdoCheck (node *with, node *nassign)
{
    info *arg_info;
    node *lhs;
    size_t res;
    ntype *typ;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (with) == N_with, "First parameter must be a with-loop");

    arg_info = MakeInfo ();
    INFO_NASSIGN (arg_info) = nassign;
    lhs = IDS_AVIS (LET_IDS (ASSIGN_STMT (INFO_NASSIGN (arg_info))));

    DBUG_PRINT ("%s: Checking whether with-loop can be scalarized.", AVIS_NAME (lhs));

    TRAVpush (TR_wlsc);
    with = TRAVdo (with, arg_info);
    TRAVpop ();

    if (INFO_POSSIBLE (arg_info)) {

        if (INFO_INNERWITHID (arg_info) == NULL) {
            /*
             * If there is no inner with-loop, the number of scalarizable
             * dimensions is only given by the CEXPRs
             */

            typ = ID_NTYPE (INFO_CEXPR (arg_info));
            res = TYgetDim (typ);

            /*
             * If there is one CEXPRS, and it happens to be an
             * empty array, do not attempt WLS. Other optimizations
             * will eliminate the outer WL.
             */
            if (0 == SHgetUnrLen (TYgetShape (typ))) {
                res = 0;
                DBUG_PRINT ("Skipping WLS for empty array cell CEXPRS %s",
                            AVIS_NAME (ID_AVIS (INFO_CEXPR (arg_info))));
            }
        } else {
            /*
             * In all other cases, use the number of inner index scalars
             */
            res = TCcountIds (WITHID_IDS (INFO_INNERWITHID (arg_info)));
        }
    } else {
        res = 0;
    }

    if (res > 0) {
        DBUG_PRINT ("%s: With-loop can be scalarized.", AVIS_NAME (lhs));
    } else {
        DBUG_PRINT ("%s: With-loop cannot be scalarized.", AVIS_NAME (lhs));
    }

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @static node *skipIrrelevantAssigns(node *arg_node)
 *
 * @brief skips over all those assigns that we consider cheap enough not
 *        to cause any harm by repeated execution
 *
 *        In this case, we skip Extrema, because they always disappear
 *        after optimization.
 *
 *        We also skip guards if compiling with -ecc, for the same
 *        reason.
 *
 * @param arg_node - an N_assign chain.
 * @param arg_info
 *
 * @param return
 *
 *****************************************************************************/
static node *
skipIrrelevantAssigns (node *arg_node)
{
    node *z;
    node *rhs;

    DBUG_ENTER ();

    z = arg_node;

    if (NULL != ASSIGN_NEXT (arg_node)) {
        rhs = LET_EXPR (ASSIGN_STMT (arg_node));
        if ((N_prf == NODE_TYPE (rhs))
            && ((PMMisInExtrema (PRF_PRF (rhs)))
                || (global.insertconformitychecks && PMMisInGuards (PRF_PRF (rhs))))) {
            z = skipIrrelevantAssigns (ASSIGN_NEXT (arg_node));
        }
    }

    DBUG_RETURN (z);
}

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
 *  We check that the outer WL block has a WL defining its
 *  result. In order to allow extrema to be attached ahead of
 *  the inner WL, we check for the expr being the last
 *  expression in the block.
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
#if 0
  node *lhs;
#endif
    node *wlassign;

    DBUG_ENTER ();

    /*
     * To enhance applicability of WLS, we allow empty code blocks
     * This means that the CEXPR is defined outside the block
     */
    if (BLOCK_ASSIGNS (arg_node) != NULL) {
        wlassign = skipIrrelevantAssigns (BLOCK_ASSIGNS (arg_node));
        if (wlassign != NULL) {

#if 0
      DBUG_EXECUTE (lhs = IDS_AVIS(
                                   LET_IDS(
                                     ASSIGN_STMT(
                                       INFO_NASSIGN( arg_info)))));
#endif

            /*
             * The block's first relevant assignment must be INFO_CEXPR
             * NB we are not in wls-aggressive mode here!
             */
            if (wlassign != AVIS_SSAASSIGN (ID_AVIS (INFO_CEXPR (arg_info)))) {
                INFO_POSSIBLE (arg_info) = FALSE;
#if 0
        DBUG_PRINT ("%s: CEXPR is not last assignment in block!!!",
                    AVIS_NAME( lhs));
#endif
            }

            /*
             * The assignment must have a with-loop as RHS
             */
            if (INFO_POSSIBLE (arg_info)) {
                if (N_with != NODE_TYPE (ASSIGN_RHS (wlassign))) {
                    INFO_POSSIBLE (arg_info) = FALSE;
#if 0
          DBUG_PRINT ("%s: CEXPR is not given by a with-loop!",
                    AVIS_NAME( lhs));
#endif
                }
            }

            /*
             * Then we continue the check inside of that with-loop
             */
            if (INFO_POSSIBLE (arg_info)) {
                ASSIGN_RHS (wlassign) = TRAVdo (ASSIGN_RHS (wlassign), arg_info);
            }
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
    node *lhs;

    DBUG_ENTER ();

    DBUG_ASSERT (INFO_INNERTRAV (arg_info) == FALSE,
                 "WLSCcode must only be called in outer code traversal");

    /*
     * Remember the current CEXPR
     */
    INFO_CEXPR (arg_info) = CODE_CEXPR (arg_node);

    /*
     * The CEXPR must be AKS or better
     */
    lhs = IDS_AVIS (LET_IDS (ASSIGN_STMT (INFO_NASSIGN (arg_info))));
    if ((!TYisAKS (ID_NTYPE (CODE_CEXPR (arg_node))))
        && (!TYisAKV (ID_NTYPE (CODE_CEXPR (arg_node))))) {
        INFO_POSSIBLE (arg_info) = FALSE;
        DBUG_PRINT ("%s: CEXPR is neither AKS nor AKV!!!", AVIS_NAME (lhs));
    }

    /*
     * The CEXPR must not be scalar
     */
    if (INFO_POSSIBLE (arg_info)) {
        if (TYgetDim (ID_NTYPE (CODE_CEXPR (arg_node))) == 0) {
            INFO_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("%s: With-loop already has got scalar elements.",
                        AVIS_NAME (lhs));
        }
    }

    /*
     * Its type must match the type of the previous CEXPRs (multiple partitions)
     */
    if ((INFO_POSSIBLE (arg_info)) && (INFO_CEXPR (arg_info) != NULL)) {
        if (!TYeqTypes (ID_NTYPE (INFO_CEXPR (arg_info)),
                        ID_NTYPE (CODE_CEXPR (arg_node)))) {
            INFO_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("Inner CEXPRS have different types detected at "
                        "with-loop  body expression \"%s\"!!!",
                        ID_NAME (CODE_CEXPR (arg_node)));
        }
    }

    if ((INFO_POSSIBLE (arg_info)) && (!global.optimize.dowls_aggressive)
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
    DBUG_ENTER ();

    INFO_POSSIBLE (arg_info) = FALSE;
    DBUG_PRINT ("Default generators cannot be merged");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static void genhelper(node *arg_node, info *arg_info, char *nm)
 *
 * @brief rules out generator bounds,step,width if non-null and
 *        don't eventually derive from an N_array.
 *
 * @param arg_node - some N_generator node.
 * @param arg_info
 * @param nm       - N_generator node name.
 *
 * @return void
 *
 *****************************************************************************/
void
genhelper (node *arg_node, info *arg_info, char *nm)
{
    node *lhs;
    node *argnarray = NULL;
    pattern *pat;

    DBUG_ENTER ();

    /*
     * WLS cannot be performed if the bound vectors are neither full nor
     * structural constants
     */
    lhs = IDS_AVIS (LET_IDS (ASSIGN_STMT (INFO_NASSIGN (arg_info))));
    pat = PMarray (1, PMAgetNode (&argnarray), 0);
    if ((NULL != arg_node) && (!PMmatchFlatSkipExtrema (pat, arg_node))) {
        INFO_POSSIBLE (arg_info) = FALSE;
        DBUG_PRINT ("%s: %s is not an N_array", nm, IDS_NAME (lhs));
    }
    PMfree (pat);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    /*
     * WLS cannot be performed if the bound vectors are neither full not
     * structural constants
     */
    genhelper (GENERATOR_BOUND1 (arg_node), arg_info, "GENERATOR_BOUND1");
    genhelper (GENERATOR_BOUND2 (arg_node), arg_info, "GENERATOR_BOUND2");
    genhelper (GENERATOR_STEP (arg_node), arg_info, "GENERATOR_STEP");
    genhelper (GENERATOR_WIDTH (arg_node), arg_info, "GENERATOR_WIDTH");

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
    node *lhs;

    DBUG_ENTER ();

    lhs = IDS_AVIS (LET_IDS (ASSIGN_STMT (INFO_NASSIGN (arg_info))));
    ids = WITHID_VEC (INFO_OUTERWITHID (arg_info));
    if (ID_AVIS (arg_node) == IDS_AVIS (ids)) {
        INFO_POSSIBLE (arg_info) = FALSE;
        DBUG_PRINT ("%s: Dependency of with-loop vec constructs detected",
                    AVIS_NAME (lhs));
    }

    ids = WITHID_IDS (INFO_OUTERWITHID (arg_info));
    while (ids != NULL) {
        if (ID_AVIS (arg_node) == IDS_AVIS (ids)) {
            INFO_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("%s: Dependency of with-loop ids constructs detected",
                        AVIS_NAME (lhs));
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
    DBUG_ENTER ();

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
    node *lhs;

    DBUG_ENTER ();

    if (!INFO_INNERTRAV (arg_info)) {
        /*
         * Traversal of outer with-loop
         */

        /*
         * Outer with-loop must not be a fold with-loop
         */
        lhs = IDS_AVIS (LET_IDS (ASSIGN_STMT (INFO_NASSIGN (arg_info))));
        if ((WITH_TYPE (arg_node) != N_genarray)
            && (WITH_TYPE (arg_node) != N_modarray)) {
            INFO_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("%s: Outer with-loop is not genarray/modarray with-loop",
                        AVIS_NAME (lhs));
        }

        /*
         * Outer with-loop must not be a multi-operator with-loop
         */
        if (WITHOP_NEXT (WITH_WITHOP (arg_node)) != NULL) {
            INFO_POSSIBLE (arg_info) = FALSE;
            DBUG_PRINT ("%s: Outer with-loop is multi-operator with-loop",
                        AVIS_NAME (lhs));
        }

        /*
         * Its parts must form a full partition of index space
         */
        if (INFO_POSSIBLE (arg_info)) {
            if (TCcontainsDefaultPartition (WITH_PART (arg_node))) {
                INFO_POSSIBLE (arg_info) = FALSE;
                DBUG_PRINT ("%s: Outer with-loop has no full partition", AVIS_NAME (lhs));
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
    node *lhs;

    DBUG_ENTER ();

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
                lhs = IDS_AVIS (LET_IDS (ASSIGN_STMT (INFO_NASSIGN (arg_info))));
                DBUG_PRINT ("%s: Inner with-loops' index vectors differ in length",
                            AVIS_NAME (lhs));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSCgenarray(node *arg_node, info *arg_info)
 *
 * @brief ensures the inner shape has no dependencies on the outer generator.
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
    DBUG_ENTER ();

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
 * @brief ensures the inner shape has no dependencies to the outer generator.
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
    DBUG_ENTER ();

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
 * @brief ensures the inner shape has no dependencies on the outer generator.
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
    node *lhs;

    DBUG_ENTER ();

    INFO_POSSIBLE (arg_info) = FALSE;
    lhs = IDS_AVIS (LET_IDS (ASSIGN_STMT (INFO_NASSIGN (arg_info))));
    DBUG_PRINT ("%s: Inner with-loop is not a genarray or modarray with-loop",
                AVIS_NAME (lhs));

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
    node *lhs;

    DBUG_ENTER ();

    INFO_POSSIBLE (arg_info) = FALSE;
    lhs = IDS_AVIS (LET_IDS (ASSIGN_STMT (INFO_NASSIGN (arg_info))));
    DBUG_PRINT ("Inner with-loop is not a genarray/modarray with-loop"
                "next assignment lhs is %s", AVIS_NAME (lhs));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- WLSC -->
 *****************************************************************************/

#undef DBUG_PREFIX
