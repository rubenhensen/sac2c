/*
 *
 * $Log$
 * Revision 1.1  1995/10/31 17:22:05  cg
 * Initial revision
 *
 *
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "dbug.h"
#include "traverse.h"

/*
 *
 *  functionname  : HandleObjects
 *  arguments     : 1) syntax tree
 *  description   : starts traversal mechanism for handling objects.
 *                  For each function, the global objects needed are added
 *                  to the parameter list as reference parameters.
 *                  Afterwards all reference parameters are converted to
 *                  ordinary parameters and additional return values
 *                  are added in the function's return type list and in
 *                  the return-statement itself.
 *  global vars   : act_tab, obj_tab
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG
 *
 *  remarks       :
 *
 */

node *
HandleObjects (node *syntax_tree)
{
    DBUG_ENTER ("HandleObjects");

    act_tab = obj_tab;

    DBUG_RETURN (Trav (syntax_tree, NULL));
}

/*
 *
 *  functionname  : OBJmodul
 *  arguments     : 1) pointer to N_modul node of syntax tree
 *                  2) arg_info unused
 *  description   : traverses all global objects for adding varnames
 *                  and then the functions for adding additional parameters
 *                  and return values
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
OBJmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("OBJmodul");

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_info);
    }

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : OBJfundef
 *  arguments     : 1) N_fundef node
 *                  2) arg_info unused
 *  description   : For each needed global object an additional parameter
 *                  is added to the function's parameter list with
 *                  status 'ST_artificial' and attribute 'ST_reference'
 *                  Afterwards, the modofied parameter list is traversed.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeType, MakeArg, Trav
 *  macros        : TREE, DBUG
 *
 *  remarks       :
 *
 */

node *
OBJfundef (node *arg_node, node *arg_info)
{
    nodelist *need_objs;
    node *obj, *new_arg;
    types *new_type;
    char *keep_name, *keep_mod;
    statustype keep_status;

    DBUG_ENTER ("OBJfundef");

    need_objs = FUNDEF_NEEDOBJS (arg_node);

    while (need_objs != NULL) {
        obj = NODELIST_NODE (need_objs);

        new_type = MakeType (OBJDEF_BASETYPE (obj), OBJDEF_DIM (obj), OBJDEF_SHPSEG (obj),
                             OBJDEF_TNAME (obj), OBJDEF_TMOD (obj));

        new_arg = MakeArg (OBJDEF_VARNAME (obj), new_type, ST_artificial, ST_reference,
                           FUNDEF_ARGS (arg_node));

        FUNDEF_ARGS (arg_node) = new_arg;

        need_objs = NODELIST_NEXT (need_objs);
    }

    /*
     *  The following assignments are necessary due to the old data
     *  structure which stores names and status within the types structure.
     */

    /*-------------------------------------------------------------*/
    keep_name = FUNDEF_NAME (arg_node);
    keep_mod = FUNDEF_MOD (arg_node);
    keep_status = FUNDEF_STATUS (arg_node);
    /*-------------------------------------------------------------*/

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_node);
    }

    /*-------------------------------------------------------------*/
    FUNDEF_NAME (arg_node) = keep_name;
    FUNDEF_MOD (arg_node) = keep_mod;
    FUNDEF_STATUS (arg_node) = keep_status;
    /*-------------------------------------------------------------*/

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : OBJobjdef
 *  arguments     : 1) N_objdef node
 *                  2) arg_info unused
 *  description   : Each object definition is associated with a new name,
 *                  called the varname, which is generated from the
 *                  object's original name and its module name.
 *                  This name is used when making a global object local.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strlen, strcpy, strcat, Trav, Malloc
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
OBJobjdef (node *arg_node, node *arg_info)
{
    char *buffer;

    DBUG_ENTER ("OBJobjdef");

    buffer = Malloc (strlen (OBJDEF_NAME (arg_node)) + strlen (OBJDEF_MOD (arg_node))
                     + 2 * strlen (mod_name_con) + 1);

    strcpy (buffer, mod_name_con);
    strcat (buffer, MOD (OBJDEF_MOD (arg_node)));
    strcat (buffer, mod_name_con);
    strcat (buffer, OBJDEF_NAME (arg_node));

    OBJDEF_VARNAME (arg_node) = buffer;

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), NULL);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : OBJarg
 *  arguments     : 1) N_arg node
 *                  2) N_fundef node to which this argument belongs to.
 *  description   : For each reference parameter, a new expression is
 *                  generated containing the parameter name and is added
 *                  to the list of return expressions.
 *                  A new types structure is generated as well and added
 *                  to the function's list of return types
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeId, MakeExprs, MakeType, Trav
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
OBJarg (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("OBJarg");

    if (ARG_ATTRIB (arg_node) == ST_reference) {
        new_return_expr = MakeId (ARG_NAME (arg_node), NULL, ST_artificial);
        ret = FUNDEF_RETURN (arg_info);
        RETURN_EXPRS (ret) = MakeExprs (new_return_expr, RETURN_EXPRS (ret));

        new_return_type
          = MakeType (ARG_BASETYPE (arg_node), ARG_DIM (arg_node), ARG_SHPSEG (arg_node),
                      ARG_TNAME (arg_node), ARG_TMOD (arg_node));
        TYPES_NEXT (new_return_type) = FUNDEF_TYPES (arg_info);
        FUNDEF_TYPES (arg_info) = new_return_type;
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

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
