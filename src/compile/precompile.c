/*
 *
 * $Log$
 * Revision 1.21  1998/03/17 10:41:05  dkr
 * *** empty log message ***
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
 *   node *ProjNormalize(node *proj, int shape_int)
 *
 * description:
 *   returns the normalized values of the N_index-node 'proj'
 *   (maximally outline; (width < step) or (width = step = 1))
 *
 ******************************************************************************/

node *ProjNormalize(node *proj, int shape_int)
{
  int bound1, bound2, offset, step, width, new_bound1, new_bound2;

  DBUG_ENTER("ProjNormalize");

  bound1 = NUM_VAL(INDEX_BOUND1(proj));
  bound2 = NUM_VAL(INDEX_BOUND2(proj));
  offset = NUM_VAL(INDEX_OFFSET(proj));
  step = NUM_VAL(INDEX_STEP(proj));
  width = NUM_VAL(INDEX_WIDTH(proj));

  /* assures: (width < step) or (width = step = 1) */
  if ((width >= step) && (width > 1)) {
    step = NUM_VAL(INDEX_STEP(proj)) = width = NUM_VAL(INDEX_WIDTH(proj)) = 1;
  }

  /* maximize the outline */
  new_bound1 = bound1 - (step - offset - width);
  new_bound1 = MAX(0, new_bound1);

  if ((bound2 - bound1 - offset) % step >= width) {
    new_bound2 = bound2 + step - ((bound2 - bound1 - offset) % step);
    new_bound2 = MIN(new_bound2, shape_int);
  }
  else {
    new_bound2 = bound2;
  }

  NUM_VAL(INDEX_BOUND1(proj)) = new_bound1;
  NUM_VAL(INDEX_BOUND2(proj)) = new_bound2;
  NUM_VAL(INDEX_OFFSET(proj)) = offset + bound1 - new_bound1; 

  DBUG_RETURN(proj);
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

int IsEmpty(int bound1, int bound2, int offset)
{
  int empty;

  DBUG_ENTER("IsEmpty");

  empty = (bound1 + offset >= bound2);

  DBUG_RETURN(empty);
}



/******************************************************************************
 *
 * function:
 *   int ProjIsEmpty(node *proj)
 *
 * description:
 *   computes, whether 'proj' is an empty index-set or not
 *   ('proj' is a N_index-node)
 *
 ******************************************************************************/

int ProjIsEmpty(node *proj)
{
  int empty;

  DBUG_ENTER("ProjIsEmpty");

  empty = IsEmpty(NUM_VAL(INDEX_BOUND1(proj)), NUM_VAL(INDEX_BOUND2(proj)),
                  NUM_VAL(INDEX_OFFSET(proj)));

  DBUG_RETURN(empty);
}



/******************************************************************************
 *
 * function:
 *   int lcm(int x, int y)
 *
 * description:
 *   returns the lowest-common-multiple of x, y.
 *
 ******************************************************************************/

int lcm(int x, int y)
{
  int u, v;

  DBUG_ENTER("lcm");

  u = x;
  v = y;
  while (u != v) {
    if (u < v) {
      u += x;
    }
    else {
      v += y;
    }
  }

  DBUG_RETURN(u);
}


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
 *   int ComputeOffset(int new_bound1, int bound1, int offset, int step)
 *
 * description:
 *   computes a new offset relating to 'new_bound1'
 *
 ******************************************************************************/

int ComputeOffset(int new_bound1, int bound1, int offset, int step, int width)
{
  int new_offset;

  DBUG_ENTER(" ComputeOffset");

  new_offset = offset - ((new_bound1 - bound1) % step);

  if (new_offset < 0)
    new_offset += step;
  if (new_offset > step - width)
    new_offset = 0;

  DBUG_RETURN(new_offset);
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
    offset_1b = ComputeOffset(bound1_1b, bound1_1, offset_1, step_1, width_1);
    offset_1c = ComputeOffset(bound2_1b, bound1_1, offset_1, step_1, width_1);
    bound1_2b = bound1_1b;
    bound2_2b = bound2_1b;
    offset_2b = ComputeOffset(bound1_2b, bound1_2, offset_2, step_2, width_2);
    offset_2c = ComputeOffset(bound2_2b, bound1_2, offset_2, step_2, width_2);

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
 *   int ProjEmptyIsect(node *proj1, node *proj2)
 *
 * description:
 *   computes, whether the intersection of 'proj1', 'proj2' is empty or not.
 *   ('proj1', 'proj2' are N_index-nodes)
 *
 ******************************************************************************/

int ProjEmptyIsect(node *proj1, node *proj2, int shape_int)
{
  int result;

  DBUG_ENTER("ProjEmptyIsect");

  result = (ProjIntersect(proj1, proj2, shape_int) == NULL);

  DBUG_RETURN(result);
}



/******************************************************************************
 *
 * function:
 *   int ProjCompare(node *proj1, node *proj2)
 *
 * description:
 *   compares 'proj1' and 'proj2' (N_index-nodes)
 *
 *   return: -1 -> 'proj1' < 'proj2'
 *            1 -> 'proj1' > 'proj2'
 *            0 -> 'proj1' = 'proj2'
 *
 ******************************************************************************/

int ProjCompare(node *proj1, node *proj2)
{
  int bound1_1, bound2_1, offset_1, step_1, width_1,
      bound1_2, bound2_2, offset_2, step_2, width_2;
  int result;

  DBUG_ENTER("ProjCompare");

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

  TEST_BEGIN(bound1_1, bound1_2)
    TEST_BEGIN(bound2_1, bound2_2)
      TEST_BEGIN(offset_1, offset_2)
        TEST_BEGIN(step_1, step_2)
          TEST_BEGIN(width_1, width_2)
            result = 0;
          TEST_END
        TEST_END
      TEST_END
    TEST_END
  TEST_END

#undef TEST_BEGIN
#undef TEST_END

  DBUG_RETURN(result);
}



/******************************************************************************
 *
 * function:
 *   node *ProjInsert(node *to_insert, node *projs)
 *
 * description:
 *   inserts the elements of 'to_insert' in the sorted chain 'projs'.
 *   any element of 'to_insert', that is already found in 'projs', is freed!!
 *
 *   'to_insert' and 'projs' are N_exprs-chains of N_index-nodes
 *
 ******************************************************************************/

node *ProjInsert(node *insert, node *projs)
{
  node *to_insert, *insert_here;
  int compare;

  DBUG_ENTER("ProjInsert");

  if (projs == NULL) {                  /* 'projs' is empty */
    /* return the 'insert'-chain */
    projs = insert;
  }
  else {                                /* 'projs' contains elements */
    while (insert != NULL) {
      /* insert all elements of 'insert'-chain in 'projs' */
      to_insert = insert;
      insert = EXPRS_NEXT(insert);

      compare = ProjCompare(EXPRS_EXPR(to_insert), EXPRS_EXPR(projs));
      if (compare == 0) {
        /* this projection was found already, hence of no use */
#if 0
        EXPRS_NEXT(to_insert) = NULL;
        FreeTree(to_insert);
#endif
      }
      else {
        if (compare == -1) {
          /* insert current element of 'insert'-chain at top of 'projs' */
          EXPRS_NEXT(to_insert) = projs;
          projs = to_insert;
        }
        else {
          /* search for insert-position in 'projs' */
          insert_here = projs;
          while (EXPRS_NEXT(insert_here) != NULL) {
            compare = ProjCompare(EXPRS_EXPR(to_insert),
                                  EXPRS_EXPR(EXPRS_NEXT(insert_here)));
            if (compare == 0) {
              /* this projection was found already, hence of no use */
#if 0
              EXPRS_NEXT(to_insert) = NULL;
              FreeTree(to_insert);
#else
              to_insert = NULL;
#endif
	    }
            if (compare != 1)
              break;
            insert_here = EXPRS_NEXT(insert_here);
          }

          if (to_insert != NULL) {
            /* insert current element of 'insert'-chain at the found position */
            EXPRS_NEXT(to_insert) = EXPRS_NEXT(insert_here);
            EXPRS_NEXT(insert_here) = to_insert;
	  }
	}
      }
    }
  }

  DBUG_RETURN(projs);
}



/******************************************************************************
 *
 * function:
 *   node *GetRelevantProjs(node *withpart, node *shape_vec, node *prev_projs)
 *
 * description:
 *   withpart: any N_Npart-node
 *   shape_vec: N_array-node containing the shape
 *   prev_projs: set of previous projections (dimensions 0..d-1)
 *               ('prev_proj' is a N_exprs-chain of d N_index-nodes)
 *
 *   return: the projections (dimension d) of the generator-index-sets ('withpart'),
 *           which lie in 'prev_projs'
 *           (returned as a N_exprs-chain of normalized N_index-nodes)
 *
 ******************************************************************************/

node *GetRelevantProjs(node *withpart, node *shape_vec, node *prev_projs)
{
  int relevant;
  node *part, *shape_vec1, *prev_proj, *projs, *insert, *tmp,
       *bound1, *bound2, *step, *width;

  DBUG_ENTER("GetRelevantProjs");

  /* create temporary node */
  tmp = MakeExprs(MakeIndex(NULL, NULL, MakeNum(0), NULL, NULL, NULL), NULL);

  projs = NULL;
  part = withpart;
  /* visit all generator-index-set */
  while (part != NULL) {
    relevant = 1;
    shape_vec1 = ARRAY_AELEMS(shape_vec);
    prev_proj = prev_projs;

    /* get components of current generator-index-set */
    bound1 = ARRAY_AELEMS(NGEN_BOUND1(NPART_GEN(part)));
    bound2 = ARRAY_AELEMS(NGEN_BOUND2(NPART_GEN(part)));
    step = ARRAY_AELEMS(NGEN_STEP(NPART_GEN(part)));
    width = ARRAY_AELEMS(NGEN_WIDTH(NPART_GEN(part)));

    /* test, if generator-index-set is relevant */
    while (prev_proj != NULL) {
      DBUG_ASSERT((shape_vec1 != NULL), "shape component not found");
      DBUG_ASSERT((bound1 != NULL), "bound1 of generator not found");
      DBUG_ASSERT((bound2 != NULL), "bound2 of generator not found");
      DBUG_ASSERT((step != NULL), "step of generator not found");
      DBUG_ASSERT((width != NULL), "width of generator not found");

      /* build a N_index-node */
      INDEX_BOUND1(EXPRS_EXPR(tmp)) = EXPRS_EXPR(bound1);
      INDEX_BOUND2(EXPRS_EXPR(tmp)) = EXPRS_EXPR(bound2);
      INDEX_STEP(EXPRS_EXPR(tmp)) = EXPRS_EXPR(step);
      INDEX_WIDTH(EXPRS_EXPR(tmp)) = EXPRS_EXPR(width);

      if (ProjIsEmpty(EXPRS_EXPR(tmp)) ||
         (ProjEmptyIsect(EXPRS_EXPR(tmp), EXPRS_EXPR(prev_proj),
                         NUM_VAL(EXPRS_EXPR(shape_vec1))))) {
        relevant = 0;
        break;
      }

      prev_proj = EXPRS_NEXT(prev_proj);
      shape_vec1 = EXPRS_NEXT(shape_vec1);

      bound1 = EXPRS_NEXT(bound1);
      bound2 = EXPRS_NEXT(bound2);
      step = EXPRS_NEXT(step);
      width = EXPRS_NEXT(width);
    }

    if (relevant) {
      DBUG_ASSERT((shape_vec1 != NULL), "shape component not found");
      DBUG_ASSERT((bound1 != NULL), "bound1 of generator not found");
      DBUG_ASSERT((bound2 != NULL), "bound2 of generator not found");
      DBUG_ASSERT((step != NULL), "step of generator not found");
      DBUG_ASSERT((width != NULL), "width of generator not found");

      /* build a N_index-node */
      insert = MakeExprs(MakeIndex(MakeNum(NUM_VAL(EXPRS_EXPR(bound1))),
                                   MakeNum(NUM_VAL(EXPRS_EXPR(bound2))),
                                   MakeNum(0),
                                   MakeNum(NUM_VAL(EXPRS_EXPR(step))),
                                   MakeNum(NUM_VAL(EXPRS_EXPR(width))),
                                   NULL),
                         NULL);
      EXPRS_EXPR(insert) = ProjNormalize(EXPRS_EXPR(insert),
                                         NUM_VAL(EXPRS_EXPR(shape_vec1)));
      /* store relevant generator-index-set */
      projs = ProjInsert(insert, projs);
    }

    part = NPART_NEXT(part);
  }

  /* remove temporary node */
#if 0
  FreeTree(tmp);
#endif

  DBUG_RETURN(projs);
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



/******************************************************************************
 *
 * function:
 *   node *ProjPartition(node *withpart, node *shape_vec, node *prev_proj)
 *
 * description:
 *   withpart: NWITH_PART(...)
 *   shape_vec: N_array-node containing the shape
 *   prev_proj: set of previous projections (dimensions 0..d-1)
 *              ('prev_proj' is a N_exprs-chain with d N_index-nodes)
 *
 *   return: partition of the current projection, based on the relevant
 *           generators (relating to 'prev_proj').
 *           (N_exprs-chain of N_index-nodes)
 *
 ******************************************************************************/

node *ProjPartition(node *withpart, node *shape_vec, node *prev_proj)
{
  node *projs;
  int shape_int;

  DBUG_ENTER("ProjPartition");

  projs = GetRelevantProjs(withpart, shape_vec, prev_proj);

  /* compute the right shape-component */
  shape_vec = ARRAY_AELEMS(shape_vec);
  while (prev_proj != NULL) {
    shape_vec = EXPRS_NEXT(shape_vec);
    prev_proj = EXPRS_NEXT(prev_proj);
  }
  shape_int = NUM_VAL(EXPRS_EXPR(shape_vec));

  projs = ProjPartition1(projs, shape_int);   /* step 1 */
  projs = ProjPartition2(projs, shape_int);   /* step 2 */
  projs = ProjPartition3(projs);              /* step 3 */

  DBUG_RETURN(projs);
}



/******************************************************************************
 *
 * function:
 *   void BuildProjs(int dim, int max_dim, node *withpart, node *shape,
 *                   node *proj, node *last_proj)
 *
 * description:
 *   ???
 *
 ******************************************************************************/

void BuildProjs(int dim, int max_dim, node *withpart, node *shape,
                node *proj, node *last_proj)
{
  node *partition, *curr_proj;

  DBUG_ENTER("BuildProjs");

  if (dim == max_dim) {
    /* projections complete ('proj') !!! */
    dim = dim;  /* dummy */
  }
  else {
    partition = ProjPartition(withpart, shape, proj);

    while (partition != NULL) {
      curr_proj = partition;
      partition = EXPRS_NEXT(partition);
      EXPRS_NEXT(curr_proj) = NULL;

      if (last_proj == NULL) {
        proj = curr_proj;
      }
      else {
        EXPRS_NEXT(last_proj) = curr_proj;
      }

      BuildProjs(dim+1, max_dim, withpart, shape, proj, curr_proj);
    }
  }
}
#endif

/******************************************************************************
 *
 * function:
 *   int CompareProj(node *proj1, node *proj2)
 *
 * description:
 *
 *
 ******************************************************************************/

int CompareProj (node *proj1, node *proj2) /* dkr: works only with proj-chains !?! */
{
    node *grid1, *grid2;
    int result;

    DBUG_ENTER ("CompareProj");

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

    do {
        DBUG_ASSERT ((proj1 != NULL), "two identical projs found");
        DBUG_ASSERT ((proj2 != NULL), "two identical projs found");

        grid1 = WLPROJ_INNER (proj1);
        DBUG_ASSERT ((WLGRID_NEXT (grid1) == NULL), "more than one grid found");
        grid2 = WLPROJ_INNER (proj2);
        DBUG_ASSERT ((WLGRID_NEXT (grid2) == NULL), "more than one grid found");

        TEST_BEGIN (WLPROJ_BOUND1 (proj1), WLPROJ_BOUND1 (proj2))
        TEST_BEGIN (WLPROJ_BOUND2 (proj1), WLPROJ_BOUND2 (proj2))
        TEST_BEGIN (WLPROJ_STEP (proj1), WLPROJ_STEP (proj2))
        TEST_BEGIN (WLGRID_OFFSET (grid1), WLGRID_OFFSET (grid2))
        TEST_BEGIN (WLGRID_WIDTH (grid1), WLGRID_WIDTH (grid2))
        result = 0;
        TEST_END
        TEST_END
        TEST_END
        TEST_END
        TEST_END

        proj1 = WLGRID_NEXTDIM (grid1);
        proj2 = WLGRID_NEXTDIM (grid2);
    } while (result == 0);

#undef TEST_BEGIN
#undef TEST_END

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *InsertProj(node *insert, node *proj)
 *
 * description:
 *
 *
 ******************************************************************************/

node *InsertProj (node *insert, node *proj) /* dkr: works only with proj-chains !?! */
{
    node *to_insert, *insert_here;

    DBUG_ENTER ("InsertProj");

    while (insert != NULL) {
        /* insert all elements of 'insert'-chain in 'proj' */
        to_insert = insert;
        insert = WLPROJ_NEXT (insert);

        if (proj == NULL) { /* 'proj' is empty */
            /* insert current element of 'insert'-chain at head of 'proj' */
            WLPROJ_NEXT (to_insert) = proj;
            proj = to_insert;
        } else { /* 'proj' contains elements */
            if (CompareProj (to_insert, proj) == -1) {
                /* insert current element of 'insert'-chain at head of 'proj' */
                WLPROJ_NEXT (to_insert) = proj;
                proj = to_insert;
            } else {
                /* search for insert-position in 'proj' */
                insert_here = proj;
                while (WLPROJ_NEXT (insert_here) != NULL) {
                    if (CompareProj (to_insert, WLPROJ_NEXT (insert_here)) == -1)
                        break;
                    insert_here = WLPROJ_NEXT (insert_here);
                }

                /* insert current element of 'insert'-chain at the found position */
                WLPROJ_NEXT (to_insert) = WLPROJ_NEXT (insert_here);
                WLPROJ_NEXT (insert_here) = to_insert;
            }
        }
    }

    DBUG_RETURN (proj);
}

/******************************************************************************
 *
 * function:
 *   node *NormalizeProj(node *proj, int shape_int)
 *
 * description:
 *
 *
 ******************************************************************************/

node *NormalizeProj (node *proj_chain, int shape_int) /* dkr: need a chain??? */
{ /* dkr: works only with proj-chains !?! */
    node *cur_proj, *proj, *grid;
    int bound1, bound2, offset, step, width, new_bound1, new_bound2;

    DBUG_ENTER ("NormalizeProj");

    cur_proj = proj_chain;
    while (cur_proj != NULL) {

        proj = cur_proj;
        while (proj != NULL) {
            DBUG_ASSERT ((NODE_TYPE (proj) == N_WLproj), "not a WLproj-node");

            grid = WLPROJ_INNER (proj);
            DBUG_ASSERT ((WLGRID_NEXT (grid) == NULL), "more than one grid found");

            bound1 = WLPROJ_BOUND1 (proj);
            bound2 = WLPROJ_BOUND2 (proj);
            step = WLPROJ_STEP (proj);
            width = WLGRID_WIDTH (grid);
            offset = WLGRID_OFFSET (grid);

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

            proj = WLGRID_NEXTDIM (grid);
        }

        cur_proj = WLPROJ_NEXT (cur_proj);
    }

    DBUG_RETURN (proj_chain);
}

/******************************************************************************
 *
 * function:
 *   node* Parts2Proj(node *parts, node *shape)
 *
 * description:
 *   converts a N_Npart-chain (parts) into a N_WLproj-chain (return value).
 *   'shape_vec' is a N_array node containing the shape.
 *
 ******************************************************************************/

node *
Parts2Proj (node *parts, node *shape_vec)
{
    node *parts_proj = NULL;
    node *proj, *new_proj, *last_grid, *shape_elems, *gen, *bound1, *bound2, *step,
      *width;
    int dim, d;

    DBUG_ENTER ("Parts2Proj");

    dim = TYPES_DIM (ARRAY_TYPE (shape_vec));

    while (parts != NULL) {
        proj = NULL;
        shape_elems = ARRAY_AELEMS (shape_vec);

        gen = NPART_GEN (parts);
        DBUG_ASSERT ((NGEN_OP1 (gen) == F_le), "op1 in generator is not <=");
        DBUG_ASSERT ((NGEN_OP2 (gen) == F_lt), "op2 in generator is not <");

        /* get components of current generator */
        bound1 = ARRAY_AELEMS (NGEN_BOUND1 (gen));
        bound2 = ARRAY_AELEMS (NGEN_BOUND2 (gen));
        step = ARRAY_AELEMS (NGEN_STEP (gen));
        width = ARRAY_AELEMS (NGEN_WIDTH (gen));

        for (d = 0; d < dim; d++) {
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
            new_proj = NormalizeProj (new_proj, NUM_VAL (EXPRS_EXPR (shape_elems)));

            /* append 'new_proj' to 'proj'-chain */
            if (proj == NULL) {
                proj = new_proj;
            } else {
                WLGRID_NEXTDIM (last_grid) = new_proj;
            }
            last_grid = WLPROJ_INNER (new_proj);

            /* go to next dimension */
            shape_elems = EXPRS_NEXT (shape_elems);
            bound1 = EXPRS_NEXT (bound1);
            bound2 = EXPRS_NEXT (bound2);
            step = EXPRS_NEXT (step);
            width = EXPRS_NEXT (width);
        }
        DBUG_ASSERT ((shape_elems == NULL), "shape contains more elements");

        WLGRID_CODE (last_grid) = NPART_CODE (parts);
        parts_proj = InsertProj (proj, parts_proj);

        parts = NPART_NEXT (parts);
    }

    DBUG_RETURN (parts_proj);
}

/******************************************************************************
 *
 * function:
 *   node *ComputeRects(node *proj)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
ComputeRects (node *proj)
{
    DBUG_ENTER ("ComputeRects");

    DBUG_RETURN (proj);
}

/******************************************************************************
 *
 * function:
 *   node *SetSegAttribs(node *seg, int dim)
 *
 * description:
 *   sets the attributes BV, UBV for segment 'seg' (N_WLseg node):
 *   - calulates the lcm of all grid-steps in segment (sv)
 *   - based on this, BV and UBV are set
 *
 ******************************************************************************/

node *
SetSegAttribs (node *seg)
{
    long *sv;
    int dim = WLSEG_DIM (seg);

    DBUG_ENTER ("SetSegAttribs");

    sv = (long *)MALLOC (sizeof (long) * dim);

    DBUG_RETURN (seg);
}

/******************************************************************************
 *
 * function:
 *   node *SetSegs(node *rects, int dim)
 *
 * description:
 *   returns chain of segments (based on the calculated rectangles 'rects')
 *
 ******************************************************************************/

node *
SetSegs (node *rects, int dim)
{
    node *segs, *new_seg, *last_seg, *rect;

    DBUG_ENTER ("SetSegs");

#if 0
  /*
   * choose the hole array as the only segment
   *
   */

  segs = MakeWLseg(dim, rects, NULL);
  segs = SetSegAttribs(segs);
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
        new_seg = MakeWLseg (dim, rect, NULL);
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
 *   node *BlockWL(node *proj)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
BlockWL (node *proj)
{
    DBUG_ENTER ("BlockWL");

    DBUG_RETURN (proj);
}

/******************************************************************************
 *
 * function:
 *   node *UnrollBlockWL(node *proj)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
UnrollBlockWL (node *proj)
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
    int dim;

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

    shape = NWITHOP_SHAPE (NWITH2_WITHOP (new_node));
    dim = TYPES_DIM (ARRAY_TYPE (shape));
    DBUG_ASSERT ((NODE_TYPE (shape) == N_array), "shape of with-genarray is unknown");
    rects = ComputeRects (Parts2Proj (NWITH_PART (arg_node), shape));
    segs = SetSegs (rects, dim);

    seg = segs;
    while (seg != NULL) {
        WLSEG_INNER (seg) = BlockWL (WLSEG_INNER (seg));
        WLSEG_INNER (seg) = UnrollBlockWL (WLSEG_INNER (seg));
        WLSEG_INNER (seg) = SplitMergeWL (WLSEG_INNER (seg));
        WLSEG_INNER (seg) = OptimizeWL (WLSEG_INNER (seg));
        WLSEG_INNER (seg) = NormalizeWL (WLSEG_INNER (seg));
        WLSEG_INNER (seg) = FitWL (WLSEG_INNER (seg));
        seg = WLSEG_NEXT (seg);
    }

    NWITH2_SEG (new_node) = segs;

    FreeTree (arg_node);

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
