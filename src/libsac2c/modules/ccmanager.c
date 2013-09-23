#include "ccmanager.h"
#include "system.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"

#define DBUG_PREFIX "CCM"
#include "debug.h"

#include "ctinfo.h"
#include "globals.h"
#include "config.h"
#include "filemgr.h"
#include "resource.h"
#include "stringset.h"
#include "filemgr.h"
#include "cygwinhelpers.h"

static void
AddOptimizeFlag (str_buf *buffer)
{
    DBUG_ENTER ();

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

    DBUG_RETURN ();
}

static void
AddDebugFlag (str_buf *buffer)
{
    DBUG_ENTER ();

    if (global.cc_debug) {
        SBUFprintf (buffer, "%s ", global.config.opt_g);
    }

    DBUG_RETURN ();
}

static char *
GetCCCall (void)
{
    str_buf *buffer;
    char *result;

    DBUG_ENTER ();

    if ((global.backend == BE_cuda || global.backend == BE_cudahybrid)
        && STReq (global.config.cuda_arch, "")) {
        CTIwarn ("CUDA architecture cannot be detected, set to default(1.0)\n");
    }

    buffer = SBUFcreate (128);

    SBUFprintf (buffer, "%s %s %s ", global.config.cc, global.config.ccflags,
                global.ccflags);

    result = SBUF2str (buffer);

    buffer = SBUFfree (buffer);

    DBUG_RETURN (result);
}

static char *
GetCompilationFlags (void)
{
    str_buf *buffer;
    char *result;

    DBUG_ENTER ();

    buffer = SBUFcreate (1024);

    SBUFprintf (buffer, "%s ", global.config.ccincdir);

    AddOptimizeFlag (buffer);
    AddDebugFlag (buffer);

    result = SBUF2str (buffer);

    buffer = SBUFfree (buffer);

    DBUG_RETURN (result);
}

static char *
GetLinkingFlags (void)
{
    str_buf *buffer;
    char *result;

    DBUG_ENTER ();

    buffer = SBUFcreate (1024);

    SBUFprintf (buffer, "%s %s -L%s ", global.config.ldflags, global.config.cclibdir,
                global.tmp_dirname);

    result = SBUF2str (buffer);

    buffer = SBUFfree (buffer);

    DBUG_RETURN (result);
}

static void
AddCCLibs (str_buf *buffer)
{
    DBUG_ENTER ();

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
    if (global.backend != BE_cuda
        && global.backend != BE_cudahybrid) { /* Does not work with cuda compiler nvcc */
        SBUFprintf (buffer, "%s ", global.config.ccmtlink);
    }
#endif

#if ENABLE_RTSPEC
    SBUFprintf (buffer, "%s ", global.config.ccdllink);
#endif

    DBUG_RETURN ();
}

static void
AddSacLib (str_buf *buffer)
{
    DBUG_ENTER ();

    SBUFprint (buffer, "-lsac");
    SBUFprint (buffer, global.config.lib_variant);

    switch (global.mtmode) {
    case MT_none:
        if (global.backend == BE_omp) {
            SBUFprint (buffer, ".mt.omp ");
        } else {
            SBUFprint (buffer, ".seq ");
        }
        break;

    case MT_createjoin: /* PThreads */
    case MT_startstop:  /* PThreads, default */
    case MT_mtstblock:  /* ?? */
        if (STReq (global.config.mt_lib, "lpel")) {
            SBUFprint (buffer, ".mt.lpel -llpel");
        } else if (STReq (global.config.mt_lib, "pthread")) {
            SBUFprint (buffer, ".mt.pth ");
        }
        break;
    }

    DBUG_RETURN ();
}

static void
AddPhmLib (str_buf *buffer)
{
    DBUG_ENTER ();

    SBUFprint (buffer, "-lsacphm");
    if (!global.optimize.dophm) {
        // Compatibility mode.
        SBUFprint (buffer, "c");
    }

    SBUFprint (buffer, global.config.lib_variant);

    if ((global.mtmode == MT_none) && (global.backend != BE_omp)) {
        SBUFprint (buffer, ".seq");
    } else {
        /* multithreaded */
        if (global.tool == TOOL_sac2c) {
            /* standalone; classical mt phm */
            SBUFprint (buffer, ".mt");
        } else {
            /* possibly externally multithreaded (xt) */
            SBUFprint (buffer, ".xt");
        }
    }

    if (global.runtimecheck.heap) {
        SBUFprint (buffer, ".diag ");
    } else {
        SBUFprint (buffer, " ");
    }

    DBUG_RETURN ();
}

static char *
GetLibs (void)
{
    str_buf *buffer;
    char *result;

    DBUG_ENTER ();

    buffer = SBUFcreate (256);

    AddPhmLib (buffer);
    AddSacLib (buffer);
    AddCCLibs (buffer);

    result = SBUF2str (buffer);
    buffer = SBUFfree (buffer);

    DBUG_RETURN (result);
}

static str_buf *
AddLibPath (const char *path, str_buf *buf)
{
    char *flag;

    DBUG_ENTER ();

    flag = STRsubstToken (global.config.ld_path, "%path%", path);
    buf = SBUFprintf (buf, "%s ", flag);
    flag = MEMfree (flag);

    DBUG_RETURN (buf);
}

