/*
 * $Log$
 * Revision 1.1  2004/09/20 19:55:13  sah
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include "tree_basic.h"
#include "filemgr.h"
#include "stdio.h"
#include "serialize_info.h"

/*
 * INFO functions
 */

info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_SER_FILE (result) = NULL :

      DBUG_RETURN (result);
}

info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}
