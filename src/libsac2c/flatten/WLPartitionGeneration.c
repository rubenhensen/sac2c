/**
 *
 * $Id$
 *
 * @file WLPartition Generation.c
 *
 *
 * In this traversal AKS and AKD genarray/modarray withloops
 * with non-full partitions of the considered array are
 * transformed into withloops with full partitions.
 *
 * Ex. of AKS withloop:
 *    A = with(iv)
 *         ([3] <= iv < [6])
 *         {res = ...;
 *         }: res
 *        genarray([9]);
 *
 * is transformed into this form, with flattened generators:
 *
 *    A = with(iv)
 *         t3 = [3];
 *         t6 = [6];
 *         t0 = [0];
 *         t7 = [7];
 *         t9 = [9];
 *         (t3 <= iv < t6)
 *         {res = ...;
 *         }: res
 *         (t0 <= iv < t3)
 *         {res = 0;
 *         }: res
 *         (t7 <= iv < t9)
 *         {res = 0;
 *         }: res
 *        genarray([9]);
 *
 * The modification of AKS withloops was already enforced by
 * WLT a subphase of WithloopFolding but is here extended for AKDs.
 *
 * Ex. of AKD withloop:
 *   int[1] a;
 *   int[1] b;
 *   int[1] c;
 *    B = with(iv)
 *         (a < iv <= b)
 *         {res = ...;
 *         }: res
 *        genarray(c);
 *
 * is transformed into
 *
 *    A = with(iv)
 *         a0 = [a[0]];
 *         b0 = [b[0]];
 *         c0 = [c[0]];
 *         z0 = [0];
 *         (a0 <= iv < b0)
 *         {res = ...;
 *         }: res
 *         (z0 <= iv < a0)
 *         {res = 0;
 *         }: res
 *         (b0 <= iv < c0)
 *         {res = 0;
 *         }: res
 *        genarray(c0);
 *
 */

#include "new_types.h"
#include "new_typecheck.h"
#include "type_utils.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "shape.h"
#include "free.h"
#include "DupTree.h"
#include "ctinfo.h"
#include "globals.h"
#include "dbug.h"
#include "traverse.h"
#include "constants.h"
#include "wlanalysis.h"
#include "LookUpTable.h"
#include "wldefaultpartition.h"
#include "WLPartitionGeneration.h"
#include "pattern_match.h"

typedef enum { SP_mod, SP_func } sub_phase_t;

/**
 * INFO structure
 */
struct INFO {
    node *wl;
    node *fundef;
    node *let;
    node *nassigns;
    sub_phase_t subphase;
};

/*******************************************************************************
 *  Usage of arg_info:
 *  - node : WL      : reference to base node of current WL (N_with)
 *  - node : FUNDEF  : pointer to last fundef node. needed to access vardecs.
 *  - node : LET     : pointer to N_let node of current WL.
 *                     LET_EXPR(ID) == INFO_WL.
 *  - node : NASSIGNS: pointer to a list of new assigns, which where needed
 *                     to build structural constants and had to be inserted
 *                     in front of the considered with-loop.
 *
 ******************************************************************************/
#define INFO_WL(n) (n->wl)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LET(n) (n->let)
#define INFO_NASSIGNS(n) (n->nassigns)
#define INFO_SUBPHASE(n) (n->subphase)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_WL (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_LET (result) = NULL;
    INFO_NASSIGNS (result) = NULL;
    INFO_SUBPHASE (result) = SP_mod;

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
 *
 * @fn node *CreateArrayOfShapeSels( node *array, int dim, info *arg_info)
 *
 *   @brief expects (array) to point to an identifier and generates a
 *          structural constant of shape selections
 *
 *          [ idx_shape_sel( 0, A), ..., idx_shape_sel( dim-1, A)]
 *
 *   @param  node *array   :  N_id
 *           int dim       :
 *           info *arg_info:  info node, will contain flattened code
 *   @return node *        :  An array of length dim containing shape selections
 *
 ******************************************************************************/

static node *
CreateArrayOfShapeSels (node *array, int dim, info *arg_info)
{
    node *res = NULL;
    node *firstarray = NULL;
    int i;

    DBUG_ENTER ("CreateArrayOfShapeSels");

    DBUG_ASSERT ((NODE_TYPE (array) == N_id),
                 "CreateArrayOfShapeSels not called with N_id");

    /* Find first array in chain with same shape as array */
    firstarray = PMOshapePrimogenitor (array);

    for (i = dim - 1; i >= 0; i--) {
        node *sel_avis;

        sel_avis = TBmakeAvis (TRAVtmpVarName (ID_NAME (firstarray)),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (sel_avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

        INFO_NASSIGNS (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (sel_avis, NULL),
                                     TCmakePrf2 (F_idx_shape_sel, TBmakeNum (i),
                                                 TBmakeId (ID_AVIS (firstarray)))),
                          INFO_NASSIGNS (arg_info));

        /*
         * set correct backref to defining assignment
         */
        AVIS_SSAASSIGN (sel_avis) = INFO_NASSIGNS (arg_info);

        /*
         * create element of structural constant
         */
        res = TBmakeExprs (TBmakeId (sel_avis), res);
    }

    /*
     * create structural constant
     */
    res = TCmakeIntVector (res);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *NewIds( node *nd, node *fundef)
 *
 *   @brief creates new N_id nodes.
 *
 *   @param  node *nd       :  if N_id node, the new name and type will be
 *                             inferred from it. Otherwise, the type
 *                             will be integer scalar.
 *           node *fundef   :  N_fundef
 *   @return node *         :  new Ids
 ******************************************************************************/

static node *
NewIds (node *nd, node *fundef)
{
    node *vardec, *_ids;
    char *nvarname;

    DBUG_ENTER ("NewIds");

    DBUG_ASSERT ((nd != NULL) && ((NODE_TYPE (nd) == N_id) || (NODE_TYPE (nd) == N_num)),
                 "NULL N_id or not N_id/N_num");

    if (NODE_TYPE (nd) == N_id) {
        nvarname = TRAVtmpVarName (ID_NAME (nd));
        _ids
          = TBmakeIds (TBmakeAvis (nvarname, TYeliminateAKV (AVIS_TYPE (ID_AVIS (nd)))),
                       NULL);

        vardec = TBmakeVardec (IDS_AVIS (_ids), NULL);

    } else {
        nvarname = TRAVtmpVar ();
        _ids = TBmakeIds (TBmakeAvis (nvarname, TYmakeAKS (TYmakeSimpleType (T_int),
                                                           SHmakeShape (0))),
                          NULL);

        vardec = TBmakeVardec (IDS_AVIS (_ids), NULL);
    }

    fundef = TCaddVardecs (fundef, vardec);

    DBUG_RETURN (_ids);
}

/** <!--********************************************************************-->
 *
 * @fn node CreateEntryFlatArray(int entry, int number)
 *
 *   @brief creates an flat array with 'number' elements consisting of
 *          'entry'
 *
 *   @param  int entry   : entry within the new array
 *           int number  : number of elements in the new array
 *   @return node *      : N_array
 ******************************************************************************/

static node *
CreateEntryFlatArray (int entry, int number)
{
    node *tmp;
    int i;

    DBUG_ENTER ("CreateOneArray");

    tmp = NULL;
    for (i = 0; i < number; i++) {
        tmp = TBmakeExprs (TBmakeNum (entry), tmp);
    }
    tmp = TCmakeIntVector (tmp);

    DBUG_RETURN (tmp);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateNewPart(node * lb, node *ub, node * step, node *width,
 *                        node *withid, node *coden, info *arg_info)
 *
 *   @brief creates a new N_part.
 *
 *   If the compiler is operating in ssaiv mode, the
 *   withids will be renamed into SSA form, and any references
 *   in the code block will be changed appropriately.
 *
 *   @param  node *lb           : lower bound of new part
 *           node *ub           : upper bound of new part
 *           node *step, *width : step, width of new part
 *           node *withid       : withid of new part
 *           node *coden        : Pointer of N_code node where the
 *                                new generator shall point to.
 *           info *arg_info     : local information block
 *   @return node *             : N_part
 ******************************************************************************/

static node *
CreateNewPart (node *lb, node *ub, node *step, node *width, node *withid, node *coden,
               info *arg_info)
{
    node *genn, *partn;
    node *newwithid;
    node *newcoden;
    node *lbavis;
    node *ubavis;
    node *lbid;
    node *ubid;
    node *nas;
    lut_t *lut;
    int genshape;

    DBUG_ENTER ("CreateNewPart");

    if (global.ssaiv) {
        /* Create flattened WL lower bound */
        genshape = SHgetUnrLen (TYgetShape (IDS_NTYPE (WITHID_VEC (withid))));
        lb = CreateEntryFlatArray (0, genshape);
        lbavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                       SHcreateShape (1, genshape)));
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (lbavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
        nas
          = TBmakeAssign (TBmakeLet (TBmakeIds (lbavis, NULL), DUPdoDupTree (lb)), NULL);
        AVIS_SSAASSIGN (lbavis) = nas;
        INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), nas);
        lbid = TBmakeId (lbavis);

        /* Create flattened WL upper bound */
        ubavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                       SHcreateShape (1, genshape)));
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (ubavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
        nas
          = TBmakeAssign (TBmakeLet (TBmakeIds (ubavis, NULL), DUPdoDupTree (ub)), NULL);
        AVIS_SSAASSIGN (ubavis) = nas;
        INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), nas);
        ubid = TBmakeId (ubavis);

        /* TODO: Probably should treat STEP and WIDTH the same as BOUNDs */

        genn = TBmakeGenerator (F_wl_le, F_wl_lt, lbid, ubid, DUPdoDupTree (step),
                                DUPdoDupTree (width));
        lut = LUTgenerateLut ();
        newwithid = DUPdoDupTreeLutSsa (withid, lut, INFO_FUNDEF (arg_info));
        newcoden = DUPdoDupTreeLutSsa (coden, lut, INFO_FUNDEF (arg_info));
        LUTremoveLut (lut);
        partn = TBmakePart (newcoden, newwithid, genn);
        CODE_NEXT (newcoden) = WITH_CODE (INFO_WL (arg_info));
        WITH_CODE (INFO_WL (arg_info)) = newcoden;
        CODE_INC_USED (newcoden);
    } else {
        genn = TBmakeGenerator (F_wl_le, F_wl_lt, DUPdoDupTree (lb), DUPdoDupTree (ub),
                                DUPdoDupTree (step), DUPdoDupTree (width));
        partn = TBmakePart (coden, DUPdoDupTree (withid), genn);
        CODE_INC_USED (coden);
    }

    DBUG_RETURN (partn);
}

