/*****************************************************************************
 *
 * $Id$
 *
 * file:   setlinksign.c
 *
 * prefix: SLS
 *
 * description:
 *
 *   The linksign pragma supports a wide range of reorganisations of a
 *   function prototype for external functions, like reordering of return
 *   values and parameters or mapping of returns to parameters leading to
 *   destructive updates (in the C world). In order to have a single uniform
 *   code generation backend for all SAC and C functions the actual code
 *   generation is uniformly based on indices attached to each ret and arg
 *   node that specify the final location of the ret or arg in the compiled
 *   C function signature. These indices may have been derived already from
 *   the before mentioned linksign pragma. However, for all SAC functions and
 *   those external functions without a linksign pragma appropriate indices
 *   are annotated in this compiler phase.
 *
 *   A note on the HASLINKSIGN flag:
 *   Until here this flag is used to mark whether or not the numerical index
 *   does indeed express linksign information. From this phase on, the link
 *   sign indices are always properly set. Hence, there is no immediate need
 *   to set the HASLINKSIGN flag as well.
 *
 *****************************************************************************/

#include "setlinksign.h"

#include "tree_basic.h"
#include "traverse.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "str.h"
#include "memory.h"

/*
 * INFO structure
 */

struct INFO {
    int counter;
};

/*
 * INFO macros
 */

#define INFO_COUNTER(n) ((n)->counter)

/*
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_COUNTER (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * Traversal functions
 */

node *
SLSret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_COUNTER (arg_info)++;

    if (!RET_HASLINKSIGNINFO (arg_node)) {
        RET_LINKSIGN (arg_node) = INFO_COUNTER (arg_info);
    }

    RET_NEXT (arg_node) = TRAVopt (RET_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SLSarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_COUNTER (arg_info)++;

    if (!ARG_HASLINKSIGNINFO (arg_node)) {
        ARG_LINKSIGN (arg_node) = INFO_COUNTER (arg_info);
    }

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SLSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_COUNTER (arg_info) = 0;

    FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SLSmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNDECS (arg_node) = TRAVopt (MODULE_FUNDECS (arg_node), arg_info);
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);
    MODULE_THREADFUNS (arg_node) = TRAVopt (MODULE_THREADFUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/*
 * Traversal start function
 */

node *
SLSdoSetLinksign (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_sls);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
