/* $Id$ */

#include "ive_reusewl_and_scalarize.h"

#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "free.h"
#include "str.h"
#include "memory.h"
#include "compare_tree.h"
#include "globals.h"

/**
 * forward declarations
 */
typedef struct OFFSETINFO offsetinfo;
typedef struct IVINFO ivinfo;

/**
 * INFO structure
 */
struct INFO {
    ivinfo *ivinfo;
    node *lhs;
    node *vardecs;
    node *fundef;
};

/**
 * INFO macros
 */
#define INFO_IVINFO(n) ((n)->ivinfo)
#define INFO_LHS(n) ((n)->lhs)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_IVINFO (result) = NULL;
    INFO_LHS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

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
 * IV            stores the avis of the indexvector
 * OFFSETS       contains all offsetvar avis and their corresponding
 *               shapeexpression
 * LOCALOFFSETS  contains offset that are computed "locally" within the
 *               current block
 * SCALARS       contains the N_ids chain of scalars that can be used instead
 *               of the index vector
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

    DBUG_ENTER ("GenOffsetInfo");

    if (lhs != NULL) {
        DBUG_ASSERT ((withops != NULL), "# withops does not match # lhs ids");

        next = GenOffsetInfo (IDS_NEXT (lhs), WITHOP_NEXT (withops));

        if (((NODE_TYPE (withops) == N_genarray)
             || (NODE_TYPE (withops) == N_modarray))) {
            /*
             * only genarray and modarray wls have a built in index.
             */

            DBUG_PRINT ("IVEO", ("adding offset %s and lhs id %s", WITHOP_IDX (withops),
                                 AVIS_NAME (IDS_AVIS (lhs))));

            result = MEMmalloc (sizeof (offsetinfo));

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
    DBUG_ENTER ("FreeOffsetInfo");

    if (info != NULL) {
        WITHOFFSET_NEXT (info) = FreeOffsetInfo (WITHOFFSET_NEXT (info));

        info = MEMfree (info);
    }

    DBUG_RETURN (info);
}

/** <!-- ****************************************************************** -->
 * @fn ivinfo *PushIV( ivinfo *info, node *withid, node *lhs, node *withops)
 *
 * @brief Pushed the given withid on the withloop information stack given
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

    DBUG_ENTER ("PushIV");

    DBUG_PRINT ("IVERAS",
                ("adding withid %s", AVIS_NAME (IDS_AVIS (WITHID_VEC (withid)))));

    result = MEMmalloc (sizeof (ivinfo));

    WITHIV_IV (result) = IDS_AVIS (WITHID_VEC (withid));
    WITHIV_OFFSETS (result) = GenOffsetInfo (lhs, withops);
    WITHIV_LOCALOFFSETS (result) = NULL;
    WITHIV_SCALARS (result) = WITHID_IDS (withid);
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

    DBUG_ENTER ("PopIV");

    DBUG_ASSERT ((info != NULL), "IVINFO stack already empty!");

    DBUG_PRINT ("IVERAS", ("removing withid %s", AVIS_NAME (WITHIV_IV (info))));

    result = WITHIV_NEXT (info);

    WITHIV_OFFSETS (info) = FreeOffsetInfo (WITHIV_OFFSETS (info));
    WITHIV_LOCALOFFSETS (info) = FreeOffsetInfo (WITHIV_LOCALOFFSETS (info));

    info = MEMfree (info);

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

    DBUG_ENTER ("PushLocalOffset");

    newinfo = MEMmalloc (sizeof (offsetinfo));

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
    DBUG_ENTER ("PopLocalOffsets");

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

    DBUG_ENTER ("FindIVOffset");

    DBUG_PRINT ("IVERAS", ("looking up offset for %s.", AVIS_NAME (iv)));

    while ((info != NULL) && (WITHIV_IV (info) != iv)) {
        info = WITHIV_NEXT (info);
    }

    if (info != NULL) {
        /*
         * search global wl-offset-vars
         */
        oinfo = WITHIV_OFFSETS (info);

        while (
          (oinfo != NULL)
          && (CMPTdoCompareTree (shapeexpr, WITHOFFSET_SHAPEEXPR (oinfo)) != CMPT_EQ)) {
            oinfo = WITHOFFSET_NEXT (oinfo);
        }

        if (oinfo != NULL) {
            result = WITHOFFSET_AVIS (oinfo);
        } else {
            /*
             * search local offsets
             */
            oinfo = WITHIV_LOCALOFFSETS (info);

            while ((oinfo != NULL)
                   && (CMPTdoCompareTree (shapeexpr, WITHOFFSET_SHAPEEXPR (oinfo))
                       != CMPT_EQ)) {
                oinfo = WITHOFFSET_NEXT (oinfo);
            }

            if (oinfo != NULL) {
                result = WITHOFFSET_AVIS (oinfo);
            }
        }
    }

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

    DBUG_ENTER ("FindIVScalars");

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

    DBUG_ENTER ("ReplaceByWithOffset");

    DBUG_ASSERT ((INFO_IVINFO (arg_info) != NULL),
                 "found an id which was identified as a withid (no SSAASSIGN "
                 "and not N_arg) although not inside a withloop.");

    offset = FindIVOffset (INFO_IVINFO (arg_info), ID_AVIS (PRF_ARG2 (arg_node)),
                           PRF_ARG1 (arg_node));

    if ((offset != NULL) && (global.iveo & IVEO_wlidx)) {
        DBUG_PRINT ("IVEO", ("replacing vect2offset by wlidx %s",
                             AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node)))));

        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TBmakeId (offset);
    } else if (global.iveo & IVEO_idxs) {
        DBUG_PRINT ("IVERAS", ("replacing vect2offset by wl-idxs2offset"));

        scalars = FindIVScalars (INFO_IVINFO (arg_info), ID_AVIS (PRF_ARG2 (arg_node)));

        if (scalars != NULL) {
            offset
              = TBmakePrf (F_idxs2offset, TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                                                       TCids2Exprs (scalars)));
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

    DBUG_ENTER ("ReplaceByIdx2Offset");

    DBUG_ASSERT ((NODE_TYPE (PRF_ARG2 (arg_node)) == N_id),
                 "ReplaceByIdx2Offset called with iv being non N_id node");
    DBUG_ASSERT ((AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node))) != NULL),
                 "ReplaceByIdx2Offset with AVIS_SSAASSIGN of iv being NULL");

    ivassign = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node)));

    DBUG_ASSERT ((NODE_TYPE (ASSIGN_RHS (ivassign)) == N_array),
                 "ReplaceByIdx2Offset with non N_array AVIS_SSAASSIGN");

    DBUG_PRINT ("IVERAS", ("replacing by idxs2offset"));

    idxs = ARRAY_AELEMS (ASSIGN_RHS (ivassign));

    result = TBmakePrf (F_idxs2offset, TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                                                    DUPdoDupTree (idxs)));

    arg_node = FREEdoFreeNode (arg_node);

    DBUG_RETURN (result);
}

