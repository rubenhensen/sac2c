/*
 *
 * $Log$
 * Revision 3.13  2004/10/14 13:47:20  sbs
 * adjusted the call to IdxChangeId to shape rather than types
 *
 * Revision 3.12  2004/09/07 11:25:09  khf
 * added support for multioperator WLs
 *
 * Revision 3.11  2004/08/16 11:58:50  ktr
 * Inserted some break after default labels.
 *
 * Revision 3.10  2004/08/13 14:15:03  ktr
 * Fixed a bug in treatment of F_fill which resulted in never traversing
 * the first argument of F_fill.
 *
 * Revision 3.9  2004/07/17 17:07:16  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.8  2004/06/09 09:51:51  ktr
 * adjusted treatment of f_fill to new paramter constellation
 *
 * Revision 3.7  2004/06/08 14:27:46  ktr
 * New Entryfunction GetReuseCandidates yields an N_exprs chain of
 * identifiers which could be reused.
 * Important: NWITH2_DEC_RC_IDS is ignored, the caller must assure that
 * the current reference is the last in the given context.
 *
 * Revision 3.6  2003/11/18 17:16:59  dkr
 * bug fixed: NWITHOP_DEFAULT may be NULL
 *
 * Revision 3.5  2003/11/18 17:02:15  dkr
 * NWITHOP_DEFAULT added
 *
 * Revision 3.4  2002/10/08 17:08:55  dkr
 * Support for dynamic shapes added:
 * TypesAreEqual(), ReuseLet() modified.
 *
 * Revision 3.3  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.2  2001/05/17 12:08:40  dkr
 * FREE, MALLOC eliminated
 *
 * Revision 3.1  2000/11/20 18:01:02  sacbase
 * new release made
 *
 * Revision 2.5  2000/11/05 13:40:02  dkr
 * arg_info is removed now after finishing traversal
 *
 * Revision 2.4  2000/10/31 23:17:58  dkr
 * Trav: NWITH2_CODE might be NULL
 *
 * Revision 2.3  2000/02/04 10:23:59  dkr
 * comment changed
 *
 * Revision 2.2  2000/01/26 17:30:08  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.1  1999/02/23 12:42:24  sacbase
 * new release made
 *
 * Revision 1.6  1999/02/03 16:27:18  dkr
 * fixed a bug in ReuseNwith2() concerning nested WLs
 *
 * Revision 1.5  1998/06/19 19:59:54  dkr
 * fixed a bug:
 *   now a negative mask (INFO_REUSE_NEGMASK) is used, too.
 *
 * Revision 1.4  1998/06/19 17:26:46  dkr
 * fixed a bug in ReuseNwith2:
 *   traversal order is now correct
 *
 * Revision 1.3  1998/06/19 16:53:02  dkr
 * fixed a bug in ReuseLet
 *
 * Revision 1.2  1998/06/08 13:48:27  dkr
 * fixed a bug
 *
 * Revision 1.1  1998/06/07 18:43:10  dkr
 * Initial revision
 *
 */

/******************************************************************************
 *
 * This module contains 'GetReuseArray'.
 *
 * 'GetReuseArray' searchs in the given with-loop for possibly reuseable
 * arrays:
 *
 *     A = with (... <= idx < ...) {       A = with (... <= idx < ...) {
 *           <assigns>                           <assigns>
 *         }                                   }
 *         genarray( ...)                      modarray( B, ...)
 *
 * In modarray with-loops we can possibly reuse "B".
 * In modarray/genarray with-loops we can possibly reuse all arrays ("C")
 * found in <assigns> with the following characteristics:
 *
 *   +) basetype( C) == basetype( A)
 *   +) dim( C) == dim( A)
 *   +) shape( C) == shape( A)  [and these shapes are statically known]
 *
 *   +) "C" does not occur on a left side.
 *      [Because such arrays do not exist outside the with-loop.]
 *
 *   +) If "C" occurs on a right side, it always looks like
 *          "sel( idx, C)"  or  "idx_sel( idx_flat, C)"
 *      where "idx_flat" is the flat offset of "idx" (IVE).
 *      [Otherwise reuse might miss data dependencies!]
 *
 *   +) "C" is found in 'NWITH2_DEC_RC_IDS'!!!!!
 *      [Otherwise "C" is not consumed by the with-loop because of RCO
 *       --- it is not the last occurance --- therefore must not be reused!!!]
 *
 * The arrays that are identified to be reuseable are stored in NWITH2_REUSE.
 *
 ******************************************************************************/