/** <!--********************************************************************-->
 *
 * @fn int WLPGnormalizeStepWidth(node **step, node **width)
 *
 *   @brief normalizes step and width. There are several forbidden and
 *          ambiguous combinations of bounds, step (s) and width (w).
 *
 *          allowed is the following (for each component):
 *          - w < s and s > 1 or
 *          - w == 1 and s == 1
 *
 *          transformations:
 *          - w == s: set w = s = 1 (componentwise)
 *          - vector s == 1 and vector w == 1: set both to NULL
 *          - vector s != NULL and vector w == NULL: create vector w = 1.
 *
 *
 *          errors:
 *          - w > s        (error no 1)
 *          - 1 > w        (error no 2)
 *          - w without s  (error no 3)
 *
 *   @param  node **step  :  step vector
 *           node **width :  width vector
 *   @return int         :  returns 0 if no error was detected, else error no
 *                          (see above).
 ******************************************************************************/

int
WLPGnormalizeStepWidth (node **step, node **width)
{
    node *stp, *wth, *array;
    node *stpar = NULL;
    node *wthar = NULL;
    pattern *pat;
    int stpnum, wthnum, veclen;
    int error = 0, is_1 = 1;

    DBUG_ENTER ("WLPGnormalizeStepWidth");

    if ((*width) != NULL && (*step) == NULL) {
        error = 3;
    } else {
        if ((*step) != NULL) {
            pat = PMarray (1, PMAgetNode (&array), 1, PMskip (0));

            if ((*width) == NULL) {
                /*  create width with constant 1 */
                if (PMmatchFlat (pat, *step)) {
                    stpar = array;
                } else {
                    DBUG_ASSERT (FALSE, ("WLPGnormalizeStepWidth needs N_array step"));
                }
                veclen = TCcountExprs (ARRAY_AELEMS (stpar));

                (*width) = CreateEntryFlatArray (1, veclen);
            }
            if (PMmatchFlat (pat, *step)) {
                stpar = array;
            } else {
                DBUG_ASSERT (FALSE, ("WLPGnormalizeStepWidth needs N_array step"));
            }
            if (PMmatchFlat (pat, *width)) {
                wthar = array;
            } else {
                DBUG_ASSERT (FALSE, ("WLPGnormalizeStepWidth needs N_array width"));
            }
            pat = PMfree (pat);

            stp = ARRAY_AELEMS (stpar);
            wth = ARRAY_AELEMS (wthar);

            while (stp && !error) {
                DBUG_ASSERT ((wth != NULL), "step shape > width shape!");

                if ((NODE_TYPE (EXPRS_EXPR (stp)) == N_num)
                    && (NODE_TYPE (EXPRS_EXPR (wth)) == N_num)) {

                    stpnum = NUM_VAL (EXPRS_EXPR (stp));
                    wthnum = NUM_VAL (EXPRS_EXPR (wth));

                    if (wthnum > stpnum)
                        error = 1;
                    else if (1 > wthnum)
                        error = 2;
                    else if (wthnum == stpnum && stpnum != 1)
                        NUM_VAL (EXPRS_EXPR (stp)) = NUM_VAL (EXPRS_EXPR (wth)) = stpnum
                          = 1;

                    is_1 = is_1 && 1 == stpnum;
                } else if ((NODE_TYPE (EXPRS_EXPR (stp)) == N_id)
                           && (NODE_TYPE (EXPRS_EXPR (wth)) == N_id)) {
                    if (STReq (ID_NAME (EXPRS_EXPR (stp)), ID_NAME (EXPRS_EXPR (wth)))) {
                        EXPRS_EXPR (stp) = FREEdoFreeTree (EXPRS_EXPR (stp));
                        EXPRS_EXPR (stp) = TBmakeNum (1);
                        EXPRS_EXPR (wth) = FREEdoFreeTree (EXPRS_EXPR (wth));
                        EXPRS_EXPR (wth) = TBmakeNum (1);
                        is_1 = is_1 && TRUE;
                    } else {
                        is_1 = FALSE;
                    }
                } else {
                    is_1 = FALSE;
                }

                stp = EXPRS_NEXT (stp);
                wth = EXPRS_NEXT (wth);
            }
            DBUG_ASSERT ((wth == NULL), "width shape > step shape!");

            /* if both vectors are 1 this is equivalent to no grid */

            if (!error && is_1) {
                (*step) = FREEdoFreeTree (*step);
                (*width) = FREEdoFreeTree (*width);
            }
        }
    }

    DBUG_RETURN (error);
}

/** <!--********************************************************************-->
 *
 * @fn static node *flattenBound( node *arg_node, info *arg_info)
 *
 *   @brief  Flattens the WL bound at arg_node.
 *           I.e., if the generator looks like this on entry:
 *            s0 = _idx_shape_sel(0,x);
 *            s1 = _idx_shape_sel(1,x);
 *            z = with {
 *             (. <= iv < [s0, s1]) ...
 *            }
 *
 *          it will look like this on the way out:
 *            int[2] TMP;
 *            ...
 *            s0 = _idx_shape_sel(0,x);
 *            s1 = _idx_shape_sel(1,x);
 *            TMP = [s0, s1];
 *            z = with {
 *             (. <= iv < TMP) ...
 *            }
 *
 *          The only rationale for this change is to ensure that
 *          WL bounds are named. This allows us to associate an
 *          N_avis node with each bound, which will be used to
 *          store AVIS_MINVAL and AVIS_MAXVAL for the bound.
 *          These fields, in turn, will be used by the constant
 *          folder to remove guards and do other swell optimizations.
 *
 *   @param  node *arg_node: a WL PART BOUND to be flattened.
 *           info *arg_info:
 *
 *   @return node *node:      N_id node for flattened bound
 ******************************************************************************/
