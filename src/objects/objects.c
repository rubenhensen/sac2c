/*
 *
 * $Log$
 * Revision 1.14  1996/04/25 11:31:36  cg
 * bug fixed, now even in bodies of with-loops new return values are bound to new
 * variables.
 *
 * Revision 1.13  1996/04/02  19:39:25  cg
 * bug fixed in function OBJlet:
 * runs now with functions with variable argument lists.
 *
 * Revision 1.12  1996/01/22  18:40:07  cg
 * All object definitions are rearranged in a sequence that allows their
 * sequential initialization. If this is not possible, then an error
 * message occurrs.
 *
 * Revision 1.11  1996/01/21  16:02:47  cg
 * bug fixed in OBJobjdef
 *
 * Revision 1.10  1995/12/28  10:28:10  cg
 * bug fixed in OBJarg: names of artificial return types won't be shared
 * in any situation now.
 *
 * Revision 1.9  1995/12/01  17:23:56  cg
 * now shape segments and strings are always copied when generated
 * from existing nodes.
 *
 * Revision 1.8  1995/11/16  19:45:50  cg
 * Some bug fixes
 *
 * Revision 1.7  1995/11/06  14:20:20  cg
 * bug fixed in generating correct references of identifiers
 * to their respective vardec or arg nodes
 *
 * Revision 1.6  1995/11/02  13:14:40  cg
 * Now, the necessary references to variable declarations or
 * function parameters respectively are generated for all
 * additional identifiers.
 *
 * Revision 1.5  1995/11/01  16:33:55  cg
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
#include "Error.h"

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
 *  functionname  : InsertIntoInitlist
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

int
InsertIntoInitlist (node *objdef, nodelist **already_done)
{
    nodelist *needed, *tmp;
    int success = 1;

    DBUG_ENTER ("InsertIntoInitlist");

    if (OBJDEF_NEEDOBJS (objdef) != NULL) {
        needed = OBJDEF_NEEDOBJS (objdef);
    } else {
        needed = FUNDEF_NEEDOBJS (AP_FUNDEF (OBJDEF_EXPR (objdef)));
    }

    while (needed != NULL) {
        tmp = *already_done;

        while (tmp != NULL) {
            if (NODELIST_NODE (needed) == NODELIST_NODE (tmp)) {
                break;
            }

            tmp = NODELIST_NEXT (tmp);
        }

        if (tmp == NULL) {
            success = 0;
            break;
        }

        needed = NODELIST_NEXT (needed);
    }

    if (success) {
        *already_done = MakeNodelist (objdef, ST_regular, *already_done);
    }

    DBUG_RETURN (success);
}

/*
 *
 *  functionname  : RearrangeObjdefs
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
RearrangeObjdefs (node *objects)
{
    node *tmp, *first, *last;
    nodelist *already_done = NULL;
    int inserted, not_ready;

    DBUG_ENTER ("RearrangeObjdefs");

    do {
        inserted = 0;
        not_ready = 0;
        tmp = objects;

        while (tmp != NULL) {
            if (OBJDEF_ATTRIB (tmp) == ST_regular) {
                not_ready = 1;

                if (1 == InsertIntoInitlist (tmp, &already_done)) {
                    inserted = 1;
                    OBJDEF_ATTRIB (tmp) = ST_resolved;

                    DBUG_PRINT ("OBJ", ("Object %s added to init list", ItemName (tmp)));
                }
            }

            tmp = OBJDEF_NEXT (tmp);
        }
    } while (inserted);

    if (not_ready) {
        ERROR (0, ("The following global objects cannot be initialized due "
                   "to mutual dependencies"));

        tmp = objects;

        while (tmp != NULL) {
            if (OBJDEF_ATTRIB (tmp) == ST_regular) {
                CONT_ERROR (("'%s`", ItemName (tmp)));
            }

            tmp = OBJDEF_NEXT (tmp);
        }

        ABORT_ON_ERROR;
    }

    DBUG_ASSERT (already_done != NULL, "RearrangeObjdefs called with 0 objects");

    last = NODELIST_NODE (already_done);
    OBJDEF_NEXT (last) = NULL;
    already_done = NODELIST_NEXT (already_done);
    first = last;

    DBUG_PRINT ("OBJ", ("Rearranging object %s", ItemName (first)));

    /***************************************/
#ifndef NEWTREE
    last->nnode = 1;
