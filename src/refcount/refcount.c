/*
 *
 * $Log$
 * Revision 1.1  1995/03/09 16:17:01  hw
 * Initial revision
 *
 *
 */

#include "tree.h"
#include "my_debug.h"
#include "dbug.h"
#include "typecheck.h" /* to use LookupType */
#include "traverse.h"
#include "refcount.h"

#define BODY 1
#define LOOP 2
#define COND 3

#define CONTEXT info.cint
#define DUB_ID_NODE(a, b)                                                                \
    a = MakeNode (N_id);                                                                 \
    a->info.id = StringCopy (b->info.id);                                                \
    a->node[1] = b->node[1]
#define MAX(a, b) (a >= b) ? a : b

/*
 *
 *  functionname  : IsArray
 *  arguments     : 1) N_typedef node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
int
IsArray (node *arg_node)
{
    node *type_node;
    int ret = 0;

    DBUG_ENTER ("IsArray");

    if (1 <= arg_node->info.types->dim)
        ret = 1;
    else if (T_user == arg_node->info.types->simpletype) {
        type_node
          = LookupType (arg_node->info.types->name, arg_node->info.types->name_mod, 042);
        /* 042 is only a dummy argument */
        if (1 <= type_node->info.types->dim)
            ret = 1;
    }
    DBUG_RETURN (ret);
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
 *  external funs :
 *  macros        : DBUG...
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
        if (0 == strcmp (id, tmp->info.id)) {
            ret_node = tmp;
            break;
        } else
            tmp = tmp->node[0];
    DBUG_RETURN (ret_node);
}

/*
 *
 *  functionname  : MaxInRefs
 *  arguments     : 1)
 *                  2)
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
void
MaxInRefs (node *id_nodes1, node *id_nodes2, node *ref_nodes)
{
    node *done1, *done2, *last1, *last2, *tmp1, *tmp2, *ref_node, *new_id_node;
    int found;

    DBUG_ENTER ("MaxInRefs");
    while (NULL != id_nodes1) {
        found = 0;
        last2 = id_nodes2;
        tmp2 = last2;
        while ((NULL != tmp2) && (0 == found))
            if (0 == strcmp (tmp2->info.id, id_nodes1->info.id)) {
                /* store max refcount */
                if (NULL == ref_nodes)
                    id_nodes1->node[1]->refcnt = MAX (id_nodes1->refcnt, tmp2->refcnt);
                else {
                    ref_node = LookupId (id_nodes1->info.id, ref_nodes);
                    if (NULL == ref_node) {
                        /* make new N_id node with information of id_nodes1 */
                        DUB_ID_NODE (new_id_node, id_nodes1);
                        new_id_node->refcnt = MAX (id_nodes1->refcnt, tmp2->refcnt);
                        new_id_node->node[0] = ref_nodes;
                        ref_nodes = new_id_node;
                    } else
                        ref_node->refcnt = MAX (id_nodes1->refcnt, tmp2->refcnt);
                }

                /* displace current id_nodes1 to N_id node chain done1 */
                tmp1 = id_nodes1->node[0];
                id_nodes1->node[0] = done1;
                done1 = id_nodes1;
                id_nodes1 = tmp1;

                /* displace tmp2 to N_id node chain done2 */
                last2->node[0] = tmp2->node[0];
                tmp2->node[0] = done2;
                done2 = tmp2;
                found = 1;
            } else {
                last2 = tmp2;
                tmp2 = tmp2->node[0];
            }

        if (0 == found) {
            id_nodes1->node[1]->refcnt = id_nodes1->refcnt;
            /* store max refcount */
            if (NULL == ref_nodes)
                id_nodes1->node[1]->refcnt = id_nodes1->refcnt;
            else {
                ref_node = LookupId (id_nodes1->info.id, ref_nodes);
                if (NULL == ref_node) {
                    /* make new N_id node with information of id_nodes1 */
                    DUB_ID_NODE (new_id_node, id_nodes1);

                    new_id_node->refcnt = id_nodes1->refcnt;
                    new_id_node->node[0] = ref_nodes;
                    ref_nodes = new_id_node;
                } else
                    ref_node->refcnt = id_nodes1->refcnt;
            }

            /* displace current id_nodes1 to N_id node chain done1 */
            tmp1 = id_nodes1->node[0];
            id_nodes1->node[0] = done1;
            done1 = id_nodes1;
            id_nodes1 = tmp1;
        }
    }

    while (NULL != id_nodes2) {
        found = 0;
        last1 = id_nodes1;
        tmp1 = last1;
        while ((NULL != tmp1) && (0 == found))
            if (0 == strcmp (tmp1->info.id, id_nodes2->info.id)) {
                /* store max refcount */
                if (NULL == ref_nodes)
                    id_nodes2->node[1]->refcnt = MAX (id_nodes2->refcnt, tmp1->refcnt);
                else {
                    ref_node = LookupId (id_nodes2->info.id, ref_nodes);
                    if (NULL == ref_node) {
                        /* make new N_id node with information of id_nodes1 */
                        DUB_ID_NODE (new_id_node, id_nodes1);
                        new_id_node->refcnt = MAX (id_nodes2->refcnt, tmp1->refcnt);
                        new_id_node->node[0] = ref_nodes;
                        ref_nodes = new_id_node;
                    } else
                        ref_node->refcnt = MAX (id_nodes2->refcnt, tmp1->refcnt);
                }

                /* displace current id_nodes2 to N_id node chain done2 */
                tmp2 = id_nodes2->node[0];
                id_nodes2->node[0] = done2;
                done2 = id_nodes2;
                id_nodes2 = tmp2;

                /* displace tmp1 to N_id node chain done1 */
                last1->node[0] = tmp1->node[0];
                tmp1->node[0] = done1;
                done1 = tmp1;
                found = 1;
            } else {
                last1 = tmp1;
                tmp1 = tmp1->node[0];
            }

        if (0 == found) {
            /* store max refcount */
            if (NULL == ref_nodes)
                id_nodes2->node[1]->refcnt = id_nodes2->refcnt;
            else {
                ref_node = LookupId (id_nodes2->info.id, ref_nodes);
                if (NULL == ref_node) {
                    /* make new N_id node with information of id_nodes1 */
                    DUB_ID_NODE (new_id_node, id_nodes1);
                    new_id_node->refcnt = id_nodes2->refcnt;
                    new_id_node->node[0] = ref_nodes;
                    ref_nodes = new_id_node;
                } else
                    ref_node->refcnt = id_nodes2->refcnt;
            }

            /* displace current id_nodes2 to N_id node chain done1 */
            tmp2 = id_nodes2->node[0];
            id_nodes2->node[0] = done2;
            done2 = id_nodes2;
            id_nodes2 = tmp2;
        }
    }
    DBUG_ASSERT ((NULL == id_nodes1), "id_nodes1 isn't NULL");
    DBUG_ASSERT ((NULL == id_nodes2), "id_nodes2 isn't NULL");

    id_nodes1 = done1;
    id_nodes2 = done2;

    DBUG_ASSERT ((NULL != id_nodes1), "id_nodes1 is NULL");
    DBUG_ASSERT ((NULL != id_nodes2), "id_nodes2 is NULL");

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : RCassign
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
 *  functionname  : RCwhile
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
RCwhile (node *arg_node, node *arg_info)
{
    node *new_info;

    DBUG_ENTER ("RCwhile");
    /* first traverse termination condition */
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    /* create new info node */
    new_info = MakeNode (N_info);
    new_info->CONTEXT = LOOP;

    /* traverse body of loop */
    arg_node->node[1] = Trav (arg_node->node[1], new_info);

    /* store new_info for use while compilation */
    arg_node->node[2] = new_info;

    DBUG_RETURN (arg_node);
}

