/*
 *
 * $Log$
 * Revision 1.8  2005/01/14 08:44:39  cg
 * Beautified layout of error messages.
 *
 * Revision 1.7  2005/01/11 12:32:52  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 1.6  2004/11/26 20:01:01  sah
 * *** empty log message ***
 *
 * Revision 1.5  2004/11/08 19:43:37  sah
 * should work using dlcompat for Mac OS X now as well
 *
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
#include "ctinfo.h"

#include <dlfcn.h>

static const char *
LibManagerError ()
{
    const char *error;

    DBUG_ENTER ("LibManagerError");

    error = dlerror ();

    if (error == NULL) {
        error = "unknown error";
    }

    DBUG_RETURN (error);
}

dynlib_t
LIBMloadLibrary (const char *name)
{
    dynlib_t result;

    DBUG_ENTER ("LIBMloadLibrary");

    DBUG_PRINT ("LIB", ("Loading library `%s'", name));

#ifdef RTLD_WORLD
    result = dlopen (name, RTLD_WORLD | RTLD_LAZY);
#else
    result = dlopen (name, RTLD_GLOBAL | RTLD_LAZY);
#endif

    if (result == NULL) {
        CTIabort ("Cannot open library `%s':\n%s", name, LibManagerError ());
    }

    DBUG_PRINT ("LIB", ("Done loading library"));

    DBUG_RETURN (result);
}

dynlib_t
LIBMunLoadLibrary (dynlib_t lib)
{
    int result;

    DBUG_ENTER ("LIBMunLoadLibrary");

    DBUG_PRINT ("LIB", ("Unoading library"));

    result = dlclose (lib);

    if (result != 0) {
        CTIabort ("Cannot close library:\n%s", LibManagerError ());
    }

    DBUG_PRINT ("LIB", ("Done unloading library"));

    DBUG_RETURN ((dynlib_t)NULL);
}

dynfun_t
LIBMgetLibraryFunction (const char *name, dynlib_t lib)
{
    dynfun_t result;

    DBUG_ENTER ("LIBMgetLibraryFunction");

    DBUG_PRINT ("LIB", ("Getting library function `%s'", name));

    result = dlsym (lib, name);

    if (result == NULL) {
        CTIabort ("Cannot open library function `%s':\n%s", name, LibManagerError ());
    }

    DBUG_PRINT ("LIB", ("Done getting library function"));

    DBUG_RETURN (result);
}
