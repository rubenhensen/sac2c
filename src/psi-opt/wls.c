/*
 *
 * $Log$
 * Revision 1.1  2004/10/07 12:35:45  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup wls With-Loop Scalarization
 * @ingroup opt
 *
 * @brief With-Loop Scalarization is a high-level optimization which composes
 *        a single withloop from nested ones in order to minimize memory-
 *        transactions and thereby improving program efficiency.
 *
 * <pre>
 *
 * A bottom-up traversal of the SAC-program is performed.
 * When a with-loop is met, its codes are traversed first in order to
 * scalarize nested with-loops inside of these codes.
 *
 * Scalarization is then performed by means of three seperate traversals:
 *
 *  - WLSCheck checks whether all criteria are met to scalarize the current WL.
 *    These criteria are described in wlscheck.c
 *
 *  - WLSWithloopifification modifies a with-loop's codes in order to create
 *    the pattern of perferctly nested with-loops as seen in the example above.
 *
 *  - WLSBuild then finally replaces the with-loop nesting with a new,
 *    scalarized version of that with-loop
 *
 *
 * An example for with-loop scalarization is given below:
 *
 *   A = with ( lb_1 <= iv < ub_1 ) {
 *         B = with ( lb_2 <= jv < ub_2 ) {
 *               val = expr( iv, jv);
 *             } : val
 *             genarray( shp_2);
 *       } : B
 *       genarray( shp_1);
 *
 *   is transformed into
 *
 *   A = with ( lb_1++lb_2 <= kv < ub_1++ub_2) {
 *         iv = take( shape( lb_1), kv);
 *         jv = drop( shape( lb_1), kv);
 *         val = expr( iv, jv);
 *       } : val
 *       genarray( shp_1++shp_2);
 *
 * </pre>
 *
 * @{
 */

/**
 *
 * @file wls.c
 *
 * Implements the top-level traversal of Withloop-Scalarization.
 * A bottom-up traversal of the SAC-program is performed.
 * When a with-loop is met, its codes are traversed first in order to
 * scalarize nested with-loops inside of these codes.
 *
 * Scalarization is then performed by means of three seperate traversals:
 *
 *  - WLSCheck checks whether all criteria are met to scalarize the current WL.
 *    These criteria are described in wlscheck.c
 *
 *  - WLSWithloopifification modifies a with-loop's codes in order to create
 *    the pattern of perferctly nested with-loops as seen in the example above.
 *
 *  - WLSBuild then finally replaces the with-loop nesting with a new,
 *    scalarized version of that with-loop
 *
 */

#define NEW_INFO

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "new_types.h"
#include "print.h"
#include "wls.h"

/**
 * INFO structure
 */
struct INFO {
    node *fundef;
};

/**
 * INFO macros
 */
#define INFO_WLS_FUNDEF(n) (n->fundef)

/**
 * INFO functions
 */
static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_WLS_FUNDEF (result) = fundef;

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
 * @fn node *WLSWithloopScalarization( node *fundef)
 *
 * @brief starting point of WithloopScalarization.
 *
 * @param fundef Fundef-Node to start WLS in. Note that special functions
 *               (LAC-Function) are traversed in order of appearance.
 *
 * @return modified fundef.
 *
 *****************************************************************************/
node *
WLSWithloopScalarization (node *fundef)
{
    funtab *old_tab;

    DBUG_ENTER ("WLSWithloopScalarization");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "WLSWithloopScalarization called for non-fundef node");

    if (!FUNDEF_IS_LACFUN (fundef)) {
        old_tab = act_tab;
        act_tab = wls_tab;

        fundef = Trav (fundef, NULL);

        act_tab = old_tab;
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * WLS traversal (wls_tab)
 *
 * prefix: WLS
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *WLSap(node *arg_node, info *arg_info)
 *
 * @brief traverses into special fundefs when they are applied.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
WLSap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSap");

    /*
     * special functions must be traversed when they are used
     */
    if ((FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_WLS_FUNDEF (arg_info))) {

        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSassign(node *arg_node, info *arg_info)
 *
 * @brief performs a bottom-up traversal.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
WLSassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSassign");

    /*
     * Bottom-up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    /*
     * Traverse RHS
     */
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSfundef(node *arg_node, info *arg_info)
 *
 * @brief applies WLS to a given fundef.
 *        An INFO-structure containing a pointer to the current fundef is
 *        created before traversal of the function body.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 ******************************************************************************/
node *
WLSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSfundef");

    DBUG_PRINT ("OPT", ("starting WithloopScalarization in function %s",
                        FUNDEF_NAME (arg_node)));

    if (FUNDEF_BODY (arg_node) != NULL) {
        /*
         * Create new INFO structure
         */
        arg_info = MakeInfo (arg_node);

        /*
         * traverse block of fundef
         */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        /*
         * Free INFO structure
         */
        arg_info = FreeInfo (arg_info);
    }

    DBUG_PRINT ("OPT", ("WithloopScalarization in function %s completed.",
                        FUNDEF_NAME (arg_node)));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSwith(node *arg_node, info *arg_info)
 *
 * @brief applies WLS to a with-loop in a bottom-up manner.
 *        This means, inner with-loops are scalarized first by traversing the
 *        with-loop's codes.
 *        After that it is checked whether WLS is applicable using the
 *        WLSCheck traversal.
 *        If it is, the base case of scalarization (perfectly nested
 *        with-loops) is created using WLSWithloopification.
 *        Thereafter, a new with-loop is built using the WLSBuild traversal.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return a new with-loop with scalar elements
 *
 *****************************************************************************/
node *
WLSwith (node *arg_node, info *arg_info)
{
    int innerdims;
    bool wls_possible;

    DBUG_ENTER ("WLSwith");

    /*
     * First, traverse all the codes in order to apply WLS
     * to inner with-loops
     */
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    /*
     * Afterwards, try to scalarize the current with-loop
     */
    DBUG_EXECUTE ("WLS", PrintNode (arg_node););

    /*
     * Scalarization is possible iff WLSCheck does not return 0
     */
    innerdims = WLSCheck (arg_node);

    if (innerdims > 0) {

        /*
         * Apply withloopification
         */
        arg_node = WLSWithloopify (arg_node, INFO_WLS_FUNDEF (arg_info), innerdims);

        /*
         * Build the new with-loop
         */
        arg_node = WLSBuild (arg_node, INFO_WLS_FUNDEF (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/*@}*/ /* defgroup wls */
