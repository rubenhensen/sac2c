/* $Id$ */

#include "object_analysis.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "namespaces.h"
#include "new_types.h"
#include "DupTree.h"
#include "internal_lib.h"

/*
 * INFO structure
 */
struct INFO {
    node *objects;
    node *objdefs;
    node *fundefs;
    int changes;
    bool wasused;
};

/*
 * INFO macros
 */
#define INFO_OBJECTS(n) ((n)->objects)
#define INFO_OBJDEFS(n) ((n)->objdefs)
#define INFO_FUNDEFS(n) ((n)->fundefs)
#define INFO_CHANGES(n) ((n)->changes)
#define INFO_WASUSED(n) ((n)->wasused)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_OBJECTS (result) = NULL;
    INFO_OBJDEFS (result) = NULL;
    INFO_FUNDEFS (result) = NULL;
    INFO_CHANGES (result) = 0;
    INFO_WASUSED (result) = FALSE;

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
 * Local helper function
 */
static node *
CreateObjectWrapper (node *fundef)
{
    node *result;
    node *body;
    node *block;
    node *ids;
    node *vardecs = NULL;

    DBUG_ENTER ("CreateObjectWrapper");

    DBUG_PRINT ("OAN", ("Creating object wrapper for %s...", CTIitemName (fundef)));

    /*
     * remove body for copying
     */
    body = FUNDEF_BODY (fundef);
    FUNDEF_BODY (fundef) = NULL;

    /*
     * create a localized copy of the function header
     */
    result = DUPdoDupNode (fundef);
    FUNDEF_NS (result) = NSfreeNamespace (FUNDEF_NS (result));
    FUNDEF_NS (result) = NSdupNamespace (global.modulenamespace),
              FUNDEF_WASIMPORTED (result) = FALSE;
    FUNDEF_WASUSED (result) = FALSE;
    FUNDEF_ISLOCAL (result) = TRUE;

    /*
     * add body again
     */
    FUNDEF_BODY (fundef) = body;

    /*
     * for non external funs we create an appropriate
     * function body calling the original function
     */
    if (!FUNDEF_ISEXTERN (fundef)) {
        ids = TCcreateIdsFromRets (FUNDEF_RETS (result), &vardecs);

        block = TBmakeBlock (TBmakeAssign (TBmakeLet (ids,
                                                      TBmakeAp (fundef,
                                                                TCcreateExprsFromArgs (
                                                                  FUNDEF_ARGS (result)))),
                                           TBmakeAssign (TBmakeReturn (
                                                           TCcreateExprsFromIds (ids)),
                                                         NULL)),
                             NULL);

        BLOCK_VARDEC (block) = vardecs;
        FUNDEF_BODY (result) = block;
    }

    FUNDEF_ISOBJECTWRAPPER (result) = TRUE;
    FUNDEF_IMPL (result) = fundef;

    DBUG_RETURN (result);
}

static node *
CollectObjects (node *fundef, info *info)
{
    DBUG_ENTER ("CollectObjects");

    TCSetUnion (&INFO_OBJECTS (info), FUNDEF_OBJECTS (fundef));

    DBUG_RETURN (fundef);
}

static node *
ProjectObjects (node *fundef, info *info)
{
    DBUG_ENTER ("ProjectObjects");

    if ((FUNDEF_ISLOCAL (fundef) && !FUNDEF_WASIMPORTED (fundef))
        || INFO_WASUSED (info)) {
        /*
         * this is either a local instance or the entire
         * wrapper is not local and thus all its instances.
         * In both cases we can safely modify the instances.
         */
        if (FUNDEF_OBJECTS (fundef) != NULL) {
            FUNDEF_OBJECTS (fundef) = FREEdoFreeTree (FUNDEF_OBJECTS (fundef));
        }
        if (INFO_OBJECTS (info) != NULL) {
            FUNDEF_OBJECTS (fundef) = DUPdoDupTree (INFO_OBJECTS (info));
        }
    } else {
        /*
         * we cannot modify the object dependencies for non local functions
         * as these might be shared between multiple wrappers. Thus, we
         * insert special object wrappers.
         */
        if (INFO_OBJECTS (info) != NULL) {
            fundef = CreateObjectWrapper (fundef);
            INFO_FUNDEFS (info) = TCappendFundef (INFO_FUNDEFS (info), fundef);

            if (FUNDEF_OBJECTS (fundef) != NULL) {
                FUNDEF_OBJECTS (fundef) = FREEdoFreeTree (FUNDEF_OBJECTS (fundef));
            }
            if (INFO_OBJECTS (info) != NULL) {
                FUNDEF_OBJECTS (fundef) = DUPdoDupTree (INFO_OBJECTS (info));
            }
        }
    }

    DBUG_RETURN (fundef);
}

