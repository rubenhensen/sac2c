/** <!--********************************************************************-->
 *
 * @defgroup gpukernel pragma compile functions
 *
 *   This module implements the functions that are needed for implementing the
 *   #pragma gpukernel. It contains the following functionality:
 *
 *  Compiler for the thread space transformation pragmas
 *  Compiler for the index vector transformation pragmas
 *
 *  When icm2c finds the ICM CUDA_THREAD_SPACE, it extracts the
 *  pragma and it calls GKCOcompHostKernelPragma (spap, nbounds, bounds)
 *  from this file.
 *
 *  // TODO: which ICM?
 *  When icm2c finds the ICM ???, it extracts the
 *  pragma again and it calls GKCOcompGPUDkernelPragma(spap, nbounds, bounds)
 *  from this file.
 *
 *  It is the purpose of the pragma functions to generate transformations for
 *  the thread space (GKCOcompHostKernelPragma) and for the inexes within the
 *  kernel function (GKCOcompGPUDkernelPragma). The information about the
 *  threadspace is kept in a structure internal to this file of type
 *
 *  gpukernelres_t == struct GPUKERNELRES; 
 *
 *  The pragma functions expect the N_spap node of the pragma
 *  and it obtains a vector of strings (preceeded by its length) as
 *  further arguments. These strings represent the generator bounds
 *  in scalarised form (CUDA kernels are only for AKD at least!).
 *  The strings are either variable reads (as .hr-icms) or, in case
 *  the compiler knows, constants.
 *
 *  From these arguments, the pragma functions generate a nesting
 *  of function calls that reflects the nesting and arguments of the
 *  pragma's functions. How exactly this happens is specific for each
 *  pragma function. The two pragma functions together generate three
 *  nestings of function calls, called passes. Each pass is identified
 *  with a parameter in gen, or a statement in GKCOcompInvGridBlock.
 *  This pass parameter is stored in gpukernelres_t, so all operations
 *  can directly determine what mode to execute in.
 *  Let us take an example:
 *
 *  #pragma GridBlock (2, CompressGrid ([1, 0, 1], Gen))
 *
 *  for this example, GKCOcompHostKernelPragma will transform it into
 *  the calls
 *
 *   // Pass 1: PASS_HOST
 *   GKCOcompGridBlock (2,
 *       GKCOcompCompressGrid ([1, 0, 1],
 *           GKCOcompGen( nbounds, bounds, PASS_HOST)))
 *
 *  and GKCOcompGPUDkernelPragma will transform it into the calls
 *
 *   // Pass 2: PASS_KERNEL_THREADSPACE
 *   gpukernelres_t* res =
 *      GKCOcompGridBlock(2,
 *          GKCOcompCompressGrid([1, 0, 1],
 *              GKCOcompGen(nbounds, bounds, PASS_KERNEL_THREADSPACE)))
 *
 *   // Pass 3: PASS_KERNEL_WLIDS
 *   // Note that this pass is inverted. It will retrieve the gpukernelres_t
 *   // generated in pass 2.
 *   GKCOcompInvGen(iv_var,
 *      GKCOcompInvCompressGrid([1, 0, 1],
 *          GKCOcompGridBlock(2, res)))
 *
 *  The actual construction of this function is achieved through a
 *  local function dispatch which uses the info in gpukernel_funs.mac
 *  to generate the string comparisons and functional calls as needed.
 *  As a consequence, new pragma functions can be added without 
 *  the need to modify GKCOcompHostKernelPragma or the dipatch function.
 *  All that is needed is a new entry in gpukernel_funs.mac and an 
 *  implementation of the coresponding GKCOcomp-function.
 *
 *  All GKCOcomp-functions obtain as last argument a threadspace
 *  representation of type gpukernelres_t and they return back a
 *  modified version of it.
 *
 *  All GKCOcomp-functions also print the ICM's needed for the
 *  transformation computation. After compilation, the generated ICM
 *  code should look something like this:
 *
 *  SAC_ND_DEF_FUN_BEGIN(funname, ...) {
 *      ...
 *
 *      // Pass 1: PASS_HOST
 *      // Declare and redefine variables, as we cannot modify the original upperbound variables
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5651_tmp)

		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5653_ub)
		SAC_GKCO_OPD_REDEFINE (SAC_gkco_prt_5653_ub, SAC_gkco_prt_5653_ub)

		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5657_ub)
		SAC_GKCO_OPD_REDEFINE (SAC_gkco_prt_5657_ub, SAC_gkco_prt_5657_ub)

		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5661_ub)
		SAC_GKCO_OPD_REDEFINE (SAC_gkco_prt_5661_ub, SAC_gkco_prt_5661_ub)


        // Handle step and width for the first and third dimension (determined by the parameter [1, 0, 1])
		SAC_GKCO_HOST_OPD_COMPRESS_SW (SAC_gkco_prt_5653_ub, SAC_gkco_prt_5653_ub, SAC_gkco_prt_5654_st, SAC_gkco_prt_5655_wi, SAC_gkco_prt_5651_tmp)
		SAC_GKCO_HOST_OPD_COMPRESS_SW (SAC_gkco_prt_5661_ub, SAC_gkco_prt_5661_ub, SAC_gkco_prt_5662_st, SAC_gkco_prt_5663_wi, SAC_gkco_prt_5651_tmp)


        // Make the grid and block spaces
		SAC_GKCO_HOST_OPM_SET_GRID (2147483647, 65535, 65535, 0, SAC_gkco_prt_5653_ub, SAC_gkco_prt_5657_ub)
		SAC_GKCO_HOST_OPM_SET_BLOCK (1024, 1024, 64, 1024, SAC_gkco_prt_5661_ub)

		// End pass

		...

		// Spawn kernels on gpu using the generated grid and block, using function `funname_kernel`
		...
	}

    funname_kernel(...) {
        ...

        // Pass 2: PASS_KERNEL_THREADSPACE
        // This pass is very similar to pass 1
        // One big difference: in many cases variables are not updated, but
        // new variables are created. This is because intermediate values
        // may later be needed to reconstruct the original index.
        // This need does not really become clear in this example however.
        // TODO: use actually generated code here, this is not exactly correct

 *      // Declare and redefine variables, as we cannot modify the original upperbound variables
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5651_tmp)

		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5653_ub)
		SAC_GKCO_OPD_REDEFINE (SAC_gkco_prt_5653_ub, SAC_gkco_prt_5653_ub)

		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5657_ub)
		SAC_GKCO_OPD_REDEFINE (SAC_gkco_prt_5657_ub, SAC_gkco_prt_5657_ub)

		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5661_ub)
		SAC_GKCO_OPD_REDEFINE (SAC_gkco_prt_5661_ub, SAC_gkco_prt_5661_ub)


        // Handle step and width for the first and third dimension (determined by the parameter [1, 0, 1])
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5662_ub)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5663_ub)
		SAC_GKCO_HOST_OPD_COMPRESS_SW (SAC_gkco_prt_5653_ub, SAC_gkco_prt_5662_ub, SAC_gkco_prt_5654_st, SAC_gkco_prt_5655_wi, SAC_gkco_prt_5651_tmp)
		SAC_GKCO_HOST_OPD_COMPRESS_SW (SAC_gkco_prt_5661_ub, SAC_gkco_prt_5663_ub, SAC_gkco_prt_5662_st, SAC_gkco_prt_5663_wi, SAC_gkco_prt_5651_tmp)

	    // Pass 2: PASS_KERNEL_WLIDS

	    // GridBlock, it's parameter determines how many dimensions are represented in the grid,
	    // and how many in the blocks
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5664_idx)
		SAC_GKCO_OPD_REDIFINE(THREADIDX_X, SAC_gkco_prt_5664_idx)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5665_idx)
		SAC_GKCO_OPD_REDIFINE(THREADIDX_Y, SAC_gkco_prt_5665_idx)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5666_idx)
		SAC_GKCO_OPD_REDIFINE(BLOCKIDX_X, SAC_gkco_prt_5666_idx)

		// CompressGrid
		SAC_GKCO_GPUD_OPD_COMPRESS_SW (SAC_gkco_prt_5654_st, SAC_gkco_prt_5655_wi, SAC_gkco_prt_5664_idx)
		SAC_GKCO_GPUD_OPD_COMPRESS_SW (SAC_gkco_prt_5662_st, SAC_gkco_prt_5663_wi, SAC_gkco_prt_5666_idx)

		// Gen
		SAC_GKCO_GPUD_DECLARE_IV(iv_var, 3)
		SAC_GKCO_GPUD_DEF_IV(iv_var, 0, SAC_gcko_prt_5664_idx)
		SAC_GKCO_GPUD_DEF_IV(iv_var, 1, SAC_gcko_prt_5665_idx)
		SAC_GKCO_GPUD_DEF_IV(iv_var, 2, SAC_gcko_prt_5666_idx)
    }

 *
 * @ingroup
 *
 * @{
 *
 *****************************************************************************/
