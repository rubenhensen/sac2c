/*
 * $Id: ivextrema.c 15815 2008-10-24 18:04:47Z rbe $
 */

/** <!--********************************************************************-->
 *
 * @defgroup ivexi Index Vector Extrema Insertion Traversal
 *
 * This traversal inserts maxima and minima for index vector variables
 * in with-loops. Later optimizations will propagate these maxima
 * and minima, and use then as input for other optimizations,
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
 *             AVIS_MAXVAL(ID_AVIS(tmp)) = GENERATOR_BOUND2(wl) - 1 ;
 *
 * Note that the extrema are exact. This makes computation
 * on them much easier.
 *
 * With any luck, these min/val values will be propagated down to the
 * X[tmp], at which point SWLFI can introduce inferences on them
 * for partition intersection computation. Although not required
 * by the above example, it is required for more complex linear functions
 * of the index vector, such as:
 *
 *              X[ (k*tmp) + offset ];
 *
 * They do, however, mark the end of shared N_code blocks in WLs,
 * just as -ssaiv does.
 *
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
                 "IVEXIdoIndexVectorExtremaInsertion expected N_modules");

    arg_info = MakeInfo ();
    INFO_LUTVARS (arg_info) = LUTgenerateLut ();
    INFO_LUTCODES (arg_info) = LUTgenerateLut ();

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

/** <!--********************************************************************-->
 *
 * @fn node *IVEXIattachExtrema( node *minv, node *maxv, node *id,
 *                               node **vardecs, node **preassigns,
 *                               prf nprf)
 *
 * @brief:
 *      Insert a temp, its vardec, and an assign for id,
 *      along with the extrema values minv and maxv, using the
 *      guard function nprf. Typically, nprf will be
 *      F_attachextrema or F_attachextreman.
 *
 *      We start with N_id id:
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
 *
 * @return: The N_avis of the new iv'.
 *
 *****************************************************************************/
node *
IVEXIattachExtrema (node *minv, node *maxv, node *id, node **vardecs, node **preassigns,
                    prf nprf)

{
    node *avis;
    node *nas;
    node *args;
    node *ivavis;
    node *prf;

    DBUG_ENTER ("IVEXIattachExtrema");

    DBUG_ASSERT (N_id == NODE_TYPE (id), "IVEXIattachExtrema expected N_id for id");
    DBUG_ASSERT (N_id == NODE_TYPE (minv), "IVEXIattachExtrema expected N_id for minv");
    DBUG_ASSERT (N_id == NODE_TYPE (maxv), "IVEXIattachExtrema expected N_id for maxv");

    DBUG_ASSERT (((F_attachextrema != nprf) || isSameTypeShape (id, minv)),
                 ("IVEXIattachExtrema type mismatch: id, minv"));
    DBUG_ASSERT (((F_attachextrema != nprf) || isSameTypeShape (id, maxv)),
                 ("IVEXIattachExtrema type mismatch: id, maxv"));

    ivavis = ID_AVIS (id);
    avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (ivavis)));
    if (isSAAMode ()) {
        AVIS_DIM (avis) = DUPdoDupTree (AVIS_DIM (ivavis));
        AVIS_SHAPE (avis) = DUPdoDupTree (AVIS_SHAPE (ivavis));
    }
    *vardecs = TBmakeVardec (avis, *vardecs);

    args = TBmakeExprs (id, TBmakeExprs (minv, TBmakeExprs (maxv, NULL)));

    prf = TBmakePrf (nprf, args);
    PRF_EXTREMAATTACHED (prf) = TRUE;
    nas = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), prf), NULL);
    AVIS_SSAASSIGN (avis) = nas;
    *preassigns = TCappendAssign (*preassigns, nas);

    if (F_attachextrema == nprf) {
        AVIS_MINVAL (avis) = ID_AVIS (minv);
        AVIS_MAXVAL (avis) = ID_AVIS (maxv);
    }

    DBUG_PRINT ("IVEXI", ("IVEXIattachExtrema introduced temp index variable: %s for: %s",
                          AVIS_NAME (avis), AVIS_NAME (ivavis)));
    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 *
 * @fn node *IVEXIadjustExtremaBound(node *arg_node, int k,
 *                             node **vardecs, node **preassigns)
 *
 *   @brief arg_node is the N_avis of a GENERATOR_BOUND2 node, which
 *          we will adjust to be an exact extrema value
 *          by adding/subtracting 1 from it. This makes life
 *          easier downstream, when we start to do arithmetic
 *          on the extrema.
 *          Actually, arg_node can also be a WITHID_IDS node,
 *          so we have to decide whether to generate vector
 *          or scalar add.
 *
 *          We generate, along with a vardec for b2':
 *
 *          b2' = _add_VxS_( b2, k);
 *
 *   @param  arg_node: a GENERATOR_BOUND2 N_avis node.
 *           int n:    Constant value to be used as
 *           vardecs:  Address of a vardecs chain that we will append to.
 *           preassigns: Address of a preassigns chain we will append to.
 *
 *   @return The N_avis result, b2', of the adjustment computation.
 *
 ******************************************************************************/
