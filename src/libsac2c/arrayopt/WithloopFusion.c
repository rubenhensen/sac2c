#include <stdio.h>
#include <stdlib.h>

#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "shape.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "ctinfo.h"
#include "globals.h"

#define DBUG_PREFIX "WLFS"
#include "debug.h"

#include "traverse.h"
#include "constants.h"
#include "print.h"
#include "WithloopFusion.h"
#include "detectdependencies.h"
#include "tagdependencies.h"
#include "resolvedependencies.h"
#include "math_utils.h"
#include "with_loop_utilities.h"

typedef enum { WOT_gen_mod, WOT_fold, WOT_gen_mod_fold, WOT_unknown } wo_type_t;

typedef enum { WL_fused, WL_2fuse, WL_travback, WL_nothing } wl_action_t;

typedef enum {
    GEN_equal,
    GEN_equal_var,
    GEN_constant,
    GEN_variable,
    GEN_diffdim
} gen_property_t;

typedef enum { ARRAY_aks, ARRAY_akd, ARRAY_unknown } array_types_t;

/**
 * INFO structure
 */
struct INFO {
    node *wl;
    node *fundef;
    node *assign;
    node *lhs_wl;
    int wlaction;
    int genproperty;
    wo_type_t wl_wotype;
    bool wl_lb_is_zero;
    bool wldependent;
    int wl_array_type;
    constant *wl_shape;
    node *fusionable_wl;
    nodelist *references_fusionable;
    int fwl_wotype;
    bool fwl_lb_is_zero;
    int fwl_array_type;
    constant *fwl_shape;
    node *assigns2shift;
};

/**
 * GRIDINFO structure
 */
typedef struct GRIDINFO {
    node *new_lb;
    node *new_ub;
    node *new_step;
    node *new_width;
    int max_dim;
    node *nlb;
    node *nub;
    node *nstep;
    node *nwidth;
    node *off_1;
    node *off_2;
    node *stp_1;
    node *stp_2;
    node *wth_1;
    node *wth_2;
    int dim;
    node *withid_1;
    node *withid_2;
    node *ncode_1;
    node *ncode_2;
    node *npart_1;
    node *npart_2;
    node *nparts_1;
    node *nparts_2;
} gridinfo;

/* usage of arg_info: */
#define INFO_WL(n) (n->wl)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_ASSIGN(n) (n->assign)
#define INFO_LHS_WL(n) (n->lhs_wl)
#define INFO_WLACTION(n) (n->wlaction)
#define INFO_GENPROPERTY(n) (n->genproperty)
#define INFO_WL_WOTYPE(n) (n->wl_wotype)
#define INFO_WLDEPENDENT(n) (n->wldependent)
#define INFO_WL_LB_IS_ZERO(n) (n->wl_lb_is_zero)
#define INFO_WL_ARRAY_TYPE(n) (n->wl_array_type)
#define INFO_WL_SHAPE(n) (n->wl_shape)

#define INFO_FUSIONABLE_WL(n) (n->fusionable_wl)
#define INFO_REFERENCES_FUSIONABLE(n) (n->references_fusionable)
#define INFO_FWL_WOTYPE(n) (n->fwl_wotype)
#define INFO_FWL_LB_IS_ZERO(n) (n->fwl_lb_is_zero)
#define INFO_FWL_ARRAY_TYPE(n) (n->fwl_array_type)
#define INFO_FWL_SHAPE(n) (n->fwl_shape)
#define INFO_ASSIGNS2SHIFT(n) (n->assigns2shift)

/* usage of arg_gridinfo: */
#define GRIDINFO_NEW_LB(n) (n->new_lb)
#define GRIDINFO_NEW_UB(n) (n->new_ub)
#define GRIDINFO_NEW_STEP(n) (n->new_step)
#define GRIDINFO_NEW_WIDTH(n) (n->new_width)
#define GRIDINFO_MAX_DIM(n) (n->max_dim)
#define GRIDINFO_NEW_LB_AELEMS(n) (n->nlb)
#define GRIDINFO_NEW_LB_ELEM(n) (NUM_VAL (EXPRS_EXPR (n->nlb)))
#define GRIDINFO_NEW_UB_AELEMS(n) (n->nub)
#define GRIDINFO_NEW_UB_ELEM(n) (NUM_VAL (EXPRS_EXPR (n->nub)))
#define GRIDINFO_NEW_STEP_AELEMS(n) (n->nstep)
#define GRIDINFO_NEW_STEP_ELEM(n) (NUM_VAL (EXPRS_EXPR (n->nstep)))
#define GRIDINFO_NEW_WIDTH_AELEMS(n) (n->nwidth)
#define GRIDINFO_NEW_WIDTH_ELEM(n) (NUM_VAL (EXPRS_EXPR (n->nwidth)))
#define GRIDINFO_OFFSET_1_AELEMS(n) (n->off_1)
#define GRIDINFO_OFFSET_1_ELEM(n) (NUM_VAL (EXPRS_EXPR (n->off_1)))
#define GRIDINFO_OFFSET_2_AELEMS(n) (n->off_2)
#define GRIDINFO_OFFSET_2_ELEM(n) (NUM_VAL (EXPRS_EXPR (n->off_2)))
#define GRIDINFO_STEP_1_AELEMS(n) (n->stp_1)
#define GRIDINFO_STEP_1_ELEM(n) (NUM_VAL (EXPRS_EXPR (n->stp_1)))
#define GRIDINFO_STEP_2_AELEMS(n) (n->stp_2)
#define GRIDINFO_STEP_2_ELEM(n) (NUM_VAL (EXPRS_EXPR (n->stp_2)))
#define GRIDINFO_WIDTH_1_AELEMS(n) (n->wth_1)
#define GRIDINFO_WIDTH_1_ELEM(n) (NUM_VAL (EXPRS_EXPR (n->wth_1)))
#define GRIDINFO_WIDTH_2_AELEMS(n) (n->wth_2)
#define GRIDINFO_WIDTH_2_ELEM(n) (NUM_VAL (EXPRS_EXPR (n->wth_2)))
#define GRIDINFO_CURRENT_DIM(n) (n->dim)
#define GRIDINFO_WITHID_1(n) (n->withid_1)
#define GRIDINFO_WITHID_2(n) (n->withid_2)
#define GRIDINFO_NCODE_1(n) (n->ncode_1)
#define GRIDINFO_NCODE_2(n) (n->ncode_2)
#define GRIDINFO_NPART_1(n) (n->npart_1)
#define GRIDINFO_NPART_2(n) (n->npart_2)
#define GRIDINFO_NPARTS_1(n) (n->nparts_1)
#define GRIDINFO_NPARTS_2(n) (n->nparts_2)

#define CONTAINS_FOLD(wotype) ((wotype == WOT_fold) || (wotype == WOT_gen_mod_fold))

/*
 * these macro is used in 'WLFSNgenerator' and 'FindFittingPart'
 */

#define RESULT_GEN_PROP(gen_prob, gen_prob_ret)                                          \
    if (gen_prob == GEN_equal) {                                                         \
        gen_prob = gen_prob_ret;                                                         \
    } else if (gen_prob_ret == GEN_variable) {                                           \
        gen_prob = gen_prob_ret;                                                         \
    } else if (gen_prob == GEN_constant && gen_prob_ret == GEN_equal_var) {              \
        gen_prob = GEN_variable;                                                         \
    } else if (gen_prob == GEN_equal_var && gen_prob_ret == GEN_constant) {              \
        gen_prob = GEN_variable;                                                         \
    }

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
    INFO_ASSIGN (result) = NULL;
    INFO_LHS_WL (result) = NULL;
    INFO_WLACTION (result) = WL_nothing;
    INFO_GENPROPERTY (result) = GEN_equal;
    INFO_WL_WOTYPE (result) = WOT_unknown;
    INFO_WL_LB_IS_ZERO (result) = FALSE;
    INFO_WLDEPENDENT (result) = FALSE;
    INFO_WL_ARRAY_TYPE (result) = ARRAY_unknown;
    INFO_WL_SHAPE (result) = NULL;
    INFO_FUSIONABLE_WL (result) = NULL;
    INFO_REFERENCES_FUSIONABLE (result) = NULL;
    INFO_FWL_WOTYPE (result) = WOT_unknown;
    INFO_FWL_LB_IS_ZERO (result) = FALSE;
    INFO_FWL_ARRAY_TYPE (result) = ARRAY_unknown;
    INFO_FWL_SHAPE (result) = NULL;
    INFO_ASSIGNS2SHIFT (result) = NULL;

    DBUG_RETURN (result);
}

static info *
InitInfo (info *arg_info)
{
    DBUG_ENTER ();

    INFO_WL_LB_IS_ZERO (arg_info) = FALSE;
    INFO_WL_ARRAY_TYPE (arg_info) = ARRAY_unknown;
    if (INFO_WL_SHAPE (arg_info) != NULL) {
        INFO_WL_SHAPE (arg_info) = COfreeConstant (INFO_WL_SHAPE (arg_info));
    }
    INFO_WL_WOTYPE (arg_info) = WOT_unknown;
    INFO_GENPROPERTY (arg_info) = GEN_equal;

    DBUG_RETURN (arg_info);
}

