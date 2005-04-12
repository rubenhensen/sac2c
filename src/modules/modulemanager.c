/*
 *
 * $Log$
 * Revision 1.11  2005/04/12 13:57:00  sah
 * now, MODIMP_PATH is used to find module
 * libraries
 *
 * Revision 1.10  2005/03/10 09:41:09  cg
 * Added #include "internal_lib.h"
 *
 * Revision 1.9  2005/01/11 12:32:52  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 1.8  2004/11/25 21:57:09  sah
 * COMPILES
 *
 *
 * Revision 1.1  2004/09/21 20:37:57  sah
 * Initial revision
 *
 */

#include "modulemanager.h"
#include "libmanager.h"
#include "filemgr.h"
#include "ctinfo.h"
#include "dbug.h"
#include "internal_lib.h"

#include <string.h>

struct MODULE_T {
    char *name;
    char *sofile;
    dynlib_t lib;
    module_t *next;
    int usecount;
};

static module_t *modulepool = NULL;

typedef sttable_t *(*symtabfun_p) ();
typedef stringset_t *(*deptabfun_p) ();

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

    result->sofile = ILIBstringCopy (FMGRfindFile (PK_modimp_path, tmp));

    if (result->sofile == NULL) {
        CTIabort ("Cannot find library `%s'", tmp);
    }

    DBUG_PRINT ("MODM", ("Found library file '%s'.", result->sofile));

    tmp = ILIBfree (tmp);

    result->name = ILIBstringCopy (name);
    result->lib = LIBMloadLibrary (result->sofile);
    result->next = modulepool;
    modulepool = result;
    result->usecount = 1;

    DBUG_RETURN (result);
}

static module_t *
RemoveModuleFromPool (module_t *module)
{
    DBUG_ENTER ("RemoveModuleFromPool");

    module->usecount--;

    if (module->usecount == 0) {

        /* unpool the module */

        if (modulepool == module) {
            modulepool = module->next;
        } else {
            module_t *pos = modulepool;
            while (pos->next != module) {
                pos = pos->next;
            }
            pos->next = module->next;
        }

        /* unload the library */

        module->lib = LIBMunLoadLibrary (module->lib);

        /* free the structure */

        module->sofile = ILIBfree (module->sofile);
        module->name = ILIBfree (module->name);
    }

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

    DBUG_RETURN (result);
}

sttable_t *
MODMgetSymbolTable (module_t *module)
{
    symtabfun_p symtabfun;
    sttable_t *result;

    DBUG_ENTER ("MODMgetSymbolTable");

    symtabfun = GetSymbolTableFunction (module);

    result = symtabfun ();

    DBUG_RETURN (result);
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