static node *
flattenBound (node *arg_node, info *arg_info)
{
    node *bavis;
    node *nas;
    node *bid;
    int shp;

    DBUG_ENTER ("flattenBound");

    if ((global.ssaiv && (N_array == NODE_TYPE (arg_node)))) {
        shp = TCcountExprs (ARRAY_AELEMS (arg_node));
        bavis = TBmakeAvis (TRAVtmpVar (),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, shp)));
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (bavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
        nas = TBmakeAssign (TBmakeLet (TBmakeIds (bavis, NULL), DUPdoDupTree (arg_node)),
                            NULL);
        AVIS_SSAASSIGN (bavis) = nas;
        INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), nas);
        bid = TBmakeId (bavis);
        FREEdoFreeTree (arg_node);
        arg_node = bid;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AppendPart2WL(node * wln, node *partn, info *arg_info)
 *
 *   @brief append N_part npart to the existing parts of
 *          the N_with node wln
 *
 *   @param  node *wln    : N_with
 *           node *npart  : N_part
 *   @return node *       : modified N_with
 ******************************************************************************/

static node *
AppendPart2WL (node *wln, node *partn, info *arg_info)
{
    node *parts;
    int no_parts;

    DBUG_ENTER ("AppendPart2WL");

    PART_BOUND1 (partn) = flattenBound (PART_BOUND1 (partn), arg_info);
    PART_BOUND2 (partn) = flattenBound (PART_BOUND2 (partn), arg_info);
    /* TODO: Should treat STEP and WIDTH the same as BOUNDs */
    parts = WITH_PART (wln);
    /* at least one part exists */
    no_parts = 1;

    while (PART_NEXT (parts)) {
        parts = PART_NEXT (parts);
        no_parts++;
    }

    PART_NEXT (parts) = partn;
    no_parts++;
    WITH_PARTS (wln) = no_parts;

    DBUG_RETURN (wln);
}

/** <!--********************************************************************-->
 *
 * @fn node *CutSlices( node *ls, node *us, int dim,
 *                       node *wln, node *coden, info *arg_info,
 *                       node *withid)
 *
 *   @brief  Creates a (full) partition by adding new N_generator nodes
 *           to the N_with node.
 *           If the known part is a grid, this is ignored here (so the
 *           resulting N_with node may still not be a full partition,
 *           see CompleteGrid()).
 *
 *   @param  node *ls, *us : bounds of the whole array
 *           int dim       : number of elements in ls, us, l, u
 *           node *wln     : Pointer of N_with node where the new generators
 *                           shall be inserted.
 *           node *coden   : Pointer of N_code node where the new generators
 *                           shall point to.
 *           info *arg_info: local information block
 *   @return node *        : modified N_with
 ******************************************************************************/

static node *
CutSlices (node *ls, node *us, int dim, node *wln, node *coden, info *arg_info,
           node *withid)
{
    node *lsc = NULL;
    node *usc = NULL;
    node *le, *ue, *partn, *ubn, *lbn;
    node *lsce = NULL;
    node *usce = NULL;
    node *lb = NULL;
    node *ub = NULL;
    node *pbnd = NULL;
    constant *lbco = NULL;
    constant *ubco = NULL;
    constant *pbndshp = NULL;
    constant *lsco = NULL;
    constant *usco = NULL;
    int i, d;
    int lnum;
    int lscnum;
    int unum;
    int uscnum;

    DBUG_ENTER ("CutSlices");

    /* Because this gets called in phase 10, we may have a constant
     * arising from the typechecker that has not found its way to
     * the RHS yet. The next few lines involving COconstant2AST
     * perform this check.
     * (As an alternative, we could call CF in phase 10 before WLPG.)
     */
    if (PMO (PMOarray (&lbco, &lb, WITH_BOUND1 (wln)))) {
        lbco = COfreeConstant (lbco);
    } else {
        lbco = COaST2Constant (WITH_BOUND1 (wln));
        if (NULL != lbco) {
            lb = COconstant2AST (lbco);
            lbco = COfreeConstant (lbco);
        }
    }

    if (PMO (PMOarray (&ubco, &ub, WITH_BOUND2 (wln)))) {
        ubco = COfreeConstant (ubco);
    } else {
        ubco = COaST2Constant (WITH_BOUND2 (wln));
        if (NULL != ubco) {
            ub = COconstant2AST (ubco);
            ubco = COfreeConstant (ubco);
        }
    }

    DBUG_ASSERT ((N_array == NODE_TYPE (lb) && (N_array == NODE_TYPE (ub))),
                 "CutSlices expected N_array bounds.");
    /* create local copies of the arrays which are modified here */
    lsc = DUPdoDupTree (ls);
    usc = DUPdoDupTree (us);

    if (PMO (PMOarray (&lsco, &lsce, lsc))) {
        lsco = COfreeConstant (lsco);
    } else {
        lsco = COaST2Constant (lsc);
        if (NULL != lsco) {
            lsce = COconstant2AST (lsco);
            lsco = COfreeConstant (lsco);
        }
    }

    if (PMO (PMOarray (&usco, &usce, usc))) {
        usco = COfreeConstant (usco);
    } else {
        usco = COaST2Constant (lsc);
        if (NULL != usco) {
            usce = COconstant2AST (usco);
            usco = COfreeConstant (usco);
        }
    }

    lsce = ARRAY_AELEMS (lsce);
    usce = ARRAY_AELEMS (usce);

    le = ARRAY_AELEMS (lb);
    ue = ARRAY_AELEMS (ub);

    for (d = 0; d < dim;
         d++) { /* Check whether there is a cuboid above (below) the given one. */

        if (NODE_TYPE (EXPRS_EXPR (le)) == N_num) {
            lnum = NUM_VAL (EXPRS_EXPR (le));
            lscnum = NUM_VAL (EXPRS_EXPR (lsce));
            if (lnum > lscnum) {
                partn = CreateNewPart (lsc, usc, NULL, NULL, withid, coden, arg_info);
                pbnd = NULL;
                pbndshp = NULL;
                if (!PMO (PMOarray (&pbndshp, &pbnd, PART_BOUND2 (partn)))) {
                    DBUG_ASSERT (FALSE, "CutSlices expected N_array partn BOUND2");
                }
                pbndshp = COfreeConstant (pbndshp);
                ubn = ARRAY_AELEMS (pbnd);
                for (i = 0; i < d; i++) {
                    ubn = EXPRS_NEXT (ubn);
                }
                if (NODE_TYPE (EXPRS_EXPR (ubn)) == N_num) {
                    NUM_VAL (EXPRS_EXPR (ubn)) = lnum;
                } else {
                    EXPRS_EXPR (ubn) = FREEdoFreeTree (EXPRS_EXPR (ubn));
                    EXPRS_EXPR (ubn) = DUPdoDupTree (EXPRS_EXPR (le));
                }
                wln = AppendPart2WL (wln, partn, arg_info);
            }
        } else {
            partn = CreateNewPart (lsc, usc, NULL, NULL, withid, coden, arg_info);
            pbnd = NULL;
            pbndshp = NULL;
            if (!PMO (PMOarray (&pbndshp, &pbnd, PART_BOUND2 (partn)))) {
                DBUG_ASSERT (FALSE, "CutSlices expected another N_array partn BOUND2");
            }
            pbndshp = COfreeConstant (pbndshp);
            ubn = ARRAY_AELEMS (pbnd);
            for (i = 0; i < d; i++) {
                ubn = EXPRS_NEXT (ubn);
            }
            EXPRS_EXPR (ubn) = FREEdoFreeTree (EXPRS_EXPR (ubn));
            EXPRS_EXPR (ubn) = DUPdoDupTree (EXPRS_EXPR (le));
            wln = AppendPart2WL (wln, partn, arg_info);
        }

        if ((NODE_TYPE (EXPRS_EXPR (ue)) == N_num)
            && (NODE_TYPE (EXPRS_EXPR (usce)) == N_num)) {
            unum = NUM_VAL (EXPRS_EXPR (ue));
            uscnum = NUM_VAL (EXPRS_EXPR (usce));
            if (unum < uscnum) {
                partn = CreateNewPart (lsc, usc, NULL, NULL, withid, coden, arg_info);
                pbnd = NULL;
                pbndshp = NULL;
                if (!PMO (PMOarray (&pbndshp, &pbnd, PART_BOUND1 (partn)))) {
                    DBUG_ASSERT (FALSE, "CutSlices expected N_array partn BOUND1");
                }
                pbndshp = COfreeConstant (pbndshp);
                lbn = ARRAY_AELEMS (pbnd);
                for (i = 0; i < d; i++) {
                    lbn = EXPRS_NEXT (lbn);
                }
                NUM_VAL (EXPRS_EXPR (lbn)) = unum;
                wln = AppendPart2WL (wln, partn, arg_info);
            }
        } else {
            partn = CreateNewPart (lsc, usc, NULL, NULL, withid, coden, arg_info);
            if (!PMO (PMOarray (&pbndshp, &pbnd, PART_BOUND1 (partn)))) {
                DBUG_ASSERT (FALSE, "CutSlices expected another N_array partn BOUND1");
            }
            pbndshp = COfreeConstant (pbndshp);
            lbn = ARRAY_AELEMS (pbnd);
            for (i = 0; i < d; i++) {
                lbn = EXPRS_NEXT (lbn);
            }
            EXPRS_EXPR (lbn) = FREEdoFreeTree (EXPRS_EXPR (lbn));
            EXPRS_EXPR (lbn) = DUPdoDupTree (EXPRS_EXPR (ue));
            wln = AppendPart2WL (wln, partn, arg_info);
        }

        /* and modify array bounds to continue with next dimension */
        if (NODE_TYPE (EXPRS_EXPR (le)) == N_num) {
            NUM_VAL (EXPRS_EXPR (lsce)) = NUM_VAL (EXPRS_EXPR (le));
        } else {
            EXPRS_EXPR (lsce) = FREEdoFreeTree (EXPRS_EXPR (lsce));
            EXPRS_EXPR (lsce) = DUPdoDupTree (EXPRS_EXPR (le));
        }

        if ((NODE_TYPE (EXPRS_EXPR (ue)) == N_num)
            && (NODE_TYPE (EXPRS_EXPR (usce)) == N_num)) {
            NUM_VAL (EXPRS_EXPR (usce)) = NUM_VAL (EXPRS_EXPR (ue));
        } else {
            EXPRS_EXPR (usce) = FREEdoFreeTree (EXPRS_EXPR (usce));
            EXPRS_EXPR (usce) = DUPdoDupTree (EXPRS_EXPR (ue));
        }

        le = EXPRS_NEXT (le);
        lsce = EXPRS_NEXT (lsce);
        ue = EXPRS_NEXT (ue);
        usce = EXPRS_NEXT (usce);
    }

    lsc = FREEdoFreeTree (lsc);
    usc = FREEdoFreeTree (usc);

    if (WITH_PARTS (wln) == -1) {
        /**
         * no new slice needed to be cut. Hence, the original generator is full!
         */
        WITH_PARTS (wln) = 1;
    }

    DBUG_RETURN (wln);
}

