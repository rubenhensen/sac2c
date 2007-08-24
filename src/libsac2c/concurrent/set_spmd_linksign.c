#include "set_spmd_linksign.h"

#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "memory.h"
#include "LookUpTable.h"

/**
 * INFO structure
 */

struct INFO {
    node *with_rets;
    node *fun_rets;
    lut_t *lut;
    int ls_num;
    bool mem;
    bool returns;
};

/**
 * INFO macros
 */

#define INFO_WITH_RETS(n) (n->with_rets)
#define INFO_FUN_RETS(n) (n->fun_rets)
#define INFO_LUT(n) (n->lut)
#define INFO_LS_NUM(n) (n->ls_num)
#define INFO_MEM(n) (n->mem)
#define INFO_RETURNS(n) (n->returns)
/**
 * INFO functions
 */

static info *
MakeInfo ()
{
    DBUG_ENTER ("MakeInfo");

    info *result;
    result = MEMmalloc (sizeof (info));

    INFO_WITH_RETS (result) = NULL;
    INFO_FUN_RETS (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_LS_NUM (result) = 0;
    INFO_MEM (result) = FALSE;
    INFO_RETURNS (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/

node *
SSPMDLSarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSarg");

    node *nret = NULL;
    nret = LUTsearchInLutPp (INFO_LUT (arg_info), ARG_AVIS (arg_node));

    if (nret != ARG_AVIS (arg_node)) {
        ARG_LINKSIGN (arg_node) = RET_LINKSIGN (nret);
    } else {
        ARG_LINKSIGN (arg_node) = INFO_LS_NUM (arg_info);
        INFO_LS_NUM (arg_info) += 1;
    }

    ARG_HASLINKSIGNINFO (arg_node) = TRUE;

    DBUG_PRINT ("SSPMDLS",
                ("Add LS %i to arg %s", ARG_LINKSIGN (arg_node), ARG_NAME (arg_node)));

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/

node *
SSPMDLSid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSid");

    if (INFO_MEM (arg_info)) {
        DBUG_PRINT ("SSPMDLS",
                    ("Insert %s->%s into LUT (id)", IDS_NAME (INFO_WITH_RETS (arg_info)),
                     ID_NAME (arg_node)));
        INFO_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_WITH_RETS (arg_info)),
                               ID_AVIS (arg_node));
    } else if (INFO_RETURNS (arg_info)) {
        node *avis = NULL;
        DBUG_PRINT ("SSPMDLS",
                    ("Looking up arg for retexpr %s", AVIS_NAME (ID_AVIS (arg_node))));
        avis = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));

        DBUG_PRINT ("SSPMDLS", ("...found %s", AVIS_NAME (avis)));

        INFO_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, INFO_FUN_RETS (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/

node *
SSPMDLSexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSexprs");

    if (INFO_RETURNS (arg_info)) {
        EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);
        if (EXPRS_NEXT (arg_node) != NULL) {
            INFO_FUN_RETS (arg_info) = RET_NEXT (INFO_FUN_RETS (arg_info));
            DBUG_ASSERT (INFO_FUN_RETS (arg_info) != NULL, "#Returns != #Rets!");
            EXPRS_NEXT (arg_node) = TRAVdo (EXPRS_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/

node *
SSPMDLSreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSreturn");

    INFO_RETURNS (arg_info) = TRUE;

    RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);

    INFO_RETURNS (arg_info) = FALSE;
    INFO_FUN_RETS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/

node *
SSPMDLSpropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSpropagate");

    INFO_MEM (arg_info) = TRUE;

    PROPAGATE_DEFAULT (arg_node) = TRAVdo (PROPAGATE_DEFAULT (arg_node), arg_info);

    INFO_MEM (arg_info) = FALSE;

    if (PROPAGATE_NEXT (arg_node) != NULL) {
        INFO_WITH_RETS (arg_info) = IDS_NEXT (INFO_WITH_RETS (arg_info));
        DBUG_ASSERT (INFO_WITH_RETS (arg_info) != NULL, "#ids != #with-returns!");
        PROPAGATE_NEXT (arg_node) = TRAVdo (PROPAGATE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/

node *
SSPMDLSgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSgenarray");

    INFO_MEM (arg_info) = TRUE;

    GENARRAY_MEM (arg_node) = TRAVdo (GENARRAY_MEM (arg_node), arg_info);

    INFO_MEM (arg_info) = FALSE;

    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_WITH_RETS (arg_info) = IDS_NEXT (INFO_WITH_RETS (arg_info));
        DBUG_ASSERT (INFO_WITH_RETS (arg_info) != NULL, "#ids != #with-returns!");
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/

node *
SSPMDLSmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSmodarray");

    INFO_MEM (arg_info) = TRUE;

    MODARRAY_MEM (arg_node) = TRAVdo (MODARRAY_MEM (arg_node), arg_info);

    INFO_MEM (arg_info) = FALSE;

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_WITH_RETS (arg_info) = IDS_NEXT (INFO_WITH_RETS (arg_info));
        DBUG_ASSERT (INFO_WITH_RETS (arg_info) != NULL, "#ids != #with-returns!");
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/

node *
SSPMDLSbreak (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSbreak");

    INFO_MEM (arg_info) = TRUE;

    BREAK_MEM (arg_node) = TRAVdo (BREAK_MEM (arg_node), arg_info);

    INFO_MEM (arg_info) = FALSE;

    if (BREAK_NEXT (arg_node) != NULL) {
        INFO_WITH_RETS (arg_info) = IDS_NEXT (INFO_WITH_RETS (arg_info));
        DBUG_ASSERT (INFO_WITH_RETS (arg_info) != NULL, "#ids != #with-returns!");
        BREAK_NEXT (arg_node) = TRAVdo (BREAK_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/

node *
SSPMDLSwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSwith2");

    /* We just have to traverse the withops*/
    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    INFO_WITH_RETS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/

node *
SSPMDLSlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSlet");

    /* Keep LHS, if RHS is withloop, it is needed.*/
    INFO_WITH_RETS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/

node *
SSPMDLSret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSret");

    if (INFO_LS_NUM (arg_info) > 0) {

        /* Set linksign*/
        RET_LINKSIGN (arg_node) = INFO_LS_NUM (arg_info);
        RET_HASLINKSIGNINFO (arg_node) = TRUE;

        DBUG_PRINT ("SSPMDLS", ("Add LS %i to ret", RET_LINKSIGN (arg_node)));

        INFO_LS_NUM (arg_info) += 1;

        if (RET_NEXT (arg_node) != NULL) {
            RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/

node *
SSPMDLSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSfundef");

    if (FUNDEF_ISSPMDFUN (arg_node)) {
        INFO_LS_NUM (arg_info) = 1;

        /* First of all, add LS to rets*/
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);

        /* Keep rets to link them to the arguments*/
        INFO_FUN_RETS (arg_info) = FUNDEF_RETS (arg_node);

        /* Traverse through the body to link the rets to the args*/
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        /* Traverse the args to add the LS*/
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);

        INFO_LS_NUM (arg_info) = 0;
    }

    INFO_LUT (arg_info) = LUTremoveContentLut (INFO_LUT (arg_info));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/

node *
SSPMDLSmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLmodule");

    INFO_LUT (arg_info) = LUTgenerateLut ();

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param syntax_tree
 *
 * @return
 ******************************************************************************/

node *
SSPMDLSdoSetSpmdLinksign (node *syntax_tree)
{
    DBUG_ENTER ("SSPMDLSdoSetSpmdLinksign");

    info *info;

    info = MakeInfo ();

    TRAVpush (TR_sspmdls);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
