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

    SBUFprintf (buffer, "%s %s %s %s -L%s ", global.config.cc, global.config.ccflags,
                global.config.ldflags, global.config.ccdir, global.tmp_dirname);

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

    DBUG_VOID_RETURN;
}

static void
AddSacLibs (str_buf *buffer)
{
    DBUG_ENTER ("AddSacLibs");

    if (global.optimize.dophm) {
        if (global.mtmode == MT_none) {
            if (global.runtimecheck.heap) {
                SBUFprint (buffer, "-lsacphm_seq_diag ");
            } else {
                SBUFprint (buffer, "-lsacphm_seq ");
            }
        } else {
            if (global.runtimecheck.heap) {
                SBUFprint (buffer, "-lsacphm_mt_diag ");
            } else {
                SBUFprint (buffer, "-lsacphm_mt ");
            }
        }
    }

    if (global.mtmode == MT_none) {
        SBUFprint (buffer, "-lsac_seq ");
    } else {
        SBUFprint (buffer, "-lsac_mt -lpthread");
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

    AddSacLibs (buffer);
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
        result = MEMmalloc (sizeof (char) * (STRlen (lib) + 6));
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

    DBUG_ENTER ("InvokeCCModule");

    callstring = STRcat (cccall, ccflags);

    /*
     * compile non-PIC code
     */
    FMGRforEach (global.tmp_dirname, "fun.*\\.c", callstring,
                 (void (*) (const char *, const char *, void *))CompileOneFile);
    CompileOneFile (global.tmp_dirname, "globals.c", callstring);

    if (!STReq (global.config.ld_dynamic, "")) {
        /*
         * compile PIC code for shared libs if needed
         */
        FMGRforEach (global.tmp_dirname, "fun.*\\.c", callstring,
                     (void (*) (const char *, const char *, void *))CompileOneFilePIC);
        CompileOneFilePIC (global.tmp_dirname, "globals.c", callstring);
    }

    SYScall ("cd %s; %s %s -c serialize.c", global.tmp_dirname, global.config.tree_cc,
             global.config.ccdir);
    SYScall ("cd %s; %s %s -c filenames.c", global.tmp_dirname, global.config.tree_cc,
             global.config.ccdir);
    SYScall ("cd %s; %s %s -c namespacemap.c", global.tmp_dirname, global.config.tree_cc,
             global.config.ccdir);
    SYScall ("cd %s; %s %s -c symboltable.c", global.tmp_dirname, global.config.tree_cc,
             global.config.ccdir);
    SYScall ("cd %s; %s %s -c dependencytable.c", global.tmp_dirname,
             global.config.tree_cc, global.config.ccdir);

    callstring = MEMfree (callstring);

    DBUG_VOID_RETURN;
}

static void
InvokeCCWrapper (char *cccall, char *ccflags)
{
    char *callstring;

    DBUG_ENTER ("InvokeCCWrapper");

    callstring = STRcat (cccall, ccflags);

    /*
     * compile non-PIC code
     */
    FMGRforEach (global.tmp_dirname, "fun.*\\.c", callstring,
                 (void (*) (const char *, const char *, void *))CompileOneFile);
    CompileOneFile (global.tmp_dirname, "globals.c", callstring);
    CompileOneFile (global.tmp_dirname, "interface.c", callstring);
    CompileOneFile (global.tmp_dirname, "sacargcopy.c", callstring);
    CompileOneFile (global.tmp_dirname, "sacargfree.c", callstring);

    /*
     * compile PIC code
     */
    FMGRforEach (global.tmp_dirname, "fun.*\\.c", callstring,
                 (void (*) (const char *, const char *, void *))CompileOneFilePIC);
    CompileOneFilePIC (global.tmp_dirname, "globals.c", callstring);
    CompileOneFilePIC (global.tmp_dirname, "interface.c", callstring);
    CompileOneFilePIC (global.tmp_dirname, "sacargcopy.c", callstring);
    CompileOneFilePIC (global.tmp_dirname, "sacargfree.c", callstring);

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

    if (global.filetype == F_prog) {
        InvokeCCProg (cccall, ccflags, libs, deps);
    } else if (global.filetype == F_cmod) {
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
