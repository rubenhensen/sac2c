/*
 * $Id$
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "dbug.h"
#include "system.h"
#include "str.h"
#include "memory.h"
#include "ctinfo.h"
#include "free.h"
#include "types.h"
#include "filemgr.h"
#include "resource.h"
#include "namespaces.h"
#include "tree_basic.h"
#include "globals.h"

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

    SYScall ("echo \"int main(){return(0);}\" >%s/SAC_XX_syslibtest.c",
             global.tmp_dirname);

    result = SYScallNoErr ("%s %s %s -l%s -o %s/SAC_XX_syslibtest %s/SAC_XX_syslibtest.c",
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

    tmp = STRcatn (3, dir, "/", name);

    file = fopen (tmp, "r");
    tmp = MEMfree (tmp);

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

    ptoken = STRcpy (path);

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

    ptoken = MEMfree (ptoken);

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

/*
 *
 *  functionname  : FMGRappendOpen
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
FMGRappendOpen (const char *format, ...)
{
    va_list arg_p;
    static char buffer[MAX_PATH_LEN];
    FILE *file;

    DBUG_ENTER ("FMGRappendOpen");

    va_start (arg_p, format);
    vsprintf (buffer, format, arg_p);
    va_end (arg_p);

    file = fopen (buffer, "a");

    if (file == NULL) {
        CTIabort ("Unable to append file \"%s\"", buffer);
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
        global.modulename = STRcpy (NSgetName (MODULE_NAMESPACE (module)));

        if (global.outfilename == NULL) {
            global.outfilename = "a.out";
            global.cfilename = "a.out.c";
            global.targetdir = "";
        } else {
            global.cfilename = STRcat (global.outfilename, ".c");
            global.targetdir = "";
        }
    } else {
        if (global.doprofile && global.genlib.sac) {
            CTIwarn ("Option -p turned off for module/class compilation");
            global.doprofile = FALSE;
        }

        if (global.sacfilename != NULL) {
            buffer = STRcat (NSgetName (MODULE_NAMESPACE (module)), ".sac");

            if (!STReq (buffer, global.puresacfilename)) {
                CTIwarn ("Module/class '%s` should be in a file named \"%s\" "
                         "instead of \"%s\"",
                         NSgetName (MODULE_NAMESPACE (module)), buffer,
                         global.sacfilename);
            }
            MEMfree (buffer);
        }

        if (global.outfilename == NULL) {
            global.targetdir = "";
        } else {
            global.targetdir = STRcat (global.outfilename, "/");
        }

        global.modulenamespace = NSdupNamespace (MODULE_NAMESPACE (module));
        global.modulename = STRcpy (NSgetName (MODULE_NAMESPACE (module)));
        global.cfilename = STRcat (global.modulename, ".c");
        global.outfilename = STRcat (global.modulename, ".out");
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

    if (global.tmp_dirname != NULL) {
        SYScall ("%s %s", global.config.rmdir, global.tmp_dirname);
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

    global.tmp_dirname = (char *)MEMmalloc (strlen (global.config.mkdir) + 12);
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

    SYScall ("%s %s", global.config.mkdir, global.tmp_dirname);

    /* Failure of the system call is detected in SYScall */

#endif /* HAVE_MKDTEMP */

    DBUG_VOID_RETURN;
}