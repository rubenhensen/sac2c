/*
 *
 * $Log$
 * Revision 1.1  2004/04/08 08:15:56  khf
 * Initial revision
 *
 *
 *
 */

#define INFO_WLFS_WL(n) (n->node[0])
#define INFO_WLFS_FUNDEF(n) (n->node[1])
#define INFO_WLFS_ASSIGN(n) (n->node[2])
#define INFO_WLFS_LET(n) (n->node[3])
#define INFO_WLFS_WOTYPE(n) (n->counter)
#define INFO_WLFS_INSIDEWL(n) (n->int_data)
#define INFO_WLFS_GENAREEQUAL(n) (n->varno)
#define INFO_WLFS_WL2FUSION(n) ((nodelist *)(n->dfmask[0]))
#define INFO_WLFS_WLREFERENCED(n) ((nodelist *)(n->dfmask[1]))

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

/** <!--********************************************************************-->
 *
 * @fn nodelist *CopyNodeList( nodelist *nl)
 *
 *   @brief copies nodelist nl
 *
 *   @param  nodelist *nl :
 *   @return nodelist *   :  copied nodelist
 ******************************************************************************/
static nodelist *
CopyNodeList (nodelist *nl)
{
    nodelist *tmpnl;
    nodelist *newnl = NULL;

    DBUG_ENTER ("CopyNodeList");

    tmpnl = nl;
    while (tmpnl) {
        newnl = NodeListAppend (newnl, NODELIST_NODE (tmpnl), NULL);
        tmpnl = NODELIST_NEXT (tmpnl);
    }

    DBUG_RETURN (newnl);
}

/** <!--********************************************************************-->
 *
 * @fn nodelist *DeleteIds( nodelist *nlsource, nodelist *nldel)
 *
 *   @brief deletes identifiers from nldel out of nlsource
 *
 *   @param  nodelist *nlsource :  contains identifiers of possible
 *                                 fusionable withloops
 *           nodelist *wlref    :  contains idenfiers of nested withloops
 *   @return nodelist *         :  modified nlsource
 ******************************************************************************/
