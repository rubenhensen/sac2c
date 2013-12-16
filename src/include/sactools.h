#ifndef _SAC_H_
#define _SAC_H_

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sacdirs.h"

typedef union {
    void *v;
    int (*f) (int, char **);
} sacmain_u;

#ifdef DBUG_OFF
#define DLOPEN_FLAGS RTLD_GLOBAL | RTLD_LAZY
#else /* DBUG_OFF */
#define DLOPEN_FLAGS RTLD_GLOBAL | RTLD_NOW
#endif /* DBUG_OFF */

/*
 * we use a macro here instead of an external function definitions
 * as we do not want to have any exported symbols apart from main
 * in our launchers. The other alternative would be to reproduce
 * this code in each and every launcher, which would be more
 * difficult to maintain.
 */

static inline void
report_error (void)
{
    char *error = dlerror ();

    if (error)
        fprintf (stderr, "dlopen: %s\n", error);
}

static inline int
launch_function_from_library (const char *library, const char *mainfun, int argc,
                              char *argv[])
{
    void *libsac2c = dlopen (library, DLOPEN_FLAGS);
    sacmain_u mainptr;
    int ret;

    if (!libsac2c) {
        char *tmp = malloc (strlen (DLL_DIR) + strlen (library) + 2);
        strcpy (tmp, DLL_DIR);
        strcat (tmp, "/");
        strcat (tmp, library);
        libsac2c = dlopen (tmp, DLOPEN_FLAGS);
        free (tmp);
        if (!libsac2c) {
            fprintf (stderr, "ERROR: library '%s' not found, also not as '%s'.\n",
                     library, tmp);
            report_error ();
            exit (10);
        }
    }

    mainptr.v = dlsym (libsac2c, mainfun);

    if (!mainptr.f) {
        fprintf (stderr, "ERROR: symbol '%s' not found in library '%s'.\n", mainfun,
                 library);
        report_error ();
        exit (10);
    }

    ret = mainptr.f (argc, argv);

    if (dlclose (libsac2c) != 0) {
        fprintf (stderr, "ERROR: cannot unload library '%s'.\n", library);
        report_error ();
        exit (10);
    }

    return ret;
}

#endif
