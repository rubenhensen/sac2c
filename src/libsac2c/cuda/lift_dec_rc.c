
#include "lift_dec_rc.h"

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

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool removable;
    node *postassigns;
    node *ap;
    bool from_ap;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_REMOVABLE(n) (n->removable)
#define INFO_POSTASSIGNS(n) (n->postassigns)
#define INFO_AP(n) (n->ap)
#define INFO_FROM_AP(n) (n->from_ap)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_REMOVABLE (result) = FALSE;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_AP (result) = NULL;
    INFO_FROM_AP (result) = FALSE;

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
LDRCdoLiftDecRc (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("LDRCdoLiftDecRc");

    info = MakeInfo ();
    TRAVpush (TR_ldrc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

node *
LDRCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LDRCfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    if ((!FUNDEF_ISCUDAGLOBALFUN (arg_node) && !FUNDEF_ISCUDASTGLOBALFUN (arg_node))
        || INFO_FROM_AP (arg_info)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
LDRCassign (node *arg_node, info *arg_info)
{
    node *fundef;
    node *lifted_assign;

    DBUG_ENTER ("LDRCassign");

    fundef = INFO_FUNDEF (arg_info);

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_REMOVABLE (arg_info)) {
        lifted_assign = arg_node;
        arg_node = ASSIGN_NEXT (lifted_assign);
        ASSIGN_NEXT (lifted_assign) = NULL;
        INFO_POSTASSIGNS (arg_info)
          = TCappendAssign (lifted_assign, INFO_POSTASSIGNS (arg_info));

        INFO_REMOVABLE (arg_info) = FALSE;
    } else if (!FUNDEF_ISCUDAGLOBALFUN (fundef) && !FUNDEF_ISCUDASTGLOBALFUN (fundef)
               && INFO_POSTASSIGNS (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_POSTASSIGNS (arg_info), ASSIGN_NEXT (arg_node));
        INFO_POSTASSIGNS (arg_info) = NULL;
    } else {
        /* Do nothing */
    }

    DBUG_RETURN (arg_node);
}

node *
LDRCap (node *arg_node, info *arg_info)
{
    node *old_fundef;
    node *fundef;

    DBUG_ENTER ("LDRCap");

    fundef = AP_FUNDEF (arg_node);
    old_fundef = INFO_FUNDEF (arg_info);

    if (fundef != NULL
        && (FUNDEF_ISCUDAGLOBALFUN (fundef) || FUNDEF_ISCUDASTGLOBALFUN (fundef))) {
        INFO_FROM_AP (arg_info) = TRUE;
        INFO_AP (arg_info) = arg_node;
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        INFO_AP (arg_info) = NULL;
        INFO_FROM_AP (arg_info) = FALSE;
    }

    INFO_FUNDEF (arg_info) = old_fundef;

    DBUG_RETURN (arg_node);
}

node *
LDRCprf (node *arg_node, info *arg_info)
{
    node *fundef, *ap;
    node *fundef_args, *ap_args;
    node *array_id;

    DBUG_ENTER ("LDRCprf");

    fundef = INFO_FUNDEF (arg_info);
    fundef_args = FUNDEF_ARGS (fundef);

    if (PRF_PRF (arg_node) == F_dec_rc) {
        if (FUNDEF_ISCUDAGLOBALFUN (fundef) || FUNDEF_ISCUDASTGLOBALFUN (fundef)) {
            ap = INFO_AP (arg_info);
            ap_args = AP_ARGS (ap);
            array_id = PRF_ARG1 (arg_node);
            if (CUisDeviceArrayTypeNew (AVIS_TYPE (ID_AVIS (array_id)))) {
                DBUG_ASSERT (NODE_TYPE (AVIS_DECL (ID_AVIS (array_id))) == N_arg,
                             "First argument of removable F_dec_rc is not a kernel "
                             "paramenter!");
                while (fundef_args != NULL && ap_args != NULL) {
                    if (fundef_args == AVIS_DECL (ID_AVIS (array_id)))
                        break;
                    fundef_args = ARG_NEXT (fundef_args);
                    ap_args = EXPRS_NEXT (ap_args);
                }

                ID_AVIS (array_id) = ID_AVIS (EXPRS_EXPR (ap_args));

                printf ("Lift dec_rc: %s\n", ID_NAME (array_id));

                INFO_REMOVABLE (arg_info) = TRUE;
            }
        }
    }

    DBUG_RETURN (arg_node);
}
