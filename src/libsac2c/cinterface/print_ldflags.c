/* $Id$ */

#include "print_ldflags.h"
#include "ccmanager.h"
#include "memory.h"
#include "dbug.h"
#include "ctinfo.h"

#include <stdio.h>

node *
PLDFdoPrintLDFlags (node *syntax_tree)
{
    char *flags;

    DBUG_ENTER ("PLDFdoPrintLDFlags");

    flags = CCMgetLinkerFlags (syntax_tree);
    printf (flags);
    flags = MEMfree (flags);

    CTIterminateCompilation (syntax_tree);

    DBUG_RETURN (syntax_tree);
}
