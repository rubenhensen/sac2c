/*
 *
 * $Log$
 * Revision 1.4  2000/01/28 12:39:47  dkr
 * DupNcode, DupNpart, DupWLgrid changed
 * use of LUT added
 *
 * Revision 1.3  2000/01/26 23:25:33  dkr
 * DupTreePre() and DupTreePost() added.
 * Some code brushing done.
 *
 * Revision 1.2  2000/01/26 17:27:50  dkr
 * type of traverse-function-table changed.
 *
 * Revision 1.1  2000/01/21 11:16:25  dkr
 * Initial revision
 *
 * Revision 2.16  2000/01/20 16:39:15  cg
 * Bug fixed in DupAssign: data flow masks are only copied
 * when a new node is actually created.
 *
 * Revision 2.15  1999/09/01 17:11:01  jhs
 * Fixed Duplicating of masks in DupAssign.
 *
 * Revision 2.14  1999/08/27 11:12:55  jhs
 * Added copy of naive-refcounter while copying IDS.
 * Added copy of DEF-, USE- and MRDMASKs during DupAssign.
 *
 * Revision 2.13  1999/07/19 14:45:26  jhs
 * updated duplication of N_sync (SYNC_[FIRST|LAST are duplicated correctly).
 * Changed signature of MakeSync by the way.
 *
 * Revision 2.12  1999/07/07 15:04:25  sbs
 * DupVinfo added; it implicitly generates consistent VINFO_DOLLAR
 * pointers!!!
 *
 * Revision 2.11  1999/05/17 11:21:26  jhs
 * CopyConstVec will be called only if ID/ARRAY_ISCONST.
 *
 * Revision 2.10  1999/05/14 15:20:35  jhs
 * DupId checks whether the ID has a constant vector annotated before
 * it copies this constant vector.
 *
 * Revision 2.9  1999/05/14 09:25:13  jhs
 * Dbugged constvec annotations and their housekeeping in various compilation stages.
 *
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
 * [ ... ]
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

#include "DupTree.h"
#include "DataFlowMask.h"
#include "LookUpTable.h"
#include "Inline.h"
#include "typecheck.h"
#include "scheduling.h"
#include "generatemasks.h"

/******************************************************************************/

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

#define DUP(s, d)                                                                        \
    d->refcnt = s->refcnt;                                                               \
    d->flag = s->flag;                                                                   \
    d->counter = s->counter;                                                             \
    d->varno = s->varno;                                                                 \
    d->lineno = s->lineno;

/******************************************************************************/

static lut_t *lut;

/******************************************************************************/

/*
 *  DupTree duplicates the whole sub tree behind the given pointer.
 *
 *  DupNode duplicates only the given node without next node.
 *
 *  Usage of arg_info is not very nice:
 *  If the given arg_info is NULL, everything is copied in DUP_NORMAL mode,
 *  else the mode is taken from INFO_DUP_TYPE.
 *
 */

