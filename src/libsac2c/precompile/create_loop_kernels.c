
#include "create_loop_kernels.h"

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
#include "DataFlowMaskUtils.h"
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
#include "infer_dfms.h"

/*
 * INFO structure
 */
struct INFO {
    bool incudadoloop;
    node *with;
};

/*
 * INFO macros
 */
#define INFO_INCUDADOLOOP(n) (n->incudadoloop)
#define INFO_WITH(n) (n->with)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_INCUDADOLOOP (result) = FALSE;
    INFO_WITH (result) = NULL;

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
CULKNLdoCreateCudaLoopKernels (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CULKNLdoCreateCudaLoopKernels");

    /*
     * Infer dataflow masks
     */
    syntax_tree = INFDFMSdoInferDfms (syntax_tree, HIDE_LOCALS_NEVER);

    info = MakeInfo ();
    TRAVpush (TR_culknl);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

node *
CULKNLdo (node *arg_node, info *arg_info)
{
    bool old_incudadoloop;

    DBUG_ENTER ("CULKNLdo");

    if (DO_ISCUDARIZABLE (arg_node)) {

        FILE *fout = fopen ("maskout.c", "a");

        if (STReq (DO_LABEL (arg_node), "_dup_13193____f2l_10925_label")) {
            DFMprintMask (fout, "%s\n", DO_IN_MASK (arg_node));
            // DFMprintMaskDetailed( fout, DO_OUT_MASK( arg_node));
            // DFMprintMaskDetailed( fout, DO_LOCAL_MASK( arg_node));
        }

        old_incudadoloop = INFO_INCUDADOLOOP (arg_info);
        INFO_INCUDADOLOOP (arg_info) = TRUE;
        DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);
        INFO_INCUDADOLOOP (arg_info) = old_incudadoloop;
    } else {
        DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
