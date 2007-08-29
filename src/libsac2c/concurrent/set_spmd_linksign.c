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
 *
 * @fn node *SSPMDLSarg(node *arg_node, info *arg_info)
 *
 *    @brief This function adds the LS of the corresponding ret or a new one
 *           to the arg.
 *
 *    @param arg_node N_arg node
 *    @param arg_info INFO structure
 *
 *    @return N_arg arg_node
 ******************************************************************************/

node *
SSPMDLSarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSarg");

    node *nret = NULL;
    nret = LUTsearchInLutPp (INFO_LUT (arg_info), ARG_AVIS (arg_node));

    /* If current avis is in LUT, this argument is ret.*/
    if (nret != ARG_AVIS (arg_node)) {
        ARG_LINKSIGN (arg_node) = RET_LINKSIGN (nret);
    } else /* New argument*/
    {
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
 *
 * @fn node *SSPMDLSid(node *arg_node, info *arg_info)
 *
 *    @brief This function inserts withret-avis -> memval (default) into the
 *           LUT if it is reached from through genarray, modarray, break or
 *           propagate. Later when this node is reached through a N_return,
 *           memval->ret is inserted in the LUT.
 *           In this way we get the chain withrets -> memval -> rets.
 *           The avis of the memvals and the avis of the functionargs is the
 *           same so later when we reach the arguments, it is possible to find
 *           the argument which belongs to a certain return.
 *
 *    @param arg_node N_id node
 *    @param arg_info INFO struct
 *
 *    @return unchanged N_id node
 ******************************************************************************/

node *
SSPMDLSid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSid");

    /* if INFO_MEM is set, this is a Memval (genarray, modarray, break) or a
     * default (propagate). In this case insert withret-avis -> memval into the
     * LUT.*/
    if (INFO_MEM (arg_info)) {
        DBUG_PRINT ("SSPMDLS",
                    ("Insert %s->%s into LUT (id)", IDS_NAME (INFO_WITH_RETS (arg_info)),
                     ID_NAME (arg_node)));
        INFO_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_WITH_RETS (arg_info)),
                               ID_AVIS (arg_node));
    } else if (INFO_RETURNS (arg_info)) {
        /* if this is an id of a return, get memval and insert memval -> ret into
         * the LUT*/
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
 *
 * @fn SSPMDLSexprs(node *arg_node, info *arg_info)
 *
 *    @brief This function just checks if this is a return-expression. If this
 *           is the case we have to traverse further.
 *
 *    @param arg_node N_exprs node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_exprs node
 ******************************************************************************/

node *
SSPMDLSexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSexprs");

    /* As we are just adding linksigns to arguments and rets, we just have to
     * traverse further if this is a return-expression.*/
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
 *
 * @fn node *SSPMDLSreturn(node *arg_node, info *arg_info)
 *
 *    @brief This function just sets the flag "INFO_RETURN" which indicates
 *           in the N_expr node that it is reached from a N_return node because
 *           this is the only case in which we have to traverse further into
 *           the expression.
 *
 *    @param arg_node N_return node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_return node
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
 *
 * @fn node *SSPMDLSpropagate(node *arg_node, info *arg_info)
 *
 *    @brief This function traverses into the default of the propagate and in
 *           case of further withops shifts the INFO_WITH_RETS one position
 *           further befor traversing into the next op.
 *
 *    @param arg_node N_propagate node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_propagate node
 ******************************************************************************/

node *
SSPMDLSpropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSpropagate");

    INFO_MEM (arg_info) = TRUE;

    PROPAGATE_DEFAULT (arg_node) = TRAVdo (PROPAGATE_DEFAULT (arg_node), arg_info);

    INFO_MEM (arg_info) = FALSE;

    /* as we have to find out each pair of indirectly connected function
     * arguments and function returns, we try to keep track of the variable
     * change which is caused by the with loop.
     * Therefor we traverse through all withops and shift the returns of the
     * withloop parallel to this traversing.*/
    if (PROPAGATE_NEXT (arg_node) != NULL) {
        INFO_WITH_RETS (arg_info) = IDS_NEXT (INFO_WITH_RETS (arg_info));
        DBUG_ASSERT (INFO_WITH_RETS (arg_info) != NULL, "#ids != #with-returns!");
        PROPAGATE_NEXT (arg_node) = TRAVdo (PROPAGATE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SSPMDLSgenarray(node *arg_node, info *arg_info)
 *
 *    @brief This function traverses into the memval of genarray and in case
 *           of further withops shifts the INFO_WITH_RETS one position further
 *           further befor traversing into the next op.
 *
 *    @param arg_node N_genarray node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_genarray node
 ******************************************************************************/

node *
SSPMDLSgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSgenarray");

    INFO_MEM (arg_info) = TRUE;

    GENARRAY_MEM (arg_node) = TRAVdo (GENARRAY_MEM (arg_node), arg_info);

    INFO_MEM (arg_info) = FALSE;

    /* as we have to find out each pair of indirectly connected function
     * arguments and function returns, we try to keep track of the variable
     * change which is caused by the with loop.
     * Therefor we traverse through all withops and shift the returns of the
     * withloop parallel to this traversing.*/
    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_WITH_RETS (arg_info) = IDS_NEXT (INFO_WITH_RETS (arg_info));
        DBUG_ASSERT (INFO_WITH_RETS (arg_info) != NULL, "#ids != #with-returns!");
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SSPMDLSmodarray(node *arg_node, info *arg_info)
 *
 *    @brief This function traverses into the memval of the modarray and in
 *           case of further withops shifts the INFO_WITH_RETS one position
 *           further befor traversing into the next op.
 *
 *    @param arg_node N_modarray node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_modarray node
 ******************************************************************************/

node *
SSPMDLSmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSmodarray");

    INFO_MEM (arg_info) = TRUE;

    MODARRAY_MEM (arg_node) = TRAVdo (MODARRAY_MEM (arg_node), arg_info);

    INFO_MEM (arg_info) = FALSE;

    /* as we have to find out each pair of indirectly connected function
     * arguments and function returns, we try to keep track of the variable
     * change which is caused by the with loop.
     * Therefor we traverse through all withops and shift the returns of the
     * withloop parallel to this traversing.*/
    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_WITH_RETS (arg_info) = IDS_NEXT (INFO_WITH_RETS (arg_info));
        DBUG_ASSERT (INFO_WITH_RETS (arg_info) != NULL, "#ids != #with-returns!");
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SSPMDLSbreak(node *arg_node, info *arg_info)
 *
 *    @brief This function traverses into the memval of the break and in case
 *           of further withops shifts the INFO_WITH_RETS one position further
 *           befor traversing into the next op.
 *
 *    @param arg_node N_break node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_break node
 ******************************************************************************/

node *
SSPMDLSbreak (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSbreak");

    INFO_MEM (arg_info) = TRUE;

    BREAK_MEM (arg_node) = TRAVdo (BREAK_MEM (arg_node), arg_info);

    INFO_MEM (arg_info) = FALSE;

    /* as we have to find out each pair of indirectly connected function
     * arguments and function returns, we try to keep track of the variable
     * change which is caused by the with loop.
     * Therefor we traverse through all withops and shift the returns of the
     * withloop parallel to this traversing.*/
    if (BREAK_NEXT (arg_node) != NULL) {
        INFO_WITH_RETS (arg_info) = IDS_NEXT (INFO_WITH_RETS (arg_info));
        DBUG_ASSERT (INFO_WITH_RETS (arg_info) != NULL, "#ids != #with-returns!");
        BREAK_NEXT (arg_node) = TRAVdo (BREAK_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SSPMDLSwith2(node *arg_node, info *arg_info)
 *
 *    @brief This function just traverses further into the withops as we are
 *           just interested in the MEMVALs and DEFAULTs.
 *
 *    @param arg_node N_with2 node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_with2 node
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
 *
 * @fn node *SSPMDLSlet(node *arg_node, info *arg_info)
 *
 *    @brief This function keeps the LHS of the let in the INFO structure just
 *           in case of a RHS with-loop.
 *
 *    @param arg_node N_let node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_let node
 ******************************************************************************/

node *
SSPMDLSlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSPMDLSlet");

    /* Keep LHS, if RHS is withloop, it is needed to associate it to the
     * corresponding memvals, respectively defaults.*/
    INFO_WITH_RETS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SSPMDLSret(node *arg_node, info *arg_info)
 *
 *    @brief This function sets the linksine of the current function ret.
 *
 *    @param arg_node N_ret node
 *    @param arg_info INFO structure
 *
 *    @return N_ret node
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
 *
 * @fn node *SSPMDLSfundef(node *arg_node, info *arg_info)
 *
 *    @brief This function traverses the different parts of the fundef in the
 *           right order to keep track of the pairs of corresponding
 *           args/returns.
 *
 *    @param arg_node N_fundef node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_fundef node
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
 *
 * @fn node *SSPMDLSmodule(node *arg_node, info *arg_info)
 *
 *    @brief This function just initializes the LUT.
 *
 *    @param arg_node N_module node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_module node
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
 *
 * @fn node *SSPMDLSdoSetSpmdLinksign(node *syntax_tree)
 *
 *    @brief This function triggers the traversal.
 *
 *    @param syntax_tree N_module node
 *
 *    @return syntax tree with LS pragmas on SPMD functionsparams and rets.
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
