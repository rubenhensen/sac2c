/*
 *
 * $Log$
 * Revision 1.29  1998/02/06 18:49:14  dkr
 * new function RCNwithid()
 *
 * Revision 1.28  1998/02/05 15:33:15  dkr
 * adjusted refcnt in N_pre and N_post
 *
 * Revision 1.27  1998/01/28 19:42:18  dkr
 * added dummy-implementations of RC-funs for new with-loop
 *
 * Revision 1.26  1997/08/29 09:07:44  sbs
 * RCprf modified!
 * if N_prf == F_reshape  do NOT set arg_info to N_prf!
 * Since F_reshape is compiled in exactly the same way as an assignment,
 * it has to be refcounted in the same manner!
 *
 * Revision 1.25  1997/05/02 09:30:51  cg
 * IsArray(): Arrays with known dimension but unknown shape are now recognized.
 *
 * Revision 1.24  1997/03/19  15:29:59  cg
 * Now, module/class implementations without any functions are supported
 *
 * Revision 1.23  1996/09/02  17:41:51  sbs
 * commented ref_dump in RefLoop
 *
 * Revision 1.22  1996/05/29  16:48:11  sbs
 * -Inserted tree-macros in RCfundef, RCassign, RCid, RClet
 * -Inserted Comments in RCfundef, RCassign, RCid, RClet
 *
 * -N_info node in Refcount deleted.
 * -Inserted RCprf. RCprf traverse the args with arg_info pointing to the N_prf!
 * -arg_info is relevant for N_id nodes only:
 *    NULL  indicates a normal use i.e. in user-defined
 *          functions or aliasing.
 *    *node indicates a use as argument to a primitive
 *          function. *node points to the respective N_prf
 *          node
 *
 * if opt_rco==1 (default) all ids in arg-pos of a prf are either set to 1
 * (= last usage of the id) or -1 (=non-last usage of the id).
 *
 * Revision 1.21  1996/01/22  17:34:14  cg
 * IsBoxed and IsUnique moved to refcount.c
 *
 * Revision 1.20  1996/01/21  14:18:12  cg
 * new macro MUST_REFCOUNT to distinguish between refcounted
 * and not refcounted variables.
 *
 * Revision 1.19  1995/12/06  09:48:22  cg
 * external implicit types (void*) are now refcounted too.
 *
 * Revision 1.18  1995/10/06  17:08:34  cg
 * adjusted calls to function MakeIds (now 3 parameters)
 *
 * Revision 1.17  1995/06/26  13:04:02  hw
 * DBUG_ASSERT in RCwith inserted
 *
 * Revision 1.16  1995/06/26  08:12:35  asi
 * parameter for GenerateMasks changed
 *
 * Revision 1.15  1995/06/07  14:26:49  asi
 * inserted call of function GenerateMasks in function Refcount
 *
 * Revision 1.14  1995/05/19  13:29:59  hw
 * - bug fixed in RCloop ( refcounts of variables that are used before they
 *   will be defined will be increased)
 *
 * Revision 1.13  1995/05/18  15:46:17  hw
 * - changed RCwith ( increase refcount of array that will be modified
 *    in modarray_with-loop in all cases (used or not used in the
 *    with-loop body))
 *
 * Revision 1.12  1995/05/17  14:37:58  hw
 * bug fixed in RCloop ( refcounts of variables that belongs to
 *  'virtual function application' will be set correctly now, i hope ;-)
 *
 * Revision 1.11  1995/05/09  14:46:06  hw
 * bug fixed in RCwith ( set refcount of used variables correctly )
 *
 * Revision 1.10  1995/05/03  07:46:17  hw
 * bug fixed in Store, StoreAndInit & Restore (now refcounting will
 *  work after optimization ; eliminated variable declarations don't
 *  matter anymore )
 *
 * Revision 1.9  1995/04/28  17:27:34  hw
 * - added RCgen
 * - store information about used before defined variables
 *   only arrays) of a with_loop in arg_node->node[2] od N_with
 * - bug fixed in RCwith (set new refcount correctly )
 *
 * Revision 1.8  1995/04/11  15:10:55  hw
 * changed args of functio IsArray
 *
 * Revision 1.7  1995/04/04  16:21:04  hw
 * changed IsArray   (now arrays with unknown shapes will be treated as arrays)
 *
 * Revision 1.6  1995/03/28  12:09:38  hw
 * added #include "internal_lib.h"
 *
 * Revision 1.5  1995/03/16  17:40:35  hw
 * RCwith and RCcon (used for N_genarray and N_modarray) inserted
 * bug fixed in RCfundef
 *
 * Revision 1.4  1995/03/16  14:04:51  hw
 * changed mechanism of refcounting
 *  - refcounts are counted in var-decs and are stored and restored
 *    when necessary
 *  - refcounts in loops and conditionals correctly (i hope )
 *  - with-statements are still missing
 *
 * Revision 1.3  1995/03/14  18:45:21  hw
 * renamed RCwhile to RCloop
 * this version handles do- and while-loops correctly.
 * conditionals are not implemente correctly
 *
 * Revision 1.2  1995/03/13  15:18:47  hw
 * RCfundef and Refcount inserted
 *
 * Revision 1.1  1995/03/09  16:17:01  hw
 * Initial revision
 *
 *
 */

