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
 *            See also ast.xml.
 *
 *   AVIS_MIN: The lower bound of a value, represented as an N_id.
 *
 *   AVIS_MAX: The upper bound of a value, represented as an N_id.
 *
 *   Precision: We would like extrema to be precise, but this turns out
 *              to be impractical in practice. Hence, extrema may be
 *              conservative estimates of a value.
 *              For example, AVIS_MINof the result of a
 *              F_non_negval_A guard will be [0]. Guard removal
 *              may upgrade AVIS_MINto a better estimate or
 *              known value.
 *
 *   Overwriting: When a better estimate of an extremum becomes available,
 *                IVEXP or other optimization will replace it.
 *
 * Examples:
 *
 *   WL index vector:
 *     (lb <= iv=[i] < ub)...
 *
 *     AVIS_MIN( iv) = DUPdoDupNode( lb);
 *     AVIS_MAX( iv) = DUPdoDupNode( ub);
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
 *     s' = _noteminval( s, minv, NULL);
 *     s'' = [ s'];
 *     s''', p = _neg_neg_val_V( s'');
 *
 *   The minv extrema will propagate through the N_array this way:
 *
 *     minv' = [ AVIS_MIN( s)];
 *     s'''' = [ s'];
 *     s'' = _noteminval( s'''', minv', NULL);
 *     s''', p = _non_neg_val_V( s'');
 *
 *   When CF encounters the _non_neg_val_V, it will evaluate
 *   AVIS_MIN, find it to be non_negative, at which point
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
 *             AVIS_MIN(ID_AVIS(tmp)) = DUPdoDupNode( GENERATOR_BOUND1(wl));
 *             AVIS_MAX(ID_AVIS(tmp)) = DUPdoDupNode( GENERATOR_BOUND2(wl));
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
 *   phases will reference them, and their now-dead AVIS_MIN/MAX
 *   values, causing 'orrible pain. Hence, the presence of dcr2
 *   right after ivexi.
 *
 *   The other requirements are:
 *
 *      - ivexi must precede ivexp.
 *      - ivexp must precede awlfi.
 *      - awlfi must precede awlf.
 *      - scc and dcr3 must follow awlf, eventually.
 *
 * NB. traversal order:
 *
 *   The DUPdoDupNodeLutSsa call that renames CODE_BLOCK WITHID references
 *   has the side effect of cloning any lacfns that occur within those
 *   blocks. Those clones are not attached to the FUNDEF spine until
 *   this traversal is complete. Since the original lacfns may appear
 *   in the FUNDEF spine before OR after the current fundef, it is not
 *   possible to ensure that they have been traversed by this code.
 *
 *   Two possible solutions to this are:
 *
 *    a. Avoid the DUP call entirely. Rewrite this traversal to
 *       cover all code explicitly, and use AVIS_SUBST to do
 *       the renaming.
 *
 *    b. Traverse all lacfns as if they were inline, by traversing
 *       them when we encounter an N_ap that invokes them. This ensures
 *       that each lacfn is traversed before it is cloned.
 *
 *       This is the approach we have taken.
 *
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

#define DBUG_PREFIX "IVEXI"
#include "debug.h"

#include "traverse.h"
#include "free.h"
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"
#include "tree_utils.h"
#include "type_utils.h"
#include "shape.h"
#include "constants.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "pattern_match.h"
#include "LookUpTable.h"
#include "phase.h"
#include "globals.h"
#include "check_lib.h"
#include "wls.h"
#include "ivexpropagation.h"
#include "flattengenerators.h"
#include "algebraic_wlfi.h"
#include "symbolic_constant_simplification.h"
#include "lacfun_utilities.h"
#include "with_loop_utilities.h"

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
    node *lhs;
    bool fromap;
};