/** <!--********************************************************************-->
 *
 * @fn node *CompleteGrid( node *ls, node *us, node *step, node *width,
 *                         int dim, node *wln, node *coden, info *arg_info)
 *
 *   @brief  adds new parts to N_with which specify the elements
 *           left out by a grid.
 *
 *   @param  node *ls,*us       : bounds of the given part
 *           node *step, *width :
 *           int dim            : number of elements of ls, us
 *           node *wln          : N_with where to add the new parts.
 *                                If ig != NULL, the same pointer is returned.
 *           node *coden        : Pointer of N_code node where the new
 *                                generators shall point to.
 *           info *arg_info     : N_INFO is needed to create new vars
 *   @return intern_gen *       : chain of intern_gen struct
 ******************************************************************************/

static node *
CompleteGrid (node *ls, node *us, node *step, node *width, int dim, node *wln,
              node *coden, info *arg_info)
{
    node *nw, *stpe, *wthe, *nwe, *partn, *withidn, *wthn, *tmp1, *tmp2, *nassign, *_ids;
    node *stp = NULL;
    constant *stpfs = NULL;
    node *lbn = NULL;
    constant *lbnfs = NULL;
    int i, d, stpnum, wthnum;

    DBUG_ENTER ("CompleteGrid");

    /* create local copies of the arrays which are modified here*/
    DBUG_ASSERT (N_array == NODE_TYPE (width), ("CompleteGrid needs N_array WIDTH"));
    if (PMO (PMOarray (&stpfs, &stp, step))) {
        COfreeConstant (stpfs);
    } else {
        DBUG_ASSERT (FALSE, ("CompleteGrid needs N_array STEP"));
    }
    nw = DUPdoDupTree (stp);

    stpe = ARRAY_AELEMS (stp);
    wthe = ARRAY_AELEMS (width);
    nwe = ARRAY_AELEMS (nw);

    withidn = DUPdoDupTree (PART_WITHID (WITH_PART (wln)));

    for (d = 0; d < dim; d++) {

        if ((NODE_TYPE (EXPRS_EXPR (stpe)) == N_num)
            && (NODE_TYPE (EXPRS_EXPR (wthe)) == N_num)) {
            stpnum = NUM_VAL (EXPRS_EXPR (stpe));
            wthnum = NUM_VAL (EXPRS_EXPR (wthe));

            if (stpnum > wthnum) { /* create new grids */

                partn = CreateNewPart (ls, us, step, nw, withidn, coden, arg_info);
                /* Extract bound and width_arrays */
                lbn = NULL;
                lbnfs = NULL;
                if (PMO (PMOarray (&lbnfs, &lbn, PART_BOUND1 (partn)))) {
                    COfreeConstant (lbnfs);
                } else {
                    DBUG_ASSERT (FALSE, ("CompleteGrid expected N_id or N_array BOUND"));
                }

                lbn = ARRAY_AELEMS (lbn);
                wthn = ARRAY_AELEMS (PART_WIDTH (partn));

                for (i = 0; i < d; i++) {
                    wthn = EXPRS_NEXT (wthn);
                    lbn = EXPRS_NEXT (lbn);
                }

                if (NODE_TYPE (EXPRS_EXPR (lbn)) == N_num) {
                    NUM_VAL (EXPRS_EXPR (lbn)) = NUM_VAL (EXPRS_EXPR (lbn)) + wthnum;
                } else {

                    _ids = NewIds (EXPRS_EXPR (lbn), INFO_FUNDEF (arg_info));

                    /* the identifier to add wthnum to */
                    tmp1 = DUPdoDupTree (EXPRS_EXPR (lbn));
                    /* N_num wthnum */
                    tmp2 = TBmakeExprs (DUPdoDupTree (EXPRS_EXPR (wthe)), NULL);

                    tmp1 = TBmakePrf (F_add_SxS, TBmakeExprs (tmp1, tmp2));

                    nassign = TBmakeAssign (TBmakeLet (_ids, tmp1), NULL);
                    /* set correct backref to defining assignment */
                    AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;

                    INFO_NASSIGNS (arg_info)
                      = TCappendAssign (INFO_NASSIGNS (arg_info), nassign);

                    EXPRS_EXPR (lbn) = FREEdoFreeTree (EXPRS_EXPR (lbn));
                    EXPRS_EXPR (lbn) = DUPdupIdsId (_ids);
                }
                NUM_VAL (EXPRS_EXPR (wthn)) = stpnum - wthnum;
                i = WLPGnormalizeStepWidth (&(PART_STEP (partn)), &(PART_WIDTH (partn)));
                DBUG_ASSERT (!i, ("internal normalization failure"));

                wln = AppendPart2WL (wln, partn, arg_info);
            }
        } else {
            partn = CreateNewPart (ls, us, step, nw, withidn, coden, arg_info);
            /* Extract bound and width_arrays */
            lbn = NULL;
            lbnfs = NULL;
            if (PMO (PMOarray (&lbnfs, &lbn, PART_BOUND1 (partn)))) {
                COfreeConstant (lbnfs);
            } else {
                DBUG_ASSERT (FALSE, ("CompleteGrid expected N_id or N_array BOUND1"));
            }

            lbn = ARRAY_AELEMS (lbn);
            wthn = ARRAY_AELEMS (PART_WIDTH (partn));

            for (i = 0; i < d; i++) {
                wthn = EXPRS_NEXT (wthn);
                lbn = EXPRS_NEXT (lbn);
            }

            if ((NODE_TYPE (EXPRS_EXPR (lbn)) == N_num)
                && (NODE_TYPE (EXPRS_EXPR (wthe)) == N_num)) {
                NUM_VAL (EXPRS_EXPR (lbn))
                  = NUM_VAL (EXPRS_EXPR (lbn)) + NUM_VAL (EXPRS_EXPR (wthe));
            } else {

                _ids = NewIds (EXPRS_EXPR (lbn), INFO_FUNDEF (arg_info));

                /* the identifier to add current width to */
                tmp1 = DUPdoDupTree (EXPRS_EXPR (lbn));
                /* current width as N_num or N_id */
                tmp2 = TBmakeExprs (DUPdoDupTree (EXPRS_EXPR (wthe)), NULL);

                tmp1 = TBmakePrf (F_add_SxS, TBmakeExprs (tmp1, tmp2));

                nassign = TBmakeAssign (TBmakeLet (_ids, tmp1), NULL);
                /* set correct backref to defining assignment */
                AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;

                INFO_NASSIGNS (arg_info)
                  = TCappendAssign (INFO_NASSIGNS (arg_info), nassign);

                EXPRS_EXPR (lbn) = FREEdoFreeTree (EXPRS_EXPR (lbn));
                EXPRS_EXPR (lbn) = DUPdupIdsId (_ids);
            }

            if ((NODE_TYPE (EXPRS_EXPR (stpe)) == N_num)
                && (NODE_TYPE (EXPRS_EXPR (wthe)) == N_num)) {
                NUM_VAL (EXPRS_EXPR (wthn))
                  = NUM_VAL (EXPRS_EXPR (stpe)) - NUM_VAL (EXPRS_EXPR (wthe));
            } else {

                _ids = NewIds (EXPRS_EXPR (wthn), INFO_FUNDEF (arg_info));

                /* the first identifier */
                tmp1 = DUPdoDupTree (EXPRS_EXPR (stpe));
                /* the second identifier */
                tmp2 = TBmakeExprs (DUPdoDupTree (EXPRS_EXPR (wthe)), NULL);

                tmp1 = TBmakePrf (F_sub_SxS, TBmakeExprs (tmp1, tmp2));

                nassign = TBmakeAssign (TBmakeLet (_ids, tmp1), NULL);
                /* set correct backref to defining assignment */
                AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;

                INFO_NASSIGNS (arg_info)
                  = TCappendAssign (INFO_NASSIGNS (arg_info), nassign);

                EXPRS_EXPR (wthn) = FREEdoFreeTree (EXPRS_EXPR (wthn));
                EXPRS_EXPR (wthn) = DUPdupIdsId (_ids);
            }

            i = WLPGnormalizeStepWidth (&(PART_STEP (partn)), &(PART_WIDTH (partn)));
            DBUG_ASSERT (!i, ("internal normalization failure"));

            wln = AppendPart2WL (wln, partn, arg_info);
        }

        /* and modify array bounds to continue with next dimension */
        if ((NODE_TYPE (EXPRS_EXPR (nwe)) == N_num)
            && (NODE_TYPE (EXPRS_EXPR (wthe)) == N_num)) {
            NUM_VAL (EXPRS_EXPR (nwe)) = NUM_VAL (EXPRS_EXPR (wthe));
        } else {
            EXPRS_EXPR (nwe) = FREEdoFreeTree (EXPRS_EXPR (nwe));
            EXPRS_EXPR (nwe) = DUPdoDupTree (EXPRS_EXPR (wthe));
        }

        stpe = EXPRS_NEXT (stpe);
        wthe = EXPRS_NEXT (wthe);
        nwe = EXPRS_NEXT (nwe);
    }

    nw = FREEdoFreeTree (nw);

    DBUG_RETURN (wln);
}