#include <stdlib.h>

#include "tree.h"
#include "my_debug.h"
#include "dbug.h"
#include "typecheck.h" /* to use LookupType */
#include "traverse.h"
#include "internal_lib.h"
#include "free.h"
#include "refcount.h"

#define TYPES info.types
#define ID info.ids->id
#define VAR_DEC info.ids->node
#define CONTEXT info.cint
#define ID_REF refcnt

#define DUB_ID_NODE(a, b)                                                                \
    a = MakeNode (N_id);                                                                 \
    a->info.ids = MakeIds (StringCopy (b->info.ids->id), NULL, ST_regular);              \
    a->VAR_DEC = b->VAR_DEC

#define VAR_DEC_2_ID_NODE(a, b)                                                          \
    a = MakeNode (N_id);                                                                 \
    a->info.ids = MakeIds (StringCopy (b->info.types->id), NULL, ST_regular);            \
    a->VAR_DEC = b;                                                                      \
    a->ID_REF = b->refcnt;

#define MUST_REFCOUNT(type) (IsArray (type) || IsNonUniqueHidden (type))

extern node *GenerateMasks (node *arg_node, node *arg_info);
/* from optimize.c */

static int varno;         /* used to store the number of known variables in a
                             sac-function (used for mask[])
                           */
static int args_no;       /* number of arguments of current function */
static node *fundef_node; /* pointer to current function declaration */

/*
 *
 *  functionname  : IsBoxed
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
IsBoxed (types *type)
{
    int ret = 0;
    node *tdef;

    DBUG_ENTER ("IsBoxed");

    if (TYPES_DIM (type) != 0) {
        ret = 1;
    } else {
        if (TYPES_BASETYPE (type) == T_user) {
            tdef = LookupType (TYPES_NAME (type), TYPES_MOD (type), 042);
            DBUG_ASSERT (tdef != NULL, "Failed attempt to look up typedef");

            if ((TYPEDEF_DIM (tdef) != 0) || (TYPEDEF_BASETYPE (tdef) == T_hidden)) {
                ret = 1;
            }
        }
    }

    DBUG_RETURN (ret);
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

int
IsUnique (types *type)
{
    int ret = 0;
    node *tdef;

    DBUG_ENTER ("IsUnique");

    if (TYPES_BASETYPE (type) == T_user) {
        tdef = LookupType (TYPES_NAME (type), TYPES_MOD (type), 042);
        DBUG_ASSERT (tdef != NULL, "Failed attempt to look up typedef");

        if (TYPEDEF_ATTRIB (tdef) == ST_unique) {
            ret = 1;
        }
    }

    DBUG_RETURN (ret);
}

/*
 *
 *  functionname  : IsArray
 *  arguments     : 1) types struct
 *  description   : checks whether 1) is a declaration of an array or not
 *  global vars   :
 *  internal funs :
 *  external funs : LookupType
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
int
IsArray (types *type)
{
    node *tdef;
    int ret = 0;

    DBUG_ENTER ("IsArray");

    DBUG_PRINT ("RC", ("looking for %s", type->id));

    if ((SCALAR != TYPES_DIM (type)) && (ARRAY_OR_SCALAR != TYPES_DIM (type))) {
        ret = 1;
    } else {
        if (T_user == TYPES_BASETYPE (type)) {
            tdef = LookupType (TYPES_NAME (type), TYPES_MOD (type), 042);
            /* 042 is only a dummy argument */
            DBUG_ASSERT (tdef != NULL, "Failed attempt to look up typedef");

            if ((SCALAR != TYPEDEF_DIM (tdef))
                && (ARRAY_OR_SCALAR != TYPEDEF_DIM (tdef))) {
                ret = 1;
            }
        }
    }

    DBUG_PRINT ("RC", ("%d", ret));

    DBUG_RETURN (ret);
}

/*
 *
 *  functionname  : IsNonUniqueHidden
 *  arguments     : 1) type of a N_vardec or N_arg node
 *  description   : checks if the type is a non-unique void* one
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : LookupType
 *  macros        : DBUG, TREE
 *
 *  remarks       : used to detect non-array variables which have to be
 *                  refcounted.
 *
 */

