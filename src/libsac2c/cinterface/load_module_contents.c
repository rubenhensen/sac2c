#include "load_module_contents.h"

#define DBUG_PREFIX "LMC"
#include "debug.h"

#include "globals.h"
#include "tree_basic.h"
#include "namespaces.h"
#include "stringset.h"
#include "deserialize.h"
#include "modulemanager.h"
#include "symboltable.h"
#include "stringset.h"
#include "traverse.h"
#include "ctinfo.h"

static node *
CheckForObjdefs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CTIerror (EMPTY_LOC, "Wrapper cannot be built due to global object `%s'.",
              CTIitemName (arg_node));

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = TRAVdo (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
LoadModule (const char *name, strstype_t kind, node *syntax_tree)
{
    module_t *module;
    const sttable_t *table;
    stsymboliterator_t *iterator;

    DBUG_ENTER ();

    DBUG_PRINT ("Loading module '%s'.", name);

    module = MODMloadModule (name);
    table = MODMgetSymbolTable (module);
    iterator = STsymbolIteratorGet (table);

    while (STsymbolIteratorHasMore (iterator)) {
        stsymbol_t *symbol = STsymbolIteratorNext (iterator);

        if ((STsymbolVisibility (symbol) == SVT_provided)
            || (STsymbolVisibility (symbol) == SVT_exported)) {
            DBUG_PRINT ("Adding Symbol '%s'.", STsymbolName (symbol));

            DSaddSymbolByName (STsymbolName (symbol), SET_wrapperhead, name);
        }
    }

    iterator = STsymbolIteratorRelease (iterator);
    module = MODMunLoadModule (module);

    global.dependencies = STRSadd (name, STRS_saclib, global.dependencies);

    DBUG_RETURN (syntax_tree);
}

node *
LMCdoLoadModuleContents (node *syntax_tree)
{
    anontrav_t checktypes[2] = {{N_objdef, &CheckForObjdefs}, {(nodetype)0, NULL}};

    DBUG_ENTER ();

    DBUG_ASSERT (syntax_tree == NULL,
                 "SMCdoLoadModuleContents can only be called as long as no syntax tree "
                 "has been created!");

    syntax_tree
      = TBmakeModule (NSgetCWrapperNamespace (), FT_cmod, NULL, NULL, NULL, NULL, NULL);

    DSinitDeserialize (syntax_tree);

    syntax_tree = (node *)STRSfold ((strsfoldfun_p)&LoadModule, global.exported_modules,
                                    syntax_tree);

    DSfinishDeserialize (syntax_tree);

    if (MODULE_OBJS (syntax_tree) != NULL) {
        TRAVpushAnonymous (checktypes, &TRAVsons);
        MODULE_OBJS (syntax_tree) = TRAVdo (MODULE_OBJS (syntax_tree), NULL);
        TRAVpop ();
    }

    CTIabortOnError ();

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
