/*
 *
 * $Log$
 * Revision 1.5  1995/11/01 16:33:55  cg
 * Now, additional parameters and return values are concerned in
 * function applications as well as in function definitions.
 *
 * Revision 1.4  1995/11/01  09:38:08  cg
 * Now, the special meaning of the return type T_void is considered.
 *
 * Revision 1.3  1995/11/01  08:30:25  cg
 * some bug fixes in using nnode.
 *
 * Revision 1.2  1995/10/31  17:39:12  cg
 * first compilable revision.
 *
 * Revision 1.1  1995/10/31  17:22:05  cg
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
#include "internal_lib.h"

#include <string.h>

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

        /*-------------------------------------------------------------*/
        if (FUNDEF_ARGS (arg_node) == NULL) {
            arg_node->nnode += 1;
            new_arg->nnode = 0;
        }
        /*-------------------------------------------------------------*/

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

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
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
                     + strlen (mod_name_con) + 1);

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
    node *new_return_expr, *ret;
    types *new_return_type;

    DBUG_ENTER ("OBJarg");

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    if (ARG_ATTRIB (arg_node) == ST_reference) {
        ret = FUNDEF_RETURN (arg_info);

        if (ret != NULL) {
            new_return_expr = MakeId (ARG_NAME (arg_node), NULL, ST_artificial);
            new_return_expr = MakeExprs (new_return_expr, RETURN_EXPRS (ret));

            /*-------------------------------------------------------------*/
            if (RETURN_EXPRS (ret) == NULL) {
                ret->nnode += 1;
                new_return_expr->nnode = 1;
            }
            /*-------------------------------------------------------------*/

            RETURN_EXPRS (ret) = new_return_expr;
        }

        if (FUNDEF_BASETYPE (arg_info) == T_void) {
            FUNDEF_BASETYPE (arg_info) = ARG_BASETYPE (arg_node);
            FUNDEF_DIM (arg_info) = ARG_DIM (arg_node);
            FUNDEF_SHPSEG (arg_info) = ARG_SHPSEG (arg_node);
            FUNDEF_TNAME (arg_info) = ARG_TNAME (arg_node);
            FUNDEF_TMOD (arg_info) = ARG_TMOD (arg_node);
        } else {
            new_return_type = MakeType (ARG_BASETYPE (arg_node), ARG_DIM (arg_node),
                                        ARG_SHPSEG (arg_node), ARG_TNAME (arg_node),
                                        ARG_TMOD (arg_node));
            TYPES_NEXT (new_return_type) = FUNDEF_TYPES (arg_info);
            FUNDEF_TYPES (arg_info) = new_return_type;
        }

        ARG_ATTRIB (arg_node) = ST_was_reference;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : OBJap
 *  arguments     : 1) pointer to N_ap node
 *                  2) arg_info unused
 *  description   : For each global object which is needed by the applied
 *                  function, a new current argument is added,
 *                  i.e. the global object is made local.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeId, MakeExprs, Trav
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
OBJap (node *arg_node, node *arg_info)
{
    nodelist *need_objs;
    node *obj, *new_arg;

    DBUG_ENTER ("OBJap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), NULL);
    }

    need_objs = FUNDEF_NEEDOBJS (AP_FUNDEF (arg_node));

    while (need_objs != NULL) {
        obj = NODELIST_NODE (need_objs);

        new_arg = MakeId (OBJDEF_VARNAME (obj), NULL, ST_artificial);

        new_arg = MakeExprs (new_arg, AP_ARGS (arg_node));

        /*-------------------------------------------------------------*/
        if (AP_ARGS (arg_node) == NULL) {
            arg_node->nnode += 1;
            new_arg->nnode = 1;
        }
        /*-------------------------------------------------------------*/

        AP_ARGS (arg_node) = new_arg;

        need_objs = NODELIST_NEXT (need_objs);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : OBJid
 *  arguments     : 1) pointer to N_id node
 *                  2) arg_info unused
 *  description   : For all applied appearances of global objects, the
 *                  "varname" is used as name instead of the combination
 *                  object and  module name.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : TREE, DBUG
 *
 *  remarks       :
 *
 */

node *
OBJid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("OBJid");

    if (ID_ATTRIB (arg_node) == ST_global) {
        ID_NAME (arg_node) = OBJDEF_VARNAME (ID_OBJDEF (arg_node));
        ID_MOD (arg_node) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : OBJlet
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
OBJlet (node *arg_node, node *arg_info)
{
    node *args, *params;
    ids *new_ids = NULL, *last_ids;

    DBUG_ENTER ("OBJlet");

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
        args = AP_ARGS (LET_EXPR (arg_node));
        params = FUNDEF_ARGS (AP_FUNDEF (LET_EXPR (arg_node)));

        while (params != NULL) {
            if (ARG_ATTRIB (params) == ST_was_reference) {
                if (new_ids == NULL) {
                    new_ids = MakeIds (ID_NAME (EXPRS_EXPR (args)), NULL, ST_artificial);
                    last_ids = new_ids;
                } else {
                    IDS_NEXT (last_ids)
                      = MakeIds (ID_NAME (EXPRS_EXPR (args)), NULL, ST_artificial);
                    last_ids = IDS_NEXT (last_ids);
                }
            }
            args = EXPRS_NEXT (args);
            params = ARG_NEXT (params);
        }

        LET_IDS (arg_node) = AppendIdsChain (new_ids, LET_IDS (arg_node));
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