int
IsNonUniqueHidden (types *type)
{
    int ret = 0;
    node *tdef;

    DBUG_ENTER ("IsNonUniqueHidden");

    if (TYPES_BASETYPE (type) == T_user) {
        tdef = LookupType (TYPES_NAME (type), TYPES_MOD (type), 042);
        /* 042 is only a dummy argument */
        DBUG_ASSERT (tdef != NULL, "Failed attempt to look up typedef");

        if ((TYPEDEF_BASETYPE (tdef) == T_hidden)
            || (TYPEDEF_BASETYPE (tdef) == T_user)) {
            if (TYPEDEF_TNAME (tdef) == NULL) {
                if (TYPEDEF_ATTRIB (tdef) == ST_regular) {
                    ret = 1;
                }
            } else {
                tdef = LookupType (TYPEDEF_TNAME (tdef), TYPEDEF_TMOD (tdef), 042);
                DBUG_ASSERT (tdef != NULL, "Failed attempt to look up typedef");

                if (TYPEDEF_ATTRIB (tdef) == ST_regular) {
                    ret = 1;
                }
            }
        }
    }

    DBUG_RETURN (ret);
}

/*
 *
 *  functionname  : FindVarDec
 *  arguments     : 1) number of variable
 *  description   : returns pointer to vardec if found
 *                  returns NULL if not found
 *  global vars   : fundef_node, args_no
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
FindVarDec (int var_no)
{
    node *tmp, *ret_node = NULL;

    DBUG_ENTER ("FindVarDec");

    if (var_no < args_no)
        tmp = fundef_node->node[2]; /* set tmp to arguments of function */
    else
        tmp = fundef_node->node[0]->node[1]; /* set tmp to variable declaration */
    while (NULL != tmp)
        if (tmp->varno == var_no) {
            ret_node = tmp;
            break;
        } else
            tmp = tmp->node[0];

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : LookupId
 *  arguments     : 1) name
 *                  2) chain of N_id nodes
 *  description   : looks 1) up in 2)
 *                  returns pointer to N_id node if found
 *                  returns NULL if not found
 *  global vars   :
 *  internal funs :
 *  external funs : strcmp
 *  macros        : DBUG..., ID, P_FORMAT, NULL
 *
 *  remarks       :
 *
 */
node *
LookupId (char *id, node *id_node)
{
    node *tmp, *ret_node = NULL;

    DBUG_ENTER ("LookupId");

    tmp = id_node;
    while (NULL != tmp)
        if (0 == strcmp (id, tmp->ID)) {
            ret_node = tmp;
            break;
        } else
            tmp = tmp->node[0];
    DBUG_PRINT ("RC", ("found %s:" P_FORMAT, id, ret_node));
    DBUG_RETURN (ret_node);
}

/*
 *
 *  functionname  : StoreAndInit
 *  arguments     : 1) number to initialize
 *  description   : returns an arry of int that contains the refcount stored in
 *                  the variable declaration part
 *                  the refcount in the variable declartion part is set to 1) if
 *                   refcount was >= 1
 *  global vars   : fundef_node, args_no, varno
 *  internal funs :
 *  external funs : Malloc, sizeof
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
int *
StoreAndInit (int n)
{
    int *dump, i, k;
    node *var_dec;

    DBUG_ENTER ("StoreAndInit");
    dump = (int *)Malloc (sizeof (int) * varno);
    var_dec = fundef_node->node[2]; /* arguments of function */
    for (i = 0; i < args_no; i++) {
        dump[i] = var_dec->refcnt;
        if (var_dec->refcnt > 0)
            var_dec->refcnt = n;
        var_dec = var_dec->node[0];
    }
    var_dec = fundef_node->node[0]->node[1]; /* variable declaration */
    for (k = i; k < varno; k++) {
        if (k == var_dec->varno) {
            /* var_dec is the right node belonging to 'k', so store the refcount */
            dump[k] = var_dec->refcnt;
            if (var_dec->refcnt > 0)
                var_dec->refcnt = n;
            var_dec = var_dec->node[0];
        } else
            dump[k] = -1; /* var_dec belonging to 'k' was eliminated while
                           * optimasation, so store -1 at this position
                           */
    }

    DBUG_RETURN (dump);
}

