/*
 *
 * $Id$
 *
 */

#include "ccmanager.h"
#include "system.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "dbug.h"
#include "ctinfo.h"
#include "globals.h"
#include "config.h"
#include "filemgr.h"
#include "resource.h"
#include "stringset.h"
#include "filemgr.h"

static char *
GetCCCall ()
{
    str_buf *buffer;
    char *result;

    DBUG_ENTER ("GetCCCall");

    buffer = SBUFcreate (128);

    SBUFprintf (buffer, "%s %s %s %s %s -L%s ", global.config.cc, global.config.ccflags,
                global.ccflags, global.config.ldflags, global.config.ccdir,
                global.tmp_dirname);

    result = SBUF2str (buffer);

    buffer = SBUFfree (buffer);

    DBUG_RETURN (result);
}

static void
AddOptimizeFlag (str_buf *buffer)
{
    DBUG_ENTER ("AddOptimizeFlag");

    switch (global.cc_optimize) {
    case 0:
        SBUFprintf (buffer, "%s ", global.config.opt_O0);
        break;
    case 1:
        SBUFprintf (buffer, "%s ", global.config.opt_O1);
        break;
    case 2:
        SBUFprintf (buffer, "%s ", global.config.opt_O2);
        break;
    case 3:
        SBUFprintf (buffer, "%s ", global.config.opt_O3);
        break;
    default:
        break;
    }

    DBUG_VOID_RETURN;
}

static void
AddDebugFlag (str_buf *buffer)
{
    DBUG_ENTER ("AddDebugFlag");

    if (global.cc_debug) {
        SBUFprintf (buffer, "%s ", global.config.opt_g);
    }

    DBUG_VOID_RETURN;
}

static char *
GetCCFlags ()
{
    str_buf *buffer;
    char *result;

    DBUG_ENTER ("GetCCFlags");

    buffer = SBUFcreate (1024);

    AddOptimizeFlag (buffer);
    AddDebugFlag (buffer);

    result = SBUF2str (buffer);
    buffer = SBUFfree (buffer);

    DBUG_RETURN (result);
}

static void
AddCCLibs (str_buf *buffer)
{
    DBUG_ENTER ("AddCCLibs");

    SBUFprintf (buffer, "%s ", global.config.cclink);

    /*
     * We link with the two libraries below if they are installed but regardless
     * of whether multithreading or dynamic linking are actually utilized in the
     * given program or any of the libraries it depends on.
     *
     * The latter is difficult to decide in the presence of the module system,
     * where any of the subordinate modules may have been compiled with any possible
     * set of compiler flags. Hence, the linker flags cannot be determined by the
     * compiler flags used when compiling the final program alone.
     *
     * Instead of creating a complex mechanism that tracks such dependencies across
     * modules, we just employ the linker to do this for us.
     */

#if ENABLE_MT
    if (global.backend != BE_cuda) { /* Does not work with cuda compiler nvcc */
        SBUFprintf (buffer, "%s ", global.config.ccmtlink);
    }
#endif

#if ENABLE_RTSPEC
    SBUFprintf (buffer, "%s ", global.config.ccdllink);
#endif

    DBUG_VOID_RETURN;
}

static void
AddSacLib (str_buf *buffer)
{
    DBUG_ENTER ("AddSacLib");

    SBUFprint (buffer, "-lsac");
    SBUFprint (buffer, global.config.lib_variant);

    if (global.mtmode == MT_none) {
        if (global.backend == BE_omp) {
            SBUFprint (buffer, ".mt.omp ");
        } else {
            SBUFprint (buffer, ".seq ");
        }
    } else {
        SBUFprint (buffer, ".mt.pth ");
    }

    DBUG_VOID_RETURN;
}

static void
AddPhmLib (str_buf *buffer)
{
    DBUG_ENTER ("AddPhmLib");

    if (global.optimize.dophm) {
        SBUFprint (buffer, "-lsacphm");
        SBUFprint (buffer, global.config.lib_variant);

        if ((global.mtmode == MT_none) && (global.backend != BE_omp)) {
            SBUFprint (buffer, ".seq");
        } else {
            SBUFprint (buffer, ".mt");
        }

        if (global.runtimecheck.heap) {
            SBUFprint (buffer, ".diag ");
        } else {
            SBUFprint (buffer, " ");
        }
    }

    DBUG_VOID_RETURN;
}

static void
AddEfenceLib (str_buf *buffer)
{
    DBUG_ENTER ("AddEfenceLib");

    if (global.use_efence) {
        char *efence;

        efence = STRcpy (FMGRfindFile (PK_extlib_path, "libefence.a"));

        if (efence == NULL) {
            CTIwarn ("Unable to find `libefence.a' in EXTLIB_PATH");
        } else {
            SBUFprintf (buffer, "%s ", efence);
        }
    }

    DBUG_VOID_RETURN;
}

static char *
GetLibs ()
{
    str_buf *buffer;
    char *result;

    DBUG_ENTER ("GetLibs");

    buffer = SBUFcreate (256);

    AddPhmLib (buffer);
    AddSacLib (buffer);
    AddCCLibs (buffer);
    AddEfenceLib (buffer);

    result = SBUF2str (buffer);
    buffer = SBUFfree (buffer);

    DBUG_RETURN (result);
}

static str_buf *
AddLibPath (const char *path, str_buf *buf)
{
    char *flag;

    DBUG_ENTER ("AddLibPath");

    flag = STRsubstToken (global.config.ld_path, "%path%", path);
    buf = SBUFprintf (buf, "%s ", flag);
    flag = MEMfree (flag);

    DBUG_RETURN (buf);
}

static char *
GetLibPath ()
{
    char *result;
    str_buf *buffer = SBUFcreate (255);

    DBUG_ENTER ("GetLibPath");

    buffer = (str_buf *)FMGRmapPath (PK_lib_path,
                                     (void *(*)(const char *, void *))AddLibPath, buffer);

    buffer = (str_buf *)FMGRmapPath (PK_extlib_path,
                                     (void *(*)(const char *, void *))AddLibPath, buffer);

    result = SBUF2str (buffer);
    buffer = SBUFfree (buffer);

    DBUG_RETURN (result);
}

static void *
BuildDepLibsStringProg (const char *lib, strstype_t kind, void *rest)
{
    char *result;

    DBUG_ENTER ("BuildDepLibsStringProg");

    switch (kind) {
    case STRS_saclib:
        result = MEMmalloc (sizeof (char)
                            * (STRlen (lib) + 6 + STRlen (global.config.lib_variant)));
        sprintf (result, "-l%sMod%s", lib, global.config.lib_variant);

        break;
    case STRS_extlib:
        result = MEMmalloc (sizeof (char) * (STRlen (lib) + 3));
        sprintf (result, "-l%s", lib);

        break;
    case STRS_objfile:
        result = MEMmalloc (sizeof (char) * (STRlen (lib) + 3));
        sprintf (result, "%s", lib);
        break;
    default:
        result = STRcpy ("");
        break;
    }

    if (rest != NULL) {
        char *temp
          = MEMmalloc (sizeof (char) * (STRlen ((char *)rest) + STRlen (result) + 2));

        sprintf (temp, "%s %s", (char *)rest, result);

        result = MEMfree (result);
        rest = MEMfree (rest);
        result = temp;
    }

    DBUG_RETURN (result);
}

static void
InvokeCCProg (char *cccall, char *ccflags, char *libs, stringset_t *deps)
{
    char *libpath;
    char *deplibs;

    DBUG_ENTER ("InvokeCCProg");

    libpath = GetLibPath ();

    deplibs = (char *)STRSfold (&BuildDepLibsStringProg, deps, STRcpy (""));

    SYScall ("%s %s -o %s %s %s %s %s", cccall, ccflags, global.outfilename,
             global.cfilename, libpath, deplibs, libs);

    libpath = MEMfree (libpath);
    deplibs = MEMfree (deplibs);

    DBUG_VOID_RETURN;
}

static void
CompileOneFile (const char *dir, const char *file, const char *callstring)
{
    char *basename;

    DBUG_ENTER ("CompileOneFile");

    basename = STRtok (file, ".");

    SYScall ("cd %s; %s -c %s -o %s_nonpic.o", dir, callstring, file, basename);

    basename = MEMfree (basename);

    DBUG_VOID_RETURN;
}

static void
CompileOneFilePIC (const char *dir, const char *file, const char *callstring)
{
    char *basename;

    DBUG_ENTER ("CompileOneFilePIC");

    basename = STRtok (file, ".");

    SYScall ("cd %s; %s %s -c %s -o %s_pic.o", dir, callstring, global.config.genpic,
             file, basename);

    basename = MEMfree (basename);

    DBUG_VOID_RETURN;
}

