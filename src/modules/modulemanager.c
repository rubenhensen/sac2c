/*
 *
 * $Log$
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

struct MODULE_T {
    char *name;
    dynlib_t lib;
};

module_t *
LoadModule (char *name)
{
    module_t *result;

    DBUG_ENTER ("LoadModule");

    result = Malloc (sizeof (module_t));

    result->name = StringCopy (name);
    result->lib = LoadLibrary (result->name);

    DBUG_RETURN (result);
}

module_t *
UnLoadModule (module_t *module)
{
    DBUG_ENTER ("UnLoadModule");

    module->lib = UnLoadLibrary (module->lib);
    module->name = Free (module->name);

    module = Free (module);

    DBUG_RETURN (module);
}

serfun_t
GetDeSerializeFunction (char *symbol, module_t *module)
{
    serfun_t result;

    DBUG_ENTER ("GetDeSerializeFunction");

    result = GetLibraryFunction (symbol, module->lib);

    DBUG_RETURN (result);
}