static nodelist *
DeleteIds (nodelist *nlsource, nodelist *nldel)
{
    nodelist *tmpnl;

    DBUG_ENTER ("DeleteIds");

    tmpnl = nldel;

    while (tmpnl != NULL) {
        NodeListDelete (nlsource, NODELIST_NODE (tmpnl), TRUE);
        tmpnl = NODELIST_NEXT (tmpnl);
    }

    DBUG_RETURN (nlsource);
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
 *           node *test_wl_assign   :  N_assign node of fusionable withloop
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
 * @fn bool CheckRef( node * checkpart, node *parts)
 *
 *   @brief checks if checkpart's N_Ncode is referenced several.
 *
 *   @param  node *checkpart : N_Npart
 *           node *parts     : N_Npart list
 *   @return bool            : TRUE iff no other N_Npart has the same
 *                             N_Ncode
 ******************************************************************************/
static bool
CheckRef (node *checkpart, node *parts)
{
    node *ncode;
    bool unique_ref = FALSE;
    int used;
    int num = 0;

    DBUG_ENTER ("CheckRef");

    ncode = NPART_CODE (checkpart);

    while (parts != NULL) {
        if (ncode == NPART_CODE (parts))
            num++;
        parts = NPART_NEXT (parts);
    }
    if (num == 1)
        unique_ref = TRUE;

    used = NCODE_USED (ncode);

    DBUG_RETURN (unique_ref);
}

/** <!--********************************************************************-->
 *
 * @fn node *FuseNCodes( node *extent_part, node *fitting_part,
 *                       bool unique_ref, node *wl2fuse)
 *
 *   @brief inserts the whole assignment block of 'fitting_part' in
 *          assignment block of 'extent_part'. If unique_ref is false, the
 *          N_Ncode of 'extent_part' is referenced several, so that it has
 *          to be deepcopied before extent.
 *
 *   @param  node *extent_part  : N_Npart
 *           node *fitting_part : N_Npart
 *           bool unique_ref    : TRUE iff N_Ncode of extent_part is unique
 *                                referenced
 *           node *wl2fuse      : N_Nwith current wl will be fused with
 *   @return node               : modified extent_part
 ******************************************************************************/
static node *
FuseNCodes (node *extent_part, node *fitting_part, bool unique_ref, node *wl2fuse)
{
    node *nassigns_start, *nassigns, *ext_withid, *fit_withid, *wl2fuse_ncode,
      *extent_nblock, *extent_assigns, *fitting_assigns, *extent_exprs;
    ids *ext_ids_tmp, *fit_ids_tmp;

    DBUG_ENTER ("FuseNCodes");

    if (!unique_ref) {
        wl2fuse_ncode = NWITH_CODE (wl2fuse);

        if (wl2fuse_ncode == NPART_CODE (extent_part)) {
            NCODE_USED (wl2fuse_ncode)--;
            NPART_CODE (extent_part) = DupTree (NPART_CODE (extent_part));
            NCODE_USED (NPART_CODE (extent_part)) = 1;
            NWITH_CODE (wl2fuse) = NPART_CODE (extent_part);
            NCODE_NEXT (NPART_CODE (extent_part)) = wl2fuse_ncode;
        } else {
            while (NCODE_NEXT (wl2fuse_ncode) != NPART_CODE (extent_part)) {
                wl2fuse_ncode = NCODE_NEXT (wl2fuse_ncode);
            }
            NCODE_USED (NCODE_NEXT (wl2fuse_ncode))--;
            NPART_CODE (extent_part) = DupTree (NPART_CODE (extent_part));
            NCODE_USED (NPART_CODE (extent_part)) = 1;
            NCODE_NEXT (NPART_CODE (extent_part)) = NCODE_NEXT (wl2fuse_ncode);
            NCODE_NEXT (wl2fuse_ncode) = NPART_CODE (extent_part);
        }
    }

    /* create new assignments for N_Nwithid sons of fitting_part */

    fit_withid = NPART_WITHID (fitting_part);
    ext_withid = NPART_WITHID (extent_part);

    nassigns_start = MakeAssign (MakeLet (DupIds_Id (NWITHID_VEC (ext_withid)),
                                          DupOneIds (NWITHID_VEC (fit_withid))),
                                 NULL);

    fit_ids_tmp = NWITHID_IDS (fit_withid);
    ext_ids_tmp = NWITHID_IDS (ext_withid);

    nassigns = nassigns_start;
    while (fit_ids_tmp != NULL) {
        ASSIGN_NEXT (nassigns)
          = MakeAssign (MakeLet (DupIds_Id (ext_ids_tmp), DupOneIds (fit_ids_tmp)), NULL);
        fit_ids_tmp = IDS_NEXT (fit_ids_tmp);
        ext_ids_tmp = IDS_NEXT (ext_ids_tmp);
        nassigns = ASSIGN_NEXT (nassigns);
    }

    /*
     * extent N_block of extent_part's N_Ncode by new assignments
     * and N_block of fitting_part
     */
    extent_nblock = NCODE_CBLOCK (NPART_CODE (extent_part));
    extent_assigns = BLOCK_INSTR (extent_nblock);
    fitting_assigns = BLOCK_INSTR (NCODE_CBLOCK (NPART_CODE (fitting_part)));

    if (NODE_TYPE (extent_assigns) == N_empty) {
        /* there is no instruction in the block right now. */
        BLOCK_INSTR (extent_nblock) = FreeTree (BLOCK_INSTR (extent_nblock));

        BLOCK_INSTR (extent_nblock) = nassigns_start;
        if (NODE_TYPE (fitting_assigns) != N_empty) {
            /* intructions of fitting_part after new assignments */
            ASSIGN_NEXT (nassigns) = fitting_assigns;
        }
    } else {
        BLOCK_INSTR (extent_nblock) = nassigns_start;
        /* intructions of extent_part after new assignments */
        ASSIGN_NEXT (nassigns) = extent_assigns;

        /* intructions of fitting_part after intructions of extent_part */
        while (ASSIGN_NEXT (nassigns) != NULL) {
            nassigns = ASSIGN_NEXT (nassigns);
        }
        ASSIGN_NEXT (nassigns) = fitting_assigns;
    }

    /*
     * extent N_exprs of extent_part's N_Ncode by N_exprs of fitting_part
     */
    extent_exprs = NCODE_CEXPRS (NPART_CODE (extent_part));
    while (EXPRS_NEXT (extent_exprs) != NULL) {
        extent_exprs = EXPRS_NEXT (extent_exprs);
    }
    EXPRS_NEXT (extent_exprs) = DupTree (NCODE_CEXPRS (NPART_CODE (fitting_part)));

    DBUG_RETURN (extent_part);
}

/** <!--********************************************************************-->
 *
 * @fn node *FuseWithloops( node *wl, node *arg_info, node *wl2fuse_assign)
 *
 *   @brief Fuses the two withloops together. Important for dependencies
 *          in code the current withloop has to fuse with the other such
 *          the current wl will be removed by DCR.
 *
 *   @param  node *wl       :  current N_Nwith node
 *           node *arg_info :  N_INFO
 *           node *wl2fuse  :  N_assign node of the second withloop where the
 *                             current withloop will be fused to
 *   @return node *         :  modified current N_Nwith node for DCR
 ******************************************************************************/
static node *
FuseWithloops (node *wl, node *arg_info, node *wl2fuse_assign)
{
    node *wl2fuse, *tmp_withop, *vardec, *parts, *fitting_part;
    ids *tmp_ids, *_ids, *idsl;
    bool unique_ref;
    char *nvarname;

    DBUG_ENTER ("FuseWithloops");

    /* N_Nwith where current withloop will be fused with */
    wl2fuse = ASSIGN_RHS (wl2fuse_assign);

    /*
     * 1. extend LHS of wl2fuse_assign by LHS of wl assignment
     */
    tmp_ids = ASSIGN_LHS (wl2fuse_assign);
    while (IDS_NEXT (tmp_ids) != NULL) {
        tmp_ids = IDS_NEXT (tmp_ids);
    }
    IDS_NEXT (tmp_ids) = DupAllIds (ASSIGN_LHS (INFO_WLFS_ASSIGN (arg_info)));

    /*
     * 2. extend each N_Npart's N_Ncode of the withloop belonging to
     *    'wl2fuse_assign' by whole assignment block of N_Npart's N_Ncode
     *    of 'wl' if both N_Ngenerators
     *    are equal. WARNING: N_Ncode can be referenced several times.
     */
    parts = NWITH_PART (wl2fuse);
    while (parts != NULL) {
        fitting_part = FindFittingPart (NPART_GEN (parts), NWITH_PART (wl));
        DBUG_ASSERT ((fitting_part != NULL), "no fittig N_Npart is available!");
        unique_ref = NCODE_USED (NPART_CODE (parts)) == 1;
        parts = FuseNCodes (parts, fitting_part, unique_ref, wl2fuse);
        parts = NPART_NEXT (parts);
    }

    /*
     * 3. extent N_Nwithop(s) of wl2fuse by N_Nwithop of 'wl'
     */
    tmp_withop = NWITH_WITHOP (wl2fuse);
    while (NWITHOP_NEXT (tmp_withop) != NULL) {
        tmp_withop = NWITHOP_NEXT (tmp_withop);
    }
    NWITHOP_NEXT (tmp_withop) = NWITH_WITHOP (wl);

    /*
     * 4. the current withloop gets new identifiers, so that there will be
     *    no futher reference and DCR can be applied
     */
    tmp_ids = ASSIGN_LHS (INFO_WLFS_ASSIGN (arg_info));
    idsl = NULL;
    while (tmp_ids != NULL) {
        nvarname = TmpVarName (IDS_NAME (tmp_ids));
        _ids = MakeIds (nvarname, NULL, ST_regular);
        vardec
          = MakeVardec (StringCopy (nvarname), DupOneTypes (IDS_TYPE (tmp_ids)), NULL);
        IDS_VARDEC (_ids) = vardec;
        IDS_AVIS (_ids) = VARDEC_AVIS (vardec);

        INFO_WLFS_FUNDEF (arg_info) = AddVardecs (INFO_WLFS_FUNDEF (arg_info), vardec);

        /* set correct backref to defining assignment */
        AVIS_SSAASSIGN (IDS_AVIS (_ids)) = INFO_WLFS_ASSIGN (arg_info);

        if (idsl != NULL) {
            IDS_NEXT (idsl) = _ids;
            idsl = IDS_NEXT (idsl);
        } else
            idsl = _ids;

        tmp_ids = IDS_NEXT (tmp_ids);
    }
    ASSIGN_LHS (INFO_WLFS_ASSIGN (arg_info))
      = FreeAllIds (ASSIGN_LHS (INFO_WLFS_ASSIGN (arg_info)));

    ASSIGN_LHS (INFO_WLFS_ASSIGN (arg_info)) = idsl;

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
    INFO_WLFS_INSIDEWL (arg_info) = FALSE;
    INFO_WLFS_WL2FUSION (arg_info) = NULL;
    INFO_WLFS_WLREFERENCED (arg_info) = NULL;

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
    DBUG_ENTER ("WLFSassign");

    INFO_WLFS_ASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSid(node *arg_node, node *arg_info)
 *
 *   @brief  If this Id is a reference to a WL (N_Nwith) and we are inside
 *           a WL append the node to INFO_WLFS_WLREFERENCED. Therefore the
 *           WL node has to be found via avis_ssaassign backlink.
 *
 *   @param  node *arg_node:  N_id
 *           node *arg_info:  N_info
 *   @return node *        :  N_id
 ******************************************************************************/
node *
WLFSid (node *arg_node, node *arg_info)
{
    node *assignn;

    DBUG_ENTER ("WLFSid");

    if (INFO_WLFS_INSIDEWL (arg_info) == TRUE) {
        /* get the definition assignment via the AVIS_SSAASSIGN backreference */
        assignn = AVIS_SSAASSIGN (ID_AVIS (arg_node));
        if ((assignn != NULL) && (NODE_TYPE (ASSIGN_RHS (assignn)) == N_Nwith)) {
            INFO_WLFS_WLREFERENCED (arg_info)
              = NodeListAppend (INFO_WLFS_WLREFERENCED (arg_info), assignn, NULL);
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
    node *let_tmp;
    nodelist *wl2fusion_tmp, *wl2fusion_cp, *wlnl;
    int inside_wl;
    bool is_equal = TRUE;
    bool fused = FALSE;

    DBUG_ENTER ("WLFSNwith");

    /*
     * Information about last N_let, insidewl, wl2fusion had to be stacked
     * before traversal
     */
    let_tmp = ASSIGN_INSTR (INFO_WLFS_ASSIGN (arg_info));
    inside_wl = INFO_WLFS_INSIDEWL (arg_info);
    INFO_WLFS_INSIDEWL (arg_info) = TRUE;
    wl2fusion_tmp = INFO_WLFS_WL2FUSION (arg_info);

    /* The nested WLs are only local */
    INFO_WLFS_WL2FUSION (arg_info) = NULL;

    /*
     * The CODEs have to be traversed as they may contain further (nested) WLs
     * and I want to modify bottom up.
     */
    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    /* Pop */
    INFO_WLFS_LET (arg_info) = let_tmp;
    INFO_WLFS_INSIDEWL (arg_info) = inside_wl;
    if (INFO_WLFS_WL2FUSION (arg_info) != NULL) {
        INFO_WLFS_WL2FUSION (arg_info)
          = NodeListFree (INFO_WLFS_WL2FUSION (arg_info), TRUE);
    }
    INFO_WLFS_WL2FUSION (arg_info) = wl2fusion_tmp;

    /*
     * to avoid dependencies between two possible fusionable withloops,
     * we had to delete assignments of WLs from INFO_WLFS_WL2FUSION which
     * are referenced in INFO_WLFS_WLREFERENCED. This are only local
     * modification therefore we had to held a copy of wl2fusion for later.
     */

    wl2fusion_cp = CopyNodeList (INFO_WLFS_WL2FUSION (arg_info));
    INFO_WLFS_WL2FUSION (arg_info)
      = DeleteIds (INFO_WLFS_WL2FUSION (arg_info), INFO_WLFS_WLREFERENCED (arg_info));

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
         * Now, we traverse the WITHOP sons for checking whether this wl
         * is from type WO_genarray.
         */
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

        if (INFO_WLFS_WOTYPE (arg_info) == WO_gen) {

            if (INFO_WLFS_WL2FUSION (arg_info) != NULL) {

                wlnl = INFO_WLFS_WL2FUSION (arg_info);
                while (wlnl != NULL) {
                    /* is the number of parts equal? */
                    is_equal = (NWITH_PARTS (arg_node)
                                == NWITH_PARTS (ASSIGN_RHS (NODELIST_NODE (wlnl))));
                    if (!is_equal)
                        break;

                    /* is the size of both arrays equal? */
                    is_equal = CheckArraySize (INFO_WLFS_ASSIGN (arg_info),
                                               NODELIST_NODE (wlnl));
                    if (!is_equal)
                        break;

                    /*
                     * traverse the N_PARTs.
                     * one value is computed during this traversal:
                     *
                     *  INFO_WLFS_GENAREEQUAL(arg_info) !!
                     *
                     * For this the first entry in the NodeList of
                     * INFO_WLFS_WL2FUSION( arg_info) has to be the
                     * comparative withloop. Therefore we have to stack wl2fusion
                     */
                    wl2fusion_tmp = INFO_WLFS_WL2FUSION (arg_info);
                    INFO_WLFS_WL2FUSION (arg_info) = wlnl;
                    INFO_WLFS_GENAREEQUAL (arg_info) = TRUE;

                    DBUG_ASSERT ((NWITH_PART (arg_node) != NULL),
                                 "NWITH_PARTS is >= 1 although no PART is available!");
                    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

                    /* pop wl2fusion */
                    INFO_WLFS_WL2FUSION (arg_info) = wl2fusion_tmp;

                    if (INFO_WLFS_GENAREEQUAL (arg_info)) {
                        arg_node
                          = FuseWithloops (arg_node, arg_info, NODELIST_NODE (wlnl));
                        fused = TRUE;
                        break;
                    }

                    wlnl = NODELIST_NEXT (wlnl);
                }
            }

            /*
             * if the current withloop isn't fused it has to be append to
             *  wl2fusion nodelist
             */
            if (!fused) {
                wl2fusion_cp
                  = NodeListAppend (wl2fusion_cp, INFO_WLFS_ASSIGN (arg_info), NULL);
            }
        }
    }

    if (INFO_WLFS_WL2FUSION (arg_info) != NULL) {
        INFO_WLFS_WL2FUSION (arg_info)
          = NodeListFree (INFO_WLFS_WL2FUSION (arg_info), TRUE);
    }
    INFO_WLFS_WL2FUSION (arg_info) = wl2fusion_cp;

    if (INFO_WLFS_INSIDEWL (arg_info) == FALSE) {
        INFO_WLFS_WLREFERENCED (arg_info)
          = NodeListFree (INFO_WLFS_WLREFERENCED (arg_info), TRUE);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFSNwithop( node *arg_node, node *arg_info)
 *
 *   @brief
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

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        current_type = WO_gen;
        break;

    case WO_modarray:
        current_type = WO_mod;
        break;

    case WO_foldfun:
        /* here is no break missing */
    case WO_foldprf:
        current_type = WO_fold;
        break;

    default:
        DBUG_ASSERT ((0), "illegal NWITHOP_TYPE found!");
        break;
    }

    INFO_WLFS_WOTYPE (arg_info) = current_type;

    /*
     * there is no wl with different withops for the time beeing
     * therefore no futher traversal is needed
     */

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
 *           and width of all generators of the first withloop out of
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
        wl_assign = NODELIST_NODE (INFO_WLFS_WL2FUSION (arg_info));
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
                 "WithloopFusion started with fundef node");

    DBUG_PRINT ("WLFS", ("starting WithloopFusion"));

    arg_info = MakeInfo ();

    tmp_tab = act_tab;
    act_tab = wlfs_tab;

    arg_node = Trav (arg_node, arg_info);

    arg_info = FreeTree (arg_info);
    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}
