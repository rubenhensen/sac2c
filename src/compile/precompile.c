/*
 *
 * $Log$
 * Revision 1.65  1998/04/29 18:25:14  dkr
 * include missing header-file
 *
 * Revision 1.64  1998/04/29 17:19:45  dkr
 * with-loop transformation moved to wltransform.[ch]
 *
 * Revision 1.63  1998/04/26 21:53:37  dkr
 * fixed a bug in PRECSpmd
 *
 * Revision 1.62  1998/04/26 16:47:26  dkr
 * fixed a bug in Parts2Strides
 *
 * Revision 1.61  1998/04/25 14:21:05  dkr
 * removed a bug in PRECNcode: sons are traversed now!!
 *
 * Revision 1.60  1998/04/25 13:20:33  dkr
 * extended PRECSPMD
 *
 * Revision 1.58  1998/04/24 01:15:43  dkr
 * added PrecSync
 *
 * Revision 1.57  1998/04/21 13:31:14  dkr
 * NWITH2_SEG renamed to NWITH2_SEGS
 *
 * Revision 1.56  1998/04/20 02:39:15  dkr
 * includes now tree.h
 *
 * Revision 1.55  1998/04/20 00:43:57  dkr
 * removed a bug in PrecSPMD:
 *   no sharing of strings anymore
 *
 * Revision 1.54  1998/04/20 00:06:34  dkr
 * changed PrecLet, PrecSPMD:
 *   used INFO_PREC_LETIDS to build let vars for SPMD_AP_LET
 *
 * Revision 1.53  1998/04/17 19:21:36  dkr
 * lifting of spmd-fun is performed here now
 *
 * Revision 1.52  1998/04/17 17:27:03  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.51  1998/04/16 23:21:53  dkr
 * DBUG_ASSERT added in BlockWL (bv[.] >= 1)
 *
 * Revision 1.50  1998/04/16 11:55:56  dkr
 * removed unused vars
 *
 * Revision 1.49  1998/04/10 03:13:20  dkr
 * fixed a bug in FitWL
 *
 * Revision 1.48  1998/04/10 02:25:58  dkr
 * added support for wlcomp-pragmas
 *
 * Revision 1.47  1998/04/09 14:00:07  dkr
 * attributes for N_conc nodes are now build in 'concregions.[ch]'
 *
 * Revision 1.46  1998/04/07 17:33:19  dkr
 * removed a bug in PRECnwith
 *
 * Revision 1.45  1998/04/04 21:07:29  dkr
 * changed PRECconc
 *
 * Revision 1.44  1998/04/03 21:07:54  dkr
 * changed usage of arg_info
 * changed PRECconc
 *
 * Revision 1.43  1998/04/02 18:47:31  dkr
 * added PRECconc
 *
 * Revision 1.42  1998/04/01 23:57:14  dkr
 * removed a few bugs
 *
 * Revision 1.41  1998/03/31 18:34:33  dkr
 * the UNROLLING flag in N_WLstride, N_WLgrid is now set correctly
 *
 * Revision 1.40  1998/03/31 00:03:52  dkr
 * removed unused vars
 *
 * Revision 1.39  1998/03/30 23:57:51  dkr
 * fixed a bug in PRECnwith:
 *   default value for 'PREC_break_after' is now correct
 *
 * Revision 1.38  1998/03/30 23:43:05  dkr
 * PRECnwith is completed!!!
 *
 * Revision 1.37  1998/03/29 23:28:57  dkr
 * first release with complete support for WL-PREC-phases 1-6
 *
 * Revision 1.36  1998/03/28 20:29:10  dkr
 * removed a bug in splitting phase
 * added merging phase (for blocks only)
 *
 * Revision 1.35  1998/03/27 18:39:56  dkr
 * added split phase
 * N_WLproj renamed in N_WLstride
 *
 * Revision 1.34  1998/03/26 15:38:58  dkr
 * improved sort order in CompareWLnode.
 * fixed a few bugs in PRECnwith ...
 * new usage of MakeWLgrid.
 *
 * Revision 1.33  1998/03/25 19:45:12  dkr
 * PRECnwith:
 *   added break specifiers
 *   new phase order: split - block - merge !!
 *
 * Revision 1.32  1998/03/24 21:45:41  dkr
 * changed IntersectOutline
 *
 * Revision 1.31  1998/03/24 21:31:41  dkr
 * removed a bug with WLPROJ_PART
 *
 * Revision 1.30  1998/03/24 21:09:53  dkr
 * removed a bug in IntersectOutline
 *
 * Revision 1.29  1998/03/22 23:43:35  dkr
 * N_WLgrid: OFFSET, WIDTH -> BOUND1, BOUND2
 *
 * Revision 1.28  1998/03/21 23:45:24  dkr
 * fixed a few bugs in PRECnwith
 * added parts of phase 5 (split-merge)
 *
 * Revision 1.27  1998/03/21 16:19:10  dkr
 * fixed a few bugs in PRECnwith
 *
 * Revision 1.26  1998/03/20 21:57:48  dkr
 * first version of PRECnwith:
 *   cube-building implemented
 *   choice of segments implemented (rudimental)
 *   hierarchical blocking implemented
 *
 * Revision 1.25  1998/03/20 20:52:29  dkr
 * changed usage of MakeWLseg
 *
 * Revision 1.24  1998/03/20 17:25:54  dkr
 * in N_WL... nodes: INNER is now called CONTENTS
 *
 * Revision 1.22  1998/03/19 20:57:42  dkr
 * added computation of the cubes
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

#include "tree.h"
#include "free.h"

#include "internal_lib.h"
#include "convert.h"
#include "traverse.h"
#include "refcount.h"

#include "DupTree.h"
#include "typecheck.h"
#include "dbug.h"

#include <string.h>

/******************************************************************************
 *
 * function:
 *   node *precompile(node *syntax_tree)
 *
 * description:
 *   prepares syntax tree for code generation.
 *     - renames functions and global objects
 *     - removes all casts
 *     - inserts extern declarations for function definitions
 *     - removes all artificial parameters and return values
 *     - marks reference parameters in function applications
 *     - transforms new with-loops
 *
 ******************************************************************************/

