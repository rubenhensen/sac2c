#include "generate_object_initialiser.h"

#define DBUG_PREFIX "GOI"
#include "debug.h"

#include "str.h"
#include "memory.h"
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
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_NS (result) = NULL;
    INFO_DEPS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

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

    DBUG_ENTER ();

    list = DUPdoDupTree (objlist);
    sorted = NULL;

    while (list != NULL) {
        pos = list;
        last = NULL;
        changes = 0;

        while (pos != NULL) {
            DBUG_PRINT ("trying %s ...", CTIitemName (SET_MEMBER (pos)));

            /*
             * if all dependencies of this object are already satiesfied,
             * we can add it to the list. This is try if either
             *
             * A) the object has no initfun at all!
             * B) its set of dependence objects is a subset of the already
             *    initialised objects!
             */
            if ((OBJDEF_INITFUN (SET_MEMBER (pos)) == NULL)
                || TCsetIsSubset (sorted,
                                  FUNDEF_OBJECTS (OBJDEF_INITFUN (SET_MEMBER (pos))))) {
                /*
                 * move the member to the sorted list
                 */
                DBUG_PRINT ("...adding %s to initlist", CTIitemName (SET_MEMBER (pos)));

                node *tmp = pos;
                pos = SET_NEXT (pos);
                if (last != NULL) {
                    SET_NEXT (last) = pos;
                } else {
                    list = pos;
                }
                SET_NEXT (tmp) = NULL;

                TCsetUnion (&sorted, tmp);
                changes++;
            } else {
                last = pos;
                pos = SET_NEXT (pos);
            }
        }
        if (changes == 0) {
            CTIabort (EMPTY_LOC, 
                      "Cannot compute initialisation order for objdefs. This "
                      "may be due to circular dependencies!");
        }
    }

    DBUG_RETURN (sorted);
}

static node *
AddInitFunDependencies (node *objlist)
{
    node *pos;
    node *xnew;
    int changes;

    DBUG_ENTER ();

    if (objlist != NULL) {
        do {
            xnew = DUPdoDupTree (objlist);
            pos = objlist;
            changes = 0;

            while (pos != NULL) {
                /*
                 * external objects may not have a initfun as they are externally
                 * initialised. For all others, we have to add the dependencies
                 * of their initfuns, as well!
                 */
                if (OBJDEF_INITFUN (SET_MEMBER (pos)) != NULL) {
                    changes
                      += TCsetUnion (&xnew,
                                     FUNDEF_OBJECTS (OBJDEF_INITFUN (SET_MEMBER (pos))));
                }
                pos = SET_NEXT (pos);
            }

            objlist = FREEdoFreeTree (objlist);
            objlist = xnew;
        } while (changes != 0);
    }

    DBUG_RETURN (objlist);
}

static node *
ObjdefsToInitAssigns (node *objdefs, node *assigns)
{
    node *result;

    DBUG_ENTER ();

    if (objdefs != NULL) {
        result = ObjdefsToInitAssigns (SET_NEXT (objdefs), assigns);
        if (OBJDEF_INITFUN (SET_MEMBER (objdefs)) != NULL) {
            result
              = TBmakeAssign (TBmakeLet (NULL,
                                         TBmakeAp (OBJDEF_INITFUN (SET_MEMBER (objdefs)),
                                                   TBmakeExprs (TBmakeGlobobj (
                                                                  SET_MEMBER (objdefs)),
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

    DBUG_ENTER ();

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
    result = TBmakeFundef (STRcpy ("init"), NSgetInitNamespace (), NULL, NULL,
                           TBmakeBlock (assigns, NULL), NULL);

    FUNDEF_OBJECTS (result) = objlist;
    FUNDEF_ISOBJINITFUN (result) = TRUE;

    DBUG_RETURN (result);
}

static node *
InsertInitFunCall (node *fun, node *initfun)
{
    DBUG_ENTER ();

    BLOCK_ASSIGNS (FUNDEF_BODY (fun))
      = TBmakeAssign (TBmakeLet (NULL, TBmakeAp (initfun, NULL)),
                      BLOCK_ASSIGNS (FUNDEF_BODY (fun)));

    DBUG_RETURN (fun);
}

/*
 * traversal functions
 */
node *
GOIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * check for function _MAIN::main
     */
    if (FUNDEF_ISMAIN (arg_node)) {
        if (FUNDEF_ISWRAPPERFUN (arg_node)) {
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
        } else {
            node *initfun;

            /*
             * first add all objects that are needed by the initfuns themselves
             * to the dependency list of main
             */
            FUNDEF_OBJECTS (arg_node)
              = AddInitFunDependencies (FUNDEF_OBJECTS (arg_node));

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
    DBUG_ENTER ();

    /*
     * we only generate object initialisers in programs
     * that make use of objects!
     */
    if ((MODULE_FILETYPE (arg_node) == FT_prog) && (MODULE_OBJS (arg_node) != NULL)) {
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

    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_goi);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
