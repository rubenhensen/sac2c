/*
 * $Log$
 * Revision 1.1  1995/06/02 10:06:56  sbs
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
 * During the (bottom up) traversal all information is kept at the N_vardec nodes.
 * This is realized by the introduction of a new node: "N_vinfo"
 * which is organized as follows:
 *     info.use (useflag): VECT / IDX
 *     node[0]           : next "N_vinfo node" if existent
 *     node[1]           : withbelonging shape if info.use == IDX
 *
 * there are two chaines of such nodes attached to the N_vardec nodes:
 * The "actual ones" at node[1] and the "collected ones" at node[2] which
 * are used later for the insertion of new variable delarations.
 *
 * When meeting a "N_let" with a vector to be defined, the actual VECT/IDX nodes
 * are inserted in the ids structure at ids->use;
 * subsequently copies of all nodes are appended to the "collected ones" chain
 * of the vardec node as long as there are no dublicates.
 *
 */

/*
 * Some functions for handling N_vinfo nodes:
 *   GenVect() 				: generates a new Vect-node
 *   GenIdx( node *ptr_to_shape)        : generates a new Idx-node
 *   CheckVect( node *N_vinfo_chain): checks whether VECT is in the chain
 *                                        of N_vinfo nodes
 *   CheckIdx( node *N_vinfo_chain, types *shape): dito for a specific shape
 *
 *   InsertInChain( node *N_vinfo_node, node *chain): prepands the node to the chain
 *
 *   SetVect( node *chain)	: inserts VECT-node in chain if not already present
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
    if (!CheckVect (chain))
        chain = InsertInChain (GenVect (), chain);
    DBUG_RETURN (chain);
}

/*
 *
 *  functionname  : PsiAp
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
PsiAp (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PsiId");
    Trav (arg_node->node[0], (node *)SetVect);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PsiId
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
PsiId (node *arg_node, node *arg_info)
{
    node *vardec;

    DBUG_ENTER ("PsiId");
    vardec = arg_node->info.ids->node;
    DBUG_ASSERT ((vardec->nodetype == N_vardec), "non vardec node as backref in N_id!");
    if (vardec->info.types->dim == 1)
        vardec->node[1] = ((chainmodfunptr)arg_info) (vardec->node[1]);
    DBUG_RETURN (arg_node);
}
