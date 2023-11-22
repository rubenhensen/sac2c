#include "DeadFunctionRemoval.h"

#include "DupTree.h"
#include "tree_basic.h"
#include "new_types.h"
#include "str.h"
#include "memory.h"
#include "free.h"

#define DBUG_PREFIX "DFR"
#include "debug.h"

#include "globals.h"
#include "traverse.h"
#include "ctinfo.h"

/*
 * INFO structure
 */
struct INFO {
    bool spine;
    bool localfuns;
    bool isonefundef;
};

/*
 * INFO macros
 */
#define INFO_SPINE(n) ((n)->spine)
#define INFO_LOCALFUNS(n) ((n)->localfuns)
#define INFO_ISONEFUNDEF(n) ((n)->isonefundef)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_SPINE (result) = FALSE;
    INFO_LOCALFUNS (result) = FALSE;
    INFO_ISONEFUNDEF (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

// Anonymous traversal to clear IS_NEEDED flags
static node *
ATravDFRCmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNDECS (arg_node) = TRAVopt (MODULE_FUNDECS (arg_node), arg_info);
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);
    MODULE_OBJS (arg_node) = TRAVopt (MODULE_OBJS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static node *
ATravDFRCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Initializing fundef %s as NOT needed", CTIitemName (arg_node));
    FUNDEF_ISNEEDED (arg_node) = FALSE;
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static node *
ATravDFRCobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    OBJDEF_ISNEEDED (arg_node) = FALSE;
    OBJDEF_NEXT (arg_node) = TRAVopt (OBJDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static node *
ClearIsNeededFlags (node *syntax_tree)
{
    anontrav_t dfrc_trav[4] = {{N_module, &ATravDFRCmodule},
                               {N_fundef, &ATravDFRCfundef},
                               {N_objdef, &ATravDFRCobjdef},
                               {(nodetype)0, NULL}};

    DBUG_ENTER ();

    TRAVpushAnonymous (dfrc_trav, &TRAVsons);
    syntax_tree = TRAVopt (syntax_tree, NULL);
    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

// helper functions

static node *
tagFundefAsNeeded (node *fundef, info *info)
{
    bool oldspine;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "tagFundefAsNeeded applied to non-fundef node");

    DBUG_ASSERT (!FUNDEF_ISWRAPPERFUN (fundef),
                 "tagFundefAsNeeded called on wrapper fun");

    if (!FUNDEF_ISNEEDED (fundef)) {
        DBUG_PRINT (">>> tagging fundef %s as needed", CTIitemName (fundef));
        FUNDEF_ISNEEDED (fundef) = TRUE;
        oldspine = INFO_SPINE (info);
        INFO_SPINE (info) = FALSE;

        if (FUNDEF_ARGS (fundef) != NULL) {
            DBUG_PRINT (">>> inspecting args...");
            FUNDEF_ARGS (fundef) = TRAVdo (FUNDEF_ARGS (fundef), info);
        }

        if (FUNDEF_BODY (fundef) != NULL) {
            DBUG_PRINT (">>> inspecting fundef body...");
            FUNDEF_BODY (fundef) = TRAVdo (FUNDEF_BODY (fundef), info);
        }

        INFO_SPINE (info) = oldspine;

        if (FUNDEF_ISOBJECTWRAPPER (fundef)) {
            /*
             * if this is an external object wrapper, we have to make sure that
             * its implementation is not removed, as all object wrappers
             * are stripped out in the backend.
             */
            DBUG_PRINT (">>> tagging implementation of objectwrapper...");

            FUNDEF_IMPL (fundef) = tagFundefAsNeeded (FUNDEF_IMPL (fundef), info);
        }
    }

    DBUG_RETURN (fundef);
}

static node *
tagWrapperAsNeeded (node *wrapper, info *info)
{
    bool oldspine;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (wrapper) == N_fundef,
                 "tagWrapperAsNeeded applied to non fundef node");

    DBUG_ASSERT (FUNDEF_ISWRAPPERFUN (wrapper),
                 "tagWrapperAsNeeded called on non-wrapper fun");

    if (!FUNDEF_ISNEEDED (wrapper)) {
        DBUG_PRINT (">>> tagging wrapper %s as needed", CTIitemName (wrapper));
        FUNDEF_ISNEEDED (wrapper) = TRUE;

        /*
         * the wrapper body is not a reliable source of needed
         * instances for a wrapper! it may contain fewer
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
            DBUG_PRINT (">>> inspecting args...");
            FUNDEF_ARGS (wrapper) = TRAVdo (FUNDEF_ARGS (wrapper), info);
        }

        if (FUNDEF_BODY (wrapper) != NULL) {
            DBUG_PRINT (">>> inspecting wrapper body...");
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
            DBUG_PRINT (">>> inspecting FUNDEF_IMPL...");

            FUNDEF_IMPL (wrapper) = tagFundefAsNeeded (FUNDEF_IMPL (wrapper), info);
        } else if (FUNDEF_WRAPPERTYPE (wrapper) != NULL) {
            DBUG_PRINT (">>> inspecting wrappertype...");

            FUNDEF_WRAPPERTYPE (wrapper)
              = TYmapFunctionInstances (FUNDEF_WRAPPERTYPE (wrapper), &tagFundefAsNeeded,
                                        info);
#ifndef DBUG_OFF
        } else {
            DBUG_UNREACHABLE (
              "found a wrapper with neither FUNDEF_IMPL, nor wrappertype");
#endif // DBUG_OFF
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
    DBUG_ENTER ();

    if (!OBJDEF_ISNEEDED (objdef)) {
        DBUG_PRINT (">>> tagging objdef %s as needed", CTIitemName (objdef));

        OBJDEF_ISNEEDED (objdef) = TRUE;

        if ((OBJDEF_EXPR (objdef) != NULL)
            && (NODE_TYPE (OBJDEF_EXPR (objdef)) == N_ap)) {
            AP_FUNDEF (OBJDEF_EXPR (objdef))
              = tagFundefAsNeeded (AP_FUNDEF (OBJDEF_EXPR (objdef)), info);
        }

        if (OBJDEF_INITFUN (objdef) != NULL) {
            DBUG_ASSERT (NODE_TYPE (OBJDEF_INITFUN (objdef)) == N_fundef,
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
    DBUG_ENTER ();

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

    DBUG_ENTER ();

    DBUG_ASSERT (DUPgetCopiedSpecialFundefsHook () == NULL,
                 "DFR found LaC funs on hook.");
    DBUG_PRINT_TAG ("OPTMEM", "mem currently allocated: %zu bytes",
                    global.current_allocated_mem);
    arg_info = MakeInfo ();

    TRAVpush (TR_dfr);

    // If this is a fundef-based traversal (e.g., from saacyc), we
    // just remove the fundef's dead localfns
    if ((N_fundef == NODE_TYPE (arg_node))) {
        INFO_ISONEFUNDEF (arg_info) = TRUE;
        arg_node = ClearIsNeededFlags (arg_node);
        arg_node = tagFundefAsNeeded (arg_node, arg_info);
        INFO_SPINE (arg_info) = TRUE;
    }

    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();
    arg_info = FreeInfo (arg_info);

    DBUG_PRINT_TAG ("OPTMEM", "mem currently allocated: %zu bytes",
                    global.current_allocated_mem);

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
    DBUG_ENTER ();

    /*
     * Step 1: Clear dfr flag in fundec and fundef chain.
     *         Clear dfr flar in objdefs
     */

    arg_node = ClearIsNeededFlags (arg_node);

    /*
     * Step 2: Search for needed fundecs, fundefs and objdefs in fundef bodies.
     */
    if (MODULE_OBJS (arg_node) != NULL) {
        DBUG_PRINT ("processing objects");
        INFO_SPINE (arg_info) = TRUE;
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        DBUG_PRINT ("processing module fundefs");
        INFO_SPINE (arg_info) = TRUE;
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {
        DBUG_PRINT ("processing fundecs");
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
    DBUG_ENTER ();

    if (OBJDEF_ISPROVIDED (arg_node)) {
        arg_node = tagObjdefAsNeeded (arg_node, arg_info);
    }

    OBJDEF_NEXT (arg_node) = TRAVopt(OBJDEF_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    DBUG_PRINT ("Dead Function Removal in %s: %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                CTIitemName (arg_node));

    if (INFO_SPINE (arg_info)) {
        //  remark: main is always marked as provided
        if (FUNDEF_ISSTICKY (arg_node) || FUNDEF_ISPROVIDED (arg_node)) {
            DBUG_PRINT (">>> %s is %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                        (FUNDEF_ISSTICKY (arg_node) ? "sticky" : "provided"));

            if (FUNDEF_ISWRAPPERFUN (arg_node)) {
                arg_node = tagWrapperAsNeeded (arg_node, arg_info);
            } else {
                arg_node = tagFundefAsNeeded (arg_node, arg_info);
            }
        }

        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

        if (!FUNDEF_ISNEEDED (arg_node)) {
            DBUG_PRINT ("Deleting: %s for function: %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                        CTIitemName (arg_node));
            if (!FUNDEF_ISWRAPPERFUN (arg_node)) {
                if (INFO_ISONEFUNDEF (arg_info)) {
                    global.optcounters.dead_lfun++;
                } else {
                    global.optcounters.dead_fun++;
                }
            }
            arg_node = FREEdoFreeNode (arg_node);
        } else { // search for dead local functions
            INFO_LOCALFUNS (arg_info) = TRUE;
            FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
            INFO_LOCALFUNS (arg_info) = FALSE;
        }
    } else if (INFO_LOCALFUNS (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        if (!FUNDEF_ISNEEDED (arg_node)) {
            DBUG_PRINT ("Deleting function: %s", CTIitemName (arg_node));
            arg_node = FREEdoFreeNode (arg_node);
            if (INFO_ISONEFUNDEF (arg_info)) {
                global.optcounters.dead_lfun++;
            } else {
                global.optcounters.dead_fun++;
            }
        }
    } else {
        // we came via AP_FUNDEF
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
    DBUG_ENTER ();

    if (ARG_OBJDEF (arg_node) != NULL) {
        ARG_OBJDEF (arg_node) = tagObjdefAsNeeded (ARG_OBJDEF (arg_node), arg_info);
    }

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    // This is a function application. Move into the called
    // function and mark it as needed.
    DBUG_PRINT ("Traversing N_ap for function %s", FUNDEF_NAME (AP_FUNDEF (arg_node)));
    AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    DBUG_PRINT ("Return from traversing N_ap for function %s",
                FUNDEF_NAME (AP_FUNDEF (arg_node)));

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
    DBUG_ENTER ();

    FOLD_FUNDEF (arg_node) = TRAVdo (FOLD_FUNDEF (arg_node), arg_info);
    FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
