/*
 *
 * $Log$
 * Revision 1.63  1998/04/19 23:50:16  dkr
 * changed DupIds:
 *   arg_info now can be NULL
 *
 * Revision 1.62  1998/04/17 17:26:34  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.61  1998/04/17 16:22:59  dkr
 * NCODE_USED is now adjusted in MakeWLgrid...
 *
 * Revision 1.60  1998/04/17 11:41:52  srs
 * fixed bug in DupNwith() and added DupNcode()
 *
 * Revision 1.59  1998/04/16 16:08:03  srs
 * renamed INL_TYPES to INFO_INL_TYPES
 *
 * Revision 1.58  1998/04/16 11:47:08  dkr
 * fixed a bug with DupNode:
 *   arg_info now contains an ordinary N_info node
 *   the work of DupNode is now controlled by INFO_DUPCONT(arg_info)
 *
 * Revision 1.57  1998/04/13 19:02:15  dkr
 * support for wlcomp-pragmas added in DupPragma
 *
 * Revision 1.56  1998/04/11 15:19:28  srs
 * N_Nwith nodes are now processed by DupNwith() instead of DupChain()
 *
 * Revision 1.55  1998/04/09 21:21:34  dkr
 * renamed macros:
 *   INLINE -> DUP_INLINE
 *   NORMAL -> DUP_NORMAL
 *   INVARIANT -> DUP_INVARIANT
 *
 * Revision 1.54  1998/04/08 09:29:51  dkr
 * #ifdef changed to #if
 *
 * Revision 1.53  1998/04/08 07:39:33  srs
 * removed wrong assertion in DupId
 *
 * Revision 1.52  1998/04/07 15:19:19  srs
 * renamed DupId to DupModarray and
 *         DupIIds to DupId.
 * Added Initialization of ID_WL to DupId.
 * Removed NEWTREE
 *
 * Revision 1.51  1998/04/02 17:41:00  dkr
 * added DupConc
 *
 * Revision 1.50  1998/04/01 23:55:51  dkr
 * added DupWLstriVar, DupWLgridVar
 *
 * Revision 1.49  1998/03/30 23:42:43  dkr
 * added attribute LEVEL for N_WLgrid
 *
 * Revision 1.48  1998/03/29 23:28:24  dkr
 * added temp. attribute WLGRID_MODIFIED
 *
 * Revision 1.47  1998/03/27 18:38:46  dkr
 * WLproj renamed in WLstride:
 *   WLPROJ... -> WLSTRIDE...
 *   DupWLproj -> DupWLstride
 *
 * Revision 1.46  1998/03/26 14:04:39  dkr
 * changed usage of MakeWLgrid
 *
 * Revision 1.45  1998/03/26 12:04:09  dkr
 * removed a spelling mistake in DupWLproj
 *
 * Revision 1.44  1998/03/26 11:11:33  dkr
 * changed DupWLproj
 *
 * Revision 1.43  1998/03/24 21:44:44  dkr
 * changed DupWLproj:
 *   copies WLPROJ_PART
 *
 * Revision 1.42  1998/03/24 10:17:54  srs
 * changed DupNPart
 *
 * Revision 1.41  1998/03/22 15:47:24  dkr
 * N_WLblock: BLOCKING -> STEP
 *
 * Revision 1.40  1998/03/22 15:33:16  dkr
 * N_WLproj: OFFSET, WIDTH -> BOUND1, BOUND2
 *
 * Revision 1.39  1998/03/21 21:45:31  dkr
 * added macro DUPTRAV:
 *   DupNode now skips only the NEXT node of the root
 *
 * Revision 1.38  1998/03/21 17:35:08  dkr
 * new function DupNode added
 *
 * Revision 1.37  1998/03/20 20:50:42  dkr
 * changed usage of MakeWLseg
 *
 * Revision 1.36  1998/03/20 17:25:16  dkr
 * in N_WL... nodes: INNER is now called CONTENTS
 *
 * Revision 1.35  1998/03/19 20:17:57  dkr
 * removed a bug in DupWLgrid
 *
 * Revision 1.34  1998/03/19 19:29:52  dkr
 * in DupWLgrid and DupNPart NCODE_USED is now correctly set
 *
 * Revision 1.33  1998/03/19 19:07:02  dkr
 * fixed bugs in DupWL...
 *
 * Revision 1.32  1998/03/17 10:34:01  dkr
 * changed usage of MakeWLseg
 *
 * Revision 1.31  1998/03/16 00:33:29  dkr
 * added DupWLseg, DupWLblock, DupWLublock, DupWLproj, DupWLgrid
 *
 * Revision 1.29  1998/03/03 17:32:19  dkr
 * removed unused var 'i' in DupCond(), DupLoop()
 *
 * Revision 1.28  1998/03/02 22:27:10  dkr
 * removed bugs in duplication of N_cond, N_do, N_while
 *
 * Revision 1.27  1998/02/12 16:56:49  dkr
 * added support for new with-loop
 *
 * Revision 1.26  1997/11/10 23:19:12  dkr
 * removed a bug with NEWTREE
 *
 * Revision 1.25  1997/11/07 14:17:48  dkr
 * with defined NEWTREE node.nnode is not used anymore
 *
 * Revision 1.24  1997/09/05 17:47:41  dkr
 * added the function DupShpSeg
 *
 * Revision 1.22  1997/04/25  09:20:34  sbs
 * DBUG_ASSERT in DupIds adjusted (no varargs)
 *
 * Revision 1.21  1997/04/23  12:52:36  cg
 * decleration changed to declaration
 *
 * Revision 1.20  1996/09/06  16:19:37  cg
 * bug fixed in DupIIds: empty ids lists no longer cause segmentation faults.
 *
 * Revision 1.19  1996/02/21  15:07:02  cg
 * function DupFundef reimplemented. Internal information will now be copied as well.
 * added new function DupPragma
 *
 * Revision 1.18  1996/01/05  14:36:35  cg
 * added DupId for copying sons and info.id
 *
 * Revision 1.17  1995/12/20  08:19:06  cg
 * added new function DupChar to duplicate N_char nodes.
 *
 * Revision 1.16  1995/12/04  13:51:03  cg
 * now, attrib and status are copied by function DupIds
 *
 * Revision 1.15  1995/10/06  16:35:40  cg
 * calls to MakeIds adjusted to new signature (3 parameters)
 *
 * Revision 1.14  1995/08/16  09:23:01  asi
 * renamed DupCast to DupTypes
 *
 * Revision 1.13  1995/07/28  12:58:22  asi
 * added function DupInfo
 *
 * Revision 1.12  1995/07/24  09:08:05  asi
 * macro DUP moved from DupTree.c to DupTree.h, macro TYPES renamed to INL_TYPES
 *
 * Revision 1.11  1995/07/04  11:39:58  hw
 * DupDouble inserted
 *
 * Revision 1.10  1995/06/27  16:03:05  asi
 * added DUP-macro : Duplicating simple structure elements
 * and DUP inserted in each DUP... function
 *
 * Revision 1.9  1995/06/27  09:40:09  hw
 * bug fixed in DubIds( an ids-chain will be duplicated correctly now)
 *
 * Revision 1.8  1995/06/26  10:03:52  sbs
 * ids->use copie in DupIds
 *
 * Revision 1.7  1995/06/26  08:10:21  asi
 * now linenumbers will be duplicated
 * unused vaiable i warning removed
 *
 * Revision 1.6  1995/06/23  13:09:20  hw
 * - functions "DupDec" & "DupFundef" inserted
 * -  added argument to call of 'DuplicateTypes'
 *
 * Revision 1.5  1995/06/15  15:32:37  asi
 * DupTree generates arg_info if not present
 *
 * Revision 1.4  1995/06/08  09:55:13  asi
 * if arg_info->flag is set to INLINE variables will be renamed
 *
 * Revision 1.3  1995/06/02  11:25:48  asi
 * Added functions for all nodes below fundef node
 *
 * Revision 1.2  1995/05/03  12:41:51  asi
 * added DupPrf, DupAp and DupIds
 *
 * Revision 1.1  1995/05/01  15:32:27  asi
 * Initial revision
 *
 */

#include <string.h>

#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "my_debug.h"
#include "typecheck.h"
#include "internal_lib.h"

#include "DupTree.h"
#include "optimize.h"
#include "Inline.h"

/******************************************************************************/

/* LEVEL is only needed to distinguishe between th two occurences
   of N_return: inside the old WL and at the end of functions. */
#define LEVEL arg_info->lineno

/*
 * traverses son 'node'
 */
#define DUPTRAV(node) (node != NULL) ? Trav (node, arg_info) : NULL

/*
 * DupNode: INFO_DUPCONT(arg_info) contains the root of syntaxtree.
 *  -> traverses son 'node' if and only if its parent is not the root.
 * DupTree: INFO_DUPCONT(arg_info) is NULL.
 *  -> traverses son 'node'.
 */
#define DUPCONT(node) (INFO_DUPCONT (arg_info) != arg_node) ? DUPTRAV (node) : NULL

/******************************************************************************/

/*
 *  DupTree duplicates the whole sub tree behind the given pointer.
 *
 *  DupNode duplicates only the given node without next node.
 */

node *
DupTree (node *arg_node, node *arg_info)
{
    funptr *tmp_tab;
    node *new_node = NULL;

    DBUG_ENTER ("DupTree");

    if (NULL != arg_node) {
        tmp_tab = act_tab;
        act_tab = dup_tab;

        if (NULL == arg_info) {
            arg_info = MakeNode (N_info);
            /* dkr: where can we free this N_info node?? */
        }

        LEVEL = 0;
        /*
         * we want to duplicate all sons
         */
        INFO_DUPCONT (arg_info) = NULL;
        new_node = Trav (arg_node, arg_info);

        act_tab = tmp_tab;
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNode (node *arg_node)
{
    funptr *tmp_tab;
    node *arg_info;
    node *new_node = NULL;

    DBUG_ENTER ("DupTree");

    if (NULL != arg_node) {
        tmp_tab = act_tab;
        act_tab = dup_tab;

        arg_info = MakeNode (N_info);
        /*
         * duplicatation of sons of root 'arg_node' is controlled by
         *  DUPTRAV, DUPCONT
         */
        INFO_DUPCONT (arg_info) = arg_node;
        new_node = Trav (arg_node, arg_info);
        FREE (arg_info);

        act_tab = tmp_tab;
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupInt (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupInt");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.cint = arg_node->info.cint;
    DUP (arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupChar (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupChar");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeChar (CHAR_VAL (arg_node));
    DUP (arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupFloat (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupFloat");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.cfloat = arg_node->info.cfloat;
    DUP (arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupDouble (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupDouble");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.cdbl = arg_node->info.cdbl;
    DUP (arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupStr (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupStr");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.id = StringCopy (arg_node->info.id);
    DUP (arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

ids *
DupIds (ids *old_ids, node *arg_info)
{
    ids *new_ids;

    DBUG_ENTER ("DupIds");

    if ((arg_info != NULL) && (DUPTYPE == DUP_INLINE)) {
        new_ids = MakeIds (RenameInlinedVar (IDS_NAME (old_ids)), NULL, ST_regular);
        IDS_VARDEC (new_ids) = SearchDecl (IDS_NAME (new_ids), INFO_INL_TYPES (arg_info));
        DBUG_ASSERT ((NULL != IDS_VARDEC (new_ids)), "No declaration found for ids-node");
    } else {
        new_ids = MakeIds (StringCopy (IDS_NAME (old_ids)), NULL, ST_regular);
        IDS_VARDEC (new_ids) = IDS_VARDEC (old_ids);
        IDS_USE (new_ids) = IDS_USE (old_ids);
    }

    IDS_STATUS (new_ids) = IDS_STATUS (old_ids);
    IDS_ATTRIB (new_ids) = IDS_ATTRIB (old_ids);

    if (NULL != IDS_NEXT (old_ids)) {
        IDS_NEXT (new_ids) = DupIds (IDS_NEXT (old_ids), arg_info);
    }

    DBUG_RETURN (new_ids);
}

/******************************************************************************/

/* This function is used by N_id, N_let, N_pos, N_pre, N_generator. */
node *
DupId (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupId");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));

    new_node = MakeNode (arg_node->nodetype);
    new_node->info.ids
      = (arg_node->info.ids ? DupIds (arg_node->info.ids, arg_info) : NULL);
    DUP (arg_node, new_node);
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++) {
        if (arg_node->node[i]) {
            new_node->node[i] = Trav (arg_node->node[i], arg_info);
        }
    }

    if (N_id == NODE_TYPE (arg_node)) {
        ID_WL (new_node) = arg_node; /* Withloop folding (wlf) needs this. */
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupModarray (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupModarray");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.id = StringCopy (arg_node->info.id);
    DUP (arg_node, new_node);
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++) {
        if (arg_node->node[i] != NULL) {
            new_node->node[i] = Trav (arg_node->node[i], arg_info);
        }
    }
    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupCond (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupCond");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));

    LEVEL++;
    new_node = MakeCond (DUPTRAV (COND_COND (arg_node)), DUPTRAV (COND_THEN (arg_node)),
                         DUPTRAV (COND_ELSE (arg_node)));
    LEVEL--;

    DUP (arg_node, new_node);

    COND_VARINFO (new_node) = MakeInfo ();
    COND_THENVARS (arg_node) = DUPTRAV (COND_THENVARS (arg_node));
    COND_ELSEVARS (arg_node) = DUPTRAV (COND_ELSEVARS (arg_node));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupLoop (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupLoop");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));

    LEVEL++;
    new_node = MakeDo (DUPTRAV (DO_COND (arg_node)), DUPTRAV (DO_BODY (arg_node)));
    LEVEL--;
    NODE_TYPE (new_node) = NODE_TYPE (arg_node);

    DUP (arg_node, new_node);

    DO_VARINFO (new_node) = MakeInfo ();
    DO_USEVARS (new_node) = DUPTRAV (DO_USEVARS (arg_node));
    DO_DEFVARS (new_node) = DUPTRAV (DO_DEFVARS (arg_node));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupChain (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupChain");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    DUP (arg_node, new_node);
    LEVEL++;
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++)
        if (arg_node->node[i])
            new_node->node[i] = Trav (arg_node->node[i], arg_info);
    LEVEL--;

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupAssign (node *arg_node, node *arg_info)
{
    node *new_node = NULL;
    int i;

    DBUG_ENTER ("DupAssign");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    switch (DUPTYPE) {
    case DUP_INLINE:
        if (0 == LEVEL && N_return == NODE_TYPE (ASSIGN_INSTR (arg_node)))
            break;
    default:
        new_node = MakeNode (arg_node->nodetype);
        DUP (arg_node, new_node);
        for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++)
            if (arg_node->node[i])
                new_node->node[i] = Trav (arg_node->node[i], arg_info);
        break;
    }
    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupTypes (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupTypes");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.types = DuplicateTypes (arg_node->info.types, 1);
    DUP (arg_node, new_node);
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++) {
        if (arg_node->node[i] != NULL) {
            new_node->node[i] = Trav (arg_node->node[i], arg_info);
        }
    }
    DBUG_RETURN (new_node);
}

/******************************************************************************/

shpseg *
DupShpSeg (shpseg *shp_seg)
{
    int i;
    shpseg *new_shpseg;

    DBUG_ENTER ("DupShpSeg");
    new_shpseg = MakeShpseg (NULL);
    for (i = 0; i < SHP_SEG_SIZE; i++) {
        SHPSEG_SHAPE (new_shpseg, i) = SHPSEG_SHAPE (shp_seg, i);
    }
    DBUG_RETURN (new_shpseg);
}

/******************************************************************************/

node *
DupPrf (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupPrf");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.prf = arg_node->info.prf;
    DUP (arg_node, new_node);
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++) {
        if (arg_node->node[i] != NULL) {
            new_node->node[i] = Trav (arg_node->node[i], arg_info);
        }
    }
    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupFun (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupFun");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.fun_name.id = StringCopy (arg_node->info.fun_name.id);
    new_node->info.fun_name.id_mod = StringCopy (arg_node->info.fun_name.id_mod);
    DUP (arg_node, new_node);
    new_node->node[1] = arg_node->node[1];
    new_node->node[2] = arg_node->node[2];
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++) {
        if (arg_node->node[i] != NULL) {
            new_node->node[i] = Trav (arg_node->node[i], arg_info);
        }
    }
    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupFundef (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupFundef");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));

    new_node = MakeNode (arg_node->nodetype);

    new_node->info.types = DuplicateTypes (arg_node->info.types, 1);
    DUP (arg_node, new_node);

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (new_node) = DUPTRAV (FUNDEF_BODY (arg_node));
        FUNDEF_NEEDFUNS (new_node) = CopyNodelist (FUNDEF_NEEDFUNS (arg_node));
        FUNDEF_NEEDTYPES (new_node) = CopyNodelist (FUNDEF_NEEDTYPES (arg_node));
    }

    FUNDEF_ARGS (new_node) = DUPTRAV (FUNDEF_ARGS (arg_node));
    FUNDEF_PRAGMA (new_node) = DUPTRAV (FUNDEF_PRAGMA (arg_node));
    FUNDEF_NEEDOBJS (new_node) = CopyNodelist (FUNDEF_NEEDOBJS (arg_node));

    FUNDEF_NEXT (new_node) = DUPCONT (FUNDEF_NEXT (arg_node));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

/*
 * This function is used for N_vardec & N_arg nodes
 */
node *
DupDec (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupDec");
    DBUG_ASSERT (((N_vardec == arg_node->nodetype) || (N_arg == arg_node->nodetype)),
                 "wrong nodetype");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    DUP (arg_node, new_node);
    new_node->info.types = DuplicateTypes (arg_node->info.types, 1);
    if (NULL != arg_node->node[0]) {
        new_node->node[0] = Trav (arg_node->node[0], arg_info);
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupInfo (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupInfo");
#if 0 /* was ifndef NEWTREE */
  if (UNS_NO == arg_node->flag) { /* UNS_NO: arg_info->nnode */
    /* this condition is set by InvarUnswitch only !? */
    /* copy arg_info->node[0] to new_node (new arg_node), free old arg_node */
    new_node = DupTree(UNS_NODES, arg_info);  /* UNS_NODES: arg_info->node[0] */
    FreeTree(arg_node);
  }
  else {  /* make new node in new_node */
    new_node = MakeNode(N_info);
    new_node->flag = arg_node->flag;
  }
#endif

    DBUG_ASSERT ((1 == 0), "DupInfo called!");
    /* to workaround the dirty trick with nnode above ... */

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupPragma (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupPragma");

    new_node = MakePragma ();

    if (PRAGMA_LINKNAME (arg_node) != NULL) {
        PRAGMA_LINKNAME (new_node) = StringCopy (PRAGMA_LINKNAME (arg_node));
    }

    PRAGMA_NUMPARAMS (new_node) = PRAGMA_NUMPARAMS (arg_node);

    if (PRAGMA_LINKSIGN (arg_node) != NULL) {
        PRAGMA_LINKSIGN (new_node)
          = (int *)Malloc (PRAGMA_NUMPARAMS (new_node) * sizeof (int));

        for (i = 0; i < PRAGMA_NUMPARAMS (new_node); i++) {
            PRAGMA_LINKSIGN (new_node)[i] = PRAGMA_LINKSIGN (arg_node)[i];
        }
    }

    if (PRAGMA_REFCOUNTING (arg_node) != NULL) {
        PRAGMA_REFCOUNTING (new_node)
          = (int *)Malloc (PRAGMA_NUMPARAMS (new_node) * sizeof (int));

        for (i = 0; i < PRAGMA_NUMPARAMS (new_node); i++) {
            PRAGMA_REFCOUNTING (new_node)[i] = PRAGMA_REFCOUNTING (arg_node)[i];
        }
    }

    if (PRAGMA_WLCOMP_APS (arg_node) != NULL) {
        PRAGMA_WLCOMP_APS (arg_node) = DUPTRAV (PRAGMA_WLCOMP_APS (arg_node));
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupSPMD (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupSPMD");

    new_node = MakeSPMD (DUPTRAV (SPMD_REGION (arg_node)));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNwith (node *arg_node, node *arg_info)
{
    node *new_node, *partn, *coden, *withopn;

    DBUG_ENTER ("DupNwith");

    LEVEL++;
    /* very important: copy codes before parts because NCODE_COPY has to
       be set before the parts are traversed. */
    coden = Trav (NWITH_CODE (arg_node), arg_info);
    partn = Trav (NWITH_PART (arg_node), arg_info);
    withopn = Trav (NWITH_WITHOP (arg_node), arg_info);
    LEVEL--;

    new_node = MakeNWith (partn, coden, withopn);

    /* copy attributes */
    DUP (arg_node, new_node);
    NWITH_PARTS (new_node) = NWITH_PARTS (arg_node);
    NWITH_REFERENCED_FOLD (new_node) = NWITH_REFERENCED_FOLD (arg_node);
    NWITH_REFERENCED (new_node) = NWITH_REFERENCED (arg_node);
    NWITH_COMPLEX (new_node) = NWITH_COMPLEX (arg_node);
    NWITH_FOLDABLE (new_node) = NWITH_FOLDABLE (arg_node);
    NWITH_NO_CHANCE (new_node) = NWITH_NO_CHANCE (arg_node);

    NWITH_PRAGMA (new_node) = DUPTRAV (NWITH_PRAGMA (arg_node));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNwithop (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNwithop");

    new_node = MakeNWithOp (NWITHOP_TYPE (arg_node));
    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        NWITHOP_SHAPE (new_node) = DUPTRAV (NWITHOP_SHAPE (arg_node));
        break;
    case WO_modarray:
        NWITHOP_ARRAY (new_node) = DUPTRAV (NWITHOP_ARRAY (arg_node));
        break;
    case WO_foldfun:
        NWITHOP_NEUTRAL (new_node) = DUPTRAV (NWITHOP_NEUTRAL (arg_node));
        NWITHOP_FUN (new_node) = StringCopy (NWITHOP_FUN (arg_node));
        NWITHOP_MOD (new_node) = StringCopy (NWITHOP_MOD (arg_node));
        NWITHOP_FUNDEF (new_node) = DUPTRAV (NWITHOP_FUNDEF (arg_node));
        break;
    case WO_foldprf:
        NWITHOP_NEUTRAL (new_node) = DUPTRAV (NWITHOP_NEUTRAL (arg_node));
        NWITHOP_PRF (new_node) = NWITHOP_PRF (arg_node);
        break;
    default:
        DBUG_ASSERT (0, "Unknown N_Nwithop-type found");
    }
    NWITHOP_EXPR (new_node) = DUPTRAV (NWITHOP_EXPR (arg_node));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNpart (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNpart");
    DBUG_ASSERT (NPART_CODE (arg_node), ("N_Npart node has no valid NPART_CODE"));

    new_node
      = MakeNPart (DUPTRAV (NPART_WITHID (arg_node)), DUPTRAV (NPART_GEN (arg_node)),
                   NCODE_COPY (NPART_CODE (arg_node)));
    NPART_NEXT (new_node) = DUPCONT (NPART_NEXT (arg_node));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNcode (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupNcode");

    new_node = MakeNode (arg_node->nodetype);
    DUP (arg_node, new_node);
    LEVEL++;
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++)
        if (arg_node->node[i])
            new_node->node[i] = Trav (arg_node->node[i], arg_info);
    LEVEL--;

    NCODE_USED (new_node) = NCODE_USED (arg_node);
    NCODE_COPY (arg_node) = new_node;

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNwithid (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNwithid");

    new_node = MakeNWithid (DupIds (NWITHID_VEC (arg_node), arg_info),
                            DupIds (NWITHID_IDS (arg_node), arg_info));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNgen (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNgen");

    new_node = MakeNGenerator (DUPTRAV (NGEN_BOUND1 (arg_node)),
                               DUPTRAV (NGEN_BOUND2 (arg_node)), NGEN_OP1 (arg_node),
                               NGEN_OP2 (arg_node), DUPTRAV (NGEN_STEP (arg_node)),
                               DUPTRAV (NGEN_WIDTH (arg_node)));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLseg (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupWLseg");

    new_node = MakeWLseg (WLSEG_DIMS (arg_node), DUPTRAV (WLSEG_CONTENTS (arg_node)),
                          DUPCONT (WLSEG_NEXT (arg_node)));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLblock (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupWLblock");

    new_node = MakeWLblock (WLBLOCK_LEVEL (arg_node), WLBLOCK_DIM (arg_node),
                            WLBLOCK_BOUND1 (arg_node), WLBLOCK_BOUND2 (arg_node),
                            WLBLOCK_STEP (arg_node), DUPTRAV (WLBLOCK_NEXTDIM (arg_node)),
                            DUPTRAV (WLBLOCK_CONTENTS (arg_node)),
                            DUPCONT (WLBLOCK_NEXT (arg_node)));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLublock (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupWLublock");

    new_node
      = MakeWLublock (WLUBLOCK_LEVEL (arg_node), WLUBLOCK_DIM (arg_node),
                      WLUBLOCK_BOUND1 (arg_node), WLUBLOCK_BOUND2 (arg_node),
                      WLUBLOCK_STEP (arg_node), DUPTRAV (WLUBLOCK_NEXTDIM (arg_node)),
                      DUPTRAV (WLUBLOCK_CONTENTS (arg_node)),
                      DUPCONT (WLUBLOCK_NEXT (arg_node)));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLstride (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupWLstride");

    new_node = MakeWLstride (WLSTRIDE_LEVEL (arg_node), WLSTRIDE_DIM (arg_node),
                             WLSTRIDE_BOUND1 (arg_node), WLSTRIDE_BOUND2 (arg_node),
                             WLSTRIDE_STEP (arg_node), WLSTRIDE_UNROLLING (arg_node),
                             DUPTRAV (WLSTRIDE_CONTENTS (arg_node)),
                             DUPCONT (WLSTRIDE_NEXT (arg_node)));

    WLSTRIDE_PART (new_node) = WLSTRIDE_PART (arg_node);
    WLSTRIDE_MODIFIED (new_node) = 0;

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLgrid (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupWLgrid");

    new_node
      = MakeWLgrid (WLGRID_LEVEL (arg_node), WLGRID_DIM (arg_node),
                    WLGRID_BOUND1 (arg_node), WLGRID_BOUND2 (arg_node),
                    WLGRID_UNROLLING (arg_node), DUPTRAV (WLGRID_NEXTDIM (arg_node)),
                    DUPCONT (WLGRID_NEXT (arg_node)), WLGRID_CODE (arg_node));

    WLGRID_MODIFIED (new_node) = 0;

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLstriVar (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupWLstriVar");

    new_node
      = MakeWLstriVar (WLSTRIVAR_DIM (arg_node), DUPTRAV (WLSTRIVAR_BOUND1 (arg_node)),
                       DUPTRAV (WLSTRIVAR_BOUND2 (arg_node)),
                       DUPTRAV (WLSTRIVAR_STEP (arg_node)),
                       DUPTRAV (WLSTRIVAR_CONTENTS (arg_node)),
                       DUPCONT (WLSTRIVAR_NEXT (arg_node)));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLgridVar (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupWLgridVar");

    new_node
      = MakeWLgridVar (WLGRIDVAR_DIM (arg_node), DUPTRAV (WLGRIDVAR_BOUND1 (arg_node)),
                       DUPTRAV (WLGRIDVAR_BOUND2 (arg_node)),
                       DUPTRAV (WLGRIDVAR_NEXTDIM (arg_node)),
                       DUPCONT (WLGRIDVAR_NEXT (arg_node)), WLGRIDVAR_CODE (arg_node));

    DBUG_RETURN (new_node);
}
