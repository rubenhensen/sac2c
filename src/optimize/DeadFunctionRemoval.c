/*
 *
 * $Log$
 * Revision 2.5  2000/10/31 18:08:09  cg
 * Dead function removal completely re-implemented.
 *
 * Revision 2.4  2000/07/14 12:59:25  dkr
 * DFRfundef: body of function is always traversed now
 *
 * Revision 2.3  2000/07/14 12:05:14  dkr
 * DFRblock() added
 *
 * Revision 2.2  2000/01/26 17:26:59  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.1  1999/02/23 12:41:13  sacbase
 * new release made
 *
 * Revision 1.3  1999/01/20 09:07:46  cg
 * Dead function removal may now handle programs without any functions.
 *
 * Revision 1.2  1999/01/18 15:46:02  sbs
 * DBUG_PRINT( "OPTMEM",...) inserted for mem-info during optimization
 *
 * Revision 1.1  1999/01/07 17:36:51  sbs
 * Initial revision
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "globals.h"
#include "my_debug.h"
#include "traverse.h"
#include "internal_lib.h"

#include "optimize.h"

/******************************************************************************
 *
 * Function:
 *   node *DeadFunctionRemoval( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DeadFunctionRemoval (node *arg_node, node *arg_info)
{
    funtab *tmp_tab;
    int mem_dead_fun = dead_fun;

    DBUG_ENTER ("DeadFunctionRemoval");
    DBUG_PRINT ("OPT", ("DEAD FUNCTION REMOVAL"));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    tmp_tab = act_tab;
    act_tab = dfr_tab;

    arg_info = MakeInfo ();
    arg_node = Trav (arg_node, arg_info);
    arg_info = FreeTree (arg_info);

    DBUG_PRINT ("OPT", ("                        result: %d", dead_fun - mem_dead_fun));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFRmodul(node *arg_node,node *arg_info)
 *
 * Description:
 *   Prevents DFR in modules
 *
 ******************************************************************************/

node *
DFRmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFRmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        INFO_DFR_SPINE (arg_info) = TRUE;
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFRfundef( node *arg_node, node *arg_info)
 *
 * Description:
 *   Traverses instruction- and function-chain in this order.
 *
 ******************************************************************************/

node *
DFRfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFRfundef");

    if (INFO_DFR_SPINE (arg_info)) {

        DBUG_PRINT ("DFR",
                    ("Dead Function Removal in function: %s", FUNDEF_NAME (arg_node)));

        if ((FUNDEF_STATUS (arg_node) == ST_exported)
            || (FUNDEF_STATUS (arg_node) == ST_objinitfun)) {
            FUNDEF_EXPORT (arg_node) = TRUE;

            if (FUNDEF_BODY (arg_node) != NULL) {
                INFO_DFR_SPINE (arg_info) = FALSE;
                FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
                INFO_DFR_SPINE (arg_info) = TRUE;
            }
        }

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }

        if (FUNDEF_EXPORT (arg_node)) {
            FUNDEF_EXPORT (arg_node) = FALSE;
        } else {
            dead_fun++;
            arg_node = FreeNode (arg_node);
        }
    } else {
        if (!FUNDEF_EXPORT (arg_node)) {
            FUNDEF_EXPORT (arg_node) = TRUE;
            if (FUNDEF_BODY (arg_node) != NULL) {
                FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFRblock( node *arg_node, node *arg_info)
 *
 * Description:
 *   Removes BLOCK_NEEDFUNS!
 *
 ******************************************************************************/

node *
DFRblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFRblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    /*
     * After DFR the data stored in BLOCK_NEEDFUNS is possibly corrupted!
     * -> remove it!
     */
    if (BLOCK_NEEDFUNS (arg_node) != NULL) {
        BLOCK_NEEDFUNS (arg_node) = FreeNodelist (BLOCK_NEEDFUNS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFRap( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFRap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFRap");

    AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFRwithop( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFRwithop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFRwithop");

    if (NWITHOP_TYPE (arg_node) == WO_foldfun) {
        NWITHOP_FUNDEF (arg_node) = Trav (NWITHOP_FUNDEF (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
