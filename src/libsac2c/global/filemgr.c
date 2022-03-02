#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define DBUG_PREFIX "FMGR"
#include "debug.h"

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
#include "str.h"
#include "str_buffer.h"

static str_buf *path_bufs[PK_LAST];

static void
FMGRensureInitialized (void)
{
    static int FMGRinitDone = 0;
    if (FMGRinitDone == 0) {
        int i;
        for (i = 0; i < PK_LAST; ++i)
            path_bufs[i] = SBUFcreate (1);

        FMGRinitDone = 1;
    }
}

/*
 *
 *  functionname  : FMGRfindFilePath
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
FMGRfindFilePath (pathkind_t p, const char *name)
{
    char *path = NULL;

    DBUG_ENTER ();
    FMGRensureInitialized ();

    if (name[0] == '/') { /* absolute path specified! */
        if (!FMGRcheckExistFile (name))
            CTIabort (EMPTY_LOC, "Error: cannot find/open '%s'.", name);

        path = STRcpy ("");
    } else {
        char *buffer = SBUF2str (path_bufs[p]);
        path = strtok (buffer, ":");

        while (path != NULL) {
            char *fpath = STRcatn (3, path, "/", name);
            DBUG_PRINT ("trying '%s'", fpath);
            if (FMGRcheckExistFile (fpath))
                break;
            path = strtok (NULL, ":");
        }
        if (path != NULL) {
            path = STRcpy (path);
        }
        buffer = MEMfree (buffer);
    }

    DBUG_RETURN (path);
}

char *
FMGRfindFile (pathkind_t p, const char *name)
{
    DBUG_ENTER ();

    char *result = FMGRfindFilePath (p, name);

    if (result != NULL) {
        char *tmp = STRcatn (3, result, "/", name);
        MEMfree (result);
        result = tmp;
    }

    DBUG_RETURN (result);
}