static info *
UpdateInfo (info *arg_info, info *stacked_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = INFO_FUNDEF (stacked_info);

    INFO_FUSIONABLE_WL (arg_info) = INFO_ASSIGN (stacked_info);
    INFO_REFERENCES_FUSIONABLE (arg_info)
      = TCnodeListAppend (INFO_REFERENCES_FUSIONABLE (arg_info),
                          INFO_ASSIGN (stacked_info), NULL);

    switch (INFO_FWL_WOTYPE (arg_info)) {
    case WOT_unknown:
        INFO_FWL_WOTYPE (arg_info) = INFO_WL_WOTYPE (stacked_info);
        break;

    case WOT_gen_mod:
        if (CONTAINS_FOLD (INFO_WL_WOTYPE (stacked_info)))
            INFO_FWL_WOTYPE (arg_info) = WOT_gen_mod_fold;
        break;

    case WOT_fold:
        if ((INFO_WL_WOTYPE (stacked_info) == WOT_gen_mod)
            || (INFO_WL_WOTYPE (stacked_info) == WOT_gen_mod_fold))
            INFO_FWL_WOTYPE (arg_info) = WOT_gen_mod_fold;
        break;

    case WOT_gen_mod_fold:
        /* here is nothing to do */
        break;

    default:
        DBUG_UNREACHABLE ("illegal WOTYPE found!");
        break;
    }

    INFO_FWL_LB_IS_ZERO (arg_info) = INFO_WL_LB_IS_ZERO (stacked_info);
    INFO_FWL_ARRAY_TYPE (arg_info) = INFO_WL_ARRAY_TYPE (stacked_info);
    INFO_FWL_SHAPE (arg_info) = INFO_WL_SHAPE (stacked_info);
    INFO_WL_SHAPE (stacked_info) = NULL;

    DBUG_RETURN (arg_info);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    if (INFO_REFERENCES_FUSIONABLE (info) != NULL) {
        INFO_REFERENCES_FUSIONABLE (info)
          = TCnodeListFree (INFO_REFERENCES_FUSIONABLE (info), TRUE);
    }
    if (INFO_FWL_SHAPE (info) != NULL) {
        INFO_FWL_SHAPE (info) = COfreeConstant (INFO_FWL_SHAPE (info));
    }

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * GRIDINFO functions
 */
static gridinfo *
MakeGridInfo (void)
{
    gridinfo *result;

    DBUG_ENTER ();

    result = (gridinfo *)MEMmalloc (sizeof (gridinfo));

    GRIDINFO_NEW_LB (result) = NULL;
    GRIDINFO_NEW_UB (result) = NULL;
    GRIDINFO_NEW_STEP (result) = NULL;
    GRIDINFO_NEW_WIDTH (result) = NULL;
    GRIDINFO_MAX_DIM (result) = 0;
    GRIDINFO_NEW_LB_AELEMS (result) = NULL;
    GRIDINFO_NEW_UB_AELEMS (result) = NULL;
    GRIDINFO_NEW_STEP_AELEMS (result) = NULL;
    GRIDINFO_NEW_WIDTH_AELEMS (result) = NULL;
    GRIDINFO_OFFSET_1_AELEMS (result) = NULL;
    GRIDINFO_OFFSET_2_AELEMS (result) = NULL;
    GRIDINFO_STEP_1_AELEMS (result) = NULL;
    GRIDINFO_STEP_2_AELEMS (result) = NULL;
    GRIDINFO_WIDTH_1_AELEMS (result) = NULL;
    GRIDINFO_WIDTH_2_AELEMS (result) = NULL;
    GRIDINFO_CURRENT_DIM (result) = 0;
    GRIDINFO_WITHID_1 (result) = NULL;
    GRIDINFO_WITHID_2 (result) = NULL;
    GRIDINFO_NCODE_1 (result) = NULL;
    GRIDINFO_NCODE_2 (result) = NULL;
    GRIDINFO_NPART_1 (result) = NULL;
    GRIDINFO_NPART_2 (result) = NULL;
    GRIDINFO_NPARTS_1 (result) = NULL;
    GRIDINFO_NPARTS_2 (result) = NULL;

    DBUG_RETURN (result);
}

static gridinfo *
FillGridInfo (gridinfo *arg_gridinfo, node *new_array_lb, node *new_array_ub,
              node *new_step, node *new_width, node *offset_1, node *offset_2,
              int max_dim, node *withid_1, node *withid_2, node *ncode_1, node *ncode_2)
{
    DBUG_ENTER ();

    GRIDINFO_NEW_LB (arg_gridinfo) = new_array_lb;
    GRIDINFO_NEW_UB (arg_gridinfo) = new_array_ub;
    GRIDINFO_NEW_LB_AELEMS (arg_gridinfo) = ARRAY_AELEMS (new_array_lb);
    GRIDINFO_NEW_UB_AELEMS (arg_gridinfo) = ARRAY_AELEMS (new_array_ub);
    GRIDINFO_NEW_STEP (arg_gridinfo) = new_step;
    GRIDINFO_NEW_STEP_AELEMS (arg_gridinfo) = ARRAY_AELEMS (new_step);
    GRIDINFO_NEW_WIDTH (arg_gridinfo) = new_width;
    GRIDINFO_NEW_WIDTH_AELEMS (arg_gridinfo) = ARRAY_AELEMS (new_width);
    GRIDINFO_OFFSET_1_AELEMS (arg_gridinfo) = ARRAY_AELEMS (offset_1);
    GRIDINFO_OFFSET_2_AELEMS (arg_gridinfo) = ARRAY_AELEMS (offset_2);
    GRIDINFO_MAX_DIM (arg_gridinfo) = max_dim;
    GRIDINFO_WITHID_1 (arg_gridinfo) = withid_1;
    GRIDINFO_WITHID_2 (arg_gridinfo) = withid_2;
    GRIDINFO_NCODE_1 (arg_gridinfo) = ncode_1;
    GRIDINFO_NCODE_2 (arg_gridinfo) = ncode_2;

    DBUG_RETURN (arg_gridinfo);
}

static gridinfo *
DupGridInfo (gridinfo *info)
{
    gridinfo *result;

    DBUG_ENTER ();

    result = MakeGridInfo ();

    GRIDINFO_NEW_LB (result) = GRIDINFO_NEW_LB (info);
    GRIDINFO_NEW_UB (result) = GRIDINFO_NEW_UB (info);
    GRIDINFO_NEW_STEP (result) = GRIDINFO_NEW_STEP (info);
    GRIDINFO_NEW_WIDTH (result) = GRIDINFO_NEW_WIDTH (info);
    GRIDINFO_MAX_DIM (result) = GRIDINFO_MAX_DIM (info);
    GRIDINFO_NEW_LB_AELEMS (result) = GRIDINFO_NEW_LB_AELEMS (info);
    GRIDINFO_NEW_UB_AELEMS (result) = GRIDINFO_NEW_UB_AELEMS (info);
    GRIDINFO_NEW_STEP_AELEMS (result) = GRIDINFO_NEW_STEP_AELEMS (info);
    GRIDINFO_NEW_WIDTH_AELEMS (result) = GRIDINFO_NEW_WIDTH_AELEMS (info);
    GRIDINFO_OFFSET_1_AELEMS (result) = GRIDINFO_OFFSET_1_AELEMS (info);
    GRIDINFO_OFFSET_2_AELEMS (result) = GRIDINFO_OFFSET_2_AELEMS (info);
    GRIDINFO_STEP_1_AELEMS (result) = GRIDINFO_STEP_1_AELEMS (info);
    GRIDINFO_STEP_2_AELEMS (result) = GRIDINFO_STEP_2_AELEMS (info);
    GRIDINFO_WIDTH_1_AELEMS (result) = GRIDINFO_WIDTH_1_AELEMS (info);
    GRIDINFO_WIDTH_2_AELEMS (result) = GRIDINFO_WIDTH_2_AELEMS (info);
    GRIDINFO_CURRENT_DIM (result) = GRIDINFO_CURRENT_DIM (info);
    GRIDINFO_WITHID_1 (result) = GRIDINFO_WITHID_1 (info);
    GRIDINFO_WITHID_2 (result) = GRIDINFO_WITHID_2 (info);
    GRIDINFO_NCODE_1 (result) = GRIDINFO_NCODE_1 (info);
    GRIDINFO_NCODE_2 (result) = GRIDINFO_NCODE_2 (info);
    GRIDINFO_NPART_1 (result) = GRIDINFO_NPART_1 (info);
    GRIDINFO_NPART_2 (result) = GRIDINFO_NPART_2 (info);
    GRIDINFO_NPARTS_1 (result) = GRIDINFO_NPARTS_1 (info);
    GRIDINFO_NPARTS_2 (result) = GRIDINFO_NPARTS_2 (info);

    DBUG_RETURN (result);
}

static gridinfo *
GridInfoStep (gridinfo *gridinfo)
{
    DBUG_ENTER ();

    GRIDINFO_NEW_LB_AELEMS (gridinfo) = EXPRS_NEXT (GRIDINFO_NEW_LB_AELEMS (gridinfo));
    GRIDINFO_NEW_UB_AELEMS (gridinfo) = EXPRS_NEXT (GRIDINFO_NEW_UB_AELEMS (gridinfo));
    GRIDINFO_NEW_STEP_AELEMS (gridinfo)
      = EXPRS_NEXT (GRIDINFO_NEW_STEP_AELEMS (gridinfo));
    GRIDINFO_NEW_WIDTH_AELEMS (gridinfo)
      = EXPRS_NEXT (GRIDINFO_NEW_WIDTH_AELEMS (gridinfo));
    GRIDINFO_OFFSET_1_AELEMS (gridinfo)
      = EXPRS_NEXT (GRIDINFO_OFFSET_1_AELEMS (gridinfo));
    GRIDINFO_OFFSET_2_AELEMS (gridinfo)
      = EXPRS_NEXT (GRIDINFO_OFFSET_2_AELEMS (gridinfo));
    GRIDINFO_STEP_1_AELEMS (gridinfo) = EXPRS_NEXT (GRIDINFO_STEP_1_AELEMS (gridinfo));
    GRIDINFO_STEP_2_AELEMS (gridinfo) = EXPRS_NEXT (GRIDINFO_STEP_2_AELEMS (gridinfo));
    GRIDINFO_WIDTH_1_AELEMS (gridinfo) = EXPRS_NEXT (GRIDINFO_WIDTH_1_AELEMS (gridinfo));
    GRIDINFO_WIDTH_2_AELEMS (gridinfo) = EXPRS_NEXT (GRIDINFO_WIDTH_2_AELEMS (gridinfo));
    GRIDINFO_CURRENT_DIM (gridinfo)++;

    DBUG_RETURN (gridinfo);
}

static gridinfo *
GridInfoUpdate (gridinfo *arg_gridinfo, gridinfo *new_gridinfo)
{
    DBUG_ENTER ();

    if (GRIDINFO_NPARTS_1 (arg_gridinfo) == NULL) {
        GRIDINFO_NPARTS_1 (arg_gridinfo) = GRIDINFO_NPARTS_1 (new_gridinfo);
        GRIDINFO_NPARTS_2 (arg_gridinfo) = GRIDINFO_NPARTS_2 (new_gridinfo);

        GRIDINFO_NPART_1 (arg_gridinfo) = GRIDINFO_NPART_1 (new_gridinfo);
        GRIDINFO_NPART_2 (arg_gridinfo) = GRIDINFO_NPART_2 (new_gridinfo);
    } else {
        GRIDINFO_NPART_1 (arg_gridinfo) = GRIDINFO_NPART_1 (new_gridinfo);
        GRIDINFO_NPART_2 (arg_gridinfo) = GRIDINFO_NPART_2 (new_gridinfo);
    }

    DBUG_RETURN (arg_gridinfo);
}

static gridinfo *
FreeGridInfo (gridinfo *gridinfo)
{
    DBUG_ENTER ();

    gridinfo = MEMfree (gridinfo);

    DBUG_RETURN (gridinfo);
}

/** <!--********************************************************************-->
 *
 * @fn bool CheckDependency( node *checkid, nodelist *nl);
 *
 *   @brief checks whether checkid is contained in LHS of the assignments
 *          stored in nl.
 *
 *   @param  node *checkid :  N_id
 *           nodelist *nl  :  contains assignments which depends (indirect)
 *                            on fusionable withloop
 *   @return bool          :  returns TRUE iff checkid is dependent
 ******************************************************************************/
static bool
CheckDependency (node *checkid, nodelist *nl)
{
    nodelist *nl_tmp;
    bool is_dependent = FALSE;

    DBUG_ENTER ();

    nl_tmp = nl;

    while (nl_tmp != NULL) {
        if (NODELIST_NODE (nl_tmp) == AVIS_SSAASSIGN (ID_AVIS (checkid))) {
            is_dependent = TRUE;
            break;
        }
        nl_tmp = NODELIST_NEXT (nl_tmp);
    }

    DBUG_RETURN (is_dependent);
}

/** <!--********************************************************************-->
 *
 * @fn bool CheckIterationSpace( info *arg_info)
 *
 *   @brief checks whether the size of both iteration spaces are equal.
 *
 *   @param  info *  :  N_info
 *   @return bool    :  returns TRUE iff they have equal size
 ******************************************************************************/
static bool
CheckIterationSpace (info *arg_info)
{
    bool is_equal;
    constant *tmpc, *shape1, *shape2;

    DBUG_ENTER ();

    DBUG_ASSERT ((INFO_WL_ARRAY_TYPE (arg_info) == ARRAY_aks
                  && INFO_FWL_ARRAY_TYPE (arg_info) == ARRAY_aks),
                 "Both ARRAY_TYPES had to be aks!");

    DBUG_ASSERT ((INFO_WL_SHAPE (arg_info) != NULL && INFO_FWL_SHAPE (arg_info) != NULL),
                 "Both SHAPEs had to be non empty!");

    shape1 = INFO_WL_SHAPE (arg_info);
    shape2 = INFO_FWL_SHAPE (arg_info);

    if (SHgetUnrLen (COgetShape (shape1)) == SHgetUnrLen (COgetShape (shape2))) {
        tmpc = COeq (shape1, shape2, NULL);
        if (COisTrue (tmpc, TRUE))
            is_equal = TRUE;
        else
            is_equal = FALSE;
        tmpc = COfreeConstant (tmpc);
    } else
        is_equal = FALSE;

    DBUG_RETURN (is_equal);
}

/** <!--********************************************************************-->
 *
 * @fn bool CompGenSon( node *gen_son1, node *gen_son2)
 *
 *   @brief compares the two nodes.
 *
 *   @param  node *gen_son1 :  N_array
 *           node *gen_son2 :  N_array
 *   @return gen_property_t :
 *              GEN_equal          : both nodes are equal and constant!
 *              GEN_constant       : both nodes are not equal but constant!
 *              GEN_equal_variable : both nodes are equal but have variable
 *                                   elements!
 *              GEN_variable       : both nodes have variable elements and
 *                                   aren't equal
 ******************************************************************************/
static gen_property_t
CompGenSon (node *gen_son1, node *gen_son2)
{
    node *elems1, *elems2, *elems;
    gen_property_t gen_prob = GEN_equal;

    DBUG_ENTER ();

    if ((gen_son1 == NULL) && (gen_son2 == NULL))
        gen_prob = GEN_equal;
    else if ((gen_son1 == NULL) || (gen_son2 == NULL)) {

        if (gen_son1)
            elems = ARRAY_AELEMS (gen_son1);
        else
            elems = ARRAY_AELEMS (gen_son2);

        while (elems) {
            if (NODE_TYPE (EXPRS_EXPR (elems)) == N_num) {
                if (gen_prob == GEN_equal)
                    gen_prob = GEN_constant;
            } else if (NODE_TYPE (EXPRS_EXPR (elems)) == N_id) {
                gen_prob = GEN_variable;
                break;
            } else {
                DBUG_UNREACHABLE ("Unknown elements found!");
                break;
            }
            elems = EXPRS_NEXT (elems);
        }
    } else {
        DBUG_ASSERT (((NODE_TYPE (gen_son1) == N_array)
                      && (NODE_TYPE (gen_son2) == N_array)),
                     "CompGenSon not called with N_arrays");
        elems1 = ARRAY_AELEMS (gen_son1);
        elems2 = ARRAY_AELEMS (gen_son2);

        while (elems1 && elems2) {
            if ((NODE_TYPE (EXPRS_EXPR (elems1)) == N_num)
                && (NODE_TYPE (EXPRS_EXPR (elems2)) == N_num)) {
                if ((NUM_VAL (EXPRS_EXPR (elems1)) != NUM_VAL (EXPRS_EXPR (elems2))))
                    if (gen_prob == GEN_equal)
                        gen_prob = GEN_constant;
            } else if ((NODE_TYPE (EXPRS_EXPR (elems1)) == N_id)
                       && (NODE_TYPE (EXPRS_EXPR (elems2)) == N_id)) {
                if (STReq (ID_NAME (EXPRS_EXPR (elems1)),
                           ID_NAME (EXPRS_EXPR (elems2)))) {
                    if (gen_prob == GEN_equal)
                        gen_prob = GEN_equal_var;
                    else if (gen_prob == GEN_constant) {
                        gen_prob = GEN_variable;
                    }
                } else {
                    gen_prob = GEN_variable;
                }
            } else {
                gen_prob = GEN_variable;
            }

            elems1 = EXPRS_NEXT (elems1);
            elems2 = EXPRS_NEXT (elems2);
        }

        if (elems1 != NULL || elems2 != NULL)
            gen_prob = GEN_diffdim;
    }

    DBUG_RETURN (gen_prob);
}

/** <!--********************************************************************-->
 *
 * @fn node *FuseAccu(node *assigns1, node *assigns2)
 *
 *   @brief If both assign chains contain an accu function fuse them in first
 *          assign chain together and remove accu assign form second.
 *
 *   @param  node *assigns1 :  N_assign chain
 *           node *assigns2 :  N_assign chain
 *   @return node *         :  modified assigns2
 ******************************************************************************/
static node *
FuseAccu (node *assigns1, node *assigns2)
{
    node *first_accu = NULL, *second_accu = NULL, *tmp, *predecessor, *tmp_ids;

    DBUG_ENTER ();

    DBUG_ASSERT ((assigns1 != NULL && assigns2 != NULL),
                 "FuseAccu called with empty assigns");
    DBUG_ASSERT (((NODE_TYPE (assigns1) == N_assign)
                  && (NODE_TYPE (assigns2) == N_assign)),
                 "FuseAccu not called with N_assigns");

    tmp = assigns1;
    while (tmp != NULL) {
        if ((NODE_TYPE (ASSIGN_RHS (tmp)) == N_prf)
            && (PRF_PRF (ASSIGN_RHS (tmp)) == F_accu)) {
            DBUG_ASSERT (first_accu == NULL,
                         "more than one F_accu in first assign chain!");
            first_accu = tmp;
        }
        tmp = ASSIGN_NEXT (tmp);
    }

    tmp = assigns2;
    predecessor = NULL;
    while (tmp != NULL) {
        if ((NODE_TYPE (ASSIGN_RHS (tmp)) == N_prf)
            && (PRF_PRF (ASSIGN_RHS (tmp)) == F_accu)) {

            DBUG_ASSERT (second_accu == NULL,
                         "more than one F_accu in second assign chain!");
            second_accu = tmp;
        }
        if (second_accu == NULL)
            predecessor = tmp;
        tmp = ASSIGN_NEXT (tmp);
    }

    if (first_accu != NULL && second_accu != NULL) {
        /*
         *  extend LHS of first F_accu by LHS of second
         *
         * Important: the order of identifiers is critical, because
         * they correspond with the fold operators of the withloop.
         *
         */
        tmp_ids = DUPdoDupTree (ASSIGN_LHS (second_accu));
        ASSIGN_LHS (first_accu) = TCappendIds (ASSIGN_LHS (first_accu), tmp_ids);
        while (tmp_ids != NULL) {
            /* set correct backref to defining assignment */
            AVIS_SSAASSIGN (IDS_AVIS (tmp_ids)) = first_accu;
            tmp_ids = IDS_NEXT (tmp_ids);
        }

        if (predecessor == NULL)
            assigns2 = FREEdoFreeNode (second_accu);
        else
            ASSIGN_NEXT (predecessor) = FREEdoFreeNode (second_accu);
    }

    DBUG_RETURN (assigns2);
}

/** <!--********************************************************************-->
 *
 * @fn node *FindFittingPart(node *pattern, node *parts)
 *
 *   @brief finds the N_Npart out of 'parts' which N_Ngenerator is equal
 *          to pattern.
 *
 *   @param  node *pattern  :  N_generator
 *           node *parts    :  N_part list
 *   @return node *         :  N_part with equal generator to pattern
 *                             else NULL
 ******************************************************************************/
static node *
FindFittingPart (node *pattern, node *parts)
{
    node *gen, *tmp_part = NULL;
    gen_property_t gen_prob, gen_prob_ret;

    DBUG_ENTER ();

    while (parts != NULL) {
        gen = PART_GENERATOR (parts);
        gen_prob = CompGenSon (GENERATOR_BOUND1 (pattern), GENERATOR_BOUND1 (gen));
        gen_prob_ret = CompGenSon (GENERATOR_BOUND2 (pattern), GENERATOR_BOUND2 (gen));
        RESULT_GEN_PROP (gen_prob, gen_prob_ret);
        gen_prob_ret = CompGenSon (GENERATOR_STEP (pattern), GENERATOR_STEP (gen));
        RESULT_GEN_PROP (gen_prob, gen_prob_ret);
        gen_prob_ret = CompGenSon (GENERATOR_WIDTH (pattern), GENERATOR_WIDTH (gen));
        RESULT_GEN_PROP (gen_prob, gen_prob_ret);
        if (gen_prob == GEN_equal || gen_prob == GEN_equal_var) {
            tmp_part = parts;
            break;
        }
        parts = PART_NEXT (parts);
    }

    DBUG_RETURN (tmp_part);
}

/** <!--********************************************************************-->
 *
 * @fn node *FuseCodes( node *extendable_part, node *fitting_part,
 *                       node *fusionable_assign, bool both_contain_fold,
 *                       node *fundef)
 *
 *   @brief inserts the whole assignment block of 'fitting_part' in
 *          assignment block of 'extendable_part'.
 *
 *   @param  node *extendable_part : N_part
 *           node *fitting_part    : N_part
 *           node *fusionable_wl   : N_assign of WL current WL will be fused with
 *           node *fundef          : N_fundef
 *   @return node                  : modified extendable_part
 ******************************************************************************/
static node *
FuseCodes (node *extendable_part, node *fitting_part, node *fusionable_assign,
           bool both_contain_fold, node *fundef)
{
    node *fusionable_wl, *fusionable_ncode, *fitting_nncode, *fitting_assigns,
      *extendable_block, *fitting_exprs;
    lut_t *lut;
    node *oldvec, *newvec, *oldids, *newids;
    bool resolveable_dependencies;

    DBUG_ENTER ();

    fusionable_wl = ASSIGN_RHS (fusionable_assign);
    /*
     * If the N_Ncode of 'extendable_part' is referenced several
     * it has to be deepcopied before extend.
     */
    if (CODE_USED (PART_CODE (extendable_part)) != 1) {
        fusionable_ncode = WITH_CODE (fusionable_wl);

        if (fusionable_ncode == PART_CODE (extendable_part)) {
            CODE_USED (fusionable_ncode)--;
            PART_CODE (extendable_part)
              = DUPdoDupNodeSsa (PART_CODE (extendable_part), fundef);
            CODE_USED (PART_CODE (extendable_part)) = 1;
            WITH_CODE (fusionable_wl) = PART_CODE (extendable_part);
            CODE_NEXT (PART_CODE (extendable_part)) = fusionable_ncode;
        } else {
            while (CODE_NEXT (fusionable_ncode) != PART_CODE (extendable_part)) {
                fusionable_ncode = CODE_NEXT (fusionable_ncode);
            }
            CODE_USED (CODE_NEXT (fusionable_ncode))--;
            PART_CODE (extendable_part)
              = DUPdoDupNodeSsa (PART_CODE (extendable_part), fundef);
            CODE_USED (PART_CODE (extendable_part)) = 1;
            CODE_NEXT (PART_CODE (extendable_part)) = CODE_NEXT (fusionable_ncode);
            CODE_NEXT (fusionable_ncode) = PART_CODE (extendable_part);
        }
    }

    /*
     * Rename occurrences of NWITHID_VEC and NWITHID_IDS in N_NCODE of fitting_part
     * by NWITHID_VEC and NWITHID_IDS of fusionable WL
     */

    /* Create a new Lut */
    lut = LUTgenerateLut ();

    oldvec = WITHID_VEC (PART_WITHID (fitting_part));
    newvec = WITHID_VEC (PART_WITHID (extendable_part));

    LUTinsertIntoLutP (lut, IDS_AVIS (oldvec), IDS_AVIS (newvec));

    oldids = WITHID_IDS (PART_WITHID (fitting_part));
    newids = WITHID_IDS (PART_WITHID (extendable_part));

    while (oldids != NULL) {
        LUTinsertIntoLutP (lut, IDS_AVIS (oldids), IDS_AVIS (newids));

        oldids = IDS_NEXT (oldids);
        newids = IDS_NEXT (newids);
    }

    fitting_nncode = DUPdoDupNodeLutSsa (PART_CODE (fitting_part), lut, fundef);

    resolveable_dependencies = CODE_HASRESOLVEABLEDEPENDENCIES (PART_CODE (fitting_part));

    fitting_assigns = BLOCK_ASSIGNS (CODE_CBLOCK (fitting_nncode));
    BLOCK_ASSIGNS (CODE_CBLOCK (fitting_nncode)) = NULL;

    /*
     * vardec of N_blocks which are not the first N_block of
     * the current N_fundef are empty and points to NULL.
     * This holds especially for N_blocks of Withloops.
     */

    fitting_exprs = CODE_CEXPRS (fitting_nncode);
    CODE_CEXPRS (fitting_nncode) = TBmakeExprs (NULL, NULL);

    fitting_nncode = FREEdoFreeNode (fitting_nncode);

    LUTremoveLut (lut);

    /*
     * extent N_block of extentable_part's N_Ncode by instructions
     * of N_block of fitting_part
     */
    extendable_block = CODE_CBLOCK (PART_CODE (extendable_part));

    if (fitting_assigns != NULL) {

        if (BLOCK_ASSIGNS (extendable_block) != NULL && both_contain_fold) {
            /*
             *  If both withloops contain fold operators fuse both
             *  accu functions if containing together
             */
            fitting_assigns
              = FuseAccu (BLOCK_ASSIGNS (extendable_block), fitting_assigns);
        }

        if (fitting_assigns != NULL && resolveable_dependencies) {
            /*
             *  Ncode of current WL contains resolveable dependencies -> resolve them
             */
            fitting_assigns
              = RDEPENDdoResolveDependencies (fitting_assigns,
                                              CODE_CEXPRS (PART_CODE (extendable_part)),
                                              PART_WITHID (extendable_part),
                                              fusionable_assign);
        }

        BLOCK_ASSIGNS (extendable_block)
          = TCappendAssign (BLOCK_ASSIGNS (extendable_block), fitting_assigns);
    }

    /*
     * extent N_exprs of extendable_part's N_Ncode by N_exprs of fitting_part
     */
    CODE_CEXPRS (PART_CODE (extendable_part))
      = TCappendExprs (CODE_CEXPRS (PART_CODE (extendable_part)), fitting_exprs);

    DBUG_RETURN (extendable_part);
}

/** <!--********************************************************************-->
 *
 * @fn node *FuseWithloops( node *wl, info *arg_info, node *fusionable_assign)
 *
 *   @brief Fuses the two withloops together.
 *
 *   @param  node *wl                :  current N_with node
 *           info *arg_info          :  N_INFO
 *           node *fusionable_assign :  N_assign node of the second withloop where the
 *                                      current withloop will be fused to
 *   @return node *                  :  current N_with node
 ******************************************************************************/
static node *
FuseWithloops (node *wl, info *arg_info, node *fusionable_assign)
{
    node *fusionable_wl, *tmp_withop, *parts, *fitting_part;
    node *tmp_ids;
    bool both_contain_fold;

    DBUG_ENTER ();

    /* N_Nwith where current withloop will be fused with */
    fusionable_wl = ASSIGN_RHS (fusionable_assign);

    /*
     * 1. extend LHS of fusionable_assign by LHS of WL assignment
     */
    tmp_ids = DUPdoDupTree (ASSIGN_LHS (INFO_ASSIGN (arg_info)));
    ASSIGN_LHS (fusionable_assign)
      = TCappendIds (ASSIGN_LHS (fusionable_assign), tmp_ids);
    while (tmp_ids != NULL) {
        /* set correct backref to defining assignment */
        AVIS_SSAASSIGN (IDS_AVIS (tmp_ids)) = fusionable_assign;
        tmp_ids = IDS_NEXT (tmp_ids);
    }

    /*
     * 2. extend each N_Npart's N_Ncode of the withloop belonging to
     *    'fusionable_assign' by whole assignment block of N_Npart's N_Ncode
     *    of 'wl' if both N_Ngenerators are equal.
     */

    both_contain_fold = ((CONTAINS_FOLD (INFO_WL_WOTYPE (arg_info)))
                         && (CONTAINS_FOLD (INFO_FWL_WOTYPE (arg_info))));

    parts = WITH_PART (fusionable_wl);
    while (parts != NULL) {
        fitting_part = FindFittingPart (PART_GENERATOR (parts), WITH_PART (wl));
        DBUG_ASSERT (fitting_part != NULL, "no fitting N_Npart is available!");
        parts = FuseCodes (parts, fitting_part, fusionable_assign, both_contain_fold,
                           INFO_FUNDEF (arg_info));
        parts = PART_NEXT (parts);
    }

    /*
     * 3. extent N_Nwithop(s) of fusionable_wl by N_Nwithop of 'wl'
     */
    tmp_withop = WITH_WITHOP (fusionable_wl);
    while (WITHOP_NEXT (tmp_withop) != NULL) {
        tmp_withop = WITHOP_NEXT (tmp_withop);
    }
    L_WITHOP_NEXT (tmp_withop, DUPdoDupTree (WITH_WITHOP (wl)));

    DBUG_RETURN (wl);
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

    DBUG_ENTER ();

    DBUG_ASSERT (number > 0, "dim is <= 0");

    tmp = NULL;
    for (i = 0; i < number; i++) {
        tmp = TBmakeExprs (TBmakeNum (entry), tmp);
    }
    tmp = TCmakeIntVector (tmp);

    DBUG_RETURN (tmp);
}

/** <!--********************************************************************-->
 *
 * @fn bool Match( int position, gridinfo *arg_gridinfo)
 *
 *   @brief verifies if current position in the new grid
 *          is covered by both old grids
 *
 *   @param  int position           : indicates the current position
 *                                    in new grid
 *           gridinfo *arg_gridinfo : gridinfo structure
 *   @return bool                   : TRUE iff both old grids covers current
 *                                    position of new grid
 ******************************************************************************/
static bool
Match (int position, gridinfo *arg_gridinfo)
{
    bool matches = FALSE;

    DBUG_ENTER ();

    if ((((position + GRIDINFO_OFFSET_1_ELEM (arg_gridinfo))
          % GRIDINFO_STEP_1_ELEM (arg_gridinfo))
         < GRIDINFO_WIDTH_1_ELEM (arg_gridinfo))
        && (((position + GRIDINFO_OFFSET_2_ELEM (arg_gridinfo))
             % GRIDINFO_STEP_2_ELEM (arg_gridinfo))
            < GRIDINFO_WIDTH_2_ELEM (arg_gridinfo))) {
        matches = TRUE;
    }

    DBUG_RETURN (matches);
}

/** <!--********************************************************************-->
 *
 * @fn gridinfo *IntersectGrids( gridinfo *arg_gridinfo)
 *
 *   @brief intersect two grids recursivly
 *
 *   @param  gridinfo *arg:gridinfo : gridinfo structure
 *   @return gridinfo *             : modified gridinfo structure
 ******************************************************************************/
static gridinfo *
IntersectGrids (gridinfo *arg_gridinfo)
{
    node *genn;
    gridinfo *new_gridinfo;
    int position, first, last;

    DBUG_ENTER ();

    position = first = last = 0;
    while (position < GRIDINFO_NEW_STEP_ELEM (arg_gridinfo)) {

        if (Match (position, arg_gridinfo)) {
            first = position;
            do {
                position++;
            } while (Match (position, arg_gridinfo)
                     && (position < GRIDINFO_NEW_STEP_ELEM (arg_gridinfo)));
            last = position;

            GRIDINFO_NEW_WIDTH_ELEM (arg_gridinfo) = last - first;

            if (GRIDINFO_CURRENT_DIM (arg_gridinfo)
                < (GRIDINFO_MAX_DIM (arg_gridinfo) - 1)) {
                /* compute inner dimensions if lower bound is less then upper bound */
                if ((GRIDINFO_NEW_LB_ELEM (arg_gridinfo) + first)
                    < GRIDINFO_NEW_UB_ELEM (arg_gridinfo)) {
                    GRIDINFO_NEW_LB_ELEM (arg_gridinfo) += first;

                    new_gridinfo = DupGridInfo (arg_gridinfo);
                    new_gridinfo = GridInfoStep (new_gridinfo);

                    new_gridinfo = IntersectGrids (new_gridinfo);

                    arg_gridinfo = GridInfoUpdate (arg_gridinfo, new_gridinfo);
                    new_gridinfo = FreeGridInfo (new_gridinfo);

                    GRIDINFO_NEW_LB_ELEM (arg_gridinfo) -= first;
                } else {
                    /* stop further search */
                    position = GRIDINFO_NEW_STEP_ELEM (arg_gridinfo);
                }
            } else {
                /*
                 * create new N_Parts and add it to structure
                 * if lower bound is less then upper bound
                 */
                if ((GRIDINFO_NEW_LB_ELEM (arg_gridinfo) + first)
                    < GRIDINFO_NEW_UB_ELEM (arg_gridinfo)) {
                    GRIDINFO_NEW_LB_ELEM (arg_gridinfo) += first;
                    genn
                      = TBmakeGenerator (F_wl_le, F_wl_lt,
                                         DUPdoDupNode (GRIDINFO_NEW_LB (arg_gridinfo)),
                                         DUPdoDupNode (GRIDINFO_NEW_UB (arg_gridinfo)),
                                         DUPdoDupNode (GRIDINFO_NEW_STEP (arg_gridinfo)),
                                         DUPdoDupNode (
                                           GRIDINFO_NEW_WIDTH (arg_gridinfo)));
                    GRIDINFO_NEW_LB_ELEM (arg_gridinfo) -= first;

                    if (GRIDINFO_NPARTS_1 (arg_gridinfo) != NULL) {

                        PART_NEXT (GRIDINFO_NPART_1 (arg_gridinfo))
                          = TBmakePart (GRIDINFO_NCODE_1 (arg_gridinfo),
                                        DUPdoDupNode (GRIDINFO_WITHID_1 (arg_gridinfo)),
                                        genn);
                        CODE_INC_USED (GRIDINFO_NCODE_1 (arg_gridinfo));

                        GRIDINFO_NPART_1 (arg_gridinfo)
                          = PART_NEXT (GRIDINFO_NPART_1 (arg_gridinfo));

                        PART_NEXT (GRIDINFO_NPART_2 (arg_gridinfo))
                          = TBmakePart (GRIDINFO_NCODE_2 (arg_gridinfo),
                                        DUPdoDupNode (GRIDINFO_WITHID_2 (arg_gridinfo)),
                                        DUPdoDupNode (genn));
                        CODE_INC_USED (GRIDINFO_NCODE_2 (arg_gridinfo));

                        GRIDINFO_NPART_2 (arg_gridinfo)
                          = PART_NEXT (GRIDINFO_NPART_2 (arg_gridinfo));
                    } else {
                        GRIDINFO_NPART_1 (arg_gridinfo)
                          = TBmakePart (GRIDINFO_NCODE_1 (arg_gridinfo),
                                        DUPdoDupNode (GRIDINFO_WITHID_1 (arg_gridinfo)),
                                        genn);
                        CODE_INC_USED (GRIDINFO_NCODE_1 (arg_gridinfo));

                        GRIDINFO_NPART_2 (arg_gridinfo)
                          = TBmakePart (GRIDINFO_NCODE_2 (arg_gridinfo),
                                        DUPdoDupNode (GRIDINFO_WITHID_2 (arg_gridinfo)),
                                        DUPdoDupNode (genn));
                        CODE_INC_USED (GRIDINFO_NCODE_2 (arg_gridinfo));

                        GRIDINFO_NPARTS_1 (arg_gridinfo)
                          = GRIDINFO_NPART_1 (arg_gridinfo);
                        GRIDINFO_NPARTS_2 (arg_gridinfo)
                          = GRIDINFO_NPART_2 (arg_gridinfo);
                    }
                } else {
                    /* stop further search */
                    position = GRIDINFO_NEW_STEP_ELEM (arg_gridinfo);
                }
            }
        }
        position++;
    }

    DBUG_RETURN (arg_gridinfo);
}

/** <!--********************************************************************-->
 *
 * @fn node *IntersectParts(node *parts_1, node parts_2, node **new_parts_2)
 *
 *   @brief intersect the all generators from N_Parts of the current WL
 *          with the all generators from N_Parts of the fusionable WL and
 *          creates new N_Parts for both WLs.
 *
 *   @param  node *parts_1       :  N_Part chain
 *           node *parts_2       :  N_Part chain
 *           node **new_parts_2 :  new N_Parts for WL with parts2
 *   @return node *             :  new N_Parts for WL with parts1
 ******************************************************************************/
static node *
IntersectParts (node *parts_1, node *parts_2, node **new_parts_2)
{
    node *nparts_1, *nparts_2, *npart_1, *npart_2, *parts_2_tmp, *genn, *lb_1, *ub_1,
      *lb_2, *ub_2, *new_array_lb, *new_array_ub, *lb_new, *ub_new, *new_step, *nstep,
      *step_1, *step_2, *dummy_step, *new_width, *dummy_width, *offset, *offset_1,
      *offset_2;
    gridinfo *arg_gridinfo;
    int dim, d, lb, ub, gen_counter = 0;
    constant *const_tmp;
    bool create_step;

    DBUG_ENTER ();

    const_tmp = COaST2Constant (GENERATOR_BOUND1 (PART_GENERATOR (parts_1)));
    if ((const_tmp != NULL)) {
        dim = SHgetUnrLen (COgetShape (const_tmp));
        const_tmp = COfreeConstant (const_tmp);
    } else {
        dim = 0;
        DBUG_UNREACHABLE ("lower bound of generator is not constant!");
    }

    nparts_1 = nparts_2 = NULL;
    npart_1 = npart_2 = NULL;

    offset_1 = NULL;
    offset_2 = NULL;
    dummy_step = NULL;
    dummy_width = NULL;

    parts_2_tmp = parts_2;
    while (parts_1 != NULL) {
        parts_2 = parts_2_tmp;
        while (parts_2 != NULL) {
            create_step = (PART_STEP (parts_1) || PART_STEP (parts_2));

            lb_1 = ARRAY_AELEMS (GENERATOR_BOUND1 (PART_GENERATOR (parts_1)));
            ub_1 = ARRAY_AELEMS (GENERATOR_BOUND2 (PART_GENERATOR (parts_1)));
            lb_2 = ARRAY_AELEMS (GENERATOR_BOUND1 (PART_GENERATOR (parts_2)));
            ub_2 = ARRAY_AELEMS (GENERATOR_BOUND2 (PART_GENERATOR (parts_2)));

            new_array_lb = CreateEntryFlatArray (0, dim);
            new_array_ub = CreateEntryFlatArray (0, dim);
            lb_new = ARRAY_AELEMS (new_array_lb);
            ub_new = ARRAY_AELEMS (new_array_ub);

            for (d = 0; d < dim; d++) {
                lb = MATHmax (NUM_VAL (EXPRS_EXPR (lb_1)), NUM_VAL (EXPRS_EXPR (lb_2)));
                ub = MATHmin (NUM_VAL (EXPRS_EXPR (ub_1)), NUM_VAL (EXPRS_EXPR (ub_2)));
                if (lb >= ub)
                    break; /* empty intersection */
                else {
                    NUM_VAL (EXPRS_EXPR (lb_new)) = lb;
                    NUM_VAL (EXPRS_EXPR (ub_new)) = ub;
                }
                lb_1 = EXPRS_NEXT (lb_1);
                ub_1 = EXPRS_NEXT (ub_1);
                lb_2 = EXPRS_NEXT (lb_2);
                ub_2 = EXPRS_NEXT (ub_2);
                lb_new = EXPRS_NEXT (lb_new);
                ub_new = EXPRS_NEXT (ub_new);
            }

            if (d == dim && gen_counter < global.max_newgens) {
                /* non empty generator */

                if (create_step) {

                    /* compute lowest-common-multiplier between both steps */
                    if (GENERATOR_STEP (PART_GENERATOR (parts_1)) == NULL)
                        new_step
                          = DUPdoDupNode (GENERATOR_STEP (PART_GENERATOR (parts_2)));
                    else if (GENERATOR_STEP (PART_GENERATOR (parts_2)) == NULL)
                        new_step
                          = DUPdoDupNode (GENERATOR_STEP (PART_GENERATOR (parts_1)));
                    else {
                        new_step = CreateEntryFlatArray (0, dim);
                        nstep = ARRAY_AELEMS (new_step);
                        step_1 = ARRAY_AELEMS (GENERATOR_STEP (PART_GENERATOR (parts_1)));
                        step_2 = ARRAY_AELEMS (GENERATOR_STEP (PART_GENERATOR (parts_2)));
                        for (d = 0; d < dim; d++) {
                            NUM_VAL (EXPRS_EXPR (nstep))
                              = MATHlcm (NUM_VAL (EXPRS_EXPR (step_1)),
                                         NUM_VAL (EXPRS_EXPR (step_2)));
                            nstep = EXPRS_NEXT (nstep);
                            step_1 = EXPRS_NEXT (step_1);
                            step_2 = EXPRS_NEXT (step_2);
                        }
                    }

                    /* compute offsets of both grids to new_array_lb */
                    if (GENERATOR_STEP (PART_GENERATOR (parts_1)) != NULL) {
                        if (offset_1 == NULL)
                            offset_1 = CreateEntryFlatArray (0, dim);
                        offset = ARRAY_AELEMS (offset_1);
                        lb_new = ARRAY_AELEMS (new_array_lb);
                        lb_1 = ARRAY_AELEMS (GENERATOR_BOUND1 (PART_GENERATOR (parts_1)));
                        step_1 = ARRAY_AELEMS (GENERATOR_STEP (PART_GENERATOR (parts_1)));
                        for (d = 0; d < dim; d++) {
                            NUM_VAL (EXPRS_EXPR (offset))
                              = ((NUM_VAL (EXPRS_EXPR (lb_new))
                                  - NUM_VAL (EXPRS_EXPR (lb_1)))
                                 % NUM_VAL (EXPRS_EXPR (step_1)));
                            offset = EXPRS_NEXT (offset);
                            lb_new = EXPRS_NEXT (lb_new);
                            lb_1 = EXPRS_NEXT (lb_1);
                            step_1 = EXPRS_NEXT (step_1);
                        }
                    } else {
                        if (offset_1 == NULL)
                            offset_1 = CreateEntryFlatArray (0, dim);
                        else {
                            offset = ARRAY_AELEMS (offset_1);
                            for (d = 0; d < dim; d++) {
                                NUM_VAL (EXPRS_EXPR (offset)) = 0;
                                offset = EXPRS_NEXT (offset);
                            }
                        }
                    }

                    if (GENERATOR_STEP (PART_GENERATOR (parts_2)) != NULL) {
                        if (offset_2 == NULL)
                            offset_2 = CreateEntryFlatArray (0, dim);
                        offset = ARRAY_AELEMS (offset_2);
                        lb_new = ARRAY_AELEMS (new_array_lb);
                        lb_2 = ARRAY_AELEMS (GENERATOR_BOUND1 (PART_GENERATOR (parts_2)));
                        step_2 = ARRAY_AELEMS (GENERATOR_STEP (PART_GENERATOR (parts_2)));
                        for (d = 0; d < dim; d++) {
                            NUM_VAL (EXPRS_EXPR (offset))
                              = ((NUM_VAL (EXPRS_EXPR (lb_new))
                                  - NUM_VAL (EXPRS_EXPR (lb_2)))
                                 % NUM_VAL (EXPRS_EXPR (step_2)));
                            offset = EXPRS_NEXT (offset);
                            lb_new = EXPRS_NEXT (lb_new);
                            lb_2 = EXPRS_NEXT (lb_2);
                            step_2 = EXPRS_NEXT (step_2);
                        }
                    } else {
                        if (offset_2 == NULL)
                            offset_2 = CreateEntryFlatArray (0, dim);
                        else {
                            offset = ARRAY_AELEMS (offset_2);
                            for (d = 0; d < dim; d++) {
                                NUM_VAL (EXPRS_EXPR (offset)) = 0;
                                offset = EXPRS_NEXT (offset);
                            }
                        }
                    }

                    new_width = CreateEntryFlatArray (0, dim);

                    arg_gridinfo = MakeGridInfo ();

                    /* get step and with of both parts, or create dummies */
                    if (GENERATOR_STEP (PART_GENERATOR (parts_1)) != NULL) {
                        GRIDINFO_STEP_1_AELEMS (arg_gridinfo)
                          = ARRAY_AELEMS (GENERATOR_STEP (PART_GENERATOR (parts_1)));
                        GRIDINFO_WIDTH_1_AELEMS (arg_gridinfo)
                          = ARRAY_AELEMS (GENERATOR_WIDTH (PART_GENERATOR (parts_1)));
                    } else {
                        if (dummy_step == NULL) {
                            dummy_step = CreateEntryFlatArray (1, dim);
                            dummy_width = CreateEntryFlatArray (1, dim);
                        }
                        GRIDINFO_STEP_1_AELEMS (arg_gridinfo) = ARRAY_AELEMS (dummy_step);
                        GRIDINFO_WIDTH_1_AELEMS (arg_gridinfo)
                          = ARRAY_AELEMS (dummy_width);
                    }

                    if (GENERATOR_STEP (PART_GENERATOR (parts_2)) != NULL) {
                        GRIDINFO_STEP_2_AELEMS (arg_gridinfo)
                          = ARRAY_AELEMS (GENERATOR_STEP (PART_GENERATOR (parts_2)));
                        GRIDINFO_WIDTH_2_AELEMS (arg_gridinfo)
                          = ARRAY_AELEMS (GENERATOR_WIDTH (PART_GENERATOR (parts_2)));
                    } else {
                        if (dummy_step == NULL) {
                            dummy_step = CreateEntryFlatArray (1, dim);
                            dummy_width = CreateEntryFlatArray (1, dim);
                        }
                        GRIDINFO_STEP_2_AELEMS (arg_gridinfo) = ARRAY_AELEMS (dummy_step);
                        GRIDINFO_WIDTH_2_AELEMS (arg_gridinfo)
                          = ARRAY_AELEMS (dummy_width);
                    }

                    arg_gridinfo
                      = FillGridInfo (arg_gridinfo, new_array_lb, new_array_ub, new_step,
                                      new_width, offset_1, offset_2, dim,
                                      PART_WITHID (parts_1), PART_WITHID (parts_2),
                                      PART_CODE (parts_1), PART_CODE (parts_2));

                    arg_gridinfo = IntersectGrids (arg_gridinfo);

                    if (GRIDINFO_NPARTS_1 (arg_gridinfo) != NULL) {
                        if (nparts_1) {
                            PART_NEXT (npart_1) = GRIDINFO_NPARTS_1 (arg_gridinfo);
                            PART_NEXT (npart_2) = GRIDINFO_NPARTS_2 (arg_gridinfo);
                        } else {
                            npart_1 = GRIDINFO_NPARTS_1 (arg_gridinfo);
                            npart_2 = GRIDINFO_NPARTS_2 (arg_gridinfo);
                            nparts_1 = npart_1;
                            nparts_2 = npart_2;
                            /* at least one new N_PART */
                            gen_counter++;
                        }

                        while (PART_NEXT (npart_1) != NULL) {
                            npart_1 = PART_NEXT (npart_1);
                            npart_2 = PART_NEXT (npart_2);
                            gen_counter++;
                        }
                    }

                    /* all new generators uses duplicates */
                    new_array_lb = FREEdoFreeNode (new_array_lb);
                    new_array_ub = FREEdoFreeNode (new_array_ub);
                    new_width = FREEdoFreeNode (new_width);
                    new_step = FREEdoFreeNode (new_step);
                    arg_gridinfo = FreeGridInfo (arg_gridinfo);

                    if (gen_counter > global.max_newgens) {
                        /*
                         * Max. numbers of new generators is exceeded.
                         * Remove all new generators.
                         */
                        CTIwarn (LINE_TO_LOC (global.linenum),
                                 "WLFS: number of new generators exceeds max_newgens"
                                 " -> roll back");

                        nparts_1 = FREEdoFreeTree (nparts_1);
                        nparts_2 = FREEdoFreeTree (nparts_2);
                        goto DONE;
                    }
                } else {
                    genn = TBmakeGenerator (F_wl_le, F_wl_lt, new_array_lb, new_array_ub,
                                            NULL, NULL);

                    if (nparts_1) {
                        PART_NEXT (npart_1)
                          = TBmakePart (PART_CODE (parts_1),
                                        DUPdoDupNode (PART_WITHID (parts_1)), genn);
                        CODE_INC_USED (PART_CODE (parts_1));
                        npart_1 = PART_NEXT (npart_1);

                        PART_NEXT (npart_2)
                          = TBmakePart (PART_CODE (parts_2),
                                        DUPdoDupNode (PART_WITHID (parts_2)),
                                        DUPdoDupNode (genn));
                        CODE_INC_USED (PART_CODE (parts_2));
                        npart_2 = PART_NEXT (npart_2);
                    } else {
                        npart_1 = TBmakePart (PART_CODE (parts_1),
                                              DUPdoDupNode (PART_WITHID (parts_1)), genn);
                        CODE_INC_USED (PART_CODE (parts_1));
                        nparts_1 = npart_1;

                        npart_2 = TBmakePart (PART_CODE (parts_2),
                                              DUPdoDupNode (PART_WITHID (parts_2)),
                                              DUPdoDupNode (genn));
                        CODE_INC_USED (PART_CODE (parts_2));
                        nparts_2 = npart_2;
                    }
                    gen_counter++;
                }
            } else if (d == dim) {
                /*
                 * Max. numbers of new generators is exceeded.
                 * Remove all new generators.
                 */
                CTIwarn (LINE_TO_LOC (global.linenum),
                         "WLFS: number of new generators exceeds max_newgens"
                         " -> roll back");

                nparts_1 = FREEdoFreeTree (nparts_1);
                nparts_2 = FREEdoFreeTree (nparts_2);
                new_array_lb = FREEdoFreeNode (new_array_lb);
                new_array_ub = FREEdoFreeNode (new_array_ub);
                goto DONE;
            } else {
                /* empty generator */
                new_array_lb = FREEdoFreeNode (new_array_lb);
                new_array_ub = FREEdoFreeNode (new_array_ub);
            }

            parts_2 = PART_NEXT (parts_2);
        }
        parts_1 = PART_NEXT (parts_1);
    }

DONE:
    if (offset_1)
        offset_1 = FREEdoFreeNode (offset_1);
    if (offset_2)
        offset_2 = FREEdoFreeNode (offset_2);
    if (dummy_step)
        dummy_step = FREEdoFreeNode (dummy_step);
    if (dummy_width)
        dummy_width = FREEdoFreeNode (dummy_width);

    DBUG_ASSERT ((*new_parts_2) == NULL, "new_parts_2 had to be empty");
    (*new_parts_2) = nparts_2;

    DBUG_RETURN (nparts_1);
}

/** <!--********************************************************************-->
 *
 * @fn bool BuildNewGens(node *arg_node, node *fusionable_wl)
 *
 *   @brief
 *
 *   @param  node *arg_node     : N_Nwith
 *           node *fusionable_wl: N_Nwith
 *   @return bool               : TRUE iff new generators have been build
 ******************************************************************************/
static bool
BuildNewGens (node *current_wl, node *fusionable_wl)
{
    node *new_parts_fwl, *new_parts_cwl = NULL;
    bool successful = FALSE;

    DBUG_ENTER ();

    new_parts_fwl = IntersectParts (WITH_PART (fusionable_wl), WITH_PART (current_wl),
                                    &(new_parts_cwl));

    if (new_parts_fwl != NULL) {
        successful = TRUE;

        DBUG_PRINT ("%zu new generators created", TCcountParts (new_parts_fwl));

        WITH_PART (fusionable_wl) = FREEdoFreeTree (WITH_PART (fusionable_wl));
        WITH_PART (fusionable_wl) = new_parts_fwl;
        WITH_CODE (fusionable_wl) = WLUTremoveUnusedCodes (WITH_CODE (fusionable_wl));
        DBUG_ASSERT (WITH_CODE (fusionable_wl) != NULL,
                     "all ncodes have been removed!!!");

        WITH_PART (current_wl) = FREEdoFreeTree (WITH_PART (current_wl));
        WITH_PART (current_wl) = new_parts_cwl;
        WITH_CODE (current_wl) = WLUTremoveUnusedCodes (WITH_CODE (current_wl));
        DBUG_ASSERT (WITH_CODE (fusionable_wl) != NULL,
                     "all ncodes have been removed!!!");
    }

    DBUG_RETURN (successful);
}

/****************************************************************************
 *
 * @fn bool AskFusionOracle(node *let, info *arg_info)
 *
 *   @brief  This function decides if a fusion of the current withloop and
 *           the fusionable withloop should take place.
 *           For the decision it uses the information given by the Info node.
 *
 *   @param  info *arg_info:  N_info
 *   @return bool          :  TRUE iff fusion should take place
 *****************************************************************************/
static bool
AskFusionOracle (info *arg_info)
{
    node *wl, *fwl;
    bool answer = FALSE, is_build;

    DBUG_ENTER ();

    wl = INFO_WL (arg_info);
    fwl = ASSIGN_RHS (INFO_FUSIONABLE_WL (arg_info));

    /* is the current WL independent from the fusionable WL ? */
    if (!INFO_WLDEPENDENT (arg_info)) {

        /*
         * If the generators of both withloops are at least constant
         * we can create new indentically generators for them by intersecting
         * their bounds, steps and width.
         * Additionly we need to know especially by fold operators if the
         * lower bound of at least one generator is the zero vector.
         * Only then a full partition is garantied.
         */
        if (INFO_GENPROPERTY (arg_info) == GEN_constant && INFO_WL_LB_IS_ZERO (arg_info)
            && INFO_FWL_LB_IS_ZERO (arg_info)) {

            /* is the size of both wl iteration spaces equal? */
            if (CheckIterationSpace (arg_info)) {
                /*
                 * build new generators
                 * -> INFO_GENPROPERTY(arg_info) = GEN_equal
                 */
                DBUG_PRINT ("build new generators");
                is_build = BuildNewGens (wl, fwl);
                if (is_build) {
                    INFO_GENPROPERTY (arg_info) = GEN_equal;
                }
            }
        }

        if ((INFO_GENPROPERTY (arg_info) == GEN_equal
             || INFO_GENPROPERTY (arg_info) == GEN_equal_var)
            && TCcountParts (WITH_PART (wl)) == TCcountParts (WITH_PART (fwl))) {
            DBUG_PRINT ("both With-Loops can be fused together");
            answer = TRUE;
        } else {
            DBUG_PRINT ("both With-Loops can't be fused together");
        }
    } else {
        /*
         * The current genarray-withloop depends on the fusionable. Append it to
         * INFO_REFERENCES_FUSIONABLE( arg_info)
         */
        DBUG_PRINT ("With-Loop depends on fusionable With-Loop");
        INFO_REFERENCES_FUSIONABLE (arg_info)
          = TCnodeListAppend (INFO_REFERENCES_FUSIONABLE (arg_info),
                              INFO_ASSIGN (arg_info), NULL);
    }

    DBUG_RETURN (answer);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSfundef(node *arg_node, info *arg_info)
 *
 *   @brief starts the traversal of the given fundef
 *
 *   @param  node *arg_node:  N_fundef
 *           info *arg_info:  N_info
 *   @return node *        :  N_fundef
 ******************************************************************************/
node *
WLFSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("Fusing With-Loops in function %s", FUNDEF_NAME (arg_node));

        arg_info = MakeInfo ();

        INFO_FUNDEF (arg_info) = arg_node;

        FUNDEF_ASSIGNS (arg_node) = TRAVdo (FUNDEF_ASSIGNS (arg_node), arg_info);

        arg_info = FreeInfo (arg_info);

        DBUG_PRINT ("Fusing of With-Loops in function %s complete",
                    FUNDEF_NAME (arg_node));
    }

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSblock(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_block
 *           info *arg_info:  N_info
 *   @return node *        :  N_block
 ******************************************************************************/
node *
WLFSblock (node *arg_node, info *arg_info)
{
    info *info_tmp;

    DBUG_ENTER ();

    /* Info node has to be stacked before traversal */
    info_tmp = arg_info;
    arg_info = MakeInfo ();
    INFO_FUNDEF (arg_info) = INFO_FUNDEF (info_tmp);

    BLOCK_ASSIGNS (arg_node) = TRAVopt(BLOCK_ASSIGNS (arg_node), arg_info);

    arg_info = FreeInfo (arg_info);
    arg_info = info_tmp;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSassign(node *arg_node, info *arg_info)
 *
 *   @brief store actual assign node in arg_info and traverse instruction
 *
 *   @param  node *arg_node:  N_assign
 *           info *arg_info:  N_info
 *   @return node *        :  N_assign
 ******************************************************************************/
node *
WLFSassign (node *arg_node, info *arg_info)
{
    node *assignn;
    node *assigns2shift = NULL;
    info *stacked_info = NULL;

    DBUG_ENTER ();

    INFO_ASSIGN (arg_info) = arg_node;

    if (INFO_FUSIONABLE_WL (arg_info) == NULL
        || ASSIGN_VISITED_WITH (arg_node) != INFO_FUSIONABLE_WL (arg_info)) {
        /*
         * This is the first visit of this assign with the current fusionable WL,
         * or there is no fusionable WL for now.
         */

        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

        if (ASSIGN_STMTTYPE (arg_node) == N_let
            && NODE_TYPE (ASSIGN_RHS (arg_node)) == N_with) {

            if (INFO_WLACTION (arg_info) == WL_fused) {
                /*
                 * The withloop on the rhs of current assignment is fused with
                 * another withloop, so the current one is obsolete and has to
                 * be freed.
                 */
                arg_node = FREEdoFreeNode (arg_node);

                /*
                 * Now we have to traverse back to the other withloop
                 * to finish the tranformation
                 */
                INFO_WLACTION (arg_info) = WL_travback;
                DBUG_PRINT (" starting travback");
                DBUG_RETURN (arg_node);
            } else if (INFO_WLACTION (arg_info) == WL_2fuse) {

                /* mark assign as already visited */
                ASSIGN_VISITED_WITH (arg_node) = INFO_FUSIONABLE_WL (arg_info);

                /*
                 * The current WL is not fusionable with the one
                 * stored in INFO_FUSIONABLE_WL( arg_info).
                 * We stack the current information and set
                 * INFO_FUSIONABLE_WL( arg_info) onto this WL for the further
                 * traversal.
                 */
                DBUG_PRINT ("current WL is now fusionable WL");
                INFO_WLACTION (arg_info) = WL_nothing;
                stacked_info = arg_info;
                arg_info = MakeInfo ();
                arg_info = UpdateInfo (arg_info, stacked_info);
            }
        }
    }

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    do {
        if (INFO_WLACTION (arg_info) == WL_travback) {
            /*
             * Traverse back mode
             * We traverse back to the fusionable wl, pick and append
             * all assignments to INFO_ASSIGNS2SHIFT( arg_info) which
             * are tagged with this WL and hang them out of the sytax tree.
             * To avoid a later visit on already visited assigns, we mark this
             * assigns as visited with the current fusionable WL.
             *
             * When we reach the fusionable wl, the list of assignments had to
             * be shifted in front of the WL to obtain dependencies.
             *
             * Accordingly we carry on with the search of a next withloop to fuse.
             */

            DBUG_PRINT ("travback ...");

            if (ASSIGN_NEXT (arg_node) != NULL
                && ASSIGN_TAG (ASSIGN_NEXT (arg_node)) == INFO_FUSIONABLE_WL (arg_info)) {
                /*
                 * ASSIGN_TAG( ASSIGN_NEXT( arg_node)) can't be NULL,
                 * because in this mode INFO_FUSIONABLE_WL( arg_info)
                 * can't be NULL
                 */

                DBUG_PRINT ("collect assign:");
                DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, ASSIGN_NEXT (arg_node)));

                assignn = ASSIGN_NEXT (arg_node);
                ASSIGN_NEXT (arg_node) = ASSIGN_NEXT (ASSIGN_NEXT (arg_node));

                /* to obtain the same order as in syntax tree */
                ASSIGN_NEXT (assignn) = INFO_ASSIGNS2SHIFT (arg_info);
                INFO_ASSIGNS2SHIFT (arg_info) = assignn;
            }

            if (INFO_FUSIONABLE_WL (arg_info) == arg_node) {

                INFO_WLACTION (arg_info) = WL_nothing;

                global.optcounters.wlfs_expr++;

                if (INFO_ASSIGNS2SHIFT (arg_info) != NULL) {

                    assigns2shift
                      = TCappendAssign (assigns2shift, INFO_ASSIGNS2SHIFT (arg_info));

                    INFO_ASSIGNS2SHIFT (arg_info) = NULL;
                }

                DBUG_PRINT ("fusion is finished, starting search for new WL");

                ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

            } else {
                ASSIGN_VISITED_WITH (arg_node) = INFO_FUSIONABLE_WL (arg_info);
                break;
            }
        } else if (stacked_info != NULL) {
            /*
             * Backtraversal mode
             * If we reach a withloop, where we stack a fusionable withloop
             * we had to pop it and to traverse with it down again.
             */

            arg_info = FreeInfo (arg_info);
            arg_info = stacked_info;
            stacked_info = NULL;

            DBUG_PRINT ("pop stacked fusionable WL");

            if (assigns2shift != NULL) {

                arg_node = TCappendAssign (assigns2shift, arg_node);

                DBUG_PRINT ("shifted depended assigns in front of WL");

                assigns2shift = NULL;
            }

            ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);
        } else {
            /* cleaning up */

            DBUG_PRINT ("cleaning up");
            ASSIGN_VISITED_WITH (arg_node) = NULL;
            ASSIGN_TAG (arg_node) = NULL;

            if (assigns2shift != NULL) {
                DBUG_PRINT ("shifted depended assigns in front of WL");
                arg_node = TCappendAssign (assigns2shift, arg_node);
                assigns2shift = NULL;
            }
            break;
        }

    } while (TRUE);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSid(node *arg_node, info *arg_info)
 *
 *   @brief  Checks if this Id is contained in
 *             INFO_REFERENCES_FUSIONABLE( arg_info).
 *           If it is contained the current assigment is (indirect)
 *           dependent from the fusionable withloop.
 *
 *   @param  node *arg_node:  N_id
 *           info *arg_info:  N_info
 *   @return node *        :  N_id
 ******************************************************************************/
node *
WLFSid (node *arg_node, info *arg_info)
{
    bool is_dependent;

    DBUG_ENTER ();

    if (INFO_FUSIONABLE_WL (arg_info) != NULL) {
        /*
         * Normal mode
         * All assignments which are (indirect) dependend from fusionable withloop
         * are collected
         */
        is_dependent = CheckDependency (arg_node, INFO_REFERENCES_FUSIONABLE (arg_info));
        if (is_dependent) {
            DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, INFO_ASSIGN (arg_info)));
            DBUG_PRINT ("%s references the fusionable With-Loop, note assign",
                        ID_NAME (arg_node));

            INFO_REFERENCES_FUSIONABLE (arg_info)
              = TCnodeListAppend (INFO_REFERENCES_FUSIONABLE (arg_info),
                                  INFO_ASSIGN (arg_info), NULL);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSwith(node *arg_node, info *arg_info)
 *
 *   @brief  start traversal of this WL and store information in new arg_info
 *           node. The only N_Npart node (inclusive body) is traversed.
 *           Afterwards, if certain conditions are fulfilled,
 *           the WL is transformed into a WL with generators describing a full
 *           partition.
 *
 *   @param  node *arg_node:  N_Nwith
 *           info *arg_info:  N_info
 *   @return node *        :  N_Nwith
 ******************************************************************************/

node *
WLFSwith (node *arg_node, info *arg_info)
{
    node *fwl;
    wl_action_t wl_action = WL_nothing;

    DBUG_ENTER ();

    /*
     * first traversal in CODEs
     * The CODEs have to be traversed as they may contain further (nested) WLs
     * and I want to modify bottom up.
     */
    DBUG_PRINT ("trav NCODE of WL");
    WITH_CODE (arg_node) = TRAVopt(WITH_CODE (arg_node), arg_info);
    DBUG_PRINT ("trav NCODE of WL finished");

    /* initialise some pointers */
    arg_info = InitInfo (arg_info);

    /*
     * try to fuse the current With-Loop
     */
    DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node));

    /*
     * The second traversal into the N_CODEs ought to detect possible
     * dependencies from current withloop to the fusionable.
     * The result is stored in WITH_WLDEPENDENT( arg_node)
     */
    if (INFO_FUSIONABLE_WL (arg_info) != NULL) {

        arg_node = DDEPENDdoDetectDependencies (arg_node, INFO_FUSIONABLE_WL (arg_info),
                                                INFO_REFERENCES_FUSIONABLE (arg_info));

        INFO_WLDEPENDENT (arg_info) = WITH_ISDEPENDENT (arg_node);
        WITH_ISDEPENDENT (arg_node) = FALSE;
    } else
        INFO_WLDEPENDENT (arg_info) = FALSE;

    /*
     * If the generators of the current withloop build a full partition
     * the PARTS attribute carries a positive value. Only those withloops
     * are considered further. These are at least AKD withloops.
     * Futhermore we consider only WLs with non-empty iteration space
     */

    if (!TCcontainsDefaultPartition (WITH_PART (arg_node))
        && (SHgetUnrLen (TYgetShape (AVIS_TYPE (IDS_AVIS (WITH_VEC (arg_node))))) > 0)) {

        INFO_WL (arg_info) = arg_node; /* store the current node for later */

        /*
         * Now, we traverse the WITHOP sons for checking types and shape
         */
        INFO_LHS_WL (arg_info) = ASSIGN_LHS (INFO_ASSIGN (arg_info));
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        /*
         * traverse the N_PARTs.
         * three values are computed during this traversal:
         *
         *  INFO_GENPROPERTY(arg_info)
         *
         *  and if the array type (and poss. shape) of the index vector
         *  is not yet known
         *  INFO_WL_ARRAY_TYPE(arg_info)
         *  INFO_WL_SHAPE( arg_info)
         */

        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

        /* is there a WL we can fuse with and is fusion with fold WLs permitted? */
        if ((INFO_FUSIONABLE_WL (arg_info) != NULL)
            && !(global.no_fold_fusion && CONTAINS_FOLD (INFO_WL_WOTYPE (arg_info)))) {

            wl_action = WL_2fuse;
            fwl = INFO_FUSIONABLE_WL (arg_info);

            if (AskFusionOracle (arg_info)) {
                /*
                 * Tag all assignments the current withloop depends on.
                 * Later the tagged assignments !between! the two fused withloops
                 * had to be shifted in front of the result withloop.
                 */
                arg_node = TDEPENDdoTagDependencies (arg_node, fwl);

                /* fuse both withloops together */
                DBUG_PRINT ("starting fusion of both With-Loops");
                arg_node = FuseWithloops (arg_node, arg_info, fwl);
                DBUG_PRINT ("fused With-Loops:");
                DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, fwl));
                wl_action = WL_fused;
            }
        } else {
            if (!(global.no_fold_fusion && CONTAINS_FOLD (INFO_WL_WOTYPE (arg_info)))) {
                /*
                 * this WL is the first fusionable WL
                 */
                DBUG_PRINT ("With-Loop is the first fusionable With-Loop");
                arg_info = UpdateInfo (arg_info, arg_info);
            }
        }
    } else { /* #Parts <= 0 || -2 <= dim <= 0 */
        DBUG_PRINT ("With-Loop is AUD or has empty iteration space");

        if ((INFO_FUSIONABLE_WL (arg_info) != NULL) && (INFO_WLDEPENDENT (arg_info))) {
            /*
             * The current withloop depends on the fusionable. Append it to
             * INFO_REFERENCES_FUSIONABLE( arg_info)
             */
            INFO_REFERENCES_FUSIONABLE (arg_info)
              = TCnodeListAppend (INFO_REFERENCES_FUSIONABLE (arg_info),
                                  INFO_ASSIGN (arg_info), NULL);
        }
    }

    INFO_WLACTION (arg_info) = wl_action;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSgenarray( node *arg_node, info *arg_info)
 *
 *   @brief checks the type of withloop operator, tries to determine
 *          the shape of the index vektor and the type of array
 *          (AKS, AKD, AUD).
 *
 *   @param  node *arg_node:  N_genarray
 *           info *arg_info:  N_info
 *   @return node *        :  N_genarray
 ******************************************************************************/

