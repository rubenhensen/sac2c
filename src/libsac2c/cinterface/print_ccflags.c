#include "print_ccflags.h"
#include "memory.h"

#define DBUG_PREFIX "PCCF"
#include "debug.h"

#include "str.h"
#include "globals.h"
#include "ctinfo.h"

#include <stdio.h>

node *
PCCFdoPrintCCFlags (node *syntax_tree)
{
    DBUG_ENTER ();

    printf ("%s -I%s", global.config.sacincludes, STRonNull (".", global.inc_dirname));

    CTIterminateCompilation (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
