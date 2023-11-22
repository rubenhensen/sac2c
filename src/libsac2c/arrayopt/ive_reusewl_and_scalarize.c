#include "ive_reusewl_and_scalarize.h"

#define DBUG_PREFIX "IVERAS"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "DupTree.h"
#include "free.h"
#include "str.h"
#include "memory.h"
#include "compare_tree.h"
#include "globals.h"
#include "pattern_match.h"
#include "flattengenerators.h"
#include "ctinfo.h"
#ifndef DBUG_OFF
#include "print.h"
#endif

/**
 * forward declarations
 */
typedef struct OFFSETINFO offsetinfo;
typedef struct IVINFO ivinfo;

/**
 * INFO structure
 */
struct INFO {
    ivinfo *mivinfo;
    node *lhs;
    node *vardecs;
    node *fundef;
    node *preassigns;
    bool onefundef;
};

/**
 * INFO macros
 */
#define INFO_IVINFO(n) ((n)->mivinfo)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_IVINFO (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_ONEFUNDEF (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * WITHIV information structure.
 *
 * this structure is used to store withid x shape -> offset
 * information to ease the lookup of matching withloop
 * offsets for a given vect2offset operation.
 *
 * IV            stores the avis of the index vector
 *               Under ssaiv, these avises differ for each WL partition.
 *
 * OFFSETS       contains all offsetvar avis and their corresponding
 *               shapeexpression
 *
 * LOCALOFFSETS  contains offset that are computed "locally" within the
 *               current block
 *
 * SCALARS       contains the N_ids chain of scalars that can be used instead
 *               of the index vector
 *               Under ssaiv, these avises differ for each WL partition.
 */

struct OFFSETINFO {
    node *avis;
    node *shapeexpr;
    offsetinfo *next;
};

struct IVINFO {
    node *iv;
    offsetinfo *offsets;
    offsetinfo *localoffsets;
    node *scalars;
    ivinfo *next;
};

/**
 * WITHIV macros
 */
#define WITHIV_IV(n) ((n)->iv)
#define WITHIV_OFFSETS(n) ((n)->offsets)
#define WITHIV_LOCALOFFSETS(n) ((n)->localoffsets)
#define WITHIV_SCALARS(n) ((n)->scalars)
#define WITHIV_NEXT(n) ((n)->next)
#define WITHOFFSET_AVIS(n) ((n)->avis)
#define WITHOFFSET_SHAPEEXPR(n) ((n)->shapeexpr)
#define WITHOFFSET_NEXT(n) ((n)->next)

/**
 * WITHIV functions
 */

/** <!-- ****************************************************************** -->
 * @fn node* Nids2Nid( node *withids)
 *
 * @brief Convert a with_ids chain into a chain of N_id nodes.
 *
 * @param The N_ids chain.
 *
 * @return The N_id chain.
 *
 ******************************************************************************/