node *
WLFSgenarray (node *arg_node, info *arg_info)
{
    wo_type_t current_type = WOT_unknown;
    constant *const_expr;

    DBUG_ENTER ();

    if (INFO_WL_ARRAY_TYPE (arg_info) == ARRAY_unknown) {
        const_expr = COaST2Constant (GENARRAY_SHAPE (arg_node));
        if (const_expr != NULL) {
            /*
             * since all shapes of fused WL had to be be identical so far,
             * it is sufficient to take first inferred shape
             */
            INFO_WL_ARRAY_TYPE (arg_info) = ARRAY_aks;
            INFO_WL_SHAPE (arg_info) = const_expr;
        } else
            INFO_WL_ARRAY_TYPE (arg_info) = ARRAY_akd;
    }

    if (INFO_WL_WOTYPE (arg_info) == WOT_unknown) {
        current_type = WOT_gen_mod;
    } else if (INFO_WL_WOTYPE (arg_info) == WOT_fold) {
        current_type = WOT_gen_mod_fold;
    } else {
        current_type = INFO_WL_WOTYPE (arg_info);
    }

    INFO_WL_WOTYPE (arg_info) = current_type;

    INFO_LHS_WL (arg_info) = IDS_NEXT (INFO_LHS_WL (arg_info));

    GENARRAY_NEXT (arg_node) = TRAVopt(GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSmodarray( node *arg_node, info *arg_info)
 *
 *   @brief checks the type of withloop operator, tries to determine
 *          the shape of the index vektor and the type of array
 *          (AKS, AKD, AUD).
 *
 *   @param  node *arg_node:  N_modarray
 *           info *arg_info:  N_info
 *   @return node *        :  N_modarray
 ******************************************************************************/

node *
WLFSmodarray (node *arg_node, info *arg_info)
{
    wo_type_t current_type = WOT_unknown;
    constant *const_expr;
    shape *new_shp, *shp;
    ntype *type;
    int iv_shape, i;

    DBUG_ENTER ();

    if (INFO_WL_ARRAY_TYPE (arg_info) == ARRAY_unknown) {

        type = AVIS_TYPE (IDS_AVIS (INFO_LHS_WL (arg_info)));
        if (TYisAKS (type) || TYisAKV (type)) {
            shp = TYgetShape (type);

            /* get shape of the index vector */
            iv_shape
              = SHgetExtent (TYgetShape (IDS_NTYPE (WITH_VEC (INFO_WL (arg_info)))), 0);
            DBUG_ASSERT (iv_shape > 0, "shape of index vector has to be > 0!");

            if (SHgetDim (shp) != iv_shape) {
                new_shp = SHmakeShape (iv_shape);

                for (i = 0; i < iv_shape; i++) {
                    new_shp = SHsetExtent (new_shp, i, SHgetExtent (shp, i));
                }
                const_expr = COmakeConstantFromShape (new_shp);
            } else
                const_expr = COmakeConstantFromShape (shp);

            /*
             * since all shapes of fused WL had to be be identical so far,
             * it is sufficient to take first inferred shape
             */
            INFO_WL_ARRAY_TYPE (arg_info) = ARRAY_aks;
            INFO_WL_SHAPE (arg_info) = const_expr;
        } else {
            /*
             * nothing for now
             * we try to get more information later by upper bounds of generators
             */
        }
    }

    if (INFO_WL_WOTYPE (arg_info) == WOT_unknown) {
        current_type = WOT_gen_mod;
    } else if (INFO_WL_WOTYPE (arg_info) == WOT_fold) {
        current_type = WOT_gen_mod_fold;
    } else {
        current_type = INFO_WL_WOTYPE (arg_info);
    }

    INFO_WL_WOTYPE (arg_info) = current_type;

    INFO_LHS_WL (arg_info) = IDS_NEXT (INFO_LHS_WL (arg_info));

    MODARRAY_NEXT (arg_node) = TRAVopt(MODARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSfold( node *arg_node, info *arg_info)
 *
 *   @brief checks the type of withloop operator, tries to determine
 *          the shape of the index vektor and the type of array
 *          (AKS, AKD, AUD).
 *
 *   @param  node *arg_node:  N_fold
 *           info *arg_info:  N_info
 *   @return node *        :  N_fold
 ******************************************************************************/

node *
WLFSfold (node *arg_node, info *arg_info)
{
    wo_type_t current_type = WOT_unknown;

    DBUG_ENTER ();

    if (INFO_WL_WOTYPE (arg_info) == WOT_unknown) {
        current_type = WOT_fold;
    } else if (INFO_WL_WOTYPE (arg_info) == WOT_gen_mod) {
        current_type = WOT_gen_mod_fold;
    } else {
        current_type = INFO_WL_WOTYPE (arg_info);
    }

    INFO_WL_WOTYPE (arg_info) = current_type;

    INFO_LHS_WL (arg_info) = IDS_NEXT (INFO_LHS_WL (arg_info));

    FOLD_NEXT (arg_node) = TRAVopt(FOLD_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSpart(node *arg_node, info *arg_info)
 *
 *   @brief traverse only generator to check bounds, step and width
 *
 *   @param  node *arg_node:  N_part
 *           info *arg_info:  N_info
 *   @return node *        :  N_part
 ******************************************************************************/

node *
WLFSpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    PART_NEXT (arg_node) = TRAVopt(PART_NEXT (arg_node), arg_info);

    if ((INFO_WL_SHAPE (arg_info) != NULL)
        && (INFO_WL_ARRAY_TYPE (arg_info) == ARRAY_unknown)) {
        /* the shape of the index vector has been detemined  -> AKS array*/
        INFO_WL_ARRAY_TYPE (arg_info) = ARRAY_aks;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSgenerator(node *arg_node, info *arg_info)
 *
 *   @brief  bounds, step and width vectors are compared with bounds, step
 *           and width of all generators of the withloop out of
 *           INFO_WL2FUSION( arg_info).
 *
 *           Via INFO_GENPROPERTY( arg_info) the status of the generator
 *           is returned. Possible values are :
 *           GEN_equal    : this generator is equal to one generator of the
 *                          comparative withloop!
 *           GEN_constant : this generator is not equal to any generator of the
 *                          comparative withloop, but all bounds, steps and
 *                          widths are constant!
 *           GEN_variable : this generator is not equal to any generator of the
 *                          comparative withloop and at least one bound, step
 *                          or width is variable
 *           GEN_diffdim  : generators have different dimensions
 *
 *           If the type of array of the index vector (and also the shape)
 *           is not yet known
 *            (stored in INFO_WL_ARRAY_TYPE and INFO_WL_SHAPE),
 *           we have to detemine it by maximum upper bound over every generator
 *
 *           Additionly we need to know especially by fold operators if the
 *           lower bound of at least one generator is the zero vector.
 *           Only then a full Partition is garantied.
 *
 *   @param  node *arg_node:  N_generator
 *           info *arg_info:  N_info
 *   @return node *        :  N_generator
 ******************************************************************************/

node *
WLFSgenerator (node *arg_node, info *arg_info)
{
    node *parts, *gen;
    gen_property_t gen_prob, gen_prob_ret;
    constant *const_expr, *max_shape, *tmpc, *const_lb;

    DBUG_ENTER ();

    if (INFO_FUSIONABLE_WL (arg_info) != NULL) {

        if (INFO_GENPROPERTY (arg_info) < GEN_variable) {
            parts = WITH_PART (ASSIGN_RHS (INFO_FUSIONABLE_WL (arg_info)));

            gen_prob = GEN_diffdim;
            while (parts != NULL) {
                gen = PART_GENERATOR (parts);
                gen_prob
                  = CompGenSon (GENERATOR_BOUND1 (arg_node), GENERATOR_BOUND1 (gen));
                gen_prob_ret
                  = CompGenSon (GENERATOR_BOUND2 (arg_node), GENERATOR_BOUND2 (gen));
                RESULT_GEN_PROP (gen_prob, gen_prob_ret);
                gen_prob_ret
                  = CompGenSon (GENERATOR_STEP (arg_node), GENERATOR_STEP (gen));
                RESULT_GEN_PROP (gen_prob, gen_prob_ret);
                gen_prob_ret
                  = CompGenSon (GENERATOR_WIDTH (arg_node), GENERATOR_WIDTH (gen));
                RESULT_GEN_PROP (gen_prob, gen_prob_ret);

                if (gen_prob == GEN_equal || gen_prob == GEN_equal_var)
                    break;
                parts = PART_NEXT (parts);
            }
            RESULT_GEN_PROP (INFO_GENPROPERTY (arg_info), gen_prob);
        }
    }

    if (!INFO_WL_LB_IS_ZERO (arg_info)
        && (INFO_GENPROPERTY (arg_info) == GEN_equal
            || INFO_GENPROPERTY (arg_info) == GEN_constant)) {
        const_lb = COaST2Constant (GENERATOR_BOUND1 (arg_node));
        if ((const_lb != NULL) && COisZero (const_lb, TRUE)) {
            INFO_WL_LB_IS_ZERO (arg_info) = TRUE;
            const_lb = COfreeConstant (const_lb);
        }
    }

    if (INFO_WL_ARRAY_TYPE (arg_info) == ARRAY_unknown) {
        const_expr = COaST2Constant (GENERATOR_BOUND2 (arg_node));
        if (const_expr != NULL) {
            max_shape = INFO_WL_SHAPE (arg_info);
            if (max_shape != NULL) {
                tmpc = COge (const_expr, max_shape, NULL);
                if (COisTrue (tmpc, TRUE)) {
                    INFO_WL_SHAPE (arg_info) = COfreeConstant (INFO_WL_SHAPE (arg_info));
                    INFO_WL_SHAPE (arg_info) = const_expr;
                }
                tmpc = COfreeConstant (tmpc);
            } else
                INFO_WL_SHAPE (arg_info) = const_expr;
        } else {
            INFO_WL_ARRAY_TYPE (arg_info) = ARRAY_akd;
            if (INFO_WL_SHAPE (arg_info) != NULL)
                INFO_WL_SHAPE (arg_info) = COfreeConstant (INFO_WL_SHAPE (arg_info));
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSdoWithloopFusion( node *arg_node)
 *
 *   @brief  Starting point for the withloop fusion if it was called
 *           from optimize.
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/

node *
WLFSdoWithloopFusion (node *arg_node)
{
    DBUG_ENTER ();

    TRAVpush (TR_wlfs);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
