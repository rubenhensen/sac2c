/*
 *
 * $Log$
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
 * Revision 1.23  1997/04/30 11:50:21  cg
 * *** empty log message ***
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

#define LEVEL arg_info->lineno

node *
DupTree (node *arg_node, node *arg_info)
{
    node *new_node = NULL;
    funptr *tmp_tab;

    DBUG_ENTER ("DupTree");

    if (NULL != arg_node) {
        tmp_tab = act_tab;
        act_tab = dup_tab;

        if (NULL == arg_info)
            arg_info = MakeNode (N_info);

        LEVEL = 0;
        new_node = Trav (arg_node, arg_info);

        act_tab = tmp_tab;
    }

    DBUG_RETURN (new_node);
}

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

ids *
DupIds (ids *old_ids, node *arg_info)
{
    ids *new_ids;

    DBUG_ENTER ("DupIds");
    switch (DUPTYPE) {
    case INLINE:
        new_ids = MakeIds (RenameInlinedVar (old_ids->id), NULL, ST_regular);
        new_ids->node = SearchDecl (new_ids->id, INL_TYPES);
        DBUG_ASSERT ((NULL != new_ids->node), "No declaration found for ids-node");
        break;
    default:
        new_ids = MakeIds (StringCopy (old_ids->id), NULL, ST_regular);
        new_ids->node = old_ids->node;
        new_ids->use = old_ids->use;
        break;
    }

    IDS_STATUS (new_ids) = IDS_STATUS (old_ids);
    IDS_ATTRIB (new_ids) = IDS_ATTRIB (old_ids);

    if (NULL != old_ids->next)
        new_ids->next = DupIds (old_ids->next, arg_info);
    DBUG_RETURN (new_ids);
}

/*
 * DupIIds is used for N_id node.
 */

node *
DupIIds (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupIIds");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.ids
      = ((arg_node->info.ids == NULL) ? NULL : DupIds (arg_node->info.ids, arg_info));
    DUP (arg_node, new_node);
#ifndef NEWTREE
    for (i = 0; i < arg_node->nnode; i++)
#else  /* NEWTREE */
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++)
        if (arg_node->node[i] != NULL)
#endif /* NEWTREE */
    {
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
    }
    DBUG_RETURN (new_node);
}

/*
 * DupId is used for N_modarray node.
 */

node *
DupId (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupId");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.id = StringCopy (arg_node->info.id);
    DUP (arg_node, new_node);
#ifndef NEWTREE
    for (i = 0; i < arg_node->nnode; i++)
#else  /* NEWTREE */
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++)
        if (arg_node->node[i] != NULL)
#endif /* NEWTREE */
    {
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
    }
    DBUG_RETURN (new_node);
}

node *
DupCond (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupCond");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));

    LEVEL++;
    new_node = MakeCond (Trav (COND_COND (arg_node), arg_info),
                         Trav (COND_THEN (arg_node), arg_info),
                         Trav (COND_ELSE (arg_node), arg_info));
    LEVEL--;

    DUP (arg_node, new_node);

    COND_VARINFO (new_node) = MakeInfo ();
    if (COND_THENVARS (arg_node) != NULL) {
        COND_THENVARS (new_node) = Trav (COND_THENVARS (arg_node), arg_info);
    }
    if (COND_ELSEVARS (arg_node) != NULL) {
        COND_ELSEVARS (new_node) = Trav (COND_ELSEVARS (arg_node), arg_info);
    }

    DBUG_RETURN (new_node);
}

node *
DupLoop (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupLoop");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));

    LEVEL++;
    new_node
      = MakeDo (Trav (DO_COND (arg_node), arg_info), Trav (DO_BODY (arg_node), arg_info));
    LEVEL--;
    NODE_TYPE (new_node) = NODE_TYPE (arg_node);

    DUP (arg_node, new_node);

    DO_VARINFO (new_node) = MakeInfo ();
    if (DO_USEVARS (arg_node) != NULL) {
        DO_USEVARS (new_node) = Trav (DO_USEVARS (arg_node), arg_info);
    }
    if (DO_DEFVARS (arg_node) != NULL) {
        DO_DEFVARS (new_node) = Trav (DO_DEFVARS (arg_node), arg_info);
    }

    DBUG_RETURN (new_node);
}

node *
DupChain (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupChain");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    DUP (arg_node, new_node);
#ifndef NEWTREE
    for (i = 0; i < arg_node->nnode; i++)
#else  /* NEWTREE */
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++)
        if (arg_node->node[i] != NULL)
#endif /* NEWTREE */
    {
        LEVEL++;
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
        LEVEL--;
    }
    DBUG_RETURN (new_node);
}

node *
DupAssign (node *arg_node, node *arg_info)
{
    node *new_node = NULL;
    int i;

    DBUG_ENTER ("DupAssign");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    switch (DUPTYPE) {
    case INLINE:
        if ((0 == LEVEL) && (N_return == arg_node->node[0]->nodetype))
            break;
    default:
        new_node = MakeNode (arg_node->nodetype);
        DUP (arg_node, new_node);
#ifndef NEWTREE
        for (i = 0; i < arg_node->nnode; i++)
#else  /* NEWTREE */
        for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++)
            if (arg_node->node[i] != NULL)
#endif /* NEWTREE */
        {
            new_node->node[i] = Trav (arg_node->node[i], arg_info);
#ifndef NEWTREE
            if (NULL == new_node->node[i])
                new_node->nnode = i;
#endif /* NEWTREE */
        }
        break;
    }
    DBUG_RETURN (new_node);
}

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
#ifndef NEWTREE
    for (i = 0; i < arg_node->nnode; i++)
#else  /* NEWTREE */
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++)
        if (arg_node->node[i] != NULL)