/*
 *
 *  functionname  : Store
 *  arguments     :
 *  description   : returns an arry of int that contains the refcount of
 *                  the variable declaration
 *  global vars   : fundef_node, args_no, varno
 *  internal funs :
 *  external funs : Malloc, sizeof
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
int *
Store ()
{
    int *dump, i, k;
    node *var_dec;

    DBUG_ENTER ("Store");
    dump = (int *)Malloc (sizeof (int) * varno);
    var_dec = fundef_node->node[2]; /* arguments of function */
    for (i = 0; i < args_no; i++) {
        dump[i] = var_dec->refcnt;
        var_dec = var_dec->node[0];
    }
    var_dec = fundef_node->node[0]->node[1]; /* variable declaration */
    for (k = i; k < varno; k++)
        if (k == var_dec->varno) {
            /* var_dec is the right node belonging to 'k', so store the refcount */
            dump[k] = var_dec->refcnt;
            var_dec = var_dec->node[0];
        } else
            dump[k] = -1; /* var_dec belonging to 'k' was eliminated while
                           * optimasation, so store -1 at this position
                           */

    DBUG_RETURN (dump);
}

/*
 *
 *  functionname  : Restore
 *  arguments     : 1) dump
 *  description   : copies dump-entries to refcounts of variable declaration
 *  global vars   : args_no, fundef_node, varno
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
void
Restore (int *dump)
{
    int i;
    node *var_dec;

    DBUG_ENTER ("Restore");

    var_dec = fundef_node->node[2]; /* arguments of function */
    for (i = 0; i < args_no; i++) {
        var_dec->refcnt = dump[i];
        var_dec = var_dec->node[0];
    }
    var_dec = fundef_node->node[0]->node[1]; /* variable declaration */
    while (NULL != var_dec) {
        var_dec->refcnt = dump[var_dec->varno];
        var_dec = var_dec->node[0];
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : Refcount
 *  arguments     : 1) argument node
 *  description   : starts the refcount-traversal (sets act_tab)
 *  global vars   : act_tab, refcnt_tab
 *  internal funs :
 *  external funs : MakeNode
 *  macros        : DBUG..., FREE
 *
 *  remarks       :
 *
 */
node *
Refcount (node *arg_node)
{
    DBUG_ENTER ("Refcount");

    /*
     * generate masks
     */
    arg_node = GenerateMasks (arg_node, NULL);

    act_tab = refcnt_tab;

    if (N_modul == NODE_TYPE (arg_node)) {
        if (MODUL_FUNS (arg_node) != NULL) {
            DBUG_ASSERT ((N_fundef == NODE_TYPE (MODUL_FUNS (arg_node))), "wrong node ");
            MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), NULL);
        }
    } else {
        DBUG_ASSERT ((N_fundef == NODE_TYPE (arg_node)), "wrong node ");
        arg_node = Trav (arg_node, NULL);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCarg
 *  arguments     : 1) argument node
 *                  2) info node unused
 *  description   : initializes ARG_REFCNT for refcounted (0) and not
 *                  refcounted parameters (-1)
 *                  This information is needed by compile.c to distinguish
 *                  between refcounted and not refcounted parameters.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG... , MUST_REFCOUNT
 *
 *  remarks       :
 *
 */