/** <!--********************************************************************-->
 *
 * @fn  node *CreateFullPartition( node *wln, info *arg_info)
 *
 *   @brief  generates full partition if possible:
 *           - if withop is genarray and index vector has as many elements as
 *             dimension of resulting WL (withloop on scalars).
 *           - if withop is modarray: always (needed for compilation phase).
 *           Returns wln.
 *
 *   @param  node *wl       :  N_with node of the WL to transform
 *           info *arg_info :  is needed to access the vardecs of the current
 *                             function
 *   @return node *         :  modified N_with
 ******************************************************************************/

static node *
CreateFullPartition (node *wln, info *arg_info)
{
    node *coden, *array_shape = NULL, *array_null;
    ntype *array_type;
    shape *shape, *mshape;
    node *withid;
    node *shp = NULL;
    constant *shpco = NULL;
    int gen_shape = 0;
    bool do_create;

    DBUG_ENTER ("CreateFullPartition");

    DBUG_ASSERT ((PART_NEXT (WITH_PART (wln)) != NULL)
                   && (NODE_TYPE (PART_GENERATOR (PART_NEXT (WITH_PART (wln))))
                       == N_default),
                 "Second partition is not default partition!");

    /* recycle default code for all new parts */
    withid = DUPdoDupTree (PART_WITHID (PART_NEXT (WITH_PART (wln))));
    coden = DUPdoDupNode (PART_CODE (PART_NEXT (WITH_PART (wln))));
    /* delete default partition */
    PART_NEXT (WITH_PART (wln)) = FREEdoFreeTree (PART_NEXT (WITH_PART (wln)));

    do_create = TRUE;

    /* get shape of the index vector (generator) */
    gen_shape = SHgetExtent (TYgetShape (IDS_NTYPE (WITH_VEC (wln))), 0);

    /*
     * allocate "array_shape"
     */
    switch (NODE_TYPE (WITH_WITHOP (wln))) {
    case N_modarray: {
        /* create upper array bound */
        array_type = ID_NTYPE (MODARRAY_ARRAY (WITH_WITHOP (wln)));

        if (TYisAKV (array_type) || TYisAKS (array_type)) {

            shape = TYgetShape (array_type);
            /* only for iteration space */
            mshape = SHtakeFromShape (gen_shape, shape);
            array_shape = SHshape2Array (mshape);

            if (mshape) {
                SHfreeShape (mshape);
            }
        } else {
            array_shape = CreateArrayOfShapeSels (MODARRAY_ARRAY (WITH_WITHOP (wln)),
                                                  gen_shape, arg_info);
        }
    } break;

    case N_genarray:
        array_shape = GENARRAY_SHAPE (WITH_WITHOP (wln));
        /* array_shape may be an N_id or an N_array. If the former,
         * we look up its N_array value. It may also turn out that
         * we know the array shape, but PMarray can't find it for us.
         * This can arise when we have:
         *  genarray(shp)
         * and the shape argument is:
         *  shp = _shape_A_(x)
         * and x is AKS. In that case, shp may be AKV:
         *  int[1]{7} shp;
         *
         */
        if (PMO (PMOarray (&shpco, &shp, array_shape))) {
            shpco = COfreeConstant (shpco);
            array_shape = DUPdoDupTree (shp);
        } else {
            shpco = COaST2Constant (array_shape);
            if (NULL != shpco) {
                array_shape = COconstant2AST (shpco);
                shpco = COfreeConstant (shpco);
            }
        }
        break;

    default:
        DBUG_ASSERT ((0), "illegal WITH_TYPE found!");
        array_shape = NULL;
        break;
    }

    do_create = (array_shape != NULL);

    /*
     * start creation
     */
    if (do_create) {
        /* create lower array bound */

        array_null = CreateEntryFlatArray (0, gen_shape);

        /* create surrounding cuboids */
        wln
          = CutSlices (array_null, array_shape, gen_shape, wln, coden, arg_info, withid);

        /* the original part can still be found at first position in wln.
           Now create grids. */
        if (WITH_STEP (wln))
            wln = CompleteGrid (WITH_BOUND1 (wln), WITH_BOUND2 (wln), WITH_STEP (wln),
                                WITH_WIDTH (wln), gen_shape, wln, coden, arg_info);

        /* if new codes have been created, add them to code list */
        if (PART_NEXT (WITH_PART (wln))) {
            CODE_NEXT (coden) = WITH_CODE (wln);
            WITH_CODE (wln) = coden;
        }

        /* free the above made arrays */
        array_null = FREEdoFreeTree (array_null);
    }

    if (array_shape != NULL) {
        array_shape = FREEdoFreeTree (array_shape);
    }
    withid = FREEdoFreeTree (withid);

    DBUG_RETURN (wln);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateEmptyGenWLReplacement( node *wl, info *arg_info)
 *
 *   @brief  computes the replacement for the given WL under the assumption that
 *           the generator is empty, i.e.,
 *
 *           with( lb <= iv < ub)              =>     with( 0*shp <= iv <shp)
 *           genarray( shp, exp, dexp)                genarray( shp, dexp, ---)
 *
 *           with( lb <= iv < ub)              =>     a
 *           modarray( a, NULL, exp)
 *
 *           with( lb <= iv < ub)              =>     neutral
 *           fold( fun, neutral, exp)
 *
 *           NOTE HERE, that wl is either MODIFIED in place or FREED entirely!!
 *
 *           Due to break() and propagate(x), we may find some additional
 *           operators. These require some additional changes:
 *
 *           with( lb <= iv < ub)              =>     neutral
 *           fold( fun, neutral, exp)
 *           break( )
 *
 *           AND the respective ids of the lhs needs to be eliminated
 *
 *           with( lb <= iv < ub)              =>     x
 *           propagate( x )
 *
 *           As we now have several cases where we return a value instead of
 *           keeping the WL, we may need to insert additional assignments.
 *           This is done using INFO_NASSIGNS (cf. WLPGassign).
 *
 *           NOTE here, that the current implementation CANNOT handle
 *           arbitrary multi operator WLs. This would require a proper
 *           rewrite of this function + a different abstraction:
 *           Instead of modifying the RHS only, we would need to replace the
 *           entire assignment with a new assignment chain.
 *
 *   @param  node *wl       :  N_with
 *           info *arg_info :  N_INFO
 *   @return node *         :  modified N_with or replacement code
 ******************************************************************************/

static node *
CreateEmptyGenWLReplacement (node *wl, info *arg_info)
{
    node *let_ids, *last_ids;
    node *wlop;
    int dim, i;
    node *lb = NULL;
    node *ub = NULL;
    node *lbe, *ube;
    node *code;
    node *tmpn, *assignn, *blockn, *cexpr;
    node *nassigns = NULL;
    node *_ids;
    node *res = NULL;
    node *array;
    ntype *array_type;
    pattern *pat;

    DBUG_ENTER ("CreateEmptyGenWLReplacement");

    wlop = WITH_WITHOP (wl);
    let_ids = LET_IDS (INFO_LET (arg_info));
    last_ids = NULL;

    while (wlop != NULL) {
        DBUG_ASSERT (let_ids != NULL, "lhs does not match number of WLops");

        switch (NODE_TYPE (wlop)) {
        case N_genarray:
            res = wl;
            /*
             * First, we change the generator to full scope.
             */
            dim = SHgetDim (TYgetShape (AVIS_TYPE (IDS_AVIS (let_ids))));
            pat = PMarray (1, PMAgetNode (&array), 1, PMskip (0));
            if (PMmatchFlat (pat, WITH_BOUND1 (wl))) {
                lb = array;
            }
            if (PMmatchFlat (pat, WITH_BOUND2 (wl))) {
                ub = array;
            }
            pat = PMfree (pat);
            lbe = ARRAY_AELEMS (lb);
            ube = ARRAY_AELEMS (ub);

            i = 0;
            while ((i < dim) && (lbe != NULL)) {
                NUM_VAL (EXPRS_EXPR (lbe)) = 0;
                NUM_VAL (EXPRS_EXPR (ube))
                  = SHgetExtent (TYgetShape (IDS_NTYPE (let_ids)), i);

                lbe = EXPRS_NEXT (lbe);
                ube = EXPRS_NEXT (ube);
                i++;
            }

            if (WITH_STEP (wl)) {
                WITH_STEP (wl) = FREEdoFreeTree (WITH_STEP (wl));
            }
            if (WITH_WIDTH (wl)) {
                WITH_WIDTH (wl) = FREEdoFreeTree (WITH_WIDTH (wl));
            }

            /*
             * Now we have to change the code. Either we can use DEFAULT (easy)
             * or we have to create zeros (ugly).
             */
            code = WITH_CODE (wl);
            if (GENARRAY_DEFAULT (WITH_WITHOP (wl)) != NULL) {
                CODE_CBLOCK (code) = FREEdoFreeTree (CODE_CBLOCK (code));
                CODE_CBLOCK (code) = TBmakeBlock (TBmakeEmpty (), NULL);

                EXPRS_EXPR (CODE_CEXPRS (code))
                  = FREEdoFreeTree (EXPRS_EXPR (CODE_CEXPRS (code)));
                EXPRS_EXPR (CODE_CEXPRS (code))
                  = DUPdoDupTree (GENARRAY_DEFAULT (WITH_WITHOP (wl)));
            } else {
                blockn = CODE_CBLOCK (code);
                tmpn = BLOCK_INSTR (blockn);
                cexpr = EXPRS_EXPR (CODE_CEXPRS (code));
                array_type = ID_NTYPE (cexpr);

                if (N_empty == NODE_TYPE (tmpn)) {
                    /* there is no instruction in the block right now. */
                    _ids = NewIds (cexpr, INFO_FUNDEF (arg_info));

                    if (TYisAKV (array_type) || TYisAKS (array_type)) {
                        tmpn = CreateZeros (array_type, INFO_FUNDEF (arg_info));
                    } else {
                        CTIabortLine (global.linenum,
                                      "Cexpr of Genarray with-loop is not AKV/AKS."
                                      " Unfortunately, a AKV/AKS cexpr is necessary here"
                                      " to generate code for empty WL replacement");
                    }

                    /* replace N_empty with new assignment "_ids = [0,..,0]" */
                    assignn = TBmakeAssign (TBmakeLet (_ids, tmpn), NULL);
                    nassigns = TCappendAssign (nassigns, assignn);
                    BLOCK_INSTR (blockn)
                      = TCappendAssign (BLOCK_INSTR (blockn), nassigns);

                    /* set correct backref to defining assignment */
                    AVIS_SSAASSIGN (IDS_AVIS (_ids)) = assignn;

                    /* replace CEXPR */
                    tmpn = WITH_CODE (INFO_WL (arg_info));
                    EXPRS_EXPR (CODE_CEXPRS (tmpn))
                      = FREEdoFreeTree (EXPRS_EXPR (CODE_CEXPRS (tmpn)));
                    EXPRS_EXPR (CODE_CEXPRS (tmpn)) = DUPdupIdsId (_ids);

                } else {
                    /* we have a non-empty block.
                       search cexpr assignment and make it the only one in the block. */

                    assignn = DUPdoDupNode (AVIS_SSAASSIGN (ID_AVIS (cexpr)));
                    BLOCK_INSTR (blockn) = FREEdoFreeTree (BLOCK_INSTR (blockn));
                    LET_EXPR (ASSIGN_INSTR (assignn))
                      = FREEdoFreeTree (LET_EXPR (ASSIGN_INSTR (assignn)));

                    if (TYisAKV (array_type) || TYisAKS (array_type)) {
                        tmpn = CreateZeros (array_type, INFO_FUNDEF (arg_info));
                    } else {
                        CTIabortLine (global.linenum,
                                      "Cexpr of Genarray with-loop is not AKV/AKS."
                                      " Unfortunately, a AKV/AKS cexpr is necessary here"
                                      " to generate code for empty WL replacement");
                    }

                    CODE_VISITED (code) = TRUE;
                    LET_EXPR (ASSIGN_INSTR (assignn)) = tmpn;
                    nassigns = TCappendAssign (nassigns, assignn);

                    BLOCK_INSTR (blockn) = nassigns;
                }
            }
            wlop = GENARRAY_NEXT (wlop);
            break;
        case N_modarray:
            res = DUPdoDupTree (MODARRAY_ARRAY (WITH_WITHOP (wl)));
            wlop = MODARRAY_NEXT (wlop);
            break;
        case N_fold:
            res = DUPdoDupTree (FOLD_NEUTRAL (WITH_WITHOP (wl)));
            wlop = FOLD_NEXT (wlop);
            break;
        case N_break:
            DBUG_ASSERT ((last_ids != NULL), "break is first op in WL");
            IDS_NEXT (last_ids) = FREEdoFreeNode (let_ids);
            let_ids = last_ids;

            wlop = BREAK_NEXT (wlop);
            break;
        case N_propagate:
            DBUG_ASSERT ((last_ids != NULL), "propagate is first op in WL");
            INFO_NASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (let_ids,
                                         DUPdoDupTree (PROPAGATE_DEFAULT (wlop))),
                              INFO_NASSIGNS (arg_info));
            AVIS_SSAASSIGN (IDS_AVIS (let_ids)) = INFO_NASSIGNS (arg_info);
            IDS_NEXT (last_ids) = IDS_NEXT (let_ids);
            IDS_NEXT (let_ids) = NULL;
            let_ids = last_ids;

            wlop = PROPAGATE_NEXT (wlop);
            break;
        default:
            DBUG_ASSERT ((0), "illegal WITHOP node found!");
            break;
        }
        DBUG_ASSERT (((wlop == NULL) || (NODE_TYPE (wlop) == N_break)
                      || (NODE_TYPE (wlop) == N_propagate)),
                     "illegal mopWL");
        last_ids = let_ids;
        let_ids = IDS_NEXT (let_ids);
    }

    if (res != wl) {
        wl = FREEdoFreeTree (wl);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node RemoveUnusedCodes(node *codes)
 *
 *   @brief removes all unused N_codes recursively
 *
 *   @param  node *codes : N_code chain
 *   @return node *      : modified N_code chain
 ******************************************************************************/
static node *
RemoveUnusedCodes (node *codes)
{
    DBUG_ENTER ("RemoveUnusedCodes");

    DBUG_ASSERT ((codes != NULL), "no codes available!");

    DBUG_ASSERT ((NODE_TYPE (codes) == N_code), "type of codes is not N_code!");

    if (CODE_NEXT (codes) != NULL) {
        CODE_NEXT (codes) = RemoveUnusedCodes (CODE_NEXT (codes));
    }

    if (CODE_USED (codes) == 0) {
        codes = FREEdoFreeNode (codes);
    }

    DBUG_RETURN (codes);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGmodule(node *arg_node, info *arg_info)
 *
 *   @brief first traversal of function definitions of WLPartitionGeneration
 *
 *   @param  node *arg_node:  N_module
 *           info *arg_info:  N_info
 *   @return node *        :  N_module
 ******************************************************************************/

node *
WLPGmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLPGmodule");

    DBUG_PRINT ("WLPG", ("WLPartitionGeneration module-wise"));

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGfundef(node *arg_node, info *arg_info)
 *
 *   @brief starts the traversal of the given fundef.
 *
 *   @param  node *arg_node:  N_fundef
 *           info *arg_info:  N_info
 *   @return node *        :  N_fundef
 ******************************************************************************/

node *
WLPGfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLPGfundef");

    INFO_WL (arg_info) = NULL;
    INFO_FUNDEF (arg_info) = arg_node;
    INFO_NASSIGNS (arg_info) = NULL;
    INFO_LET (arg_info) = NULL;

    if (INFO_SUBPHASE (arg_info) == SP_mod) {

        if (FUNDEF_BODY (arg_node)) {
            FUNDEF_INSTR (arg_node) = TRAVdo (FUNDEF_INSTR (arg_node), arg_info);
        }

        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else if (INFO_SUBPHASE (arg_info) == SP_func) {
        /* compiler_phase == PH_sacopt */
        if (FUNDEF_BODY (arg_node)) {
            FUNDEF_INSTR (arg_node) = TRAVdo (FUNDEF_INSTR (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGassign(node *arg_node, info *arg_info)
 *
 *   @brief traverse instruction. If this creates new assignments in
 *          INFO_NASSIGNS these are inserted in front of the actual one.
 *
 *   @param  node *arg_node:  N_assign
 *           info *arg_info:  N_info
 *   @return node *        :  N_assign
 ******************************************************************************/

node *
WLPGassign (node *arg_node, info *arg_info)
{
    node *iterator;

    DBUG_ENTER ("WLPGassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    iterator = NULL;

    if (INFO_NASSIGNS (arg_info) != NULL) {
        iterator = INFO_NASSIGNS (arg_info);
        while (ASSIGN_NEXT (iterator)) {
            iterator = ASSIGN_NEXT (iterator);
        }
        ASSIGN_NEXT (iterator) = arg_node;
        /* to traverse not in circle */
        iterator = ASSIGN_NEXT (iterator);

        arg_node = INFO_NASSIGNS (arg_info);
        INFO_NASSIGNS (arg_info) = NULL;

        ASSIGN_NEXT (iterator) = TRAVopt (ASSIGN_NEXT (iterator), arg_info);
    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGlet(node *arg_node, info *arg_info)
 *
 *   @brief traverses in expression and checks assigned ids for constant
 *          value and creates corresponding akv type
 *
 *   @param  node *arg_node:  N_let
 *           info *arg_info:  N_info
 *   @return node *        :  N_let
 ******************************************************************************/
node *
WLPGlet (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("WLPGlet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "N_let with empty EXPR attribute.");

    INFO_LET (arg_info) = arg_node;

    /*
     * Only ids nodes with one entry are considered.
     * Tuple of constants are not provided/supported in SaC until now.
     */
    if ((LET_IDS (arg_node) != NULL) && (IDS_NEXT (LET_IDS (arg_node)) == NULL)) {

        if (!TYisAKV (AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node))))) {
            constant *new_co = NULL;
            simpletype simple;

            new_co = COaST2Constant (LET_EXPR (arg_node));
            if (NULL != new_co) {
                /*
                 * it is possible to infer constant value
                 */
                simple = TYgetSimpleType (
                  TYgetScalar (AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)))));
                AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)))
                  = TYfreeType (AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node))));
                AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)))
                  = TYmakeAKV (TYmakeSimpleType (simple), new_co);
            }
        }
    }

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGap(node *arg_node, info *arg_info)
 *
 *   @brief traverse in the fundef of the ap node if special function
 *          and current compiler phase is sacopt
 *
 *   @param  node *arg_node:  N_ap
 *           info *arg_info:  info
 *   @return node *        :  N_ap
 ******************************************************************************/
