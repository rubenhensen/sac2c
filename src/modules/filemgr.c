/*
 *
 * $Log$
 * Revision 1.14  1998/03/05 16:41:08  srs
 * compile error. AppendPath() made none-static
 *
 * Revision 1.13  1998/03/04 16:23:27  cg
 *  C compiler invocations and file handling converted to new
 * to usage of new  configuration files.
 *
 * Revision 1.12  1997/04/24 09:55:55  cg
 * standard library search paths relative to environment variable SACBASE
 * added in addition to user-defined paths
 *
 * Revision 1.11  1997/03/19  13:48:03  cg
 * new function AbsolutePathname() added.
 * Converted  to single tmp directory tmp_dirname instaed of build_dirname
 * and store_dirname
 *
 * Revision 1.10  1997/03/11  16:25:33  cg
 * Improved function AbsolutePathname: Now, minimal absolute pathnames
 * are constructed from relative path names, removing leading ../../
 *
 * Revision 1.9  1996/09/11  06:22:51  cg
 * Modified construction of paths.
 * Now: 1. paths added by command line option, 2. cwd, 3.shell variable
 *
 * Revision 1.8  1996/01/22  17:31:38  cg
 * modified InitPaths
 *
 * Revision 1.7  1996/01/07  16:58:23  cg
 * handling of temporary directories modified.
 * cccall.c now only needs 2 of these
 *
 * Revision 1.6  1996/01/05  12:36:29  cg
 * added global variables tmp_dirname, store_dirname, build_dirname
 * and functions WriteOpen, CreateTmpDirectories, RemoveDirectory
 *
 * Revision 1.5  1995/04/10  11:08:02  sbs
 * some DBUG_PRINTs inserted
 * FindFile only looks for the explicit name if preceeded by "/"
 * i.e. an absolute path is given
 *
 * Revision 1.4  1995/04/07  09:36:35  sbs
 * bug resulting from change FILE * => char * eliminated
 *
 * Revision 1.3  1995/04/05  17:24:16  sbs
 * GenLinkList inserted
 *
 * Revision 1.2  1995/02/22  14:14:36  hw
 * changed FindFile (now look first for name and later have a look at path )
 *
 * Revision 1.1  1994/12/11  17:33:27  sbs
 * Initial revision
 *
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "dbug.h"
#include "internal_lib.h"
#include "Error.h"

#include "filemgr.h"
#include "resource.h"

static char path_bufs[3][MAX_PATH_LEN];
static int bufsize[3];

/*
 *
 *  functionname  : FindFile
 *  arguments     :
 *  description   :
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

char *
FindFile (pathkind p, char *name)
{
    FILE *file = NULL;
    static char buffer[MAX_FILE_NAME];
    static char buffer2[MAX_PATH_LEN];
    char *path;
    char *result = NULL;

    DBUG_ENTER ("FindFile");

    if (name[0] == '/') { /* absolute path specified! */
        strcpy (buffer, name);
        file = fopen (buffer, "r");
    } else {
        strcpy (buffer2, path_bufs[p]);
        path = strtok (buffer2, ":");
        while ((file == NULL) && (path != NULL)) {
            strcpy (buffer, path);
            strcat (buffer, "/");
            strcat (buffer, name);
            DBUG_PRINT ("FILE", ("trying file %s\n", buffer));
            file = fopen (buffer, "r");
            if (file == NULL) {
                path = strtok (NULL, ":");
            }
        }
    }
    if (file) {
        fclose (file);
        result = buffer;
    }

    DBUG_RETURN (result);
}

/*
 *
 *  functionname  : InitPaths
 *  arguments     :
 *  description   :
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strcpy
 *  macros        : ---
 *
 *  remarks       :
 *
 */

