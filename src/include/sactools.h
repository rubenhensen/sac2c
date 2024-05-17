#ifndef _SAC_H_
#define _SAC_H_

#include <dlfcn.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include "sacdirs.h"

typedef union {
    void *v;
    int (*f) (int, char **);
} sacmain_u;

typedef union {
    void *v;
    char *(*f) (void);
} version_u;

typedef union {
    void *v;
    void (*f) (char *, char *);
} sac2crc_u;

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

static inline void *
get_pointer_to_symbol (void *library, const char *libname, const char *symbolname)
{
    void *ptr;
    ptr = dlsym (library, symbolname);

    if (!ptr) {
        fprintf (stderr, "ERROR: symbol '%s' not found in library '%s'.\n", symbolname,
                 libname);
        report_error ();
        exit (10);
    }

    return ptr;
}

/**
 * Try to load the local build library, if this does not
 * work we exit.
 */
static inline void *
load_local_library (const char *library)
{
    void *libptr;
    char *tmp = malloc (strlen (DLL_BUILD_DIR) + strlen (library) + 2);

    strcpy (tmp, DLL_BUILD_DIR);
    strcat (tmp, "/");
    strcat (tmp, library);

    libptr = dlopen (tmp, DLOPEN_FLAGS);

    free (tmp);

    if (!libptr) {
        fprintf (stderr, "ERROR: library '%s' not found in '%s'.\n",
                         library, DLL_BUILD_DIR);
        report_error ();
        exit (10);
    }

    return libptr;
}

/**
 * Try to load the global library, if this does not work,
 * we fallback on loading the local build library.
 */
static inline void *
load_global_library (const char *library)
{
    void *libptr;

    // we try to load via ldconfig/LD_LIBRARY_PATH
    libptr = dlopen (library, DLOPEN_FLAGS);
    if (!libptr) {
        // if this fails, we load via an absolute path
        char *tmp = malloc (strlen (DLL_DIR) + strlen (library) + 2);
        strcpy (tmp, DLL_DIR);
        strcat (tmp, "/");
        strcat (tmp, library);
        libptr = dlopen (tmp, DLOPEN_FLAGS);
        if (!libptr) {
            fprintf (stderr, "ERROR: unable to load library '%s' from "
                             "'%s', trying a different path...\n", library, DLL_DIR);
            report_error ();
            // finally we give up on global scope, and look in the build dir
            libptr = load_local_library (library);
        }
        free (tmp);
    }

    return libptr;
}

static inline int
launch_function_from_library (const char *library, const char *sac2crc,
                              const char *mainfun, int argc, char *argv[])
{
    void *libsac2c; // library pointer
    const char *version = SAC2C_VERSION;
    const int is_dirty = SAC2C_IS_DIRTY;
    sacmain_u mainptr;
    version_u libversion;
    sac2crc_u set_sac2crc_location;
    sac2crc_u set_sac2clib_location;
    char global_sac2crc_path[PATH_MAX];
    char build_sac2crc_path[PATH_MAX];
    char global_sac2clib_path[PATH_MAX];
    char build_sac2clib_path[PATH_MAX];
    int ret;

    /**
     * The idea here is that when `dirty', we only
     * try to load the local build version.
     */
    if (is_dirty) {
        libsac2c = load_local_library (library);
    } else {
        libsac2c = load_global_library (library);
    }

    libversion.v = get_pointer_to_symbol (libsac2c, library, "getLibsac2cVersion");
    if (strcmp (version, libversion.f ())) {
        fprintf (stderr,
                 "ERROR: version mismatch between this binary and the library.\n"
                 "   binary: %s\n"
                 "  library: %s\n",
                 version, libversion.f ());
        exit (20);
    }

    // call     setLibsac2cInfo    from libsac2c/global/main.c
    set_sac2clib_location.v
      = get_pointer_to_symbol (libsac2c, library, "setLibsac2cInfo");
    snprintf (global_sac2clib_path, sizeof (global_sac2clib_path), "%s", DLL_DIR);
    snprintf (build_sac2clib_path, sizeof (build_sac2clib_path), "%s", DLL_BUILD_DIR);
    set_sac2clib_location.f (global_sac2clib_path, build_sac2clib_path);

    // call     setLibsac2cInfo    from libsac2c/global/resource.c
    set_sac2crc_location.v
      = get_pointer_to_symbol (libsac2c, library, "RSCsetSac2crcLocations");
    snprintf (global_sac2crc_path, sizeof (global_sac2crc_path), "%s/%s", SAC2CRC_DIR,
              sac2crc);
    snprintf (build_sac2crc_path, sizeof (build_sac2crc_path), "%s/%s", SAC2CRC_BUILD_DIR,
              sac2crc);
    set_sac2crc_location.f (global_sac2crc_path, build_sac2crc_path);

    // call     SACrunSac2c/SACrun...    from libsac2c/global/main.c
    mainptr.v = get_pointer_to_symbol (libsac2c, library, mainfun);
    ret = mainptr.f (argc, argv);

    if (dlclose (libsac2c) != 0) {
        fprintf (stderr, "ERROR: cannot unload library '%s'.\n", library);
        report_error ();
        exit (30);
    }

    return ret;
}

#endif
// vim: ts=2 sw=2 et:
