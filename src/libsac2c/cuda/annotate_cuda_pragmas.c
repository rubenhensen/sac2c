/*****************************************************************************
 *
 * @defgroup gpukernel pragma generation
 *
 * The algorithms in cuda/gpukernel_comp_funs map a with loop index space to an
 * index space that fits on the GPU. To do this, it needs to know what mappings
 * have to be executed, in what order, and with what arguments. This
 * information is stored in an AST datastructure, and can be specified using a
 * #pragma gpukernel in the SAC source code. If no such pragma is implemented
 * in the SAC code, a pragma has to be generated. This file is responsible for
 * the pragma generation. Note that pragma's have to be created for each
 * partition.
 *
 * This file uses an info datastructure to pass through all information the
 * algorithm needs. The most important properties are the pragma property, and
 * the dims property:
 *
 *   - Pragma stores the current generated pragma Spap AST node.
 *   - Dims stores the _current_ dimensionality.
 *
 * All helper functions that generate Spap AST nodes for mappings update these
 * two fields in the info datastructure. This means that when a mapping
 * function is called, the dimensionality will be updated in the case of
 * SplitLast and FoldLast2. All generated mappings are stored in the Pragma
 * property, and the old pragma property value is taken as the inner mappings
 * (with the exception of Gen, which has no inner mapping).
 *
 * The file contains multiple sections:
 *
 *   - The info datastructure
 *   - The AST traversal start
 *   - The static helper functions: functions that create the Spap AST nodes
 *     (and sons) for any gpu mapping function
 *   - The strategy helper functions: middleware functions that help the
 *     implemented mapping strategies with some abstract tasks
 *   - The strategies: Multiple sections, each implementing a mapping strategy
 *     and generate a pragma for the current with-loop partition. The strategy
 *     is explained at the start of the section.
 *   - The AST traversal functions, invoking the strategy functions.
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file annotate_cuda_pragmas.c
 *
 * Prefix: ACP
 *
 *****************************************************************************/
#include "annotate_cuda_pragmas.h"

#include <stdlib.h>
#include <new_types.h>
#include <shape.h>
#include <constants.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"
#include "traverse.h"

#define DBUG_PREFIX "ACP"

#include "debug.h"

static char *GEN = "Gen";
static char *GRIDBLOCK = "GridBlock";
static char *SHIFTLB = "ShiftLB";
static char *COMPRESSGRID = "CompressGrid";
static char *PERMUTE = "Permute";
static char *SPLITLAST = "SplitLast";
static char *FOLDLAST2 = "FoldLast2";
static char *PADLAST = "PadLast";

/** <!--********************************************************************-->
 * INFO structure
 *****************************************************************************/
struct INFO {
    // The nodes for the with-loop, the partition, and the generator are stored
    // here, so the strategies can extract info from them.
    node *with;
    node *part;
    node *gen;

    // The currently generated pragma
    node *pragma;
    // The current dimensionality
    size_t dims;
};

#define INFO_WITH(n) n->with
#define INFO_PART(n) n->part
#define INFO_GEN(n) n->gen

#define INFO_PRAGMA(n) n->pragma
#define INFO_DIMS(n) n->dims

/**
 * Constructor for the info datastructure.
 *
 * @return                  The newly generated info datastructure.
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    INFO_WITH (result) = NULL;
    INFO_PART (result) = NULL;
    INFO_GEN (result) = NULL;
    INFO_PRAGMA (result) = NULL;
    INFO_DIMS (result) = 0;

    DBUG_RETURN (result);
}

/**
 * The info datastructure deconstructor. Note that the AST nodes should NOT be
 * freed, as they are only borrowed.
 *
 * @param info              The info datastructure to be freed.
 * @return                  NULL
 */
static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

#define MK_CONST(...) COmakeConstantFromDynamicArguments (T_int, 1, __VA_ARGS__)

/** <!--********************************************************************-->
 * Entry functions
 *****************************************************************************/

/**
 * Start the AST traversal
 *
 * @param syntax_tree
 * @return
 */
node *
ACPdoAnnotateCUDAPragmas (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_acp);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/**
 * Compute the dimensionality of the current partition. It uses the lowerbound
 * information inside the Generator AST node.
 *
 * @param arg_info          The info datastructure with the Generator node set
 * @return                  The dimensionality of the generator AST node
 */