static node *
Nids2Nid (node *withids)
{
    node *z = NULL;

    DBUG_ENTER ();

    if (NULL != withids) {
        z = TBmakeExprs (TBmakeId (IDS_AVIS (withids)), (NULL != IDS_NEXT (withids))
                                                          ? Nids2Nid (IDS_NEXT (withids))
                                                          : NULL);
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 * @fn offsetinfo *GenOffsetInfo( node *lhs, node *withops)
 *
 * @brief Generates the offset information table for a given set of withops
 *        and withloop lhs. This structure contains a mapping from shape
 *        expressions of the results to the appropriate withloop offsets
 *        and vice versa.
 *
 * @param lhs      left-hand-sides of the withloop at hand
 * @param withops  withops if the withloop at hand
 *
 * @return structure containing the offset information
 ******************************************************************************/
static offsetinfo *
GenOffsetInfo (node *lhs, node *withops)
{
    offsetinfo *result;
    offsetinfo *next;

    DBUG_ENTER ();

    if (lhs != NULL) {
        DBUG_ASSERT (withops != NULL, "# withops does not match # lhs ids");

        next = GenOffsetInfo (IDS_NEXT (lhs), WITHOP_NEXT (withops));

        if (((NODE_TYPE (withops) == N_genarray)
             || (NODE_TYPE (withops) == N_modarray))) {
            /*
             * only genarray and modarray wls have a built in index.
             */

            DBUG_PRINT_TAG ("IVEO", "adding offset %s and lhs id %s",
                            AVIS_NAME (WITHOP_IDX (withops)), AVIS_NAME (IDS_AVIS (lhs)));

            result = (offsetinfo *)MEMmalloc (sizeof (offsetinfo));

            WITHOFFSET_SHAPEEXPR (result) = AVIS_SHAPE (IDS_AVIS (lhs));
            WITHOFFSET_AVIS (result) = WITHOP_IDX (withops);
            WITHOFFSET_NEXT (result) = next;
        } else {
            result = next;
        }
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn offsetinfo *FreeOffsetInfo( offsetinfo *info)
 *
 * @brief Frees a offsetinfo structure.
 *
 * @param info the structure to free
 *
 * @return NULL pointer
 ******************************************************************************/
static offsetinfo *
FreeOffsetInfo (offsetinfo *info)
{
    DBUG_ENTER ();

    if (info != NULL) {
        WITHOFFSET_NEXT (info) = FreeOffsetInfo (WITHOFFSET_NEXT (info));

        info = MEMfree (info);
    }

    DBUG_RETURN (info);
}

/** <!-- ****************************************************************** -->
 * @fn ivinfo *PushIV( ivinfo *info, node *withid, node *lhs, node *withops)
 *
 * @brief Push the given withid on the withloop information stack given
 *        by the first argument. The lhs and withops arguments are used
 *        for constructing the offset information table.
 *
 * @param info    withloop information structure
 * @param withid  withloop index-vector id avis-node
 * @param lhs     left hand sides of the withloop at hand
 * @param withops withops of the withloop at hand
 *
 * @return
 ******************************************************************************/
static ivinfo *
PushIV (ivinfo *info, node *withid, node *lhs, node *withops)
{
    ivinfo *result;

    DBUG_ENTER ();

    DBUG_PRINT ("adding WL withid %s", AVIS_NAME (IDS_AVIS (WITHID_VEC (withid))));

    result = (ivinfo *)MEMmalloc (sizeof (ivinfo));

    if (global.ssaiv) { /* Fill in WITHIV at N_part node */
        WITHIV_IV (result) = NULL;
        WITHIV_SCALARS (result) = NULL;
    } else {
        WITHIV_IV (result) = IDS_AVIS (WITHID_VEC (withid));
        WITHIV_SCALARS (result) = WITHID_IDS (withid);
    }

    WITHIV_OFFSETS (result) = GenOffsetInfo (lhs, withops);
    WITHIV_LOCALOFFSETS (result) = NULL;
    WITHIV_NEXT (result) = info;

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Removes the top most withloop index from the given withloop
 *        information stack.
 *
 * @param info withloop information stack
 *
 * @return withloop information stack with topmost element popped off.
 ******************************************************************************/
static ivinfo *
PopIV (ivinfo *info)
{
    ivinfo *result;

    DBUG_ENTER ();

    DBUG_ASSERT (info != NULL, "IVINFO stack already empty!");

    DBUG_PRINT ("removing withid %s", AVIS_NAME (WITHIV_IV (info)));

    result = WITHIV_NEXT (info);

    WITHIV_OFFSETS (info) = FreeOffsetInfo (WITHIV_OFFSETS (info));
    WITHIV_LOCALOFFSETS (info) = FreeOffsetInfo (WITHIV_LOCALOFFSETS (info));

    info = (ivinfo *)MEMfree ((void *)info);

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn ivinfo *PushLocalOffset( ivinfo *ivinfo, node *avis, node *shapeexpr)
 *
 * @brief Adds a local offset to the offsets of the topmost element of
 *        the given withloop information stack. This is used to keep track
 *        of offsets generated by calls to _vect2offset_ or _idxs2offset_
 *        within the body of withloops.
 *
 * @param ivinfo    withloop information stack
 * @param avis      avis node of the index offset
 * @param shapeexpr shapeexpr this offset indexes into
 *
 * @return
 ******************************************************************************/
static ivinfo *
PushLocalOffset (ivinfo *ivinfo, node *avis, node *shapeexpr)
{
    offsetinfo *newinfo;

    DBUG_ENTER ();

    newinfo = (offsetinfo *)MEMmalloc (sizeof (offsetinfo));

    WITHOFFSET_SHAPEEXPR (newinfo) = shapeexpr;
    WITHOFFSET_AVIS (newinfo) = avis;
    WITHOFFSET_NEXT (newinfo) = WITHIV_LOCALOFFSETS (ivinfo);

    WITHIV_LOCALOFFSETS (ivinfo) = newinfo;

    DBUG_RETURN (ivinfo);
}

/** <!-- ****************************************************************** -->
 * @fn ivinfo *PopLocalOffsets( ivinfo *ivinfo)
 *
 * @brief Removes all local offsets from the top-most element of the
 *        withloop stack. This needs to be done whenever a withloop
 *        code block is left and thus the scope changes which invalidates
 *        these local indexes.
 *
 * @param ivinfo withloop information stack
 *
 * @return modified withloop information stack
 ******************************************************************************/
static ivinfo *
PopLocalOffsets (ivinfo *ivinfo)
{
    DBUG_ENTER ();

    WITHIV_LOCALOFFSETS (ivinfo) = FreeOffsetInfo (WITHIV_LOCALOFFSETS (ivinfo));

    DBUG_RETURN (ivinfo);
}

/** <!-- ****************************************************************** -->
 * @fn node *FindIVOffset( ivinfo *info, node *iv, node *shapeexpr)
 *
 * @brief Returns the avis node of the offset for indexing into an array
 *        of the shape represented by the given shape expression and iv
 *        as index. Returns NULL if no offset can be found.
 *
 * @param info        withloop information stack
 * @param iv          avis of index vector used for the selection
 * @param shapeexpr  shape expression of array used for the selection
 *
 * @return avis node of the offset or NULL
 ******************************************************************************/
static node *
FindIVOffset (ivinfo *info, node *iv, node *shapeexpr)
{
    node *result = NULL;
    offsetinfo *oinfo;
    pattern *pat;
    node *arrayRestA, *arrayRestB;
    int one = 1;

    DBUG_ENTER ();

    DBUG_PRINT ("looking up offset for %s.", AVIS_NAME (iv));

    while ((info != NULL) && (WITHIV_IV (info) != iv)) {
        info = WITHIV_NEXT (info);
    }

    pat
      = PMmulti (2,
                 PMarray (0, /* don't care for attributes */
                          2, PMskipN (&one, 0), PMskip (1, PMAgetNode (&arrayRestA))),
                 PMarray (0, 2, PMskipN (&one, 0), PMskip (1, PMAgetNode (&arrayRestB))));

    if (info != NULL) {
        /*
         * search global wl-offset-vars
         */
        oinfo = WITHIV_OFFSETS (info);

        while ((oinfo != NULL)
               && (CMPTdoCompareTree (shapeexpr, WITHOFFSET_SHAPEEXPR (oinfo)) != CMPT_EQ)
               && ((!PMmatchFlat (pat, PMmultiExprs (2, shapeexpr,
                                                     WITHOFFSET_SHAPEEXPR (oinfo))))
                   || ((arrayRestA == NULL) && (NULL != arrayRestB))
                   || ((arrayRestA != NULL) && (NULL == arrayRestB))
                   || !((arrayRestA == arrayRestB)
                        || (CMPTdoCompareTree (arrayRestA, arrayRestB) == CMPT_EQ)))) {
            DBUG_PRINT ("no match");
            DBUG_EXECUTE (PRTdoPrintNode (shapeexpr);
                          PRTdoPrintNode (WITHOFFSET_SHAPEEXPR (oinfo)));
            oinfo = WITHOFFSET_NEXT (oinfo);
        }

        if (oinfo != NULL) {
            result = WITHOFFSET_AVIS (oinfo);
        } else {
            /*
             * search local offsets
             */
            oinfo = WITHIV_LOCALOFFSETS (info);

            while (
              (oinfo != NULL)
              && (CMPTdoCompareTree (shapeexpr, WITHOFFSET_SHAPEEXPR (oinfo)) != CMPT_EQ)
              && ((!PMmatchFlat (pat, PMmultiExprs (2, shapeexpr,
                                                    WITHOFFSET_SHAPEEXPR (oinfo))))
                  || ((arrayRestA == NULL) && (NULL != arrayRestB))
                  || ((arrayRestA != NULL) && (NULL == arrayRestB))
                  || !((arrayRestA == arrayRestB)
                       || (CMPTdoCompareTree (arrayRestA, arrayRestB) == CMPT_EQ)))) {
                oinfo = WITHOFFSET_NEXT (oinfo);
            }

            if (oinfo != NULL) {
                result = WITHOFFSET_AVIS (oinfo);
            }
        }
    }

    pat = PMfree (pat);

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *FindIVScalars( ivinfo *info, node *iv)
 *
 * @brief Returns the scalars belonging to the index vector given as
 *        second argument. If no scalars can be found (e.g. AUD withloop)
 *        NULL is returned.
 *
 * @param info withloop information stack
 * @param iv   avis of index vector
 *
 * @return IDS chain if scalars belonging to the iv or NULL
 ******************************************************************************/
static node *
FindIVScalars (ivinfo *info, node *iv)
{
    node *result = NULL;

    DBUG_ENTER ();

    while ((info != NULL) && (WITHIV_IV (info) != iv)) {
        info = WITHIV_NEXT (info);
    }

    if (info != NULL) {
        result = WITHIV_SCALARS (info);
    }

    DBUG_RETURN (result);
}

/**
 * optimizer functions
 */

/** <!-- ****************************************************************** -->
 * @fn node *ReplaceByWithOffset( node *arg_node, info *arg_info)
 *
 * @brief Tries to replace the _vect2offset_ referenced by arg_node by
 *        a withloop index. If that does not work out, a conversion into
 *        _idxs2offset_ is tried.
 *
 * @param arg_node _vect2offset_ prf
 * @param arg_info info structure
 *
 * @return transformed _vect2offset_ prf
 ******************************************************************************/
static node *
ReplaceByWithOffset (node *arg_node, info *arg_info)
{
    node *offset;
    node *scalars;

    DBUG_ENTER ();

    DBUG_ASSERT (INFO_IVINFO (arg_info) != NULL,
                 "found an id which was identified as a withid (no SSAASSIGN "
                 "and not N_arg) although not inside a withloop.");

    offset = FindIVOffset (INFO_IVINFO (arg_info), ID_AVIS (PRF_ARG2 (arg_node)),
                           PRF_ARG1 (arg_node));

    if ((offset != NULL) && (global.iveo & IVEO_wlidx)) {
        DBUG_PRINT_TAG ("IVEO", "replacing vect2offset by wlidx %s",
                        AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));

        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TBmakeId (offset);
    } else if (global.iveo & IVEO_idxs) {
        DBUG_PRINT ("replacing vect2offset by wl-idxs2offset");

        scalars = FindIVScalars (INFO_IVINFO (arg_info), ID_AVIS (PRF_ARG2 (arg_node)));
        scalars = Nids2Nid (scalars);
        if (scalars != NULL) {
            offset
              = TBmakePrf (F_idxs2offset,
                           TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)), scalars));
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = offset;
        }

        /*
         * this offset is a non-wl-var offset but a locally
         * generated one, so we store it within the ivinfo
         */
        INFO_IVINFO (arg_info)
          = PushLocalOffset (INFO_IVINFO (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                             PRF_ARG1 (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *FlattenEachExprsNode( node *arg_node, info *arg_info)
 *
 * @brief Flattens N_exprs nodes whose contents are, e.g.,
 *           z = [ 1, 2, 3]
 *        to be:
 *           t1 = 1;
 *           t2 = 2;
 *           t3 = 3;
 *           z = [ t1, t2, t3];
 *
 *        This is required so that later phases, such as the TC,
 *        are not offended by the presence of non-flattened N_prf
 *        arguments.
 *
 *        Eventually, the post-optimization CP traversal will
 *        reunflatten these nodes.
 *
 * @param arg_node an N_exprs chain
 * @param arg_info  info structure
 *
 * @return The flattened version of the chain.
 *
 ******************************************************************************/
static node *
FlattenEachExprsNode (node *arg_node, info *arg_info)
{
    node *z;
    node *newz;
    node *exprs;
    ntype *typ;

    DBUG_ENTER ();

    z = NULL;

    exprs = arg_node;
    while (NULL != exprs) {
        typ = TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0));
        newz = FLATGexpression2Avis (DUPdoDupNode (EXPRS_EXPR (exprs)),
                                     &INFO_VARDECS (arg_info),
                                     &INFO_PREASSIGNS (arg_info), typ);
        z = TCappendExprs (z, TBmakeExprs (TBmakeId (newz), NULL));
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 * @fn node *ReplaceByIdx2Offset( node *arg_node, info *arg_info)
 *
 * @brief Tries to replace a _vect2offset_ operation by an _idxs2offset_
 *        operation whereever possible.
 *
 * @param arg_node _vect2offset_ prf
 * @param arg_info  info structure
 *
 * @return  transformed _vect2offset_ prf
 ******************************************************************************/
static node *
ReplaceByIdx2Offset (node *arg_node, info *arg_info)
{
    node *result;
    node *ivassign;
    node *idxs;
    node *shape;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id,
                 "ReplaceByIdx2Offset called with iv being non N_id node");
    DBUG_ASSERT (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node))) != NULL,
                 "ReplaceByIdx2Offset with AVIS_SSAASSIGN of iv being NULL");

    ivassign = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node)));

    DBUG_ASSERT (NODE_TYPE (ASSIGN_RHS (ivassign)) == N_array,
                 "ReplaceByIdx2Offset with non N_array AVIS_SSAASSIGN");

    idxs = ARRAY_AELEMS (ASSIGN_RHS (ivassign));
    shape = PRF_ARG1 (arg_node);

    if ((TCcountExprs (idxs) == 1)
        && (((NODE_TYPE (shape) == N_id) && (TUshapeKnown (AVIS_TYPE (ID_AVIS (shape))))
             && (SHgetExtent (TYgetShape (AVIS_TYPE (ID_AVIS (shape))), 0) == 1))
            || ((NODE_TYPE (shape) == N_array)
                && (TCcountExprs (ARRAY_AELEMS (shape)) == 1)))) {
        /* trivial case: the index is the offset :-) */
        DBUG_PRINT ("replacing by trivial offset (1d case)");
        result = DUPdoDupTree (EXPRS_EXPR (idxs));
    } else {
        DBUG_PRINT ("replacing by idxs2offset");
        idxs = FlattenEachExprsNode (idxs, arg_info);
        result = TBmakePrf (F_idxs2offset,
                            TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)), idxs));
    }

    arg_node = FREEdoFreeNode (arg_node);

    DBUG_RETURN (result);
}