static void
UnifyOverloadedFunctions (node *funs, info *info)
{
    DBUG_ENTER ("UnifyOverloadedFunctions");

    while (funs != NULL) {
        if (FUNDEF_ISWRAPPERFUN (funs)) {
            DBUG_PRINT ("OAN",
                        ("Unifying objects of function %s...", CTIitemName (funs)));

            if (TYisFun (FUNDEF_WRAPPERTYPE (funs))) {
                INFO_OBJECTS (info) = FUNDEF_OBJECTS (funs);
                INFO_WASUSED (info) = FUNDEF_WASUSED (funs);

                FUNDEF_WRAPPERTYPE (funs)
                  = TYmapFunctionInstances (FUNDEF_WRAPPERTYPE (funs), CollectObjects,
                                            info);

                FUNDEF_WRAPPERTYPE (funs)
                  = TYmapFunctionInstances (FUNDEF_WRAPPERTYPE (funs), ProjectObjects,
                                            info);

                FUNDEF_OBJECTS (funs) = INFO_OBJECTS (info);
                INFO_OBJECTS (info) = NULL;
            } else {
                if (FUNDEF_OBJECTS (funs) != NULL) {
                    FUNDEF_OBJECTS (funs) = FREEdoFreeTree (FUNDEF_OBJECTS (funs));
                }
                if (FUNDEF_OBJECTS (FUNDEF_IMPL (funs)) != NULL) {
                    FUNDEF_OBJECTS (funs)
                      = DUPdoDupTree (FUNDEF_OBJECTS (FUNDEF_IMPL (funs)));
                }
            }
        }
        funs = FUNDEF_NEXT (funs);
    }

    DBUG_VOID_RETURN;
}

static node *
LookupObjdef (namespace_t *ns, const char *name, node *objs)
{
    node *result;

    DBUG_ENTER ("LookupObjdef");

    if (objs == NULL) {
        result = NULL;
    } else {
        if (NSequals (OBJDEF_NS (objs), ns)
            && ILIBstringCompare (OBJDEF_NAME (objs), name)) {
            result = objs;
        } else {
            result = LookupObjdef (ns, name, OBJDEF_NEXT (objs));
        }
    }

    DBUG_RETURN (result);
}

static node *
AddAffectedObjects (node **exprs, node *list, info *info)
{
    node *objdef;
    node *spid;

    DBUG_ENTER ("AddAffectedObjects");

    if (*exprs != NULL) {
        spid = EXPRS_EXPR (*exprs);

        objdef = LookupObjdef (SPID_NS (spid), SPID_NAME (spid), INFO_OBJDEFS (info));

        if (objdef == NULL) {
            CTIerrorLine (NODE_LINE (spid),
                          "Objdef %s:%s referenced in effect pragma is undefined",
                          NSgetName (SPID_NS (spid)), SPID_NAME (spid));
        } else {
            /*
             * remove all aliasings first. We do this here for
             * the sake of a better error message above!
             */
            objdef = TCunAliasObjdef (objdef);

            DBUG_PRINT ("OAN", (">>> adding effect on %s...", CTIitemName (objdef)));

            INFO_CHANGES (info) += TCSetAdd (&list, objdef);
        }

        *exprs = FREEdoFreeNode (*exprs);

        list = AddAffectedObjects (exprs, list, info);
    }

    DBUG_RETURN (list);
}

/*
 * start of traversal
 */
