/*
 * $Id: ivextrema.c 15815 2008-10-24 18:04:47Z rbe $
 */

/** <!--********************************************************************-->
 *
 * @defgroup ivexi Index Vector Extrema Insertion Traversal
 *
 * This traversal inserts maxima and minima for index vector variables
 * in with-loops. Later optimizations will propagate these maxima
 * and minima, and use then as input for other optimizations,
 * such as with-loop folding and guard removal.
 *
 * Because -ssaiv is likely never to work unless one of us wins
 * the lottery and can hire minions to rewrite all the SAC
 * optimizations, we adopt the following kludge:
 *
 * Rather than annotating the WITH_IDs, we introduce temps, annotate
 * those, and rename all code block references to the WITH_IDs
 * to refer to the temps. E.g:
 *
 *   z = with {   (lb <= iv < ub) :
 *                  X[iv];
 *            } : genarray(shp,defaultcell);
 *
 *  This will become:
 *
 *   z = with {   (lb <= iv < ub) :
 *                  tmp = iv;
 *                  X[tmp];
 *            } : genarray(shp,defaultcell);
 *
 * We then set
 *             AVIS_MINVAL(ID_AVIS(tmp)) = GENERATOR_BOUND1(wl)
 *             AVIS_MAXVAL(ID_AVIS(tmp)) = GENERATOR_BOUND2(wl);
 *
 * With any luck, these min/val values will be propagated down to the
 * X[tmp], at which point SWLFI can introduce inferences on them
 * for partition intersection computation. Although not required
 * by the above example, it is required for more complex linear functions
 * of the index vector, such as:
 *
 *              X[ (k*tmp) + offset ];
 *
 * They do, however, mark the end of shared N_code blocks in WLs,
 * just as -ssaiv does.
 *
 * @ingroup ivexi
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file ivextrema.c
 *
 * Prefix: IVEXI
 *
 *****************************************************************************/
#include "ivextrema.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"
#include "shape.h"
#include "constants.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "pattern_match.h"
#include "LookUpTable.h"
#include "check.h"
#include "makedimexpr.h"
#include "makeshapeexpr.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *vardecs;
    node *preassignspart;
    node *preassignswith;
    node *code;
    lut_t *lutvars;
    lut_t *lutcodes;
};

/**
 * INFO macros
 */
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASSIGNSPART(n) ((n)->preassignspart)
#define INFO_PREASSIGNSWITH(n) ((n)->preassignswith)
#define INFO_CODE(n) ((n)->code)
#define INFO_LUTVARS(n) ((n)->lutvars)
#define INFO_LUTCODES(n) ((n)->lutcodes)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNSPART (result) = NULL;
    INFO_PREASSIGNSWITH (result) = NULL;
    INFO_CODE (result) = NULL;
    INFO_LUTVARS (result) = NULL;
    INFO_LUTCODES (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}
/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *IVEXIdoInsertIndexVectorExtrema( node *arg_node)
 *
 *****************************************************************************/
