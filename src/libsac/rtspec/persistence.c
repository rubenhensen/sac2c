/** <!--********************************************************************-->
 * @file  persistence.c
 *
 * @brief  This file contains the persistence handling for runtime
 * specializations.
 *
 *****************************************************************************/

#include "persistence.h"

#if SAC_DO_RTSPEC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <dlfcn.h>

#include "registry.h"                 // SAC_register_specialization
#include "trace.h"                    // SAC_RTSPEC_TR_Print

#define SAC_DO_TRACE 1

#include "runtime/essentials_h/std.h" // TRUE, ...
#include "libsac/essentials/message.h"    // SAC_RuntimeError,...


#define MAX_INT_DIGITS 21

static int do_trace;

static bool persistence_enabled;

static char *cachedir;
static int strlen_cachedir;
static int strlen_extension;

/** <!--*******************************************************************-->
 *
 * @fn encodeShapes( int *shapes)
 *
 * @brief  Creates a string representation of the shape information stored in
 * the integer array 'shapes'.
 *
 * The format of both the shape array and the string representation is as
 * follows:
 *
 * {
 * number_of_arguments
 *  dimension_arg_1
 *    extent_dim_1
 *    ...
 *    extent_dim_n
 *  dimension_arg_2
 *   ...
 *  dimension_arg_n
 * }
 *
 * @param shapes  The array of shape information.
 ****************************************************************************/