#include <traverse.h>
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#define DBUG_PREFIX "GKCO"

#include "debug.h"

#include "free.h"
#include "DupTree.h"
#include "gpukernel_comp_funs.h"
#include "gpukernel_check_funs.h"
#include "ctinfo.h"
#include "globals.h"
#include "memory.h"
#include "str.h"
#include "str_vec.h"
#include "print.h"

#define CONST_ZERO "0"
#define CONST_ONE "1"

#define CONST_VAR_PREFIX "SAC_gkco"
#define CONST_UB_POSTFIX "ub"
#define CONST_IDX_POSTFIX "idx"
#define CONST_TMP_POSTFIX "tmp"

/**
 * Create a new temporary variable, with proper pre- and postfixes. The prefix is fixed to CONST_PREFIX.
 * The postfix is an argument, as this function can be used for different purpose variables.
 *
 * @param postfix The postfix to be added
 * @return The properly pre- and postfixed variable name.
 */
static char*
GKCOvarCreate(char* postfix) {
    DBUG_ENTER();

    char* without_prefix = TRAVtmpVarName(postfix);
    char* var            = STRcat(CONST_VAR_PREFIX, without_prefix);
    fprintf(global.outfile, "SAC_GKCO_OPD_DECLARE(%s)\n", var);

    MEMfree(without_prefix);

    DBUG_RETURN(var);
}

/**
 * Destroy a variable name. It is just an alias for MEMfree.
 *
 * @param var The variable name to be freed
 * @return NULL
 */
static char*
GKCOvarDestroy(char* var) {
    DBUG_ENTER();

    MEMfree(var);

    DBUG_RETURN(NULL);
}

/**
 * Transform a node array into an integer array. Easier to work with, especially in cases where the
 * nodes cannot be accessed sequentially.
 *
 * @param nums_node The node to be transformed into an array
 * @param length The expected length of the array. If the actual length of the parameter is different,
 *               a compiler error will be thrown.
 * @param pragma The pragma name, used for the error generation.
 * @return An integer array with the values from the nodes array.
 */
static int*
getNumArrayFromNodes(node* nums_node, size_t length, char* pragma) {
    DBUG_ENTER ();

    checkArgsLength(nums_node, length, pragma);

    node* exprs = ARRAY_AELEMS(nums_node);
    int * array = (int*) MEMmalloc(sizeof(int*) * length);

    for (size_t i = 0; i < length; i++) {
        node* arg = EXPRS_EXPR(exprs);
        array[i] = NUM_VAL(arg);
        exprs = EXPRS_NEXT(exprs);
    }

    DBUG_RETURN(array);
}

/**
 * Shorthand for a GKCOcomp-function end. Wraps up stuff nicely and automatically.
 */
#define COMP_FUN_END(gkr)                                                                                   \
    fprintf(global.outfile, "\n");                                                                          \
    DBUG_EXECUTE(PrintGPUkernelres(gkr, stderr));                                                           \
    DBUG_EXECUTE(fprintf(stderr, "\n"));                                                                    \
    DBUG_RETURN(gkr);

/**
 * Flag locations inside the PASS identifier variables.
 */
#define PASS_TS_BUILDING 1          // Pass is filling the xx_at stack in gpukernelres_t
#define PASS_TS_CONSUMING 2         // Pass is consuming the xx_at stack in gpukernelres_t
#define PASS_ID_COMPUTING 4         // Pass is handling and generating code for id variables

/**
 * Enum to specify a pass over the Pragma functions. These enum value toggle certain functionalities in
 * the compile functions. It is not actually an enum, as ISO C forbids forward declarations of enums.
 */
// (bottom up) Create a thread space to spawn the kernels
#define PASS_HOST 0
// (bottom up) Recreate the intermediate thread space computation values
#define PASS_KERNEL_THREADSPACE PASS_TS_BUILDING
// (top down)  Generate the iv from the kernel coordinates and the pass above
#define PASS_KERNEL_WLIDS       PASS_TS_CONSUMING | PASS_ID_COMPUTING


/**
 * GPU kernel representation struct. It contains:
 *   - A dimensionality of type size_t
 *   - Four string vectors, containing the variable names of the index space (lowerbound, upperbound, step and width)
 *     for each dimension
 *
 * IMPORTANT NOTE: In these compiling functions, the only values that we need to edit are the upperbounds and idx
 * variables. For this this reason, we copy all upperbounds and idx's into new variables _owned by the GPU
 * kernel res_. The other variables are all represented using strings _owned by others_.
 */
struct GPUKERNELRES {
    // Current pass, contains flags for turning specific parts of the code on and off
    pass_t currentpass;
    // The dimensionality
    size_t dim;

