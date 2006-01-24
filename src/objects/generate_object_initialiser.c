/* $Id$ */

#include "generate_object_initialiser.h"

#include "dbug.h"
#include "internal_lib.h"
#include "traverse.h"
#include "namespaces.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "DupTree.h"
#include "ctinfo.h"

/*
 * INFO structure
 */
struct INFO {
    node **deps;
    namespace_t *ns;
};

/*
 * INFO macros
 */
#define INFO_NS(n) ((n)->ns)
#define INFO_DEPS(n) ((n)->deps)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_NS (result) = NULL;
    INFO_DEPS (result) = NULL;

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
SortObjdefList (node *objlist)
{
    node *list;
    node *sorted;
    node *pos;
    node *last;
    int changes;

    DBUG_ENTER ("SortObjdefList");

    list = DUPdoDupTree (objlist);
    sorted = NULL;

    while (list != NULL) {
        pos = list;
        last = NULL;
        changes = 0;

        while (pos != NULL) {
            DBUG_PRINT ("GOI", ("trying %s ...", CTIitemName (LINKLIST_LINK (pos))));

            if (TClinklistIsSubset (sorted, FUNDEF_OBJECTS (
                                              OBJDEF_INITFUN (LINKLIST_LINK (pos))))) {
                /*
                 * move the link to the sorted list
                 */
                DBUG_PRINT ("GOI", ("...adding %s to initlist",
                                    CTIitemName (LINKLIST_LINK (pos))));

                node *tmp = pos;
                pos = LINKLIST_NEXT (pos);
                if (last != NULL) {
                    LINKLIST_NEXT (last) = pos;
                } else {
                    list = pos;
                }
                LINKLIST_NEXT (tmp) = NULL;

                TCaddLinksToLinks (&sorted, tmp);
                changes++;
            } else {
                last = pos;
                pos = LINKLIST_NEXT (pos);
            }
        }
        if (changes == 0) {
            CTIabort ("Cannot compute initialisation order for objdefs. This "
                      "may be due to circular dependencies!");
        }
    }

    DBUG_RETURN (sorted);
}

static node *
AddInitFunDependencies (node *objlist)
{
    node *pos;
    node *new;
    int changes;

    DBUG_ENTER ("AddInitFunDependencies");

    if (objlist != NULL) {
        do {
            new = DUPdoDupTree (objlist);
            pos = objlist;
            changes = 0;

            while (pos != NULL) {
                changes += TCaddLinksToLinks (&new, FUNDEF_OBJECTS (OBJDEF_INITFUN (
                                                      LINKLIST_LINK (pos))));
                pos = LINKLIST_NEXT (pos);
            }

            objlist = FREEdoFreeTree (objlist);
            objlist = new;
        } while (changes != 0);
    }

    DBUG_RETURN (objlist);
}

static node *
ObjdefsToInitAssigns (node *objdefs, node *assigns)
{
    node *result;

    DBUG_ENTER ("ObjdefsToInitAssigns");

    if (objdefs != NULL) {
        result = ObjdefsToInitAssigns (LINKLIST_NEXT (objdefs), assigns);
        if (OBJDEF_INITFUN (LINKLIST_LINK (objdefs)) != NULL) {
            result
              = TBmakeAssign (TBmakeLet (NULL, TBmakeAp (OBJDEF_INITFUN (
                                                           LINKLIST_LINK (objdefs)),
                                                         TBmakeExprs (TBmakeGlobobj (
                                                                        LINKLIST_LINK (
                                                                          objdefs)),
                                                                      NULL))),
                              result);
        }
    } else {
        result = assigns;
    }

    DBUG_RETURN (result);
}

