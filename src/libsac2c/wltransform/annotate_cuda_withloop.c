/*
 * $Id: create_cuda_call.c 15999 2009-02-13 18:44:22Z sah $
 *
 * @file create_cuda_call.c
 *
 * This file inserts cuda function call in sac source
 */

#include "annotate_cuda_withloop.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"

#include "globals.h"
#include "dbug.h"
#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "DataFlowMask.h"
#include "NameTuplesUtils.h"
#include "scheduling.h"
#include "wl_bounds.h"
#include "new_types.h"
#include "user_types.h"
#include "shape.h"
#include "LookUpTable.h"
#include "convert.h"
#include "math_utils.h"
#include "types.h"

/*
 * INFO structure
 */
struct INFO {
    node *outerwl;
};

/*
 * INFO macros
 */
#define INFO_OUTERWL(n) (n->outerwl)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_OUTERWL (result) = NULL;
    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

node *
ACUWLdoAnnotateCUDAWL (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ("ACUWLdoAnnotateCUDAWL");

    info = MakeInfo ();
    TRAVpush (TR_acuwl);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

node *
ACUWLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ACUWLassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL)
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ACUWLlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLlet");

    if (INFO_OUTERWL (arg_info) == NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    } else {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ACUWLids (node *arg_node, info *arg_info)
{
    node *ids;
    int dim;

    DBUG_ENTER ("ACUWLids");

    ids = arg_node;

    if (INFO_OUTERWL (arg_info) != NULL) {
        while (ids != NULL) {
            dim = TYgetDim (AVIS_TYPE (IDS_AVIS (ids)));
            if (dim > 0) {
                if (NODE_TYPE (INFO_OUTERWL (arg_info)) == N_with2)
                    WITH2_CUDARIZABLE (INFO_OUTERWL (arg_info)) = FALSE;
                else
                    WITH_CUDARIZABLE (INFO_OUTERWL (arg_info)) = FALSE;
            }
            ids = IDS_NEXT (ids);
        }
    }
    DBUG_RETURN (arg_node);
}

node *
ACUWLcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLcode");

    if (INFO_OUTERWL (arg_info) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
        if (CODE_NEXT (arg_node) != NULL)
            CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

node *
ACUWLwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLwith2");

    if (INFO_OUTERWL (arg_info) == NULL) {
        INFO_OUTERWL (arg_info) = arg_node;
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        INFO_OUTERWL (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
ACUWLwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ACUWLwith");

    if (INFO_OUTERWL (arg_info) == NULL) {
        INFO_OUTERWL (arg_info) = arg_node;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_OUTERWL (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}
