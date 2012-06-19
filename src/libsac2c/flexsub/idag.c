/*
 * $Id$
 */

/*
 *  Overview:
 *  ---------
 *
 *  This file implements support for infinite graphs. The overall
 *  idea is as follows: We conceptually create an infinite graph
 *  but materialise only finite parts of it whenever needed.
 *  This need has to be externally triggered by requesting
 *  the materialisation of individual vertices.
 *  The key to this materialisation is the idea that the vertices
 *  of an infinite graph can be partitioned into parameterised
 *  sets of vertices with bijections between the parameters and
 *  the elements in the individual sets. That way, we can describe
 *  the entire graph without immediately materialising it:
 *  In case of finite vertex sets, these can be materialised
 *  straight away by using IDAGaddVertex.
 *  In case of inifinite sets, these have to be materisalised
 *  on demand. Here the idea is to add a description of the set,
 *  referred to as "vertex family". Each vertex family has a
 *  comparison function CMPFUN attached to it which allows the IDAG
 *  package to identify whether a given vertex has been materialised
 *  or not by simply comparing the corresponding parameters.
 *
 *  Similarly, we introduce the notion of "edge families" to describe
 *  infinitly many edges. We allow edge families to be defined
 *  between vertex families. However, we typically do not want to have
 *  edges between all virtexes from one vertex family to all vertexes
 *  from another vertex family. Therefore, we introduce a CHECKFUN
 *  which, when presented with the parameters of two vertices from
 *  two vertex families, returns TRUE or FALSE to indicate the
 *  existance of an edge.
 *
 *  Implementation:
 *  ---------------
 *
 *  The overall idea is to utilise a finite dag to represent
 *  all materialised parts of the infinite graph.
 *  This allows us to map most operations directly into
 *  the corresponding ones of the finite graphs.
 *
 *  In addition to that graph we keep the information  for
 *  vertex families and edge families so that we can materialise
 *  further parts of the graph whenever needed.
 *  For that purpose we maintain two lists, a list of vertex
 *  families (VFAMS) and a list of edge families (EFAMS).
 *  We draw from these and their relation whenever we materialise
 *  vertices.
 *
 *  NB: we actually cater for the fact that you can add
 *  edge families *after* materialising vertices already. In that case
 *  we check all potential vertex pairs and create the edges needed!
 *
 *  We implement all data structures as node * from the AST.
 *  In essence, we introduce 5 new node types:
 *
 *  N_idag: holds all info together. Its main component is a DAG:
 *
 *     IDAG_DAG:
 *          which holds those vertices and edges that have already
 *          been materialised. Furthermore, we have
 *     IDAG_VFAMS:
 *          which holds a chain of vertex family entries, and
 *     IDAG_EFAMS:
 *          which holds a chain of edge family entries.
 *
 * N_idagvfam: represents the head entry of a vertex family chain.
 *             Each entry carries
 *     IDAGVFAM_CMPFUN:
 *             a comparison function for checking whether a given
 *             vertex-parameter has been materialised yet
 *     IDAGVFAM_VERTICES:
 *             a chain of links to all family members that have
 *             already been materialised
 *     IDAGVFAM_FROMS:
 *             a chain of links to edge family entries that point
 *             away from this vertex family
 *     IDAGVFAM_TOS:
 *             a chain of links to edge family entries that point
 *             towards this vertex family
 *     NEXT:
 *             points to next vertex family entry.
 *
 * N_idagefam: represents the head entry of a edge family chain.
 *             Each entry carries
 *
 *     IDAGEFAM_FROM:
 *             vertex or vertex family this edge family connects from
 *     IDAGEFAM_TO:
 *             vertex or vertex family this edge family connects to
 *     IDAGEFAM_CHECKFUN:
 *             a function that computes from two sets of parameters
 *             (one from the from family and one from the to family)
 *             whether an edge should exist
 *     NEXT:
 *             points to next edge family entry.
 *
 * N_idagvertices: a simple chain of links to vertices.
 *     IDAGVERTICES_VERTEX: a link to a vertex
 *     IDAGVERTICES_NEXT: next chain entry
 *
 * N_idagefams: a simple chain of links to edge family entries.
 *     IDAGEFAMS_EFAM: a link to an edge family entry
 *     IDAGEFAMS_NEXT: next chain entry
 *
 * Whenever we insert a vertex family, we simply stick it into the list
 * of vertex family entries. We materialise vertices only upon registry.
 * The parameter (void *) serves as unique tag for family members. A comparison
 * function provided at creation of vertex families allows us to figure out
 * the existance of entries. Currently, we use a simply linked list for insertion;
 * we may have to change this into hashtables in case performance is not satisfactory.
 *
 * Edges are being added in two ways: Upon registration of edge families, we
 * materialise all those edges that concern *materialised* vertices.
 * Whenever we materialise a new vertex family member, we materialise
 * all potential incoming and outgoing edges from this new vertex!
 * This ensures completeness at all time. However, it requires us to
 * be able to find all edge families that relate to a given vertex family.
 * This is the reason why we have the FROMS and TOS chains at each vertex
 * family entry.
 *
 */

#include "dag.h"
#include "idag.h"

#define DBUG_PREFIX "IDAG"
#include "debug.h"

#include "tree_basic.h"

idag *
IDAGgenInfiniteGraph ()
{
    DBUG_ENTER ();
    DBUG_RETURN (TBmakeIdag (DAGgenGraph (), NULL, NULL));
}

ivertex *
IDAGaddVertex (idag *g, void *annotation)
{
    DBUG_ENTER ();
    DBUG_RETURN (DAGaddVertex (IDAG_DAG (g), annotation));
}

