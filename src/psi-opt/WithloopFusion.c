/*
 *
 * $Log$
 * Revision 1.10  2004/09/23 17:25:41  khf
 * proceeding implementation (intersection of generators
 * without step and width)
 *
 * Revision 1.9  2004/08/31 19:33:21  khf
 * proproceeding implementation
 *
 * Revision 1.8  2004/08/29 11:00:40  khf
 * proceeding implementation
 *
 * Revision 1.7  2004/08/26 15:06:52  khf
 * detection and tag of dependencies are sourced out into
 * detectdependencies.c and tagdependencies.c
 *
 * Revision 1.6  2004/07/22 17:28:37  khf
 * Special functions are now traversed when they are used
 *
 * Revision 1.5  2004/07/21 12:47:35  khf
 * switch to new INFO structure
 *
 * Revision 1.4  2004/06/30 12:24:54  khf
 * Only WLs with non-empty iteration space are considered
 *
 * Revision 1.3  2004/05/07 13:07:08  khf
 * some debugging
 *
 * Revision 1.2  2004/05/04 17:06:49  khf
 * some debugging
 *
 * Revision 1.1  2004/04/08 08:15:56  khf
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "DupTree.h"
#include "Error.h"
#include "globals.h"
#include "dbug.h"
#include "traverse.h"
#include "constants.h"
#include "ssa.h"
#include "optimize.h"
#include "WithloopFusion.h"
#include "detectdependencies.h"
#include "tagdependencies.h"

/**
 * INFO structure
 */
struct INFO {
    node *wl;
    node *fundef;
    node *assign;
    node *let;
    int wlaction;
    int genproperty;
    int wotype;
    bool wo_contains_fold;
    bool no_step_width;
    bool wldependent;
    int wl_array_type;
    constant *wl_shape;
    node *fusionable_wl;
    nodelist *references_fusionable;
    bool fwo_contains_fold;
    int fwl_array_type;
    constant *fwl_shape;
    node *assigns2shift;
};

/* usage of arg_info: */
#define INFO_WLFS_WL(n) (n->wl)
#define INFO_WLFS_FUNDEF(n) (n->fundef)
#define INFO_WLFS_ASSIGN(n) (n->assign)
#define INFO_WLFS_LET(n) (n->let)
#define INFO_WLFS_WLACTION(n) (n->wlaction)
#define INFO_WLFS_GENPROPERTY(n) (n->genproperty)
#define INFO_WLFS_WOTYPE(n) (n->wotype)
#define INFO_WLFS_WO_CONTAINS_FOLD(n) (n->wo_contains_fold)
#define INFO_WLFS_NO_STEP_WIDTH(n) (n->no_step_width)
#define INFO_WLFS_WLDEPENDENT(n) (n->wldependent)
#define INFO_WLFS_WL_ARRAY_TYPE(n) (n->wl_array_type)
#define INFO_WLFS_WL_SHAPE(n) (n->wl_shape)

