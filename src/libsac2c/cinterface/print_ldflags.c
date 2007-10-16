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
    printf ("-L%s/lib/ %s%s/lib/ -L%s %s%s -l%s %s",
            STRonNull (".", getenv (SAC2CBASEENV)), global.config.ld_path,
            STRonNull (".", getenv (SAC2CBASEENV)), STRonNull (".", global.lib_dirname),
            global.config.ld_path, STRonNull (".", global.lib_dirname),
            global.outfilename, flags);
    flags = MEMfree (flags);

    CTIterminateCompilation (syntax_tree);

    DBUG_RETURN (syntax_tree);
}
