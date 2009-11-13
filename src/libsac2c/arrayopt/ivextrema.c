/*
 * $Id: ivextrema.c 15815 2008-10-24 18:04:47Z rbe $
 */

/** <!--********************************************************************-->
 *
 *  Extrema Design Rules
 *
 *  This section describes what are believed to be the current rules
 *  about contents and usage of extrema:
 *
 *  Definitions:
 *
 *   Extrema: The array-valued lower and upper bounds of index
 *            vectors and their descendants. These are represented
 *            as the semi-open interval [ min, max), in the same
 *            way that GENERATORBOUND1/2 are.
 *
 *   AVIS_MINVAL: The lower bound of a value, represented as an N_avis pointer.
 *
 *   AVIS_MAXVAL: The upper bound of a value, represented as an N_avis pointer.
 *
 *   Precision: We would like extrema to be precise, but this turns out
 *              to be impractical in practice. Hence, extrema may be
 *              conservative estimates of a value.
 *              For example, AVIS_MINVAL of the result of a
 *              F_non_negval_A guard will be [0]. Guard removal
 *              may upgrade AVIS_MINVAL to a better estimate or
 *              known value.
 *
 *   Overwriting: When a better estimate of an extremum becomes available,
 *                IVEXP or other optimization will rewrite it.
 *
 * Examples:
 *
 *   WL index vector:
 *     (lb <= iv=[i] < ub)...
 *
 *     AVIS_MINVAL( iv) = ID_AVIS( lb);
 *     AVIS_MAXVAL( iv) = ID_AVIS( ub);
 *
 * Rationale for non-exact extrema:
 *
 *   We wish to allow Constant Folding to remove a _non_neg_val_V guard.
 *   The intermediate SAC code looks like this:
 *
 *     v = iota( id( 5));  NB. v is AKD; as id() hides value 5.
 *     s = idx_shape_sel( 0, v);
 *     s' = [ s];
 *     s'', p = _neg_neg_val_V( s');
 *
 *   Looking at the code, it is clear that s is always non-negative.
 *   However, if we have exact extrema, we are unable to pass
 *   that information on to CF.
 *
 *   If we allow extrema to be conservative estimates, we can allow
 *   the non-negative property of shape vector elements to be
 *   propagated to s', in this way:
 *
 *     v = iota( id( 5));  NB. v is AKD; as id() hides value 5.
 *     s = idx_shape_sel( 0, v);
 *     minv = 0;
 *     NB. We are unable to set maxv, as we have no idea of its value
 *     s' = _attachextrema ( s, minv, NULL);
 *     s'' = [ s'];
 *     s''', p = _neg_neg_val_V( s'');
 *
 *   The minv extrema will propagate through the N_array this way:
 *
 *     minv' = [ AVIS_MINVAL( s)];
 *     s'''' = [ s'];
 *     s'' = _attachextrema( s'''', minv', NULL);
 *     s''', p = _non_neg_val_V( s'');
 *
 *   When CF encounters the _non_neg_val_V, it will evaluate
 *   AVIS_MINVAL, find it to be non_negative, at which point
 *   it will eliminate the_non_neg_val_V guard.
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @defgroup ivexi Index Vector Extrema Insertion Traversal
 *
 * This traversal inserts maxima and minima for index vector variables
 * in with-loops and for other code, such as constraint guard removal.
 *
 * It is part of SAACYC because SWLF may slice a folderWL partition
 * into sub-partitions, and we will have to generate extrema for
 * those partitions
 *
 * Later optimizations will propagate these maxima
 * and minima, and use then as guidance for other optimizations,
 * such as with-loop folding and guard removal.
 *
 * Because -ssaiv is likely never to work unless one of us wins
 * the lottery and can hire minions to rewrite all the SAC
 * optimizations, we adopt the following kludge:
 *
 * Rather than annotating the WITH_IDs, we introduce temps, annotate
 * those, and rename all code block references to the WITH_IDs
 * to refer to the temps. E.g:
 *
 *   z = with {   (lb <= iv < ub) :
 *                  X[iv];
 *            } : genarray(shp,defaultcell);
 *
 *  This will become:
 *
 *   z = with {   (lb <= iv < ub) :
 *                  tmp = iv;
 *                  X[tmp];
 *            } : genarray(shp,defaultcell);
 *
 * We then set the extrema values:
 *
 *             AVIS_MINVAL(ID_AVIS(tmp)) = GENERATOR_BOUND1(wl)
 *             AVIS_MAXVAL(ID_AVIS(tmp)) = GENERATOR_BOUND2(wl);
 *
 * With any luck, these min/val values will be propagated down to the
 * X[tmp], at which point SWLFI can introduce inferences on them
 * for partition intersection computation. Although not required
 * by the above example, it is required for more complex linear functions
 * of the index vector, such as:
 *
 *              X[ (k*tmp) + offset ];
 *
 * They do, however, mark the end of shared N_code blocks in WLs.
 *
 * Some notes on phase ordering for ivextrema, ivexpropagate,
 * swlfi, and swlf:
 *
 *   ivextrema introduces new vardecs, and leaves old ones
 *   hanging around. The latter have to be removed, or later
 *   phases will reference them, and their now-dead AVIS_MINVAL/MAXVAL
 *   values, causing 'orrible pain. Hence, the presence of dcr2
 *   right after ivexi.
 *
 *   The other requirements are:
 *
 *      - ivexi must precede ivexp.
 *      - ivexp must precede swlfi.
 *      - swlfi must precede swlf.
 *      - scc and dcr3 must follow swlf, eventually.
 *
 * NB. traversal order:
 *   The DUPdoDupNodeLutSsa call that renames CODE_BLOCK WITHID references
 *   has the side effect of cloning any lacfns that occur within those
 *   blocks. Those clones are not attached to the FUNDEF spine until
 *   this traversal is complete. Since the original lacfns may appear
 *   in the FUNDEF spine before OR after the current fundef, it is not
 *   possible to ensure that they have been traversed by this code.
 *
 *   Two possible solutions to this are:
 *    a. Avoid the DUP call entirely. Rewrite this traversal to
 *       cover all code explicitly, and use AVIS_SUBST to do
 *       the renaming.
 *
 *    b. Traverse all lacfns as if they were inline, by traversing
 *       them when we encounter an N_ap that invokes them. This ensures
 *       that each lacfn is traversed before it is cloned.
 *       This is the approach we have taken.
 *       It uses the INFO_FROMAP flag to mark each traversal taking
 *       place in an N_ap call.
 *
 * @ingroup ivexi
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file ivextrema.c
 *
 * Prefix: IVEXI
 *
 *****************************************************************************/
