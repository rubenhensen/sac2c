/******************************************************************************
 *
 * Type patterns allow arbitrary conditions to be defined after the function
 * signature. E.g.:
 *
 * int[n,m] foo(int[n,n] a)
 *   | n > 1, foo(n), bar(n,m)
 *
 * As can be seen in this example, these expressions can contain features
 * defined by arguments, return values, or both. The N_fundef node contains an
 * N_exprs chain PreConds that in the parsing phase is filled with all of these
 * conditions.
 *
 * Some conditions might rely on features that are defined by return types,
 * such as bar(n, m). These conditions can only be executed in the post-check
 * function, and should thus be moved to the PostConds N_exprs chain of the
 * function instead.
 *
 * To do so we first traverse the arguments and return types of a function to
 * find whether features are defined by an argument. We then traverse each
 * condition. If all identifiers used in the conditions are defined in the
 * arguments, we leave it in the PreConds. If at least one identifier is only
 * defined in the return types, we instead move it to the PostConds chain.
 *
 * Additionally, we check if that identifier is actually defined in one of the
 * return type's type patterns. If not, we give the user a descriptive error.
 * Whilst this error will also be caught by the type checker, checking for it
 * here allows us to give the user a more descriptive error message.
 *
 ******************************************************************************/
#include "ctinfo.h"
#include "DupTree.h"
#include "free.h"
#include "memory.h"
#include "print.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "type_pattern_constraints.h"

#define DBUG_PREFIX "FFC"
#include "debug.h"

#include "filter_fundef_conditions.h"

typedef enum {
    FFC_args,
    FFC_rets,
    FFC_search
} trav_mode;

struct INFO {
    trav_mode mode;
    node *in_arg;
    node *in_ret;
    bool all_found;
};

#define INFO_MODE(n) ((n)->mode)
#define INFO_INARG(n) ((n)->in_arg)
#define INFO_INRET(n) ((n)->in_ret)
#define INFO_ALLFOUND(n) ((n)->all_found)

