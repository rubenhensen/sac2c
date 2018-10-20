#include "modulemanager.h"
#include "libmanager.h"
#include "serialize.h"
#include "filemgr.h"
#include "ctinfo.h"
#define DBUG_PREFIX "MODM"
#include "debug.h"
#include "build.h"
#include "str.h"
#include "stringset.h"
#include "memory.h"
#include "tree_basic.h"
#include "globals.h"
#include "config.h"

#if IS_CYGWIN
#include "cygwinhelpers.h"
#endif /* IS_CYGWIN */

#undef MODM_UNLOAD_MODULES

static module_t *modulepool = NULL;

typedef sttable_t *(*symtabfun_p) (void);
typedef union {
    void *v;
    sttable_t *(*f) (void);
} symtabfun_u;

typedef stringset_t *(*deptabfun_p) (void);
typedef union {
    void *v;
    stringset_t *(*f) (void);
} deptabfun_u;

typedef union {
    void *v;
    stringset_t *(*f) (void);
} modheaders_u;

typedef union {
    void *v;
    const char *(*f) (void);
} astversionfun_u;

typedef union {
    void *v;
    const char *(*f) (void);
} mixedcasenamefun_u;

typedef union {
    void *v;
    int (*f) (void);
} serversionfun_u;

typedef union {
    void *v;
    int (*f) (void);
} flagfun_u;

typedef union {
    void *v;
    const char *(*f) (void);
} deprecatedfun_u;

typedef union {
    void *v;
    void (*f) (void);
} nsmapfun_u;

static void
checkMixedCasesCorrect (module_t *module)
{
    mixedcasenamefun_u mixedcasenamefun;
    char *name;

    DBUG_ENTER ();

    name = (char *)MEMmalloc (sizeof (char) * (STRlen (module->name) + 17));
    sprintf (name, "__%s_MIXEDCASENAME", module->name);

    STRtoupper (name, 2, 2 + STRlen (module->name));

    mixedcasenamefun.v = LIBMgetLibraryFunction (name, module->lib);

    if (mixedcasenamefun.f == NULL)
        CTIabort ("The module '%s' (%s) is either corrupted or uses an outdated "
                  "file format.",
                  module->name, module->sofile);

    if (!STReq (mixedcasenamefun.f (), module->name))
        CTIabort ("Module '%s' not found; file-system search returned a module "
                  "named '%s'. "
                  "Most likely, you are using a case-insensitive filesystem. "
                  "Please check your module spelling and make sure you do "
                  "not attempt to use two modules that differ in their cases only.",
                  module->name, mixedcasenamefun.f ());

    DBUG_RETURN ();
}

static void
checkHasSameASTVersion (module_t *module)
{
    astversionfun_u astverfun;
    serversionfun_u serverfun;
    char *name;

    DBUG_ENTER ();

    name = (char *)MEMmalloc (sizeof (char) * (STRlen (module->name) + 14));
    sprintf (name, "__%s_ASTVERSION", module->name);

    astverfun.v = LIBMgetLibraryFunction (name, module->lib);

    if (astverfun.f == NULL)
        CTIabort ("The module '%s' (%s) is either corrupted or uses an outdated "
                  "file format.",
                  module->name, module->sofile);

    if (!STReq (astverfun.f (), build_ast))
        CTIabort ("The module '%s' (%s) uses an incompatible syntax tree layout. "
                  "Please update the module and compiler to the most "
                  "recent version.",
                  module->name, module->sofile);

    sprintf (name, "__%s_SERIALIZER", module->name);

    serverfun.v = LIBMgetLibraryFunction (name, module->lib);

    if (serverfun.f == NULL)
        CTIabort ("The module '%s' (%s) is either corrupted or uses an outdated "
                  "file format.",
                  module->name, module->sofile);

    name = MEMfree (name);

    if (!(serverfun.f () == SAC_SERIALIZE_VERSION))
        CTIabort ("The module '%s' (%s) uses an incompatible serialisation algorithm. "
                  "Please update the module and compiler to the most "
                  "recent version.",
                  module->name, module->sofile);

    DBUG_RETURN ();
}

