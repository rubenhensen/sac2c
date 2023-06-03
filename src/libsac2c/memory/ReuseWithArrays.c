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
 *
 * In modarray/genarray with-loops we can possibly reuse all arrays ("C")
 * found in <assigns> with the following characteristics:
 *
 *   +) "C" does not occur on a left side.
 *      [Because such arrays do not exist outside the with-loop.]
 *
 *   +) If "C" occurs on a right side, it always looks like
 *          "sel( idx, C)"
 *      where the current index-vector of the potentially nested
 *      withloops is a prefix of idx. I.e, the index vector has
 *      no offset.
 *
 *      [Otherwise reuse might miss data dependencies!]
 *
 *
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

#define DBUG_PREFIX "WRCI_S"
#include "debug.h"

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
#include "pattern_match.h"
#include "indexvectorutils.h"
#include "DupTree.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *iv;
    node *ivids;
    dfmask_t *mask;
    dfmask_t *negmask;
    node *partn;
};

#define INFO_IV(n) ((n)->iv)
#define INFO_IVIDS(n) ((n)->ivids)
#define INFO_MASK(n) ((n)->mask)
#define INFO_NEGMASK(n) ((n)->negmask)
#define INFO_PARTN(n) ((n)->partn)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_IV (result) = NULL;
    INFO_IVIDS (result) = NULL;
    INFO_MASK (result) = NULL;
    INFO_NEGMASK (result) = NULL;
    INFO_PARTN (result) = NULL;

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

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (with) == N_with, "Illegal Argument!");

    maskbase = DFMgenMaskBase (FUNDEF_ARGS (fundef), FUNDEF_VARDECS (fundef));

    info = MakeInfo ();

    INFO_MASK (info) = DFMgenMaskClear (maskbase);
    INFO_NEGMASK (info) = DFMgenMaskClear (maskbase);

    TRAVpush (TR_reuse);
    with = TRAVdo (with, info);
    TRAVpop ();

    avis = DFMgetMaskEntryAvisSet (INFO_MASK (info));
    while (avis != NULL) {
        cand = TBmakeExprs (TBmakeId (avis), cand);
        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    DBUG_PRINT ("Initializing dataflow mask");
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
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn bool IsValidIndex( node *index, node *ivs, node *ivids)
 *
 * @brief Checks whether the index refers to elements within the
 *        sub-pane given by ivs and ivids.
 *
 * @param index node representing index under investigation
 * @param ivs   set of index vectors
 * @param ivids set of index scalars
 * @param partn N_part of innermost WL
 *
 * @return true iff the index is valid
 *
 *
 *****************************************************************************/
static bool
IsValidIndexHelper (node *index, node **ivs, node **ivids, node *partn)
{
    node *array;
    node *aexprs = NULL;
    node *iv1 = NULL;
    node *iv2 = NULL;
    node *iv = NULL;
    node *ivavis = NULL;
    pattern *pat1, *pat2;
    pattern *pat3;
    pattern *pat4;
    node *index2 = NULL;
    bool result = FALSE;
    bool madeNid = FALSE;
    node *el = NULL;
    node *ids = NULL;
    node *idsid = NULL;

    DBUG_ENTER ();

    /*
     * index can be:
     *
     * Case 1:
     *
     * iv1 ++ iv2 : result is true iff  iv1 is prefix of current ivs
     *                                  && iv2 is valid index for the remainder
     *
     *              Since this function is recursive, it also supports:
     *                iv1 ++ iv2 ++iv2
     *              and Case 1 in combination with Case 2 and Case 3.
     *
     * Case 2:
     *
     * [i0, i1, i2, i3, ...] => Case 2a: the scalars are a prefix of ivids, or
     *                          Case 2b: ivids is a prefix of the scalars
     *
     *  NB. Case 2a is not implemented here, from what I can see.
     *
     * Case 3:
     *
     *     iv : result is true iff iv is in ivs
     *
     * Case 4: index is an offset for _idx_sel( offset, mat), created
     *         by vect2offset or idxs2offset.
     *
     */

    pat1 = PMprf (1, PMAisPrf (F_cat_VxV), 2, PMvar (1, PMAgetNode (&iv1), 0),
                  PMvar (1, PMAgetNode (&iv2), 0));
    pat2 = PMarray (1, PMAgetNode (&array), 1, PMskip (0));
    pat3 = PMprf (1, PMAisPrf (F_vect2offset), 2, PMvar (1, PMAgetNode (&iv1), 0),
                  PMvar (1, PMAgetNode (&iv2), 0));
    pat4 = PMvar (1, PMAisVar (&el), 0);

    // First, we try to map Case 4 into Case 2.

    if (PMmatchFlat (pat3, index)) {
        iv = IVUTfindOffset2Iv (index);
        if (NULL != iv) {
            index2 = iv;
        }
    }

    if (index2 == NULL) {
        ivavis = IVUTfindIvWithid (index, partn);
        if (NULL != ivavis) {
            index2 = TBmakeId (ivavis);
            madeNid = TRUE;
        }
    }

    index2 = (NULL == index2) ? index : index2;

    // Now, do the dirty work.

    if (PMmatchFlat (pat1, index2)) {
        // Case 1:  ivs matches iv1 ++ iv2
        result = IsValidIndexHelper (iv1, ivs, ivids, partn)
                 && IsValidIndexHelper (iv2, ivs, ivids, partn);

    } else if (PMmatchFlat (pat2, index2)) {
        // Case 2b: ivids matches prefix of scalars
        // ivids is a vector of index vectors, e.g., if we have
        // a nest of WLs, with these generators:
        //   iv1 = [i,j];
        //   iv2 = [k];
        //   then, ivids =  ( [i,j], [k] ),
        //   and we obtain a (perhaps partial) match if array = [ i, j, k,...]
        //
        result = TRUE;
        aexprs = ARRAY_AELEMS (array);

        while (result &&                        /* we don't know better */
               (*ivids != NULL) &&              /* still more nesting levels */
               (SET_MEMBER (*ivids) != NULL) && /* this level has idx scalars */
               (aexprs != NULL)) {              /* more elements in index */

            ids = SET_MEMBER (*ivids);
            while (result && (NULL != aexprs) && (NULL != ids)) {
                idsid = TBmakeId (IDS_AVIS (ids));
                el = EXPRS_EXPR (aexprs);
                // N_num is a valid index, but PMmatchFlat will croak on it.
                result = (N_num == NODE_TYPE (el)) || PMmatchFlat (pat4, idsid);
                idsid = FREEdoFreeNode (idsid);
                aexprs = EXPRS_NEXT (aexprs);
                ids = IDS_NEXT (ids);
            }

            *ivs = SET_NEXT (*ivs);
            *ivids = SET_NEXT (*ivids);
        }
    } else if (NODE_TYPE (index2) == N_id) {
        // Case 3: index matches WITHID_VEC.
        if (IVUTisIvMatchesWithid (index2, SET_MEMBER (*ivs), SET_MEMBER (*ivids))) {
            *ivs = SET_NEXT (*ivs);
            *ivids = SET_NEXT (*ivids);
            result = TRUE;
        }
    }

    index2 = madeNid ? FREEdoFreeNode (index2) : NULL;

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);

    DBUG_RETURN (result);
}

