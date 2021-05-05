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
 *      // Declare some helper variables. The ret_col variable is used for some optimizations
 *      // (almost branchless implementation).
        SAC_GKCO_OPD_DECLARE(SAC_gkco_prt_5653_tmp)
        SAC_GKCO_OPD_DECLARE(SAC_gkco_prt_5654_tmp)
        SAC_GKCO_OPD_DECLARE(SAC_gkco_prt_5655_ret_col)
        SAC_GKCO_OPM_RETURN_COL_INIT(SAC_gkco_prt_5655_ret_col)

 *      // Declare and redefine upperbound variables, as we cannot modify the original upperbound variables
        SAC_GKCO_OPD_DECLARE(SAC_gkco_prt_5657_ub)
        SAC_GKCO_OPD_REDEFINE(..., SAC_gkco_prt_5657_ub)

        SAC_GKCO_OPD_DECLARE(SAC_gkco_prt_5661_ub)
        SAC_GKCO_OPD_REDEFINE(..., SAC_gkco_prt_5661_ub)

        SAC_GKCO_OPD_DECLARE(SAC_gkco_prt_5665_ub)
        SAC_GKCO_OPD_REDEFINE(..., SAC_gkco_prt_5665_ub)


        // Handle step and width for the first and third dimension (determined by the parameter [1, 0, 1])
        SAC_GKCO_HOST_OPD_COMPRESS_SW_BL(SAC_gkco_prt_5657_ub, SAC_gkco_prt_5657_ub, SAC_gkco_prt_5658_st, SAC_gkco_prt_5659_wi, SAC_gkco_prt_5653_tmp, SAC_gkco_prt_5654_tmp)

        SAC_GKCO_HOST_OPD_COMPRESS_SW_BL(SAC_gkco_prt_5665_ub, SAC_gkco_prt_5665_ub, SAC_gkco_prt_5666_st, SAC_gkco_prt_5667_wi, SAC_gkco_prt_5653_tmp, SAC_gkco_prt_5654_tmp)


        // Make the grid and block spaces
        SAC_GKCO_HOST_OPM_SET_GRID(2147483647   , 65535   , 65535   , 0, SAC_gkco_prt_5657_ub, SAC_gkco_prt_5661_ub)
        SAC_GKCO_HOST_OPM_SET_BLOCK(1024   , 1024   , 64   , 1024, SAC_gkco_prt_5665_ub)

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

 *      // Declare some helper variables. The ret_col variable is used for some optimizations
 *      // (almost branchless implementation).
        SAC_GKCO_OPD_DECLARE(SAC_gkco_prt_5672_tmp)
        SAC_GKCO_OPD_DECLARE(SAC_gkco_prt_5673_tmp)
        SAC_GKCO_OPD_DECLARE(SAC_gkco_prt_5674_ret_col)
        SAC_GKCO_OPM_RETURN_COL_INIT(SAC_gkco_prt_5674_ret_col)

 *      // Declare and redefine upperbound variables, as we cannot modify the original upperbound variables
        SAC_GKCO_OPD_DECLARE(SAC_gkco_prt_5676_ub)
        SAC_GKCO_OPD_REDEFINE(SAC_gkco_prt_5676_ub, SAC_gkco_prt_5676_ub)

        SAC_GKCO_OPD_DECLARE(SAC_gkco_prt_5680_ub)
        SAC_GKCO_OPD_REDEFINE(SAC_gkco_prt_5680_ub, SAC_gkco_prt_5680_ub)

        SAC_GKCO_OPD_DECLARE(SAC_gkco_prt_5684_ub)
        SAC_GKCO_OPD_REDEFINE(SAC_gkco_prt_5684_ub, SAC_gkco_prt_5684_ub)


        // Handle step and width for the first and third dimension (determined by the parameter [1, 0, 1])
        SAC_GKCO_OPD_DECLARE(SAC_gkco_prt_5687_ub)
        SAC_GKCO_HOST_OPD_COMPRESS_SW_BL(SAC_gkco_prt_5676_ub, SAC_gkco_prt_5687_ub, SAC_gkco_prt_5677_st, SAC_gkco_prt_5678_wi, SAC_gkco_prt_5672_tmp, SAC_gkco_prt_5673_tmp)

        SAC_GKCO_OPD_DECLARE(SAC_gkco_prt_5688_ub)
        SAC_GKCO_HOST_OPD_COMPRESS_SW_BL(SAC_gkco_prt_5684_ub, SAC_gkco_prt_5688_ub, SAC_gkco_prt_5685_st, SAC_gkco_prt_5686_wi, SAC_gkco_prt_5672_tmp, SAC_gkco_prt_5673_tmp)

	    // Pass 3: PASS_KERNEL_WLIDS

	    // GridBlock, it's parameter determines how many dimensions are represented in the grid,
	    // and how many in the blocks. As we did not compress the second dimension, the idx var
	    // will be checked against the step and width variables here (in a branchless way)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5689_idx)
		SAC_GKCO_OPD_REDEFINE (THREADIDX_X, SAC_gkco_prt_5689_idx)

		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5690_idx)
		SAC_GKCO_OPD_REDEFINE (THREADIDX_Y, SAC_gkco_prt_5690_idx)

		SAC_GKCO_GPUD_OPD_UNSTEPWIDTH_BL (SAC_gkco_prt_5681_st, SAC_gkco_prt_5682_wi, SAC_gkco_prt_5690_idx, SAC_gkco_prt_5674_ret_col)

		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5691_idx)
		SAC_GKCO_OPD_REDEFINE (BLOCKIDX_X, SAC_gkco_prt_5691_idx)

		// CompressGrid
		SAC_GKCO_GPUD_OPD_COMPRESS_SW (SAC_gkco_prt_5654_st, SAC_gkco_prt_5655_wi, SAC_gkco_prt_5664_idx)
		SAC_GKCO_GPUD_OPD_COMPRESS_SW (SAC_gkco_prt_5662_st, SAC_gkco_prt_5663_wi, SAC_gkco_prt_5666_idx)

		// Gen. All if statements that return are collected and executed at once here, in a single if.
		// This way we have less branches in our code.
		SAC_GKCO_GPUD_OPM_RETURN_IF_COLLECTED (SAC_gkco_prt_5674_ret_col)

        // Declare the index vector
		SAC_GKCO_GPUD_DECLARE_IV(iv_var, 3)
		SAC_GKCO_GPUD_DEF_IV(iv_var, 0, SAC_gcko_prt_5664_idx)
		SAC_GKCO_GPUD_DEF_IV(iv_var, 1, SAC_gcko_prt_5665_idx)
		SAC_GKCO_GPUD_DEF_IV(iv_var, 2, SAC_gcko_prt_5666_idx)

		// End pass

		...
 *  }
 *
 * CODE FILE INDEX:
 *
 *   - Definitions of constants, including the pass constants and flags
 *   - Definition of gpukernelres_t, it's getters and setters, and helper functions/defines
 *   - The pragma traversal loop and the entry functions to this code
 *   - The mappings and their inverses
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

/**********************************************************************************************************************\
************************************************************************************************************************
**
**    Definitions of constants, including the pass constants and flags
**
************************************************************************************************************************
\**********************************************************************************************************************/

#define BRANCHLESS_IMPLEMENTATION true

char* CONST_ZERO               = "0";
char* CONST_ONE                = "1";
char* CONST_VAR_PREFIX         = "SAC_gkco";
char* CONST_UB_POSTFIX         = "ub";
char* CONST_IDX_POSTFIX        = "idx";
char* CONST_TMP_POSTFIX        = "tmp";
char* CONST_RETURN_COL_POSTFIX = "ret_col";

/**
 * Flag locations inside the PASS identifier variables.
 */
// Pass should preserve replaced variable identifiers and values, using the correct stack in the gpukernelres_t
#define PASS_VAL_PRESERVE 1
// Pass should consume replaced variable identifiers and values, using the correct stack in the gpukernelres_t
#define PASS_VAL_CONSUME  2
// Pass is handling and generating code for idx variables
#define PASS_IDX_COMPUTE  4
// Pass should emit code for grid/block dim3 variables
#define PASS_GB_EMIT      8
// Pass should generate pragma errors (to prevent duplicate errors)
#define PASS_CHECK_PRAGMA 16
// Pass should execute branchless. It will be interesting to see how much difference it makes.
#define PASS_BRANCHLESS  (32 * BRANCHLESS_IMPLEMENTATION)

/**
 * Enum to specify a pass over the Pragma functions. These enum value toggle certain functionalities in
 * the compile functions. It is not actually an enum, as ISO C forbids forward declarations of enums.
 */
// (bottom up) Create a thread space to spawn the kernels
#define PASS_HOST               (PASS_CHECK_PRAGMA | PASS_GB_EMIT)
// (bottom up) Recreate the intermediate thread space computation values
#define PASS_KERNEL_THREADSPACE (PASS_BRANCHLESS | PASS_VAL_PRESERVE | PASS_IDX_COMPUTE)
// (top down)  Generate the iv from the kernel coordinates and the pass above
#define PASS_KERNEL_WLIDS       (PASS_BRANCHLESS | PASS_VAL_CONSUME | PASS_IDX_COMPUTE)


/**********************************************************************************************************************\
************************************************************************************************************************
**
**    Definition of gpukernelres_t, it's getters and setters, and helper functions/defines
**
************************************************************************************************************************
\**********************************************************************************************************************/

/**
 * GPU kernel representation struct. It contains vectors and variables to represent all variable identifiers and
 * constants used in the generated code, along with some helper variables.
 *
 * IMPORTANT NOTE: In these compiling functions, the only values that we need to edit are the upperbounds and idx
 * variables. For this this reason, we copy all upperbounds and idx's into new variables _owned by the GPU
 * kernel res_. The other variables are all represented using strings _owned by others_.
 */
struct GPUKERNELRES {
    // Current pass, contains flags for turning specific parts of the code on and off. For the pass documentation,
    // see above.
    pass_t currentpass;
    // The dimensionality
    size_t dim;

