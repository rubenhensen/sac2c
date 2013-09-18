#ifndef _SAC_H_
#define _SAC_H_

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef union {
    void *v;
    int (*f) (int, char **);
} sacmain_u;

#define SAC2CBASEENV "SAC2CBASE"

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

#define REPORTEDERROR                                                                    \
    {                                                                                    \
        char *error = dlerror ();                                                        \
                                                                                         \
        if (error != NULL) {                                                             \
            printf ("The system returned the following error message: "                  \
                    "%s\n",                                                              \
                    error);                                                              \
        }                                                                                \
    }

#define LAUNCHFUNCTIONFROMLIB(library, mainfun, argc, argv, result)                      \
    {                                                                                    \
        char *sac2cbase;                                                                 \
        char *libname;                                                                   \
        void *libsac2c;                                                                  \
        sacmain_u mainptr;                                                               \
                                                                                         \
        sac2cbase = getenv (SAC2CBASEENV);                                               \
                                                                                         \
        libname = (char *)malloc (                                                       \
          sizeof (char)                                                                  \
          * (strlen (library) + ((sac2cbase == NULL) ? 0 : strlen (sac2cbase)) + 1));    \
                                                                                         \
        if (sac2cbase != NULL) {                                                         \
            libname = strcpy (libname, sac2cbase);                                       \
        } else {                                                                         \
            printf ("WARNING: SAC2CBASE is not set.\n");                                 \
            libname[0] = '\0';                                                           \
        }                                                                                \
        libname = strcat (libname, library);                                             \
                                                                                         \
        libsac2c = dlopen (libname, DLOPEN_FLAGS);                                       \
        free (libname);                                                                  \
                                                                                         \
        if (libsac2c == NULL) {                                                          \
            printf ("ERROR: Cannot load shared library '%s'... aborting.\n", libname);   \
            REPORTEDERROR;                                                               \
            exit (10);                                                                   \
        }                                                                                \
                                                                                         \
        mainptr.v = dlsym (libsac2c, mainfun);                                           \
                                                                                         \
        if (mainptr.f == NULL) {                                                         \
            printf ("ERROR: Cannot find symbol '%s' in shared library "                  \
                    "'%s'... aborting.\n",                                               \
                    mainfun, libname);                                                   \
            REPORTEDERROR;                                                               \
            exit (10);                                                                   \
        }                                                                                \
                                                                                         \
        result = mainptr.f (argc, argv);                                                 \
                                                                                         \
        if (dlclose (libsac2c) != 0) {                                                   \
            printf ("ERROR: Cannot unload shared library '%s'... "                       \
                    "aborting.\n",                                                       \
                    libname);                                                            \
            REPORTEDERROR;                                                               \
            exit (10);                                                                   \
        }                                                                                \
    }

static inline void
report_error (void)
{
    char *error = dlerror ();

    if (!error)
        fprintf (stderr,
                 "The system returned the following error "
                 "message: \"%s\"\n",
                 error);
}

static inline int
launch_function_from_library (const char *library, const char *mainfun, int argc,
                              char *argv[])
{
    void *libsac2c = dlopen (library, DLOPEN_FLAGS);
    sacmain_u mainptr;
    int ret;

    if (!libsac2c) {
        fprintf (stderr,
                 "ERROR: cannot load shared library '%s'.  "
                 "Aborting for now.\n",
                 library);
        report_error ();
        exit (10);
    }

    mainptr.v = dlsym (libsac2c, mainfun);

    if (!mainptr.f) {
        fprintf (stderr,
                 "ERROR: cannot find symbol '%s' in shared "
                 "library '%s'.  Aborting for now.\n",
                 mainfun, library);
        report_error ();
        exit (10);
    }

    ret = mainptr.f (argc, argv);
    if (dlclose (libsac2c) != 0) {
        fprintf (stderr,
                 "ERROR: cannot unload shared library '%s'.  "
                 "Aborting for now.\n",
                 library);
        report_error ();
        exit (10);
    }

    return ret;
}

#endif
