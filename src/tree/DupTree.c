/*
 *
 * $Log$
 * Revision 3.70  2002/08/15 11:47:39  dkr
 * type LUT_t* replaced by LUT_t
 *
 * Revision 3.69  2002/08/13 13:43:28  dkr
 * interface to LookUpTable.[ch] modified
 *
 * Revision 3.68  2002/08/12 14:58:52  sbs
 * N_mop representation changed
 *
 * Revision 3.67  2002/08/09 16:36:21  sbs
 * basic support for N_mop written.
 *
 * Revision 3.66  2002/07/03 16:55:16  dkr
 * ID_UNQCONV removed for TAGGED_ARRAYS
 *
 * Revision 3.65  2002/07/02 09:27:59  dkr
 * DupExprs_NT() moved from compile.tagged.c to DupTree.c
 *
 * Revision 3.64  2002/07/02 09:20:29  dkr
 * DupNode_NT() added
 *
 * Revision 3.63  2002/06/25 14:01:53  sbs
 * DupDot added.
 *
 * Revision 3.62  2002/06/20 15:23:25  dkr
 * signature of MakeNWithOp modified
 *
 * Revision 3.61  2002/06/02 21:47:23  dkr
 * ID_NT_TAG modified
 *
 * Revision 3.60  2002/04/09 16:39:20  dkr
 * okay... *now* DupFundef() should work correctly $%&-(
 *
 * Revision 3.59  2002/04/08 20:03:51  dkr
 * bug in DupFundef() fixed
 *
 * Revision 3.58  2002/03/07 02:20:12  dkr
 * duplication of AP_ARGTAB and FUNDEF_ARGTAB added
 *
 * Revision 3.57  2002/03/01 02:40:09  dkr
 * DupFundef() and DupAp() modified: ARGTAB is duplicated now
 *
 * Revision 3.56  2002/02/22 14:27:53  dkr
 * functions Dup...TypesOnly(), DupOneTypesOnly_Inplace() removed
 *
 * Revision 3.55  2002/02/20 16:09:31  dkr
 * function DupOneTypesOnly_Inplace() added
 *
 * Revision 3.54  2002/02/20 15:02:11  dkr
 * fundef DupTypes() renamed into DupAllTypes()
 * fundef DupTypesOnly() renamed into DupAllTypesOnly()
 * fundefs DupOneTypes() and DupOneTypesOnly() added
 *
 * Revision 3.53  2001/12/12 11:14:39  dkr
 * functions DupIds_Id_NT, DupId_NT added
 *
 * Revision 3.52  2001/06/27 12:38:37  ben
 * SCHCopyTasksel inserted
 *
 * Revision 3.51  2001/05/18 11:40:31  dkr
 * DBUG_ASSERT containing IsEmptyLUT() added
 *
 * Revision 3.50  2001/05/17 11:37:24  dkr
 * InitDupTree() added
 * FREE/MALLOC eliminated
 *
 * Revision 3.49  2001/05/16 13:17:37  dkr
 * handling of NULL arguments streamlined:
 * NULL arguments allowed for static ..._() functions but *not* allowed
 * for exported functions.
 *
 * Revision 3.48  2001/05/03 17:26:20  dkr
 * MAXHOMDIM replaced by HOMSV
 *
 * Revision 3.47  2001/05/03 16:50:08  nmw
 * increment fundef used counter in DupIcm for ND_AP icm
 *
 * Revision 3.46  2001/04/26 21:06:21  dkr
 * DupTypesOnly() added
 *
 * Revision 3.45  2001/04/26 11:55:56  nmw
 * ICM_FUNDEF attribute added to DupIcm
 *
 * Revision 3.44  2001/04/26 01:50:04  dkr
 * - reference counnting on fundefs works correctly now (FUNDEF_USED)
 * - DFMs are never duplicated now!!
 *
 * Revision 3.43  2001/04/24 14:15:01  dkr
 * some DBUG_ASSERTs about FUNDEF_USED added
 *
 * Revision 3.42  2001/04/24 09:16:21  dkr
 * P_FORMAT replaced by F_PTR
 *
 * Revision 3.41  2001/04/06 18:46:17  dkr
 * - DupAvis: call of DupTree replaced by DUPTRAV.
 * - The LUT is no longer allocated and removed for each call of
 *   DupTree/DupNode. Instead, we generate *once* a single LUT and
 *   reuse it for further calls.
 *
 * [ ... ]
 *
 */

/******************************************************************************
 *
 * file  : DupTree.c
 *
 * PREFIX: Dup
 *
 * description:
 *   Traversal for duplication of nodes and trees.
 *
 * flags for some special behaviour ('type'):
 *   DUP_NORMAL : no special behaviour
 *   DUP_INLINE : do not duplicate N_assign nodes which contain a N_return
 *   DUP_WLF    : set ID_WL
 *
 * CAUTION:
 *   Do *NOT* call the external functions Dup...() within a DUP-traversal!!!!
 *   For duplicating nodes DUPTRAV should be used and for duplicating non-node
 *   structures appropriate Dup..._() functions are provided!
 *
 *   When adding new functions: Please name top-level functions Dup...()
 *   *without* trailing '_' and name static functions Dup..._() *with*
 *   trailing '_'.
 *
 ******************************************************************************/

#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"

#include "DupTree.h"
#include "NameTuplesUtils.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "LookUpTable.h"
#include "scheduling.h"
#include "generatemasks.h"
#include "constants.h"

/*
 * DFMs are *not* duplicated for several reasons:
 *  - Most likely the old DFMs are not suitable for the new context.
 *  - Only valid DFMs can be duplicated. Therefore, duplicating the
 *    DFMs requires them to be valid during all compilation phases.
 *    For the time being this cannot be guaranteed.
 */
#define DUP_DFMS 0

static LUT_t dup_lut = NULL;

/*
 * always traverses son 'node'
 *
 * The macro is to be used within traversal functions where arg_node and
 * arg_info exist.
 */
#define DUPTRAV(node) ((node) != NULL) ? Trav (node, arg_info) : NULL

/*
 * If INFO_DUP_CONT contains the root of syntaxtree
 *   -> traverses son 'node' if and only if its parent is not the root
 * If INFO_DUP_CONT is NULL
 *   -> traverses son 'node'
 *
 * The macro is to be used within traversal functions where arg_node and
 * arg_info exist.
 */
#define DUPCONT(node) (INFO_DUP_CONT (arg_info) != arg_node) ? DUPTRAV (node) : NULL

/******************************************************************************
 *
 * Function:
 *   void InitDupTree()
 *
 * Description:
 *
 *
 ******************************************************************************/