/**
 * INFO macros
 */
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASSIGNSPART(n) ((n)->preassignspart)
#define INFO_PREASSIGNSWITH(n) ((n)->preassignswith)
#define INFO_WITH(n) ((n)->with)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_LUTVARS(n) ((n)->lutvars)
#define INFO_LUTCODES(n) ((n)->lutcodes)
#define INFO_FROMAP(n) ((n)->fromap)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNSPART (result) = NULL;
    INFO_PREASSIGNSWITH (result) = NULL;
    INFO_WITH (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_LUTVARS (result) = NULL;
    INFO_LUTCODES (result) = NULL;
    INFO_FROMAP (result) = FALSE;

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

node *
IVEXIdoInsertIndexVectorExtrema (node *arg_node)
{
    info *arg_info;
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "Expected N_fundef");

    arg_info = MakeInfo ();
    INFO_LUTVARS (arg_info) = LUTgenerateLut ();
    INFO_LUTCODES (arg_info) = LUTgenerateLut ();

    DBUG_PRINT ("Traversing body of (%s) %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                FUNDEF_NAME (arg_node));

    TRAVpush (TR_ivexi);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    INFO_LUTVARS (arg_info) = LUTremoveLut (INFO_LUTVARS (arg_info));
    INFO_LUTCODES (arg_info) = LUTremoveLut (INFO_LUTCODES (arg_info));

    DBUG_PRINT ("Index vector extrema insertion complete.");

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
 * @fn bool IVEXIisExtremaActive( void)
 *
 *   @brief
 *
 *   @return TRUE is compiler is currently in a phase where extrema are valid.
 *
 *  NB. This probably belongs in globals somewhere, but I don't see a good spot
 *      for it just now.
 *
 ******************************************************************************/
bool
IVEXIisExtremaActive (void)
{
    bool z;

    DBUG_ENTER ();

    z = (global.compiler_anyphase >= PH_opt_saacyc_isaa)
        && (global.compiler_anyphase < PH_opt_esaa);

    DBUG_RETURN (z);
}

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
 *   @return N_avis node for fid.
 ******************************************************************************/
node *
IVEXImakeIntScalar (int k, node **vardecs, node **preassigns)
{
    node *favis;
    node *fass;

    DBUG_ENTER ();
    favis = TBmakeAvis (TRAVtmpVarName ("is"),
                        TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
    *vardecs = TBmakeVardec (favis, *vardecs);

    fass = TBmakeAssign (TBmakeLet (TBmakeIds (favis, NULL), TBmakeNum (k)), NULL);
    AVIS_SSAASSIGN (favis) = fass;
    *preassigns = TCappendAssign (*preassigns, fass);

    DBUG_RETURN (favis);
}

/** <!--********************************************************************-->
 *
 * @fn static bool isSameTypeShape( node *ida, node *idb)
 *
 * @brief predicate to see if two N_id nodes have same
 *        type, shape, and rank, ignoring AKS vs. AKV differences.
 *        If ida or idb is NULL, return TRUE.
 *
 *****************************************************************************/
static bool
isSameTypeShape (node *ida, node *idb)
{
    bool z = TRUE;
    ntype *typa;
    ntype *typb;

    DBUG_ENTER ();

    if ((NULL != ida) && (NULL != idb)) {
        typa = TYeliminateAKV (AVIS_TYPE (ID_AVIS (ida)));
        typb = TYeliminateAKV (AVIS_TYPE (ID_AVIS (idb)));

        z = TYeqTypes (typa, typb);

        typa = TYfreeType (typa);
        typb = TYfreeType (typb);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXIattachExtrema( node *extremum, node *id,
 *                                      node **vardecs, node **preassigns,
 *                                      prf nprf)
 *
 * @brief:
 *      Insert a temp, its vardec, and an assign for id,
 *      along with the extremum value (min/max), using the
 *      guard function nprf, F_noteminval, or F_notemaxval
 *
 *      We start with an N_avis, ivavis:
 *
 *           iv
 *
 *      and want to end up with:
 *
 *           iv' = _noteminval(iv, extremum);
 *
 *      and an appropriate vardec for iv', of course, such as:
 *
 *           int[.] iv';
 *
 * @params:
 *     extremum: the N_avis of an AVIS_MIN/AVIS_MAX to be attached to iv'.
 *     ivavis: The N_avis of iv, the name for which we want to build iv'.
 *     preassigns: The address of an INFO_PREASSIGNS( arg_info) node
 *                 in the caller's environment.
 *     vardecs:    The address of an INFO_VARDECS( arg_info) node
 *                 in the caller's environment.
 *     nprf: FIXME
 *
 *
 * @return: The N_avis of the new iv'.
 *
 *****************************************************************************/
node *
IVEXIattachExtrema (node *extremum, node *ivavis, node **vardecs, node **preassigns,
                    prf nprf)

{
    node *nas;
    node *args;
    node *lhsavis;
    node *prf;
    node *ivid;
    node *extid;

    DBUG_ENTER ();

    DBUG_ASSERT (N_avis == NODE_TYPE (ivavis), "Expected N_avis for ivavis");
    DBUG_ASSERT (N_avis == NODE_TYPE (extremum), "Expected N_avis for extremum");

    ivid = TBmakeId (ivavis);
    extid = TBmakeId (extremum);
    /* without this check, length errors between x and extrema
     * break far from here, in a very confusing manner. */
    if (!isSameTypeShape (ivid, extid)) {
        DBUG_PRINT ("WARNING: type/shape mismatch: id=%s, extremum=%s",
                    AVIS_NAME (ID_AVIS (ivid)), AVIS_NAME (extremum));
    }

    lhsavis = TBmakeAvis (TRAVtmpVarName ("ext"), TYeliminateAKV (AVIS_TYPE (ivavis)));
    *vardecs = TBmakeVardec (lhsavis, *vardecs);

    args = TBmakeExprs (ivid, TBmakeExprs (extid, NULL));
    prf = TBmakePrf (nprf, args);
    nas = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL), prf), NULL);
    AVIS_SSAASSIGN (lhsavis) = nas;

    *preassigns = TCappendAssign (*preassigns, nas);

    if ((F_noteminval == nprf)) {
        IVEXPsetMinvalIfNotNull (lhsavis, extremum);
    }

    if ((F_notemaxval == nprf)) {
        IVEXPsetMaxvalIfNotNull (lhsavis, extremum);
    }

    DBUG_PRINT ("Introduced temp index variable: %s for: %s", AVIS_NAME (lhsavis),
                AVIS_NAME (ivavis));
    global.optcounters.ivexp_expr++;

    DBUG_RETURN (lhsavis);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXItmpVec( node *arg_node, info *arg_info, node* ivavis))
 *
 * @brief:
 *      Insert a temp, its vardec, and an assign for ivavis.
 *      We start with an WITHID_VEC:
 *
 *           (lb <= iv=[i,j,k...] < ub)
 *
 *      and want:
 *
 *           iv'  =  _noteminval( iv, GENERATOR_BOUND1( partn));
 *           iv'' =  _notemaxval( iv, GENERATOR_BOUND2( partn));
 *
 *      and appropriate vardecs for iv' and iv'', of course.
 *
 *      We have to build an N_id for iv because WL WITHIDs don't have them,
 *      just N_avis nodes.
 *      . If -ssaiv becomes a reality, this
 *      can be scrapped, and we can attach the extrema to iv
 *      directly.
 *
 * @params:
 *     ivavis: The N_avis of the name for which we want to build iv'.
 *     arg_node: An N_part of the WL.
 *     arg_info: Your basic arg_info stuff.
 *
 * @return: The N_avis of the new temp, iv''.
 *
 *****************************************************************************/
node *
IVEXItmpVec (node *arg_node, info *arg_info, node *ivavis)
{
    node *avisp;
    node *avispp;
    node *b1;
    node *b2;

    DBUG_ENTER ();

    DBUG_ASSERT (N_avis == NODE_TYPE (ivavis), "Expected N_avis");
    b1 = GENERATOR_BOUND1 (PART_GENERATOR (arg_node));
    b1 = WLSflattenBound (DUPdoDupNode (b1), &INFO_VARDECS (arg_info),
                          &INFO_PREASSIGNSWITH (arg_info));
    b2 = GENERATOR_BOUND2 (PART_GENERATOR (arg_node));
    b2 = WLSflattenBound (DUPdoDupNode (b2), &INFO_VARDECS (arg_info),
                          &INFO_PREASSIGNSWITH (arg_info));
    avisp = IVEXIattachExtrema (b1, ivavis, &INFO_VARDECS (arg_info),
                                &INFO_PREASSIGNSPART (arg_info), F_noteminval);

    avispp = IVEXIattachExtrema (b2, avisp, &INFO_VARDECS (arg_info),
                                 &INFO_PREASSIGNSPART (arg_info), F_notemaxval);

    DBUG_RETURN (avispp);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXItmpIds( node *arg_node, info *arg_info, node* oldavis, size_t k)
 *
 * @brief:  Same as IVEXItmpVec, except this one handles one
 *          of the WITHID_IDS scalar variables i,j,k in
 *            IV = [i,j,k]
 *
 *          This is slightly more complex, because we have to
 *          select the appropriate generator elements.
 *
 *     iavis: The N_avis of the WITHID_IDS node for which we want to build iv'.
 *     arg_node: An N_part of the WL.
 *     arg_info: Your basic arg_info stuff.
 *     k:        the index of the scalar in IV, e.g., i=0, j=1, k=2
 * @return: The N_avis of the new temp.
 *
 *****************************************************************************/
node *
IVEXItmpIds (node *curpart, node *iavis, size_t k, node **preassignspart, node **vardecs)
{
    node *avisp;
    node *avispp;
    node *b1;
    node *b2;

    DBUG_ENTER ();

    DBUG_PRINT ("Working on %s", AVIS_NAME (iavis));

    b1 = GENERATOR_BOUND1 (PART_GENERATOR (curpart));
    b1 = WLUTfindArrayForBound (b1);
    b1 = TCgetNthExprsExpr (k, ARRAY_AELEMS (b1));
    b1 = FLATGexpression2Avis (DUPdoDupNode (b1), vardecs, preassignspart,
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

    b2 = GENERATOR_BOUND2 (PART_GENERATOR (curpart));
    b2 = WLUTfindArrayForBound (b2);
    b2 = TCgetNthExprsExpr (k, ARRAY_AELEMS (b2));
    b2 = FLATGexpression2Avis (DUPdoDupNode (b2), vardecs, preassignspart,
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

    avisp = IVEXIattachExtrema (b1, iavis, vardecs, preassignspart, F_noteminval);

    avispp = IVEXIattachExtrema (b2, avisp, vardecs, preassignspart, F_notemaxval);

    DBUG_PRINT ("Introduced: %s and %s for: %s", AVIS_NAME (avisp), AVIS_NAME (avispp),
                AVIS_NAME (iavis));

    DBUG_RETURN (avispp);
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
    size_t k = 0;

    DBUG_ENTER ();

    /* Populate LUTVARS with WITHID_VEC and new name. */
    oldavis = IDS_AVIS (WITHID_VEC (PART_WITHID (arg_node)));
    navis = IVEXItmpVec (arg_node, arg_info, oldavis);
    LUTinsertIntoLutP (INFO_LUTVARS (arg_info), oldavis, navis);
    DBUG_PRINT ("Inserting WITHID_VEC into lut: oldid: %s, newid: %s",
                AVIS_NAME (oldavis), AVIS_NAME (navis));

    /* Rename withid scalars */
    ids = WITHID_IDS (PART_WITHID (arg_node));
    while (ids != NULL) {
        oldavis = IDS_AVIS (ids);
        navis = IVEXItmpIds (arg_node, oldavis, k, &INFO_PREASSIGNSPART (arg_info),
                             &INFO_VARDECS (arg_info));
        DBUG_PRINT ("Inserting WITHID_IDS into lut: oldid: %s, newid: %s",
                    AVIS_NAME (oldavis), AVIS_NAME (navis));
        LUTinsertIntoLutP (INFO_LUTVARS (arg_info), oldavis, navis);
        ids = IDS_NEXT (ids);
        k++;
    }
    DBUG_RETURN ();
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
    DBUG_ENTER ();

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

    DBUG_ENTER ();
    DBUG_PRINT ("IVEXI in %s %s begins",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

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
        BLOCK_VARDECS (FUNDEF_BODY (arg_node))
          = TCappendVardec (INFO_VARDECS (arg_info),
                            BLOCK_VARDECS (FUNDEF_BODY (arg_node)));
        INFO_VARDECS (arg_info) = NULL;
    }

    DBUG_PRINT ("IVEXI in %s %s ends",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

    arg_info = FreeInfo (arg_info);
    INFO_FROMAP (old_info) = FALSE;

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
 *
 ******************************************************************************/
node *
IVEXIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Traversing block");
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

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
    node *lastwith;

    DBUG_ENTER ();

    DBUG_PRINT ("Traversing with");

    lastwith = INFO_WITH (arg_info);
    INFO_WITH (arg_info) = arg_node;

    /* Traverse the partitions, to define new temps. */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = WLUTremoveUnusedCodes (WITH_CODE (arg_node));

    INFO_WITH (arg_info) = lastwith;

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
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = NULL;

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

    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    if ((N_let == NODE_TYPE (ASSIGN_STMT (arg_node)))
        && (N_with == NODE_TYPE (LET_EXPR (ASSIGN_STMT (arg_node))))) {
        old_info = arg_info;
        arg_info = MakeInfo ();
        INFO_FUNDEF (arg_info) = INFO_FUNDEF (old_info);
        INFO_VARDECS (arg_info) = INFO_VARDECS (old_info);
        INFO_LUTVARS (arg_info) = INFO_LUTVARS (old_info);
        INFO_LUTCODES (arg_info) = INFO_LUTCODES (old_info);
    }

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /* Handle any preassigns generated with this WL.
     * If not in a WL, just put the assigns above this assign
     */
    if ((NULL == INFO_WITH (arg_info))
        || ((N_let == NODE_TYPE (ASSIGN_STMT (arg_node)))
            && (N_with == NODE_TYPE (LET_EXPR (ASSIGN_STMT (arg_node)))))) {

        if ((NULL != INFO_PREASSIGNSWITH (arg_info))) {
            DBUG_PRINT ("Prepending PREASSIGNSWITH");
            arg_node = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), arg_node);
            INFO_PREASSIGNSWITH (arg_info) = NULL;
        }
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
 *   Rename ALL LHS values within the code block. This is mandatory,
 *   because they may have extrema associated with them, and those
 *   must not be shared across partitions.
 *   If
 *
 *   Set AVIS_MIN, AVIS_MAX from the partition
 *   generator bounds.
 *
 *   We fix all the code blocks here, and chain
 *   together for IVEXIwith.
 *
 *   When this phase completes, all sharing of WITH_CODE blocks is lost.
 *   Sorry...
 *
 ******************************************************************************/
node *
IVEXIpart (node *arg_node, info *arg_info)
{
    node *newcode;

    DBUG_ENTER ();

    DBUG_PRINT ("Traversing part");
    /* We will deal with our partition after looking inside it */
    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);

    /* Don't try this on empty code blocks, kids. */
    /* Or default partitions, either! */
    if ((NULL != BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (arg_node))))
        && (N_default != NODE_TYPE (PART_GENERATOR (arg_node)))
        && (!WLUTisEmptyGenerator (arg_node))
        && (!CODE_HASEXTREMA (PART_CODE (arg_node)))) {

        populateLUTVars (arg_node, arg_info);

        /* If this partition is not shared, we must not rename
         * LHS values within the partition. This is because some
         * of the LHS values may have AVIS son nodes whose values
         * came from within this partition. If we rename them,
         * those sons become orphans, causing mayhem in VP following
         * a DCR.
         *
         * On the other hand,
         * if the partition IS shared, we MUST rename all the LHS
         * values within the partition, because some of them will
         * eventually have extrema attached, and those extrema may
         * vary across partitions.
         */

        /* FIXME: Notwithstanding the above comment, we end up
         * with transpose.sac generating huge numbers of partitions,
         * because AVIS_SSAASSIGN ends up pointing at the arg_node
         * copy, ont the newcode copy, of the code block.
         * At the time of DUP(), they point properly to newcode,
         * but by end-of-phase, they points to the unrenamed
         * arg_node. Weird....  2010-08-25
         */
        if (FALSE && (1 == CODE_USED (PART_CODE (arg_node)))) { /* FIXME */
            newcode = DUPdoDupNodeLut (PART_CODE (arg_node), INFO_LUTVARS (arg_info));
            /* At this point, AVIS_SSAASSIGN nodes are
             * now wrong in the old code block. So, we will
             * delete the old code block later on in this
             * traversal.
             */
        } else {
            newcode = DUPdoDupNodeLutSsa (PART_CODE (arg_node), INFO_LUTVARS (arg_info),
                                          INFO_FUNDEF (arg_info));
        }
        (CODE_USED (PART_CODE (arg_node)))--;
        CODE_USED (newcode) = 1;
        PART_CODE (arg_node) = newcode;
        LUTremoveContentLut (INFO_LUTVARS (arg_info));

        /* Link new code block to WLs chain */
        CODE_NEXT (newcode) = WITH_CODE (INFO_WITH (arg_info));
        WITH_CODE (INFO_WITH (arg_info)) = newcode;

        if (NULL != INFO_PREASSIGNSPART (arg_info)) {
            BLOCK_ASSIGNS (CODE_CBLOCK (newcode))
              = TCappendAssign (INFO_PREASSIGNSPART (arg_info),
                                BLOCK_ASSIGNS (CODE_CBLOCK (newcode)));
            INFO_PREASSIGNSPART (arg_info) = NULL;
        }
        CODE_HASEXTREMA (PART_CODE (arg_node)) = TRUE;
    }

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIprf(node *arg_node, info *arg_info)
 *
 * description:
 *   Insert extrema for primitives.
 *
 ******************************************************************************/
node *
IVEXIprf (node *arg_node, info *arg_info)
{
    node *minv = NULL;
    node *lhsavis;
    ntype *typ;
    constant *con;
    shape *shp;

    DBUG_ENTER ();

    DBUG_PRINT ("Traversing prf");

    switch (PRF_PRF (arg_node)) {
    default:
        break;

    case F_shape_A:
        /* If shape() result is AKD or AKS, we can establish a minval */
        lhsavis = IDS_AVIS (INFO_LHS (arg_info));

        if ((!IVEXPisAvisHasMin (lhsavis)) && (!AVIS_ISMINHANDLED (lhsavis))
            && (TUshapeKnown (AVIS_TYPE (lhsavis)))) {

            shp = SHcopyShape (TYgetShape (AVIS_TYPE (lhsavis)));
            con = COmakeZero (T_int, shp);
            if (NULL != con) {
                typ = TYmakeAKV (TYmakeSimpleType (T_int), con);
                minv = COconstant2AST (con);
                minv = FLATGexpression2Avis (minv, &INFO_VARDECS (arg_info),
                                             &INFO_PREASSIGNSWITH (arg_info), typ);
                IVEXPsetMinvalIfNotNull (IDS_AVIS (INFO_LHS (arg_info)), minv);
            }
        }
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
    DBUG_ENTER ();

    DBUG_PRINT ("Traversing cond");
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
    DBUG_ENTER ();

    DBUG_PRINT ("Traversing funcond");
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
    DBUG_ENTER ();

    DBUG_PRINT ("Traversing while");
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
    DBUG_ENTER ();

    DBUG_PRINT ("Traversing ap");
    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("Non-recursive call of %s from %s",
                    FUNDEF_NAME (AP_FUNDEF (arg_node)),
                    FUNDEF_NAME (INFO_FUNDEF (arg_info)));
        INFO_FROMAP (arg_info) = TRUE;
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        INFO_FROMAP (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 *  The following functions are kludges that we require because of
 *  the duplication of WITHID_VEC and WITHID_IDS names across
 *  WL N_part nodes. This duplication prevents us from associating
 *  extrema with those names. Hence, we have to copy the WITHID_IDS/VEC
 *  to a new name, and associate the extrema with those new names.
 *
 *
 * function:
 *   node *IVEXIwithidsKludge(int offset, node *withidvec, node *curwith,
 *                            node **preassigns, node **vardecs)
 *
 *
 * description: Perform z =  withidvec[ offset];
 *                      z = attachextrema( z);
 *
 *              Return the N_avis of z, or NULL.
 *
 ******************************************************************************/
node *
IVEXIwithidsKludge (size_t offset, node *withidids, node *curpart, node **preassignspart,
                    node **vardecs)
{
    node *z = NULL;
    node *ijk;
    bool isIdsMember;

    DBUG_ENTER ();

    ijk = TCgetNthExprsExpr (offset, ARRAY_AELEMS (withidids));
    if (NULL != curpart) {
        offset = LFUindexOfMemberIds (ID_AVIS (ijk), WITHID_IDS (PART_WITHID (curpart)), &isIdsMember);
        if (isIdsMember) {
            z = TCgetNthIds (offset, WITHID_IDS (PART_WITHID (curpart)));
            if (IVEXIisExtremaActive ()) {
                z = IVEXItmpIds (curpart, z, offset, preassignspart, vardecs);
            }
        }
    }

    DBUG_RETURN (z);
}

#undef DBUG_PREFIX
