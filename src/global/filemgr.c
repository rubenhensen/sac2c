/*
 *
 * $Log$
 * Revision 1.20  2005/09/13 16:48:49  sah
 * minor bugfix
 *
 * Revision 1.19  2005/09/13 16:31:01  sah
 * removed FMGRfindLocationOfFile (not needed any more)
 * added FMGRmapPath
 *
 * Revision 1.18  2005/07/27 14:58:56  sah
 * added global.modulenamespace
 *
 * Revision 1.17  2005/07/18 15:45:59  sah
 * added findFilePath
 *
 * Revision 1.16  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 1.15  2005/06/18 18:07:09  sah
 * fixed a memory sharing bug
 *
 * Revision 1.14  2005/06/01 18:01:24  sah
 * finished printing of dependencies
 *
 * Revision 1.13  2005/06/01 12:47:45  sah
 * added lots of runtime paths
 *
 * Revision 1.12  2005/05/31 09:45:35  sah
 * libbuilding process now uses -o flag to determine
 * targetdir
 *
 * Revision 1.11  2005/04/26 16:13:54  sah
 * adopted path_bufs size to new situation
 *
 * Revision 1.10  2005/04/12 15:15:36  sah
 * cleaned up module system compiler args
 * and sac2crc parameters
 *
 * Revision 1.9  2005/04/12 13:57:31  sah
 * made returned strings constant as they point to a static buffer.
 * modified implementation accordingly.
 *
 * Revision 1.8  2005/03/10 09:41:09  cg
 * Handling of paths and creation of temporary directory brushed.
 *
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
#include "namespaces.h"
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

const char *
FMGRfindFilePath (pathkind_t p, const char *name)
{
    FILE *file = NULL;
    static char buffer[MAX_FILE_NAME];
    static char buffer2[MAX_PATH_LEN];
    char *path;
    char *result = NULL;

    DBUG_ENTER ("FMGRfindFilePath");

    if (name[0] == '/') { /* absolute path specified! */
        file = fopen (name, "r");
        path = "";
    } else {
        strcpy (buffer2, path_bufs[p]);
        path = strtok (buffer2, ":");
        while ((file == NULL) && (path != NULL)) {
            if (path[0] != '\0') {
                strcpy (buffer, path);
                strcat (buffer, "/");
                strcat (buffer, name);
                DBUG_PRINT ("FMGR", ("trying file %s\n", buffer));
                file = fopen (buffer, "r");
                if (file == NULL) {
                    path = strtok (NULL, ":");
                }
            } else {
                path = strtok (NULL, ":");
            }
        }
    }
    if (file != NULL) {
        fclose (file);
        result = path;
    }

    DBUG_RETURN (result);
}

const char *
FMGRfindFile (pathkind_t p, const char *name)
{
    static char buffer[MAX_FILE_NAME];
    const char *result;

    DBUG_ENTER ("FMGRfindFile");

    result = FMGRfindFilePath (p, name);

    if (result != NULL) {
        snprintf (buffer, MAX_FILE_NAME - 1, "%s/%s", result, name);
        result = buffer;
    }

    DBUG_RETURN (result);
}

