/*
 *
 * $Log$
 * Revision 1.11  2000/02/22 12:00:04  jhs
 * Adapted NODE_TEXT.
 * /
 *
 * Revision 1.10  2000/02/17 16:18:37  cg
 * Function DuplicateTypes() moved from typecheck.c.
 * New function DupTypes() added.
 *
 * Revision 1.9  2000/02/09 14:13:57  dkr
 * WLSEGVAR_MAXHOMDIM removed
 *
 * Revision 1.8  2000/02/03 17:30:30  dkr
 * GenLUT renamed to GenerateLUT
 * DupTreeLUT and DupNodeLUT added
 *
 * Revision 1.7  2000/02/03 08:35:43  dkr
 * GenLUT renamed to GenerateLUT
 *
 * Revision 1.6  2000/01/31 14:00:30  dkr
 * redundant macro DUP removed
 *
 * Revision 1.5  2000/01/31 13:28:32  dkr
 * Code brushed
 * Some Functions renamed
 * Some Functions specialized (DupChain, ... removed)
 *
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

/******************************************************************************/

static lut_t *dup_lut;

/******************************************************************************/

/*
 *  DupTree duplicates the whole sub tree behind the given pointer.
 *  The needed LUT is created from scratch.
 *
 *  DupTreeLUT duplicates the whole sub tree behind the given pointer.
 *  The needed LUT can be given as an argument.
 *
 *  DupNode duplicates only the given node without next node.
 *
 *  Usage of arg_info is not very nice:
 *  If the given arg_info is NULL, everything is copied in DUP_NORMAL mode,
 *  else the mode is taken from INFO_DUP_TYPE.
 *
 */

