/*
 *
 * $Log$
 * Revision 1.23  1998/03/19 23:08:50  dkr
 * *** empty log message ***
 *
 * Revision 1.22  1998/03/19 20:57:42  dkr
 * added computation of the rectangles
 *
 * Revision 1.20  1998/03/15 23:22:39  dkr
 * changed PRECnwith()
 *
 * Revision 1.18  1998/03/03 23:00:11  dkr
 * added PRECncode()
 *
 * Revision 1.17  1998/03/02 22:26:36  dkr
 * added PRECnwith()
 *
 * Revision 1.16  1997/11/24 15:45:45  dkr
 * removed a bug in PREClet(), PRECassign():
 * - LET_EXPR, ASSIGN_INSTR now only freed once!
 *
 * Revision 1.15  1997/11/22 15:41:31  dkr
 * problem with shared linknames fixed:
 * RenameFun() now copies the linkname to FUNDEF_NAME
 *
 * Revision 1.14  1997/09/05 13:46:04  cg
 * All cast expressions are now removed by rmvoidfun.c. Therefore,
 * the respective attempts in precompile.c and ConstantFolding.c
 * are removed. Cast expressions are only used by the type checker.
 * Afterwards, they are useless, and they are not supported by
 * Constant Folding as well as code generation.
 *
 * Revision 1.13  1997/04/30 11:55:34  cg
 * Artificial return values and arguments are removed even in the case
 * of function inlining.
 *
 * Revision 1.12  1997/04/25  09:37:40  sbs
 * DBUG_ASSERT in PRECfundef adjusted (no varargs)
 *
 * Revision 1.11  1996/03/05  15:32:04  cg
 * bug fixed in handling of functions with variable argument list
 *
 * Revision 1.10  1996/01/26  15:33:01  cg
 * applications of class conversion functions are removed
 * where necessary we mark where to copy
 *
 * Revision 1.9  1996/01/22  18:36:37  cg
 * new implementation of object initializations
 *
 * Revision 1.8  1996/01/09  09:19:43  cg
 * implemented pragma linkname for global objects
 *
 * Revision 1.7  1995/12/29  10:44:10  cg
 * some DBUG_PRINTs added.
 *
 * Revision 1.6  1995/12/18  16:30:55  cg
 * many bugs fixed, function PRECexprs replaced by PRECexprs_ap and
 * PRECexprs_return
 *
 * Revision 1.5  1995/12/04  17:00:04  cg
 * added function PRECcast
 * All casts are now eliminated by the precompiler
 *
 * Revision 1.4  1995/12/04  13:38:16  cg
 * bug fixed in string handling of function PRECtypedef
 *
 * Revision 1.3  1995/12/01  20:29:24  cg
 * now the prefix "SAC__" is added to all SAC-identifiers
 * in order to avoid name clashes with C indentifiers.
 *
 * Revision 1.2  1995/12/01  17:23:26  cg
 * first working revision
 *
 * Revision 1.1  1995/11/28  12:23:34  cg
 * Initial revision
 *
 *
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"

#include "internal_lib.h"
#include "convert.h"
#include "traverse.h"

#include "refcount.h"
#include "DupTree.h"
#include "dbug.h"

#include <string.h>

/*
 *
 *  functionname  : precompile
 *  arguments     : 1) syntax tree
 *  description   : prepares syntax tree for code generation.
 *                  - renames functions and global objects
 *                  - removes all casts
 *                  - inserts extern declarations for function definitions
 *                  - removes all artificial parameters and return values
 *                  - marks reference parameters in function applications
 *  global vars   : act_tab, precomp_tab
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG
 *
 *  remarks       :
 *
 */

node *
precompile (node *syntax_tree)
{
    DBUG_ENTER ("precompile");

    act_tab = precomp_tab;

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
 *  functionname  : RenameTypes
 *  arguments     : 1) types structure
 *  description   : renames the given type if it is a user-defined
 *                  SAC-type. The new name is constructed from the
 *                  prefix "SAC_", the module name and the type name.
 *                  Chains of types structures are considered.
 *  global vars   : mod_name_con
 *  internal funs : ---
 *  external funs : Malloc, sprintf, strlen
 *  macros        : TREE, DBUG, FREE
 *
 *  remarks       : The complete new name is stored in NAME while
 *                  MOD is set to NULL.
 *
 */

types *
RenameTypes (types *type)
{
    char *tmp;

    DBUG_ENTER ("RenameTypes");

    if (TYPES_MOD (type) != NULL) {
        tmp = (char *)Malloc (sizeof (char)
                              * (strlen (TYPES_NAME (type))
                                 + strlen (MOD (TYPES_MOD (type)))
                                 + 2 * strlen (mod_name_con) + 4));
        sprintf (tmp, "SAC%s%s%s%s", mod_name_con, MOD (TYPES_MOD (type)), mod_name_con,
                 TYPES_NAME (type));

        DBUG_PRINT ("PREC", ("renaming type %s:%s to %s", MOD (TYPES_MOD (type)),
                             TYPES_NAME (type), tmp));

        FREE (TYPES_NAME (type));
        TYPES_NAME (type) = tmp;
        TYPES_MOD (type) = NULL;
    }

    if (TYPES_NEXT (type) != NULL) {
        TYPES_NEXT (type) = RenameTypes (TYPES_NEXT (type));
    }

    DBUG_RETURN (type);
}

/*
 *
 *  functionname  : InsertObjInits
 *  arguments     : 1) fundef node of main function
 *                  2) chain of objdef nodes
 *  description   : For each global object defined in SAC an application
 *                  of its generic initialization function is inserted
 *                  at the beginning of main.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeAssign
 *  macros        : DBUG, TREE
 *
 *  remarks       : The Let_nodes are generated by PRECobjdef
 *
 */

node *
InsertObjInits (node *block, node *objects)
{
    ids *new_ids;
    node *new_assigns, *first;

    DBUG_ENTER ("InsertObjInits");

    new_assigns = MakeAssign (NULL, NULL);
    first = new_assigns;

    while (objects != NULL) {
        new_ids = MakeIds (StringCopy (OBJDEF_NAME (objects)), NULL, ST_regular);
        IDS_VARDEC (new_ids) = objects;
        IDS_ATTRIB (new_ids) = ST_global;

        if (IsArray (OBJDEF_TYPE (objects))) {
            IDS_REFCNT (new_ids) = 1;
        } else {
            IDS_REFCNT (new_ids) = -1;
        }

        ASSIGN_NEXT (first) = MakeAssign (MakeLet (OBJDEF_EXPR (objects), new_ids), NULL);

        first = ASSIGN_NEXT (first);

        OBJDEF_EXPR (objects) = NULL;

        objects = OBJDEF_NEXT (objects);
    }

    if (ASSIGN_NEXT (new_assigns) != NULL) {
        ASSIGN_NEXT (first) = BLOCK_INSTR (block);
        BLOCK_INSTR (block) = ASSIGN_NEXT (new_assigns);
    }

    FREE (new_assigns);

    DBUG_RETURN (block);
}

/*
 *
 *  functionname  : RenameFun
 *  arguments     : 1) N_fundef node
 *  description   : renames the given function.
 *                  For SAC-functions, a new name is created from the module
 *                  name, the original name and the argument's types.
 *                  For C-functions, a new name is taken from the pragma
 *                  'linkname' if present.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Malloc, strlen, strcat, sprintf, Type2String
 *  macros        : DBUG, TREE, FREE
 *
 *  remarks       :
 *
 */

node *
RenameFun (node *fun)
{
    node *args;
    char *new_name;
    int length = 0;

    DBUG_ENTER ("RenameFun");

    if (FUNDEF_MOD (fun) != NULL) {
        /*
         *  These are SAC-functions which may be overloaded.
         */

        args = FUNDEF_ARGS (fun);

        while (args != NULL) {
            length += strlen (ARG_TYPESTRING (args)) + 1;
            args = ARG_NEXT (args);
        }

        length += (strlen (FUNDEF_NAME (fun)) + strlen (MOD (FUNDEF_MOD (fun)))
                   + 2 * strlen (mod_name_con) + 6);

        new_name = (char *)Malloc (sizeof (char) * length);

        sprintf (new_name, "SAC%s%s%s%s", mod_name_con, MOD (FUNDEF_MOD (fun)),
                 mod_name_con, FUNDEF_NAME (fun));

        args = FUNDEF_ARGS (fun);

        while (args != NULL) {
            strcat (new_name, "_");
            strcat (new_name, ARG_TYPESTRING (args));
            args = ARG_NEXT (args);
        }

        FREE (FUNDEF_NAME (fun));

        /* don't free FUNDEF_MOD(fun) because it is shared !! */

        FUNDEF_NAME (fun) = new_name;
        FUNDEF_MOD (fun) = NULL;
    } else {
        if ((FUNDEF_PRAGMA (fun) != NULL) && (FUNDEF_LINKNAME (fun) != NULL)) {
            /*
             *  These are C-functions with additional pragma 'linkname'.
             */

            FREE (FUNDEF_NAME (fun));

            /* don't free FUNDEF_MOD(fun) because it is shared !! */

            FUNDEF_NAME (fun) = StringCopy (FUNDEF_LINKNAME (fun));
        }
    }

    DBUG_RETURN (fun);
}

/*
 *
 *  functionname  : PRECmodul
 *  arguments     : 1) N_modul node
 *                  2) arg_info unused
 *  description   : starts traversal mechanism for objdef and fundef nodes
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECmodul");

    if (MODUL_TYPES (arg_node) != NULL) {
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
    }

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_info);
    }

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_node);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECtypedef
 *  arguments     : 1) N_typedef node
 *                  2) arg_info unused
 *  description   : renames types,
 *                  All types defined in SAC get the prefix "SAC_"
 *                  to avoid name clashes with C identifiers.
 *  global vars   : mod_name_con
 *  internal funs : ---
 *  external funs : Malloc, sprintf, strlen
 *  macros        : DBUG, FREE, TREE
 *
 *  remarks       :
 *
 */

