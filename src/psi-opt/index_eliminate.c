/*
 *
 * $Id$
 *
 */

#include "index_eliminate.h"

#include <string.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "shape.h"
#include "new_types.h"
#include "dbug.h"
#include "traverse.h"
#include "DupTree.h"
#include "type_utils.h"
#include "free.h"
#include "shape_cliques.h"
#include "index_infer.h"
#include "wrci.h"
#include "makedimexpr.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 *
 */

/**
 *
 * @defgroup ive IVE
 * @ingroup opt
 *
 * @brief "index vector elimination" (IVE)
 *	does eliminate index vectors, at times.
 *	Specifically, it adds code that converts index
 *	vectors to array offsets. If all references to an
 *	index vector are sel or modarray operations, then
 *	dead code elimination in a later phase will, in fact,
 *	eliminate the index vector. If the index vector is
 *	referenced by some other operation, it will not be
 *	eliminated, but performance may still be improved,
 *	due to improved sel/modarray operation.
 *
 *	As of 2006-06-21, it replaces ALL sel and modarray operations
 *	for AKD/AUD arrays (B) operations as follows:
 *
 *	   x = sel(iv,B)            ->      iv_B = vect2offset(iv,_shape_(B));
 *	                                    x = _idx_sel(iv_B,B);
 *
 *
 *     X = modarray(B, iv, v);  ->      iv_B = vect2offset(iv,iv,_shape_(B));
 *                                      X = idx_modarray(iv_B, B, v);
 *
 *     NB. Note different argument order in modarray vs idx_modarray!
 *
 *     In the case where an index vector is used by more than one
 *     indexing operation on arrays that are in the same shape clique
 *     (and hence known to have the same shape), the offset (iv_B)
 *     will eventually be shared by them, though CSE and DCR:
 *
 *     A = reverse(B);                  NB. A and B are in same shape clique
 *     ...
 *	   x = sel(iv,B)            ->      iv_B = vect2offset(iv,_shape_(B));
 *	   ...                              x = _idx_sel(iv_B,B);
 *	   q = sel(iv,A)            ->      iv_A = vect2offset(iv,_shape_(B));
 *	                                    x = _idx_sel(iv_A,A);
 *
 *     NB. The second argument of vect2offset will use the name
 *     of the earliest array encountered in that shape clique. Note in the
 *     above that the computation of q uses the shape of A, not B.
 *
 *     NB. It is possible that code motion may be able to lift the
 *     calculation of iv_B to an earlier point in the computation.
 *     I should measure this...
 *
 * The index_optimize phase (q.v.) that follows this phase may make
 * further improvements to some of this code
 *
 * IVE treatment of AKS arrays is more aggressive than for AKD/AUD arrays,
 * in that it:
 *
 *      1. Places the vect2offset operation directly after the definition
 *         of the index vector.
 *      2. Uses the (statically known) shape of the array in the vect2offset
 *         operation. This approach can not be used for AKD/AUD arrays,
 *         because the array itself may not exist at the location of the
 *         definition of the index vector.
 *      3. If the offset created by the vect2offset is used by more than
 *         one AKS indexing operation, it will be shared by those operations.
 *
 *
 * <pre>
 * Example:
 *
 *            a = reshape([4,4], [1,2,...,16]);
 *            b = a + 1;
 *            iv = [2,3];
 *            ...
 *            z = a[iv];
 *            q = b[iv];
 *
 * is transformed into:
 *
 *            a = reshape([4,4], [1,2,...,16]);
 *            b = a + 1;
 *            iv = [2,3];
 *            __iv_4_4 = 11;
 *            ...
 *            z = idx_sel(a, __iv_4_4);
 *            q = idx_sel(b, __iv_4_4);
 * The iv =... will be deleted by DCR, if possible.
 *
 * </pre>
 *
 * @{
 */

/**
 * @{
 */

/**
 *
 * @file IndexVectorElimination.c
 *
 *  This file contains the implementation of IVE (index vector elimination).
 *
 *
 * <pre>
 * 1. Purported benefits of this optimization:
 *  a. Elimination of the need to allocate and initialize
 *     the index vector itself, if all references to the
 *     index vector are index operations.
 *
 *  b. Replacement of the one or more computations of the array
 *     offset [the inner product of index vector and array bounds]
 *     at each index site by a one-time computation of that
 *     offset, using vect2offset.
 *
 *  c. If the array shape and index vector values are
 *     statically known, the offset will be computed as a scalar
 *     by later optimization phases.
 *
 * 2. Potholes to watch for:
 *  a. With-loops do not have an index vector per se,
 *     so they are treated differently from normal index vectors.
 *  b. With-loops can have multiple bodies for different array
 *     sections; each body requires its own vect2offset calls.
 *
 * 3. Implementation strategy:
 *	Traverse all arguments and each vardec in each block.
 *
 *      For each of these, find its avis node, and generate
 *      vect2offset calls and assigns to scalar offset variables
 *	for each indexed array type defined in the avis.
 *	These calls are later appended to the index vector
 *	assigns.
 *
 *	Replace each sel or modarray reference to an index vector
 *	as above.
 * </pre>
 */