static void
checkWasBuildUsingSameFlags (module_t *module)
{
    flagfun_u flagfun;
    char *name;

    DBUG_ENTER ();

    name = (char *)MEMmalloc (sizeof (char) * (STRlen (module->name) + 13));
    sprintf (name, "__%s_USEDFLAGS", module->name);

    flagfun.v = LIBMgetLibraryFunction (name, module->lib);

    if (flagfun.f == NULL)
        CTIabort ("The module '%s' (%s) is either corrupted or uses an outdated "
                  "file format.",
                  module->name, module->sofile);

#if 0
  if (!(flagfun( ) == GLOBALS_MODFLAGS))
    CTIabort( "The module '%s' (%s) was built using incompatible compiler "
              "settings. The settings were [%s]. Please recompile the "
              "module using the current settings or switch the "
              "settings used when compiling the module.", module->name,
              module->sofile, GLOBALS_MODFLAGS_TEXT( flagfun( )));

#endif

    DBUG_RETURN ();
}

static void
checkWhetherDeprecated (module_t *module)
{
    deprecatedfun_u dfun;
    char *name;
    const char *msg;

    DBUG_ENTER ();

    name = (char *)MEMmalloc (sizeof (char) * (STRlen (module->name) + 14));
    sprintf (name, "__%s_DEPRECATED", module->name);

    dfun.v = LIBMgetLibraryFunction (name, module->lib);

    if (dfun.f == NULL)
        CTIabort ("The module '%s' (%s) is either corrupted or uses an outdated "
                  "file format.",
                  module->name, module->sofile);

    msg = dfun.f ();
    if (msg != NULL)
        CTIwarn ("The module '%s' (%s) is deprecated: %s", module->name, module->sofile,
                 msg);

    DBUG_RETURN ();
}

static stringset_t *
loadModuleHeaders (module_t *module)
{
    modheaders_u result;
    char *name;

    DBUG_ENTER ();

    name = (char *)MEMmalloc (sizeof (char) * (STRlen (module->name) + 11));
    sprintf (name, "__%s_HEADERS", module->name);
    result.v = LIBMgetLibraryFunction (name, module->lib);

    if (result.f == NULL)
        CTIabort ("Error loading external header information for "
                  "module `%s' (%s): %s",
                  module->name, module->sofile, LIBMgetError ());

    name = MEMfree (name);

    DBUG_RETURN (result.f ());
}

static void
addNamespaceMappings (module_t *module)
{
    nsmapfun_u mapfun;
    char *name;

    DBUG_ENTER ();

    name = (char *)MEMmalloc (sizeof (char) * (STRlen (module->name) + 19));
    sprintf (name, "__%s__MapConstructor", module->name);

    mapfun.v = LIBMgetLibraryFunction (name, module->lib);

    if (mapfun.f == NULL)
        CTIabort ("Error loading namespace mapping information for "
                  "module `%s' (%s): %s",
                  module->name, module->sofile, LIBMgetError ());

    mapfun.f ();

    name = MEMfree (name);

    DBUG_RETURN ();
}

static module_t *
LookupModuleInPool (const char *name)
{
    module_t *result = NULL;
    module_t *pos = modulepool;

    DBUG_ENTER ();

    while ((result == NULL) && (pos != NULL)) {
        if (STReq (pos->name, name)) {
            result = pos;
            result->usecount++;
        }
        pos = pos->next;
    }

    DBUG_RETURN (result);
}

bool
MODMmoduleExists (const char *name)
{
    char *treerelpath = STRcatn (6, "tree/", global.config.target_env, "/lib", name,
                                 "Tree", global.config.tree_dllext);

    bool r = !!FMGRfindFile (PK_tree_path, treerelpath);
    MEMfree (treerelpath);
    return r;
}

