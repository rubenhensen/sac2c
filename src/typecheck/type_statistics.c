/*
 *
 * $Log$
 * Revision 1.5  2004/07/30 17:29:21  sbs
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.4  2003/09/18 11:14:26  sbs
 * break in default branch of switch added.
 *
 * Revision 1.3  2003/09/18 08:57:10  sbs
 * missing colon after DBUG_VOID_RETURN added.
 *
 * Revision 1.2  2003/09/17 13:06:26  sbs
 * output reniced.
 *
 * Revision 1.1  2003/09/17 12:36:46  sbs
 * Initial revision
 *
 *
 */

#define NEW_INFO

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "convert.h"
#include "internal_lib.h"
#include "globals.h"

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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_TS_AKS (result) = 0;
    INFO_TS_AKD (result) = 0;
    INFO_TS_AUD (result) = 0;
    INFO_TS_ANY (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *ExamineTypes( types *type, info *info)
 *
 *   @brief       modifies the counters in info according to the type found
 *   @param type  var/arg type to be counted
 *   @param info  N_info that carries the counters
 *   @return      modified info
 *
 ******************************************************************************/

static info *
ExamineTypes (types *type, info *info)
{
    int dim;
    DBUG_ENTER ("ExamineTypes");
    dim = TYPES_DIM (type);
    if (KNOWN_SHAPE (dim)) {
        INFO_TS_AKS (info) += 1;
    } else if (KNOWN_DIMENSION (dim)) {
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

    DBUG_ENTER ("PrintStatistics");

    buf = StrBufCreate (80);
    buf = StrBufprintf (buf, "  %s( ", FUNDEF_NAME (fundef));

    arg = FUNDEF_ARGS (fundef);
    while (arg != NULL) {
        buf = StrBufprintf (buf, "%s", Type2String (ARG_TYPE (arg), 0, 0));
        arg = ARG_NEXT (arg);
        if (arg != NULL) {
            buf = StrBufprint (buf, ", ");
        }
    }
    buf = StrBufprint (buf, "):\n");

    switch (spec_mode) {
    case (SS_aks):
        if (INFO_TS_AKD (info) > 0) {
            buf = StrBufprintf (buf, "    %d akd variables left\n", INFO_TS_AKD (info));
            flag = TRUE;
            INFO_TS_ANY (info) = TRUE;
        }
        /* here no break is missing! */
    case (SS_akd):
        if (INFO_TS_AUD (info) > 0) {
            buf = StrBufprintf (buf, "    %d aud variables left\n", INFO_TS_AUD (info));
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
        tmp = StrBuf2String (buf);
        NOTE (("%s", tmp));
        tmp = Free (tmp);
    }
    buf = StrBufFree (buf);

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn node *PrintTypeStatistics( node *arg_node)
 *
 *   @brief           compares the existing type annotation against the goal
 *                    of the specialization strategy chosen!
 *   @param arg_node  AST to be inspected
 *   @return          unmodified AST
 *
 ******************************************************************************/

node *
PrintTypeStatistics (node *arg_node)
{
    funtab *tmp_tab;
    info *arg_info;

    DBUG_ENTER ("PrintTypeStatistics");

    tmp_tab = act_tab;
    act_tab = ts_tab;

    NOTE ((""));
    NOTE (("type statistics:"));

    arg_info = MakeInfo ();
    INFO_TS_ANY (arg_info) = FALSE;
    arg_node = Trav (arg_node, arg_info);

    if (INFO_TS_ANY (arg_info)) {
        NOTE (("  ... for all other functions %s-info could be inferred.",
               spec_mode_str[spec_mode]));
    } else {
        NOTE (
          ("  for all functions %s-info could be inferred.", spec_mode_str[spec_mode]));
    }
    arg_info = FreeInfo (arg_info);

    act_tab = tmp_tab;

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
    DBUG_ENTER ("TSfundef");

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
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        }

        /**
         * count vardecs:
         */
        if (FUNDEF_VARDEC (arg_node) != NULL) {
            FUNDEF_VARDEC (arg_node) = Trav (FUNDEF_VARDEC (arg_node), arg_info);
        }

        PrintStatistics (arg_node, arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }
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
    types *type;

    DBUG_ENTER ("TSarg");
    type = ARG_TYPE (arg_node);
    arg_info = ExamineTypes (type, arg_info);
    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }
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
    types *type;

    DBUG_ENTER ("TSvardec");
    type = VARDEC_TYPE (arg_node);
    arg_info = ExamineTypes (type, arg_info);
    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}
