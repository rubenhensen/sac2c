
#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "ctinfo.h"
#include "abort.h"

node *
MTABabort (node *syntax_tree)
{
    DBUG_ENTER ();

    CTIabort (EMPTY_LOC, "MT mode 3 cannot be compiled any further!");

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
