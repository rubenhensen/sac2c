/*
 *
 * $Log$
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

/* usage of arg_info: */
#define INFO_WLFS_WL(n) (n->node[0])
#define INFO_WLFS_FUNDEF(n) (n->node[1])
#define INFO_WLFS_ASSIGN(n) (n->node[2])
#define INFO_WLFS_LET(n) (n->node[3])
#define INFO_WLFS_WLACTION(n) (n->flag)
#define INFO_WLFS_GENAREEQUAL(n) ((bool)(n->varno))
#define INFO_WLFS_INSIDEWL(n) ((bool)(n->varno))
#define INFO_WLFS_WOTYPE(n) (n->counter)
#define INFO_WLFS_DCTDEPENDENCIES(n) ((bool)(n->int_data))
#define INFO_WLFS_TAGDEPENDENCIES(n) ((bool)(n->lineno))
#define INFO_WLFS_WLDEPENDENT(n) ((bool)(n->refcnt))
#define INFO_WLFS_WL2EXTEND(n) (n->dfmask[0])
#define INFO_WLFS_SONSOFWL2EXTEND(n) ((nodelist *)(n->dfmask[1]))
#define INFO_WLFS_ASSIGNS2SHIFT(n) (n->dfmask[2])

/* Macros for N_assign: */
#define WLFS_TRAVWITHWL2EXTEND(n) (n->dfmask[0])
#define WLFS_TAGGEDWITHWL(n) (n->dfmask[1])

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
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

typedef enum { WO_gen, WO_mod, WO_fold } wo_type_t;

typedef enum { WL_fused, WL_2fuse, WL_travback, WL_nothing } wl_action_t;

/** <!--********************************************************************-->
 *
 * @fn bool CheckDependency( node *checkid, nodelist *nl);
 *
 *   @brief checks whether checkid is contained in LHS of the assignments
 *          stored in nl.
 *
 *   @param  node *checkid :  N_id
 *           nodelist *nl  :  contains assignments which depends (indirect)
 *                            on extendable withloop
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
 * @fn bool CheckArraySize( node *current_wl_assign, node *test_wl_assign)
 *
 *   @brief checks whether the size of both withloops is equal.
 *          For this version it is enough to check the first idenfifiers
 *          of both withloop.
 *
 *   @param  node *current_wl_assign:  N_assign node of current withloop
 *           node *test_wl_assign   :  N_assign node of extendable withloop
 *   @return bool                   :  returns TRUE iff they have equal size
 ******************************************************************************/