#include "ivextrema.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "constants.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "pattern_match.h"
#include "LookUpTable.h"
#include "phase.h"
#include "globals.h"
#include "check.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *vardecs;
    node *preassignspart;
    node *preassignswith;
    node *with;
    lut_t *lutvars;
    lut_t *lutcodes;
    bool fromap;
    bool onefundef;
};

/**
 * INFO macros
 */
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASSIGNSPART(n) ((n)->preassignspart)
#define INFO_PREASSIGNSWITH(n) ((n)->preassignswith)
#define INFO_WITH(n) ((n)->with)
#define INFO_LUTVARS(n) ((n)->lutvars)
#define INFO_LUTCODES(n) ((n)->lutcodes)
#define INFO_FROMAP(n) ((n)->fromap)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNSPART (result) = NULL;
    INFO_PREASSIGNSWITH (result) = NULL;
    INFO_WITH (result) = NULL;
    INFO_LUTVARS (result) = NULL;
    INFO_LUTCODES (result) = NULL;
    INFO_FROMAP (result) = FALSE;
    INFO_ONEFUNDEF (result) = FALSE;

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
 * @fn node *IVEXIdoInsertIndexVectorExtrema( node *arg_node)
 *
 * @brief:
 *   Create two LUTs:
 *     LUTVARS holds information for renaming WITHID references
 *             to temp names within each WITH_CODE block.
 *     LUTCODES holds the old and new WITH_CODE block pointers,
 *             to let IVEXIpart correct the N_code pointers in
 *             each partition.
 *     Traverse subtree.
 *
 *****************************************************************************/
