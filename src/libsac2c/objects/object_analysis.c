#include "object_analysis.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "namespaces.h"
#include "new_types.h"
#include "DupTree.h"
#include "str.h"
#include "memory.h"
#include "ctinfo.h"

#define DBUG_PREFIX "OAN"
#include "debug.h"

#include "globals.h"
#include "specialization_oracle_static_shape_knowledge.h"
#include "strip_external_signatures.h"
#include "map_fun_trav.h"

/*
 * INFO structure
 */
struct INFO {
    node *objects;
    node *objdefs;
    node *fundefs;
    node *wrapper;
    int changes;
    bool wasused;
};

/*
 * INFO macros
 */
#define INFO_OBJECTS(n) ((n)->objects)
#define INFO_OBJDEFS(n) ((n)->objdefs)
#define INFO_FUNDEFS(n) ((n)->fundefs)
#define INFO_WRAPPER(n) ((n)->wrapper)
#define INFO_CHANGES(n) ((n)->changes)
#define INFO_WASUSED(n) ((n)->wasused)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_OBJECTS (result) = NULL;
    INFO_OBJDEFS (result) = NULL;
    INFO_FUNDEFS (result) = NULL;
    INFO_WRAPPER (result) = NULL;
    INFO_CHANGES (result) = 0;
    INFO_WASUSED (result) = FALSE;

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
 * Local helper function
 */
static node *
ResetArgs (node *args)
{
    DBUG_ENTER ();

    if (args != NULL) {
        ARG_HASLINKSIGNINFO (args) = FALSE;
        ARG_LINKSIGN (args) = 0;
        ARG_ISREFCOUNTED (args) = TRUE;

        ARG_NEXT (args) = ResetArgs (ARG_NEXT (args));
    }

    DBUG_RETURN (args);
}

static node *
ResetRets (node *rets)
{
    DBUG_ENTER ();

    if (rets != NULL) {
        RET_HASLINKSIGNINFO (rets) = FALSE;
        RET_LINKSIGN (rets) = 0;
        RET_ISREFCOUNTED (rets) = TRUE;

        RET_NEXT (rets) = ResetRets (RET_NEXT (rets));
    }

    DBUG_RETURN (rets);
}

static node *
CreateObjectWrapper (node *wrapper, node *fundef)
{
    node *result;
    node *body;
    node *block;
    node *ids;
    node *vardecs = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("Creating object wrapper for %s for " F_PTR "...", CTIitemName (fundef),
                (void *)fundef);
    DBUG_PRINT ("The corresponding wrapper is %s...", CTIitemName (wrapper));

    /*
     * remove body for copying
     */
    body = FUNDEF_BODY (fundef);
    FUNDEF_BODY (fundef) = NULL;

    /*
     * create a localized copy of the function header in a view
     * that corresponds to the wrappers namespace
     */
    result = DUPdoDupNode (fundef);
    FUNDEF_NS (result) = NSfreeNamespace (FUNDEF_NS (result));
    if (NSequals (FUNDEF_NS (wrapper), global.modulenamespace)) {
        /*
         * this is a local wrapper, so we put the object wrapper in
         * that namespace, too.
         */
        FUNDEF_NS (result) = NSdupNamespace (FUNDEF_NS (wrapper));
    } else {
        /*
         * the wrapper is non-local. So we create an according
         * view in the current namespace and put the object wrapper
         * into that.
         */
        FUNDEF_NS (result) = NSbuildView (FUNDEF_NS (wrapper));
    }

    /*
     * reset the functions state
     */
    FUNDEF_WASIMPORTED (result) = FALSE;
    FUNDEF_WASUSED (result) = FALSE;
    FUNDEF_ISLOCAL (result) = TRUE;
    result = SOSSKresetFundefDemand (result);
    result = SESstripOneFunction (result);
    if (FUNDEF_ISEXTERN (result)) {
        FUNDEF_ARGS (result) = ResetArgs (FUNDEF_ARGS (result));
        FUNDEF_RETS (result) = ResetRets (FUNDEF_RETS (result));
        if (FUNDEF_LINKNAME (result) != NULL) {
            FUNDEF_LINKNAME (result) = MEMfree (FUNDEF_LINKNAME (result));
        }
        FUNDEF_ISEXTERN (result) = FALSE;
    }

    /*
     * we mark it as inline, as this will get rid of the object
     * wrapper if a dispatch is possible.
     */
    FUNDEF_ISINLINE (result) = TRUE;

    /*
     * add body again
     */
    FUNDEF_BODY (fundef) = body;

    /*
     * we create an appropriate function body calling the original function
     */
    ids = TCcreateIdsFromRets (FUNDEF_RETS (result), &vardecs);

    block
      = TBmakeBlock (TBmakeAssign (TBmakeLet (ids,
                                              TBmakeAp (fundef, TCcreateExprsFromArgs (
                                                                  FUNDEF_ARGS (result)))),
                                   TBmakeAssign (TBmakeReturn (
                                                   TCcreateExprsFromIds (ids)),
                                                 NULL)),
                     NULL);

    BLOCK_VARDECS (block) = vardecs;
    FUNDEF_BODY (result) = block;

    FUNDEF_ISOBJECTWRAPPER (result) = TRUE;
    FUNDEF_IMPL (result) = fundef;

    DBUG_PRINT ("The result is %s...", CTIitemName (result));

    DBUG_RETURN (result);
}

