/*
 *
 * $Log$
 * Revision 1.7  2005/01/12 15:51:54  cg
 * Added FMGRcreateTmpDir()
 *
 * Revision 1.6  2005/01/11 11:28:11  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 1.5  2005/01/07 16:49:00  cg
 * Added function FMGRcleanUp.
 *
 * Revision 1.4  2004/11/29 17:29:49  sah
 * global.targetdir is now always set
 *
 * Revision 1.3  2004/11/25 17:53:48  cg
 * SacDevCamp 04
 *
 * Revision 1.2  2004/11/22 15:49:57  cg
 * Moved from subdirectory modules to global.
 *
 * Revision 1.1  2004/11/22 15:45:35  cg
 * Initial revision
 *
 * Moved from subdirectory modules
 *
 * Revision 3.6  2004/10/11 16:55:48  sah
 * removes TempFileName again
 *
 * Revision 3.5  2004/09/21 16:32:42  sah
 * Added TempFileName
 *
 * Revision 3.4  2003/03/25 17:18:20  sah
 * CheckSystemLibrary now uses config.cc to
 * get the current c compiler.
 *
 * Revision 3.3  2003/03/25 14:40:41  sah
 * added CheckSystemLibrary
 *
 * Revision 3.2  2001/05/17 13:08:53  nmw
 * MALLOC/FREE replaced by Malloc/Free, using result of Free()
 *
 * Revision 3.1  2000/11/20 18:00:51  sacbase
 * new release made
 *
 * Revision 2.3  2000/11/17 16:14:53  sbs
 * locationtype FindLocationOfFile( char *file)
 * added;
 *
 * Revision 2.2  1999/05/18 11:21:46  cg
 * added function CheckExistFile().
 *
 * Revision 2.1  1999/02/23 12:42:04  sacbase
 * new release made
 *
 * Revision 1.15  1998/03/17 12:14:24  cg
 * added resource SYSTEM_LIBPATH.
 * This makes the gcc special feature '--print-file-name' obsolete.
 * A fourth search path is used instead for system libraries.
 * This additional path may only be set via the sac2crc file,
 * but not by environment variables or command line parameters.
 *
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
#include "ctinfo.h"
#include "free.h"
#include "types.h"
#include "filemgr.h"
#include "resource.h"
#include "tree_basic.h"

static char path_bufs[4][MAX_PATH_LEN];
static int bufsize[4];

/*
 *
 *  functionname  : FMGRfindFile
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
FMGRfindFile (pathkind_t p, char *name)
{
    FILE *file = NULL;
    static char buffer[MAX_FILE_NAME];
    static char buffer2[MAX_PATH_LEN];
    char *path;
    char *result = NULL;

    DBUG_ENTER ("FMGRfindFile");

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
            DBUG_PRINT ("FMGR", ("trying file %s\n", buffer));
            file = fopen (buffer, "r");
            if (file == NULL) {
                path = strtok (NULL, ":");
            }
        }
    }
    if (file != NULL) {
        fclose (file);
        result = buffer;
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   bool FMGRcheckSystemLibrary( char *name)
 *
 * description:
 *
 *   This function checks whether a given system library is found
 *   by the systems linker (eg ld). It does so by just calling ld
 *   with the given library and a dummy C program.
 *
 ******************************************************************************/

bool
FMGRcheckSystemLibrary (char *name)
{
    int result;

    DBUG_ENTER ("FMGRcheckSystemLibrary");

    /* remove trailing 'lib' */
    name += 3;

    /* create a dummy C program to compile and link against */
    /* the library.                                         */

    ILIBsystemCall ("echo \"int main(){return(0);}\" >%s/SAC_XX_syslibtest.c",
                    global.tmp_dirname);

    result
      = ILIBsystemCall2 ("%s %s %s -l%s -o %s/SAC_XX_syslibtest %s/SAC_XX_syslibtest.c",
                         global.config.cc, global.config.ccflags, global.config.ldflags,
                         name, global.tmp_dirname, global.tmp_dirname);

    /* reverse result, because a result of 0 means true here. */

    DBUG_RETURN (result != 0);
}

/******************************************************************************
 *
 * function:
 *   bool FMGRcheckExistFile(char *dir, char *name)
 *
 * description:
 *
 *   This function checks whether a given file exists in a given directory.
 *   If the given directory is NULL then the current directory is taken.
 *   More precisely, it is checked whether or not the file may be opened
 *   for reading which in most cases is what we want to know.
 *
 ******************************************************************************/

bool
FMGRcheckExistFile (char *dir, char *name)
{
    char *tmp;
    FILE *file;
    bool res;

    DBUG_ENTER ("FMGRcheckExistFile");

    DBUG_ASSERT ((name != NULL), "Function FMGRcheckExistFile() called with name NULL");

    if (dir == NULL) {
        dir = "";
    }

    tmp = ILIBstringConcat3 (dir, "/", name);

    file = fopen (tmp, "r");
    tmp = ILIBfree (tmp);

    if (file == NULL) {
        res = FALSE;
    } else {
        res = TRUE;
        fclose (file);
    }

    DBUG_RETURN (res);
}

