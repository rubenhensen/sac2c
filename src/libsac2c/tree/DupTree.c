/******************************************************************************
 *
 * PREFIX: DUP
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
 *   Note that DUPTRAV, unlike usual traversal functions, does not fail if its
 *   argument is NULL. Instead, it is the identity on the argument in this case.
 *
 * NB. DUP does NOT, with rare exceptions, copy flags, and it apparently
 *     never copies attributes. I consider both of these to be bugs,
 *     but am leary about messing with it right now.
 *
 *     See FLAGSTRUCTURE references for dealing with flags. This could
 *     better be handled by xml-generated code, or even by the cruder
 *     switch() in DupFlags
 *
 *
 ******************************************************************************/

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "shape.h"
#include "free.h"

#define DBUG_PREFIX "DUP"
#include "debug.h"

#include "new_types.h"
#include "DupTree.h"
#include "NameTuplesUtils.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "LookUpTable.h"
#include "scheduling.h"
#include "constants.h"
#include "stringset.h"
#include "namespaces.h"
#include "vector.h"
#include "ctinfo.h"
#include "int_matrix.h"

/*
 * INFO structure
 */
struct INFO {
    int type;
    node *cont;
    node *fundef;
    lut_t *lut;
    bool inspecial;
    node *assign;
    node *fundefssa;
    node *spawns;
};

/*
 * INFO macros
 */