node *
IVEXIdoInsertIndexVectorExtrema (node *arg_node)
{
    info *arg_info;
    DBUG_ENTER ("IVEXIdoIndexVectorExtremaInsertion");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "IVEXIdoIndexVectorExtremaInsertion expected N_modules");

    arg_info = MakeInfo ();

    DBUG_PRINT ("IVEXI", ("Starting index vector extrema insertion traversal."));

    TRAVpush (TR_ivexi);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DBUG_PRINT ("IVEXI", ("Index vector extrema insertion complete."));

    arg_info = FreeInfo (arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *generateSelect( node *arg_node, info *arg_info, int k)
 *
 * @brief: Create:
 *             kk = k;          NB Flatten k
 *             k' = [kk];
 *             s0 = _sel_VxA_([k], bound);
 *             and associated vardecs.
 * @params:
 *     arg_node: The N_id node for a generator bound.
 *     arg_info: Your basic arg_info stuff.
 *     k:        which element of the bound we want to select.
 *
 * @return: The N_id of the new temp, s0.
 *
 *****************************************************************************/
node *
generateSelect (node *bound, info *arg_info, int k)
{
    node *bavis;

    node *kavis;
    node *kid;
    node *kids;
    node *kass;

    node *zavis;
    node *zid;
    node *zids;
    node *zass;

    node *favis;
    node *fid;
    node *fids;
    node *fass;

    DBUG_ENTER ("generateSelect");

    bavis = ID_AVIS (bound);

    /* Flatten k */

    favis = MakeScalarAvis (TRAVtmpVarName (AVIS_NAME (bavis)));
    INFO_VARDECS (arg_info) = TBmakeVardec (favis, INFO_VARDECS (arg_info));

    fid = TBmakeId (favis);
    fids = TBmakeIds (favis, NULL);
    fass = TBmakeAssign (TBmakeLet (fids, TBmakeNum (k)), NULL);
    INFO_PREASSIGNSWITH (arg_info)
      = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), fass);
    AVIS_SSAASSIGN (favis) = fass;
    AVIS_DIM (favis) = TBmakeNum (0);
    AVIS_SHAPE (favis) = TCmakeIntVector (NULL);
    AVIS_MINVAL (favis) = favis;
    AVIS_MAXVAL (favis) = favis;

    /* Create k' = [k]; */
    kavis = MakeVectAvis (TRAVtmpVarName (AVIS_NAME (bavis)), TBmakeNum (1));
    INFO_VARDECS (arg_info) = TBmakeVardec (kavis, INFO_VARDECS (arg_info));

    kid = TBmakeId (kavis);
    kids = TBmakeIds (kavis, NULL);
    kass
      = TBmakeAssign (TBmakeLet (kids, TBmakeArray (TYmakeAKS (TYmakeSimpleType (T_int),
                                                               SHcreateShape (0)),
                                                    SHcreateShape (1, 1),
                                                    TBmakeExprs (fid, NULL))),
                      NULL);
    INFO_PREASSIGNSWITH (arg_info)
      = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), kass);
    AVIS_SSAASSIGN (kavis) = kass;
    AVIS_DIM (kavis) = TBmakeNum (1);
    AVIS_SHAPE (kavis) = TCmakeIntVector (TBmakeExprs (TBmakeNum (1), NULL));
    AVIS_MINVAL (kavis) = kavis;
    AVIS_MAXVAL (kavis) = kavis;

    /* Create s0 = _sel_VxA_([k], bound);  */

    zavis
      = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (bavis)), TYcopyType (AVIS_TYPE (bavis)));
    INFO_VARDECS (arg_info) = TBmakeVardec (zavis, INFO_VARDECS (arg_info));
    zid = TBmakeId (zavis);
    zids = TBmakeIds (zavis, NULL);
    zass
      = TBmakeAssign (TBmakeLet (zids, TCmakePrf2 (F_sel_VxA, kid, DUPdoDupTree (bound))),
                      NULL);
    INFO_PREASSIGNSWITH (arg_info)
      = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), zass);
    AVIS_SSAASSIGN (zavis) = zass;
    AVIS_DIM (zavis) = TBmakeNum (0);
    AVIS_SHAPE (zavis) = TCmakeIntVector (NULL);
    AVIS_MINVAL (zavis) = zavis;
    AVIS_MAXVAL (zavis) = zavis;

    DBUG_PRINT ("IVEXI", ("generateSelect introduced temp index variable: %s for: %s",
                          AVIS_NAME (zavis), AVIS_NAME (ID_AVIS (bound))));
    DBUG_RETURN (zid);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXItmpVec( node *arg_node, info *arg_info, node* oldavis))
 *
 * @brief:
 *      Insert a temp, its vardec, and an assign for oldavis IV.
 *      We start with an WITHID_VEC:
 *
 *           IV
 *
 *      and want:
 *
 *           iv' = _attachminmax_(IV, GENERATOR_BOUND1(partn),
 *                                    GENERATOR_BOUND2(partn));
 *
 *      and an appropriate vardec for iv', of course.
 *
 *      We build an N_id for IV because WITHIDs don't have them,
 *      just N_avis nodes. If -ssaiv becomes a reality, this
 *      can be scrapped, and we can attach the minmax to IV
 *      directly.
 *
 * @params:
 *     oldavis: The N_avis of the name for which we want to build iv'.
 *     arg_node: An N_part of the WL.
 *     arg_info: Your basic arg_info stuff.
 *
 * @return: The N_id of the new temp.
 *
 *****************************************************************************/
node *
IVEXItmpVec (node *arg_node, info *arg_info, node *oldavis)
{
    node *avis;
    node *nas;
    node *nid;
    node *args;
    node *b1;
    node *b2;

    DBUG_ENTER ("IVEXItmpVec");

    DBUG_ASSERT (N_avis == NODE_TYPE (oldavis), "IVEXItmpVec expected N_avis");
    b1 = GENERATOR_BOUND1 (PART_GENERATOR (arg_node));
    b2 = GENERATOR_BOUND2 (PART_GENERATOR (arg_node));

    DBUG_ASSERT (N_id == NODE_TYPE (b1),
                 "IVEXItmpVec expected N_id for GENERATOR_BOUND1");
    DBUG_ASSERT (N_id == NODE_TYPE (b2),
                 "IVEXItmpVec expected N_id for GENERATOR_BOUND2");
    avis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (oldavis)),
                       TYcopyType (AVIS_TYPE (oldavis)));
    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

    args = TBmakeExprs (TBmakeId (oldavis),
                        TBmakeExprs (DUPdoDupTree (b1),
                                     TBmakeExprs (DUPdoDupTree (b2), NULL)));
    nas = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                   TBmakePrf (F_attachminmax, args)),
                        NULL);
    INFO_PREASSIGNSPART (arg_info) = TCappendAssign (INFO_PREASSIGNSPART (arg_info), nas);
    AVIS_SSAASSIGN (avis) = nas;
    AVIS_DIM (avis) = DUPdoDupTree (AVIS_DIM (oldavis));
    AVIS_SHAPE (avis) = DUPdoDupTree (AVIS_SHAPE (oldavis));
    AVIS_MINVAL (avis) = ID_AVIS (b1);
    AVIS_MAXVAL (avis) = ID_AVIS (b2);
    nid = TBmakeId (avis);
    DBUG_PRINT ("IVEXI", ("IVEXItmpVec introduced temp index variable: %s for: %s",
                          AVIS_NAME (avis), AVIS_NAME (oldavis)));
    DBUG_RETURN (nid);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXItmpIds( node *arg_node, info *arg_info, node* oldavis, int k)
 *
 * @brief:  Same as IVEXItmpVec, except this one handles one
 *          of the WITHID_IDS scalar variables i,j,k in
 *            IV = [i,j,k]
 *
 *          This is slightly more complex, because we have to
 *          select the appropriate generator elements.
 *
 *     oldavis: The N_avis of the name for which we want to build iv'.
 *     arg_node: An N_part of the WL.
 *     arg_info: Your basic arg_info stuff.
 *     k:        the index of the scalar, e.g., i=0, j=1, k=2
 * @return: The N_id of the new temp.
 *
 *****************************************************************************/
node *
IVEXItmpIds (node *arg_node, info *arg_info, node *oldavis, int k)
{
    node *avis;
    node *nas;
    node *nid;
    node *args;
    node *b1;
    node *b2;

    DBUG_ENTER ("IVEXItmpIds");

    DBUG_ASSERT (N_avis == NODE_TYPE (oldavis), "IVEXItmpIds expected N_avis");

    b1 = GENERATOR_BOUND1 (PART_GENERATOR (arg_node));
    b2 = GENERATOR_BOUND2 (PART_GENERATOR (arg_node));
    DBUG_ASSERT (N_id == NODE_TYPE (b1),
                 "IVEXItmpIds expected N_id for GENERATOR_BOUND1");
    DBUG_ASSERT (N_id == NODE_TYPE (b2),
                 "IVEXItmpIds expected N_id for GENERATOR_BOUND2");

    b1 = generateSelect (b1, arg_info, k);
    b2 = generateSelect (b2, arg_info, k);

    avis = MakeScalarAvis (TRAVtmpVarName (AVIS_NAME (oldavis)));
    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

    args = TBmakeExprs (TBmakeId (oldavis), TBmakeExprs (b1, TBmakeExprs (b2, NULL)));
    nas = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                   TBmakePrf (F_attachminmax, args)),
                        NULL);
    INFO_PREASSIGNSPART (arg_info) = TCappendAssign (INFO_PREASSIGNSPART (arg_info), nas);
    AVIS_SSAASSIGN (avis) = nas;
    AVIS_DIM (avis) = DUPdoDupTree (AVIS_DIM (oldavis));
    AVIS_SHAPE (avis) = DUPdoDupTree (AVIS_SHAPE (oldavis));
    AVIS_MINVAL (avis) = ID_AVIS (b1);
    AVIS_MAXVAL (avis) = ID_AVIS (b2);
    nid = TBmakeId (avis);
    DBUG_PRINT ("IVEXI", ("IVEXItmpIds introduced temp index variable: %s for: %s",
                          AVIS_NAME (avis), AVIS_NAME (oldavis)));
    DBUG_RETURN (nid);
}