/*
 *
 *  functionname  : FMGRinitPaths
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
FMGRinitPaths ()
{
    int i;

    DBUG_ENTER ("FMGRinitPaths");

    for (i = 0; i < 4; i++) {
        bufsize[i] = 1;
        strcpy (path_bufs[i], ".");
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : FMGRappendPath
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
FMGRappendPath (pathkind_t p, char *path)
{
    int len;

    DBUG_ENTER ("FMGRappendPath");

    len = strlen (path) + 1;
    if (len + bufsize[p] >= MAX_PATH_LEN) {
        CTIabort ("MAX_PATH_LEN too low");
    } else {
        strcat (path_bufs[p], ":");
        strcat (path_bufs[p], path);
        DBUG_PRINT ("FMGR", ("appending \":%s\" to path %d", path, p));
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
AppendEnvVar (pathkind_t p, char *var)
{
    int len;
    char *buffer;

    DBUG_ENTER ("AppendEnvVar");

    buffer = getenv (var);
    if (buffer != NULL) {
        len = (strlen (buffer) + 1);
        if (len + bufsize[p] >= MAX_PATH_LEN) {
            CTIabort ("MAX_PATH_LEN too low");
        } else {
            strcat (path_bufs[p], ":");
            strcat (path_bufs[p], buffer);
            DBUG_PRINT ("FMGR", ("appending \":%s\" to path %d", buffer, p));
            bufsize[p] += len;
        }
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void AppendConfigPaths(int pathkind, char *path
 *
 * description:
 *   This function adds the paths for module/class declarations and
 *   libraries of the standard library read from the configuration files
 *   to the internal paths.
 *
 *
 ******************************************************************************/

