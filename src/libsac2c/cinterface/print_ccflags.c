#include "print_ccflags.h"
#include "ccmanager.h"
#include "memory.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "str.h"
#include "globals.h"
#include "ctinfo.h"

#include <stdio.h>
#include <stdlib.h>

#define SAC2CBASEENV "SAC2CBASE"

node *
PCCFdoPrintCCFlags (node *syntax_tree)
{
    DBUG_ENTER ();

    printf ("-I%s -I%s/include", STRonNull (".", global.inc_dirname),
            STRonNull (".", getenv (SAC2CBASEENV)));

    CTIterminateCompilation (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
