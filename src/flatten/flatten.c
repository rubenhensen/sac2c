/*
 *
 * $Log$
 * Revision 1.20  1995/04/25 09:08:02  hw
 * index vector of with-loop will be renamed if necessary
 *
 * Revision 1.19  1995/04/21  12:44:18  hw
 * -added  FltnId, FltnLet & FltnArgs
 * -- removed FltnPrf
 * - now varibales in with_loops will be renamed and initialized
 *   if necessary
 *
 * Revision 1.18  1995/04/18  09:50:04  hw
 * bug fixed in FltnExprs (constant arrays will not be abstracted out of
 *  argumentposition of primitive functions)
 *
 * Revision 1.17  1995/04/07  15:33:34  hw
 * FltnExprs will now flatten its arguments depending on the context
 * (modified FltExprs, FltnWhile, FltnDo)
 * N_ap will be "abstracted" out of the termination condition of a loop
 *  (N_prf won't)
 *
 * Revision 1.16  1995/04/07  13:37:42  hw
 * FltnAp, FltnReturn inserted
 * modified FtnExprs to flatten N_exprs depending on the context
 *
 * Revision 1.15  1995/03/13  17:03:44  hw
 * changover from 'info.id' to 'info.ids' of node N_id,
 * N_post, N_pre done
 *
 * Revision 1.14  1995/03/08  17:01:41  hw
 * changed FltnExprs (if an N_array node is a child of N_exprs node
 *                    then N_array will be flattened too)
 *
 * Revision 1.13  1995/03/07  11:00:03  hw
 * added function FltnGen (flatten N_generator)
 *
 * Revision 1.12  1995/01/12  14:04:53  hw
 * initialized node of structure 'ids' with NULL
 *
 * Revision 1.11  1995/01/06  16:45:15  hw
 * added FltnFundef
 *
 * Revision 1.10  1994/12/15  11:47:06  hw
 * inserted FltnModul
 *
 * Revision 1.9  1994/12/12  16:03:59  asi
 * Error fixed in FltnDo
 *
 * Revision 1.8  1994/11/22  17:27:48  hw
 * added function DuplicateNode
 * call DuplicateNode in function FltnWhile to have only one referenze to
 * each node
 *
 * Revision 1.7  1994/11/22  16:40:08  hw
 * added FltnDo
 *
 * Revision 1.6  1994/11/18  13:13:10  hw
 * changed FltnWhile
 * now the flattened stop condition of the while loop is inserted infront of
 * the while statement and also at the end of the loop body
 *
 * Revision 1.5  1994/11/17  16:51:58  hw
 * added FltnWhile & FltnWith
 *
 * Revision 1.4  1994/11/15  14:49:29  hw
 * deleted FltFor
 * bug fixed in FltnPrf
 *
 * Revision 1.3  1994/11/14  17:49:23  hw
 * added FltnCond FltnFor
 * last one doesn`t work correkt at all
 *
 * Revision 1.2  1994/11/10  15:39:42  sbs
 * RCS-header inserted
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "internal_lib.h"
#include "access_macros.h"

#define VAR "__tmp"       /* name of new variable */
#define VAR_LENGTH 10     /* dimension for array of char */
#define P_FORMAT "(%06x)" /* formatstring for pointer address */
#define WITH_PREFIX "__w" /* name of new variable in with-statement */
#define WITH_PREFIX_LENGTH 3
#define FREE(a) free (a)

/* macros are used as tag in  arg_info->info.cint for flatten of N_exprs
 */
#define NORMAL 0
#define AP 1
#define RET 2
#define LOOP 3

#define STACK_SIZE 1000
#define PUSH(old, new, n)                                                                \
    if (tos < stack_limit) {                                                             \
        tos->id_old = old;                                                               \
        tos->id_new = new;                                                               \
        tos->w_level = n;                                                                \
        tos++;                                                                           \
    } else                                                                               \
        ERROR2 (1, (" stack overflow (local)"))

extern node *MakeNode (nodetype); /* defined in sac.y or y.tab.c respectively */

typedef struct LOCAL_STACK {
    char *id_old;
    char *id_new;
    int w_level;
} local_stack;

static int var_counter = 0;
static int with_level = 0;
static local_stack *tos, *stack, *stack_limit;

/*
 *
 *  functionname  : DuplicateNode
 *  arguments     :  1) source node
 *  description   : returns a duplicate of arg1
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeNode
 *  macros        : ---
 *
 *  remarks       :
 *
 */
