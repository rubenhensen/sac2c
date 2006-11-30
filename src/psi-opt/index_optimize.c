/* $Id$ */

#include "index_optimize.h"

#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "free.h"
#include "internal_lib.h"
#include "type_utils.h"
#include "new_types.h"
#include "shape_cliques.h"
#include "shape.h"
#include "constants.h"
#include "makedimexpr.h"
#include "wrci.h"

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
    node *preassigns;
    node *fundef;
};

/**
 * INFO macros
 */
#define INFO_IVINFO(n) ((n)->ivinfo)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_FUNDEF(n) ((n)->fundef)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_IVINFO (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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
 * OFFSETS       contains all offsetvar avis and their corresponding clique
 * LOCALOFFSETS  contains offset that are computed "locally" within the
 *               current block
 * SCALARS       contains the N_ids chain of scalars that can be used instead
 *               of the index vector
 */

struct OFFSETINFO {
    node *avis;
    node *clique;
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
#define WITHOFFSET_CLIQUE(n) ((n)->clique)
#define WITHOFFSET_NEXT(n) ((n)->next)

/**
 * WITHIV functions
 */
/** <!-- ****************************************************************** -->
 * @fn offsetinfo *GenOffsetInfo( node *lhs, node *withops)
 *
 * @brief Generates the offset information table for a given set of withops
 *        and withloop lhs. This structure contains a mapping from shape
 *        cliques of the results to the appropriate withloop offsets
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

            DBUG_PRINT ("IVEO", ("adding offset %s and clique member %s",
                                 WITHOP_IDX (withops), AVIS_NAME (IDS_AVIS (lhs))));

            result = ILIBmalloc (sizeof (offsetinfo));

            WITHOFFSET_CLIQUE (result) = IDS_AVIS (lhs);
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

        info = ILIBfree (info);
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

    DBUG_PRINT ("IVEO", ("adding withid %s", AVIS_NAME (IDS_AVIS (WITHID_VEC (withid)))));

    result = ILIBmalloc (sizeof (ivinfo));

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

    DBUG_PRINT ("IVEO", ("removing withid %s", AVIS_NAME (WITHIV_IV (info))));

    result = WITHIV_NEXT (info);

    WITHIV_OFFSETS (info) = FreeOffsetInfo (WITHIV_OFFSETS (info));
    WITHIV_LOCALOFFSETS (info) = FreeOffsetInfo (WITHIV_LOCALOFFSETS (info));

    info = ILIBfree (info);

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn ivinfo *PushLocalOffset( ivinfo *ivinfo, node *avis, node *clique)
 *
 * @brief Adds a local offset to the offsets of the topmost element of
 *        the given withloop information stack. This is used to keep track
 *        of offsets generated by calls to _vect2offset_ or _idxs2offset_
 *        within the body of withloops.
 *
 * @param ivinfo withloop information stack
 * @param avis   avis node of the index offset
 * @param clique clique this offset indexes into
 *
 * @return
 ******************************************************************************/
static ivinfo *
PushLocalOffset (ivinfo *ivinfo, node *avis, node *clique)
{
    offsetinfo *newinfo;

    DBUG_ENTER ("PushLocalOffset");

    newinfo = ILIBmalloc (sizeof (offsetinfo));

    WITHOFFSET_CLIQUE (newinfo) = clique;
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
 * @fn node *FindIVOffset( ivinfo *info, node *iv, node *clique)
 *
 * @brief Returns the avis node of the offset for indexing into the
 *        shape clique given by the clique argument using the iv argument
 *        as index. Returns NULL if no offset can be found.
 *
 * @param info    withloop information stack
 * @param iv      avis of index vector used for the selection
 * @param clique  shape clique of array used for the selection
 *
 * @return avis node of the offset or NULL
 ******************************************************************************/
static node *
FindIVOffset (ivinfo *info, node *iv, node *clique)
{
    node *result = NULL;
    offsetinfo *oinfo;

    DBUG_ENTER ("FindIVOffset");

    DBUG_PRINT ("IVEO", ("looking up offset for %s and clique member %s", AVIS_NAME (iv),
                         AVIS_NAME (clique)));

    while ((info != NULL) && (WITHIV_IV (info) != iv)) {
        info = WITHIV_NEXT (info);
    }

    if (info != NULL) {
        /*
         * search global wl-offset-vars
         */
        oinfo = WITHIV_OFFSETS (info);

        while ((oinfo != NULL) && (!ShapeVarsMatch (clique, WITHOFFSET_CLIQUE (oinfo)))) {
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
                   && (!ShapeVarsMatch (clique, WITHOFFSET_CLIQUE (oinfo)))) {
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

/** <!-- ****************************************************************** -->
 * @fn node *GetAvis4Clique( node *iv, node *clique, info *arg_info)
 *
 * @brief Returns the avis of the offset variable that can be used for a
 *        selection into an array from the given clique, instead of the
 *        given index vector. If none can be found, NULL is returned.
 *
 * @param iv        avis of index vector, iv, used for selection
 * @param clique    clique of the array X, used for selection in X[iv]
 * @param arg_info  info structure (needed for fundef reference)
 *
 * @return avis of matching offset or NULL
 ******************************************************************************/
static node *
GetAvis4Clique (node *iv, node *clique, info *arg_info)
{
    node *result = NULL;
    node *cliques;
    node *ids;

    DBUG_ENTER ("GetAvis4Clique");

    cliques = AVIS_IDXSHAPES (iv);
    ids = AVIS_IDXIDS (iv);

    if ((cliques != NULL) && (ids == NULL)) {
        /*
         * we have an index vector of a withloop here! as offsets
         * for wl-indexvectors are local w.r.t. the current code
         * block, they are not annotated to the withiv-avis. so
         * we need to look it up in the withiv info structure!
         */
        result = FindIVOffset (INFO_IVINFO (arg_info), iv, clique);
    } else {
        while (cliques != NULL) {
            DBUG_ASSERT ((ids != NULL), "# of ids does not match # of cliques");

            if (ShapeVarsMatch (clique, ID_AVIS (EXPRS_EXPR (cliques)))) {
                result = IDS_AVIS (ids);
                break;
            }

            cliques = EXPRS_NEXT (cliques);
            ids = IDS_NEXT (ids);
        }
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *Scalar2Offset( node *scalar, int dims, node *shape,
 *                          info *arg_info)
 * @brief Creates code to compute the offset into an array of a given
 *        shape using a scalar value as index. The scalar value is first
 *        expanded into a vector of correct length (using the dims argument)
 *        and then a _idxs2offset_ call is constructed. The shape argument
 *        needs to point to a N_array expression defining the shape
 *        of the array that is used for selection. None of the arguments
 *        is consumed.
 *
 * @param scalar    node defining a scalar value
 * @param dims      number of dimensions of the array used for selection
 * @param shape     node defining the shape of the array used for selection
 * @param arg_info  info node (needed to insert new assignments)
 *
 * @return Avis referencing the computed offset
 ******************************************************************************/
static node *
Scalar2Offset (node *scalar, int dims, node *shape, info *arg_info)
{
    node *args = NULL;
    node *avis;
    node *let;
    int cnt;

    DBUG_ENTER ("Scalar2Offset");

    avis = MakeScalarAvis (ILIBtmpVar ());
    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

    /*
     * create a list of dims scalar arguments
     */
    for (cnt = 0; cnt < dims; cnt++) {
        args = TBmakeExprs (DUPdoDupNode (scalar), args);
    }

    /*
     * add shape array
     */
    args = TCappendExprs (DUPdoDupTree (ARRAY_AELEMS (shape)), args);

    /*
     * tmpvar = _idxs2offset( args)
     */
    let = TBmakeLet (TBmakeIds (avis, NULL), TBmakePrf (F_idxs2offset, args));

    INFO_PREASSIGNS (arg_info) = TBmakeAssign (let, INFO_PREASSIGNS (arg_info));

    AVIS_SSAASSIGN (avis) = INFO_PREASSIGNS (arg_info);

    DBUG_RETURN (avis);
}

/** <!-- ****************************************************************** -->
 * @fn node *FindAvisForShapeExpression( node *shape, info *info)
 *
 * @brief For a given shape expression of a F_vect2offset application, this
 *        function returns the associated avis of the array the offset
 *        indexes into.
 *        In order for this to work, the shape has to be an array of ids
 *        which are defined by applications of F_idx_shape_sel.
 *
 * @param shape N_array node of the shape argument of F_vect2offset
 * @param info  info structure
 *
 * @return Avis node of the corresponding array
 ******************************************************************************/
node *
FindAvisForShapeExpression (node *shapeexpr, info *info)
{
    node *result;
    node *assign;
    node *rhs;
    shape *shp;

    DBUG_ENTER ("FindAvisForShapeExpression");

    DBUG_ASSERT ((NODE_TYPE (shapeexpr) == N_array),
                 "Found a vect2offset whose first argument is not an "
                 "array constructor!");

    if (COisConstant (shapeexpr)) {
        /* AKS */

        shp = SHarray2Shape (shapeexpr);
        result = SCIfindShapeCliqueForShape (shp, INFO_FUNDEF (info));

        DBUG_ASSERT ((result != NULL),
                     "Unable to find a shape clique for an AKS shape expression.");

    } else {
        /* AKD */

        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (shapeexpr))) == N_id),
                     "FindAvisForShapeExpression cannot be called on constant "
                     "shapes.");

        DBUG_ASSERT ((AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (ARRAY_AELEMS (shapeexpr))))
                      != NULL),
                     "Found a shape argument to vect2offset with SSAASSIGN being "
                     "NULL.");

        assign = AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (ARRAY_AELEMS (shapeexpr))));

