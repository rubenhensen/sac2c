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
CheckApIds (node *ids)
{
    bool res = TRUE;
    ntype *type;

    DBUG_ENTER ("CheckApIds");

    while (ids != NULL) {
        type = IDS_NTYPE (ids);
        res = res && (TUisScalar (type) || TYisAKS (type) || TYisAKD (type))
              && !AVIS_ISHOSTREFERENCED (IDS_AVIS (ids));
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMdoTagExecutionmode(node *arg_node, info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
TCULACdoTagCudaLac (node *lac_ap, node *ap_ids, node *fundef_args)
{
    info *arg_info;
    node *fundef;
    bool ids_args_ok = TRUE;

    DBUG_ENTER ("TCULACdoTagCudaLac");

    fundef = AP_FUNDEF (lac_ap);
    DBUG_ASSERT (fundef != NULL, "Found a lac fun with empty fundef!");

    ids_args_ok = CheckApIds (ap_ids);

    if (ids_args_ok) {

        arg_info = MakeInfo ();

        TRAVpush (TR_tculac);
        fundef = TRAVdo (fundef, arg_info);
        TRAVpop ();

        FUNDEF_ISCUDALACFUN (fundef) = INFO_ISCUDARIZABLE (arg_info);

        arg_info = FreeInfo (arg_info);
    } else {
        FUNDEF_ISCUDALACFUN (fundef) = FALSE;
    }

    DBUG_RETURN (lac_ap);
}

node *
TCULACfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TCULACfundef");

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
TCULACap (node *arg_node, info *arg_info)
{
    node *fundef;

    DBUG_ENTER ("TCULACap");

    DBUG_ASSERT ((FUNDEF_ISCONDFUN (fundef) || FUNDEF_ISDOFUN (fundef)),
                 "TCULAC traverses non-lac function!");

    fundef = AP_FUNDEF (arg_node);

    if (fundef != NULL) {
        if (FUNDEF_ISCONDFUN (fundef)) {
            fundef = TRAVdo (fundef, arg_info);
        } else if (FUNDEF_ISDOFUN (fundef)) {
            INFO_ISCUDARIZABLE (arg_info) = FALSE;
        } else {
            INFO_ISCUDARIZABLE (arg_info) = FALSE;
        }
    }

    DBUG_RETURN (arg_node);
}

node *
TCULACwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TCULACwith");

    INFO_ISCUDARIZABLE (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}
