/*
 *
 * $Log$
 * Revision 1.7  2005/04/12 15:15:36  sah
 * cleaned up module system compiler args
 * and sac2crc parameters
 *
 * Revision 1.6  2005/02/18 10:37:23  sah
 * module system fixes
 *
 * Revision 1.5  2005/01/11 12:32:52  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 1.4  2004/11/24 18:10:32  sah
 * COMPILES
 *
 * Revision 1.3  2004/11/07 18:05:01  sah
 * improved dependency handling
 * for external function added
 *
 * Revision 1.2  2004/10/28 17:21:24  sah
 * the dependencytable is compiled and linked
 * now as well
 *
 * Revision 1.1  2004/10/17 17:47:33  sah
 * Initial revision
 *
 *
 *
 */

#include "libbuilder.h"
#include "dbug.h"
#include "ctinfo.h"
#include "internal_lib.h"
#include "stringset.h"
#include "resource.h"

#include <string.h>

static void *
BuildDepLibsStringMod (const char *lib, strstype_t kind, void *rest)
{
    char *result;

    DBUG_ENTER ("BuildDepLibsStringMod");

    switch (kind) {
    case STRS_objfile:
        result = ILIBstringCopy (lib);
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

void
LIBBcreateLibrary (stringset_t *deps)
{
    char *deplibs;

    DBUG_ENTER ("LIBBcreateLibrary");

    CTInote ("Creating static SAC library `lib%s.a'", global.modulename);

    deplibs = STRSfold (&BuildDepLibsStringMod, deps, ILIBstringCopy (""));

    ILIBsystemCall ("%s lib%s.a %s/fun*.o %s", global.config.ar_create, global.modulename,
                    global.tmp_dirname, deplibs);

    if (global.config.ranlib[0] != '\0') {
        ILIBsystemCall ("%s lib%s.a", global.config.ranlib, global.modulename);
    }

    deplibs = ILIBfree (deplibs);

    CTInote ("Creating shared SAC library `lib%s.so'", global.modulename);

    ILIBsystemCall ("%s -o lib%s.so %s/serialize.o %s/symboltable.o"
                    " %s/dependencytable.o",
                    global.config.ld_dynamic, global.modulename, global.tmp_dirname,
                    global.tmp_dirname, global.tmp_dirname);

    DBUG_VOID_RETURN;
}
