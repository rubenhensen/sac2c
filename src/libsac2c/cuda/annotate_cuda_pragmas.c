/*****************************************************************************
 *
 * @defgroup
 *
 * description:
 *
 *   Niek will explain :-)
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

static char* GEN          = "Gen";
static char* GRIDBLOCK    = "GridBlock";
static char* SHIFTLB      = "ShiftLB";
static char* COMPRESSGRID = "CompressGrid";
static char* PERMUTE      = "Permute";
static char* SPLITLAST    = "SplitLast";
static char* FOLDLAST2    = "FoldLast2";
static char* PADLAST      = "PadLast";

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node* with;
    node* part;
    node* gen;

    node* pragma;
    size_t dims;
    size_t dims_before;
};

#define INFO_WITH(n) n->with
#define INFO_PART(n) n->part
#define INFO_GEN(n) n->gen

#define INFO_PRAGMA(n) n->pragma
#define INFO_DIMS(n) n->dims
#define INFO_DIMS_BEFORE(n) n->dims_before

static info*
MakeInfo(void) {
    info* result;

    DBUG_ENTER ();

    result = (info*) MEMmalloc(sizeof(info));
    INFO_WITH (result)   = NULL;
    INFO_PART (result)   = NULL;
    INFO_GEN (result)    = NULL;
    INFO_PRAGMA (result) = NULL;
    INFO_DIMS(result)    = 0;

    DBUG_RETURN (result);
}

static info*
FreeInfo(info* info) {
    DBUG_ENTER ();

    info = MEMfree(info);

    DBUG_RETURN (info);
}

#define MK_CONST(...) COmakeConstantFromDynamicArguments(T_int, 1, __VA_ARGS__)

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
 * @fn node *ACPdoAnnotateCUDAPragmas( node *syntax_tree)
 *
 *****************************************************************************/
