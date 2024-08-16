/******************************************************************************
 *
 * By enabling -ecc or -check c, constraints for type pattens are inserted into
 * the program. To ensure that these constraints are not incorrectly optimised
 * away we insert a guard function that works as a barrier for optimisation.
 *
 * This approach might hinder optimisation, especially if not all constraints
 * could be resolved statically, in this traversal we inform the user about
 * constraints that could not be resolved statically, so that they can decide
 * whether they want to disable -ecc or -check c before running the program.
 *
 ******************************************************************************/
#include "ctinfo.h"
#include "globals.h"
#include "memory.h"
#include "new_types.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"

#define DBUG_PREFIX "TPS"
#include "debug.h"

#include "type_pattern_statistics.h"

struct INFO {
    bool all_gone;
};

#define INFO_ALLGONE(n) ((n)->all_gone)

static info *
MakeInfo (void)
{
    info *res;

    DBUG_ENTER ();

    res = (info *)MEMmalloc (sizeof (info));

    INFO_ALLGONE (res) = TRUE;

    DBUG_RETURN (res);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * @fn node *TPSdoPrintTypePatternStatistics (node *arg_node)
 *
 * @brief Print all type pattern constraints that could not be statically
 * resolved.
 *
 ******************************************************************************/
node *
TPSdoPrintTypePatternStatistics (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "called with non-module node");

    arg_info = MakeInfo ();

    TRAVpush (TR_tps);

    CTItell (0, "*********************************************************************");
    CTItell (0, "** Type pattern resolution feedback                                **");
    CTItell (0, "*********************************************************************");

    if (!global.runtimecheck.conformity) {
        CTItell (0, "  No constraints to resolve as none have been injected.");
        CTItell (0, "  Use -ecc and -check c to turn constraint injection on.");
    } else {
        CTItell (0, "  The following constraints could not be statically resolved:");
        arg_node = TRAVdo (arg_node, arg_info);
        if (INFO_ALLGONE (arg_info)) {
            CTItell (0, "  All constraints were statically resolved.");
        }
    }

    CTItell (0, "*********************************************************************");

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *TPSprf (node *arg_node, info *arg_info)
 *
 * @brief If we encounter a type pattern error at this point, we know that we
 * were not able to statically resolve whether it is true of false. Print the
 * error message of this type pattern error.
 *
 ******************************************************************************/
node *
TPSprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_conditional_error:
        INFO_ALLGONE (arg_info) = FALSE;
        char *err = STR_STRING (PRF_ARG3 (arg_node));
        CTItell (0, "    %s", err);
        break;

    default:
        // Nothing to do
        break;
    }

    DBUG_RETURN (arg_node);
}
