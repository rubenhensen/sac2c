/**
 *
 * @file wlanalysis.c
 *
 * This traversal performs, among other things, these checks on WL
 * bounds:
 *     bounds have same shape as result (genarray shape)
 *     BOUND1 >= 0.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "new_types.h"
#include "type_utils.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "shape.h"
#include "DupTree.h"
#include "globals.h"

#define DBUG_PREFIX "WLPG"
#include "debug.h"

#include "traverse.h"
#include "constants.h"
#include "WLPartitionGeneration.h"
#include "wlanalysis.h"
#include "ctinfo.h"
#include "pattern_match.h"

typedef enum {
    GV_constant = 0,
    GV_struct_constant = 1,
    GV_known_shape = 2,
    GV_unknown_shape = 3
} gen_shape_t;

/**
 * INFO structure
 */
struct INFO {
    node *wl;
    node *fundef;
    node *let;
    node *nassigns;
    gen_prop_t genprop;
    gen_shape_t genshp;
    int shpext;
};

#define INFO_WL(n) (n->wl)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LET(n) (n->let)
#define INFO_NASSIGNS(n) (n->nassigns)
#define INFO_GENPROP(n) (n->genprop)
#define INFO_GENSHP(n) (n->genshp)
#define INFO_SHPEXT(n) (n->shpext)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_WL (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_LET (result) = NULL;
    INFO_NASSIGNS (result) = NULL;
    INFO_GENPROP (result) = GPT_empty;
    INFO_GENSHP (result) = GV_constant;
    INFO_SHPEXT (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

#ifndef DBUG_OFF
static char *gen_prop_str[]
  = {"GPT_unknown", "GPT_empty", "GPT_full", "GPT_partial", "GPT_unknown"};

#endif

/** <!--********************************************************************-->
 *
 * @fn void VectVar2StructConst( node **expr, node *fundef, info *arg_info)
 *
 *   @brief expects (expr) to point to an identifier and generates as many
 *          new assigns as INFO_SHPEXT indicates.
 *          I.e., if **expr is a 3-element GENERATOR_BOUND1 LB, we might have:
 *             ( LB <= iv < UB)
 *          This will be replaced by the following:
 *             shp0 = _sel_VxA_([0], LB);
 *             shp1 = _sel_VxA_([0], LB);
 *             shp2 = _sel_VxA_([0], LB);
 *             LB'  = [shp0, shp1, shp2];
 *             ( LB' <= iv < UB)
 *
 *           Obviously, this requires that LB be AKS or better.
 *   @param  node *expr   :  expr
 *           node *fundef :  N_fundef
 *           info *arg_info: your basic arg_info node
 *
 *   @return nothing
 ******************************************************************************/
static void
VectVar2StructConst (node **expr, node *fundef, info *arg_info)
{
    int i;
    node *idx_avis, *res_avis, *nassigns, *exprs;
    node *lb_avis;
    node *lb_assign;
    node *lb_id;
    int shpext;

    DBUG_ENTER ();

    DBUG_ASSERT (*expr != NULL, "Expr is empty");
    DBUG_ASSERT (NODE_TYPE (*expr) == N_id, "VectVar2StructConst not called with N_id");

    shpext = INFO_SHPEXT (arg_info);
    nassigns = NULL;
    exprs = NULL;

    for (i = shpext - 1; i >= 0; i--) {
        idx_avis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                         SHcreateShape (1, 1)));
        res_avis = TBmakeAvis (TRAVtmpVarName (ID_NAME (*expr)),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        fundef
          = TCaddVardecs (fundef, TBmakeVardec (idx_avis, TBmakeVardec (res_avis, NULL)));
        /**
         * res_avis = F_sel_VxA( idx_avis, expr);
         */
        nassigns = TBmakeAssign (TBmakeLet (TBmakeIds (res_avis, NULL),
                                            TCmakePrf2 (F_sel_VxA, TBmakeId (idx_avis),
                                                        DUPdoDupNode (*expr))),
                                 nassigns);
        AVIS_SSAASSIGN (res_avis) = nassigns;

        /**
         * idx_avis = [i];
         */
        nassigns = TBmakeAssign (TBmakeLet (TBmakeIds (idx_avis, NULL),
                                            TCmakeIntVector (TCmakeExprsNum (i))),
                                 nassigns);
        AVIS_SSAASSIGN (idx_avis) = nassigns;

        /**
         * Now, we build the exprs chain:
         */
        exprs = TBmakeExprs (TBmakeId (res_avis), exprs);
    }

    INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), nassigns);

    /**
     * Now, we build LB' and the assign to it: LB' = [shp0, shp1, shp2];
     */
    if (global.ssaiv) {
        lb_avis
          = TBmakeAvis (TRAVtmpVarName (ID_NAME (*expr)),
                        TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, shpext)));
        fundef = TCaddVardecs (fundef, TBmakeVardec (lb_avis, NULL));
        lb_assign
          = TBmakeAssign (TBmakeLet (TBmakeIds (lb_avis, NULL), TCmakeIntVector (exprs)),
                          NULL);
        AVIS_SSAASSIGN (lb_avis) = lb_assign;
        INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), lb_assign);
        lb_id = TBmakeId (lb_avis);
    } else {
        lb_id = TCmakeIntVector (exprs);
    }
    /*
     * Now, replace the GENERATOR or whatever it may be.
     */
    *expr = FREEdoFreeTree (*expr);
    *expr = lb_id;

    DBUG_RETURN ();
}

