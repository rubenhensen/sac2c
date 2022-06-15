/** <!--********************************************************************-->
 *
 * @defgroup pwlf Polyhedral With-Loop Folding
 *
 * @terminology:
 *
 * @brief Polyhedral With-Loop Folding (PWLF)
 *
 * @ingroup opt
 *
 * @{
 *
 * This code likes to run AFTER prfunr, because its operation depends
 * on the availability of scalars for affine function tree creation.
 * Otherwise, we lose an entire SAACYC cycle.
 *
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file polyhedral_wlf.c
 *
 * Prefix: PWLF
 *
 *****************************************************************************/
#include "polyhedral_wlf.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"

#define DBUG_PREFIX "PWLF"
#include "debug.h"

#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "compare_tree.h"
#include "DupTree.h"
#include "free.h"
#include "globals.h"
#include "wl_cost_check.h"
#include "wl_needcount.h"
#include "inferneedcounters.h"
#include "pattern_match.h"
#include "constants.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "tree_utils.h"
#include "check.h"
#include "ivextrema.h"
#include "phase.h"
#include "namespaces.h"
#include "wls.h"
#include "SSAWithloopFolding.h"
#include "new_typecheck.h"
#include "ivexpropagation.h"
#include "string.h"
#include "constant_folding.h"
#include "variable_propagation.h"
#include "reorder_equalityprf_arguments.h"
#include "transform_gtge_to_ltle.h"
#include "ElimSubDiv.h"
#include "UndoElimSubDiv.h"
#include "arithmetic_simplification.h"
#include "associative_law.h"
#include "distributive_law.h"
#include "UndoElimSubDiv.h"
#include "inlining.h"
#include "elim_alpha_types.h"
#include "elim_bottom_types.h"
#include "insert_symb_arrayattr.h"
#include "dispatchfuncalls.h"
#include "SSACSE.h"
#include "loop_invariant_removal.h"
#include "withloop_invariant_removal.h"
#include "cubeslicer.h"
#include "prfunroll.h"
#include "flattengenerators.h"
#include "indexvectorutils.h"
#include "deadcoderemoval.h"
#include "with_loop_utilities.h"
#include "set_withloop_depth.h"
#include "symbolic_constant_simplification.h"
#include "polyhedral_utilities.h"
#include "isl_utilities.h"
#include "polyhedral_defs.h"
#include "algebraic_wlfi.h"
#include "polyhedral_wlf.h"
#include "polyhedral_setup.h"
#include "deserialize.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *vardecs;
    node *preassigns;        /* These go above current statement */
    node *preassignswl;      /* These go above the consumerWL */
    node *consumerwlpart;    /* The current consumerWL partition */
    node *consumerwl;        /* The current consumerWL N_with */
    node *consumerwlids;     /* The current consumerWL N_ids */
    node *producerwl;        /* The producerWL N_with for this PWL */
    node *producerwllhs;     /* The producerWL LHS for this consumerWL */
    node *let;               /* The N_let node */
    node *withids;           /* the WITHID_IDS entry for an iv element */
    node *zwithids;          /* zwithids for GENERATOR_BOUNDs */
    lut_t *foldlut;          /* LUT for renames during fold */
    lut_t *varlut;           /* LUT for ISL set variables */
    node *lacfun;            /* Marker that this is a LACFUN call */
    node *nassign;           /* The N_assign node of a LACFUN N_ap call */
    int defdepth;            /* The current nesting level of WLs. This
                              * is used to ensure that an index expression
                              * refers to an earlier WL in the same code
                              * block, rather than to a WL within this
                              * WL.
                              */
    bool producerwlfoldable; /* producerWL may be legally foldable. */
                             /* (If index sets prove to be OK)     */
    bool finverseswap;       /* If TRUE, must swp min/max */
    bool finverseintroduced; /* If TRUE, most simplify F-inverse */
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_PREASSIGNSWL(n) ((n)->preassignswl)
#define INFO_CONSUMERWLPART(n) ((n)->consumerwlpart)
#define INFO_CONSUMERWL(n) ((n)->consumerwl)
#define INFO_CONSUMERWLIDS(n) ((n)->consumerwlids)
#define INFO_PRODUCERWL(n) ((n)->producerwl)
#define INFO_PRODUCERWLLHS(n) ((n)->producerwllhs)
#define INFO_LET(n) ((n)->let)
#define INFO_WITHIDS(n) ((n)->withids)
#define INFO_ZWITHIDS(n) ((n)->zwithids)
#define INFO_FOLDLUT(n) ((n)->foldlut)
#define INFO_VARLUT(n) ((n)->varlut)
#define INFO_LACFUN(n) ((n)->lacfun)
#define INFO_NASSIGN(n) ((n)->nassign)
#define INFO_DEFDEPTH(n) ((n)->defdepth)
#define INFO_PRODUCERWLFOLDABLE(n) ((n)->producerwlfoldable)
#define INFO_FINVERSESWAP(n) ((n)->finverseswap)
#define INFO_FINVERSEINTRODUCED(n) ((n)->finverseintroduced)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_PREASSIGNSWL (result) = NULL;
    INFO_CONSUMERWLPART (result) = NULL;
    INFO_CONSUMERWL (result) = NULL;
    INFO_CONSUMERWLIDS (result) = NULL;
    INFO_PRODUCERWL (result) = NULL;
    INFO_PRODUCERWLLHS (result) = NULL;
    INFO_LET (result) = NULL;
    INFO_WITHIDS (result) = NULL;
    INFO_ZWITHIDS (result) = NULL;
    INFO_FOLDLUT (result) = NULL;
    INFO_VARLUT (result) = NULL;
    INFO_LACFUN (result) = NULL;
    INFO_NASSIGN (result) = NULL;
    INFO_DEFDEPTH (result) = 0;
    INFO_PRODUCERWLFOLDABLE (result) = TRUE;
    INFO_FINVERSESWAP (result) = FALSE;
    INFO_FINVERSEINTRODUCED (result) = FALSE;

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

/******************************************************************************
 *
 * function:
 *   node *PWLFdoPolyhedralWithLoopFolding(node *arg_node)
 *
 * @brief Global entry point of Polyhedral With-Loop folding
 *        Applies Polyhedral WL folding to a fundef.
 *
 *****************************************************************************/
node *
PWLFdoPolyhedralWithLoopFolding (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "Called for non-fundef node");

    arg_info = MakeInfo (arg_node);
    INFO_FOLDLUT (arg_info) = LUTgenerateLut ();
    INFO_VARLUT (arg_info) = LUTgenerateLut ();

    TRAVpush (TR_pwlf);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    INFO_FOLDLUT (arg_info) = LUTremoveLut (INFO_FOLDLUT (arg_info));
    INFO_VARLUT (arg_info) = LUTremoveLut (INFO_VARLUT (arg_info));
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn static node *populateFoldLut( node *arg_node, info *arg_info, shape *shp)
 *
 * @brief Generate a clone name for a PWL WITHID.
 *        Populate one element of a look up table with
 *        said name and its original, which we will use
 *        to do renames in the copied PWL code block.
 *        See caller for description. Basically,
 *        we have a PWL with generator of this form:
 *
 *    PWL = with {
 *         ( . <= iv=[i,j] <= .) : _sel_VxA_( iv, AAA);
 *
 *        We want to perform renames in the PWL code block as follows:
 *
 *        iv --> iv'
 *        i  --> i'
 *        j  --> j'
 *
 * @param: arg_node: one N_avis node of the PWL generator (e.g., iv),
 *                   to serve as iv for above assigns.
 *         arg_info: your basic arg_info.
 *         shp:      the shape descriptor of the new LHS.
 *
 * @result: New N_avis node, e.g, iv'.
 *          Side effect: mapping iv -> iv' entry is now in LUT.
 *                       New vardec for iv'.
 *
 *****************************************************************************/
static node *
populateFoldLut (node *arg_node, info *arg_info, shape *shp)
{
    node *navis;

    DBUG_ENTER ();

    /* Generate a new LHS name for WITHID_VEC/IDS */
    navis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (arg_node)),
                        TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (arg_node))), shp));
    INFO_VARDECS (arg_info) = TBmakeVardec (navis, INFO_VARDECS (arg_info));
    LUTinsertIntoLutP (INFO_FOLDLUT (arg_info), arg_node, navis);
    DBUG_PRINT ("Inserted WITHID_VEC into lut: oldname: %s, newname %s",
                AVIS_NAME (arg_node), AVIS_NAME (navis));
    DBUG_RETURN (navis);
}