static void
InvokeCCModule (char *cccall, char *ccflags)
{
    char *callstring;
    char *str;

    DBUG_ENTER ("InvokeCCModule");

    callstring = STRcat (cccall, ccflags);

    /*
     * compile non-PIC code
     */
    str = STRcat ("fun.*\\.", global.config.cext);
    FMGRforEach (global.tmp_dirname, str, callstring,
                 (void (*) (const char *, const char *, void *))CompileOneFile);
    MEMfree (str);

    str = STRcat ("globals.", global.config.cext);
    CompileOneFile (global.tmp_dirname, str, callstring);
    MEMfree (str);
    if (!STReq (global.config.ld_dynamic, "")) {
        /*
         * compile PIC code for shared libs if needed
         */
        str = STRcat ("fun.*\\.", global.config.cext);
        FMGRforEach (global.tmp_dirname, str, callstring,
                     (void (*) (const char *, const char *, void *))CompileOneFilePIC);
        MEMfree (str);

        str = STRcat ("globals.", global.config.cext);
        CompileOneFilePIC (global.tmp_dirname, str, callstring);
        MEMfree (str);
    }

    SYScall ("cd %s; %s %s -c serialize.%s", global.tmp_dirname, global.config.tree_cc,
             global.config.ccdir, global.config.tree_cext);
    SYScall ("cd %s; %s %s -c filenames.%s", global.tmp_dirname, global.config.tree_cc,
             global.config.ccdir, global.config.tree_cext);
    SYScall ("cd %s; %s %s -c namespacemap.%s", global.tmp_dirname, global.config.tree_cc,
             global.config.ccdir, global.config.tree_cext);
    SYScall ("cd %s; %s %s -c symboltable.%s", global.tmp_dirname, global.config.tree_cc,
             global.config.ccdir, global.config.tree_cext);
    SYScall ("cd %s; %s %s -c dependencytable.%s", global.tmp_dirname,
             global.config.tree_cc, global.config.ccdir, global.config.tree_cext);

    callstring = MEMfree (callstring);

    DBUG_VOID_RETURN;
}

static void
InvokeCCWrapper (char *cccall, char *ccflags)
{
    char *callstring;
    char *str;
    DBUG_ENTER ("InvokeCCWrapper");

    callstring = STRcat (cccall, ccflags);

    /*
     * compile non-PIC code
     */
    str = STRcat ("fun.*\\.", global.config.cext);
    FMGRforEach (global.tmp_dirname, str, callstring,
                 (void (*) (const char *, const char *, void *))CompileOneFile);
    MEMfree (str);

    str = STRcat ("globals.", global.config.cext);
    CompileOneFile (global.tmp_dirname, str, callstring);
    MEMfree (str);

    str = STRcat ("interface.", global.config.cext);
    CompileOneFile (global.tmp_dirname, str, callstring);
    MEMfree (str);

    str = STRcat ("sacargcopy.", global.config.cext);
    CompileOneFile (global.tmp_dirname, str, callstring);
    MEMfree (str);

    str = STRcat ("sacargfree.", global.config.cext);
    CompileOneFile (global.tmp_dirname, str, callstring);
    MEMfree (str);

    /*
     * compile PIC code
     */
    str = STRcat ("fun.*\\.", global.config.cext);
    FMGRforEach (global.tmp_dirname, str, callstring,
                 (void (*) (const char *, const char *, void *))CompileOneFilePIC);
    MEMfree (str);

    str = STRcat ("globals.", global.config.cext);
    CompileOneFilePIC (global.tmp_dirname, str, callstring);
    MEMfree (str);

    str = STRcat ("interface.", global.config.cext);
    CompileOneFilePIC (global.tmp_dirname, str, callstring);
    MEMfree (str);

    str = STRcat ("sacargcopy.", global.config.cext);
    CompileOneFilePIC (global.tmp_dirname, str, callstring);
    MEMfree (str);

    str = STRcat ("sacargfree.", global.config.cext);
    CompileOneFilePIC (global.tmp_dirname, str, callstring);
    MEMfree (str);

    callstring = MEMfree (callstring);

    DBUG_VOID_RETURN;
}

node *
CCMinvokeCC (node *syntax_tree)
{
    char *ccflags;
    char *cccall;
    char *libs;
    stringset_t *deps = global.dependencies;

    DBUG_ENTER ("CCMinvokeCC");

    if (global.gen_cccall) {
        /*
         * enable system call tracking
         */
        SYSstartTracking ();
    }

    cccall = GetCCCall ();
    ccflags = GetCCFlags ();
    libs = GetLibs ();

    if (global.filetype == FT_prog) {
        InvokeCCProg (cccall, ccflags, libs, deps);
    } else if (global.filetype == FT_cmod) {
        InvokeCCWrapper (cccall, ccflags);
    } else {
        InvokeCCModule (cccall, ccflags);
    }

    cccall = MEMfree (cccall);
    ccflags = MEMfree (ccflags);
    libs = MEMfree (libs);

    if (global.gen_cccall) {
        /*
         * stop tracking and close file
         */
        SYSstopTracking ();
    }

    DBUG_RETURN (syntax_tree);
}

char *
CCMgetLinkerFlags (node *syntax_tree)
{
    char *libs;
    char *paths;
    char *deplibs;
    char *result;

    DBUG_ENTER ("CCMgetLinkerFlags");

    libs = GetLibs ();
    paths = GetLibPath ();
    deplibs
      = (char *)STRSfold (&BuildDepLibsStringProg, global.dependencies, STRcpy (""));

    result = STRcatn (5, paths, " ", libs, " ", deplibs);

    libs = MEMfree (libs);
    paths = MEMfree (paths);
    deplibs = MEMfree (deplibs);

    DBUG_RETURN (result);
}
