/*
 * $Log$
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

#include "pad.h"
#include "pad_transform.h"

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

/* Nwith-node */
node *
APTNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APTNwith");

    DBUG_PRINT ("AP", ("Nwith-node detected\n"));

    /*  if (FUNDEF_BODY(arg_node) != NULL) {
      FUNDEF_BODY(arg_node) = Trav(FUNDEF_BODY(arg_node), arg_info);

      } */

    DBUG_RETURN (arg_node);
}
