/*
 *
 * $Log$
 * Revision 1.1  2003/09/17 12:36:46  sbs
 * Initial revision
 *
 *
 */

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
 */
#define INFO_TS_AKS(n) (n->refcnt)
#define INFO_TS_AKD(n) (n->flag)
#define INFO_TS_AUD(n) (n->counter)

/** <!--********************************************************************-->
 *
 * @fn node *ExamineTypes( types *type, node *info)
 *
 *   @brief       modifies the counters in info according to the type found
 *   @param type  var/arg type to be counted
 *   @param info  N_info that carries the counters
 *   @return      modified info
 *
 ******************************************************************************/

static node *
ExamineTypes (types *type, node *info)
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
 * @fn void PrintStatistics( node *fundef, node * info)
 *
 *   @brief         print the statistics found
 *   @param fundef  N_fundef under investigation
 *   @param info    N_info that carries the counters
 *
 ******************************************************************************/

static void
PrintStatistics (node *fundef, node *info)
{
    node *arg;
    str_buf *buf;
    char *tmp;
    bool flag = FALSE;

    DBUG_ENTER ("PrintStatistics");

    buf = StrBufCreate (80);
    buf = StrBufprintf (buf, "%s( ", FUNDEF_NAME (fundef));

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
            buf = StrBufprintf (buf, "AKD variables left: %d\n", INFO_TS_AKD (info));
            flag = TRUE;
        }
        /* here no break is missing! */
    case (SS_akd):
        if (INFO_TS_AUD (info) > 0) {
            buf = StrBufprintf (buf, "AUD variables left: %d\n", INFO_TS_AUD (info));
            flag = TRUE;
        }
        break;
    case (SS_aud):
    default:
        /* print nothing */
    }
    if (flag) {
        tmp = StrBuf2String (buf);
        NOTE (("%s", tmp));
        tmp = Free (tmp);
    }
    buf = StrBufFree (buf);

    DBUG_VOID_RETURN
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
    node *arg_info;

    DBUG_ENTER ("PrintTypeStatistics");

    tmp_tab = act_tab;
    act_tab = ts_tab;

    arg_info = MakeInfo ();
    arg_node = Trav (arg_node, arg_info);
    arg_info = FreeNode (arg_info);

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TSfundef( node *arg_node, node *arg_info)
 *
 *   @brief          initiate statistics and print result
 *   @param arg_node N_fundef node
 *   @param arg_info N_info node
 *   @return         unmodified arg_node
 *
 ******************************************************************************/

node *
TSfundef (node *arg_node, node *arg_info)
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
 * @fn node *TSarg( node *arg_node, node *arg_info)
 *
 *   @brief          collect argument statistics
 *   @param arg_node N_arg node
 *   @param arg_info N_info node
 *   @return         unmodified arg_node
 *
 ******************************************************************************/

node *
TSarg (node *arg_node, node *arg_info)
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
 * @fn node *TSvardec( node *arg_node, node *arg_info)
 *
 *   @brief          collect variable statistics
 *   @param arg_node N_vardec node
 *   @param arg_info N_info node
 *   @return         unmodified arg_node
 *
 ******************************************************************************/

node *
TSvardec (node *arg_node, node *arg_info)
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