#define NEW_INFO

#include "dbug.h"
#include "types.h"
#include "shape.h"
#include "free.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "index.h"

/*
 * INFO structure
 */
struct INFO {
    ids *wl_ids;
    node *fundef;
    ids *idx;
    ids *dec_rc_ids;
    void *mask;
    void *negmask;
    int nodec;
};

/*
 * INFO macros
 */
#define INFO_REUSE_WL_IDS(n) (n->wl_ids)
#define INFO_REUSE_FUNDEF(n) (n->fundef)
#define INFO_REUSE_IDX(n) (n->idx)
#define INFO_REUSE_DEC_RC_IDS(n) (n->dec_rc_ids)
#define INFO_REUSE_MASK(n) (n->mask)
#define INFO_REUSE_NEGMASK(n) (n->negmask)
#define INFO_REUSE_NODEC(n) (n->nodec)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_REUSE_WL_IDS (result) = NULL;
    INFO_REUSE_FUNDEF (result) = NULL;
    INFO_REUSE_IDX (result) = NULL;
    INFO_REUSE_DEC_RC_IDS (result) = NULL;
    INFO_REUSE_NODEC (result) = 0;
    INFO_REUSE_MASK (result) = NULL;
    INFO_REUSE_NEGMASK (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   bool TypesAreEqual( types *t1, types *t2)
 *
 * description:
 *   returns TRUE iff 't1' and 't2' are equal types (only basetype, dim, shape).
 *
 ******************************************************************************/

bool
TypesAreEqual (types *t1, types *t2)
{
    bool compare;
    shpseg *shpseg1, *shpseg2;
    int dim1, dim2;
    int d;

    DBUG_ENTER ("TypesAreEqual");

    shpseg1 = Type2Shpseg (t1, &dim1);
    shpseg2 = Type2Shpseg (t2, &dim2);

    compare = ((dim1 >= 0) && (dim1 == dim2) && (GetBasetype (t1) == GetBasetype (t2)));

    if (compare) {
        for (d = 0; d < dim1; d++) {
            if (SHPSEG_SHAPE (shpseg1, d) != SHPSEG_SHAPE (shpseg2, d)) {
                compare = FALSE;
            }
        }
    }

    DBUG_RETURN (compare);
}

/******************************************************************************
 *
 * function:
 *   bool IsFound( char *varname, ids *ids_chain)
 *
 * description:
 *   returns TRUE iff 'varname' is found in 'ids_chain'.
 *
 ******************************************************************************/

bool
IsFound (char *varname, ids *ids_chain)
{
    bool found = FALSE;

    DBUG_ENTER ("IsFound");

    while (ids_chain != NULL) {
        if (!strcmp (varname, IDS_NAME (ids_chain))) {
            found = TRUE;
            break;
        }
        ids_chain = IDS_NEXT (ids_chain);
    }

    DBUG_RETURN (found);
}

/******************************************************************************
 *
 * function:
 *   node *ReuseFundef( node *arg_node, info *arg_info)
 *
 * description:
 *   stores fundef node in 'INFO_REUSE_FUNDEF( arg_info)'.
 *
 ******************************************************************************/

node *
ReuseFundef (node *arg_node, info *arg_info)
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
 *   node *ReuseNwith2( node *arg_node, info *arg_info)
 *
 * description:
 *   generates a new DFM for reuse-arrays and stores it in
 *   'NWITH2_REUSE( arg_node)'.
 *   'INFO_REUSE_MASK( arg_info)' contains a pointer to this mask.
 *
 *   generates a new DFM for no-reuse-arrays and stores it in
 *   'INFO_REUSE_NEGMASK( arg_info)'.
 *
 *   'INFO_REUSE_FUNDEF( arg_info)' contains a pointer to the current fundef
 *     node.
 *   'INFO_REUSE_IDX( arg_info)' contains a pointer to the index vector of
 *     the current with-loop.
 *   'INFO_REUSE_NODEC( arg_info)' contains a flag specifying whether
 *     'INFO_REUSE_DEC_RC_IDS( arg_info)' shall be ignored
 *   'INFO_REUSE_DEC_RC_IDS( arg_info)' contains a pointer to
 *     'NWITH2_DEC_RC_IDS( arg_node)'.
 *
 ******************************************************************************/

node *
ReuseNwith2 (node *arg_node, info *arg_info)
{
    node *withop;
    bool gen_mod_wl = FALSE;

    DBUG_ENTER ("REUSENwith2");

    if (INFO_REUSE_MASK (arg_info) == NULL) {
        /*
         * This is the upper-most with-loop
         *   -> Create new reuse-mask
         */

        withop = NWITH2_WITHOP (arg_node);
        while (withop != NULL) {
            if ((NWITHOP_TYPE (withop) == WO_genarray)
                || (NWITHOP_TYPE (withop) == WO_modarray)) {
                gen_mod_wl = TRUE;
                break;
            }
            withop = NWITHOP_NEXT (withop);
        }

        if (gen_mod_wl) {
            /*
             * Generate new mask for reuse-arrays
             */
            DBUG_ASSERT ((INFO_REUSE_FUNDEF (arg_info) != NULL), "no fundef found");
            NWITH2_REUSE (arg_node)
              = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_REUSE_FUNDEF (arg_info)));
            INFO_REUSE_MASK (arg_info) = NWITH2_REUSE (arg_node);
            INFO_REUSE_NEGMASK (arg_info)
              = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_REUSE_FUNDEF (arg_info)));
            INFO_REUSE_IDX (arg_info) = NWITH2_VEC (arg_node);
            if (!INFO_REUSE_NODEC (arg_info)) {
                INFO_REUSE_DEC_RC_IDS (arg_info) = NWITH2_DEC_RC_IDS (arg_node);
            }
        }
    } else {
        /*
         * This is an inner with-loop
         *   -> Do not build a new reuse-mask, because the current traversal
         *        concerns the upper-most with-loop only!!!
         *      'Compile' will call 'GetReuseArrays()' oncemore for this WL!
         */
    }

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ReuseNwithop( node *arg_node, info *arg_info)
 *
 * description:
 *   stores 'NWITHOP_ARRAY( arg_node)' in the reuse-mask
 *   ('INFO_REUSE_MASK( arg_info)').
 *
 ******************************************************************************/

