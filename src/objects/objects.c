/*
 *
 * $Log$
 * Revision 3.11  2004/07/17 14:30:09  sah
 * switch to INFO structure
 * PHASE I
 *
 * Revision 3.10  2004/02/20 08:27:38  mwe
 * now functions with (MODUL_FUNS) and without (MODUL_FUNDECS) body are separated
 * changed tree traversal according to that
 *
 * Revision 3.9  2003/05/30 16:25:55  dkr
 * bug in OBJlet() fixed: read-only references are NOT propagated to the
 * LHS
 *
 * Revision 3.8  2002/10/22 13:26:25  sbs
 * bug in setting flags for reference_only functions solved.
 * Now, ListIO should compile correctly 8-)
 *
 * Revision 3.7  2002/10/18 13:41:42  sbs
 * handling of the various ID-ATTRIBUTES changed to the new
 * FLAG system.
 * Furthermore, support for the correct handling of the
 * prf F_type_error_ as used by the new TC incorporated.
 *
 * Revision 3.6  2002/02/20 14:54:06  dkr
 * fundef DupTypes() renamed into DupAllTypes()
 *
 * Revision 3.5  2002/02/12 16:28:32  dkr
 * OBJarg(), OBJlet(): non-unique reference parameters cause an error now
 *
 * Revision 3.4  2001/04/24 09:39:41  dkr
 * CHECK_NULL renamed into STR_OR_EMPTY
 *
 * Revision 3.3  2001/02/14 10:19:30  dkr
 * no changes done
 *
 * Revision 3.2  2000/12/06 19:22:16  cg
 * Removed compiler warnings in production mode.
 *
 * Revision 3.1  2000/11/20 18:01:59  sacbase
 * new release made
 *
 * Revision 2.7  2000/10/30 19:24:06  dkr
 * OBJarg:
 * new_return_type->attrib is set explicitly in order to get smarter
 * output
 *
 * Revision 2.6  2000/10/26 13:58:13  dkr
 * CopyShpseg replaced by DupShpseg (DupTree.[ch])
 *
 * Revision 2.5  2000/10/24 12:55:06  dkr
 * AppendIdsChain renamed into AppendIds
 *
 * Revision 2.4  2000/07/10 14:24:53  cg
 * Artificial return types of functions that are inserted during the
 * resolution of reference parameters and global objects are now
 * tagged ST_artificial.
 *
 * Revision 2.3  2000/06/23 14:00:16  dkr
 * nodetype N_with removed
 *
 * Revision 2.2  2000/02/23 17:27:01  cg
 * The entry TYPES_TDEF of the TYPES data structure now contains a
 * reference to the corresponding N_typedef node for all user-defined
 * types.
 * Therefore, most calls to LookupType() are eliminated.
 * Please, keep the back references up to date!!
 *
 * Revision 2.1  1999/02/23 12:43:21  sacbase
 * new release made
 *
 * Revision 1.19  1998/06/05 15:27:49  cg
 * global variable mod_name_con and macros MOD_NAME_CON MOD MOD_NAME MOD_CON
 * removed
 * Now, module name and symbol name are combined correctly by ':'.
 * Only when it really comes to the generation of C code, the ':' is
 * replaced by '__'. This is done by the renaming of all identifiers
 * during the precompilation phase.
 *
 * Revision 1.18  1998/02/05 11:14:02  srs
 * OBJlet(): added traversal of new withloop analogous to old withloops.
 *
 * Revision 1.17  1997/11/25 08:41:27  cg
 * bug fixed in OBJlet.
 * OBJlet can now handle cast expressions in front of with-loops
 * or function applications.
 *
 * Revision 1.16  1997/11/07 14:46:17  dkr
 * eliminated another nnode
 *
 * Revision 1.15  1997/03/18 14:43:08  cg
 * The attribute of N_id nodes is now set correctly:
 * ST_regular, ST_global, ST_reference, ST_readonly_reference
 *
 * Revision 1.14  1996/04/25  11:31:36  cg
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
 */

#define NEW_INFO

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "dbug.h"
#include "traverse.h"
#include "internal_lib.h"
#include "Error.h"
#include "DupTree.h"
#include "CheckAvis.h"

#include <string.h>

/*
 * enumeration used to distinguish between
 * traversal phases
 */
enum OBJT_phases { T_phase1, T_phase2 };
typedef enum OBJT_phases objt_phases;

/*
 * INFO structure
 */
struct INFO {
    objt_phases phase;
    node *fundef;
};

/*
 * INFO macros
 */
#define INFO_OBJECTS_PHASE(n) (n->phase)
#define INFO_OBJECTS_FUNDEF(n) (n->fundef)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_OBJECTS_PHASE (result) = T_phase1;
    INFO_OBJECTS_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

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
 *
 */

