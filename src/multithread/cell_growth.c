/*
 * $Log$
 * Revision 1.1  2004/08/17 09:06:38  skt
 * Initial revision
 *
 */

/**
 *
 * @defgroup cegro Cell Growth
 * @ingroup muth
 *
 * @brief move each assignment into a cell
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file cell_growth.c
 *
 * prefix: CEGRO
 *
 * description:
 *
 *****************************************************************************/

#define NEW_INFO

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "print.h"
#include "cell_growth.h"
#include "multithread.h"
#include "multithread_lib.h"

/*
 * INFO structure
 */
struct INFO {
    node *next_cell;
};

/*
 * INFO macros
 *    node*  CEGRO_NEXTCELL          (the cell which has to be the next in the
 *                                    assignment-chain)
 */
#define INFO_CEGRO_NEXTCELL(n) (n->next_cell)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_CEGRO_NEXTCELL (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/* TODO: TravNone in node_info.mac for:
 *                nwith2, N_let, N_return, N_ex, N_st, N_mt
 */

/** <!--********************************************************************-->
 *
 * @fn node *CellGrowth(node *arg_node)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_Modul
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CellGrowth (node *arg_node)
{
    funtab *old_tab;
    info *arg_info;
    DBUG_ENTER ("CellGrowth");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "CellGrowth expects a N_modul as arg_node");

    arg_info = MakeInfo ();
    /* push info ... */
    old_tab = act_tab;
    /*act_tab = cegro_tab;*/

    DBUG_PRINT ("CEGRO", ("trav into modul-funs"));
    MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    DBUG_PRINT ("CEGRO", ("trav from modul-funs"));

    /* pop info ... */
    act_tab = old_tab;

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

node *
CEGROblock (node *arg_node, info *arg_info)
{
    node *old_nextcell;
    DBUG_ENTER ("CEGROblock");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_block), "arg_node is not a N_block");

    /* push info... */
    old_nextcell = INFO_CEGRO_NEXTCELL (arg_info);
    INFO_CEGRO_NEXTCELL (arg_info) = NULL;

    /* continue traversal */
    if (BLOCK_INSTR (arg_node) != NULL) {
        DBUG_PRINT ("CEGRO", ("trav into instruction(s)"));
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
        DBUG_PRINT ("CEGRO", ("trav from instruction(s)"));
    }

    /* pop info... */
    INFO_CEGRO_NEXTCELL (arg_info) = old_nextcell;

    DBUG_RETURN (arg_node);
}

node *
CEGROassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CEGROassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), "arg_node is no a N_assign");

    /* traverse into the instruction - it could be a conditional */
    if (ASSIGN_INSTR (arg_node) != NULL) {
        DBUG_PRINT ("CEGRO", ("trav into instruction"));
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        DBUG_PRINT ("CEGRO", ("trav from instruction"));
    }
    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("CEGRO", ("trav into next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("CEGRO", ("trav from next"));
    }
    /* bottom-up traversal */

    if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_ex)
        || (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_st)
        || (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_mt)) {
        if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_ex) {
            ASSIGN_NEXT (BLOCK_INSTR (EX_REGION (ASSIGN_INSTR (arg_node))))
              = ASSIGN_NEXT (arg_node);
        } else if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_st) {
            ASSIGN_NEXT (BLOCK_INSTR (ST_REGION (ASSIGN_INSTR (arg_node))))
              = ASSIGN_NEXT (arg_node);
        } else if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_mt) {
            ASSIGN_NEXT (BLOCK_INSTR (MT_REGION (ASSIGN_INSTR (arg_node))))
              = ASSIGN_NEXT (arg_node);
        }
        ASSIGN_NEXT (arg_node) = INFO_CEGRO_NEXTCELL (arg_info);

        INFO_CEGRO_NEXTCELL (arg_info) = arg_node;
        arg_node = NULL;
    }

    DBUG_RETURN (arg_node);
}