    // Variable identifier storage (not idx is only initiated and used when PASS_ID_COMPUTING flag is set)
    strvec* lowerbound;
    strvec* upperbound;
    strvec* step;
    strvec* width;
    strvec* idx;

    // Variable identifier stack, used for recovering variable identifiers when older versions of them
    // are needed.
    // This stack is pushed to in RemoveDimension or when GKR_XX_PUSH is called, only when PASS_TS_BUILDING is set
    // This stack is popped from in AddDimension or when GKR_XX_POP is called, only when PASS_TS_CONSUMING is set
    strvec* lowerbound_at;
    strvec* upperbound_at;
    strvec* step_at;
    strvec* width_at;

    // Single helper variable that was assigned. It can be used by all ICM's, as they are called one by one.
    char* helper_a;
};

// Flag macro's
#define GKR_PASS(gkr) gkr->currentpass
#define GKR_CHANGE_PASS(gkr, pass) GKR_PASS(gkr) = pass
#define GKR_IS_TS_BUILDING(gkr) (GKR_PASS(gkr) & PASS_TS_BUILDING)
#define GKR_IS_TS_CONSUMING(gkr) (GKR_PASS(gkr) & PASS_TS_CONSUMING)
#define GKR_IS_ID_COMPUTING(gkr) (GKR_PASS(gkr) & PASS_ID_COMPUTING)

// DIM getter/setter
#define GKR_DIM(gkr) gkr->dim

// Variable identifier getters/setters
#define GKR_LB(gkr) gkr->lowerbound
#define GKR_UB(gkr) gkr->upperbound
#define GKR_ST(gkr) gkr->step
#define GKR_WI(gkr) gkr->width
#define GKR_IDX(gkr) gkr->idx

// Variable identifier for specific dimension getters
#define GKR_LB_D_READ(gkr, i) STRVECsel(GKR_LB(gkr), i)
#define GKR_UB_D_READ(gkr, i) STRVECsel(GKR_UB(gkr), i)
#define GKR_ST_D_READ(gkr, i) STRVECsel(GKR_ST(gkr), i)
#define GKR_WI_D_READ(gkr, i) STRVECsel(GKR_WI(gkr), i)
#define GKR_IDX_D_READ(gkr, i) STRVECsel(GKR_IDX(gkr), i)

// Variable identifier stack getters
#define GKR_LB_AT(gkr) gkr->lowerbound_at
#define GKR_UB_AT(gkr) gkr->upperbound_at
#define GKR_ST_AT(gkr) gkr->step_at
#define GKR_WI_AT(gkr) gkr->width_at

// Variable identifier stack push functions
#define GKR_LB_PUSH(gkr, dim, str)                                                                          \
    if (GKR_IS_TS_BUILDING(gkr)) {                                                                          \
        STRVECappend(GKR_LB_AT(gkr), GKR_LB_D_READ(gkr, dim));                                              \
        STRVECswap(GKR_LB(gkr), dim, str);                                                                  \
    }
#define GKR_UB_PUSH(gkr, dim, str)                                                                          \
    if (GKR_IS_TS_BUILDING(gkr)) {                                                                          \
        STRVECappend(GKR_UB_AT(gkr), GKR_UB_D_READ(gkr, dim));                                              \
        STRVECswap(GKR_UB(gkr), dim, str);                                                                  \
    }
#define GKR_ST_PUSH(gkr, dim, str)                                                                          \
    if (GKR_IS_TS_BUILDING(gkr)) {                                                                          \
        STRVECappend(GKR_ST_AT(gkr), GKR_ST_D_READ(gkr, dim));                                              \
        STRVECswap(GKR_ST(gkr), dim, str);                                                                  \
    }
#define GKR_WI_PUSH(gkr, dim, str)                                                                          \
    if (GKR_IS_TS_BUILDING(gkr)) {                                                                          \
        STRVECappend(GKR_WI_AT(gkr), GKR_WI_D_READ(gkr, dim));                                              \
        STRVECswap(GKR_WI(gkr), dim, str);                                                                  \
    }

#define GKR_UB_WRITABLE(gkr, dim) WritableUpperboundVar(gkr, dim)

// Variable identifier stack pop functions
#define GKR_LB_POP(gkr, dim)                                                                                \
    if (GKR_IS_TS_CONSUMING(gkr)) {                                                                         \
        STRVECswap(GKR_LB(gkr), dim, STRVECpop(GKR_LB_AT(gkr)));                                            \
    }
#define GKR_UB_POP(gkr, dim)                                                                                \
    if (GKR_IS_TS_CONSUMING(gkr)) {                                                                         \
        STRVECswap(GKR_UB(gkr), dim, STRVECpop(GKR_UB_AT(gkr)));                                            \
    }
#define GKR_ST_POP(gkr, dim)                                                                                \
    if (GKR_IS_TS_CONSUMING(gkr)) {                                                                         \
        STRVECswap(GKR_ST(gkr), dim, STRVECpop(GKR_ST_AT(gkr)));                                            \
    }
#define GKR_WI_POP(gkr, dim)                                                                                \
    if (GKR_IS_TS_CONSUMING(gkr)) {                                                                         \
        STRVECswap(GKR_WI(gkr), dim, STRVECpop(GKR_WI_AT(gkr)));                                            \
    }

// Helper variable getter
#define GKR_HELPER_A(gkr) gkr->helper_a

/**
 * Make a new gpu kernel representation struct. The vectors containing the variable names will be empty at this point.
 * This is because the variables have to be inserted manually at GKCOcompGen.
 *
 * @param dim The dimensionality of the gpu kernel representation
 * @return The newly created gpu kernel representation
 */
static gpukernelres_t*
MakeGPUkernelres(size_t dim, pass_t pass) {
    DBUG_ENTER();

    gpukernelres_t* gkr = (gpukernelres_t*) MEMmalloc(sizeof(gpukernelres_t));

    GKR_PASS(gkr) = pass;
    GKR_DIM(gkr)  = dim;
    GKR_LB(gkr)   = STRVECempty(dim);
    GKR_UB(gkr)   = STRVECempty(dim);
    GKR_ST(gkr)   = STRVECempty(dim);
    GKR_WI(gkr)   = STRVECempty(dim);
    GKR_IDX(gkr)  = STRVECempty(dim);

    GKR_LB_AT(gkr) = STRVECempty(0);
    GKR_UB_AT(gkr) = STRVECempty(0);
    GKR_ST_AT(gkr) = STRVECempty(0);
    GKR_WI_AT(gkr) = STRVECempty(0);

    GKR_HELPER_A(gkr) = GKCOvarCreate(CONST_TMP_POSTFIX);

    DBUG_RETURN(gkr);
}

/**
 * Free a GPU kernel res. Per convention, the gkr only owns all strings in the upperbound vector and the idx
 * vector. These vectors will be deep freed, while the others will be shallow freed.
 *
 * @param gkr The struct to be freed
 * @return the null pointer
 */