static bool
IsValidIndex (node *index, node *ivs, node *ivids, node *partn)
{
    bool result;

    DBUG_ENTER ();

    result = IsValidIndexHelper (index, &ivs, &ivids, partn) && (ivs == NULL)
             && (ivids == NULL);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
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
 *   'INFO_IV( arg_info)' contains a set of pointers to the index vectors
 *                        in the current scope
 *   'INFO_IVIDS( arg_info)' contains a set of pointers to the index scalars
 *                           in the current scope
 *
 ******************************************************************************/

node *
REUSEwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* Decide what info we want to collect */
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *REUSEpart( node *arg_node, info *arg_info)
 *
 * description:
 *   'INFO_MASK( arg_info)' contains a pointer to the mask of reusable arrays
 *   'INFO_NEGMASK( arg_info)' contains a pointer to the mask of not reuseables
 *
 *   'INFO_IV( arg_info)' contains a set of pointers to the index vectors
 *                        in the current scope
 *   'INFO_IVIDS( arg_info)' contains a set of pointers to the index scalars
 *                           in the current scope
 e
 *  With the advent of ssaiv, we have to perform this set membership
 *  operation at the partition level, because WITHIDS differ for each of them.
 ******************************************************************************/

node *
REUSEpart (node *arg_node, info *arg_info)
{
    node *oldpartn;

    DBUG_ENTER ();

    oldpartn = INFO_PARTN (arg_info);
    INFO_PARTN (arg_info) = arg_node;
    /*
     * add current index information to sets
     */
    INFO_IV (arg_info)
      = TCappendSet (INFO_IV (arg_info), TBmakeSet (PART_VEC (arg_node), NULL));
    INFO_IVIDS (arg_info)
      = TCappendSet (INFO_IVIDS (arg_info), TBmakeSet (PART_IDS (arg_node), NULL));

    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);

    /*
     * pop one level from index information
     */
    INFO_IV (arg_info) = TCdropSet (-1, INFO_IV (arg_info));
    INFO_IVIDS (arg_info) = TCdropSet (-1, INFO_IVIDS (arg_info));

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    INFO_PARTN (arg_info) = oldpartn;

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
    DBUG_ENTER ();

    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);

    GENARRAY_DEFAULT (arg_node) = TRAVopt (GENARRAY_DEFAULT (arg_node), arg_info);

    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    /*
     * we can possibly reuse the modarray-array.
     */
    if ((NODE_TYPE (MODARRAY_ARRAY (arg_node)) == N_id)
        && (!DFMtestMaskEntry (INFO_NEGMASK (arg_info),
                               ID_AVIS (MODARRAY_ARRAY (arg_node))))) {

        DFMsetMaskEntrySet (INFO_MASK (arg_info),
                            ID_AVIS (MODARRAY_ARRAY (arg_node)));
    }

    MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    /*
     * removes all left hand side ids from the reuse-mask
     */

    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

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
    DBUG_ENTER ();

    if ((PRF_PRF (arg_node) == F_sel_VxA) || (PRF_PRF (arg_node) == F_idx_sel)) {
        DBUG_PRINT ("selection found into %s", AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
        if (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id) {
            if (!DFMtestMaskEntry (INFO_NEGMASK (arg_info),
                                   ID_AVIS (PRF_ARG2 (arg_node)))) {
                DBUG_PRINT ("%s not yet in DF mask",
                            AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
                if (IsValidIndex (PRF_ARG1 (arg_node), INFO_IV (arg_info),
                                  INFO_IVIDS (arg_info), INFO_PARTN (arg_info))) {
                    DBUG_PRINT ("%s is valid index; adding to DF mask",
                                AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));

                    /*
                     * 'arg2' is used in a WL-sel that only references
                     * elements of the sub-pane of the current iteration
                     *  -> we can possibly reuse this array
                     */
                    DFMsetMaskEntrySet (INFO_MASK (arg_info),
                                        ID_AVIS (PRF_ARG2 (arg_node)));
                } else {
                    DBUG_PRINT ("%s has invalid index: not suitable for reuse",
                                AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
                    DFMsetMaskEntryClear (INFO_MASK (arg_info),
                                          ID_AVIS (PRF_ARG2 (arg_node)));
                    DFMsetMaskEntrySet (INFO_NEGMASK (arg_info),
                                        ID_AVIS (PRF_ARG2 (arg_node)));
                }
            }
        }
    } else {
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
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
    DBUG_ENTER ();

    /*
     * remove left hand side ids from the reuse-mask
     */
    DBUG_PRINT ("%s is lhs ids: removed from DFM", AVIS_NAME (IDS_AVIS (arg_node)));
    DFMsetMaskEntryClear (INFO_MASK (arg_info), IDS_AVIS (arg_node));
    DFMsetMaskEntrySet (INFO_NEGMASK (arg_info), IDS_AVIS (arg_node));

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

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
 *   because this is a reference on the right hand side of an assignment.
 *
 ******************************************************************************/

node *
REUSEid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("%s is ref on rhs: removed from DFM", AVIS_NAME (ID_AVIS (arg_node)));
    DFMsetMaskEntryClear (INFO_MASK (arg_info), ID_AVIS (arg_node));
    DFMsetMaskEntrySet (INFO_NEGMASK (arg_info), ID_AVIS (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Reuseable with-loop argument inference -->
 *****************************************************************************/

#undef DBUG_PREFIX
