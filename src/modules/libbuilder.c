/*
 *
 * $Log$
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
#include "Error.h"
#include "internal_lib.h"
#include "resource.h"

static void *
BuildDepLibsStringMod (const char *lib, SStype_t kind, void *rest)
{
    char *result;

    DBUG_ENTER ("BuildDepLibsStringMod");

    switch (kind) {
    case SS_objfile:
        result = StringCopy (lib);
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

void
CreateLibrary (stringset_t *deps)
{
    char *deplibs;

    DBUG_ENTER ("CreateLibrary");

    NOTE (("Creating static SAC library `lib%s.a'", modulename));

    deplibs = SSFold (&BuildDepLibsStringMod, deps, StringCopy (""));

    SystemCall ("%s lib%s.a %s/fun*.o %s", config.ar_create, modulename, tmp_dirname,
                deplibs);

    if (config.ranlib[0] != '\0') {
        SystemCall ("%s lib%s.a", config.ranlib, modulename);
    }

    deplibs = Free (deplibs);

    NOTE (("Creating shared SAC library `lib%s.so'", modulename));

    SystemCall ("%s -o lib%s.so %s/serialize.o %s/symboltable.o"
                " %s/dependencytable.o",
                config.ld_dynamic, modulename, tmp_dirname, tmp_dirname, tmp_dirname);

    DBUG_VOID_RETURN;
}
