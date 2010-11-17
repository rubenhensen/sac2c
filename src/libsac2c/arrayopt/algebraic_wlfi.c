/*
 * $Id: algebraic_wlfi.c 16030 2009-04-09 19:40:34Z rbe $
 */

/** <!--********************************************************************-->
 *
 * @defgroup awlfi Algebraic With-Loop Folding Inference
 *
 * @terminology:
 *        ProducerWL:
 *        The WL that will no longer
 *        exist after this phase completes. In the example
 *        below, A is the producerWL.
 *
 *        ConsumerWL:
 *        the WL that will absorb the block(s) from
 *        the producerWL. In the example below, B is the ConsumerWL.
 *
 *        Intersection expression: the expression that
 *        specifies the set of index vectors in one WL partition
 *        that are present in a partition of another WL.
 *
 * @brief Algebraic With-Loop Folding (AWLF)
 *        AWLF performs WLF on some arrays that are not foldable
 *        by WLF. Features of AWLF are described in algebraic_wlf.c
 *
 *        This phase begins by computing AVIS_NEEDCOUNT for
 *        all N_id nodes. This is done as a cost-function
 *        heuristic to prevent the same producerWL element
 *        from being computed repeatedly.
 *        [ We could develop a better cost function. For example,
 *          iota(N) has a cost of zero, because the producerWL
 *          element cost is just an access to the WITH_ID.]
 *
 *        The phase then makes a depth-first traversal
 *        of the function, computing WL_REFERENCED_FOLD(producerWL)++
 *        for all sel() operations in WLs that are of the form:
 *
 *             _sel_VxA_(iv, producerWL)
 *
 *        where producerWL is the result of another WL.
 *        At present, only one consumerWL is allowed to contribute to the
 *        total. AWLF will permit the folding to occur if, among
 *        other requirements:
 *
 *         AVIS_NEEDCOUNT(producerWL) == WL_REFERENCED_FOLD(producerWL)
 *
 *        This phase inserts _attachextrema "intersection expressions"
 *        before _sel_(iv, X) statements, if iv is a function of
 *        the WITH_ID(consumerWL) and X is the result of a producerWL.
 *
 *        An intersection expression provides the information required
 *        to cut a consumerWL partition so that its index set
 *        is entirely contained with the index set of
 *        one partition of the producerWL.
 *
 *        Once the cutting, if needed, has been performed, folding of the
 *        producerWL into the consumerWL can be performed directly,
 *        by replacing the above _sel_(iv, X) expression with
 *        the code from the producerWL partition, and performing
 *        appropriate renames of WITH_IDs.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file algebraic_wlfi.c
 *
 * Prefix: AWLFI
 *
 *****************************************************************************/
#include "algebraic_wlfi.h"
#include "algebraic_wlf.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "print.h"
#include "dbug.h"
#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "inferneedcounters.h"
#include "compare_tree.h"
#include "DupTree.h"
#include "free.h"
#include "globals.h"
#include "wl_cost_check.h"
#include "wl_needcount.h"
#include "pattern_match.h"
#include "constants.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "check.h"
#include "ivextrema.h"
#include "phase.h"
#include "namespaces.h"
#include "deserialize.h"
#include "wls.h"
#include "SSAWithloopFolding.h"

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
    node *consumerpart;      /* The current consumerWL partition */
    node *consumerwl;        /* The current consumerWL N_with */
    node *producerwl;        /* The producerWL LHS for this consumerWL */
    node *let;               /* The N_let node */
    int level;               /* The current nesting level of WLs. This
                              * is used to ensure that an index expression
                              * refers to an earlier WL in the same code
                              * block, rather than to a WL within this
                              * WL. I think...
                              */
    bool producerWLFoldable; /* producerWL may be legally foldable. */
                             /* (If index sets prove to be OK)     */
    bool onefundef;          /* fundef-based traversal */
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_PREASSIGNSWL(n) ((n)->preassignswl)
#define INFO_CONSUMERPART(n) ((n)->consumerpart)
#define INFO_CONSUMERWL(n) ((n)->consumerwl)
#define INFO_PRODUCERWL(n) ((n)->producerwl)
#define INFO_LET(n) ((n)->let)
#define INFO_LEVEL(n) ((n)->level)
#define INFO_PRODUCERWLFOLDABLE(n) ((n)->producerWLFoldable)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_PREASSIGNSWL (result) = NULL;
    INFO_CONSUMERPART (result) = NULL;
    INFO_CONSUMERWL (result) = NULL;
    INFO_PRODUCERWL (result) = NULL;
    INFO_LET (result) = NULL;
    INFO_LEVEL (result) = 0;
    INFO_PRODUCERWLFOLDABLE (result) = FALSE;
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

/******************************************************************************
 *
 * function:
 *   node *AWLFIdoAlgebraicWithLoopFoldingOneFunction(node *arg_node)
 *
 * @brief Global entry point of Algebraic With-Loop folding
 *        Applies Algebraic WL folding to a fundef.
 *
 *****************************************************************************/
node *
AWLFIdoAlgebraicWithLoopFoldingOneFunction (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("AWLFIdoAlgebraicWithLoopFoldingOneFunction");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 ("AWLFIdoAlgebraicWithLoopFoldingOneFunction called for non-fundef"));

    arg_info = MakeInfo (arg_node);
    INFO_ONEFUNDEF (arg_info) = TRUE;

    DSinitDeserialize (global.syntax_tree);

    TRAVpush (TR_awlfi);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DSfinishDeserialize (global.syntax_tree);

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
 * @fn node *GenerateInverseFunction( node *arg_node, node **vardecs)
 *
 * @brief: Generate inverse function for affine expression arg_node.
 *         Typically, this will be the IV' to be used in AWLF.
 *
 * @params: arg_node: Starting (result) node of expression.
 *          vardecs: Pointer to pointer to vardecs chain, for adding
 *          new variables.
 *
 * @result: Inverse function expression tree, or NULL if
 *          no inverse can be found.
 *
 *****************************************************************************/