node *
WLPGap (node *arg_node, info *arg_info)
{
    info *tmp;

    DBUG_ENTER ("WLPGap");

    if ((INFO_SUBPHASE (arg_info) == SP_func)
        && (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))) {
        /*
         * special functions must be traversed when they are used
         */
        if (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info)) {

            /* stack arg_info */
            tmp = arg_info;
            arg_info = MakeInfo ();
            /* take current flag SUBPHASE */
            INFO_SUBPHASE (arg_info) = INFO_SUBPHASE (tmp);

            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            arg_info = FreeInfo (arg_info);
            arg_info = tmp;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGwith(node *arg_node, info *arg_info)
 *
 *   @brief  start traversal of this WL and store information in new arg_info
 *           node. The only N_part node (inclusive body) is traversed.
 *           Afterwards, if certain conditions are fulfilled,
 *           the WL is transformed into a WL with generators describing a full
 *           partition.
 *
 *   @param  node *arg_node:  N_with
 *           info *arg_info:  N_info
 *   @return node *        :  N_with
 ******************************************************************************/

node *
WLPGwith (node *arg_node, info *arg_info)
{
    node *let_tmp, *nassigns = NULL;
    bool replace_wl = FALSE;
    gen_prop_t genprop = GPT_empty;

    DBUG_ENTER ("WLPGwith");

    /*
     * Information about last N_let has to be stacked before traversal
     */
    let_tmp = INFO_LET (arg_info);

    /*
     * The CODEs have to be traversed as they may contain further (nested) WLs
     * and I want to modify bottom up.
     */
    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);

    /* Pop N_let */
    INFO_LET (arg_info) = let_tmp;

    /*
     * Once a full partition has been created, we do not need to inspect
     * the generators anymore. The indicator for this is the PARTS attribute.
     * Initially, it is set to -1; if we have successfully generated a full
     * partition, it carries a positive value.
     */
    if ((WITH_PARTS (arg_node) == -1) && TUshapeKnown (IDS_NTYPE (WITH_VEC (arg_node)))) {

        /*
         * initialize WL traversal
         */
        INFO_WL (arg_info) = arg_node; /* store the current node for later */

        /*
         * analyse and prepare WL for generating a full partition
         * Besides changes in the generator, two one is computed
         * during this traversal:
         */
        DBUG_PRINT ("WLPG", ("call WLAdoWlAnalysis"));
        arg_node = WLAdoWlAnalysis (arg_node, INFO_FUNDEF (arg_info), INFO_LET (arg_info),
                                    &(nassigns), &genprop);

        DBUG_PRINT ("WLPG", ("WLAdoWlAnalysis ended"));

        INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), nassigns);

        switch (genprop) {
        case GPT_empty:
            /*
             * with-loop is empty
             */
            arg_node = CreateEmptyGenWLReplacement (arg_node, arg_info);
            replace_wl = TRUE;
            break;

        case GPT_full:
            /*
             * Generator covers whole index range
             */
            WITH_PARTS (arg_node) = 1;

            /*
             * delete default partition
             */
            if (PART_NEXT (WITH_PART (arg_node)) != NULL) {
                PART_NEXT (WITH_PART (arg_node))
                  = FREEdoFreeTree (PART_NEXT (WITH_PART (arg_node)));
            }

            /*
             * the default element cannot always be deleted if
             * it is a scalar! If the type of the generator expression
             * is AUD/AKD, the default element is still needed
             * by the memory allocation in alloc.c.
             * TODO: find a better way to decide whether the defexpr
             *       can be deleted
             */
#if 0
      /*
       * Delete default element if default element is AKS
       */
      if( NODE_TYPE( WITH_WITHOP( arg_node)) == N_genarray) {
        node *def = GENARRAY_DEFAULT( WITH_WITHOP( arg_node));
        
        if( ( def != NULL) &&
            ( TYisAKV( ID_NTYPE( def)) ||
              TYisAKS( ID_NTYPE( def)))) {
          GENARRAY_DEFAULT( WITH_WITHOP( arg_node)) 
            = FREEdoFreeNode( GENARRAY_DEFAULT( WITH_WITHOP( arg_node)));
        }
      }
#endif
            break;

        case GPT_partial:
            /*
             * Generator does not cover the whole index range
             */
            arg_node = CreateFullPartition (arg_node, arg_info);
            break;

        case GPT_unknown:
            /*
             * Nothing has been inferred
             */
            DBUG_ASSERT ((FALSE), "WLPG failure: IV is AKS and GPT_unknown was inferred");
            break;
        }
    }

    /*
     * Replace with-loops generating empty arrays with an equivalent reshape
     *   reshape( shp++shape(def), [:basetype]);
     *
     * This can only be performed iff the wl has not been replaced already
     */
    if ((NODE_TYPE (arg_node) == N_with)
        && (NODE_TYPE (WITH_WITHOP (arg_node)) == N_genarray)) {
        node *genarray = WITH_WITHOP (arg_node);
        ntype *shptype;
        ntype *exprtype;
        shape *shpshape = NULL;
        shape *exprshape = NULL;

        shptype = NTCnewTypeCheck_Expr (GENARRAY_SHAPE (genarray));
        if (GENARRAY_DEFAULT (genarray) != NULL) {
            exprtype = NTCnewTypeCheck_Expr (GENARRAY_DEFAULT (genarray));
        } else {
            exprtype = NTCnewTypeCheck_Expr (WITH_CEXPR (arg_node));
            DBUG_ASSERT (TUshapeKnown (exprtype),
                         "With-loop with non-AKS elements encountered.\n"
                         "Default-element is required!");
        }

        if (TYisAKV (shptype)) {
            shpshape = COconstant2Shape (TYgetValue (shptype));
        }

        if (TUshapeKnown (exprtype)) {
            exprshape = SHcopyShape (TYgetShape (exprtype));
        }

        if (((shpshape != NULL) && (SHgetUnrLen (shpshape) == 0))
            || ((exprshape != NULL) && (SHgetUnrLen (exprshape) == 0))) {
            ntype *scalar;
            node *rhs;
            node *ass;

            node *shpavis;
            node *defshpavis;
            node *newshpavis;
            ntype *newshptype, *rhs_type;
            node *arrayavis;

            /*
             * The generated array is empty. Build an alternative representation.
             */
            if (exprshape != NULL) {
                /*
                 * defshp = [[exprshape]];
                 *
                 * shp'   = shp;
                 * newshp = cat( shp', defshp);
                 * array  = [:BASETYPE]
                 * result = reshape( newshp, array);
                 */
                defshpavis = TBmakeAvis (TRAVtmpVar (),
                                         TYmakeAKV (TYmakeSimpleType (T_int),
                                                    COmakeConstantFromShape (exprshape)));
                FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (defshpavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

                ass = TBmakeAssign (TBmakeLet (TBmakeIds (defshpavis, NULL),
                                               SHshape2Array (exprshape)),
                                    NULL);
                AVIS_SSAASSIGN (defshpavis) = ass;
                INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), ass);
            } else {
                /*
                 * defshp = shape( def);
                 *
                 * shp'   = shp;
                 * newshp = cat( shp', defshp);
                 * array  = [:BASETYPE]
                 * result = reshape( newshp, array);
                 */
                rhs = TCmakePrf1 (F_shape_A,
                                  TBmakeId (ID_AVIS (GENARRAY_DEFAULT (genarray))));

                rhs_type = NTCnewTypeCheck_Expr (rhs);

                defshpavis = TBmakeAvis (TRAVtmpVar (), TYgetProductMember (rhs_type, 0));

                rhs_type = TYfreeTypeConstructor (rhs_type);

                FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (defshpavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

                ass = TBmakeAssign (TBmakeLet (TBmakeIds (defshpavis, NULL), rhs), NULL);
                AVIS_SSAASSIGN (defshpavis) = ass;
                INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), ass);
            }

            /*
             * shp' = shp;
             */
            shpavis = TBmakeAvis (TRAVtmpVar (), TYcopyType (shptype));

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (shpavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

            ass = TBmakeAssign (TBmakeLet (TBmakeIds (shpavis, NULL),
                                           DUPdoDupNode (GENARRAY_SHAPE (genarray))),
                                NULL);
            AVIS_SSAASSIGN (shpavis) = ass;

            INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), ass);

            /*
             * newshp = cat( shp', defshp);
             */
            rhs = TCmakePrf2 (F_cat_VxV, TBmakeId (shpavis), TBmakeId (defshpavis));

            /*
             * ATTENTION!
             *
             * the type of a function is a product type as it potentially
             * may return multiple results. As we need the type of the singe
             * result the F_cat_VxV prf yields, we have to grab it from
             * the returned product type!
             */
            newshptype = NTCnewTypeCheck_Expr (rhs);

            newshpavis = TBmakeAvis (TRAVtmpVar (), TYgetProductMember (newshptype, 0));
            newshptype = TYfreeTypeConstructor (newshptype);

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (newshpavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

            ass = TBmakeAssign (TBmakeLet (TBmakeIds (newshpavis, NULL), rhs), NULL);
            AVIS_SSAASSIGN (newshpavis) = ass;

            INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), ass);

            /*
             * array = [:BASETYPE]
             */
            scalar = TYgetScalar (IDS_NTYPE (LET_IDS (INFO_LET (arg_info))));
            rhs = TCmakeVector (TYmakeAKS (TYcopyType (scalar), SHmakeShape (0)), NULL);

            arrayavis = TBmakeAvis (TRAVtmpVar (), NTCnewTypeCheck_Expr (rhs));

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (arrayavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

            ass = TBmakeAssign (TBmakeLet (TBmakeIds (arrayavis, NULL), rhs), NULL);
            AVIS_SSAASSIGN (arrayavis) = ass;

            INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), ass);

            /*
             * replace with-loop with
             * reshape( newshp, array);
             */
            arg_node = FREEdoFreeNode (arg_node);
            arg_node
              = TCmakePrf2 (F_reshape_VxA, TBmakeId (newshpavis), TBmakeId (arrayavis));
        }

        if (shpshape != NULL) {
            shpshape = SHfreeShape (shpshape);
        }

        if (exprshape != NULL) {
            exprshape = SHfreeShape (exprshape);
        }

        shptype = TYfreeType (shptype);
        exprtype = TYfreeType (exprtype);
    }

    /*
     * Remove unused codes (former default codes)
     * This can only be performed iff the wl has not been replaced
     */
    if (NODE_TYPE (arg_node) == N_with) {
        WITH_CODE (arg_node) = RemoveUnusedCodes (WITH_CODE (arg_node));
    }

    INFO_WL (arg_info) = NULL;
    INFO_LET (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGdoPartitionGeneration( node *arg_node)
 *
 *   @brief  Starting point for the partition generation if it was called
 *           from main.
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/

node *
WLPGdoWlPartitionGeneration (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("WLPGdoWlPartitionGeneration");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "WLPGdoWlPartitionGeneration not started with module node");

    DBUG_PRINT ("WLPG", ("starting WLPGdoWlPartitionGeneration"));

    arg_info = MakeInfo ();
    INFO_SUBPHASE (arg_info) = SP_mod;

    TRAVpush (TR_wlpg);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGdoPartitionGenerationOpt( node *arg_node)
 *
 *   @brief  Starting point for the partition generation if it was called
 *           from optimize.
 *
 *   @param  node *arg_node:  N_fundef
 *   @return node *        :  transformed N_fundef
 ******************************************************************************/

node *
WLPGdoWlPartitionGenerationOpt (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("WLPGdoWlPartitionGenerationOpt");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "WLPGdoWlPartitionGenerationOpt not started with fundef node");

    DBUG_PRINT ("WLPG", ("starting WLPGdoWlPartitionGenerationOpt"));

    arg_info = MakeInfo ();
    INFO_SUBPHASE (arg_info) = SP_func;

    TRAVpush (TR_wlpg);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}
