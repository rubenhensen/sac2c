/*
 *
 * $Log$
 * Revision 1.1  2004/10/17 17:04:46  sah
 * Initial revision
 *
 *
 *
 */

#include "ccmanager.h"
#include "internal_lib.h"
#include "dbug.h"
#include "Error.h"
#include "globals.h"
#include "config.h"
#include "filemgr.h"
#include "resource.h"

static char *
GetCCCall ()
{
    str_buf *buffer;
    char *result;

    DBUG_ENTER ("GetCCCall");

    buffer = StrBufCreate (128);

    StrBufprintf (buffer, "%s %s %s %s -L%s ", config.cc, config.ccflags, config.ldflags,
                  config.ccdir, tmp_dirname);

    result = StrBuf2String (buffer);

    buffer = StrBufFree (buffer);

    DBUG_RETURN (result);
}

static void
AddOptimizeFlag (str_buf *buffer)
{
    DBUG_ENTER ("AddOptimizeFlag");

    switch (cc_optimize) {
    case 0:
        StrBufprintf (buffer, "%s ", config.opt_O0);
        break;
    case 1:
        StrBufprintf (buffer, "%s ", config.opt_O1);
        break;
    case 2:
        StrBufprintf (buffer, "%s ", config.opt_O2);
        break;
    case 3:
        StrBufprintf (buffer, "%s ", config.opt_O3);
        break;
    default:
    }

    DBUG_VOID_RETURN;
}

static void
AddDebugFlag (str_buf *buffer)
{
    DBUG_ENTER ("AddDebugFlag");

    if (cc_debug) {
        StrBufprintf (buffer, "%s ", config.opt_g);
    }

    DBUG_VOID_RETURN;
}

static char *
GetCCFlags ()
{
    str_buf *buffer;
    char *result;

    DBUG_ENTER ("GetCCFlags");

    buffer = StrBufCreate (1024);

    AddOptimizeFlag (buffer);
    AddDebugFlag (buffer);

    result = StrBuf2String (buffer);

    buffer = StrBufFree (buffer);

    DBUG_RETURN (result);
}

static void
AddCCLibs (str_buf *buffer)
{
    DBUG_ENTER ("AddCCLibs");

    StrBufprintf (buffer, "%s ", config.cclink);

    DBUG_VOID_RETURN;
}

static void
AddSacLibs (str_buf *buffer)
{
    DBUG_ENTER ("AddSacLibs");

    if (optimize & OPT_PHM) {
        if (mtmode == MT_none) {
            if (runtimecheck & RUNTIMECHECK_HEAP) {
                StrBufprint (buffer, "-lsac_heapmgr_diag ");
            } else {
                StrBufprint (buffer, "-lsac_heapmgr ");
            }
        } else {
            if (runtimecheck & RUNTIMECHECK_HEAP) {
                StrBufprint (buffer, "-lsac_heapmgr_mt_diag ");
            } else {
                StrBufprint (buffer, "-lsac_heapmgr_mt ");
            }
        }
    }

    StrBufprint (buffer, "-lsac ");

    DBUG_VOID_RETURN;
}

static void
AddEfenceLib (str_buf *buffer)
{
    DBUG_ENTER ("AddEfenceLib");

    if (use_efence) {
        char *efence;

        efence = FindFile (SYSTEMLIB_PATH, "libefence.a");
        if (efence == NULL) {
            SYSWARN (("Unable to find `libefence.a' in SYSTEMLIB_PATH"));
        } else {
            StrBufprintf (buffer, "%s ", efence);
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

    buffer = StrBufCreate (256);

    AddSacLibs (buffer);
    AddCCLibs (buffer);
    AddEfenceLib (buffer);

    result = StrBuf2String (buffer);

    buffer = StrBufFree (buffer);

    DBUG_RETURN (result);
}

static void
InvokeCCProg (char *cccall, char *ccflags, char *libs)
{
    DBUG_ENTER ("InvokeCCProg");

    SystemCall ("%s %s -o %s %s %s", cccall, ccflags, outfilename, cfilename, libs);

    DBUG_VOID_RETURN;
}

static void
InvokeCCModule (char *cccall, char *ccflags)
{
    DBUG_ENTER ("InvokeCCModule");

    SystemCall ("%s %s -c %s/*.c", cccall, ccflags, tmp_dirname);

    DBUG_VOID_RETURN;
}

void
InvokeCC ()
{
    char *ccflags;
    char *cccall;
    char *libs;

    DBUG_ENTER ("InvokeCC");

    cccall = GetCCCall ();
    ccflags = GetCCFlags ();
    libs = GetLibs ();

    if (filetype == F_prog)
        InvokeCCProg (cccall, ccflags, libs);
    else
        InvokeCCModule (cccall, ccflags);

    cccall = Free (cccall);
    ccflags = Free (ccflags);
    libs = Free (libs);

    DBUG_VOID_RETURN;
}