#ifdef UNDERCONSTRUCTION
static node *
GenerateInverseFunction (node *arg_node, node **vardecs)
{
    node *z = NULL;

    DBUG_ENTER ("GenerateInverseFunction");

    if (NULL != arg_node) {

        switch (NODE_TYPE (arg_node)) {
        default:
            newnode = NULL;
            break;
        case N_prf:
            switch (PRF_PRF (arg_node)) {
            default:
                newnode = NULL;
                break;
            case F_add_SxS:
                newnode = DUPdoDupNode (arg_node);

                xxx;
                break;
            case F_add_SxV:
            }
        }
    }

    DBUG_RETURN (z);
}
#endif // UNDERCONSTRUCTION

#ifdef DEADCODE

/** <!--********************************************************************-->
 *
 * @fn node *TraceIndexScalars(...)
 *
 * @brief Chase the set of N_array scalars back to their WITH_IDS,
 *        if possible.
 *
 * @params:  arg_node: is an ARRAY_AELEMS node, that represents
 *                     the iv' elements.
 *                     Or, it's an N_id node from that N_array.
 *           arg_info: Your basic arg_info node.
 *           dim: the number of elements in arg_node
 *           n:   the current element index into arg_node that
 *                we want to examine.
 *
 * @result: An N_exprs node comprising the WITH_IDS rererenced
 *          in arg_node, iff we found a full set of them.
 *          Else, NULL.
 *
 *          FIXME: This should allow for mixed indexing, such as:
 *                 sel( [0, j, i], producerWL). However, it doesn't
 *                 do that yet.
 *
 *****************************************************************************/