info *
ACPcomputeDimensionality (info *arg_info)
{
    DBUG_ENTER ();

    INFO_DIMS (arg_info)
            = (size_t)SHgetExtent (ARRAY_FRAMESHAPE (GENERATOR_BOUND1 (INFO_GEN (arg_info))),
                                   0);

    DBUG_RETURN (arg_info);
}

/**
 * Make a SPAP AST node for a certain GPU mapping function
 *
 * @param arg_info          The info datastructure, where the SPAP node should
 *                          be added.
 * @param staticName        The name string of the mapping function. The name
 *                          should be a pointer to a static string. The string
 *                          will be copied before it is added to the AST node.
 * @param nargs             The number of arguments this mapping has. This does
 *                          NOT include the inner mapping function.
 * @param args              An array of AST nodes, containing the argument
 *                          values.
 * @return                  The updated info node, where the SPAP node has been
 *                          inserted.
 */
info *
ACPmakeSpap (info *arg_info, char *staticName, size_t nargs, node **args)
{
    DBUG_ENTER ();

    node *exprs = TBmakeExprs (INFO_PRAGMA (arg_info), NULL);
    for (size_t i = nargs - 1; i != (size_t)-1; i--)
        exprs = TBmakeExprs (args[i], exprs);

    INFO_PRAGMA (arg_info) = TBmakeSpap (TBmakeSpid (NULL, STRcpy (staticName)), exprs);

    DBUG_RETURN (arg_info);
}

/**
 * Create a Gen mapping and store it in the info node.
 *
 * @param arg_info          The info datastructure to be updated
 * @return                  The updated info datastructure
 */
info *
ACPmakeGen (info *arg_info)
{
    DBUG_ENTER ();

    INFO_PRAGMA (arg_info) = TBmakeSpap (TBmakeSpid (NULL, STRcpy (GEN)), NULL);

    DBUG_RETURN (arg_info);
}

/**
 * Create a GridBlock mapping and store it in the info node.
 *
 * @param block_dims        The number of block dimensions. This value is
 *                          asserted to be at least 0, and at most the current
 *                          dimensionality stored in the info datastructure.
 * @param inner             The info datastructure to be updated
 * @return                  The updated info datastructure
 */
info *
ACPmakeGridBlock (int block_dims, info *inner)
{
    DBUG_ENTER ();
    DBUG_ASSERT (0 <= block_dims && block_dims < (int)INFO_DIMS (inner),
                 "block_dims (%i) should be between 0 and the current dimensionality "
                 "(%zu)",
                 block_dims, INFO_DIMS (inner));

    node *args[1] = {TBmakeNum (block_dims)};
    inner = ACPmakeSpap (inner, GRIDBLOCK, 1, args);

    DBUG_RETURN (inner);
}

/**
 * Create a ShiftLB mapping and store it in the info node.
 *
 * @param inner             The info datastructure to be updated
 * @return                  The updated info datastructure
 */
info *
ACPmakeShiftLB (info *inner)
{
    DBUG_ENTER ();

    inner = ACPmakeSpap (inner, SHIFTLB, 0, NULL);

    DBUG_RETURN (inner);
}

/**
 * Create a CompressGrid mapping and store it in the info node.
 *
 * @param block_dims        A constant that denotes which dimensions should be
 *                          compressed. The shape of the constant should be
 *                          one-dimensional and with length equal to the
 *                          current dimensionality.
 * @param inner             The info datastructure to be updated
 * @return                  The updated info datastructure
 */
info *
ACPmakeCompressGrid (constant *compressDims, info *inner)
{
    DBUG_ENTER ();
    DBUG_ASSERT (COgetDim (compressDims) == 1,
                 "Dimensionality of compressDims array (%i) should be 1",
                 COgetDim (compressDims));
    DBUG_ASSERT (COgetExtent (compressDims, 0) == (int)INFO_DIMS (inner),
                 "Length of compressDims array (%i) should be equal to the "
                 "dimensionality (%zu)",
                 COgetExtent (compressDims, 0), INFO_DIMS (inner));

    node *args[1] = {COconstant2AST (compressDims)};
    inner = ACPmakeSpap (inner, COMPRESSGRID, 1, args);

    DBUG_RETURN (inner);
}

