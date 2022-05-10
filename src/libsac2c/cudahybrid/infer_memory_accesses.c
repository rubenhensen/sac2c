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
    int constant;
    node *avis;
} add_t;

typedef struct {
    int add;
} idxs2offset_t;

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
    node *withids;
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
 * INFO_WITHIDS       N_exprs chain of withids used in the current code block.
 *
 */

#define INFO_INWL(n) (n->in_wl)
#define INFO_NESTEDWL(n) (n->nested_wl)
#define INFO_LUT(n) (n->lut)
#define INFO_ADDLUT(n) (n->add_lut)
#define INFO_IDSAVIS(n) (n->ids_avis)
#define INFO_OFFSETAVISLUT(n) (n->offset_avis_lut)
#define INFO_WITHIDS(n) (n->withids)

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
    INFO_WITHIDS (result) = NULL;

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

static lut_t *updateAddTable (lut_t *table, node *result_avis, int val, node *operand);

static bool calculateOffsetDim (node *argid, node *withid, int *val,
                                lut_t *addition_table);

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
    int *extents, block_dim, block_offset, i;
    shape *src_shape;

    DBUG_ENTER ();

    /* calculate block size */
    src_shape = TYgetShape (AVIS_TYPE (src_avis));
    block_dim = SHgetDim (src_shape) - 1;
    extents = SHshape2IntVec (src_shape);
#if 0
    block_size = 1;
    for (i = 0; i < block_dim; i++) {
        block_size *= extents[i];
    }