#endif
    /***************************************/

    while (already_done != NULL) {
        first = NODELIST_NODE (already_done);

        DBUG_PRINT ("OBJ", ("Rearranging object %s", ItemName (first)));

        OBJDEF_NEXT (first) = last;

        /***************************************/
#ifndef NEWTREE
        last->nnode = 2;
#endif
        /***************************************/

        last = first;

        already_done = NODELIST_NEXT (already_done);
    }

    DBUG_RETURN (first);
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

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = RearrangeObjdefs (MODUL_OBJS (arg_node));
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
 *                  Afterwards, the modified parameter list is traversed.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeType, MakeArg, Trav
 *  macros        : TREE, DBUG
 *
 *  remarks       : The function body is traversed twice, because for
 *                  adding additional parameters to function applications
 *                  or making global arguments local a reference to the
 *                  new argument node of the function is needed.
 *                  This is cannot be obtained easily in direct way.
 *                  Therefore, this reference is stored in the respective
 *                  objdef node when the new arg node is generated.
 *                  For this reason, the body must be traversed right
 *                  after the arguments of the same function to avoid
 *                  the overwriting of this information.
 *
 *                  Unfortunately, for resolving reference parameters it
 *                  is necessary that all function definitions have been
 *                  traversed before, because new reference parameters
 *                  are added for global objects.
 *
 *
 */