#define INFO_TYPE(n) (n->type)
#define INFO_CONT(n) (n->cont)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LUT(n) (n->lut)
#define INFO_INSPECIAL(n) (n->inspecial)
#define INFO_ASSIGN(n) (n->assign)
#define INFO_FUNDEFSSA(n) (n->fundefssa)
#define INFO_SPAWNS(n) (n->spawns)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_TYPE (result) = 0;
    INFO_CONT (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_INSPECIAL (result) = FALSE;
    INFO_ASSIGN (result) = NULL;
    INFO_FUNDEFSSA (result) = NULL;
    INFO_SPAWNS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * DFMs are *not* duplicated for several reasons:
 *  - Most likely the old DFMs are not suitable for the new context.
 *  - Only valid DFMs can be duplicated. Therefore, duplicating the
 *    DFMs requires them to be valid during all compilation phases.
 *    For the time being this cannot be guaranteed.
 */
#define DUP_DFMS 0

static lut_t *dup_lut = NULL;
static node *store_copied_special_fundefs = NULL;

/*
 * always traverses son 'node'
 *
 * The macro is to be used within traversal functions where arg_node and
 * arg_info exist.
 */
#define DUPTRAV(node) ((node) != NULL) ? TRAVdo (node, arg_info) : NULL

/*
 * If INFO_CONT contains the root of syntaxtree
 *   -> traverses son 'node' if and only if its parent is not the root
 * If INFO_CONT is NULL
 *   -> traverses son 'node'
 *
 * The macro is to be used within traversal functions where arg_node and
 * arg_info exist.
 */
#define DUPCONT(node) (INFO_CONT (arg_info) != arg_node) ? DUPTRAV (node) : NULL

/******************************************************************************
 *
 * function:
 *   static node *DupTreeOrNodeLutType( bool NodeOnly,
 *                                      node *arg_node, lut_t *lut, int type,
 *                                      node *fundef)
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
 *       value for INFO_TYPE.
 *   - fundef:
 *       optional: only necessary if type value == DUP_SSA. It is
 *                 required for new vardecs.
 *                 Else it points to NULL.
 *
 ******************************************************************************/

static node *
DupTreeOrNodeLutType (bool node_only, node *arg_node, lut_t *lut, int type, node *fundef)
{
    info *arg_info;
    node *new_node;

    DBUG_ENTER ();

    if (arg_node != NULL) {
        arg_info = MakeInfo ();

        INFO_TYPE (arg_info) = type;
        INFO_ASSIGN (arg_info) = NULL;
        INFO_FUNDEF (arg_info) = NULL;
        INFO_FUNDEFSSA (arg_info) = fundef;

        /*
         * Via this (ugly) macro DUPCONT the decision to copy the whole tree
         * starting from arg_node or only the node itself (meaning not to
         * traverse and copy xxx_NEXT) is done.
         * DUPCONT compares the actual arg_node of a traversal function with the
         * value in INFO_CONT. If they are the same the xxx_NEXT will be
         * ignored, otherwise it will be traversed. If the start-node is stored as
         * INFO_CONT it's xx_NEXT will not be duplicated, but the xxx_NEXT's
         * of all sons are copied, because they differ from INFO_CONT.
         * If NULL is stored in INFO_CONT (and in a traversal the arg_node
         * never is NULL) all nodes and their xxx_NEXT's are duplicated.
         * So we set INFO_CONT with NULL to copy all, arg_node to copy
         * start_node (as decribed above) only.
         */
        if (node_only) {
            INFO_CONT (arg_info) = arg_node;
        } else {
            INFO_CONT (arg_info) = NULL;
        }

        if (lut == NULL) {
            if (dup_lut == NULL) {
                dup_lut = LUTgenerateLut ();
            }
            DBUG_ASSERT (LUTisEmptyLut (dup_lut), "LUT for DupTree is not empty!");
            INFO_LUT (arg_info) = dup_lut;
        } else {
            INFO_LUT (arg_info) = lut;
        }

        TRAVpush (TR_dup);
        new_node = TRAVdo (arg_node, arg_info);
        TRAVpop ();

        if (lut == NULL) {
            /*
             * Here, we just remove the content of the LUT but *not* the LUT itself.
             * Guess what: Most likely we will need the LUT again soon ;-)
             */
            dup_lut = LUTremoveContentLut (dup_lut);
        }

        arg_info = FreeInfo (arg_info);
    } else {
        new_node = NULL;
    }

    DBUG_RETURN (new_node);
}

cuda_index_t *
DUPCudaIndex (cuda_index_t *index)
{
    cuda_index_t *tmp, *new_index = NULL;

    DBUG_ENTER ();

    while (index != NULL) {
        tmp = (cuda_index_t *)MEMmalloc (sizeof (cuda_index_t));
        CUIDX_TYPE (tmp) = CUIDX_TYPE (index);
        CUIDX_COEFFICIENT (tmp) = CUIDX_COEFFICIENT (index);
        CUIDX_ID (tmp) = CUIDX_ID (index);
        CUIDX_LOOPLEVEL (tmp) = CUIDX_LOOPLEVEL (index);
        CUIDX_NEXT (tmp) = new_index;
        new_index = tmp;
        index = CUIDX_NEXT (index);
    }

    DBUG_RETURN (new_index);
}

static cuda_access_info_t *
DUPCudaAccessInfo (cuda_access_info_t *access_info, node *new_array, info *arg_info)
{
    int i;
    cuda_access_info_t *new_access_info;

    DBUG_ENTER ();

    new_access_info = (cuda_access_info_t *)MEMmalloc (sizeof (cuda_access_info_t));

    CUAI_MATRIX (new_access_info) = DupMatrix (CUAI_MATRIX (access_info));
    CUAI_ARRAY (new_access_info) = new_array;
    CUAI_ARRAYSHP (new_access_info) = DUPTRAV (CUAI_ARRAYSHP (access_info));
    CUAI_SHARRAY (new_access_info) = CUAI_SHARRAY (access_info);
    ;
    CUAI_SHARRAYSHP_PHY (new_access_info) = DUPTRAV (CUAI_SHARRAYSHP_PHY (access_info));
    CUAI_SHARRAYSHP_LOG (new_access_info) = DUPTRAV (CUAI_SHARRAYSHP_LOG (access_info));
    CUAI_DIM (new_access_info) = CUAI_DIM (access_info);
    ;
    CUAI_NESTLEVEL (new_access_info) = CUAI_NESTLEVEL (access_info);
    ;

    for (i = 0; i < MAX_REUSE_DIM; i++) {
        CUAI_INDICES (new_access_info, i) = DUPCudaIndex (CUAI_INDICES (access_info, i));
        CUAI_ISCONSTANT (new_access_info, i) = CUAI_ISCONSTANT (access_info, i);
    }

    DBUG_RETURN (new_access_info);
}

/******************************************************************************
 *
 * function:
 *   node *DupTreeTravPre( node *arg_node, info *arg_info)
 *
 * description:
 *   This function is called before the traversal of each node.
 *
 ******************************************************************************/

node *
DUPtreeTravPre (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Duplicating - %s", NODE_TEXT (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DUPdupTreeTravPost( node *arg_node, info *arg_info)
 *
 * description:
 *   This function is called after the traversal of each node.
 *
 ******************************************************************************/

node *
DUPtreeTravPost (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DupFlags( node *new_node, node *old_node)
 *
 * Description: For reasons I do not comprehend, DUP does not
 *              copy AST flags. This function looks like a kludge
 *              to achieve selective copying of flags when that
 *              is needed.
 *
 ******************************************************************************/

static node *
DupFlags (node *new_node, node *old_node)
{
    DBUG_ENTER ();

    /* TODO : Copy of flagvector (has to be done by sah) */

    /* A quick fix to enable cuda unrolling - jgo */
    if (NODE_TYPE (new_node) == N_range) {
        RANGE_NEEDCUDAUNROLL (new_node) = RANGE_NEEDCUDAUNROLL (old_node);
    }

    if (NODE_TYPE (new_node) == N_fundef) {
        FUNDEF_LOOPCOUNT (new_node) = FUNDEF_LOOPCOUNT (old_node);
    }

    DBUG_RETURN (new_node);
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
    DBUG_ENTER ();

    NODE_LINE (new_node) = NODE_LINE (old_node);
    NODE_FILE (new_node) = NODE_FILE (old_node);

    if (NODE_ERROR (new_node) != NULL) {
        NODE_ERROR (new_node) = DUPerror (NODE_ERROR (old_node), NULL);
    }

    new_node = DupFlags (new_node, old_node);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   dfmask_t* DupDfmask( dfmask_t* mask, info *arg_info)
 *
 * Description:
 *   Duplicates the given dfmask.
 *   The real duplication is done by DFMDuplicateMask:
 *   If a new DFMbase is found in the LUT the new one is used,
 *   otherwise the old one (this is done by the LUTmechanismi, called
 *   within this function).
 *
 ******************************************************************************/

static dfmask_t *
DupDfmask (dfmask_t *mask, info *arg_info)
{
    dfmask_t *new_mask;

    DBUG_ENTER ();

#if DUP_DFMS
    if (mask != NULL) {
        new_mask = DFMduplicateMask (mask, LUTsearchInLutPp (INFO_LUT (arg_info),
                                                             DFMgetMaskBase (mask)));
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
 *   nodelist *DupNodelist( nodelist *nl, info *arg_info)
 *
 * Remark:
 *   'arg_info' might be NULL, because this function is not only used by
 *   the traversal mechanism but also by DupNodelist()!
 *
 ******************************************************************************/

static nodelist *
DupNodelist (nodelist *nl, info *arg_info)
{
    nodelist *new_nl;

    DBUG_ENTER ();

    if (nl != NULL) {
        new_nl = TBmakeNodelistNode ((node *)LUTsearchInLutPp (INFO_LUT (arg_info),
                                                               NODELIST_NODE (nl)),
                                     DupNodelist (NODELIST_NEXT (nl), arg_info));
        NODELIST_STATUS (new_nl) = NODELIST_STATUS (nl);
    } else {
        new_nl = NULL;
    }

    DBUG_RETURN (new_nl);
}

/******************************************************************************
 *
 * Function:
 *   argtab_t *DupArgtab( argtab_t *argtab, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static argtab_t *
DupArgtab (argtab_t *argtab, info *arg_info)
{
    argtab_t *new_argtab;
    size_t i;

    DBUG_ENTER ();

    if (argtab != NULL) {
        new_argtab = TBmakeArgtab (argtab->size);

        for (i = 0; i < argtab->size; i++) {
            new_argtab->tag[i] = argtab->tag[i];
            new_argtab->ptr_in[i]
              = (argtab->ptr_in[i] != NULL)
                  ? (node *)LUTsearchInLutPp (INFO_LUT (arg_info), argtab->ptr_in[i])
                  : NULL;
            new_argtab->ptr_out[i]
              = (argtab->ptr_out[i] != NULL)
                  ? (node *)LUTsearchInLutPp (INFO_LUT (arg_info), argtab->ptr_out[i])
                  : NULL;
        }
    } else {
        new_argtab = NULL;
    }

    DBUG_RETURN (new_argtab);
}

/******************************************************************************/

node *
DUPnum (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeNum (NUM_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    NUM_FLAGSTRUCTURE (new_node) = NUM_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPnumbyte (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeNumbyte (NUMBYTE_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    NUMBYTE_FLAGSTRUCTURE (new_node) = NUMBYTE_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPnumshort (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeNumshort (NUMSHORT_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    NUMSHORT_FLAGSTRUCTURE (new_node) = NUMSHORT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPnumint (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeNumint (NUMINT_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    NUMINT_FLAGSTRUCTURE (new_node) = NUMINT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPnumlong (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeNumlong (NUMLONG_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    NUMLONG_FLAGSTRUCTURE (new_node) = NUMLONG_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPnumlonglong (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeNumlonglong (NUMLONGLONG_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    NUMLONGLONG_FLAGSTRUCTURE (new_node) = NUMLONGLONG_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPnumubyte (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeNumubyte (NUMUBYTE_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    NUMUBYTE_FLAGSTRUCTURE (new_node) = NUMUBYTE_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPnumushort (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeNumushort (NUMUSHORT_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    NUMUSHORT_FLAGSTRUCTURE (new_node) = NUMUSHORT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPnumuint (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeNumuint (NUMUINT_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    NUMUINT_FLAGSTRUCTURE (new_node) = NUMUINT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPnumulong (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeNumulong (NUMULONG_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    NUMULONG_FLAGSTRUCTURE (new_node) = NUMULONG_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPnumulonglong (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeNumulonglong (NUMULONGLONG_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    NUMULONGLONG_FLAGSTRUCTURE (new_node) = NUMULONGLONG_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPbool (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeBool (BOOL_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    BOOL_FLAGSTRUCTURE (new_node) = BOOL_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPnested_init (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeNested_init ();

    CopyCommonNodeData (new_node, arg_node);

    NESTED_INIT_FLAGSTRUCTURE (new_node) = NESTED_INIT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPfloat (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeFloat (FLOAT_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    FLOAT_FLAGSTRUCTURE (new_node) = FLOAT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPfloatvec (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeFloatvec (FLOATVEC_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    FLOATVEC_FLAGSTRUCTURE (new_node) = FLOATVEC_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPdouble (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeDouble (DOUBLE_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DOUBLE_FLAGSTRUCTURE (new_node) = DOUBLE_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPchar (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeChar (CHAR_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    CHAR_FLAGSTRUCTURE (new_node) = CHAR_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPstr (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeStr (STRcpy (STR_STRING (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    STR_FLAGSTRUCTURE (new_node) = STR_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPtype (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeType (TYcopyType (TYPE_TYPE (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    TYPE_FLAGSTRUCTURE (new_node) = TYPE_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPdot (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeDot (DOT_NUM (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DOT_FLAGSTRUCTURE (new_node) = DOT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPsetwl (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeSetwl (TRAVdo (SETWL_VEC (arg_node), arg_info),
                            TRAVdo (SETWL_EXPR (arg_node), arg_info));

    CopyCommonNodeData (new_node, arg_node);

    SETWL_FLAGSTRUCTURE (new_node) = SETWL_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPconstraint (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeConstraint (DUPTRAV (CONSTRAINT_PREDAVIS (arg_node)),
                                 DUPTRAV (CONSTRAINT_EXPR (arg_node)),
                                 DUPCONT (CONSTRAINT_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    CONSTRAINT_FLAGSTRUCTURE (new_node) = CONSTRAINT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPid (node *arg_node, info *arg_info)
{
    node *new_node, *avis;

    DBUG_ENTER ();

    avis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));
    new_node = TBmakeId (avis);

    if (INFO_TYPE (arg_info) == DUP_WLF) {
        /* Withloop folding (wlf) needs this. */
        if (ID_WL (arg_node) && (NODE_TYPE (ID_WL (arg_node)) == N_id)) {
            /* new code in new_codes, see 'usage of ID_WL' in WLF.c for more infos */
            ID_WL (new_node) = ID_WL (arg_node);
        } else {
            ID_WL (new_node) = arg_node; /* original code */
        }
    }

    if (ID_NT_TAG (arg_node) != NULL) {
        ID_NT_TAG (new_node) = STRcpy (ID_NT_TAG (arg_node));
    }

    /*
     * furthermore, we have to copy the ICMTEXT attribute
     * if it still exists
     */
    ID_ICMTEXT (new_node) = STRcpy (ID_ICMTEXT (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    ID_ISSCLPRF (new_node) = ID_ISSCLPRF (arg_node);

    ID_FLAGSTRUCTURE (new_node) = ID_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPspid (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeSpid (NSdupNamespace (SPID_NS (arg_node)), STRcpy (SPID_NAME (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    SPID_FLAGSTRUCTURE (new_node) = SPID_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPcast (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeCast (TYcopyType (CAST_NTYPE (arg_node)), DUPTRAV (CAST_EXPR (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    CAST_FLAGSTRUCTURE (new_node) = CAST_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPmodule (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeModule (NSdupNamespace (MODULE_NAMESPACE (arg_node)),
                      MODULE_FILETYPE (arg_node), DUPTRAV (MODULE_INTERFACE (arg_node)),
                      DUPTRAV (MODULE_TYPES (arg_node)), DUPTRAV (MODULE_OBJS (arg_node)),
                      DUPTRAV (MODULE_FUNS (arg_node)),
                      DUPTRAV (MODULE_FUNDECS (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    MODULE_FLAGSTRUCTURE (new_node) = MODULE_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPstructdef (node *arg_node, info *arg_info)
{
    node *new_vardecs;
    node *new_node;

    DBUG_ENTER ();

    new_vardecs = DUPTRAV (STRUCTDEF_STRUCTELEM (arg_node));

    new_node = TBmakeStructdef (STRcpy (STRUCTDEF_NAME (arg_node)), new_vardecs,
                                DUPCONT (STRUCTDEF_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    STRUCTDEF_FLAGSTRUCTURE (new_node) = STRUCTDEF_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPtypedef (node *arg_node, info *arg_info)
{
    node *new_node;
    node *new_args;

    DBUG_ENTER ();

    new_args = DUPTRAV (TYPEDEF_ARGS (arg_node));

    new_node = TBmakeTypedef (STRcpy (TYPEDEF_NAME (arg_node)),
                              NSdupNamespace (TYPEDEF_NS (arg_node)),
                              STRcpy (TYPEDEF_NAME (arg_node)),
                              TYcopyType (TYPEDEF_NTYPE (arg_node)), new_args,
                              DUPCONT (TYPEDEF_NEXT (arg_node)));

    TYPEDEF_FLAGSTRUCTURE (new_node) = TYPEDEF_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, new_node);

    TYPEDEF_FLAGSTRUCTURE (new_node) = TYPEDEF_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPobjdef (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeObjdef (TYcopyType (OBJDEF_TYPE (arg_node)),
                      NSdupNamespace (OBJDEF_NS (arg_node)),
                      STRcpy (OBJDEF_NAME (arg_node)), DUPTRAV (OBJDEF_EXPR (arg_node)),
                      DUPCONT (OBJDEF_NEXT (arg_node)));

    OBJDEF_FLAGSTRUCTURE (new_node) = OBJDEF_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, new_node);

    OBJDEF_FLAGSTRUCTURE (new_node) = OBJDEF_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPfundef (node *arg_node, info *arg_info)
{
    node *new_node, *old_fundef, *new_ssacnt;

    DBUG_ENTER ();

    /*
     * DUP_INLINE
     *  -> N_return nodes are not duplicated
     *  -> would result in an illegal N_fundef node
     *  -> stop here!
     */
    DBUG_ASSERT (INFO_TYPE (arg_info) != DUP_INLINE,
                 "N_fundef node can't be duplicated in DUP_INLINE mode!");

    DBUG_PRINT ("start dubbing of fundef %s", FUNDEF_NAME (arg_node));

    /*
     * We can't copy the FUNDEF_DFM_BASE and DFMmasks belonging to this base
     * directly!
     * Such DFMmasks are attached to N_with, N_with2, N_sync and N_spmd.
     * All of them can be found in the body of the function.
     * But when we copy the body we must already know the base to create the
     * new masks. On the other hand to create the base we must already have
     * the new FUNDEF_ARGS and FUNDEF_VARDECSS available.
     * Therefore we first create the raw function without the body via
     * MakeFundef(), then we create the base while duplicating the body
     * and finally we attach the body to the fundef.
     */

    /*
     * INFO_FUNDEF is a pointer to the OLD fundef node!!
     * We can get the pointer to the NEW fundef via the LUT!
     */
    old_fundef = INFO_FUNDEF (arg_info);
    INFO_FUNDEF (arg_info) = arg_node;

    new_node = TBmakeFundef (STRcpy (FUNDEF_NAME (arg_node)),
                             NSdupNamespace (FUNDEF_NS (arg_node)),
                             DUPTRAV (FUNDEF_RETS (arg_node)),
                             NULL, /* must be duplicated later on */
                             NULL, /* must be duplicated later on */
                             NULL);

    /* now we copy all the other things ... */
    FUNDEF_FUNNO (new_node) = FUNDEF_FUNNO (arg_node);
    FUNDEF_PRAGMA (new_node) = DUPTRAV (FUNDEF_PRAGMA (arg_node));
    FUNDEF_FLAGSTRUCTURE (new_node) = FUNDEF_FLAGSTRUCTURE (arg_node);
    FUNDEF_LIVEVARS (new_node) = DUPTRAV (FUNDEF_LIVEVARS (arg_node));
    FUNDEF_FPFRAMENAME (new_node) = STRcpy (FUNDEF_FPFRAMENAME (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    FUNDEF_FLAGSTRUCTURE (new_node) = FUNDEF_FLAGSTRUCTURE (arg_node);

    FUNDEF_NEXT (new_node) = DUPCONT (FUNDEF_NEXT (arg_node));

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

    /*
     * we have to insert the new fundef into the LUT prior to
     * traversing the body as otherwise recursive calls could
     * not be properly linked to the new fundef
     */
    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, new_node);

    FUNDEF_ARGS (new_node) = DUPTRAV (FUNDEF_ARGS (arg_node));
    FUNDEF_BODY (new_node) = DUPTRAV (FUNDEF_BODY (arg_node));

    /*
     * ARGTAB must be duplicated *after* TYPES and ARGS!!!
     */
    FUNDEF_ARGTAB (new_node) = DupArgtab (FUNDEF_ARGTAB (arg_node), arg_info);

    if (FUNDEF_BODY (new_node) != NULL) {
        BLOCK_SSACOUNTER (FUNDEF_BODY (new_node)) = new_ssacnt;
    }

    /*
     * must be done after traversal of BODY
     */

#if DUP_DFMS
    FUNDEF_DFM_BASE (new_node)
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), FUNDEF_DFM_BASE (arg_node));
#else
    FUNDEF_DFM_BASE (new_node) = NULL;
#endif

    FUNDEF_RETURN (new_node)
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), FUNDEF_RETURN (arg_node));

    if (FUNDEF_ISLOOPFUN (new_node)) {
        DBUG_ASSERT (FUNDEF_ISLOOPFUN (arg_node),
                     "Mismatch in copying flag structure of N_fundef node");

        DBUG_ASSERT (FUNDEF_LOOPRECURSIVEAP (arg_node) != NULL,
                     "Do-fun without link to recursive application found: %s.",
                     FUNDEF_NAME (arg_node));

        FUNDEF_LOOPRECURSIVEAP (new_node)
          = (node *)LUTsearchInLutPp (INFO_LUT (arg_info),
                                      FUNDEF_LOOPRECURSIVEAP (arg_node));
        DBUG_ASSERT (FUNDEF_LOOPRECURSIVEAP (new_node) != NULL,
                     "Recursive application not found in LUT: %s.",
                     FUNDEF_NAME (new_node));
    }

    FUNDEF_IMPL (new_node)
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), FUNDEF_IMPL (arg_node));

    if (FUNDEF_WRAPPERTYPE (arg_node) != NULL) {
        FUNDEF_WRAPPERTYPE (new_node) = TYcopyType (FUNDEF_WRAPPERTYPE (arg_node));
    }

    /*
     * copy the object information
     */
    FUNDEF_OBJECTS (new_node) = DUPTRAV (FUNDEF_OBJECTS (arg_node));
    FUNDEF_AFFECTEDOBJECTS (new_node) = DUPTRAV (FUNDEF_AFFECTEDOBJECTS (arg_node));

    INFO_FUNDEF (arg_info) = old_fundef;

    DBUG_RETURN (new_node);
}

/******************************************************************************/
// Fangyong add: user-defined constraints, not finish yet.
node *
DUPudcs (node *arg_node, info *arg_info)
{
    // node *new_node;

    DBUG_ENTER ();

    //  new_node = TBmakeUdcs(DUPTRAV(UDCS_UDC( arg_node)),NULL);

    DBUG_RETURN (arg_node);
}

node *
DUParg (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    /* SAA requires that N_arg nodes be traversed left-to-right,
     * because the incoming AVIS_DIM/SHAPE information for an
     * argument may be an N_id, which is defined as an earlier
     * (left-more) argument.
     */
    new_node = TBmakeArg (DUPTRAV (ARG_AVIS (arg_node)), NULL);

    ARG_OBJDEF (new_node) = ARG_OBJDEF (arg_node);
    ARG_LINKSIGN (new_node) = ARG_LINKSIGN (arg_node);
    ARG_FLAGSTRUCTURE (new_node) = ARG_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, new_node);

    /* correct backreference */
    AVIS_DECL (ARG_AVIS (new_node)) = new_node;

    ARG_NEXT (new_node) = DUPCONT (ARG_NEXT (arg_node));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPret (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeRet (TYcopyType (RET_TYPE (arg_node)), DUPCONT (RET_NEXT (arg_node)));

    RET_LINKSIGN (new_node) = RET_LINKSIGN (arg_node);

    RET_FLAGSTRUCTURE (new_node) = RET_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    RET_FLAGSTRUCTURE (new_node) = RET_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPexprs (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeExprs (DUPTRAV (EXPRS_EXPR (arg_node)), DUPCONT (EXPRS_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, new_node);

    EXPRS_FLAGSTRUCTURE (new_node) = EXPRS_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPblock (node *arg_node, info *arg_info)
{
    node *new_vardecs;
    node *new_node;
    node *v;
    node *avis;
    node *nid;

#if DUP_DFMS
    DFMmask_base_t old_base, new_base;
#endif

    DBUG_ENTER ();

    new_vardecs = DUPTRAV (BLOCK_VARDECS (arg_node));

#if DUP_DFMS
    if (INFO_FUNDEF (arg_info) != NULL) {
        old_base = FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info));
    } else {
        old_base = NULL;
    }

    /*
     * If the current block is the top most block of the current fundef
     * we have to copy FUNDEF_DFMBASE.
     * Look at DupFundef() for further comments.
     */
    if ((old_base != NULL) && (arg_node == FUNDEF_BODY (INFO_FUNDEF (arg_info)))) {
        new_base
          = DFMgenMaskBase (FUNDEF_ARGS (LUTsearchInLutPp (INFO_LUT (arg_info),
                                                           INFO_FUNDEF (arg_info))),
                            new_vardecs);

        INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), old_base, new_base);
    }
#endif

    new_node = TBmakeBlock (DUPTRAV (BLOCK_ASSIGNS (arg_node)), new_vardecs);
    BLOCK_CACHESIM (new_node) = STRcpy (BLOCK_CACHESIM (arg_node));

    /*
     * BLOCK_SSACOUNTER is adjusted correctly later on by DupFundef()
     */
    BLOCK_SSACOUNTER (new_node) = BLOCK_SSACOUNTER (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    BLOCK_FLAGSTRUCTURE (new_node) = BLOCK_FLAGSTRUCTURE (arg_node);

    BLOCK_ISMTSEQUENTIALBRANCH (new_node) = BLOCK_ISMTSEQUENTIALBRANCH (arg_node);
    BLOCK_ISMTPARALLELBRANCH (new_node) = BLOCK_ISMTPARALLELBRANCH (arg_node);

    /* This block of code ensures that the N_vardec son nodes are
     * updated properly.
     */

    /* Have to defer updating extrema until all vardecs in place */
    v = BLOCK_VARDECS (new_node);

    while (NULL != v) {
        avis = VARDEC_AVIS (v);
        DBUG_PRINT ("DUPblock vardec scan looking at %s", AVIS_NAME (avis));

        /* I can't figure this SAA stuff working at all unless
         * these fields are either N_num or N_id!
         *
         * For now, this code assumes/requires that an AVIS_DIM
         * be either an N_num or N_id, and that an AVIS_SHAPE be
         * either an N_array or an N_id. In the N_id cases, we are
         * guaranteed that all the avis entries have already been
         * processed.
         *
         * For N_id nodes, we pick up the new name from the LUR.
         * N_num and N_array nodes are left intact.
         *
         */
        if (NULL != AVIS_DIM (avis)) {
            if (N_id == NODE_TYPE (AVIS_DIM (avis))) {
                nid = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), AVIS_DIM (avis));
                if (nid != AVIS_DIM (avis)) { /* nilpotent rename */
                    DBUG_ASSERT (N_id == NODE_TYPE (nid),
                                 "Found non-id AVIS_DIM rename target");
                    DBUG_PRINT ("renaming AVIS_DIM from %s to %s",
                                AVIS_NAME (ID_AVIS (AVIS_DIM (avis))),
                                AVIS_NAME (ID_AVIS (nid)));
                    AVIS_DIM (avis) = FREEdoFreeNode (AVIS_DIM (avis));
                    AVIS_DIM (avis) = TBmakeId (ID_AVIS (nid));
                }
            } else if (N_num == NODE_TYPE (AVIS_DIM (avis))) {
                AVIS_DIM (avis) = DUPCONT (AVIS_DIM (avis));
            } else {
                DBUG_UNREACHABLE ("found oddball AVIS_DIM node type");
            }
        }

        if (NULL != AVIS_SHAPE (avis)) {
            if (N_id == NODE_TYPE (AVIS_SHAPE (avis))) {
                nid = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), AVIS_SHAPE (avis));
                if (nid != AVIS_SHAPE (avis)) { /* nilpotent rename */
                    DBUG_ASSERT (N_id == NODE_TYPE (nid),
                                 "Found non-id AVIS_SHAPE rename target");
                    DBUG_PRINT ("renaming AVIS_SHAPE from %s to %s",
                                AVIS_NAME (ID_AVIS (AVIS_SHAPE (avis))),
                                AVIS_NAME (ID_AVIS (nid)));
                    AVIS_SHAPE (avis) = FREEdoFreeNode (AVIS_SHAPE (avis));
                    AVIS_SHAPE (avis) = TBmakeId (ID_AVIS (nid));
                }
            } else if (N_array == NODE_TYPE (AVIS_SHAPE (avis))) {
                AVIS_SHAPE (avis) = DUPCONT (AVIS_SHAPE (avis));
            } else {
                DBUG_UNREACHABLE ("found oddball AVIS_SHAPE node type");
            }
        }

        // AVIS_SCALARS is NULL or an N_array, but see comment below re AVIS_MIN
        if (NULL != AVIS_SCALARS (avis)) {
            if (N_id == NODE_TYPE (AVIS_SCALARS (avis))) {
                nid = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), AVIS_SCALARS (avis));
                if (nid != AVIS_SCALARS (avis)) { /* nilpotent rename */
                    DBUG_ASSERT (N_id == NODE_TYPE (nid),
                                 "Found non-id AVIS_SCALARS rename target");
                    DBUG_PRINT ("renaming AVIS_SCALARS from %s to %s",
                                AVIS_NAME (ID_AVIS (AVIS_SCALARS (avis))),
                                AVIS_NAME (ID_AVIS (nid)));
                    AVIS_SCALARS (avis) = FREEdoFreeNode (AVIS_SCALARS (avis));
                    AVIS_SCALARS (avis) = TBmakeId (ID_AVIS (nid));
                }
            } else if (N_array == NODE_TYPE (AVIS_SCALARS (avis))) {
                AVIS_SCALARS (avis) = DUPCONT (AVIS_SCALARS (avis));
            } else {
                DBUG_UNREACHABLE ("found oddball AVIS_SCALARS node type");
            }
        }
        /* AVIS_MIN and AVIS_MAX are restricted to N_id nodes, but
         * we use clones of the AVIS_DIM/SHAPE code, just to keep them
         * looking the same.
         */
        if (NULL != AVIS_MIN (avis)) {
            if (N_id == NODE_TYPE (AVIS_MIN (avis))) {
                nid = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), AVIS_MIN (avis));
                if (nid != AVIS_MIN (avis)) { /* nilpotent rename */
                    DBUG_ASSERT (N_id == NODE_TYPE (nid),
                                 "Found non-id AVIS_MIN rename target");
                    DBUG_PRINT ("renaming AVIS_MIN from %s to %s",
                                AVIS_NAME (ID_AVIS (AVIS_MIN (avis))),
                                AVIS_NAME (ID_AVIS (nid)));
                    AVIS_MIN (avis) = FREEdoFreeNode (AVIS_MIN (avis));
                    AVIS_MIN (avis) = TBmakeId (ID_AVIS (nid));
                }
            } else if (N_array == NODE_TYPE (AVIS_MIN (avis))) {
                AVIS_MIN (avis) = DUPCONT (AVIS_MIN (avis));
            } else {
                DBUG_UNREACHABLE ("found oddball AVIS_MIN node type");
            }
        }

        if (NULL != AVIS_MAX (avis)) {
            if (N_id == NODE_TYPE (AVIS_MAX (avis))) {
                nid = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), AVIS_MAX (avis));
                if (nid != AVIS_MAX (avis)) { /* nilpotent rename */
                    DBUG_ASSERT (N_id == NODE_TYPE (nid),
                                 "Found non-id AVIS_MAX rename target");
                    DBUG_PRINT ("renaming AVIS_MAX from %s to %s",
                                AVIS_NAME (ID_AVIS (AVIS_MAX (avis))),
                                AVIS_NAME (ID_AVIS (nid)));
                    AVIS_MAX (avis) = FREEdoFreeNode (AVIS_MAX (avis));
                    AVIS_MAX (avis) = TBmakeId (ID_AVIS (nid));
                }
            } else if (N_array == NODE_TYPE (AVIS_MAX (avis))) {
                AVIS_MAX (avis) = DUPCONT (AVIS_MAX (avis));
            } else {
                DBUG_UNREACHABLE ("found oddball AVIS_MAX node type");
            }
        }

        v = VARDEC_NEXT (v);
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPstructelem (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeStructelem (STRcpy (STRUCTELEM_NAME (arg_node)),
                                 TYcopyType (STRUCTELEM_TYPE (arg_node)),
                                 DUPCONT (STRUCTELEM_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    STRUCTELEM_FLAGSTRUCTURE (new_node) = STRUCTELEM_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPvardec (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeVardec (DUPTRAV (VARDEC_AVIS (arg_node)), DUPCONT (VARDEC_NEXT (arg_node)));

    VARDEC_FLAGSTRUCTURE (new_node) = VARDEC_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, new_node);
    /* correct backreference */
    AVIS_DECL (VARDEC_AVIS (new_node)) = new_node;

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPreturn (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeReturn (DUPTRAV (RETURN_EXPRS (arg_node)));

    RETURN_CRET (new_node)
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), RETURN_CRET (arg_node));

    RETURN_FLAGSTRUCTURE (new_node) = RETURN_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPassign (node *arg_node, info *arg_info)
{
    node *new_node;
    node *stacked_assign;

    DBUG_ENTER ();

    if ((INFO_TYPE (arg_info) != DUP_INLINE)
        || (NODE_TYPE (ASSIGN_STMT (arg_node)) != N_return)) {

        new_node = TBmakeAssign (NULL, NULL);

        stacked_assign = INFO_ASSIGN (arg_info);
        INFO_ASSIGN (arg_info) = new_node;

        ASSIGN_STMT (new_node) = DUPTRAV (ASSIGN_STMT (arg_node));

        INFO_ASSIGN (arg_info) = stacked_assign;

        /*
         * ----->
         * Is the order of execution really valid???
         */
        ASSIGN_NEXT (new_node) = DUPCONT (ASSIGN_NEXT (arg_node));

        INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, new_node);
        /*
         * <----
         */

        ASSIGN_FLAGSTRUCTURE (new_node) = ASSIGN_FLAGSTRUCTURE (arg_node);

        if (ASSIGN_ACCESS_INFO (arg_node) != NULL) {
            DBUG_ASSERT (NODE_TYPE (ASSIGN_RHS (arg_node)) == N_prf, "Wrong node type!");
            DBUG_ASSERT (PRF_PRF (ASSIGN_RHS (arg_node)) == F_idx_sel,
                         "Wrong primitive type!");

            ASSIGN_ACCESS_INFO (new_node)
              = DUPCudaAccessInfo (ASSIGN_ACCESS_INFO (arg_node),
                                   ID_AVIS (PRF_ARG2 (ASSIGN_RHS (new_node))), arg_info);
        }

        CopyCommonNodeData (new_node, arg_node);
        ASSIGN_FLAGSTRUCTURE (new_node) = ASSIGN_FLAGSTRUCTURE (arg_node);
    } else {
        new_node = NULL;
    }

    DBUG_PRINT ("Duplicating N_assign node %p to %p", (void *)arg_node, (void *)new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPcond (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeCond (DUPTRAV (COND_COND (arg_node)), DUPTRAV (COND_THEN (arg_node)),
                           DUPTRAV (COND_ELSE (arg_node)));

    COND_ISTHENNOOP (new_node) = COND_ISTHENNOOP (arg_node);
    COND_ISELSENOOP (new_node) = COND_ISELSENOOP (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    COND_FLAGSTRUCTURE (new_node) = COND_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPdo (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeDo (DUPTRAV (DO_COND (arg_node)), DUPTRAV (DO_BODY (arg_node)));

    DO_SKIP (new_node) = DUPTRAV (DO_SKIP (arg_node));
    DO_LABEL (new_node)
      = (DO_LABEL (arg_node) != NULL ? TRAVtmpVarName (DO_LABEL (arg_node)) : NULL);

    DO_ISCUDARIZABLE (new_node) = DO_ISCUDARIZABLE (arg_node);
    DO_ISFORLOOP (new_node) = DO_ISFORLOOP (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DO_FLAGSTRUCTURE (new_node) = DO_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwhile (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeWhile (DUPTRAV (WHILE_COND (arg_node)), DUPTRAV (WHILE_BODY (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    WHILE_FLAGSTRUCTURE (new_node) = WHILE_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPlet (node *arg_node, info *arg_info)
{
    node *new_node;
    node *syncvar;

    DBUG_ENTER ();

    new_node = TBmakeLet (DUPTRAV (LET_IDS (arg_node)), NULL);

    /*
     * EXPR must be traversed after IDS (for AP_ARGTAB)
     */
    LET_EXPR (new_node) = DUPTRAV (LET_EXPR (arg_node));

    CopyCommonNodeData (new_node, arg_node);
    LET_FLAGSTRUCTURE (new_node) = LET_FLAGSTRUCTURE (arg_node);

    LET_SPAWNSYNCINDEX (new_node) = LET_SPAWNSYNCINDEX (arg_node);

    // TODO: are id's in livevars should point to duplicated nodes
    if (LET_LIVEVARS (arg_node) != NULL) {
        LET_LIVEVARS (new_node) = DUPTRAV (LET_LIVEVARS (arg_node));
    }

    // update matching spawns and syncs
    // TODO: don't put on let and don't compare names, but nodes
    if (NODE_TYPE (LET_EXPR (new_node)) == N_ap && AP_ISSPAWNED (LET_EXPR (new_node))) {
        DBUG_PRINT ("Encountered a spawned statement");

        // add spawn to list
        INFO_SPAWNS (arg_info) = TBmakeSet (new_node, INFO_SPAWNS (arg_info));
    } else {
        if (NODE_TYPE (LET_EXPR (new_node)) == N_prf
            && PRF_PRF (LET_EXPR (new_node)) == F_sync) {
            DBUG_PRINT ("Encountered a sync statement");
            syncvar = PRF_ARG1 (LET_EXPR (arg_node));

            // find matching spawn statement
            node *set = INFO_SPAWNS (arg_info);

            DBUG_PRINT ("Sync:  %s", ID_NAME (syncvar));

            do {
                DBUG_PRINT ("Spawn: %s", IDS_NAME (LET_IDS (SET_MEMBER (set))));

                if (STRsuffix (ID_NAME (syncvar),
                               IDS_NAME (LET_IDS (SET_MEMBER (set))))) {
                    // update sync to point to spawn and vice-versa
                    DBUG_PRINT ("Found matching spawn and sync");
                    LET_MATCHINGSPAWNSYNC (SET_MEMBER (set)) = new_node;
                    LET_MATCHINGSPAWNSYNC (new_node) = SET_MEMBER (set);
                }

                set = SET_NEXT (set);
            } while (set != NULL);
        }
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPids (node *arg_node, info *arg_info)
{
    node *new_node, *avis;

    DBUG_ENTER ();

    if ((INFO_TYPE (arg_info) == DUP_SSA)
        && (LUTsearchInLutPp (INFO_LUT (arg_info), IDS_AVIS (arg_node))
            == IDS_AVIS (arg_node))) {
        node *newavis;
        /*
         * To maintain SSA form, new variables must be generated
         */
        newavis = TBmakeAvis (TRAVtmpVarName (IDS_NAME (arg_node)),
                              TYcopyType (IDS_NTYPE (arg_node)));

        if (AVIS_SSAASSIGN (IDS_AVIS (arg_node)) != NULL) {
            AVIS_SSAASSIGN (newavis) = INFO_ASSIGN (arg_info);
        }

        FUNDEF_VARDECS (INFO_FUNDEFSSA (arg_info))
          = TBmakeVardec (newavis, FUNDEF_VARDECS (INFO_FUNDEFSSA (arg_info)));

        INFO_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (arg_node), newavis);
    }

    avis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), IDS_AVIS (arg_node));

    AVIS_ISALIAS (avis) = AVIS_ISALIAS (IDS_AVIS (arg_node));
    AVIS_ISCUDALOCAL (avis) = AVIS_ISCUDALOCAL (IDS_AVIS (arg_node));
    AVIS_HASDTTHENPROXY (avis) = AVIS_HASDTTHENPROXY (IDS_AVIS (arg_node));
    AVIS_HASDTELSEPROXY (avis) = AVIS_HASDTELSEPROXY (IDS_AVIS (arg_node));

    AVIS_DIM (avis) = DUPTRAV (AVIS_DIM (IDS_AVIS (arg_node)));
    AVIS_SHAPE (avis) = DUPTRAV (AVIS_SHAPE (IDS_AVIS (arg_node)));
    AVIS_MIN (avis) = DUPTRAV (AVIS_MIN (IDS_AVIS (arg_node)));
    AVIS_MAX (avis) = DUPTRAV (AVIS_MAX (IDS_AVIS (arg_node)));
    AVIS_SCALARS (avis) = DUPTRAV (AVIS_SCALARS (IDS_AVIS (arg_node)));

    if ((INFO_ASSIGN (arg_info) != NULL) && (AVIS_SSAASSIGN (avis) != NULL)) {
        AVIS_SSAASSIGN (avis) = INFO_ASSIGN (arg_info);
    }

    new_node = TBmakeIds (avis, DUPCONT (IDS_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    IDS_FLAGSTRUCTURE (new_node) = IDS_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPspids (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeSpids (STRcpy (SPIDS_NAME (arg_node)), DUPCONT (SPIDS_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    SPIDS_FLAGSTRUCTURE (new_node) = SPIDS_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPap (node *arg_node, info *arg_info)
{
    node *old_fundef, *new_fundef;
    node *new_node;
    node *fundef;
    const char *funname;

    DBUG_ENTER ();

    fundef = INFO_FUNDEF (arg_info);
    funname = (NULL == fundef) ? "?" : FUNDEF_NAME (fundef);
    DBUG_PRINT ("duplicating N_ap call to %s() from %s",
                (AP_FUNDEF (arg_node) != NULL) ? FUNDEF_NAME (AP_FUNDEF (arg_node)) : "?",
                funname);

    old_fundef = AP_FUNDEF (arg_node);

    if (old_fundef != NULL) {
        new_fundef = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), old_fundef);

        DBUG_ASSERT (((AP_ISRECURSIVEDOFUNCALL (arg_node))
                      || (!FUNDEF_ISLACFUN (old_fundef) || (new_fundef == old_fundef))),
                     "found a condfun ap that points to an already copied function !?!");

        if (FUNDEF_ISCONDFUN (old_fundef)
            || (FUNDEF_ISLACINLINE (old_fundef) && (!AP_ISRECURSIVEDOFUNCALL (arg_node)))
            || (FUNDEF_ISLOOPFUN (old_fundef) && (!AP_ISRECURSIVEDOFUNCALL (arg_node)))) {
            /*
             * Definitions of special functions must be duplicated immediately
             * to retain one-to-one correspondence between application and
             * definition.
             *
             * INFO_CONT must be reset to avoid copying of entire fundef
             * chain.
             */
            node *store_dup_cont;
            int store_dup_type;

            DBUG_PRINT ("LaC function: copying in-place %s() ...",
                        (AP_FUNDEF (arg_node) != NULL)
                          ? CTIitemName (AP_FUNDEF (arg_node))
                          : "?");

            store_dup_cont = INFO_CONT (arg_info);
            store_dup_type = INFO_TYPE (arg_info);

            INFO_CONT (arg_info) = old_fundef;
            INFO_TYPE (arg_info) = DUP_NORMAL;

            new_fundef = TRAVdo (old_fundef, arg_info);

            INFO_TYPE (arg_info) = store_dup_type;
            INFO_CONT (arg_info) = store_dup_cont;

            DBUG_ASSERT (FUNDEF_NEXT (new_fundef) == NULL, "Too many functions copied.");

            FUNDEF_NAME (new_fundef) = MEMfree (FUNDEF_NAME (new_fundef));
            FUNDEF_NAME (new_fundef) = TRAVtmpVarName (FUNDEF_NAME (old_fundef));

            DBUG_PRINT ("LaC function: new copy is %s() ...",
                        (AP_FUNDEF (arg_node) != NULL)
                          ? CTIitemName (AP_FUNDEF (arg_node))
                          : "?");

            /*
             * Unfortunately, there is no proper way to insert the new fundef
             * into the fundef chain. This is postponed until certain safe places
             * in program execution are reached, e.g. N_module nodes. Meanwhile,
             * the new fundefs are stored in an internal fundef chain of
             * duplicated special functions.
             */
            FUNDEF_NEXT (new_fundef) = store_copied_special_fundefs;
            store_copied_special_fundefs = new_fundef;
            DBUG_PRINT_TAG ("DUPSF", "Added to DupTree special function hook:\n %s( %s)",
                            CTIitemName (new_fundef), CTIfunParams (new_fundef));
        } else {
            new_fundef = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), old_fundef);
        }
    } else {
        /*
         * This case is only (!) used during lac2fun conversion.
         */
        new_fundef = NULL;
    }

    new_node = TBmakeAp (new_fundef, DUPTRAV (AP_ARGS (arg_node)));

    AP_ARGTAB (new_node) = DupArgtab (AP_ARGTAB (arg_node), arg_info);

    AP_SPAWNPLACE (new_node) = STRcpy (AP_SPAWNPLACE (arg_node));

    CopyCommonNodeData (new_node, arg_node);
    AP_FLAGSTRUCTURE (new_node) = AP_FLAGSTRUCTURE (arg_node);

    if (AP_ISRECURSIVEDOFUNCALL (arg_node)) {
        INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, new_node);
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPspap (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeSpap (DUPTRAV (SPAP_ID (arg_node)), DUPTRAV (SPAP_ARGS (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    SPAP_FLAGSTRUCTURE (new_node) = SPAP_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPspmop (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    DBUG_PRINT ("duplicating multi operation ...");

    new_node
      = TBmakeSpmop (DUPTRAV (SPMOP_OPS (arg_node)), DUPTRAV (SPMOP_EXPRS (arg_node)));

    SPMOP_FLAGSTRUCTURE (new_node) = SPMOP_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    SPMOP_FLAGSTRUCTURE (new_node) = SPMOP_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUParray (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg_node)),
                            SHcopyShape (ARRAY_FRAMESHAPE (arg_node)),
                            DUPTRAV (ARRAY_AELEMS (arg_node)));

    ARRAY_STRING (new_node) = STRcpy (ARRAY_STRING (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    ARRAY_FLAGSTRUCTURE (new_node) = ARRAY_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPprf (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakePrf (PRF_PRF (arg_node), DUPTRAV (PRF_ARGS (arg_node)));

    PRF_FLAGSTRUCTURE (new_node) = PRF_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    PRF_FLAGSTRUCTURE (new_node) = PRF_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPfuncond (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeFuncond (DUPTRAV (FUNCOND_IF (arg_node)), DUPTRAV (FUNCOND_THEN (arg_node)),
                       DUPTRAV (FUNCOND_ELSE (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    FUNCOND_FLAGSTRUCTURE (new_node) = FUNCOND_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPannotate (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeAnnotate (ANNOTATE_TAG (arg_node), ANNOTATE_FUNNUMBER (arg_node),
                               ANNOTATE_FUNAPNUMBER (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    ANNOTATE_FLAGSTRUCTURE (new_node) = ANNOTATE_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPex (node *arg_node, info *arg_info)
{

    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeEx (DUPTRAV (EX_REGION (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    EX_FLAGSTRUCTURE (new_node) = EX_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPdataflowgraph (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_UNREACHABLE ("DUPdataflowgraph until now not implemented!! :-(");
    DBUG_RETURN ((node *)NULL);
}

/*******************************************************************************/

node *
DUPdataflownode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_UNREACHABLE ("DUPdataflownode until now not implemented!! :-(");

    DBUG_RETURN ((node *)NULL);
}

/*******************************************************************************/

node *
DUPimport (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeImport (STRcpy (IMPORT_MOD (arg_node)), DUPCONT (IMPORT_NEXT (arg_node)),
                      DUPTRAV (IMPORT_SYMBOL (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    IMPORT_FLAGSTRUCTURE (new_node) = IMPORT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPexport (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeExport (DUPCONT (EXPORT_NEXT (arg_node)),
                             DUPTRAV (EXPORT_SYMBOL (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    EXPORT_FLAGSTRUCTURE (new_node) = EXPORT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPuse (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeUse (STRcpy (USE_MOD (arg_node)), DUPCONT (USE_NEXT (arg_node)),
                          DUPTRAV (USE_SYMBOL (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    USE_FLAGSTRUCTURE (new_node) = USE_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPprovide (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeProvide (DUPCONT (PROVIDE_NEXT (arg_node)),
                              DUPTRAV (PROVIDE_SYMBOL (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    PROVIDE_FLAGSTRUCTURE (new_node) = PROVIDE_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPset (node *arg_node, info *arg_info)
{
    node *new_node;
    node *link;

    DBUG_ENTER ();

    link = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), SET_MEMBER (arg_node));

    if (link == NULL) {
        link = SET_MEMBER (arg_node);
    }

    new_node = TBmakeSet (link, DUPCONT (SET_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    SET_FLAGSTRUCTURE (new_node) = SET_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPnums (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeNums (NUMS_VAL (arg_node), DUPCONT (NUMS_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    NUMS_FLAGSTRUCTURE (new_node) = NUMS_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPsymbol (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeSymbol (STRcpy (SYMBOL_ID (arg_node)), DUPCONT (SYMBOL_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    SYMBOL_FLAGSTRUCTURE (new_node) = SYMBOL_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPglobobj (node *arg_node, info *arg_info)
{
    node *new_node;
    node *link;

    DBUG_ENTER ();

    link = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), GLOBOBJ_OBJDEF (arg_node));

    if (link == NULL) {
        link = GLOBOBJ_OBJDEF (arg_node);
    }

    new_node = TBmakeGlobobj (link);

    CopyCommonNodeData (new_node, arg_node);

    GLOBOBJ_FLAGSTRUCTURE (new_node) = GLOBOBJ_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPpragma (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakePragma ();

    PRAGMA_READONLY (new_node) = DUPTRAV (PRAGMA_READONLY (arg_node));
    PRAGMA_REFCOUNTING (new_node) = DUPTRAV (PRAGMA_REFCOUNTING (arg_node));
    PRAGMA_EFFECT (new_node) = DUPTRAV (PRAGMA_EFFECT (arg_node));
    PRAGMA_LINKSIGN (new_node) = DUPTRAV (PRAGMA_LINKSIGN (arg_node));

    PRAGMA_LINKNAME (new_node) = STRcpy (PRAGMA_LINKNAME (arg_node));

    PRAGMA_INITFUN (new_node) = STRcpy (PRAGMA_INITFUN (arg_node));
    PRAGMA_WLCOMP_APS (new_node) = DUPTRAV (PRAGMA_WLCOMP_APS (arg_node));

    PRAGMA_COPYFUN (new_node) = STRcpy (PRAGMA_COPYFUN (arg_node));
    PRAGMA_FREEFUN (new_node) = STRcpy (PRAGMA_FREEFUN (arg_node));
    PRAGMA_LINKMOD (new_node) = STRSduplicate (PRAGMA_LINKMOD (arg_node));
    PRAGMA_LINKOBJ (new_node) = STRSduplicate (PRAGMA_LINKOBJ (arg_node));
    PRAGMA_NUMPARAMS (new_node) = PRAGMA_NUMPARAMS (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    PRAGMA_FLAGSTRUCTURE (new_node) = PRAGMA_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPicm (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeIcm (ICM_NAME (arg_node), DUPTRAV (ICM_ARGS (arg_node)));

    /*
     * The ICM name is not copied here because ICM names are predominantly static
     * string constants and therefore aren't freed anyway.
     */

    ICM_INDENT_BEFORE (new_node) = ICM_INDENT_BEFORE (arg_node);
    ICM_INDENT_AFTER (new_node) = ICM_INDENT_AFTER (arg_node);

    ICM_FLAGSTRUCTURE (new_node) = ICM_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    ICM_FLAGSTRUCTURE (new_node) = ICM_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwith (node *arg_node, info *arg_info)
{
    node *new_node, *partn, *coden, *withopn, *vardec, *oldids;
    node *newavis;

    DBUG_ENTER ();

    if ((INFO_TYPE (arg_info) == DUP_SSA) && (NODE_TYPE (WITH_VEC (arg_node)) == N_ids)) {
        /*
         * to maintain the ssa-form we have to create new ids
         * for the elements of N_Nwithid and insert them into LUT
         */
        oldids = WITH_VEC (arg_node);

        newavis = TBmakeAvis (TRAVtmpVarName (IDS_NAME (oldids)),
                              TYcopyType (IDS_NTYPE (oldids)));

        vardec = TBmakeVardec (newavis, NULL);

        INFO_FUNDEFSSA (arg_info) = TCaddVardecs (INFO_FUNDEFSSA (arg_info), vardec);

        INFO_LUT (arg_info) = LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (oldids),
                                                 AVIS_NAME (newavis));

        INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_DECL (oldids),
                                                 AVIS_DECL (newavis));

        INFO_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (oldids), newavis);

        oldids = WITH_IDS (arg_node);
        while (oldids != NULL) {

            newavis = TBmakeAvis (TRAVtmpVarName (IDS_NAME (oldids)),
                                  TYcopyType (IDS_NTYPE (oldids)));

            vardec = TBmakeVardec (newavis, NULL);

            INFO_FUNDEFSSA (arg_info) = TCaddVardecs (INFO_FUNDEFSSA (arg_info), vardec);

            INFO_LUT (arg_info)
              = LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (oldids),
                                   AVIS_NAME (newavis));

            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_DECL (oldids),
                                   AVIS_DECL (newavis));

            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (oldids), newavis);
            oldids = IDS_NEXT (oldids);
        }
    }

    /*
     * very important: duplicate codes before parts! Otherwise the code
     * references of the parts can not be set correctly!
     */
    coden = DUPTRAV (WITH_CODE (arg_node));
    partn = DUPTRAV (WITH_PART (arg_node));
    withopn = DUPTRAV (WITH_WITHOP (arg_node));

    new_node = TBmakeWith (partn, coden, withopn);

    /* copy attributes */
    WITH_PRAGMA (new_node) = DUPTRAV (WITH_PRAGMA (arg_node));
    WITH_PARTS (new_node) = WITH_PARTS (arg_node);
    WITH_REFERENCED (new_node) = WITH_REFERENCED (arg_node);
    WITH_REFERENCED_FOLD (new_node) = WITH_REFERENCED_FOLD (arg_node);
    WITH_REFERENCES_FOLDED (new_node) = WITH_REFERENCES_FOLDED (arg_node);

    WITH_IN_MASK (new_node) = DupDfmask (WITH_IN_MASK (arg_node), arg_info);
    WITH_OUT_MASK (new_node) = DupDfmask (WITH_OUT_MASK (arg_node), arg_info);
    WITH_LOCAL_MASK (new_node) = DupDfmask (WITH_LOCAL_MASK (arg_node), arg_info);

    WITH_DIST (new_node) = STRcpy (WITH_DIST (arg_node));

    WITH_FLAGSTRUCTURE (new_node) = WITH_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPgenarray (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeGenarray (DUPTRAV (GENARRAY_SHAPE (arg_node)),
                               DUPTRAV (GENARRAY_DEFAULT (arg_node)));

    GENARRAY_MEM (new_node) = DUPTRAV (GENARRAY_MEM (arg_node));
    GENARRAY_SUB (new_node) = DUPTRAV (GENARRAY_SUB (arg_node));
    GENARRAY_RC (new_node) = DUPTRAV (GENARRAY_RC (arg_node));
    GENARRAY_DEFSHAPEEXPR (new_node) = DUPTRAV (GENARRAY_DEFSHAPEEXPR (arg_node));

    GENARRAY_IDX (new_node)
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), GENARRAY_IDX (arg_node));

    GENARRAY_NEXT (new_node) = DUPCONT (GENARRAY_NEXT (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    GENARRAY_FLAGSTRUCTURE (new_node) = GENARRAY_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/
node *
DUPmodarray (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeModarray (DUPTRAV (MODARRAY_ARRAY (arg_node)));

    MODARRAY_MEM (new_node) = DUPTRAV (MODARRAY_MEM (arg_node));
    MODARRAY_SUB (new_node) = DUPTRAV (MODARRAY_SUB (arg_node));
    MODARRAY_RC (new_node) = DUPTRAV (MODARRAY_RC (arg_node));

    MODARRAY_IDX (new_node)
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), MODARRAY_IDX (arg_node));

    MODARRAY_NEXT (new_node) = DUPCONT (MODARRAY_NEXT (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    MODARRAY_FLAGSTRUCTURE (new_node) = MODARRAY_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/
node *
DUPfold (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeFold (FOLD_FUNDEF (arg_node), DUPTRAV (FOLD_NEUTRAL (arg_node)));

    FOLD_ARGS (new_node) = DUPTRAV (FOLD_ARGS (arg_node));

    FOLD_GUARD (new_node) = DUPTRAV (FOLD_GUARD (arg_node));

    FOLD_ISPARTIALFOLD (new_node) = FOLD_ISPARTIALFOLD (arg_node);

    FOLD_FUNDEF (new_node)
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), FOLD_FUNDEF (arg_node));

    FOLD_NEXT (new_node) = DUPCONT (FOLD_NEXT (arg_node));

    if (FOLD_INITIAL (arg_node) != NULL) {
        FOLD_INITIAL (new_node) = DUPTRAV (FOLD_INITIAL (arg_node));
    }

    if (FOLD_PARTIALMEM (arg_node) != NULL) {
        FOLD_PARTIALMEM (new_node) = DUPTRAV (FOLD_PARTIALMEM (arg_node));
    }

    CopyCommonNodeData (new_node, arg_node);

    FOLD_FLAGSTRUCTURE (new_node) = FOLD_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/
node *
DUPspfold (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeSpfold (DUPTRAV (SPFOLD_NEUTRAL (arg_node)));

    SPFOLD_GUARD (new_node) = DUPTRAV (SPFOLD_GUARD (arg_node));

    SPFOLD_FN (new_node) = DUPspid (SPFOLD_FN (arg_node), arg_info);

    SPFOLD_NEXT (new_node) = DUPCONT (SPFOLD_NEXT (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    SPFOLD_FLAGSTRUCTURE (new_node) = SPFOLD_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/
node *
DUPbreak (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeBreak ();

    BREAK_MEM (new_node) = DUPTRAV (BREAK_MEM (arg_node));
    BREAK_NEXT (new_node) = DUPCONT (BREAK_NEXT (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    BREAK_FLAGSTRUCTURE (new_node) = BREAK_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/
node *
DUPpropagate (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakePropagate (DUPTRAV (PROPAGATE_DEFAULT (arg_node)));

    PROPAGATE_NEXT (new_node) = DUPCONT (PROPAGATE_NEXT (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    PROPAGATE_FLAGSTRUCTURE (new_node) = PROPAGATE_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPpart (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();
    DBUG_ASSERT (PART_CODE (arg_node), "N_part node has no valid PART_CODE");

    new_node
      = TBmakePart ((node *)LUTsearchInLutPp (INFO_LUT (arg_info), PART_CODE (arg_node)),
                    DUPTRAV (PART_WITHID (arg_node)),
                    DUPTRAV (PART_GENERATOR (arg_node)));

    CODE_INC_USED (PART_CODE (new_node));
    PART_NEXT (new_node) = DUPCONT (PART_NEXT (arg_node));

    PART_FLAGSTRUCTURE (new_node) = PART_FLAGSTRUCTURE (arg_node);

    if (PART_THREADBLOCKSHAPE (arg_node) != NULL) {
        PART_THREADBLOCKSHAPE (new_node)
          = DUParray (PART_THREADBLOCKSHAPE (arg_node), arg_info);
    }

    CopyCommonNodeData (new_node, arg_node);

    PART_FLAGSTRUCTURE (new_node) = PART_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPcode (node *arg_node, info *arg_info)
{
    node *new_node, *new_block, *new_cexprs;

    DBUG_ENTER ();

    /*
     * very important: duplicate cblock before cexprs! Otherwise the code
     * references of the cexprs can not set correctly!
     */
    new_block = DUPTRAV (CODE_CBLOCK (arg_node));
    new_cexprs = DUPTRAV (CODE_CEXPRS (arg_node));

    new_node = TBmakeCode (new_block, new_cexprs);

    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, new_node);

    CODE_NEXT (new_node) = DUPCONT (CODE_NEXT (arg_node));

    /*
     * CODE_USED is incremented in DUPpart() via CODE_INC_USED
     *                          in DUPwgrid() via CODE_INC_USED
     */
    CODE_USED (new_node) = 0;
    CODE_FLAGSTRUCTURE (new_node) = CODE_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    CODE_FLAGSTRUCTURE (new_node) = CODE_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwithid (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeWithid (DUPTRAV (WITHID_VEC (arg_node)), DUPTRAV (WITHID_IDS (arg_node)));

    WITHID_IDXS (new_node) = DUPTRAV (WITHID_IDXS (arg_node));

    WITHID_FLAGSTRUCTURE (new_node) = WITHID_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    WITHID_FLAGSTRUCTURE (new_node) = WITHID_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPgenerator (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeGenerator (GENERATOR_OP1 (arg_node), GENERATOR_OP2 (arg_node),
                                DUPTRAV (GENERATOR_BOUND1 (arg_node)),
                                DUPTRAV (GENERATOR_BOUND2 (arg_node)),
                                DUPTRAV (GENERATOR_STEP (arg_node)),
                                DUPTRAV (GENERATOR_WIDTH (arg_node)));

    GENERATOR_GENWIDTH (new_node) = DUPTRAV (GENERATOR_GENWIDTH (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    GENERATOR_FLAGSTRUCTURE (new_node) = GENERATOR_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPdefault (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeDefault ();

    CopyCommonNodeData (new_node, arg_node);

    DEFAULT_FLAGSTRUCTURE (new_node) = DEFAULT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwith2 (node *arg_node, info *arg_info)
{
    node *new_node, *id, *segs, *code, *withop, *memid;

    DBUG_ENTER ();

    /*
     * very important: duplicate codes before parts! Otherwise the code
     * references of the parts cannot be set correctly!
     */
    id = DUPTRAV (WITH2_WITHID (arg_node));
    code = DUPTRAV (WITH2_CODE (arg_node));
    segs = DUPTRAV (WITH2_SEGS (arg_node));
    withop = DUPTRAV (WITH2_WITHOP (arg_node));
    memid = DUPTRAV (WITH2_MEMID (arg_node));

    new_node = TBmakeWith2 (WITH2_DIMS (arg_node), id, segs, code, withop);

    WITH2_MEMID (new_node) = memid;

    WITH2_SIZE (new_node) = WITH2_SIZE (arg_node);

    WITH2_IN_MASK (new_node) = DupDfmask (WITH2_IN_MASK (arg_node), arg_info);
    WITH2_OUT_MASK (new_node) = DupDfmask (WITH2_OUT_MASK (arg_node), arg_info);
    WITH2_LOCAL_MASK (new_node) = DupDfmask (WITH2_LOCAL_MASK (arg_node), arg_info);

    WITH2_DIST (new_node) = STRcpy (WITH2_DIST (arg_node));

    WITH2_FLAGSTRUCTURE (new_node) = WITH2_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    WITH2_FLAGSTRUCTURE (new_node) = WITH2_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwlseg (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeWlseg (WLSEG_DIMS (arg_node), DUPTRAV (WLSEG_CONTENTS (arg_node)),
                            DUPCONT (WLSEG_NEXT (arg_node)));

    WLSEG_IDXINF (new_node) = DUPTRAV (WLSEG_IDXINF (arg_node));
    WLSEG_IDXSUP (new_node) = DUPTRAV (WLSEG_IDXSUP (arg_node));

    WLSEG_UBV (new_node) = DUPTRAV (WLSEG_UBV (arg_node));

    WLSEG_BLOCKS (new_node) = WLSEG_BLOCKS (arg_node);

    WLSEG_BV (new_node) = DUPTRAV (WLSEG_BV (arg_node));
    WLSEG_SV (new_node) = DUPTRAV (WLSEG_SV (arg_node));
    WLSEG_HOMSV (new_node) = DUPTRAV (WLSEG_HOMSV (arg_node));

    if (WLSEG_SCHEDULING (arg_node) != NULL) {
        WLSEG_SCHEDULING (new_node) = SCHcopyScheduling (WLSEG_SCHEDULING (arg_node));
    }

    if (WLSEG_TASKSEL (arg_node) != NULL) {
        WLSEG_TASKSEL (new_node) = SCHcopyTasksel (WLSEG_TASKSEL (arg_node));
    }

    WLSEG_FLAGSTRUCTURE (new_node) = WLSEG_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwlblock (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeWlblock (WLBLOCK_LEVEL (arg_node), WLBLOCK_DIM (arg_node),
                              DUPTRAV (WLBLOCK_BOUND1 (arg_node)),
                              DUPTRAV (WLBLOCK_BOUND2 (arg_node)),
                              DUPTRAV (WLBLOCK_STEP (arg_node)),
                              DUPTRAV (WLBLOCK_NEXTDIM (arg_node)),
                              DUPTRAV (WLBLOCK_CONTENTS (arg_node)),
                              DUPCONT (WLBLOCK_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    WLBLOCK_FLAGSTRUCTURE (new_node) = WLBLOCK_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwlublock (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeWlublock (WLUBLOCK_LEVEL (arg_node), WLUBLOCK_DIM (arg_node),
                               DUPTRAV (WLUBLOCK_BOUND1 (arg_node)),
                               DUPTRAV (WLUBLOCK_BOUND2 (arg_node)),
                               DUPTRAV (WLUBLOCK_STEP (arg_node)),
                               DUPTRAV (WLUBLOCK_NEXTDIM (arg_node)),
                               DUPTRAV (WLUBLOCK_CONTENTS (arg_node)),
                               DUPCONT (WLUBLOCK_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    WLUBLOCK_FLAGSTRUCTURE (new_node) = WLUBLOCK_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwlstride (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeWlstride (WLSTRIDE_LEVEL (arg_node), WLSTRIDE_DIM (arg_node),
                               DUPTRAV (WLSTRIDE_BOUND1 (arg_node)),
                               DUPTRAV (WLSTRIDE_BOUND2 (arg_node)),
                               DUPTRAV (WLSTRIDE_STEP (arg_node)),
                               DUPTRAV (WLSTRIDE_CONTENTS (arg_node)),
                               DUPCONT (WLSTRIDE_NEXT (arg_node)));

    WLSTRIDE_PART (new_node) = WLSTRIDE_PART (arg_node);
    WLSTRIDE_FLAGSTRUCTURE (new_node) = WLSTRIDE_FLAGSTRUCTURE (arg_node);

    /*
     * duplicated strides are not modified yet ;)
     */
    WLSTRIDE_ISMODIFIED (new_node) = FALSE;

    CopyCommonNodeData (new_node, arg_node);

    WLSTRIDE_FLAGSTRUCTURE (new_node) = WLSTRIDE_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwlgrid (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeWlgrid (WLGRID_LEVEL (arg_node), WLGRID_DIM (arg_node),
                             (node *)LUTsearchInLutPp (INFO_LUT (arg_info),
                                                       WLGRID_CODE (arg_node)),
                             DUPTRAV (WLGRID_BOUND1 (arg_node)),
                             DUPTRAV (WLGRID_BOUND2 (arg_node)),
                             DUPTRAV (WLGRID_NEXTDIM (arg_node)),
                             DUPCONT (WLGRID_NEXT (arg_node)));

    if (WLGRID_CODE (new_node) != NULL) {
        CODE_INC_USED (WLGRID_CODE (new_node));
    }

    WLGRID_FLAGSTRUCTURE (new_node) = WLGRID_FLAGSTRUCTURE (arg_node);

    /*
     * duplicated grids are not modified yet ;)
     */
    WLGRID_ISMODIFIED (new_node) = FALSE;

    CopyCommonNodeData (new_node, arg_node);

    WLGRID_FLAGSTRUCTURE (new_node) = WLGRID_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *DUPmt( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a N_mt, especially the DFMmasks are copied.
 *
 ******************************************************************************/

node *
DUPmt (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeMt (DUPTRAV (MT_REGION (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    MT_FLAGSTRUCTURE (new_node) = MT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupSt( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a N_st, especially the DFMmasks are copied.
 *
 ******************************************************************************/

node *
DUPst (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeSt (DUPTRAV (ST_REGION (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    ST_FLAGSTRUCTURE (new_node) = ST_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupCudaSt( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a N_cudast, especially the DFMmasks are copied.
 *
 ******************************************************************************/

node *
DUPcudast (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeSt (DUPTRAV (CUDAST_REGION (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    CUDAST_FLAGSTRUCTURE (new_node) = CUDAST_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupAvis( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a N_avis node. Does not set AVIS_DECL!!
 *
 ******************************************************************************/

node *
DUPavis (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeAvis (STRcpy (LUTsearchInLutSs (INFO_LUT (arg_info), AVIS_NAME (arg_node))),
                    TYcopyType (AVIS_TYPE (arg_node)));

    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, new_node);
    DBUG_PRINT ("DUPavis will map %s to %s", AVIS_NAME (arg_node), AVIS_NAME (new_node));

    AVIS_SSACOUNT (new_node)
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), AVIS_SSACOUNT (arg_node));

    AVIS_SSAASSIGN (new_node)
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), AVIS_SSAASSIGN (arg_node));

    AVIS_DECLTYPE (new_node) = TYcopyType (AVIS_DECLTYPE (arg_node));
    AVIS_CONSTRTYPE (new_node) = TYcopyType (AVIS_CONSTRTYPE (arg_node));
    AVIS_CONSTRVAR (new_node) = DUPTRAV (AVIS_CONSTRVAR (arg_node));
    AVIS_CONSTRSET (new_node) = DUPTRAV (AVIS_CONSTRSET (arg_node));
    AVIS_SSALPINV (new_node) = AVIS_SSALPINV (arg_node);
    AVIS_SSASTACK (new_node) = DUPTRAV (AVIS_SSASTACK (arg_node));

    AVIS_SSADEFINED (new_node) = AVIS_SSADEFINED (arg_node);
    AVIS_SSATHEN (new_node) = AVIS_SSATHEN (arg_node);
    AVIS_SSAELSE (new_node) = AVIS_SSAELSE (arg_node);
    AVIS_NEEDCOUNT (new_node) = AVIS_NEEDCOUNT (arg_node);
    AVIS_SUBST (new_node) = AVIS_SUBST (arg_node);

    AVIS_WITH3FOLD (new_node) = AVIS_WITH3FOLD (arg_node);

    /* DEBUG TEMP FIXME  chasing missing info in N_ap */
    AVIS_DIM (new_node) = DUPTRAV (AVIS_DIM (arg_node));
    AVIS_SHAPE (new_node) = DUPTRAV (AVIS_SHAPE (arg_node));
    AVIS_MIN (new_node) = DUPTRAV (AVIS_MIN (arg_node));
    AVIS_MAX (new_node) = DUPTRAV (AVIS_MAX (arg_node));
    /* DEBUG TEMP   FIXME chasing missing info in N_ap */

    AVIS_FLAGSTRUCTURE (new_node) = AVIS_FLAGSTRUCTURE (arg_node);

    if (AVIS_DEMAND (arg_node) != NULL) {
        AVIS_DEMAND (new_node) = COcopyConstant (AVIS_DEMAND (arg_node));
    }

    CopyCommonNodeData (new_node, arg_node);

    AVIS_FLAGSTRUCTURE (new_node) = AVIS_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupSSAstack( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a N_ssastack node.
 *
 ******************************************************************************/

node *
DUPssastack (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeSsastack (SSASTACK_AVIS (arg_node), SSASTACK_NESTLEVEL (arg_node),
                               DUPCONT (SSASTACK_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    SSASTACK_FLAGSTRUCTURE (new_node) = SSASTACK_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupSSAcnt( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a N_ssacnt node.
 *
 ******************************************************************************/

node *
DUPssacnt (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeSsacnt (SSACNT_COUNT (arg_node), STRcpy (SSACNT_BASEID (arg_node)),
                             DUPCONT (SSACNT_NEXT (arg_node)));

    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, new_node);

    CopyCommonNodeData (new_node, arg_node);

    SSACNT_FLAGSTRUCTURE (new_node) = SSACNT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *Duperror( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a Error node.
 *
 ******************************************************************************/

node *
DUPerror (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeError (STRcpy (ERROR_MESSAGE (arg_node)), ERROR_ANYPHASE (arg_node),
                            DUPTRAV (ERROR_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    ERROR_FLAGSTRUCTURE (new_node) = ERROR_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DUPfunbundle( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a FunBundle node.
 *
 ******************************************************************************/

node *
DUPfunbundle (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeFunbundle (STRcpy (FUNBUNDLE_NAME (arg_node)),
                                NSdupNamespace (FUNBUNDLE_NS (arg_node)),
                                STRcpy (FUNBUNDLE_EXTNAME (arg_node)),
                                FUNBUNDLE_ARITY (arg_node),
                                DUPTRAV (FUNBUNDLE_FUNDEF (arg_node)),
                                DUPCONT (FUNBUNDLE_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    FUNBUNDLE_FLAGSTRUCTURE (new_node) = FUNBUNDLE_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DUPlivevars( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a Livevars node.
 *
 ******************************************************************************/

node *
DUPlivevars (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeLivevars (LIVEVARS_AVIS (arg_node), DUPCONT (LIVEVARS_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    LIVEVARS_FLAGSTRUCTURE (new_node) = LIVEVARS_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *DUPwiths( node *arg_node, info *arg_info)
 *
 * @brief Duplicates a withs node.
 *
 * @param arg_node withs node
 * @param arg_info info structure
 *
 * @return copy of withs node
 ******************************************************************************/
node *
DUPwiths (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node
      = TBmakeWiths (DUPTRAV (WITHS_WITH (arg_node)), DUPTRAV (WITHS_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    WITHS_FLAGSTRUCTURE (new_node) = WITHS_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *DUPwith3( node *arg_node, info *arg_info)
 *
 * @brief Duplicates a with3 node.
 *
 * @param arg_node with3 node
 * @param arg_info info structure
 *
 * @return copy of with3 node
 ******************************************************************************/
node *
DUPwith3 (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = TBmakeWith3 (DUPTRAV (WITH3_RANGES (arg_node)),
                            DUPTRAV (WITH3_OPERATIONS (arg_node)));

    WITH3_DIST (new_node) = STRcpy (WITH3_DIST (arg_node));

    WITH3_FLAGSTRUCTURE (new_node) = WITH3_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *DUPrange( node *arg_node, info *arg_info)
 *
 * @brief Duplicates a range node.
 *
 * @param arg_node range node
 * @param arg_info info structure
 *
 * @return copy of range node
 ******************************************************************************/
node *
DUPrange (node *arg_node, info *arg_info)
{
    node *new_node, *body, *result, *index, *idxs;

    DBUG_ENTER ();

    /* Force correct traversal order of ids, id */
    idxs = DUPTRAV (RANGE_IDXS (arg_node));
    index = DUPTRAV (RANGE_INDEX (arg_node));
    body = DUPTRAV (RANGE_BODY (arg_node));
    result = DUPTRAV (RANGE_RESULTS (arg_node));

    new_node = TBmakeRange (index, DUPTRAV (RANGE_LOWERBOUND (arg_node)),
                            DUPTRAV (RANGE_UPPERBOUND (arg_node)),
                            DUPTRAV (RANGE_CHUNKSIZE (arg_node)), body, result, idxs,
                            DUPCONT (RANGE_NEXT (arg_node)));

    if (RANGE_IIRR (arg_node) != NULL) {
        RANGE_IIRR (new_node) = DUPTRAV (RANGE_IIRR (arg_node));
    }

    CopyCommonNodeData (new_node, arg_node);

    RANGE_FLAGSTRUCTURE (new_node) = RANGE_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * functions:
 *   node *DupTree( node *arg_node)
 *   node *DupTreeSSA( node *arg_node, node *fundef)
 *   node *DupTree_Type( node *arg_node, lut_t * lut, int type)
 *   node *DupTreeLUT( node *arg_node, lut_t * lut)
 *   node *DupTreeLUTSSA( node *arg_node, lut_t * lut, node *fundef)
 *   node *DupTreeLUT_Type( node *arg_node, lut_t * lut, int type)
 *   node *DupNode( node *arg_node)
 *   node *DupNodeSSA( node *arg_node, node *fundef)
 *   node *DupNode_Type( node *arg_node, lut_t * lut, int type)
 *   node *DupNodeLUT( node *arg_node, lut_t * lut)
 *   node *DupNodeLUTSSA( node *arg_node, lut_t * lut, node *fundef)
 *   node *DupNodeLUT_Type( node *arg_node, lut_t * lut, int type)
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
 *   - If you need some special behaviour triggered by INFO_TYPE use the
 *     specific DupXxx_Type() version.
 *     Legal values for the parameter 'type' are DUP_INLINE, DUP_WLF, ...
 *   - If you want to have the copy in ssa-form use one of the DupXxxSSA()
 *     functions. This functions may only be used to copy code which will be
 *     used inside the passed N_fundef node fundef. This restriction is caused
 *     by the fact that all new vardecs will be put at the fundef's vardec
 *     chain
 *
 ******************************************************************************/

node *
DUPdoDupTree (node *arg_node)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = DupTreeOrNodeLutType (FALSE, arg_node, NULL, DUP_NORMAL, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupTreeSsa (node *arg_node, node *fundef)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = DupTreeOrNodeLutType (FALSE, arg_node, NULL, DUP_SSA, fundef);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupTreeType (node *arg_node, int type)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = DupTreeOrNodeLutType (FALSE, arg_node, NULL, type, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupTreeLut (node *arg_node, lut_t *lut)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = DupTreeOrNodeLutType (FALSE, arg_node, lut, DUP_NORMAL, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupTreeLutSsa (node *arg_node, lut_t *lut, node *fundef)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = DupTreeOrNodeLutType (FALSE, arg_node, lut, DUP_SSA, fundef);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupTreeLutType (node *arg_node, lut_t *lut, int type)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = DupTreeOrNodeLutType (FALSE, arg_node, lut, type, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupNode (node *arg_node)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = DupTreeOrNodeLutType (TRUE, arg_node, NULL, DUP_NORMAL, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupNodeSsa (node *arg_node, node *fundef)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = DupTreeOrNodeLutType (TRUE, arg_node, NULL, DUP_SSA, fundef);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupNodeType (node *arg_node, int type)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = DupTreeOrNodeLutType (TRUE, arg_node, NULL, type, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupNodeLut (node *arg_node, lut_t *lut)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = DupTreeOrNodeLutType (TRUE, arg_node, lut, DUP_NORMAL, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupNodeLutSsa (node *arg_node, lut_t *lut, node *fundef)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = DupTreeOrNodeLutType (TRUE, arg_node, lut, DUP_SSA, fundef);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupNodeLutType (node *arg_node, lut_t *lut, int type)
{
    node *new_node;

    DBUG_ENTER ();

    new_node = DupTreeOrNodeLutType (TRUE, arg_node, lut, type, NULL);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * Function:
 *   nodelist *DUPdupNodelist( nodelist *nl)
 *
 * Description:
 *
 *
 ******************************************************************************/

nodelist *
DUPdupNodelist (nodelist *nl)
{
    nodelist *new_nl;

    DBUG_ENTER ();

    new_nl = DupNodelist (nl, NULL);

    DBUG_RETURN (new_nl);
}

/******************************************************************************
 *
 * Function:
 *   node *DUPdupIdsId( ids *arg_ids)
 *
 * Description:
 *   Duplicates an IDS and returns a *N_id* node.
 *
 ******************************************************************************/

node *
DUPdupIdsId (node *arg_ids)
{
    node *new_id;

    DBUG_ENTER ();

    new_id = TBmakeId (IDS_AVIS (arg_ids));

    DBUG_RETURN (new_id);
}

/******************************************************************************
 *
 * Function:
 *   ids *DUPdupIdIds( node *old_id)
 *
 * Description:
 *   Duplicates a N_id node and returns an *IDS*.
 *
 ******************************************************************************/

node *
DUPdupIdIds (node *old_id)
{
    node *new_ids;

    DBUG_ENTER ();

    new_ids = TBmakeIds (ID_AVIS (old_id), NULL);

    DBUG_RETURN (new_ids);
}

/******************************************************************************
 *
 * Function:
 *   node *DUPdupIdsIdNt( ids *arg_ids)
 *
 * Description:
 *   Duplicates an IDS and returns a *N_id* node.
 *   Sets ID_NT_TAG.
 *
 ******************************************************************************/

node *
DUPdupIdsIdNt (node *arg_ids)
{
    node *new_id;

    DBUG_ENTER ();

    new_id = DUPdupIdsId (arg_ids);

    DBUG_ASSERT (IDS_NTYPE (arg_ids) != NULL, "NT_TAG: no type found!");
    ID_NT_TAG (new_id) = NTUcreateNtTagFromNType (IDS_NAME (arg_ids), IDS_NTYPE (arg_ids));

    DBUG_RETURN (new_id);
}

/******************************************************************************
 *
 * Function:
 *   node *DUPdupIdNt( info *arg_id)
 *
 * Description:
 *   Duplicates a N_id node.
 *   Sets ID_NT_TAG.
 *
 ******************************************************************************/

node *
DUPdupIdNt (node *arg_id)
{
    node *new_id;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_id) == N_id, "DupId_NT: no N_id node found!");
    new_id = DUPdoDupNode (arg_id);

    DBUG_ASSERT (ID_NTYPE (arg_id) != NULL, "NT_TAG: no type found!");

    ID_NT_TAG (new_id) = NTUcreateNtTagFromNType (ID_NAME (arg_id), ID_NTYPE (arg_id));

    DBUG_RETURN (new_id);
}

/******************************************************************************
 *
 * Function:
 *   node *DUPdupNodeNt( node *arg_node)
 *
 * Description:
 *   Duplicates a node. Sets ID_NT_TAG for N_id nodes.
 *
 ******************************************************************************/

node *
DUPdupNodeNt (node *arg_node)
{
    node *new_node;

    DBUG_ENTER ();

    if (NODE_TYPE (arg_node) == N_id) {
        new_node = DUPdupIdNt (arg_node);
    } else {
        new_node = DUPdoDupNode (arg_node);
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DUPdupExprsNt( node *exprs)
 *
 * Description:
 *   Duplicates a N_exprs chain and transforms all N_id nodes found into
 *   tagged N_id nodes.
 *
 ******************************************************************************/

node *
DUPdupExprsNt (node *exprs)
{
    node *expr;
    node *new_exprs;

    DBUG_ENTER ();

    if (exprs != NULL) {
        DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "no N_exprs node found!");

        expr = EXPRS_EXPR (exprs);
        DBUG_ASSERT (expr != NULL, "N_exprs node contains no data!");

        new_exprs = TBmakeExprs (DUPdupNodeNt (expr), DUPdupExprsNt (EXPRS_NEXT (exprs)));
    } else {
        new_exprs = NULL;
    }

    DBUG_RETURN (new_exprs);
}

/** <!--********************************************************************-->
 *
 * @fn node *DUPgetCopiedSpecialFundefs( void)
 *
 *   @brief  provides duplicated special functions for safe introduction
 *           into global fundef chain. By default this function is called
 *           by TRAVdo after having traversed an N_module node.
 *
 *   @return the N_fundef chain of duplicated special functions
 *
 *****************************************************************************/

node *
DUPgetCopiedSpecialFundefs (void)
{
    node *store;

    DBUG_ENTER ();

    DBUG_EXECUTE_TAG ("DUPSF", do {
        node *fundef;
        fundef = store_copied_special_fundefs;
        while (fundef != NULL) {
            DBUG_PRINT_TAG ("DUPSF",
                            "Released from DupTree special function hook:\n %s( %s)",
                            CTIitemName (fundef), CTIfunParams (fundef));
            fundef = FUNDEF_NEXT (fundef);
        }
    } while (0));

    store = store_copied_special_fundefs;
    store_copied_special_fundefs = NULL;

    DBUG_RETURN (store);
}

/** <!--********************************************************************-->
 *
 * @fn node *DUPgetCopiedSpecialFundefsHook( void)
 *
 *   @brief  provides duplicated special functions hook for inspection.
 *           This is needed in Lacfun check.
 *
 *   @return the N_fundef chain of duplicated special functions
 *
 *****************************************************************************/

node *
DUPgetCopiedSpecialFundefsHook (void)
{
    DBUG_ENTER ();

    DBUG_RETURN (store_copied_special_fundefs);
}

/** <!--********************************************************************-->
 *
 * @fn node *DUPtf( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @return
 *
 *****************************************************************************/

node *
DUPtfspec (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    node *new_node;
    new_node
      = TBmakeTfspec (DUPTRAV (TFSPEC_DEFS (arg_node)), DUPTRAV (TFSPEC_RELS (arg_node)));

    DBUG_RETURN (new_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DUPtf( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @return
 *
 *****************************************************************************/

node *
DUPtfdag (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    node *new_node;
    new_node = TBmakeTfdag (DUPTRAV (TFSPEC_DEFS (arg_node)));

    DBUG_RETURN (new_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DUPtf( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @return
 *
 *****************************************************************************/

node *
DUPtfvertex (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    node *new_node;

    new_node = TBmakeTfvertex (DUPTRAV (TFVERTEX_PARENTS (arg_node)),
                               DUPTRAV (TFVERTEX_CHILDREN (arg_node)),
                               DUPCONT (TFVERTEX_NEXT (arg_node)));

    DBUG_RETURN (new_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DUPtf( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @return
 *
 *****************************************************************************/

node *
DUPtfrel (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    node *new_node;

    new_node
      = TBmakeTfrel (STRcpy (TFREL_SUPERTAG (arg_node)), STRcpy (TFREL_SUBTAG (arg_node)),
                     DUPTRAV (TFREL_COND (arg_node)), DUPCONT (TFREL_NEXT (arg_node)));

    DBUG_RETURN (new_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DUPtf( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @return
 *
 *****************************************************************************/

node *
DUPtfedge (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    node *new_node;

    new_node = TBmakeTfedge (TFEDGE_TARGET (arg_node), DUPCONT (TFEDGE_NEXT (arg_node)));

    DBUG_RETURN (new_node);
}

node *
DUPtypecomponentarg (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    node *new_node;
    new_node = TBmakeTypecomponentarg (TYPECOMPONENTARG_TAG (arg_node),
                                       TYPECOMPONENTARG_TAGTYPE (arg_node),
                                       DUPCONT (TYPECOMPONENTARG_NEXT (arg_node)));

    DBUG_RETURN (new_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DUPtf( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @return
 *
 *****************************************************************************/

node *
DUPtfexpr (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    node *new_node, *operand1, *operand2;

    if (TFEXPR_OPERAND1 (arg_node) == NULL) {
        operand1 = NULL;
    } else {
        operand1 = DUPTRAV (TFEXPR_OPERAND1 (arg_node));
    }

    if (TFEXPR_OPERAND2 (arg_node) == NULL) {
        operand2 = NULL;
    } else {
        operand2 = DUPTRAV (TFEXPR_OPERAND2 (arg_node));
    }

    if (TFEXPR_OPERATOR (arg_node) != NULL) {
        new_node = TBmakeTfexpr (STRcpy (TFEXPR_OPERATOR (arg_node)), operand1, operand2);
    } else {
        new_node = TBmakeTfexpr (NULL, operand1, operand2);
    }

    TFEXPR_ASSIGNEEID (new_node) = STRcpy (TFEXPR_ASSIGNEEID (arg_node));
    TFEXPR_VALUE (new_node) = TFEXPR_VALUE (arg_node);

    DBUG_RETURN (new_node);
}

#undef DBUG_PREFIX
