/*
 *
 * $Log$
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
};

typedef symboltable_t *(*symtabfun_p) ();

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

    result = Malloc (sizeof (module_t));

    result->sofile = Malloc (sizeof (char) * (strlen (name) + 6));
    sprintf (result->sofile, "lib%s.so", name);

    result->name = StringCopy (name);
    result->lib = LoadLibrary (result->sofile);

    DBUG_RETURN (result);
}

module_t *
UnLoadModule (module_t *module)
{
    DBUG_ENTER ("UnLoadModule");

    module->lib = UnLoadLibrary (module->lib);
    module->name = Free (module->name);
    module->sofile = Free (module->sofile);

    module = Free (module);

    DBUG_RETURN (module);
}

symtabfun_p
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

symboltable_t *
GetSymbolTable (module_t *module)
{
    symtabfun_p symtabfun;
    symboltable_t *result;

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
