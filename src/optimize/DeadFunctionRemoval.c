/*
 *
 * $Log$
 * Revision 3.14  2004/11/19 10:17:34  sah
 * objinitfuns are never removed
 *
 * Revision 3.13  2004/10/26 09:31:35  sah
 * uses new PROVIDED/EXPORTED flags in newast mode now
 *
 * Revision 3.12  2004/09/02 17:48:32  skt
 * added warning for use of DFR with SSA-form
 *
 * Revision 3.11  2004/08/09 08:31:47  ktr
 * Fold functions are no longer removed in EMM as they are used by MT.
 *
 * Revision 3.10  2004/08/04 17:06:28  ktr
 * In EMM, Functions can be dead although they are referenced by a fold-withop
 *
 * Revision 3.9  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 3.8  2004/06/03 09:08:05  khf
 * DFRwithop(): traverse into NWITHOP_NEXT added
 *
 * Revision 3.7  2004/02/20 08:18:59  mwe
 * now functions with (MODUL_FUNS) and without (MODUL_FUNDECS) body are separated
 * changed tree traversal according to that
 *
 * Revision 3.6  2003/11/18 17:41:30  dkr
 * no changes done
 *
 * Revision 3.5  2001/05/02 11:13:57  nmw
 * missing initialization added
 *
 * Revision 3.4  2001/05/02 09:09:53  nmw
 * change implementation to be more agressive when removing dead functions
 * from SAC programs
 *
 * Revision 3.3  2001/03/22 21:11:25  dkr
 * include of tree.h eliminated
 *
 * Revision 3.2  2000/11/23 16:16:50  sbs
 * mem_dead_fun in DeadFunctionRemoval enclosed in ifndef DBUG_OFF
 * for avoiding a warning when compiling product version.
 *
 * Revision 3.1  2000/11/20 18:00:29  sacbase
 * new release made
 *
 * Revision 2.5  2000/10/31 18:08:09  cg
 * Dead function removal completely re-implemented.
 *
 * Revision 2.4  2000/07/14 12:59:25  dkr
 * DFRfundef: body of function is always traversed now
 *
 * Revision 2.3  2000/07/14 12:05:14  dkr
 * DFRblock() added
 *
 * Revision 2.2  2000/01/26 17:26:59  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.1  1999/02/23 12:41:13  sacbase
 * new release made
 *
 * Revision 1.3  1999/01/20 09:07:46  cg
 * Dead function removal may now handle programs without any functions.
 *
 * Revision 1.2  1999/01/18 15:46:02  sbs
 * DBUG_PRINT( "OPTMEM",...) inserted for mem-info during optimization
 *
 * Revision 1.1  1999/01/07 17:36:51  sbs
 * Initial revision
 *
 */

#define NEW_INFO

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "globals.h"
#include "my_debug.h"
#include "traverse.h"

#include "optimize.h"

/*
 * INFO structure
 */
struct INFO {
    int flag;
};

/*
 * INFO macros
 */