    // Variable identifier storage (not idx is only filled and used when PASS_IDX_COMPUTE flag is set)
    strvec* lowerbound;
    strvec* upperbound;
    strvec* step;
    strvec* width;
    strvec* idx;

    // Variable identifier stack, used for recovering variable identifiers when older versions of them
    // are needed.
    // These stacks should be filled when the PASS_VAL_PRESERVE flag is set.
    // These stacks should be consumed when the PASS_VAL_PRESERVE flag is set.
    strvec* lowerbound_at;
    strvec* upperbound_at;
    strvec* step_at;
    strvec* width_at;
    strvec* idx_at;

    // Helper variables. They can be freely used by all GKCOcomp-functions, as long as they are done with it once
    // the function ends.
    char* helper_a;
    char* helper_b;

    // Boolean variable in the generated code that is set to true once the code detects that this thread should not
    // be executed. Only used when the PASS_BRANCHLESS flag is set.
    char* return_collector;

    // Vector with all owned variables, so they can be freed when this struct is cleaned up
    strvec* owned_vars;
};

// Flag macro's
#define GKR_PASS(gkr) gkr->currentpass
#define GKR_CHANGE_PASS(gkr, pass) GKR_PASS(gkr) = pass
#define GKR_VAL_PRESERVE(gkr) (GKR_PASS(gkr) & PASS_VAL_PRESERVE)
#define GKR_VAL_CONSUME(gkr) (GKR_PASS(gkr) & PASS_VAL_CONSUME)
#define GKR_IDX_COMPUTE(gkr) (GKR_PASS(gkr) & PASS_IDX_COMPUTE)
#define GKR_GB_EMIT(gkr) (GKR_PASS(gkr) & PASS_GB_EMIT)
#define GKR_CHECK_PRAGMA(gkr) (GKR_PASS(gkr) & PASS_CHECK_PRAGMA)
#define GKR_BRANCHLESS(gkr) (GKR_PASS(gkr) & PASS_BRANCHLESS)

// DIM getter/setter
#define GKR_DIM(gkr) gkr->dim

// Variable identifier vector getters/setters
#define GKR_LB(gkr) gkr->lowerbound
#define GKR_UB(gkr) gkr->upperbound
#define GKR_ST(gkr) gkr->step
#define GKR_WI(gkr) gkr->width
#define GKR_ID(gkr) gkr->idx

// Variable identifier for specific dimension getters
#define GKR_LB_D_READ(gkr, dim) STRVECsel(GKR_LB(gkr), dim)
#define GKR_UB_D_READ(gkr, dim) STRVECsel(GKR_UB(gkr), dim)
#define GKR_ST_D_READ(gkr, dim) STRVECsel(GKR_ST(gkr), dim)
#define GKR_WI_D_READ(gkr, dim) STRVECsel(GKR_WI(gkr), dim)
#define GKR_ID_D_READ(gkr, dim) STRVECsel(GKR_ID(gkr), dim)

// Variable identifier for specific dimension setters
#define GKR_LB_D_REPLACE(gkr, dim, val) STRVECswap(GKR_LB(gkr), dim, val)
#define GKR_UB_D_REPLACE(gkr, dim, val) STRVECswap(GKR_UB(gkr), dim, val)
#define GKR_ST_D_REPLACE(gkr, dim, val) STRVECswap(GKR_ST(gkr), dim, val)
#define GKR_WI_D_REPLACE(gkr, dim, val) STRVECswap(GKR_WI(gkr), dim, val)
#define GKR_ID_D_REPLACE(gkr, dim, val) STRVECswap(GKR_ID(gkr), dim, val)

// Variable identifier stack getters
#define GKR_LB_AT(gkr) gkr->lowerbound_at
#define GKR_UB_AT(gkr) gkr->upperbound_at
#define GKR_ST_AT(gkr) gkr->step_at
#define GKR_WI_AT(gkr) gkr->width_at
#define GKR_ID_AT(gkr) gkr->idx_at

// Helper variable getters
#define GKR_HELPER_A(gkr) gkr->helper_a
#define GKR_HELPER_B(gkr) gkr->helper_b
#define GKR_RETURN_COL(gkr) gkr->return_collector
#define GKR_OWNED_VARS(gkr) gkr->owned_vars

// Create new variable, and return it
#define GKR_UB_NEW(gkr, dim) NewUpperboundVariable(gkr, dim)

char* NewUpperboundVariable(gpukernelres_t* gkr, size_t dim) {
    DBUG_ENTER();
    char* var;

    if (GKR_VAL_PRESERVE(gkr)) {
        var = GKCOvarCreate(gkr, CONST_UB_POSTFIX);
        GKR_UB_D_REPLACE(gkr, dim, var);
    } else
        var = GKR_UB_D_READ(gkr, dim);

    DBUG_RETURN(var);
}

// Variable identifier stack push functions
#define GKR_LB_PUSH(gkr, dim)                                                                               \
    if (GKR_VAL_PRESERVE(gkr))                                                                              \
        STRVECappend(GKR_LB_AT(gkr), GKR_LB_D_READ(gkr, dim));
#define GKR_UB_PUSH(gkr, dim)                                                                               \
    if (GKR_VAL_PRESERVE(gkr))                                                                              \
        STRVECappend(GKR_UB_AT(gkr), GKR_UB_D_READ(gkr, dim));
#define GKR_ST_PUSH(gkr, dim)                                                                               \
    if (GKR_VAL_PRESERVE(gkr))                                                                              \
        STRVECappend(GKR_ST_AT(gkr), GKR_ST_D_READ(gkr, dim));
#define GKR_WI_PUSH(gkr, dim)                                                                               \
    if (GKR_VAL_PRESERVE(gkr))                                                                              \
        STRVECappend(GKR_WI_AT(gkr), GKR_WI_D_READ(gkr, dim));
#define GKR_ID_PUSH(gkr, dim)                                                                               \
    if (GKR_VAL_PRESERVE(gkr))                                                                              \
        STRVECappend(GKR_ID_AT(gkr), GKR_ID_D_READ(gkr, dim));

// Variable identifier stack pop functions
#define GKR_LB_POP(gkr, dim)                                                                                \
    if (GKR_VAL_CONSUME(gkr))                                                                               \
        STRVECswap(GKR_LB(gkr), dim, STRVECpop(GKR_LB_AT(gkr)));
#define GKR_UB_POP(gkr, dim)                                                                                \
    if (GKR_VAL_CONSUME(gkr))                                                                               \
        STRVECswap(GKR_UB(gkr), dim, STRVECpop(GKR_UB_AT(gkr)));
#define GKR_ST_POP(gkr, dim)                                                                                \
    if (GKR_VAL_CONSUME(gkr))                                                                               \
        STRVECswap(GKR_ST(gkr), dim, STRVECpop(GKR_ST_AT(gkr)));
#define GKR_WI_POP(gkr, dim)                                                                                \
    if (GKR_VAL_CONSUME(gkr))                                                                               \
        STRVECswap(GKR_WI(gkr), dim, STRVECpop(GKR_WI_AT(gkr)));
#define GKR_ID_POP(gkr, dim)                                                                                \
    if (GKR_VAL_CONSUME(gkr))                                                                               \
        STRVECswap(GKR_ID(gkr), dim, STRVECpop(GKR_ID_AT(gkr)));

// Inner loop code for looping over dimensions of a gpukernelres_t. Note that, in order to pop in inverse order
// in which variable identifiers are pushed, the loop should be inversed as well. To simplify the code creation
// for this process, these two macro's were created.
#define LOOP_DIMENSIONS(gkr, dim) size_t dim = 0; (dim) < GKR_DIM(gkr); (dim)++
#define LOOP_DIMENSIONS_INV(gkr, dim) size_t dim = GKR_DIM(gkr) - 1; (dim) != (size_t) -1; (dim)--

/**
 * Macro for a GKCOcomp-function end. It adds some whitespace (in the logs, as well as the generated code),
 * prints the gkr, and returns it.
 */
#define COMP_FUN_DBUG_RETURN(gkr)                                                                           \
    fprintf(global.outfile, "\n");                                                                          \
    DBUG_EXECUTE(PrintGPUkernelres(gkr, stderr));                                                           \
    DBUG_EXECUTE(fprintf(stderr, "\n"));                                                                    \
    DBUG_RETURN(gkr);

/**
 * Make a new gpu kernel representation struct. The vectors containing the variable identifiers and constants will be
 * empty at this point. This is because the variables have to be inserted manually at GKCOcompGen.
 *
 * @param pass              The pass for which the gpu kernel representation is created.
 * @return                  The newly created gpu kernel representation
 */
static gpukernelres_t*
MakeGPUkernelres(pass_t pass) {
    DBUG_ENTER();

    gpukernelres_t* gkr     = (gpukernelres_t*) MEMmalloc(sizeof(gpukernelres_t));

    // This has to be allocated first, as some of the initialization functions below push to it already
    GKR_OWNED_VARS(gkr) = STRVECempty(0);

    GKR_PASS(gkr) = pass;
    GKR_DIM(gkr)  = 0;
    GKR_LB(gkr)   = STRVECempty(0);
    GKR_UB(gkr)   = STRVECempty(0);
    GKR_ST(gkr)   = STRVECempty(0);
    GKR_WI(gkr)   = STRVECempty(0);
    GKR_ID(gkr)   = STRVECempty(0);

    GKR_LB_AT(gkr) = STRVECempty(0);
    GKR_UB_AT(gkr) = STRVECempty(0);
    GKR_ST_AT(gkr) = STRVECempty(0);
    GKR_WI_AT(gkr) = STRVECempty(0);
    GKR_ID_AT(gkr) = STRVECempty(0);

    GKR_HELPER_A(gkr) = GKCOvarCreate(gkr, CONST_TMP_POSTFIX);
    GKR_HELPER_B(gkr) = GKCOvarCreate(gkr, CONST_TMP_POSTFIX);
    if (GKR_BRANCHLESS(gkr))
        GKR_RETURN_COL(gkr) = GKCOvarCreate(gkr, CONST_RETURN_COL_POSTFIX);

    DBUG_RETURN(gkr);
}