static bool
CheckArraySize (node *current_wl_assign, node *test_wl_assign)
{
    shpseg *shape1, *shape2;
    int dim1, dim2;
    bool is_equal;

    DBUG_ENTER ("CheckArraySize");

    shape1 = Type2Shpseg (IDS_TYPE (LET_IDS (ASSIGN_INSTR (current_wl_assign))), &dim1);

    shape2 = Type2Shpseg (IDS_TYPE (LET_IDS (ASSIGN_INSTR (test_wl_assign))), &dim2);

    if (dim1 == dim2) {
        if (dim1 > 0) {
            is_equal = EqualShpseg (dim1, shape2, shape1);
        } else
            is_equal = TRUE;
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
 *   @return bool           :  returns TRUE iff both are equal.
 ******************************************************************************/
static bool
CompGenSon (node *gen_son1, node *gen_son2)
{
    node *elems1, *elems2;
    bool is_equal;

    DBUG_ENTER ("CompGenSon");

    if ((gen_son1 == NULL) && (gen_son2 == NULL))
        is_equal = TRUE;
    else {
        DBUG_ASSERT (((NODE_TYPE (gen_son1) == N_array)
                      && (NODE_TYPE (gen_son2) == N_array)),
                     "CompGenSon not called with N_arrays");
        elems1 = ARRAY_AELEMS (gen_son1);
        elems2 = ARRAY_AELEMS (gen_son2);

        while (elems1 && elems2) {
            if ((NODE_TYPE (EXPRS_EXPR (elems1)) == N_num)
                && (NODE_TYPE (EXPRS_EXPR (elems2)) == N_num)) {
                if ((NUM_VAL (EXPRS_EXPR (elems1)) == NUM_VAL (EXPRS_EXPR (elems2))))
                    is_equal = TRUE;
                else {
                    is_equal = FALSE;
                    break;
                }
            } else if ((NODE_TYPE (EXPRS_EXPR (elems1)) == N_id)
                       && (NODE_TYPE (EXPRS_EXPR (elems2)) == N_id)) {
                if (!strcmp (ID_NAME (EXPRS_EXPR (elems1)),
                             ID_NAME (EXPRS_EXPR (elems2))))
                    is_equal = TRUE;
                else {
                    is_equal = FALSE;
                    break;
                }
            } else {
                is_equal = FALSE;
                break;
            }

            elems1 = EXPRS_NEXT (elems1);
            elems2 = EXPRS_NEXT (elems2);
        }

        if ((elems1 || elems2) && is_equal == TRUE) {
            is_equal = FALSE;
        }
    }

    DBUG_RETURN (is_equal);
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
    bool is_equal;

    DBUG_ENTER ("FindFittingPart");

    while (parts != NULL) {
        gen = NPART_GEN (parts);
        is_equal = CompGenSon (NGEN_BOUND1 (pattern), NGEN_BOUND1 (gen));
        is_equal = is_equal && CompGenSon (NGEN_BOUND2 (pattern), NGEN_BOUND2 (gen));
        is_equal = is_equal && CompGenSon (NGEN_STEP (pattern), NGEN_STEP (gen));
        is_equal = is_equal && CompGenSon (NGEN_WIDTH (pattern), NGEN_WIDTH (gen));
        if (is_equal) {
            tmp_part = parts;
            break;
        }
        parts = NPART_NEXT (parts);
    }

    DBUG_RETURN (tmp_part);
}

/** <!--********************************************************************-->
 *
 * @fn node *DeleteFittingPart(node *fit_part, node *wl)
 *
 *   @brief deletes N_Npart fit_part out of N_Nwith.
 *
 *   @param  node *fit_part  : N_Npart
 *           node *parts    :  N_Nwith
 *   @return node *         :  modified N_Nwith
 ******************************************************************************/
static node *
DeleteFittingPart (node *fit_part, node *wl)
{
    node *tmp_part = NULL;

    DBUG_ENTER ("DeleteFittingPart");

    if (fit_part == NWITH_PART (wl)) {
        NWITH_PART (wl) = FreeNode (NWITH_PART (wl));
    } else {
        tmp_part = NWITH_PART (wl);

        while (NPART_NEXT (tmp_part) != fit_part) {
            tmp_part = NPART_NEXT (tmp_part);
        }
        NPART_NEXT (tmp_part) = FreeNode (NPART_NEXT (tmp_part));
    }

    DBUG_RETURN (wl);
}

/** <!--********************************************************************-->
 *
 * @fn node *FuseNCodes( node *extend_part, node *fit_part,
 *                       bool unique_ref, node *wl2extend, node *fundef)
 *
 *   @brief inserts the whole assignment block of 'fit_part' in
 *          assignment block of 'extent_part'. If unique_ref is false, the
 *          N_Ncode of 'extent_part' is referenced several, so that it has
 *          to be deepcopied before extent.
 *
 *   @param  node *extend_part  : N_Npart
 *           node *fit_part     : N_Npart
 *           bool unique_ref    : TRUE iff N_Ncode of extent_part is unique
 *                                referenced
 *           node *wl2extend    : N_Nwith current WL will be fused with
 *           node *fundef       : N_fundef
 *   @return node               : modified extent_part
 ******************************************************************************/
static node *
FuseNCodes (node *extend_part, node *fit_part, bool unique_ref, node *wl2extend,
            node *fundef)
{
    node *wl2extend_ncode, *fit_nncode, *extend_nblock, *fit_assigns, *fit_exprs;
    LUT_t lut;
    ids *oldvec, *newvec, *oldids, *newids;

    DBUG_ENTER ("FuseNCodes");

    if (!unique_ref) {
        wl2extend_ncode = NWITH_CODE (wl2extend);

        if (wl2extend_ncode == NPART_CODE (extend_part)) {
            NCODE_USED (wl2extend_ncode)--;
            NPART_CODE (extend_part) = DupNodeSSA (NPART_CODE (extend_part), fundef);
            NCODE_USED (NPART_CODE (extend_part)) = 1;
            NWITH_CODE (wl2extend) = NPART_CODE (extend_part);
            NCODE_NEXT (NPART_CODE (extend_part)) = wl2extend_ncode;
        } else {
            while (NCODE_NEXT (wl2extend_ncode) != NPART_CODE (extend_part)) {
                wl2extend_ncode = NCODE_NEXT (wl2extend_ncode);
            }
            NCODE_USED (NCODE_NEXT (wl2extend_ncode))--;
            NPART_CODE (extend_part) = DupNodeSSA (NPART_CODE (extend_part), fundef);
            NCODE_USED (NPART_CODE (extend_part)) = 1;
            NCODE_NEXT (NPART_CODE (extend_part)) = NCODE_NEXT (wl2extend_ncode);
            NCODE_NEXT (wl2extend_ncode) = NPART_CODE (extend_part);
        }
    }

    /*
     * Rename occurrences of NWITHID_VEC and NWITHID_IDS in N_NCODE of fit_part
     * by NWITHID_VEC and NWITHID_IDS of extendable WL
     */

    /* Create a new LUT */
    lut = GenerateLUT ();

    oldvec = NWITHID_VEC (NPART_WITHID (fit_part));
    newvec = NWITHID_VEC (NPART_WITHID (extend_part));

    InsertIntoLUT_S (lut, IDS_NAME (oldvec), IDS_NAME (newvec));
    InsertIntoLUT_P (lut, IDS_VARDEC (oldvec), IDS_VARDEC (newvec));
    InsertIntoLUT_P (lut, IDS_AVIS (oldvec), IDS_AVIS (newvec));

    oldids = NWITHID_IDS (NPART_WITHID (fit_part));
    newids = NWITHID_IDS (NPART_WITHID (extend_part));

    while (oldids != NULL) {
        InsertIntoLUT_S (lut, IDS_NAME (oldids), IDS_NAME (newids));
        InsertIntoLUT_P (lut, IDS_VARDEC (oldids), IDS_VARDEC (newids));
        InsertIntoLUT_P (lut, IDS_AVIS (oldids), IDS_AVIS (newids));

        oldids = IDS_NEXT (oldids);
        newids = IDS_NEXT (newids);
    }

    fit_nncode = DupNodeLUTSSA (NPART_CODE (fit_part), lut, fundef);

    fit_assigns = BLOCK_INSTR (NCODE_CBLOCK (fit_nncode));
    BLOCK_INSTR (NCODE_CBLOCK (fit_nncode)) = MakeEmpty ();

    /*
     * vardec of N_blocks which are not the first N_block of
     * the current N_fundef are empty and points to NULL.
     * This holds especially for N_blocks of Withloops.
     */

    fit_exprs = NCODE_CEXPRS (fit_nncode);
    NCODE_CEXPRS (fit_nncode) = MakeExprs (NULL, NULL);

    fit_nncode = FreeNode (fit_nncode);

    RemoveLUT (lut);

    /*
     * extent N_block of extent_part's N_Ncode by instructions
     * of N_block of fit_part
     */
    extend_nblock = NCODE_CBLOCK (NPART_CODE (extend_part));

    if (NODE_TYPE (fit_assigns) != N_empty) {
        BLOCK_INSTR (extend_nblock)
          = AppendAssign (BLOCK_INSTR (extend_nblock), fit_assigns);
    }

    /*
     * extent N_exprs of extend_part's N_Ncode by N_exprs of fit_part
     */
    NCODE_CEXPRS (NPART_CODE (extend_part))
      = AppendExprs (NCODE_CEXPRS (NPART_CODE (extend_part)), fit_exprs);

    DBUG_RETURN (extend_part);
}

/** <!--********************************************************************-->
 *
 * @fn node *FuseWithloops( node *wl, node *arg_info, node *wl2extend_assign)
 *
 *   @brief Fuses the two withloops together. Important for dependencies
 *          in code the current withloop has to fuse with the other in such
 *          way the current WL can be removed later.
 *
 *   @param  node *wl               :  current N_Nwith node
 *           node *arg_info         :  N_INFO
 *           node *wl2extend_assign :  N_assign node of the second withloop where the
 *                                     current withloop will be fused to
 *   @return node *                 :  modified current N_Nwith node for DCR
 ******************************************************************************/
static node *
FuseWithloops (node *wl, node *arg_info, node *wl2extend_assign)
{
    node *wl2extend, *tmp_withop, *parts, *fitting_part;
    ids *tmp_ids;
    bool unique_ref;

    DBUG_ENTER ("FuseWithloops");

    /* N_Nwith where current withloop will be fused with */
    wl2extend = ASSIGN_RHS (wl2extend_assign);

    /*
     * 1. extend LHS of wl2extend_assign by LHS of WL assignment
     */
    tmp_ids = ASSIGN_LHS (wl2extend_assign);
    while (IDS_NEXT (tmp_ids) != NULL) {
        tmp_ids = IDS_NEXT (tmp_ids);
    }
    IDS_NEXT (tmp_ids) = DupAllIds (ASSIGN_LHS (INFO_WLFS_ASSIGN (arg_info)));
    tmp_ids = IDS_NEXT (tmp_ids);
    while (tmp_ids != NULL) {
        /* set correct backref to defining assignment */
        AVIS_SSAASSIGN (IDS_AVIS (tmp_ids)) = wl2extend_assign;
        tmp_ids = IDS_NEXT (tmp_ids);
    }

    /*
     * 2. extend each N_Npart's N_Ncode of the withloop belonging to
     *    'wl2extend_assign' by whole assignment block of N_Npart's N_Ncode
     *    of 'wl' if both N_Ngenerators
     *    are equal. WARNING: N_Ncode can be referenced several times.
     */
    parts = NWITH_PART (wl2extend);
    while (parts != NULL) {
        fitting_part = FindFittingPart (NPART_GEN (parts), NWITH_PART (wl));
        DBUG_ASSERT ((fitting_part != NULL), "no fittig N_Npart is available!");
        unique_ref = NCODE_USED (NPART_CODE (parts)) == 1;
        parts = FuseNCodes (parts, fitting_part, unique_ref, wl2extend,
                            INFO_WLFS_FUNDEF (arg_info));
        /* fitting_part is obsolete now */
        wl = DeleteFittingPart (fitting_part, wl);
        parts = NPART_NEXT (parts);
    }

    /*
     * 3. extent N_Nwithop(s) of wl2extend by N_Nwithop of 'wl'
     */
    tmp_withop = NWITH_WITHOP (wl2extend);
    while (NWITHOP_NEXT (tmp_withop) != NULL) {
        tmp_withop = NWITHOP_NEXT (tmp_withop);
    }
    NWITHOP_NEXT (tmp_withop) = DupTree (NWITH_WITHOP (wl));

    /*
     * 4. set attribute INFO_WLFS_WLACTION( arg_info) = WL_fused, so that
     *    the current WL will be removed later.
     */

    INFO_WLFS_WLACTION (arg_info) = WL_fused;

    DBUG_RETURN (wl);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSfundef(node *arg_node, node *arg_info)
 *
 *   @brief starts the traversal of the given fundef
 *
 *   @param  node *arg_node:  N_fundef
 *           node *arg_info:  N_info
 *   @return node *        :  N_fundef
 ******************************************************************************/

node *
WLFSfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFSfundef");

    INFO_WLFS_WL (arg_info) = NULL;
    INFO_WLFS_FUNDEF (arg_info) = arg_node;
    INFO_WLFS_WLACTION (arg_info) = WL_nothing;
    INFO_WLFS_DCTDEPENDENCIES (arg_info) = FALSE;
    INFO_WLFS_TAGDEPENDENCIES (arg_info) = FALSE;
    INFO_WLFS_WLDEPENDENT (arg_info) = FALSE;
    INFO_WLFS_INSIDEWL (arg_info) = FALSE;
    INFO_WLFS_WL2EXTEND (arg_info) = NULL;
    INFO_WLFS_SONSOFWL2EXTEND (arg_info) = NULL;
    INFO_WLFS_ASSIGNS2SHIFT (arg_info) = NULL;

    if (FUNDEF_BODY (arg_node)) {
        FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSassign(node *arg_node, node *arg_info)
 *
 *   @brief store actual assign node in arg_info and traverse instruction
 *
 *   @param  node *arg_node:  N_assign
 *           node *arg_info:  N_info
 *   @return node *        :  N_assign
 ******************************************************************************/

node *
WLFSassign (node *arg_node, node *arg_info)
{
    node *iterator, *assignn;
    node *wl2extend_tmp = NULL;
    nodelist *sonsofwl2extend_tmp = NULL;
    bool is_stacked = FALSE;

    DBUG_ENTER ("WLFSassign");

    INFO_WLFS_ASSIGN (arg_info) = arg_node;

    if (INFO_WLFS_DCTDEPENDENCIES (arg_info)) {
        /*
         * Detect dependencies mode
         * We are inside N_code of a withloop and we check if the current withloop
         * depends on the extendable withloop.
         * If TRUE, the withloop is dependent and no more traversal is necessary.
         */
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        if (INFO_WLFS_WLDEPENDENT (arg_info))
            DBUG_RETURN (arg_node);
    } else if (INFO_WLFS_TAGDEPENDENCIES (arg_info)) {
        /*
         * Tag dependencies mode
         * Finds all assignments the fused withloop depends on. This assignments
         * had to be tagged. Also already visited assignments had to be tagged,
         * to avoid reiterations. To avoid confusions the assignments are tagged
         * with the pointer on the current extendable withloop.
         */

        WLFS_TAGGEDWITHWL (arg_node) = INFO_WLFS_WL2EXTEND (arg_info);

        if (INFO_WLFS_INSIDEWL (arg_info)) {
            /*
             * We are inside a WL and have to traverse the instructions and
             * the next assignment
             */

            ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

            if (ASSIGN_NEXT (arg_node) != NULL) {
                ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
            }
        } else if (NODE_TYPE (ASSIGN_RHS (arg_node)) == N_Nwith) {
            /*
             * We are not inside a WL but we traverse in one, so we had to
             * set INFO_WLFS_INSIDEWL( arg_info) TRUE and traverse in the
             * instruction
             */
            INFO_WLFS_INSIDEWL (arg_info) = TRUE;
            ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        } else {
            /*
             * We are not inside a WL and we traverse in none, so we only had to
             * traverse in the instruction.
             */
            ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        }
    } else if (INFO_WLFS_WL2EXTEND (arg_info) == NULL
               || WLFS_TRAVWITHWL2EXTEND (arg_node) != INFO_WLFS_WL2EXTEND (arg_info)) {
        /*
         * Normal mode
         * This is the first visit of this assign with the current extendable WL,
         * or there is no extedable WL for now.
         * To avoid a second, we set WLFS_TRAVWITHWL2EXTEND( arg_node)
         * on the current extendable WL.
         */
        WLFS_TRAVWITHWL2EXTEND (arg_node) = INFO_WLFS_WL2EXTEND (arg_info);

        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

        if (NODE_TYPE (ASSIGN_RHS (arg_node)) == N_Nwith) {

            if (INFO_WLFS_WLACTION (arg_info) == WL_fused) {
                /*
                 * The withloop on the rhs of current assignment is fused with
                 * another withloop, so the current one is obsolete and has to
                 * be freed.
                 */
                arg_node = FreeNode (arg_node);
                /*
                 * Now we have to traverse back to the other withloop
                 * to finish the tranformation
                 */
                INFO_WLFS_WLACTION (arg_info) = WL_travback;
                DBUG_RETURN (arg_node);
            } else if (INFO_WLFS_WLACTION (arg_info) == WL_2fuse) {
                /*
                 * The current genarray-withloop is not fuseable with the WL
                 * stored in INFO_WLFS_WL2EXTEND( arg_info).
                 * We stack the current information and set
                 * INFO_WLFS_WL2EXTEND( arg_info) on this WL for the further
                 * traversal.
                 */
                wl2extend_tmp = INFO_WLFS_WL2EXTEND (arg_info);
                sonsofwl2extend_tmp = INFO_WLFS_SONSOFWL2EXTEND (arg_info);
                INFO_WLFS_WL2EXTEND (arg_info) = arg_node;
                INFO_WLFS_SONSOFWL2EXTEND (arg_info)
                  = NodeListAppend (NULL, arg_node, NULL);
                is_stacked = TRUE;
                INFO_WLFS_WLACTION (arg_info) = WL_nothing;
            }
        }
    }

    if (!INFO_WLFS_TAGDEPENDENCIES (arg_info)) {
        /*
         * In tag dependencies mode
         * traversal to next assignment is only permitted in some cases above
         */
        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    do {

        if (INFO_WLFS_WLACTION (arg_info) == WL_travback) {
            /*
             * Traverse back mode
             * We traverse back to the current extendable wl, pick and append
             * all assignments to INFO_WLFS_ASSIGNS2SHIFT( arg_info) which
             * are tagged with this WL and hang them out of the sytax tree.
             *
             * When we reach the extendable wl, the list of assignments had to
             * be shifted in front of the WL to obtain dependencies.
             *
             * Accordingly we carry on with the search of a next withloop to fuse.
             */

            if (WLFS_TAGGEDWITHWL (ASSIGN_NEXT (arg_node))
                == INFO_WLFS_WL2EXTEND (arg_info)) {
                /*
                 * WLFS_TAGGEDWITHWL( ASSIGN_NEXT( arg_node)) can't be NULL,
                 * because in this mode INFO_WLFS_WL2EXTEND( arg_info)
                 * can't be NULL
                 */

                assignn = ASSIGN_NEXT (arg_node);
                ASSIGN_NEXT (arg_node) = ASSIGN_NEXT (ASSIGN_NEXT (arg_node));

                /* to obtain the same order as in syntax tree */
                ASSIGN_NEXT (assignn) = INFO_WLFS_ASSIGNS2SHIFT (arg_info);
                INFO_WLFS_ASSIGNS2SHIFT (arg_info) = assignn;
            }

            if (INFO_WLFS_WL2EXTEND (arg_info) == arg_node) {

                INFO_WLFS_WLACTION (arg_info) = WL_nothing;

                wlfs_expr++;

                if (INFO_WLFS_ASSIGNS2SHIFT (arg_info) != NULL) {
                    iterator = INFO_WLFS_ASSIGNS2SHIFT (arg_info);
                    while (ASSIGN_NEXT (iterator)) {
                        iterator = ASSIGN_NEXT (iterator);
                    }
                    ASSIGN_NEXT (iterator) = arg_node;
                    /* to traverse not in circle */
                    iterator = ASSIGN_NEXT (iterator);

                    arg_node = INFO_WLFS_ASSIGNS2SHIFT (arg_info);
                    INFO_WLFS_ASSIGNS2SHIFT (arg_info) = NULL;

                    if (ASSIGN_NEXT (iterator) != NULL) {
                        ASSIGN_NEXT (iterator) = Trav (ASSIGN_NEXT (iterator), arg_info);
                    }

                } else if (ASSIGN_NEXT (arg_node) != NULL) {
                    ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
                }

            } else
                DBUG_RETURN (arg_node);

        } else if ((INFO_WLFS_WLACTION (arg_info) == WL_nothing) && is_stacked) {
            /*
             * Backtraversal mode
             * If we reach a withloop, where we stack a extendable withloop
             * we had to pop it and to traverse down again.
             */

            /* Pop and clean */
            INFO_WLFS_WL2EXTEND (arg_info) = wl2extend_tmp;
            if (INFO_WLFS_SONSOFWL2EXTEND (arg_info) != NULL) {
                INFO_WLFS_SONSOFWL2EXTEND (arg_info)
                  = NodeListFree (INFO_WLFS_SONSOFWL2EXTEND (arg_info), TRUE);
            }
            INFO_WLFS_SONSOFWL2EXTEND (arg_info) = sonsofwl2extend_tmp;
            is_stacked = FALSE;

            if (ASSIGN_NEXT (arg_node) != NULL) {
                ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
            }
        } else
            DBUG_RETURN (arg_node);

    } while (TRUE);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSid(node *arg_node, node *arg_info)
 *
 *   @brief  Checks if this Id is contained in
 *             INFO_WLFS_SONSOFWL2EXTEND( arg_info).
 *           If it is contained the current assigment is (indirect)
 *           dependent from the extendable withloop.
 *
 *   @param  node *arg_node:  N_id
 *           node *arg_info:  N_info
 *   @return node *        :  N_id
 ******************************************************************************/
node *
WLFSid (node *arg_node, node *arg_info)
{
    node *assignn;
    bool is_dependent, insidewl_tmp;

    DBUG_ENTER ("WLFSid");

    if (INFO_WLFS_DCTDEPENDENCIES (arg_info)) {
        /*
         * Detect dependencies mode
         * We are inside N_code of a withloop and we check if the current withloop
         * depends on the extendable withloop.
         */
        is_dependent = CheckDependency (arg_node, INFO_WLFS_SONSOFWL2EXTEND (arg_info));
        INFO_WLFS_WLDEPENDENT (arg_info)
          = (INFO_WLFS_WLDEPENDENT (arg_info) || is_dependent);
    } else if (INFO_WLFS_TAGDEPENDENCIES (arg_info)) {
        /*
         * Tag dependencies mode
         * Finds all assignments the fused withloop depends on. This assignments
         * had to be tagged. Also already visited assignments had to be tagged,
         * to avoid reiterations. To avoid confusions the assignments are tagged
         * with the pointer on the current extendable withloop. Assignments which are
         * tagged with the current extendable withloop were already traversed. We only
         * consider untagged assignments.
         */

        /* get the definition assignment via the AVIS_SSAASSIGN backreference */
        assignn = AVIS_SSAASSIGN (ID_AVIS (arg_node));

        /* is there a definition assignment? */
        if (assignn != NULL) {
            /* is the assignment already visited */
            if (WLFS_TAGGEDWITHWL (assignn) != INFO_WLFS_WL2EXTEND (arg_info)) {
                /* stack INFO_WLFS_INSIDEWL( arg_info) */
                insidewl_tmp = INFO_WLFS_INSIDEWL (arg_info);
                INFO_WLFS_INSIDEWL (arg_info) = FALSE;
                assignn = Trav (assignn, arg_info);
                INFO_WLFS_INSIDEWL (arg_info) = insidewl_tmp;
            }
        }

    } else if (INFO_WLFS_WL2EXTEND (arg_info) != NULL) {
        /*
         * Normal mode
         * All assignments which are (indirect) dependend from extendable withloop
         * are collected
         */
        is_dependent = CheckDependency (arg_node, INFO_WLFS_SONSOFWL2EXTEND (arg_info));
        if (is_dependent) {
            INFO_WLFS_SONSOFWL2EXTEND (arg_info)
              = NodeListAppend (INFO_WLFS_SONSOFWL2EXTEND (arg_info),
                                INFO_WLFS_ASSIGN (arg_info), NULL);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSNwith(node *arg_node, node *arg_info)
 *
 *   @brief  start traversal of this WL and store information in new arg_info
 *           node. The only N_Npart node (inclusive body) is traversed.
 *           Afterwards, if certain conditions are fulfilled,
 *           the WL is transformed into a WL with generators describing a full
 *           partition.
 *
 *   @param  node *arg_node:  N_Nwith
 *           node *arg_info:  N_info
 *   @return node *        :  N_Nwith
 ******************************************************************************/

node *
WLFSNwith (node *arg_node, node *arg_info)
{
    node *assign_tmp, *wl2extend_tmp, *wln;
    nodelist *sonsofwl2extend_tmp;
    bool wldependent_tmp;
    bool is_equal = TRUE;
    wl_action_t wl_action = WL_nothing;

    DBUG_ENTER ("WLFSNwith");

    if (INFO_WLFS_DCTDEPENDENCIES (arg_info) || INFO_WLFS_TAGDEPENDENCIES (arg_info)) {
        /*
         * Detect dependencies mode
         * We are inside N_code of a withloop and we check if the this withloop
         * depends on the extendable withloop.
         *
         * Tag dependencies mode
         * Finds all assignments the fused withloop depends on. This assignments
         * had to be tagged.
         *
         * To find dependent assignments we had to traverse
         * into N_CODE and N_WITHOP.
         */

        if (NWITH_CODE (arg_node) != NULL) {
            NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
        }
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    } else {
        /*
         * Information about last N_assign had to be stacked
         * before traversal
         */
        assign_tmp = INFO_WLFS_ASSIGN (arg_info);

        /*
         * The first traversal into the N_CODEs ought to dectect possible
         * dependencies from current withloop to the extendable.
         * The result is stored in INFO_WLFS_WLDEPENDENT( arg_info)
         */
        if (INFO_WLFS_WL2EXTEND (arg_info) != NULL) {

            INFO_WLFS_DCTDEPENDENCIES (arg_info) = TRUE;
            INFO_WLFS_WLDEPENDENT (arg_info) = FALSE;
            if (NWITH_CODE (arg_node) != NULL) {
                NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
            }
            INFO_WLFS_DCTDEPENDENCIES (arg_info) = FALSE;
        }

        /*
         * Now normal traversal
         * Information about wl2extend, sonsofwl2extend and wldependent had to be
         * stacked before traversal
         */
        wl2extend_tmp = INFO_WLFS_WL2EXTEND (arg_info);
        sonsofwl2extend_tmp = INFO_WLFS_SONSOFWL2EXTEND (arg_info);
        wldependent_tmp = INFO_WLFS_WLDEPENDENT (arg_info);

        /* The nested WLs are only local */
        INFO_WLFS_WL2EXTEND (arg_info) = NULL;
        INFO_WLFS_SONSOFWL2EXTEND (arg_info) = NULL;
        INFO_WLFS_WLDEPENDENT (arg_info) = FALSE;

        /*
         * The CODEs have to be traversed as they may contain further (nested) WLs
         * and I want to modify bottom up.
         */
        if (NWITH_CODE (arg_node) != NULL) {
            NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
        }

        /* Pop */
        INFO_WLFS_ASSIGN (arg_info) = assign_tmp;
        INFO_WLFS_LET (arg_info) = ASSIGN_INSTR (INFO_WLFS_ASSIGN (arg_info));
        INFO_WLFS_WL2EXTEND (arg_info) = wl2extend_tmp;
        if (INFO_WLFS_SONSOFWL2EXTEND (arg_info) != NULL) {
            INFO_WLFS_SONSOFWL2EXTEND (arg_info)
              = NodeListFree (INFO_WLFS_SONSOFWL2EXTEND (arg_info), TRUE);
        }
        INFO_WLFS_SONSOFWL2EXTEND (arg_info) = sonsofwl2extend_tmp;
        INFO_WLFS_WLDEPENDENT (arg_info) = wldependent_tmp;

        /*
         * If the generators of the current withloop build a full partition
         * the PARTS attribute carries a positive value. Only those withloops
         * are considered further.
         */
        if (NWITH_PARTS (arg_node) >= 1) {

            /*
             * initialize WL traversal
             */
            INFO_WLFS_WL (arg_info) = arg_node; /* store the current node for later */

            /*
             * Now, we traverse the WITHOP sons for checking the type and possible
             * dependencies.
             */
            NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

            if (INFO_WLFS_WOTYPE (arg_info) == WO_gen) {

                if (INFO_WLFS_WL2EXTEND (arg_info) != NULL) {

                    wl_action = WL_2fuse;

                    /* depends the current WL on the extendable WL */
                    if (!INFO_WLFS_WLDEPENDENT (arg_info)) {

                        wln = INFO_WLFS_WL2EXTEND (arg_info);
                        /* is the number of parts equal? */

                        is_equal
                          = (NWITH_PARTS (arg_node) == NWITH_PARTS (ASSIGN_RHS (wln)));

                        /* is the size of both arrays equal? */
                        is_equal = (is_equal
                                    && CheckArraySize (INFO_WLFS_ASSIGN (arg_info), wln));
                        if (is_equal) {
                            /*
                             * traverse the N_PARTs.
                             * one value is computed during this traversal:
                             *
                             *  INFO_WLFS_GENAREEQUAL(arg_info) !!
                             */
                            INFO_WLFS_GENAREEQUAL (arg_info) = TRUE;

                            DBUG_ASSERT ((NWITH_PART (arg_node) != NULL),
                                         "NWITH_PARTS is >= 1 although no PART is "
                                         "available!");
                            NWITH_PART (arg_node)
                              = Trav (NWITH_PART (arg_node), arg_info);

                            if (INFO_WLFS_GENAREEQUAL (arg_info)) {
                                /*
                                 * traverse the N_CODEs to find all assignments, the
                                 * N_CODEs depends on. This assigments had to be tagged.
                                 * Later the tagged assignments !between! the two fused
                                 * withloops had to be shifted in front of the extendable
                                 * withloop.
                                 *
                                 * Information about last N_assign had to be stacked
                                 * before traversal
                                 */
                                assign_tmp = INFO_WLFS_ASSIGN (arg_info);

                                INFO_WLFS_TAGDEPENDENCIES (arg_info) = TRUE;
                                INFO_WLFS_INSIDEWL (arg_info) = TRUE;
                                if (NWITH_CODE (arg_node) != NULL) {
                                    NWITH_CODE (arg_node)
                                      = Trav (NWITH_CODE (arg_node), arg_info);
                                }
                                INFO_WLFS_TAGDEPENDENCIES (arg_info) = FALSE;
                                INFO_WLFS_INSIDEWL (arg_info) = FALSE;
                                INFO_WLFS_ASSIGN (arg_info) = assign_tmp;

                                arg_node = FuseWithloops (arg_node, arg_info, wln);

                                wl_action = WL_fused;
                            }
                        }
                    } else {
                        /*
                         * The current genarray-withloop depends on the extendable. Append
                         * it to INFO_WLFS_SONSOFWL2EXTEND( arg_info)
                         */
                        INFO_WLFS_SONSOFWL2EXTEND (arg_info)
                          = NodeListAppend (INFO_WLFS_SONSOFWL2EXTEND (arg_info),
                                            INFO_WLFS_ASSIGN (arg_info), NULL);
                    }

                } else {
                    /*
                     * this WL is the first WL to fuse
                     */
                    INFO_WLFS_WL2EXTEND (arg_info) = INFO_WLFS_ASSIGN (arg_info);
                    INFO_WLFS_SONSOFWL2EXTEND (arg_info)
                      = NodeListAppend (INFO_WLFS_SONSOFWL2EXTEND (arg_info),
                                        INFO_WLFS_ASSIGN (arg_info), NULL);
                }
            }
        }

        INFO_WLFS_WLDEPENDENT (arg_info) = FALSE;
        INFO_WLFS_WLACTION (arg_info) = wl_action;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSNwithop( node *arg_node, node *arg_info)
 *
 *   @brief checks the type of withloop and checks dependencies,
 *          if the type is modarray or fold.
 *
 *   @param  node *arg_node:  N_Nwithop
 *           node *arg_info:  N_info
 *   @return node *        :  N_Nwithop
 ******************************************************************************/

node *
WLFSNwithop (node *arg_node, node *arg_info)
{
    wo_type_t current_type;

    DBUG_ENTER ("WLFSNwithop");

    if (INFO_WLFS_DCTDEPENDENCIES (arg_info) || INFO_WLFS_TAGDEPENDENCIES (arg_info)) {
        /*
         * Detect dependencies mode
         * We are inside N_code of a withloop and we check if the this withloop
         * depends on the extendable withloop.
         *
         * Tag dependencies mode
         * Finds all assignments the fused withloop depends on.
         *
         * To find dependent assignments we had to traverse in N_WITHOP_ARRAY
         * if we are in a modarray withloop.
         */
        if (NWITHOP_TYPE (arg_node) == WO_modarray) {
            NWITHOP_ARRAY (arg_node) = Trav (NWITHOP_ARRAY (arg_node), arg_info);
        }
    } else {
        /* Normal mode */

        switch (NWITHOP_TYPE (arg_node)) {
        case WO_genarray:
            current_type = WO_gen;
            break;

        case WO_modarray:
            current_type = WO_mod;
            if (INFO_WLFS_WLDEPENDENT (arg_info)
                || CheckDependency (NWITHOP_ARRAY (arg_node),
                                    INFO_WLFS_SONSOFWL2EXTEND (arg_info))) {
                INFO_WLFS_SONSOFWL2EXTEND (arg_info)
                  = NodeListAppend (INFO_WLFS_SONSOFWL2EXTEND (arg_info),
                                    INFO_WLFS_ASSIGN (arg_info), NULL);
            }
            break;

        case WO_foldfun:
            /* here is no break missing */
        case WO_foldprf:
            current_type = WO_fold;
            if (INFO_WLFS_WLDEPENDENT (arg_info)) {
                INFO_WLFS_SONSOFWL2EXTEND (arg_info)
                  = NodeListAppend (INFO_WLFS_SONSOFWL2EXTEND (arg_info),
                                    INFO_WLFS_ASSIGN (arg_info), NULL);
            }
            break;

        default:
            DBUG_ASSERT ((0), "illegal NWITHOP_TYPE found!");
            break;
        }

        INFO_WLFS_WOTYPE (arg_info) = current_type;

        /*
         * If there are more than one withop, this are only genarray withops
         * for the time beeing, therefore no futher traversal is needed so far
         */
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSNpart(node *arg_node, node *arg_info)
 *
 *   @brief traverse only generator to check bounds, step and width
 *
 *   @param  node *arg_node:  N_Npart
 *           node *arg_info:  N_info
 *   @return node *        :  N_Npart
 ******************************************************************************/

node *
WLFSNpart (node *arg_node, node *arg_info)
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
 * @fn node *WLFSNgenerator(node *arg_node, node *arg_info)
 *
 *   @brief  bounds, step and width vectors are compared with bounds, step
 *           and width of all generators of the withloop out of
 *           INFO_WLFS_WL2FUSION( arg_info).

 *           Via INFO_WLFS_GENAREEQUAL( arg_info) the status of the generator
 *           is returned. Possible values are :
 *           TRUE  : this generator is equal to one generator of the
 *                   comparative withloop!
 *           FALSE : this generator is equal to not any generator of the
 *                   comparative withloop!
 *
 *   @param  node *arg_node:  N_Ngenerator
 *           node *arg_info:  N_info
 *   @return node *        :  N_Ngenerator
 ******************************************************************************/

node *
WLFSNgenerator (node *arg_node, node *arg_info)
{
    node *wl_assign, *parts, *gen;
    bool is_equal;

    DBUG_ENTER ("WLFSNgenerator");

    if (INFO_WLFS_GENAREEQUAL (arg_info)) {
        wl_assign = INFO_WLFS_WL2EXTEND (arg_info);
        parts = NWITH_PART (ASSIGN_RHS (wl_assign));

        while (parts != NULL) {
            gen = NPART_GEN (parts);
            is_equal = CompGenSon (NGEN_BOUND1 (arg_node), NGEN_BOUND1 (gen));
            is_equal = is_equal && CompGenSon (NGEN_BOUND2 (arg_node), NGEN_BOUND2 (gen));
            is_equal = is_equal && CompGenSon (NGEN_STEP (arg_node), NGEN_STEP (gen));
            is_equal = is_equal && CompGenSon (NGEN_WIDTH (arg_node), NGEN_WIDTH (gen));
            if (is_equal)
                break;
            parts = NPART_NEXT (parts);
        }
        INFO_WLFS_GENAREEQUAL (arg_info) = is_equal;
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
    node *arg_info;

    DBUG_ENTER ("WithloopFusion");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "WithloopFusion not started with fundef node");

    DBUG_PRINT ("WLFS", ("starting WithloopFusion"));

    arg_info = MakeInfo ();

    tmp_tab = act_tab;
    act_tab = wlfs_tab;

    arg_node = Trav (arg_node, arg_info);

    if (INFO_WLFS_SONSOFWL2EXTEND (arg_info) != NULL) {
        INFO_WLFS_SONSOFWL2EXTEND (arg_info)
          = NodeListFree (INFO_WLFS_SONSOFWL2EXTEND (arg_info), TRUE);
    }
    arg_info = FreeTree (arg_info);
    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}