static char *
GetLibPath (void)
{
    char *result;
    str_buf *buffer = SBUFcreate (255);

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    switch (kind) {
    case STRS_saclib:
        result = (char *)MEMmalloc (
          sizeof (char) * (STRlen (lib) + 6 + STRlen (global.config.lib_variant)));
        sprintf (result, "-l%sMod%s", lib, global.config.lib_variant);

        break;
    case STRS_extlib:
        result = (char *)MEMmalloc (sizeof (char) * (STRlen (lib) + 3));
        sprintf (result, "-l%s", lib);

        break;
    case STRS_objfile:
        result = (char *)MEMmalloc (sizeof (char) * (STRlen (lib) + 3));
        sprintf (result, "%s", lib);
        break;
    default:
        result = STRcpy ("");
        break;
    }

    if (rest != NULL) {
        char *temp = (char *)MEMmalloc (sizeof (char)
                                        * (STRlen ((char *)rest) + STRlen (result) + 2));

        sprintf (temp, "%s %s", (char *)rest, result);

        result = MEMfree (result);
        rest = MEMfree (rest);
        result = temp;
    }

    DBUG_RETURN (result);
}

static void
InvokeCCProg (char *cccall, char *incflags, char *linkflags, char *libs,
              stringset_t *deps)
{
    char *libpath;
    char *deplibs;

    DBUG_ENTER ();

    libpath = GetLibPath ();

/*
 * If cygwin/windows environment then build dependencies based
 * on ported PE COFF ld linker.
 */
#if IS_CYGWIN
    deplibs = CYGHgetCompleteLibString (CYGH_sac);
#else
    deplibs = (char *)STRSfold (&BuildDepLibsStringProg, deps, STRcpy (""));
#endif

    SYScall ("%s %s -o %s %s %s %s %s %s", cccall, incflags, global.outfilename,
             global.cfilename, linkflags, libpath, deplibs, libs);

    libpath = MEMfree (libpath);
    deplibs = MEMfree (deplibs);

    DBUG_RETURN ();
}

static void
CompileOneFile (const char *dir, const char *file, const char *callstring)
{
    char *basename;

    DBUG_ENTER ();

    basename = STRtok (file, ".");

    SYScall ("cd %s; %s -c %s -o %s_nonpic.o", dir, callstring, file, basename);

    basename = MEMfree (basename);

    DBUG_RETURN ();
}

static void
CompileOneFilePIC (const char *dir, const char *file, const char *callstring)
{
    char *basename;

    DBUG_ENTER ();

    basename = STRtok (file, ".");

    SYScall ("cd %s; %s %s -c %s -o %s_pic.o", dir, callstring, global.config.genpic,
             file, basename);

    basename = MEMfree (basename);

    DBUG_RETURN ();
}

static void
InvokeCCModule (char *cccall, char *compilationflags)
{
    char *str;
    char *callstring;

    DBUG_ENTER ();

    callstring = STRcat (cccall, compilationflags);

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
    MEMfree (callstring);

    char *compile_tree = STRsubstTokend (STRsubstToken (global.config.compile_tree,
                                                        "%path%", global.tmp_dirname),
                                         "%tree_cflags%", global.ccflags);

#define DO_TREE_COMPILE(NAME)                                                            \
    do {                                                                                 \
        char *tmp = STRcat (global.tmp_dirname, "/" NAME ".c");                          \
        char *compile_cmd                                                                \
          = STRsubstTokend (STRsubstToken (compile_tree, "%target%", NAME ".o"),         \
                            "%source%", tmp);                                            \
        SYScall ("%s", compile_cmd);                                                     \
        compile_cmd = MEMfree (compile_cmd);                                             \
        tmp = MEMfree (tmp);                                                             \
    } while (0)

    DO_TREE_COMPILE ("serialize");
    DO_TREE_COMPILE ("filenames");
    DO_TREE_COMPILE ("namespacemap");
    DO_TREE_COMPILE ("symboltable");
    DO_TREE_COMPILE ("dependencytable");

    MEMfree (compile_tree);

    DBUG_RETURN ();
}

static void
InvokeCCWrapper (char *cccall, char *compileflags)
{
    char *str;
    char *callstring;
    DBUG_ENTER ();

    callstring = STRcat (cccall, compileflags);

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

    MEMfree (callstring);

    DBUG_RETURN ();
}

node *
CCMinvokeCC (node *syntax_tree)
{
    char *cccall;
    char *compileflags;
    char *linkflags;
    char *libs;
    stringset_t *deps = global.dependencies;

    DBUG_ENTER ();

    if (global.gen_cccall) {
        /*
         * enable system call tracking
         */
        SYSstartTracking ();
    }

    cccall = GetCCCall ();
    compileflags = GetCompilationFlags ();
    linkflags = GetLinkingFlags ();
    libs = GetLibs ();

    if (global.filetype == FT_prog) {
        InvokeCCProg (cccall, global.config.ccincdir, linkflags, libs, deps);
    } else if (global.filetype == FT_cmod) {
        InvokeCCWrapper (cccall, compileflags);
    } else {
        InvokeCCModule (cccall, compileflags);
    }

    cccall = MEMfree (cccall);
    compileflags = MEMfree (compileflags);
    linkflags = MEMfree (linkflags);
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

    DBUG_ENTER ();

    libs = GetLibs ();
    paths = GetLibPath ();

    /*
     * Cygwin/windows specific linker flags.
     */

#if (IS_CYGWIN)
    deplibs
      = (char *)STRSfold (&CYGHbuildLibSearchString, global.dependencies, STRcpy (""));
#else
    deplibs
      = (char *)STRSfold (&BuildDepLibsStringProg, global.dependencies, STRcpy (""));
#endif

    result = STRcatn (5, paths, " ", libs, " ", deplibs);

    libs = MEMfree (libs);
    paths = MEMfree (paths);
    deplibs = MEMfree (deplibs);

    DBUG_RETURN (result);
}

#undef DBUG_PREFIX
