/*****************************************************************************
 * author: michael
 * function: to find all the OpenMP private variables
 * date: 20100620
 *
 *****************************************************************************/

#include "omp_find_private.h"

#define DBUG_PREFIX "OFP"
#include "debug.h"

#include "memory.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "tree_basic.h"
#include "str.h"
#include "str_buffer.h"
#include "DupTree.h"
#include "namespaces.h"
#include "node_basic.h"
#include "traverse.h"
#include "cleanup_decls.h"
#include "remove_dfms.h"
#include "tree_compound.h"
#include "infer_dfms.h"
#include "free.h"
#include "new_types.h"

/*
 * INFO structure
 */
struct INFO {
    int num_with_loop;
    bool in_with_loop_id;
    char *omp_private_list;
    char *omp_reducation_var;
};

#define INFO_NUM_WITH_LOOP(n) ((n)->num_with_loop)
#define INFO_IN_WITH_LOOP_ID(n) ((n)->in_with_loop_id)
#define INFO_OMP_PRIVATE_LIST(n) ((n)->omp_private_list)
#define INFO_OMP_REDUCTION_VAR(n) ((n)->omp_reducation_var)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_NUM_WITH_LOOP (result) = 0;
    INFO_IN_WITH_LOOP_ID (result) = FALSE;
    INFO_OMP_PRIVATE_LIST (result) = NULL;
    INFO_OMP_REDUCTION_VAR (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

node *
OFPdoFindPrivate (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_ofp);

    arg_node = TRAVdo (arg_node, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

node *
OFPmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_TYPES (arg_node) = TRAVopt (MODULE_TYPES (arg_node), arg_info);
    MODULE_OBJS (arg_node) = TRAVopt (MODULE_OBJS (arg_node), arg_info);
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    MODULE_FUNDECS (arg_node) = TRAVopt (MODULE_FUNDECS (arg_node), arg_info);
    MODULE_THREADFUNS (arg_node) = TRAVopt (MODULE_THREADFUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
OFPlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * if it is fold operation, the ids should be tagged as OpenMP reduction variable
     */
    if ((NODE_TYPE (LET_EXPR (arg_node)) == N_with2)
        && (NODE_TYPE (WITH2_WITHOP (LET_EXPR (arg_node))) == N_fold)) {
        DBUG_PRINT ("the ids should be tagged as OpenMP reduction var\n");
        AVIS_ISOMPREDUCTION (IDS_AVIS (LET_IDS (arg_node))) = TRUE;
        INFO_OMP_REDUCTION_VAR (arg_info) = AVIS_NAME (IDS_AVIS (LET_IDS (arg_node)));
    }

    if ((INFO_NUM_WITH_LOOP (arg_info) > 0) && (LET_IDS (arg_node) != NULL)) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    LET_EXPR (arg_node) = TRAVopt(LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
OFPwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_NUM_WITH_LOOP (arg_info)++;

    WITH2_WITHID (arg_node) = TRAVopt(WITH2_WITHID (arg_node), arg_info);

    WITH2_CODE (arg_node) = TRAVopt(WITH2_CODE (arg_node), arg_info);

    INFO_NUM_WITH_LOOP (arg_info)--;

    DBUG_PRINT ("OpenMP reducation var %s\n", INFO_OMP_REDUCTION_VAR (arg_info));
    DBUG_PRINT ("OpenMP private list %s\n", INFO_OMP_PRIVATE_LIST (arg_info));

    WITH2_OMP_PRIVATE_LIST (arg_node) = INFO_OMP_PRIVATE_LIST (arg_info);
    INFO_OMP_PRIVATE_LIST (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

node *
OFPwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_IN_WITH_LOOP_ID (arg_info) = TRUE;

    WITHID_VEC (arg_node) = TRAVopt(WITHID_VEC (arg_node), arg_info);

    WITHID_IDS (arg_node) = TRAVopt(WITHID_IDS (arg_node), arg_info);

    WITHID_IDXS (arg_node) = TRAVopt(WITHID_IDXS (arg_node), arg_info);

    INFO_IN_WITH_LOOP_ID (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

node *
OFPwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

node *
OFPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNDEF_BODY (arg_node) = TRAVopt(FUNDEF_BODY (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/*
 * inside the code of WITH-loop, whenever we encounter an assignment
 * we set the left hand side variable as OpenMP private variable
 */
node *
OFPids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_NUM_WITH_LOOP (arg_info) > 0) {
        if ((!AVIS_ISOMPPRIVATE (IDS_AVIS (arg_node)))
            && (!AVIS_ISOMPREDUCTION (IDS_AVIS (arg_node)))) {
            AVIS_ISOMPPRIVATE (IDS_AVIS (arg_node)) = TRUE;

            if (INFO_OMP_PRIVATE_LIST (arg_info) == NULL) {
                INFO_OMP_PRIVATE_LIST (arg_info)
                  = STRcat (INFO_OMP_PRIVATE_LIST (arg_info),
                            AVIS_NAME (IDS_AVIS (arg_node)));
            } else {
                INFO_OMP_PRIVATE_LIST (arg_info)
                  = STRcat (INFO_OMP_PRIVATE_LIST (arg_info),
                            STRcat (",", AVIS_NAME (IDS_AVIS (arg_node))));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 * inside WITHID, whenever we encounter a variable
 * we set it as OpenMP private variable
 */
node *
OFPid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_IN_WITH_LOOP_ID (arg_info)) {
        if (!AVIS_ISOMPPRIVATE (ID_AVIS (arg_node))) {
            AVIS_ISOMPPRIVATE (ID_AVIS (arg_node)) = TRUE;

            if (INFO_OMP_PRIVATE_LIST (arg_info) == NULL) {
                INFO_OMP_PRIVATE_LIST (arg_info)
                  = STRcat (INFO_OMP_PRIVATE_LIST (arg_info),
                            AVIS_NAME (ID_AVIS (arg_node)));
            } else {
                INFO_OMP_PRIVATE_LIST (arg_info)
                  = STRcat (INFO_OMP_PRIVATE_LIST (arg_info),
                            STRcat (",", AVIS_NAME (ID_AVIS (arg_node))));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
