/*
 * $Log$
 * Revision 1.8  2000/07/21 14:43:43  mab
 * added APCcode and APCwithop dummies
 *
 * Revision 1.7  2000/07/19 12:37:30  mab
 * added AccessClass2Group
 *
 * Revision 1.6  2000/06/29 10:23:05  mab
 * renamed APCNwith to APCwith
 *
 * Revision 1.5  2000/06/15 14:38:01  mab
 * dummies for APC block and let added
 *
 * Revision 1.4  2000/06/14 10:43:19  mab
 * dummies for APC ap, exprs, id, prf, fundef added
 *
 * Revision 1.3  2000/06/08 11:13:37  mab
 * added functions for nodes arg, vardec, array
 *
 * Revision 1.2  2000/05/31 16:16:58  mab
 * initial version
 *
 * Revision 1.1  2000/05/26 13:42:00  sbs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   pad_collect.c
 *
 * prefix: APC
 *
 * description:
 *
 *   This compiler module collects information needed to infer new array
 *   shapes for the inference-phase.
 *
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"

#include "pad_info.h"
#include "pad_collect.h"

/*****************************************************************************
 *
 * function:
 *   void APcollect (node *arg_node)
 *
 * description:
 *   main function for collection-phase of array padding
 *
 *****************************************************************************/
/* main function */
void
APcollect (node *arg_node)
{

    node *arg_info;
    funtab *tmp_tab;

    DBUG_ENTER ("APcollect");

    DBUG_PRINT ("APC", ("Array Padding: collecting data..."));

    tmp_tab = act_tab;
    act_tab = apc_tab;

    arg_info = MakeInfo ();

    arg_node = Trav (arg_node, arg_info);

    FREE (arg_info);
    act_tab = tmp_tab;

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   static shpseg* AccessClass2Group(accessclass_t class, int dim)
 *
 * description:
 *   convert access class into vector of integer factors
 *   accessing with ACL_const means [0]*index+offset
 *   accessing with ACL_offset means [1]*index+offset
 *   currently no other access classes are supported
 *   result is NULL, if access class is unsupported
 *
 *****************************************************************************/

static shpseg *
AccessClass2Group (accessclass_t class, int dim)
{

    shpseg *vector;
    int element;
    int i;

    DBUG_ENTER ("AccessClass2Vector");

    switch (class) {
    case ACL_offset:
        element = 1;
        break;
    case ACL_const:
        element = 0;
        break;
    default:
        element = -1;
        break;
    }

    if (element != -1) {

        /* supported access class */

        vector = MakeShpseg (NULL);

        for (i = 0; i < dim; i++) {
            SHPSEG_SHAPE (vector, i) = element;
        }
    } else {

        /* unsupported access class */
        vector = NULL;
    }

    DBUG_RETURN (vector);
}

/*****************************************************************************
 *
 * function:
 *   node *APCarg(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCarg");

    DBUG_PRINT ("APC", ("arg-node detected\n"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCvardec(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCvardec");

    DBUG_PRINT ("APC", ("vardec-node detected\n"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCarray(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCarray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCarray");

    DBUG_PRINT ("APC", ("array-node detected\n"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCwith(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCwith");

    DBUG_PRINT ("APC", ("with-node detected\n"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCap(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCap");

    DBUG_PRINT ("APC", ("ap-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCexprs(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCexprs (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCexprs");

    DBUG_PRINT ("APC", ("exprs-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCid(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCid");

    DBUG_PRINT ("APC", ("id-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCprf(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCprf");

    DBUG_PRINT ("APC", ("prf-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCfundef");

    DBUG_PRINT ("APC", ("fundef-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCblock(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCblock");

    DBUG_PRINT ("APC", ("block-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APClet(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APClet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APClet");

    DBUG_PRINT ("APC", ("let-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCwithop(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCwithop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCwithop");

    DBUG_PRINT ("APC", ("withop-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCcode(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APCcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APCcode");

    DBUG_PRINT ("APC", ("code-node detected"));

    DBUG_RETURN (arg_node);
}