#define INFO_WLFS_FUSIONABLE_WL(n) (n->fusionable_wl)
#define INFO_WLFS_REFERENCES_FUSIONABLE(n) (n->references_fusionable)
#define INFO_WLFS_FWO_CONTAINS_FOLD(n) (n->fwo_contains_fold)
#define INFO_WLFS_FWL_ARRAY_TYPE(n) (n->fwl_array_type)
#define INFO_WLFS_FWL_SHAPE(n) (n->fwl_shape)
#define INFO_WLFS_ASSIGNS2SHIFT(n) (n->assigns2shift)

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
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_WLFS_WL (result) = NULL;
    INFO_WLFS_FUNDEF (result) = NULL;
    INFO_WLFS_ASSIGN (result) = NULL;
    INFO_WLFS_LET (result) = NULL;
    INFO_WLFS_WLACTION (result) = WL_nothing;
    INFO_WLFS_GENPROPERTY (result) = GEN_equal;
    INFO_WLFS_WOTYPE (result) = WOT_unknown;
    INFO_WLFS_WO_CONTAINS_FOLD (result) = FALSE;
    INFO_WLFS_NO_STEP_WIDTH (result) = TRUE;
    INFO_WLFS_WLDEPENDENT (result) = FALSE;
    INFO_WLFS_WL_ARRAY_TYPE (result) = ARRAY_unknown;
    INFO_WLFS_WL_SHAPE (result) = NULL;
    INFO_WLFS_FUSIONABLE_WL (result) = NULL;
    INFO_WLFS_REFERENCES_FUSIONABLE (result) = NULL;
    INFO_WLFS_FWO_CONTAINS_FOLD (result) = FALSE;
    INFO_WLFS_FWL_ARRAY_TYPE (result) = ARRAY_unknown;
    INFO_WLFS_FWL_SHAPE (result) = NULL;
    INFO_WLFS_ASSIGNS2SHIFT (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    if (INFO_WLFS_REFERENCES_FUSIONABLE (info) != NULL) {
        INFO_WLFS_REFERENCES_FUSIONABLE (info)
          = NodeListFree (INFO_WLFS_REFERENCES_FUSIONABLE (info), TRUE);
    }
    if (INFO_WLFS_FWL_SHAPE (info) != NULL) {
        INFO_WLFS_FWL_SHAPE (info) = COFreeConstant (INFO_WLFS_FWL_SHAPE (info));
    }

    info = Free (info);

    DBUG_RETURN (info);
}

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

    DBUG_ENTER ("CheckDependency");

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

    DBUG_ENTER ("CheckIterationSpace");

    DBUG_ASSERT ((INFO_WLFS_WL_ARRAY_TYPE (arg_info) == ARRAY_aks
                  && INFO_WLFS_FWL_ARRAY_TYPE (arg_info) == ARRAY_aks),
                 "Both ARRAY_TYPES had to be aks!");

    DBUG_ASSERT ((INFO_WLFS_WL_SHAPE (arg_info) != NULL
                  && INFO_WLFS_FWL_SHAPE (arg_info) != NULL),
                 "Both SHAPEs had to be non empty!");

    shape1 = INFO_WLFS_WL_SHAPE (arg_info);
    shape2 = INFO_WLFS_FWL_SHAPE (arg_info);

    if (SHGetUnrLen (COGetShape (shape1)) == SHGetUnrLen (COGetShape (shape2))) {
        tmpc = COEq (shape1, shape2);
        if (COIsTrue (tmpc, TRUE))
            is_equal = TRUE;
        else
            is_equal = FALSE;
        tmpc = COFreeConstant (tmpc);
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

    DBUG_ENTER ("CompGenSon");

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
                DBUG_ASSERT ((0), "Unknown elements found!");
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
                if (!strcmp (ID_NAME (EXPRS_EXPR (elems1)),
                             ID_NAME (EXPRS_EXPR (elems2)))) {
                    if (gen_prob == GEN_equal)
                        gen_prob = GEN_equal_var;
                    else if (gen_prob == GEN_constant) {
                        gen_prob = GEN_variable;
                        break;
                    }
                } else {
                    gen_prob = GEN_variable;
                    break;
                }
            } else {
                gen_prob = GEN_variable;
                break;
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
    node *first_accu = NULL, *second_accu = NULL, *tmp, *predecessor;
    ids *tmp_ids;

    DBUG_ENTER ("FuseAccu");

    DBUG_ASSERT ((assigns1 != NULL && assigns2 != NULL),
                 "FuseAccu called with empty assigns");
    DBUG_ASSERT (((NODE_TYPE (assigns1) == N_assign)
                  && (NODE_TYPE (assigns2) == N_assign)),
                 "FuseAccu not called with N_assigns");

    tmp = assigns1;
    while (tmp != NULL) {
        if ((NODE_TYPE (ASSIGN_RHS (tmp)) == N_prf)
            && (PRF_PRF (ASSIGN_RHS (tmp)) == F_accu)) {
            DBUG_ASSERT ((first_accu == NULL),
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

            DBUG_ASSERT ((second_accu == NULL),
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
        tmp_ids = DupAllIds (ASSIGN_LHS (second_accu));
        ASSIGN_LHS (first_accu) = AppendIds (ASSIGN_LHS (first_accu), tmp_ids);
        while (tmp_ids != NULL) {
            /* set correct backref to defining assignment */
            AVIS_SSAASSIGN (IDS_AVIS (tmp_ids)) = first_accu;
            tmp_ids = IDS_NEXT (tmp_ids);
        }

        if (predecessor == NULL)
            assigns2 = FreeNode (second_accu);
        else
            ASSIGN_NEXT (predecessor) = FreeNode (second_accu);
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
 *   @param  node *pattern  :  N_Ngenerator
 *           node *parts    :  N_Npart list
 *   @return node *         :  N_Npart with equal generator to pattern
 *                             else NULL
 ******************************************************************************/
static node *
FindFittingPart (node *pattern, node *parts)
{
    node *gen, *tmp_part = NULL;
    gen_property_t gen_prob, gen_prob_ret;

    DBUG_ENTER ("FindFittingPart");

    while (parts != NULL) {
        gen = NPART_GEN (parts);
        gen_prob = CompGenSon (NGEN_BOUND1 (pattern), NGEN_BOUND1 (gen));
        gen_prob_ret = CompGenSon (NGEN_BOUND2 (pattern), NGEN_BOUND2 (gen));
        RESULT_GEN_PROP (gen_prob, gen_prob_ret);
        gen_prob_ret = CompGenSon (NGEN_STEP (pattern), NGEN_STEP (gen));
        RESULT_GEN_PROP (gen_prob, gen_prob_ret);
        gen_prob_ret = CompGenSon (NGEN_WIDTH (pattern), NGEN_WIDTH (gen));
        RESULT_GEN_PROP (gen_prob, gen_prob_ret);
        if (gen_prob == GEN_equal || gen_prob == GEN_equal_var) {
            tmp_part = parts;
            break;
        }
        parts = NPART_NEXT (parts);
    }

    DBUG_RETURN (tmp_part);
}

/** <!--********************************************************************-->
 *
 * @fn node *FuseNCodes( node *extendable_part, node *fitting_part,
 *                       node *fusionable_wl, bool both_contain_fold,
 *                       node *fundef)
 *
 *   @brief inserts the whole assignment block of 'fitting_part' in
 *          assignment block of 'extendable_part'.
 *
 *   @param  node *extendable_part : N_Npart
 *           node *fitting_part    : N_Npart
 *           node *fusionable_wl   : N_Nwith current WL will be fused with
 *           node *fundef          : N_fundef
 *   @return node                  : modified extendable_part
 ******************************************************************************/
static node *
FuseNCodes (node *extendable_part, node *fitting_part, node *fusionable_wl,
            bool both_contain_fold, node *fundef)
{
    node *fusionable_ncode, *fitting_nncode, *fitting_assigns, *extendable_assigns,
      *fitting_exprs;
    LUT_t lut;
    ids *oldvec, *newvec, *oldids, *newids;

    DBUG_ENTER ("FuseNCodes");

    /*
     * If the N_Ncode of 'extendable_part' is referenced several
     * it has to be deepcopied before extend.
     */
    if (NCODE_USED (NPART_CODE (extendable_part)) != 1) {
        fusionable_ncode = NWITH_CODE (fusionable_wl);

        if (fusionable_ncode == NPART_CODE (extendable_part)) {
            NCODE_USED (fusionable_ncode)--;
            NPART_CODE (extendable_part)
              = DupNodeSSA (NPART_CODE (extendable_part), fundef);
            NCODE_USED (NPART_CODE (extendable_part)) = 1;
            NWITH_CODE (fusionable_wl) = NPART_CODE (extendable_part);
            NCODE_NEXT (NPART_CODE (extendable_part)) = fusionable_ncode;
        } else {
            while (NCODE_NEXT (fusionable_ncode) != NPART_CODE (extendable_part)) {
                fusionable_ncode = NCODE_NEXT (fusionable_ncode);
            }
            NCODE_USED (NCODE_NEXT (fusionable_ncode))--;
            NPART_CODE (extendable_part)
              = DupNodeSSA (NPART_CODE (extendable_part), fundef);
            NCODE_USED (NPART_CODE (extendable_part)) = 1;
            NCODE_NEXT (NPART_CODE (extendable_part)) = NCODE_NEXT (fusionable_ncode);
            NCODE_NEXT (fusionable_ncode) = NPART_CODE (extendable_part);
        }
    }

    /*
     * Rename occurrences of NWITHID_VEC and NWITHID_IDS in N_NCODE of fitting_part
     * by NWITHID_VEC and NWITHID_IDS of fusionable WL
     */

    /* Create a new LUT */
    lut = GenerateLUT ();

    oldvec = NWITHID_VEC (NPART_WITHID (fitting_part));
    newvec = NWITHID_VEC (NPART_WITHID (extendable_part));

    InsertIntoLUT_S (lut, IDS_NAME (oldvec), IDS_NAME (newvec));
    InsertIntoLUT_P (lut, IDS_VARDEC (oldvec), IDS_VARDEC (newvec));
    InsertIntoLUT_P (lut, IDS_AVIS (oldvec), IDS_AVIS (newvec));

    oldids = NWITHID_IDS (NPART_WITHID (fitting_part));
    newids = NWITHID_IDS (NPART_WITHID (extendable_part));

    while (oldids != NULL) {
        InsertIntoLUT_S (lut, IDS_NAME (oldids), IDS_NAME (newids));
        InsertIntoLUT_P (lut, IDS_VARDEC (oldids), IDS_VARDEC (newids));
        InsertIntoLUT_P (lut, IDS_AVIS (oldids), IDS_AVIS (newids));

        oldids = IDS_NEXT (oldids);
        newids = IDS_NEXT (newids);
    }

    fitting_nncode = DupNodeLUTSSA (NPART_CODE (fitting_part), lut, fundef);

    fitting_assigns = BLOCK_INSTR (NCODE_CBLOCK (fitting_nncode));
    BLOCK_INSTR (NCODE_CBLOCK (fitting_nncode)) = MakeEmpty ();

    /*
     * vardec of N_blocks which are not the first N_block of
     * the current N_fundef are empty and points to NULL.
     * This holds especially for N_blocks of Withloops.
     */

    fitting_exprs = NCODE_CEXPRS (fitting_nncode);
    NCODE_CEXPRS (fitting_nncode) = MakeExprs (NULL, NULL);

    fitting_nncode = FreeNode (fitting_nncode);

    RemoveLUT (lut);

    /*
     * extent N_block of extentable_part's N_Ncode by instructions
     * of N_block of fitting_part
     */
    extendable_assigns = BLOCK_INSTR (NCODE_CBLOCK (NPART_CODE (extendable_part)));

    if (NODE_TYPE (fitting_assigns) != N_empty) {

        if ((NODE_TYPE (extendable_assigns) != N_empty) && both_contain_fold) {
            /*
             *   If both withloops contain fold operators fuse both
             *   accu functions if containing together
             */
            fitting_assigns = FuseAccu (extendable_assigns, fitting_assigns);
        }

        extendable_assigns = AppendAssign (extendable_assigns, fitting_assigns);
    }

    /*
     * extent N_exprs of extendable_part's N_Ncode by N_exprs of fitting_part
     */
    NCODE_CEXPRS (NPART_CODE (extendable_part))
      = AppendExprs (NCODE_CEXPRS (NPART_CODE (extendable_part)), fitting_exprs);

    DBUG_RETURN (extendable_part);
}

/** <!--********************************************************************-->
 *
 * @fn node *FuseWithloops( node *wl, info *arg_info, node *fusionable_assign)
 *
 *   @brief Fuses the two withloops together.
 *
 *   @param  node *wl                :  current N_Nwith node
 *           info *arg_info          :  N_INFO
 *           node *fusionable_assign :  N_assign node of the second withloop where the
 *                                      current withloop will be fused to
 *   @return node *                  :  current N_Nwith node
 ******************************************************************************/
static node *
FuseWithloops (node *wl, info *arg_info, node *fusionable_assign)
{
    node *fusionable_wl, *tmp_withop, *parts, *fitting_part;
    ids *tmp_ids;
    bool both_contain_fold;

    DBUG_ENTER ("FuseWithloops");

    /* N_Nwith where current withloop will be fused with */
    fusionable_wl = ASSIGN_RHS (fusionable_assign);

    /*
     * 1. extend LHS of fusionable_assign by LHS of WL assignment
     */
    tmp_ids = DupAllIds (ASSIGN_LHS (INFO_WLFS_ASSIGN (arg_info)));
    ASSIGN_LHS (fusionable_assign) = AppendIds (ASSIGN_LHS (fusionable_assign), tmp_ids);
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

    if (INFO_WLFS_FWO_CONTAINS_FOLD (arg_info) && INFO_WLFS_WO_CONTAINS_FOLD (arg_info))
        both_contain_fold = TRUE;
    else
        both_contain_fold = FALSE;

    parts = NWITH_PART (fusionable_wl);
    while (parts != NULL) {
        fitting_part = FindFittingPart (NPART_GEN (parts), NWITH_PART (wl));
        DBUG_ASSERT ((fitting_part != NULL), "no fitting N_Npart is available!");
        parts = FuseNCodes (parts, fitting_part, fusionable_wl, both_contain_fold,
                            INFO_WLFS_FUNDEF (arg_info));
        parts = NPART_NEXT (parts);
    }

    /*
     * 3. extent N_Nwithop(s) of fusionable_wl by N_Nwithop of 'wl'
     */
    tmp_withop = NWITH_WITHOP (fusionable_wl);
    while (NWITHOP_NEXT (tmp_withop) != NULL) {
        tmp_withop = NWITHOP_NEXT (tmp_withop);
    }
    NWITHOP_NEXT (tmp_withop) = DupTree (NWITH_WITHOP (wl));

    DBUG_RETURN (wl);
}

/** <!--********************************************************************-->
 *
 * @fn node *IntersectParts(node *parts1, node parts2)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_Nwith
 *           info *arg_info:  N_info
 *   @return node *        :  N_Nwith
 ******************************************************************************/
static nodelist *
IntersectParts (node *parts1, node *parts2)
{
    node *new_parts_1 = NULL, *new_parts_2 = NULL, *new_part_1, *new_part_2, *parts2_tmp,
         *genn, *lb_1, *ub_1, *lb_2, *ub_2, *array_lb, *array_ub, *new_array_lb,
         *new_array_ub, *lb_new, *ub_new;
    nodelist *ret = NULL;
    int dim, d, lb, ub;

    DBUG_ENTER ("IntersectParts");

    dim = ARRAY_VECLEN (NGEN_BOUND1 (NPART_GEN (parts1)));

    while (parts1 != NULL) {
        parts2_tmp = parts2;
        while (parts2_tmp != NULL) {
            lb_1 = ARRAY_AELEMS (NGEN_BOUND1 (NPART_GEN (parts1)));
            ub_1 = ARRAY_AELEMS (NGEN_BOUND2 (NPART_GEN (parts1)));
            array_lb = NGEN_BOUND1 (NPART_GEN (parts2_tmp));
            array_ub = NGEN_BOUND2 (NPART_GEN (parts2_tmp));
            lb_2 = ARRAY_AELEMS (array_lb);
            ub_2 = ARRAY_AELEMS (array_ub);

            new_array_lb = DupNode (array_lb);
            new_array_ub = DupNode (array_ub);
            lb_new = ARRAY_AELEMS (new_array_lb);
            ub_new = ARRAY_AELEMS (new_array_ub);

            for (d = 0; d < dim; d++) {
                lb = MAX (NUM_VAL (EXPRS_EXPR (lb_1)), NUM_VAL (EXPRS_EXPR (lb_2)));
                ub = MIN (NUM_VAL (EXPRS_EXPR (ub_1)), NUM_VAL (EXPRS_EXPR (ub_2)));
                if (lb > ub)
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

            if (d == dim) {
                /* non empty generator */
                genn
                  = MakeNGenerator (new_array_lb, new_array_ub, F_le, F_lt, NULL, NULL);

                if (new_parts_1) {
                    NPART_NEXT (new_part_1) = MakeNPart (DupNode (NPART_WITHID (parts1)),
                                                         genn, NPART_CODE (parts1));
                    ;
                    new_part_1 = NPART_NEXT (new_part_1);
                    NPART_NEXT (new_part_2)
                      = MakeNPart (DupNode (NPART_WITHID (parts2)), DupNode (genn),
                                   NPART_CODE (parts2));
                    new_part_2 = NPART_NEXT (new_part_2);
                } else {
                    new_part_1 = MakeNPart (DupNode (NPART_WITHID (parts1)), genn,
                                            NPART_CODE (parts1));
                    new_parts_1 = new_part_1;

                    new_part_2 = MakeNPart (DupNode (NPART_WITHID (parts2)),
                                            DupNode (genn), NPART_CODE (parts2));
                    new_parts_2 = new_part_2;
                }
            } else {
                lb_new = FreeNode (lb_new);
                ub_new = FreeNode (ub_new);
            }

            parts2_tmp = NPART_NEXT (parts2_tmp);
        }
        parts1 = NPART_NEXT (parts1);
    }

    ret = NodeListAppend (ret, new_parts_2, NULL);
    ret = NodeListAppend (ret, new_parts_1, NULL);

    DBUG_RETURN (ret);
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
    node *tmp;
    nodelist *ret, *ret_tmp;
    int number_parts = 0;
    bool successfull = FALSE;

    DBUG_ENTER ("BuildNewGens");

    ret = IntersectParts (NWITH_PART (fusionable_wl), NWITH_PART (current_wl));

    if (ret != NULL) {
        successfull = TRUE;
        ret_tmp = ret;

        tmp = NODELIST_NODE (ret_tmp);

        DBUG_ASSERT ((tmp != NULL), "Number of Parts is 0!!!");

        while (tmp != NULL) {
            number_parts++;
            tmp = NPART_NEXT (tmp);
        }

        NWITH_PART (fusionable_wl) = FreeTree (NWITH_PART (fusionable_wl));
        NWITH_PART (fusionable_wl) = NODELIST_NODE (ret_tmp);
        ;
        NWITH_PARTS (fusionable_wl) = number_parts;

        ret_tmp = NODELIST_NEXT (ret_tmp);

        NWITH_PART (current_wl) = FreeTree (NWITH_PART (current_wl));
        NWITH_PART (current_wl) = NODELIST_NODE (ret_tmp);
        NWITH_PARTS (current_wl) = number_parts;

        ret = NodeListFree (ret, TRUE);
    }

    DBUG_RETURN (successfull);
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

    DBUG_ENTER ("AskFusionOracle");

    wl = INFO_WLFS_WL (arg_info);
    fwl = ASSIGN_RHS (INFO_WLFS_FUSIONABLE_WL (arg_info));

    /* is the current WL independent from the fusionable WL? */
    if (!INFO_WLFS_WLDEPENDENT (arg_info)) {

        if (INFO_WLFS_GENPROPERTY (arg_info) == GEN_constant
            && !INFO_WLFS_WO_CONTAINS_FOLD (arg_info)
            && !INFO_WLFS_FWO_CONTAINS_FOLD (arg_info)
            && INFO_WLFS_NO_STEP_WIDTH (arg_info)) {

            /* is the size of both wl iteration spaces equal? */
            if (CheckIterationSpace (arg_info)) {
                /*
                 * build new generators
                 * -> INFO_WLFS_GENPROPERTY(arg_info) = GEN_equal
                 */
                is_build = BuildNewGens (wl, fwl);
                if (is_build) {
                    INFO_WLFS_GENPROPERTY (arg_info) = GEN_equal;
                }
            }
        }

        if ((INFO_WLFS_GENPROPERTY (arg_info) == GEN_equal
             || INFO_WLFS_GENPROPERTY (arg_info) == GEN_equal_var)
            && NWITH_PARTS (wl) == NWITH_PARTS (fwl)) {
            answer = TRUE;
        }

    } else {
        /*
         * The current genarray-withloop depends on the fusionable. Append it to
         * INFO_WLFS_REFERENCES_FUSIONABLE( arg_info)
         */
        INFO_WLFS_REFERENCES_FUSIONABLE (arg_info)
          = NodeListAppend (INFO_WLFS_REFERENCES_FUSIONABLE (arg_info),
                            INFO_WLFS_ASSIGN (arg_info), NULL);
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
    DBUG_ENTER ("WLFSfundef");

    INFO_WLFS_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node)) {
        FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);
    }

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
    node *fusionable_wl_tmp = NULL;
    nodelist *references_fusionable_tmp = NULL;
    node *assigns2shift = NULL;
    bool is_stacked = FALSE;
    bool fwo_contains_fold = FALSE;
    int fwl_array_type;
    constant *fwl_shape;

    DBUG_ENTER ("WLFSassign");

    INFO_WLFS_ASSIGN (arg_info) = arg_node;

    if (INFO_WLFS_FUSIONABLE_WL (arg_info) == NULL
        || ASSIGN_VISITED_WITH (arg_node) != INFO_WLFS_FUSIONABLE_WL (arg_info)) {
        /*
         * This is the first visit of this assign with the current fusionable WL,
         * or there is no fusionable WL for now.
         */

        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

        if (NODE_TYPE (ASSIGN_RHS (arg_node)) == N_Nwith) {

            if (INFO_WLFS_WLACTION (arg_info) == WL_fused) {
                /*
                 * The withloop on the rhs of current assignment is fused with
                 * another withloop, so the current one is obsolete and has to
                 * be freed.
                 */
                arg_node = FreeNode (arg_node);

                INFO_WLFS_FWO_CONTAINS_FOLD (arg_info)
                  = (INFO_WLFS_FWO_CONTAINS_FOLD (arg_info)
                     || INFO_WLFS_WO_CONTAINS_FOLD (arg_info));

                /*
                 * Now we have to traverse back to the other withloop
                 * to finish the tranformation
                 */
                INFO_WLFS_WLACTION (arg_info) = WL_travback;
                DBUG_RETURN (arg_node);
            } else if (INFO_WLFS_WLACTION (arg_info) == WL_2fuse) {

                /*
                 * The current WL is not fusionable with the one
                 * stored in INFO_WLFS_FUSIONABLE_WL( arg_info).
                 * We stack the current information and set
                 * INFO_WLFS_FUSIONABLE_WL( arg_info) on this WL for the further
                 * traversal.
                 */
                fusionable_wl_tmp = INFO_WLFS_FUSIONABLE_WL (arg_info);
                references_fusionable_tmp = INFO_WLFS_REFERENCES_FUSIONABLE (arg_info);
                fwo_contains_fold = INFO_WLFS_FWO_CONTAINS_FOLD (arg_info);
                fwl_array_type = INFO_WLFS_FWL_ARRAY_TYPE (arg_info);
                fwl_shape = INFO_WLFS_FWL_SHAPE (arg_info);
                is_stacked = TRUE;

                INFO_WLFS_FUSIONABLE_WL (arg_info) = arg_node;
                INFO_WLFS_REFERENCES_FUSIONABLE (arg_info)
                  = NodeListAppend (NULL, arg_node, NULL);
                INFO_WLFS_FWO_CONTAINS_FOLD (arg_info)
                  = INFO_WLFS_WO_CONTAINS_FOLD (arg_info);
                INFO_WLFS_FWL_ARRAY_TYPE (arg_info) = INFO_WLFS_WL_ARRAY_TYPE (arg_info);
                INFO_WLFS_FWL_SHAPE (arg_info) = INFO_WLFS_WL_SHAPE (arg_info);
                INFO_WLFS_WL_SHAPE (arg_info) = NULL;

                INFO_WLFS_WLACTION (arg_info) = WL_nothing;
            }
        }
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    do {
        if (INFO_WLFS_WLACTION (arg_info) == WL_travback) {
            /*
             * Traverse back mode
             * We traverse back to the fusionable wl, pick and append
             * all assignments to INFO_WLFS_ASSIGNS2SHIFT( arg_info) which
             * are tagged with this WL and hang them out of the sytax tree.
             * To avoid a later visit on already visited assigns, we mark this
             * assigns as visited with the current fusionable WL.
             *
             * When we reach the fusionable wl, the list of assignments had to
             * be shifted in front of the WL to obtain dependencies.
             *
             * Accordingly we carry on with the search of a next withloop to fuse.
             */

            if (ASSIGN_TAG (ASSIGN_NEXT (arg_node))
                == INFO_WLFS_FUSIONABLE_WL (arg_info)) {
                /*
                 * ASSIGN_TAG( ASSIGN_NEXT( arg_node)) can't be NULL,
                 * because in this mode INFO_WLFS_FUSIONABLE_WL( arg_info)
                 * can't be NULL
                 */

                assignn = ASSIGN_NEXT (arg_node);
                ASSIGN_NEXT (arg_node) = ASSIGN_NEXT (ASSIGN_NEXT (arg_node));

                /* to obtain the same order as in syntax tree */
                ASSIGN_NEXT (assignn) = INFO_WLFS_ASSIGNS2SHIFT (arg_info);
                INFO_WLFS_ASSIGNS2SHIFT (arg_info) = assignn;
            }

            if (INFO_WLFS_FUSIONABLE_WL (arg_info) == arg_node) {

                INFO_WLFS_WLACTION (arg_info) = WL_nothing;

                wlfs_expr++;

                if (INFO_WLFS_ASSIGNS2SHIFT (arg_info) != NULL) {

                    assigns2shift
                      = AppendAssign (assigns2shift, INFO_WLFS_ASSIGNS2SHIFT (arg_info));

                    INFO_WLFS_ASSIGNS2SHIFT (arg_info) = NULL;
                }

                if (ASSIGN_NEXT (arg_node) != NULL) {
                    ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
                }

            } else {
                ASSIGN_VISITED_WITH (arg_node) = INFO_WLFS_FUSIONABLE_WL (arg_info);
                break;
            }
        } else if (is_stacked) {
            /*
             * Backtraversal mode
             * If we reach a withloop, where we stack a fusionable withloop
             * we had to pop it and to traverse with it down again.
             */

            /* Pop and clean */
            INFO_WLFS_FUSIONABLE_WL (arg_info) = fusionable_wl_tmp;
            if (INFO_WLFS_REFERENCES_FUSIONABLE (arg_info) != NULL) {
                INFO_WLFS_REFERENCES_FUSIONABLE (arg_info)
                  = NodeListFree (INFO_WLFS_REFERENCES_FUSIONABLE (arg_info), TRUE);
            }
            INFO_WLFS_REFERENCES_FUSIONABLE (arg_info) = references_fusionable_tmp;
            INFO_WLFS_FWO_CONTAINS_FOLD (arg_info) = fwo_contains_fold;
            INFO_WLFS_FWL_ARRAY_TYPE (arg_info) = fwl_array_type;
            if (INFO_WLFS_FWL_SHAPE (arg_info) != NULL) {
                INFO_WLFS_FWL_SHAPE (arg_info)
                  = COFreeConstant (INFO_WLFS_FWL_SHAPE (arg_info));
            }
            INFO_WLFS_FWL_SHAPE (arg_info) = fwl_shape;
            is_stacked = FALSE;

            if (assigns2shift != NULL) {

                arg_node = AppendAssign (assigns2shift, arg_node);
                assigns2shift = NULL;
            }

            if (ASSIGN_NEXT (arg_node) != NULL) {
                ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
            }
        } else {
            /* cleaning up */
            ASSIGN_VISITED_WITH (arg_node) = NULL;
            ASSIGN_TAG (arg_node) = NULL;

            if (assigns2shift != NULL) {
                arg_node = AppendAssign (assigns2shift, arg_node);
                assigns2shift = NULL;
            }
            break;
        }

    } while (TRUE);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSap(node *arg_node, info *arg_info)
 *
 *   @brief traverse in the fundef of the ap node if special function
 *
 *   @param  node *arg_node:  N_ap
 *           info *arg_info:  info
 *   @return node *        :  N_ap
 ******************************************************************************/
node *
WLFSap (node *arg_node, info *arg_info)
{
    info *tmp;

    DBUG_ENTER ("WLFSap");

    if ((FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))) {
        /*
         * special functions must be traversed when they are used
         */
        if (AP_FUNDEF (arg_node) != INFO_WLFS_FUNDEF (arg_info)) {

            /* stack arg_info */
            tmp = arg_info;
            arg_info = MakeInfo ();

            AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), arg_info);

            arg_info = FreeInfo (arg_info);
            arg_info = tmp;
        }
    }

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSid(node *arg_node, info *arg_info)
 *
 *   @brief  Checks if this Id is contained in
 *             INFO_WLFS_REFERENCES_FUSIONABLE( arg_info).
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

    DBUG_ENTER ("WLFSid");

    if (INFO_WLFS_FUSIONABLE_WL (arg_info) != NULL) {
        /*
         * Normal mode
         * All assignments which are (indirect) dependend from fusionable withloop
         * are collected
         */
        is_dependent
          = CheckDependency (arg_node, INFO_WLFS_REFERENCES_FUSIONABLE (arg_info));
        if (is_dependent) {
            INFO_WLFS_REFERENCES_FUSIONABLE (arg_info)
              = NodeListAppend (INFO_WLFS_REFERENCES_FUSIONABLE (arg_info),
                                INFO_WLFS_ASSIGN (arg_info), NULL);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSNwith(node *arg_node, info *arg_info)
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
WLFSNwith (node *arg_node, info *arg_info)
{
    node *fwl;
    wl_action_t wl_action = WL_nothing;
    info *info_tmp;

    DBUG_ENTER ("WLFSNwith");

    /*
     * first traversal in CODEs
     * The CODEs have to be traversed as they may contain further (nested) WLs
     * and I want to modify bottom up.
     * Info node has to be stacked before traversal
     */

    info_tmp = arg_info;
    arg_info = MakeInfo ();

    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    arg_info = FreeInfo (arg_info);
    arg_info = info_tmp;

    /*
     * The second traversal into the N_CODEs ought to detect possible
     * dependencies from current withloop to the fusionable.
     * The result is stored in NWITH_WLDEPENDENT( arg_node)
     */
    if (INFO_WLFS_FUSIONABLE_WL (arg_info) != NULL) {

        NWITH_REFERENCES_FUSIONABLE (arg_node)
          = INFO_WLFS_REFERENCES_FUSIONABLE (arg_info);

        arg_node = DetectDependencies (arg_node);

        INFO_WLFS_WLDEPENDENT (arg_info) = NWITH_DEPENDENT (arg_node);
        NWITH_DEPENDENT (arg_node) = FALSE;
        NWITH_REFERENCES_FUSIONABLE (arg_node) = NULL;
    } else
        INFO_WLFS_WLDEPENDENT (arg_info) = FALSE;

    /*
     * If the generators of the current withloop build a full partition
     * the PARTS attribute carries a positive value. Only those withloops
     * are considered further. These are at least AKD withloops.
     * Futhermore we consider only WLs with non-empty iteration space
     */

    if (NWITH_PARTS (arg_node) >= 1
        && SHGetUnrLen (TYGetShape (AVIS_TYPE (IDS_AVIS (NWITH_VEC (arg_node))))) > 0) {

        INFO_WLFS_WL (arg_info) = arg_node; /* store the current node for later */

        /*
         * Now, we traverse the WITHOP sons for checking types
         */
        INFO_WLFS_WO_CONTAINS_FOLD (arg_info) = FALSE;
        INFO_WLFS_WL_ARRAY_TYPE (arg_info) = ARRAY_unknown;
        if (INFO_WLFS_WL_SHAPE (arg_info) != NULL) {
            INFO_WLFS_WL_SHAPE (arg_info)
              = COFreeConstant (INFO_WLFS_WL_SHAPE (arg_info));
        }
        INFO_WLFS_WOTYPE (arg_info) = WOT_unknown;

        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

        /*
         * traverse the N_PARTs.
         * one value is computed during this traversal:
         *
         *  INFO_WLFS_GENPROPERTY(arg_info) !!
         */
        INFO_WLFS_GENPROPERTY (arg_info) = GEN_equal;
        INFO_WLFS_NO_STEP_WIDTH (arg_info) = TRUE;

        DBUG_ASSERT ((NWITH_PART (arg_node) != NULL),
                     "NWITH_PARTS is >= 1 although no PART is available!");
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

        if (INFO_WLFS_WL_ARRAY_TYPE (arg_info) == ARRAY_unknown) {
            INFO_WLFS_WL_ARRAY_TYPE (arg_info) = ARRAY_aks;
        }

        if (INFO_WLFS_FUSIONABLE_WL (arg_info) != NULL) {

            wl_action = WL_2fuse;
            fwl = INFO_WLFS_FUSIONABLE_WL (arg_info);

            if (AskFusionOracle (arg_info)) {
                /*
                 * Tag all assignments the current withloop depends on.
                 * Later the tagged assignments !between! the two fused withloops
                 * had to be shifted in front of the result withloop.
                 */
                NWITH_FUSIONABLE_WL (arg_node) = fwl;

                arg_node = TagDependencies (arg_node);

                NWITH_FUSIONABLE_WL (arg_node) = NULL;

                /* fuse both withloops together */
                arg_node = FuseWithloops (arg_node, arg_info, fwl);
                wl_action = WL_fused;
            }
        } else {
            /*
             * this WL is the first fusionable WL
             */
            INFO_WLFS_FUSIONABLE_WL (arg_info) = INFO_WLFS_ASSIGN (arg_info);
            INFO_WLFS_REFERENCES_FUSIONABLE (arg_info)
              = NodeListAppend (INFO_WLFS_REFERENCES_FUSIONABLE (arg_info),
                                INFO_WLFS_ASSIGN (arg_info), NULL);
            INFO_WLFS_FWO_CONTAINS_FOLD (arg_info)
              = INFO_WLFS_WO_CONTAINS_FOLD (arg_info);
            INFO_WLFS_FWL_ARRAY_TYPE (arg_info) = INFO_WLFS_WL_ARRAY_TYPE (arg_info);
            INFO_WLFS_FWL_SHAPE (arg_info) = INFO_WLFS_WL_SHAPE (arg_info);
            INFO_WLFS_WL_SHAPE (arg_info) = NULL;
        }
    } else { /* #Parts <= 0 || -2 <= dim <= 0 */
        if ((INFO_WLFS_FUSIONABLE_WL (arg_info) != NULL)
            && (INFO_WLFS_WLDEPENDENT (arg_info))) {
            /*
             * The current withloop depends on the fusionable. Append it to
             * INFO_WLFS_REFERENCES_FUSIONABLE( arg_info)
             */
            INFO_WLFS_REFERENCES_FUSIONABLE (arg_info)
              = NodeListAppend (INFO_WLFS_REFERENCES_FUSIONABLE (arg_info),
                                INFO_WLFS_ASSIGN (arg_info), NULL);
        }
    }

    INFO_WLFS_WLACTION (arg_info) = wl_action;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSNwithop( node *arg_node, info *arg_info)
 *
 *   @brief checks the type of withloop operator.
 *
 *   @param  node *arg_node:  N_Nwithop
 *           info *arg_info:  N_info
 *   @return node *        :  N_Nwithop
 ******************************************************************************/

node *
WLFSNwithop (node *arg_node, info *arg_info)
{
    wo_type_t current_type = WOT_unknown;
    constant *const_expr;
    shape *new_shp, *shp;
    ntype *type;
    int iv_shape, i;

    DBUG_ENTER ("WLFSNwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        if (INFO_WLFS_WL_ARRAY_TYPE (arg_info) == ARRAY_unknown) {
            const_expr = COAST2Constant (NWITHOP_SHAPE (arg_node));
            if (const_expr != NULL) {
                INFO_WLFS_WL_ARRAY_TYPE (arg_info) = ARRAY_aks;
                INFO_WLFS_WL_SHAPE (arg_info) = const_expr;
            } else
                INFO_WLFS_WL_ARRAY_TYPE (arg_info) = ARRAY_akd;
        }

        if (INFO_WLFS_WOTYPE (arg_info) == WOT_unknown)
            current_type = WOT_gen_mod;
        else if (INFO_WLFS_WOTYPE (arg_info) == WOT_fold)
            current_type = WOT_gen_mod_fold;
        else
            current_type = INFO_WLFS_WOTYPE (arg_info);

        break;

    case WO_modarray:
        if (INFO_WLFS_WL_ARRAY_TYPE (arg_info) == ARRAY_unknown) {

            type = AVIS_TYPE (IDS_AVIS (ASSIGN_LHS (INFO_WLFS_ASSIGN (arg_info))));
            if (TYIsAKS (type)) {
                shp = TYGetShape (type);

                /* get shape of the index vector */
                iv_shape = IDS_SHAPE (NWITH_VEC (INFO_WLFS_WL (arg_info)), 0);
                DBUG_ASSERT ((iv_shape > 0), "shape of index vector has to be > 0!");

                if (SHGetDim (shp) != iv_shape) {
                    new_shp = SHMakeShape (iv_shape);

                    for (i = 0; i < iv_shape; i++) {
                        new_shp = SHSetExtent (new_shp, i, SHGetExtent (shp, i));
                    }
                    const_expr = COMakeConstantFromShape (new_shp);
                } else
                    const_expr = COMakeConstantFromShape (shp);

                INFO_WLFS_WL_ARRAY_TYPE (arg_info) = ARRAY_aks;
                INFO_WLFS_WL_SHAPE (arg_info) = const_expr;
            } else {
                /*
                 * nothing for now
                 * we try to get more information by upper bound of generators later
                 */
            }
        }

        if (INFO_WLFS_WOTYPE (arg_info) == WOT_unknown)
            current_type = WOT_gen_mod;
        else if (INFO_WLFS_WOTYPE (arg_info) == WOT_fold)
            current_type = WOT_gen_mod_fold;
        else
            current_type = INFO_WLFS_WOTYPE (arg_info);

        break;

    case WO_foldfun:
        /* here is no break missing */
    case WO_foldprf:
        if (INFO_WLFS_WOTYPE (arg_info) == WOT_unknown) {
            current_type = WOT_fold;
            INFO_WLFS_WO_CONTAINS_FOLD (arg_info) = TRUE;
        } else if (INFO_WLFS_WOTYPE (arg_info) == WOT_gen_mod)
            current_type = WOT_gen_mod_fold;
        else
            current_type = INFO_WLFS_WOTYPE (arg_info);
        break;

    default:
        DBUG_ASSERT ((0), "illegal NWITHOP_TYPE found!");
        break;
    }

    INFO_WLFS_WOTYPE (arg_info) = current_type;

    if (NWITHOP_NEXT (arg_node) != NULL) {
        NWITHOP_NEXT (arg_node) = Trav (NWITHOP_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSNpart(node *arg_node, info *arg_info)
 *
 *   @brief traverse only generator to check bounds, step and width
 *
 *   @param  node *arg_node:  N_Npart
 *           info *arg_info:  N_info
 *   @return node *        :  N_Npart
 ******************************************************************************/

node *
WLFSNpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLFSNpart");

    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

    if (NPART_NEXT (arg_node) != NULL) {
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSNgenerator(node *arg_node, info *arg_info)
 *
 *   @brief  bounds, step and width vectors are compared with bounds, step
 *           and width of all generators of the withloop out of
 *           INFO_WLFS_WL2FUSION( arg_info).

 *           Via INFO_WLFS_GENPROPERTY( arg_info) the status of the generator
 *           is returned. Possible values are :
 *           GEN_equal    : this generator is equal to one generator of the
 *                          comparative withloop!
 *           GEN_constant : this generator is not equal to any generator of the
 *                          comparative withloop, but all bounds, steps and
 *                          widths are constant!
 *           GEN_variable : this generator is not equal to any generator of the
 *                          comparative withloop and at least one bound, step
 *                          or width is variable
 *
 *   @param  node *arg_node:  N_Ngenerator
 *           info *arg_info:  N_info
 *   @return node *        :  N_Ngenerator
 ******************************************************************************/

node *
WLFSNgenerator (node *arg_node, info *arg_info)
{
    node *parts, *gen;
    gen_property_t gen_prob, gen_prob_ret;
    constant *const_expr, *max_shape, *tmpc;

    DBUG_ENTER ("WLFSNgenerator");

    if (INFO_WLFS_FUSIONABLE_WL (arg_info) != NULL) {

        if (INFO_WLFS_GENPROPERTY (arg_info) < GEN_variable) {
            parts = NWITH_PART (ASSIGN_RHS (INFO_WLFS_FUSIONABLE_WL (arg_info)));

            gen_prob = GEN_diffdim;
            while (parts != NULL) {
                gen = NPART_GEN (parts);
                gen_prob = CompGenSon (NGEN_BOUND1 (arg_node), NGEN_BOUND1 (gen));
                gen_prob_ret = CompGenSon (NGEN_BOUND2 (arg_node), NGEN_BOUND2 (gen));
                RESULT_GEN_PROP (gen_prob, gen_prob_ret);
                gen_prob_ret = CompGenSon (NGEN_STEP (arg_node), NGEN_STEP (gen));
                if (NGEN_STEP (arg_node) || NGEN_STEP (gen))
                    INFO_WLFS_NO_STEP_WIDTH (arg_info) = FALSE;
                RESULT_GEN_PROP (gen_prob, gen_prob_ret);
                gen_prob_ret = CompGenSon (NGEN_WIDTH (arg_node), NGEN_WIDTH (gen));
                RESULT_GEN_PROP (gen_prob, gen_prob_ret);

                if (gen_prob == GEN_equal || gen_prob == GEN_equal_var)
                    break;
                parts = NPART_NEXT (parts);
            }
            RESULT_GEN_PROP (INFO_WLFS_GENPROPERTY (arg_info), gen_prob);
        }
    }

    if (INFO_WLFS_WL_ARRAY_TYPE (arg_info) == ARRAY_unknown) {
        const_expr = COAST2Constant (NGEN_BOUND1 (arg_node));
        if (const_expr != NULL) {
            max_shape = INFO_WLFS_WL_SHAPE (arg_info);
            if (max_shape != NULL) {
                tmpc = COGe (const_expr, max_shape);
                if (COIsTrue (tmpc, TRUE)) {
                    INFO_WLFS_WL_SHAPE (arg_info)
                      = COFreeConstant (INFO_WLFS_WL_SHAPE (arg_info));
                    INFO_WLFS_WL_SHAPE (arg_info) = const_expr;
                }
                tmpc = COFreeConstant (tmpc);
            } else
                INFO_WLFS_WL_SHAPE (arg_info) = const_expr;
        } else {
            INFO_WLFS_WL_ARRAY_TYPE (arg_info) = ARRAY_akd;
            if (INFO_WLFS_WL_SHAPE (arg_info) != NULL)
                INFO_WLFS_WL_SHAPE (arg_info)
                  = COFreeConstant (INFO_WLFS_WL_SHAPE (arg_info));
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WithloopFusion( node *arg_node)
 *
 *   @brief  Starting point for the withloop fusion if it was called
 *           from optimize.
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/

node *
WithloopFusion (node *arg_node)
{
    funtab *tmp_tab;
    info *arg_info;

    DBUG_ENTER ("WithloopFusion");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "WithloopFusion not started with fundef node");

    DBUG_PRINT ("WLFS", ("starting WithloopFusion"));

    arg_info = MakeInfo ();

    tmp_tab = act_tab;
    act_tab = wlfs_tab;

    arg_node = Trav (arg_node, arg_info);

    arg_info = FreeInfo (arg_info);
    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}