void
InitPaths ()
{
    int i;

    DBUG_ENTER ("InitPaths");

    for (i = 0; i < 3; i++) {
        bufsize[i] = 1;
        strcpy (path_bufs[i], ".");
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : AppendPath
 *  arguments     : pathkind, path to be appended
 *  description   :
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

void
AppendPath (pathkind p, char *path)
{
    int len;

    DBUG_ENTER ("AppendPath");

    len = (strlen (path) + 1);
    if (len + bufsize[p] >= MAX_PATH_LEN) {
        SYSABORT (("MAX_PATH_LEN too low"));
    } else {
        strcat (path_bufs[p], ":");
        strcat (path_bufs[p], path);
        DBUG_PRINT ("FILE", ("appending \":%s\" to path %d", path, p));
        bufsize[p] += len;
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : AppendEnvVar
 *  arguments     : pathkind, path to be appended
 *  description   :
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

static void
AppendEnvVar (pathkind p, char *var)
{
    int len;
    char *buffer;

    DBUG_ENTER ("AppendEnvVar");

    buffer = getenv (var);
    if (buffer != NULL) {
        len = (strlen (buffer) + 1);
        if (len + bufsize[p] >= MAX_PATH_LEN) {
            SYSABORT (("MAX_PATH_LEN too low"));
        } else {
            strcat (path_bufs[p], ":");
            strcat (path_bufs[p], buffer);
            DBUG_PRINT ("FILE", ("appending \":%s\" to path %d", buffer, p));
            bufsize[p] += len;
        }
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void AppendStdlibPaths(int pathkind, char *path
 *
 * description:
 *   This function adds the paths for module/class declarations and
 *   libraries of the standard library read from the configuration files
 *   to the internal paths.
 *
 *
 ******************************************************************************/

static void
AppendStdlibPaths (int pathkind, char *path)
{
    char *pathentry;
    char buffer[MAX_PATH_LEN];
    char *envvar_end;
    char *envvar;
    int envvar_length;

    DBUG_ENTER ("AppendStdlibPaths");

    pathentry = strtok (path, ":");

    while (pathentry != NULL) {
        if (pathentry[0] == '$') {
            envvar_end = strchr (pathentry, '/');
            if (envvar_end == NULL) {
                envvar = getenv (pathentry);
                if (envvar != NULL) {
                    AppendPath (pathkind, envvar);
                }
            } else {
                envvar_length = strlen (pathentry + 1) - strlen (envvar_end);
                strncpy (buffer, pathentry + 1, envvar_length);
                buffer[envvar_length] = '\0';
                envvar = getenv (buffer);
                if (envvar != NULL) {
                    strcpy (buffer, envvar);
                    strcat (buffer, envvar_end);
                } else {
                    strcpy (buffer, envvar_end);
                }
                AppendPath (pathkind, buffer);
            }
        } else {
            AppendPath (pathkind, pathentry);
        }

        if (pathentry != path) {
            *(pathentry - 1) = ':';
        }

        pathentry = strtok (NULL, ":");
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : RearrangePaths
 *  arguments     : ---
 *  description   : If additional search paths are provided as command line
 *                  parameters, then these have a higher priority than the
 *                  standard search path (current directory+shell variables).
 *                  This function modifies the internal path representation
 *                  if necessary after scanning the command line.
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
RearrangePaths ()
{
    int i;
    char buffer[MAX_PATH_LEN];

    DBUG_ENTER ("RearrangePaths");

    for (i = 0; i <= 2; i++) {
        if (strlen (path_bufs[i]) > 1) {
            strcpy (buffer, path_bufs[i] + 2);
            strcat (buffer, ":.");
            strcpy (path_bufs[i], buffer);
        }
    }

    AppendEnvVar (MODDEC_PATH, "SAC_DEC_PATH");
    AppendEnvVar (MODIMP_PATH, "SAC_LIBRARY_PATH");
    AppendEnvVar (PATH, "SAC_PATH");

    AppendStdlibPaths (MODDEC_PATH, config.stdlib_decpath);
    AppendStdlibPaths (MODIMP_PATH, config.stdlib_libpath);

#if 0
    for (i=0; i<NUMDECPATHS; i++)
    {
      strcpy(buffer, sac_home);
      strcat(buffer, "/");
      strcat(buffer, stddecpaths[i]);
      AppendPath(MODDEC_PATH, buffer);
    }
    
    for (i=0; i<NUMLIBPATHS; i++)
    {
      strcpy(buffer, sac_home);
      strcat(buffer, "/");
      strcat(buffer, stdlibpaths[i]);
      AppendPath(MODIMP_PATH, buffer);
    }
#endif

    DBUG_PRINT ("PATH", ("PATH is %s", path_bufs[PATH]));
    DBUG_PRINT ("PATH", ("MODDEC_PATH is %s", path_bufs[MODDEC_PATH]));
    DBUG_PRINT ("PATH", ("MODIMP_PATH is %s", path_bufs[MODIMP_PATH]));

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : AbsolutePathname
 *  arguments     : 1) path name
 *  description   : turns relative path names into absolute path names
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strcpy, strncmp, getcwd, strrchr, strcat
 *  macros        : MAX_PATH_LEN
 *
 *  remarks       :
 *
 */

char *
AbsolutePathname (char *path)
{
    char *tmp;
    static char buffer[MAX_PATH_LEN];

    DBUG_ENTER ("AbsolutePathname");

    if (path[0] == '/') {
        strcpy (buffer, path);
    } else {
        getcwd (buffer, MAX_PATH_LEN);

        while (0 == strncmp ("../", path, 3)) {
            path += 3;
            tmp = strrchr (buffer, '/');
            *tmp = 0;
        }

        if (0 == strncmp ("./", path, 2)) {
            path += 2;
        }

        strcat (buffer, "/");
        strcat (buffer, path);
    }

    DBUG_RETURN (buffer);
}

/*
 *
 *  functionname  : WriteOpen
 *  arguments     : 1) format string like that of printf
 *                  2) variable argument list for 1)
 *  description   : opens the given file for writing. If this fails,
 *                  an error message
 *                  occurs and compilation is aborted.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : vsprintf, va_start, va_end, fopen
 *  macros        : vararg macros, ERROR
 *
 *  remarks       :
 *
 */

FILE *
WriteOpen (char *format, ...)
{
    va_list arg_p;
    static char buffer[MAX_PATH_LEN];
    FILE *file;

    DBUG_ENTER ("WriteOpen");

    va_start (arg_p, format);
    vsprintf (buffer, format, arg_p);
    va_end (arg_p);

    file = fopen (buffer, "w");

    if (file == NULL) {
        SYSABORT (("Unable to write file \"%s\"", buffer));
    }

    DBUG_RETURN (file);
}