static node *
CollectObjects (node *fundef, info *info)
{
    DBUG_ENTER ();

    TCsetUnion (&INFO_OBJECTS (info), FUNDEF_OBJECTS (fundef));

    DBUG_RETURN (fundef);
}

static node *
ProjectObjects (node *fundef, info *info)
{
    DBUG_ENTER ();

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
    } else if (global.runtime == 1) {
        /*
         * We're in runtime mode and want to pretend as though all instances
         * are local, so runtime loaded specializations work as expected.
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
         * insert special object wrappers. We pass the wrapper here to
         * figure out the target namespace.
         */
        if (INFO_OBJECTS (info) != NULL) {
            fundef = CreateObjectWrapper (INFO_WRAPPER (info), fundef);
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
    DBUG_ENTER ();

    while (funs != NULL) {
        if (FUNDEF_ISWRAPPERFUN (funs)) {
            DBUG_PRINT ("Unifying objects of function %s...", CTIitemName (funs));

            INFO_WRAPPER (info) = funs;

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

            INFO_WRAPPER (info) = NULL;
        }
        funs = FUNDEF_NEXT (funs);
    }

    DBUG_RETURN ();
}

static node *
LookupObjdef (namespace_t *ns, const char *name, node *objs)
{
    node *result;

    DBUG_ENTER ();

    if (objs == NULL) {
        result = NULL;
    } else {
        if (NSequals (OBJDEF_NS (objs), ns) && STReq (OBJDEF_NAME (objs), name)) {
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

    DBUG_ENTER ();

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

            DBUG_PRINT (">>> adding effect on %s...", CTIitemName (objdef));

            INFO_CHANGES (info) += TCsetAdd (&list, objdef);
        }

        *exprs = FREEdoFreeNode (*exprs);

        list = AddAffectedObjects (exprs, list, info);
    }

    DBUG_RETURN (list);
}

static node *
ProjectObjectsToFunSpecs (node *spec, info *arg_info)
{
    DBUG_ENTER ();

    FUNDEF_OBJECTS (spec) = DUPdoDupTree (FUNDEF_OBJECTS (FUNDEF_IMPL (spec)));

    DBUG_RETURN (spec);
}

/*
 * start of traversal
 */
node *
OANdoObjectAnalysis (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

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
    DBUG_ENTER ();

    INFO_OBJDEFS (arg_info) = MODULE_OBJS (arg_node);

    /*
     * first we iterate the FUNDECS once to add the
     * affectedobjects to the object list
     */
    DBUG_PRINT ("!!! processing fundecs...");

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

        DBUG_PRINT ("!!! starting new iteration");
        /*
         * we have to trust the programmer for FUNDECS,
         * so we only traverse FUNS
         */

        if (MODULE_FUNS (arg_node) != NULL) {
            MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
        }

        DBUG_PRINT ("!!! last iteration added %d objects.", INFO_CHANGES (arg_info));

        /*
         * propagate the infered objects to the wrappers and ensure
         * that all instances are marked for the same set of objects.
         * This is important as otherwise the signature of a wrapper
         * and its instances would not match once the objects have
         * been resolved. Furthermore, it has to be done after each
         * iteration as otherwise the object usage is not propagated
         * over wrapper calls!
         */
        DBUG_PRINT ("unifying dependencies of overloaded instances...");

        UnifyOverloadedFunctions (MODULE_FUNS (arg_node), arg_info);

        DBUG_PRINT ("unifying completed.");
    } while (INFO_CHANGES (arg_info) != 0);

    /*
     * append the created object wrappers
     */
    if (INFO_FUNDEFS (arg_info) != NULL) {
        MODULE_FUNS (arg_node)
          = TCappendFundef (MODULE_FUNS (arg_node), INFO_FUNDEFS (arg_info));
        INFO_FUNDEFS (arg_info) = NULL;
    }

    /*
     * project object information on funspecs
     */
    MODULE_FUNSPECS (arg_node)
      = MFTdoMapFunTrav (MODULE_FUNSPECS (arg_node), NULL, ProjectObjectsToFunSpecs);

    /*
     * check for selfdependencies in object initialisers
     */
    MODULE_OBJS (arg_node) = TRAVopt (MODULE_OBJS (arg_node), arg_info);
    CTIabortOnError ();

    DBUG_RETURN (arg_node);
}