/** <!--********************************************************************-->
 *
 * @ fn static node *makeIdxAssigns( node *arg_node, node *cwlpart, node *pwlpart)
 *
 * @brief This function does setup for renaming PWL index vector elements
 *        to correspond to their CWL brethren.
 *
 *        For a PWL partition, with generator:
 *
 *        (. <= iv=[i,j] < .)
 *
 *        and a consumer _sel_VxA_( idx, PWL),
 *
 *        we generate an N_assign chain of this form:
 *
 *        iv = idx;
 *        k0 = [0];
 *        i  = _sel_VxA_( k0, idx);
 *        k1 = [1];
 *        j  = _sel_VxA_( k1, idx);
 *
 *        Also, iv, i, j, k are placed into the LUT.
 *
 * @params arg_node: An N_assign node.
 *
 * @result: an N_assign chain as above.
 *
 *****************************************************************************/
static node *
makeIdxAssigns (node *arg_node, info *arg_info, node *cwlpart, node *pwlpart)
{
    node *z = NULL;
    node *ids;
    node *narray;
    node *idxavis;
    node *navis;
    node *nass;
    node *lhsids;
    node *lhsavis;
    node *sel;
    int k;

    DBUG_ENTER ();
    ids = WITHID_IDS (PART_WITHID (pwlpart));
    idxavis = IVUToffset2Vect (arg_node, &INFO_VARDECS (arg_info),
                               &INFO_PREASSIGNS (arg_info), cwlpart, pwlpart);
    DBUG_ASSERT (NULL != idxavis, "Could not rebuild iv for _sel_VxA_(iv, PWL)");

    k = 0;

    while (NULL != ids) {
        /* Build k0 = [k]; */
        /* First, the k */
        narray = TCmakeIntVector (TBmakeExprs (TBmakeNum (k), NULL));
        navis = TBmakeAvis (TRAVtmpVar (),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, 1)));

        nass = TBmakeAssign (TBmakeLet (TBmakeIds (navis, NULL), narray), NULL);
        AVIS_SSAASSIGN (navis) = nass;
        z = TCappendAssign (nass, z);
        INFO_VARDECS (arg_info) = TBmakeVardec (navis, INFO_VARDECS (arg_info));

        lhsavis = populateFoldLut (IDS_AVIS (ids), arg_info, SHcreateShape (0));
        DBUG_PRINT ("created %s = _sel_VxA_(%d, %s)", AVIS_NAME (lhsavis), k,
                    AVIS_NAME (idxavis));

        sel = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL),
                                       TCmakePrf2 (F_sel_VxA, TBmakeId (navis),
                                                   TBmakeId (idxavis))),
                            NULL);
        z = TCappendAssign (z, sel);
        AVIS_SSAASSIGN (lhsavis) = sel;
        ids = IDS_NEXT (ids);
        k++;
    }

    /* Now generate iv = idx; */
    lhsids = WITHID_VEC (PART_WITHID (pwlpart));
    lhsavis = populateFoldLut (IDS_AVIS (lhsids), arg_info, SHcreateShape (1, k));
    z = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL), TBmakeId (idxavis)), z);
    AVIS_SSAASSIGN (lhsavis) = z;
    DBUG_PRINT ("makeIdxAssigns created %s = %s)", AVIS_NAME (lhsavis),
                AVIS_NAME (idxavis));
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *PWLFperformFold( ... )
 *
 * @brief Generate preassign code for producerWL block being
 *        folded by the arg_node sel().
 *
 *    pwl = with...  elb = _sel_VxA_(iv=[i,j], AAA) ...
 *    consumerWL = with...  elc = _sel_VxA_(idx, pwl) ...
 *
 *   We generate:
 *     iv = idx;
 *     i = _sel_VxA_([0], idx);
 *     j = _sel_VxA_([1], idx);
 *     tmp = {code block from pwl, with SSA renames}
 *     z = tmp;
 *
 * @params
 *    arg_node: the _sel_(iv, pwl) N_prf
 *    pwlpart: pwl N_part
 *
 * @result: new rhs for sel. As side effect, append the
 *          generated assigns to the preassigns list for the sel().
 *
 *****************************************************************************/
static node *
PWLFperformFold (node *arg_node, node *pwlpart, info *arg_info)
{
    node *pwlblock;
    node *cwlpart;
    node *cellexpr;
    node *newblock = NULL;
    node *idxassigns;
    node *newsel = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("Replacing code block in CWL=%s",
                AVIS_NAME (IDS_AVIS (LET_IDS (INFO_LET (arg_info)))));
    cwlpart = INFO_CONSUMERWLPART (arg_info);
    idxassigns = makeIdxAssigns (arg_node, arg_info, cwlpart, pwlpart);
    INFO_PREASSIGNS (arg_info) = TCappendAssign (INFO_PREASSIGNS (arg_info), idxassigns);
    cellexpr = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (PART_CODE (pwlpart))));
    pwlblock = BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (pwlpart)));

    if (NULL != pwlblock) {
        // If PWL code block is empty, don't duplicate code block.
        // If the cell is non-scalar, we still need a _sel_() to pick
        // the proper cell element.
        newblock = DUPdoDupTreeLutSsa (pwlblock, INFO_FOLDLUT (arg_info),
                                       INFO_FUNDEF (arg_info));
        INFO_PREASSIGNS (arg_info)
          = TCappendAssign (INFO_PREASSIGNS (arg_info), newblock);
    }

    cellexpr = (node *)LUTsearchInLutPp (INFO_FOLDLUT (arg_info), cellexpr);
    newsel = TBmakeId (cellexpr);
    LUTremoveContentLut (INFO_FOLDLUT (arg_info));

    DBUG_RETURN (newsel);
}

/** <!--********************************************************************-->
 *
 * @fn node *BuildInverseProjectionScalar(...)
 *
 * @brief Chase one element of an N_array back to its WITHID_IDS,
 *        if possible.
 *
 * @params:  iprime: The current expression we are tracing,
 *           An N_id or an N_num.
 *           Or, it can be the N_avis of a WITHID_IDS node.
 *
 *           arg_info: Your basic arg_info node.
 *
 *           lbub[ ivindx]: The current inverse projection.
 *                This is normalized, a la WL bounds.
 *                If this is a recursive call, lbub is scalar, and
 *                is an N_avis.
 *
 * @result: An N_avis node that gives the result of the F-inverse mapping
 *          function to take us from iv'->iv, or
 *          a N_num or else NULL, if no such node can be found.
 *
 *          Side effect: Set INFO_WITHIDS, if possible.
 *
 *****************************************************************************/
static node *
FlattenLbubel (node *lbub, size_t ivindx, info *arg_info)
{
    node *lbubelavis;
    node *lbubel;

    DBUG_ENTER ();

    if (N_avis == NODE_TYPE (lbub)) {
        lbubelavis = lbub;
    } else {
        lbubel = TCgetNthExprsExpr (ivindx, ARRAY_AELEMS (lbub));
        if (N_num == NODE_TYPE (lbubel)) {
            lbubelavis
              = FLATGexpression2Avis (DUPdoDupTree (lbubel), &INFO_VARDECS (arg_info),
                                      &INFO_PREASSIGNS (arg_info), NULL);
        } else {
            lbubelavis = ID_AVIS (lbubel);
        }
    }

    DBUG_RETURN (lbubelavis);
}