node *
RCarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCarg");

    if (MUST_REFCOUNT (ARG_TYPE (arg_node))) {
        ARG_REFCNT (arg_node) = 0;
    } else {
        ARG_REFCNT (arg_node) = -1;
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCfundef
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   : calls Trav to traverse body of function
 *                  sets varno ,fundef_node and arg_no
 *  global vars   : varno, fundef_node, args_no
 *  internal funs :
 *  external funs :
 *  macros        : DBUG..., NULL
 *
 *  remarks       :
 *
 */
node *
RCfundef (node *arg_node, node *arg_info)
{
    node *args;

    DBUG_ENTER ("RCfundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        /*
         *  The args are traversed to initialize ARG_REFCNT.
         *  Refcounted objects are initialized by 0 others by -1.
         *  This information is needed by compile.c .
         */
    }

    if (NULL != FUNDEF_BODY (arg_node)) {
        /* setting some global variables to use with the 'mask'
         * and storage of refcounts
         */
        varno = FUNDEF_VARNO (arg_node);
        fundef_node = arg_node;
        args = FUNDEF_ARGS (arg_node);
        args_no = 0;
        while (NULL != args) {
            args_no += 1;
            args = ARG_NEXT (args);
        }

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (NULL != FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCassign
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   : Realizes bottom-up traversal for refcounting
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
RCassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCassign");

    /* Bottom up traversal!! */
    if (NULL != ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    } else /* this must be the return-statement!!! */
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), NULL);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCloop
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   : refcounts while- and do-loops
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       : - v1: set of vars that are used before they will be defined
 *                        in the body of the loop
 *                  - v2: set of vars that are defined in the body of the loop
 *                        and are used in the rest of the program
 *                  ( array-vars are only considered )
 */
node *
RCloop (node *arg_node, node *arg_info)
{
    node *new_info, *var_dec, *id_node;
    long *defined_mask, *used_mask;
    int i, again = 0, use_old = 0;
    int *ref_dump;

    DBUG_ENTER ("RCloop");

    /* traverse body of loop */
    ref_dump = StoreAndInit (1); /* store the current ref-cnt values from
                                    the VARDECs to ref_dump and initialize
                                    all ref-cnts from the function's vardec
                                    with 1 */

    arg_node->node[1] = Trav (arg_node->node[1], arg_info);

    if (NULL == arg_node->node[2])
        new_info = MakeNode (N_info); /* create new info node */
    else {
        new_info = arg_node->node[2];
        use_old = 1;
    }

    defined_mask = arg_node->node[1]->mask[0]; /* mask of defined variables */
    used_mask = arg_node->node[1]->mask[1];    /* mask of used variables */

    /* first compute sets v1 and v2 */
    for (i = 0; i < varno; i++)
        if ((defined_mask[i] > 0) || (used_mask[i] > 0)) {
            var_dec = FindVarDec (i);
            DBUG_ASSERT ((NULL != var_dec), "variable not found");
            if (MUST_REFCOUNT (var_dec->TYPES)) {

                /* first store used and defined variables (v1), (v2) */
                if ((defined_mask[i] > 0) && (ref_dump[i] > 0)) {
                    /* store refcount of defined variables in new_info->node[1] (v2)
                     */
                    if (0 == use_old) {
                        VAR_DEC_2_ID_NODE (id_node, var_dec);
                        id_node->ID_REF = ref_dump[i];
                        id_node->node[0] = new_info->node[1];
                        new_info->node[1] = id_node;
                        DBUG_PRINT ("RC", ("store defined var (v2) %s:%d", id_node->ID,
                                           id_node->ID_REF));
                    } else {
                        id_node = LookupId (var_dec->info.types->id, new_info->node[1]);
                        DBUG_ASSERT ((NULL != id_node), "var not found");
                        id_node->ID_REF = ref_dump[i];
                        DBUG_PRINT ("RC", ("changed defined var (v2) %s:%d", id_node->ID,
                                           id_node->ID_REF));
                    }
                }
                if ((used_mask[i] > 0) && (0 < var_dec->refcnt)) {
                    /* store refcount of used variables in new_info->node[0] (v1)
                     */
                    if (0 == use_old) {
                        VAR_DEC_2_ID_NODE (id_node, var_dec);
                        id_node->node[0] = new_info->node[0];
                        new_info->node[0] = id_node;
                        DBUG_PRINT ("RC", ("store used var (v1) %s:%d", id_node->ID,
                                           id_node->ID_REF));
                    } else {
                        id_node = LookupId (var_dec->info.types->id, new_info->node[0]);
                        DBUG_ASSERT ((NULL != id_node), "var not found");
                        id_node->ID_REF = var_dec->refcnt;
                        DBUG_PRINT ("RC", ("changed used var (v1) %s:%d", id_node->ID,
                                           id_node->ID_REF));
                    }
                }
            }
        }

    if (NULL != new_info->node[0]) {
        id_node = new_info->node[0];
        while ((0 == again) && (NULL != id_node)) {
            if ((0 < id_node->VAR_DEC->refcnt)
                && (0 == ref_dump[id_node->VAR_DEC->varno]))
                again = 1;
            else
                id_node = id_node->node[0];
        }
        if (1 == again) {
            DBUG_PRINT ("RC", ("while  loop again "));
            Restore (ref_dump);
            ref_dump = StoreAndInit (1);
            id_node = new_info->node[0];
            /* init all variables that are member of v1 with refcount 1 */
            while (NULL != id_node) {
                id_node->VAR_DEC->refcnt = 1;
                id_node = id_node->node[0];
            }

            /* refcount body of while loop again */
            arg_node->node[1] = Trav (arg_node->node[1], arg_info);
        }
    }

    /* compute new refcounts because of 'virtuell function application' */
    for (i = 0; i < varno; i++)
        if ((defined_mask[i] > 0) || (used_mask[i] > 0)) {
            var_dec = FindVarDec (i);
            DBUG_ASSERT ((NULL != var_dec), "variable not found");
            if (MUST_REFCOUNT (var_dec->TYPES)) {
                if ((used_mask[i] > 0) && (0 < var_dec->refcnt) && (1 == again)) {
                    /* update refcount of used variables  (v1)
                     */
                    id_node = LookupId (var_dec->info.types->id, new_info->node[0]);
                    DBUG_ASSERT ((NULL != id_node), "var not found");

                    id_node->ID_REF = var_dec->refcnt;
                    DBUG_PRINT ("RC", ("(v1) %s:%d", id_node->ID, id_node->ID_REF));
                }
                /* now compute new refcounts, because of 'virtuall function
                 * application'
                 */
                if ((defined_mask[i] > 0) && (ref_dump[i] > 0)) {
                    /* these will be the return-values of the virtual function */
                    if (N_do == arg_node->nodetype) {
                        /* check whether current variable is an argument of
                         * 'virtual function'
                         */
                        if (0 == var_dec->refcnt)
                            /* in this case the variable will be defined before it
                             * will be used, so we don't need it as argument of
                             * the virtual function (it can be freed earlier)
                             */
                            ref_dump[i] = 0;
                        else
                            ref_dump[i] = 1;
                    } else
                        ref_dump[i] = 1;

                    DBUG_PRINT ("RC", ("set refcount of %s(%d) to: %d",
                                       var_dec->info.types->id, i, ref_dump[i]));
                } else if ((used_mask[i] > 0) && (0 < var_dec->refcnt)) {
                    /* these variables are arguments of the virtual function */
                    ref_dump[i] += 1;
                    DBUG_PRINT ("RC", ("increased  refcount of %s(%d) to: %d",
                                       var_dec->info.types->id, i, ref_dump[i]));
                }
            }
        }

    /* store new_info for use while compilation */
    arg_node->node[2] = new_info;

    /* restore old vardec refcounts */
    Restore (ref_dump);
    FREE (ref_dump);

    /* traverse termination condition */
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCprf
 *  arguments     : 1) argument node
 *                  2) ignored
 *  description   : traverse the args with arg_info pointing to the N_prf!
 *                  if N_prf is F_reshape it has to be treated in exactly
 *                  the same way as if we had an assignment, i.e., traverse
 *                  with unchanged arg_info!
 *                  The reason for this situation is that it is compiled
 *                  as if it would be a simple assignment!
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *  remarks       :
 *
 */
node *
RCprf (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("RCprf");

    DBUG_PRINT ("RC", ("traversing args of prf-node (%08x) %s", arg_node,
                       mdb_prf[PRF_PRF (arg_node)]));
    if (PRF_PRF (arg_node) == F_reshape)
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
    else
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_node);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCid
 *  arguments     : 1) argument node
 *                  2) NULL == normal N_id node
 *                     *node == argument of N_prf-node!!
 *  description   : depending on the arg_info do one of the following:
 *                    a) Increment refcnt at vardec and set local refcnt
 *                       of the N_id node accordingly. (RC op needed)
 *                    b) Set local refcnt to -1. (no RC op)
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *  remarks       :
 *
 */
