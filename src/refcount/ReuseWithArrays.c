/******************************************************************************
 *
 * $Id$
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
 ******************************************************************************/

#include "ReuseWithArrays.h"

#include "dbug.h"
#include "types.h"
#include "shape.h"
#include "free.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DataFlowMask.h"
#include "free.h"
#include "internal_lib.h"
#include "new_types.h"

#include <string.h>

/*
 * INFO structure
 */
struct INFO {
    node *wl_ids;
    node *iv;
    node *idxs;
    void *mask;
    void *negmask;
};

/*
 * INFO macros
 */
#define INFO_WL_IDS(n) (n->wl_ids)
#define INFO_IV(n) (n->iv)
#define INFO_IDXS(n) (n->idxs)
#define INFO_MASK(n) (n->mask)
#define INFO_NEGMASK(n) (n->negmask)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_WL_IDS (result) = NULL;
    INFO_IV (result) = NULL;
    INFO_IDXS (result) = NULL;
    INFO_MASK (result) = NULL;
    INFO_NEGMASK (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   bool SameSize( ntype *t1, ntype *t2)
 *
 * description:
 *   returns TRUE iff both types have the same size
 *
 *****************************************************************************/
static bool
SameSize (ntype *t1, ntype *t2)
{
    ntype *aks1, *aks2;
    bool res;

    DBUG_ENTER ("SameSize");

    aks1 = TYeliminateAKV (t1);
    aks2 = TYeliminateAKV (t2);

    res = TYisAKS (aks1) && TYeqTypes (aks1, aks2);

    aks1 = TYfreeType (aks1);
    aks2 = TYfreeType (aks2);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *REUSEwith2( node *arg_node, info *arg_info)
 *
 * description:
 *   'INFO_MASK( arg_info)' contains a pointer to the mask of reusable arrays
 *   'INFO_NEGMASK( arg_info)' contains a pointer to the mask of not reuseables
 *
 *   'INFO_IV( arg_info)' contains a pointer to the index vector of
 *     the current with-loop.
 *   'INFO_IDXS( arg_info)' contains a pointer to the offset variables of
 *     the current with-loop.
 *
 ******************************************************************************/

node *
REUSEwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("REUSENwith2");

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);
    WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);

    if (WITH2_CODE (arg_node) != NULL) {
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *REUSEgenarray( node *arg_node, info *arg_info)
 *
 * description:
 *   stores 'GENARRAY_ARRAY( arg_node)' in the reuse-mask
 *   ('INFO_MASK( arg_info)').
 *
 ******************************************************************************/

node *
REUSEgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("REUSEgenarray");

    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);

    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    }

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *REUSEmodarray( node *arg_node, info *arg_info)
 *
 * description:
 *   stores 'MODARRAY_ARRAY( arg_node)' in the reuse-mask
 *   ('INFO_MASK( arg_info)').
 *
 ******************************************************************************/

node *
REUSEmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("REUSEmodarray");

    /*
     * we can possibly reuse the modarray-array.
     */
    if ((NODE_TYPE (MODARRAY_ARRAY (arg_node)) == N_id)
        && (!DFMtestMaskEntry (INFO_NEGMASK (arg_info), NULL,
                               ID_AVIS (MODARRAY_ARRAY (arg_node))))) {
        DFMsetMaskEntrySet (INFO_MASK (arg_info), NULL,
                            ID_AVIS (MODARRAY_ARRAY (arg_node)));
    }

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *REUSEfold( node *arg_node, info *arg_info)
 *
 * description:
 *
 *   ('INFO_MASK( arg_info)').
 *
 ******************************************************************************/

