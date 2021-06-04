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
};

#define INFO_WITH(n) n->with
#define INFO_PART(n) n->part
#define INFO_GEN(n) n->gen

#define INFO_PRAGMA(n) n->pragma
#define INFO_DIMS(n) n->dims

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

    INFO_DIMS(arg_info) = (size_t) SHgetExtent(ARRAY_FRAMESHAPE(GENERATOR_BOUND1(INFO_GEN(arg_info))), 0);

    DBUG_RETURN(arg_info);
}

node*
ACPmakeSpap(char* staticName, size_t nargs, node** args) {
    DBUG_ENTER();

    node* exprs = NULL;
    for (size_t i = nargs - 1; i != (size_t) -1; i--)
        exprs = TBmakeExprs(args[i], exprs);

    node* spap = TBmakeSpap(
            TBmakeSpid(NULL, STRcpy(staticName)),
            exprs);

    DBUG_RETURN(spap);
}

node*
ACPmakeArray(size_t nelems, int* elems) {
    DBUG_ENTER();

    node* exprs = NULL;
    for (size_t i = nelems; i != (size_t) -1; i--)
        exprs = TBmakeExprs(TBmakeNum(elems[i]), exprs);

    node* array = TBmakeArray(TYmakeSimpleType(T_int), SHcreateShape(1, nelems), exprs);

    DBUG_RETURN(array);
}

node*
ACPmakeGen(void) {
    DBUG_ENTER();

    node* spap = ACPmakeSpap(GEN, 0, NULL);

    DBUG_RETURN(spap);
}

node*
ACPmakeGridBlock(int block_dims, node* inner) {
    DBUG_ENTER();

    node* args[2] = {
            TBmakeNum(block_dims),
            inner};
    node* spap    = ACPmakeSpap(GRIDBLOCK, 2, args);

    DBUG_RETURN(spap);
}

node*
ACPmakeShiftLB(node* inner) {
    DBUG_ENTER();

    node* args[1] = {inner};
    node* spap    = ACPmakeSpap(SHIFTLB, 1, args);

    DBUG_RETURN(spap);
}

node*
ACPmakeCompressGrid(size_t dims, int* compressDims, node* inner) {
    DBUG_ENTER();

    node* args[2] = {
            ACPmakeArray(dims, (int*) compressDims),
            inner};
    node* spap    = ACPmakeSpap(COMPRESSGRID, 2, args);

    DBUG_RETURN(spap);
}

node*
ACPmakePermute(size_t dims, int* permutation, node* inner) {
    DBUG_ENTER();

    node* args[2] = {
            ACPmakeArray(dims, (int*) permutation),
            inner};
    node* spap    = ACPmakeSpap(PERMUTE, 2, args);

    DBUG_RETURN(spap);
}

node*
ACPmakeSplitLast(int innersize, node* inner) {
    DBUG_ENTER();

    node* args[2] = {
            TBmakeNum(innersize),
            inner};
    node* spap    = ACPmakeSpap(SPLITLAST, 2, args);

    DBUG_RETURN(spap);
}

node*
ACPmakeFoldLast2(node* inner) {
    DBUG_ENTER();

    node* args[1] = {inner};
    node* spap    = ACPmakeSpap(FOLDLAST2, 1, args);

    DBUG_RETURN(spap);
}

node*
ACPmakePadLast(int paddedsize, node* inner) {
    DBUG_ENTER();

    node* args[2] = {
            TBmakeNum(paddedsize),
            inner};
    node* spap    = ACPmakeSpap(PADLAST, 2, args);

    DBUG_RETURN(spap);
}


/** <!--********************************************************************-->
 * @}  <!-- Strategy helper functions -->
 *****************************************************************************/

info*
ACPpermuteRotate(info* arg_info, int offset) {
    DBUG_ENTER();

    int* permutation = (int*) MEMmalloc(sizeof(int) * INFO_DIMS(arg_info));
    for (size_t i=0; i<INFO_DIMS(arg_info); i++)
        permutation[i] = ((int) i - offset + (int) INFO_DIMS(arg_info)) % (int) INFO_DIMS(arg_info);

    INFO_PRAGMA(arg_info) = ACPmakePermute(INFO_DIMS(arg_info), permutation,INFO_PRAGMA(arg_info));

    MEMfree(permutation);

    DBUG_RETURN(arg_info);
}

