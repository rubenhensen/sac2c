/*
 *
 * $Id$
 *
 */

#include "ccmanager.h"
#include "internal_lib.h"
#include "dbug.h"
#include "ctinfo.h"
#include "globals.h"
#include "config.h"
#include "filemgr.h"
#include "resource.h"
#include "stringset.h"
#include "filemgr.h"

#include <string.h>

static char *
GetCCCall ()
{
    str_buf *buffer;
    char *result;

    DBUG_ENTER ("GetCCCall");

    buffer = ILIBstrBufCreate (128);

    ILIBstrBufPrintf (buffer, "%s %s %s %s -L%s ", global.config.cc,
                      global.config.ccflags, global.config.ldflags, global.config.ccdir,
                      global.tmp_dirname);

    result = ILIBstrBuf2String (buffer);

    buffer = ILIBstrBufFree (buffer);

    DBUG_RETURN (result);
}

static void
AddOptimizeFlag (str_buf *buffer)
{
    DBUG_ENTER ("AddOptimizeFlag");

    switch (global.cc_optimize) {
    case 0:
        ILIBstrBufPrintf (buffer, "%s ", global.config.opt_O0);
        break;
    case 1:
        ILIBstrBufPrintf (buffer, "%s ", global.config.opt_O1);
        break;
    case 2:
        ILIBstrBufPrintf (buffer, "%s ", global.config.opt_O2);
        break;
    case 3:
        ILIBstrBufPrintf (buffer, "%s ", global.config.opt_O3);
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
        ILIBstrBufPrintf (buffer, "%s ", global.config.opt_g);
    }

    DBUG_VOID_RETURN;
}

static char *
GetCCFlags ()
{
    str_buf *buffer;
    char *result;

    DBUG_ENTER ("GetCCFlags");

    buffer = ILIBstrBufCreate (1024);

    AddOptimizeFlag (buffer);
    AddDebugFlag (buffer);

    result = ILIBstrBuf2String (buffer);

    buffer = ILIBstrBufFree (buffer);

    DBUG_RETURN (result);
}

static void
AddCCLibs (str_buf *buffer)
{
    DBUG_ENTER ("AddCCLibs");

    ILIBstrBufPrintf (buffer, "%s ", global.config.cclink);

    DBUG_VOID_RETURN;
}

static void
AddSacLibs (str_buf *buffer)
{
    DBUG_ENTER ("AddSacLibs");

    if (global.optimize.dophm) {
        if (global.mtmode == MT_none) {
            if (global.runtimecheck.heap) {
                ILIBstrBufPrint (buffer, "-lsacphm_diag ");
            } else {
                ILIBstrBufPrint (buffer, "-lsacphm ");
            }
        } else {
            if (global.runtimecheck.heap) {
                ILIBstrBufPrint (buffer, "-lsacphm_mt_diag ");
            } else {
                ILIBstrBufPrint (buffer, "-lsacphm_mt ");
            }
        }
    }

    if (global.mtmode == MT_none) {
        ILIBstrBufPrint (buffer, "-lsac ");
    } else {
        ILIBstrBufPrint (buffer, "-lsac_mt -lpthread");
    }

    DBUG_VOID_RETURN;
}