node *
precompile (node *syntax_tree)
{
    node *info;

    DBUG_ENTER ("precompile");

    info = MakeInfo ();
    act_tab = precomp_tab;

    syntax_tree = Trav (syntax_tree, info);

    FREE (info);

    DBUG_RETURN (syntax_tree);
}

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
 *  remarks       : The Let_nodes are generated by PRECObjdef
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

/******************************************************************************
 *
 * function:
 *   node *PRECModul( node *arg_node, node *arg_info)
 *
 * description:
 *   starts traversal mechanism for objdef and fundef nodes.
 *
 ******************************************************************************/

node *
PRECModul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECModul");

    INFO_PREC_MODUL (arg_info) = arg_node;

    if (MODUL_TYPES (arg_node) != NULL) {
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
    }

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
 *  functionname  : PRECTypedef
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
PRECTypedef (node *arg_node, node *arg_info)
{
    char *tmp;

    DBUG_ENTER ("PRECTypedef");

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
 *  functionname  : PRECObjdef
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
PRECObjdef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECObjdef");

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

/******************************************************************************
 *
 * function:
 *   node *PRECFundef(node *arg_node, node *arg_info)
 *
 * description:
 *   precompilation of an N_fundef node.
 *
 ******************************************************************************/

node *
PRECFundef (node *arg_node, node *arg_info)
{
    char *keep_name, *keep_mod;
    statustype keep_status, keep_attrib;
    int i;

    DBUG_ENTER ("PRECFundef");

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
    INFO_PREC_CNT_ARTIFICIAL (arg_info) = 0;
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    /*
     *  All artificial return types are removed.
     *  It is necessary to keep name, module name, status, and attrib
     *  because in the real syntax tree these are stored within the types
     *  structure and not as part of the fundef node as in the virtual
     *  syntax tree.
     */
    if (INFO_PREC_CNT_ARTIFICIAL (arg_info) > 0) {
        keep_name = FUNDEF_NAME (arg_node);
        keep_mod = FUNDEF_MOD (arg_node);
        keep_status = FUNDEF_STATUS (arg_node);
        keep_attrib = FUNDEF_ATTRIB (arg_node);

        for (i = 1; i < INFO_PREC_CNT_ARTIFICIAL (arg_info); i++) {
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
        FUNDEF_BODY (arg_node) = InsertObjInits (FUNDEF_BODY (arg_node),
                                                 MODUL_OBJS (INFO_PREC_MODUL (arg_info)));
    } else {
        if (FUNDEF_MOD (arg_node) == NULL) {
            FUNDEF_STATUS (arg_node) = ST_Cfun;
        }

        arg_node = RenameFun (arg_node);
        FUNDEF_TYPES (arg_node) = RenameTypes (FUNDEF_TYPES (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECArg(node *arg_node, node *arg_info)
 *
 * description:
 *  An artificial argument is removed, the attribs are switched:
 *       ST_readonly_reference -> ST_regular
 *       ST_was_reference -> ST_inout
 *
 *  INFO_PREC_CNT_ARTIFICIAL is used to count the number of artificial
 *   return values.
 *
 ******************************************************************************/

node *
PRECArg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECArg");

    if (ARG_ATTRIB (arg_node) == ST_was_reference) {
        INFO_PREC_CNT_ARTIFICIAL (arg_info)++;
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
 *  functionname  : PRECVardec
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
PRECVardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECVardec");

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

/******************************************************************************
 *
 * function:
 *   node *PRECAssign( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
PRECAssign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECAssign");

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_INSTR (arg_node) == NULL) {
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

/******************************************************************************
 *
 * function:
 *   node *PRECLet( node *arg_node, node *arg_info)
 *
 * description:
 *   removes all artificial identifiers on the left hand side of a let.
 *
 ******************************************************************************/

node *
PRECLet (node *arg_node, node *arg_info)
{
    ids *let_ids;

    DBUG_ENTER ("PRECLet");

    let_ids = LET_IDS (arg_node);

    while ((let_ids != NULL) && (IDS_STATUS (let_ids) == ST_artificial)) {
        let_ids = FreeOneIds (let_ids);
    }

    LET_IDS (arg_node) = let_ids;

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    if (LET_EXPR (arg_node) == NULL) {
        arg_node = FreeTree (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECExprs_ap
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
 *  remarks       :
 *
 */

node *
PRECExprs_ap (node *current, node *formal)
{
    node *expr;

    DBUG_ENTER ("PRECExprs_ap");

    if (EXPRS_NEXT (current) != NULL) {
        EXPRS_NEXT (current)
          = PRECExprs_ap (EXPRS_NEXT (current),
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
 *  functionname  : PRECExprs_return
 *  arguments     : 1) N_exprs chain of an N_return node
 *                  2) respective N_return node
 *  description   : removes all artificial return values from the chain.
 *                  A new chain is built up for all those return values
 *                  which belong to original reference parameters.
 *                  These are stored in RETURN_REFERENCE for later use
 *                  in compile.c
 *  remarks       :
 *
 */

node *
PRECExprs_return (node *ret_exprs, node *ret_node)
{
    node *tmp;

    DBUG_ENTER ("PRECExprs_return");

    if (EXPRS_NEXT (ret_exprs) != NULL) {
        EXPRS_NEXT (ret_exprs) = PRECExprs_return (EXPRS_NEXT (ret_exprs), ret_node);
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
 *  functionname  : PRECAp
 *  arguments     : 1) N_ap node
 *                  2) arg_info unused
 *  description   : traverses the current arguments using function
 *                  PRECExprs_ap that is given a pointer
 *                  to the first formal parameter of the applied function.
 *  remarks       :
 *
 */

node *
PRECAp (node *arg_node, node *arg_info)
{
    node *ap;

    DBUG_ENTER ("PRECAp");

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
              = PRECExprs_ap (AP_ARGS (arg_node), FUNDEF_ARGS (AP_FUNDEF (arg_node)));
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECReturn
 *  arguments     : 1) N_return node
 *                  2) arg_info unused
 *  description   : traverses the return values using function
 *                  PRECExprs_return.
 *  remarks       :
 *
 */

node *
PRECReturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECReturn");

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = PRECExprs_return (RETURN_EXPRS (arg_node), arg_node);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECId
 *  arguments     : 1) N_id node;
 *                  2) arg_info unused
 *  description   : Applied occurrences of global objects may be renamed,
 *                  if the global object was renamed.
 *  remarks       :
 *
 */

node *
PRECId (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECId");

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
