/*
 *
 * $Id$
 *
 */

#include "modulemanager.h"
#include "libmanager.h"
#include "serialize.h"
#include "filemgr.h"
#include "ctinfo.h"
#include "dbug.h"
#include "build.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
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
typedef const char *(*astversionfun_p) ();
typedef int (*serversionfun_p) ();
typedef int (*flagfun_p) ();
typedef void (*nsmapfun_p) ();

static void
checkHasSameASTVersion (module_t *module)
{
    astversionfun_p astverfun;
    serversionfun_p serverfun;
    char *name;

    DBUG_ENTER ("checkHasSameASTVersion");

    name = MEMmalloc (sizeof (char) * (strlen (module->name) + 14));
    sprintf (name, "__%s_ASTVERSION", module->name);

    astverfun = (astversionfun_p)LIBMgetLibraryFunction (name, module->lib);

    if (astverfun == NULL) {
        CTIabort ("The module '%s' is either corrupted or uses an outdated "
                  "file format.",
                  module->name);
    }

    if (!STReq (astverfun (), build_ast)) {
        CTIabort ("The module '%s' uses an incompatible syntax tree layout. "
                  "Please update the module and compiler to the most "
                  "recent version.",
                  module->name);
    }

    sprintf (name, "__%s_SERIALIZER", module->name);

    serverfun = (serversionfun_p)LIBMgetLibraryFunction (name, module->lib);

    if (astverfun == NULL) {
        CTIabort ("The module '%s' is either corrupted or uses an outdated "
                  "file format.",
                  module->name);
    }

    name = MEMfree (name);

    if (!(serverfun () == SAC_SERIALIZE_VERSION)) {
        CTIabort ("The module '%s' uses an incompatible serialisation algorithm. "
                  "Please update the module and compiler to the most "
                  "recent version.",
                  module->name);
    }

    DBUG_VOID_RETURN;
}

static void
checkWasBuildUsingSameFlags (module_t *module)
{
    flagfun_p flagfun;
    char *name;

    DBUG_ENTER ("wasBuildUsingSameFlags");

    name = MEMmalloc (sizeof (char) * (strlen (module->name) + 13));
    sprintf (name, "__%s_USEDFLAGS", module->name);

    flagfun = (flagfun_p)LIBMgetLibraryFunction (name, module->lib);

    if (flagfun == NULL) {
        CTIabort ("The module '%s' is either corrupted or uses an outdated "
                  "file format.",
                  module->name);
    }

    if (!(flagfun () == GLOBALS_MODFLAGS)) {
        CTIabort ("The module '%s' was built using incompatible compiler "
                  "settings. The settings were [%s]. Please recompile the "
                  "module using the current settings or switch the "
                  "settings used when compiling the module.",
                  module->name, GLOBALS_MODFLAGS_TEXT (flagfun ()));
    }

    DBUG_VOID_RETURN;
}

static void
addNamespaceMappings (module_t *module)
{
    nsmapfun_p mapfun;
    char *name;

    DBUG_ENTER ("addNamespaceMappings");

    name = MEMmalloc (sizeof (char) * (strlen (module->name) + 19));
    sprintf (name, "__%s__MapConstructor", module->name);

    mapfun = (nsmapfun_p)LIBMgetLibraryFunction (name, module->lib);

    if (mapfun == NULL) {
        CTIabort ("Error loading namespace mapping information: %s", LIBMgetError ());
    }

    mapfun ();

    name = MEMfree (name);

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

    result = MEMmalloc (sizeof (module_t));

    tmp = MEMmalloc (sizeof (char) * (strlen (name) + 7));
    sprintf (tmp, "lib%s.so", name);

    result->sofile = STRcpy (FMGRfindFile (PK_lib_path, tmp));

    if (result->sofile == NULL) {
        CTIabort ("Cannot find library `%s' for module `%s'", tmp, name);
    }

    DBUG_PRINT ("MODM", ("Found library file '%s'.", result->sofile));

    tmp = MEMfree (tmp);

    result->name = STRcpy (name);
    result->lib = LIBMloadLibrary (result->sofile);
    result->stable = NULL;
    result->next = modulepool;
    modulepool = result;
    result->usecount = 1;

    if (result->lib == NULL) {
        CTIabort ("Unable to open module `%s'. The reported error was: %s", name,
                  LIBMgetError ());
    }

    checkHasSameASTVersion (result);
    checkWasBuildUsingSameFlags (result);

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

    name = MEMmalloc (sizeof (char) * (strlen (module->name) + 11));
    sprintf (name, "__%s__SYMTAB", module->name);

    result = (symtabfun_p)LIBMgetLibraryFunction (name, module->lib);

    if (result == NULL) {
        CTIabort ("Error loading symbol table: %s", LIBMgetError ());
    }

    name = MEMfree (name);

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

    name = MEMmalloc (sizeof (char) * (strlen (module->name) + 11));
    sprintf (name, "__%s__DEPTAB", module->name);

    result = (deptabfun_p)LIBMgetLibraryFunction (name, module->lib);

    if (result == NULL) {
        CTIabort ("Error loading dependency table: %s", LIBMgetError ());
    }

    name = MEMfree (name);

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
