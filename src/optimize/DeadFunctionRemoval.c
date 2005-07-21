/*
 *
 * $Log$
 * Revision 3.21  2005/07/21 16:18:12  sah
 * now all instances get tagged correctly
 *
 * Revision 3.20  2005/07/21 14:22:33  sah
 * improved DFR on external functions
 *
 * Revision 3.19  2005/07/15 17:41:49  sah
 * prevented the deletion of instances that might be
 * used for dispatch later on
 *
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
#include "new_types.h"
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

static node *
tagFundefAsNeeded (node *fundef, info *info)
{
    DBUG_ENTER ("tagFundefAsNeeded");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "tagFundefAsNeeded applied to non fundef node");

    DBUG_ASSERT ((!FUNDEF_ISWRAPPERFUN (fundef)),
                 "tagFundefAsNeeded called on wrapper fun");

    if (!FUNDEF_ISNEEDED (fundef)) {
        DBUG_PRINT ("DFR", (">>> tagging fundef %s", CTIitemName (fundef)));

        FUNDEF_ISNEEDED (fundef) = TRUE;

        if (FUNDEF_BODY (fundef) != NULL) {
            bool oldspine = INFO_DFR_SPINE (info);
            INFO_DFR_SPINE (info) = FALSE;

            DBUG_PRINT ("DFR", (">>> inspecting body..."));
            FUNDEF_BODY (fundef) = TRAVdo (FUNDEF_BODY (fundef), info);

            INFO_DFR_SPINE (info) = oldspine;
        }
    }

    DBUG_RETURN (fundef);
}

static node *
tagWrapperAsNeeded (node *wrapper, info *info)
{
    DBUG_ENTER ("tagWrapperAsNeeded");

    DBUG_ASSERT ((NODE_TYPE (wrapper) == N_fundef),
                 "tagWrapperAsNeeded applied to non fundef node");

    DBUG_ASSERT ((FUNDEF_ISWRAPPERFUN (wrapper)),
                 "tagFundefAsNeeded called on non-wrapper fun");

    if (!FUNDEF_ISNEEDED (wrapper)) {
        DBUG_PRINT ("DFR", (">>> tagging wrapper %s", CTIitemName (wrapper)));

        FUNDEF_ISNEEDED (wrapper) = TRUE;

        if (FUNDEF_BODY (wrapper) != NULL) {
            bool oldspine = INFO_DFR_SPINE (info);
            INFO_DFR_SPINE (info) = FALSE;

            DBUG_PRINT ("DFR", (">>> inspecting body..."));
            FUNDEF_BODY (wrapper) = TRAVdo (FUNDEF_BODY (wrapper), info);

            INFO_DFR_SPINE (info) = oldspine;
        } else if (FUNDEF_IMPL (wrapper) != NULL) {
            /*
             * we found a wrapper that has FUNDEF_IMPL set
             * this is a dirty hack telling us that the function
             * has no arguments and thus only one instance!
             * so we tag that instance
             */
            DBUG_PRINT ("DFR", (">>> inspecting FUNDEF_IMPL..."));

            FUNDEF_IMPL (wrapper) = tagFundefAsNeeded (FUNDEF_IMPL (wrapper), info);
        } else if (FUNDEF_WRAPPERTYPE (wrapper) != NULL) {
            DBUG_PRINT ("DFR", (">>> inspecting wrappertype..."));

            FUNDEF_WRAPPERTYPE (wrapper)
              = TYmapFunctionInstances (FUNDEF_WRAPPERTYPE (wrapper), &tagFundefAsNeeded,
                                        info);
#ifndef DBUG_OFF
        } else {
            DBUG_ASSERT (0, "found a wrapper with neither FUNDEF_IMPL, nor wrappertype");
#endif
        }
    }

    DBUG_RETURN (wrapper);
}

node *
DFRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DFRfundef");

    if (INFO_DFR_SPINE (arg_info)) {
        DBUG_PRINT ("DFR", ("Dead Function Removal in %s: %s",
                            (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                            CTIitemName (arg_node)));

        /*
         * remark: main is always tagged as provided
         */
        if (FUNDEF_ISPROVIDED (arg_node)) {
            DBUG_PRINT ("DFR", (">>> %s is provided",
                                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef")));

            if (FUNDEF_ISWRAPPERFUN (arg_node)) {
                arg_node = tagWrapperAsNeeded (arg_node, arg_info);
            } else {
                arg_node = tagFundefAsNeeded (arg_node, arg_info);
            }
        }

        /* traverse next fundef */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }

        if (!FUNDEF_ISNEEDED (arg_node)) {
            DBUG_PRINT ("DFR", ("Going to delete %s for %s",
                                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                                CTIitemName (arg_node)));
            dead_fun++;
            arg_node = FREEdoFreeNode (arg_node);
        } else {
            /*
             * clean up for next run -- remove the tag
             */
            FUNDEF_ISNEEDED (arg_node) = FALSE;
        }
    } else {
        if (!FUNDEF_ISNEEDED (arg_node)) {
            if (FUNDEF_ISWRAPPERFUN (arg_node)) {
                arg_node = tagWrapperAsNeeded (arg_node, arg_info);
            } else {
                arg_node = tagFundefAsNeeded (arg_node, arg_info);
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