        rhs = LET_EXPR (ASSIGN_INSTR (assign));

        DBUG_ASSERT ((NODE_TYPE (rhs) == N_prf),
                     "Found a vect2offset whose shape is not defined using "
                     "a prf!");

        DBUG_ASSERT ((PRF_PRF (rhs) == F_idx_shape_sel),
                     "Found a vect2offset whose shape is not defined using "
                     "shape_sel!");

        result = ID_AVIS (PRF_ARG2 (rhs));
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
    node *clique;

    DBUG_ENTER ("ReplaceByWithOffset");

    DBUG_ASSERT ((INFO_IVINFO (arg_info) != NULL),
                 "found an id which was identified as a withid (no SSAASSIGN "
                 "and not N_arg) although not inside a withloop.");

    clique = FindAvisForShapeExpression (PRF_ARG1 (arg_node), arg_info);

    offset = FindIVOffset (INFO_IVINFO (arg_info), ID_AVIS (PRF_ARG2 (arg_node)), clique);

    if ((offset != NULL) && (global.iveo & IVEO_wlidx)) {
        DBUG_PRINT ("IVEO", ("replacing vect2offset by wlidx %s",
                             AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node)))));

        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TBmakeId (offset);
    } else if (global.iveo & IVEO_idxs) {
        /*
         * this offset is a non-wl-var offset but a locally
         * generated one, so we store it within the ivinfo
         */
        INFO_IVINFO (arg_info) = PushLocalOffset (INFO_IVINFO (arg_info),
                                                  IDS_AVIS (INFO_LHS (arg_info)), clique);

        DBUG_PRINT ("IVEO", ("replacing vect2offset by wl-idxs2offset"));

        scalars = FindIVScalars (INFO_IVINFO (arg_info), ID_AVIS (PRF_ARG2 (arg_node)));

        if (scalars != NULL) {
            offset
              = TBmakePrf (F_idxs2offset, TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                                                       TCids2Exprs (scalars)));
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = offset;
        }
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

    DBUG_PRINT ("IVEO", ("replacing by idxs2offset"));

    idxs = ARRAY_AELEMS (ASSIGN_RHS (ivassign));

    result = TBmakePrf (F_idxs2offset, TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                                                    DUPdoDupTree (idxs)));

    arg_node = FREEdoFreeNode (arg_node);

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *OptimizeComputation( node *arg_node, info *arg_info)
 *
 * @brief Tries to lift primitive functions from the index vector level
 *        to the offset level. As an example, the following code
 *
 *         <pre>
 *           ivo  = _vect2offset_( [1,2,3], iv);
 *           ivo2 = _vect2offset_( [1,2,3], iv + 1);
 *         </pre>
 *
 *        is replaced by
 *
 *         <pre>
 *           ivo  = _vect2offset_( [1,2,3], iv);
 *           tmp  = _idxs2offset_( [1,2,3], 1, 1, 1);
 *           ivo2 - ivo + tmp;
 *         </pre>
 *
 * @param arg_node _vect2offset_ prf
 * @param arg_info info structure
 *
 * @return transformed _vect2offset_ prf
 ******************************************************************************/
