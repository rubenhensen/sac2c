
/**
 *
 * @defgroup inb Indentify noop conditional branch
 *
 * @ingroup mm
 *
 * @{
 */

/**
 *
 * @file reusewithoffset.c
 *
 * Prefix: RWO
 */
#include "reusewithoffset.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "RWO"
#include "debug.h"

#include "print.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "new_types.h"

/*
 * INFO structure
 */
struct INFO {
    node *cond;
};

/*
 * INFO macros
 */
#define INFO_COND(n) (n->cond)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_COND (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static bool
IsInplaceSelect (node *id)
{
    bool res = FALSE;
    node *cass;

    DBUG_ENTER ();

    cass = AVIS_SSAASSIGN (ID_AVIS (id));

    if ((cass != NULL) && (NODE_TYPE (ASSIGN_RHS (cass)) == N_prf)
        && (PRF_PRF (ASSIGN_RHS (cass)) == F_sel_VxA)) {
        res = PRF_ISINPLACESELECT (ASSIGN_RHS (cass));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *INBdoIdentifyNoopBranch( node *syntax_tree)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
INBdoIdentifyNoopBranch (node *hotpart)
{
    info *info;
    node *hotpartnext;

    DBUG_ENTER ();

    DBUG_ASSERT (!PART_ISCOPY (hotpart), "Copy partition found!");

    info = MakeInfo ();

    hotpartnext = PART_NEXT (hotpart);
    PART_NEXT (hotpart) = NULL;

    TRAVpush (TR_inb);
    hotpart = TRAVdo (hotpart, info);
    TRAVpop ();

    PART_NEXT (hotpart) = hotpartnext;

    info = FreeInfo (info);

    DBUG_RETURN (hotpart);
}

/** <!--********************************************************************-->
 *
 * @fn node *INBfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
INBfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (FUNDEF_ISCONDFUN (arg_node),
                 "Only conditional function can be traversed!");

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *INBap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
INBap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* We only look at conditional function */
    if (AP_FUNDEF (arg_node) != NULL && FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        AP_FUNDEF (arg_node) = TRAVopt (AP_FUNDEF (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *INBfuncond( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
INBfuncond (node *arg_node, info *arg_info)
{
    node *_then, *_else;

    DBUG_ENTER ();

    _then = FUNCOND_THEN (arg_node);
    _else = FUNCOND_ELSE (arg_node);

    DBUG_ASSERT (NODE_TYPE (_then) == N_id && NODE_TYPE (_else) == N_id,
                 "Both then and else of N_funcond must be N_id nodes!");

    COND_ISTHENNOOP (INFO_COND (arg_info)) = IsInplaceSelect (_then);
    COND_ISELSENOOP (INFO_COND (arg_info)) = IsInplaceSelect (_else);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *INBcond( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
INBcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVopt (COND_ELSE (arg_node), arg_info);
    INFO_COND (arg_info) = arg_node;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *INBassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
INBassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *INBcode( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
INBcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *INBpart( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
INBpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
