/*
 *
 * $Log$
 * Revision 1.4  1997/09/05 17:44:38  dkr
 * fixed a bug in function RMVassign with nnode
 *
 * Revision 1.3  1997/09/05 13:46:04  cg
 * All cast expressions are now removed by rmvoidfun.c. Therefore,
 * the respective attempts in precompile.c and ConstantFolding.c
 * are removed. Cast expressions are only used by the type checker.
 * Afterwards, they are useless, and they are not supported by
 * Constant Folding as well as code generation.
 *
 * Revision 1.2  1997/03/19 15:31:08  cg
 * Now, module/class implementations without any functions are supported
 *
 * Revision 1.1  1995/11/16  19:47:38  cg
 * Initial revision
 *
 *
 *
 */

/*
 *  sac2c compiler module: rmvoidfun
 *
 *  The purpose of this compiler phase is to generate purely functional
 *  code. What actually is done is removing all function declarations
 *  as well as definitions with return type "void". Although, this seems
 *  to be a task for dead code removal, this is done separately because
 *  the implementation of the SAC optimizations assumes all functions
 *  to have at least one return value.
 *
 *  Additionally, all cast expressions are removed by this compiler phase.
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "dbug.h"
#include "traverse.h"
#include "internal_lib.h"
#include "Error.h"
#include "free.h"

/*
 *
 *  functionname  : RemoveVoidFunctions
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
RemoveVoidFunctions (node *syntax_tree)
{
    DBUG_ENTER ("RemoveVoidFunctions");

    act_tab = rmvoid_tab;

    DBUG_RETURN (Trav (syntax_tree, NULL));
}

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

/*
 *
 *  functionname  : RMVblock
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
RMVblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RMVblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

        if (BLOCK_INSTR (arg_node) == NULL) {
            BLOCK_INSTR (arg_node) = MakeEmpty ();
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RMVassign
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
RMVassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RMVassign");

    while ((arg_node != NULL) && (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)
           && (LET_IDS (ASSIGN_INSTR (arg_node)) == NULL)) {
        /* remove all void-functions */
        arg_node = FreeNode (arg_node);
    }

    if (arg_node != NULL) {
        if (ASSIGN_INSTR (arg_node) != NULL) {
            /* travers the instruction of the actual assignment to remove the casts */
            ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        }

        if (ASSIGN_NEXT (arg_node) != NULL) {
            /* if another assignment follows, travers this one */
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

            /*-------------------------------------------------------------------*/
#ifndef NEWTREE
            if (ASSIGN_NEXT (arg_node) == NULL) {
                /* if the following assignment had been removed, fix the node-counter */
                arg_node->nnode -= 1;
            }
#endif
            /*-------------------------------------------------------------------*/
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RMVfundef
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
RMVfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RMVfundef");

    while ((arg_node != NULL) && (FUNDEF_BASETYPE (arg_node) == T_void)) {
        DBUG_PRINT ("RMVOID", ("Removed void function %s", ItemName (arg_node)));

        arg_node = FreeNode (arg_node);
    }

    if (arg_node != NULL) {
        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        }

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);

            /*-------------------------------------------------------------------*/
#ifndef NEWTREE
            if (FUNDEF_NEXT (arg_node) == NULL) {
                arg_node->nnode -= 1;
            }
#endif
            /*-------------------------------------------------------------------*/
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RMVmodul
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
RMVmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RMVmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RMVcast
 *  arguments     : 1) N_cast node
 *                  2) arg_info unused
 *  description   : deletes all casts
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav, FreeOneTypes
 *  macros        : DBUG, TREE, FREE
 *
 *  remarks       :
 *
 */

node *
RMVcast (node *arg_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("RMVcast");

    tmp = arg_node;
    arg_node = Trav (CAST_EXPR (arg_node), arg_info);

    FreeOneTypes (CAST_TYPE (tmp));
    FREE (tmp);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */
