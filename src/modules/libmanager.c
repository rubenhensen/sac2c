/*
 *
 * $Log$
 * Revision 1.4  2004/11/04 14:56:27  sah
 * removed link.h as it seems not to be needed
 * and creates problems on some platforms.
 *
 * Revision 1.3  2004/09/30 20:02:59  sah
 * added some DBUG_PRINTs
 *
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

    DBUG_PRINT ("LIB", ("Loading library `%s'", name));

    result = dlopen (name, RTLD_WORLD | RTLD_LAZY);

    if (result == NULL) {
        SYSABORT (("Cannot open library `%s': %s", name, LibManagerError ()));
    }

    DBUG_PRINT ("LIB", ("Done loading library"));

    DBUG_RETURN (result);
}

dynlib_t
UnLoadLibrary (dynlib_t lib)
{
    int result;

    DBUG_ENTER ("UnLoadLibrary");

    DBUG_PRINT ("LIB", ("Unoading library"));

    result = dlclose (lib);

    if (result != 0)
        SYSABORT (("Cannot close library: %s", LibManagerError ()));

    DBUG_PRINT ("LIB", ("Done unloading library"));

    DBUG_RETURN ((dynlib_t)NULL);
}

dynfun_t
GetLibraryFunction (const char *name, dynlib_t lib)
{
    dynfun_t result;

    DBUG_ENTER ("GetLibraryFunction");

    DBUG_PRINT ("LIB", ("Getting library function `%s'", name));

    result = dlsym (lib, name);

    if (result == NULL) {
        SYSABORT (("Cannot open library function `%s': %s", name, LibManagerError ()));
    }

    DBUG_PRINT ("LIB", ("Done getting library function"));

    DBUG_RETURN (result);
}