node *
PRECtypedef (node *arg_node, node *arg_info)
{
    char *tmp;

    DBUG_ENTER ("PRECtypedef");

    if (TYPEDEF_MOD (arg_node) != NULL) {
        tmp = (char *)Malloc (sizeof (char)
                              * (strlen (TYPEDEF_NAME (arg_node))
                                 + strlen (MOD (TYPEDEF_MOD (arg_node)))
                                 + 2 * strlen (mod_name_con) + 5));
        sprintf (tmp, "SAC%s%s%s%s", mod_name_con, MOD (TYPEDEF_MOD (arg_node)),
                 mod_name_con, TYPEDEF_NAME (arg_node));

        FREE (TYPEDEF_NAME (arg_node));
        TYPEDEF_NAME (arg_node) = tmp;
        TYPEDEF_MOD (arg_node) = NULL;

        TYPEDEF_TYPE (arg_node) = RenameTypes (TYPEDEF_TYPE (arg_node));
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECobjdef
 *  arguments     : 1) N_objdef node
 *                  2) arg_info unused
 *  description   : renames global objects.
 *                  For SAC-functions the VARNAME, a combination of module
 *                  name and object name is used, for C-functions the
 *                  optional 'linkname' is used if present.
 *                  Additionally, the object's type is renamed as well.
 *  global vars   : ---
 *  internal funs : RenameTypes
 *  external funs : Trav, Malloc, strcpy, strcat, MakeIds, MakeAp,
 *                  MakeLet, sprintf
 *  macros        : DBUG, TREE, FREE
 *
 *  remarks       :
 *
 */

