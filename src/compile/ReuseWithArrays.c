/*
 *
 * $Log$
 * Revision 1.3  1998/06/19 16:53:02  dkr
 * fixed a bug in ReuseLet
 *
 * Revision 1.2  1998/06/08 13:48:27  dkr
 * fixed a bug
 *
 * Revision 1.1  1998/06/07 18:43:10  dkr
 * Initial revision
 *
 *
 *
 */

/******************************************************************************
 *
 * This module contains 'GetReuseArray'.
 *
 * 'GetReuseArray' searchs in the given with-loop for reuseable arrays:
 *
 *     A = with (... <= idx < ...) {       A = with (... <= idx < ...) {
 *           <assigns>                       <assigns>
 *         }                               }
 *         genarray( ...)                  modarray( B, ...)
 *
 * In modarray with-loops we can possibly reuse "B".
 * In modarray/genarray with-loops we can possibly reuse all arrays ("C")
 * found in <assings> with the following characteristic:
 *
 *   +) basetype( C) == basetype( A)
 *   +) dim( C) == dim( A)
 *   +) shape( C) == shape( A)
 *
 *   +) "C" does not occur on a left side
 *   +) if "C" occurs on a right side, it looks like
 *          "psi( idx, C)"  or  "idx_psi( idx_flat, C)"
 *      where "idx_flat" is the flat offset of "idx" (IVE).
 *
 *   +) "C" is found in 'NWITH2_DEC_RC_IDS'!!!!!
 *      (otherwise "C" is not consumed by the with-loop because of RCO
 *       --- it is not the last occur --- therefore must not be reused!!!)
 *
 ******************************************************************************/

#include "dbug.h"
#include "types.h"
#include "free.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DataFlowMask.h"
#include "index.h"

/*
 * For the time beeing compile calls 'GetReuseArray' not once for
 * the whole syntax_tree, but for each with-loop.
 * Therefore we better not use the recursive-call-mechanismus.
 */
#define RECURSIVE
#undef RECURSIVE

/******************************************************************************
 *
 * function:
 *   int CompareTypes( types *t1, types *t2)
 *
 * description:
 *   returns ...
 *      ... 0, if 't1' and 't2' are equal types (only basetype, dim, shape);
 *      ... 1, otherwise.
 *
 ******************************************************************************/

int
CompareTypes (types *t1, types *t2)
{
    int compare, d;

    DBUG_ENTER ("CompareTypes");

    compare = ((TYPES_BASETYPE (t1) != TYPES_BASETYPE (t2))
               || (TYPES_DIM (t1) != TYPES_DIM (t2)));

    if (compare == 0) {
        for (d = 0; d < TYPES_DIM (t1); d++) {
            if (SHPSEG_SHAPE (TYPES_SHPSEG (t1), d)
                != SHPSEG_SHAPE (TYPES_SHPSEG (t2), d)) {
                compare = 1;
            }
        }
    }

    DBUG_RETURN (compare);
}

/******************************************************************************
 *
 * function:
 *   int IsFound( char *varname, ids *ids_chain)
 *
 * description:
 *   returns ...
 *     ... 1, if 'varname' is found in 'ids_chain';
 *     ... 0, otherwise.
 *
 ******************************************************************************/

int
IsFound (char *varname, ids *ids_chain)
{
    int found = 0;

    DBUG_ENTER ("IsFound");

    while (ids_chain != NULL) {
        if (strcmp (varname, IDS_NAME (ids_chain)) == 0) {
            found = 1;
            break;
        }
        ids_chain = IDS_NEXT (ids_chain);
    }

    DBUG_RETURN (found);
}

/******************************************************************************
 *
 * function:
 *   node *ReuseFundef( node *arg_node, node *arg_info)
 *
 * description:
 *   stores fundef node in 'INFO_REUSE_FUNDEF( arg_info)'.
 *
 ******************************************************************************/

node *
ReuseFundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ReuseFundef");

    INFO_REUSE_FUNDEF (arg_info) = arg_node;

    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ReuseNwith2( node *arg_node, node *arg_info)
 *
 * description:
 *   generates a new DFM for reuse-arrays and stores it in
 *   'NWITH2_REUSE( arg_node)'.
 *   'INFO_REUSE_MASK( arg_info)' contains a pointer to this mask.
 *   'INFO_REUSE_IDX( arg_info)' contains a pointer to the index vector of
 *    the with-loop.
 *   'INFO_REUSE_DEC_RC_IDS( arg_info)' contains a pointer to
 *   'NWITH2_DEC_RC_IDS( arg_node)'.
 *
 ******************************************************************************/