node *
ReuseNwithop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ReuseNwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        if (NWITHOP_DEFAULT (arg_node) != NULL) {
            NWITHOP_DEFAULT (arg_node) = Trav (NWITHOP_DEFAULT (arg_node), arg_info);
        }
        break;
    case WO_modarray:
        /*
         * we can possibly reuse the modarray-array.
         */
        if ((NODE_TYPE (NWITHOP_ARRAY (arg_node)) == N_id)
            && ((INFO_REUSE_NODEC (arg_info))
                || (IsFound (ID_NAME (NWITHOP_ARRAY (arg_node)),
                             INFO_REUSE_DEC_RC_IDS (arg_info))))
            && (DFMTestMaskEntry (INFO_REUSE_NEGMASK (arg_info),
                                  ID_NAME (NWITHOP_ARRAY (arg_node)), NULL)
                == 0)) {
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

    if (NWITHOP_NEXT (arg_node) != NULL) {
        NWITHOP_NEXT (arg_node) = Trav (NWITHOP_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ReuseSel( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
bool
ReuseSel (node *arg1, node *arg2, info *arg_info)
{
    if ((NODE_TYPE (arg1) == N_id)
        && (!strcmp (ID_NAME (arg1), IDS_NAME (INFO_REUSE_IDX (arg_info))))
        && (NODE_TYPE (arg2) == N_id)
        && TypesAreEqual (ID_TYPE (arg2), IDS_TYPE (INFO_REUSE_WL_IDS (arg_info)))
        && ((INFO_REUSE_NODEC (arg_info))
            || (IsFound (ID_NAME (arg2), INFO_REUSE_DEC_RC_IDS (arg_info))))
        && (DFMTestMaskEntry (INFO_REUSE_NEGMASK (arg_info), ID_NAME (arg2), NULL)
            == 0)) {
        /*
         * 'arg2' is used in a normal WL-sel()
         *  -> we can possibly reuse this array
         */
        DFMSetMaskEntrySet (INFO_REUSE_MASK (arg_info), ID_NAME (arg2), NULL);
        /*
         * we must not traverse the args!
         */
        return (FALSE);
    } else {
        return (TRUE);
    }
}

/******************************************************************************
 *
 * function:
 *   node *ReuseIdxSel( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
bool
ReuseIdxSel (node *arg1, node *arg2, info *arg_info)
{
    bool traverse;
    types *type;
    shape *shp;
    char *idx_sel_name;

    traverse = TRUE;

    if ((NODE_TYPE (arg1) == N_id) && (NODE_TYPE (arg2) == N_id)
        && TypesAreEqual (ID_TYPE (arg2), IDS_TYPE (INFO_REUSE_WL_IDS (arg_info)))
        && ((INFO_REUSE_NODEC (arg_info))
            || (IsFound (ID_NAME (arg2), INFO_REUSE_DEC_RC_IDS (arg_info))))
        && (DFMTestMaskEntry (INFO_REUSE_NEGMASK (arg_info), ID_NAME (arg2), NULL)
            == 0)) {

        type = IDS_TYPE (INFO_REUSE_WL_IDS (arg_info));
        shp = Type2Shape (type);
        idx_sel_name = IdxChangeId (IDS_NAME (INFO_REUSE_IDX (arg_info)), shp);
        shp = SHFreeShape (shp);

        if (!strcmp (ID_NAME (arg1), idx_sel_name)) {
            /*
             * 'arg2' is used in a (flattened) normal WL-sel()
             *  -> we can possibly reuse this array
             */
            DFMSetMaskEntrySet (INFO_REUSE_MASK (arg_info), ID_NAME (arg2), NULL);
            /*
             * we must not traverse the args!
             */
            traverse = FALSE;
        }
        idx_sel_name = Free (idx_sel_name);
    }
    return (traverse);
}

/******************************************************************************
 *
 * function:
 *   node *ReuseLet( node *arg_node, info *arg_info)
 *
 * description:
 *   Removes all left hand side ids from the reuse-mask (and stores them into
 *     the no-reuse-mask).
 *   If on the right hand side a "sel( idx, A)" or "idx_sel( idx_flat, A)"
 *     where ...
 *       ... "idx" is the index-vector of the current with-loop;
 *       ... "idx_flat" is the flat offset of this index-vector (IVE);
 *       ... "A" has the same type as the with-loop result;
 *     is found, "A" is stored in the reuse-mask.
 *   Otherwise the right hand side is traversed to remove all found id's
 *     from the reuse-mask (and store them into the no-reuse-mask).
 *
 ******************************************************************************/

node *
ReuseLet (node *arg_node, info *arg_info)
{
    node *arg1, *arg2, *tmpnode;
    ids *tmp;
    bool traverse;

    DBUG_ENTER ("ReuseLet");

    /*
     * removes all left hand side ids from the reuse-mask
     */
    tmp = LET_IDS (arg_node);
    while (tmp != NULL) {
        DFMSetMaskEntryClear (INFO_REUSE_MASK (arg_info), IDS_NAME (tmp), NULL);
        DFMSetMaskEntrySet (INFO_REUSE_NEGMASK (arg_info), IDS_NAME (tmp), NULL);

        tmp = IDS_NEXT (tmp);
    }

    traverse = TRUE;
    DBUG_ASSERT ((INFO_REUSE_IDX (arg_info) != NULL), "no idx found");
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_prf) {
        switch (PRF_PRF (LET_EXPR (arg_node))) {
        case F_fill:
            PRF_EXPRS2 (LET_EXPR (arg_node))
              = Trav (PRF_EXPRS2 (LET_EXPR (arg_node)), arg_info);
            tmpnode = PRF_ARGS (LET_EXPR (arg_node));
            if (NODE_TYPE (EXPRS_EXPR (tmpnode)) == N_prf) {
                switch (PRF_PRF (EXPRS_EXPR (tmpnode))) {
                case F_sel:
                    arg1 = EXPRS_EXPR (PRF_ARGS (EXPRS_EXPR (tmpnode)));
                    arg2 = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (EXPRS_EXPR (tmpnode))));
                    traverse = ReuseSel (arg1, arg2, arg_info);
                    break;

                case F_idx_sel:
                    arg1 = EXPRS_EXPR (PRF_ARGS (EXPRS_EXPR (tmpnode)));
                    arg2 = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (EXPRS_EXPR (tmpnode))));
                    traverse = ReuseIdxSel (arg1, arg2, arg_info);
                    break;

                default:
                    break;
                }
            }
            break;
        case F_alloc:
        case F_alloc_or_reuse:
            /* Probably the first two arguments should be traversed */
            traverse = FALSE;
            break;
        case F_sel:
            arg1 = EXPRS_EXPR (PRF_ARGS (LET_EXPR (arg_node)));
            arg2 = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (LET_EXPR (arg_node))));
            traverse = ReuseSel (arg1, arg2, arg_info);
            break;

        case F_idx_sel:
            arg1 = EXPRS_EXPR (PRF_ARGS (LET_EXPR (arg_node)));
            arg2 = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (LET_EXPR (arg_node))));
            traverse = ReuseIdxSel (arg1, arg2, arg_info);
            break;

        default:
            break;
        }
    }

    if (traverse) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ReuseId( node *arg_node, info *arg_info)
 *
 * description:
 *   Removes 'arg_node' from the reuse-mask ('INFO_REUSE_MASK( arg_info)')
 *   and inserts it into the no-reuse-mask ('INFO_REUSE_NEGMASK( arg_info)'),
 *   because this is an occur on a right hand side of an assignment.
 *
 ******************************************************************************/