/**
 * Free a GPU kernel res. All variables owned by the gpu kernel res are stored in the vector GKR_OWNED_VARS(gkr).
 * This vector will be deep-freed, while all other vectors are shallow freed. The single helper variables don't
 * have to be freed either, for the same reason.
 *
 * @param gkr               The struct to be freed
 * @return                  The null pointer
 */
static gpukernelres_t*
FreeGPUkernelres(gpukernelres_t* gkr) {
    DBUG_ENTER();

    // Clean up the internal vectors
    STRVECfree(GKR_LB(gkr));
    STRVECfree(GKR_UB(gkr));
    STRVECfree(GKR_ST(gkr));
    STRVECfree(GKR_WI(gkr));
    STRVECfree(GKR_ID(gkr));

    STRVECfree(GKR_LB_AT(gkr));
    STRVECfree(GKR_UB_AT(gkr));
    STRVECfree(GKR_ST_AT(gkr));
    STRVECfree(GKR_WI_AT(gkr));

    // We do not free the helper variables, because they are already in owned_vars

    // Clean up all variables owned by the gpu kernel res, and the vector itself as well
    STRVECfreeDeep(GKR_OWNED_VARS(gkr));

    // Clean up the datastructure itself
    MEMfree(gkr);

    DBUG_RETURN(NULL);
}

/**
 * Add a dimension to the GPU kernel representation.
 *
 * If PASS_VAL_CONSUME is set, we are restoring a dimension that has disappeared in a previous pass. In this case, we
 * pop the lb, ub, st and wi values from their stacks.
 * If PASS_VAL_CONSUME is not set, lb will be set to 0, and step and width will be set to 1. A new variable for ub will
 * be created and declared in the generated code. It will not be set to a value in the generated code, however.
 * If PASS_IDX_COMPUTE is set, we do the same for the id variable.
 *
 * @param gkr               The GPU kernel representation
 */
static void
AddDimension(gpukernelres_t* gkr) {
    DBUG_ENTER();

    GKR_DIM(gkr)++;

    if (!GKR_VAL_CONSUME(gkr)) {
        // We are not consuming the stacks, so new variables have to be created
        STRVECappend(GKR_LB(gkr), CONST_ZERO);
        STRVECappend(GKR_UB(gkr), GKCOvarCreate(gkr, CONST_UB_POSTFIX));
        STRVECappend(GKR_ST(gkr), CONST_ONE);
        STRVECappend(GKR_WI(gkr), CONST_ONE);
        // Handle the id variable only when needed
        if (GKR_IDX_COMPUTE(gkr))
            STRVECappend(GKR_ID(gkr), GKCOvarCreate(gkr, CONST_IDX_POSTFIX));
    } else {
        // We are consuming the stacks, so we use these variables to fill the new dimension
        size_t new_dim = GKR_DIM(gkr) - 1;
        STRVECappend(GKR_LB(gkr), NULL);
        STRVECappend(GKR_UB(gkr), NULL);
        STRVECappend(GKR_ST(gkr), NULL);
        STRVECappend(GKR_WI(gkr), NULL);
        GKR_LB_POP(gkr, new_dim)
        GKR_UB_POP(gkr, new_dim)
        GKR_ST_POP(gkr, new_dim)
        GKR_WI_POP(gkr, new_dim)
        // Handle the id variable only when needed
        if (GKR_IDX_COMPUTE(gkr)) {
            STRVECappend(GKR_ID(gkr), NULL);
            GKR_ID_POP(gkr, new_dim);
        }
    }

    DBUG_RETURN();
}

/**
 * Remove a dimension from the GPU kernel representation.
 *
 * If PASS_VAL_PRESERVE is set, the variables lb, ub, st and wi will be pushed to their stacks.
 * If not, they simply disappear.
 * If PASS_IDX_COMPUTE is set, we do the same for the id variable.
 *
 * @param gkr               The GPU kernel representation from which the dimension has to be removed
 */
static void
RemoveDimension(gpukernelres_t* gkr) {
    DBUG_ENTER();

    if (GKR_VAL_PRESERVE(gkr)) {
        // If we preserve the variables, push them to their stacks first
        size_t old_dim = GKR_DIM(gkr) - 1;
        GKR_LB_PUSH(gkr, old_dim)
        GKR_UB_PUSH(gkr, old_dim)
        GKR_ST_PUSH(gkr, old_dim)
        GKR_WI_PUSH(gkr, old_dim)
        // Handle the id variable only when needed
        if (GKR_IDX_COMPUTE(gkr))
            GKR_ID_PUSH(gkr, old_dim)
    }

    // Remove the last dimension from all variable arrays
    STRVECpop(GKR_LB(gkr));
    STRVECpop(GKR_UB(gkr));
    STRVECpop(GKR_ST(gkr));
    STRVECpop(GKR_WI(gkr));
    // Handle the id variable only when needed
    if (GKR_IDX_COMPUTE(gkr))
        STRVECpop(GKR_ID(gkr));

    GKR_DIM(gkr)--;

    DBUG_RETURN();
}

/**
 * Helper function for var creation. This version can also be used when no gpu kernel res is available, but variable
 * string memory management is not handled implicitly.
 *
 * @param postfix           The postfix to be added
 * @return                  The properly pre- and postfixed variable name.
 */
char*
VarCreate(char* postfix) {
    DBUG_ENTER();

    // Create the new variable name
    char* without_prefix = TRAVtmpVarName(postfix);
    char* var            = STRcat(CONST_VAR_PREFIX, without_prefix);
    // Free up the intermediate string
    MEMfree(without_prefix);

    // Declare the new variable
    INDENT
    fprintf(global.outfile, "SAC_GKCO_OPD_DECLARE(%s)\n", var);

    DBUG_RETURN(var);
}

/**
 * Create a new temporary variable, with proper pre- and postfixes. The prefix is fixed to CONST_PREFIX.
 * The postfix is an argument, as this function can be used for different purpose variables.
 * The variable is automatically declared in the generated code.
 *
 * @param gkr               The gpu kernel res used for string variable management
 * @param postfix           The postfix to be added
 * @return                  The properly pre- and postfixed variable name.
 */
char*
GKCOvarCreate(gpukernelres_t* gkr, char* postfix) {
    DBUG_ENTER();

    char* var = VarCreate(postfix);

    // Append the variable to the list of owned variable names
    STRVECappend(GKR_OWNED_VARS(gkr), var);

    DBUG_RETURN(var);
}

/**
 * Transform a node array into an integer array. An integer array is easier to work with,
 * especially in cases where the nodes cannot be accessed sequentially in order.
 *
 * @param nums_node         The node to be transformed into an array
 * @param length            The expected length of the array. If the actual length of the node is different,
 *                          a compiler error will be thrown.
 * @param pragma            The pragma name, used for the error generation.
 * @return                  An integer array with the values from the nodes array.
 */
static int*
getNumArrayFromNodes(node* nums_node, size_t length, char* pragma) {
    DBUG_ENTER ();

    // Check the expected length against the actual length
    checkArgsLength(nums_node, length, pragma);

    // Get the correct AST node, and create the array
    node* exprs = ARRAY_AELEMS(nums_node);
    int * array = (int*) MEMmalloc(sizeof(int*) * length);

    // Fill up the array
    for (size_t i = 0; i < length; i++) {
        node* arg = EXPRS_EXPR(exprs);
        array[i] = NUM_VAL(arg);
        exprs = EXPRS_NEXT(exprs);
    }

    DBUG_RETURN(array);
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

    size_t linesize = 80;
    fprintf(stream, "    GPU kernelres (dim: %zu)\n", GKR_DIM(gkr));
    fprintf(stream, "    lb  = ");
    STRVECprint(GKR_LB(gkr), stream, linesize, 9);
    fprintf(stream, "    ub  = ");
    STRVECprint(GKR_UB(gkr), stream, linesize, 9);
    fprintf(stream, "    st  = ");
    STRVECprint(GKR_ST(gkr), stream, linesize, 9);
    fprintf(stream, "    wi  = ");
    STRVECprint(GKR_WI(gkr), stream, linesize, 9);
    fprintf(stream, "    idx = ");
    STRVECprint(GKR_ID(gkr), stream, linesize, 9);
    fprintf(stream, "\n");
    fprintf(stream, "    lbt = ");
    STRVECprint(GKR_LB_AT(gkr), stream, linesize, 9);
    fprintf(stream, "    ubt = ");
    STRVECprint(GKR_UB_AT(gkr), stream, linesize, 9);
    fprintf(stream, "    stt = ");
    STRVECprint(GKR_ST_AT(gkr), stream, linesize, 9);
    fprintf(stream, "    wit = ");
    STRVECprint(GKR_WI_AT(gkr), stream, linesize, 9);
    fprintf(stream, "    idt = ");
    STRVECprint(GKR_ID_AT(gkr), stream, linesize, 9);
    fprintf(stream, "    helper_a = %s", GKR_HELPER_A(gkr));
    fprintf(stream, "\n");
    fprintf(stream, "    own = ");
    STRVECprint(GKR_OWNED_VARS(gkr), stream, linesize, 9);
    fprintf(stream, "\n");

    DBUG_RETURN();
}


/** <!-- ****************************************************************** -->
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

/**********************************************************************************************************************\
************************************************************************************************************************
**
**    The pragma traversal loop and the entry functions to this code
**
************************************************************************************************************************
\**********************************************************************************************************************/

/** <!-- ****************************************************************** -->
 *
 * @brief generates the actual function dispatch based on the info in
 *        gpukernel_funs.mac. It also handles the special case of the
 *        innermost N_spid "Gen" as a call to GKCOcompGen.
 *
 *        This version handles the bottom-up dispatch order used in
 *        the first and second passes. It calls the non-inverse versions
 *        of the GKCOcomp-functions.
 *
 * @param spap   either N_spap of an outer function or N_spid for "Gen"
 * @param res    the gpukernelres_t that has to be passed through
 * @param bnum   number of bound elements (== number of strings in bounds)
 * @param bounds the actual bounds as strings, these are either SAC runtime
 *               variable reads or constants.
 *
 * @return the result of the dispatched function call.
 ******************************************************************************/