#endif

    /* calculate index */
    if (val == 0) {
        block_offset = 0;
    } else if (val > 0) {
        block_offset = (val - 1) / extents[block_dim] + 1;
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
 * @fn static lut_t *updateAddTable ( lut_t *table, node *result_avis,
 *                                    int val, node *operand)
 *
 * @brief Updates the given LUT's entry for arg_avis with value val. If the
 *        entry does not exist, one is created.
 *
 *****************************************************************************/

static lut_t *
updateAddTable (lut_t *table, node *result_avis, int val, node *operand)
{
    void **lut_pointer;
    add_t *access;

    DBUG_ENTER ();

    lut_pointer = LUTsearchInLutP (table, result_avis);
    if (lut_pointer == NULL) {
        /*
         * we only create a new offset structure if we don't have one yet
         */
        access = MEMmalloc (sizeof (add_t));
        access->constant = val;
        access->avis = operand;
        table = LUTinsertIntoLutP (table, result_avis, access);
        DBUG_PRINT ("created add access for %s, it is now %s + %d",
                    AVIS_NAME (result_avis), AVIS_NAME (operand), val);
    } else {
        DBUG_PRINT ("addition for %s already present", AVIS_NAME (result_avis));
    }

    DBUG_RETURN (table);
}

/** <!--********************************************************************-->
 *
 * @fn static bool calculateOffsetDim(node *prfid, node *withid, int *val,
 *                                    lut_t *addition_table)
 *
 * @brief Calculates how much has been added to withid in prfid, if possible.
 *        Returns whether the operation was successful or not.
 *
 *****************************************************************************/

static bool
calculateOffsetDim (node *argid, node *withid, int *val, lut_t *addition_table)
{
    void **lut_pointer;
    add_t *addition;
    bool res;
    node *arg_avis;
    int acc;

    DBUG_ENTER ();

    acc = 0;
    res = TRUE;
    arg_avis = ID_AVIS (argid);
    while (res == TRUE) {
        if (arg_avis == ID_AVIS (withid)) {
            DBUG_PRINT ("Argument and withid match.");
            *val = acc;
            break;
        } else {
            lut_pointer = LUTsearchInLutP (addition_table, arg_avis);
            if (lut_pointer != NULL) {
                addition = (add_t *)(*lut_pointer);
                DBUG_PRINT ("Found avis in addition table, %s = %s + %d",
                            AVIS_NAME (arg_avis), AVIS_NAME (addition->avis),
                            addition->constant);
                acc += addition->constant;
                arg_avis = addition->avis;
            } else {
                DBUG_PRINT ("Argument not withid and not in addition table, giving up.");
                res = FALSE;
            }
        }
    }

    DBUG_PRINT ("Argument is %s within a constant offset (%d) of a withid.",
                res ? "" : "not", acc);
    DBUG_RETURN (res);
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

        INFO_WITHIDS (arg_info) = WITH_IDS (arg_node);
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

        INFO_WITHIDS (arg_info) = WITH2_IDS (arg_node);
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
    node *operand_avis, *result_avis, *src_avis;
    node *array_exprs, *withid_exprs, *prf_exprs, *array_expr;
    int val, arraysize, dim_size, dim_val;
    void *access_p;
    bool inferred;
    idxs2offset_t *idxs2offset;

    DBUG_ENTER ();

    if (INFO_INWL (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_idxs2offset:
            // skip nested with-loops
            if (!INFO_NESTEDWL (arg_info)) {
                /* This is the let for a indexes to offset instruction */
                DBUG_PRINT ("Found idxs2offset %s", AVIS_NAME (INFO_IDSAVIS (arg_info)));

                val = 0;
                inferred = TRUE;

                /*calculate size of array */
                arraysize = 1;
                array_exprs = ARRAY_AELEMS (PRF_ARG1 (arg_node));
                for (array_exprs = ARRAY_AELEMS (PRF_ARG1 (arg_node));
                     array_exprs != NULL; array_exprs = EXPRS_NEXT (array_exprs)) {
                    array_expr = EXPRS_EXPR (array_exprs);
                    if (NODE_TYPE (array_expr) != N_num) {
                        DBUG_PRINT ("Non AKS array");
                        inferred = FALSE;
                        break;
                    }
                    arraysize *= NUM_VAL (EXPRS_EXPR (array_exprs));
                }
                if (inferred == FALSE)
                    break;
                DBUG_PRINT ("Array size: %d", arraysize);

                // travel each dimension, acumulating how much the offset from iv is
                array_exprs = ARRAY_AELEMS (PRF_ARG1 (arg_node));
                withid_exprs = INFO_WITHIDS (arg_info);
                prf_exprs = EXPRS_NEXT (PRF_ARGS (arg_node));
                dim_size = arraysize;
                while (withid_exprs != NULL) {
                    inferred = calculateOffsetDim (EXPRS_EXPR (prf_exprs),
                                                   EXPRS_EXPR (withid_exprs), &dim_val,
                                                   INFO_ADDLUT (arg_info));
                    if (inferred == FALSE)
                        break;

                    // multiply by other dim size
                    dim_size /= NUM_VAL (EXPRS_EXPR (array_exprs));
                    val += dim_val * dim_size;
                    array_exprs = EXPRS_NEXT (array_exprs);
                    withid_exprs = EXPRS_NEXT (withid_exprs);
                    prf_exprs = EXPRS_NEXT (prf_exprs);
                    DBUG_PRINT ("Dim size: %d", dim_size * dim_size);
                }
                if (inferred == FALSE)
                    break;

                // insert result into table
                idxs2offset = (idxs2offset_t *)malloc (sizeof (idxs2offset_t));
                idxs2offset->add = val;
                INFO_OFFSETAVISLUT (arg_info)
                  = LUTinsertIntoLutP (INFO_OFFSETAVISLUT (arg_info),
                                       INFO_IDSAVIS (arg_info), idxs2offset);
                DBUG_PRINT ("Found idxs2offset %s with offset: %d",
                            AVIS_NAME (INFO_IDSAVIS (arg_info)), val);
            }
            break;
        case F_add_SxS:
            /* Recording constant additions to some ID */
            if (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id) {
                operand_avis = ID_AVIS (PRF_ARG2 (arg_node));
                result_avis = INFO_IDSAVIS (arg_info);
                if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_num) {
                    val = NUM_VAL (PRF_ARG1 (arg_node));
                    DBUG_PRINT ("Found addition to %s by %d, result in %s",
                                AVIS_NAME (operand_avis), val, AVIS_NAME (result_avis));

                    /* If the addition is to a offset, we create a new idxs2offset_t
                     * structure. Else, we record the addition in the addition table.*/
                    access_p
                      = LUTsearchInLutP (INFO_OFFSETAVISLUT (arg_info), operand_avis);
                    if (access_p == NULL) {
                        DBUG_PRINT ("Not a (known) offset, we just record addition.");
                        updateAddTable (INFO_ADDLUT (arg_info), result_avis, val,
                                        operand_avis);
                    } else {
                        idxs2offset = *(idxs2offset_t **)(access_p);
                        val += idxs2offset->add;
                        idxs2offset = (idxs2offset_t *)malloc (sizeof (idxs2offset_t));
                        idxs2offset->add = val;
                        DBUG_PRINT ("Addition to a known offset, we add %d to the "
                                    "offsetavis table.",
                                    val);
                        INFO_OFFSETAVISLUT (arg_info)
                          = LUTinsertIntoLutP (INFO_OFFSETAVISLUT (arg_info),
                                               INFO_IDSAVIS (arg_info), idxs2offset);
                    }
                } else
                    DBUG_PRINT ("Found non-constant addition to %s, result in %s.",
                                AVIS_NAME (operand_avis), AVIS_NAME (result_avis));
            } else {
                DBUG_PRINT ("Found addition with constant argument.");
            }
            break;
        case F_idx_sel:
            /* We are accessing some memory value, get the offset from iv from the
             * LUT and associate it with the accessed array.*/
            src_avis = ID_AVIS (PRF_ARG2 (arg_node));
            operand_avis = ID_AVIS (PRF_ARG1 (arg_node));
            access_p = LUTsearchInLutP (INFO_OFFSETAVISLUT (arg_info), operand_avis);
            if (access_p == NULL) {
                DBUG_PRINT ("Found idxsel %s into %s with unknown offset",
                            AVIS_NAME (operand_avis), AVIS_NAME (src_avis));
                INFO_LUT (arg_info)
                  = updateOffsetsTable (INFO_LUT (arg_info), src_avis, 0, FALSE, FALSE);
            } else {
                idxs2offset = *(idxs2offset_t **)(access_p);
                val = idxs2offset->add;
                DBUG_PRINT ("Found idxsel %s into %s with offset %d",
                            AVIS_NAME (operand_avis), AVIS_NAME (src_avis), val);
                INFO_LUT (arg_info)
                  = updateOffsetsTable (INFO_LUT (arg_info), src_avis, val, FALSE, TRUE);
            }
            break;
        default:
            DBUG_PRINT ("Found other prf.");
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
