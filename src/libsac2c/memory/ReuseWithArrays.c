/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup REUSE Reusable With-loop Argument Inference
 *
 * Provides an inference for searching a given with-loop for possibly reuseable
 * arrays.
 *
 * <pre>
 *     A = with (... <= idx < ...) {       A = with (... <= idx < ...) {
 *           <assigns>                           <assigns>
 *         }                                   }
 *         genarray( ...)                      modarray( B, ...)
 *
 * In modarray with-loops we can possibly reuse "B".
 * In modarray/genarray with-loops we can possibly reuse all arrays ("C")
 * found in <assigns> with the following characteristics:
 *
 *   +) "C" does not occur on a left side.
 *      [Because such arrays do not exist outside the with-loop.]
 *
 *   +) If "C" occurs on a right side, it always looks like
 *          "sel( idx, C)"
 *      [Otherwise reuse might miss data dependencies!]
 * </pre>
 *
 * @ingroup mm
 *
 * @{
 *
 ******************************************************************************/

/** <!--********************************************************************-->
 *
 * @file ReuseWithArrays.c
 *
 * Prefix: REUSE
 *
 *****************************************************************************/
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
#include "str.h"
#include "memory.h"
#include "new_types.h"

#include <string.h>

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *iv;
    void *mask;
    void *negmask;
};

#define INFO_IV(n) (n->iv)
#define INFO_MASK(n) (n->mask)
#define INFO_NEGMASK(n) (n->negmask)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_IV (result) = NULL;
    INFO_MASK (result) = NULL;
    INFO_NEGMASK (result) = NULL;

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
 * @fn node *REUSEdoGetReuseArrays( node *with-loop, node *fundef)
 *
 * @brief starts the traversal to search for reuseable arrays.
 *
 *****************************************************************************/
node *
REUSEdoGetReuseArrays (node *with, node *fundef)
{
    info *info;
    node *cand = NULL, *avis;
    dfmask_base_t *maskbase;

    DBUG_ENTER ("REUSEdoGetReuseArrays");

    DBUG_ASSERT (NODE_TYPE (with) == N_with, "Illegal Argument!");

    maskbase = DFMgenMaskBase (FUNDEF_ARGS (fundef), FUNDEF_VARDEC (fundef));

    info = MakeInfo ();

    INFO_MASK (info) = DFMgenMaskClear (maskbase);
    INFO_NEGMASK (info) = DFMgenMaskClear (maskbase);
    INFO_IV (info) = WITH_VEC (with);

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
    maskbase = DFMremoveMaskBase (maskbase);

    info = FreeInfo (info);

    DBUG_RETURN (cand);
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

/******************************************************************************
 *
 * function:
 *   node *REUSEwith( node *arg_node, info *arg_info)
 *
 * description:
 *   'INFO_MASK( arg_info)' contains a pointer to the mask of reusable arrays
 *   'INFO_NEGMASK( arg_info)' contains a pointer to the mask of not reuseables
 *
 *   'INFO_IV( arg_info)' contains a pointer to the index vector of
 *     the current with-loop.
 *
 ******************************************************************************/

node *
REUSEwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("REUSENwith");

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

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
 *   node *REUSElet( node *arg_node, info *arg_info)
 *
 * description:
 *   Removes all left hand side ids from the reuse-mask (and stores them into
 *     the no-reuse-mask).
 *   If on the right hand side a "sel( idx, A)" where "idx" is the
 *   index-vector of the current with-loop "A" is stored in the reuse-mask.
 *   Otherwise the right hand side is traversed to remove all found id's
 *     from the reuse-mask (and store them into the no-reuse-mask).
 *
 *****************************************************************************/

node *
REUSElet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("REUSElet");

    /*
     * removes all left hand side ids from the reuse-mask
     */
    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *REUSEprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
REUSEprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("REUSEprf");

    if ((PRF_PRF (arg_node) == F_sel) && (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id)
        && (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id)
        && (ID_AVIS (PRF_ARG1 (arg_node)) == IDS_AVIS (INFO_IV (arg_info)))
        && (!DFMtestMaskEntry (INFO_NEGMASK (arg_info), NULL,
                               ID_AVIS (PRF_ARG2 (arg_node))))) {

        /*
         * 'arg2' is used in a normal WL-sel()
         *  -> we can possibly reuse this array
         */
        DFMsetMaskEntrySet (INFO_MASK (arg_info), NULL, ID_AVIS (PRF_ARG2 (arg_node)));
    } else {
        if (PRF_ARGS (arg_node) != NULL) {
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *REUSEids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
REUSEids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("REUSEids");

    /*
     * remove left hand side ids from the reuse-mask
     */
    DFMsetMaskEntryClear (INFO_MASK (arg_info), NULL, IDS_AVIS (arg_node));
    DFMsetMaskEntrySet (INFO_NEGMASK (arg_info), NULL, IDS_AVIS (arg_node));

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
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

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Reuseable with-loop argument inference -->
 *****************************************************************************/
