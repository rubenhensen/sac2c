/*
 * $Log$
 * Revision 1.1  2000/07/21 08:18:37  nmw
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "convert.h"
#include "filemgr.h"
#include "globals.h"
#include "free.h"
#include "resource.h"
#include "import_specialization.h"

node *
IMPSPECfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IMPSPECfundef");

    DBUG_RETURN (arg_node);
}

node *
IMPSPECmodspec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IMPSPECmodspec");

    DBUG_RETURN (arg_node);
}

node *
IMPSPECarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IMPSPECarg");

    DBUG_RETURN (arg_node);
}

node *
ImportSpecialization (node *syntax_tree)
{
    DBUG_ENTER ("ImportSpecialization");

    DBUG_RETURN (syntax_tree);
}