static gpukernelres_t*
FreeGPUkernelres(gpukernelres_t* gkr) {
    DBUG_ENTER();

    STRVECfree(GKR_LB(gkr));
    STRVECfreeDeep(GKR_UB(gkr));
    STRVECfree(GKR_ST(gkr));
    STRVECfree(GKR_WI(gkr));
    STRVECfreeDeep(GKR_IDX(gkr));

    STRVECfree(GKR_LB_AT(gkr));
    STRVECfree(GKR_UB_AT(gkr));
    STRVECfree(GKR_ST_AT(gkr));
    STRVECfree(GKR_WI_AT(gkr));

    MEMfree(GKR_HELPER_A(gkr));

    MEMfree(gkr);

    DBUG_RETURN(NULL);
}

/**
 * Add a dimension to the GPU kernel representation.
 *
 * Because it is a new dimension, lb will be set to 0, and step and width will be set to 1. The value for ub will be
 * initiated as a new variable with postfix "_ub", but not set.
 *
 * @param gkr The GPU kernel representation
 */
static void
AddDimension(gpukernelres_t* gkr) {
    DBUG_ENTER();

    GKR_DIM(gkr)++;

    if (!GKR_IS_TS_CONSUMING(gkr)) {
        STRVECappend(GKR_LB(gkr), CONST_ZERO);
        STRVECappend(GKR_UB(gkr), GKCOvarCreate(CONST_UB_POSTFIX));
        STRVECappend(GKR_ST(gkr), CONST_ONE);
        STRVECappend(GKR_WI(gkr), CONST_ONE);
    } else {
        size_t new_dim = GKR_DIM(gkr) - 1;
        GKR_LB_POP(gkr, new_dim);
        GKR_UB_POP(gkr, new_dim);
        GKR_ST_POP(gkr, new_dim);
        GKR_WI_POP(gkr, new_dim);
    }

    if (GKR_IS_ID_COMPUTING(gkr))
        STRVECappend(GKR_IDX(gkr), GKCOvarCreate(CONST_IDX_POSTFIX));

    DBUG_RETURN();
}

/**
 * Remove a dimension from the GPU kernel representation.
 *
 * @param gkr The GPU kernel representation from which the dimension has to be removed
 */
static void
RemoveDimension(gpukernelres_t* gkr) {
    DBUG_ENTER();

    if (!GKR_IS_TS_BUILDING(gkr)) {
        STRVECpop(GKR_LB(gkr));
        GKCOvarDestroy(STRVECpop(GKR_UB(gkr)));
        STRVECpop(GKR_ST(gkr));
        STRVECpop(GKR_WI(gkr));
    } else {
        size_t old_dim = GKR_DIM(gkr) - 1;
        GKR_LB_PUSH(gkr, old_dim, NULL)
        GKR_UB_PUSH(gkr, old_dim, NULL)
        GKR_ST_PUSH(gkr, old_dim, NULL)
        GKR_WI_PUSH(gkr, old_dim, NULL)
    }

    if (GKR_IS_ID_COMPUTING(gkr))
        GKCOvarDestroy(STRVECpop(GKR_UB(gkr)));

    GKR_DIM(gkr)--;

    DBUG_RETURN();
}

/**
 * Retrieve a writable upperbound variable for dimension dim. If the PASS_TS_BUILDING flag is set, a new
 * variable is generated and stored, and the old one is pushed to the stack. If not, the current upperbound
 * variable is returned.
 *
 * @param gkr The GPU kernel representation
 * @param dim The dimension in question
 * @return A string containing the writable upperbound variable identifier for dimension dim
 */
static char*
WritableUpperboundVar(gpukernelres_t* gkr, size_t dim) {
    if (GKR_IS_TS_BUILDING(gkr)) {
        char* new_str = GKCOvarCreate(CONST_UB_POSTFIX);
        char* old_str = STRVECswap(GKR_UB(gkr), dim, new_str);
        STRVECappend(GKR_UB_AT(gkr), old_str);
        return new_str;
    }
    return GKR_UB_D_READ(gkr, dim);
}

#ifndef DBUG_OFF

/**
 * Print a GPU kernel representation. For debugging purposes only.
 *
 * @param gkr The GPU kernelres to be printed
 * @param stream The stream to print to
 */
static void
PrintGPUkernelres(gpukernelres_t* gkr, FILE* stream) {
    DBUG_ENTER();

    size_t linesize = 120;
    fprintf(stream, "GPU kernelres (dim: %zu)\n", GKR_DIM(gkr));
    fprintf(stream, "lb = ");
    STRVECprint(GKR_LB(gkr), stream, linesize);
    fprintf(stream, "ub = ");
    STRVECprint(GKR_UB(gkr), stream, linesize);
    fprintf(stream, "step = ");
    STRVECprint(GKR_ST(gkr), stream, linesize);
    fprintf(stream, "width = ");
    STRVECprint(GKR_WI(gkr), stream, linesize);
    fprintf(stream, "idx = ");
    STRVECprint(GKR_IDX(gkr), stream, linesize);
    fprintf(stream, "helpersa = %s\n", GKR_HELPER_A(gkr));

    DBUG_RETURN();
}


/** <!-- ****************************************************************** -->
 * @fn void
 *     printNumArray (node *array)
 *
 * @brief helper function printing N_array with N_num and N-id elems only
 *        to stderr.
 *        Solely needed for debugging.
 *
 * @param array  N_array node to be printed.
 *               variable reads or constants.
 *
 ******************************************************************************/
static
void
printNumArray(node* array) {
    node* elems;

    elems = ARRAY_AELEMS(array);
    if (NODE_TYPE (EXPRS_EXPR(elems)) == N_num) {
        fprintf(stderr, "[ %d", NUM_VAL(EXPRS_EXPR(elems)));
    } else {
        fprintf(stderr, "[ %s", ID_NAME (EXPRS_EXPR(elems)));
    }
    elems = EXPRS_NEXT(elems);
    while (elems != NULL) {
        if (NODE_TYPE (EXPRS_EXPR(elems)) == N_num) {
            fprintf(stderr, ", %d", NUM_VAL(EXPRS_EXPR(elems)));
        } else {
            fprintf(stderr, ", %s", ID_NAME (EXPRS_EXPR(elems)));
        }
        elems = EXPRS_NEXT(elems);
    }
    fprintf(stderr, "]");
}

#endif


/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     dispatch (node *spap, unsigned int bnum, char **bounds)
 *
 * @brief generates the actual function dispatch based on the info in 
 *        gpukernel_funs.mac. It also handles the special case of the
 *        innermost N_spid "Gen" as a call to GKCOcompGen.
 *
 * @param spap   either N_spap of an outer function or N_spid for "Gen"
 * @param bnum   number of bound elements (== number of strings in bounds)
 * @param bounds the actual bounds as strings, these are either SAC runtime
 *               variable reads or constants.
 *
 * @return the result of the dispatched function call.
 ******************************************************************************/
