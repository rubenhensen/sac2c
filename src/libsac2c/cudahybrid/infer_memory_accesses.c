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

typedef struct {
    int add;
} add_access_t;

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool in_wl;
    bool in_kernel;
    lut_t *lut;
    lut_t *part_lut;
    lut_t *add_lut;
    node *ids_avis;
    node *offset_avis;
};

/*
 * INFO_INWL          Flag indicating whether the code currently being
 *                    traversed is in a N_withs
 *
 * INFO_INKERNEL      Flag indicating whether the code currently being
 *                    traversed is in a cuda kernel N_fundef
 *
 * INFO_LUT           Lookup table storing pairs of Avis->{min,max}
 *                    were min and max are the minimum and maximum offsets
 *                    being accessed in a with-loop relative to the index vector
 *
 * INFO_PARTLUT       Same as above, but the recorded data is relative to one
 *                    with-loop partition.
 *
 * INFO_ADDLUT        Lookup table storing pairs of Avis->offset
 *                    were offset is relative to the index vector.
 *
 * INFO_IDSAVIS       The N_avis of the LHS of the current let
 *
 * INFO_OFFSETAVIS    The N_avis of the offset variable of a with-loop
 *
 */

#define INFO_INWL(n) (n->in_wl)
#define INFO_INKERNEL(n) (n->in_kernel)
#define INFO_LUT(n) (n->lut)
#define INFO_ADDLUT(n) (n->add_lut)
#define INFO_IDSAVIS(n) (n->ids_avis)
#define INFO_OFFSETAVIS(n) (n->offset_avis)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_INWL (result) = FALSE;
    INFO_INKERNEL (result) = FALSE;
    INFO_LUT (result) = NULL;
    INFO_ADDLUT (result) = NULL;
    INFO_IDSAVIS (result) = NULL;
    INFO_OFFSETAVIS (result) = NULL;

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

static lut_t *updateOffsetsTable (lut_t *table, node *src_avis, node *new_offset_avis,
                                  int val, node *idx_offset_avis);

/** <!--********************************************************************-->
 *
 * @fn static lut_t *updateOffsetsTable( lut_t *table, node* src_avis, int val,
 *                               node* offset_avis)
 *
 * @brief Updates the given LUT's entry for arg_avis with value val. If the
 *        entry does not exist, one is created.
 *
 *****************************************************************************/

static lut_t *
updateOffsetsTable (lut_t *table, node *src_avis, node *new_offset_avis, int val,
                    node *idx_offset_avis)
{
    void **lut_pointer;
    offset_t *offset;

    DBUG_ENTER ();

    lut_pointer = LUTsearchInLutP (table, src_avis);
    if (lut_pointer != NULL) {
        offset = (offset_t *)(*lut_pointer);
        /* offset structure exists, update it */
        offset->min = MATHmin (offset->min, val);
        offset->max = MATHmax (offset->max, val);
        DBUG_PRINT ("updated offset for %s, it is now [%d,%d]", AVIS_NAME (src_avis),
                    offset->min, offset->max);
    } else if (val != 0 || new_offset_avis == idx_offset_avis) {
        /*
         * we only create a new offset structure if the value is not 0 (which
         * is the default for a NLUT) or we are using the with-loop index
         * offset
         */
        offset = MEMmalloc (sizeof (offset_t));
        offset->min = val;
        offset->max = val;
        table = LUTinsertIntoLutP (table, src_avis, offset);
        DBUG_PRINT ("created offset for %s, it is now [%d,%d]", AVIS_NAME (src_avis), val,
                    val);
    } else {
        DBUG_PRINT ("did not update table for %s", AVIS_NAME (src_avis));
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

    DBUG_ASSERT (FUNDEF_ISCUDAGLOBALFUN (arg_node),
                 "IMA traversing non-kernel function!");

    INFO_INKERNEL (arg_info) = TRUE;
    INFO_LUT (arg_info) = LUTgenerateLut ();
    INFO_ADDLUT (arg_info) = LUTgenerateLut ();
    DBUG_PRINT ("Traversing function %s", FUNDEF_NAME (arg_node));

    FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

    FUNDEF_ACCESS (arg_node) = INFO_LUT (arg_info);
    INFO_ADDLUT (arg_info) = LUTremoveLut (INFO_ADDLUT (arg_info));

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
 * @fn node *IMAwith2( node *arg_node, info *arg_info)
 *
 * @brief Traverse only through the partitions.
 *
 *****************************************************************************/
node *
IMAwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT ("Found with");

    if (!INFO_INKERNEL (arg_info) && !INFO_INWL (arg_info)) {
        INFO_INWL (arg_info) = TRUE;
        INFO_LUT (arg_info) = LUTgenerateLut ();
        INFO_ADDLUT (arg_info) = LUTgenerateLut ();

        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
        WITH2_ACCESS (arg_node) = INFO_LUT (arg_info);

        INFO_ADDLUT (arg_info) = LUTremoveLut (INFO_ADDLUT (arg_info));
    } else {
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    }

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
    add_access_t *access;
    int val;
    void *access_p;

    DBUG_ENTER ();

    if (INFO_INWL (arg_info) || INFO_INKERNEL (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_idxs2offset:
            /* This is the let for the with-loop index offset */
            DBUG_PRINT ("Found index offset %s", AVIS_NAME (INFO_IDSAVIS (arg_info)));
            INFO_OFFSETAVIS (arg_info) = INFO_IDSAVIS (arg_info);
            break;
        case F_add_SxS:
            /* If we are adding to the with-loop index offset variable, we record
             how much we are adding. */
            idx_offset_avis = ID_AVIS (PRF_ARG2 (arg_node));
            new_offset_avis = INFO_IDSAVIS (arg_info);
            if (idx_offset_avis == INFO_OFFSETAVIS (arg_info)) {
                val = NUM_VAL (PRF_ARG1 (arg_node));
                DBUG_PRINT ("Found addition to offset of %d, new offset in %s", val,
                            AVIS_NAME (new_offset_avis));
                access = MEMmalloc (sizeof (add_access_t));
                access->add = val;
                INFO_ADDLUT (arg_info)
                  = LUTinsertIntoLutP (INFO_ADDLUT (arg_info), new_offset_avis, access);
            } else {
                DBUG_PRINT ("Found addition without the index offset.");
            }
            break;
        case F_idx_sel:
            /* We are accessing some memory value, get the offset from the index
             offset(!) from the NLUT and associate it with the accessed array using
             the LUT */
            src_avis = ID_AVIS (PRF_ARG2 (arg_node));
            new_offset_avis = ID_AVIS (PRF_ARG1 (arg_node));
            access_p = LUTsearchInLutP (INFO_ADDLUT (arg_info), new_offset_avis);
            idx_offset_avis = INFO_OFFSETAVIS (arg_info);
            if (access_p == NULL) {
                DBUG_PRINT ("Found idxsel %s into %s with unknown offset",
                            AVIS_NAME (new_offset_avis), AVIS_NAME (src_avis));
            } else {
                val = (*(add_access_t **)access_p)->add;
                DBUG_PRINT ("Found idxsel %s into %s with offset %d",
                            AVIS_NAME (new_offset_avis), AVIS_NAME (src_avis), val);
                INFO_LUT (arg_info)
                  = updateOffsetsTable (INFO_LUT (arg_info), src_avis, new_offset_avis,
                                        val, idx_offset_avis);
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