#endif /* NEWTREE */
    {
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
    }
    DBUG_RETURN (new_node);
}

shpseg *
DupShpSeg (shpseg *shp_seg)
{
    int i;
    shpseg *new_shpseg;

    DBUG_ENTER ("DupShpSeg");
    new_shpseg = MakeShpseg (NULL);
    for (i = 0; i < SHP_SEG_SIZE; i++)
        SHPSEG_SHAPE (new_shpseg, i) = SHPSEG_SHAPE (shp_seg, i);
    DBUG_RETURN (new_shpseg);
}

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
#ifndef NEWTREE
    for (i = 0; i < arg_node->nnode; i++)
#else  /* NEWTREE */
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++)
        if (arg_node->node[i] != NULL)
#endif /* NEWTREE */
    {
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
    }
    DBUG_RETURN (new_node);
}

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
#ifndef NEWTREE
    for (i = 0; i < arg_node->nnode; i++)
#else  /* NEWTREE */
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++)
        if (arg_node->node[i] != NULL)
#endif /* NEWTREE */
    {
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
    }
    DBUG_RETURN (new_node);
}

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
        FUNDEF_BODY (new_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_NEEDFUNS (new_node) = CopyNodelist (FUNDEF_NEEDFUNS (arg_node));
        FUNDEF_NEEDTYPES (new_node) = CopyNodelist (FUNDEF_NEEDTYPES (arg_node));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (new_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (new_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_PRAGMA (arg_node) != NULL) {
        FUNDEF_PRAGMA (new_node) = Trav (FUNDEF_PRAGMA (arg_node), arg_info);
    }

    FUNDEF_NEEDOBJS (new_node) = CopyNodelist (FUNDEF_NEEDOBJS (arg_node));

    DBUG_RETURN (new_node);
}

/*
 * This function is used for N_vardec & N_arg nodes
 *
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
#ifndef NEWTREE
        new_node->nnode = 1;
#endif /* NEWTREE */
    }

    DBUG_RETURN (new_node);
}

node *
DupInfo (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupInfo");
#ifndef NEWTREE
    if (UNS_NO == arg_node->flag) /* UNS_NO: arg_info->nnode */
                                  /* this condition is set by InvarUnswitch only !? */
    { /* copy arg_info->node[0] to new_node (new arg_node), free old arg_node */
        new_node = DupTree (UNS_NODES, arg_info); /* UNS_NODES: arg_info->node[0] */
        FreeTree (arg_node);
    } else { /* make new node in new_node */
        new_node = MakeNode (N_info);
        new_node->flag = arg_node->flag;
    }
#else
    DBUG_ASSERT ((1 == 0), "DupInfo called!");
    /* to workaround the dirty trick with nnode above ... */
#endif

    DBUG_RETURN (new_node);
}

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

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupNwithop(node *arg_node, node *arg_info)
 *
 * description:
 *   duplicates a N_Nwithop-node
 *
 *
 ******************************************************************************/

node *
DupNwithop (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNwithop");
    new_node = MakeNWithOp (NWITHOP_TYPE (arg_node));
    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        NWITHOP_SHAPE (new_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        break;
    case WO_modarray:
        NWITHOP_ARRAY (new_node) = Trav (NWITHOP_ARRAY (arg_node), arg_info);
        break;
    case WO_foldfun:
        NWITHOP_NEUTRAL (new_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        NWITHOP_FUN (new_node) = StringCopy (NWITHOP_FUN (arg_node));
        NWITHOP_MOD (new_node) = StringCopy (NWITHOP_MOD (arg_node));
        NWITHOP_FUNDEF (new_node) = Trav (NWITHOP_FUNDEF (arg_node), arg_info);
        break;
    case WO_foldprf:
        NWITHOP_NEUTRAL (new_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        NWITHOP_PRF (new_node) = NWITHOP_PRF (arg_node);
        break;
    default:
        DBUG_ASSERT (0, "Unknown N_Nwithop-type found");
    }
    if (NWITHOP_EXPR (arg_node) != NULL)
        NWITHOP_EXPR (new_node) = Trav (NWITHOP_EXPR (arg_node), arg_info);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupNpart(node *arg_node, node *arg_info)
 *
 * description:
 *   duplicates a N_Npart-node
 *
 *
 ******************************************************************************/

node *
DupNpart (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNpart");

    new_node = MakeNPart (Trav (NPART_WITHID (arg_node), arg_info),
                          Trav (NPART_GEN (arg_node), arg_info));
    if (NPART_NEXT (arg_node) != NULL)
        NPART_NEXT (new_node) = Trav (NPART_NEXT (arg_node), arg_info);
    NPART_CODE (new_node) = NPART_CODE (arg_node);
    NCODE_USED (NPART_CODE (new_node))++;

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupNwithid(node *arg_node, node *arg_info)
 *
 * description:
 *   duplicates a N_Nwithid-node
 *
 *
 ******************************************************************************/

node *
DupNwithid (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNwithid");
    new_node = MakeNWithid (DupIds (NWITHID_VEC (arg_node), arg_info),
                            DupIds (NWITHID_IDS (arg_node), arg_info));

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupNgen(node *arg_node, node *arg_info)
 *
 * description:
 *   duplicates a N_Ngenerator-node
 *
 *
 ******************************************************************************/

node *
DupNgen (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNgen");
    new_node
      = MakeNGenerator (Trav (NGEN_BOUND1 (arg_node), arg_info),
                        Trav (NGEN_BOUND2 (arg_node), arg_info), NGEN_OP1 (arg_node),
                        NGEN_OP2 (arg_node), Trav (NGEN_STEP (arg_node), arg_info),
                        Trav (NGEN_WIDTH (arg_node), arg_info));

    DBUG_RETURN (new_node);
}
