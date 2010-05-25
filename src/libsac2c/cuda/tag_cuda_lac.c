/** <!--********************************************************************-->
 *
 * @file cuda_TAG_executionmode.c
 *
 * prefix: CUTEM
 *
 * description:
 *   Tags the assignments, whether their executionmode is CUDA_HOST_SINGLE,
 *   CUDA_DEVICE_SINGLE, or CUDA_DEVICE_MULTI
 *
 *****************************************************************************/

#include "tag_cuda_lac.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "dbug.h"
#include "globals.h"
#include "type_utils.h"
#include "new_types.h"

/*
 * INFO structure
 */
struct INFO {
    bool cudarizable;
};

#define INFO_CUDARIZABLE(n) (n->cudarizable)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_CUDARIZABLE (result) = TRUE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static bool
CheckIds (node *ids)
{
    bool res = TRUE;
    ntype *type

      DBUG_ENTER ("CheckIds");

    while (ids != NULL) {
        type = IDS_NTYPE (ids);
        res = res && (TUisScalar (type) || TYisAKS (type) || TYisAKD (type));
    }

    DBUG_RETURN (res);
}

node *
TCULACdoTagCudaLac (node *funap, node *ids, node *fundef_args)
{
    bool ids_ok;

    DBUG_ENTER ("TCULACdoTagCudaLac");

    ids_ok = CheckIds (ids);

    DBUG_RETURN (funap);
}

node *
TCULACfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTEMfundef");
    DBUG_RETURN (arg_node);
}

node *
TCULACwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTEMwith");
    DBUG_RETURN (arg_node);
}

node *
TCULACap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTEMap");
    DBUG_RETURN (arg_node);
}