static node *
PropagateConstArrayIdentifier (node *expr)
{
    node *result = expr;
    node *assign;
    node *defexpr;

    DBUG_ENTER ();

    if ((NODE_TYPE (expr) == N_id) && (AVIS_SSAASSIGN (ID_AVIS (expr)) != NULL)) {
        assign = AVIS_SSAASSIGN (ID_AVIS (expr));

        if (NODE_TYPE (ASSIGN_STMT (assign)) == N_let) {
            defexpr = LET_EXPR (ASSIGN_STMT (assign));

            if (NODE_TYPE (defexpr) == N_array) {
                expr = FREEdoFreeTree (expr);
                result = DUPdoDupTree (defexpr);
            } else if (NODE_TYPE (defexpr) == N_id) {
                expr = FREEdoFreeTree (expr);
                result = DUPdoDupTree (defexpr);
                result = PropagateConstArrayIdentifier (result);
            }
        }
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn gen_shape_t DetectVectorConstants( node *arg_node)
 *
 *   @brief expects argument to point to an identifier.
 *          The argument is not changed.
 *
 *          It returns GV_constant iff argument is constant or NULL.
 *
 *   @param  node *arg_node
 *   @return gen_shape_t  :  GV_constant, GV_struct_constant, GV_known_shape,
 *                           GV_unknown_shape
 ******************************************************************************/
static gen_shape_t
DetectVectorConstants (node *arg_node)
{
    gen_shape_t gshape;
    ntype *t;
    pattern *pat;

    DBUG_ENTER ();

    if (NULL != arg_node) {

        DBUG_ASSERT (NODE_TYPE (arg_node) == N_id,
                     "nonN_id found as argument to DetectVectorConstants");
        t = AVIS_TYPE (ID_AVIS (arg_node));
        pat = PMarray (0, 1, PMskip (0));

        if (TYisAKV (t)) {
            gshape = GV_constant;
        } else if (PMmatchFlat (pat, arg_node)) {
            gshape = GV_struct_constant;
        } else if (TUshapeKnown (t)) {
            gshape = GV_known_shape;
        } else {
            gshape = GV_unknown_shape;
        }
        pat = PMfree (pat);
    } else {
        gshape = GV_constant; /* Elided GENERATOR_STEP, GENERATOR_WIDTH
                               * has value of (vector) 1, ergo constant.
                               */
    }

    DBUG_RETURN (gshape);
}

/******************************************************************************
 *
 * function:
 *   void CheckBounds( node *wl, shape *max_shp)
 *
 * description:
 *   expects the BOUND expression for lb and ub to be constant!
 *   expects also the WL either to by WO_modarray or WO_genarray!
 *
 *   Checks whether lb and ub fit into the shape given by max_shp.
 *   Also checks that lb and ub are the same shape.
 *
 *   The long-term development goal is to replace CropBounds by CheckBounds.
 *
 *
 *<!--********************************************************************-->*/

static void
CheckBounds (node *arg_node, shape *max_shp)
{
    node *lbe;
    node *ube;
    node *lbv;
    node *ubv;
    constant *lbco;
    constant *ubco;

    int dim;
    int lbnum, ubnum, tnum;

    DBUG_ENTER ();

    DBUG_ASSERT (((NODE_TYPE (WITH_WITHOP (arg_node)) == N_modarray)
                  || (NODE_TYPE (WITH_WITHOP (arg_node)) == N_genarray)),
                 "CheckBounds applied to wrong WL type!");

    /* Turn both arguments in N_array values for now */
    lbco = COaST2Constant (WITH_BOUND1 (arg_node));
    lbv = COconstant2AST (lbco);
    lbco = COfreeConstant (lbco);

    ubco = COaST2Constant (WITH_BOUND2 (arg_node));
    ubv = COconstant2AST (ubco);
    ubco = COfreeConstant (ubco);

    DBUG_ASSERT (((N_array == NODE_TYPE (lbv)) && (N_array == NODE_TYPE (ubv))),
                 "CheckBounds expected N_array BOUNDS");

    lbe = ARRAY_AELEMS (lbv);
    ube = ARRAY_AELEMS (ubv);

    dim = 0;
    while (lbe) {
        DBUG_ASSERT (ube != NULL,
                     "upper WL bound has lower dimensionality than lower bound.");
        DBUG_ASSERT (((NODE_TYPE (EXPRS_EXPR (lbe)) == N_num)
                      && (NODE_TYPE (EXPRS_EXPR (ube)) == N_num)),
                     "generator bounds must be constant!");
        lbnum = NUM_VAL (EXPRS_EXPR (lbe));
        ubnum = NUM_VAL (EXPRS_EXPR (ube));

        DBUG_ASSERT (dim < SHgetDim (max_shp),
                     "dimensionality of lb greater than that of the result!");
        tnum = SHgetExtent (max_shp, dim);
        if (lbnum < 0) {
            NUM_VAL (EXPRS_EXPR (lbe)) = 0;
            CTIerror (NODE_LOCATION (arg_node),
                      "Lower bound of WL-generator in dim %d below zero: %d", dim,
                      lbnum);
        }
        if (ubnum > tnum) {
            NUM_VAL (EXPRS_EXPR (ube)) = tnum;
            CTIerror (NODE_LOCATION (arg_node),
                      "Upper bound of WL-generator in dim %d greater than shape %d: %d",
                      dim, tnum, ubnum);
        }

        dim++;
        lbe = EXPRS_NEXT (lbe);
        ube = EXPRS_NEXT (ube);
    }
    DBUG_ASSERT (NULL == ube,
                 "lower WL bound has lower dimensionality than upper bound.");

    lbv = FREEdoFreeTree (lbv);
    ubv = FREEdoFreeTree (ubv);
    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn gen_shape_t PropagateVectorConstants( node **expr)
 *
 *   @brief expects (*expr) to point either to a scalar constant, to an N_array,
 *          or to an identifyer.
 *          If (**expr) is constant or structural constant, the according node
 *          (N_array / scalar) is freshly generated and (*expr) is modified to
 *          point to the new node. The old one is freed!
 *          If (**expr) is an identifier, the node is checked if it is possible
 *          to create an structural constant.
 *          It returns GV_constant iff (**expr) is constant or
 *          (*expr) == NULL!!
 *
 *   @param  node **expr  :  expr
 *   @return gen_shape_t  :  GV_constant, GV_struct_constant, GV_known_shape,
 *                           GV_unknown_shape
 ******************************************************************************/
static gen_shape_t
PropagateVectorConstants (node **expr)
{
    constant *const_expr;
    gen_shape_t gshape;

    DBUG_ENTER ();

    if (global.ssaiv) {
        gshape = DetectVectorConstants (*expr);
    } else {
        gshape = GV_unknown_shape;

        if ((*expr) != NULL) {
            const_expr = COaST2Constant ((*expr));
            if (const_expr != NULL) {
                gshape = GV_constant;
                (*expr) = FREEdoFreeTree (*expr);
                (*expr) = COconstant2AST (const_expr);
                const_expr = COfreeConstant (const_expr);

            } else {
                if ((NODE_TYPE (*expr) == N_id)
                    && TUisIntVect (AVIS_TYPE (ID_AVIS (*expr)))) {
                    /*
                     * type-wise this is an int-vector, so lets see
                     * whether we can find an N_array node for it
                     */
                    *expr = PropagateConstArrayIdentifier (*expr);
                }

                if (NODE_TYPE (*expr) == N_array) {
                    gshape = GV_struct_constant;
                } else {
                    if (TUshapeKnown (ID_NTYPE (*expr))) {
                        gshape = GV_known_shape;
                    }
                }
            }
        } else {
            gshape = GV_constant;
        }
    }
    DBUG_RETURN (gshape);
}

/******************************************************************************
 *
 * function:
 *   node * CropBounds( node *wl, shape *max_shp)
 *
 * description:
 *   expects the bound expression for lb and ub to be constants!
 *   expects also the WL either to by WO_modarray or WO_genarray!
 *
 *   Checks, whether lb and ub fit into the shape given by max_shp.
 *   If they exceed max_shp, they are "fitted"!
 *   The potentially modified WL is returned.
 *
 ******************************************************************************/

static node *
CropBounds (node *wl, shape *max_shp)
{
    node *lbe, *ube;
    int dim;
    int lbnum, ubnum, tnum;

    DBUG_ENTER ();

    DBUG_ASSERT (((NODE_TYPE (WITH_WITHOP (wl)) == N_modarray)
                  || (NODE_TYPE (WITH_WITHOP (wl)) == N_genarray)),
                 "CropBounds applied to wrong WL type!");
    lbe = ARRAY_AELEMS (WITH_BOUND1 (wl));
    ube = ARRAY_AELEMS (WITH_BOUND2 (wl));

    dim = 0;
    while (lbe) {
        DBUG_ASSERT (ube != NULL, "dimensionality differs in lower and upper bound!");
        DBUG_ASSERT (((NODE_TYPE (EXPRS_EXPR (lbe)) == N_num)
                      && (NODE_TYPE (EXPRS_EXPR (ube)) == N_num)),
                     "generator bounds must be constant!");
        lbnum = NUM_VAL (EXPRS_EXPR (lbe));
        ubnum = NUM_VAL (EXPRS_EXPR (ube));

        DBUG_ASSERT (dim < SHgetDim (max_shp),
                     "dimensionality of lb greater than that of the result!");
        tnum = SHgetExtent (max_shp, dim);
        if (lbnum < 0) {
            NUM_VAL (EXPRS_EXPR (lbe)) = 0;
            CTIerror (NODE_LOCATION (wl),
                      "Lower bound of WL-generator in dim %d below zero: %d", dim,
                      lbnum);
        }
        if (ubnum > tnum) {
            NUM_VAL (EXPRS_EXPR (ube)) = tnum;
            CTIerror (NODE_LOCATION (wl),
                      "Upper bound of WL-generator in dim %d greater than shape %d: "
                      "%d",
                      dim, tnum, ubnum);
        }

        dim++;
        lbe = EXPRS_NEXT (lbe);
        ube = EXPRS_NEXT (ube);
    }
    DBUG_RETURN (wl);
}

/******************************************************************************
 *
 * function:
 *   gen_prop_t ComputeGeneratorProperties( node *wl, shape *max_shp)
 *
 * description:
 *   Computes the properties of the given WLs generator.
 *
 ******************************************************************************/

static gen_prop_t
ComputeGeneratorProperties (node *wl, shape *max_shp)
{
    node *lbe, *ube, *steps;
    gen_prop_t res = GPT_unknown;
    bool const_bounds, non_empty_bounds;
    constant *lbc, *ubc, *shpc, *tmpc, *tmp;
    shape *sh;

    DBUG_ENTER ();

    lbe = WITH_BOUND1 (wl);
    ube = WITH_BOUND2 (wl);
    steps = WITH_STEP (wl);

    lbc = COaST2Constant (lbe);
    ubc = COaST2Constant (ube);
    if (max_shp != NULL) {
        shpc = COmakeConstantFromShape (max_shp);
    } else {
        shpc = NULL;
    }
    const_bounds = ((lbc != NULL) && (ubc != NULL));

    /**
     * First, we check for emptyness:
     * (this is done prior to checking on GPT_full, as we may have both properties
     *  GPT_empty and GPT_full, in which case we prefer to obtain GPT_empty as
     *  result....)
     */
    if (const_bounds) {
        non_empty_bounds = (SHgetUnrLen (COgetShape (lbc)) > 0);

        if (!non_empty_bounds) {
            tmpc = COge (lbc, ubc, NULL);
            if (COisTrue (tmpc, FALSE)) {
                res = GPT_empty;
            }
            tmpc = COfreeConstant (tmpc);
        }
    }

    if (res == GPT_unknown) {
        if (NODE_TYPE (WITH_WITHOP (wl)) == N_fold
            || NODE_TYPE (WITH_WITHOP (wl)) == N_propagate) {
            res = GPT_full;
        } else {
            /**
             * We are dealing with a modarray or a genarray WL here!
             */
            if (const_bounds && (shpc != NULL)) {
                /**
                 * In order to obtain be a full partition,
                 * ubc must be a prefix of shpc,
                 * all elements of lbc must be zero
                 * and there must be no step vector
                 */
                sh = COgetShape (ubc);
                tmp = COmakeConstantFromShape (sh);
                tmpc = COtake (tmp, shpc, NULL);

                tmp = COfreeConstant (tmp);
                shpc = COfreeConstant (shpc);

                shpc = tmpc;

                tmpc = COeq (ubc, shpc, NULL);
                if (COisZero (lbc, TRUE) && COisTrue (tmpc, TRUE)) {
                    if (steps == NULL) {
                        res = GPT_full;
                    } else {
                        res = GPT_partial;
                    }
                } else {
                    res = GPT_partial;
                }
                tmpc = COfreeConstant (tmpc);
            } else if (const_bounds) {
                /**
                 * the bounds are constant and the shape is unknown,
                 * we assume a shape greater than the bounds!
                 */
                res = GPT_partial;
            }
        }
    }

    /* Clean up constants */
    shpc = (shpc != NULL ? COfreeConstant (shpc) : NULL);
    ubc = (ubc != NULL ? COfreeConstant (ubc) : NULL);
    lbc = (lbc != NULL ? COfreeConstant (lbc) : NULL);

    DBUG_PRINT ("generator property of with loop in line %zu : %s", global.linenum,
                gen_prop_str[res]);
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLAwith(node *arg_node, info *arg_info)
 *
 *   @brief  start traversal of this WL and store information in arg_info
 *           node. The only N_part node (besides possible default part)
 *           is traversed.
 *
 *   @param  node *arg_node:  N_with
 *           info *arg_info:  N_info
 *   @return node *        :  N_with
 ******************************************************************************/

node *
WLAwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * traverse the one and only (!) regular PART.
     * Besides minor changes in the generator, two values are computed
     * during this traversal:
     *
     *  INFO_WLPG_GENSHP(arg_info) and INFO_WLPG_GENPROP(arg_info) !!
     */
    DBUG_ASSERT (WITH_PART (arg_node) != NULL,
                 "WITH_PARTS is -1 although no PART is available!");
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    /*
     * Now, we traverse the WITHOP sons for propagating (structural) constants.
     */
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLApart(node *arg_node, info *arg_info)
 *
 *   @brief infer shape property of indexvector and
 *          traverse generator to propagate arrays
 *
 *   @param  node *arg_node:  N_part
 *           info *arg_info:  N_info
 *   @return node *        :  N_part
 ******************************************************************************/

node *
WLApart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    DBUG_ASSERT ((PART_NEXT (arg_node) == NULL)
                   || (NODE_TYPE (PART_GENERATOR (PART_NEXT (arg_node))) == N_default),
                 "Second partition is no default partition!");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLAgenerator(node *arg_node, info *arg_info)
 *
 *   @brief  bounds, step and width vectors are substituted into the generator.
 *           Generators that surmount the array bounds (like [-5,3] or
 *           [11,10] > [maxdim1,maxdim2] = [10,10]) are changed to fitting
 *           gens.
 *           Via INFO_WLPG_GENPROP( arg_info) the status of the generator is
 *           returned. Possible values are (poss. ambiguities are resolved top
 *           to bottom):
 *           GPT_empty   : the generator is empty!
 *           GPT_full    : the generator covers the entire range!
 *           GPT_partial : 1. the generator has constant upper and lower bounds,
 *                            but  - most likely  - only a part is covered!
 *                         2. the generator contains at least one structural constant
 *                            at position of bounds, step and width
 *           GPT_unknown : we don't know anything !
 *
 *           Via INFO_WLPG_GENSHP( arg_info) the status of the bounds, step and
 *           width vectors is returned.
 *           Possible values are (poss. ambiguities are resolved top
 *           to bottom):
 *           GV_constant        : the vectors are constant!
 *           GV_struct_constant : the vectors are at least structural constants!
 *           GV_unknown_shape   : we don't know anything !
 *
 *   @param  node *arg_node:  N_generator
 *           info *arg_info:  N_info
 *   @return node *        :  N_generator
 ******************************************************************************/

node *
WLAgenerator (node *arg_node, info *arg_info)
{
    node *wln, *f_def;
    node *let_ids;
    shape *shp;
    ntype *type;
    bool check_bounds, check_stepwidth;
    gen_prop_t gprop;
    gen_shape_t current_shape, gshape;

    DBUG_ENTER ();

    wln = INFO_WL (arg_info);
    f_def = INFO_FUNDEF (arg_info);

    /*
     * First, we try to propagate (structural) constants into all sons:
     */
    current_shape = PropagateVectorConstants (&(GENERATOR_BOUND1 (arg_node)));
    if (current_shape >= GV_known_shape) {
        VectVar2StructConst (&(GENERATOR_BOUND1 (arg_node)), f_def, arg_info);
        gshape = GV_struct_constant;
    } else {
        gshape = current_shape;
    }

    current_shape = PropagateVectorConstants (&(GENERATOR_BOUND2 (arg_node)));
    if (current_shape >= GV_known_shape) {
        VectVar2StructConst (&GENERATOR_BOUND2 (arg_node), f_def, arg_info);
        current_shape = GV_struct_constant;
    }
    if (gshape < current_shape) {
        gshape = current_shape;
    }
    check_bounds = (gshape == GV_constant);

    current_shape = PropagateVectorConstants (&(GENERATOR_STEP (arg_node)));
    if (current_shape >= GV_known_shape) {
        VectVar2StructConst (&GENERATOR_STEP (arg_node), f_def, arg_info);
        current_shape = GV_struct_constant;
    }
    if (gshape < current_shape) {
        gshape = current_shape;
    }

    check_stepwidth = (current_shape <= GV_struct_constant);

    current_shape = PropagateVectorConstants (&(GENERATOR_WIDTH (arg_node)));
    if ((current_shape == GV_known_shape) || (current_shape == GV_unknown_shape)) {
        VectVar2StructConst (&GENERATOR_WIDTH (arg_node), f_def, arg_info);
        current_shape = GV_struct_constant;
    }
    if (gshape < current_shape) {
        gshape = current_shape;
    }

    check_stepwidth
      = (check_stepwidth
         && ((current_shape == GV_constant) || (current_shape <= GV_struct_constant)));

    /**
     * find out the generator properties:
     */
    let_ids = LET_IDS (INFO_LET (arg_info));
    type = AVIS_TYPE (IDS_AVIS (let_ids));

    if (TUshapeKnown (type)) {
        shp = TYgetShape (type);
        if (check_bounds
            && ((NODE_TYPE (WITH_WITHOP (wln)) == N_modarray)
                || (NODE_TYPE (WITH_WITHOP (wln)) == N_genarray))) {
            if (global.ssaiv) {
                CheckBounds (wln, shp);
            } else {
                wln = CropBounds (wln, shp);
            }
        }
        gprop = ComputeGeneratorProperties (wln, shp);
    } else {
        gprop = ComputeGeneratorProperties (wln, NULL);
    }

    if (gshape == GV_struct_constant) {
        if (NODE_TYPE (WITH_WITHOP (wln)) == N_fold
            || NODE_TYPE (WITH_WITHOP (wln)) == N_propagate) {
            gprop = GPT_full;
        } else {
            gprop = GPT_partial;
        }
    }

#if 0
  if (check_stepwidth){
    /* normalize step and width */
    switch (WLPGnormalizeStepWidth( &GENERATOR_STEP( arg_node),
                                    &GENERATOR_WIDTH( arg_node))) {
    case 1:
      CTIabort (NODE_LOCATION (wln), "Component of width greater than step");
      break;
    case 2:
      CTIabort (NODE_LOCATION (wln), "Component of width less than zero");
      break;
    case 3:
      CTIabort (NODE_LOCATION (wln), "Width vector without step vector");
      break;
    }
  }
#endif

    INFO_GENPROP (arg_info) = gprop;
    INFO_GENSHP (arg_info) = gshape;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLAgenarray( node *arg_node, info *arg_info)
 *
 *   @brief Substitutes GENARRAY_SHAPE into the N_genarray node.
 *
 *   @param  node *arg_node:  N_genarray
 *           info *arg_info:  N_info
 *   @return node *        :  N_genarray
 ******************************************************************************/

node *
WLAgenarray (node *arg_node, info *arg_info)
{
    node *fundef;
    gen_shape_t current_shape;

    DBUG_ENTER ();

    fundef = INFO_FUNDEF (arg_info);

    GENARRAY_SHAPE (arg_node) = TRAVopt(GENARRAY_SHAPE (arg_node), arg_info);

    current_shape = PropagateVectorConstants (&(GENARRAY_SHAPE (arg_node)));

    if ((current_shape == GV_known_shape) || (current_shape == GV_unknown_shape)) {
        VectVar2StructConst (&GENARRAY_SHAPE (arg_node), fundef, arg_info);
        current_shape = GV_struct_constant;
    }

    if (INFO_GENSHP (arg_info) < current_shape) {
        INFO_GENSHP (arg_info) = current_shape;

        if (current_shape == GV_unknown_shape) {
            INFO_GENPROP (arg_info) = GPT_unknown;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * node *WLAdoWlAnalysis( node *arg_node, node *fundef, node *let,
 *                        node** nassigns,
 *                        gen_prop_t *gprop)
 *
 *   @brief  Starting point for traversal WlAnalysis.
 *
 *   @param  node *arg_node       :  N_with
 *           node *fundef         :  N_fundef
 *           node *let            :  N_let of current WL
 *           node **nassigns      :  returning N_assign chain of new assignments
 *                                   which has to be placed before current WL
 *           gen_prop_t **gprop   :  Via gprop the status of the generator is
 *                                   returned. Possible values are
 *                                    (poss. ambiguities are resolved top
 *                                   to bottom):
 *                  GPT_empty   : the generator is empty!
 *                  GPT_full    : the generator covers the entire range!
 *                  GPT_partial : 1. the generator has constant
 *                                   upper and lower bounds,
 *                                   but  - most likely  - only
 *                                   a part is covered!
 *                                2. the generator contains
 *                                   at least one structural
 *                                   constant at position of
 *                                   bounds, step and width
 *                  GPT_unknown : we don't know anything
 *
 *   @return node *               :  modified N_with
 ******************************************************************************/

node *
WLAdoWlAnalysis (node *arg_node, node *fundef, node *let, node **nassigns,
                 gen_prop_t *gprop)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_with,
                 "WLAnalysis not started with N_with node");

    DBUG_ASSERT (TUshapeKnown (IDS_NTYPE (WITH_VEC (arg_node))),
                 "Only with-loops with AKS index vector can be modified");

    DBUG_ASSERT ((fundef != NULL && NODE_TYPE (fundef) == N_fundef), "no N_fundef found");

    DBUG_ASSERT ((let != NULL && NODE_TYPE (let) == N_let), "no N_let found");

    DBUG_ASSERT ((*nassigns) == NULL, "nassigns should point to Null");

    DBUG_PRINT ("starting with-loop analysis");

    arg_info = MakeInfo ();

    INFO_WL (arg_info) = arg_node;
    INFO_FUNDEF (arg_info) = fundef;
    INFO_LET (arg_info) = let;
    INFO_SHPEXT (arg_info) = SHgetUnrLen (TYgetShape (IDS_NTYPE (WITH_VEC (arg_node))));

    TRAVpush (TR_wla);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    (*gprop) = INFO_GENPROP (arg_info);
    (*nassigns) = INFO_NASSIGNS (arg_info);

    DBUG_PRINT ("with-loop analysis complete");

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
