/*
 *
 * $Log$
 * Revision 1.5  2004/10/26 09:32:36  sah
 * changed functiontype for serialize functions
 *
 * Revision 1.4  2004/10/25 11:58:47  sah
 * major code cleanup
 *
 * Revision 1.3  2004/10/21 17:20:24  sah
 * modules are now pooled internally
 *
 * Revision 1.2  2004/09/23 21:14:23  sah
 * ongoing implementation
 *
 * Revision 1.1  2004/09/21 20:37:57  sah
 * Initial revision
 *
 *
 *
 */

#include "modulemanager.h"
#include "libmanager.h"
#include "dbug.h"
#include "Error.h"
#include <string.h>

struct MODULE_T {
    char *name;
    char *sofile;
    dynlib_t lib;
    module_t *next;
    int usecount;
};

static module_t *modulepool = NULL;

typedef STtable_t *(*symtabfun_p) ();

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

    DBUG_ENTER ("AddModuleToPool");

    result = Malloc (sizeof (module_t));

    result->sofile = Malloc (sizeof (char) * (strlen (name) + 6));
    sprintf (result->sofile, "lib%s.so", name);

    result->name = StringCopy (name);
    result->lib = LoadLibrary (result->sofile);
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

        module->lib = UnLoadLibrary (module->lib);

        /* free the structure */

        module->sofile = Free (module->sofile);
        module->name = Free (module->name);
    }

    module = NULL;

    DBUG_RETURN (module);
}

const char *
GetModuleName (module_t *module)
{
    DBUG_ENTER ("GetModuleName");

    DBUG_ASSERT ((module != NULL), "GetModuleName called with NULL pointer");

    DBUG_RETURN (module->name);
}

module_t *
LoadModule (const char *name)
{
    module_t *result;

    DBUG_ENTER ("LoadModule");

    result = LookupModuleInPool (name);

    if (result == NULL) {
        result = AddModuleToPool (name);
    }

    DBUG_RETURN (result);
}

module_t *
UnLoadModule (module_t *module)
{
    DBUG_ENTER ("UnLoadModule");

    module = RemoveModuleFromPool (module);

    DBUG_RETURN (module);
}

static symtabfun_p
GetSymbolTableFunction (module_t *module)
{
    char *name;
    symtabfun_p result;

    DBUG_ENTER ("GetSymbolTableFunction");

    name = Malloc (sizeof (char) * (strlen (module->name) + 11));
    sprintf (name, "__%s__SYMTAB", module->name);

    result = (symtabfun_p)GetLibraryFunction (name, module->lib);

    DBUG_RETURN (result);
}

STtable_t *
GetSymbolTable (module_t *module)
{
    symtabfun_p symtabfun;
    STtable_t *result;

    DBUG_ENTER ("GetSymbolTable");

    symtabfun = GetSymbolTableFunction (module);

    result = symtabfun ();

    DBUG_RETURN (result);
}

serfun_p
GetDeSerializeFunction (const char *name, module_t *module)
{
    serfun_p result;

    DBUG_ENTER ("GetDeSerializeFunction");

    result = (serfun_p)GetLibraryFunction (name, module->lib);

    DBUG_RETURN (result);
}
