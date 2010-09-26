
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
#include "matrix.h"

typedef enum { trav_normal, trav_collect } travmode_t;

/* Essential information of a WL partition */
typedef struct PART_INFO {
    int dim;           /* How many ids */
    unsigned int type; /* ThreadIdx or LoopIdx */
    int nth; /* Position of a particularly searching id. See function SearchIndex */
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
    int idxdim;
    int cuwldim;
    node *pragma;
    cuda_access_info_t *access_info;
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
#define INFO_IDXDIM(n) (n->idxdim)
#define INFO_CUWLDIM(n) (n->cuwldim)
#define INFO_PRAGMA(n) (n->pragma)
#define INFO_ACCESS_INFO(n) (n->access_info)

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
    INFO_COEFFICIENT (result) = 1;
    INFO_IDXDIM (result) = 0;
    INFO_CUWLDIM (result) = 0;
    INFO_PRAGMA (result) = NULL;
    INFO_ACCESS_INFO (result) = NULL;

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

static unsigned int
DecideThreadIdx (node *ids, int dim, node *avis)
{
    unsigned int res = -1;

    DBUG_ENTER ("DecideThreadIdx");

    if (dim == 1) {
        DBUG_ASSERT ((IDS_AVIS (ids) == avis), "Unknown wl ids found!");
        res = IDX_THREADIDX_X;
    } else if (dim == 2) {
        if (IDS_AVIS (ids) == avis) {
            res = IDX_THREADIDX_Y;
        } else if (IDS_AVIS (IDS_NEXT (ids)) == avis) {
            res = IDX_THREADIDX_X;
        } else {
            DBUG_ASSERT ((0), "Found withids with more than 2 ids!");
        }
    } else {
        DBUG_ASSERT ((0), "Found withids with more than 2 ids!");
    }

    DBUG_RETURN (res);
}

static void
AddIndex (unsigned int type, int coefficient, node *idx, int looplevel, int dim,
          info *arg_info)
{
    DBUG_ENTER ("AddIndex");

    CUAI_INDICES (INFO_ACCESS_INFO (arg_info), dim)
      = TBmakeCudaIndex (type, coefficient, idx, looplevel,
                         CUAI_INDICES (INFO_ACCESS_INFO (arg_info), dim));

    DBUG_VOID_RETURN;
}

static void
ActOnId (node *avis, info *arg_info)
{
    node *ssa_assign;
    part_info_t *part_info;

    DBUG_ENTER ("ActOnId");

    /* This function checks the id (avis) and performs the following
     * actions:
     *   - If the ssa assign of this id is NULL:
     *       - If it's a function argument, we add it as an external
     *         variable to the indices;
     *       - If it's a WL ids, we either add a ThreadIdx or a LoopIdx
     *         depending on whether it belongs to outermost cuda WL or
     *         inner WLs. In this case, we also need to update the coefficient
     *         matrix.
     *   - If the ssa assign of this id is NOT NULL:
     *       - If the defining assignment is outside of the current cuda
     *         WL, we add it as an external variable to the indices;
     *       - Otherwise, we start backtracking with a "collect" mode.
     */

    ssa_assign = AVIS_SSAASSIGN (avis);
    /*
     * SSAASSIGN is NULL can be in two cases:
     *  - the id is a function argument;
     *  - the id is withids.
     */
    if (ssa_assign == NULL) {
        if (NODE_TYPE (AVIS_DECL (avis)) == N_arg) {
            AddIndex (IDX_EXTID, INFO_COEFFICIENT (arg_info), avis, 0,
                      INFO_IDXDIM (arg_info), arg_info);
        } else if ((part_info = SearchIndex (INFO_PART_INFO (arg_info), avis)) != NULL) {
            unsigned int type = IDX_LOOPIDX;
            DBUG_ASSERT ((PART_INFO_TYPE (part_info) == IDX_THREADIDX
                          || PART_INFO_TYPE (part_info) == IDX_LOOPIDX),
                         "Found indices which are neither thread indiex nor loop index!");

            if (PART_INFO_TYPE (part_info) == IDX_THREADIDX) {
                type = DecideThreadIdx (PART_INFO_WLIDS (part_info),
                                        PART_INFO_DIM (part_info), avis);
            }
            /* If either LOOPIDX of THREADIDX apprears in this dimension, it's no longer
             * constant */
            CUAI_ISCONSTANT (INFO_ACCESS_INFO (arg_info), INFO_IDXDIM (arg_info)) = FALSE;
            AddIndex (type, INFO_COEFFICIENT (arg_info), avis,
                      PART_INFO_NTH (part_info) + 1, INFO_IDXDIM (arg_info), arg_info);
            MatrixSetEntry (CUAI_MATRIX (INFO_ACCESS_INFO (arg_info)),
                            PART_INFO_NTH (part_info), INFO_IDXDIM (arg_info),
                            INFO_COEFFICIENT (arg_info));
        } else {
            DBUG_ASSERT ((0), "Found id whose ssaassign is NULL and it is neither an arg "
                              "or a withids!");
        }
    } else {
        /* If this id is defined by an assignment outside the current cuda WL */
        if (ASSIGN_LEVEL (ssa_assign) == 0) {
            AddIndex (IDX_EXTID, INFO_COEFFICIENT (arg_info), avis, 0,
                      INFO_IDXDIM (arg_info), arg_info);
        }
        /* Otherwise, we start backtracking to collect data access information */
        else {
            ASSIGN_INSTR (ssa_assign) = TRAVopt (ASSIGN_INSTR (ssa_assign), arg_info);
        }
    }

    DBUG_VOID_RETURN;
}

static cuda_access_info_t *
CreateSharedMemory (cuda_access_info_t *access_info, info *arg_info)
{
    int i, coefficient, shmem_size, dim;
    cuda_index_t *index;
    int DIMS[2][2]
      = {{1, global.cuda_1d_block_x}, {global.cuda_2d_block_y, global.cuda_2d_block_x}};
    node *sharray_shp = NULL;

    DBUG_ENTER ("CreateSharedMemory");

    dim = CUAI_DIM (access_info);

    for (i = dim - 1; i >= 0; i--) {
        index = CUAI_INDICES (access_info, i);
        DBUG_ASSERT ((index != NULL), "Found NULL index!");

        shmem_size = 0;

        while (index != NULL) {
            coefficient = abs (CUIDX_COEFFICIENT (index));

            switch (CUIDX_TYPE (index)) {
            case IDX_THREADIDX_X:
                shmem_size += (coefficient * DIMS[dim - 1][1]);
                break;
            case IDX_THREADIDX_Y:
                shmem_size += (coefficient * DIMS[dim - 1][0]);
                break;
            case IDX_LOOPIDX:
                shmem_size += (coefficient * DIMS[dim - 1][1]);
                /* Set the block size for the loop dim */
                AVIS_NEEDBLOCKED (CUIDX_ID (index)) = TRUE;
                AVIS_BLOCKSIZE (CUIDX_ID (index)) = DIMS[dim - 1][1];
                break;
            default:
                break;
            }

            index = CUIDX_NEXT (index);
        }

        if (shmem_size == 0) {
            if (dim == 2) { /* X dimension */
                shmem_size = DIMS[dim - 1][i];
            } else if (dim == 1) { /* Y dimension */
                shmem_size = global.cuda_2d_block_y;
            }
        }

        /* For 2D share memory, size of each dimension must be a multiple
         * of the corresonding block size */
        if (dim == 2) {
            if (shmem_size % DIMS[1][i] != 0) {
                shmem_size = ((shmem_size + DIMS[1][i]) / DIMS[1][i]) * DIMS[1][i];
            }
        }

        sharray_shp = TBmakeExprs (TBmakeNum (shmem_size), sharray_shp);
    }

    CUAI_SHARRAYSHP (access_info)
      = TBmakeArray (TYmakeSimpleType (T_int), SHcreateShape (1, dim), sharray_shp);
    CUAI_SHARRAY (access_info)
      = TBmakeAvis (TRAVtmpVarName ("shmem"),
                    TYmakeAKS (TYmakeSimpleType (
                                 CUd2shSimpleTypeConversion (TYgetSimpleType (
                                   TYgetScalar (AVIS_TYPE (CUAI_ARRAY (access_info)))))),
                               SHarray2Shape (CUAI_SHARRAYSHP (access_info))));

    /*
      FUNDEF_VARDEC( INFO_FUNDEF( arg_info)) =
        TCappendVardec( FUNDEF_VARDEC( INFO_FUNDEF( arg_info)),
                        TBmakeVardec( CUAI_SHARRAY( access_info), NULL));
    */
    DBUG_RETURN (access_info);
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

        /* If this is the outermost cudarizable withloop, we
         * store its dimentionality */
        if (INFO_NEST_LEVEL (arg_info) == 0) {
            INFO_CUWLDIM (arg_info) = dim;
        }

        INFO_NEST_LEVEL (arg_info) += dim;
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        INFO_NEST_LEVEL (arg_info) -= dim;

        if (INFO_PRAGMA (arg_info) != NULL) {
            WITH_PRAGMA (arg_node) = INFO_PRAGMA (arg_info);
            INFO_PRAGMA (arg_info) = NULL;
        }
    } else {
        /* Outer most WL is either more than 2D or
         * not cudarizable at all. Do nothing */
    }

    DBUG_RETURN (arg_node);
}

