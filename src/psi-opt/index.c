/*
 * $Log$
 * Revision 1.3  1995/06/06 15:18:10  sbs
 * first usable version ; does not include conditional stuff
 *
 * Revision 1.2  1995/06/02  17:05:47  sbs
 * IdxAssign, IdxLet inserted.
 *
 * Revision 1.1  1995/06/02  10:06:56  sbs
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include "dbug.h"
#include "tree.h"
#include "traverse.h"
#include "index.h"

/*
 * The "Uses" information is stored as follows:
 * During the (bottom up) traversal all information is kept at the N_vardec/N_arg nodes.
 * This is realized by the introduction of a new node: "N_vinfo"
 * which is organized as follows:
 *     info.use (useflag): VECT / IDX
 *     node[0]           : next "N_vinfo node" if existent
 *     node[1]           : withbelonging shape if info.use == IDX
 *
 * there are two chaines of such nodes attached to the N_vardec/N_arg nodes:
 * The "actual ones" at node[1] and the "collected ones" at node[2] which
 * are used later for the insertion of new variable delarations.
 *
 * When meeting a "N_let" with a vector to be defined, the actual VECT/IDX nodes
 * are inserted in the ids structure at ids->use;
 * subsequently copies of all nodes are appended to the "collected ones" chain
 * of the vardec/arg node as long as there are no dublicates.
 *
 */

/*
 * Some functions for handling N_vinfo nodes:
 *   GenVect() 				: generates a new Vect-node
 *   GenIdx( types *shape)              : generates a new Idx-node
 *   CheckVect( node *N_vinfo_chain)    : checks whether VECT is in the chain
 *                                        of N_vinfo nodes
 *   CheckIdx( node *N_vinfo_chain, types *shape): dito for a specific shape
 *
 *   InsertInChain( node *N_vinfo_node, node *chain): prepands the node to the chain
 *
 *   SetVect( node *chain, types *shape): inserts VECT-node in chain if not already
 * present
 *
 *   SetIdx( node *chain, types *shape)	: inserts IDX-node if not already present
 */

/*
 *
 *  functionname  : GenVect
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs : MakeNode
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
GenVect ()
{
    node *vnode;

    DBUG_ENTER ("GenVect");
    vnode = MakeNode (N_vinfo);
    vnode->info.use = VECT;
    vnode->nnode = 1;
    DBUG_RETURN (vnode);
}

/*
 *
 *  functionname  : GenIdx
 *  arguments     : 1) types * shape
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs : MakeNode
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
GenIdx (types *vshape)
{
    node *vnode;

    DBUG_ENTER ("GenIdx");
    vnode = MakeNode (N_vinfo);
    vnode->info.use = IDX;
    vnode->nnode = 1;
    vnode->node[1] = (node *)vshape;
    DBUG_RETURN (vnode);
}

/*
 *
 *  functionname  : CheckVect
 *  arguments     :
 *  description   : inserts a node in the given node-chain
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

int
CheckVect (node *chain)
{
    DBUG_ENTER ("CheckVect");
    while (chain && (chain->info.use != VECT))
        chain = chain->node[0];
    DBUG_RETURN ((chain != NULL));
}

/*
 *
 *  functionname  : EqTypes
 *  arguments     :
 *  description   : compares two types with respect to the shape
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

int
EqTypes (types *type1, types *type2)
{
    int i, res;

    DBUG_ENTER ("EqTypes");
    res = 0;
    if (type1->dim == type2->dim) {
        res = 1;
        for (i = 0; i < type2->dim; i++)
            if (type1->shpseg->shp[i] != type2->shpseg->shp[i])
                res = 0;
    }
    DBUG_RETURN (res);
}

/*
 *
 *  functionname  : CheckIdx
 *  arguments     :
 *  description   : inserts a node in the given node-chain
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

int
CheckIdx (node *chain, types *vshape)
{
    DBUG_ENTER ("CheckIdx");
    while (chain
           && ((chain->info.use != IDX) || !EqTypes ((types *)chain->node[1], vshape)))
        chain = chain->node[0];
    DBUG_RETURN ((chain != NULL));
}

/*
 *
 *  functionname  : InsertInChain
 *  arguments     :
 *  description   : inserts a node in the given node-chain
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
InsertInChain (node *elem, node *chain)
{
    DBUG_ENTER ("InsertInChain");
    if (chain)
        elem->node[0] = chain;
    else
        elem->node[0] = NULL;
    DBUG_RETURN (elem);
}

/*
 *
 *  functionname  : SetVect
 *  arguments     :
 *  description   : inserts a Vect node in the given node-chain if there exists none
 *  global vars   :
 *  internal funs : CheckVect, GenVect, InsertInChain
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
SetVect (node *chain)
{
    DBUG_ENTER ("SetVect");
    DBUG_PRINT ("IDX", ("VECT assigned"));
    if (!CheckVect (chain))
        chain = InsertInChain (GenVect (), chain);
    DBUG_RETURN (chain);
}

/*
 *
 *  functionname  : SetIdx
 *  arguments     :
 *  description   : inserts a Vect node in the given node-chain if there exists none
 *  global vars   :
 *  internal funs : CheckVect, GenVect, InsertInChain
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
SetIdx (node *chain, types *vartype)
{
    DBUG_ENTER ("SetIdx");
    if (!CheckIdx (chain, vartype))
        DBUG_PRINT ("IDX", ("IDX() assigned"));
    chain = InsertInChain (GenIdx (vartype), chain);
    DBUG_RETURN (chain);
}

/*
 *
 *  functionname  : CpyIdx
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs : CheckVect, GenVect, InsertInChain
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
CpyIdx (node *from, node *to)
{
    DBUG_ENTER ("CpyIdx");
    while (from) {
        if (from->info.use == IDX)
            to = SetIdx (to, (types *)from->node[1]);
        from = from->node[0];
    }
    DBUG_PRINT ("IDX", ("IDX() assigned"));
    DBUG_RETURN (to);
}

/*
 *
 *  functionname  : IdxModul
 *  arguments     :
 *  description   : Quasi-dummy
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxModul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxModul");
    if (NULL != arg_node->node[2])
        arg_node->node[2] = Trav (arg_node->node[2], NULL);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxAssign
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
IdxAssign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxAssign");
    /* Bottom up traversal!! */
    if (arg_node->node[1])
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxLet
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
IdxLet (node *arg_node, node *arg_info)
{
    ids *vars;
    node *vardec, *vinfo;

    DBUG_ENTER ("IdxLet");
    /* first, we move the actual chain to the respective variabledef
       and supply copies to the vardec-collected chain! */
    vars = arg_node->info.ids;
    do {
        vardec = vars->node;
        vars->use = vardec->node[1];
        vardec->node[1] = NULL; /* freeing the actual chain! */
        vinfo = vars->use;
        while (vinfo) {
            if (vinfo->info.use == VECT)
                vardec->node[2] = SetVect (vardec->node[2]);
            else
                vardec->node[2] = SetIdx (vardec->node[2], (types *)vinfo->node[1]);
            vinfo = vinfo->node[0];
        }
        vars = vars->next;
    } while (vars);
    /* then, we traverse the body of the let */
    arg_node->node[0] = Trav (arg_node->node[0], arg_node->info.ids->use);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxPrf
 *  arguments     :
 *  description   : case prf of:
 *                    psi   : SetIdx
 *                    binop : SetVect
 *                    others: SetVect
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxPrf (node *arg_node, node *arg_info)
{
    node *arg1, *arg2;
    types *shp;

    DBUG_ENTER ("IdxPrf");
    switch (arg_node->info.prf) {
    case F_psi:
        arg1 = arg_node->node[0]->node[0];
        arg2 = arg_node->node[0]->node[1]->node[0];
        if (arg1->nodetype == N_id) {
            if (arg2->nodetype == N_id)
                shp = arg2->info.ids->node->info.types;
            else
                DBUG_ASSERT ((NULL), "not yet implemented constant arg in psi!");
            arg1->info.ids->node->node[1] = SetIdx (arg1->info.ids->node->node[1], shp);
        }
        Trav (arg2, arg_info);
        break;
    case F_add_AxA:
    case F_sub_AxA:
    case F_mul_AxA:
    case F_div_AxA:
        arg1 = arg_node->node[0]->node[0];
        arg2 = arg_node->node[0]->node[1]->node[0];
        if (arg1->nodetype == N_id)
            arg1->info.ids->node->node[1]
              = CpyIdx (arg_info, arg1->info.ids->node->node[1]);
        if (arg2->nodetype == N_id)
            arg2->info.ids->node->node[1]
              = CpyIdx (arg_info, arg2->info.ids->node->node[1]);
        break;
    case F_add_SxA:
    case F_sub_SxA:
    case F_mul_SxA:
    case F_div_SxA:
        arg2 = arg_node->node[0]->node[1]->node[0];
        if (arg2->nodetype == N_id)
            arg2->info.ids->node->node[1]
              = CpyIdx (arg_info, arg2->info.ids->node->node[1]);
        break;
    case F_add_AxS:
    case F_sub_AxS:
    case F_mul_AxS:
    case F_div_AxS:
        arg1 = arg_node->node[0]->node[0];
        if (arg1->nodetype == N_id)
            arg1->info.ids->node->node[1]
              = CpyIdx (arg_info, arg1->info.ids->node->node[1]);
        break;
    default:
        Trav (arg_node->node[0], arg_info);
        break;
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxId
 *  arguments     :
 *  description   : examines whether variable is a one-dimensional array;
 *                  if so, SetVect to the "N_vardec" node belonging to the "N_id" node.
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxId (node *arg_node, node *arg_info)
{
    node *vardec;

    DBUG_ENTER ("IdxId");
    vardec = arg_node->info.ids->node;
    DBUG_ASSERT (((vardec->nodetype == N_vardec) || (vardec->nodetype == N_arg)),
                 "non vardec node as backref in N_id!");
    if (vardec->info.types->dim == 1) {
        DBUG_PRINT ("IDX", ("assigning to %s:", arg_node->info.ids->id));
        vardec->node[1] = SetVect (vardec->node[1]);
    }
    DBUG_RETURN (arg_node);
}