void *
IDAGgetVertexAnnotation (idag *g, ivertex *from)
{
    DBUG_ENTER ();
    DBUG_RETURN (DAGgetVertexAnnotation (IDAG_DAG (g), from));
}

iedge *
IDAGaddEdge (idag *g, ivertex *from, ivertex *to)
{
    DBUG_ENTER ();
    DBUG_RETURN (DAGaddEdge (IDAG_DAG (g), from, to));
}

ivertex *
IDAGgetLub (idag *g, ivertex *v1, ivertex *v2)
{
    DBUG_ENTER ();
    DBUG_RETURN (DAGgetLub (IDAG_DAG (g), v1, v2));
}

ivertex_fam *
IDAGaddVertexFamily (idag *g, idag_fun_t cmpfun)
{
    node *vfam;

    DBUG_ENTER ();

    vfam = TBmakeIdagvfam (cmpfun, IDAG_VFAMS (g));
    IDAG_VFAMS (g) = vfam;

    DBUG_RETURN (vfam);
}

static void
AddEdgesFrom (dag *g, vertex *from, node *from_efams)
{
    node *to_vs;
    vertex *to;
    idag_fun_t checkfun;
    edge *e;

    DBUG_ENTER ();

    while (from_efams != NULL) {
        to_vs = IDAGVFAM_VERTICES (IDAGEFAM_TO (IDAGEFAMS_EFAM (from_efams)));
        checkfun = IDAGEFAM_CHECKFUN (IDAGEFAMS_EFAM (from_efams));
        while (to_vs != NULL) {
            to = IDAGVERTICES_VERTEX (to_vs);
            if (checkfun (DAGgetVertexAnnotation (g, from),
                          DAGgetVertexAnnotation (g, to))) {
                e = DAGaddEdge (g, from, to);
            }
            to_vs = IDAGVERTICES_NEXT (to_vs);
        }

        from_efams = IDAGEFAMS_NEXT (from_efams);
    }
    DBUG_RETURN ();
}

static void
AddEdgesTo (dag *g, node *to_efams, vertex *to)
{
    node *from_vs;
    vertex *from;
    idag_fun_t checkfun;
    edge *e;

    DBUG_ENTER ();

    while (to_efams != NULL) {
        from_vs = IDAGVFAM_VERTICES (IDAGEFAM_FROM (IDAGEFAMS_EFAM (to_efams)));
        checkfun = IDAGEFAM_CHECKFUN (IDAGEFAMS_EFAM (to_efams));
        while (from_vs != NULL) {
            from = IDAGVERTICES_VERTEX (from_vs);
            if (checkfun (DAGgetVertexAnnotation (g, from),
                          DAGgetVertexAnnotation (g, to))) {
                e = DAGaddEdge (g, from, to);
            }
            from_vs = IDAGVERTICES_NEXT (from_vs);
        }

        to_efams = IDAGEFAMS_NEXT (to_efams);
    }
    DBUG_RETURN ();
}

ivertex *
IDAGregisterVertexFamilyMember (idag *g, ivertex_fam *vfam, void *params)
{
    idag_fun_t cmpfun;
    node *vs;
    ivertex *res;

    DBUG_ENTER ();
    cmpfun = IDAGVFAM_CMPFUN (vfam);
    vs = IDAGVFAM_VERTICES (vfam);
    while ((vs != NULL)
           && !cmpfun (params,
                       DAGgetVertexAnnotation (IDAG_DAG (g), IDAGVERTICES_VERTEX (vs)))) {
        vs = IDAGVERTICES_NEXT (vs);
    }
    if (vs != NULL) {
        res = IDAGVERTICES_VERTEX (vs);
    } else {
        res = DAGaddVertex (IDAG_DAG (g), params);
        AddEdgesFrom (IDAG_DAG (g), res, IDAGVFAM_FROMS (vfam));
        AddEdgesTo (IDAG_DAG (g), IDAGVFAM_TOS (vfam), res);
    }
    DBUG_RETURN (res);
}

static void
AddEdgesBetween (dag *g, node *froms, node *tos, idag_fun_t checkfun)
{
    node *to_vs;
    vertex *from, *to;
    edge *e;

    DBUG_ENTER ();

    while (froms != NULL) {
        from = IDAGVERTICES_VERTEX (froms);
        to_vs = tos;
        while (to_vs != NULL) {
            to = IDAGVERTICES_VERTEX (to_vs);
            if (checkfun (DAGgetVertexAnnotation (g, from),
                          DAGgetVertexAnnotation (g, to))) {
                e = DAGaddEdge (g, from, to);
            }
            to_vs = IDAGVERTICES_NEXT (to_vs);
        }

        froms = IDAGVERTICES_NEXT (froms);
    }
    DBUG_RETURN ();
}

iedge_fam *
IDAGaddEdgeFamily (idag *g, ivertex_fam *from, ivertex_fam *to, idag_fun_t checkfun)
{
    node *efam;
    DBUG_ENTER ();

    /* insert edge family in idag */
    efam = TBmakeIdagefam (checkfun, from, to, IDAG_EFAMS (g));

    /* register edge family in vertex families */
    IDAGVFAM_FROMS (from) = TBmakeIdagefams (efam, IDAGVFAM_FROMS (from));
    IDAGVFAM_TOS (to) = TBmakeIdagefams (efam, IDAGVFAM_TOS (to));

    /* create edges for all materialised vertices between the 'from' and the 'to' family:
     */
    AddEdgesBetween (IDAG_DAG (g), IDAGVFAM_VERTICES (from), IDAGVFAM_VERTICES (to),
                     checkfun);

    DBUG_RETURN (efam);
}
