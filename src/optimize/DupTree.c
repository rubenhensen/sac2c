/*
 *
 * $Log$
 * Revision 2.8  1999/05/12 09:56:35  jhs
 * Adjusted macros to access constant vectors.
 *
 * Revision 2.7  1999/04/14 13:17:29  jhs
 * DupBlock does not traverse into not existing vardecs or exprs anymore.
 *
 * Revision 2.6  1999/04/13 14:01:48  cg
 * added function DupBlock for duplication of N_block nodes.
 *
 * Revision 2.5  1999/03/15 18:55:43  dkr
 * some modifications made (for CC)
 *
 * Revision 2.4  1999/03/15 14:04:57  bs
 * Access macros renamed (take a look at tree_basic.h).
 * DupArray modified.
 *
 * Revision 2.3  1999/03/03 09:46:49  bs
 * I did some code-cosmetics in DupArray for a better understanding.
 *
 * Revision 2.2  1999/02/25 10:58:42  bs
 * DupArray added.
 *
 * Revision 2.1  1999/02/23 12:41:17  sacbase
 * new release made
 *
 * Revision 1.107  1998/08/11 14:36:00  dkr
 * DupWLstriVar, DupWLgridVar changed
 *
 * Revision 1.106  1998/08/11 12:10:06  dkr
 * DupWLsegVar changed
 *
 * Revision 1.105  1998/08/11 00:11:21  dkr
 * minor bug in DupFundef fixed
 * changed DupWLsegVar
 *
 * Revision 1.104  1998/08/07 14:36:57  dkr
 * DupWLsegVar added
 *
 * Revision 1.103  1998/07/20 19:06:50  dkr
 * In DupOneIds:
 *   ASSERT is replaced by a warning
 *
 * Revision 1.102  1998/07/20 16:52:28  dkr
 * fixed a bug in DupPragma
 *
 * Revision 1.101  1998/06/18 13:44:04  cg
 * file is now able to deal correctly with data objects of
 * the abstract data type for the representation of schedulings.
 *
 * Revision 1.100  1998/06/09 16:52:42  dkr
 * fixed a bug in DupWLseg()
 *
 * Revision 1.99  1998/06/09 16:45:49  dkr
 * IDX_MIN, IDX_MAX now segment-specific
 *
 * Revision 1.98  1998/06/08 12:37:52  cg
 * Function DupTypes now only duplicates the types structure if
 * an original one is present.
 *
 * Revision 1.97  1998/06/06 13:30:20  dkr
 * fixed a bug in DupCond, DupLoop:
 *   ids duplicated only, if != NULL !!!
 *
 * Revision 1.96  1998/06/05 15:32:34  cg
 * Attributes COND_THENVARS COND_ELSEVARS DO_USEVARS DO_DEFVARS are
 * now duplicated as ids-chains
 *
 * Revision 1.95  1998/05/28 23:51:00  dkr
 * fixed a bug in DupNwithop:
 *   NWITHOP_FUNDEF is copied not duplicated
 *
 * Revision 1.94  1998/05/24 00:40:25  dkr
 * removed WLGRID_CODE_TEMPLATE
 *
 * Revision 1.93  1998/05/21 15:27:16  dkr
 * fixed a bug in DupNwith, DupNwith2, DupNcode:
 *   argument of DupIds is always != NULL now
 *
 * Revision 1.92  1998/05/21 13:31:12  dkr
 * renamed NCODE_DEC_RC_IDS into NCODE_INC_RC_IDS
 *
 * Revision 1.91  1998/05/21 10:16:14  dkr
 * fixed a bug in DupNcode, DupNwith2:
 *   now ..._DEC_RC_IDS is duplicated, too
 *
 * Revision 1.90  1998/05/17 00:10:11  dkr
 * WLGRID_CEXPR_TEMPLATE is now WLGRID_CODE_TEMPLATE
 * fixed a bug with CODE_TEMPLATE:
 *   CODE_TEMPLATE is duplicated, too
 *
 * Revision 1.89  1998/05/15 23:54:21  dkr
 * changed DupNwith2
 *
 * Revision 1.88  1998/05/14 21:38:59  dkr
 * added WLGRID_CEXPR_TEMPLATE in DupWLgrid()
 *
 * Revision 1.87  1998/05/12 22:41:21  dkr
 * added NWITH2_DIM, NWITH2_IDX_MIN, NWITH2_IDX_MAX
 *
 * Revision 1.86  1998/05/12 18:46:06  dkr
 * removed ???_VARINFO
 *
 * Revision 1.85  1998/05/12 18:43:25  srs
 * inserted DUP_WLF
 *
 * Revision 1.84  1998/05/08 00:56:24  dkr
 * removed unused var
 *
 * Revision 1.83  1998/05/06 21:17:08  dkr
 * added support for DFMasks
 *
 * Revision 1.82  1998/05/06 15:11:23  srs
 * INFO_DUP_CON in now initialized correctly.
 * If arg_info is craeted here, it is set free afterwards.
 *
 * Revision 1.81  1998/05/06 13:18:00  dkr
 * Changed DupFundef
 *
 * Revision 1.80  1998/05/06 12:24:17  dkr
 * insert access macros
 *
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
#include "DataFlowMask.h"
#include "optimize.h"
#include "Inline.h"
#include "scheduling.h"

/******************************************************************************/