static module_t *
AddModuleToPool (const char *name)
{
    module_t *result;

    DBUG_ENTER ();

#if IS_CYGWIN
    /*
     * Done under cygwin to resolve symbols at compile time.
     * This adds the dependency to the set global.dependencies.
     */
    CYGHaddToDeps (name);
#endif

    DBUG_PRINT ("Start loading module '%s'.", name);

    result = (module_t *)MEMmalloc (sizeof (module_t));

    char *treerelpath = STRcatn (6, "tree/", global.config.target_env, "/lib", name,
                                 "Tree", global.config.tree_dllext);

    result->sofile = STRcpy (FMGRfindFile (PK_tree_path, treerelpath));

    if (result->sofile == NULL)
        CTIabort ("Cannot find library `%s' for module `%s'", treerelpath, name);

    MEMfree (treerelpath);

    DBUG_PRINT ("Found library file '%s'.", result->sofile);

    result->name = STRcpy (name);
    result->lib = LIBMloadLibrary (result->sofile);
    result->headers = loadModuleHeaders (result);
    result->stable = NULL;
    result->next = modulepool;
    modulepool = result;
    result->usecount = 1;

    if (result->lib == NULL)
        CTIabort ("Unable to open module `%s' (%s). The reported error was: %s", name,
                  result->sofile, LIBMgetError ());

#if IS_CYGWIN
    CYGHpassToMod (result->lib);
#endif /* IS_CYGWIN */

    checkMixedCasesCorrect (result);
    checkHasSameASTVersion (result);
    checkWhetherDeprecated (result);

    switch (global.tool) {
    case TOOL_sac2c:
        /*
         * we only need to check this for those tools
         * that actually compile sac code!
         */
        checkWasBuildUsingSameFlags (result);
        break;
    case TOOL_sac4c:
    case TOOL_sac2tex:
        break;
    default:
        DBUG_UNREACHABLE ("unknown tool!");
    }

    addNamespaceMappings (result);

    DBUG_RETURN (result);
}

static module_t *
RemoveModuleFromPool (module_t *module)
{
    DBUG_ENTER ();

    module->usecount--;

    DBUG_PRINT ("Module %s has usage count %d.", module->name, module->usecount);

    module = NULL;

    DBUG_RETURN (module);
}

const char *
MODMgetModuleName (module_t *module)
{
    DBUG_ENTER ();

    DBUG_ASSERT (module != NULL, "MODMgetModuleName called with NULL pointer");

    DBUG_RETURN (module->name);
}

module_t *
MODMloadModule (const char *name)
{
    module_t *result;

    DBUG_ENTER ();

    result = LookupModuleInPool (name);

    if (result == NULL)
        result = AddModuleToPool (name);

    DBUG_RETURN (result);
}

module_t *
MODMunLoadModule (module_t *module)
{
    DBUG_ENTER ();

    module = RemoveModuleFromPool (module);

    DBUG_RETURN (module);
}

static symtabfun_p
GetSymbolTableFunction (module_t *module)
{
    char *name;
    symtabfun_u result;

    DBUG_ENTER ();

    name = (char *)MEMmalloc (sizeof (char) * (STRlen (module->name) + 11));
    sprintf (name, "__%s__SYMTAB", module->name);

    result.v = LIBMgetLibraryFunction (name, module->lib);

    if (result.f == NULL)
        CTIabort ("Error loading symbol table for module `%s' (%s): %s", module->name,
                  module->sofile, LIBMgetError ());

    name = MEMfree (name);

    DBUG_RETURN (result.f);
}

const sttable_t *
MODMgetSymbolTable (module_t *module)
{
    symtabfun_p symtabfun;

    DBUG_ENTER ();

    if (module->stable == NULL) {
        symtabfun = GetSymbolTableFunction (module);

        module->stable = symtabfun ();
    }

    DBUG_RETURN (module->stable);
}

static deptabfun_p
GetDependencyTableFunction (module_t *module)
{
    deptabfun_u result;
    char *name;

    DBUG_ENTER ();

    name = (char *)MEMmalloc (sizeof (char) * (STRlen (module->name) + 11));
    sprintf (name, "__%s__DEPTAB", module->name);

    result.v = LIBMgetLibraryFunction (name, module->lib);

    if (result.f == NULL)
        CTIabort ("Error loading dependency table for module `%s' (%s): %s", module->name,
                  module->sofile, LIBMgetError ());

    name = MEMfree (name);

    DBUG_RETURN (result.f);
}

stringset_t *
MODMgetDependencyTable (module_t *module)
{
    deptabfun_p deptabfun;
    stringset_t *result;

    DBUG_ENTER ();

    deptabfun = GetDependencyTableFunction (module);

    result = deptabfun ();

    DBUG_RETURN (result);
}

serfun_p
MODMgetDeSerializeFunction (const char *name, module_t *module)
{
    serfun_u result;

    DBUG_ENTER ();

    result.v = LIBMgetLibraryFunction (name, module->lib);

    DBUG_RETURN (result.f);
}

#undef DBUG_PREFIX
