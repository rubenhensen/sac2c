/*
 *
 * $Log$
 * Revision 1.2  2004/09/23 21:15:39  sah
 * interface complete and
 * working implementation for Solaris
 *
 * Revision 1.1  2004/09/21 17:54:00  sah
 * Initial revision
 *
 *
 *
 */

#include "libmanager.h"
#include "dbug.h"
#include "Error.h"
#include <link.h>
#include <dlfcn.h>

static const char *
LibManagerError ()
{
    char *error;

    DBUG_ENTER ("LibManagerError");

    error = dlerror ();

    if (error == NULL)
        error = "unknown error";

    DBUG_RETURN (error);
}

dynlib_t
LoadLibrary (const char *name)
{
    dynlib_t result;

    DBUG_ENTER ("LoadLibrary");

    result = dlopen (name, RTLD_WORLD | RTLD_LAZY);

    if (result == NULL) {
        SYSABORT (("Cannot open library `%s': %s", name, LibManagerError ()));
    }

    DBUG_RETURN (result);
}

dynlib_t
UnLoadLibrary (dynlib_t lib)
{
    int result;

    DBUG_ENTER ("UnLoadLibrary");

    result = dlclose (lib);

    if (result != 0)
        SYSABORT (("Cannot close library: %s", LibManagerError ()));

    DBUG_RETURN ((dynlib_t)NULL);
}

dynfun_t
GetLibraryFunction (const char *name, dynlib_t lib)
{
    dynfun_t result;

    DBUG_ENTER ("GetDeserializeFun");

    result = dlsym (lib, name);

    if (result == NULL) {
        SYSABORT (("Cannot open library function `%s': %s", name, LibManagerError ()));
    }

    DBUG_RETURN (result);
}
