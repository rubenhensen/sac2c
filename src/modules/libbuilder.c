/*
 *
 * $Id$
 *
 */

#include "libbuilder.h"
#include "dbug.h"
#include "ctinfo.h"
#include "internal_lib.h"
#include "stringset.h"
#include "resource.h"
#include "filemgr.h"

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

node *
LIBBcreateLibrary (node *syntax_tree)
{
    char *deplibs;
    stringset_t *deps = global.dependencies;

    DBUG_ENTER ("LIBBcreateLibrary");

    CTInote ("Creating static SAC library `lib%s.a'", global.modulename);

    if (global.gen_cccall) {
        /*
         * enable system call tracking
         */
        ILIBsystemCallStartTracking ();
    }

    deplibs = STRSfold (&BuildDepLibsStringMod, deps, ILIBstringCopy (""));

    ILIBsystemCall ("%s %slib%s.a %s/fun*.o %s/globals.o %s", global.config.ar_create,
                    global.targetdir, global.modulename, global.tmp_dirname,
                    global.tmp_dirname, deplibs);

    if (global.config.ranlib[0] != '\0') {
        ILIBsystemCall ("%s %slib%s.a", global.config.ranlib, global.targetdir,
                        global.modulename);
    }

    deplibs = ILIBfree (deplibs);

    CTInote ("Creating shared SAC library `lib%s.so'", global.modulename);

    ILIBsystemCall ("%s -o %slib%s.so %s/serialize.o %s/symboltable.o"
                    " %s/dependencytable.o %s/namespacemap.o %s/filenames.o",
                    global.config.ld_dynamic, global.targetdir, global.modulename,
                    global.tmp_dirname, global.tmp_dirname, global.tmp_dirname,
                    global.tmp_dirname, global.tmp_dirname);

    if (global.gen_cccall) {
        /*
         * stop tracking and close file
         */
        ILIBsystemCallStopTracking ();
    }

    DBUG_RETURN (syntax_tree);
}