/**
 * Create a CompressGrid mapping and store it in the info node. This is an
 * alternative to the CompressGrid function above that just compresses
 * all dimensions.
 *
 * @param inner             The info datastructure to be updated
 * @return                  The updated info datastructure
 */
info *
ACPmakeCompressAll (info *inner)
{
    DBUG_ENTER ();

    constant *compressDims = COmakeOne (T_int, SHcreateShape (1, (int)INFO_DIMS (inner)));
    inner = ACPmakeCompressGrid (compressDims, inner);

    DBUG_RETURN (inner);
}

/**
 * Create a Permute mapping and store it in the info node.
 *
 * @param permutation       A constant that contains the permutation to be
 *                          executed. The shape of the constant should be
 *                          one-dimensional and with length equal to the
 *                          current dimensionality.
 * @param inner             The info datastructure to be updated
 * @return                  The updated info datastructure
 */
info *
ACPmakePermute (constant *permutation, info *inner)
{
    DBUG_ENTER ();
    DBUG_ASSERT (COgetDim (permutation) == 1,
                 "Dimensionality of permutation array (%i) should be 1",
                 COgetDim (permutation));
    DBUG_ASSERT (COgetExtent (permutation, 0) == (int)INFO_DIMS (inner),
                 "Length of permutation array (%i) should be equal to the dimensionality "
                 "(%zu)",
                 COgetExtent (permutation, 0), INFO_DIMS (inner));

    node *args[2] = {COconstant2AST (permutation)};
    inner = ACPmakeSpap (inner, PERMUTE, 1, args);

    DBUG_RETURN (inner);
}

/**
 * Create a SplitLast mapping and store it in the info node.
 *
 * @param innersize         The size of the inner dimension to be created
 * @param inner             The info datastructure to be updated
 * @return                  The updated info datastructure
 */
info *
ACPmakeSplitLast (int innersize, info *inner)
{
    DBUG_ENTER ();
    DBUG_ASSERT (INFO_DIMS (inner) >= 1, "Dimensionality (%zu) should be at least 1",
                 INFO_DIMS (inner));

    node *args[1] = {TBmakeNum (innersize)};
    inner = ACPmakeSpap (inner, SPLITLAST, 1, args);
    INFO_DIMS (inner)++;

    DBUG_RETURN (inner);
}

/**
 * Create a FoldLast2 mapping and store it in the info node.
 *
 * @param inner             The info datastructure to be updated
 * @return                  The updated info datastructure
 */
info *
ACPmakeFoldLast2 (info *inner)
{
    DBUG_ENTER ();
    DBUG_ASSERT (INFO_DIMS (inner) >= 2, "Dimensionality (%zu) should be at least 2",
                 INFO_DIMS (inner));

    inner = ACPmakeSpap (inner, FOLDLAST2, 0, NULL);
    INFO_DIMS (inner)--;

    DBUG_RETURN (inner);
}

/**
 * Create a PadLast mapping and store it in the info node.
 *
 * @param paddedsize        A divisor of the size of the innermost dimension
 *                          after padding. The innermost dimension is padded
 *                          to a multiple of this size.
 * @param inner             The info datastructure to be updated
 * @return                  The updated info datastructure
 */
info *
ACPmakePadLast (int paddedsize, info *inner)
{
    DBUG_ENTER ();
    DBUG_ASSERT (INFO_DIMS (inner) >= 1, "Dimensionality (%zu) should be at least 1",
                 INFO_DIMS (inner));

    node *args[1] = {TBmakeNum (paddedsize)};
    inner = ACPmakeSpap (inner, PADLAST, 1, args);

    DBUG_RETURN (inner);
}

/** <!--********************************************************************-->
 * @}  <!-- Strategy helper functions -->
 *****************************************************************************/

/**
 * Create a permutation that rotates the dimensions with a given offset to the
 * right. A negative offset can be used to rotate left, but it should be bigger
 * then (-current dimensionality).
 *
 * @param offset            The offset with which should be rotated.
 * @param inner             The info datastructure to be updated
 * @return                  The updated info datastructure
 */
