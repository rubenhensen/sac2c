/*
 *
 * $Log$
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

void
CreateLibrary ()
{
    DBUG_ENTER ("CreateLibrary");

    NOTE (("Creating static SAC library `lib%s.a'", modulename));

    SystemCall ("%s lib%s.a %s/fun*.o", config.ar_create, modulename, tmp_dirname);

    if (config.ranlib[0] != '\0') {
        SystemCall ("%s lib%s.a", config.ranlib, modulename);
    }

    NOTE (("Creating shared SAC library `lib%s.so'", modulename));

    SystemCall ("%s -o lib%s.so %s/serialize.o %s/symboltable.o", config.ld_dynamic,
                modulename, tmp_dirname, tmp_dirname);

    DBUG_VOID_RETURN;
}