/**
 * traversal functions
 */

/** <!-- ****************************************************************** -->
 * @brief node *IVERASassign( node *arg_node, info *arg_info)
 *
 * @param N_assign node and info node.
 *
 * @return Updated N_assign node. Side effect is to handle
 *         any preassigns created by flattening N_array elements.
 *
 ******************************************************************************/
node *
IVERASassign (node *arg_node, info *arg_info)
{
    node *oldpreassigns;

    DBUG_ENTER ();

    oldpreassigns = INFO_PREASSIGNS (arg_info);
    INFO_PREASSIGNS (arg_info) = NULL;

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    if (NULL != INFO_PREASSIGNS (arg_info)) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    INFO_PREASSIGNS (arg_info) = oldpreassigns;

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief node *IVERASlet( node *arg_node, info *arg_info)
 *
 * @param N_let node and info node.
 *
 * @return
 *
 ******************************************************************************/
node *
IVERASlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    DBUG_PRINT ("Looking at %s...", IDS_NAME (LET_IDS (arg_node)));

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief node *IVERASpart( node *arg_node, info *arg_info)
 *  Under -ssaiv, each WL partition has its own set of WITH_IDs.
 *  Hence, these have to be pushed info the INFO block at the
 *  N_part node , rather than at the IVERASwith node.
 *
 *  We traverse the code block for the partition after setting up
 *  the WITHIDS.
 *
 * @param N_part node and info node.
 *
 * @return arg_node is unchanged, but arg_info updated as side effect.
 *
 ******************************************************************************/
node *
IVERASpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (global.ssaiv) {
        DBUG_PRINT ("Adding partition withid %s",
                    AVIS_NAME (IDS_AVIS (WITHID_VEC (PART_WITHID (arg_node)))));
        WITHIV_IV (INFO_IVINFO (arg_info))
          = IDS_AVIS (WITHID_VEC (PART_WITHID (arg_node)));
        WITHIV_SCALARS (INFO_IVINFO (arg_info)) = WITHID_IDS (PART_WITHID (arg_node));
    }

    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief node *IVERASwith( node *arg_node, info *arg_info)
 *
 *
 * @param N_part node and info node.
 *
 * @return updated arg_node
 *
 ******************************************************************************/
node *
IVERASwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Handling WL for partition 0 WITHID: %s",
                AVIS_NAME (IDS_AVIS (WITHID_VEC (WITH_WITHID (arg_node)))));

    INFO_IVINFO (arg_info) = PushIV (INFO_IVINFO (arg_info), WITH_WITHID (arg_node),
                                     INFO_LHS (arg_info), WITH_WITHOP (arg_node));

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    INFO_IVINFO (arg_info) = PopIV (INFO_IVINFO (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief node *IVERAScode( node *arg_node, info *arg_info)
 *
 *
 * @param N_part node and info node.
 *
 * @return updated arg_node
 *
 ******************************************************************************/

node *
IVERAScode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    /*
     * remove local withoffsets gathered within the block
     */
    INFO_IVINFO (arg_info) = PopLocalOffsets (INFO_IVINFO (arg_info));

    CODE_NEXT (arg_node) = TRAVopt(CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief node *IVERASprf( node *arg_node, info *arg_info)
 *
 * @param N_prf node and info node.
 *
 * @return updated arg_node
 *
 ******************************************************************************/
node *
IVERASprf (node *arg_node, info *arg_info)
{
    node *ivarg;
    node *ivassign;

    DBUG_ENTER ();

    if (PRF_PRF (arg_node) == F_vect2offset) {
        ivarg = PRF_ARG2 (arg_node);
        ivassign = AVIS_SSAASSIGN (ID_AVIS (ivarg));

        if (ivassign != NULL) {
            if ((NODE_TYPE (ASSIGN_RHS (ivassign)) == N_array)
                && (global.iveo & IVEO_idxs)) {
                /*
                 * this index vector is defined as a array of
                 * scalars.
                 */
                DBUG_PRINT ("Trying to scalarise vect2offset for iv %s...",
                            AVIS_NAME (ID_AVIS (ivarg)));

                arg_node = ReplaceByIdx2Offset (arg_node, arg_info);
            }
        } else {
            if (NODE_TYPE (AVIS_DECL (ID_AVIS (ivarg))) != N_arg) {
                /*
                 * This id has no defining assignment and is not an argument.
                 * The only possible reason for this is that this id is a
                 * withloop index vector. as this is a local vect2offset
                 * within a withloop code block, we have to remember
                 * the lhs avis for this shape for later use. This is done
                 * by ReplaceByWithOffset.
                 */
                DBUG_PRINT ("Trying to replace vect2offset for iv %s by wlidx...",
                            AVIS_NAME (ID_AVIS (ivarg)));

                arg_node = ReplaceByWithOffset (arg_node, arg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief node *IVERASfundef( node *arg_node, info *arg_info)
 *
 * @param N_fundef node and info node.
 *
 * @return updated arg_node
 *
 ******************************************************************************/
node *
IVERASfundef (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    DBUG_PRINT ("IVERAS in %s %s begins",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    /* If new vardecs were made, append them to the current set */
    if (INFO_VARDECS (arg_info) != NULL) {
        BLOCK_VARDECS (FUNDEF_BODY (arg_node))
          = TCappendVardec (INFO_VARDECS (arg_info),
                            BLOCK_VARDECS (FUNDEF_BODY (arg_node)));
        INFO_VARDECS (arg_info) = NULL;
    }
    DBUG_PRINT ("IVERAS in %s %s ends",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Reuse Withloop Offset and Index Vector Scalarization on the given
 *        syntax tree.
 *
 * @param syntax_tree the syntax tree to optimize
 *        or N_fundef if we are in single function mode
 *
 * @return optimized syntax tree/ N_fundef
 ******************************************************************************/
node *
IVERASdoWithloopReuseAndOptimisation (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    TRAVpush (TR_iveras);

    arg_info = MakeInfo ();
    INFO_ONEFUNDEF (arg_info) = (N_fundef == NODE_TYPE (arg_node));

    arg_node = TRAVdo (arg_node, arg_info);

    arg_info = FreeInfo (arg_info);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
