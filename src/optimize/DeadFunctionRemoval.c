/*
 * $Id$
 */

#include "DeadFunctionRemoval.h"
#include "tree_basic.h"
#include "new_types.h"
#include "internal_lib.h"
#include "free.h"
#include "dbug.h"
#include "globals.h"
#include "traverse.h"

/*
 * INFO structure
 */
struct INFO {
    bool flag;
};

/*
 * INFO macros
 */
#define INFO_SPINE(n) ((n)->flag)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_SPINE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 * helper functions
 */
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
            bool oldspine = INFO_SPINE (info);
            INFO_SPINE (info) = FALSE;

            DBUG_PRINT ("DFR", (">>> inspecting body..."));
            FUNDEF_BODY (fundef) = TRAVdo (FUNDEF_BODY (fundef), info);

            INFO_SPINE (info) = oldspine;
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
                 "tagWrapperAsNeeded called on non-wrapper fun");

    if (!FUNDEF_ISNEEDED (wrapper)) {
        DBUG_PRINT ("DFR", (">>> tagging wrapper %s", CTIitemName (wrapper)));

        FUNDEF_ISNEEDED (wrapper) = TRUE;

        /*
         * the wrapper body is no reliable source of needed
         * instances for a wrapper! it may contain less
         * instances than the wrapper type (e.g. one instance
         * returns a constant value and its call within the
         * wrapper is thus replaced by that value). but as the
         * wrapper type is used for dispatch, all instances that
         * are denoted within that type need to be present!
         *
         * the wrapper body has still to be traversed to mark
         * functions used by that body (especially cond funs)
         * as needed!
         */

        if (FUNDEF_BODY (wrapper) != NULL) {
            bool oldspine = INFO_SPINE (info);
            INFO_SPINE (info) = FALSE;

            DBUG_PRINT ("DFR", (">>> inspecting wrapper body..."));
            FUNDEF_BODY (wrapper) = TRAVdo (FUNDEF_BODY (wrapper), info);

            INFO_SPINE (info) = oldspine;
        }

        if (FUNDEF_IMPL (wrapper) != NULL) {
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

    DBUG_ENTER ("DFRdoDeadFunctionRemoval");

#ifdef SHOW_MALLOC
    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));
#endif

    arg_info = MakeInfo ();

    TRAVpush (TR_dfr);

    arg_node = TRAVdo (arg_node, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

#ifdef SHOW_MALLOC
    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));
#endif

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
     * Step 2a: Search for needed fundecs and fundefs in objdef init exprs.
     */
    if (MODULE_OBJS (arg_node) != NULL) {
        INFO_SPINE (arg_info) = TRUE;
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
    }

    /*
     * Step 2b: Search for needed fundecs and fundefs in fundef bodies.
     */

    if (MODULE_FUNS (arg_node) != NULL) {
        INFO_SPINE (arg_info) = TRUE;
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    /*
     * Step 3: Remove all zombies from fundec and fundef chain.
     *
     *  Done implicitly when leaving N_module node.
     */

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *DFRobjdef( node *arg_node, info *arg_info)
 *
 * @brief tags all fundefs which are used as Objdef init expression as needed.
 *
 * @param arg_node N_objdef node
 * @param arg_info info structure
 *
 * @return unmodified N_objdef node
 ******************************************************************************/

node *
DFRobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DFRobjdef");

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = TRAVdo (OBJDEF_NEXT (arg_node), arg_info);
    }

    if (OBJDEF_EXPR (arg_node) != NULL) {
        DBUG_ASSERT ((NODE_TYPE (OBJDEF_EXPR (arg_node)) == N_ap),
                     "found non N_ap node as objdef init expr.");

        DBUG_PRINT ("DFR",
                    ("Dead Function Removal for Objdef %s", CTIitemName (arg_node)));

        AP_FUNDEF (OBJDEF_EXPR (arg_node))
          = tagFundefAsNeeded (AP_FUNDEF (OBJDEF_EXPR (arg_node)), arg_info);
    }

    if (OBJDEF_INITFUN (arg_node) != NULL) {
        DBUG_PRINT ("DFR",
                    ("Dead Function Removal for Objdef %s", CTIitemName (arg_node)));

        OBJDEF_INITFUN (arg_node)
          = tagFundefAsNeeded (OBJDEF_INITFUN (arg_node), arg_info);
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

    if (INFO_SPINE (arg_info)) {
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
            global.optcounters.dead_fun++;
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
