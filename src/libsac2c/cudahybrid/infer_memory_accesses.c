/** <!--********************************************************************-->
 *
 * @defgroup Infer Memory Accesses
 *
 *           Infers memory access patterns for distributed array accesses and
 *           stores them in a LUT.
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file infer_memory_accesses.c
 *
 * Prefix: IMA
 *
 *****************************************************************************/
#include "infer_memory_accesses.h"

/*
 * Other includes go here
 */
#include <stdlib.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "IMA"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "LookUpTable.h"
#include "type_utils.h"
#include "math_utils.h"
#include "shape.h"
#include "new_types.h"

typedef struct {
    int add;
    bool constant;
} add_access_t;

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool in_wl;
    bool nested_wl;
    lut_t *lut;
    lut_t *part_lut;
    lut_t *add_lut;
    node *ids_avis;
    lut_t *offset_avis_lut;
};

/*
 * INFO_INWL          Flag indicating whether the code currently being
 *                    traversed is in a with loop
 *
 * INFO_NESTEDWL      Flag indicating whether the code currently being
 *                    traversed is in a nested with loop
 *
 * INFO_LUT           Lookup table storing pairs of Name->{min,max}
 *                    were min and max are the minimum and maximum offsets
 *                    being accessed in a with-loop relative to the index vector
 *                    We store arrays by name because the avises here may be
 *                    different from those available on IMEMDIST (traversing
 *                    kernel functions for example)
 *
 * INFO_PARTLUT       Same as above, but the recorded data is relative to one
 *                    with-loop partition.
 *
 * INFO_ADDLUT        Lookup table storing pairs of Avis->offset
 *                    were offset is relative to the index vector.
 *
 * INFO_IDSAVIS       The N_avis of the LHS of the current let
 *
 * INFO_OFFSETAVISLUT A LUT storing the N_avis of each offset variable of a
 *                    with-loop
 *
 */

