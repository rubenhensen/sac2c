/*
 *
 * $Log$
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

static char *
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
LoadLibrary (char *name)
{
    dynlib_t result;

    DBUG_ENTER ("LoadLibrary");

    result = dlopen (name, RTLD_WORLD | RTLD_GROUP);

    if (result == NULL) {
        SYSERROR (("Cannot open library `%s': %s", name, LibManagerError ()));
    }

    DBUG_RETURN (result);
}

dynfun_t
GetLibraryFunction (char *name, dynlib_t lib)
{
    dynfun_t result;

    DBUG_ENTER ("GetDeserializeFun");

    result = dlsym (lib, name);

    if (result == NULL) {
        SYSERROR (("Cannot open library function `%s': %s", name, LibManagerError ()));
    }

    DBUG_RETURN (result);
}