node *
OANglobobj (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (GLOBOBJ_OBJDEF (arg_node) != NULL, "found a global id without objdef!");

    DBUG_PRINT (">>> adding object %s", CTIitemName (GLOBOBJ_OBJDEF (arg_node)));

    INFO_CHANGES (arg_info)
      += TCsetAdd (&INFO_OBJECTS (arg_info), GLOBOBJ_OBJDEF (arg_node));

    DBUG_RETURN (arg_node);
}

node *
OANap (node *arg_node, info *arg_info)
{
    int newdeps;

    DBUG_ENTER ();

    DBUG_PRINT (">>> adding dependencies of %s", CTIitemName (AP_FUNDEF (arg_node)));

    newdeps
      = TCsetUnion (&INFO_OBJECTS (arg_info), FUNDEF_OBJECTS (AP_FUNDEF (arg_node)));

    DBUG_PRINT (">>> %d dependencies added", newdeps);

    INFO_CHANGES (arg_info) += newdeps;

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
OANfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (INFO_OBJECTS (arg_info) == NULL,
                 "entering fundef with objects left over ?!?");

    /*
     * only process local functions, all others do have
     * correct annotations already!
     */
    if (FUNDEF_ISLOCAL (arg_node)) {
        DBUG_PRINT ("entering fundef %s", CTIitemName (arg_node));

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

        DBUG_PRINT ("leaving fundef %s", CTIitemName (arg_node));
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
OANobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((OBJDEF_INITFUN (arg_node) != NULL)
        && (TCsetContains (FUNDEF_OBJECTS (OBJDEF_INITFUN (arg_node)), arg_node))) {
        CTIerror (EMPTY_LOC, 
                  "The initialisation expression of global object `%s' depends on "
                  "its own result (the global object). Most likely it uses a "
                  "function that requires the object to already exist.",
                  CTIitemName (arg_node));
    }

    OBJDEF_NEXT (arg_node) = TRAVopt (OBJDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