node*
ACPdoAnnotateCUDAPragmas(node* syntax_tree) {
    info* info;
    DBUG_ENTER ();

    info = MakeInfo();
    TRAVpush(TR_acp);
    syntax_tree = TRAVdo(syntax_tree, info);
    TRAVpop();
    info = FreeInfo(info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

info*
ACPcomputeDimensionality(info* arg_info) {
    DBUG_ENTER();

    INFO_DIMS(arg_info) = (size_t) SHgetExtent(
            ARRAY_FRAMESHAPE(GENERATOR_BOUND1(INFO_GEN(arg_info))),
            0);

    DBUG_RETURN(arg_info);
}

info*
ACPmakeSpap(info* arg_info, char* staticName, size_t nargs, node** args) {
    DBUG_ENTER();

    node* exprs = TBmakeExprs(INFO_PRAGMA(arg_info), NULL);
    for (size_t i = nargs - 1; i != (size_t) -1; i--)
        exprs = TBmakeExprs(args[i], exprs);

    INFO_PRAGMA(arg_info) = TBmakeSpap(
            TBmakeSpid(NULL, STRcpy(staticName)),
            exprs);

    DBUG_RETURN(arg_info);
}

info*
ACPmakeGen(info* arg_info) {
    DBUG_ENTER();

    INFO_PRAGMA(arg_info) = TBmakeSpap(
            TBmakeSpid(NULL, STRcpy(GEN)),
            NULL);

    DBUG_RETURN(arg_info);
}

info*
ACPmakeGridBlock(int block_dims, info* inner) {
    DBUG_ENTER();
    DBUG_ASSERT(0 <= block_dims && block_dims < (int) INFO_DIMS(inner),
                "block_dims (%i) should be between 0 and the current dimensionality (%zu)",
                block_dims, INFO_DIMS(inner));

    node* args[1] = {TBmakeNum(block_dims)};
    inner = ACPmakeSpap(inner, GRIDBLOCK, 1, args);

    DBUG_RETURN(inner);
}

info*
ACPmakeShiftLB(info* inner) {
    DBUG_ENTER();

    inner = ACPmakeSpap(inner, SHIFTLB, 0, NULL);

    DBUG_RETURN(inner);
}

info*
ACPmakeCompressGrid(constant* compressDims, info* inner) {
    DBUG_ENTER();
    DBUG_ASSERT(COgetExtent(compressDims, 0) == (int) INFO_DIMS(inner),
                "Length of compressDims array (%i) should be equal to the dimensionality (%zu)",
                COgetExtent(compressDims, 0), INFO_DIMS(inner));

    node* args[1] = {COconstant2AST(compressDims)};
    inner = ACPmakeSpap(inner, COMPRESSGRID, 1, args);

    DBUG_RETURN(inner);
}

info*
ACPmakeCompressAll(info* inner) {
    DBUG_ENTER();

    constant* compressDims = COmakeOne(T_int, SHcreateShape(1, (int) INFO_DIMS(inner)));
    inner = ACPmakeCompressGrid(compressDims, inner);

    DBUG_RETURN(inner);
}

info*
ACPmakePermute(constant* permutation, info* inner) {
    DBUG_ENTER();
    DBUG_ASSERT(COgetExtent(permutation, 0) == (int) INFO_DIMS(inner),
                "Length of permutation array (%i) should be equal to the dimensionality (%zu)",
                COgetExtent(permutation, 0), INFO_DIMS(inner));

    node* args[2] = {COconstant2AST(permutation)};
    inner = ACPmakeSpap(inner, PERMUTE, 1, args);

    DBUG_RETURN(inner);
}

info*
ACPmakeSplitLast(int innersize, info* inner) {
    DBUG_ENTER();
    DBUG_ASSERT(INFO_DIMS(inner) >= 1,
                "Dimensionality (%zu) should be at least 1",
                INFO_DIMS(inner));

    node* args[1] = {TBmakeNum(innersize)};
    inner = ACPmakeSpap(inner, SPLITLAST, 1, args);
    INFO_DIMS(inner)++;

    DBUG_RETURN(inner);
}

info*
ACPmakeFoldLast2(info* inner) {
    DBUG_ENTER();
    DBUG_ASSERT(INFO_DIMS(inner) >= 2,
                "Dimensionality (%zu) should be at least 2",
                INFO_DIMS(inner));

    inner = ACPmakeSpap(inner, FOLDLAST2, 0, NULL);
    INFO_DIMS(inner)--;

    DBUG_RETURN(inner);
}

info*
ACPmakePadLast(int paddedsize, info* inner) {
    DBUG_ENTER();
    DBUG_ASSERT(INFO_DIMS(inner) >= 1,
                "Dimensionality (%zu) should be at least 1",
                INFO_DIMS(inner));

    node* args[1] = {TBmakeNum(paddedsize)};
    inner = ACPmakeSpap(inner, PADLAST, 1, args);

    DBUG_RETURN(inner);
}


/** <!--********************************************************************-->
 * @}  <!-- Strategy helper functions -->
 *****************************************************************************/

info*
ACPpermuteRotate(int offset, info* inner) {
    DBUG_ENTER();

    int dims = (int) INFO_DIMS(inner);

    // Note: ownership of permutation is transfered to the constant
    int* permutation = (int*) MEMmalloc(sizeof(int) * (size_t) dims);
    for (int i = 0; i < dims; i++)
        permutation[i] = (i - offset + dims) % dims;

    constant* permConst = COmakeConstant(T_int, SHcreateShape(1, dims), permutation);
    inner = ACPmakePermute(permConst, inner);

    DBUG_RETURN(inner);
}

info*
ACPreduceDimensionality(info* inner) {
    DBUG_ENTER();

    if (INFO_DIMS(inner) % 2 == 1)
        ACPpermuteRotate(1, inner);

    size_t      nr_folds = INFO_DIMS(inner) / 2;
    for (size_t i        = 0; i < nr_folds; i++) {
        inner = ACPmakeFoldLast2(inner);
        inner = ACPpermuteRotate(1, inner);
    }

    DBUG_RETURN(inner);
}

/** <!--********************************************************************-->
 * @}  <!-- Jings method Strategy -->
 *****************************************************************************/

info*
ACPjingGeneratePragma(info* inner, bool ext) {
    DBUG_ENTER();

    switch (INFO_DIMS(inner)) {
        case 0:
            inner = ACPmakeGridBlock(0, inner);
            break;
        case 1:
            inner = ACPmakeSplitLast(global.config.cuda_1d_block_x, inner);
            inner = ACPmakeGridBlock(1, inner);
            break;
        case 2:
            inner = ACPmakeSplitLast(global.config.cuda_2d_block_x, inner);
            inner = ACPmakePermute(MK_CONST(3, 1, 2, 0), inner);
            inner = ACPmakeSplitLast(global.config.cuda_2d_block_y, inner);
            inner = ACPmakePermute(MK_CONST(4, 2, 0, 3, 1), inner);
            inner = ACPmakeGridBlock(2, inner);
            break;
        case 3:
        case 4:
        case 5:
            inner = ACPmakeGridBlock((int) INFO_DIMS(inner) - 2, inner);
            break;
        default:
            if (ext) {
                inner = ACPreduceDimensionality(inner);
                inner = ACPjingGeneratePragma(inner, ext);
            } else
                CTIerrorLoc(NODE_LOCATION(INFO_WITH(inner)),
                            "Dimensionality of with loop (%zu) too high for gpu mapping strategy \"Jing's method\""
                            "(-gpu_mapping_strategy jings_method) can be at most 5. For higher dimensionalities, "
                            "use the extended Jing's method (-gpu_mapping_strategy jings_method_ext).",
                            INFO_DIMS(inner));
            break;
    }

    DBUG_RETURN(inner);
}

/** <!--********************************************************************-->
 * @}  <!-- Foldall Strategy -->
 *****************************************************************************/

info*
ACPfoldallGeneratePragma(info* inner) {
    DBUG_ENTER();

    INFO_DIMS_BEFORE(inner) = INFO_DIMS(inner);

    while (INFO_DIMS(inner) > 1)
        inner = ACPmakeFoldLast2(inner);

    // Split of first dimension to (rest | bx)
    inner = ACPmakeSplitLast(global.config.cuda_2d_block_x, inner);

    // Split of second dimension to (rest | by, bx)
    if (INFO_DIMS_BEFORE(inner) >= 2) {
        inner = ACPmakePermute(MK_CONST(2, 1, 0), inner);
        inner = ACPmakeSplitLast(global.config.cuda_2d_block_y, inner);
        inner = ACPmakePermute(MK_CONST(3, 1, 2, 0), inner);
    }

    // Split of third dimension to (ty, rest | by, bx)
    if (INFO_DIMS_BEFORE(inner) >= 3) {
        inner = ACPmakePermute(MK_CONST(3, 1, 2, 0), inner);
        inner = ACPmakeSplitLast(global.config.cuda_3d_thread_y, inner);
        inner = ACPmakePermute(MK_CONST(4, 3, 2, 0, 1), inner);
    }

    inner = ACPmakeGridBlock(INFO_DIMS_BEFORE(inner) == 1 ? 1 : 2, inner);

    DBUG_RETURN(inner);
}

/** <!--********************************************************************-->
 * @}  <!-- Strategy choosing -->
 *****************************************************************************/

info*
ACPgeneratePragma(info* arg_info) {
    DBUG_ENTER();

    arg_info     = ACPmakeShiftLB(ACPmakeGen(arg_info));
    if (global.gpu_mapping_compress)
        arg_info = ACPmakeCompressAll(arg_info);

    switch (global.gpu_mapping_strategy) {
        case Jings_method:
            arg_info = ACPjingGeneratePragma(arg_info, FALSE);
            break;
        case Jings_method_ext:
            arg_info = ACPjingGeneratePragma(arg_info, TRUE);
            break;
        case Foldall:
            arg_info = ACPfoldallGeneratePragma(arg_info);
            break;
        default:
            DBUG_UNREACHABLE("Invalid cuda mapping strategy");
    }

    DBUG_RETURN(arg_info);
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
node*
ACPwith(node* arg_node, info* arg_info) {
    DBUG_ENTER ();

    if (WITH_CUDARIZABLE(arg_node)) {
        INFO_WITH(arg_info) = arg_node;
        WITH_PART(arg_node) = TRAVdo(WITH_PART(arg_node), arg_info);
    } else {
        WITH_CODE(arg_node) = TRAVopt(WITH_CODE(arg_node), arg_info);
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
node*
ACPpart(node* arg_node, info* arg_info) {
    DBUG_ENTER ();

    if (PART_PRAGMA(arg_node) == NULL) {
        INFO_PART(arg_info) = arg_node;
        PART_GENERATOR(arg_node) = TRAVdo(PART_GENERATOR(arg_node), arg_info);
        DBUG_ASSERT (INFO_PRAGMA(arg_info) != NULL, "failed to generate pragma "
                                                    "for partition");
        PART_PRAGMA(arg_node) = INFO_PRAGMA (arg_info);
        INFO_PRAGMA (arg_info) = NULL;
    }

    PART_NEXT(arg_node) = TRAVopt(PART_NEXT(arg_node), arg_info);

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
node*
ACPgenerator(node* arg_node, info* arg_info) {
    DBUG_ENTER ();

    INFO_GEN(arg_info) = arg_node;
    arg_info = ACPcomputeDimensionality(arg_info);

    arg_info = ACPgeneratePragma(arg_info);
    node* pragma = TBmakePragma();
    PRAGMA_GPUKERNEL_APS(pragma) = INFO_PRAGMA(arg_info);
    INFO_PRAGMA (arg_info) = pragma;

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