static void
AppendConfigPaths (pathkind_t pathkind, char *path)
{
    char *pathentry;
    char buffer[MAX_PATH_LEN];
    char *envvar_end;
    char *envvar;
    int envvar_length;

    DBUG_ENTER ("AppendConfigPaths");

    pathentry = strtok (path, ":");

    while (pathentry != NULL) {
        if (pathentry[0] == '$') {
            envvar_end = strchr (pathentry, '/');
            if (envvar_end == NULL) {
                envvar = getenv (pathentry);
                if (envvar != NULL) {
                    FMGRappendPath (pathkind, envvar);
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
                FMGRappendPath (pathkind, buffer);
            }
        } else {
            FMGRappendPath (pathkind, pathentry);
        }

        if (pathentry != path) {
            *(pathentry - 1) = ':';
        }

        pathentry = strtok (NULL, ":");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void FMGRrearrangePaths()
 *
 * description:
 *
 * Now, we set our search paths for the source program, module declarations,
 * and module implementations...
 *
 * The original search path is ".".
 * Then, additional paths specified by the respective compiler options are
 * appended after having been transformed into absolute paths.
 * If this has happened, the current directory is moved to the end of the
 * path list because those paths specified on the command line are intended
 * to have a higher priority.
 * At last, the paths specified by environment variables are appended.
 * These have a lower priority.
 * At very last, the required paths for using the SAC standard library
 * relative to the shell variable SAC_HOME are added. These have the
 * lowest priority.
 *
 *
 ******************************************************************************/

void
FMGRrearrangePaths ()
{
    int i;
    char buffer[MAX_PATH_LEN];

    DBUG_ENTER ("FMGRrearrangePaths");

    for (i = 0; i <= 2; i++) {
        if (strlen (path_bufs[i]) > 1) {
            strcpy (buffer, path_bufs[i] + 2);
            strcat (buffer, ":.");
            strcpy (path_bufs[i], buffer);
        }
    }

    AppendEnvVar (PK_moddec_path, "SAC_DEC_PATH");
    AppendEnvVar (PK_modimp_path, "SAC_LIBRARY_PATH");
    AppendEnvVar (PK_path, "SAC_PATH");

    AppendConfigPaths (PK_moddec_path, global.config.stdlib_decpath);
    AppendConfigPaths (PK_modimp_path, global.config.stdlib_libpath);
    AppendConfigPaths (PK_systemlib_path, global.config.system_libpath);

    DBUG_PRINT ("FMGR", ("PATH is %s", path_bufs[PK_path]));
    DBUG_PRINT ("FMGR", ("MODDEC_PATH is %s", path_bufs[PK_moddec_path]));
    DBUG_PRINT ("FMGR", ("MODIMP_PATH is %s", path_bufs[PK_modimp_path]));
    DBUG_PRINT ("FMGR", ("SYSTEMLIB_PATH is %s", path_bufs[PK_systemlib_path]));

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : FMGRabsolutePathname
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
FMGRabsolutePathname (char *path)
{
    char *tmp;
    static char buffer[MAX_PATH_LEN];

    DBUG_ENTER ("FMGRabsolutePathname");

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
 *  functionname  : FMGRwriteOpen
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
FMGRwriteOpen (char *format, ...)
{
    va_list arg_p;
    static char buffer[MAX_PATH_LEN];
    FILE *file;

    DBUG_ENTER ("FMGRwriteOpen");

    va_start (arg_p, format);
    vsprintf (buffer, format, arg_p);
    va_end (arg_p);

    file = fopen (buffer, "w");

    if (file == NULL) {
        CTIabort ("Unable to write file \"%s\"", buffer);
    }

    DBUG_RETURN (file);
}

/******************************************************************************
 *
 * function:
 *   locationtype FindLocationOfFile( char *file)
 *
 * description:
 *   This function checks wether file contains "$SACBASE/stdlib/".
 *   If so, LOC_stdlib is returned, otherwise LOC_usr.
 *
 *
 ******************************************************************************/

locationtype
FindLocationOfFile (char *file)
{
    static char stdlib_loc[MAX_FILE_NAME];
    char *sacbase;
    locationtype loc;

    sacbase = getenv ("SACBASE");
    strcpy (stdlib_loc, sacbase);
    strcat (stdlib_loc, "/stdlib/");

    if (strstr (file, stdlib_loc)) {
        loc = LOC_stdlib;
    } else {
        loc = LOC_usr;
    }

    return (loc);
}

/******************************************************************************
 *
 * Function:
 *   void FMGRsetFileNames( node *module)
 *
 * Description:
 *   Sets the global variables
 *     modulename, outfilename, cfilename, targetdir
 *   according to the kind of file and the -o command line option.
 *
 ******************************************************************************/

void
FMGRsetFileNames (node *module)
{
    char *buffer;

    /*
     * TODO: clean up this function
     */
    DBUG_ENTER ("FMGRsetFileNames");

    global.filetype = MODULE_FILETYPE (module);

    if (MODULE_FILETYPE (module) == F_prog) {

        global.modulename = MODULE_NAME (module);

        if (global.outfilename == NULL) {
            global.outfilename = "a.out";
            global.cfilename = "a.out.c";
            global.targetdir = "./";
        } else {
            global.cfilename = ILIBstringConcat (global.outfilename, ".c");
            global.targetdir = "";
        }
    } else {
        if (global.doprofile && global.genlib.sac) {
            CTIwarn ("Option -p turned off for module/class compilation");
            global.doprofile = FALSE;
        }

        if (global.sacfilename != NULL) {
            buffer = ILIBstringConcat (MODULE_NAME (module), ".sac");

            if (!ILIBstringCompare (buffer, global.puresacfilename)) {
                CTIwarn ("Module/class '%s` should be in a file named \"%s\" "
                         "instead of \"%s\"",
                         MODULE_NAME (module), buffer, global.sacfilename);
            }
            ILIBfree (buffer);
        }

        if (global.outfilename == NULL) {
            global.targetdir = "./";
        } else {
            global.targetdir = ILIBstringConcat (global.outfilename, "/");
        }

        global.modulename = MODULE_NAME (module);
        global.cfilename = ILIBstringConcat (MODULE_NAME (module), ".c");
        global.outfilename = ILIBstringConcat (MODULE_NAME (module), ".lib");

        global.targetdir = FMGRabsolutePathname (global.targetdir);
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void FMGRcleanUp()
 *
 *   @brief  removes all files and directories generated by sac2c
 *           except the actual result file. Used to clean up the
 *           file system in regular as well as irregular program
 *           termination.
 *
 ******************************************************************************/

void
FMGRcleanUp ()
{
    DBUG_ENTER ("FMGRcleanUp");

    if (global.cleanup && (global.tmp_dirname != NULL)) {
        ILIBsystemCall ("%s %s", global.config.rmdir, global.tmp_dirname);
    }

    DBUG_VOID_RETURN;
}

/*
 * Now, we create tmp directories for files generated during the
 * compilation process.
 *
 * Actually, only one temp directory is created whose name may be
 * accessed trough the global variable global.tmp_dirname
 * which is defined in globals.c.
 */

void
FMGRcreateTmpDir ()
{
    DBUG_ENTER ("FMGRcreateTmpDir");

#ifdef HAVE_MKDTEMP
    /* mkdtemp is safer than tempnam and recommended */
    /* on linux/bsd platforms.                       */

    global.tmp_dirname = (char *)ILIBmalloc (strlen (global.config.mkdir) + 12);
    global.tmp_dirname = strcpy (global.tmp_dirname, global.config.tmpdir);
    global.tmp_dirname = strcat (global.tmp_dirname, "/SAC_XXXXXX");

    global.tmp_dirname = mkdtemp (global.tmp_dirname);

    if (global.tmp_dirname == NULL) {
        CTIabort ("System failed to create temporary directory");
    }

#else /* HAVE_MKDTEMP */

    /* the old way for platforms not */
    /* supporting mkdtemp            */

    global.tmp_dirname = tempnam (global.config.tmpdir, "SAC_");

    if (global.tmp_dirname == NULL) {
        CTIabort ("System failed to create temporary directory");
    }

    ILIBsystemCall ("%s %s", global.config.mkdir, global.tmp_dirname);

    /* Failure of the system call is detected in ILIBsystemCall */

#endif /* HAVE_MKDTEMP */

    DBUG_VOID_RETURN;
}