node *
RCid (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("RCid");

    if (MUST_REFCOUNT (ID_TYPE (arg_node))) {
        if ((arg_info == NULL) || (VARDEC_REFCNT (ID_VARDEC (arg_node)) == 0)
            || !opt_rco) {
            /* This N_id node either is NOT an argument of a primitive
             * function, or it is the last usage within the body of the
             * current function. (or refcount-optimization is turned off!)
             * In both cases, the refcnt is incremented and attached to
             * the N_id node.
             */
            DBUG_PRINT ("RC", ("RC for %s increased:", ID_NAME (arg_node)));
            VARDEC_REFCNT (ID_VARDEC (arg_node))++;
            ID_REFCNT (arg_node) = VARDEC_REFCNT (ID_VARDEC (arg_node));
        } else {
            /* This N_id node is argument to the N_prf node (*arg_info)
             * AND it is definitly not the last usage of it.
             * Therefore -1 is attached to the refcnt field if N_id (to
             * indicate that this operation does not need any refcnt
             * adjustments).
             */
            ID_REFCNT (arg_node) = -1;
        }
    } else {
        ID_REFCNT (arg_node) = -1; /* variable needs no refcount */
    }

    DBUG_PRINT ("RC",
                ("set refcnt of %s to %d:", ID_NAME (arg_node), ID_REFCNT (arg_node)));

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RClet
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   : set the refcnts of the defined variables to the
 *                  actual refcnt-values of the respective variable
 *                  declarations and reset these values to 0.
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
RClet (node *arg_node, node *arg_info)
{
    ids *ids;

    DBUG_ENTER ("RClet");
    DBUG_PRINT ("RC", ("line: %d", NODE_LINE (arg_node)));

    ids = LET_IDS (arg_node);
    while (NULL != ids) {
        if (MUST_REFCOUNT (IDS_VARDEC_TYPE (ids))) {
            IDS_REFCNT (ids) = VARDEC_REFCNT (IDS_VARDEC (ids));
            VARDEC_REFCNT (IDS_VARDEC (ids)) = 0;
        } else {
            IDS_REFCNT (ids) = -1;
        }
        ids = IDS_NEXT (ids);
    }

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCcond
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   : refcounts then- and else-part of conditional and computes
 *                  the maximum of refcounts in both parts
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
RCcond (node *arg_node, node *arg_info)
{
    node *new_info, *var_dec, *id_node;
    int *rest_dump, *then_dump, *else_dump, i, use_old = 0;
    DBUG_ENTER ("RCcond");

    /* store current vardec refcounts in rest_dump */
    rest_dump = Store ();
    arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    /* store vardec refcounts after refcounting then part */
    then_dump = Store ();

    /* get same refcounts as before refcounting else part */
    Restore (rest_dump);

    arg_node->node[2] = Trav (arg_node->node[2], arg_info);
    /* store vardec refcounts after refcounting else part */
    else_dump = Store ();

    /* now compute maximum of then- and else-dump and store it in vardec refcnt
     * store differences between then- and else dump in new_info->node[x]
     * -  x=0 if then_dump[i] < else_dump[i] (else_dump[i] - then_dump[i])
     * -  x=1 if else_dump[i] < then_dump[i] (then_dump[i] - else_dump[i])
     */
    if (NULL == arg_node->node[3])
        new_info = MakeNode (N_info);
    else {
        use_old = 1;
        new_info = arg_node->node[3];
    }

    for (i = 0; i < varno; i++)
        if (then_dump[i] < else_dump[i]) {
            var_dec = FindVarDec (i);
            DBUG_ASSERT ((NULL != var_dec), " var not found");
            if (0 == use_old) {
                VAR_DEC_2_ID_NODE (id_node, var_dec);
                id_node->ID_REF = else_dump[i] - then_dump[i];
                id_node->node[0] = new_info->node[0];
                new_info->node[0] = id_node;
                DBUG_PRINT ("RC",
                            ("append %s :%d to then-part", id_node->ID, id_node->ID_REF));
            } else {
                id_node = LookupId (var_dec->info.types->id, new_info->node[0]);
                DBUG_ASSERT ((NULL != id_node), "var not found");
                id_node->ID_REF = else_dump[i] - then_dump[i];
                DBUG_PRINT ("RC", ("changed %s :%d in then-part", id_node->ID,
                                   id_node->ID_REF));
            }
            var_dec->refcnt = else_dump[i];
            DBUG_PRINT ("RC", ("set refcount of %s to %d", id_node->ID, var_dec->refcnt));
        } else if (else_dump[i] < then_dump[i]) {
            var_dec = FindVarDec (i);
            DBUG_ASSERT ((NULL != var_dec), " var not found");
            if (0 == use_old) {
                VAR_DEC_2_ID_NODE (id_node, var_dec);
                id_node->ID_REF = then_dump[i] - else_dump[i];
                id_node->node[0] = new_info->node[1];
                new_info->node[1] = id_node;
                DBUG_PRINT ("RC",
                            ("append %s :%d to else-part", id_node->ID, id_node->ID_REF));
            } else {
                id_node = LookupId (var_dec->info.types->id, new_info->node[1]);
                DBUG_ASSERT ((NULL != id_node), "var not found");
                id_node->ID_REF = then_dump[i] - else_dump[i];
                DBUG_PRINT ("RC", ("changed %s :%d in then-part", id_node->ID,
                                   id_node->ID_REF));
            }
            var_dec->refcnt = then_dump[i];
            DBUG_PRINT ("RC", ("set refcount of %s to %d", id_node->ID, var_dec->refcnt));
        }

    /* store refcount information for use while compilation */
    arg_node->node[3] = new_info;

    /* free the dumps */
    FREE (rest_dump);
    FREE (then_dump);
    FREE (else_dump);

    /* last but not least, traverse condition */
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCprepost(node *arg_node, node *arg_info)
 *
 * description:
 *   sets the rc of a N_pre/N_post-identificator to -1 (=> no refcounting)
 *
 *
 ******************************************************************************/

node *
RCprepost (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCprepost");
    PRE_REFCNT (arg_node) = -1; /* is not a refcount-object !! */
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCwith
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
RCwith (node *arg_node, node *arg_info)
{
    int *ref_dump, *with_dump, index_vec_varno, i, mod_array_varno;
    node *var_dec, *new_info, *id_node;
    long *used_mask;

    DBUG_ENTER ("RCwith");

    /* store refcounts */
    ref_dump = StoreAndInit (0);

    WITH_OPERATOR (arg_node) = Trav (WITH_OPERATOR (arg_node), arg_info);
    WITH_GEN (arg_node) = Trav (WITH_GEN (arg_node), arg_info);
    index_vec_varno = WITH_GEN (arg_node)->VAR_DEC->varno;
    if (N_modarray == WITH_OPERATOR (arg_node)->nodetype) {
        DBUG_ASSERT (N_id == MODARRAY_ARRAY (WITH_OPERATOR (arg_node))->nodetype,
                     "wrong nodetype != N_id");
        mod_array_varno = MODARRAY_ARRAY (WITH_OPERATOR (arg_node))->VAR_DEC->varno;
    } else
        mod_array_varno = -1;

    with_dump = Store ();

    used_mask = arg_node->mask[1]; /* mask of used variables */
    new_info = MakeNode (N_info);
    /* store refcounts of variables that are used before they will be defined in
     * a with_loop in new_info.
     */
    for (i = 0; i < varno; i++)
        if (used_mask[i] > 0) {
            var_dec = FindVarDec (i);
            DBUG_ASSERT ((NULL != var_dec), "variable not found");
            if (MUST_REFCOUNT (var_dec->TYPES))
                if (0 < var_dec->refcnt) {
                    /* store refcount of used variables in new_info->node[0]
                     */

                    VAR_DEC_2_ID_NODE (id_node, var_dec);
                    id_node->node[0] = new_info->node[0];
                    new_info->node[0] = id_node;
                    DBUG_PRINT ("RC", ("store used variables %s:%d", id_node->ID,
                                       id_node->ID_REF));
                }
        }
    arg_node->node[2] = new_info;

    /* now increase refcount of variables that are used before they will be
     * defined in a with_loop.
     */
    for (i = 0; i < varno; i++) {
        if ((with_dump[i] > 0) && (i != index_vec_varno) && (i != mod_array_varno)) {
            var_dec = FindVarDec (i);
            DBUG_ASSERT ((NULL != var_dec), "var not found");
            if (0 < var_dec->refcnt) {
                ref_dump[i]++;
                DBUG_PRINT ("RC", ("set refcount of %s to %d:", var_dec->info.types->id,
                                   ref_dump[i]));
            }
        }
    }

    if (-1 != mod_array_varno) {
        /* now increase refcount of modified array */
        var_dec = FindVarDec (mod_array_varno);
        DBUG_ASSERT ((NULL != var_dec), "var not found");
        ref_dump[mod_array_varno]++;
        DBUG_PRINT ("RC", ("set refcount of %s to %d:", var_dec->info.types->id,
                           ref_dump[mod_array_varno]));
    }

    Restore (ref_dump);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCcon
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   : traverses first body of with-loop and then gen- modarray
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
RCcon (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCcon");
    GENARRAY_BODY (arg_node) = Trav (GENARRAY_BODY (arg_node), arg_info);
#if 0
   GENARRAY_ARRAY(arg_node)=Trav(GENARRAY_ARRAY(arg_node), arg_info);
#endif
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCgen
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   : traverses generator of with_loop
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
RCgen (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCgen");
    GEN_RIGHT (arg_node) = Trav (GEN_RIGHT (arg_node), arg_info);
    GEN_LEFT (arg_node) = Trav (GEN_LEFT (arg_node), arg_info);
    /* set refcount of index_vector */
    IDS_REFCNT (GEN_IDS (arg_node)) = VARDEC_REFCNT (IDS_VARDEC (GEN_IDS (arg_node)));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node * RCNwith(node * arg_node, node * arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
RCNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCNwith");

    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node * RCNpart(node * arg_node, node * arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
RCNpart (node *arg_node, node *arg_info)
{
    int *ref_dump;

    DBUG_ENTER ("RCNpart");

    ref_dump = StoreAndInit (0); /* store refcounts */

    NPART_CODE (arg_node) = Trav (NPART_CODE (arg_node), arg_info);
    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);
    NPART_IDX (arg_node) = Trav (NPART_IDX (arg_node), arg_info);

    Restore (ref_dump); /* restore refcounts */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node * RCNcode(node * arg_node, node * arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
RCNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCNcode");

    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node * RCNgen(node * arg_node, node * arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
RCNgen (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCNgen");

    NGEN_BOUND1 (arg_node) = Trav (NGEN_BOUND1 (arg_node), arg_info);
    NGEN_BOUND2 (arg_node) = Trav (NGEN_BOUND2 (arg_node), arg_info);
    NGEN_STEP (arg_node) = Trav (NGEN_STEP (arg_node), arg_info);
    NGEN_WIDTH (arg_node) = Trav (NGEN_WIDTH (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* RCNwithid(node* arg_node, node* arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
RCNwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCNwithid");

    IDS_REFCNT (NWITHID_VEC (arg_node))
      = VARDEC_REFCNT (IDS_VARDEC (NWITHID_VEC (arg_node)));

    DBUG_RETURN (arg_node);
}
