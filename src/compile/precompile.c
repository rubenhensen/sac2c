/*
 * $Log$
 * Revision 2.11  2000/05/26 19:26:53  dkr
 * signature of AdjustFoldFundef modified
 *
 * Revision 2.10  2000/05/25 23:04:44  dkr
 * PREClet(): call of AdjustFoldFundef() added
 *
 * Revision 2.9  2000/04/18 14:00:48  jhs
 * Added DBUGs.
 *
 * Revision 2.8  2000/03/23 16:04:49  dkr
 * DBUG_ASSERT in RenameId() added
 *
 * Revision 2.7  2000/03/02 13:07:44  jhs
 * Commented out usage of FUNDEF_ATTRIB (search it to find the place)
 * because these flag is cleared and reused bei multithreaded.
 *
 * Revision 2.6  2000/02/23 20:16:34  cg
 * Node status ST_imported replaced by ST_imported_mod and
 * ST_imported_class in order to allow distinction between enteties
 * that are imported from a module and those that are imported from a
 * class.
 *
 * Revision 2.5  2000/02/23 17:47:34  cg
 * Header file refcount.h no longer included.
 * Type property functions IsUnique(<type>), IsBoxed(<type>)
 * moved from refcount.c to tree_compound.c.
 *
 * Revision 2.4  1999/09/01 17:12:39  jhs
 * Expanded COMPSync to refcounters in barriers.
 *
 * Revision 2.3  1999/06/25 15:19:43  rob
 * Avoid genning GetUniFromTypes if not TAGGED_ARRAYS
 *
 * Revision 2.2  1999/06/25 14:52:25  rob
 * Introduce definitions and utility infrastructure for tagged array support.
 *
 * Revision 2.1  1999/02/23 12:42:52  sacbase
 * new release made
 *
 * Revision 1.76  1998/11/08 14:31:15  dkr
 * PRECNcode:
 *   NCODE_CBLOCK should be never NULL
 *   (if so, we will get an assert now :-)
 *
 * Revision 1.75  1998/08/07 16:06:13  dkr
 * PRECWLsegVar added
 *
 *  [...]
 *
 * Revision 1.2  1995/12/01  17:23:26  cg
 * first working revision
 *
 * Revision 1.1  1995/11/28  12:23:34  cg
 * Initial revision
 */

#include "dbug.h"
#include "types.h"
#include "tree.h"
#include "DupTree.h"
#include "free.h"
#include "traverse.h"
#include "internal_lib.h"
#include "convert.h"
#include "DataFlowMask.h"
#include "typecheck.h"
#include "scheduling.h"
#include "adjust_ids.h"
#include "precompile.h"

#include <string.h>

#ifdef TAGGED_ARRAYS

/******************************************************************************
 *
 * function:
 *   data_class_t GetClassFromTypes(types *typ)
 *
 * description:
 *   Returns the Data Class of an object (usually an array) from its type
 *
 * NB: As of 1999-06-22, it only returns C_aks or C_akd or C_scl.
 *                       It can't deduce HID.
 *
 ******************************************************************************/

