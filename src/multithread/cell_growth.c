/*
 * $Log$
 * Revision 1.6  2004/11/24 19:40:47  skt
 * SACDevCampDK 2k4
 *
 * Revision 1.5  2004/11/23 20:52:11  skt
 * big compiler brushing during SACDevCampDK 2k4
 *
 * Revision 1.4  2004/11/23 14:38:13  skt
 * SACDevCampDK 2k4
 *
 * Revision 1.3  2004/11/22 13:48:10  skt
 * code brushing in SACDevCampDK 2004
 *
 * Revision 1.2  2004/08/17 15:47:39  skt
 * comments added
 *
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

#include "tree_basic.h"
#include "traverse.h"
#include "cell_growth.h"
#include "internal_lib.h"

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

    result = ILIBmalloc (sizeof (info));

    INFO_CEGRO_NEXTCELL (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *CEGROdoCellGrowth(node *arg_node)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_modul
 *   @return the N_modul which N_blocks have only cells of assignments and
 *           no single assignment left. The only exception is a block which
 *           assignments have just MUTH_ANY-assignments (they will be
 *           specialized in a following phase)
 *
 *****************************************************************************/
node *
CEGROdoCellGrowth (node *arg_node)
{
    info *arg_info;
    trav_t traversaltable;
    DBUG_ENTER ("CEGROdoCellGrowth");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "CEGROdoCellGrowth expects a N_module as arg_node");

    arg_info = MakeInfo ();

    TRAVpush (TR_cegro);

    DBUG_PRINT ("CEGRO", ("trav into module-funs"));
    MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    DBUG_PRINT ("CEGRO", ("trav from module-funs"));

    traversaltable = TRAVpop ();
    DBUG_ASSERT ((traversaltable == TR_cegro), "Popped incorrect traversal table");

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CEGROblock(node *arg_node, info *arg_info)
 *
 *   @brief Traverses into the assignment-chain to let the cells grow.
 *          Afterwards the block-leading MUTH_ANY assignment-chain is moved
 *          into the first cell of the block.
 *   <pre>
 *        example: let arg_node be
 *        N_block->N_assign(A)
 *                     |
 *                 N_assign->N_mt->N_block->N_assign(B)
 *                     |
 *                 N_assign(C)
 *
 *        the traversal will return
 *        N_block->N_assign(A)
 *                     |
 *                 N_assign->N_mt->N_block->N_assign(B)
 *                                              |
 *                                          N_assign(C)
 *
 *        and CEGROblock rebuilds it into
 *        N_block->N_assign->N_mt->N_block->N_assign(A)
 *                                              |
 *                                          N_assign(B)
 *                                              |
 *                                          N_assign(C)
 *   </pre>
 *   @param arg_node a N_block
 *   @param arg_info
 *   @return the N_block with full cells
 *
 *****************************************************************************/
node *
CEGROblock (node *arg_node, info *arg_info)
{
    node *xcell;
    node *iterator;
    node *old_nextcell;
    DBUG_ENTER ("CEGROblock");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_block), "arg_node is not a N_block");

    /* push info... */
    old_nextcell = INFO_CEGRO_NEXTCELL (arg_info);
    INFO_CEGRO_NEXTCELL (arg_info) = NULL;

    /* continue traversal */
    if (BLOCK_INSTR (arg_node) != NULL) {
        DBUG_PRINT ("CEGRO", ("trav into instruction(s)"));
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
        DBUG_PRINT ("CEGRO", ("trav from instruction(s)"));

        /* add the first assignments to the belonging cell, if their
         * executionmode is MUTH_ANY and there exists an cell inside
         * (see above comments for further details) */
        if ((ASSIGN_EXECMODE (BLOCK_INSTR (arg_node)) == MUTH_ANY)
            && (INFO_CEGRO_NEXTCELL (arg_info) != NULL)) {

            iterator = BLOCK_INSTR (arg_node);
            while (ASSIGN_NEXT (iterator) != NULL) {
                iterator = ASSIGN_NEXT (iterator);
            }
            /* due to the algorithm in CEGROassign iterator points to the last
             * assignment of the block, which isn't in a cell */
            DBUG_ASSERT ((ASSIGN_EXECMODE (iterator) == MUTH_ANY),
                         "the executionmode has to be MUTH_ANY");

            /* get the EX-/ST-/MT-Cell (to make the code clearer) */
            xcell = ASSIGN_INSTR (INFO_CEGRO_NEXTCELL (arg_info));

            if (NODE_TYPE (xcell) == N_ex) {
                ASSIGN_NEXT (iterator) = BLOCK_INSTR (EX_REGION (xcell));
                BLOCK_INSTR (EX_REGION (xcell)) = BLOCK_INSTR (arg_node);
            } else if (NODE_TYPE (xcell) == N_st) {
                ASSIGN_NEXT (iterator) = BLOCK_INSTR (ST_REGION (xcell));
                BLOCK_INSTR (ST_REGION (xcell)) = BLOCK_INSTR (arg_node);
            } else if (NODE_TYPE (xcell) == N_mt) {
                ASSIGN_NEXT (iterator) = BLOCK_INSTR (MT_REGION (xcell));
                BLOCK_INSTR (MT_REGION (xcell)) = BLOCK_INSTR (arg_node);
            }
            BLOCK_INSTR (arg_node) = INFO_CEGRO_NEXTCELL (arg_info);
        }
    }

    /* pop info... */
    INFO_CEGRO_NEXTCELL (arg_info) = old_nextcell;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CEGROassign(node *arg_node, info *arg_info)
 *
 *   @brief  fills the existing EX-/ST-/MT-cells with the following assignments
 *           with the same executionmode/MUTH_ANY-executionmode
 *   <pre>
 *     example: let arg_node be
 *      N_assign(A)
 *         |
 *      N_assign->N_st->N_block->N_assign(B)
 *         |
 *      N_assign(C)
 *         |
 *      N_assign->N_mt->N_block->N_assign(D)
 *         |
 *      N_assign(E)
 *         |
 *      N_assign(F)
 *
 *      this will lead into (N_assign(A) will be handled in CECROblock)
 *      N_assign(A)
 *         |
 *      N_assign->N_st->N_block->N_assign(B)
 *         |                        |
 *         |                     N_assign(C)
 *         |
 *      N_assign->N_mt->N_block->N_assign(D)
 *                                  |
 *                               N_assign(E)
 *                                  |
 *                               N_assign(F)*
 *   @param arg_node a N_assign
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CEGROassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CEGROassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), "arg_node is no a N_assign");

    /* traverse into the instruction - it could be a conditional */
    if (ASSIGN_INSTR (arg_node) != NULL) {
        DBUG_PRINT ("CEGRO", ("trav into instruction"));
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
        DBUG_PRINT ("CEGRO", ("trav from instruction"));
    }
    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("CEGRO", ("trav into next"));
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
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
