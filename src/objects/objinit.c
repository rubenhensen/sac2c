/*
 *
 * $Log$
 * Revision 1.3  1995/10/18 13:35:36  cg
 * now Malloc is used instead of malloc,
 * so error messages are no longer needed.
 *
 * Revision 1.2  1995/10/17  08:28:15  cg
 * all automatically generated functions now have status ST_objinitfun.
 * This tag is used by the typechecker.
 *
 * Revision 1.1  1995/10/16  12:22:44  cg
 * Initial revision
 *
 *
 *
 */

#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"

/*
 *
 *  functionname  : OImodul
 *  arguments     : 1) pointer to N_modul node
 *                  2) arg_info unused
 *  description   : traverses all global object definitions
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
OImodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("OImodul");

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_node);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : OIobjdef
 *  arguments     : 1) pointer to objdef node
 *                  2) pointer to modul node (head of syntax_tree)
 *  description   : A new function definition is generated that has no
 *                  formal parameters, the return type is identical to
 *                  the type of the global object and the initialization
 *                  expression is moved to the return-statement of the
 *                  new function. In the objdef node it's replaced by
 *                  an application of the new function.
 *
 *                  This mechanism allows for arbitrary expression to
 *                  initialize global objects.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Malloc, strlen, strcpy, strcat,
 *                  MakeType, MakeExprs, MakeReturn, MakeAssign, MakeBlock,
 *                  MakeFundef, MakeAp, Trav
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
OIobjdef (node *arg_node, node *arg_info)
{
    node *new_node;
    types *new_fun_type;
    char *new_fun_name;

    DBUG_ENTER ("OIobjdef");

    new_fun_type = MakeType (OBJDEF_BASETYPE (arg_node), OBJDEF_DIM (arg_node),
                             OBJDEF_SHPSEG (arg_node), OBJDEF_TNAME (arg_node),
                             OBJDEF_TMOD (arg_node));

    new_fun_name = (char *)Malloc (strlen (OBJDEF_NAME (arg_node)) + 10);

    new_fun_name = strcpy (new_fun_name, "_CREATE_");
    new_fun_name = strcat (new_fun_name, OBJDEF_NAME (arg_node));

    new_node = MakeExprs (OBJDEF_EXPR (arg_node), NULL);
    NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

    /*------------------------------------------------------------------*/
    new_node->nnode = 1;
    /*------------------------------------------------------------------*/

    new_node = MakeReturn (new_node);
    NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

    new_node = MakeAssign (new_node, NULL);
    NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

    /*------------------------------------------------------------------*/
    new_node->nnode = 1;
    /*------------------------------------------------------------------*/

    new_node = MakeBlock (new_node, NULL);
    NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

    /*------------------------------------------------------------------*/
    new_node->nnode = 1;
    /*------------------------------------------------------------------*/

    new_node = MakeFundef (new_fun_name, OBJDEF_MOD (arg_node), NULL, new_fun_type, NULL,
                           new_node, MODUL_FUNS (arg_info));
    NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

    /*------------------------------------------------------------------*/
    new_node->nnode = (MODUL_FUNS (arg_info) == NULL) ? 1 : 2;
    /*------------------------------------------------------------------*/

    FUNDEF_STATUS (new_node) = ST_objinitfun;

    /*
     * The new functions have status ST_objinitfun.
     * This tag is needed by the typechecker to ensure that these functions
     * are actually typechecked even if they are exclusively applied in
     * a global object initialization of a SAC-program.
     * In the typechecker, their status is changed to ST_regular.
     */

    MODUL_FUNS (arg_info) = new_node;

    new_node = MakeAp (new_fun_name, OBJDEF_MOD (arg_node), NULL);
    OBJDEF_EXPR (arg_node) = new_node;

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : objinit
 *  arguments     : 1) syntax_tree (pointer to N_modul node
 *  description   : starts the traversal mechanism for the objinit
 *                  compilation phase. Here, all initialization expressions
 *                  of global objects are moved to new generated functions
 *                  and replaced by applications of the respective functions.
 *  global vars   : act_tab, objinit_tab
 *  internal funs : ---
 *  external funs : Trav
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
objinit (node *syntax_tree)
{
    DBUG_ENTER ("objinit");

    act_tab = objinit_tab;

    DBUG_RETURN (Trav (syntax_tree, NULL));
}
