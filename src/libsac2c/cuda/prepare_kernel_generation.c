
#include "prepare_kernel_generation.h"

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
#include "deadcoderemoval.h"
#include "NumLookUpTable.h"
#include "cuda_utils.h"
#include "constants.h"

/*
 * INFO structure
 */
struct INFO {
    node *preassign;
    bool incudawl;
};

/*
 * INFO macros
 */
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_INCUDAWL(n) (n->incudawl)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_PREASSIGN (result) = NULL;
    INFO_INCUDAWL (result) = FALSE;

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
PKNLGdoPrepareKernelGeneration (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("PKNLGdoPrepareKernelGeneration");

    info = MakeInfo ();
    TRAVpush (TR_pknlg);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

node *
PKNLGassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PKNLGassign");

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
PKNLGwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PKNLGwith");

    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_INCUDAWL (arg_info) = TRUE;
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
        INFO_INCUDAWL (arg_info) = FALSE;
    } else if (INFO_INCUDAWL (arg_info)) {
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
    } else {
        /* Not cudarizable, not in cuda withloop, ignore */
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PKNLGprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PKNLGprf (node *arg_node, info *arg_info)
{
    node *id, *avis;

    DBUG_ENTER ("PKNLGprf");

    if (INFO_INCUDAWL (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_sel_VxA:
            id = PRF_ARG2 (arg_node);
            DBUG_ASSERT ((NODE_TYPE (id) == N_id), "2nd arg of F_sel_VxA is no N_id!");

            avis = ID_AVIS (id);

            printf ("Checking varaible %s\n", ID_NAME (id));
            if (TYisAKV (AVIS_TYPE (avis))) {
                INFO_PREASSIGN (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (ID_AVIS (id), NULL),
                                             COconstant2AST (
                                               TYgetValue (AVIS_TYPE (avis)))),
                                  NULL);
                printf ("Add constant assignment before sel_VxA\n");
            }
            break;
        default:
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
            break;
        }
    }

    DBUG_RETURN (arg_node);
}
