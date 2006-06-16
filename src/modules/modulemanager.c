/*
 *
 * $Id$
 *
 */

#include "modulemanager.h"
#include "libmanager.h"
#include "filemgr.h"
#include "ctinfo.h"
#include "dbug.h"
#include "build.h"
#include "internal_lib.h"
#include "tree_basic.h"

#include <string.h>

#undef MODM_UNLOAD_MODULES

struct MODULE_T {
    char *name;
    char *sofile;
    dynlib_t lib;
    sttable_t *stable;
    module_t *next;
    int usecount;
};

static module_t *modulepool = NULL;

typedef sttable_t *(*symtabfun_p) ();
typedef stringset_t *(*deptabfun_p) ();
typedef const char *(*modversionfun_p) ();
typedef void (*nsmapfun_p) ();

static bool
hasSameASTVersion (module_t *module)
{
    modversionfun_p verfun;
    char *name;

    DBUG_ENTER ("hasSameASTVersion");

    name = ILIBmalloc (sizeof (char) * (strlen (module->name) + 12));
    sprintf (name, "__%s_VERSION", module->name);

    verfun = (modversionfun_p)LIBMgetLibraryFunction (name, module->lib);

    if (verfun == NULL) {
        CTIabort ("Error loading module version information: %s", LIBMgetError ());
    }

    name = ILIBfree (name);

    DBUG_RETURN (ILIBstringCompare (verfun (), build_ast));
}

static void
addNamespaceMappings (module_t *module)
{
    nsmapfun_p mapfun;
    char *name;

    DBUG_ENTER ("addNamespaceMappings");

    name = ILIBmalloc (sizeof (char) * (strlen (module->name) + 19));
    sprintf (name, "__%s__MapConstructor", module->name);

    mapfun = (nsmapfun_p)LIBMgetLibraryFunction (name, module->lib);

    if (mapfun == NULL) {
        CTIabort ("Error loading namespace mapping information: %s", LIBMgetError ());
    }

    mapfun ();

    name = ILIBfree (name);

    DBUG_VOID_RETURN;
}

static module_t *
LookupModuleInPool (const char *name)
{
    module_t *result = NULL;
    module_t *pos = modulepool;

    DBUG_ENTER ("LookupModuleInPool");

    while ((result == NULL) && (pos != NULL)) {
        if (!strcmp (pos->name, name)) {
            result = pos;
            result->usecount++;
        }
        pos = pos->next;
    }

    DBUG_RETURN (result);
}

static module_t *
AddModuleToPool (const char *name)
{
    module_t *result;
    char *tmp;

    DBUG_ENTER ("AddModuleToPool");

    DBUG_PRINT ("MODM", ("Start loading module '%s'.", name));

    result = ILIBmalloc (sizeof (module_t));

    tmp = ILIBmalloc (sizeof (char) * (strlen (name) + 7));
    sprintf (tmp, "lib%s.so", name);

    result->sofile = ILIBstringCopy (FMGRfindFile (PK_lib_path, tmp));

    if (result->sofile == NULL) {
        CTIabort ("Cannot find library `%s' for module `%s'", tmp, name);
    }

    DBUG_PRINT ("MODM", ("Found library file '%s'.", result->sofile));

    tmp = ILIBfree (tmp);

    result->name = ILIBstringCopy (name);
    result->lib = LIBMloadLibrary (result->sofile);
    result->stable = NULL;
    result->next = modulepool;
    modulepool = result;
    result->usecount = 1;

    if (result->lib == NULL) {
        CTIabort ("Unable to open module `%s'. The reported error was: %s", name,
                  LIBMgetError ());
    }

    if (!hasSameASTVersion (result)) {
        CTIabort ("Module `%s' [%s] was compiled using an incompatible version of "
                  "sac2c.",
                  name, result->sofile);
    }

    addNamespaceMappings (result);

    DBUG_RETURN (result);
}

static module_t *
RemoveModuleFromPool (module_t *module)
{
    DBUG_ENTER ("RemoveModuleFromPool");

    module->usecount--;

    DBUG_PRINT ("MODM",
                ("Module %s has usage count %d.", module->name, module->usecount));

    module = NULL;

    DBUG_RETURN (module);
}

const char *
MODMgetModuleName (module_t *module)
{
    DBUG_ENTER ("MODMgetModuleName");

    DBUG_ASSERT ((module != NULL), "MODMgetModuleName called with NULL pointer");

    DBUG_RETURN (module->name);
}

module_t *
MODMloadModule (const char *name)
{
    module_t *result;

    DBUG_ENTER ("MODMloadModule");

    result = LookupModuleInPool (name);

    if (result == NULL) {
        result = AddModuleToPool (name);
    }

    DBUG_RETURN (result);
}

module_t *
MODMunLoadModule (module_t *module)
{
    DBUG_ENTER ("MODMunLoadModule");

    module = RemoveModuleFromPool (module);

    DBUG_RETURN (module);
}

static symtabfun_p
GetSymbolTableFunction (module_t *module)
{
    char *name;
    symtabfun_p result;

    DBUG_ENTER ("GetSymbolTableFunction");

    name = ILIBmalloc (sizeof (char) * (strlen (module->name) + 11));
    sprintf (name, "__%s__SYMTAB", module->name);

    result = (symtabfun_p)LIBMgetLibraryFunction (name, module->lib);

    if (result == NULL) {
        CTIabort ("Error loading symbol table: %s", LIBMgetError ());
    }

    name = ILIBfree (name);

    DBUG_RETURN (result);
}

const sttable_t *
MODMgetSymbolTable (module_t *module)
{
    symtabfun_p symtabfun;

    DBUG_ENTER ("MODMgetSymbolTable");

    if (module->stable == NULL) {
        symtabfun = GetSymbolTableFunction (module);

        module->stable = symtabfun ();
    }

    DBUG_RETURN (module->stable);
}

static deptabfun_p
GetDependencyTableFunction (module_t *module)
{
    deptabfun_p result;
    char *name;

    DBUG_ENTER ("GetDependencyTableFunction");

    name = ILIBmalloc (sizeof (char) * (strlen (module->name) + 11));
    sprintf (name, "__%s__DEPTAB", module->name);

    result = (deptabfun_p)LIBMgetLibraryFunction (name, module->lib);

    if (result == NULL) {
        CTIabort ("Error loading dependency table: %s", LIBMgetError ());
    }

    name = ILIBfree (name);

    DBUG_RETURN (result);
}

stringset_t *
MODMgetDependencyTable (module_t *module)
{
    deptabfun_p deptabfun;
    stringset_t *result;

    DBUG_ENTER ("MODMgetDependencyTable");

    deptabfun = GetDependencyTableFunction (module);

    result = deptabfun ();

    DBUG_RETURN (result);
}

serfun_p
MODMgetDeSerializeFunction (const char *name, module_t *module)
{
    serfun_p result;

    DBUG_ENTER ("MODMgetDeSerializeFunction");

    result = (serfun_p)LIBMgetLibraryFunction (name, module->lib);

    DBUG_RETURN (result);
}
