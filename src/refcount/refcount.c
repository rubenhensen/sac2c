/*
 *
 * $Log$
 * Revision 1.3  1995/03/14 18:45:21  hw
 * renamed RCwhile to RCloop
 * this version handles do- and while-loops correct.
 * conditionals are not implemented
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
#include "refcount.h"

#define BODY 1
#define WHILE 2
#define DO 3
#define COND 4

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

#define MAX(a, b) (a >= b) ? a : b
#define FREE(a) free (a)

static int varno;         /* used to store the number of known variables in a
                             sac-function (used for mask[])
                           */
static int args_no;       /* number of arguments of current function */
static node *fundef_node; /* pointer to current function declaration */

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
    DBUG_PRINT ("RC", ("looking for %s", arg_node->info.types->id));

    if (1 <= arg_node->info.types->dim)
        ret = 1;
    else if (T_user == arg_node->info.types->simpletype) {
        type_node
          = LookupType (arg_node->info.types->name, arg_node->info.types->name_mod, 042);
        /* 042 is only a dummy argument */
        if (1 <= type_node->info.types->dim)
            ret = 1;
    }
    DBUG_PRINT ("RC", ("%d", ret));

    DBUG_RETURN (ret);
}

/*
 *
 *  functionname  : FindVarDec
 *  arguments     : 1) number of variable
 *  description   : returns pointer to variabledeclaration if found
 *                  returns NULL if not found
 *  global vars   : fundef_node, arg_no
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
        if (0 == strcmp (id, tmp->ID)) {
            ret_node = tmp;
            break;
        } else
            tmp = tmp->node[0];
    DBUG_PRINT ("RC", ("found %s:" P_FORMAT, id, ret_node));
    DBUG_RETURN (ret_node);
}
#if 0
/*
 *
 *  functionname  : MaxRefs
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
void MaxRefs(node *id_nodes1, node *id_nodes2, node *ref_nodes)
{
   node *done1, *done2, *last1, *last2, *tmp1, *tmp2, *ref_node, 
        *new_id_node;
   int found;
   
   DBUG_ENTER("MaxRefs");
   while(NULL != id_nodes1) 
   {
      found=0;
      last2=id_nodes2;
      tmp2=last2;
      while((NULL !=tmp2) && (0 == found))
         if(0==strcmp(tmp2->info.id, id_nodes1->info.id))
         {
            /* store max refcount */
            if(NULL == ref_nodes)
            {
               id_nodes1->node[0]->ID_REF=MAX(id_nodes1->ID_REF, tmp2->ID_REF);
               DBUG_PRINT("RC",("set refcnt of %s to %d",
                                id_nodes1->node[0]->ID, 
                                id_nodes1->node[0]->ID_REF));
            }
            else
            {
               ref_node=LookupId(id_nodes1->ID, ref_nodes->node[0]);
               if(NULL == ref_node)
               {
                  /* make new N_id node with information of id_nodes1 */
                  DUB_ID_NODE(new_id_node, id_nodes1); 
                  new_id_node->ID_REF=MAX(id_nodes1->ID_REF, tmp2->ID_REF);
                  new_id_node->node[1]=ref_nodes;
                  ref_nodes=new_id_node;
               }
               else
               {
                  ref_node->ID_REF=MAX(id_nodes1->ID_REF, tmp2->ID_REF);
                  DBUG_PRINT("RC",("set refcnt of %s to %d",
                                   ref_node->ID, 
                                   ref_node->ID_REF));
               }
            }
            
            /* displace current id_nodes1 to N_id node chain done1 */
            tmp1=id_nodes1->node[1];
            id_nodes1->node[1]=done1;
            done1=id_nodes1;
            id_nodes1=tmp1;
            
            /* displace tmp2 to N_id node chain done2 */
            if(last2 != tmp2)
            {
               last2->node[1]=tmp2->node[1];
               tmp2->node[1]=done2;
               done2=tmp2;
            }
            else
            {  /* displace first node of id_nodes2 to done2 */
               DBUG_ASSERT((id_nodes2==last2)," last2 != id_nodes2");
               id_nodes2=id_nodes2->node[1];
               tmp2->node[1]=done2;
               done2=tmp2;
            }
            
            found=1;
         }
         else
         {
            last2=tmp2;
            tmp2=tmp2->node[1];
         }
      
      if(0 == found)
      {
         /* store max refcount */
         if(NULL == ref_nodes)
         {
            id_nodes1->node[0]->ID_REF=id_nodes1->ID_REF;
            DBUG_PRINT("RC",("set refcnt of %s to %d",
                             id_nodes1->node[0]->ID, 
                             id_nodes1->node[0]->ID_REF));
         }
         else
         {
            ref_node=LookupId(id_nodes1->info.id, ref_nodes->node[0]);
            if(NULL == ref_node)
            {
               /* make new N_id node with information of id_nodes1 */
               DUB_ID_NODE(new_id_node, id_nodes1); 
               
               new_id_node->ID_REF=id_nodes1->ID_REF;
               new_id_node->node[1]=ref_nodes;
               ref_nodes=new_id_node;
            }
            else
            {
               ref_node->ID_REF=id_nodes1->ID_REF;
               DBUG_PRINT("RC",("set refcnt of %s to %d",
                                ref_node->ID, 
                                ref_node->ID_REF));
            }
            
         }
         
         /* displace current id_nodes1 to N_id node chain done1 */
         tmp1=id_nodes1->node[1];
         id_nodes1->node[1]=done1;
         done1=id_nodes1;
         id_nodes1=tmp1;
      }
   }

   while(NULL != id_nodes2) 
   {
      found=0;
      last1=id_nodes1;
      tmp1=last1;
      while((NULL !=tmp1) && (0 == found))
         if(0==strcmp(tmp1->info.id, id_nodes2->info.id))
         {
            /* store max refcount */
            if(NULL == ref_nodes)
            {
               id_nodes2->node[0]->ID_REF=MAX(id_nodes2->ID_REF, tmp1->ID_REF);
               DBUG_PRINT("RC",("set refcnt of %s to %d",
                                id_nodes2->node[0]->ID, 
                                id_nodes2->node[0]->ID_REF));
            }
            else
            {
               ref_node=LookupId(id_nodes2->info.id, ref_nodes->node[0]);
               if(NULL == ref_node)
               {
                  /* make new N_id node with information of id_nodes1 */
                  DUB_ID_NODE(new_id_node, id_nodes1); 
                  new_id_node->ID_REF=MAX(id_nodes2->ID_REF, tmp1->ID_REF);
                  new_id_node->node[1]=ref_nodes;
                  ref_nodes=new_id_node;
               }
               else
               {
                  
                  ref_node->ID_REF=MAX(id_nodes2->ID_REF, tmp1->ID_REF);
                  DBUG_PRINT("RC",("set refcnt of %s to %d",
                                   ref_node->ID, 
                                   ref_node->ID_REF));
               }
            }
            
            /* displace current id_nodes2 to N_id node chain done2 */
            tmp2=id_nodes2->node[1];
            id_nodes2->node[1]=done2;
            done2=id_nodes2;
            id_nodes2=tmp2;
            
            /* displace tmp1 to N_id node chain done1 */
            if(last1 != tmp1)
            {
               last1->node[1]=tmp1->node[1];
               tmp1->node[1]=done1;
               done1=tmp1;
            }
            else
            {
               /* displace first node of id_nodes1 to done1 */
               DBUG_ASSERT((id_nodes1==last1)," last1 != id_nodes1");
               id_nodes1=id_nodes1->node[1];
               tmp1->node[1]=done1;
               done1=tmp1;
            }
               
            found=1;
         }
         else
         {
            last1=tmp1;
            tmp1=tmp1->node[1];
         }
      
      if(0 == found)
      {
         /* store max refcount */
         if(NULL == ref_nodes)
         {
            id_nodes2->node[0]->ID_REF=id_nodes2->ID_REF;
            DBUG_PRINT("RC",("set refcnt of %s to %d",
                             id_nodes2->node[0]->ID, 
                             id_nodes2->node[0]->ID_REF));
         }
         else
         {
            ref_node=LookupId(id_nodes2->info.id, ref_nodes->node[0]);
            if(NULL == ref_node)
            {
               /* make new N_id node with information of id_nodes1 */
               DUB_ID_NODE(new_id_node, id_nodes1); 
               new_id_node->ID_REF=id_nodes2->ID_REF;
               new_id_node->node[1]=ref_nodes;
               ref_nodes=new_id_node;
            }
            else
            {
               ref_node->ID_REF=id_nodes2->ID_REF;
               DBUG_PRINT("RC",("set refcnt of %s to %d",
                                ref_node->ID, 
                                ref_node->ID_REF));
            }
         }
        
         /* displace current id_nodes2 to N_id node chain done1 */
         tmp2=id_nodes2->node[1];
         id_nodes2->node[1]=done2;
         done2=id_nodes2;
         id_nodes2=tmp2;
      }
   }
   DBUG_ASSERT((NULL == id_nodes1),"id_nodes1 isn't NULL");
   DBUG_ASSERT((NULL == id_nodes2),"id_nodes2 isn't NULL");
   
   id_nodes1=done1;
   id_nodes2=done2;

   DBUG_ASSERT((NULL != id_nodes1),"id_nodes1 is NULL");
   DBUG_ASSERT((NULL != id_nodes2),"id_nodes2 is NULL");
   
   
   DBUG_VOID_RETURN;
}
#endif

