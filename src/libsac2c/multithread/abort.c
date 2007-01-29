#include "dbug.h"
#include "ctinfo.h"
#include "abort.h"

node *
MTABabort (node *syntax_tree)
{
    DBUG_ENTER ("MTABabort");

    CTIabort ("MT mode 3 cannot be compiled any further!");

    DBUG_RETURN (syntax_tree);
}
