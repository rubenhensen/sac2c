/*
 *
 * $Id$
 *
 */

/**
 *
 * @file wlanalysis.c
 *
 * In this traversal
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
#include "dbug.h"
#include "traverse.h"
#include "constants.h"
#include "SSAConstantFolding.h"
#include "WLPartitionGeneration.h"
#include "wlanalysis.h"
#include "ctinfo.h"

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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

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
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

#ifndef DBUG_OFF
static char *gen_prop_str[]
  = {"GPT_unknown", "GPT_empty", "GPT_full", "GPT_partial", "GPT_unknown"};

#endif

/** <!--********************************************************************-->
 *
 * @fn node *VectVar2StructConst( node **expr, node *fundef, int shpext)
 *
 *   @brief expects (expr) to point to an identifier and generates as many
 *          new assigns as (shpext) indicates.
 *
 *   @param  node *expr   :  expr
 *           node *fundef :  N_fundef
 *           int   shpext :
 *   @return node *       :  a chained list of N_assign nodes
 ******************************************************************************/
static node *
VectVar2StructConst (node **expr, node *fundef, int shpext)
{
    int i;
    node *idx_avis, *res_avis, *nassigns, *exprs;

    DBUG_ENTER ("VectVar2StructConst");

    DBUG_ASSERT ((*expr != NULL), "Expr is empty");
    DBUG_ASSERT ((NODE_TYPE (*expr) == N_id), "VectVar2StructConst not called with N_id");

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
    *expr = FREEdoFreeTree (*expr);
    *expr = TCmakeIntVector (exprs);

    DBUG_RETURN (nassigns);
}

