/*
 * $Log$
 * Revision 1.3  2000/06/08 11:14:14  mab
 * added functions for arg, vardec, array
 *
 * Revision 1.2  2000/05/31 16:16:58  mab
 * initial version
 *
 * Revision 1.1  2000/05/26 13:42:33  sbs
 * Initial revision
 *
 *
 */

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"

#include "pad_info.h"
#include "pad_transform.h"

#include "my_debug.h"

/* main function */
void
APtransform (node *arg_node)
{

    node *arg_info;
    funtab *tmp_tab;

    DBUG_ENTER ("APtransform");

    DBUG_PRINT ("AP", ("Array Padding: applying transformation..."));

    tmp_tab = act_tab;
    act_tab = apt_tab;

    arg_info = MakeInfo ();

    arg_node = Trav (arg_node, arg_info);

    FREE (arg_info);
    act_tab = tmp_tab;

    DBUG_VOID_RETURN;
}

node *
APTarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APTarg");

    DBUG_PRINT ("AP", ("arg-node detected"));
    if (ARG_NAME (arg_node) != NULL)
        DBUG_PRINT ("AP", ("NAME=%s", ARG_NAME (arg_node)));
    else
        DBUG_PRINT ("AP", ("NAME=(NULL)"));
    DBUG_PRINT ("AP", ("TYPE=%s\n", mdb_type[TYPES_BASETYPE (ARG_TYPE (arg_node))]));

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
APTvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APTvardec");

    DBUG_PRINT ("AP", ("vardec-node detected"));
    if (VARDEC_NAME (arg_node) != NULL)
        DBUG_PRINT ("AP", ("NAME=%s", VARDEC_NAME (arg_node)));
    else
        DBUG_PRINT ("AP", ("NAME=(NULL)s"));
    DBUG_PRINT ("AP", ("TYPE=%s\n", mdb_type[TYPES_BASETYPE (VARDEC_TYPE (arg_node))]));

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
APTarray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APTarray");

    DBUG_PRINT ("AP", ("array-node detected\n"));

    DBUG_RETURN (arg_node);
}

node *
APTNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APTNwith");

    DBUG_PRINT ("AP", ("Nwith-node detected\n"));

    DBUG_RETURN (arg_node);
}