info *
ACPpermuteRotate (int offset, info *inner)
{
    DBUG_ENTER ();

    int dims = (int)INFO_DIMS (inner);

    // Note: ownership of permutation is transfered to the constant
    int *permutation = (int *)MEMmalloc (sizeof (int) * (size_t)dims);
    for (int i = 0; i < dims; i++)
        permutation[i] = (i - offset + dims) % dims;

    constant *permConst = COmakeConstant (T_int, SHcreateShape (1, dims), permutation);
    inner = ACPmakePermute (permConst, inner);

    DBUG_RETURN (inner);
}

/**
 * Reduce the current dimensionality by half (rounded up), by folding each pair
 * of dimensions together. If the number of dimensions is uneven, the innermost
 * dimension remains untouched.
 *
 * @param inner             The info datastructure to be updated
 * @return                  The updated info datastructure
 */
info *
ACPreduceDimensionality (info *inner)
{
    DBUG_ENTER ();

    if (INFO_DIMS (inner) % 2 == 1)
        ACPpermuteRotate (1, inner);

    size_t nr_folds = INFO_DIMS (inner) / 2;
    for (size_t i = 0; i < nr_folds; i++) {
        inner = ACPmakeFoldLast2 (inner);
        inner = ACPpermuteRotate (1, inner);
    }

    DBUG_RETURN (inner);
}

/** <!--********************************************************************-->
 * @}  <!-- Jings method Strategy -->
 *
 * This strategy resembles Jing's original method. If the boolean ext is set
 * to true, the dimensionality is reduced to 3-5 until Jings method can be
 * applied. Note that for 3, 4 or 5 dimensions, the innermost 2 dimensions can be at
 *most 32.
 *
 * Jing's method handles 1-5 dimensions:
 *  note: read the dimension specs as
 *        (thread dimensions z-x | block dimensions z-x)
 *  note: read the constant variables as constants defined in global.config.
 *        The currently used values are for the sm_61 architecture.
 *   - 1: (1/32 | 32)
 *   - 2: (2/32, 1/32 | 32, 32)
 *   - 3: (3 | 2, 1)
 *   - 4: (4, 3 | 2, 1)
 *   - 5: (5, 4, 3 | 2, 1)
 *****************************************************************************/

/**
 * Generate a pragma AST structure for Jing's method.
 *
 * @param ext               A boolean to toggle extended mode
 * @param inner             The info datastructure to be updated
 * @return                  The updated info datastructure
 */
info *
ACPjingGeneratePragma (bool ext, info *inner)
{
    DBUG_ENTER ();

    switch (INFO_DIMS (inner)) {
        case 0:
            inner = ACPmakeGridBlock (0, inner);
            break;
        case 1:
            inner = ACPmakeSplitLast (global.config.cuda_1d_block_x, inner);
            inner = ACPmakeGridBlock (1, inner);
            break;
        case 2:
            inner = ACPmakeSplitLast (global.config.cuda_2d_block_x, inner);
            inner = ACPmakePermute (MK_CONST (3, 1, 2, 0), inner);
            inner = ACPmakeSplitLast (global.config.cuda_2d_block_y, inner);
            inner = ACPmakePermute (MK_CONST (4, 2, 0, 3, 1), inner);
            inner = ACPmakeGridBlock (2, inner);
            break;
        case 3:
        case 4:
        case 5:
            inner = ACPmakeGridBlock ((int)INFO_DIMS (inner) - 2, inner);
            break;
        default:
            if (ext) {
                // If the dimensionality is too high and extended mode is enabled,
                // reduce the dimensionality and try again.
                inner = ACPreduceDimensionality (inner);
                inner = ACPjingGeneratePragma (ext, inner);
            } else
                // If the dimensionality is too high and extended mode is disabled, throw an
                // error.
                CTIerrorLoc (NODE_LOCATION (INFO_WITH (inner)),
                             "Dimensionality of with loop (%zu) too high for gpu mapping "
                             "strategy \"Jing's method\""
                             "(-gpu_mapping_strategy jings_method) can be at most 5. For "
                             "higher dimensionalities, "
                             "use the extended Jing's method (-gpu_mapping_strategy "
                             "jings_method_ext).",
                             INFO_DIMS (inner));
            break;
    }

    DBUG_RETURN (inner);
}

