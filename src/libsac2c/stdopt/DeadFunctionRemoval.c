/*
 * $Id$
 */

#include "DeadFunctionRemoval.h"
#include "tree_basic.h"
#include "new_types.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "dbug.h"
#include "globals.h"
#include "traverse.h"
#include "ctinfo.h"

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

    result = MEMmalloc (sizeof (info));

    INFO_SPINE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * helper functions
 */
static node *
tagFundefAsNeeded (node *fundef, info *info)
{
    bool oldspine;

    DBUG_ENTER ("tagFundefAsNeeded");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "tagFundefAsNeeded applied to non fundef node");

    DBUG_ASSERT ((!FUNDEF_ISWRAPPERFUN (fundef)),
                 "tagFundefAsNeeded called on wrapper fun");

    if (!FUNDEF_ISNEEDED (fundef)) {
        DBUG_PRINT ("DFR", (">>> tagging fundef %s", CTIitemName (fundef)));

        FUNDEF_ISNEEDED (fundef) = TRUE;

        oldspine = INFO_SPINE (info);
        INFO_SPINE (info) = FALSE;

        if (FUNDEF_ARGS (fundef) != NULL) {
            DBUG_PRINT ("DFR", (">>> inspecting args..."));
            FUNDEF_ARGS (fundef) = TRAVdo (FUNDEF_ARGS (fundef), info);
        }

        if (FUNDEF_BODY (fundef) != NULL) {
            DBUG_PRINT ("DFR", (">>> inspecting fundef body..."));
            FUNDEF_BODY (fundef) = TRAVdo (FUNDEF_BODY (fundef), info);
        }

        INFO_SPINE (info) = oldspine;

        if (FUNDEF_ISOBJECTWRAPPER (fundef)) {
            /*
             * if this is an external object wrapper, we have to make sure that
             * its implementation is not removed, as all object wrappers
             * are stripped out in the backend.
             */
            DBUG_PRINT ("DFR", (">>> tagging implementation of objectwrapper..."));

            FUNDEF_IMPL (fundef) = tagFundefAsNeeded (FUNDEF_IMPL (fundef), info);
        }
    }

    DBUG_RETURN (fundef);
}

static node *
tagWrapperAsNeeded (node *wrapper, info *info)
{
    bool oldspine;

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
        oldspine = INFO_SPINE (info);
        INFO_SPINE (info) = FALSE;

        if (FUNDEF_ARGS (wrapper) != NULL) {
            DBUG_PRINT ("DFR", (">>> inspecting args..."));
            FUNDEF_ARGS (wrapper) = TRAVdo (FUNDEF_ARGS (wrapper), info);
        }

        if (FUNDEF_BODY (wrapper) != NULL) {
            DBUG_PRINT ("DFR", (">>> inspecting wrapper body..."));
            FUNDEF_BODY (wrapper) = TRAVdo (FUNDEF_BODY (wrapper), info);
        }

        INFO_SPINE (info) = oldspine;

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

/** <!-- ****************************************************************** -->
 * @fn node *tagObjdefAsNeeded( node *objdef, info *info)
 *
 * @brief tags all fundefs which are used as Objdef init expression as needed.
 *
 * @param objdef N_objdef node
 * @param arg_info info structure
 *
 * @return
 ******************************************************************************/

static node *
tagObjdefAsNeeded (node *objdef, info *info)
{
    DBUG_ENTER ("tagObjdefAsNeeded");

    if (!OBJDEF_ISNEEDED (objdef)) {
        DBUG_PRINT ("DFR", (">>> tagging objdef %s", CTIitemName (objdef)));

        OBJDEF_ISNEEDED (objdef) = TRUE;

        if ((OBJDEF_EXPR (objdef) != NULL)
            && (NODE_TYPE (OBJDEF_EXPR (objdef)) == N_ap)) {
            AP_FUNDEF (OBJDEF_EXPR (objdef))
              = tagFundefAsNeeded (AP_FUNDEF (OBJDEF_EXPR (objdef)), info);
        }

        if (OBJDEF_INITFUN (objdef) != NULL) {
            DBUG_ASSERT ((NODE_TYPE (OBJDEF_INITFUN (objdef)) == N_fundef),
                         "found non N_fundef node as objdef init function.");
            OBJDEF_INITFUN (objdef) = tagFundefAsNeeded (OBJDEF_INITFUN (objdef), info);
        }
    }

    DBUG_RETURN (objdef);
}

/** <!-- ****************************************************************** -->
 * @fn node *freeObjdefs( node *objdef)
 *
 ******************************************************************************/

static node *
freeObjdefs (node *objdef)
{
    DBUG_ENTER ("freeObjdefs");

    if (OBJDEF_NEXT (objdef) != NULL) {
        OBJDEF_NEXT (objdef) = freeObjdefs (OBJDEF_NEXT (objdef));
    }

    if (!OBJDEF_ISNEEDED (objdef)) {
        objdef = FREEdoFreeNode (objdef);
    }

    DBUG_RETURN (objdef);
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

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "DFR can only be called on entire modules");

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
    node *objdef;

    DBUG_ENTER ("DFRmodul");

    /*
     * Step 1: Clear dfr flag in fundec and fundef chain.
     *         Clear dfr flar in objdefs
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

    if (MODULE_OBJS (arg_node) != NULL) {
        objdef = MODULE_OBJS (arg_node);
        while (objdef != NULL) {
            OBJDEF_ISNEEDED (objdef) = FALSE;
            objdef = OBJDEF_NEXT (objdef);
        }
    }

    /*
     * Step 2: Search for needed fundecs, fundefs and objdefs in fundef bodies.
     */
    DBUG_PRINT ("DFR", ("processing objects..."));
    if (MODULE_OBJS (arg_node) != NULL) {
        INFO_SPINE (arg_info) = TRUE;
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
    }

    DBUG_PRINT ("DFR", ("processing fundefs..."));
    if (MODULE_FUNS (arg_node) != NULL) {
        INFO_SPINE (arg_info) = TRUE;
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_PRINT ("DFR", ("processing fundecs..."));
    if (MODULE_FUNDECS (arg_node) != NULL) {
        INFO_SPINE (arg_info) = TRUE;
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    /*
     * Step 3: Remove all objdefs not needed,
     *         Remove all zombies from fundec and fundef chain.
     *            ->Done implicitly when leaving N_module node.
     */
    if (MODULE_OBJS (arg_node) != NULL) {
        MODULE_OBJS (arg_node) = freeObjdefs (MODULE_OBJS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *DFRobjdef( node *arg_node, info *arg_info)
 *
 * @brief Marks provided objdefs and their init functions as needed
 *
 * @param arg_node N_objdef node
 * @param arg_info info structure
 *
 ******************************************************************************/
node *
DFRobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DFRobjdef");

    if (OBJDEF_ISPROVIDED (arg_node)) {
        arg_node = tagObjdefAsNeeded (arg_node, arg_info);
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = TRAVdo (OBJDEF_NEXT (arg_node), arg_info);
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
            if (!FUNDEF_ISWRAPPERFUN (arg_node)) {
                global.optcounters.dead_fun++;
            }
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

/** <!--********************************************************************-->
 *
 * @fn node *DFRarg( node *arg_node, info *arg_info)
 *
 * @brief marks required global objects as needed
 *
 *****************************************************************************/
node *
DFRarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DFRarg");

    if (ARG_OBJDEF (arg_node) != NULL) {
        ARG_OBJDEF (arg_node) = tagObjdefAsNeeded (ARG_OBJDEF (arg_node), arg_info);
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
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