void *
FMGRmapPath (pathkind_t p, void *(*mapfun) (const char *, void *), void *neutral)
{
    DBUG_ENTER ();
    FMGRensureInitialized ();

    void *result = neutral;

    char *buffer = SBUF2str (path_bufs[p]);
    char *path = strtok (buffer, ":");

    while (path != NULL) {
        result = mapfun (path, result);
        path = strtok (NULL, ":");
    }
    buffer = MEMfree (buffer);

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn bool FMGRcheckExistDir(const char *dir)
 *
 * @brief The function checks whether the given directory path exists.
 *
 * @param dir directory path
 *
 * @return TRUE if the directory exists and is writeable.
 ******************************************************************************/
bool
FMGRcheckExistDir (const char *dir)
{
    bool res;

    DBUG_ENTER ();

    DBUG_ASSERT (dir != NULL, "Function FMGRcheckExistDir() called with dir NULL");

    DIR *d = opendir (dir);
    if (d == NULL) {
        res = FALSE;
    } else {
        res = TRUE;
        closedir (d);
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn bool FMGRcheckExistFile(const char *file)
 *
 * @brief The function checks whether the given file path exists.
 *
 * @param file file path
 *
 * @return TRUE if the file exists and is readable.
 ******************************************************************************/
bool
FMGRcheckExistFile (const char *file)
{
    bool res;

    DBUG_ENTER ();

    DBUG_ASSERT (file != NULL, "Function FMGRcheckExistFile() called with file NULL");

    FILE *f = fopen (file, "r");
    if (f == NULL) {
        res = FALSE;
    } else {
        res = TRUE;
        fclose (f);
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
    DBUG_ENTER ();
    FMGRensureInitialized ();

    SBUFprintf (path_bufs[p], ":%s", path);

    DBUG_PRINT ("appending \":%s\" to path %d", path, p);

    DBUG_RETURN ();
}

void
FMGRprependPath (pathkind_t p, const char *path)
{
    DBUG_ENTER ();
    FMGRensureInitialized ();

    char *tmp = SBUF2str (path_bufs[p]);
    SBUFflush (path_bufs[p]);
    SBUFprintf (path_bufs[p], "%s:%s", path, tmp);
    MEMfree (tmp);

    DBUG_PRINT ("prepending \"%s:\" to path %d", path, p);

    DBUG_RETURN ();
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
    DBUG_ENTER ();
    FMGRensureInitialized ();

    char *buffer = getenv (var);
    if (buffer != NULL) {
        SBUFprintf (path_bufs[p], ":%s", buffer);

        DBUG_PRINT ("appending \":%s\" to path %d", buffer, p);
    }
    DBUG_RETURN ();
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
    DBUG_ENTER ();

    /*
     * since the format of the argument path
     * is the same as the stored set of paths
     * (colon-separated strings), we don't have much to do...
     */
    char *ptoken = STRcpy (path);
    char *pathentry = strtok (ptoken, ":");
    while (pathentry != NULL) {
        FMGRappendPath (pathkind, pathentry);
        pathentry = strtok (NULL, ":");
    }
    ptoken = MEMfree (ptoken);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void FMGRsetupPaths( void)
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
 *
 *
 ******************************************************************************/

void
FMGRsetupPaths (void)
{
    DBUG_ENTER ();

    FMGRappendPath (PK_path, ".");
    AppendEnvVar (PK_path, "SAC_PATH");
    DBUG_PRINT ("Source files searched in %s", SBUFgetBuffer (path_bufs[PK_path]));

#define INIT_PATH(Path, RName, Var)                                                      \
    AppendEnvVar (PK_##Path##_path, Var);                                                \
    AppendConfigPaths (PK_##Path##_path, global.config.Path##path);                      \
    DBUG_PRINT (#RName "PATH is %s", SBUFgetBuffer (path_bufs[PK_##Path##_path]));

    INIT_PATH (imp, IMP, "SAC_IMPLEMENTATION_PATH");
    INIT_PATH (inc, INC, "SAC_INCLUDES_PATH");
    INIT_PATH (extlib, EXTLIB, "SAC_EXTERNAL_LIBRARY_PATH");

#define INIT_PATH2(Path, RName, Var)                                                     \
    AppendEnvVar (PK_##Path##_path, Var);                                                \
    AppendConfigPaths (PK_##Path##_path, global.config.Path##_outputdir);                \
    AppendConfigPaths (PK_##Path##_path, global.config.Path##path);                      \
    DBUG_PRINT (#RName "PATH is %s", SBUFgetBuffer (path_bufs[PK_##Path##_path]));

    INIT_PATH2 (lib, LIB, "SAC_LIBRARY_PATH");
    INIT_PATH2 (tree, TREE, "SAC_TREE_PATH");

    DBUG_RETURN ();
}

/*
 * Retrieve the "directory" part of the path.
 */
char *
FMGRdirname (const char *path)
{
    DBUG_ENTER ();

    const char *last = strrchr (path, '/');
    char *result;
    size_t len = (size_t)(last - path);

    if (last == NULL) {
        /* No dir */
        result = STRcpy (".");
    } else {
        result = MEMmalloc (len + 1);
        memcpy (result, path, len);
        result[len] = '\0';
    }

    char *newresult = FMGRabsName (result);
    result = MEMfree (result);
    result = newresult;

    DBUG_RETURN (result);
}

/*
 * Retrieve the absolute path of a path.
 * The result is always a new string.
 */
char *
FMGRabsName (const char *path)
{
    if (path[0] != '/') {
        /* first reduce as follows:
           "./xxxx" -> reduce("xxxx")
           "./"     -> ""
           "."      -> ""
        */
        while (path[0] == '.' && (!path[1] || path[1] == '/'))
            path += path[1] ? 2 : 1;

        /* end situation:
           ""    -> cwd
           "xxx" -> cwd/xxx
        */
        return path[0] ? STRcatn (3, global.cwd, "/", path) : STRcpy (global.cwd);
    } else
        return STRcpy (path);
}

/*
 * Retrieve the "basename" part of the path.
 */
char *
FMGRbasename (const char *path)
{
    DBUG_ENTER ();

    const char *last = strrchr (path, '/');
    if (last == NULL) {
        last = path;
    } else {
        ++last;
    }
    char *result = STRcpy (last);

    DBUG_RETURN (result);
}

/*
 * Convert file name to identifier
 * Make non alpha numeric '_'
 */
char *
FMGRfile2id (const char *path)
{
    DBUG_ENTER ();

    char *result = STRcpy (path);
    char *current = result;

    while (*current != '\0') {
        if (!isalnum (*current))
            *current = '_';
        ++current;
    }

    DBUG_RETURN (result);
}

/*
 *
 *  functionname  : FMGRwriteOpen
 *  functionname  : FMGRreadWriteOpen
 *  arguments     : 1) format string like that of printf
 *                  2) variable argument list for 1)
 *  description   : FMGRwriteOpen opens the given file for writing.
 *                  If this fails, an error message occurs and compilation is aborted.
 *                  FMGRreadWriteOpen opens the given file for both reading and writing.
 *                  If this fails, an error message occurs and compilation is aborted.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : vsprintf, va_start, va_end, fopen,
 *  macros        : vararg macros, ERROR
 *
 *  remarks       :
 *
 */

FILE *
FMGRwriteOpen (const char *format, ...)
{
    va_list arg_p;
    static char buffer[PATH_MAX];
    FILE *file;

    DBUG_ENTER ();

    va_start (arg_p, format);
    vsnprintf (buffer, PATH_MAX - 1, format, arg_p);
    va_end (arg_p);

    file = fopen (buffer, "w");

    if (file == NULL) {
        CTIabort (EMPTY_LOC, "Unable to write file \"%s\"", buffer);
    } else {
        DBUG_PRINT ("Opening file \"%s\" for writing", buffer);
    }

    DBUG_RETURN (file);
}

FILE *
FMGRreadWriteOpen (const char *format, ...)
{
    va_list arg_p;
    static char buffer[PATH_MAX];
    FILE *file;

    DBUG_ENTER ();

    va_start (arg_p, format);
    vsnprintf (buffer, PATH_MAX - 1, format, arg_p);
    va_end (arg_p);

    file = fopen (buffer, "w+");

    if (file == NULL) {
        CTIabort (EMPTY_LOC, "Unable to write file \"%s\"", buffer);
    } else {
        DBUG_PRINT ("Opening file \"%s\" for writing and reading", buffer);
    }

    DBUG_RETURN (file);
}

/*
 *
 *  functionname  : FMGRreadOpen
 *  arguments     : 1) format string like that of printf
 *                  2) variable argument list for 1)
 *  description   : opens the given file for writing. If this fails,
 *                  an error message
 *                  occurs and compilation is aborted.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : vsprintf, va_start, va_end, fopen,
 *  macros        : vararg macros, ERROR
 *
 *  remarks       :
 *
 */

FILE *
FMGRreadOpen (const char *format, ...)
{
    va_list arg_p;
    static char buffer[PATH_MAX];
    FILE *file;

    DBUG_ENTER ();

    va_start (arg_p, format);
    vsnprintf (buffer, PATH_MAX - 1, format, arg_p);
    va_end (arg_p);

    file = fopen (buffer, "r");

    if (file == NULL) {
        CTIabort (EMPTY_LOC, "Unable to read file \"%s\"", buffer);
    }

    DBUG_RETURN (file);
}

/*
 *
 *  functionname  : FMGRwriteOpenExecutable
 *  arguments     : 1) format string like that of printf
 *                  2) variable argument list for 1)
 *  description   : opens the given file for writing. If this fails,
 *                  an error message
 *                  occurs and compilation is aborted.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : vsprintf, va_start, va_end, fopen, open, fdopen
 *  macros        : vararg macros, ERROR
 *
 *  remarks       :
 *
 */

FILE *
FMGRwriteOpenExecutable (const char *format, ...)
{
    va_list arg_p;
    static char buffer[PATH_MAX];
    FILE *file;
    int fd;

    DBUG_ENTER ();

    va_start (arg_p, format);
    vsnprintf (buffer, PATH_MAX - 1, format, arg_p);
    va_end (arg_p);

    fd = open (buffer, O_CREAT | O_WRONLY | O_TRUNC,
               S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    file = fdopen (fd, "w");

    if (file == NULL) {
        CTIabort (EMPTY_LOC, "Unable to write file \"%s\"", buffer);
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
    static char buffer[PATH_MAX];
    FILE *file;

    DBUG_ENTER ();

    va_start (arg_p, format);
    vsnprintf (buffer, PATH_MAX - 1, format, arg_p);
    va_end (arg_p);

    file = fopen (buffer, "a");

    if (file == NULL) {
        CTIabort (EMPTY_LOC, "Unable to append file \"%s\"", buffer);
    }

    DBUG_RETURN (file);
}

/** <!-- ****************************************************************** -->
 * @fn FILE *FMGRclose( FILE *file)
 *
 * @brief Closes the given file and aborts on error.
 *
 * @param file file handle of file to close
 *
 * @return always returns NULL
 ******************************************************************************/

FILE *
FMGRclose (FILE *file)
{
    DBUG_ENTER ();

    if (fclose (file) != 0) {
        CTIabort (EMPTY_LOC, "There was an error while closing a file.");
    }

    DBUG_RETURN ((FILE *)NULL);
}

/******************************************************************************
 *
 * Function:
 *   void FMGRsetFileNames( node *module)
 *
 * Description:
 *   Sets the global variables
 *     modulename, modulenamespace, outfilename, targetdir
 *   according to the kind of file and the -o command line option.
 *
 ******************************************************************************/

void
FMGRsetFileNames (node *module)
{
    char *buffer;

    DBUG_ENTER ();

    global.filetype = MODULE_FILETYPE (module);

    if (MODULE_FILETYPE (module) == FT_prog) {

        global.modulenamespace = NSdupNamespace (MODULE_NAMESPACE (module));
        global.modulename = STRcpy (NSgetName (MODULE_NAMESPACE (module)));

        if (global.outfilename == NULL) {
            global.outfilename = STRcpy ("a.out");
        }

        global.targetdir = FMGRdirname (global.outfilename);
        char *tmp = FMGRbasename (global.outfilename);
        MEMfree (global.outfilename);
        global.outfilename = tmp;
    } else {
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
            global.targetdir
              = FMGRabsName (global.install ? global.config.tree_outputdir : ".");
        } else {
            global.targetdir = FMGRabsName (global.outfilename);
            if (!FMGRcheckExistDir (global.targetdir)) {
                CTIabort (EMPTY_LOC, "Target directory `%s' does not exist.", global.targetdir);
            }
        }

        global.modulenamespace = NSdupNamespace (MODULE_NAMESPACE (module));
        global.modulename = STRcpy (NSgetName (MODULE_NAMESPACE (module)));
        global.outfilename = STRcpy (global.modulename);
    }

    if (global.target_modlibdir == NULL)
        global.target_modlibdir = global.install ? STRcpy (global.config.lib_outputdir)
                                                 : STRcpy (global.targetdir);

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void FMGRdeleteTmpDir( void)
 *
 *   @brief  removes all files and directories generated by sac2c
 *           except the actual result file. Used to clean up the
 *           file system in regular as well as irregular program
 *           termination.
 *
 ******************************************************************************/

void
FMGRdeleteTmpDir (void)
{
    DBUG_ENTER ();

    if (global.tmp_dirname != NULL) {
        SYScall ("%s %s", global.config.rmdir, global.tmp_dirname);
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void FMGRcreateTmpDir( void)
 *
 *   @brief  creates tmp directory and stores path in global variable
 *
 ******************************************************************************/

#ifdef HAVE_MKDTEMP
/* mkdtemp is safer than tempnam and recommended */
/* on linux/bsd platforms.                       */

void
FMGRcreateTmpDir (void)
{
    DBUG_ENTER ();

    global.tmp_dirname = STRcat (global.config.tmpdir, "/SAC_XXXXXX");
    global.tmp_dirname = mkdtemp (global.tmp_dirname);

    if (global.tmp_dirname == NULL) {
        CTIabort (EMPTY_LOC, "System failed to create temporary directory");
    }

    global.system_cleanup = STRcatn (3, global.config.rmdir, " ", global.tmp_dirname);
    /*
     * We set this variable already here to avoid the associated string and memory
     * handling at a later stage when the compiler is in an inconsistent state
     * signalled via an interrupt.
     */

    DBUG_RETURN ();
}

#else /* HAVE_MKDTEMP */

/* the old way for platforms not */
/* supporting mkdtemp            */

void
FMGRcreateTmpDir (void)
{
    DBUG_ENTER ();

    global.tmp_dirname = tempnam (global.config.tmpdir, "SAC_");

    if (global.tmp_dirname == NULL) {
        CTIabort (EMPTY_LOC, "System failed to create temporary directory");
    }

    SYScall ("%s %s", global.config.mkdir, global.tmp_dirname);

    /* Failure of the system call is detected in SYScall */

    global.system_cleanup = STRcatn (3, global.config.rmdir, " ", global.tmp_dirname);
    /*
     * We set this variable already here to avoid the associated string and memory
     * handling at a later stage when the compiler is in an inconsistent state
     * signalled via an interrupt.
     */

    DBUG_RETURN ();
}

#endif /* HAVE_MKDTEMP */

/** <!--********************************************************************-->
 *
 * @fn void FMGRforEach(const char *path, const char *filterexpr, void *funargs,
 *                 void (fun) (const char *path, const char *file, void *params))
 *
 * @brief
 *
 ******************************************************************************/

void
FMGRforEach (const char *path, const char *filterexpr, void *funargs,
             void(fun) (const char *path, const char *file, void *params))
{
    DIR *currdir;
    struct dirent *direntry;
    regex_t regexpr;
    int error;
    char *fullpattern;

    DBUG_ENTER ();

    /*
     * ensure the pattern only matches entire lines
     */
    fullpattern = STRcatn (3, "^", filterexpr, "$");

    currdir = opendir (path);

    if (currdir == NULL) {
        CTIabort (EMPTY_LOC, "Cannot read directory `%s'.", path);
    }

    error = regcomp (&regexpr, fullpattern, REG_NOSUB);
    DBUG_ASSERT (error == 0, "Illegal regular expression!");

    direntry = readdir (currdir);
    while (direntry != NULL) {
        if (regexec (&regexpr, direntry->d_name, 0, NULL, 0) == 0) {
            fun (path, direntry->d_name, funargs);
        }
        direntry = readdir (currdir);
    }

    regfree (&regexpr);

    closedir (currdir);
    fullpattern = MEMfree (fullpattern);

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