static node *
TraceIndexScalars (node *arg_node, info *arg_info, int dim, int n)
{
    node *z = NULL;
    node *z2;
    node *idx = NULL;
    node *i;
    int m;

    pattern *pat;

    DBUG_ENTER ("TraceIndexScalars");

    pat = PMany (1, PMAgetNode (&idx), 0);
    i = (N_array == NODE_TYPE (arg_node)) ? TCgetNthExprsExpr (n, arg_node) : arg_node;
    n = n + 1;
    if (PMmatchFlatSkipExtremaAndGuards (pat, i)) {
        switch (NODE_TYPE (idx)) {
        case N_id:
            m = WLFlocateIndexVar (idx, INFO_CONSUMERWL (arg_info));
            if (0 < m) {
                z = TBmakeExprs (TBmakeNum (m), NULL);
                if (n < dim) {
                    z2 = TraceIndexScalars (arg_node, arg_info, dim, n);
                    if (NULL != z2) {
                        z = TCappendExprs (z, z2);
                    } else {
                        z = FREEdoFreeTree (z);
                    }
                }
            }
            break;

        case N_prf:
            /* Since we want to trace the axis permutations only,
             * we must not execute any scalar functions. We just
             * trace the non-constant argument to the function.
             */
            switch (PRF_PRF (idx)) {
            default:
                break;
            case F_add_SxS:
            case F_sub_SxS:
            case F_mul_SxS:
                if (COisConstant (PRF_ARG2 (idx))) {
                    z = TraceIndexScalars (PRF_ARG1 (idx), arg_info, 0, 0);
                } else {
                    DBUG_PRINT ("AWLFI", ("Punting to chase N_prf argument"));
                    z = TraceIndexScalars (PRF_ARG2 (idx), arg_info, 0, 0);
                }
                break;
            }
            break;

        default:
            DBUG_PRINT ("AWLFI", ("Cannot chase scalar index"));
            break;
        }
    }

    pat = PMfree (pat);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *
 *
 * @brief
 *
 * @params:
 *
 * @result:
 *
 *
 *****************************************************************************/
static node *
TraceIndexVector (node *arg_node)
{
    node *z = NULL;
    DBUG_ENTER ("TraceIndexVector");

    DBUG_ASSERT (FALSE, " FIXME please ");

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *BuildIVPV( node *arg_node, info *arg_info)
 *
 * @brief Given a consumerWL _sel_VxA_( iv', producerWL) arg_node,
 *        create a symbiotic expression to compute the permutation
 *        of the consumerWL's withid, iv, that will give us iv'.
 *
 *        We start by searching backwards from iv'.
 *
 * @params: arg_node, arg_info: as above
 *
 * @result: An N_id node to compute the PV.
 *          If we can't solve the permutation, we return a vector
 *          of -1s.
 *
 *
 *****************************************************************************/
static node *
BuildIVPV (node *arg_node, info *arg_info)
{
    node *z;
    node *wliv;
    node *seliv = NULL;
    node *arr = NULL;
    pattern *pat1;
    pattern *pat2;
    int step = 0;
    int origin = -1;
    int dim;
    int ivindx;
    DBUG_ENTER ("BuildIVPV");

    pat1 = PMany (1, PMAgetNode (&seliv), 0);
    pat2 = PMarray (1, PMAgetNode (&arr), 1, PMskip (0));

    if (PMmatchFlatSkipExtremaAndGuards (pat1, PRF_ARG1 (arg_node))) {
        switch (NODE_TYPE (seliv)) {
        case N_id:
            ivindx = WLFlocateIndexVar (seliv, INFO_CONSUMERWL (arg_info));
            switch (ivindx) {
            case 0: /* No idea. build identity, because we need non-NULL exprs */
                DBUG_PRINT ("AWLFI", (" Chasing N_id WITH_VEC"));
                z = TraceIndexVector (seliv);
                /* FIXME */
                break;

            case -1: /* Case 1: iv' or its predecessor is WITHID_VEC */
                wliv = WITHID_VEC (PART_WITHID (WITH_PART (INFO_CONSUMERWL (arg_info))));
                origin = 0;
                step = 1;
                dim = SHgetUnrLen (TYgetShape (IDS_NTYPE (wliv)));
                break;

            default: /* Case N: iv' or predecessor is WITH_IDS */
                DBUG_ASSERT (FALSE, "Found scalar index in sel()");
                break;
            }
            break;

        case N_array: /* iv' is N_array */
            wliv = WITHID_IDS (PART_WITHID (WITH_PART (INFO_CONSUMERWL (arg_info))));
            dim = SHgetUnrLen (TYgetShape (IDS_NTYPE (wliv)));
            if (PMmatchFlatSkipExtremaAndGuards (pat2, PRF_ARG1 (arg_node))) {
                DBUG_PRINT ("AWLFI", (" Chasing N_array WITH_IDS"));
                z = TraceIndexScalars (ARRAY_AELEMS (seliv), arg_info, dim, 0);
            }
            break;

        default:
            DBUG_PRINT ("AWLFI", ("FIXME: Need to chase something else"));
            break;
        }
    }

    z = (NULL != z) ? TCmakeIntVector (z) : TCcreateIntVector (dim, origin, step);
    z = AWLFIflattenExpression (z, &INFO_VARDECS (arg_info), &INFO_PREASSIGNS (arg_info),
                                TYmakeAKS (TYmakeSimpleType (T_int),
                                           SHcreateShape (1, dim)));
    z = TBmakeId (z);
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (z);
}

#endif // DEADCODE

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIgetWlWith( node *arg_node)
 *
 * @brief Given an N_id, return its N_with node, or NULL, if arg_node
 *        was not created by a WL.
 *        This skips over, e.g., afterguards in the way.
 *
 * @params: arg_node
 * @result: The N_with node of the WL
 *
 *****************************************************************************/
node *
AWLFIgetWlWith (node *arg_node)
{
    node *wl = NULL;
    node *z;
    pattern *pat;

    DBUG_ENTER ("AWLFIgetWlWith");

    pat = PMwith (1, PMAgetNode (&wl), 0);
    z = ((PMmatchFlatWith (pat, arg_node))
         && (N_array == NODE_TYPE (GENERATOR_BOUND1 (PART_GENERATOR (WITH_PART (wl))))))
          ? wl
          : NULL;
    pat = PMfree (pat);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIfindWlId( node *arg_node)
 *
 * @brief: Determine if N_id arg_node was created by a WL.
 *
 * @param: arg_node: an N_id node
 *
 * @return: The N_id that created the WL, or NULL if the N_id node
 *          was not created by a WL, or if the WL generators
 *          are not N_array nodes.
 *
 *****************************************************************************/
node *
AWLFIfindWlId (node *arg_node)
{
    node *z = NULL;
    node *wl;
    pattern *pat;

    DBUG_ENTER ("AWLFIfindWlId");

    pat = PMvar (1, PMAgetNode (&z), 0);
    if (PMmatchFlatSkipGuards (pat, arg_node)) {
        wl = AWLFIgetWlWith (z);
        if (NULL != wl) {
            DBUG_PRINT ("AWLFI",
                        ("Found WL:%s: WITH_REFERENCED_FOLD=%d",
                         AVIS_NAME (ID_AVIS (arg_node)), WITH_REFERENCED_FOLD (wl)));
        }
    } else {
        z = NULL;
        DBUG_PRINT ("AWLFI", ("Did not find WL:%s", AVIS_NAME (ID_AVIS (arg_node))));
    }
    pat = PMfree (pat);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool noDefaultPartition( node *arg_node)
 *
 *   @brief  TRUE if the N_with does not contain a default N_part.
 *
 *   @param  node *arg_node: an N_with node.
 *   @return Boolean.
 *
 ******************************************************************************/
static bool
noDefaultPartition (node *arg_node)
{
    node *partn;
    bool z = TRUE;

    DBUG_ENTER ("noDefaultPartition");

    partn = WITH_PART (arg_node);

    while ((NULL != partn) && z) {
        z = z & (N_default != NODE_TYPE (PART_GENERATOR (partn)));
        partn = PART_NEXT (partn);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIflattenExpression(node *arg_node, node **vardecs,
 *                                  node **preassigns, ntype *restype)
 *
 *   @brief  Flattens the expression at arg_node.
 *           E.g., if the expression is:
 *
 *            _max_VxV_(a, b);
 *
 *          it will look like this on the way out:
 *           TYPETHINGY  TMP;
 *            ...
 *
 *            TMP = _max_VxV_(a, b);
 *            TMP
 *
 *   NB: It is the caller's responsibility to DUP the arg_node,
 *       if required.
 *
 *   @param  node *arg_node: a node to be flattened.
 *           node **vardecs: a pointer to a vardecs chain that
 *                           will have a new vardec appended to it.
 *           node **preassigns: a pointer to a preassigns chain that
 *                           will have a new assign appended to it.
 *           node *restype:  the ntype of TMP.
 *
 *   @return node *node:      N_avis node for flattened node
 *
 ******************************************************************************/
node *
AWLFIflattenExpression (node *arg_node, node **vardecs, node **preassigns, ntype *restype)
{
    node *avis;
    node *nas;

    DBUG_ENTER ("AWLFIflattenExpression");

    if (N_id == NODE_TYPE (arg_node)) {
        avis = ID_AVIS (arg_node);
    } else {
        avis = TBmakeAvis (TRAVtmpVar (), restype);
        *vardecs = TBmakeVardec (avis, *vardecs);
        nas = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), arg_node), NULL);
        *preassigns = TCappendAssign (*preassigns, nas);
        AVIS_SSAASSIGN (avis) = nas;
        DBUG_PRINT ("AWLFI", ("Generated assign for %s", AVIS_NAME (avis)));
    }

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *IntersectBoundsBuilderOne( node *arg_node, info *arg_info,
 *                                      node *producerwlPart, int boundnum,
 *                                      node *ivminmax)
 *
 * @brief Build a pair of expressions for intersecting the bounds of
 *        a single producerWL partition with consumerWL index set, of the form:
 *
 *           iv' = (k*iv) + ivoffset;
 *           z = sel( iv', producerWL)
 *
 *        We determine the intersection of the consumerWL iv'
 *        index set with the producerWL's partition bounds this way:
 *
 *          genshp = _shape_A_(GENERATOR_BOUND1( producerwlPart));
 *
 *          (The next few lines compute:
 *           intlo = _max_VxV_( AVIS_MIN( iv'),
 *                              GENERATOR_BOUND1(producerwlPart));
 *
 *          but the optimizers are unable to compute common
 *          expressions such as:
 *
 *           _max_VxV_( iv, iv + 1)
 *
 *          Hence, we use the following:
 *
 *           xl = AVIS_MIN( iv');
 *           yl = GENERATOR_BOUND1( producerwlPart);
 *           d  = _sub_VxV_( xl, yl);
 *           zero = 0;
 *           p = _gt_VxS_( d, zero);
 *           p0intlo = _mesh_( p, xl, yl);
 *           )
 *
 *          ( Similar treatment for _min_VxV_ as above:
 *
 *          xh = AVIS_MAX( iv');
 *          yh = GENERATOR_BOUND2( producerwlPart);
 *          d'  = _sub_VxV_( xh, yh);
 *          p' = _lt_VxS_( d', zero);
 *          p0inthi = _mesh_( p', xh, yh);
 *          )
 *
 *          iv'' = _noteintersect(iv',
 *                                 p0bound1, p0bound2, p0intlo, p0inthi, p0int,
 *                                 p1bound1, p1bound2, p1intlo, p1inthi, p1int,
 *                                 ...);
 *          z = sel( iv'', producerWL)
 *
 *        where int1 evaluates to the lower bound of the intersection,
 *        and   int2 evaluates to the upper bound of the intersection
 *        and p0, p1... are the partitions of the producerWL.
 *
 *
 *        NB. ALL lower bounds precede all upper bounds in the
 *            _attach_extrema_ arguments.
 *
 *        NB. We need the producerWL partition bounds for at least
 *            two reasons: a producerWL partition may be split
 *            between the time we build this code and the time
 *            we look at the answer. When we find a potential
 *            hit, we have to go look up the partition bounds
 *            in the producerWL again, to ensure that the requisite
 *            partition still exists. Also, even if the partitions
 *            remain unchanged in size, their order may be
 *            shuffled, so we have to search for the right one.
 *
 *        NB. The null intersect computation is required so that
 *            we can distinguish it from non-null intersects,
 *             for the cases where cube slicing will be required.
 *
 * @params arg_node: the _sel_VxA_( idx, producerWL)
 * @params arg_info.
 * @params producerPart: An N_part of the producerWL.
 * @params boundnum: 1 for bound1,     or 2 for bound2
 * @params ivmin: AVIS_MIN( idx)
 * @params ivmax: AVIS_MAX( idx)
 *
 * @return An N_avis pointing to an N_exprs for the two intersect expressions.
 *
 *****************************************************************************/

static node *
IntersectBoundsBuilderOne (node *arg_node, info *arg_info, node *producerPart,
                           int boundnum, node *ivmin, node *ivmax)
{
    node *producerGenerator;
    node *resavis;
    node *fncall;
    node *bound1;
    pattern *pat;
    node *gen = NULL;
    char *fun;
    int shp;

    DBUG_ENTER ("IntersectBoundsBuilderOne");

    producerGenerator = (boundnum == 1)
                          ? GENERATOR_BOUND1 (PART_GENERATOR (producerPart))
                          : GENERATOR_BOUND2 (PART_GENERATOR (producerPart));
    shp = SHgetUnrLen (ARRAY_FRAMESHAPE (producerGenerator));
    bound1 = GENERATOR_BOUND1 (PART_GENERATOR (INFO_CONSUMERPART (arg_info)));
    DBUG_ASSERT (N_array == NODE_TYPE (bound1), "Expected N_array bound1");
    bound1 = AWLFIflattenExpression (DUPdoDupTree (bound1), &INFO_VARDECS (arg_info),
                                     &INFO_PREASSIGNS (arg_info),
                                     TYmakeAKS (TYmakeSimpleType (T_int),
                                                SHcreateShape (1, shp)));

    pat = PMvar (1, PMAgetNode (&gen), 0);
    if (PMmatchFlatSkipExtrema (pat, producerGenerator)) {
        producerGenerator = gen;
    }

    fun = (boundnum == 1) ? "partitionIntersectMax" : "partitionIntersectMin";

    DBUG_ASSERT (N_array == NODE_TYPE (producerGenerator),
                 "Expected N_array producerGenerator");
    producerGenerator
      = WLSflattenBound (DUPdoDupTree (producerGenerator), &INFO_VARDECS (arg_info),
                         &INFO_PREASSIGNS (arg_info));

    fncall = DSdispatchFunCall (NSgetNamespace ("sacprelude"), fun,
                                TCcreateExprsChainFromAvises (4, producerGenerator,
                                                              ID_AVIS (ivmin),
                                                              ID_AVIS (ivmax), bound1));
    resavis = AWLFIflattenExpression (fncall, &INFO_VARDECS (arg_info),
                                      &INFO_PREASSIGNS (arg_info),
                                      TYmakeAKS (TYmakeSimpleType (T_int),
                                                 SHcreateShape (1, shp)));
    pat = PMfree (pat);

    DBUG_RETURN (resavis);
}

/** <!--********************************************************************-->
 *
 * @fn node *IntersectNullComputationBuilder( node *idxavismin,
 *                                            node *idxavismax,
 *                                            node *bound1, node *bound2,
 *                                            info *arg_info)
 *
 * @brief:  Emit symbiotic expression to determine if intersection
 *          of index vector set and partition bounds is null.
 *
 * @params: idxavismin: AVIS_MIN( consumerWL partition index vector)
 * @params: idxavismax: AVIS_MAX( consumerWL partition index vector)
 * @params: bound1: N_avis of GENERATOR_BOUND1 of producerWL partition.
 * @params: bound2: N_avis of GENERATOR_BOUND2 of producerWL partition.
 * @params: arg_info: your basic arg_info node
 *
 * @result: N_avis node of generated computation's boolean result.
 *
 *****************************************************************************/

static node *
IntersectNullComputationBuilder (node *idxavismin, node *idxavismax, node *bound1,
                                 node *bound2, info *arg_info)
{
    node *fncall;
    node *resavis;
    int shp;

    DBUG_ENTER ("IntersectNullComputationBuilder");

    DBUG_ASSERT (N_avis == NODE_TYPE (bound1), "Expected N_avis bound1");
    DBUG_ASSERT (N_avis == NODE_TYPE (bound2), "Expected N_avis bound2");

    fncall = DSdispatchFunCall (NSgetNamespace ("sacprelude"), "isPartitionIntersectNull",
                                TCcreateExprsChainFromAvises (4, ID_AVIS (idxavismin),
                                                              ID_AVIS (idxavismax),
                                                              bound1, bound2));

    shp = SHgetUnrLen (TYgetShape (AVIS_TYPE (bound1)));
    resavis = AWLFIflattenExpression (fncall, &INFO_VARDECS (arg_info),
                                      &INFO_PREASSIGNS (arg_info),
                                      TYmakeAKS (TYmakeSimpleType (T_bool),
                                                 SHcreateShape (1, shp)));

    DBUG_RETURN (resavis);
}

/** <!--********************************************************************-->
 *
 * @fn node *IntersectBoundsBuilder( node *arg_node, info *arg_info,
 *                                   node *producerwlPart, node *ivavis)
 *
 * @brief Build a set of expressions for intersecting the bounds of
 *        all producerWL partitions with consumerWL index set, of the form:
 *
 *           sel((k*iv) + ivoffset, producerWL)
 *
 *        We traverse all producerWL partitions, building a set of
 *        intersect calcuations for each of them.
 *
 * @params arg_node: The _sel_VxA( iv, producerWL).
 * @params arg_info.
 * @params boundnum: 1 for bound1,        2 for bound2
 * @params ivavis: the N_avis of the index vector used by the sel() operation.
 * @return An N_exprs node containing the ( 2 * # producerwlPart partitions)
 *         intersect expressions, in the form:
 *           p0bound1, p0bound2, p0intlo, p0inthi, p0nullint,
 *           p1bound1, p1bound2, p1intlo, p1inthi, p1nullint,
 *           ...
 *
 *****************************************************************************/

static node *
IntersectBoundsBuilder (node *arg_node, info *arg_info, node *ivavis)
{
    node *expn = NULL;
    node *partn;
    node *curavis;
    node *g1;
    node *g2;
    node *gen1 = NULL;
    node *gen2 = NULL;
    pattern *pat1;
    pattern *pat2;

    DBUG_ENTER ("IntersectBoundsBuilder");

    partn = WITH_PART (AWLFIgetWlWith (INFO_PRODUCERWL (arg_info)));
    pat1 = PMvar (1, PMAgetNode (&gen1), 0);
    pat2 = PMvar (1, PMAgetNode (&gen2), 0);

    while (NULL != partn) {
        g1 = GENERATOR_BOUND1 (PART_GENERATOR (partn));
        if (PMmatchFlatSkipExtrema (pat1, g1)) {
            g1 = gen1;
        }
        g1 = WLSflattenBound (DUPdoDupTree (g1), &INFO_VARDECS (arg_info),
                              &INFO_PREASSIGNS (arg_info));

        g2 = GENERATOR_BOUND2 (PART_GENERATOR (partn));
        if (PMmatchFlatSkipExtrema (pat2, g2)) {
            g2 = gen2;
        }
        g2 = WLSflattenBound (DUPdoDupTree (g2), &INFO_VARDECS (arg_info),
                              &INFO_PREASSIGNS (arg_info));

        expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (g1), NULL));
        expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (g2), NULL));

        curavis = IntersectBoundsBuilderOne (arg_node, arg_info, partn, 1,
                                             AVIS_MIN (ivavis), AVIS_MAX (ivavis));
        expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (curavis), NULL));

        curavis = IntersectBoundsBuilderOne (arg_node, arg_info, partn, 2,
                                             AVIS_MIN (ivavis), AVIS_MAX (ivavis));
        expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (curavis), NULL));
        curavis = IntersectNullComputationBuilder (AVIS_MIN (ivavis), AVIS_MAX (ivavis),
                                                   g1, g2, arg_info);
        expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (curavis), NULL));
#ifdef DEADCODE
        expn = TCappendExprs (expn, TBmakeExprs (BuildIVPV (arg_node, arg_info), NULL));
#endif // DEADCODE

        partn = PART_NEXT (partn);
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (expn);
}