/**
 * INFO structure
 */
struct INFO {
    node *postassigns;
    node *preassigns;
    node *precondassigns;
    node *vardecs;
    node *lhs;
    node *withid;
    node *fundef;
    node *lastap;
};

/**
 * INFO macros
 */
#define INFO_POSTASSIGNS(n) ((n)->postassigns)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_PRECONDASSIGNS(n) ((n)->precondassigns)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_WITHID(n) ((n)->withid)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LASTAP(n) ((n)->lastap)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_POSTASSIGNS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_PRECONDASSIGNS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_WITHID (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_LASTAP (result) = NULL;

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
 * counts the number of index vectors removed during optimization.
 */
static int ive_expr;

/**
 * helper functions
 */

/** <!-- ****************************************************************** -->
 * @brief Find identifier to use as offset in idx_sel operation.
 *        This is done by searching iv's avis for an array entry in the
 *        same shape clique as bavis, and returning the corresponding
 *        AVIS_IDXIDS. This N_avis was created when the vect2offset
 *        was emitted. E.g., if it generated:
 *          iv_B = vect2offset( iv, _shape_(B));
 *       the result N_avis is that of iv_B.
 *
 * @param ivavis -  N_avis node of the selection index, iv
 * @param bavis  -  N_avis of array, B, potentially being indexed by the ivavis entry
 *                  in B[iv] or B[iv] = v
 *
 * @return avis entry for iv_B that matches bavis, or NULL, if no match is found
 ******************************************************************************/