char *
encodeShapes (int *shapes)
{
    int num_args, i, j, k, l, length;

    if (shapes == NULL) {
        fprintf (stderr, "ERROR -- \t Missing shape information!");
        return calloc (1, sizeof (char));
    }

    num_args = shapes[0];

    /*
     * Encode the base type information.
     */
    size_t shape_string_size = MAX_INT_DIGITS + 1;

    i = 1;
    k = 1;
    for (; i <= num_args; i++) {
        if (shapes[k] > 0) {
            j = 0;
            l = shapes[k];
            for (; j <= l; j++) {
                shape_string_size += MAX_INT_DIGITS;
                k++;
            }
        } else {
            shape_string_size += MAX_INT_DIGITS;
            k++;
        }
    }

    char *current = malloc (shape_string_size * sizeof (char));
    if (current == NULL) {
        SAC_RuntimeError ("Allocation failed\n");
    }

    current[0] = '\0';

    length = 0;

    length += sprintf (current, "%d", num_args);

    i = 1;
    k = 1;
    for (; i <= num_args; i++) {
        if (shapes[k] > 0) {
            j = 0;
            l = shapes[k];
            for (; j <= l; j++) {
                length += sprintf (current + length, "-%d", shapes[k]);
                k++;
            }
        } else {
            length += sprintf (current + length, "-%d", shapes[k]);
            k++;
        }
    }

    return current;
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_persistence_init( int argc, char *argv[], int trace)
 *
 * @brief Setup the persistence directory.
 *
 ****************************************************************************/
void
SAC_persistence_init (int argc, char *argv[], int trace)
{
    char *homedir;

    do_trace = trace;

    persistence_enabled = TRUE;

    if (argv) {
        for (int i = 1; i <= argc - 1; i++) {
            if ((argv[i][0] == '-') && (argv[i][1] == 'n') && (argv[i][2] == 'o')
                && (argv[i][3] == '-') && (argv[i][4] == 'p') && (argv[i][5] == 'e')
                && (argv[i][6] == 'r') && (argv[i][7] == 's') && (argv[i][8] == 'i')
                && (argv[i][9] == 's') && (argv[i][10] == 't') && (argv[i][11] == 'e')
                && (argv[i][12] == 'n') && (argv[i][13] == 'c') && (argv[i][14] == 'e')
                && (argv[i][15] == '\0')) {
                persistence_enabled = FALSE;
                break;
            }
        }
    }

    if (!persistence_enabled) {
        SAC_RTSPEC_TR_Print ("Runtime specialization: Persistence disabled.") return;
    }

    if ((homedir = getenv ("HOME")) == NULL) {
        homedir = getpwuid (getuid ())->pw_dir;
    }

    int strlen_homedir = strlen (homedir);
    int strlen_target = strlen (SAC_TARGET_ENV_STRING);
    int strlen_sbi = strlen (SAC_SBI_STRING);

    cachedir = malloc (sizeof (char)
                       * (strlen_homedir + strlen_target + strlen_sbi + 18));

    sprintf (cachedir, "%s/.sac2c/rtspec/" SAC_TARGET_ENV_STRING "/" SAC_SBI_STRING,
             homedir);

    strlen_cachedir = strlen (cachedir);
    strlen_extension = strlen (SAC_MODEXT_STRING);

    if (access (cachedir, F_OK) == 0) {
        return;
    }

    char *basedir;
    char *rtspecdir;
    char *targetdir;

    basedir = malloc (sizeof (char) * (strlen_homedir + 9));
    rtspecdir = malloc (sizeof (char) * (strlen_homedir + 16));
    targetdir = malloc (sizeof (char) * (strlen_homedir + strlen_target + 16));

    sprintf (basedir, "%s/.sac2c/", homedir);
    sprintf (rtspecdir, "%s/.sac2c/rtspec/", homedir);
    sprintf (targetdir, "%s/.sac2c/rtspec/" SAC_TARGET_ENV_STRING, homedir);

    mkdir (basedir, 0755);
    mkdir (rtspecdir, 0755);
    mkdir (targetdir, 0755);

    free (basedir);
    free (rtspecdir);
    free (targetdir);

    if (mkdir (cachedir, 0755) < 0) {
        int err = errno;

        if (err == EEXIST) {
            return;
        }

        persistence_enabled = FALSE;

        fprintf (stderr, "Couldn't create persistence directory. Continuing without "
                         "persistence!\n");
        SAC_RTSPEC_TR_Print ("Runtime specialization: Persistence disabled.")
    } else {
        SAC_RTSPEC_TR_Print (
          "Runtime specialization: Persistence directory created at %s", cachedir)
    }
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_persistence_add( char *filename, char *func_name, char *uuid,
 *                          char *type_info, char *shape, char *mod_name)
 *
 * @brief Add a specialization to the persistence layer
 *
 * @return The filename of the library to load.
 *
 ****************************************************************************/
char *
SAC_persistence_add (char *filename, char *func_name, char *uuid, char *type_info,
                     char *shape, char *mod_name)
{
    if (!persistence_enabled) {
        return filename;
    }

    char *destdir;
    char *destination;

    int strlen_func_name = strlen (func_name);
    int strlen_uuid = strlen (uuid);
    int strlen_type_info = strlen (type_info);
    int strlen_shape = strlen (shape);
    int strlen_mod_name = strlen (mod_name);

    int strlen_destdir = strlen_cachedir + strlen_mod_name + strlen_func_name
                         + strlen_uuid + strlen_type_info + 5;

    destdir = malloc (sizeof (char) * strlen_destdir);
    if (destdir == NULL) {
        SAC_RuntimeError ("Allocation failed");
    }

    sprintf (destdir, "%s/%s/%s/%s/%s", cachedir, mod_name, func_name, uuid, type_info);

    if (access (destdir, F_OK) != 0) {
        char *moddir;
        char *fundir;
        char *uuiddir;

        moddir = malloc (sizeof (char) * (strlen_cachedir + strlen_mod_name + 2));
        fundir = malloc (
          sizeof (char) * (strlen_cachedir + strlen_mod_name + strlen_func_name + 3));
        uuiddir = malloc (
          sizeof (char)
          * (strlen_cachedir + strlen_mod_name + strlen_func_name + strlen_uuid + 4));

        sprintf (moddir, "%s/%s", cachedir, mod_name);
        sprintf (fundir, "%s/%s/%s", cachedir, mod_name, func_name);
        sprintf (uuiddir, "%s/%s/%s/%s", cachedir, mod_name, func_name, uuid);

        if (mkdir (moddir, 0755) < 0) {
            int err = errno;

            if (err != EEXIST) {
                SAC_RTSPEC_TR_Print ("Runtime specialization: Could not store "
                                     "specialization in persistence. Error creating %s!",
                                     moddir);
                free (moddir);

                return filename;
            }
        }

        free (moddir);

        if (mkdir (fundir, 0755) < 0) {
            int err = errno;

            if (err != EEXIST) {
                SAC_RTSPEC_TR_Print ("Runtime specialization: Could not store "
                                     "specialization in persistence. Error creating %s!",
                                     fundir);
                free (fundir);

                return filename;
            }
        }

        free (fundir);

        if (mkdir (uuiddir, 0755) < 0) {
            int err = errno;

            if (err != EEXIST) {
                SAC_RTSPEC_TR_Print ("Runtime specialization: Could not store "
                                     "specialization in persistence. Error creating %s!",
                                     uuiddir);
                free (uuiddir);

                return filename;
            }
        }

        free (uuiddir);

        if (mkdir (destdir, 0755) < 0) {
            int err = errno;

            if (err != EEXIST) {
                SAC_RTSPEC_TR_Print ("Runtime specialization: Could not store "
                                     "specialization in persistence. Error creating %s!",
                                     destdir);
                free (destdir);

                return filename;
            }
        }
    }

    destination = malloc (sizeof (char)
                          * (strlen_destdir + strlen_shape + strlen_extension
                          + 2)); // 1 "/" + null byte at the end

    sprintf (destination, "%s/%s%s", destdir, shape, SAC_MODEXT_STRING);

    // 16 chars in cmd template, 2 from re-counted strlen of destination + null byte at
    // the end
    char *cmd = malloc (
      sizeof (char)
      * (strlen (filename) + strlen_destdir + strlen_shape + strlen_extension + 19));

    sprintf (cmd, "/bin/cp -p \'%s\' \'%s\'", filename, destination);

    switch (system (cmd)) {
    default:
        SAC_RTSPEC_TR_Print (
          "Runtime specialization: Couldn't store specialization in persistence!");
        free (destdir);
        free (cmd);

        return filename;
    case 0:
        SAC_RTSPEC_TR_Print ("Runtime specialization: Specialization stored as %s.",
                             destination);
        free (destdir);
        free (cmd);

        return destination;
    }
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_persistence_load( char *filename, char *symbol_name, char *key)
 *
 * @brief Load specialization
 *
 * @return A function pointer.
 *
 ****************************************************************************/
void *
SAC_persistence_load (char *filename, char *symbol_name, char *key)
{
    SAC_RTSPEC_TR_Print ("Runtime specialization: Linking with library %s.", filename);

    void *dl_handle;
    void *func_ptr;

    /* Dynamically link with the new libary. */
    dl_handle = dlopen (filename, RTLD_NOW | RTLD_LOCAL);

    SAC_RTSPEC_TR_Print ("Runtime specialization: Check handle not being NULL.");

    /* Stop on failure. */
    if (dl_handle == NULL) {
        SAC_RTSPEC_TR_Print (
          "Runtime specialization: Could not load specialized function: %s", dlerror ());

        return NULL;
    }

    SAC_RTSPEC_TR_Print ("Runtime specialization: Check linking error.");

    dlerror ();

    SAC_RTSPEC_TR_Print (
      "Runtime specialization: Load symbol(s) for new specialization.");

    func_ptr = dlsym (dl_handle, symbol_name);

    if (func_ptr != NULL) {
        SAC_register_specialization (key, dl_handle, func_ptr);

        return func_ptr;
    } else {
        SAC_RTSPEC_TR_Print (
          "Runtime specialization: Could not load symbols for specialized function.");

        return NULL;
    }
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_persistence_get( char *key, char *func_name, char *uuid,
 *                          char *type_info, char *shape, char *mod_name)
 *
 * @brief Get a specialization from the persistence layer
 *
 * @return A function pointer
 *
 ****************************************************************************/
void *
SAC_persistence_get (char *key, char *func_name, char *uuid, char *type_info, char *shape,
                     char *mod_name)
{
    if (!persistence_enabled) {
        return NULL;
    }

    int strlen_func_name = strlen (func_name);
    int strlen_uuid = strlen (uuid);
    int strlen_type_info = strlen (type_info);
    int strlen_shape = strlen (shape);
    int strlen_mod_name = strlen (mod_name);

    char *filename
      = malloc (sizeof (char)
                * (strlen_cachedir + strlen_mod_name + strlen_func_name
                   + strlen_uuid + strlen_type_info + strlen_shape
                   + strlen_extension + 6 // 5 "/" + null string at the end
                   ));

    if (filename == NULL) {
        SAC_RuntimeError ("Allocation failed");
    }

    sprintf (filename, "%s/%s/%s/%s/%s/%s%s", cachedir, mod_name, func_name, uuid,
             type_info, shape, SAC_MODEXT_STRING);

    if (access (filename, F_OK) != 0) {
        return NULL;
    }

    return SAC_persistence_load (filename, func_name, key);
}

#endif /* SAC_DO_RTSPEC */
