/*
 *
 * $Log$
 * Revision 3.4  2001/01/09 17:26:01  dkr
 * N_WLstriVar renamed into N_WLstrideVar
 *
 * Revision 3.3  2000/12/12 12:07:53  dkr
 * NWITH_IN renamed into NWITH_IN_MASK, ...
 *
 * Revision 3.2  2000/12/06 19:26:53  dkr
 * DupTreePre renamed into DupTreeTravPre
 * DupTreePost renamed into DupTreeTravPost
 *
 * Revision 3.1  2000/11/20 18:03:19  sacbase
 * new release made
 *
 * Revision 1.41  2000/11/14 13:19:19  dkr
 * no '... might be used uninitialized' warnings anymore
 *
 * Revision 1.40  2000/11/05 13:39:18  dkr
 * bug in DupIds_Id fixed:
 * NAME and MOD are copied now ...
 *
 * Revision 1.39  2000/10/27 00:09:53  dkr
 * Pfffff... typo corrected. I should go to bed now 8-(((
 *
 * Revision 1.38  2000/10/27 00:04:37  dkr
 * Ups ... changes of revision 1.35 are missing in the current revision.
 * This is fixed now.
 *
 * Revision 1.37  2000/10/26 14:32:01  dkr
 * DupShpseg modified
 * All top-level functions no longer have an arg_info as argument :-))
 *
 * Revision 1.36  2000/10/23 10:40:05  dkr
 * DupIds_Id added
 *
 * Revision 1.35  2000/10/09 17:07:31  dkr
 * DupNodelist():
 *   wrong DBUG_ASSERT in DupNodelist removed.
 *   NODELIST_ATTRIB is duplicated now, too.
 *
 * Revision 1.34  2000/09/20 18:19:10  dkr
 * ID_MAKEUNIQUE renamed into ID_CLSCONV
 *
 * Revision 1.33  2000/07/31 10:45:52  cg
 * Eventually, the son ICM_NEXT is removed from the N_icm node.
 * The creation function MakeIcm is adjusted accordingly.
 *
 * Revision 1.32  2000/07/24 14:57:55  nmw
 * bug in DupTypes fixed. TYPES_STATUS was missing
 *
 * Revision 1.31  2000/07/14 13:44:04  dkr
 * DupModul and DupImplist added
 * fixed a bug in DupFundef
 *
 * Revision 1.30  2000/07/14 10:10:49  dkr
 * CopyNodelist() moved from tree_compound.c to DupTree.c and renamed into
 *   DupNodelist().
 * DBUG_ASSERTs for DupIds, DupOneIds, DupNodelist added to assure that
 *   (arg_info == NULL) is hold if these functions are called top-level.
 *
 * Revision 1.29  2000/07/13 11:59:29  jhs
 * Splited ICM_INDENT into ICM_INDENT_BEFORE and ICM_INDENT_AFTER.
 *
 * Revision 1.28  2000/07/13 07:14:13  jhs
 * Added Comments for DupDFMmask, DupMt, DupSt, DupMTsignal, DupMTsync
 * and DupMTalloc.
 *
 * Revision 1.27  2000/07/12 15:22:07  dkr
 * code brushed:
 * DuplicateTypes removed (use DupTypes instead!)
 * INFO_DUP_BASEFUNDEF and INFO_DUP_DFMBASE removed
 * Duplication of DFM-bases and -masks completed and simplified
 *
 * Revision 1.26  2000/07/11 15:42:51  jhs
 * Changed usage of DFMDuplicateMask to DupDFMmask,
 * DFM_BASE will be provided by lut now, not via special
 * flag at arg_info.
 *
 * Revision 1.25  2000/07/11 10:25:26  dkr
 * macro GEN_NODE replaced by MALLOC
 *
 * Revision 1.24  2000/07/04 14:38:11  jhs
 * Added Dups for MTalloc, MTsignal, MTsync.
 *
 * Revision 1.23  2000/06/23 15:33:52  dkr
 * function DupTreeInfo added
 * signature of DupTree changed
 *
 * Revision 1.22  2000/06/23 14:04:01  dkr
 * NWITH_COMPLEX removed
 *
 * Revision 1.21  2000/06/14 12:04:02  jhs
 * Dups ST_IDENTIFIER and MT_IDENTIFIER now.
 *
 * Revision 1.20  2000/03/31 12:25:43  jhs
 * Added duplication of dfmmasks at lte, return, mt, st.
 *
 * Revision 1.19  2000/03/24 00:50:39  dkr
 * the LUT is now part of arg_info :)
 *
 * Revision 1.18  2000/03/23 17:34:39  dkr
 * ARG_OBJDEF and VARDEC_OBJDEF are duplicated now
 *
 * Revision 1.17  2000/03/21 13:14:04  jhs
 * Fixed bug.
 *
 * Revision 1.16  2000/03/17 18:30:53  dkr
 * type lut_t* replaced by LUT_t
 *
 * Revision 1.15  2000/03/15 12:59:16  dkr
 * macro DUPVECT added
 * WL..._INNERSTEP removed
 *
 * Revision 1.14  2000/03/09 18:36:12  jhs
 * Comments, comments, comments, ...
 * DFMbases are copied now ...
 * All traversals Dup(Tree|Node)[LUT] based on new function
 * DupTreeOrNodeLUT now.
 *
 * Revision 1.13  2000/03/02 13:06:30  jhs
 * Added DupSt and DupMt.
 *
 * Revision 1.12  2000/02/24 15:55:53  dkr
 * RETURN_INWITH removed
 * (needed for old with-loop only, therefore obsolete now)
 *
 * Revision 1.11  2000/02/22 12:00:04  jhs
 * Adapted NODE_TEXT.
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
 * [...]
 *
 * Revision 2.1  1999/02/23 12:41:17  sacbase
 * new release made
 *
 * Revision 1.107  1998/08/11 14:36:00  dkr
 * DupWLstriVar, DupWLgridVar changed
 *
 * [ ... ]
 *
 * Revision 1.1  1995/05/01  15:32:27  asi
 * Initial revision
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
 ******************************************************************************/

#include <string.h>

#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"

#include "DupTree.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "LookUpTable.h"
#include "Inline.h"
#include "typecheck.h"
#include "scheduling.h"
#include "generatemasks.h"

/*
 *  always traverses son 'node'
 *
 *  The macro is to be used within traversal functions where arg_node and
 *  arg_info exist.
 */
#define DUPTRAV(node) (node != NULL) ? Trav (node, arg_info) : NULL

/*
 *  If INFO_DUP_CONT(arg_info) contains the root of syntaxtree.
 *    -> traverses son 'node' if and only if its parent is not the root.
 *  If INFO_DUP_CONT(arg_info) is NULL.
 *    -> traverses son 'node'.
 *
 *  The macro is to be used within traversal functions where arg_node and
 *  arg_info exist.
 */
#define DUPCONT(node) (INFO_DUP_CONT (arg_info) != arg_node) ? DUPTRAV (node) : NULL

/******************************************************************************
 *
 * function:
 *   static node *DupTreeOrNodeLUT( int NodeOnly,
 *                                  node *arg_node,
 *                                  node *arg_info,
 *                                  LUT_t lut)
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
 *   - arg_info:
 *       == NULL : An internal Arg_info is created for the traversal.
 *       != NULL : The given arg_info is used for the traversal,
 *                 flags and data can be stored in arg_info by caller.
 *                 Most of such usages is undocumented!!!
 *                 So the usage of arg_info is not very nice.
 *                 Some (not all) of what is done is the following:
 *                 - If the given arg_info is NULL, everything is copied in
 *                   DUP_NORMAL mode, else the mode is taken from INFO_DUP_TYPE.
 *                 - If the given arg_info is NULL, INFO_DUP_ALL is set to
 *                   FALSE, else the value from a handed over arg_info is
 *                   reused.
 *                   If INFO_DUP_ALL is TRUE some masks of N_assign are copied,
 *                   which is not done otherwise.
 *                 The duplication needs to store some values at the arg_info
 *                 itself, so their could be problems with colliding accesses!
 *   - lut:
 *       If you want to use yout own LUT you can hand over it here.
 *
 ******************************************************************************/