void *
FMGRmapPath (pathkind_t p, void *(*mapfun) (const char *, void *), void *neutral)
{
    void *result = neutral;
    static char buffer[MAX_FILE_NAME];
    char *path;

    DBUG_ENTER ("FMGRmapPath");

    strcpy (buffer, path_bufs[p]);
    path = strtok (buffer, ":");

    while (path != NULL) {
        result = mapfun (path, result);
        path = strtok (NULL, ":");
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   bool FMGRcheckSystemLibrary( const char *name)
 *
 * description:
 *
 *   This function checks whether a given system library is found
 *   by the systems linker (eg ld). It does so by just calling ld
 *   with the given library and a dummy C program.
 *
 ******************************************************************************/

bool
FMGRcheckSystemLibrary (const char *name)
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
 *   bool FMGRcheckExistFile(const char *dir, const char *name)
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
FMGRcheckExistFile (const char *dir, const char *name)
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
FMGRappendPath (pathkind_t p, const char *path)
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
AppendEnvVar (pathkind_t p, const char *var)
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
 *   void AppendConfigPaths(int pathkind, const char *path
 *
 * description:
 *   This function adds the paths for module/class declarations and
 *   libraries of the standard library read from the configuration files
 *   to the internal paths.
 *
 *
 ******************************************************************************/

static void
AppendConfigPaths (pathkind_t pathkind, const char *path)
{
    char *pathentry;
    char buffer[MAX_PATH_LEN];
    char *envvar_end;
    char *envvar;
    char *ptoken;
    int envvar_length;

    DBUG_ENTER ("AppendConfigPaths");

    /*
     * we have to copy path here, as strtok modifies it
     */

    ptoken = ILIBstringCopy (path);

    pathentry = strtok (ptoken, ":");

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

        pathentry = strtok (NULL, ":");
    }

    ptoken = ILIBfree (ptoken);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void FMGRsetupPaths()
 *
 * description:
 *
 * Now, we set our search paths for the source program, module declarations,
 * and module implementations...
 *
 * First, paths are specified by the respective compiler options which
 * have been transformed into absolute paths.
 * Then the current directory is moved to the end of the
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
FMGRsetupPaths ()
{
    DBUG_ENTER ("FMGRsetupPaths");

    FMGRappendPath (PK_path, ".");
    FMGRappendPath (PK_lib_path, ".");
    FMGRappendPath (PK_imp_path, ".");
    FMGRappendPath (PK_extlib_path, ".");

    AppendEnvVar (PK_path, "SAC_PATH");
    AppendEnvVar (PK_lib_path, "SAC_LIBRARY_PATH");
    AppendEnvVar (PK_imp_path, "SAC_IMPLEMENTATION_PATH");

    AppendConfigPaths (PK_lib_path, global.config.libpath);
    AppendConfigPaths (PK_imp_path, global.config.imppath);
    AppendConfigPaths (PK_extlib_path, global.config.extlibpath);

    DBUG_PRINT ("FMGR", ("PATH is %s", path_bufs[PK_path]));
    DBUG_PRINT ("FMGR", ("LIB_PATH is %s", path_bufs[PK_lib_path]));
    DBUG_PRINT ("FMGR", ("IMP_PATH is %s", path_bufs[PK_imp_path]));
    DBUG_PRINT ("FMGR", ("EXTLIB_PATH is %s", path_bufs[PK_extlib_path]));

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

const char *
FMGRabsolutePathname (const char *path)
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
FMGRwriteOpen (const char *format, ...)
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
 * Function:
 *   void FMGRsetFileNames( node *module)
 *
 * Description:
 *   Sets the global variables
 *     modulename, modulenamespace, outfilename, cfilename, targetdir
 *   according to the kind of file and the -o command line option.
 *
 ******************************************************************************/

void
FMGRsetFileNames (node *module)
{
    char *buffer;

    DBUG_ENTER ("FMGRsetFileNames");

    global.filetype = MODULE_FILETYPE (module);

    if (MODULE_FILETYPE (module) == F_prog) {

        global.modulenamespace = NSdupNamespace (MODULE_NAMESPACE (module));
        global.modulename = ILIBstringCopy (NSgetName (MODULE_NAMESPACE (module)));

        if (global.outfilename == NULL) {
            global.outfilename = "a.out";
            global.cfilename = "a.out.c";
            global.targetdir = "";
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
            buffer = ILIBstringConcat (NSgetName (MODULE_NAMESPACE (module)), ".sac");

            if (!ILIBstringCompare (buffer, global.puresacfilename)) {
                CTIwarn ("Module/class '%s` should be in a file named \"%s\" "
                         "instead of \"%s\"",
                         NSgetName (MODULE_NAMESPACE (module)), buffer,
                         global.sacfilename);
            }
            ILIBfree (buffer);
        }

        if (global.outfilename == NULL) {
            global.targetdir = "";
        } else {
            global.targetdir = ILIBstringConcat (global.outfilename, "/");
        }

        global.modulenamespace = NSdupNamespace (MODULE_NAMESPACE (module));
        global.modulename = ILIBstringCopy (NSgetName (MODULE_NAMESPACE (module)));
        global.cfilename = ILIBstringConcat (global.modulename, ".c");
        global.outfilename = ILIBstringConcat (global.modulename, ".out");
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void FMGRdeleteTmpDir()
 *
 *   @brief  removes all files and directories generated by sac2c
 *           except the actual result file. Used to clean up the
 *           file system in regular as well as irregular program
 *           termination.
 *
 ******************************************************************************/

void
FMGRdeleteTmpDir ()
{
    DBUG_ENTER ("FMGRdeleteTmpDir");

    if (global.cleanup && (global.tmp_dirname != NULL)) {
        ILIBsystemCall ("%s %s", global.config.rmdir, global.tmp_dirname);
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void FMGRcreateTmpDir()
 *
 *   @brief  creates tmp directory and stores path in global variable
 *
 ******************************************************************************/

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
