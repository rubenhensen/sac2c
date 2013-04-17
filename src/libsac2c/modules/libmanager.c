#include "libmanager.h"

#define DBUG_PREFIX "LIB"
#include "debug.h"

#if IS_CYGWIN
#include "cygwinhelpers.h"
#endif /* is_CYGWIN */

#include "ctinfo.h"

#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>

#define LIBM_ERROR_MAXLEN 256

static char LIBMerror[LIBM_ERROR_MAXLEN] = "";

static const char *
LibManagerError (void)
{
    const char *error;

    DBUG_ENTER ();

    error = dlerror ();

    if (error == NULL) {
        error = "unknown error";
    }

    DBUG_RETURN (error);
}

static void
setError (const char *format, ...)
{
    va_list args;

    DBUG_ENTER ();

    va_start (args, format);

    vsnprintf (LIBMerror, LIBM_ERROR_MAXLEN, format, args);

    va_end (args);

    DBUG_RETURN ();
}

const char *
LIBMgetError (void)
{
    DBUG_ENTER ();

    DBUG_RETURN (LIBMerror);
}

dynlib_t
LIBMloadLibrary (const char *name)
{
    dynlib_t result;

    DBUG_ENTER ();

#if IS_CYGWIN
    name = CYGHcygToWinPath (name);
#endif

    DBUG_PRINT ("Loading library `%s'", name);

#ifdef RTLD_WORLD
    result = dlopen (name, RTLD_WORLD | RTLD_LAZY);
#else
    result = dlopen (name, RTLD_GLOBAL | RTLD_LAZY);
#endif

    if (result == NULL) {
        setError ("Cannot open library `%s':\n%s", name, LibManagerError ());
    }

    DBUG_PRINT ("Done loading library");

    DBUG_RETURN (result);
}

dynlib_t
LIBMunLoadLibrary (dynlib_t lib)
{
    int result;

    DBUG_ENTER ();

    DBUG_PRINT ("Unoading library");

    result = dlclose (lib);

    if (result != 0) {
        setError ("Cannot close library:\n%s", LibManagerError ());
    }

    DBUG_PRINT ("Done unloading library");

    DBUG_RETURN ((dynlib_t)NULL);
}

dynfun_t
LIBMgetLibraryFunction (const char *name, dynlib_t lib)
{
    dynfun_t result;

    DBUG_ENTER ();

    DBUG_PRINT ("Getting library function `%s'", name);

    result = dlsym (lib, name);

#ifndef DBUG_OFF
    if (result != NULL) {
        DBUG_PRINT ("Done getting library function");
    } else {
        DBUG_PRINT ("Failed getting library function: ", LibManagerError ());
    }
#endif

    DBUG_RETURN (result);
}

#undef DBUG_PREFIX
