
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

/* Essential information of a WL partition */
typedef struct PART_INFO {
    int dim;
    int type;
    int nth;
    node *wlids;
    node *step;
    node *width;
    struct PART_INFO *next;
} part_info_t;

/*
 * INFO structure
 */
struct INFO {
    int nest_level;
    travmode_t travmode;
    node *fundef;
    node *wlidxs;
    part_info_t *part_info;
    node *current_block;
    node *lastassign;
    bool is_affine;
    int coefficient;
    int dim;
};

#define INFO_NEST_LEVEL(n) (n->nest_level)
#define INFO_TRAVMODE(n) (n->travmode)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_WLIDXS(n) (n->wlidxs)
#define INFO_PART_INFO(n) (n->part_info)
#define INFO_CURRENT_BLOCK(n) (n->current_block)
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_IS_AFFINE(n) (n->is_affine)
#define INFO_COEFFICIENT(n) (n->coefficient)
#define INFO_DIM(n) (n->dim)

#define PART_INFO_DIM(n) (n->dim)
#define PART_INFO_TYPE(n) (n->type)
#define PART_INFO_NTH(n) (n->nth)
#define PART_INFO_WLIDS(n) (n->wlids)
#define PART_INFO_STEP(n) (n->step)
#define PART_INFO_WIDTH(n) (n->width)
#define PART_INFO_NEXT(n) (n->next)

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

    INFO_NEST_LEVEL (result) = 0;
    INFO_TRAVMODE (result) = trav_normal;
    INFO_FUNDEF (result) = NULL;
    INFO_WLIDXS (result) = NULL;
    INFO_PART_INFO (result) = NULL;
    INFO_CURRENT_BLOCK (result) = NULL;
    INFO_LASTASSIGN (result) = NULL;
    INFO_IS_AFFINE (result) = TRUE;
    INFO_COEFFICIENT (result) = 0;
    INFO_DIM (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static part_info_t *
CreatePartInfo (int dim, int type, node *wlids, node *step, node *width)
{
    part_info_t *info;

    DBUG_ENTER ("CreatePartInfo");

    info = MEMmalloc (sizeof (part_info_t));

    PART_INFO_DIM (info) = dim;
    PART_INFO_TYPE (info) = type;
    PART_INFO_NTH (info) = 0;
    PART_INFO_WLIDS (info) = wlids;
    PART_INFO_STEP (info) = step;
    PART_INFO_WIDTH (info) = width;
    PART_INFO_NEXT (info) = NULL;

    DBUG_RETURN (info);
}

static part_info_t *
PushPartInfo (part_info_t *infos, part_info_t *info)
{
    DBUG_ENTER ("PushPartInfo");

    if (infos == NULL) {
        infos = info;
    } else {
        while (PART_INFO_NEXT (infos) != NULL) {
            infos = PART_INFO_NEXT (infos);
        }
        PART_INFO_NEXT (infos) = info;
    }

    DBUG_RETURN (infos);
}

static part_info_t *
PopPartInfo (part_info_t *infos)
{
    part_info_t *res;

    DBUG_ENTER ("PopPartInfo");

    DBUG_ASSERT ((infos != NULL), "Partition information chain is NULL!");

    if (PART_INFO_NEXT (infos) == NULL) {
        infos = MEMfree (infos);
        res = NULL;
    } else {
        PART_INFO_NEXT (infos) = PopPartInfo (PART_INFO_NEXT (infos));
        res = infos;
    }

    DBUG_RETURN (res);
}

static part_info_t *
SearchIndex (part_info_t *infos, node *avis)
{
    int nth = 0;
    node *wlids;
    part_info_t *res = NULL;

    DBUG_ENTER ("SearchIndex");

    while (infos != NULL) {
        wlids = PART_INFO_WLIDS (infos);
        while (wlids != NULL) {
            if (IDS_AVIS (wlids) == avis) {
                PART_INFO_NTH (infos) = nth;
                return infos;
            }
            nth += 1;
            wlids = IDS_NEXT (wlids);
        }
        infos = PART_INFO_NEXT (infos);
    }

    DBUG_RETURN (res);
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

    if ((WITH_CUDARIZABLE (arg_node) && dim <= 2) || INFO_NEST_LEVEL (arg_info) > 0) {
        INFO_NEST_LEVEL (arg_info) += dim;
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        INFO_NEST_LEVEL (arg_info) -= dim;
    } else {
        /* Outer most WL is either more than 2D or
         * not cudarizable at all. Do nothing */
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
    int dim, ids_type;
    part_info_t *p_info;
    node *old_wlidx;

    DBUG_ENTER ("DAApart");

    dim = TCcountIds (PART_IDS (arg_node));

    /* If this partition belongs to the outer most cudarizable WL,
     * its ids are annotated as cuda threadidx */
    if (INFO_NEST_LEVEL (arg_info) == dim) {
        ids_type = IDX_THREADIDX_X | IDX_THREADIDX_Y;
    }
    /* If this partition belongs to any inner WLs,
     * its ids are annotated as loop index */
    else if (INFO_NEST_LEVEL (arg_info) > dim) {
        ids_type = IDX_LOOPIDX;
    } else {
        DBUG_ASSERT ((0), "Wrong nesting level found!");
    }

    /* Push information */
    p_info = CreatePartInfo (dim, ids_type, PART_IDS (arg_node), NULL, NULL);
    INFO_PART_INFO (arg_info) = PushPartInfo (INFO_PART_INFO (arg_info), p_info);
    old_wlidx = INFO_WLIDXS (arg_info);
    INFO_WLIDXS (arg_info) = WITHID_IDXS (PART_WITHID (arg_node));

    /* Start traversing the code */
    PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);

    /* Pop information */
    INFO_WLIDXS (arg_info) = old_wlidx;
    INFO_PART_INFO (arg_info) = PopPartInfo (INFO_PART_INFO (arg_info));

    /* Continue traversing other partitions */
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
    /*
      if( INFO_RCS( arg_info) != NULL) {
        INFO_RCS( arg_info) = ConsolidateRcs( INFO_RCS( arg_info), arg_info);

        CODE_DAA_INFO( arg_node) = MEMmalloc( sizeof( reuse_info_t));
        CODE_DAA_RCCOUNT( arg_node)  = INFO_COUNT( arg_info);
        CODE_DAA_RCS( arg_node)      = INFO_RCS( arg_info);

        INFO_COUNT( arg_info) = 0;
        INFO_RCS( arg_info) = NULL;
      }
    */

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
DAAblock (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("DAAblock");

    BLOCK_INSTR (arg_node) = TRAVopt (BLOCK_INSTR (arg_node), arg_info);

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
    node *old_lastassign;

    DBUG_ENTER ("DAAassign");

    old_lastassign = INFO_LASTASSIGN (arg_info);
    INFO_LASTASSIGN (arg_info) = arg_node;

    if (INFO_TRAVMODE (arg_info) == trav_normal) {
        ASSIGN_LEVEL (arg_node) = INFO_NEST_LEVEL (arg_info);
        ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    } else if (INFO_TRAVMODE (arg_info) == trav_collect) {
        ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);
    } else {
        DBUG_ASSERT ((0), "Wrong traverse mode!");
    }

    INFO_LASTASSIGN (arg_info) = old_lastassign;

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
    if (INFO_NEST_LEVEL (arg_info) > 0) {
        switch (PRF_PRF (arg_node)) {
        case F_idx_sel:
            if (INFO_TRAVMODE (arg_info) == trav_normal) {
                node *idx = PRF_ARG1 (arg_node);
                node *arr = PRF_ARG2 (arg_node);
                int dim;

                DBUG_ASSERT (NODE_TYPE (idx) == N_id,
                             "Non-id node found in the first argument of idx_sel!");
                DBUG_ASSERT (NODE_TYPE (arr) == N_id,
                             "Non-id node found in the second argument of idx_sel!");

                dim = TYgetDim (ID_NTYPE (arr));

                /* Currently, we restrict reuse candidates to be either 1D or 2D arrays */
                if (dim == 1 || dim == 2) {
                    /* If this index has a defining assigment within the current cuda WL,
                     * we start to collect access information */
                    if (ID_SSAASSIGN (idx) != NULL
                        && ASSIGN_LEVEL (ID_SSAASSIGN (idx)) != 0) {
                        INFO_TRAVMODE (arg_info) = trav_collect;
                        ID_SSAASSIGN (idx) = TRAVopt (ID_SSAASSIGN (idx), arg_info);
                        INFO_TRAVMODE (arg_info) = trav_normal;
                    }
                }
            }
            break;
        case F_idxs2offset:
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
                node *ids, *avis, *ssa_assign;
                part_info_t *part_info;

                ids = PRF_EXPRS2 (arg_node);
                INFO_DIM (arg_info) = 0;
                while (ids != NULL) {
                    avis = ID_AVIS (EXPRS_EXPR (ids));
                    ssa_assign = AVIS_SSAASSIGN (avis);
                    /*
                     * SSAASSIGN is NULL can be in two cases:
                     *  - the id is a function argument;
                     *  - the id is withids.
                     */
                    if (ssa_assign == NULL) {
                        if (NODE_TYPE (AVIS_DECL (avis)) == N_arg) {
                        } else if ((part_info
                                    = SearchIndex (INFO_PART_INFO (arg_info), avis))
                                   != NULL) {

                        } else {
                            DBUG_ASSERT ((0), "Found id whose ssaassign is NULL and it "
                                              "is neither an arg or a withids!");
                        }
                    } else {
                    }

                    INFO_DIM (arg_info)++;
                    ids = EXPRS_NEXT (ids);
                }
                INFO_DIM (arg_info) = 0;
            }
            break;
        case F_add_SxS:
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
            }
            break;
        case F_sub_SxS:
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
            }
            break;
        default:
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
            }
            break;
        }
    }

    DBUG_RETURN (arg_node);
}
