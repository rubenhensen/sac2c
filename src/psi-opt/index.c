/*
 * $Log$
 * Revision 1.2  1995/06/02 17:05:47  sbs
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
SetVect (node *chain, types *vartype)
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
        arg_node->node[2] = Trav (arg_node->node[2], (node *)SetVect);
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
    while (vars->next) {
        vardec = vars->node;
        vars->use = vardec->node[1];
        vinfo = vars->use;
        while (vinfo)
            if (vinfo->info.use == VECT)
                vardec->node[2] = SetVect (vardec->node[2], NULL);
            else
                vardec->node[2] = SetIdx (vardec->node[2], (types *)vinfo->node[1]);
        vars = vars->next;
    }
    /* then, we traverse the body of the let */
    arg_node->node[0] = Trav (arg_node->node[0], (node *)SetVect);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxAp
 *  arguments     :
 *  description   : Since this is a user defined function all uses of "vector"-variables
 *                  do per definitionem need them as vectors. Hence, "SetVect" is
 *                  given as modification function.
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxAp (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxAp");
    Trav (arg_node->node[0], (node *)SetVect);
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
    DBUG_ENTER ("IdxAp");
    switch (arg_node->info.prf) {
    case F_psi:
        Trav (arg_node->node[0]->node[0], (node *)SetIdx);
        Trav (arg_node->node[0]->node[1], (node *)SetVect);
        break;
    case F_add:
    case F_sub:
    case F_mul:
    case F_div:
        Trav (arg_node->node[0], (node *)SetVect);
        break;
    default:
        Trav (arg_node->node[0], (node *)SetVect);
        break;
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxId
 *  arguments     :
 *  description   : examines whether variable is a one-dimensional array;
 *                  if so, it applies the given modification function (arg_info)
 *                  to the "N_vardec" node belonging to the "N_id" node.
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
        vardec->node[1]
          = ((chainmodfunptr)arg_info) (vardec->node[1], vardec->info.types);
    }
    DBUG_RETURN (arg_node);
}
