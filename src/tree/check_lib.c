
#define NEW_INFO
#include "globals.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "internal_lib.h"

struct INFO {
};

static info *
MakeInfo ()
{
    info *result;
    DBUG_ENTER ("MakeInfo");
    result = Malloc (sizeof (info));
    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");
    info = Free (info);
    DBUG_RETURN (info);
}

node *
CHKExistChild (node *child, node *parent, char *string)
{
    DBUG_ENTER ("CHKExistChild");

    if (child == NULL) {

        NODE_ERRMSG (parent) = StringConcat (NODE_ERRMSG (parent), string);
    }

    DBUG_RETURN (child);
}

node *
CHKExistAttribute (node *attribute, node *parent, char *string)
{
    DBUG_ENTER ("CHKExistAttribute");

    if (attribute == NULL) {

        NODE_ERRMSG (parent) = StringConcat (NODE_ERRMSG (parent), string);
    }

    DBUG_RETURN (attribute);
}

node *
CHKRightType (node *attribute, node *parent, char *type, char *string)
{
    DBUG_ENTER ("CHKRightType");

    //  if (String(attribute(type)) != type) {
    //  NODE_ERRMSG(parent) = StringConcat(NODE_ERRMSG(parent), string);
    //  }

    DBUG_ENTER (attribute);
}