static node *
OptimizeComputation (node *arg_node, info *arg_info)
{
    node *iv;
    node *prf;
    node *arg1;
    node *arg2;
    node *clique;

    DBUG_ENTER ("OptimizeComputation");

    iv = PRF_ARG2 (arg_node);
    prf = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (iv)));
    clique = FindAvisForShapeExpression (PRF_ARG1 (arg_node), arg_info);

    if (AVIS_NEEDCOUNT (IDS_AVIS (INFO_LHS (arg_info))) == 0) {
        /*
         * this vector is only used as index vector!
         */
        switch (PRF_PRF (prf)) {
        case F_add_AxA:
        case F_sub_AxA:
            /*
             * look up the matching offsets and replace computation
             */
            arg1 = GetAvis4Clique (ID_AVIS (PRF_ARG1 (prf)), clique, arg_info);
            arg2 = GetAvis4Clique (ID_AVIS (PRF_ARG2 (prf)), clique, arg_info);

            if ((arg1 != NULL) && (arg2 != NULL)) {
                DBUG_PRINT ("IVEO", ("computing prf on offsets instead of idx-vects"));

                arg_node = FREEdoFreeNode (arg_node);
                arg_node
                  = TCmakePrf2 ((PRF_PRF (prf) == F_add_AxA) ? F_add_SxS : F_sub_SxS,
                                TBmakeId (arg1), TBmakeId (arg2));
            }
            break;

        case F_add_SxA:
        case F_sub_SxA:
            if (TUdimKnown (ID_NTYPE (PRF_ARG2 (prf)))) {
                arg2 = GetAvis4Clique (ID_AVIS (PRF_ARG2 (prf)), clique, arg_info);

                if (arg2 != NULL) {
                    DBUG_PRINT ("IVEO",
                                ("computing prf on offsets instead of idx-vects"));

                    arg1 = Scalar2Offset (PRF_ARG1 (prf),
                                          TYgetDim (ID_NTYPE (PRF_ARG2 (prf))),
                                          PRF_ARG1 (arg_node), arg_info);

                    arg_node = FREEdoFreeNode (arg_node);
                    arg_node
                      = TCmakePrf2 ((PRF_PRF (prf) == F_add_SxA) ? F_add_SxA : F_sub_SxA,
                                    TBmakeId (arg1), TBmakeId (arg2));
                }
            }
            break;

        case F_add_AxS:
        case F_sub_AxS:
            if (TUdimKnown (ID_NTYPE (PRF_ARG1 (prf)))) {
                arg1 = GetAvis4Clique (ID_AVIS (PRF_ARG1 (prf)), clique, arg_info);

                if (arg1 != NULL) {
                    DBUG_PRINT ("IVEO",
                                ("computing prf on offsets instead of idx-vects"));

                    arg2 = Scalar2Offset (PRF_ARG2 (prf),
                                          TYgetDim (ID_NTYPE (PRF_ARG1 (prf))),
                                          PRF_ARG1 (arg_node), arg_info);

                    arg_node = FREEdoFreeNode (arg_node);
                    arg_node
                      = TCmakePrf2 ((PRF_PRF (prf) == F_add_AxS) ? F_add_AxS : F_sub_AxS,
                                    TBmakeId (arg1), TBmakeId (arg2));
                }
            }
            break;

        case F_mul_SxA:
            arg2 = GetAvis4Clique (ID_AVIS (PRF_ARG2 (prf)), clique, arg_info);

            if (arg2 != NULL) {
                DBUG_PRINT ("IVEO", ("computing prf on offsets instead of idx-vects"));

                arg1 = DUPdoDupNode (PRF_ARG1 (prf));

                arg_node = FREEdoFreeNode (arg_node);
                arg_node = TCmakePrf2 (F_mul_SxS, arg1, TBmakeId (arg2));
            }
            break;

        case F_mul_AxS:
            arg1 = GetAvis4Clique (ID_AVIS (PRF_ARG1 (prf)), clique, arg_info);

            if (arg1 != NULL) {
                DBUG_PRINT ("IVEO", ("computing prf on offsets instead of idx-vects"));

                arg2 = DUPdoDupNode (PRF_ARG2 (prf));

                arg_node = FREEdoFreeNode (arg_node);
                arg_node = TCmakePrf2 (F_mul_SxS, TBmakeId (arg1), arg2);
            }
            break;

        default:
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

/**
 * traversal functions
 */
node *
IVEOlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEOlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    DBUG_PRINT ("IVEO", ("Looking at %s...", IDS_NAME (LET_IDS (arg_node))));

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
IVEOwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEOwith");

    INFO_IVINFO (arg_info) = PushIV (INFO_IVINFO (arg_info), WITH_WITHID (arg_node),
                                     INFO_LHS (arg_info), WITH_WITHOP (arg_node));

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    INFO_IVINFO (arg_info) = PopIV (INFO_IVINFO (arg_info));

    DBUG_RETURN (arg_node);
}

node *
IVEOcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEOcode");

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
IVEOprf (node *arg_node, info *arg_info)
{
    node *ivarg;
    node *ivassign;

    DBUG_ENTER ("IVEOprf");

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
                DBUG_PRINT ("IVEO", ("Trying to scalarise vect2offset for iv %s...",
                                     AVIS_NAME (ID_AVIS (ivarg))));

                arg_node = ReplaceByIdx2Offset (arg_node, arg_info);
            } else if ((NODE_TYPE (ASSIGN_RHS (ivassign)) == N_prf)
                       && (global.iveo & IVEO_copt)) {
                /*
                 * this index vector is defined as a computation
                 * on (maybe) other index vectors or constants
                 */
                DBUG_PRINT (
                  "IVEO",
                  ("Trying to lift vect2offset for iv %s to computation on offsets...",
                   AVIS_NAME (ID_AVIS (ivarg))));

                arg_node = OptimizeComputation (arg_node, arg_info);
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
                DBUG_PRINT ("IVEO",
                            ("Trying to replace vect2offset for iv %s by wlidx...",
                             AVIS_NAME (ID_AVIS (ivarg))));

                arg_node = ReplaceByWithOffset (arg_node, arg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

node *
IVEOfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEOblock");

    INFO_VARDECS (arg_info) = NULL;
    INFO_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (INFO_VARDECS (arg_info) != NULL) {
        FUNDEF_VARDEC (arg_node)
          = TCappendVardec (INFO_VARDECS (arg_info), FUNDEF_VARDEC (arg_node));
        INFO_VARDECS (arg_info) = NULL;
    }

    INFO_FUNDEF (arg_info) = NULL;

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IVEOassign (node *arg_node, info *arg_info)
{
    node *preassigns;

    DBUG_ENTER ("IVEOassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    preassigns = INFO_PREASSIGNS (arg_info);
    INFO_PREASSIGNS (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (preassigns != NULL) {
        arg_node = TCappendAssign (preassigns, arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Performs Index Vector Elimination Optimisations on the given
 *        syntax tree.
 *
 * @param syntax_tree the syntax tree to optimize
 *
 * @return optimized syntax tree
 ******************************************************************************/
node *
IVEOdoIndexVectorEliminationOptimisation (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IVEIdoIndexVectorEliminationOptimisation");

    TRAVpush (TR_iveo);

    info = MakeInfo ();

    syntax_tree = TRAVdo (syntax_tree, info);

    info = FreeInfo (info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}