/** <!--********************************************************************-->
 *
 * @fn node *attachIntersectCalc( node *arg_node, info *arg_info)
 *
 * @brief  We are looking at the N_prf for:
 *
 *            z = _sel_VxA_( iv, producerWL);
 *         within the consumerWL, and idx now has extrema attached to it.
 *         We are now in a position to compute the intersection
 *         between idx's index set and that of the producerWL
 *         partitions.
 *
 *         We create new iv' from idx, to hold the result
 *         of the intersect computations that we build here.
 *
 *      See IntersectBoundsBuilderOne for details.
 *
 * @return the N_avis for the newly created iv' node.
 *
 *****************************************************************************/

static node *
attachIntersectCalc (node *arg_node, info *arg_info)
{
    node *ivavis;
    node *ivpavis;
    node *ivassign;
    int ivshape;
    node *intersectcalc;
    node *args;

    DBUG_ENTER ("attachIntersectCalc");

    DBUG_PRINT ("AWLFI", ("Inserting attachextrema computations"));

    /* Generate expressions for lower-bound intersection and
     * upper-bound intersection calculation.
     */
    ivavis = ID_AVIS (PRF_ARG1 (arg_node));
    intersectcalc = IntersectBoundsBuilder (arg_node, arg_info, ivavis);
    args = TBmakeExprs (TBmakeId (ivavis), NULL);
    args = TCappendExprs (args, intersectcalc);

    ivshape = SHgetUnrLen (TYgetShape (AVIS_TYPE (ivavis)));
    ivpavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (ivavis)),
                          TYeliminateAKV (AVIS_TYPE (ivavis)));

    INFO_VARDECS (arg_info) = TBmakeVardec (ivpavis, INFO_VARDECS (arg_info));
    ivassign = TBmakeAssign (TBmakeLet (TBmakeIds (ivpavis, NULL),
                                        TBmakePrf (F_noteintersect, args)),
                             NULL);
    INFO_PREASSIGNS (arg_info) = TCappendAssign (INFO_PREASSIGNS (arg_info), ivassign);
    AVIS_SSAASSIGN (ivavis) = ivassign;

    PART_ISCONSUMERPART (INFO_CONSUMERPART (arg_info)) = TRUE;

    DBUG_RETURN (ivpavis);
}