void
InitDupTree ()
{
    DBUG_ENTER ("InitDupTree");

    DBUG_ASSERT ((dup_lut == NULL), "InitDupTree() called more than once!");
    dup_lut = GenerateLUT ();

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   static node *DupTreeOrNodeLUT_Type( int NodeOnly,
 *                                       node *arg_node, LUT_t lut, int type)
 *
 * description:
 *   This routine starts a duplication-traversal, it duplicates a whole sub
 *   tree or one node only (that means all of this node, but not the xxx_NEXT).
 *   The start of the duplication is at arg_node, either the subtree starting
 *   from this node or the node only is copied.
 *
 * parameters:
 *   - NodeOnly:
 *       FALSE : duplicate whole subtree
 *       TRUE  : duplicate node only
 *   - arg_node:
 *       starting point of duplication
 *   - lut:
 *       If you want to use your own LUT you can hand over it here.
 *   - type:
 *       value for INFO_DUP_TYPE.
 *
 ******************************************************************************/

static node *
DupTreeOrNodeLUT_Type (int NodeOnly, node *arg_node, LUT_t lut, int type)
{
    funtab *old_tab;
    node *arg_info;
    node *new_node = NULL;

    DBUG_ENTER ("DupTreeOrNodeLUT_Type");

    if (arg_node != NULL) {
        old_tab = act_tab;
        act_tab = dup_tab;
        arg_info = MakeInfo ();

        INFO_DUP_TYPE (arg_info) = type;
        INFO_DUP_ASSIGN (arg_info) = NULL;
        INFO_DUP_FUNDEF (arg_info) = NULL;

        /*
         * Via this (ugly) macro DUPCONT the decision to copy the whole tree
         * starting from arg_node or only the node itself (meaning not to
         * traverse and copy xxx_NEXT) is done.
         * DUPCONT compares the actual arg_node of a traversal function with the
         * value in INFO_DUP_CONT. If they are the same the xxx_NEXT will be
         * ignored, otherwise it will be traversed. If the start-node is stored as
         * INFO_DUP_CONT it's xx_NEXT will not be duplicated, but the xxx_NEXT's
         * of all sons are copied, because they differ from INFO_DUP_CONT.
         * If NULL is stored in INFO_DUP_CONT (and in a traversal the arg_node
         * never is NULL) all nodes and their xxx_NEXT's are duplicated.
         * So we set INFO_DUP_CONT with NULL to copy all, arg_node to copy
         * start_node (as decribed above) only.
         */
        if (NodeOnly) {
            INFO_DUP_CONT (arg_info) = arg_node;
        } else {
            INFO_DUP_CONT (arg_info) = NULL;
        }

        if (lut == NULL) {
            /*
             * It would be extremly unefficient to generate a new LUT for each call
             * of DupTree/DupNode.
             * Therefore, we just generate it *once* via InitDupTree() !!
             */
            DBUG_ASSERT ((dup_lut != NULL), "InitDupTree() has not been called!");
            DBUG_ASSERT ((IsEmptyLUT (dup_lut)), "LUT for DupTree is not empty!");
            INFO_DUP_LUT (arg_info) = dup_lut;
        } else {
            INFO_DUP_LUT (arg_info) = lut;
        }

        new_node = Trav (arg_node, arg_info);

        if (lut == NULL) {
            /*
             * Here, we just remove the content of the LUT but *not* the LUT itself.
             * Guess what: Most likely we will need the LUT again soon ;-)
             */
            dup_lut = RemoveContentLUT (dup_lut);
        }

        arg_info = FreeNode (arg_info);
        act_tab = old_tab;
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupTreeTravPre( node *arg_node, node *arg_info)
 *
 * description:
 *   This function is called before the traversal of each node.
 *
 ******************************************************************************/

node *
DupTreeTravPre (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DupTreeTravPre");

    DBUG_PRINT ("DUP", ("Duplicating - %s", NODE_TEXT (arg_node)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupTreeTravPost( node *arg_node, node *arg_info)
 *
 * description:
 *   This function is called after the traversal of each node.
 *
 ******************************************************************************/

node *
DupTreeTravPost (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DupTreeTravPost");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   void CopyCommonNodeData( node *new_node, node *old_node)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
CopyCommonNodeData (node *new_node, node *old_node)
{
    DBUG_ENTER ("CopyCommonNodeData");

    NODE_LINE (new_node) = NODE_LINE (old_node);
    NODE_FILE (new_node) = NODE_FILE (old_node);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   DFMmask_t DupDFMask_( DFMmask_t mask, node *arg_info)
 *
 * Description:
 *   Duplicates the given DFMmask.
 *   The real duplication is done by DFMDuplicateMask:
 *   If a new DFMbase is found in the LUT the new one is used,
 *   otherwise the old one (this is done by the LUTmechanismi, called
 *   within this function).
 *
 ******************************************************************************/

static DFMmask_t
DupDFMask_ (DFMmask_t mask, node *arg_info)
{
    DFMmask_t new_mask;

    DBUG_ENTER ("DupDFMask_");

#if DUP_DFMS
    if (mask != NULL) {
        new_mask = DFMDuplicateMask (mask, SearchInLUT_PP (INFO_DUP_LUT (arg_info),
                                                           DFMGetMaskBase (mask)));
    } else {
        new_mask = NULL;
    }
#else
    new_mask = NULL;
#endif

    DBUG_RETURN (new_mask);
}

/******************************************************************************
 *
 * Function:
 *   ids *DupIds_( ids *arg_ids, node *arg_info)
 *
 * Remark:
 *   'arg_info' might be NULL, because this function is not only used by
 *   the traversal mechanism but also by Dup...Ids()!
 *
 ******************************************************************************/

static ids *
DupIds_ (ids *arg_ids, node *arg_info)
{
    ids *new_ids;
    char *new_name;

    DBUG_ENTER ("DupIds_");

    if (arg_ids != NULL) {
        LUT_t lut = (arg_info != NULL) ? INFO_DUP_LUT (arg_info) : NULL;

        new_name = SearchInLUT_SS (lut, IDS_NAME (arg_ids));
        new_ids = MakeIds (StringCopy (new_name), StringCopy (IDS_MOD (arg_ids)),
                           IDS_STATUS (arg_ids));

        IDS_VARDEC (new_ids) = SearchInLUT_PP (lut, IDS_VARDEC (arg_ids));
        IDS_USE (new_ids) = IDS_USE (arg_ids);

        IDS_ATTRIB (new_ids) = IDS_ATTRIB (arg_ids);
        IDS_REFCNT (new_ids) = IDS_REFCNT (arg_ids);
        IDS_NAIVE_REFCNT (new_ids) = IDS_NAIVE_REFCNT (arg_ids);

        /* set corresponding AVIS node backreference */
        if (IDS_VARDEC (arg_ids) != NULL) {
            IDS_AVIS (new_ids) = VARDEC_OR_ARG_AVIS (IDS_VARDEC (new_ids));
        } else {
            IDS_AVIS (new_ids) = NULL;
        }

        IDS_NEXT (new_ids) = DupIds_ (IDS_NEXT (arg_ids), arg_info);

        if (arg_info != NULL) {
            INFO_DUP_LUT (arg_info)
              = InsertIntoLUT_P (INFO_DUP_LUT (arg_info), arg_ids, new_ids);
        }
    } else {
        new_ids = NULL;
    }

    DBUG_RETURN (new_ids);
}

/******************************************************************************
 *
 * Function:
 *   shpseg *DupShpseg_( shpseg *arg_shpseg, node *arg_info)
 *
 * Remark:
 *   'arg_info' might be NULL, because this function is not only used by
 *   the traversal mechanism but also by DupShpseg()!
 *
 ******************************************************************************/

static shpseg *
DupShpseg_ (shpseg *arg_shpseg, node *arg_info)
{
    int i;
    shpseg *new_shpseg;

    DBUG_ENTER ("DupShpseg_");

    if (arg_shpseg != NULL) {
        new_shpseg = MakeShpseg (NULL);
        for (i = 0; i < SHP_SEG_SIZE; i++) {
            SHPSEG_SHAPE (new_shpseg, i) = SHPSEG_SHAPE (arg_shpseg, i);
        }

        SHPSEG_NEXT (new_shpseg) = DupShpseg_ (SHPSEG_NEXT (arg_shpseg), arg_info);
    } else {
        new_shpseg = NULL;
    }

    DBUG_RETURN (new_shpseg);
}

/******************************************************************************
 *
 * Function:
 *   types *DupTypes_( types* source, node *arg_info)
 *
 * Remark:
 *   'arg_info' might be NULL, because this function is not only used by
 *   the traversal mechanism but also by Dup...Types()!
 *
 ******************************************************************************/

static types *
DupTypes_ (types *arg_types, node *arg_info)
{
    types *new_types;

    DBUG_ENTER ("DupTypes_");

    if (arg_types != NULL) {
        new_types = MakeTypes (TYPES_BASETYPE (arg_types), TYPES_DIM (arg_types),
                               DupShpseg_ (TYPES_SHPSEG (arg_types), arg_info),
                               StringCopy (TYPES_NAME (arg_types)),
                               StringCopy (TYPES_MOD (arg_types)));

        TYPES_TDEF (new_types) = TYPES_TDEF (arg_types);
        TYPES_STATUS (new_types) = TYPES_STATUS (arg_types);

        DBUG_PRINT ("TYPE", ("new type" F_PTR ",old " F_PTR, new_types, arg_types));
        DBUG_PRINT ("TYPE", ("new name" F_PTR ", old name" F_PTR, TYPES_NAME (new_types),
                             TYPES_NAME (arg_types)));

        TYPES_NEXT (new_types) = DupTypes_ (TYPES_NEXT (arg_types), arg_info);

        if (arg_info != NULL) {
            INFO_DUP_LUT (arg_info)
              = InsertIntoLUT_P (INFO_DUP_LUT (arg_info), arg_types, new_types);
        }
    } else {
        new_types = NULL;
    }

    DBUG_RETURN (new_types);
}

/******************************************************************************
 *
 * Function:
 *   nodelist *DupNodelist_( nodelist *nl, node *arg_info)
 *
 * Remark:
 *   'arg_info' might be NULL, because this function is not only used by
 *   the traversal mechanism but also by DupNodelist()!
 *
 ******************************************************************************/

static nodelist *
DupNodelist_ (nodelist *nl, node *arg_info)
{
    nodelist *new_nl;

    DBUG_ENTER ("DupNodelist_");

    if (nl != NULL) {
        new_nl
          = MakeNodelist (SearchInLUT_PP (INFO_DUP_LUT (arg_info), NODELIST_NODE (nl)),
                          NODELIST_STATUS (nl),
                          DupNodelist_ (NODELIST_NEXT (nl), arg_info));
        NODELIST_ATTRIB (new_nl) = NODELIST_ATTRIB (nl);
    } else {
        new_nl = NULL;
    }

    DBUG_RETURN (new_nl);
}

/******************************************************************************
 *
 * Function:
 *   argtab_t *DupArgtab_( argtab_t *argtab, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static argtab_t *
DupArgtab_ (argtab_t *argtab, node *arg_info)
{
    argtab_t *new_argtab;
    int i;

    DBUG_ENTER ("DupArgtab_");

    if (argtab != NULL) {
        new_argtab = MakeArgtab (argtab->size);

        for (i = 0; i < argtab->size; i++) {
            new_argtab->tag[i] = argtab->tag[i];
            new_argtab->ptr_in[i]
              = (argtab->ptr_in[i] != NULL)
                  ? SearchInLUT_PP (INFO_DUP_LUT (arg_info), argtab->ptr_in[i])
                  : NULL;
            new_argtab->ptr_out[i]
              = (argtab->ptr_out[i] != NULL)
                  ? SearchInLUT_PP (INFO_DUP_LUT (arg_info), argtab->ptr_out[i])
                  : NULL;
        }
    } else {
        new_argtab = NULL;
    }

    DBUG_RETURN (new_argtab);
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
          = MakeVinfo (VINFO_FLAG (arg_node), DupTypes_ (VINFO_TYPE (arg_node), arg_info),
                       rest, VINFO_DOLLAR (rest));
    }
    VINFO_VARDEC (new_node) = VINFO_VARDEC (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNum (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNum");

    new_node = MakeNum (NUM_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupBool (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupBool");

    new_node = MakeBool (BOOL_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupFloat (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupFloat");

    new_node = MakeFloat (FLOAT_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupDouble (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupDouble");

    new_node = MakeDouble (DOUBLE_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupChar (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupChar");

    new_node = MakeChar (CHAR_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupStr (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupStr");

    new_node = MakeStr (StringCopy (STR_STRING (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupDot (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupDot");

    new_node = MakeDot (DOT_NUM (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupId (node *arg_node, node *arg_info)
{
    node *new_node;
    char *new_name;

    DBUG_ENTER ("DupId");

    new_name = SearchInLUT_SS (INFO_DUP_LUT (arg_info), ID_NAME (arg_node));
    new_node = MakeId (StringCopy (new_name), StringCopy (ID_MOD (arg_node)),
                       ID_STATUS (arg_node));

    /* ID_OBJDEF and ID_VARDEC are mapped to the same data object */
    ID_VARDEC (new_node) = SearchInLUT_PP (INFO_DUP_LUT (arg_info), ID_VARDEC (arg_node));

    ID_DEF (new_node) = SearchInLUT_PP (INFO_DUP_LUT (arg_info), ID_DEF (arg_node));

    ID_ATTRIB (new_node) = ID_ATTRIB (arg_node);
    ID_REFCNT (new_node) = ID_REFCNT (arg_node);
    ID_NAIVE_REFCNT (new_node) = ID_NAIVE_REFCNT (arg_node);
#ifndef TAGGED_ARRAYS
    ID_UNQCONV (new_node) = ID_UNQCONV (arg_node);
#endif

    ID_AVIS (new_node) = SearchInLUT_PP (INFO_DUP_LUT (arg_info), ID_AVIS (arg_node));

    if (INFO_DUP_TYPE (arg_info) == DUP_WLF) {
        /* Withloop folding (wlf) needs this. */
        if (ID_WL (arg_node) && (NODE_TYPE (ID_WL (arg_node)) == N_id)) {
            /* new code in new_codes, see 'usage of ID_WL' in WLF.c for more infos */
            ID_WL (new_node) = ID_WL (arg_node);
        } else {
            ID_WL (new_node) = arg_node; /* original code */
        }
    }

    /*
     * Coping the attibutes of constantvectors.
     * CONSTVEC itself can only be copied, if ISCONST flag is set,
     * otherwise VECTYPE might be T_unkown.
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

    if (ID_NT_TAG (arg_node) != NULL) {
        ID_NT_TAG (new_node) = StringCopy (ID_NT_TAG (arg_node));
    }

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupCast (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupCast");

    new_node = MakeCast (DUPTRAV (CAST_EXPR (arg_node)),
                         DupTypes_ (CAST_TYPE (arg_node), arg_info));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupModul (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupModul");

    new_node
      = MakeModul (StringCopy (MODUL_NAME (arg_node)), MODUL_FILETYPE (arg_node),
                   DUPTRAV (MODUL_IMPORTS (arg_node)), DUPTRAV (MODUL_TYPES (arg_node)),
                   DUPTRAV (MODUL_OBJS (arg_node)), DUPTRAV (MODUL_FUNS (arg_node)));

    MODUL_CLASSTYPE (new_node) = DupTypes_ (MODUL_CLASSTYPE (arg_node), arg_info);

#if 0
  MODUL_DECL( new_node) = ???
  MODUL_STORE_IMPORTS( new_node) = ???
  MODUL_FOLDFUNS( new_node) = ???
#endif

    CopyCommonNodeData (new_node, arg_node);

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
                            DupTypes_ (TYPEDEF_TYPE (arg_node), arg_info),
                            TYPEDEF_ATTRIB (arg_node), DUPCONT (TYPEDEF_NEXT (arg_node)));

    TYPEDEF_STATUS (new_node) = TYPEDEF_STATUS (arg_node);

#if 0
  TYPEDEF_IMPL( new_node) = ???;
  TYPEDEF_PRAGMA( new_node) = ???;
  TYPEDEF_COPYFUN( new_node) = ???;
  TYPEDEF_FREEFUN( new_node) = ???;
  TYPEDEC_DEF( new_node) = ???;
#endif

    CopyCommonNodeData (new_node, arg_node);

    INFO_DUP_LUT (arg_info)
      = InsertIntoLUT_P (INFO_DUP_LUT (arg_info), arg_node, new_node);

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
                    DupTypes_ (OBJDEF_TYPE (arg_node), arg_info),
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

    CopyCommonNodeData (new_node, arg_node);

    INFO_DUP_LUT (arg_info)
      = InsertIntoLUT_P (INFO_DUP_LUT (arg_info), arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupFundef (node *arg_node, node *arg_info)
{
    node *new_node, *old_fundef, *new_ssacnt;

    DBUG_ENTER ("DupFundef");

    /*
     * DUP_INLINE
     *  -> N_return nodes are not duplicated
     *  -> would result in an illegal N_fundef node
     *  -> stop here!
     */
    DBUG_ASSERT ((INFO_DUP_TYPE (arg_info) != DUP_INLINE),
                 "N_fundef node can't be duplicated in DUP_INLINE mode!");

    DBUG_PRINT ("DUP", ("start dubbing of fundef %s", FUNDEF_NAME (arg_node)));

    /*
     * We can't copy the FUNDEF_DFM_BASE and DFMmasks belonging to this base
     * directly!
     * Such DFMmasks are attached to N_with, N_with2, N_sync and N_spmd.
     * All of them can be found in the body of the function.
     * But when we copy the body we must already know the base to create the
     * new masks. On the other hand to create the base we must already have
     * the new FUNDEF_ARGS and FUNDEF_VARDECS available.
     * Therefore we first create the raw function without the body via
     * MakeFundef(), then we create the base while duplicating the body
     * and finally we attach the body to the fundef.
     */

    /*
     * INFO_DUP_FUNDEF is a pointer to the OLD fundef node!!
     * We can get the pointer to the NEW fundef via the LUT!
     */
    old_fundef = INFO_DUP_FUNDEF (arg_info);
    INFO_DUP_FUNDEF (arg_info) = arg_node;

    new_node = MakeFundef (StringCopy (FUNDEF_NAME (arg_node)),
                           StringCopy (FUNDEF_MOD (arg_node)),
                           DupTypes_ (FUNDEF_TYPES (arg_node), arg_info),
                           NULL, /* must be duplicated later on */
                           NULL, /* must be duplicated later on */
                           NULL);

    /* now we copy all the other things ... */
    FUNDEF_LINKMOD (new_node) = StringCopy (FUNDEF_LINKMOD (arg_node));
    FUNDEF_STATUS (new_node) = FUNDEF_STATUS (arg_node);
    FUNDEF_ATTRIB (new_node) = FUNDEF_ATTRIB (arg_node);
    FUNDEF_INLINE (new_node) = FUNDEF_INLINE (arg_node);
    FUNDEF_FUNNO (new_node) = FUNDEF_FUNNO (arg_node);
    FUNDEF_PRAGMA (new_node) = DUPTRAV (FUNDEF_PRAGMA (arg_node));
    FUNDEF_NEEDOBJS (new_node) = DupNodelist_ (FUNDEF_NEEDOBJS (arg_node), arg_info);
    FUNDEF_VARNO (new_node) = FUNDEF_VARNO (arg_node);

#if 0
  FUNDEF_SIB( new_node) = ???;
  FUNDEF_ICM( new_node) = ???;
  FUNDEC_DEF( new_node) = ???;
  FUNDEF_MASK( new_node, ?) = ???;
#endif

    CopyCommonNodeData (new_node, arg_node);

    FUNDEF_NEXT (new_node) = DUPCONT (FUNDEF_NEXT (arg_node));

    /*
     * must be done before traversal of BODY
     */
    if (FUNDEF_IS_LACFUN (new_node)) {
        FUNDEF_USED (new_node) = 0;
        FUNDEF_EXT_ASSIGNS (new_node) = NULL;
    }

    /*
     * before duplicating ARGS or VARDEC (in BODY) we have to duplicate
     * SSACOUNTER (located in the top-block, but traversed here)
     */
    if ((FUNDEF_BODY (arg_node) != NULL)
        && (BLOCK_SSACOUNTER (FUNDEF_BODY (arg_node)) != NULL)) {
        new_ssacnt = DUPTRAV (BLOCK_SSACOUNTER (FUNDEF_BODY (arg_node)));
    } else {
        new_ssacnt = NULL;
    }

    INFO_DUP_LUT (arg_info)
      = InsertIntoLUT_P (INFO_DUP_LUT (arg_info), arg_node, new_node);

    FUNDEF_ARGS (new_node) = DUPTRAV (FUNDEF_ARGS (arg_node));
    FUNDEF_BODY (new_node) = DUPTRAV (FUNDEF_BODY (arg_node));

    /*
     * ARGTAB must be duplicated *after* TYPES and ARGS!!!
     */
    FUNDEF_ARGTAB (new_node) = DupArgtab_ (FUNDEF_ARGTAB (arg_node), arg_info);

    if (FUNDEF_BODY (new_node) != NULL) {
        BLOCK_SSACOUNTER (FUNDEF_BODY (new_node)) = new_ssacnt;
    }

    /*
     * must be done after traversal of BODY
     */
#if DUP_DFMS
    FUNDEF_DFM_BASE (new_node)
      = SearchInLUT_PP (INFO_DUP_LUT (arg_info), FUNDEF_DFM_BASE (arg_node));
#else
    FUNDEF_DFM_BASE (new_node) = NULL;
#endif
    FUNDEF_RETURN (new_node)
      = SearchInLUT_PP (INFO_DUP_LUT (arg_info), FUNDEF_RETURN (arg_node));
    if (FUNDEF_IS_LACFUN (new_node)) {
        FUNDEF_INT_ASSIGN (new_node)
          = SearchInLUT_PP (INFO_DUP_LUT (arg_info), FUNDEF_INT_ASSIGN (arg_node));
    }

    INFO_DUP_FUNDEF (arg_info) = old_fundef;

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupImplist (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupImplist");

    new_node = MakeImplist (StringCopy (IMPLIST_NAME (arg_node)),
                            DupIds_ (IMPLIST_ITYPES (arg_node), arg_info),
                            DupIds_ (IMPLIST_ETYPES (arg_node), arg_info),
                            DupIds_ (IMPLIST_OBJS (arg_node), arg_info),
                            DupIds_ (IMPLIST_FUNS (arg_node), arg_info),
                            DUPCONT (IMPLIST_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupArg (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupArg");

    new_node = MakeArg (StringCopy (ARG_NAME (arg_node)),
                        DupTypes_ (ARG_TYPE (arg_node), arg_info), ARG_STATUS (arg_node),
                        ARG_ATTRIB (arg_node), DUPCONT (ARG_NEXT (arg_node)));

    ARG_VARNO (new_node) = ARG_VARNO (arg_node);
    ARG_REFCNT (new_node) = ARG_REFCNT (arg_node);
    ARG_NAIVE_REFCNT (new_node) = ARG_NAIVE_REFCNT (arg_node);
    ARG_OBJDEF (new_node) = ARG_OBJDEF (arg_node);

#if 0
  ARG_TYPESTRING( new_node) = ???;
  ARG_ACTCHN( new_node) = ???;
  ARG_COLCHN( new_node) = ???;
  ARG_FUNDEF( new_node) = ???;
#endif

    CopyCommonNodeData (new_node, arg_node);

    INFO_DUP_LUT (arg_info)
      = InsertIntoLUT_P (INFO_DUP_LUT (arg_info), arg_node, new_node);

    if (ARG_AVIS (arg_node) != NULL) {
        /* we have to duplicate the containing avis node */
        FreeAvis (ARG_AVIS (new_node), arg_info);
        ARG_AVIS (new_node) = DUPTRAV (ARG_AVIS (arg_node));
        /* correct backreference */
        AVIS_VARDECORARG (ARG_AVIS (new_node)) = new_node;
    } else {
        /* noop, the created empty avis node is ok */
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

    CopyCommonNodeData (new_node, arg_node);

    INFO_DUP_LUT (arg_info)
      = InsertIntoLUT_P (INFO_DUP_LUT (arg_info), arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupBlock (node *arg_node, node *arg_info)
{
    node *new_vardecs;
    node *new_node;
#if DUP_DFMS
    DFMmask_base_t old_base, new_base;
#endif

    DBUG_ENTER ("DupBlock");

    new_vardecs = DUPTRAV (BLOCK_VARDEC (arg_node));

#if DUP_DFMS
    if (INFO_DUP_FUNDEF (arg_info) != NULL) {
        old_base = FUNDEF_DFM_BASE (INFO_DUP_FUNDEF (arg_info));
    } else {
        old_base = NULL;
    }

    /*
     * If the current block is the top most block of the current fundef
     * we have to copy FUNDEF_DFMBASE.
     * Look at DupFundef() for further comments.
     */
    if ((old_base != NULL) && (arg_node == FUNDEF_BODY (INFO_DUP_FUNDEF (arg_info)))) {
        new_base
          = DFMGenMaskBase (FUNDEF_ARGS (SearchInLUT_PP (INFO_DUP_LUT (arg_info),
                                                         INFO_DUP_FUNDEF (arg_info))),
                            new_vardecs);

        INFO_DUP_LUT (arg_info)
          = InsertIntoLUT_P (INFO_DUP_LUT (arg_info), old_base, new_base);
    }
#endif

    new_node = MakeBlock (DUPTRAV (BLOCK_INSTR (arg_node)), new_vardecs);
    BLOCK_CACHESIM (new_node) = StringCopy (BLOCK_CACHESIM (arg_node));

    BLOCK_VARNO (new_node) = BLOCK_VARNO (arg_node);
    BLOCK_NEEDFUNS (new_node) = DupNodelist_ (BLOCK_NEEDFUNS (arg_node), arg_info);
    BLOCK_NEEDTYPES (new_node) = DupNodelist_ (BLOCK_NEEDTYPES (arg_node), arg_info);

    /*
     * BLOCK_SSACOUNTER is adjusted correctly later on by DupFundef()
     */
    BLOCK_SSACOUNTER (new_node) = BLOCK_SSACOUNTER (arg_node);

#if 0
  BLOCK_MASK( new_node, ?) = ???;
  BLOCK_SPMD_PROLOG_ICMS( new_node) = ???;
  BLOCK_SPMD_SETUP_ARGS( new_node) = ???;
#endif

    CopyCommonNodeData (new_node, arg_node);

    /*
     * after all we have to traverse the vardecs again to correct the
     * SSAASSIGN(2) attributes from the look-up-table (stored by the
     * DupAssign function in the BLOCK_INSTR traversal.
     */
    if (new_vardecs != NULL) {
        while (new_vardecs != NULL) {
            if (VARDEC_AVIS (new_vardecs) != NULL) {
                AVIS_SSAASSIGN (VARDEC_AVIS (new_vardecs))
                  = SearchInLUT_PP (INFO_DUP_LUT (arg_info),
                                    AVIS_SSAASSIGN (VARDEC_AVIS (new_vardecs)));
                AVIS_SSAASSIGN2 (VARDEC_AVIS (new_vardecs))
                  = SearchInLUT_PP (INFO_DUP_LUT (arg_info),
                                    AVIS_SSAASSIGN2 (VARDEC_AVIS (new_vardecs)));
            }
            new_vardecs = VARDEC_NEXT (new_vardecs);
        }
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupVardec (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupVardec");

    new_node = MakeVardec (StringCopy (VARDEC_NAME (arg_node)),
                           DupTypes_ (VARDEC_TYPE (arg_node), arg_info),
                           DUPCONT (VARDEC_NEXT (arg_node)));

    VARDEC_STATUS (new_node) = VARDEC_STATUS (arg_node);
    VARDEC_ATTRIB (new_node) = VARDEC_ATTRIB (arg_node);
    VARDEC_VARNO (new_node) = VARDEC_VARNO (arg_node);
    VARDEC_REFCNT (new_node) = VARDEC_REFCNT (arg_node);
    VARDEC_NAIVE_REFCNT (new_node) = VARDEC_NAIVE_REFCNT (arg_node);
    VARDEC_FLAG (new_node) = VARDEC_FLAG (arg_node);
    VARDEC_OBJDEF (new_node) = VARDEC_OBJDEF (arg_node);

#if 0
  VARDEC_ACTCHN( new_node) = ???;
  VARDEC_COLCHN( new_node) = ???;
#endif

    CopyCommonNodeData (new_node, arg_node);

    INFO_DUP_LUT (arg_info)
      = InsertIntoLUT_P (INFO_DUP_LUT (arg_info), arg_node, new_node);

    if (VARDEC_AVIS (arg_node) != NULL) {
        /* we have to duplicate the containing avis node */
        FreeAvis (VARDEC_AVIS (new_node), arg_info);
        VARDEC_AVIS (new_node) = DUPTRAV (VARDEC_AVIS (arg_node));
        /* correct backreference */
        AVIS_VARDECORARG (VARDEC_AVIS (new_node)) = new_node;
    } else {
        /* noop, the created empty avis node is ok */
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupReturn (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupReturn");

    new_node = MakeReturn (DUPTRAV (RETURN_EXPRS (arg_node)));

    RETURN_REFERENCE (new_node) = DUPTRAV (RETURN_REFERENCE (arg_node));

    RETURN_CRET (new_node)
      = SearchInLUT_PP (INFO_DUP_LUT (arg_info), RETURN_CRET (arg_node));

    RETURN_USEMASK (new_node) = DupDFMask_ (RETURN_USEMASK (arg_node), arg_info);
    RETURN_DEFMASK (new_node) = DupDFMask_ (RETURN_DEFMASK (arg_node), arg_info);

    CopyCommonNodeData (new_node, arg_node);

    INFO_DUP_LUT (arg_info)
      = InsertIntoLUT_P (INFO_DUP_LUT (arg_info), arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupEmpty (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupEmpty");

    new_node = MakeEmpty ();

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupAssign (node *arg_node, node *arg_info)
{
    node *new_node = NULL;
    node *stacked_assign;

    DBUG_ENTER ("DupAssign");

    if ((INFO_DUP_TYPE (arg_info) != DUP_INLINE)
        || (NODE_TYPE (ASSIGN_INSTR (arg_node)) != N_return)) {
        new_node = MakeAssign (NULL, NULL);

        stacked_assign = INFO_DUP_ASSIGN (arg_info);
        INFO_DUP_ASSIGN (arg_info) = new_node;
        ASSIGN_INSTR (new_node) = DUPTRAV (ASSIGN_INSTR (arg_node));
        INFO_DUP_ASSIGN (arg_info) = stacked_assign;

        ASSIGN_NEXT (new_node) = DUPCONT (ASSIGN_NEXT (arg_node));

        INFO_DUP_LUT (arg_info)
          = InsertIntoLUT_P (INFO_DUP_LUT (arg_info), arg_node, new_node);

        ASSIGN_STATUS (new_node) = ASSIGN_STATUS (arg_node);
        ASSIGN_LEVEL (new_node) = ASSIGN_LEVEL (arg_node);

#if 0
    ASSIGN_MASK( new_node, ?) = ???;
    ASSIGN_CSE( new_node) = ???;
    ASSIGN_CF( new_node) = ???;
    ASSIGN_INDEX( new_node) = ???;
    ASSIGN_DEFMASK( new_node) = DupMask( ASSIGN_DEFMASK( arg_node), 400);
    ASSIGN_USEMASK( new_node) = DupMask( ASSIGN_USEMASK( arg_node), 400);
    ASSIGN_MRDMASK( new_node) = DupMask( ASSIGN_MRDMASK( arg_node), 400);
#endif

        CopyCommonNodeData (new_node, arg_node);
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

    COND_THENVARS (new_node) = DupIds_ (COND_THENVARS (arg_node), arg_info);
    COND_ELSEVARS (new_node) = DupIds_ (COND_ELSEVARS (arg_node), arg_info);

#if 0
  COND_MASK( new_node, ?) = ???;
#endif

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupDo (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupDo");

    new_node = MakeDo (DUPTRAV (DO_COND (arg_node)), DUPTRAV (DO_BODY (arg_node)));

    DO_USEVARS (new_node) = DupIds_ (DO_USEVARS (arg_node), arg_info);
    DO_DEFVARS (new_node) = DupIds_ (DO_DEFVARS (arg_node), arg_info);

#if 0
  DO_MASK( new_node, ?) = ???;
#endif

    CopyCommonNodeData (new_node, arg_node);

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

    WHILE_USEVARS (new_node) = DupIds_ (WHILE_USEVARS (arg_node), arg_info);
    WHILE_DEFVARS (new_node) = DupIds_ (WHILE_DEFVARS (arg_node), arg_info);

#if 0
  WHILE_MASK( new_node, ?) = ???;
#endif

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupLet (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupLet");

    new_node = MakeLet (NULL, DupIds_ (LET_IDS (arg_node), arg_info));

    /*
     * EXPR must be traversed after IDS (for AP_ARGTAB)
     */
    LET_EXPR (new_node) = DUPTRAV (LET_EXPR (arg_node));

    LET_USEMASK (new_node) = DupDFMask_ (LET_USEMASK (arg_node), arg_info);
    LET_DEFMASK (new_node) = DupDFMask_ (LET_DEFMASK (arg_node), arg_info);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupAp (node *arg_node, node *arg_info)
{
    node *old_fundef, *new_fundef;
    node *new_node;

    DBUG_ENTER ("DupAp");

    DBUG_PRINT ("DUP",
                ("duplicating application of %s() ...",
                 (AP_FUNDEF (arg_node) != NULL) ? FUNDEF_NAME (AP_FUNDEF (arg_node))
                                                : "?"));

    old_fundef = AP_FUNDEF (arg_node);
    new_fundef = SearchInLUT_PP (INFO_DUP_LUT (arg_info), old_fundef);

    new_node = MakeAp (StringCopy (AP_NAME (arg_node)), StringCopy (AP_MOD (arg_node)),
                       DUPTRAV (AP_ARGS (arg_node)));

    AP_FUNDEF (new_node) = new_fundef;
    AP_ATFLAG (new_node) = AP_ATFLAG (arg_node);

    AP_ARGTAB (new_node) = DupArgtab_ (AP_ARGTAB (arg_node), arg_info);

    CopyCommonNodeData (new_node, arg_node);

    /*
     * A special function is implicit inlined code and we
     * cannot simply duplicate all dependend functions (due to
     * the problem where to store the new created fundefs).
     * Therefore, we add this new application as additional
     * external reference to the called special function and
     * increment its used counter. Before we can optimize such a
     * function with multiple references we have to duplicate it
     * in a place where we can handle the newly created fundefs.
     * (usually the XYYap traversal functions in the optimizations)
     */

    if (old_fundef != NULL) {
        DBUG_ASSERT ((new_fundef != NULL), "AP_FUNDEF not found!");

        DBUG_ASSERT (((!FUNDEF_IS_LACFUN (old_fundef))
                      || (FUNDEF_USED (old_fundef) != USED_INACTIVE)),
                     "FUNDEF_USED must be active for LaC functions!");

        DBUG_ASSERT (((!FUNDEF_IS_LACFUN (new_fundef))
                      || (FUNDEF_USED (new_fundef) != USED_INACTIVE)),
                     "FUNDEF_USED must be active for LaC functions!");

        /*
         * increment reference counter (FUNDEF_USED)
         */
        if ((FUNDEF_USED (new_fundef) != USED_INACTIVE)
            && ((!FUNDEF_IS_LOOPFUN (new_fundef))
                || (arg_node != ASSIGN_RHS (FUNDEF_INT_ASSIGN (old_fundef))))) {
            DBUG_ASSERT ((FUNDEF_USED (new_fundef) >= 0), "FUNDEF_USED dropped below 0!");

            (FUNDEF_USED (new_fundef))++;

            DBUG_PRINT ("DUP", ("used counter for %s incremented to %d",
                                FUNDEF_NAME (new_fundef), FUNDEF_USED (new_fundef)));

            if (FUNDEF_IS_LACFUN (new_fundef)) {
                /* add new application to external assignment chain */
                DBUG_ASSERT ((INFO_DUP_ASSIGN (arg_info) != NULL),
                             "no corresponding assignment node");

                FUNDEF_EXT_ASSIGNS (new_fundef)
                  = NodeListAppend (FUNDEF_EXT_ASSIGNS (new_fundef),
                                    INFO_DUP_ASSIGN (arg_info), NULL);
            }
        }
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupMop (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupMop");

    DBUG_PRINT ("DUP", ("duplicating multi operation ..."));

    new_node = MakeMop (DUPTRAV (MOP_EXPRS (arg_node)), DupAllIds (MOP_OPS (arg_node)),
                        MOP_FIX (arg_node));

    CopyCommonNodeData (new_node, arg_node);

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

    ARRAY_TYPE (new_node) = DupTypes_ (ARRAY_TYPE (arg_node), arg_info);

    ARRAY_ISCONST (new_node) = ARRAY_ISCONST (arg_node);
    ARRAY_VECLEN (new_node) = ARRAY_VECLEN (arg_node);
    ARRAY_VECTYPE (new_node) = ARRAY_VECTYPE (arg_node);
    if (ARRAY_ISCONST (new_node)) {
        ARRAY_CONSTVEC (new_node)
          = CopyConstVec (ARRAY_VECTYPE (arg_node), ARRAY_VECLEN (arg_node),
                          ARRAY_CONSTVEC (arg_node));
    }

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupPrf (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupPrf");

    new_node = MakePrf (PRF_PRF (arg_node), DUPTRAV (PRF_ARGS (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

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

    PRAGMA_EFFECT (new_node) = DupIds_ (PRAGMA_EFFECT (arg_node), arg_info);
    PRAGMA_TOUCH (new_node) = DupIds_ (PRAGMA_TOUCH (arg_node), arg_info);

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

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupIcm (node *arg_node, node *arg_info)
{
    node *new_node;
    node *fundef;

    DBUG_ENTER ("DupIcm");

    new_node = MakeIcm (ICM_NAME (arg_node), DUPTRAV (ICM_ARGS (arg_node)));

    /*
     * The ICM name is not copied here because ICM names are predominantly static

     * string constants and therefore aren't freed anyway.
     */

    ICM_INDENT_BEFORE (new_node) = ICM_INDENT_BEFORE (arg_node);
    ICM_INDENT_AFTER (new_node) = ICM_INDENT_AFTER (arg_node);
    ICM_FUNDEF (new_node)
      = SearchInLUT_PP (INFO_DUP_LUT (arg_info), ICM_FUNDEF (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    /* increment the fundef used counter for refcounted fundefs */
    fundef = ICM_FUNDEF (new_node);
    if ((fundef != NULL) && (FUNDEF_USED (fundef) != USED_INACTIVE)
        && ((!FUNDEF_IS_LOOPFUN (fundef))
            || (INFO_DUP_ASSIGN (arg_info) != ASSIGN_RHS (FUNDEF_INT_ASSIGN (fundef))))) {
        DBUG_ASSERT ((FUNDEF_USED (fundef) >= 0), "FUNDEF_USED dropped below 0!");

        (FUNDEF_USED (fundef))++;

        DBUG_PRINT ("DUP", ("used counter for %s incremented to %d", FUNDEF_NAME (fundef),
                            FUNDEF_USED (fundef)));
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupSpmd (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupSpmd");

    new_node = MakeSpmd (DUPTRAV (SPMD_REGION (arg_node)));

    SPMD_IN (new_node) = DupDFMask_ (SPMD_IN (arg_node), arg_info);
    SPMD_INOUT (new_node) = DupDFMask_ (SPMD_INOUT (arg_node), arg_info);
    SPMD_OUT (new_node) = DupDFMask_ (SPMD_OUT (arg_node), arg_info);
    SPMD_LOCAL (new_node) = DupDFMask_ (SPMD_LOCAL (arg_node), arg_info);
    SPMD_SHARED (new_node) = DupDFMask_ (SPMD_SHARED (arg_node), arg_info);

    CopyCommonNodeData (new_node, arg_node);

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

    SYNC_IN (new_node) = DupDFMask_ (SYNC_IN (arg_node), arg_info);
    SYNC_INOUT (new_node) = DupDFMask_ (SYNC_INOUT (arg_node), arg_info);
    SYNC_OUT (new_node) = DupDFMask_ (SYNC_OUT (arg_node), arg_info);
    SYNC_OUTREP (new_node) = DupDFMask_ (SYNC_OUTREP (arg_node), arg_info);
    SYNC_LOCAL (new_node) = DupDFMask_ (SYNC_LOCAL (arg_node), arg_info);

    CopyCommonNodeData (new_node, arg_node);

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
    NWITH_FOLDABLE (new_node) = NWITH_FOLDABLE (arg_node);
    NWITH_NO_CHANCE (new_node) = NWITH_NO_CHANCE (arg_node);

#if 0
  NWITH_TSI( new_node) = ???;
#endif

    NWITH_DEC_RC_IDS (new_node) = DupIds_ (NWITH_DEC_RC_IDS (arg_node), arg_info);

    NWITH_IN_MASK (new_node) = DupDFMask_ (NWITH_IN_MASK (arg_node), arg_info);
    NWITH_OUT_MASK (new_node) = DupDFMask_ (NWITH_OUT_MASK (arg_node), arg_info);
    NWITH_LOCAL_MASK (new_node) = DupDFMask_ (NWITH_LOCAL_MASK (arg_node), arg_info);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNwithop (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNwithop");

    new_node = MakeNWithOp (NWITHOP_TYPE (arg_node), NULL);

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
        NWITHOP_FUNDEF (new_node)
          = SearchInLUT_PP (INFO_DUP_LUT (arg_info), NWITHOP_FUNDEF (arg_node));
        NWITHOP_FUN (new_node) = StringCopy (NWITHOP_FUN (arg_node));
        break;
    case WO_foldprf:
        NWITHOP_NEUTRAL (new_node) = DUPTRAV (NWITHOP_NEUTRAL (arg_node));
        NWITHOP_FUNDEF (new_node)
          = SearchInLUT_PP (INFO_DUP_LUT (arg_info), NWITHOP_FUNDEF (arg_node));
        NWITHOP_PRF (new_node) = NWITHOP_PRF (arg_node);
        break;
    default:
        DBUG_ASSERT (0, "Unknown N_Nwithop-type found");
        break;
    }

#if 0
  NWITHOP_MASK( new_node, ?) = ???;
#endif

    CopyCommonNodeData (new_node, arg_node);

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
                   SearchInLUT_PP (INFO_DUP_LUT (arg_info), NPART_CODE (arg_node)));

    NPART_NEXT (new_node) = DUPCONT (NPART_NEXT (arg_node));

#if 0
  NPART_MASK( new_node, ?) = ???;
#endif

    CopyCommonNodeData (new_node, arg_node);

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

    INFO_DUP_LUT (arg_info)
      = InsertIntoLUT_P (INFO_DUP_LUT (arg_info), arg_node, new_node);

    NCODE_NEXT (new_node) = DUPCONT (NCODE_NEXT (arg_node));

    /*
     * NCODE_USED is incremented in DupNpart() via MakeNPart(),
     *                           in DupWLgrid() via MakeWLgrid(), respectively
     */
    NCODE_USED (new_node) = 0;
    NCODE_FLAG (new_node) = NCODE_FLAG (arg_node);

    NCODE_INC_RC_IDS (new_node) = DupIds_ (NCODE_INC_RC_IDS (arg_node), arg_info);

#if 0
  NCODE_MASK( new_node, ?) = ???;
#endif

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNwithid (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNwithid");

    new_node = MakeNWithid (DupIds_ (NWITHID_VEC (arg_node), arg_info),
                            DupIds_ (NWITHID_IDS (arg_node), arg_info));

    CopyCommonNodeData (new_node, arg_node);

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

    CopyCommonNodeData (new_node, arg_node);

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

    NWITH2_OFFSET_NEEDED (new_node) = NWITH2_OFFSET_NEEDED (arg_node);

    NWITH2_DEC_RC_IDS (new_node) = DupIds_ (NWITH2_DEC_RC_IDS (arg_node), arg_info);

    NWITH2_REUSE (new_node) = DupDFMask_ (NWITH2_REUSE (arg_node), arg_info);

    NWITH2_IN_MASK (new_node) = DupDFMask_ (NWITH2_IN_MASK (arg_node), arg_info);
    NWITH2_OUT_MASK (new_node) = DupDFMask_ (NWITH2_OUT_MASK (arg_node), arg_info);
    NWITH2_LOCAL_MASK (new_node) = DupDFMask_ (NWITH2_LOCAL_MASK (arg_node), arg_info);

    NWITH2_ISSCHEDULED (new_node) = NWITH2_ISSCHEDULED (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLseg (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupWLseg");

    new_node = MakeWLseg (WLSEG_DIMS (arg_node), DUPTRAV (WLSEG_CONTENTS (arg_node)),
                          DUPCONT (WLSEG_NEXT (arg_node)));

    DUP_VECT (WLSEG_IDX_MIN (new_node), WLSEG_IDX_MIN (arg_node), WLSEG_DIMS (new_node),
              int);
    DUP_VECT (WLSEG_IDX_MAX (new_node), WLSEG_IDX_MAX (arg_node), WLSEG_DIMS (new_node),
              int);

    DUP_VECT (WLSEG_UBV (new_node), WLSEG_UBV (arg_node), WLSEG_DIMS (new_node), int);

    WLSEG_BLOCKS (new_node) = WLSEG_BLOCKS (arg_node);

    for (i = 0; i < WLSEG_BLOCKS (new_node); i++) {
        DUP_VECT (WLSEG_BV (new_node, i), WLSEG_BV (arg_node, i), WLSEG_DIMS (new_node),
                  int);
    }

    DUP_VECT (WLSEG_SV (new_node), WLSEG_SV (arg_node), WLSEG_DIMS (new_node), int);
    DUP_VECT (WLSEG_HOMSV (new_node), WLSEG_HOMSV (arg_node), WLSEG_DIMS (new_node), int);

    if (WLSEG_SCHEDULING (arg_node) != NULL) {
        WLSEG_SCHEDULING (new_node) = SCHCopyScheduling (WLSEG_SCHEDULING (arg_node));
    }

    if (WLSEGX_TASKSEL (arg_node) != NULL) {
        WLSEGX_TASKSEL (new_node) = SCHCopyTasksel (WLSEGX_TASKSEL (arg_node));
    }

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLsegVar (node *arg_node, node *arg_info)
{
    node *new_node;
    int d;

    DBUG_ENTER ("DupWLsegVar");

    new_node
      = MakeWLsegVar (WLSEGVAR_DIMS (arg_node), DUPTRAV (WLSEGVAR_CONTENTS (arg_node)),
                      DUPCONT (WLSEGVAR_NEXT (arg_node)));

    MALLOC_VECT (WLSEGVAR_IDX_MIN (new_node), WLSEGVAR_DIMS (new_node), node *);
    MALLOC_VECT (WLSEGVAR_IDX_MAX (new_node), WLSEGVAR_DIMS (new_node), node *);
    for (d = 0; d < WLSEGVAR_DIMS (new_node); d++) {
        WLSEGVAR_IDX_MIN (new_node)[d] = DUPTRAV (WLSEGVAR_IDX_MIN (arg_node)[d]);
        WLSEGVAR_IDX_MAX (new_node)[d] = DUPTRAV (WLSEGVAR_IDX_MAX (arg_node)[d]);
    }

    if (WLSEGVAR_SCHEDULING (arg_node) != NULL) {
        WLSEGVAR_SCHEDULING (new_node)
          = SCHCopyScheduling (WLSEGVAR_SCHEDULING (arg_node));
    }

    if (WLSEGX_TASKSEL (arg_node) != NULL) {
        WLSEGX_TASKSEL (new_node) = SCHCopyTasksel (WLSEGX_TASKSEL (arg_node));
    }

    CopyCommonNodeData (new_node, arg_node);

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

    CopyCommonNodeData (new_node, arg_node);

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

    CopyCommonNodeData (new_node, arg_node);

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

    /*
     * duplicated strides are not modified yet ;)
     */
    WLSTRIDE_MODIFIED (new_node) = NULL;

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLstrideVar (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupWLstrideVar");

    new_node = MakeWLstrideVar (WLSTRIDEVAR_LEVEL (arg_node), WLSTRIDEVAR_DIM (arg_node),
                                DUPTRAV (WLSTRIDEVAR_BOUND1 (arg_node)),
                                DUPTRAV (WLSTRIDEVAR_BOUND2 (arg_node)),
                                DUPTRAV (WLSTRIDEVAR_STEP (arg_node)),
                                DUPTRAV (WLSTRIDEVAR_CONTENTS (arg_node)),
                                DUPCONT (WLSTRIDEVAR_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

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
                    SearchInLUT_PP (INFO_DUP_LUT (arg_info), WLGRID_CODE (arg_node)));

    WLGRID_FITTED (new_node) = WLGRID_FITTED (arg_node);
    WLGRID_NOOP (new_node) = WLGRID_NOOP (arg_node);

    /*
     * duplicated grids are not modified yet ;)
     */
    WLGRID_MODIFIED (new_node) = NULL;

    CopyCommonNodeData (new_node, arg_node);

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
                              SearchInLUT_PP (INFO_DUP_LUT (arg_info),
                                              WLGRIDVAR_CODE (arg_node)));

    WLGRIDVAR_FITTED (new_node) = WLGRIDVAR_FITTED (arg_node);
    WLGRIDVAR_NOOP (new_node) = WLGRIDVAR_NOOP (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupMt( node *arg_node, node *arg_info)
 *
 * description:
 *   Duplicates a N_mt, especially the DFMmasks are copied.
 *
 ******************************************************************************/

node *
DupMt (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupMt");

    new_node = MakeMT (DUPTRAV (MT_REGION (arg_node)));
    MT_IDENTIFIER (new_node) = MT_IDENTIFIER (arg_node);

    MT_USEMASK (new_node) = DupDFMask_ (MT_USEMASK (arg_node), arg_info);
    MT_DEFMASK (new_node) = DupDFMask_ (MT_DEFMASK (arg_node), arg_info);
    MT_NEEDLATER (new_node) = DupDFMask_ (MT_NEEDLATER (arg_node), arg_info);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupSt( node *arg_node, node *arg_info)
 *
 * description:
 *   Duplicates a N_st, especially the DFMmasks are copied.
 *
 ******************************************************************************/

node *
DupSt (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupSt");

    new_node = MakeST (DUPTRAV (ST_REGION (arg_node)));
    ST_IDENTIFIER (new_node) = ST_IDENTIFIER (arg_node);

    ST_USEMASK (new_node) = DupDFMask_ (ST_USEMASK (arg_node), arg_info);
    ST_DEFMASK (new_node) = DupDFMask_ (ST_DEFMASK (arg_node), arg_info);

    ST_NEEDLATER_ST (new_node) = DupDFMask_ (ST_NEEDLATER_ST (arg_node), arg_info);
    ST_NEEDLATER_MT (new_node) = DupDFMask_ (ST_NEEDLATER_MT (arg_node), arg_info);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupMTsignal( node *arg_node, node *arg_info)
 *
 * description:
 *   Duplicates a N_MTsignal, especially the DFMmasks are copied.
 *
 ******************************************************************************/

node *
DupMTsignal (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupMTsignal");

    new_node = MakeMTsignal ();

    MTSIGNAL_IDSET (new_node) = DupDFMask_ (MTSIGNAL_IDSET (arg_node), arg_info);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupMTsync( node *arg_node, node *arg_info)
 *
 * description:
 *   Duplicates a N_MTsync, especially the DFMmasks are copied.
 *
 ******************************************************************************/

node *
DupMTsync (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupMTsync");

    new_node = MakeMTsync ();

    MTSYNC_WAIT (new_node) = DupDFMask_ (MTSYNC_WAIT (arg_node), arg_info);
    MTSYNC_FOLD (new_node) = CopyDFMfoldmask (MTSYNC_FOLD (arg_node));
    MTSYNC_ALLOC (new_node) = DupDFMask_ (MTSYNC_ALLOC (arg_node), arg_info);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupMTalloc( node *arg_node, node *arg_info)
 *
 * description:
 *   Duplicates a N_MTalloc, especially the DFMmasks are copied.
 *
 ******************************************************************************/

node *
DupMTalloc (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupMTalloc");

    new_node = MakeMTalloc ();

    MTALLOC_IDSET (new_node) = DupDFMask_ (MTALLOC_IDSET (arg_node), arg_info);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupAvis( node *arg_node, node *arg_info)
 *
 * description:
 *   Duplicates a N_avis node. Does not set the correct backrefence!!!
 *
 ******************************************************************************/

node *
DupAvis (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupAvis");

    new_node = MakeAvis (NULL);

    INFO_DUP_LUT (arg_info)
      = InsertIntoLUT_P (INFO_DUP_LUT (arg_info), arg_node, new_node);

    AVIS_SSACOUNT (new_node)
      = SearchInLUT_PP (INFO_DUP_LUT (arg_info), AVIS_SSACOUNT (arg_node));

    AVIS_SSAASSIGN (new_node)
      = SearchInLUT_PP (INFO_DUP_LUT (arg_info), AVIS_SSAASSIGN (arg_node));
    AVIS_SSAASSIGN2 (new_node)
      = SearchInLUT_PP (INFO_DUP_LUT (arg_info), AVIS_SSAASSIGN2 (arg_node));

    if (AVIS_SSACONST (arg_node) != NULL) {
        AVIS_SSACONST (new_node) = COCopyConstant (AVIS_SSACONST (arg_node));
    }

    AVIS_SSAPHITARGET (new_node) = AVIS_SSAPHITARGET (arg_node);
    AVIS_SSALPINV (new_node) = AVIS_SSALPINV (arg_node);
    AVIS_SSASTACK (new_node) = DUPTRAV (AVIS_SSASTACK (arg_node));
    AVIS_SSAUNDOFLAG (new_node) = AVIS_SSAUNDOFLAG (arg_node);

    AVIS_SSADEFINED (new_node) = AVIS_SSADEFINED (arg_node);
    AVIS_SSATHEN (new_node) = AVIS_SSATHEN (arg_node);
    AVIS_SSAELSE (new_node) = AVIS_SSAELSE (arg_node);
    AVIS_NEEDCOUNT (new_node) = AVIS_NEEDCOUNT (arg_node);
    AVIS_SUBST (new_node) = AVIS_SUBST (arg_node);
    AVIS_SUBSTUSSA (new_node) = AVIS_SUBSTUSSA (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupSSAstack( node *arg_node, node *arg_info)
 *
 * description:
 *   Duplicates a N_ssastack node.
 *
 ******************************************************************************/

node *
DupSSAstack (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupSSAstack");

    new_node
      = MakeSSAstack (DUPCONT (SSASTACK_NEXT (arg_node)), SSASTACK_AVIS (arg_node));

    SSASTACK_INUSE (arg_node) = SSASTACK_INUSE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupSSAcnt( node *arg_node, node *arg_info)
 *
 * description:
 *   Duplicates a N_ssacnt node.
 *
 ******************************************************************************/

node *
DupSSAcnt (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupSSAcnt");

    new_node = MakeSSAcnt (DUPCONT (SSACNT_NEXT (arg_node)), SSACNT_COUNT (arg_node),
                           StringCopy (SSACNT_BASEID (arg_node)));

    INFO_DUP_LUT (arg_info)
      = InsertIntoLUT_P (INFO_DUP_LUT (arg_info), arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * functions:
 *   node *DupTree( node *arg_node)
 *   node *DupTree_Type( node *arg_node, LUT_t lut, int type);
 *   node *DupTreeLUT( node *arg_node, LUT_t lut)
 *   node *DupTreeLUT_Type( node *arg_node, LUT_t lut, int type);
 *   node *DupNode( node *arg_node)
 *   node *DupNode_Type( node *arg_node, LUT_t lut, int type);
 *   node *DupNodeLUT( node *arg_node, LUT_t lut)
 *   node *DupNodeLUT_Type( node *arg_node, LUT_t lut, int type);
 *
 * description:
 *   Copying of trees and nodes ...
 *   The node to be copied is arg_node.
 *
 *   Which function do I use???
 *   - If you want to copy a whole tree use one of the DupTreeXxx() functions.
 *     If you want to copy a node only (that means the node and all it's
 *     attributes but not the xxx_NEXT) use one of the DupNodeXxx() functions.
 *   - If you want to use a special LookUpTable (LUT) use the specific
 *     DupXxxLUT() version, otherwise use DupXxx().
 *     (If you dont't know what a LUT is good for use DupXxx() :-/ )
 *   - If you need some special behaviour triggered by INFO_DUP_TYPE use the
 *     specific DupXxx_Type() version.
 *     Legal values for the parameter 'type' are DUP_INLINE, DUP_WLF, ...
 *
 ******************************************************************************/

node *
DupTree (node *arg_node)
{
    node *new_node;

    DBUG_ENTER ("DupTree");

    new_node = DupTreeOrNodeLUT_Type (FALSE, arg_node, NULL, DUP_NORMAL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DupTree_Type (node *arg_node, int type)
{
    node *new_node;

    DBUG_ENTER ("DupTree_Type");

    new_node = DupTreeOrNodeLUT_Type (FALSE, arg_node, NULL, type);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DupTreeLUT (node *arg_node, LUT_t lut)
{
    node *new_node;

    DBUG_ENTER ("DupTreeLUT");

    new_node = DupTreeOrNodeLUT_Type (FALSE, arg_node, lut, DUP_NORMAL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DupTreeLUT_Type (node *arg_node, LUT_t lut, int type)
{
    node *new_node;

    DBUG_ENTER ("DupTreeLUT_Type");

    new_node = DupTreeOrNodeLUT_Type (FALSE, arg_node, lut, type);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DupNode (node *arg_node)
{
    node *new_node;

    DBUG_ENTER ("DupTree");

    new_node = DupTreeOrNodeLUT_Type (TRUE, arg_node, NULL, DUP_NORMAL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DupNode_Type (node *arg_node, int type)
{
    node *new_node;

    DBUG_ENTER ("DupNode_Type");

    new_node = DupTreeOrNodeLUT_Type (TRUE, arg_node, NULL, type);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DupNodeLUT (node *arg_node, LUT_t lut)
{
    node *new_node;

    DBUG_ENTER ("DupNodeLUT");

    new_node = DupTreeOrNodeLUT_Type (TRUE, arg_node, lut, DUP_NORMAL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DupNodeLUT_Type (node *arg_node, LUT_t lut, int type)
{
    node *new_node;

    DBUG_ENTER ("DupNodeLUT_Type");

    new_node = DupTreeOrNodeLUT_Type (TRUE, arg_node, lut, type);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * Function:
 *   ids *DupOneIds( ids *arg_ids)
 *
 * Description:
 *   Duplicates the first IDS of the given IDS chain.
 *
 ******************************************************************************/

ids *
DupOneIds (ids *arg_ids)
{
    ids *new_ids, *tmp;

    DBUG_ENTER ("DupOneIds");

    DBUG_ASSERT ((arg_ids != NULL), "DupOneIds: argument is NULL!");

    tmp = IDS_NEXT (arg_ids);
    IDS_NEXT (arg_ids) = NULL;
    new_ids = DupIds_ (arg_ids, NULL);
    IDS_NEXT (arg_ids) = tmp;

    DBUG_RETURN (new_ids);
}

/******************************************************************************
 *
 * Function:
 *   ids *DupAllIds( ids *arg_ids)
 *
 * Description:
 *   Duplicates an IDS chain.
 *
 ******************************************************************************/

ids *
DupAllIds (ids *arg_ids)
{
    ids *new_ids;

    DBUG_ENTER ("DupAllIds");

    DBUG_ASSERT ((arg_ids != NULL), "DupAllIds: argument is NULL!");

    new_ids = DupIds_ (arg_ids, NULL);

    DBUG_RETURN (new_ids);
}

/******************************************************************************
 *
 * Function:
 *   shpseg *DupShpseg( shpseg *arg_shpseg)
 *
 * Description:
 *
 *
 ******************************************************************************/

shpseg *
DupShpseg (shpseg *arg_shpseg)
{
    shpseg *new_shpseg;

    DBUG_ENTER ("DupShpseg");

#if 0
  DBUG_ASSERT( (arg_shpseg != NULL), "DupShpseg: argument is NULL!");
#endif

    new_shpseg = DupShpseg_ (arg_shpseg, NULL);

    DBUG_RETURN (new_shpseg);
}

/******************************************************************************
 *
 * Function:
 *   types *DupOneTypes( types *type)
 *
 * Description:
 *   Duplicates the first TYPES structure of the given TYPES chain.
 *
 *   This function duplicates the (real) types-structure. Unfortunately, it
 *   is *not* identical to the (virtual) TYPES-structure  8-((
 *
 *   For duplicating the (virtual) TYPES-structure only, use DupOneTypesOnly()
 *   or DupOneTypesOnly_Inplace() !!!
 *
 ******************************************************************************/

types *
DupOneTypes (types *arg_types)
{
    types *new_types, *tmp;

    DBUG_ENTER ("DupOneTypes");

    DBUG_ASSERT ((arg_types != NULL), "DupOneTypes: argument is NULL!");

    tmp = TYPES_NEXT (arg_types);
    TYPES_NEXT (arg_types) = NULL;
    new_types = DupTypes_ (arg_types, NULL);
    TYPES_NEXT (arg_types) = tmp;

    DBUG_RETURN (new_types);
}

/******************************************************************************
 *
 * Function:
 *   types *DupAllTypes( types* type)
 *
 * Description:
 *   This function duplicates the (real) types-structure. Unfortunately, it
 *   is *not* identical to the (virtual) TYPES-structure  8-((
 *
 *   For duplicating the (virtual) TYPES-structure only, use DupAllTypesOnly()
 *   !!!
 *
 ******************************************************************************/

types *
DupAllTypes (types *arg_types)
{
    types *new_types;

    DBUG_ENTER ("DupAllTypes");

    DBUG_ASSERT ((arg_types != NULL), "DupAllTypes: argument is NULL!");

    new_types = DupTypes_ (arg_types, NULL);

    DBUG_RETURN (new_types);
}

/******************************************************************************
 *
 * Function:
 *   nodelist *DupNodelist( nodelist *nl)
 *
 * Description:
 *
 *
 ******************************************************************************/

nodelist *
DupNodelist (nodelist *nl)
{
    nodelist *new_nl;

    DBUG_ENTER ("DupNodelist");

    DBUG_ASSERT ((nl != NULL), "DupNodelist: argument is NULL!");

    new_nl = DupNodelist_ (nl, NULL);

    DBUG_RETURN (new_nl);
}

/******************************************************************************
 *
 * Function:
 *   node *DupIds_Id( ids *arg_ids)
 *
 * Description:
 *   Duplicates an IDS and returns a *N_id* node.
 *
 ******************************************************************************/

node *
DupIds_Id (ids *arg_ids)
{
    node *new_id;

    DBUG_ENTER ("DupIds_Id");

    new_id = MakeId (NULL, NULL, ST_regular);
    ID_IDS (new_id) = DupOneIds (arg_ids);

    DBUG_RETURN (new_id);
}

/******************************************************************************
 *
 * Function:
 *   ids *DupId_Ids( node *old_id)
 *
 * Description:
 *   Duplicates a N_id node and returns an *IDS*.
 *
 ******************************************************************************/

ids *
DupId_Ids (node *old_id)
{
    ids *new_ids;

    DBUG_ENTER ("DupId_Ids");

    new_ids = MakeIds (StringCopy (ID_NAME (old_id)), StringCopy (ID_MOD (old_id)),
                       ID_STATUS (old_id));

    IDS_REFCNT (new_ids) = ID_REFCNT (old_id);
    IDS_NAIVE_REFCNT (new_ids) = ID_NAIVE_REFCNT (old_id);
    IDS_VARDEC (new_ids) = ID_VARDEC (old_id);
    IDS_ATTRIB (new_ids) = ID_ATTRIB (old_id);

#if 0
  IDS_DEF( new_ids) = ???
  IDS_USE( new_ids) = ???
#endif

    DBUG_RETURN (new_ids);
}

/******************************************************************************
 *
 * Function:
 *   node *DupIds_Id_NT( ids *arg_ids)
 *
 * Description:
 *   Duplicates an IDS and returns a *N_id* node.
 *   Sets ID_NT_TAG.
 *
 ******************************************************************************/

node *
DupIds_Id_NT (ids *arg_ids)
{
    node *new_id;

    DBUG_ENTER ("DupIds_Id_NT");

    new_id = DupIds_Id (arg_ids);

    DBUG_ASSERT ((IDS_TYPE (arg_ids) != NULL), "NT_TAG: no type found!");
    ID_NT_TAG (new_id) = CreateNtTag (IDS_NAME (arg_ids), IDS_TYPE (arg_ids));

    DBUG_RETURN (new_id);
}

/******************************************************************************
 *
 * Function:
 *   node *DupId_NT( node *arg_id)
 *
 * Description:
 *   Duplicates a N_id node.
 *   Sets ID_NT_TAG.
 *
 ******************************************************************************/

node *
DupId_NT (node *arg_id)
{
    node *new_id;

    DBUG_ENTER ("DupId_NT");

    DBUG_ASSERT ((NODE_TYPE (arg_id) == N_id), "DupId_NT: no N_id node found!");
    new_id = DupNode (arg_id);

    DBUG_ASSERT ((ID_TYPE (arg_id) != NULL), "NT_TAG: no type found!");
    ID_NT_TAG (new_id) = CreateNtTag (ID_NAME (arg_id), ID_TYPE (arg_id));

    DBUG_RETURN (new_id);
}

/******************************************************************************
 *
 * Function:
 *   node *DupNode_NT( node *arg_node)
 *
 * Description:
 *   Duplicates a node. Sets ID_NT_TAG for N_id nodes.
 *
 ******************************************************************************/

node *
DupNode_NT (node *arg_node)
{
    node *new_node;

    DBUG_ENTER ("DupNode_NT");

    new_node = DupNode (arg_node);

    if (NODE_TYPE (arg_node) == N_id) {
        DBUG_ASSERT ((ID_TYPE (arg_node) != NULL), "NT_TAG: no type found!");
        ID_NT_TAG (new_node) = CreateNtTag (ID_NAME (arg_node), ID_TYPE (arg_node));
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DupExprs_NT( node *exprs)
 *
 * Description:
 *   Duplicates a N_exprs chain and transforms all N_id nodes found into
 *   tagged N_id nodes.
 *
 ******************************************************************************/

node *
DupExprs_NT (node *exprs)
{
    node *expr;
    node *new_exprs = NULL;

    DBUG_ENTER ("DupExprs");

    if (exprs != NULL) {
        DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "no N_exprs node found!");

        expr = EXPRS_EXPR (exprs);
        DBUG_ASSERT ((expr != NULL), "N_exprs node contains no data!");

        new_exprs = MakeExprs (DupNode_NT (expr), DupExprs_NT (EXPRS_NEXT (exprs)));
    }

    DBUG_RETURN (new_exprs);
}

/******************************************************************************
 *
 * Function:
 *   node *CheckAndDupSpecialFundef( node *module, node* fundef, node *assign)
 *
 * Returns:
 *   modified module node
 *
 * Description:
 *   Checks a special fundef for multiple uses. if there are more than one
 *   external assignment/function application, this given assignment is removed
 *   from the special function list of external assignments and added to
 *   the newly duplicated and renamed version of this fundef. the concerning
 *   assignment/function application is modified to call the new fundef.
 *   the new fundef is added to the global MODUL_FUNS chain of fundefs.
 *
 * remarks:
 *   because the global fundef chain is modified during the traversal of
 *   this function chain it is necessary not to overwrite the MODUL_FUNS
 *   attribute in the bottom-up traversal!!!
 *
 *****************************************************************************/

node *
CheckAndDupSpecialFundef (node *module, node *fundef, node *assign)
{
    node *new_fundef;
    char *new_name;

    DBUG_ENTER ("CheckAndDupSpecialFundef");

    DBUG_ASSERT ((NODE_TYPE (module) == N_modul), "given module node is not a module");
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "given fundef node is not a fundef");
    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign),
                 "given assign node is not an assignment");
    DBUG_ASSERT ((FUNDEF_IS_LACFUN (fundef)), "given fundef is not a special fundef");
    DBUG_ASSERT ((FUNDEF_USED (fundef) != USED_INACTIVE),
                 "FUNDEF_USED must be active for special functions!");
    DBUG_ASSERT ((FUNDEF_USED (fundef) > 0), "fundef is not used anymore");
    DBUG_ASSERT ((FUNDEF_EXT_ASSIGNS (fundef) != NULL),
                 "fundef has no external assignments");
    DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (assign)) == N_let),
                 "assignment contains no let");
    DBUG_ASSERT ((NODE_TYPE (ASSIGN_RHS (assign)) == N_ap),
                 "assignment is to application");
    DBUG_ASSERT ((AP_FUNDEF (ASSIGN_RHS (assign)) == fundef),
                 "application of different fundef than given fundef");
    DBUG_ASSERT ((NodeListFind (FUNDEF_EXT_ASSIGNS (fundef), assign) != NULL),
                 "given assignment is not element of external assignment list");

    if (FUNDEF_USED (fundef) > 1) {
        /* multiple uses - duplicate special fundef */
        DBUG_PRINT ("DUP", ("duplicating multiple fundef %s", FUNDEF_NAME (fundef)));

        new_fundef = DupNode (fundef);

        /* rename fundef */
        new_name = TmpVarName (FUNDEF_NAME (fundef));
        Free (FUNDEF_NAME (new_fundef));
        FUNDEF_NAME (new_fundef) = new_name;

        /* rename recursive funap (only do/while fundefs */
        if ((FUNDEF_STATUS (new_fundef) == ST_dofun)
            || (FUNDEF_STATUS (new_fundef) == ST_whilefun)) {
            DBUG_ASSERT ((FUNDEF_INT_ASSIGN (new_fundef) != NULL),
                         "missing link to recursive function call");

            Free (AP_NAME (ASSIGN_RHS (FUNDEF_INT_ASSIGN (new_fundef))));
            AP_NAME (ASSIGN_RHS (FUNDEF_INT_ASSIGN (new_fundef)))
              = StringCopy (FUNDEF_NAME (new_fundef));

            AP_FUNDEF (ASSIGN_RHS (FUNDEF_INT_ASSIGN (new_fundef))) = new_fundef;
        }

        /* init external assignment list */
        FUNDEF_EXT_ASSIGNS (new_fundef) = NodeListAppend (NULL, assign, NULL);
        FUNDEF_USED (new_fundef) = 1;

        /* rename the external assign/funap */
        Free (AP_NAME (ASSIGN_RHS (assign)));
        AP_NAME (ASSIGN_RHS (assign)) = StringCopy (new_name);
        AP_FUNDEF (ASSIGN_RHS (assign)) = new_fundef;

        /* add new fundef to global chain of fundefs */
        FUNDEF_NEXT (new_fundef) = MODUL_FUNS (module);
        MODUL_FUNS (module) = new_fundef;

        DBUG_ASSERT ((NodeListFind (FUNDEF_EXT_ASSIGNS (fundef), assign) != NULL),
                     "Assignment not found in FUNDEF_EXT_ASSIGNS!");

        /* remove assignment from old external assignment list */
        FUNDEF_EXT_ASSIGNS (fundef)
          = NodeListDelete (FUNDEF_EXT_ASSIGNS (fundef), assign, FALSE);

        (FUNDEF_USED (fundef))--;

        DBUG_ASSERT ((FUNDEF_USED (fundef) >= 0), "FUNDEF_USED dropped below 0");
    } else {
        /* only single use - no duplication needed */
    }

    DBUG_RETURN (module);
}