/**
 * traversal functions
 */
node *
IVERASlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVERASlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    DBUG_PRINT ("IVERAS", ("Looking at %s...", IDS_NAME (LET_IDS (arg_node))));

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
IVERASwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVERASwith");

    INFO_IVINFO (arg_info) = PushIV (INFO_IVINFO (arg_info), WITH_WITHID (arg_node),
                                     INFO_LHS (arg_info), WITH_WITHOP (arg_node));

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    INFO_IVINFO (arg_info) = PopIV (INFO_IVINFO (arg_info));

    DBUG_RETURN (arg_node);
}

node *
IVERAScode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVERAScode");

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    /*
     * remove local withoffsets gathered within the block
     */
    INFO_IVINFO (arg_info) = PopLocalOffsets (INFO_IVINFO (arg_info));

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IVERASprf (node *arg_node, info *arg_info)
{
    node *ivarg;
    node *ivassign;

    DBUG_ENTER ("IVERASprf");

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
                DBUG_PRINT ("IVERAS", ("Trying to scalarise vect2offset for iv %s...",
                                       AVIS_NAME (ID_AVIS (ivarg))));

                arg_node = ReplaceByIdx2Offset (arg_node, arg_info);
            }
        } else {
            if (NODE_TYPE (AVIS_DECL (ID_AVIS (ivarg))) != N_arg) {
                /*
                 * this id has no defining assignment and is no argument.
                 * the only possible reason for this is, that this id is a
                 * withloop index vector. as this is a local vect2offset
                 * within a withloop code block, we have to remember
                 * the lhs avis for this shape for later use. This is done
                 * by ReplaceByWithOffset.
                 */
                DBUG_PRINT ("IVERAS",
                            ("Trying to replace vect2offset for iv %s by wlidx...",
                             AVIS_NAME (ID_AVIS (ivarg))));

                arg_node = ReplaceByWithOffset (arg_node, arg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Reuse Withloop Offset and Index Vector Scalarization on the given
 *        syntax tree.
 *
 * @param syntax_tree the syntax tree to optimize
 *
 * @return optimized syntax tree
 ******************************************************************************/
node *
IVERASdoWithloopReuseAndOptimisation (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IVEIdoWithloopReuseAndOptimisation");

    TRAVpush (TR_iveras);

    info = MakeInfo ();

    syntax_tree = TRAVdo (syntax_tree, info);

    info = FreeInfo (info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}