#define INFO_DFR_SPINE(n) (n->flag)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_DFR_SPINE (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * Function:
 *   node *DeadFunctionRemoval( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DeadFunctionRemoval (node *arg_node)
{
    funtab *tmp_tab;
    info *arg_info;
#ifndef DBUG_OFF
    int mem_dead_fun = dead_fun;
#endif

    DBUG_ENTER ("DeadFunctionRemoval");
    DBUG_PRINT ("OPT", ("DEAD FUNCTION REMOVAL"));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    tmp_tab = act_tab;
    act_tab = dfr_tab;

    arg_info = MakeInfo ();
    arg_node = Trav (arg_node, arg_info);
    arg_info = FreeInfo (arg_info);

    DBUG_PRINT ("OPT", ("                        result: %d", dead_fun - mem_dead_fun));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFRmodul(node *arg_node,info *arg_info)
 *
 * Description:
 *   Prevents DFR in modules
 *   in programs the DFR starts in fundef main.
 *
 ******************************************************************************/

node *
DFRmodul (node *arg_node, info *arg_info)
{
    node *fun;
    DBUG_ENTER ("DFRmodul");

    if (MODUL_FUNDECS (arg_node) != NULL) {
        /* clear dfr flag */
        fun = MODUL_FUNDECS (arg_node);
        while (fun != NULL) {
            FUNDEF_EXPORT (fun) = FALSE;
            fun = FUNDEF_NEXT (fun);
        }

        /* check fundefs for applications (only main in programs) */
        /*    INFO_DFR_SPINE( arg_info) = TRUE;
            MODUL_FUNDECS( arg_node) = Trav( MODUL_FUNDECS( arg_node), arg_info);*/

        /* remove all produced zombies */
        /*    MODUL_FUNDECS(arg_node) = RemoveAllZombies( MODUL_FUNDECS(arg_node));*/
    }

    if (MODUL_FUNS (arg_node) != NULL) {
        /* clear dfr flag */
        fun = MODUL_FUNS (arg_node);
        while (fun != NULL) {
            FUNDEF_EXPORT (fun) = FALSE;
            fun = FUNDEF_NEXT (fun);
        }

        /* check fundefs for applications (only main in programs) */
        INFO_DFR_SPINE (arg_info) = TRUE;
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);

        /* remove all produced zombies */
        MODUL_FUNS (arg_node) = RemoveAllZombies (MODUL_FUNS (arg_node));
    }

    if (MODUL_FUNDECS (arg_node) != NULL) {

        /* remove all produced zombies */
        MODUL_FUNDECS (arg_node) = RemoveAllZombies (MODUL_FUNDECS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFRfundef( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses instruction- and function-chain in this order.
 *
 ******************************************************************************/

node *
DFRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DFRfundef");

    if (INFO_DFR_SPINE (arg_info)) {
        DBUG_PRINT ("DFR",
                    ("Dead Function Removal in function: %s", FUNDEF_NAME (arg_node)));

        /* a warning for using DFR with SSA */
        if ((FUNDEF_STATUS (arg_node) == ST_dofun)
            || (FUNDEF_STATUS (arg_node) == ST_whilefun)
            || (FUNDEF_STATUS (arg_node) == ST_condfun)) {
            if (FUNDEF_USED (arg_node) > 1) {
                SYSWARN (("Lac-functions, which are used more than once aren't handled "
                          "correctly by DeadFunctionRemoval"));
            }
        }

        /* remark: main is always tagged as ST_exported! */
#ifndef NEW_AST
        if ((FUNDEF_STATUS (arg_node) == ST_exported)
            || (FUNDEF_STATUS (arg_node) == ST_objinitfun)) {
#else
        if ((GET_FLAG (FUNDEF, arg_node, IS_PROVIDED))
            || (FUNDEF_STATUS (arg_node) == ST_objinitfun)) {
#endif
            FUNDEF_EXPORT (arg_node) = TRUE;

            if (FUNDEF_BODY (arg_node) != NULL) {
                INFO_DFR_SPINE (arg_info) = FALSE;
                FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
                INFO_DFR_SPINE (arg_info) = TRUE;
            }
        }

        /* traverse next fundef */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }

        /* on bottom up traversal turn unused fundefs into zombies */
        if (!(FUNDEF_EXPORT (arg_node))) {
            dead_fun++;
            arg_node = FreeNode (arg_node);
        }
    } else {
        if (!FUNDEF_EXPORT (arg_node)) {
            FUNDEF_EXPORT (arg_node) = TRUE;
            if (FUNDEF_BODY (arg_node) != NULL) {
                FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFRblock( node *arg_node, info *arg_info)
 *
 * Description:
 *   Removes BLOCK_NEEDFUNS!
 *
 ******************************************************************************/

node *
DFRblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DFRblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    /*
     * After DFR the data stored in BLOCK_NEEDFUNS is possibly corrupted!
     * -> remove it!
     */
    if (BLOCK_NEEDFUNS (arg_node) != NULL) {
        BLOCK_NEEDFUNS (arg_node) = FreeNodelist (BLOCK_NEEDFUNS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFRap( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFRap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DFRap");

    AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), arg_info);

#if 0
  arg_node = TravSons( arg_node, arg_info);
#endif

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFRwithop( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFRwithop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DFRwithop");

    if (NWITHOP_TYPE (arg_node) == WO_foldfun) {
        NWITHOP_FUNDEF (arg_node) = Trav (NWITHOP_FUNDEF (arg_node), arg_info);
    }

    if (NWITHOP_NEXT (arg_node) != NULL) {
        NWITHOP_NEXT (arg_node) = Trav (NWITHOP_NEXT (arg_node), arg_info);
    }

#if 0
  arg_node = TravSons( arg_node, arg_info);
#endif

    DBUG_RETURN (arg_node);
}