static info *
MakeInfo (void)
{
    info *res;

    DBUG_ENTER ();

    res = (info *)MEMmalloc (sizeof (info));

    INFO_MODE (res) = FFC_args;
    INFO_INARG (res) = NULL;
    INFO_INRET (res) = NULL;
    INFO_ALLFOUND (res) = FALSE;

    DBUG_RETURN (res);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static bool
IsPreCondition (node *condition, info *arg_info)
{
    DBUG_ENTER ();

    INFO_ALLFOUND (arg_info) = TRUE;

    DBUG_PRINT ("filtering condition");
    DBUG_EXECUTE (PRTdoPrintNode (EXPRS_EXPR (condition)));
    EXPRS_EXPR (condition) = TRAVdo (EXPRS_EXPR (condition), arg_info);

    DBUG_RETURN (INFO_ALLFOUND (arg_info));
}

node *
FFCdoFilterFundefConditions (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "called with non-module node");

    arg_info = MakeInfo ();

    TRAVpush (TR_ffc);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    CTIabortOnError ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *FFCfundef (node *arg_node, info *arg_info)
 *
 * @brief Collect all identifiers that are defined as arguments, or are defined
 * in an argument's type pattern.
 *
 * For each constraint:
 *   for each identifier in the constraint:
 *     if that identifier is not in the list of defined identifiers,
 *     move this constraint from arg to ret
 *
 ******************************************************************************/
node *
FFCfundef (node *arg_node, info *arg_info)
{
    node *condition, *next;

    DBUG_ENTER ();

    if (FUNDEF_PRECONDS (arg_node) != NULL) {
        DBUG_PRINT ("----- filtering fundef conditions of %s -----",
                    FUNDEF_NAME (arg_node));

        // Collect identifiers that are defined by args or their type patterns
        INFO_MODE (arg_info) = FFC_args;
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

        // Collect identifiers that are defined by type patterns of rets
        INFO_MODE (arg_info) = FFC_rets;
        FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);

        INFO_MODE (arg_info) = FFC_search;

        condition = FUNDEF_PRECONDS (arg_node);
        FUNDEF_PRECONDS (arg_node) = NULL;
        next = condition;

        while (next != NULL) {
            condition = next;
            next = EXPRS_NEXT (next);
            EXPRS_NEXT (condition) = NULL;

            if (IsPreCondition (condition, arg_info)) {
                DBUG_PRINT ("condition operates only on arguments");
                FUNDEF_PRECONDS (arg_node) =
                    TCappendExprs (FUNDEF_PRECONDS (arg_node), condition);
            } else {
                DBUG_PRINT ("condition operates on return values");
                FUNDEF_POSTCONDS (arg_node) =
                    TCappendExprs (FUNDEF_POSTCONDS (arg_node), condition);
            }
        }

        INFO_INARG (arg_info) = FREEoptFreeTree (INFO_INARG (arg_info));
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *FFCret (node *arg_node, info *arg_info)
 *
 * @brief Add features of return types to INFO_INRET.
 *
 ******************************************************************************/
node *
FFCret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (RET_TYPEPATTERN (arg_node) != NULL) {
        DBUG_PRINT ("collecting definitions of ret");
        RET_TYPEPATTERN (arg_node) = TRAVdo (RET_TYPEPATTERN (arg_node),
                                             arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *FFCarg (node *arg_node, info *arg_info)
 *
 * @brief Add this argument, and its features to INFO_INARG.
 *
 ******************************************************************************/
node *
FFCarg (node *arg_node, info *arg_info)
{
    char *id;
    node *avis;

    DBUG_ENTER ();

    id = ARG_NAME (arg_node);
    if (TPCtryAddSpid (&INFO_INARG (arg_info), id)) {
        DBUG_PRINT ("marked argument %s as defined", id);
    }

    avis = ARG_AVIS (arg_node);
    if (AVIS_TYPEPATTERN (avis) != NULL) {
        DBUG_PRINT ("collecting definitions of argument %s", id);
        AVIS_TYPEPATTERN (avis) = TRAVdo (AVIS_TYPEPATTERN (avis), arg_info);
    }

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *FFCspap (node *arg_node, info *arg_info)
 *
 * @brief Ensure we only traverse the arguments of the application,
 *        and not the SPAP_ID.
 *
 ******************************************************************************/
node *
FFCspap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    SPAP_ARGS (arg_node) = TRAVopt (SPAP_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *FFCspid (node *arg_node, info *arg_info)
 *
 * @brief We traverse N_spids three times.
 * - Once to add features of arguments to INFO_INARG.
 * - Once to add features of return types to INFO_INRET.
 * - And finally to check whether this N_spid is in the INFO_INARG or INFO_INRET
 *   chain.
 *
 ******************************************************************************/
node *
FFCspid (node *arg_node, info *arg_info)
{
    char *id;

    DBUG_ENTER ();

    id = SPID_NAME (arg_node);

    switch (INFO_MODE (arg_info))
    {
    case FFC_args:
        if (TPCtryAddSpid (&INFO_INARG (arg_info), id)) {
            DBUG_PRINT ("%s is defined in an argument", id);
        }
        break;

    case FFC_rets:
        if (TPCtryAddSpid (&INFO_INRET (arg_info), id)) {
            DBUG_PRINT ("%s is defined in a ret", id);
        }
        break;

    case FFC_search:
        if (!TCspidsContains (INFO_INARG (arg_info), id)) {
            DBUG_PRINT ("%s is not defined in an argument", id);
            INFO_ALLFOUND (arg_info) = FALSE;

            // If the identifier is also not defined in a ret, give an error
            if (!TCspidsContains (INFO_INRET (arg_info), id)) {
                CTIerror (NODE_LOCATION (arg_node),
                          "Condition contains an identifier '%s' that is not "
                          "defined by an argument or a type pattern.",
                          id);
            }
        }
        break;
    }

    SPID_TDIM (arg_node) = TRAVopt (SPID_TDIM (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
