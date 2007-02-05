/*
 *
 * $Log$
 * Revision 1.9  2005/01/11 14:20:44  cg
 * Converted output generation from Error.h to ctinfo.c
 *
 * Revision 1.8  2004/11/25 17:52:55  sbs
 * compiles
 *
 * Revision 1.7  2004/11/23 20:22:31  sbs
 * SacDevCamp 04 done.
 *
 * Revision 1.6  2004/11/22 11:44:14  cg
 * Moved spec_mode_str from globals.c to this file.
 *
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

#include "type_statistics.h"
#include "dbug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "convert.h"
#include "internal_lib.h"
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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

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
    DBUG_ENTER ("ExamineTypes");
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

    DBUG_ENTER ("PrintStatistics");

    buf = ILIBstrBufCreate (80);
    buf = ILIBstrBufPrintf (buf, "  %s( ", FUNDEF_NAME (fundef));

    arg = FUNDEF_ARGS (fundef);
    while (arg != NULL) {
        buf = ILIBstrBufPrintf (buf, "%s", TYtype2String (ARG_NTYPE (arg), FALSE, 0));
        arg = ARG_NEXT (arg);
        if (arg != NULL) {
            buf = ILIBstrBufPrint (buf, ", ");
        }
    }
    buf = ILIBstrBufPrint (buf, "):\n");

    switch (global.spec_mode) {
    case (SS_aks):
        if (INFO_TS_AKD (info) > 0) {
            buf
              = ILIBstrBufPrintf (buf, "    %d akd variables left\n", INFO_TS_AKD (info));
            flag = TRUE;
            INFO_TS_ANY (info) = TRUE;
        }
        /* here no break is missing! */
    case (SS_akd):
        if (INFO_TS_AUD (info) > 0) {
            buf
              = ILIBstrBufPrintf (buf, "    %d aud variables left\n", INFO_TS_AUD (info));
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
        tmp = ILIBstrBuf2String (buf);
        CTInote ("%s", tmp);
        tmp = MEMfree (tmp);
    }
    buf = ILIBstrBufFree (buf);

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("TSdoPrintTypeStatistics");

    TRAVpush (TR_ts);

    CTInote ("\nType statistics:");

    arg_info = MakeInfo ();
    INFO_TS_ANY (arg_info) = FALSE;
    arg_node = TRAVdo (arg_node, arg_info);

    if (INFO_TS_ANY (arg_info)) {
        CTInote ("  ... for all other functions %s-info could be inferred.",
                 spec_mode_str[global.spec_mode]);
    } else {
        CTInote ("  for all functions %s-info could be inferred.",
                 spec_mode_str[global.spec_mode]);
    }
    arg_info = FreeInfo (arg_info);

    TRAVpop ();

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
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }

        /**
         * count vardecs:
         */
        if (FUNDEF_VARDEC (arg_node) != NULL) {
            FUNDEF_VARDEC (arg_node) = TRAVdo (FUNDEF_VARDEC (arg_node), arg_info);
        }

        PrintStatistics (arg_node, arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
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
    ntype *type;

    DBUG_ENTER ("TSarg");
    type = ARG_NTYPE (arg_node);
    arg_info = ExamineTypes (type, arg_info);
    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
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
    ntype *type;

    DBUG_ENTER ("TSvardec");
    type = VARDEC_NTYPE (arg_node);
    arg_info = ExamineTypes (type, arg_info);
    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}