node *
IVEXIdoInsertIndexVectorExtrema (node *arg_node)
{
    info *arg_info;
    DBUG_ENTER ("IVEXIdoIndexVectorExtremaInsertion");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "IVEXIdoIndexVectorExtremaInsertion expected N_module");

    arg_info = MakeInfo ();
    INFO_LUTVARS (arg_info) = LUTgenerateLut ();
    INFO_LUTCODES (arg_info) = LUTgenerateLut ();
    INFO_ONEFUNDEF (arg_info) = FALSE;

    DBUG_PRINT ("IVEXI", ("Starting index vector extrema insertion traversal."));

    TRAVpush (TR_ivexi);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    INFO_LUTVARS (arg_info) = LUTremoveLut (INFO_LUTVARS (arg_info));
    INFO_LUTCODES (arg_info) = LUTremoveLut (INFO_LUTCODES (arg_info));

    DBUG_PRINT ("IVEXI", ("Index vector extrema insertion complete."));

    arg_info = FreeInfo (arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *IVEXImakeIntScalar(int k, node **vardecs, node **preassigns)
 *
 *   @brief Create flattened integer scalar of value k:
 *
 *          fid = k;
 *
 *   @param: arg_info - your basic arg_info
 *   @param: k : the value to be created.
 *
 *   @return N_id node for fid.
 ******************************************************************************/
node *
IVEXImakeIntScalar (int k, node **vardecs, node **preassigns)
{
    node *favis;
    node *fid;
    node *fids;
    node *fass;

    DBUG_ENTER ("IVEXImakeIntScalar");
    favis
      = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
    *vardecs = TBmakeVardec (favis, *vardecs);

    fid = TBmakeId (favis);
    fids = TBmakeIds (favis, NULL);
    fass = TBmakeAssign (TBmakeLet (fids, TBmakeNum (k)), NULL);
    *preassigns = TCappendAssign (*preassigns, fass);
    AVIS_SSAASSIGN (favis) = fass;

    if (isSAAMode ()) {
        AVIS_DIM (favis) = TBmakeNum (0);
        AVIS_SHAPE (favis) = TCmakeIntVector (NULL);
    }

    DBUG_RETURN (fid);
}

#ifndef DBUG_OFF
/** <!--********************************************************************-->
 *
 * @fn static bool isSameTypeShape( node *ida, node *idb)
 *
 * @brief predicate to see if two N_id nodes have same
 *        type, shape, and rank, ignoring AKS vs. AKV differences.
 *
 *****************************************************************************/
static bool
isSameTypeShape (node *ida, node *idb)
{
    bool z;
    ntype *typa;
    ntype *typb;

    DBUG_ENTER ("isSameTypeShape");

    typa = TYeliminateAKV (AVIS_TYPE (ID_AVIS (ida)));
    typb = TYeliminateAKV (AVIS_TYPE (ID_AVIS (idb)));

    z = TYeqTypes (typa, typb);

    typa = TYfreeType (typa);
    typb = TYfreeType (typb);

    DBUG_RETURN (z);
}
#endif //  DBUG_OFF

/** <!--********************************************************************-->
 *
 * @fn node *IVEXIattachExtrema( node *minv, node *maxv, node *id,
 *                               node **vardecs, node **preassigns,
 *                               prf nprf, node *lhs)
 *
 * @brief:
 *      Insert a temp, its vardec, and an assign for id,
 *      along with the extrema values minv and maxv, using the
 *      guard function nprf. Typically, nprf will be
 *      F_attachextrema.
 *
 *      We start with an N_id, id:
 *
 *           iv
 *
 *      and want to end up with:
 *
 *           iv' = _attachextrema(iv, minv, maxv);
 *
 *      and an appropriate vardec for iv', of course, such as:
 *
 *           int[.] iv';
 *
 * @params:
 *     id: The N_id of the name for which we want to build iv'.
 *     minv, maxv: the AVIS_MINVAL and AVIS_MAXVAL to be attached to iv'.
 *     preassigns: The address of an INFO_PREASSIGNS( arg_info) node
 *                 in the caller's environment.
 *     vardcs:     The address of an INFO_VARDECS( arg_info) node
 *                 in the caller's environment.
 *     lhsavis: If non-NULL, the N_avis of the name to be used for the
 *          result, iv'.
 *          If NULL, we make up a name.
 *
 *
 * @return: The N_avis of the new iv'.
 *
 *****************************************************************************/
node *
IVEXIattachExtrema (node *minv, node *maxv, node *id, node **vardecs, node **preassigns,
                    prf nprf, node *lhsavis)

{
    node *nas;
    node *args;
    node *ivavis;
    node *prf;

    DBUG_ENTER ("IVEXIattachExtrema");

    DBUG_ASSERT (N_id == NODE_TYPE (id), "IVEXIattachExtrema expected N_id for id");
    DBUG_ASSERT (N_id == NODE_TYPE (minv), "IVEXIattachExtrema expected N_id for minv");
    DBUG_ASSERT (N_id == NODE_TYPE (maxv), "IVEXIattachExtrema expected N_id for maxv");

    /* breaks in confusing manner. */
    DBUG_ASSERT (((F_attachextrema == nprf) && (isSameTypeShape (id, minv))),
                 ("IVEXIattachExtrema type mismatch: id, minv"));
    DBUG_ASSERT (((F_attachextrema == nprf) && (isSameTypeShape (id, maxv))),
                 ("IVEXIattachExtrema type mismatch: id, maxv"));
    ivavis = ID_AVIS (id);

    if (NULL == lhsavis) {
        lhsavis = TBmakeAvis (TRAVtmpVar (), TYeliminateAKV (AVIS_TYPE (ivavis)));
        *vardecs = TBmakeVardec (lhsavis, *vardecs);
    }

    if (isSAAMode ()) {
        AVIS_DIM (lhsavis) = DUPdoDupTree (AVIS_DIM (ivavis));
        AVIS_SHAPE (lhsavis) = DUPdoDupTree (AVIS_SHAPE (ivavis));
    }

    args = TBmakeExprs (id, TBmakeExprs (minv, TBmakeExprs (maxv, NULL)));

    prf = TBmakePrf (nprf, args);
    PRF_EXTREMAATTACHED (prf) = TRUE;
    nas = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL), prf), NULL);
    AVIS_SSAASSIGN (lhsavis) = nas;
    *preassigns = TCappendAssign (*preassigns, nas);

    if (F_attachextrema == nprf) {
        AVIS_MINVAL (lhsavis) = ID_AVIS (minv);
        AVIS_MAXVAL (lhsavis) = ID_AVIS (maxv);
    }

    DBUG_PRINT ("IVEXI", ("IVEXIattachExtrema introduced temp index variable: %s for: %s",
                          AVIS_NAME (lhsavis), AVIS_NAME (ivavis)));
    global.optcounters.ivexp_attach++;
    DBUG_RETURN (lhsavis);
}