/** <!--********************************************************************-->
 *
 * @fn bool AWLFIisSingleOpWL( node *arg_node)
 *
 * @brief: predicate for determining if node is single-op WL
 *
 * @param: arg_node: an N_with
 *
 * @return: TRUE if only one result from WL
 *
 *****************************************************************************/
bool
AWLFIisSingleOpWL (node *arg_node)
{
    bool z;

    DBUG_ENTER ("AWLFIisSingleOpWL");

    switch (NODE_TYPE (WITH_WITHOP (arg_node))) {
    default:
        z = FALSE;
        DBUG_ASSERT (FALSE, "WITHOP confusion");
        break;
    case N_genarray:
        z = (NULL == GENARRAY_NEXT (WITH_WITHOP (arg_node)));
        break;
    case N_modarray:
        z = (NULL == MODARRAY_NEXT (WITH_WITHOP (arg_node)));
        break;
    case N_fold:
        z = (NULL == FOLD_NEXT (WITH_WITHOP (arg_node)));
        break;
    case N_spfold:
        z = (NULL == SPFOLD_NEXT (WITH_WITHOP (arg_node)));
        break;
    case N_propagate:
        z = (NULL == PROPAGATE_NEXT (WITH_WITHOP (arg_node)));
        break;
    case N_break:
        z = (NULL == BREAK_NEXT (WITH_WITHOP (arg_node)));
        break;
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool checkProducerWLFoldable( node *arg_node, info *arg_info)
 *
 * @brief We are looking at _sel_VxA_(idx, producerWL), contained
 *        within a consumerWL. We want to determine if
 *        producerWL is a WL that is a possible candidate for having some
 *        partition of itself folded into the consumerWL that
 *        contains the _sel_ expression.
 *
 *        This function concerns itself only with the characteristics
 *        of the entire producerWL: partition-dependent characteristics
 *        are determined by the AWLF phase later on.
 *
 *        The requirements for folding are:
 *
 *           - producerWL is the result of a WL.
 *
 *           - producerWL operator is a genarray or modarray.
 *
 *           - producerWL is a single-operator WL.
 *
 *           - producerWL has an SSAASSIGN (means that WITH_IDs are NOT legal),
 *
 *           - producerWL is referenced only by the consumerWL.
 *             This is not strictly needed; it is only there
 *             to avoid potentially computing the same
 *             producerWL element more than once. FIXME: wl_needcount.c
 *             changes will relax this and restriction and base it
 *             on producerWL element computation cost.
 *
 *           - producerWL has a DEFDEPTH value (which I don't understand yet).
 *
 *           - consumerWL and producerWL generator bounds are
 *             the same shape.
 *
 *        There is an added requirement, that the index set of
 *        the consumerWL partition match, or be a subset, of the
 *        producerWL partition index set.
 *
 *        The expressions hanging from the attachextrema inserted
 *        by attachExtremaCalc are intended to determine if this
 *        requirement is met. CF, CVP, and other optimizations
 *        should simplify those expressions and give AWLF
 *        the information it needs to determine if the index
 *        set requirements are met.
 *
 *
 * @param _sel_VxA_( idx, producerWL)
 * @result If some partition of the producerWL may be a legal
 *         candidate for folding into the consumerWL, return true.
 *         Else false.
 *
 *****************************************************************************/
static bool
checkProducerWLFoldable (node *arg_node, info *arg_info)
{
    node *p;
    bool z;

    DBUG_ENTER ("checkProducerWLFoldable");

    p = AWLFIgetWlWith (INFO_PRODUCERWL (arg_info));
    z = NULL != p;
    if (z) {
        z = (AWLFIisSingleOpWL (p)) && (noDefaultPartition (p))
            && (WITHOP_NEXT (WITH_WITHOP (p)) == NULL)
            && ((NODE_TYPE (WITH_WITHOP (p)) == N_genarray)
                || (NODE_TYPE (WITH_WITHOP (p)) == N_modarray));

        if (z) {
            DBUG_PRINT ("AWLFI",
                        ("ProducerWL:%s is suitable for folding; WITH_REFERENCED_FOLD=%d",
                         AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))),
                         WITH_REFERENCED_FOLD (p)));
        } else {
            DBUG_PRINT ("AWLFI", ("ProducerWL %s is not suitable for folding.",
                                  AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node)))));
        }
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool checkConsumerWLFoldable( node *arg_node, info *arg_info)
 *
 * @brief We are looking at _sel_VxA_(iv, producerWL), contained
 *        within a consumerWL. We want to determine if
 *        the consumerWL is acceptable to have something folded into it.
 *
 *        We deem the prf foldable if iv derives directly from
 *        an _attachintersect_, or if iv has extrema.
 *
 *        This may not be enough to guarantee foldability, but
 *        without extrema, we're stuck.
 *
 * @param _sel_VxA_( iv, producerWL) arg_node.
 *
 * @result True if the consumerWL (and the indexing expression)
 *         are acceptable for having another WL folded into it,
 *         else false.
 *
 *****************************************************************************/
