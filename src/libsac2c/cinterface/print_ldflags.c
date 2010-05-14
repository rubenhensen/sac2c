/* $Id$ */

#include "print_ldflags.h"
#include "ccmanager.h"
#include "memory.h"
#include "dbug.h"
#include "str.h"
#include "globals.h"
#include "ctinfo.h"

#include <stdio.h>
#include <stdlib.h>

#define SAC2CBASEENV "SAC2CBASE"

node *
PLDFdoPrintLDFlags (node *syntax_tree)
{
    char *flags;

    DBUG_ENTER ("PLDFdoPrintLDFlags");

    flags = CCMgetLinkerFlags (syntax_tree);
    printf ("%s -l%s", flags, global.outfilename);
    flags = MEMfree (flags);

    CTIterminateCompilation (syntax_tree);

    DBUG_RETURN (syntax_tree);
}