#ifdef DEADCODE
/** <!--********************************************************************-->
 *
 *
 * @fn node *IVEXIadjustExtremaBound(node *arg_node, info *arg_info, int k,
 *                             node **vardecs, node **preassigns)
 *
 *   @brief arg_node is the N_avis of a GENERATOR_BOUND2 node, which
 *          we will adjust to be an exact extrema value
 *          by adding/subtracting 1 from it. This makes life
 *          easier downstream, when we start to do intersection calculation
 *          on the extrema.
 *          Actually, arg_node can also be a WITHID_IDS node,
 *          so we have to decide whether to generate vector
 *          or scalar add.
 *
 *          We generate, along with a vardec for b2':
 *
 *          b2' = _add_VxS_( b2, 1);   NB. k=1
 *          b2' = _sub_VxS_( b2, 1);   NB. k=-1
 *
 *
 *   @param  arg_node: a GENERATOR_BOUND2 N_avis node.
 *           int k:    Constant value to be used.
 *           vardecs:  Address of a vardecs chain that we will append to.
 *           preassigns: Address of a preassigns chain we will append to.
 *
 *   @return The N_avis result, b2', of the adjustment computation.
 *
 ******************************************************************************/
node *
IVEXIadjustExtremaBound (node *arg_node, info *arg_info, int k, node **vardecs,
                         node **preassigns)
{
    node *zavis;
    node *zids;
    node *zass;
    node *kid;
    int op;

    DBUG_ENTER ("IVEXIadjustExtremaBound");

    kid = IVEXImakeIntScalar (abs (k), vardecs, preassigns);

    zavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (arg_node)),
                        TYeliminateAKV (AVIS_TYPE (arg_node)));
    DBUG_PRINT ("IVEXI", ("IVEXIadjustExtremaBound introducing adjustment: %s for: %s",
                          AVIS_NAME (zavis), AVIS_NAME (arg_node)));

    *vardecs = TBmakeVardec (zavis, *vardecs);
    zids = TBmakeIds (zavis, NULL);
    if (1 == k) {
        op = TUisScalar (AVIS_TYPE (arg_node)) ? F_add_SxS : F_add_VxS;
    } else {
        op = TUisScalar (AVIS_TYPE (arg_node)) ? F_sub_SxS : F_sub_VxS;
    }
    zass
      = TBmakeAssign (TBmakeLet (zids, TCmakePrf2 (op, TBmakeId (arg_node), kid)), NULL);

    /* Keep us from trying to add extrema to the extrema calculations.  */
    PRF_NOEXTREMAWANTED (LET_EXPR (ASSIGN_INSTR (zass))) = TRUE;

    *preassigns = TCappendAssign (*preassigns, zass);
    AVIS_SSAASSIGN (zavis) = zass;
    AVIS_MINVAL (zavis) = zavis;
    AVIS_MAXVAL (zavis) = zavis;

    if (isSAAMode ()) {
        AVIS_DIM (zavis) = DUPdoDupTree (AVIS_DIM (arg_node));
        AVIS_SHAPE (zavis) = DUPdoDupTree (AVIS_SHAPE (arg_node));
    }

    DBUG_RETURN (zavis);
}
#endif // DEADCODE

/** <!--********************************************************************-->
 *
 *
 * @static
 * void printLHS( node *arg_node, info *arg_info)
 *
 *   @brief Print the LHS of the SSAASSIGN, so we can tell
 *          what we're looking at.
 *
 *   @param  arg_node: some N_assign node
 *
 *   @return nothing
 ******************************************************************************/