node *
DupTree (node *arg_node, node *arg_info)
{
    funtab *tmp_tab;
    node *new_node = NULL;
    int new_arg_info = 0;

    DBUG_ENTER ("DupTree");

    if (arg_node != NULL) {
        tmp_tab = act_tab;
        act_tab = dup_tab;

        if (arg_info == NULL) {
            arg_info = MakeInfo ();
            INFO_DUP_TYPE (arg_info) = DUP_NORMAL;
            INFO_DUP_ALL (arg_info) = 0;
            new_arg_info = 1;
        }

        /*
         * we want to duplicate all sons
         */
        INFO_DUP_CONT (arg_info) = NULL;
        lut = GenLUT ();
        new_node = Trav (arg_node, arg_info);
        lut = RemoveLUT (lut);

        if (new_arg_info) {
            arg_info = FreeNode (arg_info);
        }

        act_tab = tmp_tab;
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNode (node *arg_node)
{
    funtab *tmp_tab;
    node *arg_info;
    node *new_node = NULL;

    DBUG_ENTER ("DupTree");

    if (NULL != arg_node) {
        tmp_tab = act_tab;
        act_tab = dup_tab;

        arg_info = MakeInfo ();
        /*
         * duplicatation of sons of root 'arg_node' is controlled by
         *  DUPTRAV, DUPCONT
         */
        INFO_DUP_CONT (arg_info) = arg_node;
        new_node = Trav (arg_node, arg_info);
        FreeNode (arg_info);

        act_tab = tmp_tab;
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupTreePre( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
DupTreePre (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DupTreePre");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[NODE_TYPE (arg_node)]));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupTreePost( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
DupTreePost (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DupTreePost");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
DupBlock (node *arg_node, node *arg_info)
{
    node *tmp, *dup_vardec, *dup_instr;

    DBUG_ENTER ("DupBlock");

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

    new_node = MakeNode (NODE_TYPE (arg_node));
    DUP (arg_node, new_node);
    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++) {
        if (arg_node->node[i]) {
            new_node->node[i] = Trav (arg_node->node[i], arg_info);
        }
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNum (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNum");

    new_node = MakeNum (NUM_VAL (arg_node));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupBool (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupBool");

    new_node = MakeBool (BOOL_VAL (arg_node));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupFloat (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupFloat");

    new_node = MakeFloat (FLOAT_VAL (arg_node));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupDouble (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupDouble");

    new_node = MakeDouble (DOUBLE_VAL (arg_node));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupChar (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupChar");

    new_node = MakeChar (CHAR_VAL (arg_node));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupStr (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupStr");

    new_node = MakeStr (StringCopy (STR_STRING (arg_node)));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupVinfo (node *arg_node, node *arg_info)
{
    node *new_node, *rest;

    DBUG_ENTER ("DupVinfo");

    rest = NULL;
    if (VINFO_NEXT (arg_node) != NULL) {
        rest = Trav (VINFO_NEXT (arg_node), arg_info);
    }

    if (VINFO_FLAG (arg_node) == DOLLAR) {
        new_node = MakeVinfoDollar (rest);
    } else {
        new_node = MakeVinfo (VINFO_FLAG (arg_node), VINFO_TYPE (arg_node), rest,
                              VINFO_DOLLAR (rest));
    }
    VINFO_VARDEC (new_node) = VINFO_VARDEC (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

ids *
DupOneIds (ids *old_ids, node *arg_info)
{
    ids *new_ids;

    DBUG_ENTER ("DupOneIds");

    if ((arg_info != NULL) && (INFO_DUP_TYPE (arg_info) == DUP_INLINE)) {
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
        IDS_VARDEC (new_ids) = SearchInLUT (lut, IDS_VARDEC (old_ids));
        IDS_USE (new_ids) = IDS_USE (old_ids);
    }

    IDS_ATTRIB (new_ids) = IDS_ATTRIB (old_ids);
    IDS_REFCNT (new_ids) = IDS_REFCNT (old_ids);
    IDS_NAIVE_REFCNT (new_ids) = IDS_NAIVE_REFCNT (old_ids);

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

node *
DupId (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupId");

    if ((arg_info != NULL) && (INFO_DUP_TYPE (arg_info) == DUP_INLINE)) {
        new_node = MakeId (RenameInlinedVar (ID_NAME (arg_node)),
                           StringCopy (ID_MOD (arg_node)), ID_STATUS (arg_node));
        /* ID_OBJDEF and ID_VARDEC are mapped to the same data object */
        ID_VARDEC (new_node) = SearchDecl (ID_NAME (new_node), INFO_INL_TYPES (arg_info));
    } else {
        new_node = MakeId (StringCopy (ID_NAME (arg_node)),
                           StringCopy (ID_MOD (arg_node)), ID_STATUS (arg_node));
        /* ID_OBJDEF and ID_VARDEC are mapped to the same data object */
        ID_VARDEC (new_node) = SearchInLUT (lut, ID_VARDEC (arg_node));
    }
    ID_DEF (new_node) = SearchInLUT (lut, ID_DEF (arg_node));

    ID_ATTRIB (new_node) = ID_ATTRIB (arg_node);
    ID_REFCNT (new_node) = ID_REFCNT (arg_node);
    ID_NAIVE_REFCNT (new_node) = ID_NAIVE_REFCNT (arg_node);
    ID_MAKEUNIQUE (new_node) = ID_MAKEUNIQUE (arg_node);

    if (DUP_WLF == INFO_DUP_TYPE (arg_info)) {
        /* Withloop folding (wlf) needs this. */
        if (ID_WL (arg_node) && (N_id == NODE_TYPE (ID_WL (arg_node)))) {
            /* new code in new_codes, see 'usage of ID_WL' in WLF.c for more infos */
            ID_WL (new_node) = ID_WL (arg_node);
        } else {
            ID_WL (new_node) = arg_node; /* original code */
        }
    }

    /*  Coping the attibutes of constantvectors.
     *  CONSTVEC itself can only be copied, if ISCONST flag is set,
     *  otherwise VECTYPE might be T_unkown.
     */
    ID_ISCONST (new_node) = ID_ISCONST (arg_node);
    ID_VECTYPE (new_node) = ID_VECTYPE (arg_node);
    ID_VECLEN (new_node) = ID_VECLEN (arg_node);
    if (ID_ISCONST (new_node)) {
        ID_CONSTVEC (new_node)
          = CopyConstVec (ID_VECTYPE (arg_node), ID_VECLEN (arg_node),
                          ID_CONSTVEC (arg_node));
    } else {
        ID_CONSTVEC (new_node) = NULL;
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupLet (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupLet");

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

    /*  Coping the attibutes of constantvectors.
     *  CONSTVEC itself can only be copied, if ISCONST flag is set,
     *  otherwise VECTYPE might be T_unkown.
     */
    ID_ISCONST (new_node) = ID_ISCONST (arg_node);
    ID_VECTYPE (new_node) = ID_VECTYPE (arg_node);
    ID_VECLEN (new_node) = ID_VECLEN (arg_node);
    if (ID_ISCONST (new_node)) {
        ID_CONSTVEC (new_node)
          = CopyConstVec (ID_VECTYPE (arg_node), ID_VECLEN (arg_node),
                          ID_CONSTVEC (arg_node));
    } else {
        ID_CONSTVEC (new_node) = NULL;
    }

    if (N_id == NODE_TYPE (arg_node) && DUP_WLF == INFO_DUP_TYPE (arg_info)) {
        DBUG_PRINT ("DUP", ("duplicating N_id ..."));
        /* Withloop folding (wlf) needs this. */
        if (ID_WL (arg_node) && N_id == NODE_TYPE (ID_WL (arg_node))) {
            /* new code in new_codes, see 'usage of ID_WL' in WLF.c for more infos */
            ID_WL (new_node) = ID_WL (arg_node);
        } else {
            ID_WL (new_node) = arg_node; /* original code */
        }
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

    ARRAY_ISCONST (new_node) = ARRAY_ISCONST (arg_node);
    ARRAY_VECLEN (new_node) = ARRAY_VECLEN (arg_node);
    ARRAY_VECTYPE (new_node) = ARRAY_VECTYPE (arg_node);
    if (ARRAY_ISCONST (new_node)) {
        ARRAY_CONSTVEC (new_node)
          = CopyConstVec (ARRAY_VECTYPE (arg_node), ARRAY_VECLEN (arg_node),
                          ARRAY_CONSTVEC (arg_node));
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupCond (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupCond");

    new_node = MakeCond (DUPTRAV (COND_COND (arg_node)), DUPTRAV (COND_THEN (arg_node)),
                         DUPTRAV (COND_ELSE (arg_node)));

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

    new_node = MakeDo (DUPTRAV (DO_COND (arg_node)), DUPTRAV (DO_BODY (arg_node)));
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

    new_node
      = MakeExprs (DUPTRAV (EXPRS_EXPR (arg_node)), DUPCONT (EXPRS_NEXT (arg_node)));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupAssign (node *arg_node, node *arg_info)
{
    node *new_node = NULL;

    DBUG_ENTER ("DupAssign");

    switch (INFO_DUP_TYPE (arg_info)) {
    case DUP_INLINE:
        if (N_return == NODE_TYPE (ASSIGN_INSTR (arg_node)))
            break;
    default:
        new_node = MakeAssign (DUPTRAV (ASSIGN_INSTR (arg_node)),
                               DUPCONT (ASSIGN_NEXT (arg_node)));
        DUP (arg_node, new_node);

        if (INFO_DUP_ALL (arg_info)) {
            if (ASSIGN_DEFMASK (arg_node) != NULL) {
                ASSIGN_DEFMASK (new_node) = DupMask (ASSIGN_DEFMASK (arg_node), 400);
            }
            if (ASSIGN_USEMASK (arg_node) != NULL) {
                ASSIGN_USEMASK (new_node) = DupMask (ASSIGN_USEMASK (arg_node), 400);
            }
            if (ASSIGN_MRDMASK (arg_node) != NULL) {
                ASSIGN_MRDMASK (new_node) = DupMask (ASSIGN_MRDMASK (arg_node), 400);
            }
        }
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

node *
DupDec (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupDec");

    DBUG_ASSERT (((N_vardec == NODE_TYPE (arg_node)) || (N_arg == NODE_TYPE (arg_node))),
                 "wrong node type");

    new_node = MakeNode (NODE_TYPE (arg_node));
    DUP (arg_node, new_node);
    new_node->info.types = DuplicateTypes (arg_node->info.types, 1);

    if (NODE_TYPE (arg_node) == N_arg) {
        ARG_NEXT (new_node) = DUPCONT (ARG_NEXT (arg_node));
    } else {
        VARDEC_NEXT (new_node) = DUPCONT (VARDEC_NEXT (arg_node));
    }

    InsertIntoLUT (lut, arg_node, new_node);

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

    new_node = MakeSync (DUPTRAV (SYNC_REGION (arg_node)));

    SYNC_FIRST (new_node) = SYNC_FIRST (arg_node);
    SYNC_LAST (new_node) = SYNC_LAST (arg_node);

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

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNwith (node *arg_node, node *arg_info)
{
    node *new_node, *partn, *coden, *withopn;

    DBUG_ENTER ("DupNwith");

    /*
     * very important: duplicate codes before parts! Otherwise the code
     * references of the parts can not set correctly!
     */
    coden = DUPTRAV (NWITH_CODE (arg_node));
    partn = DUPTRAV (NWITH_PART (arg_node));
    withopn = DUPTRAV (NWITH_WITHOP (arg_node));

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
    node *new_node;

    DBUG_ENTER ("DupNpart");
    DBUG_ASSERT (NPART_CODE (arg_node), "N_Npart node has no valid NPART_CODE");

    new_node
      = MakeNPart (DUPTRAV (NPART_WITHID (arg_node)), DUPTRAV (NPART_GEN (arg_node)),
                   SearchInLUT (lut, NPART_CODE (arg_node)));

    NPART_NEXT (new_node) = DUPCONT (NPART_NEXT (arg_node));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNcode (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNcode");

    new_node
      = MakeNCode (DUPTRAV (NCODE_CBLOCK (arg_node)), DUPTRAV (NCODE_CEXPR (arg_node)));
    NCODE_NEXT (new_node) = DUPCONT (NCODE_NEXT (arg_node));

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

    InsertIntoLUT (lut, arg_node, new_node);

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
     * very important: duplicate codes before parts! Otherwise the code
     * references of the parts can not set correctly!
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
    node *new_node;

    DBUG_ENTER ("DupWLgrid");

    new_node
      = MakeWLgrid (WLGRID_LEVEL (arg_node), WLGRID_DIM (arg_node),
                    WLGRID_BOUND1 (arg_node), WLGRID_BOUND2 (arg_node),
                    WLGRID_UNROLLING (arg_node), DUPTRAV (WLGRID_NEXTDIM (arg_node)),
                    DUPCONT (WLGRID_NEXT (arg_node)),
                    SearchInLUT (lut, WLGRID_CODE (arg_node)));

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
    node *new_node;

    DBUG_ENTER ("DupWLgridVar");

    new_node = MakeWLgridVar (WLGRIDVAR_LEVEL (arg_node), WLGRIDVAR_DIM (arg_node),
                              DUPTRAV (WLGRIDVAR_BOUND1 (arg_node)),
                              DUPTRAV (WLGRIDVAR_BOUND2 (arg_node)),
                              DUPTRAV (WLGRIDVAR_NEXTDIM (arg_node)),
                              DUPCONT (WLGRIDVAR_NEXT (arg_node)),
                              SearchInLUT (lut, WLGRIDVAR_CODE (arg_node)));

    DBUG_RETURN (new_node);
}
