
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
#include "lac2fun.h"
#include "fun2lac.h"
#include "SSATransform.h"
#include "UndoSSATransform.h"
#include "CheckAvis.h"

node *
Blir (node *syntax_tree)
{
    node *info;

    DBUG_ENTER ("Blir");

    info = MakeInfo ();

    /* Set traversal table */
    act_tab = blir_tab;

    /* syntax tree must be in SSA form */
    syntax_tree = Lac2Fun (syntax_tree);
    syntax_tree = CheckAvis (syntax_tree);
    syntax_tree = SSATransform (syntax_tree);

    /* perform BLIR */
    syntax_tree = Trav (syntax_tree, info);

    /* free info node */
    info = FreeTree (info);

    /* revert syntax tree into LaC form */
    syntax_tree = UndoSSATransform (syntax_tree);
    syntax_tree = Fun2Lac (syntax_tree);

    /* return the result */
    DBUG_RETURN (syntax_tree);
}
