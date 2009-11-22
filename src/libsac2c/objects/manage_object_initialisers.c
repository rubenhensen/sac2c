/* $Id$ */

#include "manage_object_initialisers.h"

#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "namespaces.h"
#include "ctinfo.h"

/*
 * INFO structure
 */
struct INFO {
    node *args;
    bool delete;
};

/*
 * INFO macros
 */
#define INFO_ARGS(n) ((n)->args)
#define INFO_DELETE(n) ((n)->delete)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ARGS (result) = NULL;
    INFO_DELETE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * helper functions
 */
static bool
ArgsContainAvis (node *args, node *avis)
{
    bool result;

    DBUG_ENTER ("ArgsContainAvis");

    if (args == NULL) {
        result = FALSE;
    } else {
        result = (ARG_AVIS (args) == avis) || ArgsContainAvis (ARG_NEXT (args), avis);
    }

    DBUG_RETURN (result);
}

/*
 * traversal functions
 */
node *
MOIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MOIassign");

    /*
     * we do it bottom up
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /*
     * check whether rhs contains a reference to one of
     * the args
     */
    INFO_DELETE (arg_info) = FALSE;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_DELETE (arg_info)) {
        arg_node = FREEdoFreeNode (arg_node);
        INFO_DELETE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

node *
MOIid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MOIassign");

    INFO_DELETE (arg_info)
      = INFO_DELETE (arg_info)
        || ArgsContainAvis (INFO_ARGS (arg_info), ID_AVIS (arg_node));

    DBUG_RETURN (arg_node);
}

node *
MOIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MOIassign");

    if (FUNDEF_ISOBJINITFUN (arg_node)) {
        DBUG_PRINT ("MOI", (">>> entering fundef %s", CTIitemName (arg_node)));

        /*
         * this is an init function, so we have to remove all lhs references
         * to the arguments in the body!
         */
        if (FUNDEF_BODY (arg_node) != NULL) {
            INFO_ARGS (arg_info) = FUNDEF_ARGS (arg_node);
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            INFO_ARGS (arg_info) = NULL;
        }

        DBUG_PRINT ("MOI", ("<<< leaving fundef %s", CTIitemName (arg_node)));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 * Traversal start function
 */
node *
MOIdoManageObjectInitialisers (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("MOIdoManageObjectInitialisers");

    info = MakeInfo ();
    TRAVpush (TR_moi);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