static
gpukernelres_t*
dispatch(node* spap, gpukernelres_t* res, unsigned int bnum, char** bounds) {

    DBUG_ENTER ();

    // First, we handle the gen case separately, as we stop the recursion here
    if (NODE_TYPE (spap) == N_spid) {
        res = GKCOcompGen(bnum, bounds, res);
    }

// Macro's for expanding the arguments into the GKCOcomp call
#define ARGS(nargs) ARG##nargs
#define ARG0
#define ARG1 EXPRS_EXPR (SPAP_ARGS (spap)),
// Macro's for skipping the arguments when recursively calling the dispatch function
#define SKIPS(nargs) SKIP##nargs
#define SKIP0
#define SKIP1 EXPRS_NEXT
// We now define the else-if clause as a WLP macro. Then, below, we include the
// "gpukernel_funs.mac" file, which calls the WLP macro for each available mapping.
// Note that we are lacing through the res variable, containing the current
// gpu kernel res
#define WLP(fun, nargs, checkfun)                                                                           \
    else if (STReq (SPAP_NAME (spap), #fun)) {                                                              \
        DBUG_ASSERT ((SPAP_ARGS (spap) != NULL), "missing argument in `%s' ()", #fun);                      \
        /* Because this is the bottom-up version, we do the recursive dispatch call first                   \
         * We give it the inner pragma call to the mapping. For this we first have to                       \
         * skip the arguments for the GKCOcomp function. We use the macros defined above                    \
         * for this. */                                                                                     \
        res = dispatch (EXPRS_EXPR ( SKIPS( nargs) (SPAP_ARGS (spap))),                                     \
                                          res, bnum, bounds);                                               \
        /* After the recursive call, we call the actual mapping function with the correct                   \
         * number of arguments. We use the macros defined above for this. */                                \
        res = GKCOcomp ## fun ( ARGS( nargs) res);                                                          \
    }
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

        // Finally, if none of the cases above were fired, we must have an undefined macro.
    else {
        DBUG_ASSERT(0 == 1, "expected gpukernel function, found `%s'", SPAP_NAME(spap));
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @brief generates the actual function dispatch based on the info in 
 *        gpukernel_funs.mac. It also handles the special case of the
 *        innermost N_spid "Gen" as a call to GKCOcompGen.
 *
 *        This version handles the top-down dispatch order used in
 *        the third pass. It calls the inverse versions of
 *        the GKCOcomp-functions.
 *
 * @param spap   either N_spap of an outer function or N_spid for "Gen"
 * @param iv_var the variable identifier for the iv vector, which has to be
 *               created and filled
 * @param res    the gpukernelres_t that has to be passed through
 *
 * @return the result of the dispatched function call.
 ******************************************************************************/
static
gpukernelres_t*
dispatchInv(node* spap, char* iv_var, char** bounds, gpukernelres_t* res) {

    DBUG_ENTER ();

    // First, we handle the gen case separately, as we stop the recursion here
    if (NODE_TYPE (spap) == N_spid) {
        res = GKCOcompInvGen(iv_var, bounds, res);
    }

// Macro's for expanding the arguments into the GKCOcomp call
#define ARGS(nargs) ARG##nargs
#define ARG0
#define ARG1 EXPRS_EXPR (SPAP_ARGS (spap)),
// Macro's for skipping the arguments when recursively calling the dispatch function
#define SKIPS(nargs) SKIP##nargs
#define SKIP0
#define SKIP1 EXPRS_NEXT
// We now define the else-if clause as a WLP macro. Then, below, we include the
// "gpukernel_funs.mac" file, which calls the WLP macro for each available mapping.
// Note that we are lacing through the res variable, containing the current
// gpu kernel res
#define WLP(fun, nargs, checkfun)                                                                           \
    else if (STReq (SPAP_NAME (spap), #fun)) {                                                              \
        DBUG_ASSERT ((SPAP_ARGS (spap) != NULL), "missing argument in `%s' ()", #fun);                      \
        /* Because this is the top-down version, we call the actual mapping function first.                 \
         * We give it the correct number of arguments. For this, we use the macros defined                  \
         * above */                                                                                         \
        res = GKCOcompInv ## fun (ARGS(nargs) res);                                                         \
        /* After the mapping function call, we do the recursive dispatch call. We give it                   \
         * the inner pragma call to the mapping. For this, we have to skip the arguments                    \
         * for the GKCOcomp function. We use the macros defined above for this. */                          \
        res = dispatchInv (EXPRS_EXPR ( SKIPS( nargs) (SPAP_ARGS (spap))),                                  \
                       iv_var, bounds, res);                                                          \
    }
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

        // Finally, if none of the cases above were fired, we must have an undefined macro.
    else {
        DBUG_ASSERT(0 == 1, "expected gpukernel function, found `%s'", SPAP_NAME(spap));
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @brief generates the actual function nesting for the PASS_HOST pass. It creates
 *        the gpu kernel res, and stores the pass information inside it. Then it
 *        calls the recursive helper function dispatch and cleans up the gpu
 *        kernel res again.
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

    gpukernelres_t* res = MakeGPUkernelres(PASS_HOST);
    res = dispatch(spap, res, bnum, bounds);

    FreeGPUkernelres(res);

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 *
 * @brief generates the actual function nesting for the PAST_KERNEL_* passes.
 *        It creates the gpu kernel res, stores the pass information inside
 *        it and calls the recursive helper function dispatch. The GPUD version
 *        of the pragma function needs a second pass, so it updates the pass
 *        information in the gpu kernel res and calls the inverse dispatch
 *        function this time. Lastly, the gpu kernel res function is freed.
 *
 * @result Prints the code necessary to compute the transformed real index vector
 *         from the grid/block coordinates
 *
 * @param spap              N_spap of the outer function GridBlock(...)
 * @param iv_var            the variable identifier for the iv vector, which has to be
 *                          created and filled
 * @param bnum              number of bound elements (== number of strings in bounds)
 * @param bounds            the actual bounds as strings, these are either SAC runtime
 *                          variable reads or constants.
 ******************************************************************************/
void
GKCOcompGPUDkernelPragma(node* spap, char* iv_var, unsigned int bnum, char** bounds) {
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

    gpukernelres_t* res = MakeGPUkernelres(PASS_KERNEL_THREADSPACE);
    res = dispatch(spap, res, bnum, bounds);

    GKR_CHANGE_PASS(res, PASS_KERNEL_WLIDS);
    res = dispatchInv(spap, iv_var, bounds, res);

    DBUG_EXECUTE(GKCOcompCheckGPUkernelRes(bnum, bounds, res););
    FreeGPUkernelres(res);

    DBUG_RETURN ();
}

/**********************************************************************************************************************\
************************************************************************************************************************
**
**    The mappings and their inverses
**
**    An explanation of each mapping can be found for the non-inverse function. The comments
**    for the inverse functions will be almost empty. Note that the ascii art in each
**    comment draws pass 1 and 3. Pass 2 is the same as pass 1, but executed for each thread
**    individually.
**
**    Also look at the pass configuration at the top of this file, as there are many
**    side-effects in the low-level functions that are toggled by setting or unsetting pass
**    flags.
**
************************************************************************************************************************
\**********************************************************************************************************************/

/**
 * The GridBlock mapping is responsible for handing over the gpukernelres information to
 * cuda (in the non-inverse call), and retrieving the transformed idx variables back
 * from the kernel coordinates (in the inverse call).
 *
 *              GridBlock                                         InvGridBlock
 *                                                   /--> Kernel      -->        id0...idn
 * ub0...ubn       -->   Cuda grid & block --> CUDA ----> Kernel      -->        id0...idn
 *                                                   \--> Kernel      -->        id0...idn
 *
 * Notes:
 *   - The id variables are already defined in pass 2. More info on that in GKCOcompGen
 *   - In pass 2, we are only interested in the lb, ub, st and wi variables, so the
 *     GridBlock pass is disabled there (toggled with PASS_GB_EMIT)
 *
 * @param gridDims          N_num node indicating the number of dimensions in the grid
 * @param inner             The gpu kernel res, resulting from calling all inner pragma
 *                          calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompGridBlock(node* gridDims, gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling GridBlock ( %i, inner)", NUM_VAL(gridDims));

    // Check that the paramter is less or equal to the number of dimensions
    checkNumLesseqDim(gridDims, GKR_DIM(inner), "GridBlock");

    // The ICM's to be used for either the grid or the block
    char* icm[2] = {"SAC_GKCO_HOST_OPM_SET_GRID", "SAC_GKCO_HOST_OPM_SET_BLOCK"};
    // The range of grid and block dimensions. Grid: 0 - gridDims, Block: gridDims - dimensionality
    size_t       from[2]  = {0, (size_t) NUM_VAL(gridDims)};
    size_t       to[2]    = {(size_t) NUM_VAL(gridDims), GKR_DIM(inner)};
    // The maximum number of threads per dimension, and the totals
    unsigned int max_s[8] = {
            global.cuda_options.cuda_max_x_grid, global.cuda_options.cuda_max_xy_block,  // max x
            global.cuda_options.cuda_max_yz_grid, global.cuda_options.cuda_max_xy_block, // max y
            global.cuda_options.cuda_max_yz_grid, global.cuda_options.cuda_max_z_block,  // max z
            0, global.cuda_options.cuda_max_threads_block,                               // max product (only for block)
    };

    // Toggle the emitting of the grid/block variable code
    size_t gba = GKR_GB_EMIT(inner) ? 2 : 0;

    // Two iterations, one for the grid and one for the block. Inside the loop, the arrays above will determine the
    // actual values that are used.
    for (size_t gb = 0; gb < gba; gb++) {
        INDENT
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

    COMP_FUN_DBUG_RETURN(inner)
}

/**
 * Inverse function of GKCOcompGridBlock.
 *
 * @param num               The number of grid dimensions
 * @param outer             The GPU kernel res created by the PASS_KERNEL_THREADSPACE pass.
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompInvGridBlock(node* num, gpukernelres_t* outer) {
    DBUG_ENTER();
    DBUG_PRINT("compiling idx variable generation");

    // The range of grid and block dimensions. Grid: 0 - gridDims, Block: gridDims - dimensionality
    size_t from[2] = {0, (size_t) NUM_VAL(num)};
    size_t to[2]   = {(size_t) NUM_VAL(num), GKR_DIM(outer)};
    // The thread coordinate variables, also ICM's
    char* grid_block_var[6] = {
            "THREADIDX_X", "BLOCKIDX_X",
            "THREADIDX_Y", "BLOCKIDX_Y",
            "THREADIDX_Z", "BLOCKIDX_Z",
    };

    // Two iterations, one for the grid and one for the block. Inside the loop, the arrays above will determine the
    // actual values that are used.
    for (size_t gb = 0; gb < 2; gb++) {
        // Print the dynamic arguments. From and to (defined above) determine what dimensions belong to the grid/block
        for (size_t dim = from[gb]; dim < to[gb]; dim++) {
            size_t xyz_int = dim - from[gb];
            INDENT
            fprintf(global.outfile, "SAC_GKCO_OPD_REDEFINE(%s, %s)\n\n",
                    grid_block_var[2 * xyz_int + gb], GKR_ID_D_READ(outer, dim));

            if (!STReq(GKR_ST_D_READ(outer, dim), CONST_ONE)) {
                INDENT
                if (!GKR_BRANCHLESS(outer))
                    fprintf(global.outfile, "SAC_GKCO_GPUD_OPD_UNSTEPWIDTH(%s, %s, %s)\n\n",
                            GKR_ST_D_READ(outer, dim), GKR_WI_D_READ(outer, dim), GKR_ID_D_READ(outer, dim));
                else
                    fprintf(global.outfile, "SAC_GKCO_GPUD_OPD_UNSTEPWIDTH_BL(%s, %s, %s, %s)\n\n",
                            GKR_ST_D_READ(outer, dim), GKR_WI_D_READ(outer, dim),
                            GKR_ID_D_READ(outer, dim), GKR_RETURN_COL(outer));
            }
        }
    }

    COMP_FUN_DBUG_RETURN(outer)
}

/**
 * The Gen mapping is responsible for starting the pragma traversal by initiating
 * the gpu kernel res (in the non-inverse call), and creating the IV variable
 * (in the inverse call).
 *
 *                    Gen                                                 GenInv
 *                                                      ... --> id0..idn   -->    iv[n]
 * m, bound0..boundm  -->  lb0..lbn, ub.. st.. wi.. --> ... --> id0..idn   -->    iv[n]
 *                                                      ... --> id0..idn   -->    iv[n]
 *
 * Note:
 *   - The id0..idn variable identifiers used to store the individual components of the
 *     iv var are used in later parts of the program. These are the variables that are
 *     known just before GenInv starts, and are used to set the actual iv variable.
 *     because of this, we need to back-compute what variables these are in the outer
 *     pragma calls. We do this by defining the id0..idn variables in Gen already,
 *     so they can be transformed alongside the lb, ub, st and wi.
 *   - When running in Branchless mode (PASS_BRANCHLESS), Gen sets a Return Collector
 *     variable. Every time a function would have a `if (expr) return;` statement,
 *     it can just do `ret_col |= expr;`. GenInv checks this variable, and returns if
 *     it is set to true. This eliminates all but one branch in the code.
 *
 * @param bnum              The number of bound strings provided in bounds. This is
 *                          always a multiple of 4 or 5, because we always provide
 *                          either lb, ub, st, wi or lb, ub, st, wi, id for each o
 *                          dimension. The id variable is enabled by a pass flag called
 *                          PASS_IDX_COMPUTE.
 * @param bounds            The actual bound strings. The order of the strings is:
 *                          lb0..lbn, ub0..ubn, st0..stn, wi0..win, [id0..idn]
 * @param inner             The gpu kernel res, resulting from calling all inner pragma
 *                          calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompGen(unsigned int bnum, char** bounds, gpukernelres_t* inner) {
#ifndef DBUG_OFF
#endif
    DBUG_ENTER ();
    DBUG_PRINT ("compiling Gen:");
    DBUG_EXECUTE (
            fprintf(stderr, "\n    Gen ( %u", bnum);
            for (unsigned int i = 0; i < bnum; i++) {
                fprintf(stderr, ", %s", bounds[i]);
            }
            fprintf(stderr, ")\n");
    );
    fprintf(global.outfile, "\n");


    // Divide the number of arguments by the number of variables per dimension.
    // For the first  pass, these are 4: lb, ub, st, wi
    // For the second pass, these are 5: lb, ub, st, wi, idx
    GKR_DIM(inner) = bnum / (GKR_IDX_COMPUTE(inner) ? 5 : 4);

    // Initiate return collector when in branchless mode
    if (GKR_BRANCHLESS(inner)) {
        INDENT
        fprintf(global.outfile, "SAC_GKCO_OPM_RETURN_COL_INIT(%s)\n\n",
                GKR_RETURN_COL(inner));
    }

    for (LOOP_DIMENSIONS(inner, dim)) {
        // For each dimension, we add the lb, ub, st, wi, and if PASS_IDX_COMPUTE is set also the id variables
        // Because we are going to change the ub variable, we introduce a new one so the old one does not get
        // overwritten.
        STRVECappend(GKR_LB(inner), bounds[dim + 0 * GKR_DIM(inner)]);
        STRVECappend(GKR_UB(inner), GKCOvarCreate(inner, CONST_UB_POSTFIX));
        STRVECappend(GKR_ST(inner), bounds[dim + 2 * GKR_DIM(inner)]);
        STRVECappend(GKR_WI(inner), bounds[dim + 3 * GKR_DIM(inner)]);
        if (GKR_IDX_COMPUTE(inner))
            STRVECappend(GKR_ID(inner), bounds[dim + 4 * GKR_DIM(inner)]);

        // Initiate the new ub variable with the value of the old one
        INDENT
        fprintf(global.outfile, "SAC_GKCO_OPD_REDEFINE(%s, %s)\n",
                bounds[dim + 1 * GKR_DIM(inner)], GKR_UB_D_READ(inner, dim));

        // If we have an idx variable, it needs to be declared.
        if (GKR_IDX_COMPUTE(inner)) {
            INDENT
            fprintf(global.outfile, "SAC_GKCO_OPD_DECLARE(%s)\n",
                    GKR_ID_D_READ(inner, dim));
        }

        fprintf(global.outfile, "\n");
    }

    GKCOcompCheckStart(inner);

    COMP_FUN_DBUG_RETURN(inner);
}

/**
 * Inverse function of GKCOcompGen
 *
 * @param iv_var            The iv variable identifier
 * @param outer             The GPU kernel res, resulting from calling all outer pragma calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompInvGen(char* iv_var, char** bounds, gpukernelres_t* outer) {
    DBUG_ENTER();
    DBUG_PRINT ("compiling IV generation ():");

    // Check return collector if in branchless mode
    if (GKR_BRANCHLESS(outer)) {
        INDENT
        fprintf(global.outfile, "SAC_GKCO_GPUD_OPM_RETURN_IF_COLLECTED(%s)\n\n",
                GKR_RETURN_COL(outer));
    }

    // Declare the iv variable
    INDENT
    fprintf(global.outfile, "SAC_GKCO_GPUD_OPM_DECLARE_IV(%s, %zu)\n\n",
            iv_var, GKR_DIM(outer));

    // Fill the iv variable
    for (LOOP_DIMENSIONS(outer, dim)) {
        INDENT
        fprintf(global.outfile, "SAC_GKCO_GPUD_OPD_DEF_IV(%s, %zu, %s, %s, %s, %s, %s, %s, %s, %s, %s)\n\n",
                iv_var, dim,
                GKR_LB_D_READ(outer, dim), GKR_UB_D_READ(outer, dim),
                GKR_ST_D_READ(outer, dim), GKR_WI_D_READ(outer, dim),
                GKR_ID_D_READ(outer, dim),
                bounds[0 * GKR_DIM(outer) + dim],
                bounds[1 * GKR_DIM(outer) + dim],
                bounds[2 * GKR_DIM(outer) + dim],
                bounds[3 * GKR_DIM(outer) + dim]);
    }

    GKCOcompCheckKernel(outer);

    COMP_FUN_DBUG_RETURN(outer);
}

/**
 * StepWidth is an implicit mapping that gets executed whenever step/width information cannot be preserved, for example
 * when folding/splitting. It will set step and width to 1 (in the non-inverse call), restore the step/width
 * variables and return all threads that fall outside of the grid (in the inverse call). This function will only
 * check a single dimension.
 *
 *          StepWidth                            InvStepWidth
 *                             ... --> 1, 1, idj      -->     return;           // idj not in step/width
 * stj, wij    -->    1, 1 --> ... --> 1, 1, idj      -->     stj, wij, idj     // idj in step/width
 *                             ... --> 1, 1, idj      -->     stj, wij, idj     // idj in step/width
 *
 * Notes:
 *   - When running in branchless mode (PASS_BRANCHLESS), an alternative icm for InvStepWidth is used. See CKCOcompGen
 *     for more info.
 *   - When step is already 1, nothing will be done. Width will be set to 1 in that case regardless.
 *   - Resolving step/width by this method results in redundant threads. Use the CompressGrid mapping to create a dense
 *     thread space without redundant threads.
 *
 * @param dim               The dimension for which step and width have to be 1
 * @param inner             The gpu kernel res, resulting from calling all inner pragma
 *                          calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompStepWidth(size_t dim, gpukernelres_t* inner) {
    DBUG_ENTER();

    GKR_ST_PUSH(inner, dim)
    GKR_WI_PUSH(inner, dim)

    GKR_ST_D_REPLACE(inner, dim, CONST_ONE);
    GKR_WI_D_REPLACE(inner, dim, CONST_ONE);

    DBUG_RETURN(inner);
}

/**
 * Inverse of GKCOcompStepWidth
 *
 * @param dim               The dimension for which step and width have to be 1
 * @param outer             The GPU kernel res, resulting from calling all outer pragma calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompInvStepWidth(size_t dim, gpukernelres_t* outer) {
    DBUG_ENTER();

    GKR_ST_POP(outer, dim);
    GKR_WI_POP(outer, dim);

    // If the step is already 1 after popping, we don't have to do anything
    if (!STReq(GKR_ST_D_READ(outer, dim), CONST_ONE)) {
        INDENT
        // Print out the correct ICM depending on whether we are running in branchless mode
        if (!GKR_BRANCHLESS(outer))
            fprintf(global.outfile, "SAC_GKCO_GPUD_OPD_UNSTEPWIDTH(%s, %s, %s)\n\n",
                    GKR_ST_D_READ(outer, dim), GKR_WI_D_READ(outer, dim), GKR_ID_D_READ(outer, dim));
        else
            fprintf(global.outfile, "SAC_GKCO_GPUD_OPD_UNSTEPWIDTH_BL(%s, %s, %s, %s)\n\n",
                    GKR_ST_D_READ(outer, dim), GKR_WI_D_READ(outer, dim),
                    GKR_ID_D_READ(outer, dim), GKR_RETURN_COL(outer));
    }

    DBUG_RETURN(outer);
}

/**
 * Pad is an implicit mapping that gets executed whenever a dimension should have a certain length, for example for the
 * SplitLast mapping. It will extend the upperbound to a multiple of the division parameter (in the non-inverted call),
 * restore the upperbound variables and return all threads that fall outside of the upperbound. This function will only
 * check a single dimension.
 *
 *    Pad                     InvPad
   *                ... --> ubj', idj   -->  return;    // idj >= ubj
 * ubj --> ubj' --> ... --> ubj', idj   -->  ubj, idj   // idj < ubj
   *                ... --> ubj', idj   -->  ubj, idj   // idj < ubj
 *
 * @param dim               The dimension for which the upperbound should be divisible by `divisibility`
 * @param divisiblity       The integer which should be able to divide the padded upperbound
 * @param inner             The gpu kernel res, resulting from calling all inner pragma calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompPad(size_t dim, size_t divisiblity, gpukernelres_t* inner) {
    DBUG_ENTER();

    GKR_UB_PUSH(inner, dim)

    char* ub_read  = GKR_UB_D_READ(inner, dim);
    char* ub_write = GKR_UB_NEW(inner, dim);
    INDENT
    fprintf(global.outfile, "SAC_GKCO_HOST_OPD_PAD(%s, %s, %zu)\n\n",
            ub_read, ub_write, divisiblity);

    DBUG_RETURN(inner);
}

/**
 * Inverse of GKCOcompPad
 *
 * @param dim               The dimension for which the upperbound should be divisible by `divisibility`
 * @param outer             The GPU kernel res, resulting from calling all outer pragma calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompInvPad(size_t dim, gpukernelres_t* outer) {
    DBUG_ENTER();

    GKR_UB_POP(outer, dim);
    INDENT
    if (!GKR_BRANCHLESS(outer))
        fprintf(global.outfile, "SAC_GKCO_GPUD_OPD_UNPAD(%s, %s)\n\n",
                GKR_UB_D_READ(outer, dim), GKR_ID_D_READ(outer, dim));
    else
        fprintf(global.outfile, "SAC_GKCO_GPUD_OPD_UNPAD_BL(%s, %s, %s)\n\n",
                GKR_UB_D_READ(outer, dim), GKR_ID_D_READ(outer, dim), GKR_RETURN_COL(outer));

    DBUG_RETURN(outer);
}

/**
 * The ShiftLB mapping shifts the index space so that all lowerbounds are 0 (for the non-inverted call), and shifts it
 * back to its original position (for the inverted call). It does do so for all dimensions where the lowerbound is not
 * already set to 0.
 *
 * For each dimension j:
 *          ShiftLB                                            InvShiftLB
 *                                 ... --> 0, ubj-lbj, idj-lbj     -->    lbj, ubj, idj
 * lbj, ubj   -->   0, ubj-lbj --> ... --> 0, ubj-lbj, idj-lbj     -->    lbj, ubj, idj
 *                                 ... --> 0, ubj-lbj, idj-lbj     -->    lbj, ubj, idj
 *
 * @param inner             The gpu kernel res, resulting from calling all inner pragma
 *                          calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompShiftLB(gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling ShiftLB (inner):");

    for (LOOP_DIMENSIONS(inner, dim)) {
        // Preserve the current variable identifiers for lb and ub
        GKR_LB_PUSH(inner, dim)
        GKR_UB_PUSH(inner, dim)

        // We only have to handle dimensions that aren't already normalized
        if (STReq(GKR_LB_D_READ(inner, dim), CONST_ZERO)) continue;

        // Get the correct variable identifiers
        char* lb       = GKR_LB_D_REPLACE(inner, dim, CONST_ZERO);
        char* ub_read  = GKR_UB_D_READ(inner, dim);
        char* ub_write = GKR_UB_NEW(inner, dim);
        // Compute the new upperbound variable
        INDENT
        fprintf(global.outfile, "SAC_GKCO_HOST_OPD_SHIFT_LB(%s, %s, %s)\n\n",
                lb, ub_read, ub_write);
    }

    COMP_FUN_DBUG_RETURN(inner)
}

/**
 * Inverse function of GKCOcompShiftLB.
 *
 * @param outer             The GPU kernel res, resulting from calling all outer pragma calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompInvShiftLB(gpukernelres_t* outer) {
    DBUG_ENTER();
    DBUG_PRINT ("compiling UnShiftLB (inner):");

    // To be able to pop the lb variables from the stack in the inverse order in which we pushed,
    // we need to loop the dimensions in inverse order as well
    for (LOOP_DIMENSIONS_INV(outer, dim)) {
        // Consume the preserved variable identifiers for lb and ub
        GKR_LB_POP(outer, dim)
        GKR_UB_POP(outer, dim)

        // We only have to handle dimensions that aren't already normalized
        if (STReq(GKR_LB_D_READ(outer, dim), CONST_ZERO)) continue;

        // Compute the new index variable
        INDENT
        fprintf(global.outfile, "SAC_GKCO_GPUD_OPD_UNSHIFT_LB(%s, %s)\n\n",
                GKR_LB_D_READ(outer, dim), GKR_ID_D_READ(outer, dim));
    }

    COMP_FUN_DBUG_RETURN(outer)
}

/**
 * The CompressGrid mapping compresses the grid inside the index space, so it becomes dense. This means that after
 * calling the CompressGrid mapping, no redundant threads resulting from the step/width grid will be spawned. Redundant
 * threads may still be spawned for other reasons, though. It sets step and width to 1 (for the non-inverted call) and
 * restores the step/width/idx variables (in the inverse call).
 *
 * For each dimension j, for which `shouldCompress_node[j] == true:
 *               CompressGrid                                         InvCompressGrid
 *                                           ... --> 1, 1, ubj', idj'       -->       stj, wij, ubj, idj
 * stj, wij, ubj     -->      1, 1, ubj' --> ... --> 1, 1, ubj', idj'       -->       stj, wij, ubj, idj
 *                                           ... --> 1, 1, ubj', idj'       -->       stj, wij, ubj, idj
 *
 * Note:
 *   - The lowerbound should be 0, otherwise a compile error will be thrown.
 *   - It only compresses dimensions enabled by the `shouldCompress_node` argument.
 *   - It only compresses dimensions where the step is not already 1.
 *   - The SAC_GKCO_HOST_OPD_COMPRESS_SW ICM contains a ternary expression. A branchless version exists, which is
 *     enabled when in branchless mode (PASS_BRANCHLESS)
 *   - Resolving step/width by this method may be computationally heavy, but will result in no redundant threads. Use
 *     StepWidth (implicit) for a computationally cheaper alterantive, that may spawn redundant threads.
 *
 * @param shouldCompress_node The node containing a boolean array, telling which dimensions have to be compressed.
 * @param inner             The gpu kernel res, resulting from calling all inner pragma calls first
 * @return                  The modified gpu kernel res
 */
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

    for (LOOP_DIMENSIONS(inner, dim)) {
        // Preserve the current variable identifiers for ub, st and wi
        GKR_UB_PUSH(inner, dim)
        GKR_ST_PUSH(inner, dim)
        GKR_WI_PUSH(inner, dim)

        // Skip this iteration if compress for this dimension is toggled off
        if (!shouldCompress[dim]) continue;
        // Check whether the lowerbound is indeed 0
        if (GKR_CHECK_PRAGMA(inner))
            checkLbZero(GKR_LB_D_READ(inner, dim), shouldCompress_node, "CompressGrid", dim);
        // If the step is 1, we don't need to compute anything
        if (STReq(GKR_ST_D_READ(inner, dim), CONST_ONE)) continue;

        // Get the correct variable identifiers
        char* ub_read  = GKR_UB_D_READ(inner, dim);
        char* ub_write = GKR_UB_NEW(inner, dim);
        char* step     = GKR_ST_D_REPLACE(inner, dim, CONST_ONE);
        char* width    = GKR_WI_D_REPLACE(inner, dim, CONST_ONE);

        // If the width is 1, we only need to handle the step
        INDENT
        if (STReq(width, CONST_ONE))
            fprintf(global.outfile, "SAC_GKCO_HOST_OPD_COMPRESS_S(%s, %s, %s)\n\n",
                    ub_read, ub_write, step);
            // Else, we handle the case where both the step and width are relevant
        else if (!GKR_BRANCHLESS(inner))
            fprintf(global.outfile, "SAC_GKCO_HOST_OPD_COMPRESS_SW(%s, %s, %s, %s, %s)\n\n",
                    ub_read, ub_write, step, width, GKR_HELPER_A(inner));
        else
            fprintf(global.outfile, "SAC_GKCO_HOST_OPD_COMPRESS_SW_BL(%s, %s, %s, %s, %s, %s)\n\n",
                    ub_read, ub_write, step, width, GKR_HELPER_A(inner), GKR_HELPER_B(inner));
    }

    MEMfree(shouldCompress);

    COMP_FUN_DBUG_RETURN(inner)
}

/**
 * Inverse function of GKCOcompCompressGrid.
 *
 * @param shouldCompress_node The node containing a boolean array, telling which dimensions have to be compressed.
 * @param outer             The GPU kernel res, resulting from calling all outer pragma calls first
 * @return                  The modified gpu kernel res
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
    for (LOOP_DIMENSIONS_INV(outer, dim)) {
        // Consume the preserved variable identifiers for ub, step and width
        GKR_UB_POP(outer, dim)
        GKR_ST_POP(outer, dim)
        GKR_WI_POP(outer, dim)

        // Skip this iteration if compress for this dimension is toggled off
        if (!shouldCompress[dim]) continue;
        // If the step is 1, we don't need to compute anything
        if (STReq(GKR_ST_D_READ(outer, dim), CONST_ONE)) continue;

        // If the width is 1, we only need to handle the step
        if (STReq(GKR_WI_D_READ(outer, dim), CONST_ONE)) {
            INDENT
            fprintf(global.outfile, "SAC_GKCO_GPUD_OPD_UNCOMPRESS_S(%s, %s)\n\n",
                    GKR_ST_D_READ(outer, dim), GKR_ID_D_READ(outer, dim));
        }
            // Else, we handle the case where both the step and width are relevant
        else {
            INDENT
            fprintf(global.outfile, "SAC_GKCO_GPUD_OPD_UNCOMPRESS_SW(%s, %s, %s)\n\n",
                    GKR_ST_D_READ(outer, dim), GKR_WI_D_READ(outer, dim), GKR_ID_D_READ(outer, dim));
        }
    }

    MEMfree(shouldCompress);

    COMP_FUN_DBUG_RETURN(outer)
}

/**
 * Create a permutation of a strvec. The original vector will be consumed.
 *
 * @param vec               The strvec vector to be permuted
 * @param permutation       The new order, given as an array of integers nodes
 * @return The new          The permutated strvec
 */
static strvec*
PermuteStrvec(strvec* vec, const int* permutation) {
    DBUG_ENTER();

    strvec* newvec = STRVECempty(STRVEClen(vec));
    for (size_t i = 0; i < STRVEClen(vec); i++) {
        size_t old_index = (size_t) permutation[i];
        STRVECappend(newvec, STRVECsel(vec, old_index));
    }

    STRVECfree(vec);

    DBUG_RETURN(newvec);
}

/**
 * The permute mapping changes the order of the dimensions in the non-inverted call, and changes them back in the
 * inverted call.
 *
 * Note:
 *   - This mapping does not generate any code. It only changes the gpukernelres_t, so it changes how all code inbetween
 *     Permute and InvPermute is generated.
 *
 * @param permutation_node  The permutation, as an AST node of a list of integer expressions
 * @param inner             The gpu kernel res, resulting from calling all inner pragma
 *                          calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompPermute(node* permutation_node, gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling Permute:");
    DBUG_EXECUTE (
            fprintf(stderr, "    Permute (");
            printNumArray(permutation_node);
            fprintf(stderr, ", inner)\n");
    );

    int* permutation  = getNumArrayFromNodes(permutation_node, GKR_DIM(inner), "Permute");

    GKR_LB(inner) = PermuteStrvec(GKR_LB(inner), permutation);
    GKR_UB(inner) = PermuteStrvec(GKR_UB(inner), permutation);
    GKR_ST(inner) = PermuteStrvec(GKR_ST(inner), permutation);
    GKR_WI(inner) = PermuteStrvec(GKR_WI(inner), permutation);
    if (GKR_IDX_COMPUTE(inner))
        GKR_ID(inner) = PermuteStrvec(GKR_ID(inner), permutation);

    MEMfree(permutation);

    COMP_FUN_DBUG_RETURN(inner)
}

/**
 * Create an inverse permutation of a strvec. The original vector will be consumed.
 *
 * @param vec               The strvec vector to be permuted
 * @param permutation       The new order (inversed), given as an array of integers nodes
 * @return The new          The permutated strvec
 */
static strvec*
InvPermuteStrvec(strvec* vec, const int* permutation) {
    DBUG_ENTER();

    strvec* newvec = STRVECconst(STRVEClen(vec), NULL);
    for (size_t i = 0; i < STRVEClen(vec); i++) {
        size_t old_index = (size_t) permutation[i];
        STRVECswap(newvec, old_index, STRVECsel(vec, i));
    }

    STRVECfree(vec);

    DBUG_RETURN(newvec);
}

/**
 * Inverse function of CKCOcompPermute
 *
 * @param permutation_node  The permutation, as an AST node of a list of integer expressions
 * @param outer             The GPU kernel res, resulting from calling all outer pragma calls first
 * @return                  The modified gpu kernel res
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

    GKR_LB(outer) = InvPermuteStrvec(GKR_LB(outer), permutation);
    GKR_UB(outer) = InvPermuteStrvec(GKR_UB(outer), permutation);
    GKR_ST(outer) = InvPermuteStrvec(GKR_ST(outer), permutation);
    GKR_WI(outer) = InvPermuteStrvec(GKR_WI(outer), permutation);
    GKR_ID(outer) = InvPermuteStrvec(GKR_ID(outer), permutation);

    MEMfree(permutation);

    COMP_FUN_DBUG_RETURN(outer)
}

/**
 * The FoldLast2 mapping folds the last two dimensions into one, reducing the dimensionality by 1. It multiplies the
 * last two upperbounds together to create a new upperbound that can accommodate all indexes of those two dimensions
 * combined in the non-inverted function call, and it splits these into two dimensions again (including the idx
 * variable) in the inverted function call. Dimensions j = n-2 and k = n-1 will be the major and minor dimensions
 * respectively. This means that we get the new dimension will accommodate the indexes in the order:
 *
 *   j=0 k=0..k=n, j=1 k=0..k=n, ...
 *
 * For dimension j (n-2) and k(n-1):
 *          FoldLast2                                   InvFoldLast2
 *                                ... --> ubj*ubk, idj'      -->     ubj, ubk, idj, idk
 * ubj, ubk    -->    ubj*ubk --> ... --> ubj*ubk, idj'      -->     ubj, ubk, idj, idk
 *                                ... --> ubj*ubk, idj'      -->     ubj, ubk, idj, idk
 *
 * Note:
 *   - lb, st and wi have to be 0, 1 and 1 respectively. The lowerbound should be set to 0 manually (with ShiftLB),
 *     but the step and width will be set to 1 and 1 automatically if needed.
 *
 * @param inner             The gpu kernel res, resulting from calling all inner pragma calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompFoldLast2(gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling FoldLast2 (inner)");

    size_t majordim = GKR_DIM(inner) - 2;
    size_t minordim = GKR_DIM(inner) - 1;

    // Check lowerbound, step and width variables. Step and width will be set to 1, but if lowerbound is not
    // set to 0, we throw a compiler error.
    GKCOcompStepWidth(majordim, inner);
    GKCOcompStepWidth(minordim, inner);
    if (GKR_CHECK_PRAGMA(inner)) {
        checkLbZero(GKR_LB_D_READ(inner, majordim), NULL, "SplitLast", majordim);
        checkLbZero(GKR_LB_D_READ(inner, minordim), NULL, "SplitLast", minordim);
    }

    // Preserve the upperbound variable. The minor dim upperbound variable is automarically stored using
    // RemoveDimension and AddDimension
    GKR_UB_PUSH(inner, majordim)

    // Get variable identifiers. We write back to a new upperbound variable.
    char* ub_major_read  = GKR_UB_D_READ(inner, majordim);
    char* ub_major_write = GKR_UB_NEW(inner, majordim);
    char* ub_minor       = GKR_UB_D_READ(inner, minordim);

    INDENT
    fprintf(global.outfile, "SAC_GKCO_HOST_OPM_FOLD_LAST(%s, %s, %s)\n\n",
            ub_major_read, ub_major_write, ub_minor);

    RemoveDimension(inner);

    COMP_FUN_DBUG_RETURN(inner)
}

/**
 * Inverse function of GKCOcompFoldLast2.
 *
 * @param outer             The GPU kernel res, resulting from calling all outer pragma calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompInvFoldLast2(gpukernelres_t* outer) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling UnfoldLast2 (inner)");

    AddDimension(outer);
    size_t majordim = GKR_DIM(outer) - 2;
    size_t minordim = GKR_DIM(outer) - 1;

    GKR_UB_POP(outer, majordim);

    char* idx_major = GKR_ID_D_READ(outer, majordim);
    char* idx_minor = GKR_ID_D_READ(outer, minordim);
    char* ub_minor  = GKR_UB_D_READ(outer, minordim);

    INDENT
    fprintf(global.outfile, "SAC_GKCO_GPUD_OPM_UNFOLD_LAST(%s, %s, %s)\n\n",
            idx_major, idx_minor, ub_minor);

    // We may have set the step and width to 1, so we have to invert that change.
    GKCOcompInvStepWidth(minordim, outer);
    GKCOcompInvStepWidth(majordim, outer);

    COMP_FUN_DBUG_RETURN(outer)
}

/**
 * The SplitLast mapping splits the last dimension into 2, increasing the dimensionality by 1. It divides the upperbound
 * of the last dimension by minorlen_node, and adds a dimension with length minorlen_node for the non-inverse function
 * call, and restores the old upperbound and computes the accompanying index variable in the inverse call. The notion of
 * the major and minor dimensions are the same as in then CKCOcompFoldLast2 mapping. o
 *
 * For dimension j (n-1):
 *     SplitLast                                               InvSplitLast
 *                              ... --> ubj', ubk', idj', idk'      -->     ubj, idj
 * ubj    -->    ubj', ubk' --> ... --> ubj', ubk', idj', idk'      -->     ubj, idj
 *                              ... --> ubj', ubk', idj', idk'      -->     ubj, idj
 *
 * Note:
 *   - lb, st and wi have to be 0, 1 and 1 respectively. The lowerbound should be set to 0 manually (with ShiftLB),
 *     but the step and width will be set to 1 and 1 automatically if needed.
 *
 * @param minorlen_node     The length of the minor dimension, as an N_num node
 * @param inner             The gpu kernel res, resulting from calling all inner pragma calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompSplitLast(node* minorlen_node, gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling SplitLast ( %i, inner)", NUM_VAL(minorlen_node));

    AddDimension(inner);
    size_t majordim = GKR_DIM(inner) - 2;
    size_t minordim = GKR_DIM(inner) - 1;
    size_t minorlen = (size_t) NUM_VAL(minorlen_node);

    // Check lowerbound, upperbound, step and width variables. Step and width will be set to 1, but if lowerbound is not
    // set to 0, we throw a compiler error. The upperbound should be padded so the split is possible.
    GKCOcompStepWidth(majordim, inner);
    GKCOcompPad(majordim, minorlen, inner);
    if (GKR_CHECK_PRAGMA(inner))
        checkLbZero(GKR_LB_D_READ(inner, majordim), minorlen_node, "SplitLast", majordim);

    // Preserve the upperbound variable.
    GKR_UB_PUSH(inner, majordim)

    // Get variable identifiers. We write back to a new upperbound variable.
    char* ub_major_read  = GKR_UB_D_READ(inner, majordim);
    char* ub_major_write = GKR_UB_NEW(inner, majordim);
    char* ub_minor       = GKR_UB_D_READ(inner, minordim);

    INDENT
    fprintf(global.outfile, "SAC_GKCO_HOST_OPM_SPLIT_LAST(%s, %s, %s, %zu)\n\n",
            ub_major_read, ub_major_write, ub_minor, minorlen);

    COMP_FUN_DBUG_RETURN(inner)
}

/**
 * Inverse function of GKCOcompSplitLast.
 *
 * @param minorlen_node     The length of the minor dimension, as an N_num node
 * @param outer             The GPU kernel res, resulting from calling all outer pragma calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompInvSplitLast(node* minorlen_node, gpukernelres_t* outer) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling UnsplitLast ( %i, inner)", NUM_VAL(minorlen_node));

    size_t majordim = GKR_DIM(outer) - 2;
    size_t minordim = GKR_DIM(outer) - 1;
    size_t minorlen = (size_t) NUM_VAL(minorlen_node);

    GKR_UB_POP(outer, majordim);

    char* idx_major = GKR_ID_D_READ(outer, majordim);
    char* idx_minor = GKR_ID_D_READ(outer, minordim);

    INDENT
    fprintf(global.outfile, "SAC_GKCO_GPUD_OPM_UNSPLIT_LAST(%s, %s, %zu)\n\n",
            idx_major, idx_minor, minorlen);

    // We may have set the step and width to 1 and we may have padded the upperbound,
    // so we have to invert those changes.
    GKCOcompInvPad(majordim, outer);
    GKCOcompInvStepWidth(majordim, outer);

    RemoveDimension(outer);

    COMP_FUN_DBUG_RETURN(outer)
}

/**
 * Currently, this is a function that just exposes GKCOcompPad. It only pads the last dimension. See GKCOcompPad for
 * more information.
 *
 * @param divisibility_node The number by which the last upperbound should be divisible.
 * @param inner             The gpu kernel res, resulting from calling all inner pragma calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompPadLast(node* divisibility_node, gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling PadLast ( %i, inner)", NUM_VAL(divisibility_node));

    size_t dim          = GKR_DIM(inner) - 1;
    size_t divisibility = (size_t) NUM_VAL(divisibility_node);
    GKCOcompPad(dim, divisibility, inner);

    COMP_FUN_DBUG_RETURN(inner)
}

/**
 * Inverse function of GKCOcompPadLast.
 *
 * @param divisibility_node The number by which the last upperbound should be divisible.
 * @param outer             The GPU kernel res, resulting from calling all outer pragma calls first
 * @return                  The modified gpu kernel res
 */
gpukernelres_t*
GKCOcompInvPadLast(node* divisibility_node, gpukernelres_t* outer) {
    DBUG_ENTER();
    DBUG_PRINT ("compiling UnpadLast ( %i, inner)", NUM_VAL(divisibility_node));

    size_t dim = GKR_DIM(outer) - 1;
    GKCOcompInvPad(dim, outer);

    COMP_FUN_DBUG_RETURN(outer)
}

/**********************************************************************************************************************\
************************************************************************************************************************
**
**    Runtime test functions
**
**    These functions perform tests enabled by the SAC_DO_CHECK_GPU flag. These are tests inside the
**    functions themselves, as wel as test ICM's.
**
************************************************************************************************************************
\**********************************************************************************************************************/

size_t TS_DIM;
strvec* TS_LB;
strvec* TS_UB;
strvec* TS_ST;
strvec* TS_WI;

/**
 * Check the gpu kernel res at the end of all computations against the dimensionality and idx variable identifiers it
 * should contain after executing all pragma mappings
 *
 * @param bnum              number of bound elements (== number of strings in bounds)
 * @param bounds            the actual bounds as strings, these are either SAC runtime
 *                          variable reads or constants.
 */
void
GKCOcompCheckGPUkernelRes(unsigned int bnum, char** bounds, gpukernelres_t* res) {
    DBUG_ENTER();

    size_t dims = (size_t) bnum / 5;
    DBUG_ASSERT(dims == GKR_DIM(res),
                "Dimensionality of original with loop (%zu) is not the same as "
                "the dimensionality of the generated iv (%zu)",
                dims, GKR_DIM(res));

    for (LOOP_DIMENSIONS(res, dim)) {
        DBUG_ASSERT(STReq(bounds[4 * dims + dim], GKR_ID_D_READ(res, dim)),
                    "The idx variable of dimension %zu (%s) is different then the original required idx variable (%s)",
                    dim, GKR_ID_D_READ(res, dim), bounds[4 * dims + dim]);
    }

    DBUG_RETURN();
}

void
PrintComputeFlat(size_t dims, strvec* ub, strvec* id) {
    DBUG_ENTER();

    for (size_t dim = 0; dim < dims; dim++)
        fprintf(global.outfile, "(");
    fprintf(global.outfile, "0");
    for (size_t dim = 0; dim < dims; dim++)
        fprintf(global.outfile, ") * %s + %s",
                STRVECsel(ub, dim), STRVECsel(id, dim));

    DBUG_RETURN();
}

void
PrintComputeSize(size_t dims, strvec* ub) {
    DBUG_ENTER();

    fprintf(global.outfile, "1");
    for (size_t dim = 0; dim < dims; dim++)
        fprintf(global.outfile, " * %s", STRVECsel(ub, dim));

    DBUG_RETURN();
}

void
GKCOcompCheckStart(gpukernelres_t* res) {
    DBUG_ENTER();

    fprintf(global.outfile, "SAC_BITMASK_THREADMAPPING_CHECK_START(");
    PrintComputeSize(GKR_DIM(res), GKR_UB(res));
    fprintf(global.outfile, ");\n\n\n");

    TS_DIM = GKR_DIM(res);
    TS_LB  = STRVECcopyDeep(GKR_LB(res));
    TS_UB  = STRVECcopyDeep(GKR_UB(res));
    TS_ST  = STRVECcopyDeep(GKR_ST(res));
    TS_WI  = STRVECcopyDeep(GKR_WI(res));

    DBUG_RETURN();
}

void
GKCOcompCheckKernel(gpukernelres_t* res) {
    DBUG_ENTER();

    fprintf(global.outfile, "SAC_BITMASK_THREADMAPPING_CHECK_KERNEL(");
    PrintComputeFlat(GKR_DIM(res), GKR_UB(res), GKR_ID(res));
    fprintf(global.outfile, ");\n\n\n");

    DBUG_RETURN();
}

void
CheckEndRecursiveHelper(size_t dim, size_t dims,
                        strvec* lb, strvec* ub,
                        strvec* st, strvec* wi,
                        strvec* id, char* chk) {
    DBUG_ENTER();

    if (dim < dims) {
        fprintf(global.outfile, "SAC_BITMASK_THREADMAPPING_CHECK_END_LOOP(");
        if (dim == 0) fprintf(global.outfile, "\nSAC_BITMASK_THREADMAPPING_CHECK_END_START(%s), \n", chk);
        else fprintf(global.outfile, ", \n");
        CheckEndRecursiveHelper(dim + 1, dims, lb, ub, st, wi, id, chk);
        fprintf(global.outfile, ", %s, %s, %s, %s, %s, %s)\n",
                STRVECsel(lb, dim), STRVECsel(ub, dim), STRVECsel(st, dim), STRVECsel(wi, dim),
                STRVECsel(id, dim), chk);
    } else {
        char* val = VarCreate(CONST_TMP_POSTFIX);

        fprintf(global.outfile, "SAC_BITMASK_THREADMAPPING_CHECK_END_INNER(%s, %s, \n",
                chk, val);
        PrintComputeFlat(dims, ub, id);
        fprintf(global.outfile, ");\n");

        free(val);
    }

    DBUG_RETURN();
}

void
GKCOcompCheckEnd() {
    DBUG_ENTER();

    DBUG_EXECUTE(
            char  * chk   = VarCreate(CONST_TMP_POSTFIX);
            strvec* TS_ID = STRVECempty(TS_DIM);
            for (size_t dim = 0; dim < TS_DIM; dim++)
                STRVECappend(TS_ID, VarCreate(CONST_IDX_POSTFIX));

            fprintf(global.outfile, "\nSAC_BITMASK_THREADMAPPING_CHECK_END(");
            PrintComputeSize(TS_DIM, TS_UB);
            fprintf(global.outfile, ")\n\n");
            CheckEndRecursiveHelper(0, TS_DIM, TS_LB, TS_UB, TS_ST, TS_WI, TS_ID, chk);

            free(chk);
            STRVECfreeDeep(TS_LB);
            STRVECfreeDeep(TS_UB);
            STRVECfreeDeep(TS_ST);
            STRVECfreeDeep(TS_WI);
            STRVECfreeDeep(TS_ID);
    );

    DBUG_RETURN();
}

#undef DBUG_PREFIX