#if 0
/*
 *
 *  functionname  : RCdo
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
node *RCdo(node *arg_node, node *arg_info)
{
   node *new_info;
   
   DBUG_ENTER("RCdo");
   
   /* create new info node */
   new_info=MakeNode(N_info);
   new_info->CONTEXT=LOOP;

   /* traverse body  and termination condition of loop */
   arg_node->node[1]=Trav(arg_node->node[1], new_info);
   arg_node->node[0]=Trav(arg_node->node[0], new_info);
      
   /* store new_info for use while compilation */
   arg_node->node[2]=new_info;
   
   DBUG_RETURN(arg_node);
}
#endif

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
 *  remarks       :
 *
 */
node *
RCid (node *arg_node, node *arg_info)
{
    node *id_node;

    DBUG_ENTER ("RCid");

    if (1 == IsArray (arg_node->node[0])) {
        switch (arg_info->CONTEXT) {
        case BODY: {
            arg_node->node[0]->refcnt += 1;
            arg_node->refcnt = arg_node->node[0]->refcnt;
            break;
        }
        case LOOP: {
            if (NULL == arg_info->node[0]) {
                DUB_ID_NODE (id_node, arg_node);
                id_node->refcnt = 1;
                arg_info->node[0] = id_node;
            } else {
                id_node = LookupId (arg_node->info.id, arg_info->node[0]);
                if (NULL == id_node) {
                    DUB_ID_NODE (id_node, arg_node);
                    id_node->refcnt = 1;
                    /* now put id_node in front of list of N_id */
                    id_node->node[0] = arg_info->node[0];
                    arg_info->node[0] = id_node;
                } else
                    id_node->refcnt += 1;
            }
            arg_node->refcnt = id_node->refcnt; /*set refcount */
            break;
        }
        default:
            DBUG_ASSERT (0, "unknown info tag ");
            break;
        }
    } else
        arg_node->refcnt = -1; /* variable isn`t an array */

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
    node *id_node;

    DBUG_ENTER ("RClet");

    ids = arg_node->info.ids;
    while (NULL != ids) {
        if (1 == IsArray (ids->node)) {
            switch (arg_info->CONTEXT) {
            case BODY: {
                ids->refcnt = ids->node->refcnt;
                break;
            }
            case LOOP: {
                if (NULL == arg_info->node[0])
                    ids->refcnt = 0;
                else {
                    id_node = LookupId (arg_node->info.id, arg_info->node[0]);
                    if (NULL == id_node)
                        ids->refcnt = 0;
                    else {
                        ids->refcnt = id_node->refcnt;
                        id_node->refcnt = 0;
                    }
                }
                break;
            }
            default:
                DBUG_ASSERT (0, "unknown info tag ");
                break;
            }
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
RCcond (node *arg_node, node *arg_info)
{
    node *then_info, *else_info;

    DBUG_ENTER ("RCcond");

    then_info = MakeNode (N_info);
    then_info->CONTEXT = COND;
    then_info->node[1] = arg_info->node[0];
    arg_node->node[1] = Trav (arg_node->node[1], then_info);

    else_info = MakeNode (N_info);
    else_info->CONTEXT = COND;
    else_info->node[1] = arg_info->node[0];
    arg_node->node[2] = Trav (arg_node->node[2], else_info);

    if (BODY == arg_info->CONTEXT)
        MaxRefs (then_info->node[0], else_info->node[0], NULL);
    else
        MaxRefs (then_info->node[0], else_info->node[0], arg_info);

    /* store refcount information for use while compilation */
    then_info->node[1] = else_info->node[0];
    FREE (else_info);
    arg_node->node[3] = then_info;

    /* last but not least, traverse condition */
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    DBUG_RETURN (arg_node);
}