/** <!--********************************************************************-->
 * @}  <!-- Foldall Strategy -->
 *
 * This strategy first reduces the index space to 1 dimension, and then splits
 * it again using a fixed dimension strategy. This method uses the
 * dimensionality as a heuristic for how big the index space is. A better
 * heuristic is required in future versions.
 *
 * The Foldall strategy handles 1-5 dimensions:
 *  note: read the dimension specs as
 *        (thread dimensions z-x | block dimensions z-x)
 *  note: read the constant variables as constants defined in global.config.
 *        The currently used values are for the sm_61 architecture.
 *   - 1: (1/32 | 32)
 *   - 2: (1/1024 | 32, 32)
 *   - >2: (2048, 1/2048/1024 | 32, 32)
 *****************************************************************************/

/**
 * Generate a pragma AST structure for the Foldall strategy.
 *
 * @param inner             The info datastructure to be updated
 * @return                  The updated info datastructure
 */
info *
ACPfoldallGeneratePragma (info *inner)
{
    DBUG_ENTER ();

    // The dims variable changes when we fold the dimensions together, so we have to back
    // it up
    size_t dims = INFO_DIMS (inner);

    while (INFO_DIMS (inner) > 1)
        inner = ACPmakeFoldLast2 (inner);

    // Split first dimension to (1/32 | 32)
    inner = ACPmakeSplitLast (global.config.cuda_2d_block_x, inner);

    // Split second dimension to (1/1024 | 32, 32)
    if (dims >= 2) {
        inner = ACPmakePermute (MK_CONST (2, 1, 0), inner);
        inner = ACPmakeSplitLast (global.config.cuda_2d_block_y, inner);
        inner = ACPmakePermute (MK_CONST (3, 1, 2, 0), inner);
    }

    // Split third dimension to (2048, 1/2048/1024 | 32, 32)
    if (dims >= 3) {
        inner = ACPmakePermute (MK_CONST (3, 1, 2, 0), inner);
        inner = ACPmakeSplitLast (global.config.cuda_3d_thread_y, inner);
        inner = ACPmakePermute (MK_CONST (4, 3, 2, 0, 1), inner);
    }

    inner = ACPmakeGridBlock (dims == 1 ? 1 : 2, inner);

    DBUG_RETURN (inner);
}

/** <!--********************************************************************-->
 * @}  <!-- Strategy choosing -->
 *****************************************************************************/

/**
 * Choose the correct strategy from global.gpu_mapping_strategy
 *
 * @param inner             The info datastructure to be updated
 * @return                  The updated info datastructure
 */
info *
ACPgeneratePragma (info *arg_info)
{
    DBUG_ENTER ();

    arg_info = ACPmakeShiftLB (ACPmakeGen (arg_info));
    if (global.gpu_mapping_compress)
        arg_info = ACPmakeCompressAll (arg_info);

    switch (global.gpu_mapping_strategy) {
        case Jings_method:
            arg_info = ACPjingGeneratePragma (FALSE, arg_info);
            break;
        case Jings_method_ext:
            arg_info = ACPjingGeneratePragma (TRUE, arg_info);
            break;
        case Foldall:
            arg_info = ACPfoldallGeneratePragma (arg_info);
            break;
        default:
            DBUG_UNREACHABLE ("Invalid cuda mapping strategy");
    }

    DBUG_RETURN (arg_info);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *ACPpart (node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACPwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_WITH (arg_info) = arg_node;
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    } else {
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACPpart (node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACPpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PART_PRAGMA (arg_node) == NULL) {
        INFO_PART (arg_info) = arg_node;
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
        DBUG_ASSERT (INFO_PRAGMA (arg_info) != NULL, "failed to generate pragma "
                                                     "for partition");
        PART_PRAGMA (arg_node) = INFO_PRAGMA (arg_info);
        INFO_PRAGMA (arg_info) = NULL;
    }

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* ACPgenerator (node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACPgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_GEN (arg_info) = arg_node;
    arg_info = ACPcomputeDimensionality (arg_info);

    arg_info = ACPgeneratePragma (arg_info);
    node *pragma = TBmakePragma ();
    PRAGMA_GPUKERNEL_APS (pragma) = INFO_PRAGMA (arg_info);
    INFO_PRAGMA (arg_info) = pragma;

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