static node *
CreateBlockingPragma (node *ids, int dim)
{
    node *pragma, *array, *wlcomp_aps, *block_exprs = NULL;
    /* bool needblocked = FALSE; */

    DBUG_ENTER ("CreateBlockingPragma");

    pragma = TBmakePragma ();

    /* Note that we have removed all code assocaited with the needblocked flag.
     * The effect of this is that all withloops in a cuda wl will be *blocked*
     * even the blocking size might be as trivial as 1. This ensures that each
     * dimension of the original withloop will be transformed into two with3s
     * after wlsd (at the absence of step and width). This simplifies the
     * implementation in cuda data reuse to find the indices of both ranges
     */
    while (ids != NULL) {
        if (AVIS_NEEDBLOCKED (IDS_AVIS (ids))) {
            /* needblocked = TRUE; */
            block_exprs
              = TCcombineExprs (block_exprs,
                                TBmakeExprs (TBmakeNum (AVIS_BLOCKSIZE (IDS_AVIS (ids))),
                                             NULL));

            AVIS_BLOCKSIZE (IDS_AVIS (ids)) = 0;
            AVIS_NEEDBLOCKED (IDS_AVIS (ids)) = FALSE;
        } else {
            block_exprs = TCcombineExprs (block_exprs, TBmakeExprs (TBmakeNum (1), NULL));
        }
        ids = IDS_NEXT (ids);
    }

    /* if( needblocked) { */
    array = TBmakeArray (TYmakeSimpleType (T_int), SHcreateShape (1, dim), block_exprs);

    wlcomp_aps
      = TBmakeExprs (TBmakeSpap (TBmakeSpid (NULL, "BvL0"), TBmakeExprs (array, NULL)),
                     NULL);

    PRAGMA_WLCOMP_APS (pragma) = wlcomp_aps;
    /*
      }
      else {
        block_exprs = FREEdoFreeTree( block_exprs);
        pragma = FREEdoFreeNode( pragma);
        pragma = NULL;
      }
    */

    DBUG_RETURN (pragma);
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
        ids_type = IDX_THREADIDX;
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

    /* Create N_pragma for blocking */
    INFO_PRAGMA (arg_info) = CreateBlockingPragma (PART_IDS (arg_node), dim);

    /* Continue traversing other partitions */
    /* PART_NEXT( arg_node) = TRAVopt( PART_NEXT( arg_node), arg_info);  */

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
        ASSIGN_ACCESS_INFO (arg_node) = NULL;
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
    node *operand1, *operand2;

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

                /* Currently, we restrict reuse candidates to be either 1D or 2D AKS
                 * arrays, and the dimentionality must be equal to the outermost
                 * cudarizable withloop */
                if (TYisAKS (ID_NTYPE (arr)) && (dim == 1 || dim == 2)
                    && INFO_CUWLDIM (arg_info) == dim) {
                    /* If this index has a defining assigment within the current cuda WL,
                     * we start to collect access information */
                    if (ID_SSAASSIGN (idx) != NULL
                        && ASSIGN_LEVEL (ID_SSAASSIGN (idx)) != 0) {

                        /* Create access info structure */
                        INFO_ACCESS_INFO (arg_info)
                          = TBmakeCudaAccessInfo (ID_AVIS (arr),
                                                  NULL, /* The shape information will be
                                                           assigned during traversal to
                                                           idxs2offset */
                                                  dim, INFO_NEST_LEVEL (arg_info));

                        /* Generate an empty coefficient matrix */
                        CUAI_MATRIX (INFO_ACCESS_INFO (arg_info))
                          = NewMatrix (INFO_NEST_LEVEL (arg_info), dim);

                        /* Start backtracking */
                        INFO_TRAVMODE (arg_info) = trav_collect;
                        ID_SSAASSIGN (idx) = TRAVopt (ID_SSAASSIGN (idx), arg_info);
                        INFO_TRAVMODE (arg_info) = trav_normal;

                        /* Assign the access information to the F_idx_sel assignment */
                        if (INFO_ACCESS_INFO (arg_info) != NULL) {
                            ASSIGN_ACCESS_INFO (INFO_LASTASSIGN (arg_info))
                              = INFO_ACCESS_INFO (arg_info);
                            INFO_ACCESS_INFO (arg_info) = NULL;
                        }
                    }
                }
            }
            break;
        case F_idxs2offset:
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
                node *ids, *avis, *shape;
                int rank;

                ids = PRF_EXPRS2 (arg_node);
                shape = PRF_ARG1 (arg_node);
                INFO_IDXDIM (arg_info) = 0;

                DBUG_ASSERT ((NODE_TYPE (shape) == N_array && COisConstant (shape)),
                             "Non-AKS reusable array found!");

                CUAI_ARRAYSHP (INFO_ACCESS_INFO (arg_info)) = DUPdoDupNode (shape);

                /* We loop through each index in turn */
                while (ids != NULL) {
                    avis = ID_AVIS (EXPRS_EXPR (ids));

                    INFO_IS_AFFINE (arg_info) = TRUE;
                    INFO_COEFFICIENT (arg_info) = 1;
                    ActOnId (avis, arg_info);
                    /* If any of the index is non-affine, we break and we
                     * assume this access has no reuse */
                    if (!INFO_IS_AFFINE (arg_info)) {
                        INFO_ACCESS_INFO (arg_info)
                          = TBfreeCudaAccessInfo (INFO_ACCESS_INFO (arg_info));
                        INFO_ACCESS_INFO (arg_info) = NULL;
                        break;
                    }

                    INFO_IDXDIM (arg_info)++;
                    ids = EXPRS_NEXT (ids);
                }

                INFO_IDXDIM (arg_info) = 0;

                rank = MatrixRank (CUAI_MATRIX (INFO_ACCESS_INFO (arg_info)));
                /* If rank of the coefficient matric is less than the nestlevel,
                 * we have potential data reuse for the accessed array */
                if (rank < CUAI_NESTLEVEL (INFO_ACCESS_INFO (arg_info))) {
                    INFO_ACCESS_INFO (arg_info)
                      = CreateSharedMemory (INFO_ACCESS_INFO (arg_info), arg_info);
                } else {
                    INFO_ACCESS_INFO (arg_info)
                      = TBfreeCudaAccessInfo (INFO_ACCESS_INFO (arg_info));
                    INFO_ACCESS_INFO (arg_info) = NULL;
                }
            }
            break;
        case F_add_SxS:
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
                operand1 = PRF_ARG1 (arg_node);
                operand2 = PRF_ARG2 (arg_node);

                if (NODE_TYPE (operand1) == N_num) {
                    AddIndex (IDX_CONSTANT,
                              INFO_COEFFICIENT (arg_info) * NUM_VAL (operand1), NULL, 0,
                              INFO_IDXDIM (arg_info), arg_info);
                } else if (NODE_TYPE (operand1) == N_id) {
                    ActOnId (ID_AVIS (operand1), arg_info);
                } else {
                    DBUG_ASSERT ((0), "Unknown type of node found in operands!");
                }

                if (NODE_TYPE (operand2) == N_num) {
                    AddIndex (IDX_CONSTANT,
                              INFO_COEFFICIENT (arg_info) * NUM_VAL (operand2), NULL, 0,
                              INFO_IDXDIM (arg_info), arg_info);
                } else if (NODE_TYPE (operand2) == N_id) {
                    ActOnId (ID_AVIS (operand2), arg_info);
                } else {
                    DBUG_ASSERT ((0), "Unknown type of node found in operands!");
                }
            }
            break;
        case F_sub_SxS:
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
                int old_coefficient;
                operand1 = PRF_ARG1 (arg_node);
                operand2 = PRF_ARG2 (arg_node);

                if (NODE_TYPE (operand1) == N_num) {
                    AddIndex (IDX_CONSTANT,
                              INFO_COEFFICIENT (arg_info) * NUM_VAL (operand1), NULL, 0,
                              INFO_IDXDIM (arg_info), arg_info);
                } else if (NODE_TYPE (operand1) == N_id) {
                    ActOnId (ID_AVIS (operand1), arg_info);
                } else {
                    DBUG_ASSERT ((0), "Unknown type of node found in operands!");
                }

                if (NODE_TYPE (operand2) == N_num) {
                    old_coefficient = INFO_COEFFICIENT (arg_info);
                    INFO_COEFFICIENT (arg_info) *= -1;
                    AddIndex (IDX_CONSTANT,
                              INFO_COEFFICIENT (arg_info) * NUM_VAL (operand2), NULL, 0,
                              INFO_IDXDIM (arg_info), arg_info);
                    INFO_COEFFICIENT (arg_info) = old_coefficient;
                } else if (NODE_TYPE (operand2) == N_id) {
                    old_coefficient = INFO_COEFFICIENT (arg_info);
                    INFO_COEFFICIENT (arg_info) *= -1;
                    ActOnId (ID_AVIS (operand2), arg_info);
                    INFO_COEFFICIENT (arg_info) = old_coefficient;
                } else {
                    DBUG_ASSERT ((0), "Unknown type of node found in operands!");
                }
            }
            break;
        case F_mul_SxS:
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
                int old_coefficient;
                operand1 = PRF_ARG1 (arg_node);
                operand2 = PRF_ARG2 (arg_node);

                if (NODE_TYPE (operand1) == N_id && NODE_TYPE (operand2) == N_id) {
                    /* Currently, multiplication of two ids are treated as non-affine */
                    INFO_IS_AFFINE (arg_info) = FALSE;
                } else if (NODE_TYPE (operand1) == N_id
                           && NODE_TYPE (operand2) == N_num) {
                    old_coefficient = INFO_COEFFICIENT (arg_info);
                    INFO_COEFFICIENT (arg_info) *= NUM_VAL (operand2);
                    ActOnId (ID_AVIS (operand1), arg_info);
                    INFO_COEFFICIENT (arg_info) = old_coefficient;
                } else if (NODE_TYPE (operand1) == N_num
                           && NODE_TYPE (operand2) == N_id) {
                    old_coefficient = INFO_COEFFICIENT (arg_info);
                    INFO_COEFFICIENT (arg_info) *= NUM_VAL (operand1);
                    ActOnId (ID_AVIS (operand2), arg_info);
                    INFO_COEFFICIENT (arg_info) = old_coefficient;
                } else {
                    DBUG_ASSERT ((0), "Unknown type of node found in operands!");
                }
            }
            break;
        default:
            if (INFO_TRAVMODE (arg_info) == trav_collect) {
                /* all other primitives are counted as non-affine */
                INFO_IS_AFFINE (arg_info) = FALSE;
            }
            break;
        }
    }

    DBUG_RETURN (arg_node);
}