static void
printLHS (node *arg_node, info *arg_info)
{
    node *instr;
    node *avis;

    DBUG_ENTER ("printLHS");

    instr = ASSIGN_INSTR (arg_node);
    if (N_let == NODE_TYPE (instr)) {
        avis = IDS_AVIS (LET_IDS (instr));
#ifdef VERBOSE
        DBUG_PRINT ("IVEXI", ("Looking at: %s", AVIS_NAME (avis)));
#endif // VERBOSE
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 *
 * @fn node *removeUnusedCodes(node *codes)
 *
 *   @brief removes all unused N_codes recursively
 *
 *   @param  node *codes : N_code chain
 *   @return node *      : modified N_code chain
 *
 * stolen from WithLoopFusion.c
 *
 ******************************************************************************/
static node *
removeUnusedCodes (node *codes)
{
    DBUG_ENTER ("removeUnusedCodes");

    DBUG_ASSERT ((codes != NULL), "no codes available!");
    DBUG_ASSERT ((NODE_TYPE (codes) == N_code), "type of codes is not N_code!");

    if (CODE_NEXT (codes) != NULL)
        CODE_NEXT (codes) = removeUnusedCodes (CODE_NEXT (codes));

    if (CODE_USED (codes) == 0)
        codes = FREEdoFreeNode (codes);

    DBUG_RETURN (codes);
}

/** <!--********************************************************************-->
 *
 * @fn static
 * node *generateSelect( node *arg_node, info *arg_info, int k)
 *
 * @brief:
 * Select the k-th element of a WL generator bound.
 * We start with a N_avis for a generator bound, arg_node:
 *
 *              arg_node = [ I, J ];
 * Create:
 *             kk = k;          NB. Flatten k
 *             k' = [kk];
 *             z = _sel_VxA_([k], arg_node);
 *             and associated vardecs.
 * @params:
 *     arg_node: The N_avis node for a generator bound.
 *     arg_info: Your basic arg_info stuff.
 *     k:        which element of the bound we want to select.
 *
 * @return: The N_avis of the new temp, z.
 *
 *****************************************************************************/
static node *
generateSelect (node *arg_node, info *arg_info, int k)
{
    node *kavis;
    node *kid;
    node *kids;
    node *kass;

    node *zavis;
    node *zids;
    node *zass;

    node *fid;

    DBUG_ENTER ("generateSelect");

    /* Flatten k */

    fid
      = IVEXImakeIntScalar (k, &INFO_VARDECS (arg_info), &INFO_PREASSIGNSWITH (arg_info));

    /* Create k' = [k]; */
    kavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (arg_node)),
                        TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, 1)));
    INFO_VARDECS (arg_info) = TBmakeVardec (kavis, INFO_VARDECS (arg_info));

    kid = TBmakeId (kavis);
    kids = TBmakeIds (kavis, NULL);
    kass
      = TBmakeAssign (TBmakeLet (kids, TBmakeArray (TYmakeAKS (TYmakeSimpleType (T_int),
                                                               SHcreateShape (0)),
                                                    SHcreateShape (1, 1),
                                                    TBmakeExprs (fid, NULL))),
                      NULL);
    INFO_PREASSIGNSWITH (arg_info)
      = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), kass);
    AVIS_SSAASSIGN (kavis) = kass;
    AVIS_MINVAL (kavis) = kavis;
    AVIS_MAXVAL (kavis) = kavis;

    if (isSAAMode ()) {
        AVIS_DIM (kavis) = TBmakeNum (1);
        AVIS_SHAPE (kavis) = TCmakeIntVector (TBmakeExprs (TBmakeNum (1), NULL));
    }
    DBUG_PRINT ("IVEXI", ("generateSelect flattened k: %s for: %s", AVIS_NAME (kavis),
                          AVIS_NAME (arg_node)));

    /* Create z = _sel_VxA_([k], bound);  */

    zavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (arg_node)),
                        TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
    INFO_VARDECS (arg_info) = TBmakeVardec (zavis, INFO_VARDECS (arg_info));
    zids = TBmakeIds (zavis, NULL);
    zass
      = TBmakeAssign (TBmakeLet (zids, TCmakePrf2 (F_sel_VxA, kid, TBmakeId (arg_node))),
                      NULL);

    INFO_PREASSIGNSWITH (arg_info)
      = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), zass);
    AVIS_SSAASSIGN (zavis) = zass;
    AVIS_MINVAL (zavis) = zavis;
    AVIS_MAXVAL (zavis) = zavis;

    if (isSAAMode ()) {
        AVIS_DIM (zavis) = TBmakeNum (0);
        AVIS_SHAPE (zavis) = TCmakeIntVector (NULL);
    }

    DBUG_PRINT ("IVEXI", ("generateSelect introduced temp index variable: %s for: %s",
                          AVIS_NAME (zavis), AVIS_NAME (arg_node)));
    DBUG_RETURN (zavis);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXItmpVec( node *arg_node, info *arg_info, node* ivavis))
 *
 * @brief:
 *      Insert a temp, its vardec, and an assign for ivavis.
 *      We start with an WITHID_VEC:
 *
 *           iv
 *
 *      and want:
 *
 *           iv' = _attachextrema(iv, GENERATOR_BOUND1(partn),
 *                                    GENERATOR_BOUND2(partn) - 1);
 *
 *      and an appropriate vardec for iv', of course.
 *
 *      We build an N_id for iv because WITHIDs don't have them,
 *      just N_avis nodes. If -ssaiv becomes a reality, this
 *      can be scrapped, and we can attach the extrema to iv
 *      directly.
 *
 * @params:
 *     ivavis: The N_avis of the name for which we want to build iv'.
 *     arg_node: An N_part of the WL.
 *     arg_info: Your basic arg_info stuff.
 *
 * @return: The N_avis of the new temp.
 *
 *****************************************************************************/
