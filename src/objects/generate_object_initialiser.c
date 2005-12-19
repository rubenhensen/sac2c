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
    node *main;
    namespace_t *ns;
};

/*
 * INFO macros
 */
#define INFO_NS(n) ((n)->ns)

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
    last = NULL;

    while (list != NULL) {
        pos = list;
        changes = 0;

        while (pos != NULL) {
            if (TClinklistIsSubset (sorted, FUNDEF_OBJECTS (
                                              OBJDEF_INITFUN (LINKLIST_LINK (pos))))) {
                /*
                 * move the link to the sorted list
                 */
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
ObjdefsToInitAssigns (node *objdefs, node *assigns)
{
    node *result;

    DBUG_ENTER ("ObjdefsToInitAssigns");

    if (objdefs != NULL) {
        result = ObjdefsToInitAssigns (LINKLIST_NEXT (objdefs), assigns);
        result
          = TBmakeAssign (TBmakeLet (NULL,
                                     TBmakeAp (OBJDEF_INITFUN (LINKLIST_LINK (objdefs)),
                                               TBmakeExprs (TBmakeGlobobj (
                                                              LINKLIST_LINK (objdefs)),
                                                            NULL))),
                          assigns);
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

    if (!FUNDEF_ISWRAPPERFUN (arg_node)
        && (NSequals (FUNDEF_NS (arg_node), INFO_NS (arg_info)))
        && (ILIBstringCompare (FUNDEF_NAME (arg_node), "main"))) {
        node *initfun = GenerateObjectInitFun (FUNDEF_OBJECTS (arg_node));

        arg_node = InsertInitFunCall (arg_node, initfun);

        FUNDEF_NEXT (initfun) = arg_node;
        arg_node = initfun;
    } else {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
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
