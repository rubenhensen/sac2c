
#include "adjust_shmem_access.h"

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
#include "LookUpTable.h"

/*
 * INFO structure
 */
struct INFO {
    int level;
    node *lhs;
};

#define INFO_LEVEL(n) (n->level)
#define INFO_LHS(n) (n->lhs)

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

    INFO_LEVEL (result) = 0;
    INFO_LHS (result) = NULL;

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
 * @fn node *ASHAdoAdjustShmemAccess( node *syntax_tree)
 *
 * @brief
 *
 *****************************************************************************/
node *
ASHAdoAdjustShmemAccess (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ASHAdoAdjustShmemAccess");

    info = MakeInfo ();
    TRAVpush (TR_asha);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASHAarg( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ASHAarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASHAarg");

    AVIS_SUBST (ARG_AVIS (arg_node)) = NULL;
    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASHAvardec( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ASHAvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASHAvardec");

    AVIS_SUBST (VARDEC_AVIS (arg_node)) = NULL;
    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASHAassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ASHAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASHAassign");

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASHAlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ASHAlet (node *arg_node, info *arg_info)
{
    node *subst_avis;

    DBUG_ENTER ("ASHAlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    /* if the let is of form a = b; */
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_id) {
        if (CUisShmemTypeNew (IDS_NTYPE (LET_IDS (arg_node)))
            && CUisShmemTypeNew (ID_NTYPE (LET_EXPR (arg_node)))) {
            subst_avis = AVIS_SUBST (IDS_AVIS (LET_IDS (arg_node)));
            if (subst_avis != NULL) {
                IDS_AVIS (LET_IDS (arg_node)) = subst_avis;
                AVIS_SUBST (ID_AVIS (LET_EXPR (arg_node))) = subst_avis;
                ID_AVIS (LET_EXPR (arg_node)) = subst_avis;
            } else {
                AVIS_SUBST (ID_AVIS (LET_EXPR (arg_node)))
                  = IDS_AVIS (LET_IDS (arg_node));
                ID_AVIS (LET_EXPR (arg_node)) = IDS_AVIS (LET_IDS (arg_node));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASHAwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ASHAwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASHAwith");

    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_LEVEL (arg_info)++;
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        INFO_LEVEL (arg_info)--;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASHApart( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ASHApart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASHApart");

    PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASHAcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ASHAcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASHAcode");

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASHAprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ASHAprf (node *arg_node, info *arg_info)
{
    node *subst_avis;

    DBUG_ENTER ("ASHAprf");

    /* If we are in cuda withloop */
    if (INFO_LEVEL (arg_info) > 0) {
        if (PRF_PRF (arg_node) == F_idx_modarray_AxSxS
            || PRF_PRF (arg_node) == F_idx_modarray_AxSxA) {
            if (CUisShmemTypeNew (IDS_NTYPE (INFO_LHS (arg_info)))
                && CUisShmemTypeNew (ID_NTYPE (PRF_ARG1 (arg_node)))) {
                subst_avis = AVIS_SUBST (IDS_AVIS (INFO_LHS (arg_info)));
                if (subst_avis != NULL) {
                    IDS_AVIS (INFO_LHS (arg_info)) = subst_avis;
                    AVIS_SUBST (ID_AVIS (PRF_ARG1 (arg_node))) = subst_avis;
                    ID_AVIS (PRF_ARG1 (arg_node)) = subst_avis;
                } else {
                    AVIS_SUBST (ID_AVIS (PRF_ARG1 (arg_node)))
                      = IDS_AVIS (INFO_LHS (arg_info));
                    ID_AVIS (PRF_ARG1 (arg_node)) = IDS_AVIS (INFO_LHS (arg_info));
                }
            }
        } else if (PRF_PRF (arg_node) == F_syncthreads) {
            subst_avis = AVIS_SUBST (IDS_AVIS (INFO_LHS (arg_info)));
            if (subst_avis != NULL) {
                IDS_AVIS (INFO_LHS (arg_info)) = subst_avis;
                AVIS_SUBST (ID_AVIS (PRF_ARG1 (arg_node))) = subst_avis;
                ID_AVIS (PRF_ARG1 (arg_node)) = subst_avis;
            } else {
                AVIS_SUBST (ID_AVIS (PRF_ARG1 (arg_node)))
                  = IDS_AVIS (INFO_LHS (arg_info));
                ID_AVIS (PRF_ARG1 (arg_node)) = IDS_AVIS (INFO_LHS (arg_info));
            }
        } else if (PRF_PRF (arg_node) == F_idx_sel) {
            subst_avis = AVIS_SUBST (ID_AVIS (PRF_ARG2 (arg_node)));
            if (subst_avis != NULL && CUisShmemTypeNew (ID_NTYPE (PRF_ARG2 (arg_node)))) {
                ID_AVIS (PRF_ARG2 (arg_node)) = subst_avis;
            }
        } else {
        }
    }

    DBUG_RETURN (arg_node);
}