static
gpukernelres_t*
dispatch(node* spap, unsigned int bnum, char** bounds, pass_t pass) {
    gpukernelres_t* res = NULL;

    DBUG_ENTER ();

    if (NODE_TYPE (spap) == N_spid) {
        res = GKCOcompGen(bnum, bounds, pass);

#define ARGS(nargs) ARG##nargs
#define ARG0
#define ARG1 EXPRS_EXPR (SPAP_ARGS (spap)),
#define SKIPS(nargs) SKIP##nargs
#define SKIP0
#define SKIP1 EXPRS_NEXT
#define WLP(fun, nargs, checkfun)                                                         \
    } else if (STReq (SPAP_NAME (spap), #fun)) {                                          \
        DBUG_ASSERT ((SPAP_ARGS (spap) != NULL), "missing argument in `%s' ()", #fun);    \
        res = GKCOcomp ## fun ( ARGS( nargs)                                              \
                                dispatch (EXPRS_EXPR ( SKIPS( nargs) (SPAP_ARGS (spap))), \
                                          bnum, bounds, pass));
// @formatter:off
#include "gpukernel_funs.mac"

#undef WLP
#undef ARGS
#undef ARG0
#undef ARG1
#undef SKIPS
#undef SKIP0
#undef SKIP1
// @formatter:on

    } else {
        DBUG_ASSERT(0 == 1, "expected gpukernel function, found `%s'", SPAP_NAME(spap));
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompHostKernelPragma (node *spap, unsigned int bnum, char ** bounds)
 *
 * @brief generates the actual function nesting for the PASS_HOST pass. It manually calls the
 *        mandatory outer function GKCOcompGridBlock with its parameter and
 *        it provides the innermost calls through the recursive helper function
 *        dispatch.
 *
 * @result Prints the code necessary to compute the transformed thread space.
 *
 * @param spap   N_spap of the outer function GridBlock(...)
 * @param bnum   number of bound elements (== number of strings in bounds)
 * @param bounds the actual bounds as strings, these are either SAC runtime
 *               variable reads or constants.
 ******************************************************************************/
void
GKCOcompHostKernelPragma(node* spap, unsigned int bnum, char** bounds) {
    gpukernelres_t* res;
    DBUG_ENTER ();

    DBUG_ASSERT (spap != NULL, "NULL pointer for funcall in gpukernel pragma!");
    DBUG_ASSERT (NODE_TYPE(spap) == N_spap, "non N_spap funcall in gpukernel pragma!");
    DBUG_ASSERT (STReq(SPAP_NAME(spap), "GridBlock"), "expected `GridBlock' found `%s'",
                 SPAP_NAME(spap));
    /*
     * we could dbug assert many more things here but the asssumption is that 
     * pragmas themselves are rather likely left untouched and since we have 
     * checked the pragma correctness in the parser already, this is likely
     * overkill. Moreover, the debug version provides helpful error messages
     * in case accessors yield unexpected things.
     */

    res = GKCOcompGridBlock(EXPRS_EXPR(SPAP_ARGS(spap)),
                            dispatch(EXPRS_EXPR(EXPRS_NEXT(SPAP_ARGS(spap))), bnum, bounds, PASS_HOST));
    FreeGPUkernelres(res);

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompGPUDkernelPragma (node *spap, unsigned int bnum, char ** bounds)
 *
 * @brief generates the actual function nesting for the PAST_KERNEL_* passes. It manually calls the
 *        mandatory outer function GKCOcompGridBlock with its parameter and
 *        it provides the innermost calls through the recursive helper function
 *        dispatch.
 *        // TODO: update once changed
 *
 * @result Prints the code necessary to compute the transformed real index vector
 *         from the grid/block coordinates
 *
 * @param spap   N_spap of the outer function GridBlock(...)
 * @param bnum   number of bound elements (== number of strings in bounds)
 * @param bounds the actual bounds as strings, these are either SAC runtime
 *               variable reads or constants.
 ******************************************************************************/
void
GKCOcompGPUDkernelPragma(node* spap, unsigned int bnum, char** bounds) {
    DBUG_ENTER ();

    DBUG_ASSERT (spap != NULL, "NULL pointer for funcall in gpukernel pragma!");
    DBUG_ASSERT (NODE_TYPE(spap) == N_spap, "non N_spap funcall in gpukernel pragma!");
    DBUG_ASSERT (STReq(SPAP_NAME(spap), "GridBlock"), "expected `GridBlock' found `%s'",
                 SPAP_NAME(spap));
    /*
     * we could dbug assert many more things here but the asssumption is that
     * pragmas themselves are rather likely left untouched and since we have
     * checked the pragma correctness in the parser already, this islikely
     * overkill. Moreover, the debug version provides helpful error messages
     * in case accessors yield unexpected things.
     */

    gpukernelres_t* res = GKCOcompGridBlock(EXPRS_EXPR(SPAP_ARGS(spap)),
                                            dispatch(EXPRS_EXPR(EXPRS_NEXT(SPAP_ARGS(spap))), bnum, bounds,
                                                     PASS_KERNEL_THREADSPACE));
    // TODO: inverse pragma dispatch for pass 3 PASS_KERNEL_WLIDS

    FreeGPUkernelres(res);

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompGridBlock (node *num, gpukernelres_t *inner)
 *
 * @brief 
 *
 * @param num    N_num argument of GridBlock indicating the number of
 *               dimensions in the block specification
 * @param inner  the thread space that results from applying all inner 
 *               pragma functions
 *
 * @return the resulting thread space from applying GridBlock (num) to it.
 ******************************************************************************/
gpukernelres_t*
GKCOcompGridBlock(node* num, gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling GridBlock ( %i, inner)", NUM_VAL(num));

    checkNumLesseqDim(num, GKR_DIM(inner), "GridBlock");

    char* icm[2] = {"SAC_GKCO_HOST_OPM_SET_GRID", "SAC_GKCO_HOST_OPM_SET_BLOCK"};
    size_t       from[2]  = {0, (size_t) NUM_VAL(num)};
    size_t       to[2]    = {(size_t) NUM_VAL(num), GKR_DIM(inner)};
    unsigned int max_s[8] = {
            global.cuda_options.cuda_max_x_grid, global.cuda_options.cuda_max_xy_block,  // max x
            global.cuda_options.cuda_max_yz_grid, global.cuda_options.cuda_max_xy_block, // max y
            global.cuda_options.cuda_max_yz_grid, global.cuda_options.cuda_max_z_block,  // max z
            0, global.cuda_options.cuda_max_threads_block,                               // max product (only for block)
    };

    // Two iterations, one for the grid and one for the block. Inside the loop, the arrays above will determine the
    // actual values that are used.
    for (size_t gb = 0; gb < 2; gb++) {
        // Prints the name of the macro, with the static arguments
        //                SAC_GKCO_SET_<> (max_x, max_y, max_z, max_total
        fprintf(global.outfile, "%s(%u   , %u   , %u   , %u",
                icm[gb], max_s[gb], max_s[2 + gb], max_s[4 + gb], max_s[6 + gb]);
        // Print the dynamic arguments. From and to (defined above) determine what dimensions belong to the grid/block
        for (size_t i = from[gb]; i < to[gb]; i++)
            fprintf(global.outfile, ", %s", GKR_UB_D_READ(inner, i));
        // Close off the macro
        fprintf(global.outfile, ")\n\n");
    }

    COMP_FUN_END(inner)
}

/**
 * Inverse function of GKCOcompGridBlock. This function starts the pragma chain for the ID generation. It generates
 * variables for the idx dimension variables.
 * pass PASS_KERNEL_WLIDS.
 *
 * @param num The number of grid dimensions
 * @param outer The GPU kernel res created by the PASS_KERNEL_THREADSPACE pass.
 * @return
 */
gpukernelres_t*
GKCOcompInvGridBlock(node* num, gpukernelres_t* outer, pass_t pass) {
    DBUG_ENTER();
    DBUG_PRINT("compiling idx variable generation");

    GKR_CHANGE_PASS(outer, pass);

    size_t from[2] = {0, (size_t) NUM_VAL(num)};
    size_t to[2]   = {(size_t) NUM_VAL(num), GKR_DIM(outer)};
    char* grid_block_var[6] = {
            "THREADIDX_X", "BLOCKIDX_X",
            "THREADIDX_Y", "BLOCKIDX_Y",
            "THREADIDX_Z", "BLOCKIDX_Z",
    };

    // Two iterations, one for the grid and one for the block. Inside the loop, the arrays above will determine the
    // actual values that are used.
    for (size_t gb = 0; gb < 2; gb++) {
        // Print the dynamic arguments. From and to (defined above) determine what dimensions belong to the grid/block
        for (size_t i = from[gb]; i < to[gb]; i++) {
            size_t xyz_int = i - from[gb];
            STRVECappend(GKR_IDX(outer), GKCOvarCreate(CONST_IDX_POSTFIX));
            fprintf(global.outfile, "SAC_CUDA_OPD_REDIFINE(%s, %s)\n\n",
                    grid_block_var[2 * xyz_int + gb], GKR_IDX_D_READ(outer, i));
        }
    }

    COMP_FUN_END(outer)
}

/**
 * Debug version of GKCOcompGen, can be removed when the arguments are correct
 *
 * @param bnum
 * @return
 */
gpukernelres_t*
GKCOcompGenDbug(unsigned int bnum, pass_t pass) {
    DBUG_ENTER();

    size_t dim = bnum / 2;
    gpukernelres_t* gkr = MakeGPUkernelres(dim, pass);

    for (size_t i = 0; i < dim; i++) {
        STRVECappend(GKR_LB(gkr), GKCOvarCreate("lb"));
        STRVECappend(GKR_UB(gkr), GKCOvarCreate("ub"));
        STRVECappend(GKR_ST(gkr), GKCOvarCreate("st"));
        STRVECappend(GKR_WI(gkr), GKCOvarCreate("wi"));

        fprintf(global.outfile, "SAC_GKCO_OPD_REDEFINE(%s, %s)\n\n",
                GKR_UB_D_READ(gkr, i), GKR_UB_D_READ(gkr, i));
    }

    STRVECswap(GKR_LB(gkr), 0, CONST_ZERO);

    COMP_FUN_END(gkr)
}


/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompGen ( unsigned int bnum, char **bounds)
 *
 * @brief
 *
 * @param bnum   the number of bound strings provided in bounds. This is
 *               always a multiple of 4 as we always provide all LUSW.
 * @param bounds The actual bound strings. The order of the strings is
 *               the same as in the generators of WLs, i.e., 
 *               l_0, ...l_n, u_0, ...u_n, s_0, ...s_n, w_0, ...w_n
 *
 * @return the resulting naive thread space.
 ******************************************************************************/
gpukernelres_t*
GKCOcompGen(unsigned int bnum, char** bounds, pass_t pass) {
#ifndef DBUG_OFF
    unsigned int i;
#endif
    DBUG_ENTER ();
    DBUG_PRINT ("compiling Gen:");
    DBUG_EXECUTE (
            fprintf(stderr, "\n    Gen ( %u", bnum);
            for (i = 0; i < bnum; i++) {
                fprintf(stderr, ", %s", bounds[i]);
            }
            fprintf(stderr, ")\n");
    );
    fprintf(global.outfile, "\n");

    size_t dim = bnum / 4;
    gpukernelres_t* gkr = MakeGPUkernelres(dim, pass);

    for (i = 0; i < dim; i++) {
        STRVECappend(GKR_LB(gkr), bounds[i + 0 * dim]);
        STRVECappend(GKR_UB(gkr), GKCOvarCreate(CONST_UB_POSTFIX));
        STRVECappend(GKR_ST(gkr), bounds[i + 2 * dim]);
        STRVECappend(GKR_WI(gkr), bounds[i + 3 * dim]);

        fprintf(global.outfile, "SAC_GKCO_OPD_REDEFINE(%s, %s)\n\n",
                bounds[i + 1 * dim], GKR_UB_D_READ(gkr, i));
    }

    // TODO: remove once stuff works fine and uncomment line below :)
    // COMP_FUN_END(gkr);

    fprintf(global.outfile, "\n");

    DBUG_EXECUTE(PrintGPUkernelres(gkr, stderr));
    DBUG_EXECUTE(fprintf(stderr, "\n"));

    DBUG_PRINT("Note, the GKR above is freed immedately again for debbugging purposes. ");
    DBUG_PRINT("Instead, the GKR below is used: \n");

    FreeGPUkernelres(gkr);
    gkr = GKCOcompGenDbug(bnum, pass);

    DBUG_RETURN (gkr);
}

/**
 * Inverse function of GKCOcompGen. This function ends the pragma traversal for the ID generation.
 * It declares and fills the index vector, and frees the gpukernelres_t structure.
 *
 * @param iv_var The name of the index vector to be declared and filled
 * @param outer The outer GPU kernel res, adapted by the other pragma calls
 */
gpukernelres_t*
GKCOcompInvGen(char* iv_var, gpukernelres_t* outer) {
    DBUG_ENTER();
    DBUG_PRINT ("compiling IV generation ():");

    fprintf(global.outfile, "SAC_GKCO_GPUD_DECLARE_IV(%s, %zu)\n\n",
            iv_var, GKR_DIM(outer));
    for (size_t i = 0; i < GKR_DIM(outer); i++) {
        fprintf(global.outfile, "SAC_GKCO_GPUD_DEF_IV(%s, %zu, %s)\n\n",
                iv_var, i, GKR_IDX_D_READ(outer, i));
    }

    COMP_FUN_END(outer);
}

/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompShiftLB (gpukernelres_t *inner)
 *
 * @brief
 *
 * @param inner  the thread space that results from applying all inner
 *               pragma functions
 *
 * @return the resulting thread space from applying ShiftLB (array) to it.
 ******************************************************************************/
gpukernelres_t*
GKCOcompShiftLB(gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling ShiftLB (inner):");

    for (size_t i = 0; i < GKR_DIM(inner); i++) {
        // We only have to handle dimensions that aren't already normalized
        if (!STReq(GKR_LB_D_READ(inner, i), CONST_ZERO)) {
            // Compute the new upperbound variables
            fprintf(global.outfile, "SAC_GKCO_HOST_OPD_SHIFT_LB(%s, %s)\n\n",
                    GKR_LB_D_READ(inner, i), GKR_UB_D_READ(inner, i));
        }

        // We push the pre-shift lb and ub variable identifiers to the stack, to be able to recover it later
        GKR_LB_PUSH(inner, i, CONST_ZERO)
        GKR_UB_WRITABLE(inner, i);
    }

    COMP_FUN_END(inner)
}

/**
 * Inverse function of GKCOcompShiftLB. This function assumes that the function GKCOcompShiftLB has pushed the
 * lb and ub variable identifiers to the corresponding stacks for all dimensions
 *
 * @param outer the outer GPU kernel res, resulting from applying the other pragmas
 * @return the newly computed GPU kernel res, with restored lb and ub variable identifiers.
 */
gpukernelres_t*
GKCOcompInvShiftLb(gpukernelres_t* outer) {
    DBUG_ENTER();
    DBUG_PRINT ("compiling UnShiftLB (inner):");

    // To be able to pop the lb variables from the stack in the inverse order in which we pushed,
    // we need to loop the dimensions in inverse order as well
    for (size_t i = GKR_DIM(outer) - 1; i != (size_t) -1; i--) {
        // Pop the pre-shift lb and ub variable identifiers from the stack
        GKR_LB_POP(outer, i)
        GKR_UB_POP(outer, i)

        // We only have to handle dimensions that aren't already normalized
        if (!STReq(GKR_LB_D_READ(outer, i), CONST_ZERO)) {
            // Compute the new
            fprintf(global.outfile, "SAC_GKCO_GPUD_OPD_UNSHIFT_LB(%s, %s)\n\n",
                    GKR_LB_D_READ(outer, i), GKR_IDX_D_READ(outer, i));
        }
    }

    COMP_FUN_END(outer)
}


/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompCompressGrid ( node *array, gpukernelres_t *inner)
 *
 * @brief
 *
 * @param array  N_array argument indicating for each axis whether it should
 *               be compressed or not 1 => do comrpress 0 => do not!
 * @param inner  the thread space that results from applying all inner
 *               pragma functions
 *
 * @return the resulting thread space from applying CompressGrid (array) to it.
 ******************************************************************************/
gpukernelres_t*
GKCOcompCompressGrid(node* shouldCompress_node, gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling CompressGrid:");
    DBUG_EXECUTE (
            fprintf(stderr, "    CompressGrid (");
            printNumArray(shouldCompress_node);
            fprintf(stderr, ", inner)\n");
    );

    // Get the shouldCompress variable as an array instead of AST nodes
    int* shouldCompress = getNumArrayFromNodes(shouldCompress_node, GKR_DIM(inner), "CompressGrid");

    for (size_t i = 0; i < GKR_DIM(inner); i++) {
        // Skip this iteration if compress for this dimension is toggled off
        if (!shouldCompress[i]) continue;
        // Check whether the lowerbound is indeed 0
        checkLbZero(GKR_LB_D_READ(inner, i), shouldCompress_node, "CompressGrid", i);

        // Get readable and writable variant of UB var. It automatically changes the variable identifier
        // for us if we need to reference the old variable value later.
        char* ub_read  = GKR_UB_WRITABLE(inner, i);
        char* ub_write = GKR_UB_D_READ(inner, i);

        // If the step is 1, we don't need to compute anything
        if (STReq(GKR_ST_D_READ(inner, i), CONST_ONE)) {}
            // If the width is 1, we only need to handle the step
        else if (STReq(GKR_WI_D_READ(inner, i), CONST_ONE))
            fprintf(global.outfile, "SAC_GKCO_HOST_OPD_COMPRESS_S(%s, %s, %s)\n\n",
                    ub_read, ub_write, GKR_ST_D_READ(inner, i));
            // Else, we handle the case where both the step and width are relevant
        else
            fprintf(global.outfile, "SAC_GKCO_HOST_OPD_COMPRESS_SW(%s, %s, %s, %s, %s)\n\n",
                    ub_read, ub_write, GKR_ST_D_READ(inner, i), GKR_WI_D_READ(inner, i), GKR_HELPER_A(inner));

        // Replace the step and width values with 1
        GKR_ST_PUSH(inner, i, CONST_ONE)
        GKR_WI_PUSH(inner, i, CONST_ONE)
    }

    COMP_FUN_END(inner)
}

/**
 * Inverse function of GKCOcompCompressGrid. This function assumes that the function GKCOcompCompressGrid has pushed the
 * ub, st and wi variable identifiers to the corresponding stacks for all dimensions
 *
 * @param outer the outer GPU kernel res, resulting from applying the other pragmas
 * @return the newly computed GPU kernel res, with restored ub, st and wi variable identifiers.
 */
gpukernelres_t*
GKCOcompInvCompressGrid(node* shouldCompress_node, gpukernelres_t* outer) {
    DBUG_ENTER();
    DBUG_PRINT ("compiling UnCompressGrid:");
    DBUG_EXECUTE (
            fprintf(stderr, "    UnCompressGrid (");
            printNumArray(shouldCompress_node);
            fprintf(stderr, ", inner)\n");
    );

    // Get the shouldCompress variable as an array instead of AST nodes
    int* shouldCompress = getNumArrayFromNodes(shouldCompress_node, GKR_DIM(outer), "CompressGrid");

    // To be able to pop the UB, ST and WI variables from the stack in the inverse order in which we pushed,
    // we need to loop the dimensions in inverse order as well
    for (size_t i = GKR_DIM(outer) - 1; i != (size_t) -1; i++) {
        // Skip this iteration if compress for this dimension is toggled off
        if (!shouldCompress[i]) continue;

        // Pop the pre-compress ub, st and wi variable identifiers from the stack
        GKR_UB_POP(outer, i)
        GKR_ST_POP(outer, i)
        GKR_WI_POP(outer, i)

        // If the step is 1, we don't need to compute anything
        if (STReq(GKR_ST_D_READ(outer, i), CONST_ONE)) {}
            // If the width is 1, we only need to handle the step
        else if (STReq(GKR_WI_D_READ(outer, i), CONST_ONE))
            fprintf(global.outfile, "SAC_GKCO_HOST_OPD_COMPRESS_S(%s, %s)\n\n",
                    GKR_ST_D_READ(outer, i), GKR_IDX_D_READ(outer, i));
            // Else, we handle the case where both the step and width are relevant
        else
            fprintf(global.outfile, "SAC_GKCO_HOST_OPD_COMPRESS_SW(%s, %s, %s)\n\n",
                    GKR_ST_D_READ(outer, i), GKR_WI_D_READ(outer, i), GKR_IDX_D_READ(outer, i));
    }

    fprintf(global.outfile, "\n");

    COMP_FUN_END(outer)
}

/**
 * Create a permutation of a strvec. The original vector will be consumed.
 *
 * @param vec The strvec vector to be permuted
 * @param permutation  The new order, given as an array of N_num nodes
 * @return The new permutated strvec
 */
static strvec*
GKCOcompPermuteStrvec(strvec* vec, const int* permutation) {
    DBUG_ENTER();

    strvec* newvec = STRVECempty(STRVEClen(vec));
    for (size_t i = 0; i < STRVEClen(vec); i++) {
        size_t old_index = (size_t) permutation[i];
        STRVECappend(newvec, STRVECsel(vec, old_index));
    }

    STRVECfree(vec);

    DBUG_RETURN(newvec);
}

/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompPermute (node *array, gpukernelres_t *inner)
 *
 * @brief
 *
 * @param num    N_array argument indicating for each axis where it should
 *               come from. All elements have to be N_num here.
 * @param inner  the thread space that results from applying all inner
 *               pragma functions
 *
 * @return the resulting thread space from applying Permute (array) to it.
 ******************************************************************************/
gpukernelres_t*
GKCOcompPermute(node* permutation_node, gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling Permute:");
    DBUG_EXECUTE (
            fprintf(stderr, "    Permute (");
            printNumArray(permutation_node);
            fprintf(stderr, ", inner)\n");
    );

    int* permutation = getNumArrayFromNodes(permutation_node, GKR_DIM(inner), "Permute");

    GKR_LB(inner) = GKCOcompPermuteStrvec(GKR_LB(inner), permutation);
    GKR_UB(inner) = GKCOcompPermuteStrvec(GKR_UB(inner), permutation);
    GKR_ST(inner) = GKCOcompPermuteStrvec(GKR_ST(inner), permutation);
    GKR_WI(inner) = GKCOcompPermuteStrvec(GKR_WI(inner), permutation);

    MEMfree(permutation);

    COMP_FUN_END(inner)
}

/**
 * Create an inverse permutation of a strvec. The original vector will be consumed.
 *
 * @param vec The strvec vector to be permuted
 * @param permutation  The new order (inversed), given as an array of N_num nodes
 * @return The new permutated strvec
 */
static strvec*
GKCOcompInvPermuteStrvec(strvec* vec, const int* permutation) {
    DBUG_ENTER();

    char** newvecarray = (char**) MEMmalloc(sizeof(char*) * STRVEClen(vec));
    for (size_t i = 0; i < STRVEClen(vec); i++) {
        size_t old_index = (size_t) permutation[i];
        newvecarray[old_index] = STRVECsel(vec, i);
    }

    strvec* newvec = STRVECfromArray(newvecarray, STRVEClen(vec));

    MEMfree(newvecarray);
    STRVECfree(vec);

    DBUG_RETURN(newvec);
}

/**
 *
 * @param array
 * @param outer
 * @return
 */
gpukernelres_t*
GKCOcompInvPermute(node* permutation_node, gpukernelres_t* outer) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling inverse Permute:");
    DBUG_EXECUTE (
            fprintf(stderr, "    InvPermute (");
            printNumArray(permutation_node);
            fprintf(stderr, ", outer)\n");
    );

    int* permutation = getNumArrayFromNodes(permutation_node, GKR_DIM(outer), false);

    GKR_LB(outer) = GKCOcompInvPermuteStrvec(GKR_LB(outer), permutation);
    GKR_UB(outer) = GKCOcompInvPermuteStrvec(GKR_UB(outer), permutation);
    GKR_ST(outer) = GKCOcompInvPermuteStrvec(GKR_ST(outer), permutation);
    GKR_WI(outer) = GKCOcompInvPermuteStrvec(GKR_WI(outer), permutation);

    MEMfree(permutation);

    COMP_FUN_END(outer)
}

/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompFoldLast2 (gpukernelres_t *inner)
 *
 * @brief
 *
 * @param inner  the thread space that results from applying all inner
 *               pragma functions
 *
 * @return the resulting thread space from applying FoldLast2 () to it.
 ******************************************************************************/
gpukernelres_t*
GKCOcompFoldLast2(gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling FoldLast2 (inner)");

    COMP_FUN_END(inner)
}

gpukernelres_t*
GKCOcompInvFoldLast2(gpukernelres_t* outer) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling UnfoldLast2 (inner)");

    COMP_FUN_END(outer)
}

/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompSplitLast (node *num, gpukernelres_t *inner)
 *
 * @brief
 *
 * @param array  N_num argument containing the two length of the last
 *               dimension after splitting it up.
 * @param inner  the thread space that results from applying all inner
 *               pragma functions
 *
 * @return the resulting thread space from applying SplitLast (num) to it.
 ******************************************************************************/
gpukernelres_t*
GKCOcompSplitLast(node* num, gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling SplitLast ( %i, inner)", NUM_VAL(num));

    COMP_FUN_END(inner)
}

gpukernelres_t*
GKCOcompInvSplitLast(node* num, gpukernelres_t* outer) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling UnsplitLast ( %i, inner)", NUM_VAL(num));

    COMP_FUN_END(outer)
}

/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompPadLast (node *num, gpukernelres_t *inner)
 *
 * @brief
 *
 * @param array  N_num argument defining the padding size
 *               for the last axis
 * @param inner  the thread space that results from applying all inner
 *               pragma functions
 *
 * @return the resulting thread space from applying Pad (num) to it.
 ******************************************************************************/
gpukernelres_t*
GKCOcompPadLast(node* num, gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling PadLast ( %i, inner)", NUM_VAL(num));

    COMP_FUN_END(inner)
}

gpukernelres_t*
GKCOcompInvPadLast(node* num, gpukernelres_t* outer) {
    DBUG_ENTER();

    DBUG_PRINT ("compiling UnpadLast ( %i, inner)", NUM_VAL(num));

    COMP_FUN_END(outer)
}

#undef DBUG_PREFIX
