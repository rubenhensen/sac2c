/*
 *
 * $Log$
 * Revision 3.18  2005/07/15 17:34:14  sah
 * added some better debugging facilities
 *
 * Revision 3.17  2005/03/04 21:21:42  cg
 * Optimization completely streamlined.
 * Removal of zombie functions automatized.
 *
 * Revision 3.16  2005/01/11 12:58:15  cg
 * Converted output from Error.h to ctinfo.c
 *
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

#include "DeadFunctionRemoval.h"
#include "tree_basic.h"
#include "internal_lib.h"
#include "free.h"
#include "dbug.h"
#include "globals.h"
#include "traverse.h"

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

    /*
     * Step 1: Clear dfr flag in fundec and fundef chain.
     */
    if (MODULE_FUNDECS (arg_node) != NULL) {
        fun = MODULE_FUNDECS (arg_node);
        while (fun != NULL) {
            FUNDEF_ISNEEDED (fun) = FALSE;
            fun = FUNDEF_NEXT (fun);
        }
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        fun = MODULE_FUNS (arg_node);
        while (fun != NULL) {
            FUNDEF_ISNEEDED (fun) = FALSE;
            fun = FUNDEF_NEXT (fun);
        }
    }

    /*
     * Step 2: Search for needed fundecs and fundefs in fundef bodies.
     */

    if (MODULE_FUNS (arg_node) != NULL) {
        INFO_DFR_SPINE (arg_info) = TRUE;
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    /*
     * Step 3: Remove all zombies from fundec and fundef chain.
     *
     *  Done implicitly when leaving N_module node.
     */

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
                    ("Dead Function Removal in function: %s", CTIitemName (arg_node)));

        /*
         * remark: main is always tagged as provided
         */
        if (FUNDEF_ISPROVIDED (arg_node)) {
            DBUG_PRINT ("DFR", (">>> %s is provided",
                                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef")));

            FUNDEF_ISNEEDED (arg_node) = TRUE;

            if (FUNDEF_BODY (arg_node) != NULL) {
                DBUG_PRINT ("DFR", (">>> inspecting body..."));

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
            DBUG_PRINT ("DFR", ("Going to delete %s for %s",
                                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                                CTIitemName (arg_node)));
            dead_fun++;
            arg_node = FREEdoFreeNode (arg_node);
        }
    } else {
        if (!FUNDEF_ISNEEDED (arg_node)) {
            DBUG_PRINT ("DFR",
                        (">>> fundef %s tagged as a dependency", CTIitemName (arg_node)));

            FUNDEF_ISNEEDED (arg_node) = TRUE;

            if (FUNDEF_BODY (arg_node) != NULL) {
                FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            }
#ifndef DBUG_OFF
        } else {
            DBUG_PRINT ("DFR", (">>> fundef %s already marked", CTIitemName (arg_node)));
#endif
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