node *
HandleObjects (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("HandleObjects");

    act_tab = obj_tab;
    info = MakeInfo ();
    syntax_tree = Trav (syntax_tree, info);
    valid_ssaform = FALSE;
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/*
 *
 *  functionname  : InsertIntoInitlist
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

    while (already_done != NULL) {
        first = NODELIST_NODE (already_done);

        DBUG_PRINT ("OBJ", ("Rearranging object %s", ItemName (first)));

        OBJDEF_NEXT (first) = last;

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
 *
 */

node *
OBJmodul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OBJmodul");

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_info);
    }

    if (MODUL_FUNDECS (arg_node) != NULL) {
        MODUL_FUNDECS (arg_node) = Trav (MODUL_FUNDECS (arg_node), arg_info);
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
 *                  2) arg_info used to store fundef for further
 *                     traversal
 *  description   : For each needed global object an additional parameter
 *                  is added to the function's parameter list with
 *                  status 'ST_artificial' and attribute 'ST_reference'
 *                  Afterwards, the modified parameter list is traversed.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : DupTypes, MakeArg, Trav
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
OBJfundef (node *arg_node, info *arg_info)
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
        new_type = DupAllTypes (OBJDEF_TYPE (obj));

        new_arg = MakeArg (StringCopy (OBJDEF_VARNAME (obj)), new_type, ST_artificial,
                           ST_reference, FUNDEF_ARGS (arg_node));

        NODE_LINE (new_arg) = NODE_LINE (arg_node);
        ARG_OBJDEF (new_arg) = obj;

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
        /* set traversal mode 2 and fundef in info structure */
        objt_phases oldphase = INFO_OBJECTS_PHASE (arg_info);
        INFO_OBJECTS_PHASE (arg_info) = T_phase2;
        INFO_OBJECTS_FUNDEF (arg_info) = arg_node;

        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);

        /* reset to default traversal */
        INFO_OBJECTS_PHASE (arg_info) = oldphase;
        INFO_OBJECTS_FUNDEF (arg_info) = NULL;
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
        /* signal first traversal */
        objt_phases oldphase = INFO_OBJECTS_PHASE (arg_info);
        INFO_OBJECTS_PHASE (arg_info) = T_phase1;

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        INFO_OBJECTS_PHASE (arg_info) = oldphase;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        /* traverse next fundef node */
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     *  In a second traversal of the function body, additional return values
     *  caused by reference parameters are bound to the respective variables.
     */

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* signal second traversal */
        objt_phases oldphase = INFO_OBJECTS_PHASE (arg_info);
        INFO_OBJECTS_PHASE (arg_info) = T_phase2;

        /* save fundef as a reference */
        INFO_OBJECTS_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        /* back to default traversal mode */
        INFO_OBJECTS_PHASE (arg_info) = oldphase;
        INFO_OBJECTS_FUNDEF (arg_info) = NULL;
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
 *  external funs : strlen, sprintf, Trav, Malloc
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
OBJobjdef (node *arg_node, info *arg_info)
{
    char *buffer;

    DBUG_ENTER ("OBJobjdef");

    buffer = Malloc (strlen (OBJDEF_NAME (arg_node))
                     + strlen (STR_OR_EMPTY (OBJDEF_MOD (arg_node))) + 3);

    sprintf (buffer, "%s__%s", STR_OR_EMPTY (OBJDEF_MOD (arg_node)),
             OBJDEF_NAME (arg_node));

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
 *                  2) arg_info containing the fundef this arg belongs
 *                     to
 *  description   : For each reference parameter, a new expression is
 *                  generated containing the parameter name and is added
 *                  to the list of return expressions.
 *                  A new types structure is generated as well and added
 *                  to the function's list of return types
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeId, MakeExprs, DupTypes, Trav
 *  macros        : DBUG, TREE
 *
 */

