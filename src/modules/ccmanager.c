/*
 *
 * $Log$
 * Revision 1.8  2005/01/11 12:32:52  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 1.7  2004/11/24 18:56:18  sah
 * COMPILES
 *
 * Revision 1.6  2004/11/09 01:14:55  sah
 * added a break in default case
 *
 * Revision 1.5  2004/11/07 18:05:01  sah
 * improved dependency handling
 * for external function added
 *
 * Revision 1.4  2004/11/02 12:15:21  sah
 * empty dependencies are handeled correctly now
 *
 * Revision 1.3  2004/10/28 17:18:58  sah
 * added handling of dependencies
 *
 * Revision 1.2  2004/10/17 17:49:19  sah
 * fixed shell command
 *
 * Revision 1.1  2004/10/17 17:04:46  sah
 * Initial revision
 *
 *
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
                ILIBstrBufPrint (buffer, "-lsac_heapmgr_diag ");
            } else {
                ILIBstrBufPrint (buffer, "-lsac_heapmgr ");
            }
        } else {
            if (global.runtimecheck.heap) {
                ILIBstrBufPrint (buffer, "-lsac_heapmgr_mt_diag ");
            } else {
                ILIBstrBufPrint (buffer, "-lsac_heapmgr_mt ");
            }
        }
    }

    ILIBstrBufPrint (buffer, "-lsac ");

    DBUG_VOID_RETURN;
}

static void
AddEfenceLib (str_buf *buffer)
{
    DBUG_ENTER ("AddEfenceLib");

    if (global.use_efence) {
        char *efence;

        efence = FMGRfindFile (PK_systemlib_path, "libefence.a");
        if (efence == NULL) {
            CTIwarn ("Unable to find `libefence.a' in SYSTEMLIB_PATH");
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
        result = FMGRfindFile (PK_modimp_path, libname);
        libname = ILIBfree (libname);
        break;
    case STRS_extlib:
        result = ILIBmalloc (sizeof (char) * (strlen (lib) + 3));
        sprintf (result, "-l%s", lib);
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
    char *deplibs;

    DBUG_ENTER ("InvokeCCProg");

    deplibs = (char *)STRSfold (&BuildDepLibsStringProg, deps, ILIBstringCopy (""));

    ILIBsystemCall ("%s %s -o %s %s %s %s", cccall, ccflags, global.outfilename,
                    global.cfilename, deplibs, libs);

    deplibs = ILIBfree (deplibs);

    DBUG_VOID_RETURN;
}

static void
InvokeCCModule (char *cccall, char *ccflags)
{
    DBUG_ENTER ("InvokeCCModule");

    ILIBsystemCall ("cd %s; %s %s -c *.c", global.tmp_dirname, cccall, ccflags);

    DBUG_VOID_RETURN;
}

void
CCMinvokeCC (stringset_t *deps)
{
    char *ccflags;
    char *cccall;
    char *libs;

    DBUG_ENTER ("CCMinvokeCC");

    cccall = GetCCCall ();
    ccflags = GetCCFlags ();
    libs = GetLibs ();

    if (global.filetype == F_prog)
        InvokeCCProg (cccall, ccflags, libs, deps);
    else
        InvokeCCModule (cccall, ccflags);

    cccall = ILIBfree (cccall);
    ccflags = ILIBfree (ccflags);
    libs = ILIBfree (libs);

    DBUG_VOID_RETURN;
}
