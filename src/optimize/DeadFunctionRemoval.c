/*
 *
 * $Log$
 * Revision 3.15  2004/11/26 03:16:20  sah
 * COMPILES!
 *
 * Revision 3.14  2004/11/19 10:17:34  sah
 * objinitfuns are never removed
 *
 * Revision 1.1  1999/01/07 17:36:51  sbs
 * Initial revision
 *
 */

#define NEW_INFO

#include "DeadFunctionRemoval.h"
#include "tree_basic.h"
#include "internal_lib.h"
#include "free.h"
#include "dbug.h"
#include "globals.h"
#include "traverse.h"
#include "Error.h"

#include "optimize.h"
/*
 * INFO structure
 */
struct INFO {
    int flag;
};

/*
 * INFO macros
 */
#define INFO_DFR_SPINE(n) (n->flag)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_DFR_SPINE (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * Function:
 *   node *DeadFunctionRemoval( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFRdoDeadFunctionRemoval (node *arg_node)
{
    info *arg_info;
#ifndef DBUG_OFF
    int mem_dead_fun = dead_fun;
#endif

    DBUG_ENTER ("DFRdoDeadFunctionRemoval");
    DBUG_PRINT ("OPT", ("DEAD FUNCTION REMOVAL"));
    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));

    arg_info = MakeInfo ();

    TRAVpush (TR_dfr);

    arg_node = TRAVdo (arg_node, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_PRINT ("OPT", ("                        result: %d", dead_fun - mem_dead_fun));
    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFRmodule(node *arg_node,info *arg_info)
 *
 * Description:
 *   Prevents DFR in modules
 *   in programs the DFR starts in fundef main.
 *
 ******************************************************************************/

node *
DFRmodule (node *arg_node, info *arg_info)
{
    node *fun;
    DBUG_ENTER ("DFRmodul");

    if (MODULE_FUNDECS (arg_node) != NULL) {
        /* clear dfr flag */
        fun = MODULE_FUNDECS (arg_node);
        while (fun != NULL) {
            FUNDEF_ISNEEDED (fun) = FALSE;
            fun = FUNDEF_NEXT (fun);
        }

        /*
         * the following code was disabled for a unknown reason.
         * as we have no idea why it was disabled, we have
         * reenabled it for now
         */

        /* check fundefs for applications (only main in programs) */
        INFO_DFR_SPINE (arg_info) = TRUE;
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);

        /* remove all produced zombies */
        MODULE_FUNDECS (arg_node) = FREEremoveAllZombies (MODULE_FUNDECS (arg_node));
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        /* clear dfr flag */
        fun = MODULE_FUNS (arg_node);
        while (fun != NULL) {
            FUNDEF_ISNEEDED (fun) = FALSE;
            fun = FUNDEF_NEXT (fun);
        }

        /* check fundefs for applications (only main in programs) */
        INFO_DFR_SPINE (arg_info) = TRUE;
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);

        /* remove all produced zombies */
        MODULE_FUNS (arg_node) = FREEremoveAllZombies (MODULE_FUNS (arg_node));
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {

        /* remove all produced zombies */
        MODULE_FUNDECS (arg_node) = FREEremoveAllZombies (MODULE_FUNDECS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFRfundef( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses instruction- and function-chain in this order.
 *
 ******************************************************************************/

node *
DFRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DFRfundef");

    if (INFO_DFR_SPINE (arg_info)) {
        DBUG_PRINT ("DFR",
                    ("Dead Function Removal in function: %s", FUNDEF_NAME (arg_node)));

        /* a warning for using DFR with SSA */
        if ((FUNDEF_ISDOFUN (arg_node)) || (FUNDEF_ISCONDFUN (arg_node))) {
            if (FUNDEF_USED (arg_node) > 1) {
                SYSWARN (("Lac-functions, which are used more than once aren't "
                          "handled correctly by DeadFunctionRemoval"));
            }
        }

        /*
         * remark: main is always tagged as provided
         */
        if (FUNDEF_ISPROVIDED (arg_node)) {
            FUNDEF_ISNEEDED (arg_node) = TRUE;

            if (FUNDEF_BODY (arg_node) != NULL) {
                INFO_DFR_SPINE (arg_info) = FALSE;
                FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
                INFO_DFR_SPINE (arg_info) = TRUE;
            }
        }

        /* traverse next fundef */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }

        /* on bottom up traversal turn unused fundefs into zombies */
        if (!(FUNDEF_ISNEEDED (arg_node))) {
            dead_fun++;
            arg_node = FREEdoFreeNode (arg_node);
        }
    } else {
        if (!FUNDEF_ISNEEDED (arg_node)) {
            FUNDEF_ISNEEDED (arg_node) = TRUE;

            if (FUNDEF_BODY (arg_node) != NULL) {
                FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFRap( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFRap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DFRap");

    /*
     * we have found a function application. SO move into the called
     * function and mark it as needed.
     */
    AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFRfold( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFRfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DFRwithop");

    FOLD_FUNDEF (arg_node) = TRAVdo (FOLD_FUNDEF (arg_node), arg_info);

    if (FOLD_NEXT (arg_node) != NULL) {
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
