/*
 *
 * $Log$
 * Revision 1.3  1999/01/20 09:07:46  cg
 * Dead function removal may now handle programs without any functions.
 *
 * Revision 1.2  1999/01/18 15:46:02  sbs
 * DBUG_PRINT( "OPTMEM",...) inserted for mem-info during optimization
 *
 * Revision 1.1  1999/01/07 17:36:51  sbs
 * Initial revision
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "globals.h"
#include "my_debug.h"
#include "traverse.h"

#include "optimize.h"

node *
DeadFunctionRemoval (node *arg_node, node *info_node)

{
    funptr *tmp_tab;
    int mem_dead_fun = dead_fun;

    DBUG_ENTER ("DeadFunctionRemoval");
    DBUG_PRINT ("OPT", ("DEAD FUNCTION REMOVAL"));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    tmp_tab = act_tab;
    act_tab = dfr_tab;
    info_node = MakeNode (N_info);
    arg_node = Trav (arg_node, info_node);
    FREE (info_node);

    DBUG_PRINT ("OPT", ("                        result: %d", dead_fun - mem_dead_fun));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    act_tab = tmp_tab;
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DFRmodul
 *  arguments     : 1) N_modul - node
 *                  R) N_modul - node
 *  description   : Prevents DFR in modules
 *  global vars   : syntax_tree,
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG..., MODUL_FILETYPE
 *
 *  remarks       :
 *
 *
 */
node *
DFRmodul (node *arg_node, node *info_node)
{
    DBUG_ENTER ("DFRmodul");

    if ((F_prog == MODUL_FILETYPE (arg_node)) && (MODUL_FUNS (arg_node) != NULL)) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), info_node);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DFRfundef
 *  arguments     : 1) N_fundef - node
 *                  2) N_info - node
 *                  R) N_fundef - node
 *  description   : Traverses instruction- (if not inline marked) and function-chain
 *                  in this sequence.
 *  global vars   : --
 *  internal funs : --
 *  external funs : Trav (optimize.h)
 *  macros        : FUNDEF_NAME, FUNDEF_BODY, FUNDEF_INSTR, FUNDEF_NEXT
 *
 *  remarks       : --
 *
 */
node *
DFRfundef (node *arg_node, node *arg_info)
{
    node *nextfun;

    DBUG_ENTER ("DFRfundef");
    DBUG_PRINT ("DFR", ("Dead Function Removal in function: %s", FUNDEF_NAME (arg_node)));

    if ((NULL != FUNDEF_BODY (arg_node)) && (0 == FUNDEF_INLINE (arg_node)))
        FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);

    if (NULL != FUNDEF_NEXT (arg_node))
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);

    if (1 == FUNDEF_INLINE (arg_node)) {
        dead_fun++;
        nextfun = FUNDEF_NEXT (arg_node);
        FUNDEF_NEXT (arg_node) = NULL;
        FreeTree (arg_node);
        arg_node = nextfun;
    }
    DBUG_RETURN (arg_node);
}

node *
DFRap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFRap");
    if ((1 == FUNDEF_INLINE (AP_FUNDEF (arg_node)))
        && (NULL != FUNDEF_INSTR (AP_FUNDEF (arg_node)))) {
        FUNDEF_INLINE (AP_FUNDEF (arg_node)) = 0;
        FUNDEF_INSTR (AP_FUNDEF (arg_node))
          = Trav (FUNDEF_INSTR (AP_FUNDEF (arg_node)), arg_info);
    }
    DBUG_RETURN (arg_node);
}
