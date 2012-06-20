
/** <!--********************************************************************-->
 *
 * @file dag.c
 *
 * prefix: DAG
 *
 * description:
 *
 *****************************************************************************/

#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "dfwalk.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "memory.h"
#include "tree_compound.h"
#include "types.h"
#include "dynelem.h"
#include "elemstack.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "graphtypes.h"
#include "dfwalk.h"
#include "tfprintutils.h"
#include "lubtree.h"
#include "lubcross.h"
#include "binheap.h"
#include "query.h"

static void
addEdge (node *super, node *sub)
{

    DBUG_ENTER ();

    node *itersuper, *itersub;

    itersuper = TFVERTEX_CHILDREN (super);

    if (itersuper == NULL) {
        TFVERTEX_CHILDREN (super) = TBmakeTfedge (sub, NULL);
    } else {
        while (TFEDGE_NEXT (itersuper) != NULL) {
            itersuper = TFEDGE_NEXT (itersuper);
        }
        TFEDGE_NEXT (itersuper) = TBmakeTfedge (sub, NULL);
    }

    itersub = TFVERTEX_PARENTS (sub);

    if (itersub == NULL) {
        TFVERTEX_PARENTS (sub) = TBmakeTfedge (super, NULL);
    } else {
        while (TFEDGE_NEXT (itersub) != NULL) {
            itersub = TFEDGE_NEXT (itersub);
        }
        TFEDGE_NEXT (itersub) = TBmakeTfedge (super, NULL);
    }

    /*
     * Update the number of parents that the subtype has. This information is
     * necessary while topologically sorting the subtyping hierarchy.
     */

    TFVERTEX_NUMPARENTS (sub)++;

    DBUG_RETURN ();
}

static vertex *
vlookup (dag *g, vertex *v)
{
    DBUG_ENTER ();
    vertex *res = NULL;
    node *vs = TFDAG_DEFS (g->gnode);
    while (vs != NULL) {
        if (v->vnode == vs) {
            res = v;
            break;
        }
        vs = TFVERTEX_NEXT (vs);
    }
    DBUG_RETURN (res);
}

dag *
DAGgenGraph ()
{
    DBUG_ENTER ();
    dag *g = MEMmalloc (sizeof (dag));
    g->gnode = TBmakeTfdag (NULL);
    DBUG_RETURN (g);
}

vertex *
DAGaddVertex (dag *g, void *annotation)
{
    DBUG_ENTER ();
    vertex *v = MEMmalloc (sizeof (vertex));
    v->vnode = TBmakeTfvertex (NULL, NULL, TFDAG_DEFS (g->gnode));
    TFVERTEX_WRAPPERLINK (v->vnode) = v;
    TFDAG_DEFS (g->gnode) = v->vnode;
    v->annotation = annotation;
    DBUG_RETURN (v);
}

void *
DAGgetAnnotation (dag *g, vertex *from)
{
    DBUG_ENTER ();
    void *res = NULL;
    vertex *v = vlookup (g, from);
    if (v != NULL)
        res = v->annotation;
    else
        CTIerror ("Vertex non-existant in graph");
    DBUG_RETURN (res);
}

void
DAGaddEdge (dag *g, vertex *from, vertex *to)
{
    DBUG_ENTER ();
    vertex *src = vlookup (g, from), *tar = vlookup (g, to);
    if (src == NULL || tar == NULL) {
        CTIerror ("Source or target vertex non-existant in the graph");
    } else {
        addEdge (from->vnode, to->vnode);
    }
    DBUG_RETURN ();
}

vertex *
DAGgetLub (dag *g, vertex *from, vertex *to)
{
    DBUG_ENTER ();
    vertex *res = NULL;
    node *n = GINlcaFromNodes (from->vnode, to->vnode, TFDAG_INFO (g->gnode));
    res = TFVERTEX_WRAPPERLINK (n);
    DBUG_RETURN (res);
}