node *
IVEXItmpVec (node *arg_node, info *arg_info, node *ivavis)
{
    node *avis;
    node *b1;
    node *b2;

    DBUG_ENTER ("IVEXItmpVec");

    DBUG_ASSERT (N_avis == NODE_TYPE (ivavis), "IVEXItmpVec expected N_avis");
    b1 = GENERATOR_BOUND1 (PART_GENERATOR (arg_node));
    b2 = GENERATOR_BOUND2 (PART_GENERATOR (arg_node));
    DBUG_ASSERT (N_id == NODE_TYPE (b1),
                 "IVEXItmpVec expected N_id for GENERATOR_BOUND1");
    DBUG_ASSERT (N_id == NODE_TYPE (b2),
                 "IVEXItmpVec expected N_id for GENERATOR_BOUND2");

    avis = IVEXIattachExtrema (DUPdoDupTree (b1), DUPdoDupTree (b2), TBmakeId (ivavis),
                               &INFO_VARDECS (arg_info), &INFO_PREASSIGNSPART (arg_info),
                               F_attachextrema, NULL);
    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXItmpIds( node *arg_node, info *arg_info, node* oldavis, int k)
 *
 * @brief:  Same as IVEXItmpVec, except this one handles one
 *          of the WITHID_IDS scalar variables i,j,k in
 *            IV = [i,j,k]
 *
 *          This is slightly more complex, because we have to
 *          select the appropriate generator elements.
 *
 *     oldavis: The N_avis of the name for which we want to build iv'.
 *     arg_node: An N_part of the WL.
 *     arg_info: Your basic arg_info stuff.
 *     k:        the index of the scalar, e.g., i=0, j=1, k=2
 * @return: The N_avis of the new temp.
 *
 *****************************************************************************/
node *
IVEXItmpIds (node *arg_node, info *arg_info, node *oldavis, int k)
{
    node *avis;
    node *nas;
    node *args;
    node *b1;
    node *b2;

    DBUG_ENTER ("IVEXItmpIds");

    DBUG_ASSERT (N_avis == NODE_TYPE (oldavis), "IVEXItmpIds expected N_avis");

    b1 = GENERATOR_BOUND1 (PART_GENERATOR (arg_node));
    b2 = GENERATOR_BOUND2 (PART_GENERATOR (arg_node));
    DBUG_ASSERT (N_id == NODE_TYPE (b1),
                 "IVEXItmpIds expected N_id for GENERATOR_BOUND1");
    DBUG_ASSERT (N_id == NODE_TYPE (b2),
                 "IVEXItmpIds expected N_id for GENERATOR_BOUND2");

    b1 = TBmakeId (generateSelect (ID_AVIS (b1), arg_info, k));
    b2 = TBmakeId (generateSelect (ID_AVIS (b2), arg_info, k));

    avis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (oldavis)),
                       TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

    args = TBmakeExprs (TBmakeId (oldavis), TBmakeExprs (b1, TBmakeExprs (b2, NULL)));
    nas = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                   TBmakePrf (F_attachextrema, args)),
                        NULL);
    INFO_PREASSIGNSPART (arg_info) = TCappendAssign (INFO_PREASSIGNSPART (arg_info), nas);
    AVIS_SSAASSIGN (avis) = nas;
    AVIS_MINVAL (avis) = ID_AVIS (b1);
    AVIS_MAXVAL (avis) = ID_AVIS (b2);

    if (isSAAMode ()) {
        AVIS_DIM (avis) = TBmakeNum (0);
        AVIS_SHAPE (avis) = TCmakeIntVector (NULL);
    }

    DBUG_PRINT ("IVEXI", ("IVEXItmpIds introduced temp index variable: %s for: %s",
                          AVIS_NAME (avis), AVIS_NAME (oldavis)));
    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn static void populateLUTVars( node *arg_node, info *arg_info)
 *
 * @brief:
 *    1. populate INFO_LUTVARS with WITHID names and new temp names.
 *    2. Call IVEXItmpVec to create temp names and the assigns
 *       that will start off the updated code blocks.
 *
 * @params:
 *     arg_node: An N_part of the WL.
 *     arg_info: Your basic arg_info stuff.
 *
 * @return:
 *
 *****************************************************************************/