info*
ACPreduceDimensionality(info* arg_info) {
    DBUG_ENTER();

    if (INFO_DIMS(arg_info) % 2 == 1)
        ACPpermuteRotate(arg_info, 1);

    size_t nr_folds = INFO_DIMS(arg_info) / 2;
    for (size_t i=0; i<nr_folds; i++) {
        INFO_PRAGMA(arg_info) = ACPmakeFoldLast2(INFO_PRAGMA(arg_info));
        INFO_DIMS(arg_info) --;
        ACPpermuteRotate(arg_info, 1);
    }

    DBUG_RETURN(arg_info);
}

/** <!--********************************************************************-->
 * @}  <!-- Jings method Strategy -->
 *****************************************************************************/

info*
ACPjingDo(info* arg_info, bool ext) {
    DBUG_ENTER();

    // @formatter:off
    switch(INFO_DIMS(arg_info)) {
        case 0:
            INFO_PRAGMA(arg_info) = ACPmakeGridBlock(0,INFO_PRAGMA(arg_info));
            break;
        case 1:
            INFO_PRAGMA(arg_info) = ACPmakeGridBlock(1,
                                    ACPmakeSplitLast(global.config.cuda_1d_block_x,
                                    INFO_PRAGMA(arg_info)));
            break;
        case 2:
            ;
            int permute_2d_outer[4] = {2, 0, 3, 1};
            int permute_2d_inner[3] = {1, 2, 0};
            INFO_PRAGMA(arg_info) = ACPmakeGridBlock(2,
                                    ACPmakePermute(4, permute_2d_outer,
                                    ACPmakeSplitLast(global.config.cuda_2d_block_y,
                                    ACPmakePermute(3, permute_2d_inner,
                                    ACPmakeSplitLast(global.config.cuda_2d_block_x,
                                    INFO_PRAGMA(arg_info))))));
            break;
        case 3:
        case 4:
        case 5:
            INFO_PRAGMA(arg_info) = ACPmakeGridBlock((int) INFO_DIMS(arg_info) - 2, INFO_PRAGMA(arg_info));
            break;
        default:
            if (ext) {
                arg_info = ACPreduceDimensionality(arg_info);
                arg_info = ACPjingDo(arg_info, ext);
            } else
                CTIerrorLoc(NODE_LOCATION(INFO_WITH(arg_info)),
                            "Dimensionality of with loop (%zu) too high for gpu mapping strategy \"Jing's method\""
                            "(-gpu_mapping_strategy jings_method) can be at most 5. For higher dimensionalities, "
                            "use the extended Jing's method (-gpu_mapping_strategy jings_method_ext).",
                            INFO_DIMS(arg_info));
            break;
    }
    // @formatter:on

    DBUG_RETURN(arg_info);
}

info*
ACPjingGeneratePragma(info* arg_info, bool ext) {
    DBUG_ENTER();

    INFO_PRAGMA(arg_info) = ACPmakeShiftLB(ACPmakeGen());
    ACPjingDo(arg_info, ext);

    DBUG_RETURN(arg_info);
}

/** <!--********************************************************************-->
 * @}  <!-- Foldall Strategy -->
 *****************************************************************************/

info*
ACPfoldallGeneratePragma(info* arg_info) {
    DBUG_ENTER();

    DBUG_RETURN(arg_info);
}

/** <!--********************************************************************-->
 * @}  <!-- Strategy choosing -->
 *****************************************************************************/

info*
ACPgeneratePragma(info* arg_info) {
    DBUG_ENTER();

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

    INFO_GEN(arg_info)  = arg_node;
    arg_info = ACPcomputeDimensionality(arg_info);

    arg_info = ACPgeneratePragma(arg_info);
    node* pragma = TBmakePragma();
    PRAGMA_GPUKERNEL_APS(pragma) = INFO_PRAGMA(arg_info);
    INFO_PRAGMA (arg_info) = pragma;

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