node *
DupTreeLUT (node *arg_node, node *arg_info, lut_t *lut)
{
    funtab *tmp_tab;
    node *new_node = NULL;
    int new_arg_info = 0;

    DBUG_ENTER ("DupTreeLUT");

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

        if (lut == NULL) {
            dup_lut = GenerateLUT ();
        } else {
            dup_lut = lut;
        }

        new_node = Trav (arg_node, arg_info);

        if (lut == NULL) {
            lut = dup_lut = RemoveLUT (dup_lut);
        }

        if (new_arg_info) {
            arg_info = FreeNode (arg_info);
        }

        act_tab = tmp_tab;
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupTree (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupTree");

    new_node = DupTreeLUT (arg_node, arg_info, NULL);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNodeLUT (node *arg_node, lut_t *lut)
{
    funtab *tmp_tab;
    node *arg_info;
    node *new_node = NULL;

    DBUG_ENTER ("DupNodeLUT");

    if (NULL != arg_node) {
        tmp_tab = act_tab;
        act_tab = dup_tab;

        arg_info = MakeInfo ();
        /*
         * duplicatation of sons of root 'arg_node' is controlled by
         *  DUPTRAV, DUPCONT
         */
        INFO_DUP_CONT (arg_info) = arg_node;

        if (lut == NULL) {
            dup_lut = GenerateLUT ();
        } else {
            dup_lut = lut;
        }

        new_node = Trav (arg_node, arg_info);

        if (lut == NULL) {
            lut = dup_lut = RemoveLUT (dup_lut);
        }

        FreeNode (arg_info);

        act_tab = tmp_tab;
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNode (node *arg_node)
{
    node *new_node;

    DBUG_ENTER ("DupTree");

    new_node = DupNodeLUT (arg_node, NULL);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupTreePre( node *arg_node, node *arg_info)
 *
 * description:
 *   This function is called before the traversal of each node.
 *
 ******************************************************************************/

node *
DupTreePre (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DupTreePre");

    DBUG_PRINT ("DUP", ("Duplicating - %s", NODE_TEXT (arg_node)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupTreePost( node *arg_node, node *arg_info)
 *
 * description:
 *   This function is called after the traversal of each node.
 *
 ******************************************************************************/

node *
DupTreePost (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DupTreePost");

    DBUG_RETURN (arg_node);
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

ids *
DupOneIds (ids *old_ids, node *arg_info)
{
    ids *new_ids;

    DBUG_ENTER ("DupOneIds");

    DBUG_ASSERT ((old_ids != NULL), "ids is NULL");

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
        IDS_VARDEC (new_ids) = SearchInLUT (dup_lut, IDS_VARDEC (old_ids));
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

    if (IDS_NEXT (old_ids) != NULL) {
        IDS_NEXT (new_ids) = DupIds (IDS_NEXT (old_ids), arg_info);
    }

    DBUG_RETURN (new_ids);
}

/******************************************************************************/

/*
 *
 *  functionname  : DuplicateTypes
 *  arguments     : 1) types node
 *                  2) completely unused
 *  description   : duplicates 1)
 *  global vars   :
 *  internal funs : StringCopy
 *  external funs :
 *  macros        : DBUG..., GEN_NODE, NULL
 *
 *  remarks       : IMPORTANT:
 *                  For new code the function DupTypes(<old_type>) defined
 *                  below should be used.
 *
 */

types *
DuplicateTypes (types *source, int share)
{
    types *return_types, *tmp;
    int i;

    DBUG_ENTER ("DuplicateTypes");

    if (source == NULL) {
        return_types = NULL;
    } else {
        tmp = GEN_NODE (types);
        if (tmp == NULL)
            SYSABORT (("Out of memory"));

        return_types = tmp;

        do {
            tmp->dim = source->dim;
            TYPES_BASETYPE (tmp) = TYPES_BASETYPE (source);
            if ((TYPES_DIM (source) > 0) && (TYPES_SHPSEG (source) != NULL)) {
                DBUG_ASSERT ((source->dim <= SHP_SEG_SIZE), "dimension out of range");
                tmp->shpseg = (shpseg *)Malloc (sizeof (shpseg));
                DBUG_ASSERT ((NULL != source->shpseg), "types-structur without shpseg");
                for (i = 0; i < source->dim; i++)
                    tmp->shpseg->shp[i] = source->shpseg->shp[i];
                tmp->shpseg->next = NULL;
            } else
                tmp->shpseg = NULL;

            tmp->id = StringCopy (source->id);
            tmp->name = StringCopy (source->name);

            DBUG_PRINT ("TYPE", ("new type" P_FORMAT ",old " P_FORMAT, tmp, source));
            DBUG_PRINT ("TYPE",
                        ("new id" P_FORMAT ", old id" P_FORMAT, tmp->id, source->id));
            DBUG_PRINT ("TYPE", ("new name" P_FORMAT ", old name" P_FORMAT, tmp->name,
                                 source->name));

            /*
             *  Sharing of module names is common throughout sac2c,
             *  so we can do it here as well.
             */

            tmp->id_mod = source->id_mod;
            tmp->name_mod = source->name_mod;
            tmp->id_cmod = source->id_cmod;
            tmp->attrib = source->attrib;
            tmp->status = source->status;
            TYPES_TDEF (tmp) = TYPES_TDEF (source);

            if (source->next == NULL)
                tmp->next = NULL;
            else {
                tmp->next = GEN_NODE (types);
                tmp = tmp->next;
            }
            source = source->next;
        } while (source != NULL);
    }

    DBUG_RETURN (return_types);
}

types *
DupTypes (types *source)
{
    DBUG_ENTER ("DupTypes");

    DBUG_RETURN (DuplicateTypes (source, 42));
}

/******************************************************************************/

node *
DupVinfo (node *arg_node, node *arg_info)
{
    node *new_node, *rest;

    DBUG_ENTER ("DupVinfo");

    rest = DUPCONT (VINFO_NEXT (arg_node));

    if (VINFO_FLAG (arg_node) == DOLLAR) {
        new_node = MakeVinfoDollar (rest);
    } else {
        new_node
          = MakeVinfo (VINFO_FLAG (arg_node), DuplicateTypes (VINFO_TYPE (arg_node), 1),
                       rest, VINFO_DOLLAR (rest));
    }
    VINFO_VARDEC (new_node) = VINFO_VARDEC (arg_node);

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNum (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNum");

    new_node = MakeNum (NUM_VAL (arg_node));

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupBool (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupBool");

    new_node = MakeBool (BOOL_VAL (arg_node));

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupFloat (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupFloat");

    new_node = MakeFloat (FLOAT_VAL (arg_node));

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupDouble (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupDouble");

    new_node = MakeDouble (DOUBLE_VAL (arg_node));

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupChar (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupChar");

    new_node = MakeChar (CHAR_VAL (arg_node));

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupStr (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupStr");

    new_node = MakeStr (StringCopy (STR_STRING (arg_node)));

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
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
        ID_VARDEC (new_node) = SearchInLUT (dup_lut, ID_VARDEC (arg_node));
    }
    ID_DEF (new_node) = SearchInLUT (dup_lut, ID_DEF (arg_node));

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

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupCast (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupCast");

    new_node = MakeCast (DUPTRAV (CAST_EXPR (arg_node)),
                         DuplicateTypes (CAST_TYPE (arg_node), 1));

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupReturn (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupReturn");

    new_node = MakeReturn (DUPTRAV (RETURN_EXPRS (arg_node)));
    RETURN_INWITH (new_node) = RETURN_INWITH (arg_node);

    RETURN_REFERENCE (new_node) = DUPTRAV (RETURN_REFERENCE (arg_node));

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    dup_lut = InsertIntoLUT (dup_lut, arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupBlock (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupBlock");

    new_node
      = MakeBlock (DUPTRAV (BLOCK_INSTR (arg_node)), DUPTRAV (BLOCK_VARDEC (arg_node)));
    BLOCK_CACHESIM (new_node) = StringCopy (BLOCK_CACHESIM (arg_node));

    BLOCK_VARNO (new_node) = BLOCK_VARNO (arg_node);
#if 0
  BLOCK_NEEDFUNS( new_node) = ???;
  BLOCK_NEEDTYPES( new_node) = ???;
  BLOCK_MASK( new_node, ?) = ???;
  BLOCK_SPMD_PROLOG_ICMS( new_node) = ???;
  BLOCK_SPMD_SETUP_ARGS( new_node) = ???;
#endif

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupTypedef (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupTypedef");

    new_node = MakeTypedef (StringCopy (TYPEDEF_NAME (arg_node)),
                            StringCopy (TYPEDEF_MOD (arg_node)),
                            DuplicateTypes (TYPEDEF_TYPE (arg_node), 1),
                            TYPEDEF_ATTRIB (arg_node), DUPCONT (TYPEDEF_NEXT (arg_node)));
    TYPEDEF_STATUS (new_node) = TYPEDEF_STATUS (arg_node);

#if 0
  TYPEDEF_IMPL( new_node) = ???;
  TYPEDEF_PRAGMA( new_node) = ???;
  TYPEDEF_COPYFUN( new_node) = ???;
  TYPEDEF_FREEFUN( new_node) = ???;
  TYPEDEC_DEF( new_node) = ???;
#endif

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupObjdef (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupObjdef");

    new_node
      = MakeObjdef (StringCopy (OBJDEF_NAME (arg_node)),
                    StringCopy (OBJDEF_MOD (arg_node)),
                    DuplicateTypes (OBJDEF_TYPE (arg_node), 1),
                    DUPTRAV (OBJDEF_EXPR (arg_node)), DUPCONT (OBJDEF_NEXT (arg_node)));
    OBJDEF_LINKMOD (new_node) = StringCopy (OBJDEF_LINKMOD (arg_node));
    OBJDEF_ATTRIB (new_node) = OBJDEF_ATTRIB (arg_node);
    OBJDEF_STATUS (new_node) = OBJDEF_STATUS (arg_node);

#if 0
  OBJDEF_VARNAME( new_node) = ???;
  OBJDEF_PRAGMA( new_node) = ???;
  OBJDEF_ARG( new_node) = ???;
  OBJDEF_ICM( new_node) = ???;
  OBJDEF_SIB( new_node) = ???;
  OBJDEF_NEEDOBJS( new_node) = ???;
  OBJDEC_DEF( new_node) = ???;
#endif

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupVardec (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupVardec");

    new_node = MakeVardec (StringCopy (VARDEC_NAME (arg_node)),
                           DuplicateTypes (VARDEC_TYPE (arg_node), 1),
                           DUPCONT (VARDEC_NEXT (arg_node)));
    VARDEC_STATUS (new_node) = VARDEC_STATUS (arg_node);

    VARDEC_ATTRIB (new_node) = VARDEC_ATTRIB (arg_node);
    VARDEC_VARNO (new_node) = VARDEC_VARNO (arg_node);
    VARDEC_REFCNT (new_node) = VARDEC_REFCNT (arg_node);
    VARDEC_NAIVE_REFCNT (new_node) = VARDEC_NAIVE_REFCNT (arg_node);
    VARDEC_FLAG (new_node) = VARDEC_FLAG (arg_node);
#if 0
  VARDEC_TYPEDEF( new_node) = ???;
  VARDEC_OBJDEF( new_node) = ???;
  VARDEC_ACTCHN( new_node) = ???;
  VARDEC_COLCHN( new_node) = ???;
#endif

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    dup_lut = InsertIntoLUT (dup_lut, arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupArg (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupArg");

    new_node = MakeArg (StringCopy (ARG_NAME (arg_node)),
                        DuplicateTypes (ARG_TYPE (arg_node), 1), ARG_STATUS (arg_node),
                        ARG_ATTRIB (arg_node), DUPCONT (ARG_NEXT (arg_node)));

    ARG_VARNO (new_node) = ARG_VARNO (arg_node);
    ARG_REFCNT (new_node) = ARG_REFCNT (arg_node);
    ARG_NAIVE_REFCNT (new_node) = ARG_NAIVE_REFCNT (arg_node);
#if 0
  ARG_TYPESTRING( new_node) = ???;
  ARG_OBJDEF( new_node) = ???;
  ARG_ACTCHN( new_node) = ???;
  ARG_COLCHN( new_node) = ???;
  ARG_FUNDEF( new_node) = ???;
#endif

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    dup_lut = InsertIntoLUT (dup_lut, arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupLet (node *arg_node, node *arg_info)
{
    node *new_node;
    ids *new_ids;

    DBUG_ENTER ("DupLet");

    if (LET_IDS (arg_node) != NULL) {
        new_ids = DupIds (LET_IDS (arg_node), arg_info);
    } else {
        new_ids = NULL;
    }

    new_node = MakeLet (DUPTRAV (LET_EXPR (arg_node)), new_ids);

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupArray (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupArray");

    new_node = MakeArray (DUPTRAV (ARRAY_AELEMS (arg_node)));
    ARRAY_STRING (new_node) = StringCopy (ARRAY_STRING (arg_node));

    ARRAY_TYPE (new_node) = DuplicateTypes (ARRAY_TYPE (arg_node), 1);

    ARRAY_ISCONST (new_node) = ARRAY_ISCONST (arg_node);
    ARRAY_VECLEN (new_node) = ARRAY_VECLEN (arg_node);
    ARRAY_VECTYPE (new_node) = ARRAY_VECTYPE (arg_node);
    if (ARRAY_ISCONST (new_node)) {
        ARRAY_CONSTVEC (new_node)
          = CopyConstVec (ARRAY_VECTYPE (arg_node), ARRAY_VECLEN (arg_node),
                          ARRAY_CONSTVEC (arg_node));
    }

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

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

    if (COND_THENVARS (arg_node) != NULL) {
        COND_THENVARS (new_node) = DupIds (COND_THENVARS (arg_node), arg_info);
    }
    if (COND_ELSEVARS (arg_node) != NULL) {
        COND_ELSEVARS (new_node) = DupIds (COND_ELSEVARS (arg_node), arg_info);
    }
#if 0
  COND_MASK( new_node, ?) = ???;
#endif

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupDo (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupDo");

    new_node = MakeDo (DUPTRAV (DO_COND (arg_node)), DUPTRAV (DO_BODY (arg_node)));

    if (DO_USEVARS (arg_node) != NULL) {
        DO_USEVARS (new_node) = DupIds (DO_USEVARS (arg_node), arg_info);
    }
    if (DO_DEFVARS (arg_node) != NULL) {
        DO_DEFVARS (new_node) = DupIds (DO_DEFVARS (arg_node), arg_info);
    }
#if 0
  DO_MASK( new_node, ?) = ???;
#endif

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWhile (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupWhile");

    new_node
      = MakeWhile (DUPTRAV (WHILE_COND (arg_node)), DUPTRAV (WHILE_BODY (arg_node)));

    if (WHILE_USEVARS (arg_node) != NULL) {
        WHILE_USEVARS (new_node) = DupIds (WHILE_USEVARS (arg_node), arg_info);
    }
    if (WHILE_DEFVARS (arg_node) != NULL) {
        WHILE_DEFVARS (new_node) = DupIds (WHILE_DEFVARS (arg_node), arg_info);
    }
#if 0
  WHILE_MASK( new_node, ?) = ???;
#endif

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

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

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

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

        ASSIGN_STATUS (new_node) = ASSIGN_STATUS (arg_node);
#if 0
      ASSIGN_MASK( new_node, ?) = ???;
      ASSIGN_CSE( new_node) = ???;
      ASSIGN_CF( new_node) = ???;
      ASSIGN_INDEX( new_node) = ???;
#endif
        ASSIGN_LEVEL (new_node) = ASSIGN_LEVEL (arg_node);

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

        NODE_LINE (new_node) = NODE_LINE (arg_node);
        NODE_FILE (new_node) = NODE_FILE (arg_node);
        break;
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupEmpty (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupEmpty");

    new_node = MakeEmpty ();

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupPrf (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupPrf");

    new_node = MakePrf (PRF_PRF (arg_node), DUPTRAV (PRF_ARGS (arg_node)));

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupAp (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupAp");

    new_node = MakeAp (StringCopy (AP_NAME (arg_node)), StringCopy (AP_MOD (arg_node)),
                       DUPTRAV (AP_ARGS (arg_node)));
    AP_ATFLAG (new_node) = AP_ATFLAG (arg_node);

    AP_FUNDEF (new_node) = SearchInLUT (dup_lut, AP_FUNDEF (arg_node));

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupFundef (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupFundef");

    new_node
      = MakeFundef (StringCopy (FUNDEF_NAME (arg_node)),
                    StringCopy (FUNDEF_MOD (arg_node)),
                    DuplicateTypes (FUNDEF_TYPES (arg_node), 1),
                    DUPTRAV (FUNDEF_ARGS (arg_node)), DUPTRAV (FUNDEF_BODY (arg_node)),
                    DUPCONT (FUNDEF_NEXT (arg_node)));
    FUNDEF_LINKMOD (new_node) = StringCopy (FUNDEF_LINKMOD (arg_node));
    FUNDEF_STATUS (new_node) = FUNDEF_STATUS (arg_node);
    FUNDEF_ATTRIB (new_node) = FUNDEF_ATTRIB (arg_node);
    FUNDEF_INLINE (new_node) = FUNDEF_INLINE (arg_node);
    FUNDEF_FUNNO (new_node) = FUNDEF_FUNNO (arg_node);
    FUNDEF_PRAGMA (new_node) = DUPTRAV (FUNDEF_PRAGMA (arg_node));

    FUNDEF_RETURN (new_node) = SearchInLUT (dup_lut, FUNDEF_RETURN (arg_node));
    FUNDEF_NEEDOBJS (new_node) = CopyNodelist (FUNDEF_NEEDOBJS (arg_node));
    FUNDEF_VARNO (new_node) = FUNDEF_VARNO (arg_node);
    FUNDEF_INLREC (new_node) = FUNDEF_INLREC (arg_node);

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_NEEDFUNS (new_node) = CopyNodelist (FUNDEF_NEEDFUNS (arg_node));
        FUNDEF_NEEDTYPES (new_node) = CopyNodelist (FUNDEF_NEEDTYPES (arg_node));
    }

#if 0
  FUNDEF_SIB( new_node) = ???;
  FUNDEF_ICM( new_node) = ???;
  FUNDEF_MASK( new_node, ?) = ???;
  FUNDEF_DFM_BASE( new_node) = ???;
  FUNDEF_LIFTEDFROM( new_node) = ???;
  FUNDEC_DEF( new_node) = ???;
#endif

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    dup_lut = InsertIntoLUT (dup_lut, arg_node, new_node);

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

    PRAGMA_LINKNAME (new_node) = StringCopy (PRAGMA_LINKNAME (arg_node));
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
    PRAGMA_INITFUN (new_node) = StringCopy (PRAGMA_INITFUN (arg_node));
    PRAGMA_WLCOMP_APS (new_node) = DUPTRAV (PRAGMA_WLCOMP_APS (arg_node));

    if (PRAGMA_EFFECT (arg_node) != NULL) {
        PRAGMA_EFFECT (new_node) = DupIds (PRAGMA_EFFECT (arg_node), arg_info);
    }
    if (PRAGMA_TOUCH (arg_node) != NULL) {
        PRAGMA_TOUCH (new_node) = DupIds (PRAGMA_TOUCH (arg_node), arg_info);
    }
    PRAGMA_COPYFUN (new_node) = StringCopy (PRAGMA_COPYFUN (arg_node));
    PRAGMA_FREEFUN (new_node) = StringCopy (PRAGMA_FREEFUN (arg_node));
    PRAGMA_LINKMOD (new_node) = StringCopy (PRAGMA_LINKMOD (arg_node));
    PRAGMA_NUMPARAMS (new_node) = PRAGMA_NUMPARAMS (arg_node);
#if 0
  PRAGMA_READONLY( new_node) = ???;
  PRAGMA_NEEDTYPES( new_node) = ???;
  PRAGMA_NEEDFUNS( new_node) = ???;
  PRAGMA_LINKSIGNNUMS( new_node) = ???;
  PRAGMA_REFCOUNTINGNUMS( new_node) = ???;
  PRAGMA_READONLYNUMS( new_node) = ???;
#endif

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

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
    ICM_INDENT (new_node) = ICM_INDENT (arg_node);

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
    NWITH_PRAGMA (new_node) = DUPTRAV (NWITH_PRAGMA (arg_node));
    NWITH_PARTS (new_node) = NWITH_PARTS (arg_node);
    NWITH_REFERENCED (new_node) = NWITH_REFERENCED (arg_node);
    NWITH_REFERENCED_FOLD (new_node) = NWITH_REFERENCED_FOLD (arg_node);
    NWITH_REFERENCES_FOLDED (new_node) = NWITH_REFERENCES_FOLDED (arg_node);
    NWITH_COMPLEX (new_node) = NWITH_COMPLEX (arg_node);
    NWITH_FOLDABLE (new_node) = NWITH_FOLDABLE (arg_node);
    NWITH_NO_CHANCE (new_node) = NWITH_NO_CHANCE (arg_node);
#if 0
  NWITH_TSI( new_node) = ???;
#endif

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

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

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

#if 0
  NWITHOP_MASK( new_node, ?) = ???;
#endif

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
                   SearchInLUT (dup_lut, NPART_CODE (arg_node)));

    NPART_NEXT (new_node) = DUPCONT (NPART_NEXT (arg_node));

#if 0
  NPART_MASK( new_node, ?) = ???;
#endif

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
#if 0
  NCODE_MASK( new_node, ?) = ???;
#endif

    dup_lut = InsertIntoLUT (dup_lut, arg_node, new_node);

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

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

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
                    SearchInLUT (dup_lut, WLGRID_CODE (arg_node)));

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
                              SearchInLUT (dup_lut, WLGRIDVAR_CODE (arg_node)));

    DBUG_RETURN (new_node);
}