node *
OANdoObjectAnalysis (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("OANdoObjectAnalysis");

    info = MakeInfo ();

    TRAVpush (TR_oan);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/*
 * Traversal functions
 */
node *
OANmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OANModule");

    INFO_OBJDEFS (arg_info) = MODULE_OBJS (arg_node);

    /*
     * first we iterate the FUNDECS once to add the
     * affectedobjects to the object list
     */
    DBUG_PRINT ("OAN", ("!!! processing fundecs..."));

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    /*
     * iterate until the set of objects does not
     * change any more
     */
    do {
        /*
         * reset changes counter
         */
        INFO_CHANGES (arg_info) = 0;

        DBUG_PRINT ("OAN", ("!!! starting new iteration"));
        /*
         * we have to trust the programmer for FUNDECS,
         * so we only traverse FUNS
         */

        if (MODULE_FUNS (arg_node) != NULL) {
            MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
        }

        DBUG_PRINT ("OAN",
                    ("!!! last iteration added %d objects.", INFO_CHANGES (arg_info)));

        /*
         * propagate the infered objects to the wrappers and ensure
         * that all instances are marked for the same set of objects.
         * This is important as otherwise the signature of a wrapper
         * and its instances would not match once the objects have
         * been resolved. Furthermore, it has to be done after each
         * iteration as otherwise the object usage is not propagated
         * over wrapper calls!
         */
        DBUG_PRINT ("OAN", ("unifying dependencies of overloaded instances..."));

        UnifyOverloadedFunctions (MODULE_FUNS (arg_node), arg_info);

        DBUG_PRINT ("OAN", ("unifying completed."));
    } while (INFO_CHANGES (arg_info) != 0);

    /*
     * append the created object wrappers
     */
    if (INFO_FUNDEFS (arg_info) != NULL) {
        MODULE_FUNS (arg_node)
          = TCappendFundef (MODULE_FUNS (arg_node), INFO_FUNDEFS (arg_info));
        INFO_FUNDEFS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
OANglobobj (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OANglobobj");

    DBUG_ASSERT ((GLOBOBJ_OBJDEF (arg_node) != NULL),
                 "found a global id without objdef!");

    DBUG_PRINT ("OAN", (">>> adding object %s", CTIitemName (GLOBOBJ_OBJDEF (arg_node))));

    INFO_CHANGES (arg_info)
      += TCSetAdd (&INFO_OBJECTS (arg_info), GLOBOBJ_OBJDEF (arg_node));

    DBUG_RETURN (arg_node);
}

node *
OANap (node *arg_node, info *arg_info)
{
    int newdeps;

    DBUG_ENTER ("OANap");

    DBUG_PRINT ("OAN",
                (">>> adding dependencies of %s", CTIitemName (AP_FUNDEF (arg_node))));

    newdeps
      = TCSetUnion (&INFO_OBJECTS (arg_info), FUNDEF_OBJECTS (AP_FUNDEF (arg_node)));

    DBUG_PRINT ("OAN", (">>> %d dependencies added", newdeps));

    INFO_CHANGES (arg_info) += newdeps;

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
OANfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OANfundef");

    DBUG_ASSERT ((INFO_OBJECTS (arg_info) == NULL),
                 "entering fundef with objects left over ?!?");

    /*
     * only process local functions, all others do have
     * correct annotations already!
     */
    if (FUNDEF_ISLOCAL (arg_node)) {
        DBUG_PRINT ("OAN", ("entering fundef %s", CTIitemName (arg_node)));

        /*
         * first add the objects from affectedobjects if any present
         */
        FUNDEF_OBJECTS (arg_node)
          = AddAffectedObjects (&FUNDEF_AFFECTEDOBJECTS (arg_node),
                                FUNDEF_OBJECTS (arg_node), arg_info);

        INFO_OBJECTS (arg_info) = FUNDEF_OBJECTS (arg_node);

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }

        FUNDEF_OBJECTS (arg_node) = INFO_OBJECTS (arg_info);
        INFO_OBJECTS (arg_info) = NULL;

        DBUG_PRINT ("OAN", ("leaving fundef %s", CTIitemName (arg_node)));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
