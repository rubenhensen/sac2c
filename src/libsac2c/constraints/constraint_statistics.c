#include "constraint_statistics.h"

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

#define MIN_CONSTRAINT_PRF F_guard
#define MAX_CONSTRAINT_PRF F_prod_matches_prod_shape_VxA

#define NUM_CONSTRAINT_PRFS (1 + MAX_CONSTRAINT_PRF - MIN_CONSTRAINT_PRF)

/**
 * INFO structure
 */
struct INFO {
    int *prf_cnt;
    bool all_gone_local;
    bool all_gone;
};

/**
 * INFO macros
 */

#define INFO_PRF_CNT(n) (n->prf_cnt)
#define INFO_PRF_COUNT(n, p) (n->prf_cnt[p])
#define INFO_ALL_GONE_LOCAL(n) (n->all_gone_local)
#define INFO_ALL_GONE(n) (n->all_gone)

/**
 * INFO functions
 */
static info *
InitCounters (info *info)
{
    int i;
    DBUG_ENTER ();

    for (i = 0; i < NUM_CONSTRAINT_PRFS; i++) {
        INFO_PRF_COUNT (info, i) = 0;
    }
    INFO_ALL_GONE_LOCAL (info) = TRUE;

    DBUG_RETURN (info);
}

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_PRF_CNT (result) = (int *)MEMmalloc (sizeof (int) * NUM_CONSTRAINT_PRFS);
    result = InitCounters (result);

    INFO_ALL_GONE (result) = TRUE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_PRF_CNT (info) = MEMfree (INFO_PRF_CNT (info));
    info = MEMfree (info);

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
    int i;

    DBUG_ENTER ();

    if (!INFO_ALL_GONE (info)) {
        buf = SBUFcreate (80);
        buf = SBUFprintf (buf, "%s( ", CTIitemName (fundef));

        arg = FUNDEF_ARGS (fundef);
        while (arg != NULL) {
            buf = SBUFprintf (buf, "%s", TYtype2String (ARG_NTYPE (arg), FALSE, 0));
            arg = ARG_NEXT (arg);
            if (arg != NULL) {
                buf = SBUFprint (buf, ", ");
            }
        }
        buf = SBUFprint (buf, "):\n");

        if (!INFO_ALL_GONE_LOCAL (info)) {
            for (i = 0; i < NUM_CONSTRAINT_PRFS; i++) {
                if (INFO_PRF_COUNT (info, i) > 0) {
                    buf = SBUFprintf (buf, "    %d prfs %s left\n",
                                      INFO_PRF_COUNT (info, i),
                                      global.prf_name[i + MIN_CONSTRAINT_PRF]);
                }
            }
        }
        tmp = SBUF2str (buf);
        CTItell (0, "  %s", tmp);
        tmp = MEMfree (tmp);

        buf = SBUFfree (buf);
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn node *CSdoPrintConstraintStatistics( node *arg_node)
 *
 *   @brief
 *   @param arg_node  AST to be inspected
 *   @return          unmodified AST
 *
 ******************************************************************************/

node *
CSdoPrintConstraintStatistics (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    TRAVpush (TR_cs);

    CTItell (0, " ");
    CTItell (0, "*********************************************************************");
    CTItell (0, "** Constraint resolution feedback                                  **");
    CTItell (0, "*********************************************************************");

    arg_info = MakeInfo ();

    arg_node = TRAVdo (arg_node, arg_info);

    if (INFO_ALL_GONE (arg_info)) {
        if (!global.insertconformitychecks) {
            CTItell (0, "  No constraints to resolve as none have been injected.");
            CTItell (0, "  Either use -check c or -ecc to turn constraint injection on.");
        } else {
            CTItell (0, "  For all functions all constraints were statically resolved.");
        }
    } else {
        CTItell (0, "  For all other functions all constraints were statically resolved.");
    }
    arg_info = FreeInfo (arg_info);

    TRAVpop ();

    CTItell (0, "*********************************************************************");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CSfundef( node *arg_node, info *arg_info)
 *
 *   @brief          initiate statistics and print result
 *   @param arg_node N_fundef node
 *   @param arg_info N_info node
 *   @return         unmodified arg_node
 *
 ******************************************************************************/

node *
CSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        /**
         * init counters:
         */
        arg_info = InitCounters (arg_info);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        PrintStatistics (arg_node, arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CSprf( node *arg_node, info *arg_info)
 *
 *   @brief          collect prf statistics
 *   @param arg_node N_arg node
 *   @param arg_info N_info node
 *   @return         unmodified arg_node
 *
 ******************************************************************************/

node *
CSprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((PRF_PRF (arg_node) >= MIN_CONSTRAINT_PRF)
        && (PRF_PRF (arg_node) <= MAX_CONSTRAINT_PRF)) {
        INFO_PRF_COUNT (arg_info, PRF_PRF (arg_node) - MIN_CONSTRAINT_PRF)++;
        INFO_ALL_GONE_LOCAL (arg_info) = FALSE;
        INFO_ALL_GONE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
