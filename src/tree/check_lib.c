/*
 * Revision 1.0  2004/11/22 13:50:34  jhb
 * first functions inserted
 *
 *
 */

#define NEW_INFO
#include "print.h"
#include "free.h"
#include "internal_lib.h"

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

node *
CHKexistChild (node *child, node *parent, char *string)
{
    DBUG_ENTER ("CHKExistChild");

    if (child == NULL) {

        NODE_ERRMSG (parent) = StringConcat (NODE_ERRMSG (parent), string);
    }

    DBUG_RETURN (child);
}

node *
CHKexistAttribute (node *attribute, node *parent, char *string)
{
    DBUG_ENTER ("CHKExistAttribute");

    if (attribute == NULL) {

        NODE_ERRMSG (parent) = StringConcat (NODE_ERRMSG (parent), string);
    }

    DBUG_RETURN (attribute);
}

node *
CHKrightType (node *attribute, node *parent, char *type, char *string)
{
    DBUG_ENTER ("CHKRightType");

    //  if (String(attribute(type)) != type) {
    //  NODE_ERRMSG(parent) = StringConcat(NODE_ERRMSG(parent), string);
    //  }

    DBUG_ENTER (attribute);
}