static void
populateLUTVars (node *arg_node, info *arg_info)
{
    node *oldavis;
    node *navis;
    node *ids;
    int k = 0;

    DBUG_ENTER ("populateLUTVars");

    /* Populate LUTVARS with WITHID_VEC and new name. */
    oldavis = IDS_AVIS (WITHID_VEC (PART_WITHID (arg_node)));
    navis = IVEXItmpVec (arg_node, arg_info, oldavis);
    LUTinsertIntoLutP (INFO_LUTVARS (arg_info), oldavis, navis);
    DBUG_PRINT ("IVEXI", ("Inserting WITHID_VEC into lut: oldid: %s, newid: %s",
                          AVIS_NAME (oldavis), AVIS_NAME (navis)));

    /* Rename withid scalars */
    ids = WITHID_IDS (PART_WITHID (arg_node));
    while (ids != NULL) {
        oldavis = IDS_AVIS (ids);
        navis = IVEXItmpIds (arg_node, arg_info, oldavis, k);
        DBUG_PRINT ("IVEXI", ("Inserting WITHID_IDS into lut: oldid: %s, newid: %s",
                              AVIS_NAME (oldavis), AVIS_NAME (navis)));
        LUTinsertIntoLutP (INFO_LUTVARS (arg_info), oldavis, navis);
        ids = IDS_NEXT (ids);
        k++;
    }
    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *IVEXImodule( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
IVEXImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXImodule");

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIfundef( node *arg_node, info *arg_info)
 *
 * description:
 *   Traverse a function.
 *   If the function is a lacfn and we are traversing it from
 *   the N_ap call, handle it now. Otherwise, ignore it.
 *
 *   Implication: We do not have to traverse FUNDEF_LOCALFUNS
 *   explicitly in this traversal, since all local functions will
 *   be traversed when we encounter their invocation.
 *
 ******************************************************************************/
node *
IVEXIfundef (node *arg_node, info *arg_info)
{
    info *old_info;

    DBUG_ENTER ("IVEXIfundef");
    DBUG_PRINT ("IVEXI", ("IVEXI in %s %s begins",
                          (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                          FUNDEF_NAME (arg_node)));

    old_info = arg_info;
    arg_info = MakeInfo ();

    INFO_FUNDEF (arg_info) = arg_node;
    INFO_FROMAP (arg_info) = INFO_FROMAP (old_info);
    INFO_LUTVARS (arg_info) = INFO_LUTVARS (old_info);
    INFO_LUTCODES (arg_info) = INFO_LUTCODES (old_info);

    if (((FUNDEF_ISLACFUN (arg_node)) && (INFO_FROMAP (arg_info)))
        || (!FUNDEF_ISLACFUN (arg_node))) {
        INFO_FROMAP (arg_info) = FALSE;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }

    /* If new vardecs were made, append them to the current set */
    if (INFO_VARDECS (arg_info) != NULL) {
        BLOCK_VARDEC (FUNDEF_BODY (arg_node))
          = TCappendVardec (INFO_VARDECS (arg_info),
                            BLOCK_VARDEC (FUNDEF_BODY (arg_node)));
        INFO_VARDECS (arg_info) = NULL;
    }

    DBUG_PRINT ("IVEXI", ("IVEXI in %s %s ends",
                          (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                          FUNDEF_NAME (arg_node)));

    if ((!FUNDEF_ISLACFUN (arg_node))) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }
    arg_info = FreeInfo (arg_info);
    INFO_FROMAP (old_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIvardec( node *arg_node, info *arg_info)
 *
 * description:
 *   Dead code FIXME - just delete this
 *
 ******************************************************************************/
node *
IVEXIvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIvardec");

    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIblock( node *arg_node, info *arg_info)
 *
 * description:
 *    We get here from the fundef and from WLs.
 *    Traverse into the block body.
 *    Also, set extrema for all constant-valued vardecs.
 *
 ******************************************************************************/
node *
IVEXIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIblock");

    DBUG_PRINT ("IVEXI", ("Traversing block"));
    BLOCK_VARDEC (arg_node) = TRAVopt (BLOCK_VARDEC (arg_node), arg_info);
    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIwith( node *arg_node, info *arg_info)
 *
 * description:
 *
 *   Traverse partitions to replace WITHID references by their
 *   shiny, new ones, using LUTVARS. That traversal will
 *   populate LUTCODES with old and new WITH_CODE pointer.
 *
 ******************************************************************************/
node *
IVEXIwith (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("IVEXIwith");

    DBUG_PRINT ("IVEXI", ("Traversing with"));
    if (!WITH_EXTREMAATTACHED (arg_node)) {
        INFO_WITH (arg_info) = arg_node;

        /* Traverse the partitions, to define new temps. */
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        WITH_EXTREMAATTACHED (arg_node) = TRUE;
    }

    WITH_CODE (arg_node) = removeUnusedCodes (WITH_CODE (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIlet( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
IVEXIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIlet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIassign( node *arg_node, info *arg_info)
 *
 * description:
 *  Handle insertion of new preassigns. These are generated
 *  by _sel_VxA_ ops inside a WL, and are preprended to
 *  the N_assign containing that N_with.
 *
 *  Things are slightly tricky because of the need to handle
 *  nested WLs: we construct a new arg_info node to hide any
 *  existing preassigns from the outer WL.
 *
 ******************************************************************************/
node *
IVEXIassign (node *arg_node, info *arg_info)
{
    info *old_info = NULL;

    DBUG_ENTER ("IVEXIassign");

    printLHS (arg_node, arg_info);

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    if ((N_let == NODE_TYPE (ASSIGN_INSTR (arg_node)))
        && (N_with == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node))))) {
        old_info = arg_info;
        arg_info = MakeInfo ();
        INFO_FUNDEF (arg_info) = INFO_FUNDEF (old_info);
        INFO_VARDECS (arg_info) = INFO_VARDECS (old_info);
        INFO_LUTVARS (arg_info) = INFO_LUTVARS (old_info);
        INFO_LUTCODES (arg_info) = INFO_LUTCODES (old_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /* Handle any preassigns generated with this WL */
    if ((N_let == NODE_TYPE (ASSIGN_INSTR (arg_node)))
        && (N_with == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node))))
        && (NULL != INFO_PREASSIGNSWITH (arg_info))) {
        DBUG_PRINT ("IVEXI", ("Prepending PREASSIGNSWITH"));
        arg_node = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), arg_node);
        INFO_PREASSIGNSWITH (arg_info) = NULL;
    }

    if (NULL != old_info) {
        INFO_VARDECS (old_info) = INFO_VARDECS (arg_info);
        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIpart( node *arg_node, info *arg_info)
 *
 * description:
 *   Create temp vardecs and assigns.
 *   Rename code block references to WITHIDs.
 *
 *   Set AVIS_MINVAL, AVIS_MAXVAL from the partition
 *   generator bounds.
 *
 *   We fix all the code blocks here, and chain
 *   together for IVEXIwith.
 *
 *   When this phase completes, all sharing of WITH_CODE blocks is lost.
 *
 ******************************************************************************/
node *
IVEXIpart (node *arg_node, info *arg_info)
{
    node *newcode;

    DBUG_ENTER ("IVEXIpart");

    DBUG_PRINT ("IVEXI", ("Traversing part"));
    /* We will deal with our partition after looking inside it */
    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);

    /* Don't try this on empty code blocks, kids. */
    /* Or default partitions, either! */
    if ((N_empty != NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (PART_CODE (arg_node)))))
        && (!PART_HASEXTREMA (arg_node))
        && (N_default != NODE_TYPE (PART_GENERATOR (arg_node)))) {

        populateLUTVars (arg_node, arg_info);

        newcode = DUPdoDupNodeLutSsa (PART_CODE (arg_node), INFO_LUTVARS (arg_info),
                                      INFO_FUNDEF (arg_info));
        LUTremoveContentLut (INFO_LUTVARS (arg_info));

        /* No longer using old code block */
        (CODE_USED (PART_CODE (arg_node)))--;
        CODE_USED (newcode) = 1;
        PART_CODE (arg_node) = newcode;

        /* Link new code block to WLs chain */
        CODE_NEXT (newcode) = WITH_CODE (INFO_WITH (arg_info));
        WITH_CODE (INFO_WITH (arg_info)) = newcode;

        if (NULL != INFO_PREASSIGNSPART (arg_info)) {
            BLOCK_INSTR (CODE_CBLOCK (newcode))
              = TCappendAssign (INFO_PREASSIGNSPART (arg_info),
                                BLOCK_INSTR (CODE_CBLOCK (newcode)));
            INFO_PREASSIGNSPART (arg_info) = NULL;
        }
        PART_HASEXTREMA (arg_node) = TRUE;
    }

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIprf( node *arg_node, info *arg_info)
 *
 * description:
 *   Insert extrema for some primitives, if they do not
 *   have them already.
 *
 ******************************************************************************/
node *
IVEXIprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIprf");

    switch (PRF_PRF (arg_node)) {
    case F_non_neg_val_V:
    case F_val_lt_shape_VxA:
    case F_same_shape_AxA:
    case F_shape_matches_dim_VxA:
    case F_val_le_val_VxV:
    case F_prod_matches_prod_shape_VxA:
    case F_shape_A:
    case F_idx_shape_sel:
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIcond(node *arg_node, info *arg_info)
 *
 * description:
 *   Traverse an IF statement
 *
 ******************************************************************************/
node *
IVEXIcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIcond");

    DBUG_PRINT ("IVEXI", ("Traversing cond"));
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVopt (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIfuncond(node *arg_node, info *arg_info)
 *
 * description:
 *   Traverse an  x = cond ? trueval : falseval;
 *
 ******************************************************************************/
node *
IVEXIfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIfuncond");

    DBUG_PRINT ("IVEXI", ("Traversing funcond"));
    FUNCOND_IF (arg_node) = TRAVdo (FUNCOND_IF (arg_node), arg_info);
    FUNCOND_THEN (arg_node) = TRAVopt (FUNCOND_THEN (arg_node), arg_info);
    FUNCOND_ELSE (arg_node) = TRAVopt (FUNCOND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
/******************************************************************************
 *
 * function:
 *   node *IVEXIwhile( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
IVEXIwhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIwhile");

    DBUG_PRINT ("IVEXI", ("Traversing while"));
    WHILE_COND (arg_node) = TRAVopt (WHILE_COND (arg_node), arg_info);
    WHILE_BODY (arg_node) = TRAVopt (WHILE_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIap( node *arg_node, info *arg_info)
 *
 * description:
 *   Traverse an N_ap. This is only important for lacfns.
 *   The idea is to traverse each lacfn BEFORE we DUPdoDupNodeLutSsa
 *   the function that invokes the lacfn. That ensures that
 *   the lacfns has extrema attached to itself before it is
 *   cloned. Once the lacfn is cloned, it is not visible to us
 *   during the remainder of this traversal, so it must
 *   be handled here.
 *
 *   Avoid the traversal if this is the recursive call within a loop-fun.
 *
 ******************************************************************************/
node *
IVEXIap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIap");

    DBUG_PRINT ("IVEXI", ("Traversing ap"));
    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        INFO_FROMAP (arg_info) = TRUE;
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        INFO_FROMAP (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}