/*
 *
 *  functionname  : Refcount
 *  arguments     : 1) argument node
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
RCfundef (node *arg_node, node *arg_info)
{
    node *args;

    DBUG_ENTER ("RCfundef");

    if (NULL != arg_node->node[0]) {
        arg_info->CONTEXT = BODY;

        /* setting some global variables to use with the 'mask' */
        varno = arg_node->varno;
        fundef_node = arg_node;
        args = arg_node->node[1];
        args_no = 0;
        while (NULL != args)
            args_no += 1;

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
    if (1 == arg_node->nnode) {
        arg_info->node[3] = arg_node; /* pointer to last assign */
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    } else {
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
        arg_info->node[3] = arg_node; /* pointer to last assign */
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RCloop
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       : in arg_node->node[2] some information about used variables
 *                  are stored:
 *                   - arg_node->node[2]->node[0]: chain of N_id nodes that
 *                      contain the refcounts of used variables of while-body
 *                   - arg_node->node[2]->node[1]: chain of N_id nodes that
 *                      contain the refcounts of defined variables of code
 *                      behinde while-loop
 *
 */
node *
RCloop (node *arg_node, node *arg_info)
{
    node *new_info, *var_dec, *id_node, *id_node2, *new_id_node;
    long *defined_mask, *used_mask;
    int i, found_id;

    DBUG_ENTER ("RCloop");

    /* create new info node */
    new_info = MakeNode (N_info);
    if (N_while == arg_node->nodetype)
        new_info->CONTEXT = WHILE;
    else
        new_info->CONTEXT = DO;

    /* traverse body of loop */
    arg_node->node[1] = Trav (arg_node->node[1], new_info);

    /* update refcounts for variables that are defined and used in while-loop */
    defined_mask = arg_info->node[3]->mask[0]; /* mask of defined variables */
    used_mask = arg_info->node[3]->mask[1];    /* mask of used variables */
    if (BODY == arg_info->CONTEXT) {
        for (i = 0; i < varno; i++)
            if ((defined_mask[i] > 0) || (used_mask[i] > 0)) {
                var_dec = FindVarDec (i);
                DBUG_ASSERT ((NULL != var_dec), "variable not found");
                if (defined_mask[i] > 0) {
                    /* store refcount of defined variables in new_info->node[2] */
                    VAR_DEC_2_ID_NODE (id_node, var_dec);
                    id_node->node[0] = new_info->node[2];
                    new_info->node[2] = id_node;
                    DBUG_PRINT ("RC", ("store defined var %s:%d", id_node->ID,
                                       id_node->ID_REF));

                    /* set refcount of defined variables in variable declaration.
                       this is done to emulate a function application
                     */
                    if (N_do == arg_node->nodetype) {
                        /* check weather id_node 'is an argument of virtuell function'
                         */
                        id_node2 = LookupId (id_node->ID, new_info->node[0]);
                        if (NULL != id_node2)
                            if (0 == id_node2->ID_REF)
                                var_dec->refcnt = 0;
                            else
                                var_dec->refcnt = 1;
                        else
                            var_dec->refcnt = 1;
                    } else
                        var_dec->refcnt = 1;
                } else
                    var_dec->refcnt += 1;
            }
    } else {
        for (i = 0; i < varno; i++)
            if ((defined_mask[i] > 0) || (used_mask[i] > 0)) {
                var_dec = FindVarDec (i);
                DBUG_ASSERT ((NULL != var_dec), "variable not found");
                id_node = LookupId (var_dec->info.types->id, arg_info->node[0]);
                if (NULL == id_node) {
                    VAR_DEC_2_ID_NODE (id_node, var_dec);
                    id_node->node[0] = arg_info->node[0];
                    arg_info->node[0] = id_node->node[0];
                    found_id = 0; /* indicates that id_node was NULL */
                } else
                    found_id = 1;

                /* id_node now is a N_id node of the chain behinde arg_info->node[0]
                 */

                if (defined_mask[i] > 0) {
                    /* store refcount of defined variables in new_info->node[2] */
                    if (1 == found_id) {
                        DUB_ID_NODE (new_id_node, id_node);
                        new_id_node->ID_REF = id_node->ID_REF;
                        new_id_node->node[0] = new_info->node[2];
                        new_info->node[2] = new_id_node;
                        DBUG_PRINT ("RC", ("store defined var %s:%d", new_id_node->ID,
                                           new_id_node->ID_REF));
                    }

                    /* set refcount of defined variables in surrounding context
                     * this is done to emulate a function application
                     */
                    if (N_do == arg_node->nodetype) {
                        /* check weather id_node 'is an argument of virtuell
                         * function'
                         */
                        id_node2 = LookupId (id_node->ID, new_info->node[0]);

                        /* change refcount of 'id_node->ID' in surrounding
                         * context
                         */

                        if (NULL != id_node2)
                            if (0 == id_node2->ID_REF)
                                id_node->ID_REF = 0;
                            else
                                id_node->ID_REF = 1;
                        else
                            id_node->ID_REF = 1;
                    } else
                        id_node->ID_REF = 1;
                } else
                    id_node->ID_REF += 1;
            }
    }

    /* store new_info for use while compilation */
    arg_node->node[2] = new_info;

    /* traverse termination condition */
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);

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
 *  remarks       :arg_node->info.ids->node: pointer to VarDec
 *
 */
node *
RCid (node *arg_node, node *arg_info)
{
    node *id_node;

    DBUG_ENTER ("RCid");

    if (1 == IsArray (arg_node->VAR_DEC)) {
        switch (arg_info->CONTEXT) {
        case BODY: {
            arg_node->VAR_DEC->refcnt += 1;
            arg_node->ID_REF = arg_node->VAR_DEC->refcnt;
            break;
        }
        case COND: {
            if (NULL == arg_info->node[0]) {
                DUB_ID_NODE (id_node, arg_node);
                id_node->ID_REF = arg_node->VAR_DEC->refcnt + 1;
                arg_info->node[0] = id_node;
            } else {
                id_node = LookupId (arg_node->ID, arg_info->node[0]);
                if (NULL == id_node) {
                    DUB_ID_NODE (id_node, arg_node);
                    id_node->ID_REF = 1;
                    /* now put id_node in front of list of N_id */
                    id_node->node[0] = arg_info->node[0];
                    arg_info->node[0] = id_node;
                } else
                    id_node->ID_REF += 1;
            }
            arg_node->ID_REF = id_node->ID_REF; /*set refcount */
            break;
        }
        case WHILE:
        case DO: {
            if (NULL == arg_info->node[0]) {
                DUB_ID_NODE (id_node, arg_node);
                id_node->ID_REF = 2;
                arg_info->node[0] = id_node;
            } else {
                id_node = LookupId (arg_node->ID, arg_info->node[0]);
                if (NULL == id_node) {
                    DUB_ID_NODE (id_node, arg_node);
                    id_node->ID_REF = 2;
                    /* now put id_node in front of list of N_id */
                    id_node->node[0] = arg_info->node[0];
                    arg_info->node[0] = id_node;
                } else
                    id_node->ID_REF += 1;
            }
            arg_node->ID_REF = id_node->ID_REF;
            break;
        }
        default:
            DBUG_ASSERT (0, "unknown info tag ");
            break;
        }
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
    node *id_node;

    DBUG_ENTER ("RClet");
    DBUG_PRINT ("RC", ("line: %d", arg_node->lineno));

    ids = arg_node->info.ids;
    while (NULL != ids) {
        if (1 == IsArray (ids->node)) {
            switch (arg_info->CONTEXT) {
            case BODY: {
                ids->refcnt = ids->node->refcnt;
                break;
            }
            case COND: {
                if (NULL == arg_info->node[0])
                    ids->refcnt = ids->node->refcnt;
                else {
                    id_node = LookupId (ids->id, arg_info->node[0]);
                    if (NULL == id_node)
                        ids->refcnt = 0;
                    else {
                        ids->refcnt = id_node->refcnt;
                        id_node->refcnt = 0;
                    }
                }
                break;
            }
            case WHILE:
            case DO: {
                if (NULL == arg_info->node[0])
                    ids->refcnt = 1;
                else {
                    id_node = LookupId (ids->id, arg_info->node[0]);
                    if (NULL == id_node)
                        ids->refcnt = 1;
                    else {
                        ids->refcnt = id_node->refcnt;
                        id_node->ID_REF = 0;
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

#if 0   
   if(BODY == arg_info->CONTEXT)
      MaxRefs(then_info->node[0], else_info->node[0], NULL);
   else
      MaxRefs(then_info->node[0], else_info->node[0], arg_info);
#endif

    /* store refcount information for use while compilation */
    then_info->node[1] = else_info->node[0];
    FREE (else_info);
    arg_node->node[3] = then_info;

    /* last but not least, traverse condition */
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    DBUG_RETURN (arg_node);
}
