/** <!--********************************************************************-->
 * @file  persistence.c
 *
 * @brief  This file contains the persistence handling for runtime
 * specializations.
 *
 *****************************************************************************/

#include "config.h"

#if SAC_DO_RTSPEC

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>

#include "trace.h"

#define SAC_DO_TRACE 1
#include "sac.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#define MAX_INT_DIGITS 21

static int do_trace;

static bool persistence_enabled;

char *cachedir;

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
        return '\0';
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

    char *current = (char *)malloc (shape_string_size * sizeof (char));

    current[0] = '\0';

    length = 0;

    length += sprintf (current, "%d-", num_args);

    i = 1;
    k = 1;
    for (; i <= num_args; i++) {
        if (shapes[k] > 0) {
            j = 0;
            l = shapes[k];
            for (; j <= l; j++) {
                length += sprintf (current + length, "%d-", shapes[k]);
                k++;
            }
        } else {
            length += sprintf (current + length, "%d-", shapes[k]);
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

    cachedir = (char *)malloc (sizeof (char) * (strlen (homedir) + 16));

    sprintf (cachedir, "%s/.sac2c/rtspec/", homedir);

    if (access (cachedir, F_OK) == 0) {
        return;
    }

    char *basedir;

    basedir = (char *)malloc (sizeof (char) * (strlen (homedir) + 9));

    sprintf (basedir, "%s/.sac2c/", homedir);

    mkdir (basedir, 0755);

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

    free (basedir);
}

#endif /* SAC_DO_RTSPEC */