static void
AddEfenceLib (str_buf *buffer)
{
    DBUG_ENTER ("AddEfenceLib");

    if (global.use_efence) {
        char *efence;

        efence = ILIBstringCopy (FMGRfindFile (PK_extlib_path, "libefence.a"));

        if (efence == NULL) {
            CTIwarn ("Unable to find `libefence.a' in EXTLIB_PATH");
        } else {
            ILIBstrBufPrintf (buffer, "%s ", efence);
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

    buffer = ILIBstrBufCreate (256);

    AddSacLibs (buffer);
    AddCCLibs (buffer);
    AddEfenceLib (buffer);

    result = ILIBstrBuf2String (buffer);

    buffer = ILIBstrBufFree (buffer);

    DBUG_RETURN (result);
}

static str_buf *
AddExtLib (const char *path, str_buf *buf)
{
    DBUG_ENTER ("AddExtLibPath");

    buf = ILIBstrBufPrintf (buf, "-L%s %s%s ", path, global.config.ld_path, path);

    DBUG_RETURN (buf);
}

static char *
GetExtLibPath ()
{
    char *result;
    str_buf *buffer = ILIBstrBufCreate (255);

    DBUG_ENTER ("GetExtLibPath");

    buffer = (str_buf *)FMGRmapPath (PK_extlib_path,
                                     (void *(*)(const char *, void *))AddExtLib, buffer);

    result = ILIBstrBuf2String (buffer);
    buffer = ILIBstrBufFree (buffer);

    DBUG_RETURN (result);
}

static void *
BuildDepLibsStringProg (const char *lib, strstype_t kind, void *rest)
{
    char *result;
    char *libname;

    DBUG_ENTER ("BuildDepLibsStringProg");

    switch (kind) {
    case STRS_saclib:
        /*
         * lookup lib<lib>.a
         */
        libname = ILIBmalloc (sizeof (char) * (strlen (lib) + 6));
        sprintf (libname, "lib%s.a", lib);
        result = ILIBstringCopy (FMGRfindFile (PK_lib_path, libname));

        if (result == NULL) {
            CTIabort ("Cannot find static library '%s'. The module '%s' "
                      "seems to be corrupted.",
                      libname, lib);
        }

        libname = ILIBfree (libname);
        break;
    case STRS_extlib:
        result = ILIBmalloc (sizeof (char) * (strlen (lib) + 3));
        sprintf (result, "-l%s", lib);

        break;
    case STRS_objfile:
        result = ILIBmalloc (sizeof (char) * (strlen (lib) + 3));
        sprintf (result, "%s", lib);
        break;
    default:
        result = ILIBstringCopy ("");
        break;
    }

    if (rest != NULL) {
        char *temp
          = ILIBmalloc (sizeof (char) * (strlen ((char *)rest) + strlen (result) + 2));

        sprintf (temp, "%s %s", (char *)rest, result);

        result = ILIBfree (result);
        rest = ILIBfree (rest);
        result = temp;
    }

    DBUG_RETURN (result);
}

static void
InvokeCCProg (char *cccall, char *ccflags, char *libs, stringset_t *deps)
{
    char *extpath;
    char *deplibs;

    DBUG_ENTER ("InvokeCCProg");

    extpath = GetExtLibPath ();

    deplibs = (char *)STRSfold (&BuildDepLibsStringProg, deps, ILIBstringCopy (""));

    ILIBsystemCall ("%s %s -o %s %s %s %s %s", cccall, ccflags, global.outfilename,
                    global.cfilename, extpath, deplibs, libs);

    extpath = ILIBfree (extpath);
    deplibs = ILIBfree (deplibs);

    DBUG_VOID_RETURN;
}

static void
InvokeCCModule (char *cccall, char *ccflags)
{
    DBUG_ENTER ("InvokeCCModule");

    ILIBsystemCall ("cd %s; %s %s -c fun*.c globals.c", global.tmp_dirname, cccall,
                    ccflags);

    ILIBsystemCall ("cd %s; %s -c serialize.c", global.tmp_dirname, cccall);
    ILIBsystemCall ("cd %s; %s -c filenames.c", global.tmp_dirname, cccall);
    ILIBsystemCall ("cd %s; %s -c namespacemap.c", global.tmp_dirname, cccall);
    ILIBsystemCall ("cd %s; %s -c symboltable.c", global.tmp_dirname, cccall);
    ILIBsystemCall ("cd %s; %s -c dependencytable.c", global.tmp_dirname, cccall);

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
        ILIBsystemCallStartTracking ();
    }

    cccall = GetCCCall ();
    ccflags = GetCCFlags ();
    libs = GetLibs ();

    if (global.filetype == F_prog) {
        InvokeCCProg (cccall, ccflags, libs, deps);
    } else {
        InvokeCCModule (cccall, ccflags);
    }

    cccall = ILIBfree (cccall);
    ccflags = ILIBfree (ccflags);
    libs = ILIBfree (libs);

    if (global.gen_cccall) {
        /*
         * stop tracking and close file
         */
        ILIBsystemCallStopTracking ();
    }

    DBUG_RETURN (syntax_tree);
}
