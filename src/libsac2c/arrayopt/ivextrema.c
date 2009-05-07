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

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *vardecs;
    node *preassigns;
    node *code;
    lut_t *lutvars;
    lut_t *lutcodes;
};

/**
 * INFO macros
 */
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
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
    INFO_PREASSIGNS (result) = NULL;
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
 * @fn node *IVEXItmpIV( node *arg_node, info *arg_info)
 *
 * @brief:
 *      Insert a temp, its vardec, and an assign for arg_node.
 *      We start with:
 *
 *           IV
 *
 *      and want:
 *
 *           tmp = IV;
 *
 *      and an appropriate vardec for tmp, of course.
 *
 *      We build an N_id for IV because WITHIDs don't have them,
 *      just N_avis nodes.
 *
 * @params:
 *     arg_node: The N_avis  of the name for whichwe want to build a temp.
 *     arg_info: Your basic arg_info stuff.
 *
 * @return: The N_id of the new temp.
 *
 *****************************************************************************/
node *
IVEXItmpIV (node *arg_node, info *arg_info)
{
    node *avis;
    node *ivid;
    node *nas;
    node *nid;

    DBUG_ENTER ("IVEXItmpIV");

    DBUG_ASSERT (N_avis == NODE_TYPE (arg_node), "IVEXItmpIV expected N_avis");
    ivid = TBmakeId (arg_node);
    avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (arg_node)));
    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));
    nas = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), ivid), NULL);
    INFO_PREASSIGNS (arg_info) = TCappendAssign (INFO_PREASSIGNS (arg_info), nas);
    AVIS_SSAASSIGN (avis) = nas;
    AVIS_DIM (avis) = DUPdoDupTree (AVIS_DIM (arg_node));
    AVIS_SHAPE (avis) = DUPdoDupTree (AVIS_SHAPE (arg_node));
    nid = TBmakeId (avis);
    DBUG_PRINT ("IVEXI", ("IVEXItmpIV introduced temp index variable: %s for: %s",
                          AVIS_NAME (avis), AVIS_NAME (arg_node)));
    DBUG_RETURN (nid);
}

/** <!--********************************************************************-->
 *
 * @fn static void populateLUTVars( node *arg_node, info *arg_info)
 *
 * @brief:
 *    1. populate INFO_LUTVARS with WITHID names and new temp names.
 *    2. Call IVEXItmpIV to create temp names and the assigns
 *       that will start off the updated code blocks.
 *
 * @params:
 *     arg_node: Any N_withid pointer in the WL. (If -ssaiv mode
 *               support is desired, this will have to be called
 *               for all partitions of the WL.)
 *
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

    DBUG_ENTER ("populateLUTVars");

    /* Populate LUTVARS with WITHID_VEC and new name. */
    oldavis = IDS_AVIS (WITHID_VEC (arg_node));
    newid = IVEXItmpIV (oldavis, arg_info);
    LUTinsertIntoLutP (INFO_LUTVARS (arg_info), oldavis, ID_AVIS (newid));
    DBUG_PRINT ("IVEXI", ("Inserting WITHID_VEC into lut: oldid: %s, newid: %s",
                          AVIS_NAME (oldavis), AVIS_NAME (ID_AVIS (newid))));
    /* FIXME
     *
     *             stick max/min into new vars
     * FIXME
     */

    /* Rename withid scalars */
    ids = WITHID_IDS (arg_node);
    while (ids != NULL) {
        oldavis = IDS_AVIS (ids);
        newid = IVEXItmpIV (oldavis, arg_info);
        DBUG_PRINT ("IVEXIpart", ("Inserting WITHID_IDS into lut: oldid: %s, newid: %s",
                                  AVIS_NAME (oldavis), AVIS_NAME (ID_AVIS (newid))));
        LUTinsertIntoLutP (INFO_LUTVARS (arg_info), oldavis, ID_AVIS (newid));
        ids = IDS_NEXT (ids);
    }
    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

#ifdef CRUD
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
#endif // CRUD

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

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

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
 *   We get here from IVEXIwith. Here we insert the temporary
 *   assigns:
 *
 *     tmp = iv;
 *
 *   and also put the old and new N_code pointers into LUT_CODE.
 *   When we're done, a traversal of the N_part nodes will
 *   use the LUT to correct their PART_CODE pointers.
 *
 ******************************************************************************/
node *
IVEXIcode (node *arg_node, info *arg_info)
{
    node *new_arg_node;

    DBUG_ENTER ("IVEXIcode");
    DBUG_PRINT ("IVEXI", ("Found WITH_CODE"));

    /* Save a lot of work if this is an empty code block */
    if (N_empty != NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (arg_node)))) {

        /* Visit the code block, so we can put the preassigns
         * where they belong.
     PART_CODE( arg_node) = TRAVdo( PART_CODE(arg_node), arg_info);
        PART_NEXT( arg_node) = TRAVopt( PART_NEXT(arg_node), arg_info);
         */

        /* copy the code block, doing the renames */
        new_arg_node = DUPdoDupTreeLutSsa (arg_node, INFO_LUTVARS (arg_info),
                                           INFO_FUNDEF (arg_info));
        CODE_USED (new_arg_node) = CODE_USED (arg_node);
        CODE_USED (arg_node) = 0;

        /* Prepend the preassigns. */
        if (INFO_PREASSIGNS (arg_info) != NULL) {
            BLOCK_INSTR (CODE_CBLOCK (new_arg_node))
              = TCappendAssign (INFO_PREASSIGNS (arg_info),
                                BLOCK_INSTR (CODE_CBLOCK (new_arg_node)));
            INFO_PREASSIGNS (arg_info) = NULL;
        }

        /* Insert old and new pointers into LUTCODES so that partition
         * PART_CODE pointers can be rebuilt.
         */
        LUTinsertIntoLutP (INFO_LUTCODES (arg_info), arg_node, new_arg_node);
    } else {
        /* If this partition is empty, we don't want preassigns. */
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIwith( node *arg_node, info *arg_info)
 *
 * description:
 *   Populate the LUTVARS lut with WITHID names and their
 *   new replacement names.
 *
 *   Traverse WITH_CODE to replace WITHID references by their
 *   shiny, new ones, using LUTVARS. That traversal will
 *   populate LUTCODES with old and new WITH_CODE pointer.
 *
 *   Traverse WITH_PART, to correct PART_CODE pointers using
 *   LUTCODES.
 *
 *
 ******************************************************************************/
node *
IVEXIwith (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("IVEXIwith");
    DBUG_PRINT ("IVEXI", ("Found WL"));

    if (!WITH_ISEXTREMAINSERTED (arg_node)) {

        populateLUTVars (PART_WITHID (WITH_PART (arg_node)), arg_info);

        /* Update all code blocks, mapping WITHID refs to new names, via LUTVARS. */
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

        /* Use LUTCODES to update PART_CODE pointers. */
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        WITH_ISEXTREMAINSERTED (arg_node) = TRUE;
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

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXIpart( node *arg_node, info *arg_info)
 *
 * description:
 *   Correct the PART_CODE address.
 *
 *
 ******************************************************************************/
node *
IVEXIpart (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("IVEXIpart");
    DBUG_PRINT ("IVEXI", ("Found WL partition"));

    PART_CODE (arg_node)
      = LUTsearchInLutPp (INFO_LUTCODES (arg_info), PART_CODE (arg_node));

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
