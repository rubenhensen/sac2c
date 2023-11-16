#include "print_ldflags.h"
#include "cctools.h"
#include "memory.h"

#define DBUG_PREFIX "PLDF"
#include "debug.h"

#include "str.h"
#include "globals.h"
#include "ctinfo.h"

#include <stdio.h>
#include <stdlib.h>

node *
PLDFdoPrintLDFlags (node *syntax_tree)
{
    char *flags;

    DBUG_ENTER ();

    flags = CCTreturnLinkFlags ();
    printf ("%s -l%s", flags, global.outfilename);
    flags = MEMfree (flags);

    CTIterminateCompilation (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
