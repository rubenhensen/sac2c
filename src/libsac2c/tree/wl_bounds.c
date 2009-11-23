/*
 *
 * $Id$
 *
 */

#include "wl_bounds.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "DupTree.h"
#include "NameTuplesUtils.h"
#include "str.h"
#include "new_types.h"
#include "dbug.h"

/** <!-- ****************************************************************** -->
 * @fn bool WLBidOrNumEq( node *arg1, node *arg2)
 *
 * @brief Compares the two arguments. If both are N_id nodes, the result is
 *        the comparison on the corresponding N_avis nodes. If both nodes are
 *        if kind N_num, the result is comparing their values. Otherwise, the
 *        function returns false.
 *
 * @param arg1 N_id or N_num node to compare
 * @param arg2 N_id or N_num node to compare
 *
 * @return result of comparison
 ******************************************************************************/

bool
WLBidOrNumEq (node *arg1, node *arg2)
{
    bool result;

    DBUG_ENTER ("WLBidOrNumEq");

    if ((NODE_TYPE (arg1) == N_id) && (NODE_TYPE (arg2) == N_id)) {
        result = (ID_AVIS (arg1) == ID_AVIS (arg2));
    } else if ((NODE_TYPE (arg1) == N_num) && (NODE_TYPE (arg2) == N_num)) {
        result = (NUM_VAL (arg1) == NUM_VAL (arg2));
    } else {
        result = FALSE;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn bool WLBidOrNumLe( node *arg1, node *arg2, int shape)
 *
 *   Compares two parameters of N_wlstride(Var) or N_wlgrid(Var) nodes.
 *
 *   'shape' denotes the shape component of the current dimension.
 *   If the concrete value of the shape is unknown (i.e. dynamically shaped
 *   or fold with-loop) 'shape' must equal IDX_SHAPE.
 *
 * @param arg1 N_id or N_num node to compare
 * @param arg2 N_id or N_num node to compare
 * @param shape shape of current dimensions
 *
 * @return result of comparison
 ******************************************************************************/

bool
WLBidOrNumLe (node *arg1, node *arg2, int shape)
{
    bool result;

    DBUG_ENTER ("WLBidOrNumLe");

    result = WLBidOrNumEq (arg1, arg2);

    if (((NODE_TYPE (arg1) == N_num) && (NUM_VAL (arg1) == 0))
        || ((NODE_TYPE (arg2) == N_num)
            && ((NUM_VAL (arg2) == IDX_SHAPE) || (NUM_VAL (arg2) == shape)))
        || ((NODE_TYPE (arg1) == N_num) && (NODE_TYPE (arg2) == N_num)
            && (NUM_VAL (arg1) < NUM_VAL (arg2)))) {
        result = TRUE;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *WLBidOrNumMakeIndex( node *bound, int dim, node *wl_ids)
 *
 * @brief
 *   Converts the parameter of a N_wlstride or N_wlgrid node into an index.
 *   If bound is a N_id node the function returns a new N_id/N_icm node
 *   containing the correct index selection ('id' if scalar, 'id[dim]'
 *   otherwise).
 *   If bound is a N_num node with value IDX_SHAPE, the functions returns a
 *   new N_id/N_icm node containing the shape of the current with-loop.
 *   If the N_num node has any other value, a N_num node containing that
 *   value is returned.
 *
 * @param bound N_id or N_num node
 * @param dim  dimension to select
 * @param wl_ids wl-lhs to find shape
 *
 * @return index
 ******************************************************************************/
node *
WLBidOrNumMakeIndex (node *bound, int dim, node *wl_ids)
{
    node *index;
    int sel_dim;

    DBUG_ENTER ("WLBnodeOrIntMakeIndex");

    if (NODE_TYPE (bound) == N_num) {
        if (NUM_VAL (bound) == IDX_SHAPE) {
            index = TCmakeIcm2 ("ND_A_SHAPE", DUPdupIdsIdNt (wl_ids), TBmakeNum (dim));
        } else {
            index = TBmakeNum (NUM_VAL (bound));
        }
    } else {

        DBUG_ASSERT ((ID_DECL (bound) != NULL), "no vardec/decl found!");

        /**
         * As we assume that this code is relevant for with2 only(!!),
         * we can now rely on WLPG to make bounds structural constants
         * As a consequence of that, we know, that we just need to select
         * the scalar value!
         * However, I left the original commented out in case
         * this is used in a different context too.
         */
#if 0
    sel_dim = (ID_DIM( bound) == SCALAR) ? 0 : dim;
#else
        sel_dim = 0;
#endif

        index = TCmakeIcm2 ("ND_READ", DUPdupIdNt (bound), TBmakeNum (sel_dim));
    }

    DBUG_RETURN (index);
}