node *
ReuseId (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ReuseId");

    DFMSetMaskEntryClear (INFO_REUSE_MASK (arg_info), ID_NAME (arg_node), NULL);
    DFMSetMaskEntrySet (INFO_REUSE_NEGMASK (arg_info), ID_NAME (arg_node), NULL);

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
    info *info;
    funtab *old_tab;

    DBUG_ENTER ("GetReuseArrays");

    info = MakeInfo ();
    INFO_REUSE_FUNDEF (info) = fundef;
    INFO_REUSE_WL_IDS (info) = wl_ids;
    INFO_REUSE_NODEC (info) = FALSE;

    old_tab = act_tab;
    act_tab = reuse_tab;
    syntax_tree = Trav (syntax_tree, info);
    act_tab = old_tab;

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   node *GetReuseCandidates( node *syntax_tree, node *fundef, ids *wl_ids)
 *
 * description:
 *   starts the traversal to search for reuseable arrays.
 *   - Ignores whether an ids is in INFO_REUSE_DEC_RC_IDS
 *   - returns an N_exprs chain of Reuse Candidates
 *
 ******************************************************************************/

node *
GetReuseCandidates (node *syntax_tree, node *fundef, ids *wl_ids)
{
    info *info;
    funtab *old_tab;

    DBUG_ENTER ("GetReuseArrays");

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_Nwith2, "Illegal Node Type");

    info = MakeInfo ();
    INFO_REUSE_FUNDEF (info) = fundef;
    INFO_REUSE_WL_IDS (info) = wl_ids;
    INFO_REUSE_NODEC (info) = TRUE;

    old_tab = act_tab;
    act_tab = reuse_tab;
    syntax_tree = Trav (syntax_tree, info);
    act_tab = old_tab;

    info = FreeInfo (info);

    DBUG_RETURN (Ids2Exprs (DFM2LetIds (NWITH2_REUSE (syntax_tree), NULL)));
}
