
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"
#include "print.h"
#include "blir.h"

node *
Blir (node *syntax_tree)
{
    node *info;

    DBUG_ENTER ("Blir");

    info = MakeInfo ();

    act_tab = blir_tab;
    syntax_tree = Trav (syntax_tree, info);

    info = FreeTree (info);

    DBUG_RETURN (syntax_tree);
}
