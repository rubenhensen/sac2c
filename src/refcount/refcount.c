/*
 *
 * $Log$
 * Revision 1.10  1995/05/03 07:46:17  hw
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
#include "refcount.h"

#define TYPES info.types
#define ID info.ids->id
#define VAR_DEC info.ids->node
#define CONTEXT info.cint
#define ID_REF refcnt
#define DUB_ID_NODE(a, b)                                                                \
    a = MakeNode (N_id);                                                                 \
    a->info.ids = MakeIds (StringCopy (b->info.ids->id));                                \
    a->VAR_DEC = b->VAR_DEC
#define VAR_DEC_2_ID_NODE(a, b)                                                          \
    a = MakeNode (N_id);                                                                 \
    a->info.ids = MakeIds (StringCopy (b->info.types->id));                              \
    a->VAR_DEC = b;                                                                      \
    a->ID_REF = b->refcnt;

#define FREE(a) free (a)

static int varno;         /* used to store the number of known variables in a
                             sac-function (used for mask[])
                           */
static int args_no;       /* number of arguments of current function */
static node *fundef_node; /* pointer to current function declaration */

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
    node *type_node;
    int ret = 0;

    DBUG_ENTER ("IsArray");
    DBUG_PRINT ("RC", ("looking for %s", type->id));

    if ((1 <= type->dim) || (-1 == type->dim))
        ret = 1;
    else if (T_user == type->simpletype) {
        type_node = LookupType (type->name, type->name_mod, 042);
        /* 042 is only a dummy argument */
        if ((1 <= type_node->info.types->dim) || (-1 == type_node->info.types->dim))
            ret = 1;
    }
    DBUG_PRINT ("RC", ("%d", ret));

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
    node *info_node;

    DBUG_ENTER ("Refcount");

    act_tab = refcnt_tab;

    info_node = MakeNode (N_info);
    if (N_modul == arg_node->nodetype) {
        DBUG_ASSERT ((N_fundef == arg_node->node[2]->nodetype), "wrong node ");
        arg_node->node[2] = Trav (arg_node->node[2], info_node);
    } else {
        DBUG_ASSERT ((N_fundef == arg_node->nodetype), "wrong node ");
        Trav (arg_node, info_node);
    }
    FREE (info_node);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCfundef
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   : calls Trav to traverse body of function
 *                  sets varno ,fundef_node and arg_no
 *  global vars   :varno, fundef_node, args_no
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

    if (NULL != arg_node->node[0]) {
        /* setting some global variables to use with the 'mask'
         * and storage of refcounts
         */
        varno = arg_node->varno;
        fundef_node = arg_node;
        args = arg_node->node[2];
        args_no = 0;
        while (NULL != args) {
            args_no += 1;
            args = args->node[0];
        }

        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    }
    if (NULL != arg_node->node[1])
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCassign
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   : traverses next N_assign node if possible
 *                  traverses statement where 1) points to afterwards
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

    /* goto last assign-node and start refcounting there */
    if (1 == arg_node->nnode)
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    else {
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
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
 *  remarks       : - all vars that are not used in the rest of
 *
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
    ref_dump = StoreAndInit (1);

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
            if (1 == IsArray (var_dec->TYPES)) {

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
            if (1 == IsArray (var_dec->TYPES)) {
#if 0
            if((defined_mask[i]>0) &&( ref_dump[i] >0))
            {
               /* store refcount of defined variables in new_info->node[1] (v2)
                */
               VAR_DEC_2_ID_NODE(id_node, var_dec);
               id_node->ID_REF=ref_dump[i];
               id_node->node[0]=new_info->node[1];
               new_info->node[1]=id_node;
               DBUG_PRINT("RC",("(v2) %s:%d",
                                id_node->ID, id_node->ID_REF));
            }
#endif
                if ((used_mask[i] > 0) && (0 < var_dec->refcnt) && (1 == again)) {
                    /* update refcount of used variables  (v1)
                     */
                    id_node = LookupId (var_dec->info.types->id, new_info->node[0]);
                    DBUG_ASSERT ((NULL != id_node), "var not found");

                    id_node->ID_REF = var_dec->refcnt;
                    DBUG_PRINT ("RC", ("(v1) %s:%d", id_node->ID, id_node->ID_REF));
                }
                /* now compute new refcounts, because of 'virtuell function
                 * application'
                 */
                if (defined_mask[i] > 0) {
                    if (N_do == arg_node->nodetype) {
                        /* check whether curren variable is an argument of
                         * 'virtuell function'
                         */
                        if (0 == var_dec->refcnt)
                            ref_dump[i] = 0;
                        else
                            ref_dump[i] = 1;
                    } else
                        ref_dump[i] = 1;
                } else
                    ref_dump[i] += 1;
                DBUG_PRINT ("RC", ("set refcount of %s(%d) to: %d",
                                   var_dec->info.types->id, i, ref_dump[i]));
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
 *  functionname  : RCid
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :arg_node->info.ids->node: pointer to VarDec
 *
 */
node *
RCid (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("RCid");

    if (1 == IsArray (arg_node->VAR_DEC->TYPES)) {
        arg_node->VAR_DEC->refcnt += 1;
        arg_node->ID_REF = arg_node->VAR_DEC->refcnt;
    } else
        arg_node->ID_REF = -1; /* variable isn`t an array */

    DBUG_PRINT ("RC", ("set refcnt of %s to %d:", arg_node->ID, arg_node->ID_REF));

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RClet
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
RClet (node *arg_node, node *arg_info)
{
    ids *ids;

    DBUG_ENTER ("RClet");
    DBUG_PRINT ("RC", ("line: %d", arg_node->lineno));

    ids = arg_node->info.ids;
    while (NULL != ids) {
        if (1 == IsArray (ids->node->TYPES)) {
            ids->refcnt = ids->node->refcnt;
            ids->node->refcnt = 0;
        } else
            ids->refcnt = -1;
        ids = ids->next;
    }
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);

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
    int *ref_dump, *with_dump, index_vec_varno, i;
    node *var_dec, *new_info, *id_node;
    long *used_mask;

    DBUG_ENTER ("RCwith");

    /* store refcounts */
    ref_dump = StoreAndInit (0);

    arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    index_vec_varno = arg_node->node[0]->VAR_DEC->varno;

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
            if (1 == IsArray (var_dec->TYPES))
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

    for (i = 0; i < varno; i++) {
        if ((with_dump[i] > 0) && (i != index_vec_varno)) {
            var_dec = FindVarDec (i);
            DBUG_ASSERT ((NULL != var_dec), "var not found");
            if (0 < var_dec->refcnt) {
                ref_dump[i] = +1;
                DBUG_PRINT ("RC", ("set refcount of %s to %d:", var_dec->info.types->id,
                                   ref_dump[i]));
            }
        }
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
    arg_node->node[1] = Trav (arg_node->node[1], arg_info);
#if 0
   arg_node->node[0]=Trav(arg_node->node[0], arg_info);
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
    arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    /* set refcount of index_vector */
    arg_node->info.ids->refcnt = arg_node->VAR_DEC->refcnt;

    DBUG_RETURN (arg_node);
}