static node *
BuildInverseProjectionScalar (node *iprime, info *arg_info, node *lbub, size_t ivindx)
{
    node *z = NULL;
    int markiv;
    node *xarg;
    node *ivarg;
    node *id1;
    node *id2;
    node *resavis;
    node *ids;
    node *assgn;
    node *idx = NULL;
    node *withidids;
    node *ipavis;
    size_t tcindex;
    bool isIdsMember;
    prf nprf;

    pattern *pat;

    DBUG_ENTER ();

    DBUG_PRINT ("Building inverse projection scalar");
    INFO_WITHIDS (arg_info) = NULL;
    switch (NODE_TYPE (iprime)) {
    default:
        DBUG_UNREACHABLE ("unexpected iprime NODE_TYPE");
        break;

    case N_num:
        z = DUPdoDupNode (iprime);
        break;

    case N_avis: /* iprime is WITHID_IDS - self-inverse */
        z = iprime;
        break;

    case N_id:
        ipavis = ID_AVIS (iprime);
        DBUG_PRINT ("Tracing %s", AVIS_NAME (ipavis));
        pat = PMany (1, PMAgetNode (&idx), 0);

        /* If we want to find withids, we have to skip BOTH extrema
         * and guards. E.g., in twopartoffsetWLAKD.sac
         */
        if (PMmatchFlatSkipExtremaAndGuards (pat, iprime)) {
            withidids = WITHID_IDS (PART_WITHID (INFO_CONSUMERWLPART (arg_info)));

            switch (NODE_TYPE (idx)) {
            case N_id:
                tcindex = TClookupIdsNode (withidids, ID_AVIS (idx), &isIdsMember);
                if (isIdsMember) {
                    DBUG_PRINT ("Found %s as source of iv'=%s", AVIS_NAME (ID_AVIS (idx)),
                                AVIS_NAME (ipavis));
                    INFO_WITHIDS (arg_info) = TCgetNthIds (tcindex, withidids);
                    z = FlattenLbubel (lbub, ivindx, arg_info);
                } else {
                    /* Vanilla variable */
                    DBUG_PRINT ("We lost the trail.");
                    z = NULL;
                }
                break;

            case N_prf:
                switch (PRF_PRF (idx)) {
                case F_add_SxS:
                    /* iv' = ( iv + x);   -->  iv = ( iv' - x);
                     * iv' = ( x  + iv);  -->  iv = ( iv' - x);
                     */
                    markiv
                      = AWLFIfindPrfParent2 (idx, withidids, &INFO_WITHIDS (arg_info));
                    if (0 != markiv) {
                        ivarg = (2 == markiv) ? PRF_ARG2 (idx) : PRF_ARG1 (idx);
                        xarg = (2 == markiv) ? PRF_ARG1 (idx) : PRF_ARG2 (idx);
                        DBUG_ASSERT (N_id == NODE_TYPE (xarg), "Expected N_id xarg");
                        DBUG_ASSERT (N_id == NODE_TYPE (ivarg), "Expected N_id ivarg");
                        resavis = TBmakeAvis (TRAVtmpVarName ("tisadd"),
                                              TYmakeAKS (TYmakeSimpleType (T_int),
                                                         SHcreateShape (0)));
                        INFO_VARDECS (arg_info)
                          = TBmakeVardec (resavis, INFO_VARDECS (arg_info));
                        ids = TBmakeIds (resavis, NULL);
                        assgn = TBmakeAssign (
                          TBmakeLet (ids,
                                     TCmakePrf2 (F_sub_SxS,
                                                 TBmakeId (FlattenLbubel (lbub, ivindx,
                                                                          arg_info)),
                                                 TBmakeId (ID_AVIS (xarg)))),
                          NULL);
                        INFO_PREASSIGNS (arg_info)
                          = TCappendAssign (INFO_PREASSIGNS (arg_info), assgn);
                        AVIS_SSAASSIGN (resavis) = assgn;
                        z = BuildInverseProjectionScalar (ivarg, arg_info, resavis,
                                                          ivindx);
                    }
                    break;

                case F_sub_SxS:
                    /* Case 1: iv' = ( iv - x);   -->  iv = ( iv' + x);
                     * Case 2: iv' = ( x - iv);   -->  iv = ( x - iv');
                     *         Also, must swap minval/maxval.
                     */
                    resavis = TBmakeAvis (TRAVtmpVarName ("tissub"),
                                          TYmakeAKS (TYmakeSimpleType (T_int),
                                                     SHcreateShape (0)));
                    INFO_VARDECS (arg_info)
                      = TBmakeVardec (resavis, INFO_VARDECS (arg_info));
                    markiv
                      = AWLFIfindPrfParent2 (idx, withidids, &INFO_WITHIDS (arg_info));
                    if (0 != markiv) {
                        ivarg = (2 == markiv) ? PRF_ARG2 (idx) : PRF_ARG1 (idx);
                        xarg = (2 == markiv) ? PRF_ARG1 (idx) : PRF_ARG2 (idx);
                        DBUG_ASSERT (N_id == NODE_TYPE (xarg), "Expected N_id xarg");
                        DBUG_ASSERT (N_id == NODE_TYPE (ivarg), "Expected N_id ivarg");
                        switch (markiv) {
                        case 1:
                            nprf = F_add_SxS;
                            id1 = TBmakeId (FlattenLbubel (lbub, ivindx, arg_info));
                            id2 = TBmakeId (ID_AVIS (xarg));
                            break;

                        case 2:
                            nprf = F_sub_SxS;
                            id1 = TBmakeId (ID_AVIS (xarg));
                            id2 = TBmakeId (FlattenLbubel (lbub, ivindx, arg_info));
                            INFO_FINVERSESWAP (arg_info) = !INFO_FINVERSESWAP (arg_info);
                            break;

                        default:
                            nprf = F_add_SxS;
                            id1 = NULL;
                            id2 = NULL;
                            DBUG_UNREACHABLE ("ivarg confusion");
                        }

                        ids = TBmakeIds (resavis, NULL);
                        assgn
                          = TBmakeAssign (TBmakeLet (ids, TCmakePrf2 (nprf, id1, id2)),
                                          NULL);
                        INFO_PREASSIGNS (arg_info)
                          = TCappendAssign (INFO_PREASSIGNS (arg_info), assgn);
                        AVIS_SSAASSIGN (resavis) = assgn;
                        z = BuildInverseProjectionScalar (ivarg, arg_info, resavis,
                                                          ivindx);
                    }
                    break;

                case F_mul_SxS:
                    /* iv' = ( iv *  x);  -->  iv = ( iv' / x);
                     * iv' = ( x  * iv);  -->  iv = ( iv' / x);
                     */
                    markiv
                      = AWLFIfindPrfParent2 (idx, withidids, &INFO_WITHIDS (arg_info));
                    if (0 != markiv) {
                        ivarg = (2 == markiv) ? PRF_ARG2 (idx) : PRF_ARG1 (idx);
                        xarg = (2 == markiv) ? PRF_ARG1 (idx) : PRF_ARG2 (idx);
                        DBUG_ASSERT (N_id == NODE_TYPE (xarg), "Expected N_id xarg");
                        DBUG_ASSERT (N_id == NODE_TYPE (ivarg), "Expected N_id ivarg");
                        // Check for multiply by zero, just in case.
                        if (SCSisConstantZero (xarg)) {
                            DBUG_PRINT ("multiply by zero has no inverse");
                        } else {
                            resavis = TBmakeAvis (TRAVtmpVarName ("tismul"),
                                                  TYmakeAKS (TYmakeSimpleType (T_int),
                                                             SHcreateShape (0)));
                            INFO_VARDECS (arg_info)
                              = TBmakeVardec (resavis, INFO_VARDECS (arg_info));
                            ids = TBmakeIds (resavis, NULL);
                            assgn = TBmakeAssign (
                              TBmakeLet (ids, TCmakePrf2 (F_div_SxS,
                                                          TBmakeId (
                                                            FlattenLbubel (lbub, ivindx,
                                                                           arg_info)),
                                                          TBmakeId (ID_AVIS (xarg)))),
                              NULL);
                            INFO_PREASSIGNS (arg_info)
                              = TCappendAssign (INFO_PREASSIGNS (arg_info), assgn);
                            AVIS_SSAASSIGN (resavis) = assgn;
                            z = BuildInverseProjectionScalar (ivarg, arg_info, resavis,
                                                              ivindx);
                        }
                    }
                    break;

                default:
                    if (!PMMisInGuards (PRF_PRF (idx))) {
                        /* idx may be something like an _idx_sel() that
                         * will disappear soon, due to CF */
                        DBUG_PRINT ("N_prf not recognized");
                        break;
                    } else { /* Guard may get removed in later saacyc */
                        DBUG_PRINT ("Skipping guard N_prf");
                        z = NULL;
                        break;
                    }
                }
                break;

            case N_num:
                DBUG_PRINT ("Found integer as source of iv'=%s", AVIS_NAME (ipavis));
                z = ipavis;
                break;

            case N_array:
                DBUG_ASSERT (1 == SHgetUnrLen (ARRAY_FRAMESHAPE (idx)),
                             "Expected 1-element N_array");
                DBUG_UNREACHABLE ("We are confused");
                break;

            default:
                DBUG_UNREACHABLE ("Cannot chase iv'");
                break;
            }
        }
        pat = PMfree (pat);
        break;
    }

    DBUG_PRINT ("Finished building inverse projection scalar");
    DBUG_ASSERT ((NULL == z) || (N_avis == NODE_TYPE (z)) || (N_num == NODE_TYPE (z)),
                 "failed to gen inverse");
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *BuildAxisConfluence(...)
 *
 * @brief: Generate code to perform max/min on WL-intersection of
 *         two axes that are confluent. E.g., if the CWL is extracting
 *         the major diagonal from a 2-D PWL with two partitions,
 *         we may have something like:  _sel_VxA_( [i, i+1], PWL).
 *
 *         In that case, we take the maximum of the minimum bound
 *         and the minimum of the maximum bound.
 *
 * @params:
 *          idx: index into result of this element.
 *
 *          zarr: the N_array result we are overwriting.
 *
 *          zelnew: The inverse intersect, of the shape of the CWL bounds,
 *          as an element of the ARRAY_AELEMS N_exprs chain.
 *
 *          bndel: Current element of the generator bound.
 *                 This is used to determine whether to
 *                 overwrite current result element or if we need min/max.
 *
 *          boundnum: 0 if we are computing BOUND1,
 *                    1 if we are computing BOUND2
 *
 *          arg_info: your basic arg_info node.
 *
 * @result: If zarr[ idx] is not used yet:
 *
 *              zarr[ idx] = zelnew;
 *
 *         If zarr[idx] is used, there is confluence, so we have:
 *
 *           zarr[ idx] = Max( zarr[ idx], zelnew) if 0=boundnum
 *           zarr[ idx] = Min( zarr[ idx], zelnew) if 1=boundnum
 *
 *****************************************************************************/
static node *
BuildAxisConfluence (node *zarr, size_t idx, node *zelnew, node *bndel, int boundnum,
                     info *arg_info)
{

    node *zprime;
    node *zelcur;
    const char *fn;
    node *fncall;
    node *newavis;
    node *curavis;

    DBUG_ENTER ();

    zelcur = TCgetNthExprsExpr (idx, zarr);
    if (CMPT_EQ == CMPTdoCompareTree (zelcur, bndel)) { /* not used yet */
        zprime = TCputNthExprs (idx, zarr, TBmakeId (ID_AVIS (zelnew)));
    } else {
        if (CMPT_EQ == CMPTdoCompareTree (zelcur, zelnew)) { /* No change */
            zprime = zarr;
        } else { /* confluence */
            fn = (0 == boundnum) ? "partitionMax" : "partitionMin";
            newavis = AWLFIflattenScalarNode (zelnew, arg_info);
            curavis = AWLFIflattenScalarNode (zelcur, arg_info);
            fncall
              = DSdispatchFunCall (NSgetNamespace ("sacprelude"), fn,
                                   TCcreateExprsChainFromAvises (2, curavis, newavis));
            zprime = FLATGexpression2Avis (fncall, &INFO_VARDECS (arg_info),
                                           &INFO_PREASSIGNS (arg_info),
                                           TYmakeAKS (TYmakeSimpleType (T_int),
                                                      SHcreateShape (0)));
            zprime = TCputNthExprs (idx, zarr, TBmakeId (zprime));
        }
    }

    DBUG_RETURN (zprime);
}

/** <!--********************************************************************-->
 *
 * @fn node *PermuteIntersectElements( node *zelu, node *zwithids,
                                       info *arg_info, int boundnum)
 *
 * @brief: Permute and/or merge inverse intersection elements, to
 *         construct CUBSL argument.
 *
 * @params: zelu: an N_exprs chain of an intersect calculation
 *          Its length matches that of iv in the sel( iv, producerWL)
 *          in the consumerWL.
 *          These are in denormalized form.
 *
 *          zwithids: an N_ids chain, of the same shape as
 *          zelu, comprising the WITHID_IDS related to the
 *          corresponding element of zelu.
 *
 *          arg_info: your basic arg_info node.
 *
 *          boundnum: 0 if we are computing BOUND1,
 *                    1 if we are computing BOUND2
 *
 * @result: The permuted and/or confluenced N_avis for an
 *          N_exprs chain
 *          whose length matches that of the consumerWL GENERATOR_BOUND.
 *
 *          Effectively, this code performs, in the absence
 *          of duplicate zwithids entries:
 *
 *            zarr[ withidids iota zwithids] = zelu;
 *
 *          If there are duplicates, we insert min/max ops to handle
 *          the axis confluence
 *
 *          If there is no CWL, then we can not have any permutation,
 *          so the result is zelu.
 *
 *****************************************************************************/
static node *
PermuteIntersectElements (node *zelu, node *zwithids, info *arg_info, int boundnum)
{
    node *ids;
    size_t shpz;
    size_t shpids;
    size_t shpzelu;
    size_t i;
    size_t idx;
    bool isIdsMember;
    pattern *pat;
    node *bndarr = NULL;
    node *zarr;
    node *z;
    node *zelnew;
    size_t xrho = 0;
    node *bndel;
    ntype *typ;

    DBUG_ENTER ();

    if (NULL == INFO_CONSUMERWLPART (arg_info)) {
        xrho = TCcountExprs (zelu);
        typ = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0));
        z = TBmakeArray (typ, SHcreateShape (1, xrho), zelu);
        z = FLATGexpression2Avis (z, &INFO_VARDECS (arg_info),
                                  &INFO_PREASSIGNS (arg_info),
                                  TYmakeAKS (TYmakeSimpleType (T_int),
                                             SHcreateShape (1, xrho)));
    } else {
        z = PART_GENERATOR (INFO_CONSUMERWLPART (arg_info));
        if (0 == boundnum) {
            z = GENERATOR_BOUND1 (z);
        } else {
            z = GENERATOR_BOUND2 (z);
        }

        if (N_array == NODE_TYPE (z)) {
            xrho = SHgetUnrLen (ARRAY_FRAMESHAPE (z));
            z = FLATGexpression2Avis (DUPdoDupNode (z), &INFO_VARDECS (arg_info),
                                      &INFO_PREASSIGNS (arg_info),
                                      TYmakeAKS (TYmakeSimpleType (T_int),
                                                 SHcreateShape (1, xrho)));
        } else {
            z = ID_AVIS (z);
        }

        if (1 == boundnum) { /* Denormalize BOUND2 */
            z = IVEXPadjustExtremaBound (z, -1, &INFO_VARDECS (arg_info),
                                         &INFO_PREASSIGNS (arg_info), "pie");
        }

        z = TBmakeId (z);

        pat = PMarray (1, PMAgetNode (&bndarr), 1, PMskip (0));
        if (!PMmatchFlat (pat, z)) {
            DBUG_UNREACHABLE ("Expected N_array bounds");
        }
        DBUG_ASSERT (N_exprs == NODE_TYPE (zelu), "Expected N_exprs zelu");

        zarr = DUPdoDupTree (ARRAY_AELEMS (bndarr));

        shpz = TCcountExprs (zarr);
        ids = WITHID_IDS (PART_WITHID (INFO_CONSUMERWLPART (arg_info)));
        shpids = TCcountIds (ids);
        DBUG_ASSERT (shpz == shpids, "Wrong boundary intersect shape");
        shpzelu = TCcountExprs (zelu);

        for (i = 0; i < shpzelu; i++) {
            idx = TClookupIdsNode (ids, TCgetNthIds (i, zwithids), &isIdsMember);
            if (isIdsMember) { /* skip places where idx is a constant, etc. */
                             /* E.g., sel( [ JJ, 2], PWL);                */
                zelnew = TCgetNthExprsExpr (i, zelu);
                bndel = TCgetNthExprsExpr (idx, ARRAY_AELEMS (bndarr));
                zarr = BuildAxisConfluence (zarr, idx, zelnew, bndel, boundnum, arg_info);
            }
        }

        z = DUPdoDupNode (bndarr);
        FREEdoFreeTree (ARRAY_AELEMS (z));
        ARRAY_AELEMS (z) = zarr;
        z = FLATGexpression2Avis (z, &INFO_VARDECS (arg_info),
                                  &INFO_PREASSIGNS (arg_info),
                                  TYmakeAKS (TYmakeSimpleType (T_int),
                                             SHcreateShape (1, xrho)));

        pat = PMfree (pat);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *BuildInverseProjectionOne(...)
 *
 * @brief   For a consumerWL with iv as WITHID_IDS,
 *          we have code of the form:
 *
 *             iv' = F( iv);
 *             el = producerWL( iv');
 *
 *         We are given iv' and lbub as the WL intersection
 *         between a partition of the producerWL and the consumerWL.
 *
 *         This function generates code to compute F_1,
 *         the inverse of F, and applies it to iv', to compute
 *         a new iv, which is the bound of the WL intersection
 *         in the consumerWL space. I.e.,
 *
 *            iv = F_1( iv');
 *
 * @params: arg_node is an F_noteintersect node.
 *
 *          arriv: The iv' N_array node, or the WITHID_VEC for the
 *                 consumerWL.
 *
 *          lbub: the WLintersect N_array node for lb or ub.
 *                This is denormalized, so that ub and lb are
 *                treated identically. I.e., if we have this WL generator:
 *                  ( [0] <= iv < [50])
 *                then we have these bounds:
 *                                 lb   ub
 *                  Normalized:    [0]  [50]
 *                  Denormlized:   [0]  [49]
 *
 * @result: An N_exprs node, which represents the result of
 *          mapping the WLintersect extrema back to consumerWL space,
 *          for those elements where we can do so.
 *
 *          If we are unable to compute the inverse, we
 *          return NULL. This may occur if, e.g., we multiply
 *          iv by an unknown value, k. If we cannot show
 *          that k is non-zero, we do not have an inverse.
 *
 *****************************************************************************/
static node *
BuildInverseProjectionOne (node *arg_node, info *arg_info, node *arriv, node *lbub)
{
    node *z = NULL;
    node *zw = NULL;
    node *iprime;
    node *ziavis;
    size_t dim;

    size_t ivindx;
    DBUG_ENTER ();

    dim = SHgetUnrLen (ARRAY_FRAMESHAPE (lbub));
    if (N_array != NODE_TYPE (arriv)) {
        DBUG_ASSERT (ID_AVIS (arriv)
                       == IDS_AVIS (
                            WITHID_VEC (PART_WITHID (INFO_CONSUMERWLPART (arg_info)))),
                     "arriv not WITHIDS_VEC!");
        arriv = WITHID_IDS (PART_WITHID (INFO_CONSUMERWLPART (arg_info)));
        dim = TCcountIds (arriv);
    }

    INFO_WITHIDS (arg_info) = NULL;

    for (ivindx = 0; ivindx < dim; ivindx++) {
        ziavis = NULL;
        if (N_array == NODE_TYPE (arriv)) {
            iprime = TCgetNthExprsExpr (ivindx, ARRAY_AELEMS (arriv));
        } else {
            iprime = TCgetNthIds (ivindx, arriv);
        }

        INFO_FINVERSESWAP (arg_info) = FALSE;
        ziavis = BuildInverseProjectionScalar (iprime, arg_info, lbub, ivindx);
        if (NULL != ziavis) {
            if (N_avis == NODE_TYPE (ziavis)) {
                AVIS_FINVERSESWAP (ziavis) = INFO_FINVERSESWAP (arg_info);
                ziavis = TBmakeId (ziavis);
            }

            z = TCappendExprs (z, TBmakeExprs (ziavis, NULL));
            zw = TCappendIds (zw, TBmakeIds (INFO_WITHIDS (arg_info), NULL));
        }
    }

    if (NULL != z) {
        global.optcounters.awlfi_expr += 1;
        INFO_ZWITHIDS (arg_info) = zw;
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *BuildInverseProjections()
 *
 * @brief Given an F_noteintersect node,
 *        examine each set of intersects to compute the inverse projection
 *        of the intersection of the producerWL partition bounds
 *        with the consumerWL's index vector, iv', back into
 *        the consumerWL's partition bounds.
 *
 * @params: arg_node, arg_info.
 *
 * @result: Updated F_noteintersect node.
 *
 *          We may return non-updated node if we can not
 *          compute the inverse projection.
 *          This may occur if, e.g., we multiply
 *          iv by an unknown value, k. If we cannot show
 *          that k is non-zero, we do not have an inverse.
 *          We also have multiple calls to BuildInverseProjection,
 *          one per PWL axis, and a failure on any axis is cause
 *          for failure.
 *
 *          This section of code operates with denormalized extrema.
 *
 *****************************************************************************/
static bool
MatchExpr (node *arg, node *expr)
{
    bool z;

    DBUG_ENTER ();
    z = (arg == expr);
    DBUG_RETURN (z);
}

static node *
BuildInverseProjections (node *arg_node, info *arg_info)
{
    node *zlb = NULL;
    node *zub = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    pattern *pat4;
    size_t numpart;
    size_t curpart;
    size_t curelidxlb;
    size_t curelidxub;
    bool swaplb = FALSE;
    bool swapub = FALSE;
    node *tmp;
    node *intrlb = NULL;
    node *intrub = NULL;
    node *arrlb; /* Denormalized */
    node *arrub; /* Denormalized */
    node *nlet;
    node *zwlb = NULL;
    node *zwub = NULL;
    node *zel = NULL;
    node *zeu = NULL;
    node *arriv = NULL;
    node *ivid;

    DBUG_ENTER ();

    numpart = (TCcountExprs (PRF_ARGS (arg_node)) - WLFIRST) / WLEPP;
    pat1 = PMarray (1, PMAgetNode (&arrlb), 1, PMskip (0));
    pat2 = PMarray (1, PMAgetNode (&arrub), 1, PMskip (0));

    pat3 = PMarray (1, PMAgetNode (&arriv), 1, PMskip (0));

    pat4 = PMany (1, PMAgetNode (&arriv), 0);

    /* ivid is either iv from sel(iv, PWL) or rebuilt value of same */
    ivid = TCgetNthExprsExpr (WLIVAVIS, PRF_ARGS (arg_node));
    /* Guard-skipping for the benefit of Bug #525. */
    if ((PMmatchFlatSkipGuards (pat3, ivid)) || (PMmatchFlat (pat4, ivid))) {
        /* Iterate across intersects */
        for (curpart = 0; curpart < numpart; curpart++) {
            curelidxlb = WLPROJECTION1 (curpart);
            curelidxub = WLPROJECTION2 (curpart);
            DBUG_PRINT ("Building inverse projection for %s, partition #%zu",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))), curpart);
            intrlb = TCgetNthExprsExpr (WLINTERSECTION1 (curpart), PRF_ARGS (arg_node));
            intrub = TCgetNthExprsExpr (WLINTERSECTION2 (curpart), PRF_ARGS (arg_node));

            // Denormalize upper bound before performing inverse projection.
            // Part of the reason for this is that we may swap bounds later on.
            intrub
              = IVEXPadjustExtremaBound (ID_AVIS (intrub), -1, &INFO_VARDECS (arg_info),
                                         &INFO_PREASSIGNS (arg_info), "biptop");
            intrub = TBmakeId (intrub);

            /* If naked consumer, inverse projection is identity */
            if (NULL == INFO_CONSUMERWLPART (arg_info)) {
                PRF_ARGS (arg_node) = TCputNthExprs (curelidxlb, PRF_ARGS (arg_node),
                                                     TBmakeId (ID_AVIS (intrlb)));
                PRF_ARGS (arg_node) = TCputNthExprs (curelidxub, PRF_ARGS (arg_node),
                                                     TBmakeId (ID_AVIS (intrub)));
            }

            if ((!AWLFIisHasInverseProjection (
                  TCgetNthExprsExpr (curelidxlb, PRF_ARGS (arg_node))))
                && (!AWLFIisHasInverseProjection (
                     TCgetNthExprsExpr (curelidxub, PRF_ARGS (arg_node))))) {
                if (!PMmatchFlat (pat2, intrub)) {
                    DBUG_UNREACHABLE ("lost the N_array for %s",
                                      AVIS_NAME (ID_AVIS (intrub)));
                }

                if ((PMmatchFlat (pat1, intrlb)) && (PMmatchFlat (pat2, intrub))
                    && (!WLUTisIdsMemberPartition (intrlb,
                                                   INFO_CONSUMERWLPART (arg_info)))
                    && (!WLUTisIdsMemberPartition (intrub,
                                                   INFO_CONSUMERWLPART (arg_info)))) {
                    zel = BuildInverseProjectionOne (arg_node, arg_info, arriv, arrlb);
                    zwlb = INFO_ZWITHIDS (arg_info);
                    swaplb = INFO_FINVERSESWAP (arg_info);

                    nlet
                      = TCfilterAssignArg (MatchExpr, AVIS_SSAASSIGN (ID_AVIS (intrub)),
                                           &INFO_PREASSIGNS (arg_info));
                    INFO_PREASSIGNS (arg_info)
                      = TCappendAssign (INFO_PREASSIGNS (arg_info), nlet);

                    zeu = BuildInverseProjectionOne (arg_node, arg_info, arriv, arrub);
                    zwub = INFO_ZWITHIDS (arg_info);
                    swapub = INFO_FINVERSESWAP (arg_info);
                }
            }

            if (NULL != INFO_CONSUMERWLPART (arg_info)) {

                /* If we have both new bounds, update the F_noteintersect */
                if ((NULL != zel) && (NULL != zeu)) {
                    DBUG_ASSERT (swaplb == swapub, "Swap confusion");
                    DBUG_ASSERT (N_exprs == NODE_TYPE (zel), "Expected N_exprs zel");
                    DBUG_ASSERT (N_exprs == NODE_TYPE (zeu), "Expected N_exprs zeu");
                    if (swaplb) {
                        //  DBUG_UNREACHABLE ("time2 code");
                        tmp = zel;
                        zel = zeu;
                        zeu = tmp;
                    }

                    DBUG_PRINT ("Building axis permute & confluence for %s, partn #%zu",
                                AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))), curpart);
                    zlb = PermuteIntersectElements (zel, zwlb, arg_info, 0);

                    zub = PermuteIntersectElements (zeu, zwub, arg_info, 1);
                    zub = IVEXPadjustExtremaBound (zub, 1, &INFO_VARDECS (arg_info),
                                                   &INFO_PREASSIGNS (arg_info), "bip5");

                    PRF_ARGS (arg_node)
                      = TCputNthExprs (curelidxlb, PRF_ARGS (arg_node), TBmakeId (zlb));
                    PRF_ARGS (arg_node)
                      = TCputNthExprs (curelidxub, PRF_ARGS (arg_node), TBmakeId (zub));
                }
            }
        }
        zel = NULL;
        zeu = NULL;
    } else {
        DBUG_UNREACHABLE ("Could not find N_array for %s",
                          AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));
    }

    DBUG_PRINT ("Done b inverse projection for %s",
                AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn int PWLFintersectBoundsPolyhedral(
 *
 * @brief Intersect each element of index vector iv with a single producerWL partition,
 *        pwlpart.
 *
 *        Build a maximal affine function tree for each axis of the PWL partition.
 *        Build a maximal affine function tree for each axis of the CWL iv.
 *        Intersect those trees, axis by axis, and OR-reduce the resulting
 *        POLY_xxx summaries. If they indicate that the trees match on all
 *        axes, or that the PWL is a subset of the CWL, then the fold will
 *        take place immediately.
 *
 *        Otherwise, we have three choices:
 *           - folding not possible.
 *           - we do not know if folding is possible
 *           - we have to slice the CWL, as it is a superset of the PWL.
 *
 * @params: arg_node - an N_prf: _sel_VxA_( iv, arr) or _idx_sel_( iv, arr)
 * @params: pwlpart  - the ProducerWL N_part
 * @params: arg_info - your basic arg_info
 *
 * @result: One or more POLY_RET elements from polyhedral_defs.h
 *
 ******************************************************************************/
static bool
isCanStillFold (int el)
{ // Helper predicate. (FIXME Needs extension to support slicing )
    bool z;

    DBUG_ENTER ();

    z = (POLY_RET_MATCH_BC == el) || (POLY_RET_CCONTAINSB == el)
        || (POLY_RET_UNKNOWN == el);

    DBUG_RETURN (z);
}

int
PWLFintersectBoundsPolyhedral (node *arg_node, node *pwlpart, info *arg_info)
{
    node *ivarr = NULL;
    node *ivel;
    node *pwlelavis;
    node *exprscwl = NULL;
    node *exprspwl = NULL;
    node *exprseq = NULL;
    pattern *pat;
    node *arravis;
    node *arrid;
    size_t i;
    size_t shp;
    int z = POLY_RET_UNKNOWN;

    DBUG_ENTER ();

    // Must find N_array node for iv, or give up.
    // iv = PRF_ARG1 (arg_node);
    // If iv is a scalar, we build an N_array for it
    arravis
      = IVUToffset2Vect (arg_node, &INFO_VARDECS (arg_info), &INFO_PREASSIGNS (arg_info),
                         INFO_CONSUMERWLPART (arg_info), pwlpart);
    if (NULL != arravis) {
        arrid = TBmakeId (arravis);
        pat = PMarray (1, PMAgetNode (&ivarr), 0);
        if (PMmatchFlatSkipExtrema (pat, arrid)) {
            shp = TCcountExprs (ARRAY_AELEMS (ivarr));
            // Collect affine exprs for iv, across all axes.
            // Compute the intersect and OR the result flags.
            i = 0;
            while ((i < shp) && isCanStillFold (z)) {
                // pre-cleanup
                pwlelavis = TCgetNthIds (i, WITHID_IDS (PART_WITHID (pwlpart)));
                ivel = TCgetNthExprsExpr (i, ARRAY_AELEMS (ivarr));
                ivel = PHUTskipChainedAssigns (ivel);

                exprspwl = PHUTgenerateAffineExprs (pwlelavis, INFO_FUNDEF (arg_info),
                                                    INFO_VARLUT (arg_info),
                                                    AVIS_ISLCLASSSETVARIABLE, UNR_NONE);
                exprscwl = PHUTgenerateAffineExprs (ivel, INFO_FUNDEF (arg_info),
                                                    INFO_VARLUT (arg_info),
                                                    AVIS_ISLCLASSSETVARIABLE, UNR_NONE);

                // Collect affine exprs for PWL
                exprseq
                  = PHUTgenerateAffineExprsForPwlfIntersect (pwlelavis, ivel,
                                                             INFO_VARLUT (arg_info),
                                                             INFO_FUNDEF (arg_info));

                // Don't bother calling ISL if it can't do anything for us.
                if ((NULL != exprscwl) && (NULL != exprspwl)) {
                    z = ISLUpwlfIntersect (exprspwl, exprscwl, exprseq,
                                           INFO_VARLUT (arg_info),
                                           AVIS_NAME (
                                             ID_AVIS (INFO_PRODUCERWLLHS (arg_info))));
                    // lhsname, above, is slightly misleading, as it indicates
                    // the producer, rather than the consumer
                }

                // Post-cleanup
                ivel = TCgetNthExprsExpr (i, ARRAY_AELEMS (ivarr));
                pwlelavis = TCgetNthIds (i, WITHID_IDS (PART_WITHID (pwlpart)));
                i++;
            }
        }
        pat = PMfree (pat);
        arrid = FREEdoFreeNode (arrid);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *PWLFfundef(node *arg_node, info *arg_info)
 *
 * @brief applies PWLF to a given fundef.
 *        LACFUNs are traversed from PWLFap.
 *
 *****************************************************************************/
node *
PWLFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((FUNDEF_BODY (arg_node) != NULL)
        && (PHUTisFundefKludge (arg_node))) { // Ignore fns named !=, etc.
        DBUG_PRINT ("Begin %s %s",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                    FUNDEF_NAME (arg_node));
        if ((NULL == INFO_LACFUN (arg_info)) && (NULL != FUNDEF_BODY (arg_node))) {
            /* Vanilla traversal */
            arg_node = SWLDdoSetWithloopDepth (arg_node);
            arg_node = INFNCdoInferNeedCountersOneFundef (arg_node, TR_pwlf);
            arg_node = WLNCdoWLNeedCount (arg_node);
            arg_node = WLCCdoWLCostCheck (arg_node);

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            /* If new vardecs were made, append them to the current set */
            if (INFO_VARDECS (arg_info) != NULL) {
                BLOCK_VARDECS (FUNDEF_BODY (arg_node))
                  = TCappendVardec (INFO_VARDECS (arg_info),
                                    BLOCK_VARDECS (FUNDEF_BODY (arg_node)));
                INFO_VARDECS (arg_info) = NULL;
            }
        }
        DBUG_PRINT ("End %s %s",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                    FUNDEF_NAME (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node PWLFassign( node *arg_node, info *arg_info)
 *
 * @brief performs a top-down traversal.
 *        For a foldable WL, arg_node is:  x = _sel_VxA_(iv, producerWL).
 *
 *        A bottom-up traversal is superior to top-down. Here's why:
 *        Assume we have this code, where WITHn is a WL.
 *
 *            B = WITH1();
 *            C = WITH2(B);
 *            D = WITH3(B,C);
 *
 *         If we fold top-down, we skip WITH1, because it can't fold.
 *         WITH2 will not be folded, because B is referenced by WITH2 and WITH3,
 *         and folding could double the work required to create B, so
 *         a conscious decision is made to avoid folding.
 *         (NB. We could apply a cost function to allow folding when the
 *          "cost" is low, but that's a frill.)
 *         WITH3 will fold C into D, but not B, because it is referenced
 *         by WITH2. On the next pass, we have:
 *
 *            B = WITH1();
 *            D = WITH3(B);
 *
 *         and WITH1 will be folded into D.
 *
 *         Whereas, if we operate bottom-up, we have this:
 *
 *            B = WITH1();
 *            C = WITH2(B);
 *            D = WITH3(B,C);
 *
 *           We look at WITH3, and fold C into D. At that point, B is
 *           only referenced by WITH3, so we fold B into D, in the same
 *           pass.
 *
 *****************************************************************************/
node *
PWLFassign (node *arg_node, info *arg_info)
{
    node *let;
    node *oldpreassigns;

    DBUG_ENTER ();

    oldpreassigns = INFO_PREASSIGNS (arg_info);
    INFO_PREASSIGNS (arg_info) = NULL;
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    INFO_PREASSIGNS (arg_info) = oldpreassigns;

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    let = ASSIGN_STMT (arg_node);
    if ((N_let == NODE_TYPE (let)) && (N_with == NODE_TYPE (LET_EXPR (let)))
        && (INFO_PREASSIGNSWL (arg_info) != NULL)) {
        arg_node = TCappendAssign (INFO_PREASSIGNSWL (arg_info), arg_node);
        INFO_PREASSIGNSWL (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PWLFwith( node *arg_node, info *arg_info)
 *
 * @brief applies PWLF to a with-loop in a top-down manner.
 *
 *        When we return here, we will have counted all the
 *        references to any potential producerWL that we
 *        may want to deal with. We already know the producerWL
 *        reference count, because that's computed at entry to
 *        this phase. Hence, we are then in a position to
 *        determine if the fold will be legal.
 *
 *****************************************************************************/
node *
PWLFwith (node *arg_node, info *arg_info)
{
    info *old_arg_info;
    node *consumerop;
    node *producershape;
    node *genop;

    DBUG_ENTER ();

    old_arg_info = arg_info;
    arg_info = MakeInfo (INFO_FUNDEF (arg_info));
    INFO_FOLDLUT (arg_info) = INFO_FOLDLUT (old_arg_info);
    INFO_VARLUT (arg_info) = INFO_VARLUT (old_arg_info);
    INFO_LET (arg_info) = INFO_LET (old_arg_info);
    INFO_DEFDEPTH (arg_info) = INFO_DEFDEPTH (old_arg_info) + 1;
    INFO_VARDECS (arg_info) = INFO_VARDECS (old_arg_info);
    INFO_PREASSIGNS (arg_info) = INFO_PREASSIGNS (old_arg_info);
    INFO_FINVERSEINTRODUCED (arg_info) = INFO_FINVERSEINTRODUCED (old_arg_info);

    INFO_CONSUMERWL (arg_info) = arg_node;
    INFO_CONSUMERWLIDS (arg_info) = LET_IDS (INFO_LET (old_arg_info));
    DBUG_PRINT ("Looking at %s with INFO_DEFDEPTH=%d",
                AVIS_NAME (IDS_AVIS (INFO_CONSUMERWLIDS (arg_info))),
                INFO_DEFDEPTH (arg_info));

    DBUG_PRINT ("Resetting WITH_REFERENCED_CONSUMERWL, etc.");
    WITH_REFERENCED_FOLD (arg_node) = 0;
    WITH_REFERENCED_CONSUMERWL (arg_node) = NULL;

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    /* Try to replace modarray(PWL) by genarray(shape(PWL)).
     * This has no effect on the PWL itself, but is needed to
     * eliminate the reference to PWL, so it can be removed by DCR.
     *
     * If the PWL has been folded, its NEEDCOUNT will be one, as the
     * only reference to it will be in the MODARRAY.
     * Hence, we can blindly replace the modarray by the genarray.
     */
    consumerop = WITH_WITHOP (arg_node);
    if ((N_modarray == NODE_TYPE (consumerop))) {
        DBUG_PRINT ("producerWL %s has AVIS_NEEDCOUNT=%d and AVIS_WL_NEEDCOUNT=%d",
                    AVIS_NAME (ID_AVIS (MODARRAY_ARRAY (consumerop))),
                    AVIS_NEEDCOUNT (ID_AVIS (MODARRAY_ARRAY (consumerop))),
                    AVIS_WL_NEEDCOUNT (ID_AVIS (MODARRAY_ARRAY (consumerop))));
    }

    if ((N_modarray == NODE_TYPE (consumerop))
        && (NULL != AVIS_SHAPE (ID_AVIS (MODARRAY_ARRAY (consumerop))))
        && (1 == AVIS_NEEDCOUNT (ID_AVIS (MODARRAY_ARRAY (consumerop))))) {
        producershape = AVIS_SHAPE (ID_AVIS (MODARRAY_ARRAY (consumerop)));
        genop = TBmakeGenarray (DUPdoDupTree (producershape), NULL);
        GENARRAY_NEXT (genop) = MODARRAY_NEXT (consumerop);
        consumerop = FREEdoFreeNode (consumerop);
        WITH_WITHOP (arg_node) = genop;
        DBUG_PRINT ("Replacing modarray by genarray");
    }

    INFO_VARDECS (old_arg_info) = INFO_VARDECS (arg_info);
    INFO_FINVERSEINTRODUCED (old_arg_info) = INFO_FINVERSEINTRODUCED (arg_info);
    INFO_PREASSIGNS (old_arg_info) = INFO_PREASSIGNS (arg_info);

    arg_info = FreeInfo (arg_info);
    arg_info = old_arg_info;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PWLFpart( node *arg_node, info *arg_info)
 *
 * @brief Traverse each partition of a WL.
 *
 *****************************************************************************/
node *
PWLFpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_CONSUMERWLPART (arg_info) = arg_node;

    arg_node = POLYSsetClearAvisPart (arg_node, arg_node);
    CODE_CBLOCK (PART_CODE (arg_node))
      = TRAVdo (CODE_CBLOCK (PART_CODE (arg_node)), arg_info);
    arg_node = POLYSsetClearAvisPart (arg_node, NULL);

    INFO_CONSUMERWLPART (arg_info) = NULL;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PWLFid(node *arg_node, info *arg_info)
 *
 * description:
 *   If this Id is a reference to a WL (N_with) we want to increment
 *   the number of references to the potential producerWL (WITH_REFERENCED_FOLD).
 *   We want to end up with WITH_REFERENCED_FOLD counting only
 *   references from a single WL. Hence, the checking on
 *   WITH_REFERENCED_CONSUMERWL
 *   PWLF will disallow folding if WITH_REFERENCED_FOLD != AVIS_NEEDCOUNT.
 *
 *   The intent here is to avoid doing the producer-WL computation more than once,
 *   if there are references to it in more than one consumer-WL.
 *
 ******************************************************************************/
node *
PWLFid (node *arg_node, info *arg_info)
{
    node *p;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking at %s", AVIS_NAME (ID_AVIS (arg_node)));
    p = INFO_CONSUMERWL (arg_info);
    if ((NULL != p) && (NULL == WITH_REFERENCED_CONSUMERWL (p))) {
        /* First reference to this WL. */
        WITH_REFERENCED_CONSUMERWL (p) = INFO_CONSUMERWL (arg_info);
        WITH_REFERENCED_FOLD (p) = 0;
        DBUG_PRINT ("found first reference to %s", AVIS_NAME (ID_AVIS (arg_node)));
    }

    /*
     * arg_node describes a WL, so
     * WITH_REFERENCED_FOLD( p) may have to be incremented
     */
    if ((NULL != p) && (WITH_REFERENCED_CONSUMERWL (p) == INFO_CONSUMERWL (arg_info))) {
        (WITH_REFERENCED_FOLD (p))++;
        DBUG_PRINT ("Incrementing WITH_REFERENCED_FOLD(%s) = %d",
                    AVIS_NAME (ID_AVIS (arg_node)), WITH_REFERENCED_FOLD (p));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PWLFprf( node *arg_node, info *arg_info)
 *
 * @brief
 *   Examine a _sel_VxA_(iv, producerWL) primitive to see if
 *   we may be able to fold producerWL here, assuming that
 *   the _sel_ is within a potential consumerWL, OR that idx is
 *   constant, or otherwise known.
 *
 *   When we encounter an eligible sel() operation, we use
 *   polyhedral methods to compute
 *   the intersect between the index set of iv,
 *   and each partition of the producerWL.
 *
 *   If the intersect is POLY_RET_MATCH_BC  or POLY_RET_CCONTAINSB
 *   or POLY_RET_EMPTY_SET_B, *   we perform the fold.
 *
 *   If the intersect is POLY_RET_EMPTYSET_BC, we go on to next PWL partition.
 *
 *   Otherwise, we use the result of intersect to slice the CWL, then
 *   perform the fold.
 *
 * @result: Original N_prf, or else an assign of the newly folded
 *          PWL block that will have been appended to INFO_PREASSIGNS,
 *          for placement above this N_prf when we return to PWLFassign.
 *
 *****************************************************************************/
node *
PWLFprf (node *arg_node, info *arg_info)
{
    node *pwlid;
    node *pwlpart;
    node *foldpwlpart = NULL;
    node *z = NULL;
#ifndef DBUG_OFF
    char *cwlnm;
#endif
    int plresult = POLY_RET_UNKNOWN;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    default:
        break;

    case F_sel_VxA:
    case F_idx_sel:
        pwlid = AWLFIfindWlId (PRF_ARG2 (arg_node));
        INFO_PRODUCERWLLHS (arg_info) = pwlid;
        INFO_PRODUCERWL (arg_info) = AWLFIfindWL (pwlid);
        INFO_PRODUCERWLFOLDABLE (arg_info)
          = AWLFIcheckProducerWLFoldable (pwlid)
            && AWLFIcheckBothFoldable (pwlid, INFO_CONSUMERWLIDS (arg_info),
                                       INFO_DEFDEPTH (arg_info));

        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

        if (INFO_PRODUCERWLFOLDABLE (arg_info)) {
            pwlpart = WITH_PART (INFO_PRODUCERWL (arg_info));
            while ((POLY_RET_UNKNOWN == plresult) && (N_prf == NODE_TYPE (arg_node))
                   && (NULL != pwlpart)) {
                pwlpart = POLYSsetClearAvisPart (pwlpart, pwlpart);
                foldpwlpart = pwlpart;
                plresult = PWLFintersectBoundsPolyhedral (arg_node, pwlpart, arg_info);
#ifndef DBUG_OFF
                cwlnm = (NULL != INFO_CONSUMERWLIDS (arg_info))
                          ? AVIS_NAME (IDS_AVIS (INFO_CONSUMERWLIDS (arg_info)))
                          : "(naked consumer)";
#endif
                if ((POLY_RET_MATCH_BC & plresult) || (POLY_RET_CCONTAINSB & plresult)) {
                    DBUG_PRINT ("We now fold PWL %s into CWL %s with plresult %d",
                                AVIS_NAME (ID_AVIS (pwlid)), cwlnm, plresult);
                    DBUG_PRINT ("Building inverse projection for cwl=%s", cwlnm);
                    arg_node = BuildInverseProjections (arg_node, arg_info);
                    DBUG_ASSERT (POLY_RET_CCONTAINSB == plresult,
                                 "Coding time, Bobbo. We need to slice cwl");
                    z = PWLFperformFold (arg_node, foldpwlpart, arg_info);
                    FREEdoFreeNode (arg_node);
                    arg_node = z;
                    global.optcounters.pwlf_expr += 1;
                } else {
                    if (POLY_RET_SLICENEEDED & plresult) {
                        DBUG_PRINT ("Slicing needed to fold PWL %s into CWL %s",
                                    AVIS_NAME (ID_AVIS (pwlid)), cwlnm);
                        // FIXME  int FIXAVISNPART; // after cube slicing
                    } else {
                        DBUG_PRINT ("Unable to fold PWL %s into CWL %s with plresult %d",
                                    AVIS_NAME (ID_AVIS (pwlid)), cwlnm, plresult);
                    }
                }
                pwlpart = POLYSsetClearAvisPart (pwlpart, NULL);
                pwlpart = PART_NEXT (pwlpart);
                // Clear LUT, AVIS_ISLCLASS, AVIS_ISLTREE
                PHUTpolyEpilogOne (INFO_VARLUT (arg_info));
                plresult = POLY_RET_UNKNOWN;
            }
        }
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PWLFcond(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse conditional parts in the given order.
 *
 ******************************************************************************/
node *
PWLFcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THENASSIGNS (arg_node) = TRAVopt (COND_THENASSIGNS (arg_node), arg_info);
    COND_ELSEASSIGNS (arg_node) = TRAVopt (COND_ELSEASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PWLFmodarray(node *arg_node, info *arg_info)
 *
 * description:
 *   if op is modarray( producerWL), we increment
 *   WITH_REFERENCED_FOLD for the producerWL.
 *
 ******************************************************************************/
node *
PWLFmodarray (node *arg_node, info *arg_info)
{
    node *wl;

    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    if (N_modarray == NODE_TYPE (arg_node)) {
        INFO_PRODUCERWL (arg_info) = AWLFIfindWlId (MODARRAY_ARRAY (arg_node));
        wl = INFO_PRODUCERWL (arg_info);
        (WITH_REFERENCED_FOLD (wl))++;
        DBUG_PRINT ("WITH_REFERENCED_FOLD(%s) = %d",
                    AVIS_NAME (ID_AVIS (MODARRAY_ARRAY (arg_node))),
                    WITH_REFERENCED_FOLD (wl));
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PWLFap( node *arg_node, info *arg_info)
 *
 * @brief: If this is a non-recursive call of a LACFUN,
 *         traverse the LACFUN.
 *
 *****************************************************************************/
node *
PWLFap (node *arg_node, info *arg_info)
{
    node *lacfundef;
    node *newfundef;

    DBUG_ENTER ();

    lacfundef = AP_FUNDEF (arg_node);
    if ((NULL == INFO_LACFUN (arg_info)) &&      // Vanilla traversal
        (FUNDEF_ISLACFUN (lacfundef)) &&         // Ignore non-lacfun call
        (lacfundef != INFO_FUNDEF (arg_info))) { // Ignore recursive call
        DBUG_PRINT ("Found LACFUN: %s non-recursive call from: %s",
                    FUNDEF_NAME (lacfundef), FUNDEF_NAME (INFO_FUNDEF (arg_info)));
        /* Traverse into the LACFUN */
        INFO_LACFUN (arg_info) = lacfundef; // The called lacfun
        newfundef = TRAVdo (lacfundef, arg_info);
        DBUG_ASSERT (newfundef = lacfundef,
                     "Did not expect N_fundef of LACFUN to change");
        INFO_LACFUN (arg_info) = NULL; // Back to normal traversal
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PWLFlet(node *arg_node, info *arg_info)
 *
 * description: Descend to get set AVIS_DEPTH and to handle the expr.
 *
 ******************************************************************************/
node *
PWLFlet (node *arg_node, info *arg_info)
{
    node *oldlet;

    DBUG_ENTER ();

    oldlet = INFO_LET (arg_info);
    INFO_LET (arg_info) = arg_node;
    LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LET (arg_info) = oldlet;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PWLFblock(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
PWLFblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PWLFfuncond( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
PWLFfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNCOND_IF (arg_node) = TRAVopt (FUNCOND_IF (arg_node), arg_info);
    FUNCOND_THEN (arg_node) = TRAVopt (FUNCOND_THEN (arg_node), arg_info);
    FUNCOND_ELSE (arg_node) = TRAVopt (FUNCOND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PWLFwhile( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
PWLFwhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WHILE_COND (arg_node) = TRAVopt (WHILE_COND (arg_node), arg_info);
    WHILE_BODY (arg_node) = TRAVopt (WHILE_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Polyhedral with loop folding inference -->
 *****************************************************************************/

#undef DBUG_PREFIX
