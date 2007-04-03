/*
 *
 * $Id$
 *
 */

#include "sactools.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SAC2CBASEENV "SAC2CBASE"
#define MAINFUN "SACrunSac2c"

#ifdef DBUG_OFF
#define LIBSAC2C "/lib/libsac2c.p.so"
#else
#define LIBSAC2C "/lib/libsac2c.d.so"
#endif

typedef int (*sacmain_p) (int, char **);

int
main (int argc, char *argv[])
{
    char *sac2cbase;
    char *libname;
    void *libsac2c;
    sacmain_p mainfun;
    int result;

    sac2cbase = getenv (SAC2CBASEENV);

    libname = malloc (
      sizeof (char)
      * (strlen (LIBSAC2C) + ((sac2cbase == NULL) ? 0 : strlen (sac2cbase)) + 1));

    if (sac2cbase != NULL) {
        strcpy (libname, sac2cbase);
    } else {
        libname = "\0";
    }
    strcat (libname, LIBSAC2C);

#ifdef RTLD_WORLD
    libsac2c = dlopen (libname, RTLD_WORLD | RTLD_LAZY);
#else
    libsac2c = dlopen (libname, RTLD_GLOBAL | RTLD_LAZY);
#endif

    if (libsac2c == NULL) {
        printf ("Cannot load shared library '%s'... aborting.\n", libname);
        exit (10);
    }

    mainfun = dlsym (libsac2c, MAINFUN);

    if (libsac2c == NULL) {
        printf ("Cannot find symbol '%s' in shared library '%s'... aborting.\n", MAINFUN,
                libname);
        exit (10);
    }

    result = mainfun (argc, argv);

    if (dlclose (libsac2c) != 0) {
        printf ("Cannot unload shared library '%s'... aborting.\n", libname);
        exit (10);
    }

    return (result);
}