node *
DuplicateNode (node *source_node)
{
    int i;
    node *dest_node;

    DBUG_ENTER ("DuplicateNode");

    DBUG_PRINT ("DUPLICATE",
                ("%s" P_FORMAT " number of nodes: %d",
                 mdb_nodetype[source_node->nodetype], source_node, source_node->nnode));

    dest_node = MakeNode (source_node->nodetype);
    for (i = 0; i < source_node->nnode; i++)
        dest_node->node[i] = DuplicateNode (source_node->node[i]);

    dest_node->nnode = i;
    dest_node->info = source_node->info;
    dest_node->lineno = source_node->lineno;
    DBUG_PRINT ("DUPLICATE",
                ("return :%s" P_FORMAT, mdb_nodetype[dest_node->nodetype], dest_node));

    DBUG_RETURN (dest_node);
}

/*
 *
 *  functionname  : GenTmpVar
 *  arguments     : 1) counter number
 *  description   : allocate string for temporary variable
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

char *
GenTmpVar (int count)
{
    char *string;

    DBUG_ENTER ("GenTmpVar");

    string = (char *)malloc (sizeof (char) * VAR_LENGTH);
    sprintf (string, VAR "%d", count);

    DBUG_PRINT ("TMP", ("new variable: %s", string));

    DBUG_RETURN (string);
}

/*
 *
 *  functionname  : RenameWithVar
 *  arguments     : 1) old name
 *                  2) number of nested with_loops
 *  description   : allocate string for temporary variable
 *  global vars   :
 *  internal funs : ---
 *  external funs : Malloc, sizeof, strlen
 *  macros        : ---
 *
 *  remarks       :
 *
 */

char *
RenameWithVar (char *name, int level)
{
    char *string;
    DBUG_ENTER ("RenameWithVar");

    string = (char *)Malloc (sizeof (char) * (strlen (name) + WITH_PREFIX_LENGTH + 2));
    /* 2 will be added, because 'with_level' will be part of new name */

    sprintf (string, WITH_PREFIX "%d_%s", level, name);

    DBUG_RETURN (string);
}

/*
 *
 *  functionname  : InsertIdentity
 *  arguments     : 1) last assign
 *                  2) assigned variable name
 *                  3) assigned to variable name
 *  description   :
 *  global vars   :
 *  internal funs : ---
 *  external funs :
 *  macros        : ---
 *
 *  remarks       :
 *
 */
node *
AppendIdentity (node *last_assign, char *old_name, char *new_name)
{
    node *assign_node, *let_node;
    char *name;

    DBUG_ENTER ("AppendIdentity");

    assign_node = MakeNode (N_assign);
    let_node = MakeNode (N_let);
    name = (char *)Malloc (sizeof (char) * (strlen (new_name) + 1));
    name = strcpy (name, new_name);
    let_node->IDS = MakeIds (name);
    let_node->node[0] = MakeNode (N_id);
    name = (char *)Malloc (sizeof (char) * (strlen (old_name) + 1));
    name = strcpy (name, old_name);
    let_node->node[0]->IDS = MakeIds (name);
    let_node->nnode = 1;
    assign_node->node[0] = let_node;
    if (NULL != last_assign) {
        assign_node->node[1] = last_assign;
        assign_node->nnode = 2;
    } else
        assign_node->nnode = 1;

    DBUG_RETURN (assign_node);
}

/*
 *
 *  functionname  : FindId
 *  arguments     : 1) name
 *  description   : looks for 10 on local stack, returns pointer to stack entry
 *                  if found, NULL otherwise
 *  global vars   :
 *  internal funs : ---
 *  external funs :
 *  macros        : ---
 *
 *  remarks       :
 *
 */
