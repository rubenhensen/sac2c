/*
 *
 * $Log$
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
#include "Error.h"
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

static void *
BuildDepLibsStringProg (const char *lib, SStype_t kind, void *rest)
{
    char *result;
    char *libname;

    DBUG_ENTER ("BuildDepLibsStringProg");

    switch (kind) {
    case SS_saclib:
        /*
         * lookup lib<lib>.a
         */
        libname = Malloc (sizeof (char) * (strlen (lib) + 6));
        sprintf (libname, "lib%s.a", lib);
        result = FindFile (MODIMP_PATH, libname);
        libname = Free (libname);
        break;
    case SS_extlib:
        result = Malloc (sizeof (char) * (strlen (lib) + 3));
        sprintf (result, "-l%s", lib);
        break;
    default:
        result = StringCopy ("");
        break;
    }

    if (rest != NULL) {
        char *temp
          = Malloc (sizeof (char) * (strlen ((char *)rest) + strlen (result) + 2));

        sprintf (temp, "%s %s", (char *)rest, result);

        result = Free (result);
        rest = Free (rest);
        result = temp;
    }

    DBUG_RETURN (result);
}

static void
InvokeCCProg (char *cccall, char *ccflags, char *libs, stringset_t *deps)
{
    char *deplibs;

    DBUG_ENTER ("InvokeCCProg");

    deplibs = (char *)SSFold (&BuildDepLibsStringProg, deps, StringCopy (""));

    SystemCall ("%s %s -o %s %s %s %s", cccall, ccflags, outfilename, cfilename, deplibs,
                libs);

    deplibs = Free (deplibs);

    DBUG_VOID_RETURN;
}

static void
InvokeCCModule (char *cccall, char *ccflags)
{
    DBUG_ENTER ("InvokeCCModule");

    SystemCall ("cd %s; %s %s -c *.c", tmp_dirname, cccall, ccflags);

    DBUG_VOID_RETURN;
}

void
InvokeCC (stringset_t *deps)
{
    char *ccflags;
    char *cccall;
    char *libs;

    DBUG_ENTER ("InvokeCC");

    cccall = GetCCCall ();
    ccflags = GetCCFlags ();
    libs = GetLibs ();

    if (filetype == F_prog)
        InvokeCCProg (cccall, ccflags, libs, deps);
    else
        InvokeCCModule (cccall, ccflags);

    cccall = Free (cccall);
    ccflags = Free (ccflags);
    libs = Free (libs);

    DBUG_VOID_RETURN;
}