node *
ReuseNwith2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("REUSENwith2");

    if (NWITH2_REUSE (arg_node) == NULL) {
        if ((NWITH2_TYPE (arg_node) == WO_genarray)
            || (NWITH2_TYPE (arg_node) == WO_modarray)) {
            /*
             * generate new mask for reuse-arrays
             */
            DBUG_ASSERT ((INFO_REUSE_FUNDEF (arg_info) != NULL), "no fundef found");
            NWITH2_REUSE (arg_node)
              = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_REUSE_FUNDEF (arg_info)));
            INFO_REUSE_MASK (arg_info) = NWITH2_REUSE (arg_node);
            INFO_REUSE_IDX (arg_info) = NWITH2_VEC (arg_node);
            INFO_REUSE_DEC_RC_IDS (arg_info) = NWITH2_DEC_RC_IDS (arg_node);
        }
    }

    /*
     * traverse sons
     */
    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ReuseNwithop( node *arg_node, node *arg_info)
 *
 * description:
 *   stores 'NWITHOP_ARRAY( arg_node)' in the reuse-mask
 *   ('INFO_REUSE_MASK( arg_info)').
 *
 ******************************************************************************/

node *
ReuseNwithop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ReuseNwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        break;
    case WO_modarray:
        /*
         * we can possibly reuse the modarray-array.
         */
        if ((NODE_TYPE (NWITHOP_ARRAY (arg_node)) == N_id)
            && (IsFound (ID_NAME (NWITHOP_ARRAY (arg_node)),
                         INFO_REUSE_DEC_RC_IDS (arg_info)))) {
            DFMSetMaskEntrySet (INFO_REUSE_MASK (arg_info),
                                ID_NAME (NWITHOP_ARRAY (arg_node)), NULL);
        }
        break;
    case WO_foldprf:
        /* here is no break missing!! */
    case WO_foldfun:
        NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        break;
    default:
        DBUG_ASSERT ((0), "wrong node type found");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ReuseLet( node *arg_node, node *arg_info)
 *
 * description:
 *   Removes all left hand side ids from the reuse-mask.
 *   If on the right hand side a "psi( idx, A)" or "idx_psi( idx_flat, A)"
 *     where ...
 *       ... "idx" is the index-vector of the current with-loop;
 *       ... "idx_flat" is the flat offset of this index-vector (IVE);
 *       ... "A" has the same type as the with-loop result;
 *   is found, "A" is stored in the reuse-mask.
 *   Otherwise the right hand side is traversed to remove all found id's
 *   from the reuse-mask.
 *
 ******************************************************************************/

node *
ReuseLet (node *arg_node, node *arg_info)
{
    node *arg1, *arg2;
    ids *tmp;
    char *idx_psi_name;
    int traverse;
#ifdef RECURSIVE
    DFMmask_t old_mask;
    ids *old_wl_ids;
    ids *old_idx;
    ids *old_dec_rc_ids;
#endif

    DBUG_ENTER ("ReuseLet");

#ifdef RECURSIVE
    /**************************************************************************
     * recursiv call: first we generate the reuse-mask for the inner with-loop
     */

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_Nwith2) {
        /*
         * push old arg_info
         */
        old_wl_ids = INFO_REUSE_WL_IDS (arg_info);
        old_idx = INFO_REUSE_IDX (arg_info);
        old_dec_rc_ids = INFO_REUSE_DEC_RC_IDS (arg_info);
        old_mask = INFO_REUSE_MASK (arg_info);

        /*
         * generate reuse-mask for inner with-loop
         */
        INFO_REUSE_WL_IDS (arg_info) = LET_IDS (arg_node);
        INFO_REUSE_IDX (arg_info) = NULL;
        INFO_REUSE_DEC_RC_IDS (arg_info) = NULL;
        INFO_REUSE_MASK (arg_info) = NULL;
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

        /*
         * pop old arg_info
         */
        INFO_REUSE_WL_IDS (arg_info) = old_wl_ids;
        INFO_REUSE_IDX (arg_info) = old_idx;
        INFO_REUSE_DEC_RC_IDS (arg_info) = old_dec_rc_ids;
        INFO_REUSE_MASK (arg_info) = old_mask;
    }