local_stack *
FindId (char *name)
{
    local_stack *tmp;
    int found = 0;

    DBUG_ENTER ("FindId");
    if (tos == stack)
        tmp = NULL;
    else {
        tmp = tos - 1;

        while ((tmp > stack) && (0 == found))
            if (0 == strcmp (tmp->id_old, name))
                found = 1;
            else
                tmp--;
        if (0 == found)
            if (0 != strcmp (tmp->id_old, name))
                tmp = NULL;
    }

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : Flatten
 *  arguments     : 1) syntax tree
 *  description   : eliminates nested function applications
 *                  allocates memory for local stack and sets tos
 *  global vars   : syntax_tree, act_tab, flat_tab, with_level
 *  internal funs : ---
 *  external funs : Trav, MakeNode
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
Flatten (node *arg_node)
{
    node *info_node;

    DBUG_ENTER ("Flatten");
    stack = (local_stack *)Malloc (sizeof (local_stack) * STACK_SIZE);
    stack_limit = STACK_SIZE + stack;
    tos = stack;

    var_counter = 0;
    act_tab = flat_tab;
    info_node = MakeNode (N_info);
    info_node->info.cint = NORMAL;
    info_node->nnode = 1;
    info_node->node[0] = NULL;
    with_level = 0;
    arg_node = Trav (arg_node, info_node);
    FREE (info_node);
    FREE (stack);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnAssign
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   : Insert reference to actual assign node into info node
 *                  call recursively and return adress from the info node
 *                  obtained back.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
FltnAssign (node *arg_node, node *arg_info)
{
    node *return_node;

    DBUG_ENTER ("FltnAssign");

    arg_info->node[0] = arg_node;
    DBUG_PRINT ("FLATTEN", ("arg_info->node[0] set to %08x!", arg_node));
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    return_node = arg_info->node[0];
    DBUG_PRINT ("FLATTEN", ("node %08x inserted before %08x", return_node, arg_node));
    if (arg_node->nnode == 2) /* there are more assigns that follow */
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    DBUG_RETURN (return_node);
}

/*
 *
 *  functionname  : FltnExprs
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   : Flatten each argument of the given node if neccessary
 *  global vars   : var_counter
 *  internal funs : GenTmpVar
 *  external funs : MakeNode
 *  macros        : GEN_NODE
 *
 *  remarks       : arg_info->info.cint contains a tag that gives the
 *                  context of this N_exprs
 *
 */
node *
FltnExprs (node *arg_node, node *arg_info)
{
    node *tmp_node1, *id_node, *let_node, *assign_node;
    int abstract, old_tag;

    DBUG_ENTER ("FltnExprs");

    /* compute whether to abstract an expression or not , depending on the
     * context of the expression ,given by arg_info->info.cint
     */
    switch (arg_info->info.cint) {
    case LOOP:
        abstract = (arg_node->node[0]->nodetype == N_ap);
        break;
    case RET:
        abstract = ((arg_node->node[0]->nodetype == N_num)
                    || (arg_node->node[0]->nodetype == N_float)
                    || (arg_node->node[0]->nodetype == N_bool)
                    || (arg_node->node[0]->nodetype == N_str)
                    || (arg_node->node[0]->nodetype == N_array)
                    || (arg_node->node[0]->nodetype == N_ap)
                    || (arg_node->node[0]->nodetype == N_prf));
        break;
    case AP:
        abstract = ((arg_node->node[0]->nodetype == N_array)
                    || (arg_node->node[0]->nodetype == N_prf)
                    || (arg_node->node[0]->nodetype == N_ap));
        break;
    case NORMAL:
        abstract = ((arg_node->node[0]->nodetype == N_ap)
                    || (arg_node->node[0]->nodetype == N_prf));
        break;
    default:
        DBUG_ASSERT (0, "wrong tag ");
    }

    DBUG_PRINT ("FLATTEN", ("tag: %d, abstract: %d, node[0]: %s", arg_info->info.cint,
                            abstract, mdb_nodetype[arg_node->node[0]->nodetype]));

    if (1 == abstract) {
        tmp_node1 = arg_node->node[0];

        id_node = MakeNode (N_id);
        id_node->info.ids = MakeIds (GenTmpVar (var_counter));
        arg_node->node[0] = id_node;

        let_node = MakeNode (N_let);
        let_node->info.ids = MakeIds (GenTmpVar (var_counter++));

        assign_node = MakeNode (N_assign);
        assign_node->node[0] = let_node;
        assign_node->node[1] = arg_info->node[0]; /* a new node is put in front! */
        if (NULL == assign_node->node[1])
            assign_node->nnode = 1;
        else
            assign_node->nnode = 2;

        arg_info->node[0] = assign_node;
        if (NULL != tmp_node1) {
            /* Now, we have to flatten the child "tmp_node1" recursively! */
            old_tag = arg_info->info.cint;

            if (tmp_node1->nodetype == N_ap)
                arg_info->info.cint = AP; /* set new tag */
            else if ((tmp_node1->nodetype == N_array) || (tmp_node1->nodetype == N_prf))
                arg_info->info.cint = NORMAL; /*set new tag */
            let_node->nnode = 1;
            let_node->node[0] = Trav (tmp_node1, arg_info);
            arg_info->info.cint = old_tag;
        }

    } else if ((arg_node->node[0]->nodetype == N_array)
               || (arg_node->node[0]->nodetype == N_prf)
               || (arg_node->node[0]->nodetype == N_ap)) {
        /* components of an array and arguments of a function have
         * to be flattend anyway.
         */
        old_tag = arg_info->info.cint;
        arg_info->info.cint = NORMAL;
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
        arg_info->info.cint = old_tag;
    } else if (N_id == arg_node->node[0]->nodetype)
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    /* Last, but not least remaining exprs have to be done */
    if (arg_node->nnode == 2)
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnCond
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   : Flatten each argument of the given node if neccessary
 *  global vars   :
 *  internal funs : Flatten
 *  external funs : Trav, MakeNode
 *  macros        :
 *
 *  remarks       : arg_info->node[1] contains list of assigns that have to
 *                   be put in front of a with_loop, if conditional is part
 *                   of a with_loop and there are variables to 'copy'
 *
 */

node *
FltnCond (node *arg_node, node *arg_info)
{
    int i;
    node *info_node;

    DBUG_ENTER ("FltnCond");

    info_node = MakeNode (N_info);
    info_node->nnode = 1;

    arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    for (i = 1; i < arg_node->nnode; i++) {
        info_node->node[0] = NULL;
        info_node->node[1] = arg_info->node[1]; /* list of assigns that have to be
                                                 * put infront of a with_loop
                                                 */
        arg_node->node[i] = Trav (arg_node->node[i], info_node);
        arg_info->node[1] = info_node->node[1]; /* list of assigns that have to be
                                                 * put infront of a with_loop
                                                 */
    }
    FREE (info_node);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnWhile
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   : Flatten each argument of the given node if neccessary
 *  global vars   :
 *  internal funs :
 *  external funs : Trav, MakeNode
 *  macros        :
 *
 *  remarks       : arg_info->node[1] contains list of assigns that have to
 *                   be put infront of a with_loop, if conditional is part
 *                   of a with_loop and there are variables to 'copy'
 *
 */
node *
FltnWhile (node *arg_node, node *arg_info)
{
    node *info_node, *tmp, *dest_node, *tmp_exprs;
    int old_tag;

    DBUG_ENTER ("FltnWhile");

    /* create a temporary N_exprs node to flatten the termination condition
     * in FltnExprs
     */
    tmp_exprs = MakeNode (N_exprs);
    tmp_exprs->node[0] = arg_node->node[0];
    tmp_exprs->nnode = 1;

    old_tag = arg_info->info.cint;
    arg_info->info.cint = LOOP; /* set tag for FltnExprs */
    tmp_exprs = Trav (tmp_exprs, arg_info);
    arg_node->node[0] = tmp_exprs->node[0]; /* set node of termination condition
                                             * correctly
                                             */
    FREE (tmp_exprs);
    arg_info->info.cint = old_tag; /* restore tag */

    DBUG_PRINT ("FLATTEN", ("arg_info: %s" P_FORMAT ": %s" P_FORMAT,
                            mdb_nodetype[arg_info->node[0]->nodetype], arg_info->node[0],
                            mdb_nodetype[arg_info->node[0]->node[0]->nodetype],
                            arg_info->node[0]->node[0]));

    info_node = MakeNode (N_info);
    info_node->nnode = 1;
    info_node->node[1] = arg_info->node[1]; /* list of assigns that have to be
                                             * put infront of a with_loop
                                             */

    arg_node->node[1] = Trav (arg_node->node[1], info_node);

    arg_info->node[1] = info_node->node[1]; /* list of assigns that have to be
                                             * put infront of a with_loop
                                             */

    DBUG_PRINT ("FLATTEN",
                ("info_node: %s" P_FORMAT ": %s" P_FORMAT,
                 mdb_nodetype[info_node->node[0]->nodetype], info_node->node[0],
                 mdb_nodetype[info_node->node[0]->node[0]->nodetype],
                 info_node->node[0]->node[0]));

    /*
     *  now we're looking for the last N_assign node in the pointer chain
     *  of info_node to copy the flattend arg_node->node[0] to it.
     *  This has to be done, because we must "update" (compute) the termination
     *  condition of the while loop at the end of the while-loop body
     *
     */

    tmp = info_node; /* tmp ist used to free info_node  */

    /*  looking for last N_assign node.
     *  info _node stores the pointer to the last N_assign nodes,
     *  so we look at this pointer chain instead of going through
     *  the chain behind  arg_node->node[1]
     */
    info_node = info_node->node[0];
    while (1 != info_node->nnode) {
        DBUG_ASSERT ((N_assign == info_node->nodetype), "wrong nodetype: != N_assign");
        info_node = info_node->node[1];
    }

    dest_node = arg_info->node[0];

    /*  now we create new N_assign nodes and  dublicate the flattened
     *  break condition of the while loop.
     */
    while (N_while != dest_node->node[0]->nodetype) {
        DBUG_ASSERT ((N_assign == dest_node->nodetype), "wrong nodetype: not N_assign");

        info_node->node[1] = MakeNode (N_assign);
        info_node->nnode = 2;
        info_node->node[1]->node[0] = DuplicateNode (dest_node->node[0]);
        info_node->node[1]->nnode = 1;
        info_node = info_node->node[1];
        dest_node = dest_node->node[1];
    }

    FREE (tmp);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnWith
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   :- Flatten each argument of the given node if necessary
 *                 - inserts initialisation of renamed (new) variable at
 *                   beginning of with_loop body
 *
 *  global vars   : with_level
 *  internal funs :
 *  external funs : Trav, MakeNode
 *  macros        : DBUG, ..., FREE
 *
 *  remarks       : ( initialisation assignments as N_assign chain are stored
 *                   in arg-info->node[1] )

 *
 */
node *
FltnWith (node *arg_node, node *arg_info)
{
    node *info_node, *tmp;
    local_stack *tmp_tos;

    DBUG_ENTER ("FltnWith");

    with_level += 1;

    info_node = MakeNode (N_info);
    info_node->nnode = 1;
    tmp_tos = tos;                                           /* store tos */
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);  /* traverse generator */
    arg_node->node[1] = Trav (arg_node->node[1], info_node); /* traverse  body */

    with_level -= 1; /* now decrease it */

    tos = tmp_tos; /*restore tos */

    /* insert assignments stored in arg_info->node[1] */
    if (NULL != info_node->node[1]) {
        tmp = info_node->node[1];
        while (1 < tmp->nnode)
            tmp = tmp->node[1];
        tmp->node[1] = arg_node->node[1]->node[1]->node[0];
        tmp->nnode = 2;
        arg_node->node[1]->node[1]->node[0] = info_node->node[1];
    }

    FREE (info_node);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnDo
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   : Flatten each argument of the given node if neccessary
 *  global vars   :
 *  internal funs :
 *  external funs : Trav, MakeNode
 *  macros        : DBUG, ..., FREE
 *
 *  remarks       : arg_info->node[1] contains list of assigns that have to
 *                   be put infront of a with_loop, if conditional is part
 *                   of a with_loop and there are variables to 'copy'
 *
 */
node *
FltnDo (node *arg_node, node *arg_info)
{
    node *info_node, *tmp, *last_assign, *tmp_exprs;
    int old_tag;

    DBUG_ENTER ("FltnDo");

    info_node = MakeNode (N_info);
    info_node->nnode = 1;
    info_node->node[1] = arg_info->node[1]; /* list of assigns that have to be
                                             * put infront of a with_loop
                                             */
    /* traverse body of do-loop */
    arg_node->node[1] = Trav (arg_node->node[1], info_node);

    DBUG_PRINT ("FLATTEN",
                ("info_node: %s" P_FORMAT ": %s" P_FORMAT,
                 mdb_nodetype[info_node->node[0]->nodetype], info_node->node[0],
                 mdb_nodetype[info_node->node[0]->node[0]->nodetype],
                 info_node->node[0]->node[0]));

    /* store info_node */
    tmp = info_node;

    /* looking for last N_assign node in the body of the do-loop */
    info_node = info_node->node[0];
    while (1 != info_node->nnode) {
        DBUG_ASSERT ((N_assign == info_node->nodetype), "wrong nodetype:"
                                                        "!= N_assign");
        info_node = info_node->node[1];
    }

    DBUG_PRINT ("FLATTEN",
                ("info_node: %s" P_FORMAT ": %s" P_FORMAT,
                 mdb_nodetype[info_node->nodetype], info_node,
                 mdb_nodetype[info_node->node[0]->nodetype], info_node->node[0]));

    /* store last N_assign node of the body */
    last_assign = info_node;

    /* clear info_node */
    info_node = tmp;
    info_node->node[0] = NULL;

    /* create a temporary N_exprs node to flatten the termination condition
     * in FltnExprs
     */
    tmp_exprs = MakeNode (N_exprs);
    tmp_exprs->node[0] = arg_node->node[0];
    tmp_exprs->nnode = 1;

    /* travers termination condition */
    old_tag = info_node->info.cint;
    info_node->info.cint = LOOP; /* set tag for FltnExprs */
    tmp_exprs = Trav (tmp_exprs, info_node);
    arg_node->node[0] = tmp_exprs->node[0]; /* set node of termination condition
                                             * correctly
                                             */

    arg_info->node[1] = info_node->node[1]; /* list of assigns that have to be
                                             * put infront of a with_loop
                                             */
    FREE (tmp_exprs);

    /* append flattened termination condition to last assignment
     * in the loop's body
     */
    last_assign->node[1] = info_node->node[0];
    if (NULL != last_assign->node[1]) {
        DBUG_PRINT ("FLATTEN",
                    ("info_node: %s" P_FORMAT ": %s" P_FORMAT,
                     mdb_nodetype[info_node->node[0]->nodetype], info_node->node[0],
                     mdb_nodetype[info_node->node[0]->node[0]->nodetype],
                     info_node->node[0]->node[0]));
        last_assign->nnode = 2;
    }
    FREE (tmp);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnModul
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   : call Trav to flatten the user defined functions
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        :
 *
 *  remarks       :
 *
 */
node *
FltnModul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FltnModul");
    DBUG_ASSERT ((NULL != arg_node->node[2]), "blabla");
    DBUG_ASSERT ((N_fundef == arg_node->node[2]->nodetype), "blaaa");

    arg_node->node[2] = Trav (arg_node->node[2], arg_info);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnFundef
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   : - calls Trav to flatten the user defined functions
 *                    if function body is not empty
 *                  - reset tos after flatten of the function's body
 *  global vars   : tos
 *  internal funs :
 *  external funs : Trav
 *  macros        :
 *
 *  remarks       : the formal parameters of a function will be traversed
 *                   to put their name on the local stack
 *
 */
node *
FltnFundef (node *arg_node, node *arg_info)
{
    local_stack *tmp_tos;

    DBUG_ENTER ("FltnFundef");

    tmp_tos = tos; /* store tos */

    if (NULL != arg_node->node[0]) {
        if (NULL != arg_node->node[2])
            arg_node->node[2] = Trav (arg_node->node[2], arg_info);

        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    }
    tos = tmp_tos; /* restore tos */

    if (NULL != arg_node->node[1])
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnGen
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   : - flattens each argument of the given node, if necessary
 *  global vars   : var_counter
 *  internal funs : GenTmpVar
 *  external funs : MakeNode, Trav,
 *  macros        : DBUG,..., GEN_NODE
 *
 *  remarks       :
 *
 */
node *
FltnGen (node *arg_node, node *arg_info)
{
    node *tmp_node1, *id_node, *let_node, *assign_node;
    int i;
    local_stack *tmp;
    char *old_name;

    DBUG_ENTER ("FltnGen");
    for (i = 0; i < arg_node->nnode; i++)
        if ((arg_node->node[i]->nodetype == N_ap)
            || (arg_node->node[i]->nodetype == N_prf)) {

            /* This argument is a function application and thus has to be abstracted
             ** out. Therefore a new N_assign, a new N_let, and a new temporary
             ** variable are generated and inserted.
             */
            tmp_node1 = arg_node->node[i];

            id_node = MakeNode (N_id);
            id_node->info.ids = MakeIds (GenTmpVar (var_counter));
            arg_node->node[i] = id_node;

            let_node = MakeNode (N_let);
            let_node->info.ids = MakeIds (GenTmpVar (var_counter++));

            assign_node = MakeNode (N_assign);
            assign_node->node[0] = let_node;
            assign_node->node[1] = arg_info->node[0]; /* a new node is put in front! */
            if (NULL == assign_node->node[1])
                assign_node->nnode = 1;
            else
                assign_node->nnode = 2;

            arg_info->node[0] = assign_node;

            /* Now, we have to flatten the child "tmp_node1" recursively! */
            let_node->nnode = 1;
            let_node->node[0] = Trav (tmp_node1, arg_info);
        } else
            arg_node->node[i] = Trav (arg_node->node[i], arg_info);

    /* rename index-vector if necessary */
    tmp = FindId (arg_node->IDS_ID);
    if (NULL != tmp) {
        old_name = arg_node->IDS_ID;
        arg_node->IDS_ID = RenameWithVar (old_name, with_level);
        PUSH (old_name, arg_node->IDS_ID, with_level);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnAp
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   : set tag arg_info->info.cint for flatten of arguments
 *                  call Trav to flatten arguments
 *                  if function body is not empty
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG, AP, NULL
 *
 *  remarks       :
 *
 */
node *
FltnAp (node *arg_node, node *arg_info)
{
    int old_tag;

    DBUG_ENTER ("FltnAp");

    if (NULL != arg_node->node[0]) {
        old_tag = arg_info->info.cint;
        arg_info->info.cint = AP;
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
        arg_info->info.cint = old_tag;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnReturn
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   : set tag arg_info->info.cint for flatten of arguments
 *                  call Trav to flatten arguments
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG, RET, NULL
 *
 *  remarks       :
 *
 */
node *
FltnReturn (node *arg_node, node *arg_info)
{
    int old_tag;

    DBUG_ENTER ("FltnReturn");

    if (NULL != arg_node->node[0]) {
        old_tag = arg_info->info.cint;
        arg_info->info.cint = RET;
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
        arg_info->info.cint = old_tag;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnId
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   : renames identifier if neccessary
 *  global vars   : with_level
 *  internal funs : FindFun
 *  external funs : StringCopy
 *  macros        : DBUG, RET, NULL
 *
 *  remarks       :- this function is only used for renaming of variables
 *                   of a with_loop

 *
 */
node *
FltnId (node *arg_node, node *arg_info)
{
    char *old_name;
    local_stack *tmp;

    DBUG_ENTER ("FltnId");

    if (0 < with_level) {
        tmp = FindId (arg_node->IDS_ID);
        if (NULL != tmp)
            if ((0 < with_level) && (with_level >= tmp->w_level) && (0 < tmp->w_level)) {
                old_name = arg_node->IDS_ID;
                arg_node->IDS_ID = StringCopy (tmp->id_new);
                FREE (old_name);
            }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnLet
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   : - renames variable assigned to and adds a new node
 *                    to arg_info->node[1], if necessary
 *                  - adds assigned to variable to local stack, if they
 *                    are not in ste stack
 *  global vars   : with_level
 *  internal funs : RenameWithVar, AppendIdentity
 *  external funs :
 *  macros        : DBUG, NULL
 *
 *  remarks       :  arg_info->node[1] contains list of assigns that have to
 *                   be put in front of a with_loop
 *                 - this function is only used for renaming of variables
 *                   of a with_loop
 */
node *
FltnLet (node *arg_node, node *arg_info)
{
    ids *ids;
    char *old_name;
    local_stack *tmp;

    DBUG_ENTER ("FltnLet");

    /* add assigned variable to local stack only if it is not part of it */
    ids = arg_node->IDS;
    do {
        tmp = FindId (ids->id);
        if (NULL == tmp) {
            PUSH (ids->id, ids->id, with_level);

        } else {
            if ((0 < with_level) && (with_level > tmp->w_level)) {
                old_name = ids->id;
                ids->id = RenameWithVar (old_name, with_level);
                PUSH (old_name, ids->id, with_level);
                arg_info->node[1]
                  = AppendIdentity (arg_info->node[1], StringCopy (tmp->id_new), ids->id);
            }
        }
        ids = ids->next;
    } while (NULL != ids);

    arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FltnArgs
 *  arguments     : 1) argument node
 *                  2) last assignment in arg_info->node[0]
 *  description   : adds name of formal parameters to local stack and calls
 *                  Trav to traverse next node if any
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG, NULL, PUSH
 *
 *  remarks       :
 *
 */
node *
FltnArgs (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FltnArgs");

    PUSH (arg_node->ID, arg_node->ID, 0);
    if (NULL != arg_node->node[0])
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    DBUG_RETURN (arg_node);
}
