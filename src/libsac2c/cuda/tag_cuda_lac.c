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
    bool iscudarizable;
};

#define INFO_ISCUDARIZABLE(n) (n->iscudarizable)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ISCUDARIZABLE (result) = TRUE;

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
    ntype *type;

    DBUG_ENTER ("CheckIds");

    while (ids != NULL) {
        type = IDS_NTYPE (ids);
        res = res && (TUisScalar (type) || TYisAKS (type) || TYisAKD (type));
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (res);
}

node *
TCULACdoTagCudaLac (node *funap, node *ids, node *fundef_args)
{
    bool ids_ok;
    node *fundef;
    info *arg_info;

    DBUG_ENTER ("TCULACdoTagCudaLac");

    fundef = AP_FUNDEF (funap);

    ids_ok = CheckIds (ids);

    if (ids_ok) {

        TRAVpush (TR_tculac);
        arg_info = MakeInfo ();

        fundef = TRAVdo (fundef, arg_info);
        FUNDEF_ISCUDALACFUN (fundef) = INFO_ISCUDARIZABLE (arg_info);

        arg_info = FreeInfo (arg_info);
        TRAVpop ();
    } else {
        FUNDEF_ISCUDALACFUN (fundef) = FALSE;
    }

    DBUG_RETURN (funap);
}

node *
TCULACfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTEMfundef");

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
TCULACwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTEMwith");

    INFO_ISCUDARIZABLE (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

node *
TCULACap (node *arg_node, info *arg_info)
{
    node *fundef;

    DBUG_ENTER ("CUTEMap");

    fundef = AP_FUNDEF (arg_node);

    if (fundef != NULL) {
        if (FUNDEF_ISDOFUN (fundef)) {
            INFO_ISCUDARIZABLE (arg_info) = FALSE;
        } else if (FUNDEF_ISCONDFUN (fundef)) {
            fundef = TRAVdo (fundef, arg_info);
        } else {
            INFO_ISCUDARIZABLE (arg_info) = FALSE;
        }
    }

    DBUG_RETURN (arg_node);
}
