/*
 *
 * $Log$
 * Revision 1.64  1999/02/06 14:46:20  dkr
 * RC can handle nested WLs now
 *
 * Revision 1.63  1999/01/07 13:58:55  sbs
 * *** empty log message ***
 *
 * Revision 1.62  1998/12/21 10:50:43  sbs
 * error in RCloop:
 * when traversing the loop body "again", the masks are re-generated!
 * Therefore, the local pointers have to be updated accordingly!
 *
 * Revision 1.61  1998/06/05 19:52:58  dkr
 * fixed some bugs in RC for new with
 *
 * Revision 1.60  1998/06/04 17:00:54  cg
 * information about refcounted variables in the context of loops,
 * conditionals and the old with-loop are now stored in ids-chains
 * instead of N_exprs lists.
 *
 * Revision 1.59  1998/06/03 14:54:47  cg
 * Attribute WITH_USEDVARS renamed to WITH_USEVARS and built correctly
 *
 * Revision 1.58  1998/05/28 23:52:30  dkr
 * fixed a bug in RCNwith:
 *   NWITH_DEC_RC_IDS is now set correctly for fold
 *
 * Revision 1.57  1998/05/21 15:00:06  dkr
 * changed RCNwith
 *
 * Revision 1.56  1998/05/21 13:33:22  dkr
 * renamed NCODE_DEC_RC_IDS into NCODE_INC_RC_IDS
 *
 * Revision 1.55  1998/05/19 08:54:21  cg
 * new functions for retrieving variables from masks provided by
 * DataFlowMask.c utilized.
 *
 * Revision 1.54  1998/05/15 15:17:36  dkr
 * fixed a bug in RCNwith
 *
 * Revision 1.52  1998/05/12 15:52:07  dkr
 * removed ???_VARINFO
 *
 * Revision 1.51  1998/05/12 14:52:24  dkr
 * renamed ???_RC_IDS to ???_DEC_RC_IDS
 *
 * Revision 1.50  1998/05/11 15:15:58  dkr
 * added RCicm:
 *   no refcounting of ICM-args
 *
 * ... [eliminated]
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
#include "DupTree.h"
#include "typecheck.h" /* to use LookupType */
#include "DataFlowMask.h"
#include "traverse.h"
#include "optimize.h"
#include "generatemasks.h"
#include "internal_lib.h"
#include "free.h"

#include "refcount.h"

#define DUB_ID_NODE(a, b)                                                                \
    a = MakeId (StringCopy (ID_NAME (b)), NULL, ST_regular);                             \
    ID_VARDEC (a) = ID_VARDEC (b)

#define VARDEC_2_ID_NODE(a, b)                                                           \
    a = MakeId (StringCopy (VARDEC_NAME (b)), NULL, ST_regular);                         \
    ID_VARDEC (a) = b;                                                                   \
    ID_REFCNT (a) = VARDEC_REFCNT (b);

#define MUST_REFCOUNT(type) (IsArray (type) || IsNonUniqueHidden (type))

static int varno;         /* used to store the number of known variables in a
                           * sac-function (used for mask[])
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

/******************************************************************************
 *
 * function:
 *   node *FindVardec(int varno, node *fundef)
 *
 * description:
 *   returns the vardec of var number 'varno'
 *
 ******************************************************************************/

node *
FindVardec (int varno, node *fundef)
{
    node *tmp, *result = NULL;

    DBUG_ENTER ("FindVardec");

    if (result == NULL) {
        tmp = FUNDEF_ARGS (fundef);
        while (tmp != NULL) {
            if (ARG_VARNO (tmp) == varno) {
                result = tmp;
                break;
            } else {
                tmp = ARG_NEXT (tmp);
            }
        }
    }

    if (result == NULL) {
        tmp = BLOCK_VARDEC (FUNDEF_BODY (fundef));
        while (tmp != NULL) {
            if (VARDEC_VARNO (tmp) == varno) {
                result = tmp;
                break;
            } else {
                tmp = VARDEC_NEXT (tmp);
            }
        }
    }

    DBUG_RETURN (result);
}

/*
 *
 *  functionname  : LookupId
 *  arguments     : 1) name
 *                  2) N_exprs-node with chain of N_id-nodes
 *  description   : looks 1) up in 2)
 *                  returns pointer to N_id-node if found
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
LookupId (char *id, node *id_chain)
{
    node *tmp, *ret_node = NULL;

    DBUG_ENTER ("LookupId");

    tmp = id_chain;
    while (NULL != tmp) {
        if (0 == strcmp (id, ID_NAME (EXPRS_EXPR (tmp)))) {
            ret_node = EXPRS_EXPR (tmp);
            break;
        } else {
            tmp = EXPRS_NEXT (tmp);
        }
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * function:
 *   int *AllocDump( int *dump)
 *
 * description:
 *   Initializes an array for dumping refcounts:
 *     If 'dump' is not NULL, the old memory is disposed.
 *     After that new memory is allocated.
 *
 *   depends on the global variable "varno" !!
 *
 ******************************************************************************/

int *
AllocDump (int *dump)
{
    DBUG_ENTER (" AllocDump");

    if (dump != NULL) {
        FREE (dump);
    }
    dump = (int *)Malloc (sizeof (int) * varno);

    DBUG_RETURN (dump);
}

/******************************************************************************
 *
 * function:
 *   void InitRC( int n)
 *
 * description:
 *   The refcounts in the vardecs are set to 'n' if refcount was >= 0.
 *
 ******************************************************************************/