static node *
GetIvScalarOffsetAvis (node *iavis, node *bavis)
{
    node *result = NULL;
    node *ids;
    node *shpexprs;
    node *shpid;

    DBUG_ENTER ("GetIvScalarOffsetAvis");
    DBUG_ASSERT (NODE_TYPE (iavis) == N_avis, "Expected N_avis node as iavis");
    DBUG_ASSERT (NODE_TYPE (bavis) == N_avis, "Expected N_avis node as bavis");

    /* shpexprs is exprs list of B N_avis nodes hanging off iavis */
    shpexprs = AVIS_IDXSHAPES (iavis);
    DBUG_ASSERT (NODE_TYPE (shpexprs) == N_exprs,
                 "Expected N_exprs node as shpexprs type node");
    ids = AVIS_IDXIDS (iavis);

    while (shpexprs != NULL) {
        DBUG_ASSERT ((ids != NULL),
                     "number of IDXSHAPES does not match number of IDXIDS!");

        /* Search for entry in same shape clique as B */
        shpid = EXPRS_EXPR (shpexprs);
        DBUG_ASSERT ((N_id == NODE_TYPE (shpid)),
                     "Shape clique shape chain entry not N_id");
        if (ShapeVarsMatch (bavis, ID_AVIS (shpid))) {
            result = IDS_AVIS (ids);
            break;
        }

        shpexprs = EXPRS_NEXT (shpexprs);
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief node * FindArrayWithSameShape( bid, ivavis);
 * We are generating code for B[IV], where bid and ivavis represent those
 * arrays.
 * We want to find an array B' of the shape as B, for which we
 * have already issued a vect2offset. If we find one, we will issue
 * vect2offset( iv, _shape_( B')) instead of vect2offset( iv, _shape_( B)).
 * This will be determined by the presence of a isvect2offsetissued
 * mark in the B' avis.
 * If we don't find one, we replace B' by B in the IDX_IDS entry,
 * and mark that entry for future use.
 *
 * @param ivavis - N_avis for vector being used as an array index
 * @param bid -    N_id for array being indexed
 *
 * @return - avis of array to use for the _shape_  operation.
 ******************************************************************************/
static node *
FindArrayWithSameShape (node *bid, node *ivavis)
{
    node *exprs;
    node *result;
    node *bavis;
    node *typid;

    DBUG_ENTER ("FindAvisWithSameShape");
    bavis = ID_AVIS (bid);
    exprs = FindMatchingVarShape (bavis, ivavis);
    DBUG_ASSERT (NULL != exprs, "IVE X[IV] did not find matching X IDX_SHAPE entry");
    typid = EXPRS_EXPR (exprs);
    result = ID_AVIS (typid);
    if (FALSE == ID_ISVECT2OFFSETISSUED (typid)) {
        /* No vect2offset issued for this array shape yet.
         * Mark the chain's IDXSHAPES entry as having one issued,
         * and replace the N_avis pointer in IDXSHAPES with this one.
         */
        ID_ISVECT2OFFSETISSUED (typid) = TRUE;
        ID_AVIS (typid) = bavis;
        result = bavis;
    }
    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Emit code to scalarize shape vector for typical array in
 *        bavis shape clique
 * @param info
 * @param ivavis - index vector array name to use for generating
 *                 scalar shape element names
 * @param rank - size of shape vector to scalarize
 * @param bavis - array to use for specifying shape clique
 *
 * @return exprs node containing shape_sel expressions
 ******************************************************************************/
node *
ScalarizeShape (info *info, node *ivavis, int rank, node *bid)
{ /* Emit rank-sized vector of _shape_sel(B) invocations for
   * best array in shape clique of ivavis
   */

    node *cliqueb;
    node *exprs;
    int axis;
    node *shpelavis;
    node *shpel;

    DBUG_ENTER ("ScalarizeShape");

    /*
     * we try to always use the same array here to allow
     * CSE to detect that all vect2offsets of one clique
     * are common. In case that this feature is turned off,
     * we just use the current avis.
     */
    if (global.iveo & IVEO_share) {
        cliqueb = FindArrayWithSameShape (bid, ivavis);
    } else {
        cliqueb = ID_AVIS (bid);
    }

    exprs = NULL;
    for (axis = rank - 1; axis >= 0; axis--) {
        shpelavis = MakeScalarAvis (ILIBtmpVarName (AVIS_NAME (ivavis)));
        AVIS_SHAPECLIQUEID (shpelavis) = shpelavis;
        INFO_VARDECS (info) = TBmakeVardec (shpelavis, INFO_VARDECS (info));

        shpel = TCmakePrf2 (F_idx_shape_sel, TBmakeNum (axis), TBmakeId (cliqueb));
        INFO_PREASSIGNS (info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (shpelavis, NULL), shpel),
                          INFO_PREASSIGNS (info));

        AVIS_SSAASSIGN (shpelavis) = INFO_PREASSIGNS (info);
        exprs = TBmakeExprs (TBmakeId (shpelavis), exprs);
    }
    DBUG_RETURN (exprs);
}
/** <!-- ****************************************************************** -->
 * @brief Emit Vect2Offset for AKD array indexing operations sel(iv, B)
 *        and modarray(B, iv, val)
 * AKD case: Replace  x = sel( iv, B) by:
 *                        shp0 = _shape_sel( 0, B);
 *                        shp1 = _shape_sel( 1, B);
 *                        ...
 *                        iv_B = vect2offset( [shp0, shp1...], iv);
 *                        x = idx_sel( iv_B, B);
 *
 * @param bid is the N_id for B.
 * @param  ivavis is the avis for iv
 *
 * @return The N_avis for the newly generated iv_B.
 ******************************************************************************/
static node *
EmitAKDVect2Offset (node *bid, node *ivavis, info *info)
{

    node *result;
    node *assign;
    node *offset;
    node *exprs;

    DBUG_ENTER ("EmitAKDVect2Offset");

    result = ivavis;

    if (TUdimKnown (ID_NTYPE (bid))) {
        exprs = ScalarizeShape (info, ivavis, TYgetDim (ID_NTYPE (bid)), bid);

        /* Emit     iv_B = vect2offset( [shp0, shp1, ...], iv);
         * KLUDGE: This is not flattened code: the correct approach,
         * according to the non-existent sac2c design and implementation
         * manual, is to make a temp vector, tv, from the scalars,
         * and generate iv_B = vect2Offset( tv, iv); However, we
         * don't do that...
         * See also KLUDGE in index_optimize.
         */

        offset = TCmakePrf2 (F_vect2offset, TCmakeIntVector (exprs), TBmakeId (ivavis));

        /* Generate temp name for resulting integer scalar array offset iv_B */
        result = MakeScalarAvis (ILIBtmpVarName (AVIS_NAME (ivavis)));
        AVIS_SHAPECLIQUEID (result) = result;

        INFO_VARDECS (info) = TBmakeVardec (result, INFO_VARDECS (info));

        assign = TBmakeAssign (TBmakeLet (TBmakeIds (result, NULL), offset), NULL);

        INFO_PREASSIGNS (info) = TCappendAssign (INFO_PREASSIGNS (info), assign);

        AVIS_SSAASSIGN (result) = assign;
    }

    DBUG_RETURN (result);
}
/** <!-- ****************************************************************** -->
 * @brief Emit Vect2offset for AKS array indexing operations sel(iv, B)
 *        and modarray(B, iv, val)
 *        This emits:  iv_B = vect2offset( [ <shp> ], iv);
 *
 * @param bavis is the avis for B.
 * @param  ivavis is the avis for iv
 *
 * @return Result is N_avis for iv_B
 ******************************************************************************/
static node *
EmitAKSVect2offset (node *bavis, node *ivavis, info *info)
{

    node *result;
    node *assign;
    node *offset;
    ntype *btype;

    DBUG_ENTER ("EmitAKSVect2offset");
    DBUG_ASSERT (N_avis == NODE_TYPE (ivavis), "ivavis node not of type N_avis");

    btype = AVIS_TYPE (bavis);

    offset
      = TCmakePrf2 (F_vect2offset, SHshape2Array (TYgetShape (btype)), /* e.g., [2,3,4] */
                    TBmakeId (ivavis)                                  /* iv */
      );

    /* Generate temp name for resulting integer scalar array offset */
    result = MakeScalarAvis (ILIBtmpVarName (AVIS_NAME (ivavis)));
    AVIS_SHAPECLIQUEID (result) = result;

    INFO_VARDECS (info) = TBmakeVardec (result, INFO_VARDECS (info));

    assign = TBmakeAssign (TBmakeLet (TBmakeIds (result, NULL), offset), NULL);

    INFO_POSTASSIGNS (info) = TCappendAssign (INFO_POSTASSIGNS (info), assign);

    AVIS_SSAASSIGN (result) = assign;

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Recursively generates all vect2offset assignments for the given
 *        index variable and shapes for B[iv]
 *
 * @param bexprs N_exprs chain of N_type nodes containing all shapes B
 *              for which we need to build offsets
 * @param ivavis N_avis node of the selection index, iv
 * @param info info structure
 *
 * @return N_ids chain containing all created offset ids
 ******************************************************************************/
static node *
EmitAKSVect2Offsets (node *bexprs, node *ivavis, info *info)
{
    node *result = NULL;
    node *idxavis = NULL;
    node *bavis;

    DBUG_ENTER ("EmitAKSVect2Offsets");

    if (bexprs != NULL) {
        DBUG_ASSERT (N_exprs == NODE_TYPE (bexprs), "bexprs node not of type N_exprs");
        DBUG_ASSERT (N_avis == NODE_TYPE (ivavis), "ivavis node not of type N_avis");

        bavis = ID_AVIS (EXPRS_EXPR (bexprs));
        if (TUshapeKnown (AVIS_TYPE (bavis))) {
            idxavis = EmitAKSVect2offset (bavis, ivavis, info);
        }

        result = EmitAKSVect2Offsets (EXPRS_NEXT (bexprs), ivavis, info);
        result = TBmakeIds (idxavis, result);
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Generates new arguments for the given
 *        index variable and shapes.
 *
 * @param types N_exprs chain of N_type nodes containing all shapes
 *              an offset has to be built for.
 * @param avis N_avis node of the selection index
 * @param args stores the newly generated args chain
 * @param info info structure
 *
 * @return N_ids chain containing all created new offset args
 ******************************************************************************/
static node *
IdxTypes2IdxArgs (node *types, node *avis, node **args, info *info)
{
    node *result = NULL;
    node *idxavis;

    DBUG_ENTER ("IdxTypes2IdxArgs");

    if (types != NULL) {
        result = IdxTypes2IdxArgs (EXPRS_NEXT (types), avis, args, info);

        if (TUshapeKnown (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (types))))) {
            idxavis = MakeScalarAvis (ILIBtmpVarName (AVIS_NAME (avis)));
            AVIS_SHAPECLIQUEID (idxavis) = idxavis;

            *args = TBmakeArg (idxavis, *args);
            result = TBmakeIds (idxavis, result);
        }
    }

    DBUG_RETURN (result);
}

static
  /** <!-- ****************************************************************** -->
   * @brief Generates new concrete arguments for the given idxtypes and avis.
   *        The avis must contain matching idxids for all given types!
   *        Furthermore, _vect2offset_ prfs are issued in front
   *        of the current assignment to compute the offsets.
   *
   * @param avisexpr N_exprs chain of type nodes
   * @param ivavis index vector avis
   * @param args N_exprs chain of concrete args
   * @param info info structure
   *
   * @return extended N_exprs arg chain
   ******************************************************************************/
  node *
  IdxTypes2ApArgs (node *shapes, node *ivavis, node *args, info *info)
{
    node *offset;
    node *idxavis;
    ntype *arraytype;

    DBUG_ENTER ("IdxTypes2ApArgs");

    if (shapes != NULL) {
        args = IdxTypes2ApArgs (EXPRS_NEXT (shapes), ivavis, args, info);

        arraytype = AVIS_TYPE (ID_AVIS (EXPRS_EXPR (shapes)));

        if (TUshapeKnown (arraytype)) {
            /* AKS */

            /*
             * generate new offset vardec
             */
            idxavis = MakeScalarAvis (ILIBtmpVarName (AVIS_NAME (ivavis)));
            AVIS_SHAPECLIQUEID (idxavis) = idxavis;
            /*
             * generate offset computation
             */
            offset = TBmakeAssign (TBmakeLet (TBmakeIds (idxavis, NULL),
                                              TCmakePrf2 (F_vect2offset,
                                                          SHshape2Array (
                                                            TYgetShape (arraytype)),
                                                          TBmakeId (ivavis))),
                                   NULL);

            AVIS_SSAASSIGN (idxavis) = offset;
            INFO_VARDECS (info) = TBmakeVardec (idxavis, INFO_VARDECS (info));

            /*
             * we have to be very careful on where to insert the offset
             * computation. If we are within a loop function and these offset
             * computations are triggered by the recursive call, we have to
             * make sure that they are inserted in front of the surrounding
             * if as we otherwise would destroy the special loopfun form
             */
            if (AP_FUNDEF (INFO_LASTAP (info)) == INFO_FUNDEF (info)) {
                /* recursive call inside of a dofun */
                INFO_PRECONDASSIGNS (info)
                  = TCappendAssign (INFO_PRECONDASSIGNS (info), offset);
            } else {
                INFO_PREASSIGNS (info) = TCappendAssign (INFO_PREASSIGNS (info), offset);
            }

            args = TBmakeExprs (TBmakeId (idxavis), args);
        }
    }

    DBUG_RETURN (args);
}

node *
ExtendConcreteArgs (node *concargs, node *formargs, info *info)
{
    DBUG_ENTER ("ExtendConcreteArgs");

    if (concargs != NULL) {
        EXPRS_NEXT (concargs) = IdxTypes2ApArgs (AVIS_IDXSHAPES (ARG_AVIS (formargs)),
                                                 ID_AVIS (EXPRS_EXPR (concargs)),
                                                 EXPRS_NEXT (concargs), info);

        EXPRS_NEXT (concargs)
          = ExtendConcreteArgs (EXPRS_NEXT (concargs), ARG_NEXT (formargs), info);
    }

    DBUG_RETURN (concargs);
}

/** <!-- ****************************************************************** -->
 * @brief  AKD/AUD Attempt to replace y = sel(iv,B) by:
 *          iv_B = vect2offset(iv, _shape_(BClique));
 *          y = idx_sel(iv_B,B)
 *         where BClique is earliest member of B's shape clique that
 *         has had an indexing operation already performed here.
 *         [This will allow CSE to remove some of the vect2offsets.]
 *
 * @param prf prf entry for a sel(iv,B) operation
 * @param lhs - unused in this call
 *
 * @return updated prf node
 ******************************************************************************/
static node *
CheckAndReplaceSelAKD (node *prf, info *info)
{
    node *result;
    node *ivavis;

    DBUG_ENTER ("CheckAndReplaceSelAKD");

    ivavis = EmitAKDVect2Offset (PRF_ARG2 (prf), ID_AVIS (PRF_ARG1 (prf)), info);

    if (ivavis != NULL) {
        result = TCmakePrf2 (F_idx_sel, TBmakeId (ivavis), PRF_ARG2 (prf));

        PRF_ARG2 (prf) = NULL;
        prf = FREEdoFreeTree (prf);

        ive_expr++;
    } else {
        result = prf;
    }

    DBUG_RETURN (result);
}
/** <!-- ****************************************************************** -->
 * @brief  AKS Attempt to replace y = sel( iv, B) by  y = idx_sel( iv_B, B)
 *
 * @param prf - prf entry for a sel(iv,B) operation
 * @param lhs - unused in this call
 *
 * @return updated prf node
 ******************************************************************************/
static node *
CheckAndReplaceSelAKS (node *prf, info *info)
{
    node *ivscalaravis;
    node *result = NULL;

    DBUG_ENTER ("CheckAndReplaceSelAKS");

    ivscalaravis
      = GetIvScalarOffsetAvis (ID_AVIS (PRF_ARG1 (prf)), ID_AVIS (PRF_ARG2 (prf)));

    if (ivscalaravis != NULL) {
        result
          = TCmakePrf2 (F_idx_sel, TBmakeId (ivscalaravis), /* iv scalar offset into B */
                        PRF_ARG2 (prf));                    /* B */

        PRF_ARG2 (prf) = NULL;
        prf = FREEdoFreeTree (prf);

        ive_expr++;
    } else {
        result = prf;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief  Attempt to replace:
 *      (AKS):      y = sel(iv,B) by
 *                  y = idx_sel(iv_B,B)
 *
 *      (AKD/AUD):  y = sel(iv,B) by
 *                  iv_B = vect2offset(iv, _shape_(CliqueB));
 *                  y = idx_sel(iv_B,B)
 *
 * @param prf prf entry for a sel(iv,B) operation
 * @param lhs - unused in this call
 *
 * @return updated prf node
 ******************************************************************************/
static node *
CheckAndReplaceSel (node *prf, info *info)
{
    node *result;

    DBUG_ENTER ("CheckAndReplaceSel");

    /* Performaace-analysis-only Cases:
     *  AKS array: If IVE_akd, treat as AKD
     *             Else treat as AKS
     *  AKD array: If IVE_aks, treat as AKD
     *             Else do no optimization
     *  AUD array: No optimization
     */
    if ((IVE_akd != global.ive) && TUshapeKnown (ID_NTYPE (PRF_ARG2 (prf)))) {
        result = CheckAndReplaceSelAKS (prf, info);
    } else if ((IVE_aks != global.ive) && TUdimKnown (ID_NTYPE (PRF_ARG2 (prf)))) {
        result = CheckAndReplaceSelAKD (prf, info);
    } else {
        result = prf;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief  Replace y = modarray(B,iv,val) with y = idx_modarray(B,iv_B,val)
 *         where B is AKS
 *
 * @param prf prf entry for a modarray(B,iv) operation
 * @param lhs node of "y" in above. Required only because
 *            of _idx_modarray ICM restriction noted below.
 *
 * @return updated prf node
 ******************************************************************************/
static node *
CheckAndReplaceAKSModarray (node *prf, info *info)
{
    node *result;
    node *ivoffsetavis;
    node *bid;

    DBUG_ENTER ("CheckAndReplaceAKSModarray");

    bid = PRF_ARG1 (prf);
    result = prf;
    ivoffsetavis
      = GetIvScalarOffsetAvis (ID_AVIS (PRF_ARG2 (prf)), ID_AVIS (PRF_ARG1 (prf)));

    if ((ivoffsetavis != NULL)) {
        result
          = TCmakePrf3 (F_idx_modarray, bid, TBmakeId (ivoffsetavis), PRF_ARG3 (prf));

        PRF_ARG1 (prf) = NULL;
        PRF_ARG3 (prf) = NULL;
        prf = FREEdoFreeTree (prf);

        ive_expr++;
    } else {
        result = prf;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief  Replace y = modarray(B,iv,val) with y = idx_modarray(B,iv_B,val)
 *         where B is AKD
 * @param prf prf entry for a modarray(B,iv) operation
 * @param lhs node of "y" in above. Required only because
 *            of _idx_modarray ICM restriction noted below.
 *
 * @return updated prf node
 ******************************************************************************/
static node *
CheckAndReplaceAKDModarray (node *prf, info *info)
{
    node *result;
    node *ivavis;
    node *bid;

    DBUG_ENTER ("CheckAndReplaceAKDModarray");

    bid = PRF_ARG1 (prf);
    result = prf;
    ivavis = EmitAKDVect2Offset (bid, ID_AVIS (PRF_ARG2 (prf)), info);

    if ((ivavis != NULL)) {
        result = TCmakePrf3 (F_idx_modarray, bid, TBmakeId (ivavis), PRF_ARG3 (prf));

        PRF_ARG1 (prf) = NULL;
        PRF_ARG3 (prf) = NULL;
        prf = FREEdoFreeTree (prf);

        ive_expr++;
    } else {
        result = prf;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief  Replace y = modarray(B,iv,val) with y = idx_modarray(B,iv_B,val)
 *
 * @param prf prf entry for a modarray(B,iv) operation
 * @param lhs node of "y" in above. Required only because
 *            of _idx_modarray ICM restriction noted below.
 *
 * @return updated prf node
 ******************************************************************************/
static node *
CheckAndReplaceModarray (node *prf, info *info)
{
    node *result;

    DBUG_ENTER ("CheckAndReplaceModarray");

    /* Performaace-analysis-only Cases:
     *  AKS array: If IVE_akd, treat as AKD
     *             Else treat as AKS
     *  AKD array: If IVE_aks, treat as AKD
     *             Else do no optimization
     *  AUD array: No optimization
     */
    if ((IVE_akd != global.ive) && TUshapeKnown (ID_NTYPE (PRF_ARG1 (prf)))) {
        result = CheckAndReplaceAKSModarray (prf, info);
    } else if ((IVE_aks != global.ive) && TUdimKnown (ID_NTYPE (PRF_ARG1 (prf)))) {
        result = CheckAndReplaceAKDModarray (prf, info);
    } else {
        result = prf;
    }

    DBUG_RETURN (result);
}

/**
 * traversal functions
 */

/** <!-- ****************************************************************** -->
 * @brief Starts index elimination in a given fundef node. On top-level,
 *        only non-lac functions are processed, as lac-funs are handled
 *        when reaching their application.
 *
 * @param arg_node N_fundef node
 * @param arg_info info structure
 *
 * @return unchanged N_fundef node
 ******************************************************************************/
node *
IVEfundef (node *arg_node, info *arg_info)
{
    node *lastfun;
    node *oldvardecs;

    DBUG_ENTER ("IVEfundef");

    if ((INFO_FUNDEF (arg_info) != NULL) || (!FUNDEF_ISLACFUN (arg_node))) {
        DBUG_PRINT ("IVE", ("processing fundef %s", CTIitemName (arg_node)));

        /*
         * as we enter a new context here, we have to stack
         * the current fundef and the vardecs we have created
         * so far. This is to ensure that the vardecs are
         * appended to the correct fundef!
         */
        lastfun = INFO_FUNDEF (arg_info);
        INFO_FUNDEF (arg_info) = arg_node;
        oldvardecs = INFO_VARDECS (arg_info);
        INFO_VARDECS (arg_info) = NULL;

        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }

        if (INFO_VARDECS (arg_info) != NULL) {
            FUNDEF_VARDEC (arg_node)
              = TCappendVardec (FUNDEF_VARDEC (arg_node), INFO_VARDECS (arg_info));
            INFO_VARDECS (arg_info) = NULL;
        }

        /*
         * unstack fundef and vardecs
         */

        INFO_FUNDEF (arg_info) = lastfun;
        INFO_VARDECS (arg_info) = oldvardecs;
    }

    if (INFO_FUNDEF (arg_info) == NULL) {
        /*
         * we are on top level, so continue with next fundef
         */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
IVEarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEarg");

    if (INFO_LASTAP (arg_info) == NULL) {
        /*
         * top-level function
         */
        AVIS_IDXIDS (ARG_AVIS (arg_node))
          = EmitAKSVect2Offsets (AVIS_IDXSHAPES (ARG_AVIS (arg_node)),
                                 ARG_AVIS (arg_node), arg_info);
    } else {
        /*
         * lac-function:
         *
         * here, we can try to reuse the offsets that have been
         * generated for the concrete args. to do so, we have to
         * extend the function signature.
         */
        AVIS_IDXIDS (ARG_AVIS (arg_node))
          = IdxTypes2IdxArgs (AVIS_IDXSHAPES (ARG_AVIS (arg_node)), ARG_AVIS (arg_node),
                              &ARG_NEXT (arg_node), arg_info);
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Traverses into the assignments within the block to trigger
 *        the generation of idx2offset/vect2offset assignments. Prior to
 *        doing so, any leftover POSTASSIGNS are inserted at the beginning
 *        of the block. This is necessary, as the assignments generated
 *        for a WITHID ids need to be inserted at the beginning of the
 *        block of the corresponding CODE.
 *
 * @param arg_node N_block node
 * @param arg_info info structure
 *
 * @return unchanged N_block node
 ******************************************************************************/
node *
IVEblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEblock");

    if (INFO_POSTASSIGNS (arg_info) != NULL) {
        if (NODE_TYPE (BLOCK_INSTR (arg_node)) == N_empty) {
            BLOCK_INSTR (arg_node) = FREEdoFreeNode (BLOCK_INSTR (arg_node));
            BLOCK_INSTR (arg_node) = INFO_POSTASSIGNS (arg_info);
            INFO_POSTASSIGNS (arg_info) = NULL;
        } else {
            BLOCK_INSTR (arg_node)
              = TCappendAssign (INFO_POSTASSIGNS (arg_info), BLOCK_INSTR (arg_node));
            INFO_POSTASSIGNS (arg_info) = NULL;
        }
    }

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Generates idx2offset/vect2offset assignments and
 *        annotates the corresponding ids at the avis node.
 *
 * @param arg_node N_ids node
 * @param arg_info info structure
 *
 * @return unchanged N_ids node
 ******************************************************************************/
node *
IVEids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEids");

    AVIS_IDXIDS (IDS_AVIS (arg_node))
      = EmitAKSVect2Offsets (AVIS_IDXSHAPES (IDS_AVIS (arg_node)), IDS_AVIS (arg_node),
                             arg_info);

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Replaces F_sel/F_modarray by F_idx_sel/F_idx_modarray whereever
 *        possible.
 *
 * @param arg_node N_prf node
 * @param arg_info info structure
 *
 * @return possibly replaced N_prf node
 ******************************************************************************/
node *
IVEprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEprf");

    switch (PRF_PRF (arg_node)) {
    case F_sel:
        arg_node = CheckAndReplaceSel (arg_node, arg_info);
        break;
    case F_modarray:
        arg_node = CheckAndReplaceModarray (arg_node, arg_info);
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Directs the traversal into the assignment instruction and inserts
 *        the INFO_POSTASSIGNS chain after the current node prior to
 *        traversing further down.
 *
 * @param arg_node N_assign node
 * @param arg_info info structure
 *
 * @return unchanged N_assign node
 ******************************************************************************/
node *
IVEassign (node *arg_node, info *arg_info)
{
    node *postassigns;
    node *new_node;

    DBUG_ENTER ("IVEassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    new_node = arg_node;

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        new_node = TCappendAssign (INFO_PREASSIGNS (arg_info), new_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_cond)
        && (INFO_PRECONDASSIGNS (arg_info) != NULL)) {
        new_node = TCappendAssign (INFO_PRECONDASSIGNS (arg_info), new_node);
        INFO_PRECONDASSIGNS (arg_info) = NULL;
    }

    postassigns = INFO_POSTASSIGNS (arg_info);
    INFO_POSTASSIGNS (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (postassigns != NULL) {
        ASSIGN_NEXT (arg_node) = TCappendAssign (postassigns, ASSIGN_NEXT (arg_node));
    }

    DBUG_RETURN (new_node);
}

/** <!-- ****************************************************************** -->
 * @brief Stores the LET_IDS in INFO_LHS for further usage.
 *
 * @param arg_node N_let node
 * @param arg_info info structure
 *
 * @return unchanged N_let node
 ******************************************************************************/
node *
IVElet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVElet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IVEwith (node *arg_node, info *arg_info)
{
    node *oldwithid;

    DBUG_ENTER ("IVEwith");

    oldwithid = INFO_WITHID (arg_info);
    INFO_WITHID (arg_info) = WITH_WITHID (arg_node);

    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    INFO_WITHID (arg_info) = oldwithid;

    DBUG_RETURN (arg_node);
}

node *
IVEcode (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("IVEcode");

    avis = IDS_AVIS (WITHID_VEC (INFO_WITHID (arg_info)));

    AVIS_IDXIDS (avis) = EmitAKSVect2Offsets (AVIS_IDXSHAPES (avis), avis, arg_info);

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    if (AVIS_IDXIDS (avis) != NULL) {
        AVIS_IDXIDS (avis) = FREEdoFreeTree (AVIS_IDXIDS (avis));
    }

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IVEap (node *arg_node, info *arg_info)
{
    node *oldap;

    DBUG_ENTER ("IVEap");

    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))) {
        oldap = INFO_LASTAP (arg_info);
        INFO_LASTAP (arg_info) = arg_node;

        if (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info)) {
            /* external application */
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        }

        /*
         * extend the arguments of the given application
         */

        AP_ARGS (arg_node)
          = ExtendConcreteArgs (AP_ARGS (arg_node), FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                                arg_info);

        INFO_LASTAP (arg_info) = oldap;
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief function triggering the index vector elimination on the
 *        syntax tree. the traversal relies on the information
 *        infered by index vector elimination inference.
 *
 * @param syntax_tree N_module node
 *
 * @return transformed syntax tree
 ******************************************************************************/
node *
IVEdoIndexVectorElimination (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IndexVectorElimination");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "IVE is intended to run on the entire tree");

    info = MakeInfo ();

    TRAVpush (TR_ive);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/*@}*/ /* defgroup ive */
