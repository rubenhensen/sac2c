/*
 * $Log$
 * Revision 1.1  2005/09/02 14:25:16  ktr
 * Initial revision
 *
 */

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "internal_lib.h"

#include "liftoptflags.h"

/*
 * INFO structure
 */
struct INFO {
    bool optflag;
    node *fundef;
};

/*
 * INFO macros
 */
#define INFO_OPTFLAG(n) (n->optflag)
#define INFO_FUNDEF(n) (n->fundef)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_OPTFLAG (result) = FALSE;
    INFO_FUNDEF (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *LOFdoLiftOptFlags( node *arg_node)
 *
 *****************************************************************************/
node *
LOFdoLiftOptFlags (node *arg_node)
{
    DBUG_ENTER ("LOFdoLiftOptFlags");

    TRAVpush (TR_lof);

    if (arg_node != NULL) {
        arg_node = TRAVdo (arg_node, NULL);
    }

    TRAVpop ();

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *LOFfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LOFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LOFfundef");

    if (!FUNDEF_ISLACFUN (arg_node)) {
        arg_info = MakeInfo ();
    }

    if (arg_info != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        INFO_OPTFLAG (arg_info) |= FUNDEF_WASOPTIMIZED (arg_node);

        if ((!INFO_OPTFLAG (arg_info)) && (FUNDEF_BODY (arg_node) != NULL)) {

            DBUG_PRINT ("LOF", ("%s was is yet marked for further optimization!\n",
                                FUNDEF_NAME (arg_node)));

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            FUNDEF_WASOPTIMIZED (arg_node) |= INFO_OPTFLAG (arg_info);
            if (FUNDEF_WASOPTIMIZED (arg_node)) {
                DBUG_PRINT ("LOF", ("%s is now marked for further optimization!\n",
                                    FUNDEF_NAME (arg_node)));
            }
        }
    }

    if (!FUNDEF_ISLACFUN (arg_node)) {
        arg_info = FreeInfo (arg_info);
    }

    if ((!FUNDEF_ISLACFUN (arg_node)) || (arg_info == NULL)) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *LOFassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LOFassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LOFassign");

    if (!INFO_OPTFLAG (arg_info)) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *LOFap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LOFap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LOFap");

    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {

        node *oldfundef = INFO_FUNDEF (arg_info);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = oldfundef;
    }

    DBUG_RETURN (arg_node);
}