static node *
DupTreeOrNodeLUT (int NodeOnly, node *arg_node, node *arg_info, LUT_t lut)
{
    funtab *old_tab;
    node *new_node, *old_fundef;
    bool own_arg_info;

    DBUG_ENTER ("DupTreeOrNodeLUT");

    if (arg_node != NULL) {
        old_tab = act_tab;
        act_tab = dup_tab;

        /*
         *  If there is no arg_info given, we create a new one, that will
         *  be deleted after traversal. Some standard values are set.
         */
        if (arg_info == NULL) {
            arg_info = MakeInfo ();
            INFO_DUP_TYPE (arg_info) = DUP_NORMAL;
            INFO_DUP_ALL (arg_info) = FALSE;
            own_arg_info = TRUE;
            old_fundef = NULL;
        } else {
            own_arg_info = FALSE;
            old_fundef = INFO_DUP_FUNDEF (arg_info);
        }

        /*
         *  Via this (ugly) macro DUPCONT the decision to copy the whole tree
         *  starting from arg_node or only the node itself (meaning not to
         *  traverse and copy xxx_NEXT) is done.
         *  DUPCONT compares the actual arg_node of a traversal function with the
         *  value in INFO_DUP_CONT. If they are the same the xxx_NEXT will be
         *  ignored, otherwise it will be traversed. If the start-node is stored as
         *  INFO_DUP_CONT it's xx_NEXT will not be duplicated, but the xxx_NEXT's
         *  of all sons are copied, because they differ from INFO_DUP_CONT.
         *  If NULL is stored in INFO_DUP_CONT (and in a traversal the arg_node
         *  never is NULL) all nodes and their xxx_NEXT's are duplicated.
         *  So we set INFO_DUP_CONT with NULL to copy all, arg_node to copy
         *  start_node (as decribed above) only.
         */
        if (NodeOnly) {
            INFO_DUP_CONT (arg_info) = arg_node;
        } else {
            INFO_DUP_CONT (arg_info) = NULL;
        }

        if (lut == NULL) {
            INFO_DUP_LUT (arg_info) = GenerateLUT ();
        } else {
            INFO_DUP_LUT (arg_info) = lut;
        }

        new_node = Trav (arg_node, arg_info);

        if (lut == NULL) {
            INFO_DUP_LUT (arg_info) = RemoveLUT (INFO_DUP_LUT (arg_info));
        }

        if (own_arg_info) {
            arg_info = FreeNode (arg_info);
        } else {
            /* pop information */
            INFO_DUP_FUNDEF (arg_info) = old_fundef;
        }

        act_tab = old_tab;
    } else {
        new_node = NULL;
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
 *   DFMmask_t DupDFMmask( DFMmask_t mask, node *arg_info)
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
DupDFMmask (DFMmask_t mask, node *arg_info)
{
    DFMmask_t new_mask;

    DBUG_ENTER ("DupDFMmask");

    if (mask != NULL) {
        new_mask = DFMDuplicateMask (mask, SearchInLUT (INFO_DUP_LUT (arg_info),
                                                        DFMGetMaskBase (mask)));
    } else {
        new_mask = NULL;
    }

    DBUG_RETURN (new_mask);
}

/******************************************************************************
 *
 * Function:
 *   ids *DupIds_( ids *old_ids, node *arg_info)
 *
 * Remark:
 *   'arg_info' might be NULL, because this function is not only used by
 *   the traversal mechanism but also by DupAllIds()!
 *
 ******************************************************************************/

static ids *
DupIds_ (ids *old_ids, node *arg_info)
{
    ids *new_ids;

    DBUG_ENTER ("DupIds_");

    DBUG_ASSERT ((old_ids != NULL), "DupIds_: cannot duplicate a NULL pointer");

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
        IDS_VARDEC (new_ids)
          = SearchInLUT ((arg_info != NULL) ? INFO_DUP_LUT (arg_info) : NULL,
                         IDS_VARDEC (old_ids));
        IDS_USE (new_ids) = IDS_USE (old_ids);
    }

    IDS_ATTRIB (new_ids) = IDS_ATTRIB (old_ids);
    IDS_REFCNT (new_ids) = IDS_REFCNT (old_ids);
    IDS_NAIVE_REFCNT (new_ids) = IDS_NAIVE_REFCNT (old_ids);

    if (IDS_NEXT (old_ids) != NULL) {
        IDS_NEXT (new_ids) = DupIds_ (IDS_NEXT (old_ids), arg_info);
    }

    DBUG_RETURN (new_ids);
}

/******************************************************************************
 *
 * Function:
 *   shpseg *DupShpseg_( shpseg *old_shpseg, node *arg_info)
 *
 * Remark:
 *   'arg_info' might be NULL, because this function is not only used by
 *   the traversal mechanism but also by DupShpseg()!
 *
 ******************************************************************************/

static shpseg *
DupShpseg_ (shpseg *old_shpseg, node *arg_info)
{
    int i;
    shpseg *new_shpseg;

    DBUG_ENTER ("DupShpseg_");

    if (old_shpseg != NULL) {
        new_shpseg = MakeShpseg (NULL);
        for (i = 0; i < SHP_SEG_SIZE; i++) {
            SHPSEG_SHAPE (new_shpseg, i) = SHPSEG_SHAPE (old_shpseg, i);
        }

        if (SHPSEG_NEXT (old_shpseg) != NULL) {
            SHPSEG_NEXT (new_shpseg) = DupShpseg_ (SHPSEG_NEXT (old_shpseg), arg_info);
        }
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
 *   the traversal mechanism but also by DupTypes()!
 *
 ******************************************************************************/

static types *
DupTypes_ (types *old_types, node *arg_info)
{
    types *new_types;

    DBUG_ENTER ("DupTypes_");

    if (old_types == NULL) {
        new_types = NULL;
    } else {
        new_types = MakeTypes (TYPES_BASETYPE (old_types), TYPES_DIM (old_types),
                               DupShpseg_ (TYPES_SHPSEG (old_types), arg_info),
                               StringCopy (TYPES_NAME (old_types)),
                               StringCopy (TYPES_MOD (old_types)));

        TYPES_TDEF (new_types) = TYPES_TDEF (old_types);
        TYPES_STATUS (new_types) = TYPES_STATUS (old_types);

        DBUG_PRINT ("TYPE", ("new type" P_FORMAT ",old " P_FORMAT, new_types, old_types));
        DBUG_PRINT ("TYPE", ("new name" P_FORMAT ", old name" P_FORMAT,
                             TYPES_NAME (new_types), TYPES_NAME (old_types)));

        new_types->id = StringCopy (old_types->id);
        new_types->id_mod = StringCopy (old_types->id_mod);
        new_types->id_cmod = StringCopy (old_types->id_cmod);
        new_types->attrib = old_types->attrib;
        new_types->status = old_types->status;

        if (TYPES_NEXT (old_types) != NULL) {
            TYPES_NEXT (new_types) = DupTypes_ (TYPES_NEXT (old_types), arg_info);
        }
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

    if (nl == NULL) {
        new_nl = NULL;
    } else {
        new_nl = MakeNodelist (SearchInLUT (INFO_DUP_LUT (arg_info), NODELIST_NODE (nl)),
                               NODELIST_STATUS (nl),
                               DupNodelist_ (NODELIST_NEXT (nl), arg_info));
        NODELIST_ATTRIB (new_nl) = NODELIST_ATTRIB (nl);
    }

    DBUG_RETURN (new_nl);
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

    if (INFO_DUP_TYPE (arg_info) == DUP_INLINE) {
        new_node = MakeId (RenameInlinedVar (ID_NAME (arg_node)),
                           StringCopy (ID_MOD (arg_node)), ID_STATUS (arg_node));
        /* ID_OBJDEF and ID_VARDEC are mapped to the same data object */
        ID_VARDEC (new_node) = SearchDecl (ID_NAME (new_node), INFO_INL_TYPES (arg_info));
    } else {
        new_node = MakeId (StringCopy (ID_NAME (arg_node)),
                           StringCopy (ID_MOD (arg_node)), ID_STATUS (arg_node));
        /* ID_OBJDEF and ID_VARDEC are mapped to the same data object */
        ID_VARDEC (new_node)
          = SearchInLUT (INFO_DUP_LUT (arg_info), ID_VARDEC (arg_node));
    }
    ID_DEF (new_node) = SearchInLUT (INFO_DUP_LUT (arg_info), ID_DEF (arg_node));

    ID_ATTRIB (new_node) = ID_ATTRIB (arg_node);
    ID_REFCNT (new_node) = ID_REFCNT (arg_node);
    ID_NAIVE_REFCNT (new_node) = ID_NAIVE_REFCNT (arg_node);
    ID_CLSCONV (new_node) = ID_CLSCONV (arg_node);

    if (DUP_WLF == INFO_DUP_TYPE (arg_info)) {
        /* Withloop folding (wlf) needs this. */
        if (ID_WL (arg_node) && (N_id == NODE_TYPE (ID_WL (arg_node)))) {
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
                         DupTypes_ (CAST_TYPE (arg_node), arg_info));

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

    RETURN_REFERENCE (new_node) = DUPTRAV (RETURN_REFERENCE (arg_node));

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    INFO_DUP_LUT (arg_info) = InsertIntoLUT (INFO_DUP_LUT (arg_info), arg_node, new_node);

    RETURN_USEMASK (new_node) = DupDFMmask (RETURN_USEMASK (arg_node), arg_info);
    RETURN_DEFMASK (new_node) = DupDFMmask (RETURN_DEFMASK (arg_node), arg_info);

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

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupFundef (node *arg_node, node *arg_info)
{
    node *new_node, *old_fundef;

    DBUG_ENTER ("DupFundef");

    /*
     *  We can't copy the FUNDEF_DFM_BASE and DFMmasks belonging to this base
     *  directly!
     *  Such DFMmasks are attached to N_with, N_with2, N_sync and N_spmd.
     *  All of them can be found in the body of the function.
     *  But when we copy the body we must already know the base to create the
     *  new masks. On the other hand to create the base we must already have
     *  the new FUNDEF_ARGS and FUNDEF_VARDECS available.
     *  Therefore we first create the raw function without the body via
     *  MakeFundef(), then we create the base while duplicating the body
     *  and finally we attach the body to the fundef.
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
                           DUPTRAV (FUNDEF_ARGS (arg_node)), NULL,
                           DUPCONT (FUNDEF_NEXT (arg_node)));

    INFO_DUP_LUT (arg_info) = InsertIntoLUT (INFO_DUP_LUT (arg_info), arg_node, new_node);

    /* copy the body */
    FUNDEF_BODY (new_node) = DUPTRAV (FUNDEF_BODY (arg_node));

    /* store new DFMbase */
    FUNDEF_DFM_BASE (new_node)
      = SearchInLUT (INFO_DUP_LUT (arg_info), FUNDEF_DFM_BASE (arg_node));

    /* now we copy all the other things ... */
    FUNDEF_LINKMOD (new_node) = StringCopy (FUNDEF_LINKMOD (arg_node));
    FUNDEF_STATUS (new_node) = FUNDEF_STATUS (arg_node);
    FUNDEF_ATTRIB (new_node) = FUNDEF_ATTRIB (arg_node);
    FUNDEF_INLINE (new_node) = FUNDEF_INLINE (arg_node);
    FUNDEF_FUNNO (new_node) = FUNDEF_FUNNO (arg_node);
    FUNDEF_PRAGMA (new_node) = DUPTRAV (FUNDEF_PRAGMA (arg_node));

    FUNDEF_RETURN (new_node)
      = SearchInLUT (INFO_DUP_LUT (arg_info), FUNDEF_RETURN (arg_node));
    FUNDEF_NEEDOBJS (new_node) = DupNodelist_ (FUNDEF_NEEDOBJS (arg_node), arg_info);
    FUNDEF_VARNO (new_node) = FUNDEF_VARNO (arg_node);
    FUNDEF_INLREC (new_node) = FUNDEF_INLREC (arg_node);

#if 0
  FUNDEF_SIB( new_node) = ???;
  FUNDEF_ICM( new_node) = ???;
  FUNDEF_MASK( new_node, ?) = ???;
  FUNDEF_LIFTEDFROM( new_node) = ???;
  FUNDEC_DEF( new_node) = ???;
#endif

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    INFO_DUP_FUNDEF (arg_info) = old_fundef;

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupBlock (node *arg_node, node *arg_info)
{
    node *new_vardecs;
    node *new_node;
    DFMmask_base_t old_base, new_base;

    DBUG_ENTER ("DupBlock");

    new_vardecs = DUPTRAV (BLOCK_VARDEC (arg_node));

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
        new_base = DFMGenMaskBase (FUNDEF_ARGS (
                                     ((node *)SearchInLUT (INFO_DUP_LUT (arg_info),
                                                           INFO_DUP_FUNDEF (arg_info)))),
                                   new_vardecs);

        INFO_DUP_LUT (arg_info)
          = InsertIntoLUT (INFO_DUP_LUT (arg_info), old_base, new_base);
    }

    new_node = MakeBlock (DUPTRAV (BLOCK_INSTR (arg_node)), new_vardecs);
    BLOCK_CACHESIM (new_node) = StringCopy (BLOCK_CACHESIM (arg_node));

    BLOCK_VARNO (new_node) = BLOCK_VARNO (arg_node);
    BLOCK_NEEDFUNS (new_node) = DupNodelist_ (BLOCK_NEEDFUNS (arg_node), arg_info);
    BLOCK_NEEDTYPES (new_node) = DupNodelist_ (BLOCK_NEEDTYPES (arg_node), arg_info);
#if 0
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
                            DupTypes_ (TYPEDEF_TYPE (arg_node), arg_info),
                            TYPEDEF_ATTRIB (arg_node), DUPCONT (TYPEDEF_NEXT (arg_node)));

    INFO_DUP_LUT (arg_info) = InsertIntoLUT (INFO_DUP_LUT (arg_info), arg_node, new_node);

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
                    DupTypes_ (OBJDEF_TYPE (arg_node), arg_info),
                    DUPTRAV (OBJDEF_EXPR (arg_node)), DUPCONT (OBJDEF_NEXT (arg_node)));

    INFO_DUP_LUT (arg_info) = InsertIntoLUT (INFO_DUP_LUT (arg_info), arg_node, new_node);

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

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    INFO_DUP_LUT (arg_info) = InsertIntoLUT (INFO_DUP_LUT (arg_info), arg_node, new_node);

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

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    INFO_DUP_LUT (arg_info) = InsertIntoLUT (INFO_DUP_LUT (arg_info), arg_node, new_node);

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
        new_ids = DupIds_ (LET_IDS (arg_node), arg_info);
    } else {
        new_ids = NULL;
    }

    new_node = MakeLet (DUPTRAV (LET_EXPR (arg_node)), new_ids);

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

    LET_USEMASK (new_node) = DupDFMmask (LET_USEMASK (arg_node), arg_info);
    LET_DEFMASK (new_node) = DupDFMmask (LET_DEFMASK (arg_node), arg_info);

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
        COND_THENVARS (new_node) = DupIds_ (COND_THENVARS (arg_node), arg_info);
    }
    if (COND_ELSEVARS (arg_node) != NULL) {
        COND_ELSEVARS (new_node) = DupIds_ (COND_ELSEVARS (arg_node), arg_info);
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
        DO_USEVARS (new_node) = DupIds_ (DO_USEVARS (arg_node), arg_info);
    }
    if (DO_DEFVARS (arg_node) != NULL) {
        DO_DEFVARS (new_node) = DupIds_ (DO_DEFVARS (arg_node), arg_info);
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
        WHILE_USEVARS (new_node) = DupIds_ (WHILE_USEVARS (arg_node), arg_info);
    }
    if (WHILE_DEFVARS (arg_node) != NULL) {
        WHILE_DEFVARS (new_node) = DupIds_ (WHILE_DEFVARS (arg_node), arg_info);
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

    AP_FUNDEF (new_node) = SearchInLUT (INFO_DUP_LUT (arg_info), AP_FUNDEF (arg_node));

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

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
          = (int *)MALLOC (PRAGMA_NUMPARAMS (new_node) * sizeof (int));
        for (i = 0; i < PRAGMA_NUMPARAMS (new_node); i++) {
            PRAGMA_LINKSIGN (new_node)[i] = PRAGMA_LINKSIGN (arg_node)[i];
        }
    }
    if (PRAGMA_REFCOUNTING (arg_node) != NULL) {
        PRAGMA_REFCOUNTING (new_node)
          = (int *)MALLOC (PRAGMA_NUMPARAMS (new_node) * sizeof (int));
        for (i = 0; i < PRAGMA_NUMPARAMS (new_node); i++) {
            PRAGMA_REFCOUNTING (new_node)[i] = PRAGMA_REFCOUNTING (arg_node)[i];
        }
    }
    PRAGMA_INITFUN (new_node) = StringCopy (PRAGMA_INITFUN (arg_node));
    PRAGMA_WLCOMP_APS (new_node) = DUPTRAV (PRAGMA_WLCOMP_APS (arg_node));

    if (PRAGMA_EFFECT (arg_node) != NULL) {
        PRAGMA_EFFECT (new_node) = DupIds_ (PRAGMA_EFFECT (arg_node), arg_info);
    }
    if (PRAGMA_TOUCH (arg_node) != NULL) {
        PRAGMA_TOUCH (new_node) = DupIds_ (PRAGMA_TOUCH (arg_node), arg_info);
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

    new_node = MakeIcm (ICM_NAME (arg_node), DUPTRAV (ICM_ARGS (arg_node)));

    /*
     * The ICM name is not copied here because ICM names are predominantly static
     * string constants and therefore aren't freed anyway.
     */

    ICM_INDENT_BEFORE (new_node) = ICM_INDENT_BEFORE (arg_node);
    ICM_INDENT_AFTER (new_node) = ICM_INDENT_AFTER (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupSpmd (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupSpmd");

    new_node = MakeSpmd (DUPTRAV (SPMD_REGION (arg_node)));

    SPMD_IN (new_node) = DupDFMmask (SPMD_IN (arg_node), arg_info);
    SPMD_INOUT (new_node) = DupDFMmask (SPMD_INOUT (arg_node), arg_info);
    SPMD_OUT (new_node) = DupDFMmask (SPMD_OUT (arg_node), arg_info);
    SPMD_LOCAL (new_node) = DupDFMmask (SPMD_LOCAL (arg_node), arg_info);
    SPMD_SHARED (new_node) = DupDFMmask (SPMD_SHARED (arg_node), arg_info);

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

    SYNC_IN (new_node) = DupDFMmask (SYNC_IN (arg_node), arg_info);
    SYNC_INOUT (new_node) = DupDFMmask (SYNC_INOUT (arg_node), arg_info);
    SYNC_OUT (new_node) = DupDFMmask (SYNC_OUT (arg_node), arg_info);
    SYNC_OUTREP (new_node) = DupDFMmask (SYNC_OUTREP (arg_node), arg_info);
    SYNC_LOCAL (new_node) = DupDFMmask (SYNC_LOCAL (arg_node), arg_info);

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

    if (NWITH_DEC_RC_IDS (arg_node) != NULL) {
        NWITH_DEC_RC_IDS (new_node) = DupIds_ (NWITH_DEC_RC_IDS (arg_node), arg_info);
    }

    NWITH_IN_MASK (new_node) = DupDFMmask (NWITH_IN_MASK (arg_node), arg_info);
    NWITH_OUT_MASK (new_node) = DupDFMmask (NWITH_OUT_MASK (arg_node), arg_info);
    NWITH_LOCAL_MASK (new_node) = DupDFMmask (NWITH_LOCAL_MASK (arg_node), arg_info);

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
                   SearchInLUT (INFO_DUP_LUT (arg_info), NPART_CODE (arg_node)));

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
        NCODE_INC_RC_IDS (new_node) = DupIds_ (NCODE_INC_RC_IDS (arg_node), arg_info);
    }
#if 0
  NCODE_MASK( new_node, ?) = ???;
#endif

    INFO_DUP_LUT (arg_info) = InsertIntoLUT (INFO_DUP_LUT (arg_info), arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupNwithid (node *arg_node, node *arg_info)
{
    node *new_node;
    ids *_vec, *_ids;

    DBUG_ENTER ("DupNwithid");

    _vec = (NWITHID_VEC (arg_node) != NULL) ? DupIds_ (NWITHID_VEC (arg_node), arg_info)
                                            : NULL;
    _ids = (NWITHID_IDS (arg_node) != NULL) ? DupIds_ (NWITHID_IDS (arg_node), arg_info)
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
        NWITH2_DEC_RC_IDS (new_node) = DupIds_ (NWITH2_DEC_RC_IDS (arg_node), arg_info);
    }

    NWITH2_IN_MASK (new_node) = DupDFMmask (NWITH2_IN_MASK (arg_node), arg_info);
    NWITH2_OUT_MASK (new_node) = DupDFMmask (NWITH2_OUT_MASK (arg_node), arg_info);
    NWITH2_LOCAL_MASK (new_node) = DupDFMmask (NWITH2_LOCAL_MASK (arg_node), arg_info);

    if (NWITH2_SCHEDULING (arg_node) != NULL) {
        NWITH2_SCHEDULING (new_node) = SCHCopyScheduling (NWITH2_SCHEDULING (arg_node));
    }
    NWITH2_ISSCHEDULED (new_node) = NWITH2_ISSCHEDULED (arg_node);

    NODE_LINE (new_node) = NODE_LINE (arg_node);
    NODE_FILE (new_node) = NODE_FILE (arg_node);

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

    DUPVECT (WLSEG_IDX_MIN (new_node), WLSEG_IDX_MIN (arg_node), WLSEG_DIMS (new_node),
             int);
    DUPVECT (WLSEG_IDX_MAX (new_node), WLSEG_IDX_MAX (arg_node), WLSEG_DIMS (new_node),
             int);

    WLSEG_BLOCKS (new_node) = WLSEG_BLOCKS (arg_node);

    for (i = 0; i < WLSEG_BLOCKS (new_node); i++) {
        DUPVECT (WLSEG_BV (new_node, i), WLSEG_BV (arg_node, i), WLSEG_DIMS (new_node),
                 int);
    }

    DUPVECT (WLSEG_UBV (new_node), WLSEG_UBV (arg_node), WLSEG_DIMS (new_node), int);
    DUPVECT (WLSEG_SV (new_node), WLSEG_SV (arg_node), WLSEG_DIMS (new_node), int);

    if (WLSEG_SCHEDULING (arg_node) != NULL) {
        WLSEG_SCHEDULING (new_node) = SCHCopyScheduling (WLSEG_SCHEDULING (arg_node));
    }
    WLSEG_MAXHOMDIM (new_node) = WLSEG_MAXHOMDIM (arg_node);
    DUPVECT (WLSEG_HOMSV (new_node), WLSEG_HOMSV (arg_node), WLSEG_DIMS (new_node), int);

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
                    SearchInLUT (INFO_DUP_LUT (arg_info), WLGRID_CODE (arg_node)));

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
    int i;

    DBUG_ENTER ("DupWLsegVar");

    new_node
      = MakeWLsegVar (WLSEGVAR_DIMS (arg_node), DUPTRAV (WLSEGVAR_CONTENTS (arg_node)),
                      DUPCONT (WLSEGVAR_NEXT (arg_node)));

    DUPVECT (WLSEGVAR_IDX_MIN (new_node), WLSEGVAR_IDX_MIN (arg_node),
             WLSEGVAR_DIMS (new_node), int);
    DUPVECT (WLSEGVAR_IDX_MAX (new_node), WLSEGVAR_IDX_MAX (arg_node),
             WLSEGVAR_DIMS (new_node), int);

    WLSEGVAR_BLOCKS (new_node) = WLSEGVAR_BLOCKS (arg_node);

    for (i = 0; i < WLSEGVAR_BLOCKS (new_node); i++) {
        DUPVECT (WLSEGVAR_BV (new_node, i), WLSEGVAR_BV (arg_node, i),
                 WLSEGVAR_DIMS (new_node), int);
    }

    DUPVECT (WLSEGVAR_UBV (new_node), WLSEGVAR_UBV (arg_node), WLSEGVAR_DIMS (new_node),
             int);
    DUPVECT (WLSEGVAR_SV (new_node), WLSEGVAR_SV (arg_node), WLSEGVAR_DIMS (new_node),
             int);

    if (WLSEGVAR_SCHEDULING (arg_node) != NULL) {
        WLSEGVAR_SCHEDULING (new_node)
          = SCHCopyScheduling (WLSEGVAR_SCHEDULING (arg_node));
    }

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

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DupWLgridVar (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupWLgridVar");

    new_node
      = MakeWLgridVar (WLGRIDVAR_LEVEL (arg_node), WLGRIDVAR_DIM (arg_node),
                       DUPTRAV (WLGRIDVAR_BOUND1 (arg_node)),
                       DUPTRAV (WLGRIDVAR_BOUND2 (arg_node)),
                       DUPTRAV (WLGRIDVAR_NEXTDIM (arg_node)),
                       DUPCONT (WLGRIDVAR_NEXT (arg_node)),
                       SearchInLUT (INFO_DUP_LUT (arg_info), WLGRIDVAR_CODE (arg_node)));

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

    MT_USEMASK (new_node) = DupDFMmask (MT_USEMASK (arg_node), arg_info);
    MT_DEFMASK (new_node) = DupDFMmask (MT_DEFMASK (arg_node), arg_info);
    MT_NEEDLATER (new_node) = DupDFMmask (MT_NEEDLATER (arg_node), arg_info);

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

    ST_USEMASK (new_node) = DupDFMmask (ST_USEMASK (arg_node), arg_info);
    ST_DEFMASK (new_node) = DupDFMmask (ST_DEFMASK (arg_node), arg_info);
    ST_NEEDLATER_ST (new_node) = DupDFMmask (ST_NEEDLATER_ST (arg_node), arg_info);
    ST_NEEDLATER_MT (new_node) = DupDFMmask (ST_NEEDLATER_MT (arg_node), arg_info);

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

    MTSIGNAL_IDSET (new_node) = DupDFMmask (MTSIGNAL_IDSET (arg_node), arg_info);

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

    MTSYNC_WAIT (new_node) = DupDFMmask (MTSYNC_WAIT (arg_node), arg_info);
    MTSYNC_FOLD (new_node) = CopyDFMfoldmask (MTSYNC_FOLD (arg_node));
    MTSYNC_ALLOC (new_node) = DupDFMmask (MTSYNC_WAIT (arg_node), arg_info);

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

    MTALLOC_IDSET (new_node) = DupDFMmask (MTALLOC_IDSET (arg_node), arg_info);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * functions:
 *   node *DupTreeLUT( node *arg_node, LUT_t lut)
 *   node *DupTree( node *arg_node)
 *   node *DupTreeInfo( node *arg_node, node *arg_info)
 *   node *DupNodeLUT( node *arg_node, LUT_t lut)
 *   node *DupNode( node *arg_node)
 *
 * description:
 *   Copying of trees and nodes ...
 *   The node to be copied is arg_node.
 *
 *   Which function do I use???
 *   - If you want to copy a whole tree use DupTree or DupTreeLUT.
 *     If you want to copy a node only (that means the node and all it's
 *     attributes but not the xxx_NEXT) use DupNode or DupNodeLUT.
 *   - If you want to use a special LookUpTable (LUT) use the specific
 *     DupXxxLUT version otherwise you use DupXxx only
 *     (If you dont't know what a LUT is good for use DupXxx).
 *
 * attention:
 *   DupTreeInfo can be used to bring information into the duplication-traversal
 *   (via the argument 'arg_info').
 *   The use of this function is NOT RECOMMENDED!!
 *
 ******************************************************************************/

node *
DupTreeLUT (node *arg_node, LUT_t lut)
{
    node *new_node;

    DBUG_ENTER ("DupTreeLUT");

    new_node = DupTreeOrNodeLUT (FALSE, arg_node, NULL, lut);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DupTree (node *arg_node)
{
    node *new_node;

    DBUG_ENTER ("DupTree");

    new_node = DupTreeOrNodeLUT (FALSE, arg_node, NULL, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DupTreeInfo (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupTreeInfo");

    new_node = DupTreeOrNodeLUT (FALSE, arg_node, arg_info, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DupNodeLUT (node *arg_node, LUT_t lut)
{
    node *new_node;

    DBUG_ENTER ("DupNodeLUT");

    new_node = DupTreeOrNodeLUT (TRUE, arg_node, NULL, lut);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DupNode (node *arg_node)
{
    node *new_node;

    DBUG_ENTER ("DupTree");

    new_node = DupTreeOrNodeLUT (TRUE, arg_node, NULL, NULL);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * Function:
 *   ids *DupOneIds( ids *old_ids)
 *
 * Description:
 *   Duplicates the first IDS of the given IDS chain.
 *
 ******************************************************************************/

ids *
DupOneIds (ids *old_ids)
{
    ids *new_ids, *tmp;

    DBUG_ENTER ("DupOneIds");

    tmp = IDS_NEXT (old_ids);
    IDS_NEXT (old_ids) = NULL;
    new_ids = DupAllIds (old_ids);
    IDS_NEXT (old_ids) = tmp;

    DBUG_RETURN (new_ids);
}

/******************************************************************************
 *
 * Function:
 *   ids *DupAllIds( ids *old_ids)
 *
 * Description:
 *   Duplicates an IDS chain.
 *
 ******************************************************************************/

ids *
DupAllIds (ids *old_ids)
{
    ids *new_ids;

    DBUG_ENTER ("DupAllIds");

    new_ids = DupIds_ (old_ids, NULL);

    DBUG_RETURN (new_ids);
}

/******************************************************************************
 *
 * Function:
 *   shpseg *DupShpseg( shpseg *old_shpseg)
 *
 * Description:
 *
 *
 ******************************************************************************/

shpseg *
DupShpseg (shpseg *old_shpseg)
{
    shpseg *new_shpseg;

    DBUG_ENTER ("DupShpseg");

    new_shpseg = DupShpseg_ (old_shpseg, NULL);

    DBUG_RETURN (new_shpseg);
}

/******************************************************************************
 *
 * Function:
 *   types *DupTypes( types* type)
 *
 * Description:
 *
 *
 ******************************************************************************/

types *
DupTypes (types *old_types)
{
    types *new_types;

    DBUG_ENTER ("DupTypes");

    new_types = DupTypes_ (old_types, NULL);

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

    new_nl = DupNodelist_ (nl, NULL);

    DBUG_RETURN (new_nl);
}

/******************************************************************************
 *
 * Function:
 *   node *DupIds_Id( ids *old_ids)
 *
 * Description:
 *   Duplicates an IDS and returns a *N_id* node.
 *
 ******************************************************************************/

node *
DupIds_Id (ids *old_ids)
{
    node *new_id;

    DBUG_ENTER ("DupIds_Id");

    new_id = MakeId (NULL, NULL, ST_regular);
    ID_IDS (new_id) = DupOneIds (old_ids);

    DBUG_RETURN (new_id);
}

/******************************************************************************
 *
 * Function:
 *   ids *DupId_Ids( node *old_id)
 *
 * Description:
 *   Duplicates an IDS and returns a *N_id* node.
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