static node *
GenerateObjectInitFun (node *objlist)
{
    node *result;
    node *assigns;

    DBUG_ENTER ("GenerateObjectInitFun");

    /*
     * return statement
     */
    assigns = TBmakeAssign (TBmakeReturn (NULL), NULL);

    /*
     * objdef init assigns
     */
    objlist = SortObjdefList (objlist);
    assigns = ObjdefsToInitAssigns (objlist, assigns);

    /*
     * the fundef
     */
    result = TBmakeFundef (ILIBstringCopy ("init"), NSgetInitNamespace (), NULL, NULL,
                           TBmakeBlock (assigns, NULL), NULL);

    FUNDEF_OBJECTS (result) = objlist;

    DBUG_RETURN (result);
}

static node *
InsertInitFunCall (node *fun, node *initfun)
{
    DBUG_ENTER ("InsertInitFunCall");

    BLOCK_INSTR (FUNDEF_BODY (fun))
      = TBmakeAssign (TBmakeLet (NULL, TBmakeAp (initfun, NULL)),
                      BLOCK_INSTR (FUNDEF_BODY (fun)));

    DBUG_RETURN (fun);
}

/*
 * traversal functions
 */
node *
GOIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GOIfundef");

    /*
     * check for function _MAIN::main
     */
    if (!FUNDEF_ISWRAPPERFUN (arg_node)
        && (NSequals (FUNDEF_NS (arg_node), INFO_NS (arg_info)))
        && (ILIBstringCompare (FUNDEF_NAME (arg_node), "main"))) {
        node *initfun;

        /*
         * first add all objects that are needed by the initfuns themselves
         * to the dependency list of main
         */
        FUNDEF_OBJECTS (arg_node) = AddInitFunDependencies (FUNDEF_OBJECTS (arg_node));

        /*
         * next create the init function itself
         */
        initfun = GenerateObjectInitFun (FUNDEF_OBJECTS (arg_node));

        /*
         * insert the call into main
         */
        arg_node = InsertInitFunCall (arg_node, initfun);

        /*
         * and append the function to the fundef chain
         */
        FUNDEF_NEXT (initfun) = FUNDEF_NEXT (arg_node);
        FUNDEF_NEXT (arg_node) = initfun;

        /*
         * finally update the wrapper. see below for comment...
         */
        if (INFO_DEPS (arg_info) != NULL) {
            if (*INFO_DEPS (arg_info) != NULL) {
                *INFO_DEPS (arg_info) = FREEdoFreeTree (*INFO_DEPS (arg_info));
                *INFO_DEPS (arg_info) = DUPdoDupTree (FUNDEF_OBJECTS (arg_node));
            } else {
                INFO_DEPS (arg_info) = &FUNDEF_OBJECTS (arg_node);
            }
        }
    } else {

        if (FUNDEF_ISWRAPPERFUN (arg_node)
            && (NSequals (FUNDEF_NS (arg_node), INFO_NS (arg_info)))
            && (ILIBstringCompare (FUNDEF_NAME (arg_node), "main"))) {
            /*
             * we need to correct the wrapper object dependencies as well.
             * if INFO_DEPS is NULL, we have not passed the main function
             * yet, so we store a reference to the wrappers object dependencies.
             * Otherwise we passed main already, so we copy the stored dependencies
             * to the wrapper
             */
            if (INFO_DEPS (arg_info) == NULL) {
                INFO_DEPS (arg_info) = &FUNDEF_OBJECTS (arg_node);
            } else {
                if (*INFO_DEPS (arg_info) != NULL) {
                    FUNDEF_OBJECTS (arg_node)
                      = FREEdoFreeTree (FUNDEF_OBJECTS (arg_node));
                    FUNDEF_OBJECTS (arg_node) = DUPdoDupTree (*INFO_DEPS (arg_info));
                }
            }
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
GOImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GOImodule");

    if (MODULE_FILETYPE (arg_node) == F_prog) {
        INFO_NS (arg_info) = MODULE_NAMESPACE (arg_node);

        if (MODULE_FUNS (arg_node) != NULL) {
            MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
        }

        INFO_NS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/*
 * traversal start function
 */
node *
GOIdoGenerateObjectInitialiser (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("GOIdoGenerateObjectInitialiser");

    info = MakeInfo ();
    TRAVpush (TR_goi);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
