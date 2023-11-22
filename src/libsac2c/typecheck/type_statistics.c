#include "type_statistics.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "convert.h"
#include "str_buffer.h"
#include "str.h"
#include "memory.h"
#include "globals.h"
#include "new_types.h"
#include "ctinfo.h"

/**
 *
 * Usage of the info node:
 *
 *   INFO_TS_AKS      number of AKS-vars found
 *   INFO_TS_AKD      number of AKD-vars found
 *   INFO_TS_AUD      number of AUD-vars found
 *
 *   INFO_TS_ANY      flag that indicates wether anything had to be reported
 */

/**
 * INFO structure
 */
struct INFO {
    int aks;
    int akd;
    int aud;
    bool any;
};

/**
 * INFO macros
 */

#define INFO_TS_AKS(n) (n->aks)
#define INFO_TS_AKD(n) (n->akd)
#define INFO_TS_AUD(n) (n->aud)
#define INFO_TS_ANY(n) (n->any)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_TS_AKS (result) = 0;
    INFO_TS_AKD (result) = 0;
    INFO_TS_AUD (result) = 0;
    INFO_TS_ANY (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * static global variables
 */

static char spec_mode_str[][4] = {"aks", "akd", "aud"};

/** <!--********************************************************************-->
 *
 * @fn node *ExamineTypes( ntype *type, info *info)
 *
 *   @brief       modifies the counters in info according to the type found
 *   @param type  var/arg type to be counted
 *   @param info  N_info that carries the counters
 *   @return      modified info
 *
 ******************************************************************************/

static info *
ExamineTypes (ntype *type, info *info)
{
    DBUG_ENTER ();
    if (TYisAKV (type) || TYisAKS (type)) {
        INFO_TS_AKS (info) += 1;
    } else if (TYisAKD (type)) {
        INFO_TS_AKD (info) += 1;
    } else {
        INFO_TS_AUD (info) += 1;
    }
    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn void PrintStatistics( node *fundef, info * info)
 *
 *   @brief         print the statistics found
 *   @param fundef  N_fundef under investigation
 *   @param info    N_info that carries the counters
 *
 ******************************************************************************/

static void
PrintStatistics (node *fundef, info *info)
{
    node *arg;
    str_buf *buf;
    char *tmp;
    bool flag = FALSE;

    DBUG_ENTER ();

    buf = SBUFcreate (80);
    buf = SBUFprintf (buf, "%s( ", FUNDEF_NAME (fundef));

    arg = FUNDEF_ARGS (fundef);
    while (arg != NULL) {
        buf = SBUFprintf (buf, "%s", TYtype2String (ARG_NTYPE (arg), FALSE, 0));
        arg = ARG_NEXT (arg);
        if (arg != NULL) {
            buf = SBUFprint (buf, ", ");
        }
    }
    buf = SBUFprint (buf, "):\n");

    switch (global.spec_mode) {
    case (SS_aks):
        if (INFO_TS_AKD (info) > 0) {
            buf = SBUFprintf (buf, "    %d akd variables left\n", INFO_TS_AKD (info));
            flag = TRUE;
            INFO_TS_ANY (info) = TRUE;
        }
        /* Falls through. */
    case (SS_akd):
        if (INFO_TS_AUD (info) > 0) {
            buf = SBUFprintf (buf, "    %d aud variables left\n", INFO_TS_AUD (info));
            flag = TRUE;
            INFO_TS_ANY (info) = TRUE;
        }
        break;
    case (SS_aud):
    default:
        /* print nothing */
        break;
    }
    if (flag) {
        tmp = SBUF2str (buf);
        CTItell (0, "  %s", tmp);
        tmp = MEMfree (tmp);
    }
    buf = SBUFfree (buf);

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn node *TSdoPrintTypeStatistics( node *arg_node)
 *
 *   @brief           compares the existing type annotation against the goal
 *                    of the specialization strategy chosen!
 *   @param arg_node  AST to be inspected
 *   @return          unmodified AST
 *
 ******************************************************************************/

node *
TSdoPrintTypeStatistics (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    TRAVpush (TR_ts);

    CTItell (0, " ");
    CTItell (0, "*********************************************************************");
    CTItell (0, "** Type specialisation feedback                                    **");
    CTItell (0, "*********************************************************************");

    arg_info = MakeInfo ();
    INFO_TS_ANY (arg_info) = FALSE;
    arg_node = TRAVdo (arg_node, arg_info);

    if (INFO_TS_ANY (arg_info)) {
        CTItell (0, "  For all other functions %s-info has been inferred.",
                 spec_mode_str[global.spec_mode]);
    } else {
        CTItell (0, "  For all functions %s-info has been inferred.",
                 spec_mode_str[global.spec_mode]);
    }
    arg_info = FreeInfo (arg_info);

    TRAVpop ();

    CTItell (0, "*********************************************************************");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TSfundef( node *arg_node, info *arg_info)
 *
 *   @brief          initiate statistics and print result
 *   @param arg_node N_fundef node
 *   @param arg_info N_info node
 *   @return         unmodified arg_node
 *
 ******************************************************************************/

node *
TSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        /**
         * init counters:
         */
        INFO_TS_AKS (arg_info) = 0;
        INFO_TS_AKD (arg_info) = 0;
        INFO_TS_AUD (arg_info) = 0;

        /**
         * count args:
         */
        FUNDEF_ARGS (arg_node) = TRAVopt(FUNDEF_ARGS (arg_node), arg_info);

        /**
         * count vardecs:
         */
        FUNDEF_VARDECS (arg_node) = TRAVopt(FUNDEF_VARDECS (arg_node), arg_info);

        PrintStatistics (arg_node, arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TSarg( node *arg_node, info *arg_info)
 *
 *   @brief          collect argument statistics
 *   @param arg_node N_arg node
 *   @param arg_info N_info node
 *   @return         unmodified arg_node
 *
 ******************************************************************************/

node *
TSarg (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ();
    type = ARG_NTYPE (arg_node);
    arg_info = ExamineTypes (type, arg_info);
    ARG_NEXT (arg_node) = TRAVopt(ARG_NEXT (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TSvardec( node *arg_node, info *arg_info)
 *
 *   @brief          collect variable statistics
 *   @param arg_node N_vardec node
 *   @param arg_info N_info node
 *   @return         unmodified arg_node
 *
 ******************************************************************************/

node *
TSvardec (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ();
    type = VARDEC_NTYPE (arg_node);
    arg_info = ExamineTypes (type, arg_info);
    VARDEC_NEXT (arg_node) = TRAVopt(VARDEC_NEXT (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