node *
OBJarg (node *arg_node, info *arg_info)
{
    node *new_return_expr, *ret;
    types *new_return_type;

    DBUG_ENTER ("OBJarg");

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    if (ARG_ATTRIB (arg_node) == ST_reference) {
        if (!IsUnique (ARG_TYPE (arg_node))) {
            ERROR (NODE_LINE (arg_node),
                   ("Parameter '%s` is reference parameter but not unique",
                    ARG_NAME (arg_node)));
        }

        ret = FUNDEF_RETURN (INFO_OBJECTS_FUNDEF (arg_info));

        if (ret != NULL) {
            new_return_expr
              = MakeId (StringCopy (ARG_NAME (arg_node)), NULL, ST_artificial);
            ID_VARDEC (new_return_expr) = arg_node;
            SET_FLAG (ID, new_return_expr, IS_GLOBAL, FALSE);
            SET_FLAG (ID, new_return_expr, IS_REFERENCE, FALSE);
            NODE_LINE (new_return_expr) = NODE_LINE (ret);

            DBUG_PRINT ("OBJ", ("New return value: %s", ARG_NAME (arg_node)));

            new_return_expr = MakeExprs (new_return_expr, RETURN_EXPRS (ret));
            NODE_LINE (new_return_expr) = NODE_LINE (ret);

            RETURN_EXPRS (ret) = new_return_expr;
        }

        if (FUNDEF_BASETYPE (INFO_OBJECTS_FUNDEF (arg_info)) == T_void) {
            FUNDEF_BASETYPE (INFO_OBJECTS_FUNDEF (arg_info)) = ARG_BASETYPE (arg_node);
            FUNDEF_DIM (INFO_OBJECTS_FUNDEF (arg_info)) = ARG_DIM (arg_node);
            FUNDEF_SHPSEG (INFO_OBJECTS_FUNDEF (arg_info))
              = DupShpseg (ARG_SHPSEG (arg_node));
            FUNDEF_TNAME (INFO_OBJECTS_FUNDEF (arg_info))
              = StringCopy (ARG_TNAME (arg_node));
            FUNDEF_TMOD (INFO_OBJECTS_FUNDEF (arg_info)) = ARG_TMOD (arg_node);

            TYPES_STATUS (FUNDEF_TYPES (INFO_OBJECTS_FUNDEF (arg_info))) = ST_artificial;

            DBUG_PRINT ("OBJ", ("Converted return type void to %s:%s",
                                FUNDEF_TMOD (INFO_OBJECTS_FUNDEF (arg_info)),
                                FUNDEF_TNAME (INFO_OBJECTS_FUNDEF (arg_info))));
        } else {
            new_return_type = DupAllTypes (ARG_TYPE (arg_node));
            TYPES_STATUS (new_return_type) = ST_artificial;
            TYPES_NEXT (new_return_type) = FUNDEF_TYPES (INFO_OBJECTS_FUNDEF (arg_info));
            FUNDEF_TYPES (INFO_OBJECTS_FUNDEF (arg_info)) = new_return_type;

            DBUG_PRINT ("OBJ", ("Added return type %s:%s",
                                FUNDEF_TMOD (INFO_OBJECTS_FUNDEF (arg_info)),
                                FUNDEF_TNAME (INFO_OBJECTS_FUNDEF (arg_info))));
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
 *
 */

node *
OBJap (node *arg_node, info *arg_info)
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
        SET_FLAG (ID, new_arg, IS_REFERENCE, TRUE);
        SET_FLAG (ID, new_arg, IS_READ_ONLY, FALSE);
        SET_FLAG (ID, new_arg, IS_GLOBAL, TRUE);
        NODE_LINE (new_arg) = NODE_LINE (arg_node);

        DBUG_PRINT ("OBJ", ("Adding new argument: %s", OBJDEF_VARNAME (obj)));

        new_arg = MakeExprs (new_arg, AP_ARGS (arg_node));

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
 *
 */

node *
OBJid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("OBJid");

    DBUG_PRINT ("F_IS_GLOBAL",
                ("trying to accessing IS_GLOBAL of %s", ID_NAME (arg_node)));

    if (GET_FLAG (ID, arg_node, IS_GLOBAL)) {
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
 *                  2) arg_info used to
 *                     - distinguish between traversal mode
 *                       - left side of let
 *                       - resolve reference parameters
 *                     - store current fundef node
 *  description   : First, the right hand side of the let is traversed.
 *                  If this is a function application, then the additional
 *                  return values of this function are considered and bound
 *                  to the current arguments by extending the ids-chain.
 *
 *  remarks       : The arg_info flag is necessary, because OBJlet is used
 *                  in different ways in the two traversals of the function
 *                  body.
 *
 *                  N_id nodes which are arguments in a function application
 *                  get an attribute:
 *                  ST_reference if the respective function parameter is a
 *                  reference parameter.
 *                  ST_readonly_reference if the respective function
 *                  parameter is a readonly-reference parameter.
 *                  This attribute is used by the uniqueness checker.
 *
 */

node *
OBJlet (node *arg_node, info *arg_info)
{
    node *args, *params, *let_expr, *arg_id;
    ids *old_ids;
    char *new_ids_name;
    ids *new_ids = NULL;
    ids *last_ids = NULL;

    DBUG_ENTER ("OBJlet");

    if (INFO_OBJECTS_PHASE (arg_info) == T_phase1) {
        if (LET_EXPR (arg_node) != NULL) {
            LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
        }
    } else {
        let_expr = LET_EXPR (arg_node);
        while (NODE_TYPE (let_expr) == N_cast) {
            let_expr = CAST_EXPR (let_expr);
        }

        if (NODE_TYPE (let_expr) == N_Nwith) {
            let_expr = Trav (let_expr, arg_info);
        } else {
            if (sbs == 1) {
                /*
                 * The new type checker has been running. Therefore,
                 * the reference parameters of N_ap nodes or N_prf( F_type_error) nodes
                 * can be detected by inspecting their IS_REFERENCE flag. If set they
                 * are reference parameters, otherwise they are not. This is ensureed
                 * by create_wrappers.c !!!
                 */
                if ((NODE_TYPE (let_expr) == N_ap)
                    || ((NODE_TYPE (let_expr) == N_prf)
                        && (PRF_PRF (let_expr) == F_type_error))) {
                    args = AP_OR_PRF_ARGS (let_expr);
                    while (args != NULL) {
                        arg_id = EXPRS_EXPR (args);

#ifndef DBUG_OFF
                        if (NODE_TYPE (arg_id) == N_id) {
                            DBUG_PRINT ("F_IS_REFERENCE",
                                        ("trying to access IS_REFERENCE of %s",
                                         ID_NAME (arg_id)));
                        }
#endif
                        if ((NODE_TYPE (arg_id) == N_id)
                            && (GET_FLAG (ID, arg_id, IS_REFERENCE))
                            && (!GET_FLAG (ID, arg_id, IS_READ_ONLY))) {
                            new_ids_name = StringCopy (ID_NAME (arg_id));

                            if (new_ids == NULL) {
                                new_ids = MakeIds (new_ids_name, NULL, ST_artificial);
                                last_ids = new_ids;
                            } else {
                                IDS_NEXT (last_ids)
                                  = MakeIds (new_ids_name, NULL, ST_artificial);
                                last_ids = IDS_NEXT (last_ids);
                            }
                            old_ids = LET_IDS (arg_node);
                            IDS_VARDEC (last_ids) = ID_VARDEC (arg_id);

                            DBUG_PRINT ("OBJ", ("New return value bound to %s",
                                                IDS_NAME (last_ids)));

                            if (!IsUnique (ID_TYPE (arg_id))) {
                                ERROR (
                                  NODE_LINE (arg_node),
                                  ("Argument '%s` is reference parameter but not unique",
                                   ID_NAME (arg_id)));
                            }
                        }
                        args = EXPRS_NEXT (args);
                    }
                    LET_IDS (arg_node) = AppendIds (new_ids, LET_IDS (arg_node));
                }
            } else {
                /*
                 * The old type checker has been run. Therefore, we have to find out
                 * about reference parameters by inspecting the ARG nodes of the
                 * function declaration.
                 */
                if (NODE_TYPE (let_expr) == N_ap) {

                    args = AP_ARGS (let_expr);
                    params = FUNDEF_ARGS (AP_FUNDEF (let_expr));

                    while ((params != NULL) && (ARG_BASETYPE (params) != T_dots)) {
                        arg_id = EXPRS_EXPR (args);

                        if (ARG_ATTRIB (params) == ST_was_reference) {
                            DBUG_ASSERT ((NODE_TYPE (arg_id) == N_id),
                                         "no N_id node found!");

                            new_ids_name = StringCopy (ID_NAME (arg_id));
                            SET_FLAG (ID, arg_id, IS_REFERENCE, TRUE);
                            SET_FLAG (ID, arg_id, IS_READ_ONLY, FALSE);

                            if (new_ids == NULL) {
                                new_ids = MakeIds (new_ids_name, NULL, ST_artificial);
                                last_ids = new_ids;
                            } else {
                                IDS_NEXT (last_ids)
                                  = MakeIds (new_ids_name, NULL, ST_artificial);
                                last_ids = IDS_NEXT (last_ids);
                            }

                            old_ids = LET_IDS (arg_node);

                            IDS_VARDEC (last_ids) = ID_VARDEC (arg_id);

                            DBUG_PRINT ("OBJ", ("New return value bound to %s",
                                                IDS_NAME (last_ids)));

                        } else if (ARG_ATTRIB (params) == ST_readonly_reference) {
                            DBUG_ASSERT ((NODE_TYPE (arg_id) == N_id),
                                         "no N_id node found!");
                            SET_FLAG (ID, arg_id, IS_REFERENCE, TRUE);
                            SET_FLAG (ID, arg_id, IS_READ_ONLY, TRUE);

                        } else {
                            arg_id = NULL;
                        }

                        if (arg_id != NULL) {
                            if (!IsUnique (ID_TYPE (arg_id))) {
                                ERROR (
                                  NODE_LINE (arg_node),
                                  ("Argument '%s` is reference parameter but not unique",
                                   ID_NAME (arg_id)));
                            }
                        }

                        args = EXPRS_NEXT (args);
                        params = ARG_NEXT (params);
                    }

                    LET_IDS (arg_node) = AppendIds (new_ids, LET_IDS (arg_node));
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}