node *
REUSEfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("REUSEfold");

    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    if (FOLD_NEXT (arg_node) != NULL) {
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
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

    if ((NODE_TYPE (arg1) == N_id) && (ID_AVIS (arg1) == ID_AVIS (INFO_IV (arg_info)))
        && (NODE_TYPE (arg2) == N_id)
        && SameSize (ID_NTYPE (arg2), IDS_NTYPE (INFO_WL_IDS (arg_info)))
        && (!DFMtestMaskEntry (INFO_NEGMASK (arg_info), NULL, ID_AVIS (arg2)))) {
        /*
         * 'arg2' is used in a normal WL-sel()
         *  -> we can possibly reuse this array
         */
        DFMsetMaskEntrySet (INFO_MASK (arg_info), NULL, ID_AVIS (arg2));
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
    node *idxs;

    traverse = TRUE;
    idxs = INFO_IDXS (arg_info);

    while (traverse && (idxs != NULL)) {
        if ((NODE_TYPE (arg1) == N_id) && (NODE_TYPE (arg2) == N_id)
            && (ID_AVIS (arg1) == ID_AVIS (EXPRS_EXPR (idxs)))
            && SameSize (ID_NTYPE (arg2), IDS_NTYPE (INFO_WL_IDS (arg_info)))
            && (!DFMtestMaskEntry (INFO_NEGMASK (arg_info), NULL, ID_AVIS (arg2)))) {
            /*
             * 'arg2' is used in a (flattened) normal WL-sel()
             *  -> we can possibly reuse this array
             */
            DFMsetMaskEntrySet (INFO_MASK (arg_info), NULL, ID_AVIS (arg2));
            /*
             * we must not traverse the args!
             */
            traverse = FALSE;
        }

        idxs = EXPRS_NEXT (idxs);
    }
    return (traverse);
}

/******************************************************************************
 *
 * function:
 *   node *REUSElet( node *arg_node, info *arg_info)
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
REUSElet (node *arg_node, info *arg_info)
{
    node *tmp;
    bool traverse;

    DBUG_ENTER ("REUSElet");

    /*
     * removes all left hand side ids from the reuse-mask
     */
    tmp = LET_IDS (arg_node);
    while (tmp != NULL) {
        DFMsetMaskEntryClear (INFO_MASK (arg_info), NULL, IDS_AVIS (tmp));
        DFMsetMaskEntrySet (INFO_NEGMASK (arg_info), NULL, IDS_AVIS (tmp));

        tmp = IDS_NEXT (tmp);
    }

    traverse = TRUE;

    DBUG_ASSERT ((INFO_IV (arg_info) != NULL), "no iv found");

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_prf) {
        node *prf = LET_EXPR (arg_node);

        switch (PRF_PRF (prf)) {
        case F_fill:
            PRF_EXPRS2 (prf) = TRAVdo (PRF_EXPRS2 (prf), arg_info);

            if (NODE_TYPE (PRF_ARG1 (prf)) == N_prf) {
                prf = PRF_ARG1 (prf);

                switch (PRF_PRF (prf)) {
                case F_sel:
                    traverse = ReuseSel (PRF_ARG1 (prf), PRF_ARG2 (prf), arg_info);
                    break;
                case F_idx_sel:
                    traverse = ReuseIdxSel (PRF_ARG1 (prf), PRF_ARG2 (prf), arg_info);
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

        default:
            break;
        }
    }

    if (traverse) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *REUSEid( node *arg_node, info *arg_info)
 *
 * description:
 *   Removes 'arg_node' from the reuse-mask ('INFO_MASK( arg_info)')
 *   and inserts it into the no-reuse-mask ('INFO_NEGMASK( arg_info)'),
 *   because this is an occur on a right hand side of an assignment.
 *
 ******************************************************************************/

node *
REUSEid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("REUSEid");

    DFMsetMaskEntryClear (INFO_MASK (arg_info), NULL, ID_AVIS (arg_node));
    DFMsetMaskEntrySet (INFO_NEGMASK (arg_info), NULL, ID_AVIS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *REUSEdoGetReuseArrays( node *syntax_tree, node *fundef, ids *wl_ids)
 *
 * description:
 *   starts the traversal to search for reuseable arrays.
 *
 ******************************************************************************/

node *
REUSEdoGetReuseArrays (node *with, node *fundef, node *wl_ids)
{
    info *info;
    node *cand = NULL, *avis;

    DBUG_ENTER ("REUSEdoGetReuseArrays");

    DBUG_ASSERT (NODE_TYPE (with) = N_with2, "Illegal Argument!");

    DBUG_ASSERT (FUNDEF_DFM_BASE (fundef) != NULL, "No mask base found!");

    info = MakeInfo ();
    INFO_WL_IDS (info) = wl_ids;
    INFO_MASK (info) = DFMgenMaskClear (FUNDEF_DFM_BASE (fundef));
    INFO_NEGMASK (info) = DFMgenMaskClear (FUNDEF_DFM_BASE (fundef));
    INFO_IV (info) = WITH2_VEC (with);
    INFO_IDXS (info) = WITH2_IDXS (with);

    TRAVpush (TR_reuse);
    with = TRAVdo (with, info);
    TRAVpop ();

    avis = DFMgetMaskEntryAvisSet (INFO_MASK (info));
    while (avis != NULL) {
        cand = TBmakeExprs (TBmakeId (avis), cand);
        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    INFO_MASK (info) = DFMremoveMask (INFO_MASK (info));
    INFO_NEGMASK (info) = DFMremoveMask (INFO_NEGMASK (info));

    info = FreeInfo (info);

    DBUG_RETURN (cand);
}