#endif

    /*********************************************************
     * now we update the reuse-mask for the current with-loop
     */

    tmp = LET_IDS (arg_node);
    while (tmp != NULL) {
        DFMSetMaskEntryClear (INFO_REUSE_MASK (arg_info), IDS_NAME (tmp), NULL);

        tmp = IDS_NEXT (tmp);
    }

    traverse = 1;
    DBUG_ASSERT ((INFO_REUSE_IDX (arg_info) != NULL), "no idx found");
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_prf) {
        switch (PRF_PRF (LET_EXPR (arg_node))) {
        case F_psi:
            arg1 = EXPRS_EXPR (PRF_ARGS (LET_EXPR (arg_node)));
            arg2 = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (LET_EXPR (arg_node))));
            if ((NODE_TYPE (arg1) == N_id)
                && (strcmp (ID_NAME (arg1), IDS_NAME (INFO_REUSE_IDX (arg_info))) == 0)
                && (NODE_TYPE (arg2) == N_id)
                && (CompareTypes (ID_TYPE (arg2), IDS_TYPE (INFO_REUSE_WL_IDS (arg_info)))
                    == 0)
                && (IsFound (ID_NAME (arg2), INFO_REUSE_DEC_RC_IDS (arg_info)))) {
                /*
                 * 'arg2' is used in a normal WL-psi()
                 *  -> we can possibly reuse this array
                 */
                DFMSetMaskEntrySet (INFO_REUSE_MASK (arg_info), ID_NAME (arg2), NULL);
                /*
                 * we must not traverse the args!
                 */
                traverse = 0;
            }
            break;
        case F_idx_psi:
            arg1 = EXPRS_EXPR (PRF_ARGS (LET_EXPR (arg_node)));
            arg2 = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (LET_EXPR (arg_node))));
            idx_psi_name = IdxChangeId (IDS_NAME (INFO_REUSE_IDX (arg_info)),
                                        IDS_TYPE (INFO_REUSE_WL_IDS (arg_info)));
            if ((NODE_TYPE (arg1) == N_id) && (strcmp (ID_NAME (arg1), idx_psi_name) == 0)
                && (NODE_TYPE (arg2) == N_id)
                && (CompareTypes (ID_TYPE (arg2), IDS_TYPE (INFO_REUSE_WL_IDS (arg_info)))
                    == 0)
                && (IsFound (ID_NAME (arg2), INFO_REUSE_DEC_RC_IDS (arg_info)))) {
                /*
                 * 'arg2' is used in a (flattened) normal WL-psi()
                 *  -> we can possibly reuse this array
                 */
                DFMSetMaskEntrySet (INFO_REUSE_MASK (arg_info), ID_NAME (arg2), NULL);
                /*
                 * we must not traverse the args!
                 */
                traverse = 0;
            }
            FREE (idx_psi_name);
            break;
        default:;
        }
    }

    if (traverse == 1) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ReuseId( node *arg_node, node *arg_info)
 *
 * description:
 *   removes 'arg_node' from the reuse-mask ('INFO_REUSE_MASK( arg_info)'),
 *   because this is an illegal occur of this id.
 *
 ******************************************************************************/

node *
ReuseId (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ReuseId");

    DFMSetMaskEntryClear (INFO_REUSE_MASK (arg_info), ID_NAME (arg_node), NULL);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *GetReuseArrays( node *syntax_tree, node *fundef, ids *wl_ids)
 *
 * description:
 *   starts the traversal to search for reuseable arrays.
 *
 ******************************************************************************/

node *
GetReuseArrays (node *syntax_tree, node *fundef, ids *wl_ids)
{
    node *info;
    funptr *old_tab;

    DBUG_ENTER ("GetReuseArrays");

    info = MakeInfo ();
    INFO_REUSE_FUNDEF (info) = fundef;
    INFO_REUSE_WL_IDS (info) = wl_ids;

    old_tab = act_tab;
    act_tab = reuse_tab;
    syntax_tree = Trav (syntax_tree, info);
    act_tab = old_tab;

    DBUG_RETURN (syntax_tree);
}