/** <!--********************************************************************-->
 *
 * @fn static void populateLUTVars( node *arg_node, info *arg_info)
 *
 * @brief:
 *    1. populate INFO_LUTVARS with WITHID names and new temp names.
 *    2. Call IVEXItmpVec to create temp names and the assigns
 *       that will start off the updated code blocks.
 *
 * @params:
 *     arg_node: An N_part of the WL.
 *     arg_info: Your basic arg_info stuff.
 *
 * @return:
 *
 *****************************************************************************/
static void
populateLUTVars (node *arg_node, info *arg_info)
{
    node *oldavis;
    node *newid;
    node *ids;
    int k = 0;

    DBUG_ENTER ("populateLUTVars");

    /* Populate LUTVARS with WITHID_VEC and new name. */
    oldavis = IDS_AVIS (WITHID_VEC (PART_WITHID (arg_node)));
    newid = IVEXItmpVec (arg_node, arg_info, oldavis);
    LUTinsertIntoLutP (INFO_LUTVARS (arg_info), oldavis, ID_AVIS (newid));
    DBUG_PRINT ("IVEXI", ("Inserting WITHID_VEC into lut: oldid: %s, newid: %s",
                          AVIS_NAME (oldavis), AVIS_NAME (ID_AVIS (newid))));

    /* Rename withid scalars */
    ids = WITHID_IDS (PART_WITHID (arg_node));
    while (ids != NULL) {
        oldavis = IDS_AVIS (ids);
        newid = IVEXItmpIds (arg_node, arg_info, oldavis, k);
        DBUG_PRINT ("IVEXIpart", ("Inserting WITHID_IDS into lut: oldid: %s, newid: %s",
                                  AVIS_NAME (oldavis), AVIS_NAME (ID_AVIS (newid))));
        LUTinsertIntoLutP (INFO_LUTVARS (arg_info), oldavis, ID_AVIS (newid));
        ids = IDS_NEXT (ids);
        k++;
    }
    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *IVEXImodule( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
IVEXImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXImodule");
    DBUG_PRINT ("IVEXI", ("Found module"));
    if (NULL != MODULE_FUNS (arg_node)) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIfundef( node *arg_node, info *arg_info)
 *
 * description:
 *   Create two LUTs:
 *     LUTVARS holds information for renaming WITHID references
 *             to temp names within each WITH_CODE block.
 *     LUTCODES holds the old and new WITH_CODE block pointers,
 *             to let IVEXIpart correct the N_code pointers in
 *             each partition.
 *
 ******************************************************************************/
node *
IVEXIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIfundef");
    /* FIXME */ CHKdoTreeCheck (arg_node);
    DBUG_PRINT ("IVEXI", ("IVEXI in %s %s begins",
                          (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                          FUNDEF_NAME (arg_node)));

    INFO_FUNDEF (arg_info) = arg_node;
    INFO_LUTVARS (arg_info) = LUTgenerateLut ();
    INFO_LUTCODES (arg_info) = LUTgenerateLut ();

    if (NULL != FUNDEF_BODY (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }
    INFO_FUNDEF (arg_info) = NULL;
    INFO_LUTVARS (arg_info) = LUTremoveLut (INFO_LUTVARS (arg_info));
    INFO_LUTCODES (arg_info) = LUTremoveLut (INFO_LUTCODES (arg_info));

    /* If new vardecs were made, append them to the current set */
    if (INFO_VARDECS (arg_info) != NULL) {
        BLOCK_VARDEC (FUNDEF_BODY (arg_node))
          = TCappendVardec (INFO_VARDECS (arg_info),
                            BLOCK_VARDEC (FUNDEF_BODY (arg_node)));
        INFO_VARDECS (arg_info) = NULL;
    }

    DBUG_PRINT ("IVEX", ("IVEXI in %s %s ends",
                         (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                         FUNDEF_NAME (arg_node)));

    /* FIXME */ CHKdoTreeCheck (arg_node);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    /* FIXME */ CHKdoTreeCheck (arg_node);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIblock( node *arg_node, info *arg_info)
 *
 * description:
 *    We get here from the fundef and from WLs.
 *
 ******************************************************************************/
node *
IVEXIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIblock");

    DBUG_PRINT ("IVEXI", ("Found block"));
    if (NULL != BLOCK_INSTR (arg_node)) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIcode( node *arg_node, info *arg_info)
 *
 * description:
 *   IVEXIpath has already rebuilt all the WITH_CODE blocks,
 *   but they are not linked there. Rather, they are hanging
 *   as a chain off PART_CODE.
 *
 *   Here, we zero the old WITH_CODE_USED counts, so they disappear
 *   later.
 *
 ******************************************************************************/
node *
IVEXIcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIcode");
    DBUG_PRINT ("IVEXI", ("Found WITH_CODE"));

    CODE_USED (arg_node) = 0;
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIwith( node *arg_node, info *arg_info)
 *
 * description:
 *
 *   Traverse partitions to replace WITHID references by their
 *   shiny, new ones, using LUTVARS. That traversal will
 *   populate LUTCODES with old and new WITH_CODE pointer.
 *
 ******************************************************************************/
node *
IVEXIwith (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("IVEXIwith");
    DBUG_PRINT ("IVEXI", ("Found WL"));

    if (!WITH_ISEXTREMAINSERTED (arg_node)) {

        DBUG_ASSERT (NULL == INFO_CODE (arg_info), "IVEXIwith got non-NULL INFO_CODE");
        /* Traverse the partitions, to define new temps. */
        /* FIXME */ CHKdoTreeCheck (arg_node);
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        /* FIXME */ CHKdoTreeCheck (arg_node);
        WITH_ISEXTREMAINSERTED (arg_node) = TRUE;

        WITH_CODE (arg_node) = FREEdoFreeTree (WITH_CODE (arg_node));
        /* First partition is head of free */
        WITH_CODE (arg_node) = INFO_CODE (arg_info);
        INFO_CODE (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIlet( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
IVEXIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIlet");
    DBUG_PRINT ("IVEXI", ("Found let"));
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIassign( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
IVEXIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIassign");

    DBUG_PRINT ("IVEXI", ("Found assign"));

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    if (NULL != INFO_PREASSIGNSWITH (arg_info)) {
        arg_node = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), arg_node);
        INFO_PREASSIGNSWITH (arg_info) = NULL;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIpart( node *arg_node, info *arg_info)
 *
 * description:
 *   Create temp vardecs and assigns.
 *   Rename code block references to WITHIDs.
 *
 *   Set AVIS_MINVAL, AVIS_MAXVAL from the partition
 *   generator bounds.
 *
 *   We fix all the code blocks here, and chain
 *   together for IVEXIwith.
 *
 *   When this phase completes, all sharing of WITH_CODE blocks is lost.
 *
 ******************************************************************************/
node *
IVEXIpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXIpart");
    DBUG_PRINT ("IVEXI", ("Found WL partition"));

    /* Don't try this on empty code blocks, kids. */
    if (N_empty != NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (PART_CODE (arg_node))))) {

        populateLUTVars (arg_node, arg_info);

        /* copy the code block, renaming WITHIDs refs to temp names. */
        PART_CODE (arg_node)
          = DUPdoDupTreeLutSsa (PART_CODE (arg_node), INFO_LUTVARS (arg_info),
                                INFO_FUNDEF (arg_info));
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = DUPdoDupTreeLut (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)),
                             INFO_LUTVARS (arg_info));

        LUTremoveContentLut (INFO_LUTVARS (arg_info));
        if (NULL != INFO_PREASSIGNSPART (arg_info)) {
            BLOCK_INSTR (CODE_CBLOCK (PART_CODE (arg_node)))
              = TCappendAssign (INFO_PREASSIGNSPART (arg_info),
                                BLOCK_INSTR (CODE_CBLOCK (PART_CODE (arg_node))));
            INFO_PREASSIGNSPART (arg_info) = NULL;
        }
    } else {
        PART_CODE (arg_node) = DUPdoDupTree (PART_CODE (arg_node));
    }

    CODE_USED (PART_CODE (arg_node)) = 1;
    /* Chain previous code to this new one */
    /* This chain will eventually hang from WITH_CODE */
    CODE_NEXT (PART_CODE (arg_node)) = INFO_CODE (arg_info);
    INFO_CODE (arg_info) = PART_CODE (arg_node);

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