/* LEVEL is only needed to distinguishe between th two occurences
   of N_return: inside the old WL and at the end of functions. */
#define LEVEL arg_info->lineno

/*
 * traverses son 'node'
 */
#define DUPTRAV(node) (node != NULL) ? Trav (node, arg_info) : NULL

/*
 * DupNode: INFO_DUP_CONT(arg_info) contains the root of syntaxtree.
 *  -> traverses son 'node' if and only if its parent is not the root.
 * DupTree: INFO_DUP_CONT(arg_info) is NULL.
 *  -> traverses son 'node'.
 */
#define DUPCONT(node) (INFO_DUP_CONT (arg_info) != arg_node) ? DUPTRAV (node) : NULL

/******************************************************************************/

/*
 *  DupTree duplicates the whole sub tree behind the given pointer.
 *
 *  DupNode duplicates only the given node without next node.
 *
 * Usage of arg_info is not very nice:
 * If the given arg_info is NULL, everything is copies in DUP_NORMAL mode.
 * Else the mode is taken from arg_info->flag (see DUPTYPE macro).
 *
 */

node *
DupTree (node *arg_node, node *arg_info)
{
    funptr *tmp_tab;
    node *new_node = NULL;
    int new_arg_info = 0;

    DBUG_ENTER ("DupTree");

    if (arg_node) {
        tmp_tab = act_tab;
        act_tab = dup_tab;

        if (!arg_info) {
            arg_info = MakeInfo ();
            DUPTYPE = DUP_NORMAL;
            new_arg_info = 1;
        }

        LEVEL = 0;
        /*
         * we want to duplicate all sons
         */
        INFO_DUP_CONT (arg_info) = NULL;
        new_node = Trav (arg_node, arg_info);

        if (new_arg_info)
            arg_info = FreeNode (arg_info);

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
        INFO_DUP_CONT (arg_info) = arg_node;
        new_node = Trav (arg_node, arg_info);
        FREE (arg_info);

        act_tab = tmp_tab;
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupBlock (node *arg_node, node *arg_info)
{
    node *tmp, *dup_vardec, *dup_instr;

    DBUG_ENTER ("DupBlock");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));

    if (BLOCK_INSTR (arg_node) == NULL) {
        dup_instr = NULL;
    } else {
        dup_instr = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_VARDEC (arg_node) == NULL) {
        dup_vardec = NULL;
    } else {
        dup_vardec = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    tmp = MakeBlock (dup_instr, dup_vardec);

    DBUG_PRINT ("DUP", ("Traversals finished"));

    BLOCK_VARNO (tmp) = BLOCK_VARNO (arg_node);

    NODE_LINE (tmp) = NODE_LINE (arg_node);

    if (BLOCK_CACHESIM (arg_node) != NULL) {
        BLOCK_CACHESIM (tmp) = StringCopy (BLOCK_CACHESIM (arg_node));
    }

    DBUG_RETURN (tmp);
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
DupOneIds (ids *old_ids, node *arg_info)
{
    ids *new_ids;

    DBUG_ENTER ("DupOneIds");

    if ((arg_info != NULL) && (DUPTYPE == DUP_INLINE)) {
        new_ids = MakeIds (RenameInlinedVar (IDS_NAME (old_ids)),
                           StringCopy (IDS_MOD (old_ids)), IDS_STATUS (old_ids));
        IDS_VARDEC (new_ids) = SearchDecl (IDS_NAME (new_ids), INFO_INL_TYPES (arg_info));

        /*
         * If (IDS_VARDEC == NULL) we better not abort, because this should be
         * a IDS-node of a wlcomp-pragma !!
         * We only print a warning here, 'wltransform' will check the rest !!
         */
        if (NULL == IDS_VARDEC (new_ids)) {
            /*
             * Sorry, we do not have any line numbers at IDS-nodes :(
             * This is no good, I guess !!
             */
            WARN (0, ("No declaration found for ids-node"));
        }
    } else {
        new_ids = MakeIds (StringCopy (IDS_NAME (old_ids)),
                           StringCopy (IDS_MOD (old_ids)), IDS_STATUS (old_ids));
        IDS_VARDEC (new_ids) = IDS_VARDEC (old_ids);
        IDS_USE (new_ids) = IDS_USE (old_ids);
    }

    IDS_ATTRIB (new_ids) = IDS_ATTRIB (old_ids);
    IDS_REFCNT (new_ids) = IDS_REFCNT (old_ids);

    DBUG_RETURN (new_ids);
}

/******************************************************************************/

ids *
DupIds (ids *old_ids, node *arg_info)
{
    ids *new_ids;

    DBUG_ENTER ("DupIds");

    new_ids = DupOneIds (old_ids, arg_info);

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
    DBUG_PRINT ("DUP", ("Traversals finished"));

    ID_CONSTVEC (new_node) = CopyConstVec (ID_VECTYPE (arg_node), ID_VECLEN (arg_node),
                                           ID_CONSTVEC (arg_node));

    if (N_id == NODE_TYPE (arg_node) && DUP_WLF == DUPTYPE) {
        DBUG_PRINT ("DUP", ("duplicating N_id ..."));
        /* Withloop folding (wlf) needs this. */
        if (ID_WL (arg_node) && N_id == NODE_TYPE (ID_WL (arg_node)))
            /* new code in new_codes, see 'usage of ID_WL' in WLF.c for more infos */
            ID_WL (new_node) = ID_WL (arg_node);
        else
            ID_WL (new_node) = arg_node; /* original code */
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupArray (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupArray");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[NODE_TYPE (arg_node)]));

    new_node = MakeArray (NULL);
    if (ARRAY_TYPE (arg_node) != NULL)
        ARRAY_TYPE (new_node) = DuplicateTypes (ARRAY_TYPE (arg_node), 1);
    else
        ARRAY_TYPE (new_node) = NULL;

    DUP (arg_node, new_node);
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++) {
        if (arg_node->node[i] != NULL) {
            new_node->node[i] = Trav (arg_node->node[i], arg_info);
        }
    }
    ARRAY_STRING (new_node) = StringCopy (ARRAY_STRING (arg_node));

    ARRAY_CONSTVEC (new_node)
      = CopyConstVec (ARRAY_VECTYPE (arg_node), ARRAY_VECLEN (arg_node),
                      ARRAY_CONSTVEC (arg_node));

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

    if (COND_THENVARS (arg_node) != NULL) {
        COND_THENVARS (new_node) = DupIds (COND_THENVARS (arg_node), arg_info);
    }
    if (COND_ELSEVARS (arg_node) != NULL) {
        COND_ELSEVARS (new_node) = DupIds (COND_ELSEVARS (arg_node), arg_info);
    }

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

    if (DO_USEVARS (arg_node) != NULL) {
        DO_USEVARS (new_node) = DupIds (DO_USEVARS (arg_node), arg_info);
    }
    if (DO_DEFVARS (arg_node) != NULL) {
        DO_DEFVARS (new_node) = DupIds (DO_DEFVARS (arg_node), arg_info);
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupExprs (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupExprs");

    LEVEL++;
    new_node
      = MakeExprs (DUPTRAV (EXPRS_EXPR (arg_node)), DUPCONT (EXPRS_NEXT (arg_node)));
    LEVEL--;

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupAssign (node *arg_node, node *arg_info)
{
    node *new_node = NULL;

    DBUG_ENTER ("DupAssign");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));

    switch (DUPTYPE) {

    case DUP_INLINE:
        if ((0 == LEVEL) && (N_return == NODE_TYPE (ASSIGN_INSTR (arg_node))))
            break;

    default:
        new_node = MakeAssign (DUPTRAV (ASSIGN_INSTR (arg_node)),
                               DUPCONT (ASSIGN_NEXT (arg_node)));
        DUP (arg_node, new_node);
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
    if (arg_node->info.types != NULL)
        new_node->info.types = DuplicateTypes (arg_node->info.types, 1);
    else
        new_node->info.types = NULL;

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
GetReturn (node *instr)
{
    node *ret_node;

    DBUG_ENTER ("GetReturn");

    ret_node = instr;
    while ((ret_node != NULL) && (ASSIGN_NEXT (ret_node) != NULL)) {
        ret_node = ASSIGN_NEXT (ret_node);
    }
    DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (ret_node)) == N_return),
                 "return node not found");

    DBUG_RETURN (ASSIGN_INSTR (ret_node));
}

/******************************************************************************/

node *
DupFundef (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupFundef");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));

    new_node = MakeNode (NODE_TYPE (arg_node));

    FUNDEF_TYPES (new_node) = DuplicateTypes (FUNDEF_TYPES (arg_node), 1);
    DUP (arg_node, new_node);

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (new_node) = DUPTRAV (FUNDEF_BODY (arg_node));
        FUNDEF_NEEDFUNS (new_node) = CopyNodelist (FUNDEF_NEEDFUNS (arg_node));
        FUNDEF_NEEDTYPES (new_node) = CopyNodelist (FUNDEF_NEEDTYPES (arg_node));
    }

    FUNDEF_ARGS (new_node) = DUPTRAV (FUNDEF_ARGS (arg_node));
    FUNDEF_PRAGMA (new_node) = DUPTRAV (FUNDEF_PRAGMA (arg_node));
    FUNDEF_NEEDOBJS (new_node) = CopyNodelist (FUNDEF_NEEDOBJS (arg_node));

    FUNDEF_RETURN (new_node) = GetReturn (BLOCK_INSTR (FUNDEF_BODY (new_node)));

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

    DBUG_ASSERT (((N_vardec == NODE_TYPE (arg_node)) || (N_arg == NODE_TYPE (arg_node))),
                 "wrong node type");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[NODE_TYPE (arg_node)]));

    new_node = MakeNode (NODE_TYPE (arg_node));
    DUP (arg_node, new_node);
    new_node->info.types = DuplicateTypes (arg_node->info.types, 1);

    if (NODE_TYPE (arg_node) == N_arg) {
        ARG_NEXT (new_node) = DUPCONT (ARG_NEXT (arg_node));
    } else {
        VARDEC_NEXT (new_node) = DUPCONT (VARDEC_NEXT (arg_node));
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
    new_node = MakeInfo();
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
        PRAGMA_WLCOMP_APS (new_node) = DUPTRAV (PRAGMA_WLCOMP_APS (arg_node));
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupIcm (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupIcm");

    new_node = MakeIcm (ICM_NAME (arg_node), DUPTRAV (ICM_ARGS (arg_node)),
                        DUPCONT (ICM_NEXT (arg_node)));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupSpmd (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupSpmd");

    new_node = MakeSpmd (DUPTRAV (SPMD_REGION (arg_node)));

    if (SPMD_IN (arg_node) != NULL) {
        SPMD_IN (new_node) = DFMGenMaskCopy (SPMD_IN (arg_node));
    }
    if (SPMD_INOUT (arg_node) != NULL) {
        SPMD_INOUT (new_node) = DFMGenMaskCopy (SPMD_INOUT (arg_node));
    }
    if (SPMD_OUT (arg_node) != NULL) {
        SPMD_OUT (new_node) = DFMGenMaskCopy (SPMD_OUT (arg_node));
    }
    if (SPMD_LOCAL (arg_node) != NULL) {
        SPMD_LOCAL (new_node) = DFMGenMaskCopy (SPMD_LOCAL (arg_node));
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupSync (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupSync");

    new_node = MakeSync (DUPTRAV (SYNC_REGION (arg_node)), SYNC_FIRST (arg_node));

    if (SYNC_IN (arg_node) != NULL) {
        SYNC_IN (new_node) = DFMGenMaskCopy (SYNC_IN (arg_node));
    }
    if (SYNC_INOUT (arg_node) != NULL) {
        SYNC_INOUT (new_node) = DFMGenMaskCopy (SYNC_INOUT (arg_node));
    }
    if (SYNC_OUT (arg_node) != NULL) {
        SYNC_OUT (new_node) = DFMGenMaskCopy (SYNC_OUT (arg_node));
    }
    if (SYNC_LOCAL (arg_node) != NULL) {
        SYNC_LOCAL (new_node) = DFMGenMaskCopy (SYNC_LOCAL (arg_node));
    }

    if (SYNC_SCHEDULING (arg_node) != NULL) {
        SYNC_SCHEDULING (new_node) = SCHCopyScheduling (SYNC_SCHEDULING (arg_node));
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNwith (node *arg_node, node *arg_info)
{
    node *new_node, *partn, *coden, *withopn;

    DBUG_ENTER ("DupNwith");

    LEVEL++;
    /*
     * very important: duplicate codes before parts because NCODE_COPY has to
     *  be set before the parts are traversed.
     */
    coden = DUPTRAV (NWITH_CODE (arg_node));
    partn = DUPTRAV (NWITH_PART (arg_node));
    withopn = DUPTRAV (NWITH_WITHOP (arg_node));
    LEVEL--;

    new_node = MakeNWith (partn, coden, withopn);

    /*
     * Now we must erase NCODE_COPY for every code node in 'arg_node'.
     * Otherwise we get nice ;-> errors when duplicating N_Npart- or
     * N_Ngrid-nodes from 'arg_node' without the parent N_Nwith-node !!!
     */
    coden = NWITH_CODE (arg_node);
    while (coden != NULL) {
        NCODE_COPY (coden) = NULL;
        coden = NCODE_NEXT (coden);
    }

    /* copy attributes */
    DUP (arg_node, new_node);
    NWITH_PARTS (new_node) = NWITH_PARTS (arg_node);
    NWITH_REFERENCED_FOLD (new_node) = NWITH_REFERENCED_FOLD (arg_node);
    NWITH_REFERENCED (new_node) = NWITH_REFERENCED (arg_node);
    NWITH_COMPLEX (new_node) = NWITH_COMPLEX (arg_node);
    NWITH_FOLDABLE (new_node) = NWITH_FOLDABLE (arg_node);
    NWITH_NO_CHANCE (new_node) = NWITH_NO_CHANCE (arg_node);

    NWITH_PRAGMA (new_node) = DUPTRAV (NWITH_PRAGMA (arg_node));
    if (NWITH_DEC_RC_IDS (arg_node) != NULL) {
        NWITH_DEC_RC_IDS (new_node) = DupIds (NWITH_DEC_RC_IDS (arg_node), arg_info);
    }

    if (NWITH_IN (arg_node) != NULL) {
        NWITH_IN (new_node) = DFMGenMaskCopy (NWITH_IN (arg_node));
    }
    if (NWITH_INOUT (arg_node) != NULL) {
        NWITH_INOUT (new_node) = DFMGenMaskCopy (NWITH_INOUT (arg_node));
    }
    if (NWITH_OUT (arg_node) != NULL) {
        NWITH_OUT (new_node) = DFMGenMaskCopy (NWITH_OUT (arg_node));
    }
    if (NWITH_LOCAL (arg_node) != NULL) {
        NWITH_LOCAL (new_node) = DFMGenMaskCopy (NWITH_LOCAL (arg_node));
    }

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
        NWITHOP_MOD (new_node) = StringCopy (NWITHOP_MOD (arg_node));
        NWITHOP_FUNDEF (new_node) = NWITHOP_FUNDEF (arg_node);
        NWITHOP_FUN (new_node) = StringCopy (NWITHOP_FUN (arg_node));
        break;
    case WO_foldprf:
        NWITHOP_NEUTRAL (new_node) = DUPTRAV (NWITHOP_NEUTRAL (arg_node));
        NWITHOP_FUNDEF (new_node) = NWITHOP_FUNDEF (arg_node);
        NWITHOP_PRF (new_node) = NWITHOP_PRF (arg_node);
        break;
    default:
        DBUG_ASSERT (0, "Unknown N_Nwithop-type found");
        break;
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNpart (node *arg_node, node *arg_info)
{
    node *new_node, *code;

    DBUG_ENTER ("DupNpart");
    DBUG_ASSERT (NPART_CODE (arg_node), "N_Npart node has no valid NPART_CODE");

    /* get pointer to duplicated code!!! */
    code = NCODE_COPY (NPART_CODE (arg_node));
    if (code == NULL) {
        /* code has not been duplicated, so we take the original one!! */
        code = NPART_CODE (arg_node);
    }

    new_node = MakeNPart (DUPTRAV (NPART_WITHID (arg_node)),
                          DUPTRAV (NPART_GEN (arg_node)), code);
    NPART_NEXT (new_node) = DUPCONT (NPART_NEXT (arg_node));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNcode (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNcode");

    LEVEL++;
    new_node
      = MakeNCode (DUPTRAV (NCODE_CBLOCK (arg_node)), DUPTRAV (NCODE_CEXPR (arg_node)));
    NCODE_NEXT (new_node) = DUPCONT (NCODE_NEXT (arg_node));
    LEVEL--;

    /*
     * NCODE_USED is incremented in DupNpart() via MakeNPart(),
     *                           in DupWLgrid() via MakeWLgrid(), respectively
     */
    NCODE_USED (new_node) = 0;
    NCODE_NO (new_node) = NCODE_NO (arg_node);
    NCODE_FLAG (new_node) = NCODE_FLAG (arg_node);
    if (NCODE_INC_RC_IDS (arg_node) != NULL) {
        NCODE_INC_RC_IDS (new_node) = DupIds (NCODE_INC_RC_IDS (arg_node), arg_info);
    }

    NCODE_COPY (arg_node) = new_node;

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNwithid (node *arg_node, node *arg_info)
{
    node *new_node;
    ids *_vec, *_ids;

    DBUG_ENTER ("DupNwithid");

    _vec = (NWITHID_VEC (arg_node) != NULL) ? DupIds (NWITHID_VEC (arg_node), arg_info)
                                            : NULL;
    _ids = (NWITHID_IDS (arg_node) != NULL) ? DupIds (NWITHID_IDS (arg_node), arg_info)
                                            : NULL;

    new_node = MakeNWithid (_vec, _ids);

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
DupNwith2 (node *arg_node, node *arg_info)
{
    node *new_node, *id, *segs, *code, *withop;

    DBUG_ENTER ("DupNwith2");

    /*
     * very important: copy codes before segs because NCODE_COPY has to
     *  be set before the segs (containing N_WLgrid nodes) are traversed.
     */
    code = DUPTRAV (NWITH2_CODE (arg_node));
    id = DUPTRAV (NWITH2_WITHID (arg_node));
    segs = DUPTRAV (NWITH2_SEGS (arg_node));
    withop = DUPTRAV (NWITH2_WITHOP (arg_node));

    new_node = MakeNWith2 (id, segs, code, withop, NWITH2_DIMS (arg_node));

    if (NWITH2_DEC_RC_IDS (arg_node) != NULL) {
        NWITH2_DEC_RC_IDS (new_node) = DupIds (NWITH2_DEC_RC_IDS (arg_node), arg_info);
    }

    if (NWITH2_IN (arg_node) != NULL) {
        NWITH2_IN (new_node) = DFMGenMaskCopy (NWITH2_IN (arg_node));
    }
    if (NWITH2_INOUT (arg_node) != NULL) {
        NWITH2_INOUT (new_node) = DFMGenMaskCopy (NWITH2_INOUT (arg_node));
    }
    if (NWITH2_OUT (arg_node) != NULL) {
        NWITH2_OUT (new_node) = DFMGenMaskCopy (NWITH2_OUT (arg_node));
    }
    if (NWITH2_LOCAL (arg_node) != NULL) {
        NWITH2_LOCAL (new_node) = DFMGenMaskCopy (NWITH2_LOCAL (arg_node));
    }

    if (NWITH2_SCHEDULING (arg_node) != NULL) {
        NWITH2_SCHEDULING (new_node) = SCHCopyScheduling (NWITH2_SCHEDULING (arg_node));
    }

    /*
     * Now we must erase NCODE_COPY for every code node in 'arg_node'.
     * Otherwise we get nice ;-> errors when duplicating N_Npart- or
     * N_Ngrid-nodes from 'arg_node' without the parent N_Nwith2-node !!!
     */
    code = NWITH2_CODE (arg_node);
    while (code != NULL) {
        NCODE_COPY (code) = NULL;
        code = NCODE_NEXT (code);
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLseg (node *arg_node, node *arg_info)
{
    node *new_node;
    int i, d;

    DBUG_ENTER ("DupWLseg");

    new_node = MakeWLseg (WLSEG_DIMS (arg_node), DUPTRAV (WLSEG_CONTENTS (arg_node)),
                          DUPCONT (WLSEG_NEXT (arg_node)));

    if (WLSEG_IDX_MIN (arg_node) != NULL) {
        WLSEG_IDX_MIN (new_node) = (int *)MALLOC (WLSEG_DIMS (new_node) * sizeof (int));
        for (d = 0; d < WLSEG_DIMS (new_node); d++) {
            (WLSEG_IDX_MIN (new_node))[d] = (WLSEG_IDX_MIN (arg_node))[d];
        }
    }
    if (WLSEG_IDX_MAX (arg_node) != NULL) {
        WLSEG_IDX_MAX (new_node) = (int *)MALLOC (WLSEG_DIMS (new_node) * sizeof (int));
        for (d = 0; d < WLSEG_DIMS (new_node); d++) {
            (WLSEG_IDX_MAX (new_node))[d] = (WLSEG_IDX_MAX (arg_node))[d];
        }
    }

    WLSEG_BLOCKS (new_node) = WLSEG_BLOCKS (arg_node);

    for (i = 0; i < WLSEG_BLOCKS (new_node); i++) {
        if (WLSEG_BV (arg_node, i) != NULL) {
            WLSEG_BV (new_node, i)
              = (long *)MALLOC (WLSEG_DIMS (new_node) * sizeof (int));
            for (d = 0; d < WLSEG_DIMS (new_node); d++) {
                (WLSEG_BV (new_node, i))[d] = (WLSEG_BV (arg_node, i))[d];
            }
        }
    }

    if (WLSEG_UBV (arg_node) != NULL) {
        WLSEG_UBV (new_node) = (long *)MALLOC (WLSEG_DIMS (new_node) * sizeof (int));
        for (d = 0; d < WLSEG_DIMS (new_node); d++) {
            (WLSEG_UBV (new_node))[d] = (WLSEG_UBV (arg_node))[d];
        }
    }

    if (WLSEG_SV (arg_node) != NULL) {
        WLSEG_SV (new_node) = (long *)MALLOC (WLSEG_DIMS (new_node) * sizeof (int));
        for (d = 0; d < WLSEG_DIMS (new_node); d++) {
            (WLSEG_SV (new_node))[d] = (WLSEG_SV (arg_node))[d];
        }
    }

    if (WLSEG_SCHEDULING (arg_node) != NULL) {
        WLSEG_SCHEDULING (new_node) = SCHCopyScheduling (WLSEG_SCHEDULING (arg_node));
    }
    WLSEG_MAXHOMDIM (new_node) = WLSEG_MAXHOMDIM (arg_node);

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
    WLSTRIDE_MODIFIED (new_node) = NULL;

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLgrid (node *arg_node, node *arg_info)
{
    node *new_node, *new_code = NULL;

    DBUG_ENTER ("DupWLgrid");

    /*
     * set new code
     */
    if (WLGRID_CODE (arg_node) != NULL) {
        /* get pointer to duplicated code!!! */
        new_code = NCODE_COPY (WLGRID_CODE (arg_node));
        if (new_code == NULL) {
            /* code has not been duplicated, so we take the original one!! */
            new_code = WLGRID_CODE (arg_node);
        }
    }

    new_node
      = MakeWLgrid (WLGRID_LEVEL (arg_node), WLGRID_DIM (arg_node),
                    WLGRID_BOUND1 (arg_node), WLGRID_BOUND2 (arg_node),
                    WLGRID_UNROLLING (arg_node), DUPTRAV (WLGRID_NEXTDIM (arg_node)),
                    DUPCONT (WLGRID_NEXT (arg_node)), new_code);

    /*
     * duplicated grids are not modified yet ;)
     */
    WLGRID_MODIFIED (new_node) = NULL;

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLsegVar (node *arg_node, node *arg_info)
{
    node *new_node;
    int i, d;

    DBUG_ENTER ("DupWLsegVar");

    new_node
      = MakeWLsegVar (WLSEGVAR_DIMS (arg_node), DUPTRAV (WLSEGVAR_CONTENTS (arg_node)),
                      DUPCONT (WLSEGVAR_NEXT (arg_node)));

    if (WLSEGVAR_IDX_MIN (arg_node) != NULL) {
        WLSEGVAR_IDX_MIN (new_node)
          = (int *)MALLOC (WLSEGVAR_DIMS (new_node) * sizeof (int));
        for (d = 0; d < WLSEGVAR_DIMS (new_node); d++) {
            (WLSEGVAR_IDX_MIN (new_node))[d] = (WLSEGVAR_IDX_MIN (arg_node))[d];
        }
    }
    if (WLSEGVAR_IDX_MAX (arg_node) != NULL) {
        WLSEGVAR_IDX_MAX (new_node)
          = (int *)MALLOC (WLSEGVAR_DIMS (new_node) * sizeof (int));
        for (d = 0; d < WLSEGVAR_DIMS (new_node); d++) {
            (WLSEGVAR_IDX_MAX (new_node))[d] = (WLSEGVAR_IDX_MAX (arg_node))[d];
        }
    }

    WLSEGVAR_BLOCKS (new_node) = WLSEGVAR_BLOCKS (arg_node);

    for (i = 0; i < WLSEGVAR_BLOCKS (new_node); i++) {
        if (WLSEGVAR_BV (arg_node, i) != NULL) {
            WLSEGVAR_BV (new_node, i)
              = (long *)MALLOC (WLSEGVAR_DIMS (new_node) * sizeof (int));
            for (d = 0; d < WLSEGVAR_DIMS (new_node); d++) {
                (WLSEGVAR_BV (new_node, i))[d] = (WLSEGVAR_BV (arg_node, i))[d];
            }
        }
    }

    if (WLSEGVAR_UBV (arg_node) != NULL) {
        WLSEGVAR_UBV (new_node)
          = (long *)MALLOC (WLSEGVAR_DIMS (new_node) * sizeof (int));
        for (d = 0; d < WLSEGVAR_DIMS (new_node); d++) {
            (WLSEGVAR_UBV (new_node))[d] = (WLSEGVAR_UBV (arg_node))[d];
        }
    }

    if (WLSEGVAR_SV (arg_node) != NULL) {
        WLSEGVAR_SV (new_node) = (long *)MALLOC (WLSEGVAR_DIMS (new_node) * sizeof (int));
        for (d = 0; d < WLSEGVAR_DIMS (new_node); d++) {
            (WLSEGVAR_SV (new_node))[d] = (WLSEGVAR_SV (arg_node))[d];
        }
    }

    if (WLSEGVAR_SCHEDULING (arg_node) != NULL) {
        WLSEGVAR_SCHEDULING (new_node)
          = SCHCopyScheduling (WLSEGVAR_SCHEDULING (arg_node));
    }
    WLSEGVAR_MAXHOMDIM (new_node) = WLSEGVAR_MAXHOMDIM (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLstriVar (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupWLstriVar");

    new_node = MakeWLstriVar (WLSTRIVAR_LEVEL (arg_node), WLSTRIVAR_DIM (arg_node),
                              DUPTRAV (WLSTRIVAR_BOUND1 (arg_node)),
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
    node *new_node, *new_code = NULL;

    DBUG_ENTER ("DupWLgridVar");

    /*
     * set new code
     */
    if (WLGRIDVAR_CODE (arg_node) != NULL) {
        /* get pointer to duplicated code!!! */
        new_code = NCODE_COPY (WLGRIDVAR_CODE (arg_node));
        if (new_code == NULL) {
            /* code has not been duplicated, so we take the original one!! */
            new_code = WLGRIDVAR_CODE (arg_node);
        }
    }

    new_node = MakeWLgridVar (WLGRIDVAR_LEVEL (arg_node), WLGRIDVAR_DIM (arg_node),
                              DUPTRAV (WLGRIDVAR_BOUND1 (arg_node)),
                              DUPTRAV (WLGRIDVAR_BOUND2 (arg_node)),
                              DUPTRAV (WLGRIDVAR_NEXTDIM (arg_node)),
                              DUPCONT (WLGRIDVAR_NEXT (arg_node)), new_code);

    DBUG_RETURN (new_node);
}