static bool
checkConsumerWLFoldable (node *arg_node, info *arg_info)
{
    node *iv = NULL;
    node *ivavis;
    bool z;

    DBUG_ENTER ("checkConsumerWLFoldable");

    iv = PRF_ARG1 (arg_node);
    ivavis = ID_AVIS (iv);
    /* iv has extrema or F_attachintersect */
    z = ((NULL != AVIS_MIN (ivavis)) && (NULL != INFO_CONSUMERWL (arg_info))
         && (NULL != AVIS_MAX (ivavis)))
        || isPrfArg1AttachIntersect (arg_node);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool checkBothFoldable( node *arg_node, info *arg_info)
 *
 * @brief Make any early checks we can that may show that
 *        the producerWL and consumerWL are not foldable.
 *
 *        We check that the shape of the generator bound of the producerWL
 *        matches the shape of the index vector in the consumerWL.
 *
 * @param N_prf: _sel_VxA_( idx, producerWL) arg_node.
 * @result True if the consumerWL and producerWL
 *         shapes are conformable for folding.
 *
 *****************************************************************************/
static bool
checkBothFoldable (node *arg_node, info *arg_info)
{
    node *bp;
    node *bc;
    node *pwl;
    int lev;
    int xrhob;
    int xrhoc;
    bool z = FALSE;

    DBUG_ENTER ("checkBothFoldable");

    pwl = AWLFIgetWlWith (INFO_PRODUCERWL (arg_info));
    if (NULL != pwl) {
        bp = GENERATOR_BOUND1 (PART_GENERATOR (WITH_PART (pwl)));
        xrhob = SHgetUnrLen (ARRAY_FRAMESHAPE (bp));
        bc = AVIS_MIN (ID_AVIS (PRF_ARG1 (arg_node)));
        if (NULL != bc) {
            xrhoc = SHgetUnrLen (TYgetShape (AVIS_TYPE (ID_AVIS (bc))));
            z = xrhob == xrhoc;
        }
    }

    /* Both producerWL and consumerWL must be at same nesting level */
    lev = 1 + AVIS_DEFDEPTH (ID_AVIS (INFO_PRODUCERWL (arg_info)));
    z = z && (lev == INFO_LEVEL (arg_info));

    if (z) {
        DBUG_PRINT ("AWLFI",
                    ("Producer with-loop %s is foldable", INFO_PRODUCERWL (arg_info)));
    } else {
        DBUG_PRINT ("AWLFI", ("Producer with-loop %s is not foldable",
                              INFO_PRODUCERWL (arg_info)));
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIfundef(node *arg_node, info *arg_info)
 *
 * @brief applies AWLFI to a given fundef.
 *
 *****************************************************************************/
node *
AWLFIfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ("AWLFIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("AWLFI", ("Algebraic-With-Loop-Folding Inference in %s %s begins",
                              (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                              FUNDEF_NAME (arg_node)));

        old_onefundef = INFO_ONEFUNDEF (arg_info);
        INFO_ONEFUNDEF (arg_info) = FALSE;

        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node, TR_awlfi);

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            /* If new vardecs were made, append them to the current set */
            if (INFO_VARDECS (arg_info) != NULL) {
                BLOCK_VARDEC (FUNDEF_BODY (arg_node))
                  = TCappendVardec (INFO_VARDECS (arg_info),
                                    BLOCK_VARDEC (FUNDEF_BODY (arg_node)));
                INFO_VARDECS (arg_info) = NULL;
            }

            FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

            INFO_ONEFUNDEF (arg_info) = old_onefundef;
        }

        DBUG_PRINT ("AWLFI", ("Algebraic-With-Loop-Folding Inference in %s %s ends",
                              (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                              FUNDEF_NAME (arg_node)));
    }

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node AWLFIassign( node *arg_node, info *arg_info)
 *
 * @brief performs a top-down traversal.
 *        For a foldable WL, arg_node is x = _sel_VxA_(iv, producerWL).
 *
 *****************************************************************************/
node *
AWLFIassign (node *arg_node, info *arg_info)
{
    node *let;

    DBUG_ENTER ("AWLFIassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    let = ASSIGN_INSTR (arg_node);
    if ((N_let == NODE_TYPE (let)) && (N_with == NODE_TYPE (LET_EXPR (let)))
        && (INFO_PREASSIGNSWL (arg_info) != NULL)) {
        arg_node = TCappendAssign (INFO_PREASSIGNSWL (arg_info), arg_node);
        INFO_PREASSIGNSWL (arg_info) = NULL;
    }

    /*
     * Top-down traversal
     */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIwith( node *arg_node, info *arg_info)
 *
 * @brief applies AWLFI to a with-loop in a top-down manner.
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
AWLFIwith (node *arg_node, info *arg_info)
{
    info *old_arg_info;

    DBUG_ENTER ("AWLFIwith");

    old_arg_info = arg_info;

    arg_info = MakeInfo (INFO_FUNDEF (arg_info));
    INFO_LEVEL (arg_info) = INFO_LEVEL (old_arg_info) + 1;
    INFO_VARDECS (arg_info) = INFO_VARDECS (old_arg_info);
    INFO_CONSUMERWL (arg_info) = AWLFIgetWlWith (arg_node);
    INFO_ONEFUNDEF (arg_info) = INFO_ONEFUNDEF (old_arg_info);

    DBUG_PRINT ("AWLFI", ("Resetting WITH_REFERENCED_CONSUMERWL, etc."));
    WITH_REFERENCED_FOLD (arg_node) = 0;
    WITH_REFERENCED_CONSUMERWL (arg_node) = NULL;
    WITH_REFERENCES_FOLDED (arg_node) = 0;

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    INFO_VARDECS (old_arg_info) = INFO_VARDECS (arg_info);

    arg_info = FreeInfo (arg_info);
    arg_info = old_arg_info;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIpart( node *arg_node, info *arg_info)
 *
 * @brief Traverse each partition of a WL.
 *
 *****************************************************************************/
node *
AWLFIpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIpart");

    INFO_CONSUMERPART (arg_info) = arg_node;
    CODE_CBLOCK (PART_CODE (arg_node))
      = TRAVdo (CODE_CBLOCK (PART_CODE (arg_node)), arg_info);
    INFO_CONSUMERPART (arg_info) = NULL;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIids( node *arg_node, info *arg_info)
 *
 * @brief set current With-Loop level as ids defDepth attribute
 *
 *****************************************************************************/
node *
AWLFIids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIids");

    AVIS_DEFDEPTH (IDS_AVIS (arg_node)) = INFO_LEVEL (arg_info);
    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *  (cloned from SSAWLI)
 *
 * function:
 *   node *AWLFIid(node *arg_node, info *arg_info)
 *
 * description:
 *   If this Id is a reference to a WL (N_with) we want to increment
 *   the number of references to the potential producerWL (WITH_REFERENCED_FOLD).
 *   We want to end up with WITH_REFERENCED_FOLD counting only
 *   references from a single WL. Hence, the checking on
 *   WITH_REFERENCED_CONSUMERWL
 *   AWLF will disallow folding if WITH_REFERENCED_FOLD != AVIS_NEEDCOUNT.
 *
 ******************************************************************************/
node *
AWLFIid (node *arg_node, info *arg_info)
{
    node *p;

    DBUG_ENTER ("AWLFIid");
    /* get the definition assignment via the AVIS_SSAASSIGN backreference */
#ifdef NOISY
    DBUG_PRINT ("AWLFI", ("AWLFIid looking at %s", AVIS_NAME (ID_AVIS (arg_node))));
#endif // NOISY
    p = INFO_CONSUMERWL (arg_info);
    if ((NULL != p) && (NULL == WITH_REFERENCED_CONSUMERWL (p))) {
        /* First reference to this WL. */
        WITH_REFERENCED_CONSUMERWL (p) = INFO_CONSUMERWL (arg_info);
        WITH_REFERENCED_FOLD (p) = 0;
        DBUG_PRINT ("AWLFI", ("AWLFIid found first reference to %s",
                              AVIS_NAME (ID_AVIS (arg_node))));
    }

    /*
     * arg_node describes a WL, so
     * WITH_REFERENCED_FOLD( p) may have to be
     * incremented
     */
    if ((NULL != p) && (NULL != INFO_CONSUMERWL (arg_info))
        && (WITH_REFERENCED_CONSUMERWL (p) == INFO_CONSUMERWL (arg_info))) {
        (WITH_REFERENCED_FOLD (p))++;
        DBUG_PRINT ("AWLFI", ("AWLFIid incrementing WITH_REFERENCED_FOLD(%s) = %d",
                              AVIS_NAME (ID_AVIS (arg_node)), WITH_REFERENCED_FOLD (p)));
    } else {
#ifdef NOISY
        DBUG_PRINT ("AWLFI", ("AWLFIid %s is not defined by a WL",
                              AVIS_NAME (ID_AVIS (arg_node))));
#endif // NOISY
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIprf( node *arg_node, info *arg_info)
 *
 * @brief
 *   Examine all _sel_VxA_(idx, producerWL) primitives to see if
 *   we may be able to fold producerWL here, assuming that
 *   the _sel_ is within a potential consumerWL.
 *
 *   We don't find out if all the conditions for folding can
 *   be met until this phase completes, so AWLF makes the
 *   final decision on folding.
 *
 *   When we do encounter an eligible sel() operation,
 *   with extrema available on PRF_ARG1, we construct
 *   an intersect computation between the now-available index
 *   set of idx, and each partition of the producerWL.
 *   These are attached to the sel() via an F_attachintersect
 *   guard.
 *
 *****************************************************************************/
node *
AWLFIprf (node *arg_node, info *arg_info)
{
    node *z;

    DBUG_ENTER ("AWLFIprf");

    if ((INFO_CONSUMERPART (arg_info) != NULL) && (PRF_PRF (arg_node) == F_sel_VxA)
        && (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id)
        && (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id)) {

        INFO_PRODUCERWL (arg_info) = AWLFIfindWlId (PRF_ARG2 (arg_node));
        INFO_PRODUCERWLFOLDABLE (arg_info)
          = checkConsumerWLFoldable (arg_node, arg_info)
            && checkProducerWLFoldable (arg_node, arg_info)
            && checkBothFoldable (arg_node, arg_info);

        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

        /* Maybe attach intersect calculations now. */
        if ((INFO_PRODUCERWLFOLDABLE (arg_info))
            && (!isPrfArg1AttachIntersect (arg_node))) {
            z = attachIntersectCalc (arg_node, arg_info);
            FREEdoFreeNode (PRF_ARG1 (arg_node));
            PRF_ARG1 (arg_node) = TBmakeId (z);
            DBUG_PRINT ("AWLFI", ("AWLFIprf inserted F_attachintersect at _sel_VxA_"));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFIcond(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse conditional parts in the given order.
 *
 ******************************************************************************/
node *
AWLFIcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIcond");

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THENINSTR (arg_node) = TRAVdo (COND_THENINSTR (arg_node), arg_info);
    COND_ELSEINSTR (arg_node) = TRAVdo (COND_ELSEINSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFImodarray(node *arg_node, info *arg_info)
 *
 * description:
 *   if op is modarray( producerWL), we increment
 *   WITH_REFERENCED_FOLD for the producerWL.
 *
 ******************************************************************************/
node *
AWLFImodarray (node *arg_node, info *arg_info)
{
    node *wl;

    DBUG_ENTER ("AWLFImodarray");

    arg_node = TRAVcont (arg_node, arg_info);

    if (N_modarray == NODE_TYPE (arg_node)) {
        INFO_PRODUCERWL (arg_info) = AWLFIfindWlId (MODARRAY_ARRAY (arg_node));
        wl = AWLFIgetWlWith (INFO_PRODUCERWL (arg_info));
        (WITH_REFERENCED_FOLD (wl))++;
        DBUG_PRINT ("AWLFI", ("AWLFImodarray: WITH_REFERENCED_FOLD(%s) = %d",
                              AVIS_NAME (ID_AVIS (MODARRAY_ARRAY (arg_node))),
                              WITH_REFERENCED_FOLD (wl)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFIlet(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
AWLFIlet (node *arg_node, info *arg_info)
{
    node *oldlet;

    DBUG_ENTER ("AWLFIlet");

    oldlet = INFO_LET (arg_info);
    INFO_LET (arg_info) = arg_node;
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LET (arg_info) = oldlet;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFIblock(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
AWLFIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIblock");
    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFIavis(node *arg_node, info *arg_info)
 *
 * description:
 *    Zero the AVIS_NEEDCOUNT values before traversing any code.
 *
 ******************************************************************************/
node *
AWLFIavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIavis");

    AVIS_NEEDCOUNT (arg_node) = 0;
    AVIS_ISWLFOLDED (arg_node) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFIfuncond( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
AWLFIfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIfuncond");

    FUNCOND_IF (arg_node) = TRAVopt (FUNCOND_IF (arg_node), arg_info);
    FUNCOND_THEN (arg_node) = TRAVopt (FUNCOND_THEN (arg_node), arg_info);
    FUNCOND_ELSE (arg_node) = TRAVopt (FUNCOND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFIwhile( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
AWLFIwhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIwhile");

    WHILE_COND (arg_node) = TRAVopt (WHILE_COND (arg_node), arg_info);
    WHILE_BODY (arg_node) = TRAVopt (WHILE_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Algebraic with loop folding inference -->
 *****************************************************************************/