data_class_t
GetClassFromTypes (types *typ)
{
    data_class_t z;

    DBUG_ENTER ("GetClassFromTypes");
    /*
     * If the array has known shape, it's C_aks; otherwise C_akd.
     */
    if (TYPES_DIM (typ) == SCALAR)
        z = C_scl;
    else if (KNOWN_SHAPE (TYPES_DIM (typ)))
        z = C_aks;
    else
        z = C_akd;

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   uniqueness_class_t GetUniFromTypes(types *typ)
 *
 * description:
 *   Returns the Uniqueness Class of an object (usually an array) from
 *   its type
 *
 *
 ******************************************************************************/

uniqueness_class_t
GetUniFromTypes (types *typ)
{
    uniqueness_class_t z;

    DBUG_ENTER ("GetUniFromTypes");

    if (IsUnique (typ))
        z = C_unq;
    else
        z = C_nuq;

    DBUG_RETURN (z);
}

#endif /* TAGGED_ARRAYS */

/******************************************************************************
 *
 * function:
 *   char *PRECRenameLocalIdentifier(char *id, data_class_t d_class,
 *                                   uniqueness_class_t u_class)
 *
 * description:
 *   This function renames a given local identifier name for precompiling
 *   purposes. If the identifier has been inserted by sac2c, i.e. it starts
 *   with an underscore, it is prefixed by SACp. Otherwise, it is prefixed
 *   by SACl.
 *
 *   It also maps the name into an nt (Name Tuple) for tagged arrays.
 *
 ******************************************************************************/
#ifdef TAGGED_ARRAYS
char *
PRECRenameLocalIdentifier (char *id, data_class_t nt_class, uniqueness_class_t nt_uni)
{
    char *name_prefix_table[] = {"SAC1_", "SACp"};
    char *name_prefix;
#else  /* TAGGED_ARRAYS */
char *
PRECRenameLocalIdentifier (char *id)
{
#endif /* TAGGED_ARRAYS */

    char *new_name;

    DBUG_ENTER ("PRECRenameLocalIdentifier");

#ifdef TAGGED_ARRAYS
    /*
     * Set name prefix depending on whether the identifier was inserted by sac2c
     * or whether it originated in the source code.
     */
    if (id[0] == '_')
        name_prefix = name_prefix_table[1]; /* name inserted by sac2c */
    else
        name_prefix = name_prefix_table[0]; /* name originated in source code */

    if (nt_class == C_scl) {
        new_name
          = (char *)Malloc (sizeof (char) * (strlen (id) + strlen (name_prefix) + 1));
        sprintf (new_name, "%s%s", name_prefix, id);
    } else {
        new_name = (char *)Malloc (
          sizeof (char) * (strlen (id) + strlen (name_prefix) + 1 + NT_OVERHEAD));
        sprintf (new_name, "(%s%s,(%s,(%s,NIL)))", name_prefix, id,
                 nt_class_str[nt_class], nt_uni_str[nt_uni]);
    };
#else  /* TAGGED_ARRAYS */
    if (id[0] == '_') {
        /*
         * This local identifier was inserted by sac2c.
         */
        new_name = (char *)Malloc (sizeof (char) * (strlen (id) + 5));
        sprintf (new_name, "SACp%s", id);
        /*
         * Here, we don't need an underscore after the prefix because the name already
         * starts with one.
         */
    } else {
        /*
         * This local identifier originates from the source code.
         */
        new_name = (char *)Malloc (sizeof (char) * (strlen (id) + 6));
        sprintf (new_name, "SACl_%s", id);
    }
#endif /* TAGGED_ARRAYS */

    FREE (id);

    DBUG_RETURN (new_name);
}

/******************************************************************************
 *
 * function:
 *   node *RenameId(node *idnode)
 *
 * description:
 *   This function performs the renaming of identifiers on the right hand
 *   side of assignments, i.e. the original identifiers are prefixed with
 *   SACl or SACp or are renamed according to the renaming conventions of
 *   global objects.
 *
 ******************************************************************************/

static node *
RenameId (node *idnode)
{
#ifdef TAGGED_ARRAYS
    data_class_t dclass;
    uniqueness_class_t uclass;
#endif /* TAGGED_ARRAYS */

    DBUG_ENTER ("RenameId");

    DBUG_ASSERT ((NODE_TYPE (idnode) == N_id), "Wrong argument to function RenameId()");
    DBUG_PRINT ("PRECjhs", ("id-name == %s", ID_NAME (idnode)));
    DBUG_ASSERT ((ID_VARDEC (idnode) != NULL), "Vardec not found in function RenameId()");

    if (NODE_TYPE (ID_VARDEC (idnode)) == N_objdef) {
        FREE (ID_NAME (idnode));
        ID_NAME (idnode) = StringCopy (OBJDEF_NAME (ID_VARDEC (idnode)));
        /*
         * The global object's definition has already been renamed.
         */
    } else {
#ifdef TAGGED_ARRAYS
        dclass = GetClassFromTypes (ID_TYPE (idnode));
        uclass = GetUniFromTypes (ID_TYPE (idnode));

        ID_NAME (idnode) = PRECRenameLocalIdentifier (ID_NAME (idnode), dclass, uclass);
#else  /* TAGGED ARRAYS */
        ID_NAME (idnode) = PRECRenameLocalIdentifier (ID_NAME (idnode));
#endif /* TAGGED_ARRAYS */
    }

    DBUG_RETURN (idnode);
}

/******************************************************************************
 *
 * function:
 *   ids *PrecompileIds(ids *id)
 *
 * description:
 *   This function performs the renaming of identifiers stored within ids-chains.
 *   It also removes those identifiers from the chain which are marked as
 *   'artificial'.
 *
 ******************************************************************************/

static ids *
PrecompileIds (ids *id)
{
#ifdef TAGGED_ARRAYS
    data_class_t dclass;
    uniqueness_class_t uclass;
#endif TAGGED_ARRAYS

    DBUG_ENTER ("PrecompileIds");

    while ((id != NULL) && (IDS_STATUS (id) == ST_artificial)) {
        id = FreeOneIds (id);
    }

    if (id != NULL) {

        if (NODE_TYPE (IDS_VARDEC (id)) == N_objdef) {
            FREE (IDS_NAME (id));
            IDS_NAME (id) = StringCopy (OBJDEF_NAME (IDS_VARDEC (id)));
            /*
             * The global object's definition has already been renamed.
             */
        } else {
#ifdef TAGGED_ARRAYS
            dclass = GetClassFromTypes (IDS_TYPE (id));
            uclass = GetUniFromTypes (IDS_TYPE (id));
            IDS_NAME (id) = PRECRenameLocalIdentifier (IDS_NAME (id), dclass, uclass);
#else  /* TAGGED_ARRAYS */
            IDS_NAME (id) = PRECRenameLocalIdentifier (IDS_NAME (id));
#endif /* TAGGED_ARRAYS */
        }

        if (IDS_NEXT (id) != NULL) {
            IDS_NEXT (id) = PrecompileIds (IDS_NEXT (id));
        }
    }

    DBUG_RETURN (id);
}

/*
 *
 *  functionname  : RenameTypes
 *  arguments     : 1) types structure
 *  description   : renames the given type if it is a user-defined
 *                  SAC-type.
 *                  Chains of types structures are considered.
 *  global vars   :
 *  internal funs : ---
 *  external funs : Malloc, sprintf, strlen
 *  macros        : TREE, DBUG, FREE
 *
 *  remarks       : The complete new name is stored in NAME while
 *                  MOD is set to NULL.
 *
 */

static types *
RenameTypes (types *type)
{
    char *tmp;

    DBUG_ENTER ("RenameTypes");

    if (TYPES_MOD (type) != NULL) {
        if (0 == strcmp (TYPES_MOD (type), "_MAIN")) {
            tmp = (char *)Malloc (sizeof (char) * (strlen (TYPES_NAME (type)) + 6));
            sprintf (tmp, "SACt_%s", TYPES_NAME (type));
        } else {
            tmp = (char *)Malloc (
              sizeof (char)
              * (strlen (TYPES_NAME (type)) + strlen (TYPES_MOD (type)) + 8));
            sprintf (tmp, "SACt_%s__%s", TYPES_MOD (type), TYPES_NAME (type));
        }

        DBUG_PRINT ("PREC", ("renaming type %s:%s to %s", TYPES_MOD (type),
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

static node *
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

static node *
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

        if (FUNDEF_STATUS (fun) == ST_spmdfun) {
            new_name = (char *)Malloc (sizeof (char) * (strlen (FUNDEF_NAME (fun)) + 6));
            sprintf (new_name, "SACf_%s", FUNDEF_NAME (fun));
        } else {
            args = FUNDEF_ARGS (fun);

            while (args != NULL) {
                length += strlen (ARG_TYPESTRING (args)) + 1;
                args = ARG_NEXT (args);
            }

            if (0 == strcmp (FUNDEF_MOD (fun), "_MAIN")) {
                length += (strlen (FUNDEF_NAME (fun)) + 7);

                new_name = (char *)Malloc (sizeof (char) * length);

                sprintf (new_name, "SACf_%s_", FUNDEF_NAME (fun));
            } else {
                length += (strlen (FUNDEF_NAME (fun)) + strlen (FUNDEF_MOD (fun)) + 9);

                new_name = (char *)Malloc (sizeof (char) * length);

                sprintf (new_name, "SACf_%s__%s_", FUNDEF_MOD (fun), FUNDEF_NAME (fun));
            }

            args = FUNDEF_ARGS (fun);

            while (args != NULL) {
                strcat (new_name, "_");
                strcat (new_name, ARG_TYPESTRING (args));
                args = ARG_NEXT (args);
            }
        }

        DBUG_PRINT ("PREC", ("renaming function %s:%s to %s", FUNDEF_MOD (fun),
                             FUNDEF_NAME (fun), new_name));

        FREE (FUNDEF_NAME (fun));

        /* don't free FUNDEF_MOD(fun) because it is shared !! */

        FUNDEF_NAME (fun) = new_name;
        FUNDEF_MOD (fun) = NULL;
    } else {
        if ((FUNDEF_PRAGMA (fun) != NULL) && (FUNDEF_LINKNAME (fun) != NULL)) {
            /*
             *  These are C-functions with additional pragma 'linkname'.
             */

            DBUG_PRINT ("PREC", ("renaming function %s to %s", FUNDEF_NAME (fun),
                                 FUNDEF_LINKNAME (fun)));

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
 *   node *AdjustFoldFundef( node *fundef,
 *                           ids *acc, node *cexpr)
 *
 * description:
 *   Returns the given fold-fun definition 'fundef' with adjusted var-names.
 *
 * parameters:
 *   'acc' is the accumulator variable.
 *   'cexpr' is the expression in the operation part.
 *
 ******************************************************************************/

node *
AdjustFoldFundef (node *fundef, ids *acc, node *cexpr)
{
    node *accvar, *funap, *fold_let;

    DBUG_ENTER ("AdjustFoldFundef");

    DBUG_ASSERT ((fundef != NULL), "fundef is NULL!");
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "no fundef found!");

    /*
     * first, we create a let-expression of the form
     *    <acc> = <funname>( <acc>, <cexpr>);
     */

    accvar = MakeId (StringCopy (IDS_NAME (acc)), StringCopy (IDS_MOD (acc)), ST_regular);
    ID_VARDEC (accvar) = IDS_VARDEC (acc);
    DBUG_ASSERT ((ID_VARDEC (accvar) != NULL), "vardec is missing");

    funap = MakeAp (StringCopy (FUNDEF_NAME (fundef)), StringCopy (FUNDEF_MOD (fundef)),
                    MakeExprs (accvar, MakeExprs (DupTree (cexpr, NULL), NULL)));
    AP_FUNDEF (funap) = fundef;

    fold_let = MakeLet (funap, DupOneIds (acc, NULL));

    /*
     * then we use this dummy let-expression to adjust the fundef
     */
    fundef = AdjustIdentifiers (fundef, fold_let);

    /*
     * let-expression is useless now
     */
    fold_let = FreeTree (fold_let);

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *PRECmodul( node *arg_node, node *arg_info)
 *
 * description:
 *   starts traversal mechanism for objdef and fundef nodes.
 *
 ******************************************************************************/

node *
PRECmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECmodul");

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
        if (0 == strcmp (TYPEDEF_MOD (arg_node), "_MAIN")) {
            tmp = (char *)Malloc (sizeof (char) * (strlen (TYPEDEF_NAME (arg_node)) + 6));
            sprintf (tmp, "SACt_%s", TYPEDEF_NAME (arg_node));
        } else {
            tmp = (char *)Malloc (
              sizeof (char)
              * (strlen (TYPEDEF_NAME (arg_node)) + strlen (TYPEDEF_MOD (arg_node)) + 8));
            sprintf (tmp, "SACt_%s__%s", TYPEDEF_MOD (arg_node), TYPEDEF_NAME (arg_node));
        }

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
    char *new_name;

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

        FREE (OBJDEF_VARNAME (arg_node));
        /*
         * OBJDEF_VARNAME is no longer used for the generation of the final C code
         * identifier of a global object.
         */

        if (0 == strcmp (OBJDEF_MOD (arg_node), "_MAIN")) {
            new_name
              = (char *)Malloc (sizeof (char) * (strlen (OBJDEF_NAME (arg_node)) + 6));

            sprintf (new_name, "SACo_%s", OBJDEF_NAME (arg_node));
        } else {
            new_name = (char *)Malloc (
              sizeof (char)
              * (strlen (OBJDEF_NAME (arg_node)) + strlen (OBJDEF_MOD (arg_node)) + 8));

            sprintf (new_name, "SACo_%s__%s", OBJDEF_MOD (arg_node),
                     OBJDEF_NAME (arg_node));
        }

        FREE (OBJDEF_NAME (arg_node));
        OBJDEF_MOD (arg_node) = NULL;
        OBJDEF_NAME (arg_node) = new_name;
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   precompilation of an N_fundef node.
 *
 ******************************************************************************/

node *
PRECfundef (node *arg_node, node *arg_info)
{
    char *keep_name, *keep_mod;
    statustype keep_status, keep_attrib;
    int i;

    DBUG_ENTER ("PRECfundef");
    DBUG_PRINT ("PRECjhs", ("entering %s", FUNDEF_NAME (arg_node)));

    /*
     *  The body of an imported inline function is removed.
     */
    /*  FUNDEF_ATTRIB is reused by multithread ... this here seems to be superfluous (?)
        if problems occur we need another way to store ST_call_[rep|any|mt|st] (jhs)
      if (((FUNDEF_STATUS(arg_node) == ST_imported_mod)
           || (FUNDEF_STATUS(arg_node) == ST_imported_class))&&
          (FUNDEF_ATTRIB(arg_node) != ST_generic) &&
          (FUNDEF_BODY(arg_node) != NULL)) {
        FUNDEF_BODY(arg_node) = FreeTree(FUNDEF_BODY(arg_node));
        FUNDEF_RETURN(arg_node) = NULL;
      }
    */

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
     * Now, the data flow mask base is updated.
     * This is necessary because some local identifiers are removed while all
     * others are renamed.
     */

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_DFM_BASE (arg_node)
          = DFMUpdateMaskBaseAfterRenaming (FUNDEF_DFM_BASE (arg_node),
                                            FUNDEF_ARGS (arg_node),
                                            FUNDEF_VARDEC (arg_node));
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
        if ((FUNDEF_MOD (arg_node) == NULL) && (FUNDEF_STATUS (arg_node) != ST_spmdfun)) {
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
 *   node *PRECarg(node *arg_node, node *arg_info)
 *
 * description:
 *   An artificial argument is removed, the attribs are switched:
 *       ST_readonly_reference -> ST_regular
 *       ST_was_reference -> ST_inout
 *
 *   INFO_PREC_CNT_ARTIFICIAL is used to count the number of artificial
 *   return values.
 *
 ******************************************************************************/

node *
PRECarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECarg");

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

        if (ARG_NAME (arg_node) != NULL) {
            /*
             * The attribute ARG_NAME may not be set in the case of imported function
             * declarations.
             */
#ifdef TAGGED_ARRAYS
            ARG_NAME (arg_node)
              = PRECRenameLocalIdentifier (ARG_NAME (arg_node),
                                           GetClassFromTypes (ARG_TYPE (arg_node)),
                                           GetUniFromTypes (ARG_TYPE (arg_node)));
#else  /* TAGGED_ARRAYS */
            ARG_NAME (arg_node) = PRECRenameLocalIdentifier (ARG_NAME (arg_node));
#endif /* TAGGED_ARRAYS */
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

#ifdef TAGGED_ARRAYS
        VARDEC_NAME (arg_node)
          = PRECRenameLocalIdentifier (VARDEC_NAME (arg_node),
                                       GetClassFromTypes (ARG_TYPE (arg_node)),
                                       GetUniFromTypes (ARG_TYPE (arg_node)));
#else  /* TAGGED_ARRAYS */
        VARDEC_NAME (arg_node) = PRECRenameLocalIdentifier (VARDEC_NAME (arg_node));
#endif /* TAGGED_ARRAYS */
        if (VARDEC_NEXT (arg_node) != NULL) {
            VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECassign( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
PRECassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECassign");

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
 *   node *PREClet( node *arg_node, node *arg_info)
 *
 * description:
 *   removes all artificial identifiers on the left hand side of a let.
 *
 ******************************************************************************/

node *
PREClet (node *arg_node, node *arg_info)
{
    node *wl_node;
    ids *wl_ids;

    DBUG_ENTER ("PREClet");

    LET_IDS (arg_node) = PrecompileIds (LET_IDS (arg_node));

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    if (LET_EXPR (arg_node) == NULL) {
        arg_node = FreeTree (arg_node);
    } else {
        wl_ids = LET_IDS (arg_node);
        wl_node = LET_EXPR (arg_node);
        if ((NODE_TYPE (wl_node) == N_Nwith2)
            && ((NWITH2_TYPE (wl_node) == WO_foldfun)
                || (NWITH2_TYPE (wl_node) == WO_foldprf))) {
            /*
             * Adjust definition of the pseudo fold-fun.
             *
             * Note: It is sufficient to take the CEXPR of the first code-node, because
             *       in a fold with-loop all CEXPR-ids have the same name!
             */
            NWITH2_FUNDEF (wl_node)
              = AdjustFoldFundef (NWITH2_FUNDEF (wl_node), wl_ids,
                                  NCODE_CEXPR (NWITH2_CODE (wl_node)));
        }
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
 *  remarks       :
 *
 */

static node *
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
            } else {
                if ((NODE_TYPE (ID_VARDEC (expr)) == N_vardec)
                    && (VARDEC_STATUS (ID_VARDEC (expr)) == ST_artificial)) {
                    ID_VARDEC (expr) = VARDEC_OBJDEF (ID_VARDEC (expr));
                }
            }

            EXPRS_EXPR (current) = RenameId (EXPRS_EXPR (current));
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
 *  remarks       :
 *
 */

static node *
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
            EXPRS_EXPR (tmp) = RenameId (EXPRS_EXPR (tmp));
            EXPRS_NEXT (tmp) = RETURN_REFERENCE (ret_node);
            RETURN_REFERENCE (ret_node) = tmp;
        }
    } else {
        /*
         * All expressions in a return-statement are guaranteed to be of
         * node type N_id.
         */
        EXPRS_EXPR (ret_exprs) = RenameId (EXPRS_EXPR (ret_exprs));
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
                arg_node = RenameId (arg_node);
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
             *  type which implies that it is an identifier.
             */

            arg_node = RenameId (arg_node);
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
        } else {
            if ((NODE_TYPE (ID_VARDEC (arg_node)) == N_vardec)
                && (VARDEC_STATUS (ID_VARDEC (arg_node)) == ST_artificial)) {
                ID_VARDEC (arg_node) = VARDEC_OBJDEF (ID_VARDEC (arg_node));
            }
        }

        arg_node = RenameId (arg_node);

        ID_MAKEUNIQUE (arg_node) = 0;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECgenerator(node *arg_node, node *arg_info)
 *
 * description:
 *   This function does the renaming of the index vector variable
 *   for the old with-loop.
 *
 ******************************************************************************/

node *
PRECgenerator (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECgenerator");

    GEN_LEFT (arg_node) = Trav (GEN_LEFT (arg_node), arg_info);
    GEN_RIGHT (arg_node) = Trav (GEN_RIGHT (arg_node), arg_info);

    GEN_IDS (arg_node) = PrecompileIds (GEN_IDS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECNwithid(node *arg_node, node *arg_info)
 *
 * description:
 *   This function does the renaming of the index vector variable
 *   as well as its scalar counterparts for the new with-loop.
 *
 ******************************************************************************/

node *
PRECNwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECNwithid");

    NWITHID_VEC (arg_node) = PrecompileIds (NWITHID_VEC (arg_node));
    NWITHID_IDS (arg_node) = PrecompileIds (NWITHID_IDS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECdo(node *arg_node, node *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PRECdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECdo");

    DO_COND (arg_node) = Trav (DO_COND (arg_node), arg_info);

    DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);

    DO_USEVARS (arg_node) = PrecompileIds (DO_USEVARS (arg_node));

    DO_DEFVARS (arg_node) = PrecompileIds (DO_DEFVARS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECwhile(node *arg_node, node *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PRECwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECwhile");

    WHILE_COND (arg_node) = Trav (WHILE_COND (arg_node), arg_info);

    WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);

    WHILE_USEVARS (arg_node) = PrecompileIds (WHILE_USEVARS (arg_node));

    WHILE_DEFVARS (arg_node) = PrecompileIds (WHILE_DEFVARS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECcond(node *arg_node, node *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PRECcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECcond");

    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    COND_THENVARS (arg_node) = PrecompileIds (COND_THENVARS (arg_node));

    COND_ELSEVARS (arg_node) = PrecompileIds (COND_ELSEVARS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECwith(node *arg_node, node *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PRECwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECwith");

    WITH_GEN (arg_node) = Trav (WITH_GEN (arg_node), arg_info);

    WITH_OPERATOR (arg_node) = Trav (WITH_OPERATOR (arg_node), arg_info);

    WITH_USEVARS (arg_node) = PrecompileIds (WITH_USEVARS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECNwith2(node *arg_node, node *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PRECNwith2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECNwith2");

    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);

    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);

    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    NWITH2_DEC_RC_IDS (arg_node) = PrecompileIds (NWITH2_DEC_RC_IDS (arg_node));

    /*
     *  Since the scheduling specification may contain the names of local
     *  identifiers, these have to be renamed according to the general renaming
     *  scheme implemented by this compiler phase.
     */
    if (NWITH2_SCHEDULING (arg_node) != NULL) {
        NWITH2_SCHEDULING (arg_node)
          = SCHPrecompileScheduling (NWITH2_SCHEDULING (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECNcode(node *arg_node, node *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PRECNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECNcode");

    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    NCODE_INC_RC_IDS (arg_node) = PrecompileIds (NCODE_INC_RC_IDS (arg_node));

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECsync(node *arg_node, node *arg_info)
 *
 * description:
 *   Was used for renaming SYNC_SCHEDULE, since this has move to the
 *   with-loops it's not done here anymore, but i kept the function, not
 *   knowing if there would be any problems if if i killed it. (jhs)
 *
 ******************************************************************************/

node *
PRECsync (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECsync");

    SYNC_REGION (arg_node) = Trav (SYNC_REGION (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECWLseg(node *arg_node, node *arg_info)
 *
 * description:
 *   Since the scheduling specification may contain the names of local
 *   identifiers, these have to be renamed according to the general renaming
 *   scheme implemented by this compiler phase.
 *
 ******************************************************************************/

node *
PRECWLseg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECWLseg");

    if (WLSEG_SCHEDULING (arg_node) != NULL) {
        WLSEG_SCHEDULING (arg_node)
          = SCHPrecompileScheduling (WLSEG_SCHEDULING (arg_node));
    }

    WLSEG_CONTENTS (arg_node) = Trav (WLSEG_CONTENTS (arg_node), arg_info);

    if (WLSEG_NEXT (arg_node) != NULL) {
        WLSEG_NEXT (arg_node) = Trav (WLSEG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECWLsegVar(node *arg_node, node *arg_info)
 *
 * description:
 *   Since the scheduling specification may contain the names of local
 *   identifiers, these have to be renamed according to the general renaming
 *   scheme implemented by this compiler phase.
 *
 ******************************************************************************/

node *
PRECWLsegVar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECWLsegVar");

    if (WLSEGVAR_SCHEDULING (arg_node) != NULL) {
        WLSEGVAR_SCHEDULING (arg_node)
          = SCHPrecompileScheduling (WLSEGVAR_SCHEDULING (arg_node));
    }

    WLSEGVAR_CONTENTS (arg_node) = Trav (WLSEGVAR_CONTENTS (arg_node), arg_info);

    if (WLSEGVAR_NEXT (arg_node) != NULL) {
        WLSEGVAR_NEXT (arg_node) = Trav (WLSEGVAR_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

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