/** <!--********************************************************************-->
 *
 * @fn gen_shape_t PropagateArrayConstants( node **expr)
 *
 *   @brief expects (*expr) to point either to a scalar constant, to an array,
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
PropagateArrayConstants (node **expr)
{
    constant *const_expr;
    struct_constant *sco_expr;
    node *tmp;
    gen_shape_t gshape;

    DBUG_ENTER ("PropagateArrayConstants");

    gshape = GV_unknown_shape;

    if ((*expr) != NULL) {
        const_expr = COaST2Constant ((*expr));
        if (const_expr != NULL) {
            gshape = GV_constant;
            (*expr) = FREEdoFreeTree (*expr);
            (*expr) = COconstant2AST (const_expr);
            const_expr = COfreeConstant (const_expr);

        } else {
            sco_expr = CFscoExpr2StructConstant ((*expr));
            if (sco_expr != NULL) {
                gshape = GV_struct_constant;
                /*
                 * as the sco_expr may share some subexpressions with (*expr),
                 * we have to duplicate these BEFORE deleting (*expr)!!!
                 */
                tmp = CFscoDupStructConstant2Expr (sco_expr);
                (*expr) = FREEdoFreeTree (*expr);
                (*expr) = tmp;
                sco_expr = CFscoFreeStructConstant (sco_expr);

            } else {
                if (TUshapeKnown (ID_NTYPE (*expr))) {
                    gshape = GV_known_shape;
                }
            }
        }
    } else {
        gshape = GV_constant;
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

    DBUG_ENTER ("CropBounds");

    DBUG_ASSERT (((NODE_TYPE (WITH_WITHOP (wl)) == N_modarray)
                  || (NODE_TYPE (WITH_WITHOP (wl)) == N_genarray)),
                 "CropBounds applied to wrong WL type!");
    lbe = ARRAY_AELEMS (WITH_BOUND1 (wl));
    ube = ARRAY_AELEMS (WITH_BOUND2 (wl));

    dim = 0;
    while (lbe) {
        DBUG_ASSERT ((ube != NULL), "dimensionality differs in lower and upper bound!");
        DBUG_ASSERT (((NODE_TYPE (EXPRS_EXPR (lbe)) == N_num)
                      && (NODE_TYPE (EXPRS_EXPR (ube)) == N_num)),
                     "generator bounds must be constant!");
        lbnum = NUM_VAL (EXPRS_EXPR (lbe));
        ubnum = NUM_VAL (EXPRS_EXPR (ube));

        DBUG_ASSERT ((dim < SHgetDim (max_shp)),
                     "dimensionality of lb greater than that of the result!");
        tnum = SHgetExtent (max_shp, dim);
        if (lbnum < 0) {
            NUM_VAL (EXPRS_EXPR (lbe)) = 0;
            CTIerrorLine (NODE_LINE (wl),
                          "Lower bound of WL-generator in dim %d below zero: %d", dim,
                          lbnum);
        }
        if (ubnum > tnum) {
            NUM_VAL (EXPRS_EXPR (ube)) = tnum;
            CTIerrorLine (NODE_LINE (wl),
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
    node *lbe, *ube, *steps, *width;
    gen_prop_t res = GPT_unknown;
    bool const_bounds, non_empty_bounds;
    constant *lbc, *ubc, *shpc, *tmpc, *tmp;
    shape *sh;

    DBUG_ENTER ("ComputeGeneratorProperties");

    lbe = WITH_BOUND1 (wl);
    ube = WITH_BOUND2 (wl);
    steps = WITH_STEP (wl);
    width = WITH_WIDTH (wl);

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

        if (non_empty_bounds) {
            tmpc = COge (lbc, ubc);
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
                 * and there must be not step vector
                 */
                sh = COgetShape (ubc);
                tmp = COmakeConstantFromShape (sh);
                tmpc = COtake (tmp, shpc);

                tmp = COfreeConstant (tmp);
                shpc = COfreeConstant (shpc);

                shpc = tmpc;

                tmpc = COeq (ubc, shpc);
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

    DBUG_PRINT ("WLPG", ("generator property of with loop in line %d : %s",
                         global.linenum, gen_prop_str[res]));
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
    DBUG_ENTER ("WLAwith");

    /*
     * traverse the one and only (!) regular PART.
     * Besides minor changes in the generator, two values are computed
     * during this traversal:
     *
     *  INFO_WLPG_GENSHP(arg_info) and INFO_WLPG_GENPROP(arg_info) !!
     */
    DBUG_ASSERT ((WITH_PART (arg_node) != NULL),
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
    DBUG_ENTER ("WLApart");

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
    node *wln, *f_def, *nassigns, *let_ids;
    shape *shp;
    ntype *type;
    bool check_bounds, check_stepwidth;
    gen_prop_t gprop;
    gen_shape_t current_shape, gshape;

    DBUG_ENTER ("WLAgenerator");

    wln = INFO_WL (arg_info);
    f_def = INFO_FUNDEF (arg_info);

    /*
     * First, we try to propagate (structural) constants into all sons:
     */
    current_shape = PropagateArrayConstants (&(GENERATOR_BOUND1 (arg_node)));
    if (current_shape >= GV_known_shape) {
        nassigns = VectVar2StructConst (&(GENERATOR_BOUND1 (arg_node)), f_def,
                                        INFO_SHPEXT (arg_info));
        gshape = GV_struct_constant;
        INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), nassigns);
    } else {
        gshape = current_shape;
    }

    current_shape = PropagateArrayConstants (&(GENERATOR_BOUND2 (arg_node)));
    if (current_shape >= GV_known_shape) {
        nassigns = VectVar2StructConst (&GENERATOR_BOUND2 (arg_node), f_def,
                                        INFO_SHPEXT (arg_info));
        current_shape = GV_struct_constant;
        INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), nassigns);
    }
    if (gshape < current_shape) {
        gshape = current_shape;
    }
    check_bounds = (gshape == GV_constant);

    current_shape = PropagateArrayConstants (&(GENERATOR_STEP (arg_node)));
    if (current_shape >= GV_known_shape) {
        nassigns = VectVar2StructConst (&GENERATOR_STEP (arg_node), f_def,
                                        INFO_SHPEXT (arg_info));
        current_shape = GV_struct_constant;
        INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), nassigns);
    }
    if (gshape < current_shape) {
        gshape = current_shape;
    }

    check_stepwidth = (current_shape <= GV_struct_constant);

    current_shape = PropagateArrayConstants (&(GENERATOR_WIDTH (arg_node)));
    if (current_shape >= GV_known_shape) {
        nassigns = VectVar2StructConst (&GENERATOR_WIDTH (arg_node), f_def,
                                        INFO_SHPEXT (arg_info));
        current_shape = GV_struct_constant;
        INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), nassigns);
    }
    if (gshape < current_shape) {
        gshape = current_shape;
    }

    check_stepwidth = (check_stepwidth && (current_shape <= GV_struct_constant));

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
            wln = CropBounds (wln, shp);
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

    if (check_stepwidth) {
        /* normalize step and width */
        switch (WLPGnormalizeStepWidth (&GENERATOR_STEP (arg_node),
                                        &GENERATOR_WIDTH (arg_node))) {
        case 1:
            CTIabortLine (NODE_LINE (wln), "Component of width greater than step");
            break;
        case 2:
            CTIabortLine (NODE_LINE (wln), "Component of width less than zero");
            break;
        case 3:
            CTIabortLine (NODE_LINE (wln), "Width vector without step vector");
            break;
        }
    }

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
    node *nassigns, *f_def;
    gen_shape_t current_shape;

    DBUG_ENTER ("WLAgenarray");

    f_def = INFO_FUNDEF (arg_info);

    if (GENARRAY_SHAPE (arg_node) != NULL) {
        GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    }

    current_shape = PropagateArrayConstants (&(GENARRAY_SHAPE (arg_node)));

    if (current_shape >= GV_known_shape) {
        nassigns = VectVar2StructConst (&GENARRAY_SHAPE (arg_node), f_def,
                                        INFO_SHPEXT (arg_info));
        current_shape = GV_struct_constant;
        INFO_NASSIGNS (arg_info) = TCappendAssign (INFO_NASSIGNS (arg_info), nassigns);
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
 * @fn node *WLAdoWlAnalysis(node *wl, gen_prob_t **gprob)
 *
 *   @brief  Starting point for traversal WlAnalysis.
 *
 *   @param  node *wl             :  N_with
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
WLAdoWlAnalysis (node *wl, node *fundef, node *let, node **nassigns, gen_prop_t *gprop)
{
    info *arg_info;

    DBUG_ENTER ("WLAdoWlAnalysis");

    DBUG_ASSERT ((NODE_TYPE (wl) == N_with), "WLAnalysis not started with N_with node");

    DBUG_ASSERT (TUshapeKnown (IDS_NTYPE (WITH_VEC (wl))),
                 "Only with-loops with AKS index vector can be modified");

    DBUG_ASSERT ((fundef != NULL && NODE_TYPE (fundef) == N_fundef), "no N_fundef found");

    DBUG_ASSERT ((let != NULL && NODE_TYPE (let) == N_let), "no N_let found");

    DBUG_ASSERT (((*nassigns) == NULL), "nassigns should point to Null");

    DBUG_PRINT ("WLPG", ("starting with-loop analysis"));

    arg_info = MakeInfo ();

    INFO_WL (arg_info) = wl;
    INFO_FUNDEF (arg_info) = fundef;
    INFO_LET (arg_info) = let;
    INFO_SHPEXT (arg_info) = SHgetUnrLen (TYgetShape (IDS_NTYPE (WITH_VEC (wl))));

    TRAVpush (TR_wla);
    wl = TRAVdo (wl, arg_info);
    TRAVpop ();

    (*gprop) = INFO_GENPROP (arg_info);
    (*nassigns) = INFO_NASSIGNS (arg_info);

    DBUG_PRINT ("WLPG", ("with-loop analysis complete"));

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (wl);
}