#define INFO_INWL(n) (n->in_wl)
#define INFO_NESTEDWL(n) (n->nested_wl)
#define INFO_LUT(n) (n->lut)
#define INFO_ADDLUT(n) (n->add_lut)
#define INFO_IDSAVIS(n) (n->ids_avis)
#define INFO_OFFSETAVISLUT(n) (n->offset_avis_lut)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_INWL (result) = FALSE;
    INFO_NESTEDWL (result) = FALSE;
    INFO_LUT (result) = NULL;
    INFO_ADDLUT (result) = NULL;
    INFO_IDSAVIS (result) = NULL;
    INFO_OFFSETAVISLUT (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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
 * @fn node *IMAdoInferMemoryAccesses( node *syntax_tree)
 *
 *****************************************************************************/
node *
IMAdoInferMemoryAccesses (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_ima);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

static lut_t *updateOffsetsTable (lut_t *table, node *src_avis, int val, bool own,
                                  bool inferred);

static lut_t *updateAddTable (lut_t *table, node *idx_offset_avis, int val,
                              bool constant);

/** <!--********************************************************************-->
 *
 * @fn static lut_t *updateOffsetsTable( lut_t *table, node* src_avis, int val,
 *                                       bool own, bool inferred)
 *
 * @brief Updates the given LUT's entry for src_avis with value val. If the
 *        entry does not exist, one is created.
 *
 *****************************************************************************/

static lut_t *
updateOffsetsTable (lut_t *table, node *src_avis, int val, bool own, bool inferred)
{
    void **lut_pointer;
    offset_t *offset;
    char *avis_name;
    int *extents, block_dim, block_size, block_offset, i;
    shape *src_shape;

    DBUG_ENTER ();

    /* calculate block size */
    src_shape = TYgetShape (AVIS_TYPE (src_avis));
    block_dim = SHgetDim (src_shape) - 1;
    extents = SHshape2IntVec (src_shape);
    block_size = 1;
    for (i = 0; i < block_dim; i++) {
        block_size *= extents[i];
    }

    /* calculate index */
    /* TODO: here we assume index offset is at start of a block.
     * This probably needs to be smarter */
    if (val >= 0) {
        block_offset = val / extents[block_dim];
    } else {
        block_offset = (val + 1) / extents[block_dim] - 1;
    }
    MEMfree (extents);

    /* search for avis in LUT, to update it */
    avis_name = AVIS_NAME (src_avis);
    lut_pointer = LUTsearchInLutS (table, avis_name);
    if (lut_pointer != NULL) {
        offset = (offset_t *)(*lut_pointer);
        /* offset structure exists, update it */
        offset->min = MATHmin (offset->min, block_offset);
        offset->max = MATHmax (offset->max, block_offset);
        offset->inferred = inferred && offset->inferred;
        DBUG_PRINT ("updated offset for %s, it is now [%d,%d]", avis_name, offset->min,
                    offset->max);
    } else {
        /*
         * offset structure not found, create one
         */
        offset = MEMmalloc (sizeof (offset_t));
        offset->min = block_offset;
        offset->max = block_offset;
        offset->own = own;
        offset->inferred = inferred;
        table = LUTinsertIntoLutS (table, avis_name, offset);
        DBUG_PRINT ("created offset for %s, it is now [%d,%d]", avis_name, block_offset,
                    block_offset);
    }

    DBUG_RETURN (table);
}

/** <!--********************************************************************-->
 *
 * @fn static lut_t *updateAddTable ( lut_t *table, node *idx_offset_avis,
 *                                    int val, bool constant)
 *
 * @brief Updates the given LUT's entry for arg_avis with value val. If the
 *        entry does not exist, one is created.
 *
 *****************************************************************************/

static lut_t *
updateAddTable (lut_t *table, node *idx_offset_avis, int val, bool constant)
{
    void **lut_pointer;
    add_access_t *access;

    DBUG_ENTER ();

    lut_pointer = LUTsearchInLutP (table, idx_offset_avis);
    if (lut_pointer == NULL) {
        /*
         * we only create a new offset structure if we don't have one yet
         */
        access = MEMmalloc (sizeof (add_access_t));
        access->add = val;
        access->constant = constant;
        table = LUTinsertIntoLutP (table, idx_offset_avis, access);
        DBUG_PRINT ("created add access for %s, it is now %d",
                    AVIS_NAME (idx_offset_avis), val);
    } else {
        DBUG_PRINT ("offset for %s already present", AVIS_NAME (idx_offset_avis));
    }

    DBUG_RETURN (table);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *IMAfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IMAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Traversing function %s", FUNDEF_NAME (arg_node));

    FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMAlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IMAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (LET_IDS (arg_node) != NULL) {
        INFO_IDSAVIS (arg_info) = IDS_AVIS (LET_IDS (arg_node));

        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

        INFO_IDSAVIS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMAwith( node *arg_node, info *arg_info)
 *
 * @brief Create table if not nested with-loop. Then traverse through the
 * operations and code.
 *
 *****************************************************************************/
node *
IMAwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT ("Found with");

    if (!INFO_INWL (arg_info)) {
        INFO_INWL (arg_info) = TRUE;
        INFO_LUT (arg_info) = LUTgenerateLut ();
        INFO_ADDLUT (arg_info) = LUTgenerateLut ();
        INFO_OFFSETAVISLUT (arg_info) = LUTgenerateLut ();

        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        WITH_ACCESS (arg_node) = INFO_LUT (arg_info);

        INFO_ADDLUT (arg_info) = LUTremoveLut (INFO_ADDLUT (arg_info));
        INFO_OFFSETAVISLUT (arg_info) = LUTremoveLut (INFO_OFFSETAVISLUT (arg_info));
    } else {
        INFO_NESTEDWL (arg_info) = TRUE;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_NESTEDWL (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMAwith2( node *arg_node, info *arg_info)
 *
 * @brief Create table if not nested with-loop. Then traverse through the
 * operations and code.
 *
 *****************************************************************************/
node *
IMAwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT ("Found with2");

    if (!INFO_INWL (arg_info)) {
        INFO_INWL (arg_info) = TRUE;
        INFO_LUT (arg_info) = LUTgenerateLut ();
        INFO_ADDLUT (arg_info) = LUTgenerateLut ();
        INFO_OFFSETAVISLUT (arg_info) = LUTgenerateLut ();

        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        WITH2_ACCESS (arg_node) = INFO_LUT (arg_info);

        INFO_ADDLUT (arg_info) = LUTremoveLut (INFO_ADDLUT (arg_info));
        INFO_OFFSETAVISLUT (arg_info) = LUTremoveLut (INFO_OFFSETAVISLUT (arg_info));
    } else {
        INFO_NESTEDWL (arg_info) = TRUE;
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        INFO_NESTEDWL (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMAgenarray( node *arg_node, info *arg_info)
 *
 * @brief Save access with no offset for the idx and offset of 0 for memory
 *        allocation.
 *
 *****************************************************************************/
node *
IMAgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT ("Found genarray");

    INFO_OFFSETAVISLUT (arg_info)
      = LUTinsertIntoLutP (INFO_OFFSETAVISLUT (arg_info), GENARRAY_IDX (arg_node), NULL);
    INFO_ADDLUT (arg_info)
      = updateAddTable (INFO_ADDLUT (arg_info), GENARRAY_IDX (arg_node), 0, TRUE);
    INFO_LUT (arg_info)
      = updateOffsetsTable (INFO_LUT (arg_info), ID_AVIS (GENARRAY_MEM (arg_node)), 0,
                            TRUE, TRUE);
    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMAmodarray( node *arg_node, info *arg_info)
 *
 * @brief Save access with no offset for the idx and offset of 0 for memory
 *        allocation.
 *
 *****************************************************************************/
node *
IMAmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT ("Found modarray");

    INFO_OFFSETAVISLUT (arg_info)
      = LUTinsertIntoLutP (INFO_OFFSETAVISLUT (arg_info), MODARRAY_IDX (arg_node), NULL);
    INFO_ADDLUT (arg_info)
      = updateAddTable (INFO_ADDLUT (arg_info), MODARRAY_IDX (arg_node), 0, TRUE);
    INFO_LUT (arg_info)
      = updateOffsetsTable (INFO_LUT (arg_info), ID_AVIS (MODARRAY_MEM (arg_node)), 0,
                            TRUE, TRUE);
    INFO_LUT (arg_info)
      = updateOffsetsTable (INFO_LUT (arg_info), ID_AVIS (MODARRAY_ARRAY (arg_node)), 0,
                            FALSE, TRUE);
    MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMAprf( node *arg_node, info *arg_info)
 *
 * @brief Skip return statements
 *
 *****************************************************************************/
node *
IMAprf (node *arg_node, info *arg_info)
{
    node *new_offset_avis, *idx_offset_avis, *src_avis;
    int val;
    void *access_p;
    bool inferred;

    DBUG_ENTER ();

    if (INFO_INWL (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_idxs2offset:
            // skip nested with-loops
            if (!INFO_NESTEDWL (arg_info)) {
                /* This is the let for the with-loop index offset */
                DBUG_PRINT ("Found index offset %s", AVIS_NAME (INFO_IDSAVIS (arg_info)));
                INFO_OFFSETAVISLUT (arg_info)
                  = LUTinsertIntoLutP (INFO_OFFSETAVISLUT (arg_info),
                                       INFO_IDSAVIS (arg_info), NULL);
                updateAddTable (INFO_ADDLUT (arg_info), INFO_IDSAVIS (arg_info), 0, TRUE);
            }
            break;
        case F_add_SxS:
            /* If we are adding to the with-loop index offset variable, we record
             how much we are adding. */
            if (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id) {
                idx_offset_avis = ID_AVIS (PRF_ARG2 (arg_node));
                new_offset_avis = INFO_IDSAVIS (arg_info);
                if (LUTsearchInLutP (INFO_OFFSETAVISLUT (arg_info), idx_offset_avis)
                    != NULL) {
                    if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_num) {
                        val = NUM_VAL (PRF_ARG1 (arg_node));
                        DBUG_PRINT ("Found addition to offset of %d, new offset in %s",
                                    val, AVIS_NAME (new_offset_avis));
                        updateAddTable (INFO_ADDLUT (arg_info), new_offset_avis, val,
                                        TRUE);
                    } else {
                        DBUG_PRINT ("Found non-constant addition.");
                        updateAddTable (INFO_ADDLUT (arg_info), new_offset_avis, 0,
                                        FALSE);
                    }
                } else {
                    DBUG_PRINT ("Found addition %s without the index offset.",
                                AVIS_NAME (new_offset_avis));
                }
            } else {
                DBUG_PRINT ("Found addition with constant argument.");
            }
            break;
        case F_idx_sel:
            /* We are accessing some memory value, get the offset from the index
             offset(!) from the LUT and associate it with the accessed array using
             the LUT */
            src_avis = ID_AVIS (PRF_ARG2 (arg_node));
            new_offset_avis = ID_AVIS (PRF_ARG1 (arg_node));
            access_p = LUTsearchInLutP (INFO_ADDLUT (arg_info), new_offset_avis);
            if (access_p == NULL) {
                DBUG_PRINT ("Found idxsel %s into %s with unknown offset",
                            AVIS_NAME (new_offset_avis), AVIS_NAME (src_avis));
                INFO_LUT (arg_info)
                  = updateOffsetsTable (INFO_LUT (arg_info), src_avis, 0, FALSE, FALSE);
            } else {
                val = (*(add_access_t **)access_p)->add;
                inferred = (*(add_access_t **)access_p)->constant;
                DBUG_PRINT ("Found idxsel %s into %s with offset %d",
                            AVIS_NAME (new_offset_avis), AVIS_NAME (src_avis), val);
                INFO_LUT (arg_info) = updateOffsetsTable (INFO_LUT (arg_info), src_avis,
                                                          val, FALSE, inferred);
            }
        default:
            DBUG_PRINT ("Found unknown prf.");
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