node *
IVEXIadjustExtremaBound (node *arg_node, int k, node **vardecs, node **preassigns)
{
    node *zavis;
    node *zids;
    node *zass;
    int op;

    DBUG_ENTER ("IVEXIadjustExtremaBound");

    zavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (arg_node)),
                        TYcopyType (AVIS_TYPE (arg_node)));
    *vardecs = TBmakeVardec (zavis, *vardecs);
    zids = TBmakeIds (zavis, NULL);
    op = TUisScalar (AVIS_TYPE (arg_node)) ? F_add_SxS : F_add_VxS;
    zass = TBmakeAssign (TBmakeLet (zids,
                                    TCmakePrf2 (op, TBmakeId (arg_node), TBmakeNum (k))),
                         NULL);

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

/** <!--********************************************************************-->
 *
 *
 * @static
 * void IVEXIprintLHS( node *arg_node, info *arg_info)
 *
 *   @brief Print the LHS of the SSAASSIGN, so we can tell
 *          what we're looking at.
 *
 *   @param  arg_node: some N_assign node
 *
 *   @return nothing
 ******************************************************************************/
static void
IVEXIprintLHS (node *arg_node, info *arg_info)
{
    node *instr;
    node *avis;

    DBUG_ENTER ("IVEXIprintLHS");

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
 * stolen from WithLoopFusion.c
 *
 * @fn node IVEXIRemoveUnusedCodes(node *codes)
 *
 *   @brief removes all unused N_codes recursively
 *
 *   @param  node *codes : N_code chain
 *   @return node *      : modified N_code chain
 ******************************************************************************/
static node *
IVEXIRemoveUnusedCodes (node *codes)
{
    DBUG_ENTER ("IVEXIRemoveUnusedCodes");
    DBUG_ASSERT ((codes != NULL), "no codes available!");
    DBUG_ASSERT ((NODE_TYPE (codes) == N_code), "type of codes is not N_code!");

    if (CODE_NEXT (codes) != NULL)
        CODE_NEXT (codes) = IVEXIRemoveUnusedCodes (CODE_NEXT (codes));

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

    node *favis;
    node *fid;
    node *fids;
    node *fass;

    DBUG_ENTER ("generateSelect");

    /* Flatten k */

    favis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (arg_node)),
                        TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
    INFO_VARDECS (arg_info) = TBmakeVardec (favis, INFO_VARDECS (arg_info));

    fid = TBmakeId (favis);
    fids = TBmakeIds (favis, NULL);
    fass = TBmakeAssign (TBmakeLet (fids, TBmakeNum (k)), NULL);
    INFO_PREASSIGNSWITH (arg_info)
      = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), fass);
    AVIS_SSAASSIGN (favis) = fass;

    if (isSAAMode ()) {
        AVIS_DIM (favis) = TBmakeNum (0);
        AVIS_SHAPE (favis) = TCmakeIntVector (NULL);
    }

    AVIS_MINVAL (favis) = favis;
    AVIS_MAXVAL (favis) = favis;

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

    b2 = IVEXIadjustExtremaBound (ID_AVIS (b2), -1, &INFO_VARDECS (arg_info),
                                  &INFO_PREASSIGNSPART (arg_info));

    avis = IVEXIattachExtrema (DUPdoDupTree (b1), TBmakeId (b2), TBmakeId (ivavis),
                               &INFO_VARDECS (arg_info), &INFO_PREASSIGNSPART (arg_info),
                               F_attachextrema);
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
    b2 = generateSelect (ID_AVIS (b2), arg_info, k);
    b2 = TBmakeId (IVEXIadjustExtremaBound (b2, -1, &INFO_VARDECS (arg_info),
                                            &INFO_PREASSIGNSPART (arg_info)));

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
        DBUG_PRINT ("IVEXIpart", ("Inserting WITHID_IDS into lut: oldid: %s, newid: %s",
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
    if (NULL != MODULE_FUNS (arg_node)) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIfundef( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
IVEXIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIfundef");
    DBUG_PRINT ("IVEXI", ("IVEXI in %s %s begins",
                          (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                          FUNDEF_NAME (arg_node)));

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    INFO_FUNDEF (arg_info) = NULL;

    /* If new vardecs were made, append them to the current set */
    if (INFO_VARDECS (arg_info) != NULL) {
        BLOCK_VARDEC (FUNDEF_BODY (arg_node))
          = TCappendVardec (INFO_VARDECS (arg_info),
                            BLOCK_VARDEC (FUNDEF_BODY (arg_node)));
        INFO_VARDECS (arg_info) = NULL;
    }

    DBUG_PRINT ("IVEX", ("IVEXI in %s %s ends",
                         (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                         FUNDEF_NAME (arg_node)));

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIblock( node *arg_node, info *arg_info)
 *
 * description:
 *    We get here from the fundef and from WLs.
 *
 ******************************************************************************/
node *
IVEXIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIblock");

    BLOCK_INSTR (arg_node) = TRAVopt (BLOCK_INSTR (arg_node), arg_info);

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

    if (!WITH_EXTREMAATTACHED (arg_node)) {
        INFO_WITH (arg_info) = arg_node;

        /* Traverse the partitions, to define new temps. */
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        WITH_EXTREMAATTACHED (arg_node) = TRUE;
    }

    /* If partitions were unshared, we could eliminate this. */
    WITH_CODE (arg_node) = IVEXIRemoveUnusedCodes (WITH_CODE (arg_node));

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
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIavis( node *arg_node, info *arg_info)
 *
 * description:
 *  Set extrema for AKV avis nodes.
 *
 ******************************************************************************/
node *
IVEXIavis (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("IVEXIavis");

    if (TYisAKV (AVIS_TYPE (arg_node))) {
        AVIS_MINVAL (arg_node) = arg_node;
        AVIS_MAXVAL (arg_node) = arg_node;
    }

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
    info *old_info;

    DBUG_ENTER ("IVEXIassign");

    IVEXIprintLHS (arg_node, arg_info);

    if ((N_let == NODE_TYPE (ASSIGN_INSTR (arg_node)))
        && (N_with == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node))))) {
        old_info = arg_info;
        arg_info = MakeInfo ();
        INFO_FUNDEF (arg_info) = INFO_FUNDEF (old_info);
        INFO_VARDECS (arg_info) = INFO_VARDECS (old_info);
        INFO_LUTVARS (arg_info) = INFO_LUTVARS (old_info);
        INFO_LUTCODES (arg_info) = INFO_LUTCODES (old_info);

        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

        /* Handle any preassigns from any inner WL */
        if (NULL != INFO_PREASSIGNSWITH (arg_info)) {
            arg_node = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), arg_node);
            INFO_PREASSIGNSWITH (arg_info) = NULL;
        }

        INFO_VARDECS (old_info) = INFO_VARDECS (arg_info);
        arg_info = FreeInfo (arg_info);
        arg_info = old_info;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

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
    node *codenext;
    node *newcode;

    DBUG_ENTER ("IVEXIpart");

    /* We will deal with our partition after looking inside it */
    PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);

    /* Don't try this on empty code blocks, kids. */
    /* Or default partitions, either! */
    if ((N_empty != NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (PART_CODE (arg_node)))))
        && (N_default != NODE_TYPE (PART_GENERATOR (arg_node)))) {

        populateLUTVars (arg_node, arg_info);

        /* We have to keep the renamer's paws off other partitions. */
        codenext = CODE_NEXT (PART_CODE (arg_node));
        CODE_NEXT (PART_CODE (arg_node)) = NULL;
        newcode = DUPdoDupTreeLutSsa (PART_CODE (arg_node), INFO_LUTVARS (arg_info),
                                      INFO_FUNDEF (arg_info));
        CODE_NEXT (PART_CODE (arg_node)) = codenext;
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
    }

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