node *
OBJfundef (node *arg_node, node *arg_info)
{
    nodelist *need_objs;
    node *obj, *new_arg;
    types *new_type;
    char *keep_name, *keep_mod;
    statustype keep_status, keep_attrib;

    DBUG_ENTER ("OBJfundef");

    DBUG_PRINT ("OBJ", ("Handling function %s", ItemName (arg_node)));

    need_objs = FUNDEF_NEEDOBJS (arg_node);

    while (need_objs != NULL) {
        obj = NODELIST_NODE (need_objs);

        new_type
          = MakeType (OBJDEF_BASETYPE (obj), OBJDEF_DIM (obj),
                      CopyShpseg (OBJDEF_SHPSEG (obj)), StringCopy (OBJDEF_TNAME (obj)),
                      StringCopy (OBJDEF_TMOD (obj)));

        new_arg = MakeArg (StringCopy (OBJDEF_VARNAME (obj)), new_type, ST_artificial,
                           ST_reference, FUNDEF_ARGS (arg_node));

        NODE_LINE (new_arg) = NODE_LINE (arg_node);
        ARG_OBJDEF (new_arg) = obj;

        /*-------------------------------------------------------------*/
        if (FUNDEF_ARGS (arg_node) == NULL) {
            arg_node->nnode += 1;
            new_arg->nnode = 0;
        }
        /*-------------------------------------------------------------*/

        FUNDEF_ARGS (arg_node) = new_arg;
        OBJDEF_ARG (obj) = new_arg;

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
    keep_attrib = FUNDEF_ATTRIB (arg_node);
    /*-------------------------------------------------------------*/

    DBUG_PRINT ("OBJ", ("Traversing args of function %s", ItemName (arg_node)));

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_node);
    }

    /*-------------------------------------------------------------*/
    FUNDEF_NAME (arg_node) = keep_name;
    FUNDEF_MOD (arg_node) = keep_mod;
    FUNDEF_STATUS (arg_node) = keep_status;
    FUNDEF_ATTRIB (arg_node) = keep_attrib;
    /*-------------------------------------------------------------*/

    DBUG_PRINT ("OBJ", ("Traversing body of function %s", ItemName (arg_node)));

    /*
     *  In a first traversal of the function body, additional arguments
     *  for function applications are inserted.
     */

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), NULL);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     *  In a second traversal of the function body, additional return values
     *  caused by reference parameters are bound to the respective variables.
     */

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_node);
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

    buffer = Malloc (strlen (OBJDEF_NAME (arg_node))
                     + strlen (MOD (OBJDEF_MOD (arg_node))) + strlen (mod_name_con) + 1);

    strcpy (buffer, MOD (OBJDEF_MOD (arg_node)));
    strcat (buffer, mod_name_con);
    strcat (buffer, OBJDEF_NAME (arg_node));

    DBUG_PRINT ("OBJ", ("Generating varname %s for %s", buffer, ItemName (arg_node)));

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
            new_return_expr
              = MakeId (StringCopy (ARG_NAME (arg_node)), NULL, ST_artificial);
            ID_VARDEC (new_return_expr) = arg_node;
            NODE_LINE (new_return_expr) = NODE_LINE (ret);

            DBUG_PRINT ("OBJ", ("New return value: %s", ARG_NAME (arg_node)));

            new_return_expr = MakeExprs (new_return_expr, RETURN_EXPRS (ret));
            NODE_LINE (new_return_expr) = NODE_LINE (ret);

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
            FUNDEF_SHPSEG (arg_info) = CopyShpseg (ARG_SHPSEG (arg_node));
            FUNDEF_TNAME (arg_info) = StringCopy (ARG_TNAME (arg_node));
            FUNDEF_TMOD (arg_info) = ARG_TMOD (arg_node);

            DBUG_PRINT ("OBJ", ("Converted return type void to %s:%s",
                                FUNDEF_TMOD (arg_info), FUNDEF_TNAME (arg_info)));
        } else {
            new_return_type
              = MakeType (ARG_BASETYPE (arg_node), ARG_DIM (arg_node),
                          CopyShpseg (ARG_SHPSEG (arg_node)),
                          StringCopy (ARG_TNAME (arg_node)), ARG_TMOD (arg_node));
            TYPES_NEXT (new_return_type) = FUNDEF_TYPES (arg_info);
            FUNDEF_TYPES (arg_info) = new_return_type;

            DBUG_PRINT ("OBJ", ("Added return type %s:%s", FUNDEF_TMOD (arg_info),
                                FUNDEF_TNAME (arg_info)));
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

    DBUG_PRINT ("OBJ", ("Handling application of %s", ItemName (AP_FUNDEF (arg_node))));

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), NULL);
    }

    need_objs = FUNDEF_NEEDOBJS (AP_FUNDEF (arg_node));

    while (need_objs != NULL) {
        obj = NODELIST_NODE (need_objs);

        new_arg = MakeId (StringCopy (OBJDEF_VARNAME (obj)), NULL, ST_artificial);
        ID_VARDEC (new_arg) = OBJDEF_ARG (obj);
        NODE_LINE (new_arg) = NODE_LINE (arg_node);

        DBUG_PRINT ("OBJ", ("Adding new argument: %s", OBJDEF_VARNAME (obj)));

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
        ID_NAME (arg_node) = StringCopy (OBJDEF_VARNAME (ID_OBJDEF (arg_node)));
        ID_MOD (arg_node) = NULL;
        ID_VARDEC (arg_node) = OBJDEF_ARG (ID_OBJDEF (arg_node));

        DBUG_PRINT ("OBJ",
                    ("Converting call of global object to %s", ID_NAME (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : OBJlet
 *  arguments     : 1) pointer to N_let node
 *                  2) arg_info used as flag:
 *                     ==NULL traverse right side of '='
 *                     !=NULL resolve reference parameters
 *  description   : First, the right hand side of the let is traversed.
 *                  If this is a function application, then the additional
 *                  return values of this function are considered and bound
 *                  to the current arguments by extending the ids-chain.
 *  global vars   : ---
 *  internal funs :
 *  external funs : MakeIds, AppendIdsChain, Malloc, strlen, strcpy
 *  macros        : TREE, DBUG
 *
 *  remarks       : The arg_info flag is necessary, because OBJlet is used
 *                  in different ways in the two traversals of the function
 *                  body.
 *
 */

node *
OBJlet (node *arg_node, node *arg_info)
{
    node *args, *params;
    ids *new_ids = NULL, *last_ids, *old_ids;
    char *new_ids_name;

    DBUG_ENTER ("OBJlet");

    if (arg_info == NULL) {
        if (LET_EXPR (arg_node) != NULL) {
            LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
        }
    } else {

        if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
            args = AP_ARGS (LET_EXPR (arg_node));
            params = FUNDEF_ARGS (AP_FUNDEF (LET_EXPR (arg_node)));

            while ((params != NULL) && (ARG_BASETYPE (params) != T_dots)) {
                if (ARG_ATTRIB (params) == ST_was_reference) {
                    new_ids_name = StringCopy (ID_NAME (EXPRS_EXPR (args)));

                    if (new_ids == NULL) {
                        new_ids = MakeIds (new_ids_name, NULL, ST_artificial);
                        last_ids = new_ids;
                    } else {
                        IDS_NEXT (last_ids) = MakeIds (new_ids_name, NULL, ST_artificial);
                        last_ids = IDS_NEXT (last_ids);
                    }

                    old_ids = LET_IDS (arg_node);

                    /*
                     *  task done by uniqueness checker
                     *
                              while (old_ids!=NULL)
                              {
                                if (strcmp(IDS_NAME(old_ids), IDS_NAME(last_ids))==0)
                                {
                                  ERROR(NODE_LINE(arg_node),
                                        ("Object '%s` already existing",
                                         IDS_NAME(old_ids)));
                                }

                                old_ids=IDS_NEXT(old_ids);
                              }
                    */

                    IDS_VARDEC (last_ids) = ID_VARDEC (EXPRS_EXPR (args));

                    DBUG_PRINT ("OBJ",
                                ("New return value bound to %s", IDS_NAME (last_ids)));
                }
                args = EXPRS_NEXT (args);
                params = ARG_NEXT (params);
            }

            LET_IDS (arg_node) = AppendIdsChain (new_ids, LET_IDS (arg_node));

        } else {
            if (NODE_TYPE (LET_EXPR (arg_node)) == N_with) {
                LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
            }
        }
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
