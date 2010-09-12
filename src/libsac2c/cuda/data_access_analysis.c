
#include "data_access_analysis.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "memory.h"
#include "dbug.h"
#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "new_types.h"
#include "shape.h"
#include "types.h"
#include "constants.h"
#include "pattern_match.h"
#include "pattern_match_attribs.h"
#include "cuda_utils.h"

typedef enum { trav_normal, trav_collect } travmode_t;

/*
 * INFO structure
 */
struct INFO {
    rc_t *rcs;
    int count;
    node *withids;
    int nest_level;
    bool in_cudawl;
    int travmode_t travmode;
    rc_t *current_rc;
    node *fundef;
    node *wlidxs;
};

#define INFO_RCS(n) (n->rcs)
#define INFO_COUNT(n) (n->count)
#define INFO_WITHIDS(n) (n->withids)
#define INFO_NEST_LEVEL(n) (n->nest_level)
#define INFO_IN_CUDAWL(n) (n->in_cudawl)
#define INFO_TRAVMODE(n) (n->travmode)
#define INFO_CURRENT_RC(n) (n->current_rc)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_WLIDXS(n) (n->wlidxs)

/*
 * INFO macros
 */

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_RCS (result) = NULL;
    INFO_COUNT (result) = 0;
    INFO_WITHIDS (result) = NULL;
    INFO_NEST_LEVEL (result) = 0;
    INFO_IN_CUDAWL (result) = FALSE;
    INFO_TRAVMODE (result) = trav_normal;
    INFO_CURRENT_RC (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_WLIDXS (result) = NULL;

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
 * @fn static rc_t *SearchRc( node* arr, rc_t* rc_list)
 *
 * @brief
 *
 *****************************************************************************/
static rc_t *
SearchRc (node *arr, rc_t *rc_list)
{
    rc_t *rc = NULL;

    DBUG_ENTER ("SearchRc");

    while (rc_list != NULL) {
        if (arr == RC_ARRAY (rc_list)) {
            rc = rc_list;
            break;
        }
        rc_list = RC_NEXT (rc_list);
    }

    DBUG_RETURN (rc);
}

/** <!--********************************************************************-->
 *
 * @fn static rc_t* ConsolidateRcs( rc_t *rc_list, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
static rc_t *
ConsolidateRcs (rc_t *rc_list, info *arg_info)
{
    rc_t *rc;
    int dim, i, negoff, posoff, block_sz, extent, shmem_sz = 1;
    node *shmem_shp = NULL;

    DBUG_ENTER ("ConsolidateRcs");

    rc = rc_list;

    while (rc != NULL) {
        dim = RC_DIM (rc);

        if (dim == 1) {
            block_sz = 256;
        } else if (dim == 2) {
            block_sz = 16;
        } else {
            DBUG_ASSERT ((0), "Reusable array with dimension greater than 2!");
        }

        for (i = dim - 1; i >= 0; i--) {
            negoff = RC_NEGOFFSET (rc, i);
            posoff = RC_POSOFFSET (rc, i);
            extent = negoff + posoff + block_sz;
            if ((negoff != 0 && posoff != 0)
                || ((negoff + posoff) > 0 && RC_SELFREF (rc))) {
                RC_REUSABLE (rc) = TRUE;
            }
            shmem_sz *= extent;
            shmem_shp = TBmakeExprs (TBmakeNum (extent), shmem_shp);
        }

        if (RC_REUSABLE (rc)) {
            RC_SHARRAY (rc)
              = TBmakeAvis (TRAVtmpVarName ("shmem"),
                            TYmakeAKS (TYmakeSimpleType (
                                         CUd2shSimpleTypeConversion (TYgetSimpleType (
                                           TYgetScalar (AVIS_TYPE (RC_ARRAY (rc)))))),
                                       SHcreateShape (1, shmem_sz)));
            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (RC_SHARRAY (rc), FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
            RC_SHARRAYSHP (rc)
              = TBmakeArray (TYmakeSimpleType (T_int), SHcreateShape (1, dim), shmem_shp);
        } else {
            INFO_COUNT (arg_info)--;
        }

        rc = RC_NEXT (rc);
    }

    DBUG_RETURN (rc_list);
}

/** <!--********************************************************************-->
 *
 * @fn node *DAAdoDataAccessAnalysis( node *syntax_tree)
 *
 * @brief
 *
 *****************************************************************************/
node *
DAAdoDataAccessAnalysis (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("DAAdoDataAccessAnalysis");

    info = MakeInfo ();
    TRAVpush (TR_daa);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *DAAfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
DAAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DAAfundef");

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DAAassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
DAAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DAAassign");

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_TRAVMODE (arg_info) == trav_normal) {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DAAwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
DAAwith (node *arg_node, info *arg_info)
{
    int dim;

    DBUG_ENTER ("DAAwith");

    dim = TCcountIds (WITH_IDS (arg_node));

    if (WITH_CUDARIZABLE (arg_node) && dim <= 2) {
        INFO_NEST_LEVEL (arg_info) += dim;
        INFO_IN_CUDAWL (arg_info) = TRUE;
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        INFO_IN_CUDAWL (arg_info) = FALSE;
        INFO_NEST_LEVEL (arg_info) -= dim;
    } else if (INFO_IN_CUDAWL (arg_info)) {
        INFO_NEST_LEVEL (arg_info) += dim;
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        INFO_NEST_LEVEL (arg_info) -= dim;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DAApart( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
DAApart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DAApart");

    if (CODE_DAA_INFO (PART_CODE (arg_node)) == NULL) {
        INFO_WITHIDS (arg_info) = PART_IDS (arg_node);
        INFO_WLIDXS (arg_info) = WITHID_IDXS (PART_WITHID (arg_node));
        PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);
    }
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DAAcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
DAAcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DAAcode");

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    if (INFO_RCS (arg_info) != NULL) {
        INFO_RCS (arg_info) = ConsolidateRcs (INFO_RCS (arg_info), arg_info);

        CODE_DAA_INFO (arg_node) = MEMmalloc (sizeof (reuse_info_t));
        CODE_DAA_RCCOUNT (arg_node) = INFO_COUNT (arg_info);
        CODE_DAA_RCS (arg_node) = INFO_RCS (arg_info);

        INFO_COUNT (arg_info) = 0;
        INFO_RCS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DAAprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
DAAprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DAAprf");

    /* If we are in cuda withloop */
    if (INFO_IN_CUDAWL (arg_info)) {
        if (PRF_PRF (arg_node) == F_idx_sel) {
            node *idx = PRF_ARG1 (arg_node);
            node *arr = PRF_ARG2 (arg_node);
            rc_t *rc = NULL;
            int dim;

            DBUG_ASSERT (NODE_TYPE (idx) == N_id,
                         "Non-id node found in the first argument of idx_sel!");
            DBUG_ASSERT (NODE_TYPE (arr) == N_id,
                         "Non-id node found in the second argument of idx_sel!");

            dim = TYgetDim (ID_NTYPE (arr));

            /* Currently, we restrict reuse candidates to be arrays
             * with the same dimensionality as the surrounding withloop
             * and are either 1D or 2D */
            if (dim == TCcountIds (INFO_WITHIDS (arg_info)) && dim > 0 && dim < 3) {
                rc = SearchRc (ID_AVIS (arr), INFO_RCS (arg_info));

                /* This is the first time we come across this array */
                if (rc == NULL) {
                    INFO_RCS (arg_info)
                      = TBmakeReuseCandidate (ID_AVIS (arr), dim, INFO_RCS (arg_info));
                    INFO_CURRENT_RC (arg_info) = INFO_RCS (arg_info);
                    INFO_COUNT (arg_info)++;
                } else {
                    INFO_CURRENT_RC (arg_info) = rc;
                }

                /* If the index variable is the wlidx of the withloop
                 * we set relf reference to TRUE */
                if (IDS_AVIS (INFO_WLIDXS (arg_info)) == ID_AVIS (idx)) {
                    RC_SELFREF (INFO_CURRENT_RC (arg_info)) = TRUE;
                } else { /* Start backtracing to collect reuse information */
                    INFO_TRAVMODE (arg_info) = trav_collect;
                    ID_SSAASSIGN (idx) = TRAVopt (ID_SSAASSIGN (idx), arg_info);
                    INFO_TRAVMODE (arg_info) = trav_normal;
                }
                INFO_CURRENT_RC (arg_info) = NULL;
            }
        } else if (PRF_PRF (arg_node) == F_idxs2offset
                   && INFO_TRAVMODE (arg_info) == trav_collect) {
            pattern *pat1, *pat2, *pat3;
            node *id, *ids, *withids;
            int off, dim;
            rc_t *rc;
            bool selfref;

            rc = INFO_CURRENT_RC (arg_info);
            DBUG_ASSERT ((rc != NULL), "Null reuse candidate found!");

            pat1 = PMprf (1, PMAisPrf (F_sub_SxS), 2, PMvar (1, PMAgetNode (&id), 0),
                          PMint (1, PMAgetIVal (&off), 0));
            pat2 = PMprf (1, PMAisPrf (F_add_SxS), 2, PMvar (1, PMAgetNode (&id), 0),
                          PMint (1, PMAgetIVal (&off), 0));
            pat3 = PMprf (1, PMAisPrf (F_add_SxS), 2, PMvar (1, PMAgetIVal (&off), 0),
                          PMint (1, PMAgetNode (&id), 0));

            if (RC_ARRAYSHP (rc) == NULL) {
                RC_ARRAYSHP (rc) = DUPdoDupNode (PRF_ARG1 (arg_node));
            }

            dim = 0;
            ids = PRF_EXPRS2 (arg_node);
            withids = INFO_WITHIDS (arg_info);

            /* We currenly only look at the case when the dim of
             * the array being inferred is same as the dim of the
             * surrounding withloop. */
            if (TCcountExprs (ids) == TCcountIds (withids)) {
                while (ids != NULL && withids != NULL) {
                    if (PMmatchFlat (pat1, EXPRS_EXPR (ids))) {
                        if (ID_AVIS (id) == IDS_AVIS (withids)) {
                            if (off > RC_NEGOFFSET (rc, dim)) {
                                RC_NEGOFFSET (rc, dim) = off;
                            } else if ((-off) > RC_POSOFFSET (rc, dim)) {
                                RC_POSOFFSET (rc, dim) = (-off);
                            }
                        }
                    } else if (PMmatchFlat (pat2, EXPRS_EXPR (ids))
                               || PMmatchFlat (pat3, EXPRS_EXPR (ids))) {
                        if (ID_AVIS (id) == IDS_AVIS (withids)) {
                            if (off > RC_POSOFFSET (rc, dim)) {
                                RC_POSOFFSET (rc, dim) = off;
                            } else if ((-off) > RC_NEGOFFSET (rc, dim)) {
                                RC_NEGOFFSET (rc, dim) = (-off);
                            }
                        }
                    }

                    /* Flag indicating whether the array is selected at
                     * each position */
                    selfref
                      = selfref && (ID_AVIS (EXPRS_EXPR (ids)) == IDS_AVIS (withids));

                    dim++;
                    ids = EXPRS_NEXT (ids);
                    withids = IDS_NEXT (withids);
                }
                RC_SELFREF (rc) = selfref;
            }

            pat1 = PMfree (pat1);
            pat2 = PMfree (pat2);
            pat3 = PMfree (pat3);
        } else {
        }
    }

    DBUG_RETURN (arg_node);
}
