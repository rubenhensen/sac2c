/*
 *
 * $Log$
 * Revision 1.14  2005/07/13 11:53:42  jhb
 * some little fixes
 *
 * Revision 1.13  2005/06/15 12:43:03  jhb
 * little fixes
 *
 * Revision 1.12  2005/06/08 13:37:08  jhb
 * attribute are now check correctly
 *
 * Revision 1.11  2005/05/19 13:34:44  jhb
 * begin to add the ranges for the attributes
 *
 * Revision 1.10  2005/03/03 16:44:01  jhb
 * cosmetics
 *
 * Revision 1.9  2005/02/16 14:35:23  jhb
 * test something at the three functions
 *
 * Revision 1.8  2005/02/14 14:08:48  jhb
 * change name
 *
 * Revision 1.7  2005/02/11 15:03:00  jhb
 * fix some bugs
 *
 * Revision 1.6  2005/02/11 14:48:56  jhb
 * change CHKdoCheck in CHKdoTreeCheck
 *
 * Revision 1.5  2005/02/10 14:08:49  jhb
 * change the revisionslog
 *
 * Revision 1.0  2004/11/22 13:50:34  jhb
 * first functions inserted
 *
 */

#define NEW_INFO

#include "check_lib.h"

#include "print.h"
#include "free.h"
#include "internal_lib.h"
#include "traverse.h"
#include "tree_basic.h"
#include "types.h"
#include "tree_compound.h"
#include "checktst.h"

struct INFO {
};

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

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
 * @fn node *CHKdoTreeCheck( node *syntax_tree)
 *
 *****************************************************************************/
node *
CHKdoTreeCheck (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CHKdoTreeCheck");

    DBUG_PRINT ("CHK", ("Starting the Checkmechanism"));

    info = MakeInfo ();

    syntax_tree = CHKTSTdoTreeCheckTest (syntax_tree);

    TRAVpush (TR_chk);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("CHK", ("Checkmechanism complete"));

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *CHKinsertError( node *arg_node, char *string)
 *****************************************************************************/
static node *
CHKinsertError (node *arg_node, char *string)
{
    DBUG_ENTER ("CHKinsertError");

    if (arg_node == NULL) {

        printf ("%s\n", string);
        arg_node = TBmakeError (ILIBstringCopy (string), arg_node);
    } else {

        if (!(ILIBstringCompare (string, ERROR_MESSAGE (arg_node)))) {

            ERROR_NEXT (arg_node) = CHKinsertError (ERROR_NEXT (arg_node), string);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CHKexistSon( node *child, node *arg_node, char *string)
 *
 *****************************************************************************/
node *
CHKexistSon (node *son, node *arg_node, char *string)
{
    DBUG_ENTER ("CHKexistSon");

    if (son == NULL) {

        NODE_ERROR (arg_node) = CHKinsertError (NODE_ERROR (arg_node), string);
    }

    DBUG_RETURN (son);
}

/** <!--********************************************************************-->
 *
 * @fn node *CHKexistAttribute( node *attribute, node *arg_node, char *string)
 *
 *****************************************************************************/
node *
CHKexistAttribute (void *attribute, node *arg_node, char *string)
{
    DBUG_ENTER ("CHKexistAttribute");

    if (attribute == NULL) {

        NODE_ERROR (arg_node) = CHKinsertError (NODE_ERROR (arg_node), string);
    }

    DBUG_RETURN (attribute);
}

/** <!--********************************************************************-->
 *
 * @fn node *CHKnotExistAttribute( node *attribute, node *arg_node, char *string)
 *
 *****************************************************************************/

node *
CHKnotExistAttribute (void *attribute, node *arg_node, char *string)
{
    DBUG_ENTER ("CHKexistAttribute");

    if (attribute != NULL) {

        NODE_ERROR (arg_node) = CHKinsertError (NODE_ERROR (arg_node), string);
    }

    DBUG_RETURN (attribute);
}

node *
CHKcorrectTypeInsertError (node *arg_node, char *string)
{
    DBUG_ENTER ("CHKcorrectType");

    NODE_ERROR (arg_node) = CHKinsertError (NODE_ERROR (arg_node), string);

    DBUG_RETURN (arg_node);
}
