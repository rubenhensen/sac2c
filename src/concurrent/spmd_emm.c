/*
 *
 * $Log$
 * Revision 1.4  2004/11/27 01:24:31  ktr
 * SPMDEMMdoSpmdEmm
 *
 * Revision 1.3  2004/11/24 19:29:17  skt
 * Compiler Switch during SACDevCampDK 2k4
 *
 * Revision 1.2  2004/09/23 16:33:35  ktr
 * Repaired invalid free traversal.
 *
 * Revision 1.1  2004/09/18 15:59:35  ktr
 * Initial revision
 *
 *
 */

/**
 *
 * @file   spmd_emm.c
 *
 * prefix: SPMDEMM
 *
 *   This file implements the traversal of a function body in order to
 *   move explicit allocation and reference counting of local variables
 *   into spmd-blocks
 *
 */

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DataFlowMask.h"
#include "internal_lib.h"
#include "free.h"

/**
 *
 *  Enumeration for up/downtraversal
 *
 ***************************************************************************/
typedef enum { tm_up, tm_down } spmdemm_travmode;

/**
 * INFO structure
 */
struct INFO {
    spmdemm_travmode travmode;
    node *lhs;
    node *spmds;
    node *move_begin;
    node *move_end;
};

/**
 * INFO macros
 */
#define INFO_SPMDEMM_TRAVMODE(n) (n->travmode)
#define INFO_SPMDEMM_LHS(n) (n->lhs)
#define INFO_SPMDEMM_SPMDS(n) (n->spmds)
#define INFO_SPMDEMM_MOVE_BEGIN(n) (n->move_begin)
#define INFO_SPMDEMM_MOVE_END(n) (n->move_end)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_SPMDEMM_TRAVMODE (result) = tm_down;
    INFO_SPMDEMM_LHS (result) = NULL;
    INFO_SPMDEMM_SPMDS (result) = NULL;
    INFO_SPMDEMM_MOVE_BEGIN (result) = NULL;
    INFO_SPMDEMM_MOVE_END (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    node *temp;

    DBUG_ENTER ("FreeInfo");

    temp = INFO_SPMDEMM_SPMDS (info);
    while (temp != NULL) {
        EXPRS_EXPR (temp) = NULL;
        temp = FREEdoFreeNode (temp);
    }

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * @fn SPMDEMMdoSpmdEmm
 *
 *  @brief
 *
 *  @param arg_node
 *
 *  @return
 *
 ******************************************************************************/
node *
SPMDEMMdoSpmdEmm (node *arg_node)
{
    info *info;
    trav_t traversaltable;

    DBUG_ENTER ("SPMDEMMdoSpmdEmm");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "Illegal argument node!!!");

    info = MakeInfo ();

    TRAVpush (TR_spmdemm);

    FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

    traversaltable = TRAVpop ();
    DBUG_ASSERT ((traversaltable == TR_spmdemm), "Popped incorrect traversal table");

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn SPMDEMMassign
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ******************************************************************************/
node *
SPMDEMMassign (node *arg_node, info *arg_info)
{
    node *res_node;
    node *block;

    DBUG_ENTER ("SPMDEMMassign");

    /*
     * Traverse RHS in order to find SMPD blocks
     */
    INFO_SPMDEMM_TRAVMODE (arg_info) = tm_down;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /*
     * Traverse next
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /*
     * Traverse RHS in order to find moveable
     */
    INFO_SPMDEMM_TRAVMODE (arg_info) = tm_up;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    res_node = arg_node;
    /*
     * Move instruction from behind the SPMD-Block
     */
    if (INFO_SPMDEMM_MOVE_END (arg_info) != NULL) {
        res_node = ASSIGN_NEXT (arg_node);
        block = SPMD_REGION (INFO_SPMDEMM_MOVE_END (arg_info));
        ASSIGN_NEXT (arg_node) = NULL;
        BLOCK_INSTR (block) = TCappendAssign (BLOCK_INSTR (block), arg_node);

        INFO_SPMDEMM_MOVE_END (arg_info) = NULL;
    }

    /*
     * Move instruction from before the SPMD-Block
     */
    if (INFO_SPMDEMM_MOVE_BEGIN (arg_info) != NULL) {
        res_node = ASSIGN_NEXT (arg_node);
        block = SPMD_REGION (INFO_SPMDEMM_MOVE_BEGIN (arg_info));
        ASSIGN_NEXT (arg_node) = BLOCK_INSTR (block);
        BLOCK_INSTR (block) = arg_node;

        INFO_SPMDEMM_MOVE_BEGIN (arg_info) = NULL;
    }

    DBUG_RETURN (res_node);
}

/******************************************************************************
 *
 * @fn SPMDEMMlet
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ******************************************************************************/
node *
SPMDEMMlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDEMMlet");

    INFO_SPMDEMM_LHS (arg_info) = LET_IDS (arg_node);

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn SPMDEMMprf
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ******************************************************************************/
static node *
FindSpmd (char *name, node *exprs)
{
    node *res = NULL;

    DBUG_ENTER ("FindSpmd");

    if (exprs != NULL) {
        if (DFMtestMaskEntry (SPMD_LOCAL (EXPRS_EXPR (exprs)), name, NULL)) {
            res = EXPRS_EXPR (exprs);
        } else {
            res = FindSpmd (name, EXPRS_NEXT (exprs));
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn SPMDEMMprf
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ******************************************************************************/
node *
SPMDEMMprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDEMMprf");

    if (INFO_SPMDEMM_TRAVMODE (arg_info) == tm_up) {
        switch (PRF_PRF (arg_node)) {
        case F_alloc:
            INFO_SPMDEMM_MOVE_BEGIN (arg_info)
              = FindSpmd (IDS_NAME (INFO_SPMDEMM_LHS (arg_info)),
                          INFO_SPMDEMM_SPMDS (arg_info));
            break;

        case F_dec_rc:
            INFO_SPMDEMM_MOVE_END (arg_info)
              = FindSpmd (ID_NAME (PRF_ARG1 (arg_node)), INFO_SPMDEMM_SPMDS (arg_info));

            break;

        default:
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn SPMDEMMspmd
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ******************************************************************************/
node *
SPMDEMMspmd (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDEMMspmd");

    if (INFO_SPMDEMM_TRAVMODE (arg_info) == tm_down) {
        /*
         * On downtraversal, put all SPMD-Blocks into the SPMDS list
         * in arg_info
         */
        INFO_SPMDEMM_SPMDS (arg_info)
          = TBmakeExprs (arg_node, INFO_SPMDEMM_SPMDS (arg_info));
    }

    DBUG_RETURN (arg_node);
}