void
InitRC (int n)
{
    node *vardec;

    DBUG_ENTER ("InitRC");

    FOREACH_VARDEC_AND_ARG (fundef_node, vardec, if (VARDEC_OR_ARG_REFCNT (vardec) >= 0) {
        if (NODE_TYPE (vardec) == N_arg) {
            ARG_REFCNT (vardec) = n;
        } else {
            VARDEC_REFCNT (vardec) = n;
        }
    }) /* FOREACH_VARDEC_AND_ARG */

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   int *StoreRC()
 *
 * description:
 *   returns an arry of int that contains the refcounters of the variable
 *    declaration.
 *
 ******************************************************************************/

int *
StoreRC ()
{
    node *vardec;
    int k;
    int *dump = NULL;

    DBUG_ENTER ("StoreRC");

    dump = AllocDump (dump);

    vardec = FUNDEF_ARGS (fundef_node);
    while (vardec != NULL) {
        dump[ARG_VARNO (vardec)] = ARG_REFCNT (vardec);
        vardec = ARG_NEXT (vardec);
    }
    vardec = FUNDEF_VARDEC (fundef_node);
    for (k = args_no; k < varno; k++) {
        if (k == VARDEC_VARNO (vardec)) {
            /*
             * 'vardec' is the right node belonging to 'k'.
             *   -> store the refcount.
             */
            dump[k] = VARDEC_REFCNT (vardec);
            vardec = VARDEC_NEXT (vardec);
        } else {
            /*
             * vardec belonging to 'k' was eliminated while optimisation.
             *  -> store -1 at this position.
             */
            dump[k] = -1;
        }
    }

    DBUG_RETURN (dump);
}

/******************************************************************************
 *
 * function:
 *   int *StoreAndInitRC( int n)
 *
 * description:
 *   Returns an array of int that contains the refcounts stored in the vardecs.
 *   The refcounts in the vardecs are set to 'n' if refcount was >= 1.
 *
 *   depends on the global vars "fundef_node", "args_no", and "varno" !!
 *
 ******************************************************************************/

int *
StoreAndInitRC (int n)
{
    node *argvardec;
    int k;
    int *dump = NULL;

    DBUG_ENTER ("StoreAndInitRC");

    dump = AllocDump (dump);

    argvardec = FUNDEF_ARGS (fundef_node);
    while (argvardec != NULL) {
        dump[ARG_VARNO (argvardec)] = ARG_REFCNT (argvardec);
        if (ARG_REFCNT (argvardec) > 0) {
            ARG_REFCNT (argvardec) = n;
        }
        argvardec = ARG_NEXT (argvardec);
    }
    argvardec = FUNDEF_VARDEC (fundef_node);
    for (k = args_no; k < varno; k++) {
        if (k == VARDEC_VARNO (argvardec)) {
            /*
             * 'argvardec' is the right node belonging to 'k'.
             *   -> store the refcount.
             */
            dump[k] = VARDEC_REFCNT (argvardec);
            if (VARDEC_REFCNT (argvardec) > 0) {
                VARDEC_REFCNT (argvardec) = n;
            }
            argvardec = VARDEC_NEXT (argvardec);
        } else {
            /*
             * vardec belonging to 'k' was eliminated while optimization.
             *  -> store -1 at this position.
             */
            dump[k] = -1;
        }
    }

    DBUG_RETURN (dump);
}

/******************************************************************************
 *
 * function:
 *   void RestoreRC( int *dump)
 *
 * description:
 *   copies dump-entries to refcounts of variable declaration
 *
 ******************************************************************************/

void
RestoreRC (int *dump)
{
    node *vardec;

    DBUG_ENTER ("RestoreRC");

    FOREACH_VARDEC_AND_ARG (fundef_node, vardec,
                            if (NODE_TYPE (vardec) == N_arg) {
                                ARG_REFCNT (vardec) = dump[ARG_VARNO (vardec)];
                            } else {
                                VARDEC_REFCNT (vardec) = dump[VARDEC_VARNO (vardec)];
                            }) /* FOREACH_VARDEC_AND_ARG */

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *Refcount( node *arg_node)
 *
 * description:
 *   starts the refcount-traversal (sets act_tab).
 *
 ******************************************************************************/

node *
Refcount (node *arg_node)
{
    node *arg_info;

    DBUG_ENTER ("Refcount");

    /*
     * generate masks
     */
    arg_node = GenerateMasks (arg_node, NULL);

    act_tab = refcnt_tab;
    arg_info = MakeInfo ();

    if (N_modul == NODE_TYPE (arg_node)) {
        if (MODUL_FUNS (arg_node) != NULL) {
            DBUG_ASSERT ((N_fundef == NODE_TYPE (MODUL_FUNS (arg_node))), "wrong node ");
            MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
        }
    } else {
        DBUG_ASSERT ((N_fundef == NODE_TYPE (arg_node)), "wrong node");
        arg_node = Trav (arg_node, arg_info);
    }

    FREE (arg_info);

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

/******************************************************************************
 *
 * function:
 *   node *RCfundef( node *arg_node, node *arg_info)
 *
 * description:
 *   traverses body of function; sets 'varno', 'fundef_node' and 'arg_no';
 *   generates FUNDEF_DFM_BASE.
 *
 ******************************************************************************/

node *
RCfundef (node *arg_node, node *arg_info)
{
    node *args;

    DBUG_ENTER ("RCfundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        /*
         *  The args are traversed to initialize ARG_REFCNT.
         *  Refcounted objects are initialized by 0 others by -1.
         *  This information is needed by compile.c.
         */
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (NULL != FUNDEF_BODY (arg_node)) {

        FUNDEF_DFM_BASE (arg_node)
          = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        /*
         * setting some global variables to use with the 'mask'
         *  and storage of refcounts.
         */
        varno = FUNDEF_VARNO (arg_node);
        fundef_node = arg_node;
        args = FUNDEF_ARGS (arg_node);
        args_no = 0;
        while (NULL != args) {
            args_no++;
            args = ARG_NEXT (args);
        }

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (NULL != FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCblock( node *arg_node, node *arg_info)
 *
 * description:
 *   performs the reference-counting for a N_block node:
 *     - traversal of vardecs first
 *         (initializes the refcounters of the vardecs)
 *     - then traversal of intructions
 *
 ******************************************************************************/

node *
RCblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }
    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCvardec( node *arg_node, node *arg_info)
 *
 * description:
 *   initializes the refcounters of a vardec:
 *      0, if vardec of a RC-object,
 *     -1, otherwise.
 *
 ******************************************************************************/

node *
RCvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCvardec");

    if (MUST_REFCOUNT (VARDEC_TYPE (arg_node))) {
        VARDEC_REFCNT (arg_node) = 0;
    } else {
        VARDEC_REFCNT (arg_node) = -1;
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCassign( node *arg_node, node *arg_info)
 *
 * description:
 *   Performs bottom-up traversal for refcounting.
 *   When containing a with-loop, NWITH_IN/INOUT/OUT/LOCAL are infered.
 *
 ******************************************************************************/

node *
RCassign (node *arg_node, node *arg_info)
{
    node *with;
    ids *let_ids;
    DFMmask_t ids_mask;
    int i;

    DBUG_ENTER ("RCassign");

    /*
     * if the instruction contains a with-loop, infer NWITH_IN/INOUT/OUT/LOCAL
     */

    if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)
        && (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node))) == N_Nwith)) {

        /*
         * generate masks.
         *
         * we must start at the fundef node to get the right value for VARNO
         *  in GenerateMasks().
         */
        fundef_node = GenerateMasks (fundef_node, NULL);

        let_ids = LET_IDS (ASSIGN_INSTR (arg_node));
        with = LET_EXPR (ASSIGN_INSTR (arg_node));

        /*
         * INOUT (for genarray/modarray with-loops),
         * OUT (for fold with-loops):
         *
         * the only inout/out var is the let-id (N_ids).
         */

        DBUG_ASSERT ((IDS_NEXT (let_ids) == NULL), "more than one let-ids found");

        ids_mask = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef_node));
        DFMSetMaskEntrySet (ids_mask, IDS_NAME (let_ids), IDS_VARDEC (let_ids));

        if ((NWITH_TYPE (with) == WO_genarray) || (NWITH_TYPE (with) == WO_modarray)) {
            NWITH_OUT (with) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef_node));
            NWITH_INOUT (with) = ids_mask;
        } else {
            NWITH_OUT (with) = ids_mask;
            NWITH_INOUT (with) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef_node));
        }

        /*
         * LOCAL:
         *
         * All vars that are defined in the assignment (ASSIGN_MASK(0)), except
         *  for the let-var (ids_mask), are local.
         */

        NWITH_LOCAL (with) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef_node));
        for (i = 0; i < varno; i++) {
            if ((ASSIGN_MASK (arg_node, 0))[i] > 0) {
                DFMSetMaskEntrySet (NWITH_LOCAL (with), NULL,
                                    FindVardec (i, fundef_node));
            }
        }
        DFMSetMaskMinus (NWITH_LOCAL (with), ids_mask);

        /*
         * IN:
         *
         * In-arguments of the with-loop are all vars that are used in the with-loop,
         *  except for local vars.
         */

        NWITH_IN (with) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef_node));
        for (i = 0; i < varno; i++) {
            if ((ASSIGN_MASK (arg_node, 1))[i] > 0) {
                DFMSetMaskEntrySet (NWITH_IN (with), NULL, FindVardec (i, fundef_node));
            }
        }
        DFMSetMaskMinus (NWITH_IN (with), NWITH_LOCAL (with));
    }

    /*
     * Bottom up traversal!!
     */
    if (NULL != ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    } else {
        /*
         * this must be the return-statement!!
         */
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

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
    node *vardec;
    long *defined_mask, *used_mask;
    int i, again = 0, use_old = 0;
    int *ref_dump;
    ids *usevars, *defvars, *new_ids;

    DBUG_ENTER ("RCloop");

    /* traverse body of loop */
    ref_dump = StoreAndInitRC (1); /* store the current ref-cnt values from
                                      the VARDECs to ref_dump and initialize
                                      all ref-cnts from the function's vardec
                                      with 1 */

    DBUG_PRINT ("RC", ("line: %d : entering body:", NODE_LINE (arg_node)));
    WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);
    DBUG_PRINT ("RC", ("line: %d : body finished:", NODE_LINE (arg_node)));

    usevars = DO_USEVARS (arg_node);
    defvars = DO_DEFVARS (arg_node);

    if ((usevars != NULL) || (defvars != NULL)) {
        use_old = 1;
    }

    defined_mask = BLOCK_MASK (WHILE_BODY (arg_node), 0); /* mask of defined variables */
    used_mask = BLOCK_MASK (WHILE_BODY (arg_node), 1);    /* mask of used variables */

    /* first compute sets v1 and v2 */
    for (i = 0; i < varno; i++)
        if ((defined_mask[i] > 0) || (used_mask[i] > 0)) {
            vardec = FindVardec (i, fundef_node);
            DBUG_ASSERT ((NULL != vardec), "variable not found");
            if (MUST_REFCOUNT (VARDEC_TYPE (vardec))) {

                /* first store used and defined variables (v1), (v2) */
                if ((defined_mask[i] > 0) && (ref_dump[i] > 0)) {
                    /* store refcount of defined variables in defvars (v2)
                     */
                    if (0 == use_old) {
                        new_ids = MakeIds (StringCopy (VARDEC_OR_ARG_NAME (vardec)), NULL,
                                           ST_regular);
                        IDS_REFCNT (new_ids) = ref_dump[i];
                        IDS_VARDEC (new_ids) = vardec;
                        IDS_NEXT (new_ids) = defvars;
                        defvars = new_ids;
                        /* insert 'new_ids' at the begining of 'defvars' */
                        DBUG_PRINT ("RC", ("store defined var (v2) %s:%d",
                                           IDS_NAME (new_ids), IDS_REFCNT (new_ids)));
                    } else {
                        new_ids = LookupIds (VARDEC_OR_ARG_NAME (vardec), defvars);
                        DBUG_ASSERT ((NULL != new_ids), "var not found");
                        IDS_REFCNT (new_ids) = ref_dump[i];
                        DBUG_PRINT ("RC", ("changed defined var (v2) %s:%d",
                                           IDS_NAME (new_ids), IDS_REFCNT (new_ids)));
                    }
                }
                if ((used_mask[i] > 0) && (0 < vardec->refcnt)) {
                    /* store refcount of used variables in usevars (v1)
                     */
                    if (0 == use_old) {
                        new_ids = MakeIds (StringCopy (VARDEC_OR_ARG_NAME (vardec)), NULL,
                                           ST_regular);
                        IDS_REFCNT (new_ids) = VARDEC_OR_ARG_REFCNT (vardec);
                        IDS_VARDEC (new_ids) = vardec;
                        IDS_NEXT (new_ids) = usevars;
                        usevars = new_ids;
                        /* insert 'new_ids' at the begining of 'usevars' */
                        DBUG_PRINT ("RC", ("store used var (v2) %s:%d",
                                           IDS_NAME (new_ids), IDS_REFCNT (new_ids)));
                    } else {
                        new_ids = LookupIds (VARDEC_OR_ARG_NAME (vardec), usevars);
                        DBUG_ASSERT ((NULL != new_ids), "var not found");
                        IDS_REFCNT (new_ids) = VARDEC_OR_ARG_REFCNT (vardec);
                        DBUG_PRINT ("RC", ("changed used var (v2) %s:%d",
                                           IDS_NAME (new_ids), IDS_REFCNT (new_ids)));
                    }
                }
            }
        }

    if (NULL != usevars) {
        new_ids = usevars;
        while ((0 == again) && (NULL != new_ids)) {
            if ((0 < VARDEC_OR_ARG_REFCNT (IDS_VARDEC (new_ids)))
                && (0 == ref_dump[VARDEC_OR_ARG_VARNO (IDS_VARDEC (new_ids))])) {
                again = 1;
            } else {
                new_ids = IDS_NEXT (new_ids);
            }
        }
        if (1 == again) {
            DBUG_PRINT ("RC", ("while loop again"));
            RestoreRC (ref_dump);
            ref_dump = StoreAndInitRC (1);
            new_ids = usevars;
            /* init all variables that are member of v1 with refcount 1 */
            while (NULL != new_ids) {
                if (NODE_TYPE (IDS_VARDEC (new_ids)) == N_arg) {
                    ARG_REFCNT (IDS_VARDEC (new_ids)) = 1;
                } else {
                    VARDEC_REFCNT (IDS_VARDEC (new_ids)) = 1;
                }
                new_ids = IDS_NEXT (new_ids);
            }

            /* refcount body of while loop again */
            DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);
            /*
             * traversing the loop body may change its masks!
             * Therefore, the local pointers have to be updated accordingly:
             */
            defined_mask
              = BLOCK_MASK (WHILE_BODY (arg_node), 0); /* mask of defined variables */
            used_mask
              = BLOCK_MASK (WHILE_BODY (arg_node), 1); /* mask of used variables */
        }
    }

    /* compute new refcounts because of 'virtual function application' */
    for (i = 0; i < varno; i++) {
        if ((defined_mask[i] > 0) || (used_mask[i] > 0)) {
            vardec = FindVardec (i, fundef_node);
            DBUG_ASSERT ((NULL != vardec), "variable not found");
            if (MUST_REFCOUNT (VARDEC_OR_ARG_TYPE (vardec))) {
                if ((used_mask[i] > 0) && (0 < VARDEC_OR_ARG_REFCNT (vardec))
                    && (1 == again)) {
                    /*
                     * update refcount of used variables (v1)
                     */
                    new_ids = LookupIds (VARDEC_OR_ARG_NAME (vardec), usevars);
                    DBUG_ASSERT ((NULL != new_ids), "var not found");

                    IDS_REFCNT (new_ids) = VARDEC_OR_ARG_REFCNT (vardec);
                    DBUG_PRINT ("RC",
                                ("(v1) %s:%d", IDS_NAME (new_ids), IDS_REFCNT (new_ids)));
                }
                /*
                 * now compute new refcounts, because of 'virtual function
                 * application'
                 */
                if ((defined_mask[i] > 0) && (ref_dump[i] > 0)) {
                    /* these will be the return-values of the virtual function */
                    if (N_do == NODE_TYPE (arg_node)) {
                        /*
                         * check whether current variable is an argument of
                         * 'virtual function'
                         */
                        if (0 == VARDEC_OR_ARG_REFCNT (vardec)) {
                            /*
                             * in this case the variable will be defined before it
                             * will be used, so we don't need it as argument of
                             * the virtual function (it can be freed earlier)
                             */
                            ref_dump[i] = 0;
                        } else {
                            ref_dump[i] = 1;
                        }
                    } else {
                        ref_dump[i] = 1;
                    }

                    DBUG_PRINT ("RC", ("set refcount of %s(%d) to: %d",
                                       VARDEC_OR_ARG_NAME (vardec), i, ref_dump[i]));
                } else {
                    if ((used_mask[i] > 0) && (0 < VARDEC_OR_ARG_REFCNT (vardec))) {
                        /* these variables are arguments of the virtual function */
                        ref_dump[i]++;
                        DBUG_PRINT ("RC", ("increased  refcount of %s(%d) to: %d",
                                           VARDEC_OR_ARG_NAME (vardec), i, ref_dump[i]));
                    }
                }
            }
        }
    }

    /* store new_info for use while compilation */
    DO_USEVARS (arg_node) = usevars;
    DO_DEFVARS (arg_node) = defvars;

    /* restore old vardec refcounts */
    RestoreRC (ref_dump);
    FREE (ref_dump);

    /* traverse termination condition */
    DO_COND (arg_node) = Trav (DO_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCprf( node *arg_node, node *arg_info)
 *
 * description:
 *   Traverse the args with INFO_RC_PRF( arg_info) pointing to the N_prf!
 *   This is done for RCO in RCid().
 *   If N_prf is F_reshape it has to be treated in exactly the same way as if
 *    we had a simple assignment, i.e., traverse with (INFO_RC_PRF( arg_info)
 *    == NULL)!
 *
 ******************************************************************************/

node *
RCprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCprf");

    if (PRF_PRF (arg_node) == F_reshape) {
        INFO_RC_PRF (arg_info) = NULL;
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
    } else {
        INFO_RC_PRF (arg_info) = arg_node;
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
    }
    INFO_RC_PRF (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCicm( node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses the args with INFO_RC_PRF( arg_info) pointing to the N_icm!
 *   This is done for RCO in RCid().
 *
 ******************************************************************************/

node *
RCicm (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCicm");

    INFO_RC_PRF (arg_info) = arg_node;
    ICM_ARGS (arg_node) = Trav (ICM_ARGS (arg_node), arg_info);
    INFO_RC_PRF (arg_info) = NULL;

    if (ICM_NEXT (arg_node) != NULL) {
        ICM_NEXT (arg_node) = Trav (ICM_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCid( node *arg_node, node *arg_info)
 *
 * description:
 *   Depending on 'INFO_RC_PRF( arg_info)' do one of the following:
 *     a) Increment refcnt at vardec and set local refcnt of the N_id node
 *        accordingly. (RC op needed)
 *     b) Set local refcnt to -1. (no RC op)
 *   If we have found the index-vector of a with-loop, we set RC of
 *    'NWITH_VEC' to 1 (-1 normally, see RCNwithid) --- to indicate, that we
 *    must build the index-vector in the compilat of the with-loop.
 *
 * remarks:
 *   - ('INFO_RC_PRF( arg_info)' != NULL) indicates that this N_id node is
 *       argument of a primitive function (exept F_reshape) or ICM.
 *     'INFO_RC_PRF' then points to this prf/icm.
 *   - 'INFO_RC_WITH( arg_info)' contains a N_exprs-chain with all parent
 *       WL-nodes.
 *
 ******************************************************************************/

node *
RCid (node *arg_node, node *arg_info)
{
    node *wl_node;

    DBUG_ENTER ("RCid");

    if (MUST_REFCOUNT (ID_TYPE (arg_node))) {
        if ((INFO_RC_PRF (arg_info) == NULL)
            || (VARDEC_REFCNT (ID_VARDEC (arg_node)) == 0) || (!opt_rco)) {

            /*
             * This N_id node either is *not* an argument of a primitive
             *  function, or it is the last usage within the body of the
             *  current function. (or refcount-optimization is turned off!)
             * In both cases the refcnt is incremented and attached to
             *  the N_id node.
             */

            DBUG_PRINT ("RC", ("RC for %s increased:", ID_NAME (arg_node)));
            VARDEC_REFCNT (ID_VARDEC (arg_node))++;
            ID_REFCNT (arg_node) = VARDEC_REFCNT (ID_VARDEC (arg_node));
        } else {

            /*
             * This N_id node is argument to the N_prf node *and*
             *  it is definitly not the last usage of it.
             * Therefore -1 is attached to the refcnt field if N_id
             * (to indicate that this operation does not need any refcnt
             * adjustments).
             */

            ID_REFCNT (arg_node) = -1;
        }
    } else {
        ID_REFCNT (arg_node) = -1; /* variable needs no refcount */
    }

    /*
     * if we have found the index-vector of a with-loop, we set RC of
     *  'NWITH_VEC' to 1.
     */
    wl_node = INFO_RC_WITH (arg_info);
    while (wl_node != NULL) {
        DBUG_ASSERT ((NODE_TYPE (wl_node) == N_exprs),
                     "N_exprs-chain of WL-nodes not found in arg_info");

        if (strcmp (IDS_NAME (NWITH_VEC (EXPRS_EXPR (wl_node))), ID_NAME (arg_node))
            == 0) {
            IDS_REFCNT (NWITH_VEC (EXPRS_EXPR (wl_node))) = 1;
        }

        wl_node = EXPRS_NEXT (wl_node);
    }

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
        if (MUST_REFCOUNT (IDS_TYPE (ids))) {
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
    node *vardec;
    ids *thenvars, *elsevars, *new_ids;
    int *rest_dump, *then_dump, *else_dump, i, use_old = 0;

    DBUG_ENTER ("RCcond");

    /* store current vardec refcounts in rest_dump */
    rest_dump = StoreRC ();
    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    /* store vardec refcounts after refcounting then part */
    then_dump = StoreRC ();

    /* get same refcounts as before refcounting else part */
    RestoreRC (rest_dump);

    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    /* store vardec refcounts after refcounting else part */
    else_dump = StoreRC ();

    /* now compute maximum of then- and else-dump and store it
     *  in vardec refcnt.
     * store differences between then- and else dump in COND_THEN,
     *  COND_ELSE respectively:
     *  - COND_THEN if (then_dump[i] < else_dump[i]):
     *                              (else_dump[i] - then_dump[i])
     *  - COND_ELSE if (else_dump[i] < then_dump[i]):
     *                              (then_dump[i] - else_dump[i])
     */

    thenvars = COND_THENVARS (arg_node);
    elsevars = COND_ELSEVARS (arg_node);

    if ((thenvars != NULL) || (elsevars != NULL)) {
        use_old = 1;
    }

    for (i = 0; i < varno; i++) {
        if (then_dump[i] < else_dump[i]) {
            vardec = FindVardec (i, fundef_node);
            DBUG_ASSERT ((NULL != vardec), " var not found");
            if (0 == use_old) {
                new_ids
                  = MakeIds (StringCopy (VARDEC_OR_ARG_NAME (vardec)), NULL, ST_regular);
                IDS_REFCNT (new_ids) = else_dump[i] - then_dump[i];
                IDS_VARDEC (new_ids) = vardec;
                IDS_NEXT (new_ids) = thenvars;
                thenvars = new_ids;
                /* insert 'new_ids' at the begining of 'defvars' */
                DBUG_PRINT ("RC", ("append %s :%d to then-part", IDS_NAME (new_ids),
                                   IDS_REFCNT (new_ids)));
            } else {
                new_ids = LookupIds (VARDEC_OR_ARG_NAME (vardec), thenvars);
                DBUG_ASSERT ((NULL != new_ids), "var not found");
                IDS_REFCNT (new_ids) = else_dump[i] - then_dump[i];
                DBUG_PRINT ("RC", ("changed %s :%d in then-part", IDS_NAME (new_ids),
                                   IDS_REFCNT (new_ids)));
            }
            if (NODE_TYPE (vardec) == N_arg) {
                ARG_REFCNT (vardec) = else_dump[i];
            } else {
                VARDEC_REFCNT (vardec) = else_dump[i];
            }
            DBUG_PRINT ("RC", ("set refcount of %s to %d", IDS_NAME (new_ids),
                               VARDEC_OR_ARG_REFCNT (vardec)));
        } else {
            if (else_dump[i] < then_dump[i]) {
                vardec = FindVardec (i, fundef_node);
                DBUG_ASSERT ((NULL != vardec), " var not found");
                if (0 == use_old) {
                    new_ids = MakeIds (StringCopy (VARDEC_OR_ARG_NAME (vardec)), NULL,
                                       ST_regular);
                    IDS_REFCNT (new_ids) = then_dump[i] - else_dump[i];
                    IDS_VARDEC (new_ids) = vardec;
                    IDS_NEXT (new_ids) = elsevars;
                    elsevars = new_ids;
                    /* insert 'new_ids' at the begining of 'elsevars' */
                    DBUG_PRINT ("RC", ("append %s :%d to else-part", IDS_NAME (new_ids),
                                       IDS_REFCNT (new_ids)));
                } else {
                    new_ids = LookupIds (VARDEC_OR_ARG_NAME (vardec), elsevars);
                    DBUG_ASSERT ((NULL != new_ids), "var not found");
                    IDS_REFCNT (new_ids) = then_dump[i] - else_dump[i];
                    DBUG_PRINT ("RC", ("changed %s :%d in else-part", IDS_NAME (new_ids),
                                       IDS_REFCNT (new_ids)));
                }
                if (NODE_TYPE (vardec) == N_arg) {
                    ARG_REFCNT (vardec) = then_dump[i];
                } else {
                    VARDEC_REFCNT (vardec) = then_dump[i];
                }
                DBUG_PRINT ("RC", ("set refcount of %s to %d", IDS_NAME (new_ids),
                                   VARDEC_OR_ARG_REFCNT (vardec)));
            }
        }
    }

    /* store refcount information for use while compilation */
    COND_THENVARS (arg_node) = thenvars;
    COND_ELSEVARS (arg_node) = elsevars;

    /* free the dumps */
    FREE (rest_dump);
    FREE (then_dump);
    FREE (else_dump);

    /* last but not least, traverse condition */
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

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
 ******************************************************************************/

node *
RCprepost (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCprepost");
    PRE_REFCNT (arg_node) = -1; /* is not a refcount-object!! */
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCwith(node *arg_node, node *arg_info)
 *
 * description:
 *   performs reference counting for with-loop.
 *
 ******************************************************************************/

node *
RCwith (node *arg_node, node *arg_info)
{
    int *ref_dump, *with_dump, index_vec_varno, i, mod_array_varno;
    node *vardec;
    long *used_mask;
    ids *new_ids;

    DBUG_ENTER ("RCwith");

    /* store refcounts */
    ref_dump = StoreAndInitRC (0);

    WITH_OPERATOR (arg_node) = Trav (WITH_OPERATOR (arg_node), arg_info);
    WITH_GEN (arg_node) = Trav (WITH_GEN (arg_node), arg_info);
    index_vec_varno = VARDEC_VARNO (GEN_VARDEC (WITH_GEN (arg_node)));
    if (N_modarray == WITH_OPERATOR (arg_node)->nodetype) {
        DBUG_ASSERT (N_id == MODARRAY_ARRAY (WITH_OPERATOR (arg_node))->nodetype,
                     "wrong nodetype != N_id");
        mod_array_varno
          = VARDEC_VARNO (ID_VARDEC (MODARRAY_ARRAY (WITH_OPERATOR (arg_node))));
    } else
        mod_array_varno = -1;

    with_dump = StoreRC ();

    /* mask of variables that are used in body */
    used_mask = WITH_MASK (arg_node, 1);

    /*
     * store refcounts of variables that are used in body
     * before they will be defined in a with_loop
     * in WITH_USEVARS(arg_node).
     */
    for (i = 0; i < varno; i++) {
        if (used_mask[i] > 0) {
            vardec = FindVardec (i, fundef_node);
            DBUG_ASSERT ((NULL != vardec), "variable not found");
            if (MUST_REFCOUNT (VARDEC_OR_ARG_TYPE (vardec))) {
                if (0 < VARDEC_OR_ARG_REFCNT (vardec)) {
                    /* store refcount of used variables in WITH_USEDVARS() */
                    new_ids = MakeIds (StringCopy (VARDEC_OR_ARG_NAME (vardec)), NULL,
                                       ST_regular);
                    IDS_VARDEC (new_ids) = vardec;
                    IDS_REFCNT (new_ids) = VARDEC_OR_ARG_REFCNT (vardec);
                    IDS_NEXT (new_ids) = WITH_USEVARS (arg_node);
                    WITH_USEVARS (arg_node) = new_ids;

                    DBUG_PRINT ("RC", ("store used variables %s:%d", IDS_NAME (new_ids),
                                       IDS_REFCNT (new_ids)));
                }
            }
        }
    }

    /*
     * now increase refcount of variables that are used before they will be
     * defined in a with_loop.
     */
    for (i = 0; i < varno; i++) {
        if ((with_dump[i] > 0) && (i != index_vec_varno) && (i != mod_array_varno)) {
            vardec = FindVardec (i, fundef_node);
            DBUG_ASSERT ((NULL != vardec), "var not found");
            if (0 < VARDEC_OR_ARG_REFCNT (vardec)) {
                ref_dump[i]++;
                DBUG_PRINT ("RC", ("set refcount of %s to %d:",
                                   VARDEC_OR_ARG_NAME (vardec), ref_dump[i]));
            }
        }
    }

    if (-1 != mod_array_varno) {
        /* now increase refcount of modified array */
        vardec = FindVardec (mod_array_varno, fundef_node);
        DBUG_ASSERT ((NULL != vardec), "var not found");
        ref_dump[mod_array_varno]++;
        DBUG_PRINT ("RC", ("set refcount of %s to %d:", VARDEC_OR_ARG_NAME (vardec),
                           ref_dump[mod_array_varno]));
    }

    RestoreRC (ref_dump);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCcon
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   : traverses first body of with-loop and then gen- modarray
 *
 */

node *
RCcon (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCcon");
    GENARRAY_BODY (arg_node) = Trav (GENARRAY_BODY (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCgen
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   : traverses generator of with_loop
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
 *   node *RCNwith( node *arg_node, node *arg_info)
 *
 * description:
 *   performs the refcounting for a N_Nwith node.
 *   collects in NWITH_DEC_RC_IDS all ids that are RC-arguments of the
 *    with-loop.
 *
 * remarks:
 *   - 'INFO_RC_WITH( arg_info)' contains a N_exprs-chain with all parent
 *     WL-nodes.
 *   - In 'INFO_RC_RCDUMP( arg_info)' we store the initial refcounters for
 *     RCNcode:
 *       rc(var) = 1,  if 'var' is element of 'NWITH_IN'      \ this is done,
 *               = 1,  if 'var' is index-vector of with-loop  / to fool the RCO
 *               = 0,  otherwise
 *   - The RC of 'NWITH_VEC' indicates whether we need to build the
 *     index-vector or not:
 *        1: we have found references to the index-vector in at least one
 *           code-block
 *       -1: we do not need to build the index-vector.
 *     This flag/RC is correctly set after traversal of the code-blocks.
 *
 ******************************************************************************/

node *
RCNwith (node *arg_node, node *arg_info)
{
    node *vardec, *neutral_vardec;
    ids *new_ids, *last_ids;
    int *ref_dump, *tmp_rcdump;

    DBUG_ENTER ("RCNwith");

    /*
     * insert current WL into 'INFO_RC_WITH( arg_info)' (at head of chain).
     */
    INFO_RC_WITH (arg_info) = MakeExprs (arg_node, INFO_RC_WITH (arg_info));

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    /*
     * store current refcounts, initialize them with 0
     */
    ref_dump = StoreAndInitRC (0);

    /*
     * now we set up 'INFO_RC_RCDUMP( arg_info)' (needed in RCNcode)
     */
    vardec = DFMGetMaskEntryDeclSet (NWITH_IN (arg_node));
    while (vardec != NULL) {
        if (MUST_REFCOUNT (VARDEC_OR_ARG_TYPE (vardec))) {
            if (NODE_TYPE (vardec) == N_arg) {
                ARG_REFCNT (vardec) = 1;
            } else {
                VARDEC_REFCNT (vardec) = 1;
            }
        }
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    vardec = IDS_VARDEC (NWITH_VEC (arg_node));
    if (MUST_REFCOUNT (VARDEC_OR_ARG_TYPE (vardec))) {
        if (NODE_TYPE (vardec) == N_arg) {
            ARG_REFCNT (vardec) = 1;
        } else {
            VARDEC_REFCNT (vardec) = 1;
        }
    }

    tmp_rcdump = INFO_RC_RCDUMP (arg_info);
    INFO_RC_RCDUMP (arg_info) = StoreAndInitRC (0);

    /*************************************************************
     * count references in with-loop
     */

    /*
     * CAUTION: We must traverse the (parts -> withids) before we traverse the code,
     *            to get the right RC in 'NWITH_VEC'!
     *          'RCNwithid()' initializes the RC, and while traversal of the code
     *            it is set correctly!
     */
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    /*
     * count references in with-loop
     *************************************************************/

    /*
     * restore refcounts
     */
    RestoreRC (ref_dump);

    /*
     * Increase refcount of each RC-variable that is IN-var of the with-loop.
     *
     * We collect all these ids in 'NWITH_DEC_RC_IDS( arg_node)'.
     * 'compile' generates for each var found in 'NWITH_DEC_RC_IDS' a
     *  'ND_DEC_RC'-ICM in the epilog-code of the with-loop!
     *
     * RCO: with-loops are handled like prfs:
     *      when RCO is active, we only count the last occur of IN-vars.
     *
     * CAUTION: we must initialize NCODE_INC_RC_IDS because some subtrees
     *          (e.g. bodies of while-loops) are traversed twice!!!
     *
     * There is one *exception*:
     * The neutral element of a fold-with-loop is neither RC-counted here,
     *  nor inserted into 'NWITH_DEC_RC_IDS'.
     * This is done, because 'RCNwithop' has counted it already, and the
     *  with-loop do not consume it
     * (compile generates a 'ASSIGN_ARRAY( neutral, wl_id)' instead!!).
     */

    /*
     * get vardec of neutral element
     */
    switch (NWITH_TYPE (arg_node)) {
    case WO_foldfun:
        /* here is no break missing!! */
    case WO_foldprf:
        if (NODE_TYPE (NWITHOP_NEUTRAL (NWITH_WITHOP (arg_node))) == N_id) {
            neutral_vardec = ID_VARDEC (NWITHOP_NEUTRAL (NWITH_WITHOP (arg_node)));
        }
        break;
    default:
        neutral_vardec = NULL;
    }

    if (NWITH_DEC_RC_IDS (arg_node) != NULL) {
        NWITH_DEC_RC_IDS (arg_node) = FreeAllIds (NWITH_DEC_RC_IDS (arg_node));
    }
    vardec = DFMGetMaskEntryDeclSet (NWITH_IN (arg_node));
    while (vardec != NULL) {
        if ((MUST_REFCOUNT (VARDEC_OR_ARG_TYPE (vardec))) && (vardec != neutral_vardec)
            && ((VARDEC_OR_ARG_REFCNT (vardec) == 0) || (!opt_rco))) {
            /*
             * increment RC of non-withop-params
             */
            if (NODE_TYPE (vardec) == N_arg) {
                ARG_REFCNT (vardec)++;
            } else {
                VARDEC_REFCNT (vardec)++;
            }

            new_ids
              = MakeIds (StringCopy (VARDEC_OR_ARG_NAME (vardec)), NULL, ST_regular);
            IDS_VARDEC (new_ids) = vardec;
            IDS_REFCNT (new_ids) = VARDEC_REFCNT (vardec);
            if (NWITH_DEC_RC_IDS (arg_node) == NULL) {
                NWITH_DEC_RC_IDS (arg_node) = new_ids;
            } else {
                IDS_NEXT (last_ids) = new_ids;
            }
            last_ids = new_ids;
        }
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    /*
     * we leave the with-loop
     *   -> remove current WL from 'INFO_RC_WITH( arg_info)' (head of chain!).
     *   -> reset 'INFO_RC_RCDUMP( arg_info)'.
     */
    EXPRS_EXPR (INFO_RC_WITH (arg_info)) = NULL;
    INFO_RC_WITH (arg_info) = FreeNode (INFO_RC_WITH (arg_info));
    INFO_RC_RCDUMP (arg_info) = tmp_rcdump;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCNpart( node *arg_node, node *arg_info)
 *
 * description:
 *   performs the refcounting for a N_Npart node.
 *
 ******************************************************************************/

node *
RCNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCNpart");

    /*
     * we do not need any RCs at the withids.
     *  -> mark them as non-RC-objects.
     */
    NPART_WITHID (arg_node) = Trav (NPART_WITHID (arg_node), arg_info);

    /*
     * we count the references in the generator.
     */
    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

    /*
     * traverse next part-node
     */
    if (NPART_NEXT (arg_node) != NULL) {
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCNcode( node *arg_node, node *arg_info)
 *
 * description:
 *   performs reference counting for N_Ncode nodes.
 *
 * remarks:
 *   - In 'INFO_RC_RCDUMP( arg_info)' are the initial refcounters stored:
 *       rc(var) = 1,  if 'var' is element of 'NWITH_IN'      \ this is done,
 *                 1,  if 'var' is index-vector of with-loop  / to fool the RCO
 *                 0,  otherwise
 *   - The RC of 'NWITH_VEC' indicates whether we need to build the
 *     index-vector or not:
 *        1: we have found references to the index-vector in at least one
 *           code-block
 *       -1: we do not need to build the index-vector.
 *     This flag/RC is correctly set after traversal of the code-blocks.
 *
 ******************************************************************************/

node *
RCNcode (node *arg_node, node *arg_info)
{
    node *vardec;
    ids *new_ids, *last_ids;

    DBUG_ENTER ("RCNcode");

    /*
     * initialize refcounters
     */
    RestoreRC (INFO_RC_RCDUMP (arg_info));

    /*
     * count the references in the code
     */
    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    /*
     * We collect all ids, which RC is >1, in 'NCODE_INC_RC_IDS( arg_node)'.
     * In 'IDS_REFCNT' we store (RC - 1) --- because we started the refcounting
     *  of these vars with (RC == 1)!!!
     * 'compile' generates for each var in 'NCODE_INC_RC_IDS' a 'ND_INC_RC'-ICM
     *  as first statement of the code-block!
     *
     * CAUTION: we must initialize NCODE_INC_RC_IDS because some subtrees
     *          (e.g. bodies of while-loops) are traversed two times!!!
     */

    if (NCODE_INC_RC_IDS (arg_node) != NULL) {
        NCODE_INC_RC_IDS (arg_node) = FreeAllIds (NCODE_INC_RC_IDS (arg_node));
    }
    FOREACH_VARDEC_AND_ARG (fundef_node, vardec, if (VARDEC_OR_ARG_REFCNT (vardec) > 1) {
        new_ids = MakeIds (StringCopy (VARDEC_OR_ARG_NAME (vardec)), NULL, ST_regular);
        IDS_VARDEC (new_ids) = vardec;
        IDS_REFCNT (new_ids) = VARDEC_REFCNT (vardec) - 1;

        if (NCODE_INC_RC_IDS (arg_node) == NULL) {
            NCODE_INC_RC_IDS (arg_node) = new_ids;
        } else {
            IDS_NEXT (last_ids) = new_ids;
        }
        last_ids = new_ids;
    }) /* FOREACH_VARDEC_AND_ARG */

    /*
     * count the references in next code
     */

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCNgen( node *arg_node, node *arg_info)
 *
 * description:
 *   performs the refcounting for a N_Ngen node.
 *
 ******************************************************************************/

node *
RCNgen (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCNgen");

    NGEN_BOUND1 (arg_node) = Trav (NGEN_BOUND1 (arg_node), arg_info);
    NGEN_BOUND2 (arg_node) = Trav (NGEN_BOUND2 (arg_node), arg_info);
    if (NGEN_STEP (arg_node) != NULL) {
        NGEN_STEP (arg_node) = Trav (NGEN_STEP (arg_node), arg_info);
    }
    if (NGEN_WIDTH (arg_node) != NULL) {
        NGEN_WIDTH (arg_node) = Trav (NGEN_WIDTH (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCNwithid( node *arg_node, node *arg_info)
 *
 * description:
 *   marks the index-vectors as non-RC-objects.
 *
 ******************************************************************************/

node *
RCNwithid (node *arg_node, node *arg_info)
{
    ids *my_ids;

    DBUG_ENTER ("RCNwithid");

    /*
     * we initialize the RC of the index-vector with -1.
     *  -> by default we do not need to build it.
     * (if a reference of the index-vector is found while traversal of the codes,
     *  the RC of the withid in the first part (= 'NWITH_VEC') is set to 1.)
     */

    IDS_REFCNT (NWITHID_VEC (arg_node)) = -1;

    /*
     * index-vector-components are scalar values.
     *  -> mark them as non-RC-objects.
     */

    my_ids = NWITHID_IDS (arg_node);
    while (my_ids != NULL) {
        IDS_REFCNT (my_ids) = -1;
        my_ids = IDS_NEXT (my_ids);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCNwithop( node *arg_node, node *arg_info)
 *
 * description:
 *   Performs the refcounting for a N_Nwithop node.
 *
 ******************************************************************************/

node *
RCNwithop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCNwithop");

    switch (NWITHOP_TYPE (arg_node)) {

    case WO_genarray:
        /*
         * We count this reference via 'NWITH_IN' ('RCNwith()').
         *  -> set RC to -1.
         */
        if (NODE_TYPE (NWITHOP_SHAPE (arg_node)) == N_id) {
            ID_REFCNT (NWITHOP_SHAPE (arg_node)) = -1;
        }
        break;

    case WO_modarray:
        /*
         * We count this reference via 'NWITH_IN' ('RCNwith()').
         *  -> set RC to -1.
         */
        DBUG_ASSERT ((NODE_TYPE (NWITHOP_ARRAY (arg_node)) == N_id), "no id found");
        ID_REFCNT (NWITHOP_ARRAY (arg_node)) = -1;
        break;

    case WO_foldfun:
        /* here is no break missing! */
    case WO_foldprf:
        /*
         * 'compile' needs an annotated RC at this node,
         *  therefore this reference is counted here, *not* via 'NWITH_IN'.
         */
        if (NWITHOP_NEUTRAL (arg_node) != NULL) {
            NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        }
        break;

    default:
        DBUG_ASSERT ((0), "wrong withop type found");
    }

    DBUG_RETURN (arg_node);
}
