/*
 *
 * $Log$
 * Revision 1.6  1995/12/18 16:30:55  cg
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
InsertObjInits (node *fundef, node *objdef)
{
    node *block;

    DBUG_ENTER ("InsertObjInits");

    block = FUNDEF_BODY (fundef);

    while (objdef != NULL) {
        if (OBJDEF_INIT (objdef) != NULL) {
            BLOCK_INSTR (block) = MakeAssign (OBJDEF_INIT (objdef), BLOCK_INSTR (block));
        }

        objdef = OBJDEF_NEXT (objdef);
    }

    DBUG_RETURN (fundef);
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

            FUNDEF_NAME (fun) = FUNDEF_LINKNAME (fun);
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
    ids *new_ids;

    DBUG_ENTER ("PRECobjdef");

    if (OBJDEF_MOD (arg_node) == NULL) {
        if ((OBJDEF_PRAGMA (arg_node) != NULL) && (OBJDEF_LINKNAME (arg_node) != NULL)) {
            FREE (OBJDEF_NAME (arg_node));
            OBJDEF_NAME (arg_node) = OBJDEF_LINKNAME (arg_node);
        }

        OBJDEF_INIT (arg_node) = NULL;
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

        if (OBJDEF_EXPR (arg_node) != NULL) {
            /*
             *  Here, we know that OBJDEF_EXPR contains an application of a
             *  generic object init fun. After objinit.c this is the case for
             *  all imported objdefs in a SAC program.
             */

            new_ids = MakeIds (StringCopy (OBJDEF_NAME (arg_node)), NULL, ST_regular);
            IDS_VARDEC (new_ids) = arg_node;
            IDS_ATTRIB (new_ids) = ST_global;

            OBJDEF_INIT (arg_node) = MakeLet (OBJDEF_EXPR (arg_node), new_ids);

            OBJDEF_EXPR (arg_node) = NULL;
        }
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
                     ("N_fundef node has no reference to N_return node "
                      "(function %s)",
                      FUNDEF_NAME (arg_node)));

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

        /********************************************************************/
        keep_name = FUNDEF_NAME (arg_node);
        keep_mod = FUNDEF_MOD (arg_node);
        keep_status = FUNDEF_STATUS (arg_node);
        keep_attrib = FUNDEF_ATTRIB (arg_node);
        /********************************************************************/

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

        /********************************************************************/
        FUNDEF_NAME (arg_node) = keep_name;
        FUNDEF_MOD (arg_node) = keep_mod;
        FUNDEF_STATUS (arg_node) = keep_status;
        FUNDEF_ATTRIB (arg_node) = keep_attrib;
        /********************************************************************/
    }

    /*
     *  The main function is extended by applications of the init functions
     *  of all global objects.
     *  All other functions are renamed in order to have separate name
     *  spaces for different modules.
     */

    if (strcmp (FUNDEF_NAME (arg_node), "main") == 0) {
        arg_node = InsertObjInits (arg_node, MODUL_OBJS (arg_info));
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

            /**************************************************************/
#ifndef NEWTREE
            if (ARG_NEXT (arg_node) == NULL) {
                arg_node->nnode = 0;
            }
#endif      /* NEWTREE */
            /**************************************************************/
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

    VARDEC_TYPE (arg_node) = RenameTypes (VARDEC_TYPE (arg_node));

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
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

    DBUG_ENTER ("PREClet");

    let_ids = LET_IDS (arg_node);

    while ((let_ids != NULL) && (IDS_STATUS (let_ids) == ST_artificial)) {
        let_ids = FreeOneIds (let_ids);
    }

    LET_IDS (arg_node) = let_ids;

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

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
        EXPRS_NEXT (current) = PRECexprs_ap (EXPRS_NEXT (current), ARG_NEXT (formal));
    }

    /****************************************************************/
#ifndef NEWTREE
    if (EXPRS_NEXT (current) == NULL)
        current->nnode = 1;
#endif /* NEWTREE */
    /****************************************************************/

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

    /****************************************************************/
#ifndef NEWTREE
    if (EXPRS_NEXT (ret_exprs) == NULL)
        ret_exprs->nnode = 1;
#endif /* NEWTREE */
    /****************************************************************/

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
    DBUG_ENTER ("PRECap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node)
          = PRECexprs_ap (AP_ARGS (arg_node), FUNDEF_ARGS (AP_FUNDEF (arg_node)));
    }

    /****************************************************************/
#ifndef NEWTREE
    if (AP_ARGS (arg_node) == NULL)
        arg_node->nnode = 0;
#endif /* NEWTREE */
    /****************************************************************/

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

    /****************************************************************/
#ifndef NEWTREE
    if (RETURN_EXPRS (arg_node) == NULL)
        arg_node->nnode = 0;
#endif /* NEWTREE */
    /****************************************************************/

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

    if ((NODE_TYPE (ID_VARDEC (arg_node)) == N_arg)
        && (ARG_STATUS (ID_VARDEC (arg_node)) == ST_artificial)) {
        ID_VARDEC (arg_node) = ARG_OBJDEF (ID_VARDEC (arg_node));
        ID_NAME (arg_node) = OBJDEF_NAME (ID_VARDEC (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECcast
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
PRECcast (node *arg_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("PRECcast");

    tmp = arg_node;
    arg_node = Trav (CAST_EXPR (arg_node), arg_info);

    FreeOneTypes (CAST_TYPE (tmp));
    FREE (tmp);

    DBUG_RETURN (arg_node);
}