node *
PRECobjdef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECobjdef");

    DBUG_PRINT ("PREC", ("precompiling object %s", ItemName (arg_node)));

    if (OBJDEF_MOD (arg_node) == NULL) {
        if (OBJDEF_LINKNAME (arg_node) != NULL) {
            FREE (OBJDEF_NAME (arg_node));
            OBJDEF_NAME (arg_node) = OBJDEF_LINKNAME (arg_node);
            FREE (OBJDEF_PRAGMA (arg_node));
        }
    } else {
        OBJDEF_TYPE (arg_node) = RenameTypes (OBJDEF_TYPE (arg_node));

        FREE (OBJDEF_NAME (arg_node));
        OBJDEF_NAME (arg_node) = (char *)Malloc (
          sizeof (char)
          * (strlen (OBJDEF_VARNAME (arg_node)) + strlen (mod_name_con) + 5));

        sprintf (OBJDEF_NAME (arg_node), "SAC%s%s", mod_name_con,
                 OBJDEF_VARNAME (arg_node));

        FREE (OBJDEF_VARNAME (arg_node));

        OBJDEF_MOD (arg_node) = NULL;
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECfundef
 *  arguments     : 1) N_fundef node
 *                  2) arg_info unused
 *  description   : triggers the precompilation
 *  global vars   : ---
 *  internal funs : RenameFun
 *  external funs : Trav
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECfundef (node *arg_node, node *arg_info)
{
    int cnt_artificial = 0, i;
    char *keep_name, *keep_mod;
    statustype keep_status, keep_attrib;

    DBUG_ENTER ("PRECfundef");

    /*
     *  The body of an imported inline function is removed.
     */

    if ((FUNDEF_STATUS (arg_node) == ST_imported)
        && (FUNDEF_ATTRIB (arg_node) != ST_generic) && (FUNDEF_BODY (arg_node) != NULL)) {
        FUNDEF_BODY (arg_node) = FreeTree (FUNDEF_BODY (arg_node));
        FUNDEF_RETURN (arg_node) = NULL;
    }

    /*
     *  The inline flag is set to 0
     */

    FUNDEF_INLINE (arg_node) = 0;

    /*
     *  The function body is traversed in order to remove artificial return
     *  values and parameters of function applications.
     */

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_ASSERT ((FUNDEF_RETURN (arg_node) != NULL)
                       && (NODE_TYPE (FUNDEF_RETURN (arg_node)) == N_return),
                     ("N_fundef node has no reference to N_return node "));

        /*
         * The reference checked above is actually not needed by the
         * precompiler. This is done to check consistency of the syntax
         * tree for further compilation steps.
         */

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     *  Now, traverse the following functions.
     *  All function bodies must be traversed before arguments and
     *  return values of functions are modified.
     */

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     *  The function arguments are traversed, artificial arguments are removed
     *  and the number of reference parameters (including global objects)
     *  is counted and stored in 'cnt_artificial'
     */

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), (node *)&cnt_artificial);
    }

    /*
     *  All artificial return types are removed.
     *  It is necessary to keep name, module name, status, and attrib
     *  because in the real syntax tree these are stored within the types
     *  structure and not as part of the fundef node as in the virtual
     *  syntax tree.
     */

    if (cnt_artificial > 0) {
        keep_name = FUNDEF_NAME (arg_node);
        keep_mod = FUNDEF_MOD (arg_node);
        keep_status = FUNDEF_STATUS (arg_node);
        keep_attrib = FUNDEF_ATTRIB (arg_node);

        for (i = 1; i < cnt_artificial; i++) {
            FUNDEF_TYPES (arg_node) = FreeOneTypes (FUNDEF_TYPES (arg_node));
        }

        if (TYPES_NEXT (FUNDEF_TYPES (arg_node)) == NULL) {
            FUNDEF_BASETYPE (arg_node) = T_void;
            FREE (FUNDEF_TNAME (arg_node));
            FUNDEF_TNAME (arg_node) = NULL;
            FUNDEF_TMOD (arg_node) = NULL;
        } else {
            FUNDEF_TYPES (arg_node) = FreeOneTypes (FUNDEF_TYPES (arg_node));
        }

        FUNDEF_NAME (arg_node) = keep_name;
        FUNDEF_MOD (arg_node) = keep_mod;
        FUNDEF_STATUS (arg_node) = keep_status;
        FUNDEF_ATTRIB (arg_node) = keep_attrib;
    }

    /*
     *  The main function is extended by applications of the init functions
     *  of all global objects.
     *  All other functions are renamed in order to have separate name
     *  spaces for different modules.
     */

    if (strcmp (FUNDEF_NAME (arg_node), "main") == 0) {
        FUNDEF_BODY (arg_node)
          = InsertObjInits (FUNDEF_BODY (arg_node), MODUL_OBJS (arg_info));
    } else {
        if (FUNDEF_MOD (arg_node) == NULL) {
            FUNDEF_STATUS (arg_node) = ST_Cfun;
        }

        arg_node = RenameFun (arg_node);
        FUNDEF_TYPES (arg_node) = RenameTypes (FUNDEF_TYPES (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECarg
 *  arguments     : 1) N_arg node
 *                  2) casted pointer to an integer, which is used to
 *                     count the number of artificial return values.
 *  description   : An artificial argument is removed,
 *                  the attribs are switched:
 *                    ST_readonly_reference -> ST_regular
 *                    ST_was_reference -> ST_inout
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : FreeNode, Trav
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECarg");

    if (ARG_ATTRIB (arg_node) == ST_was_reference) {
        *((int *)arg_info) += 1;
    }

    if (ARG_STATUS (arg_node) == ST_artificial) {
        arg_node = FreeNode (arg_node);

        if (arg_node != NULL) {
            arg_node = Trav (arg_node, arg_info);
        }
    } else {
        ARG_TYPESTRING (arg_node) = Type2String (ARG_TYPE (arg_node), 2);
        ARG_TYPE (arg_node) = RenameTypes (ARG_TYPE (arg_node));
        /*
         *  ARG_TYPESTRING is only used for renaming functions, so the
         *  type's actual name may be changed afterwards.
         */

        if (ARG_ATTRIB (arg_node) == ST_readonly_reference) {
            ARG_ATTRIB (arg_node) = ST_regular;
        } else {
            if (ARG_ATTRIB (arg_node) == ST_was_reference) {
                ARG_ATTRIB (arg_node) = ST_inout;
            }
        }

        if (ARG_NEXT (arg_node) != NULL) {
            ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECvardec
 *  arguments     : 1) N_vardec node
 *                  2) arg_info unused
 *  description   : renames types of declared variables
 *                  remove artificial variable declarations
 *  global vars   : ---
 *  internal funs : RenameTypes
 *  external funs : Trav
 *  macros        : TREE, DBUG
 *
 *  remarks       :
 *
 */

node *
PRECvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECvardec");

    if (VARDEC_STATUS (arg_node) == ST_artificial) {
        arg_node = FreeNode (arg_node);

        if (arg_node != NULL) {
            arg_node = Trav (arg_node, arg_info);
        }
    } else {
        VARDEC_TYPE (arg_node) = RenameTypes (VARDEC_TYPE (arg_node));

        if (VARDEC_NEXT (arg_node) != NULL) {
            VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECassign
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
PRECassign (node *arg_node, node *arg_info)
{
    node *instrs;

    DBUG_ENTER ("PRECassign");

    instrs = Trav (ASSIGN_INSTR (arg_node), NULL);

    ASSIGN_INSTR (arg_node) = instrs;
    if (instrs == NULL) {
        arg_node = FreeNode (arg_node);
        if (arg_node != NULL) {
            arg_node = Trav (arg_node, arg_info);
        }
    } else {
        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PREClet
 *  arguments     : 1) N_let node
 *                  2) arg_info unused
 *  description   : removes all artificial identifiers on the left hand
 *                  side of a let.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : FreeOneIds, Trav
 *  macros        : TREE, DBUG
 *
 *  remarks       :
 *
 */

node *
PREClet (node *arg_node, node *arg_info)
{
    ids *let_ids;
    node *expr;

    DBUG_ENTER ("PREClet");

    let_ids = LET_IDS (arg_node);

    while ((let_ids != NULL) && (IDS_STATUS (let_ids) == ST_artificial)) {
        let_ids = FreeOneIds (let_ids);
    }

    LET_IDS (arg_node) = let_ids;

    expr = Trav (LET_EXPR (arg_node), arg_info);

    LET_EXPR (arg_node) = expr;
    if (expr == NULL) {
        arg_node = FreeTree (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECexprs_ap
 *  arguments     : 1) N_exprs chain of current parameters of a
 *                     function application
 *                  2) N_arg chain of formal parameters of the
 *                     respective function definition
 *  description   : removes all artificial parameters.
 *                  The status of those current parameters which belong
 *                  to formal reference parameters is modified to ST_inout.
 *                  Global objects given as parameters to the applied
 *                  function get a reference to the object definition
 *                  and are renamed with the new name of the global object.
 *  global vars   : ---
 *  internal funs : PRECexprs_ap
 *  external funs : ---
 *  macros        : TREE, DBUG
 *
 *  remarks       :
 *
 */

node *
PRECexprs_ap (node *current, node *formal)
{
    node *expr;

    DBUG_ENTER ("PRECexprs_ap");

    if (EXPRS_NEXT (current) != NULL) {
        EXPRS_NEXT (current)
          = PRECexprs_ap (EXPRS_NEXT (current),
                          ARG_BASETYPE (formal) == T_dots ? formal : ARG_NEXT (formal));
    }

    expr = EXPRS_EXPR (current);

    if (NODE_TYPE (expr) == N_id) {
        if (ID_STATUS (expr) == ST_artificial) {
            current = FreeNode (current);
        } else {
            if (ARG_ATTRIB (formal) == ST_was_reference) {
                ID_ATTRIB (expr) = ST_inout;
            }

            if ((NODE_TYPE (ID_VARDEC (expr)) == N_arg)
                && (ARG_STATUS (ID_VARDEC (expr)) == ST_artificial)) {
                ID_VARDEC (expr) = ARG_OBJDEF (ID_VARDEC (expr));
                ID_NAME (expr) = OBJDEF_NAME (ID_VARDEC (expr));
            }

            if ((NODE_TYPE (ID_VARDEC (expr)) == N_vardec)
                && (VARDEC_STATUS (ID_VARDEC (expr)) == ST_artificial)) {
                ID_VARDEC (expr) = VARDEC_OBJDEF (ID_VARDEC (expr));
                ID_NAME (expr) = OBJDEF_NAME (ID_VARDEC (expr));
            }
        }
    }

    DBUG_RETURN (current);
}

/*
 *
 *  functionname  : PRECexprs_return
 *  arguments     : 1) N_exprs chain of an N_return node
 *                  2) respective N_return node
 *  description   : removes all artificial return values from the chain.
 *                  A new chain is built up for all those return values
 *                  which belong to original reference parameters.
 *                  These are stored in RETURN_REFERENCE for later use
 *                  in compile.c
 *  global vars   : ---
 *  internal funs : PRECexprs_return
 *  external funs : ---
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECexprs_return (node *ret_exprs, node *ret_node)
{
    node *tmp;

    DBUG_ENTER ("PRECexprs_return");

    if (EXPRS_NEXT (ret_exprs) != NULL) {
        EXPRS_NEXT (ret_exprs) = PRECexprs_return (EXPRS_NEXT (ret_exprs), ret_node);
    }

    if (ID_STATUS (EXPRS_EXPR (ret_exprs)) == ST_artificial) {
        if (ARG_STATUS (ID_VARDEC (EXPRS_EXPR (ret_exprs))) == ST_artificial) {
            /*
             *  This artificial return value belongs to a global object,
             *  so it can be removed.
             */

            ret_exprs = FreeNode (ret_exprs);
        } else {
            /*
             *  This artificial return value belongs to an original reference
             *  parameter, so it is stored in RETURN_REFERENCE to be compiled
             *  to an "inout" parameter.
             */

            tmp = ret_exprs;
            ret_exprs = EXPRS_NEXT (ret_exprs);
            EXPRS_NEXT (tmp) = RETURN_REFERENCE (ret_node);
            RETURN_REFERENCE (ret_node) = tmp;
        }
    }

    DBUG_RETURN (ret_exprs);
}

/*
 *
 *  functionname  : PRECap
 *  arguments     : 1) N_ap node
 *                  2) arg_info unused
 *  description   : traverses the current arguments using function
 *                  PRECexprs_ap that is given a pointer
 *                  to the first formal parameter of the applied function.
 *  global vars   : ---
 *  internal funs : PRECexprs_ap
 *  external funs : ---
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECap (node *arg_node, node *arg_info)
{
    node *ap;

    DBUG_ENTER ("PRECap");

    if (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_classfun) {
        ap = arg_node;
        arg_node = EXPRS_EXPR (AP_ARGS (arg_node));

        if (0 == strncmp (AP_NAME (ap), "to_", 3)) {
            if (NODE_TYPE (arg_node) == N_id) {
                if ((ID_REFCNT (arg_node) != -1)
                    && (!IsUnique (VARDEC_TYPE (ID_VARDEC (arg_node))))) {
                    /*
                     *  The base type of the class is refcounted,
                     *  so we have to make the boxed value unique.
                     */

                    ID_MAKEUNIQUE (arg_node) = 1;
                } else {
                    ID_MAKEUNIQUE (arg_node) = 0;
                }
            }
        } else {
            /*
             *  This must be a "from" function. So, the argument is of a class
             *  type which implies that it is am identifier.
             */

            ID_MAKEUNIQUE (arg_node) = 0;
        }

        FREE (AP_NAME (ap));
        FREE (ap);
    } else {
        if (AP_ARGS (arg_node) != NULL) {
            AP_ARGS (arg_node)
              = PRECexprs_ap (AP_ARGS (arg_node), FUNDEF_ARGS (AP_FUNDEF (arg_node)));
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECreturn
 *  arguments     : 1) N_return node
 *                  2) arg_info unused
 *  description   : traverses the return values using function
 *                  PRECexprs_return.
 *  global vars   : ---
 *  internal funs : PRECexprs_return
 *  external funs : ---
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECreturn");

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = PRECexprs_return (RETURN_EXPRS (arg_node), arg_node);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECid
 *  arguments     : 1) N_id node;
 *                  2) arg_info unused
 *  description   : Applied occurrences of global objects may be renamed,
 *                  if the global object was renamed.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECid");

    if (ID_STATUS (arg_node) == ST_artificial) {
        arg_node = FreeTree (arg_node);
    } else {
        if ((NODE_TYPE (ID_VARDEC (arg_node)) == N_arg)
            && (ARG_STATUS (ID_VARDEC (arg_node)) == ST_artificial)) {
            ID_VARDEC (arg_node) = ARG_OBJDEF (ID_VARDEC (arg_node));
            ID_NAME (arg_node) = OBJDEF_NAME (ID_VARDEC (arg_node));
        }

        if ((NODE_TYPE (ID_VARDEC (arg_node)) == N_vardec)
            && (VARDEC_STATUS (ID_VARDEC (arg_node)) == ST_artificial)) {
            ID_VARDEC (arg_node) = VARDEC_OBJDEF (ID_VARDEC (arg_node));
            ID_NAME (arg_node) = OBJDEF_NAME (ID_VARDEC (arg_node));
        }

        ID_MAKEUNIQUE (arg_node) = 0;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * precompilaton of new with-loop
 *
 */

#if 0
/******************************************************************************
 *
 * function:
 *   node *ProjIntersect(node *proj1, node *proj2, int shape_int)
 *
 * description:
 *   returns the intersection of 'proj1' and 'proj2'
 *     (as a N_exprs-chain of normalized N_index-nodes);
 *   returns NULL if intersection is empty.
 *
 *   'proj1', 'proj2' are normalized (!!!) N_index-nodes.
 *
 ******************************************************************************/

node *ProjIntersect(node *proj1, node *proj2, int shape_int)
{
  node *isection = NULL;
  int k, bound1, bound2, offset, step,
      bound1_1, bound2_1, offset_1, step_1, width_1,
      bound1_2, bound2_2, offset_2, step_2, width_2;

  DBUG_ENTER("ProjIntersect");

  bound1_1 = NUM_VAL(INDEX_BOUND1(proj1));
  bound2_1 = NUM_VAL(INDEX_BOUND2(proj1));
  offset_1 = NUM_VAL(INDEX_OFFSET(proj1));
  step_1 = NUM_VAL(INDEX_STEP(proj1));
  width_1 = NUM_VAL(INDEX_WIDTH(proj1));

  bound1_2 = NUM_VAL(INDEX_BOUND1(proj2));
  bound2_2 = NUM_VAL(INDEX_BOUND2(proj2));
  offset_2 = NUM_VAL(INDEX_OFFSET(proj2));
  step_2 = NUM_VAL(INDEX_STEP(proj2));
  width_2 = NUM_VAL(INDEX_WIDTH(proj2));

  if ((bound2_1 > bound1_2 + offset_2) && (bound2_2 > bound1_1 + offset_1)) {
    bound1 = MAX(bound1_1 + offset_1, bound1_2 + offset_2);
    bound2 = MIN(bound2_1, bound2_2);

    step = lcm(step_1, step_2);
    step = MIN(step, (bound2 - bound1));

    offset = -1;
    for (k = 0; k < step; k++) {
      if (((k + bound1 - (bound1_1 + offset_1)) % step_1 < width_1) &&
          ((k + bound1 - (bound1_2 + offset_2)) % step_2 < width_2)) {
        /* offset k matches both grids */
        if (offset == -1)
          /* at the beginning of a new intersection-grid */
          offset = k;
      }
      else {
        if (offset >= 0) {
          /* a completed intersection-grid is found */
          if (! IsEmpty(bound1, bound2, offset)) {
            isection = MakeExprs(MakeIndex(MakeNum(bound1), MakeNum(bound2),
                                           MakeNum(offset),
                                           MakeNum(step), MakeNum(k - offset), NULL),
                                 isection);
            /* normalize new intersection-grid and insert it in 'isection' */
            EXPRS_EXPR(isection) = ProjNormalize(EXPRS_EXPR(isection), shape_int);
          }

          offset = -1;
	}
      }
    }

    if (offset >= 0) {
      /* a completed intersection-grid is found */
      if (! IsEmpty(bound1, bound2, offset)) {
        isection = MakeExprs(MakeIndex(MakeNum(bound1), MakeNum(bound2),
                                       MakeNum(offset),
                                       MakeNum(step), MakeNum(k - offset), NULL),
                             isection);
        /* normalize new intersection-grid and insert it in 'isection' */
        EXPRS_EXPR(isection) = ProjNormalize(EXPRS_EXPR(isection), shape_int);
      }
    }
  }

  DBUG_RETURN(isection);
}



/******************************************************************************
 *
 * function:
 *   node *ProjDisjOutline(node *proj1, node *proj2, int shape_int)
 *
 * description:
 *   returns NULL, if 'proj1' and 'proj2' have disjunct outlines.
 *   otherwise 'proj1', 'proj2' are divided in parts with disjunct outlines;
 *     these parts are returned as a N_exprs-chain of N_index-nodes.
 *
 ******************************************************************************/

node *ProjDisjOutline(node *proj1, node *proj2, int shape_int)
{
  node *disj_out = NULL;
  int bound1_1, bound2_1, offset_1, step_1, width_1,
      bound1_2, bound2_2, offset_2, step_2, width_2,
      bound1_1b, bound2_1b, offset_1b, offset_1c,
      bound1_2b, bound2_2b, offset_2b, offset_2c;

  DBUG_ENTER("ProjDisjOutline");

  bound1_1 = NUM_VAL(INDEX_BOUND1(proj1));
  bound2_1 = NUM_VAL(INDEX_BOUND2(proj1));
  offset_1 = NUM_VAL(INDEX_OFFSET(proj1));
  step_1 = NUM_VAL(INDEX_STEP(proj1));
  width_1 = NUM_VAL(INDEX_WIDTH(proj1));

  bound1_2 = NUM_VAL(INDEX_BOUND1(proj2));
  bound2_2 = NUM_VAL(INDEX_BOUND2(proj2));
  offset_2 = NUM_VAL(INDEX_OFFSET(proj2));
  step_2 = NUM_VAL(INDEX_STEP(proj2));
  width_2 = NUM_VAL(INDEX_WIDTH(proj2));

  if ((bound2_1 > bound1_2) && (bound2_2 > bound1_1) &&
      ((bound1_1 != bound1_2) || (bound2_1 != bound2_2))) {
    /* 'proj1' and 'proj2' do not have disjunct outlines yet */

    /*
     * 'proj1' respectively 'proj2' must be devided in at most three parts, called:
     *    'proj1': (1a), (1b), (1c)
     *    'proj2': (2a), (2b), (2c)
     */
    bound1_1b = MAX(bound1_1, bound1_2);
    bound2_1b = MIN(bound2_1, bound2_2);
    offset_1b = ComputeOffset(bound1_1b, bound1_1, step_1, offset_1, width_1);
    offset_1c = ComputeOffset(bound2_1b, bound1_1, step_1, offset_1, width_1);
    bound1_2b = bound1_1b;
    bound2_2b = bound2_1b;
    offset_2b = ComputeOffset(bound1_2b, bound1_2, step_2, offset_2, width_2);
    offset_2c = ComputeOffset(bound2_2b, bound1_2, step_2, offset_2, width_2);

    if (IsEmpty(bound1_1b, bound2_1, offset_1b)) {      /* is (1b,1c) empty ? */
      bound2_2b = bound1_2;                             /* (2a) == empty, (2b) := empty, (2c) := (2) */
      offset_2c = offset_2;
    }
    if (IsEmpty(bound1_2, bound2_2b, offset_2)) {       /* is (2a,2b) empty ? */
      bound2_1b = bound2_1;                             /* (1c) == empty, (1b) := empty, (1a) := (1) */
    }
    if (IsEmpty(bound1_2b, bound2_2, offset_2b)) {      /* is (2b,2c) empty ? */
      bound2_1b = bound1_1;                             /* (1a) == empty, (1b) := empty, (1c) := (1) */
      offset_1c = offset_1;
    }
    if (IsEmpty(bound1_1, bound2_1b, offset_1)) {       /* is (1a,1b) empty ? */
      bound2_2b = bound2_2;                             /* (2c) == empty, (2b) := empty, (2a) := (2) */
    }

    if (! IsEmpty(bound1_1, bound1_1b, offset_1)) {     /* insert (1a) if not empty */
      disj_out = MakeExprs(MakeIndex(MakeNum(bound1_1), MakeNum(bound1_1b),
                                     MakeNum(offset_1),
                                     MakeNum(step_1), MakeNum(width_1), NULL),
                           disj_out);
    }

    if (! IsEmpty(bound1_1b, bound2_1b, offset_1b)) {   /* insert (1b) if not empty */
      disj_out = MakeExprs(MakeIndex(MakeNum(bound1_1b), MakeNum(bound2_1b),
                                     MakeNum(offset_1b),
                                     MakeNum(step_1), MakeNum(width_1), NULL),
                           disj_out);
    }

    if (! IsEmpty(bound2_1b, bound2_1, offset_1c)) {    /* insert (1c) if not empty */
      disj_out = MakeExprs(MakeIndex(MakeNum(bound2_1b), MakeNum(bound2_1),
                                     MakeNum(offset_1c),
                                     MakeNum(step_1), MakeNum(width_1), NULL),
                           disj_out);
    }

    if (! IsEmpty(bound1_2, bound1_2b, offset_2)) {     /* insert (2a) if not empty */
      disj_out = MakeExprs(MakeIndex(MakeNum(bound1_2), MakeNum(bound1_2b),
                                     MakeNum(offset_2),
                                     MakeNum(step_2), MakeNum(width_2), NULL),
                           disj_out);
    }

    if (! IsEmpty(bound1_2b, bound2_2b, offset_2b)) {   /* insert (2b) if not empty */
      disj_out = MakeExprs(MakeIndex(MakeNum(bound1_2b), MakeNum(bound2_2b),
                                     MakeNum(offset_2b),
                                     MakeNum(step_2), MakeNum(width_2), NULL),
                           disj_out);
    }

    if (! IsEmpty(bound2_2b, bound2_2, offset_2c)) {    /* insert (2c) if not empty */
      disj_out = MakeExprs(MakeIndex(MakeNum(bound2_2b), MakeNum(bound2_2),
                                     MakeNum(offset_2c),
                                     MakeNum(step_2), MakeNum(width_2), NULL),
                           disj_out);
    }
  }

  DBUG_RETURN(disj_out);
}



/******************************************************************************
 *
 * function:
 *   node *ProjPartition1(node* old_projs, int shape_int)
 *
 * description:
 *   old_projs: N_exprs-chain of current projections
 *   shape_int: relevant shape-component
 *
 *   return: partition of the current projection (step 1)
 *           (N_exprs-chain of N_index-nodes)
 *
 ******************************************************************************/

node *ProjPartition1(node *old_projs, int shape_int)
{
  node *proj1, *proj2, *projs, *isection, *tmp;
  int fixpoint;

  DBUG_ENTER("ProjPartition1");

  do {
    fixpoint = 1;
    projs = NULL;

    /* initialize NPART_MODIFIED(...) */
    proj1 = old_projs;
    while (proj1 != NULL) {
      INDEX_MODIFIED(EXPRS_EXPR(proj1)) = 0;
      proj1 = EXPRS_NEXT(proj1);
    }

    /* intersect the elements of 'old_projs' in pairs */
    proj1 = old_projs;
    while (proj1 != NULL) {
      proj2 = EXPRS_NEXT(proj1);
      while (proj2 != NULL) {
        /* intersect 'proj1' and 'proj2' */
        isection = ProjIntersect(EXPRS_EXPR(proj1), EXPRS_EXPR(proj2), shape_int);
        if (isection != NULL) {
          fixpoint = 0;
          INDEX_MODIFIED(EXPRS_EXPR(proj1)) = INDEX_MODIFIED(EXPRS_EXPR(proj2)) = 1;
          projs = ProjInsert(isection, projs);
	}

        proj2 = EXPRS_NEXT(proj2);
      }

      tmp = proj1;
      proj1 = EXPRS_NEXT(proj1);
      EXPRS_NEXT(tmp) = NULL;

      /* have 'proj1' only empty intersections with the others? */
      if (INDEX_MODIFIED(EXPRS_EXPR(tmp)) == 0) {
        /* insert 'proj1' in 'projs' */
        projs = ProjInsert(tmp, projs);
      }
      else {
        /* 'proj1' is no longer needed */
#if 0
        FreeTree(tmp);
#endif
      }
    }

    old_projs = projs;
  } while (! fixpoint);

  DBUG_RETURN(projs);
}



/******************************************************************************
 *
 * function:
 *   node *ProjPartition2(node* old_projs, int shape_int)
 *
 * description:
 *   old_projs: partition of the current projection after step 1 (N_exprs-chain)
 *   shape_int: relevant shape-component
 *
 *   return: modified partition of the current projection (step 2):
 *             outlines are disjunct, too.
 *           (N_exprs-chain of N_index-nodes)
 *
 ******************************************************************************/

node *ProjPartition2(node *old_projs, int shape_int)
{
  node *proj1, *proj2, *projs, *disj_out, *tmp;
  int fixpoint;

  DBUG_ENTER("ProjPartition2");

  do {
    fixpoint = 1;
    projs = NULL;

    /* initialize NPART_MODIFIED(...) */
    proj1 = old_projs;
    while (proj1 != NULL) {
      INDEX_MODIFIED(EXPRS_EXPR(proj1)) = 0;
      proj1 = EXPRS_NEXT(proj1);
    }

    /* intersect the outlines of 'old_projs' in pairs */
    proj1 = old_projs;
    while (proj1 != NULL) {
      if (NUM_VAL(INDEX_STEP(EXPRS_EXPR(proj1))) > 1) {
        proj2 = EXPRS_NEXT(proj1);
        while (proj2 != NULL) {
          if (NUM_VAL(INDEX_STEP(EXPRS_EXPR(proj1))) > 1) {
            /* make the outlines of 'proj1' and 'proj2' disjunct */
            disj_out = ProjDisjOutline(EXPRS_EXPR(proj1), EXPRS_EXPR(proj2), shape_int);
            if (disj_out != NULL) {
              fixpoint = 0;
              INDEX_MODIFIED(EXPRS_EXPR(proj1)) = INDEX_MODIFIED(EXPRS_EXPR(proj2)) = 1;
              projs = ProjInsert(disj_out, projs);
	    }
	  }

          proj2 = EXPRS_NEXT(proj2);
        }
      }

      tmp = proj1;
      proj1 = EXPRS_NEXT(proj1);
      EXPRS_NEXT(tmp) = NULL;

      /* have 'proj1' only empty intersections with the others? */
      if (INDEX_MODIFIED(EXPRS_EXPR(tmp)) == 0) {
        /* insert 'proj1' in 'projs' */
        projs = ProjInsert(tmp, projs);
      }
      else {
        /* 'proj1' is no longer needed */
#if 0
        FreeTree(tmp);
#endif
      }
    }

    old_projs = projs;
  } while (! fixpoint);

  DBUG_RETURN(projs);
}



/******************************************************************************
 *
 * function:
 *   node *ProjPartition3(node* projs)
 *
 * description:
 *   projs: partition of the current projection after step 2 (N_exprs-chain)
 *
 *   return: modified partition of the current projection (step 3):
 *             all grids with same outline are put together (INDEX_NEXT),
 *             and are set to the same step.
 *           (N_exprs_chain of N_index-nodes)
 *
 ******************************************************************************/

node *ProjPartition3(node *projs)
{
  node *first, *next, *append, *tmp;
  int bound1, bound2, offset, step, width,
      new_step, count_new_proj, i;

  DBUG_ENTER("ProjPartition3");

  next = projs;
  while (next != NULL) {
    first = next;
    append = EXPRS_EXPR(first);    /* store append-position of EXPRS_EXPR(first) */
    new_step = NUM_VAL(INDEX_STEP(append));
    next = EXPRS_NEXT(next);
    while ((next != NULL) &&
           (NUM_VAL(INDEX_BOUND1(EXPRS_EXPR(next)))
              == NUM_VAL(INDEX_BOUND1(append))) &&
           (NUM_VAL(INDEX_BOUND2(EXPRS_EXPR(next)))
              == NUM_VAL(INDEX_BOUND2(append)))) {
      /* append EXPRS_EXPR('next') to EXPRS_EXPR('first') */
      append = INDEX_NEXT(append) = EXPRS_EXPR(next);
      /* compute the lcm of all steps -> new step */
      new_step = lcm(new_step, NUM_VAL(INDEX_STEP(append)));

      /* remove 'next' from chain */
      EXPRS_NEXT(first) = EXPRS_NEXT(next);
      tmp = next;

      next = EXPRS_NEXT(next);

      /* free abandoned N_exprs-node */
      EXPRS_EXPR(tmp) = EXPRS_NEXT(tmp) = NULL;
      FreeTree(tmp);
    }

    /* equalize the steps of the projs in EXPRS_EXPR('first') */
    tmp = EXPRS_EXPR(first);
    bound1 = NUM_VAL(INDEX_BOUND1(tmp));
    bound2 = NUM_VAL(INDEX_BOUND2(tmp));

    while (tmp != NULL) {
      offset = NUM_VAL(INDEX_OFFSET(tmp));
      step = NUM_VAL(INDEX_STEP(tmp));
      width = NUM_VAL(INDEX_WIDTH(tmp));

      count_new_proj = new_step / NUM_VAL(INDEX_STEP(tmp));
      NUM_VAL(INDEX_STEP(tmp)) = new_step;
      for (i = 1; i < count_new_proj; i++) {
        append = INDEX_NEXT(append)
          = MakeIndex(MakeNum(bound1), MakeNum(bound2),
                      MakeNum(offset + i * step),
                      MakeNum(new_step), MakeNum(width),
                      NULL);
      }

      tmp = INDEX_NEXT(tmp);
    }
  }

  DBUG_RETURN(projs);
}
#endif

/******************************************************************************
 *
 * function:
 *   int CompareProjs(node *proj1, node *proj2)
 *
 * description:
 *   compares the N_WLproj-nodes 'proj1' and 'proj2' IN ALL DIMS.
 *   ALL GRID DATA IS IGNORED!!!
 *   eventually present next-nodes are ignored.
 *
 *   return: -2 => dim('proj1') < dim('proj2')
 *           -1 => dim('proj1') = dim('proj2'), 'proj1' < 'proj2'
 *            0 => dim('proj1') = dim('proj2'), 'proj1' = 'proj2'
 *            1 => dim('proj1') = dim('proj2'), 'proj1' > 'proj2'
 *            2 => dim('proj1') > dim('proj2')
 *
 ******************************************************************************/

int
CompareProjs (node *proj1, node *proj2)
{
    node *grid1, *grid2;
    int result = 0;

    DBUG_ENTER ("CompareProjs");

#define TEST_BEGIN(a, b)                                                                 \
    if (a > b) {                                                                         \
        result = 1;                                                                      \
    } else {                                                                             \
        if (a < b) {                                                                     \
            result = -1;                                                                 \
        } else {
#define TEST_END                                                                         \
    }                                                                                    \
    }

    while ((result == 0) && ((proj1 != NULL) || (proj2 != NULL))) {
        if (proj1 == NULL) {
            result = -2;
        } else {
            if (proj2 == NULL) {
                result = 2;
            } else {
                grid1 = WLPROJ_INNER (proj1);
                DBUG_ASSERT ((grid1 != NULL), "grid not found");
                grid2 = WLPROJ_INNER (proj2);
                DBUG_ASSERT ((grid2 != NULL), "grid not found");

                TEST_BEGIN (WLPROJ_BOUND1 (proj1), WLPROJ_BOUND1 (proj2))
                TEST_BEGIN (WLPROJ_BOUND2 (proj1), WLPROJ_BOUND2 (proj2))
                result = 0;
                TEST_END
                TEST_END

                proj1 = WLGRID_NEXTDIM (grid1);
                proj2 = WLGRID_NEXTDIM (grid2);
            }
        }
    }

#undef TEST_BEGIN
#undef TEST_END

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *InsertProj(node *projs, node *insert_projs)
 *
 * description:
 *   inserts all elements of 'insert_projs' into the sorted chain 'projs'.
 *   uses function 'CompareProj' to sort the elements.
 *
 *   insert_projs: (unsorted) chain of N_WLproj-nodes
 *   projs:        sorted chain of N_WLproj-nodes
 *   return:       sorted chain of N_WLproj-nodes
 *
 ******************************************************************************/

node *
InsertProjs (node *projs, node *insert_projs)
{
    node *insert_proj, *insert_here;

    DBUG_ENTER ("InsertProjs");

    while (insert_projs != NULL) {
        /* insert all elements of 'insert_projs' in 'projs' */
        insert_proj = insert_projs;
        insert_projs = WLPROJ_NEXT (insert_projs);

        if (projs == NULL) { /* 'projs' is empty */
            /* insert current element of 'insert_projs' at head of 'projs' */
            WLPROJ_NEXT (insert_proj) = projs;
            projs = insert_proj;
        } else { /* 'projs' contains elements */
            if (CompareProjs (insert_proj, projs) < 0) {
                /* insert current element of 'insert_projs' at head of 'projs' */
                WLPROJ_NEXT (insert_proj) = projs;
                projs = insert_proj;
            } else {
                /* search for insert-position in 'projs' */
                insert_here = projs;
                while (WLPROJ_NEXT (insert_here) != NULL) {
                    if (CompareProjs (insert_proj, WLPROJ_NEXT (insert_here)) < 0) {
                        break;
                    }
                    insert_here = WLPROJ_NEXT (insert_here);
                }

                /* insert current element of 'insert_projs' at the found position */
                WLPROJ_NEXT (insert_proj) = WLPROJ_NEXT (insert_here);
                WLPROJ_NEXT (insert_here) = insert_proj;
            }
        }
    }

    DBUG_RETURN (projs);
}

/******************************************************************************
 *
 * function:
 *   node *NormalizeProj_1(node *proj, int shape_int)
 *
 * description:
 *   returns the IN THE FIRST DIM normalized N_WLproj-node 'proj'.
 *   eventually present next-nodes are ignored.
 *
 ******************************************************************************/

node *
NormalizeProj_1 (node *proj, int shape_int)
{
    node *grid;
    int bound1, bound2, step, offset, width, new_bound1, new_bound2;

    DBUG_ENTER ("NormalizeProj_1");

    grid = WLPROJ_INNER (proj);
    DBUG_ASSERT ((grid != NULL), "grid not found");
    DBUG_ASSERT ((WLGRID_NEXT (grid) == NULL), "more than one grid found");

    bound1 = WLPROJ_BOUND1 (proj);
    bound2 = WLPROJ_BOUND2 (proj);
    step = WLPROJ_STEP (proj);
    offset = WLGRID_OFFSET (grid);
    width = WLGRID_WIDTH (grid);

    /* assures: (width < step) or (width = step = 1) */
    if ((width >= step) && (width > 1)) {
        step = WLPROJ_STEP (proj) = width = WLGRID_WIDTH (grid) = 1;
    }

    /* maximize the outline */
    new_bound1 = bound1 - (step - offset - width);
    new_bound1 = MAX (0, new_bound1);

    if ((bound2 - bound1 - offset) % step >= width) {
        new_bound2 = bound2 + step - ((bound2 - bound1 - offset) % step);
        new_bound2 = MIN (new_bound2, shape_int);
    } else {
        new_bound2 = bound2;
    }

    WLPROJ_BOUND1 (proj) = new_bound1;
    WLPROJ_BOUND2 (proj) = new_bound2;
    WLGRID_OFFSET (grid) = offset + bound1 - new_bound1;

    DBUG_RETURN (proj);
}

/******************************************************************************
 *
 * function:
 *   node* Parts2Projs(node *parts, node *shape_arr)
 *
 * description:
 *   converts a N_Npart-chain ('parts') into a N_WLproj-chain (return).
 *   'shape_arr' is a N_array node containing the shape.
 *
 ******************************************************************************/

node *
Parts2Projs (node *parts, node *shape_arr)
{
    node *parts_proj, *projs, *new_proj, *last_grid, *shape_elems, *gen, *bound1, *bound2,
      *step, *width;
    int dims, d;

    DBUG_ENTER ("Parts2Proj");

    parts_proj = NULL;

    /* get the number of dims */
    dims = SHPSEG_SHAPE (TYPES_SHPSEG (ARRAY_TYPE (shape_arr)), 0);

    while (parts != NULL) {
        projs = NULL;
        shape_elems = ARRAY_AELEMS (shape_arr);

        gen = NPART_GEN (parts);
        DBUG_ASSERT ((NGEN_OP1 (gen) == F_le), "op1 in generator is not <=");
        DBUG_ASSERT ((NGEN_OP2 (gen) == F_lt), "op2 in generator is not <");

        /* get components of current generator */
        bound1 = ARRAY_AELEMS (NGEN_BOUND1 (gen));
        bound2 = ARRAY_AELEMS (NGEN_BOUND2 (gen));
        step = ARRAY_AELEMS (NGEN_STEP (gen));
        width = ARRAY_AELEMS (NGEN_WIDTH (gen));

        for (d = 0; d < dims; d++) {
            DBUG_ASSERT ((shape_elems != NULL), "shape not complete");
            DBUG_ASSERT ((bound1 != NULL), "bound1 of generator not complete");
            DBUG_ASSERT ((bound2 != NULL), "bound2 of generator not complete");
            DBUG_ASSERT ((step != NULL), "step of generator not complete");
            DBUG_ASSERT ((width != NULL), "width of generator not complete");

            /* build N_WLproj-node of current dimension */
            new_proj
              = MakeWLproj (0, d, NUM_VAL (EXPRS_EXPR (bound1)),
                            NUM_VAL (EXPRS_EXPR (bound2)), NUM_VAL (EXPRS_EXPR (step)), 0,
                            MakeWLgrid (d, 0, NUM_VAL (EXPRS_EXPR (width)), 0, NULL, NULL,
                                        NULL),
                            NULL);
            new_proj = NormalizeProj_1 (new_proj, NUM_VAL (EXPRS_EXPR (shape_elems)));

            /* append 'new_proj' to 'projs'-chain */
            if (projs == NULL) {
                projs = new_proj;
            } else {
                WLGRID_NEXTDIM (last_grid) = new_proj;
            }
            last_grid = WLPROJ_INNER (new_proj);

            /* go to next dim */
            shape_elems = EXPRS_NEXT (shape_elems);
            bound1 = EXPRS_NEXT (bound1);
            bound2 = EXPRS_NEXT (bound2);
            step = EXPRS_NEXT (step);
            width = EXPRS_NEXT (width);
        }
        DBUG_ASSERT ((shape_elems == NULL), "shape contains more elements");

        WLGRID_CODE (last_grid) = NPART_CODE (parts);
        parts_proj = InsertProjs (parts_proj, projs);

        parts = NPART_NEXT (parts);
    }

    DBUG_RETURN (parts_proj);
}

/******************************************************************************
 *
 * function:
 *   int IsEmpty(int bound1, int bound2, int offset)
 *
 * description:
 *   computes, whether 'bound1', 'bound2', 'offset' specifiy an empty
 *    index-set or not.
 *
 ******************************************************************************/

int
IsEmpty (int bound1, int bound2, int offset)
{
    int empty;

    DBUG_ENTER ("IsEmpty");

    empty = (bound1 + offset >= bound2);

    DBUG_RETURN (empty);
}

/******************************************************************************
 *
 * function:
 *   int ComputeOffset(int new_bound1,
 *                     int bound1, int step, int offset, int width)
 *
 * description:
 *   computes a new offset relating to 'new_bound1'
 *
 ******************************************************************************/

int
ComputeOffset (int new_bound1, int bound1, int step, int offset, int width)
{
    int new_offset;

    DBUG_ENTER ("ComputeOffset");

    new_offset = offset - ((new_bound1 - bound1) % step);

    if (new_offset < 0)
        new_offset += step;
    if (new_offset > step - width)
        new_offset = 0;

    DBUG_RETURN (new_offset);
}

/******************************************************************************
 *
 * function:
 *   void IntersectOutlines(node *proj1, node *proj2,
 *                          node *shape_elems,
 *                          node **isect1, node **isect2)
 *
 * description:
 *   returns in 'isect1' and 'isect2' the part of 'proj1', 'proj2' respectively
 *     that lies in a common rectangle.
 *
 ******************************************************************************/

void
IntersectOutlines (node *proj1, node *proj2, node *shape_elems, node **isect1,
                   node **isect2)
{
    node *grid1, *grid2, *last_isect1, *last_isect2, *new_isect1, *new_isect2;
    int bound11, step1, width1, bound12, step2, width2, i_bound1, i_bound2, i_offset1,
      i_offset2;

    DBUG_ENTER ("IntersectOutlines");

    *isect1 = *isect2 = NULL;

    while (proj1 != NULL) {
        DBUG_ASSERT ((proj2 != NULL), "dim not found");

        grid1 = WLPROJ_INNER (proj1);
        DBUG_ASSERT ((grid1 != NULL), "grid not found");
        DBUG_ASSERT ((WLGRID_NEXT (grid1) == NULL), "more than one grid found");
        grid2 = WLPROJ_INNER (proj2);
        DBUG_ASSERT ((grid2 != NULL), "grid not found");
        DBUG_ASSERT ((WLGRID_NEXT (grid2) == NULL), "more than one grid found");

        bound11 = WLPROJ_BOUND1 (proj1);
        step1 = WLPROJ_STEP (proj1);
        width1 = WLGRID_WIDTH (grid1);
        bound12 = WLPROJ_BOUND1 (proj2);
        step2 = WLPROJ_STEP (proj2);
        width2 = WLGRID_WIDTH (grid2);

        i_bound1 = MAX (bound11, bound12);
        i_bound2 = MIN (WLPROJ_BOUND2 (proj1), WLPROJ_BOUND2 (proj2));
        i_offset1
          = ComputeOffset (i_bound1, bound11, step1, WLGRID_OFFSET (grid1), width1);
        i_offset2
          = ComputeOffset (i_bound1, bound12, step2, WLGRID_OFFSET (grid2), width2);

        if ((!IsEmpty (i_bound1, i_bound2, i_offset1))
            && (!IsEmpty (i_bound1, i_bound2, i_offset2))) {

            new_isect1
              = MakeWLproj (WLPROJ_LEVEL (proj1), WLPROJ_DIM (proj1), i_bound1, i_bound2,
                            step1, WLPROJ_UNROLLING (proj1),
                            MakeWLgrid (WLGRID_DIM (grid1), i_offset1, width1,
                                        WLGRID_UNROLLING (grid1), NULL, NULL, NULL),
                            NULL);
            new_isect1 = NormalizeProj_1 (new_isect1, NUM_VAL (EXPRS_EXPR (shape_elems)));

            new_isect2
              = MakeWLproj (WLPROJ_LEVEL (proj2), WLPROJ_DIM (proj2), i_bound1, i_bound2,
                            step2, WLPROJ_UNROLLING (proj2),
                            MakeWLgrid (WLGRID_DIM (grid2), i_offset2, width2,
                                        WLGRID_UNROLLING (grid2), NULL, NULL, NULL),
                            NULL);
            new_isect2 = NormalizeProj_1 (new_isect2, NUM_VAL (EXPRS_EXPR (shape_elems)));
        } else {
            new_isect1 = new_isect2 = NULL;
        }

        if (new_isect1 == NULL) {
            DBUG_ASSERT ((new_isect2 == NULL), "intersection missed");
            if (*isect1 != NULL) {
                *isect1 = FreeTree (*isect1);
            }
            if (*isect2 != NULL) {
                *isect2 = FreeTree (*isect2);
            }
            break;
        } else {
            DBUG_ASSERT ((new_isect2 != NULL), "intersection missed");
            if (*isect1 == NULL) {
                DBUG_ASSERT ((*isect2 == NULL), "intersection missed");
                *isect1 = new_isect1;
                *isect2 = new_isect2;
            } else {
                WLGRID_NEXTDIM (WLPROJ_INNER (last_isect1)) = new_isect1;
                WLGRID_NEXTDIM (WLPROJ_INNER (last_isect2)) = new_isect2;
            }
            last_isect1 = new_isect1;
            last_isect2 = new_isect2;

            /* copy the code pointers */
            if (WLGRID_NEXTDIM (grid1) == NULL) {
                DBUG_ASSERT ((WLGRID_NEXTDIM (grid2) == NULL), "too many dims");

                DBUG_ASSERT ((WLGRID_CODE (grid1) != NULL), "no code found");
                WLGRID_CODE (WLPROJ_INNER ((last_isect1))) = WLGRID_CODE (grid1);

                DBUG_ASSERT ((WLGRID_CODE (grid2) != NULL), "no code found");
                WLGRID_CODE (WLPROJ_INNER ((last_isect2))) = WLGRID_CODE (grid2);
            }
        }

        proj1 = WLGRID_NEXTDIM (grid1);
        proj2 = WLGRID_NEXTDIM (grid2);
        shape_elems = EXPRS_NEXT (shape_elems);
    }
}

/******************************************************************************
 *
 * function:
 *   node *MergeProjs(node *proj1, node *proj2)
 *
 * description:
 *   merges 'proj2' into 'proj1'
 *   (copies needed parts of 'proj2' into 'proj1').
 *
 ******************************************************************************/

node *
MergeProjs (node *proj1, node *proj2)
{
    node *grid1, *grid2, *new_grid, *tmp;
    int bound11, bound12;

    DBUG_ENTER ("MergeProjs");

    if (proj1 != NULL) {

        grid1 = WLPROJ_INNER (proj1);
        grid2 = WLPROJ_INNER (proj2);

        bound11 = WLPROJ_BOUND1 (proj1);
        bound12 = WLPROJ_BOUND1 (proj2);

        WLPROJ_BOUND1 (proj1) = MAX (bound11, bound12);
        WLPROJ_BOUND2 (proj1) = MIN (WLPROJ_BOUND2 (proj1), WLPROJ_BOUND2 (proj2));

        /*
         * compute new offset for 'grid2'
         */
        WLGRID_OFFSET (grid2)
          = ComputeOffset (WLPROJ_BOUND1 (proj1), bound12, WLPROJ_STEP (proj2),
                           WLGRID_OFFSET (grid2), WLGRID_WIDTH (grid2));

        /*
         * compute new offsets for all grids in 'proj1'
         */
        tmp = grid1;
        do {
            WLGRID_OFFSET (tmp)
              = ComputeOffset (WLPROJ_BOUND1 (proj1), bound11, WLPROJ_STEP (proj1),
                               WLGRID_OFFSET (tmp), WLGRID_WIDTH (tmp));
            tmp = WLGRID_NEXT (tmp);
        } while (tmp != NULL);

        /*
         * insert 'grid2'
         */
        if (WLGRID_OFFSET (grid1) > WLGRID_OFFSET (grid2)) {
            /* insert 'grid2' at head of grid list */
            DBUG_ASSERT ((WLGRID_OFFSET (grid1)
                          >= WLGRID_OFFSET (grid2) + WLGRID_WIDTH (grid2)),
                         "wrong offset");
            new_grid = DupTree (grid2, NULL);
            WLGRID_NEXT (new_grid) = grid1;
            WLPROJ_INNER (proj1) = new_grid;
        } else {
            /* search for correct position for insertion */
            tmp = grid1;
            while ((WLGRID_NEXT (tmp) != NULL)
                   && (WLGRID_OFFSET (tmp) != WLGRID_OFFSET (grid2))
                   && (WLGRID_OFFSET (WLGRID_NEXT (tmp)) <= WLGRID_OFFSET (grid2))) {
                tmp = WLGRID_NEXT (tmp);
            }

            if (WLGRID_OFFSET (tmp) == WLGRID_OFFSET (grid2)) {
                /* range of 'grid2' is already in grid list -> merge next dim */
                WLGRID_NEXTDIM (tmp)
                  = MergeProjs (WLGRID_NEXTDIM (tmp), WLGRID_NEXTDIM (grid2));
            } else {
                /* insert 'grid2' after 'tmp' */
                if (WLGRID_NEXT (tmp) != NULL) {
                    DBUG_ASSERT ((WLGRID_OFFSET (WLGRID_NEXT (tmp))
                                  >= WLGRID_OFFSET (grid2) + WLGRID_WIDTH (grid2)),
                                 "wrong offset");
                }
                new_grid = DupTree (grid2, NULL);
                WLGRID_NEXT (new_grid) = WLGRID_NEXT (tmp);
                WLGRID_NEXT (tmp) = new_grid;
            }
        }
    }

    DBUG_RETURN (proj1);
}

#if 0
    while (tmp != NULL) {
      offset = NUM_VAL(INDEX_OFFSET(tmp));
      step = NUM_VAL(INDEX_STEP(tmp));
      width = NUM_VAL(INDEX_WIDTH(tmp));

      count_new_proj = new_step / NUM_VAL(INDEX_STEP(tmp));
      NUM_VAL(INDEX_STEP(tmp)) = new_step;
      for (i = 1; i < count_new_proj; i++) {
        append = INDEX_NEXT(append)
          = MakeIndex(MakeNum(bound1), MakeNum(bound2),
                      MakeNum(offset + i * step),
                      MakeNum(new_step), MakeNum(width),
                      NULL);
      }

      tmp = INDEX_NEXT(tmp);
    }

#endif

/******************************************************************************
 *
 * function:
 *   node *ComputeRects(node *projs, node *shape_arr)
 *
 * description:
 *   returns the set of rectangles as a N_WLproj-chain
 *
 ******************************************************************************/

node *
ComputeRects (node *projs, node *shape_arr)
{
    node *tmp, *new_projs, *new_proj1, *last_proj2, *isect1, *isect2, *proj1, *proj2;
    int fixpoint, new_step;

    DBUG_ENTER ("ComputeRects");

    /*
     * step 1: create disjunct outlines -> every proj lies in one and only one rectangle
     */
    do {
        fixpoint = 1;
        new_projs = NULL;

        /* initialize WLPROJ_MODIFIED */
        proj1 = projs;
        while (proj1 != NULL) {
            WLPROJ_MODIFIED (proj1) = 0;
            proj1 = WLPROJ_NEXT (proj1);
        }

        /* intersect the elements of 'projs' in pairs */
        proj1 = projs;
        while (proj1 != NULL) {

            proj2 = WLPROJ_NEXT (proj1);
            while (proj2 != NULL) {

                /* intersect outlines of 'proj1' and 'proj2' */
                IntersectOutlines (proj1, proj2, ARRAY_AELEMS (shape_arr), &isect1,
                                   &isect2);
                if ((isect1 != NULL) && (CompareProjs (proj1, isect1) != 0)) {
                    fixpoint = 0;
                    WLPROJ_MODIFIED (proj1) = 1;
                    new_projs = InsertProjs (new_projs, isect1);
                }
                if ((isect2 != NULL) && (CompareProjs (proj2, isect2) != 0)) {
                    fixpoint = 0;
                    WLPROJ_MODIFIED (proj2) = 1;
                    new_projs = InsertProjs (new_projs, isect2);
                }
                proj2 = WLPROJ_NEXT (proj2);
            }

            /* have 'proj1' only empty intersections with the others? */
            if (WLPROJ_MODIFIED (tmp) == 0) {
                /* insert 'proj1' in 'new_projs' */
                tmp = proj1;
                proj1 = WLPROJ_NEXT (proj1);
                EXPRS_NEXT (tmp) = NULL;
                new_projs = InsertProjs (new_projs, tmp);
            } else {
                proj1 = FreeNode (proj1); /* 'proj1' is no longer needed */
                                          /* 'proj1' points now to his successor!! */
            }
        }

        projs = new_projs;
    } while (!fixpoint);

#if 1
    Print (projs);
#endif

    /*
     * step 2: merge all projs that lie in the same rectangle -> set of rectangles
     */
    new_projs = NULL;

    proj1 = projs;
    while (proj1 != NULL) {

        last_proj2 = proj1;
        proj2 = WLPROJ_NEXT (proj1);

        /* duplicate first node of 'proj1' */
        WLPROJ_NEXT (proj1) = NULL;
        new_proj1 = DupTree (proj1, NULL);
        new_step = WLPROJ_STEP (new_proj1); /* initial step value for rectangle */
        WLPROJ_NEXT (proj1) = proj2;

        while (proj2 != NULL) {

            /* intersect outlines of 'proj1' and 'proj2' */
            IntersectOutlines (proj1, proj2, ARRAY_AELEMS (shape_arr), &isect1, &isect2);
            if (CompareProjs (proj1, isect1) == 0) {
                DBUG_ASSERT ((CompareProjs (proj2, isect2) == 0), "wrong outline found");

                WLPROJ_NEXT (last_proj2)
                  = WLPROJ_NEXT (proj2); /* remove 'proj2' from chain */
                new_proj1
                  = MergeProjs (new_proj1, proj2); /* merge 'proj2' with 'proj1' */

                proj2 = FreeNode (proj2); /* data of 'proj2' is no longer needed */
                                          /* 'proj2' points now to his successor!! */
            } else {
                last_proj2 = proj2;          /* save last proj */
                proj2 = WLPROJ_NEXT (proj2); /* go to next proj */
            }
        }

        new_projs = InsertProjs (new_projs, new_proj1);

        proj1 = FreeNode (proj1); /* data of 'proj1' no longer needed */
                                  /* 'proj1' points now to his successor!! */
    }
    projs = new_projs;

    DBUG_RETURN (projs);
}

/******************************************************************************
 *
 * function:
 *   long *CalcSV(node *proj, long *sv)
 *
 * description:
 *   calculates the lcm of all grid-steps in N_WLproj-N_WLgrid-chain 'proj'
 *
 ******************************************************************************/

long *
CalcSV (node *proj, long *sv)
{
    DBUG_ENTER ("CalcSV");

    while (proj != NULL) {
        sv[WLPROJ_DIM (proj)] = lcm (sv[WLPROJ_DIM (proj)], WLPROJ_STEP (proj));
        DBUG_ASSERT ((WLPROJ_INNER (proj) != NULL), "no grid found");
        sv = CalcSV (WLGRID_NEXTDIM (WLPROJ_INNER (proj)), sv);

        proj = WLPROJ_NEXT (proj);
    }

    DBUG_RETURN (sv);
}

/******************************************************************************
 *
 * function:
 *   node *SetSegAttribs(node *seg, int dims)
 *
 * description:
 *   sets the attributes UBV, BV for segment 'seg' (N_WLseg node):
 *   - calulates the lcm of all grid-steps in segment (sv)
 *   - based on this, UBV and BV are set
 *
 ******************************************************************************/

node *
SetSegAttribs (node *seg)
{
    long *sv, *ubv, *bv;
    int d, dims = WLSEG_DIM (seg);

    DBUG_ENTER ("SetSegAttribs");

    /* initialize sv */
    sv = (long *)MALLOC (sizeof (long) * dims);
    for (d = 0; d < dims; d++) {
        sv[d] = 1;
    }

    /* calculate sv */
    sv = CalcSV (WLSEG_INNER (seg), sv);

    /*
     * set ubv (must be a multiple of sv,
     *          except that the first components could have value 1)
     */
    ubv = (long *)MALLOC (sizeof (long) * dims);
    for (d = 0; d < dims; d++) {
#if 1
        ubv[d] = 2 * sv[d];
#else
        ubv[d] = 1;
#endif
    }
    WLSEG_UBV (seg) = ubv;

    /*
     * set bv (must be a multiple of ubv,
     *         except that the first components could have value 1
     *         --- but in any case: bv >= ubv)
     */
    bv = (long *)MALLOC (sizeof (long) * dims);
    for (d = 0; d < dims; d++) {
#if 1
        bv[d] = 4 * sv[d];
#else
        bv[d] = 1;
#endif
    }
    WLSEG_BV (seg) = bv;

    FREE (sv);

    DBUG_RETURN (seg);
}

/******************************************************************************
 *
 * function:
 *   node *SetSegs(node *rects, int dims)
 *
 * description:
 *   returns chain of segments (based on the calculated rectangles 'rects')
 *
 ******************************************************************************/

node *
SetSegs (node *rects, int dims)
{
    node *rect, *last_seg, *new_seg, *segs;

    DBUG_ENTER ("SetSegs");

#if 1
    /*
     * choose the hole array as the only segment
     *
     */

    segs = MakeWLseg (dims, rects, NULL);
    segs = SetSegAttribs (segs);
#else
    /*
     * choose every rectangle as a segment
     *
     */

    segs = NULL;
    while (rects != NULL) {
        /*
         * extract next rectangle
         */
        rect = rects;
        rects = WLPROJ_NEXT (rects);
        WLPROJ_NEXT (rect) = NULL;
        /*
         * build new segment
         */
        new_seg = MakeWLseg (dims, rect, NULL);
        new_seg = SetSegAttribs (new_seg);
        /*
         * append 'new_seg' at 'segs'
         */
        if (segs == NULL) {
            segs = new_seg;
        } else {
            WLSEG_NEXT (last_seg) = new_seg;
        }
        last_seg = new_seg;
    }
#endif

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *BlockWL(node *proj, int dims, long *bv)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
BlockWL (node *proj, int dims, long *bv)
{
    node *last, *curr, *curr_grid, *inner, *lastdim, *block;
    int d, level = 0;

    DBUG_ENTER ("BlockWL");

#if 0
  switch (NODE_TYPE(proj)) {
    case N_WLblock:
        
      break;
    case N_WLproj:
      last = NULL;
      curr = proj;
      while (curr != NULL) {

        if (bv[WLPROJ_DIM(curr)] == 1) {
          /* no blocking -> go to next dim */
          curr_grid = WLPROJ_INNER(curr);
          while (curr_grid != NULL) {
            WLGRID_NEXTDIM(curr_grid) = BlockWL(WLGRID_NEXTDIM(curr_grid), dims, bv);

            curr_grid = WLGRID_NEXT(curr_grid);
	  }

          curr = WLPROJ_NEXT(curr);
        }
        else {
          /* blocking -> create a N_WLblock node for each following dim */
          inner = curr;
          lastdim = NULL;
          for (d = WLPROJ_DIM(curr); d < dims; d++) {
            block = MakeWLblock(level,
                                WLPROJ_DIM(inner),
                                WLPROJ_BOUND1(inner),
                                WLPROJ_BOUND2(inner),
                                bv[WLPROJ_DIM(inner)],
                                NULL,
                                NULL,
                                NULL);

            if (lastdim != NULL) {
              WLBLOCK_NEXTDIM(lastdim) = block;
	    }
            else {
              if (last != NULL) {
                WLBLOCK_NEXT(last) = block;
	      }
              else {
                proj = block;
	      }
              last = block;
	    }
            lastdim = block;

            DBUG_ASSERT((WLPROJ_INNER(inner) != NULL), "no grid found");
            inner = WLGRID_NEXTDIM(WLPROJ_INNER(inner));
	  }

          DBUG_ASSERT((lastdim != NULL), "last block node not found");
          WLBLOCK_INNER(lastdim) = curr;
          curr = WLPROJ_NEXT(curr);
          WLPROJ_NEXT(WLBLOCK_INNER(lastdim)) = NULL;   /* successor is in next block */
        }
      }

      break;
    default:
      DBUG_ASSERT((0), "wrong node type");
  }
#endif

    DBUG_RETURN (proj);
}

/******************************************************************************
 *
 * function:
 *   node *UnrollBlockWL(node *proj, int dims, long *ubv)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
UnrollBlockWL (node *proj, int dims, long *ubv)
{
    DBUG_ENTER ("UnrollBlockWL");

    DBUG_RETURN (proj);
}

/******************************************************************************
 *
 * function:
 *   node *SplitMergeWL(node *proj)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
SplitMergeWL (node *proj)
{
    DBUG_ENTER ("SplitMergeWL");

    DBUG_RETURN (proj);
}

/******************************************************************************
 *
 * function:
 *   node *OptimizeWL(node *proj)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
OptimizeWL (node *proj)
{
    DBUG_ENTER ("OptimizeWL");

    DBUG_RETURN (proj);
}

/******************************************************************************
 *
 * function:
 *   node *NormalizeWL(node *proj)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NormalizeWL (node *proj)
{
    DBUG_ENTER ("NormalizeWL");

    DBUG_RETURN (proj);
}

/******************************************************************************
 *
 * function:
 *   node *FitWL(node *proj)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
FitWL (node *proj)
{
    DBUG_ENTER ("FitWL");

    DBUG_RETURN (proj);
}

/******************************************************************************
 *
 * function:
 *   node *PRECnwith(node *arg_node, node *arg_info)
 *
 * description:
 *   precompilation of new with-loop (N_Nwith-node)
 *
 ******************************************************************************/

node *
PRECnwith (node *arg_node, node *arg_info)
{
    node *new_node, *rects, *segs, *seg, *shape;
    int dims;

    DBUG_ENTER ("PRECnwith");

    DBUG_ASSERT ((NWITHOP_TYPE (NWITH_WITHOP (arg_node)) == WO_genarray),
                 "type of with-loop is not WO_genarray");

    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    new_node = MakeNWith2 (NPART_WITHID (NWITH_PART (arg_node)), NULL,
                           NWITH_CODE (arg_node), NWITH_WITHOP (arg_node));

    /*
     * withid, code and withop are overtaken to the nwith2-tree without a change.
     * because of that, these parts are cut off from the old nwith-tree, before freeing
     * it.
     *
     */
    NPART_WITHID (NWITH_PART (arg_node)) = NULL;
    NWITH_CODE (arg_node) = NULL;
    NWITH_WITHOP (arg_node) = NULL;

    DBUG_ASSERT ((NWITHOP_TYPE (NWITH2_WITHOP (new_node)) == WO_genarray),
                 "type of with-loop is not genarray");
    /* get shape of genarray: */
    shape = NWITHOP_SHAPE (NWITH2_WITHOP (new_node));
    /* get number of dims of genarray: */
    dims = SHPSEG_SHAPE (TYPES_SHPSEG (ARRAY_TYPE (shape)), 0);
    DBUG_ASSERT ((NODE_TYPE (shape) == N_array), "shape of with-genarray is unknown");
    rects = ComputeRects (Parts2Projs (NWITH_PART (arg_node), shape), shape);
    segs = SetSegs (rects, dims);

    seg = segs;
    while (seg != NULL) {
        WLSEG_INNER (seg) = BlockWL (WLSEG_INNER (seg), dims, WLSEG_BV (seg));
        WLSEG_INNER (seg) = UnrollBlockWL (WLSEG_INNER (seg), dims, WLSEG_UBV (seg));
        WLSEG_INNER (seg) = SplitMergeWL (WLSEG_INNER (seg));
        WLSEG_INNER (seg) = OptimizeWL (WLSEG_INNER (seg));
        WLSEG_INNER (seg) = NormalizeWL (WLSEG_INNER (seg));
        WLSEG_INNER (seg) = FitWL (WLSEG_INNER (seg));
        seg = WLSEG_NEXT (seg);
    }

    NWITH2_SEG (new_node) = segs;

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECncode(node *arg_node, node *arg_info)
 *
 * description:
 *   precompilation of Ncode-nodes:
 *     CODE_NO is set in the whole Ncode-chain
 *
 ******************************************************************************/

node *
PRECncode (node *arg_node, node *arg_info)
{
    node *code;
    int no = 0;

    DBUG_ENTER ("PRECncode");

    code = arg_node;
    while (code != NULL) {
        NCODE_NO (code) = no;
        no++;
        code = NCODE_NEXT (code);
    }

    DBUG_RETURN (arg_node);
}
