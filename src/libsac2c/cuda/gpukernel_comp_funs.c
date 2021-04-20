/** <!--********************************************************************-->
 *
 * @defgroup gpukernel pragma compile functions
 *
 *   This module implements the functions that are needed for implementing the
 *   #pragma gpukernel. It contains the following functionality:
 *
 *  Compiler for the thread space
 *
 *  When icm2c finds the ICM CUDA_THREAD_SPACE, it extracts the
 *  pragma and it calls GKCOcompGpuKernelPragma (spap, nbounds, bounds)
 *  from this file.
 *
 *  It is the purpose of GKCOcompGpuKernelPragma to compute the thread space
 *  as an expression from the bounds and the pragma. The information about
 *  the threadspace is kept in a structure internal to this file of type
 *
 *  gpukernelres_t == struct GPUKERNELRES; 
 *
 *  GKCOcompGpuKernelPragma expects the N_spap node of the pragma
 *  and it obtains a vector of strings (preceeded by its length) as
 *  further arguments. These strings represent the generator bounds
 *  in scalarised form (CUDA kernels are only for AKD at least!).
 *  The strings are either variable reads (as .hr-icms) or, in case
 *  the compiler knows, constants.
 *  From these arguments, GKCOcompGpuKernelPragma generates a nesting
 *  of function calls that reflects the nesting and arguments of the
 *  pragma's functions. For example, the pragma
 *
 *   CompressGrid (GridBlock (3, Gen))
 *  
 *  will be translated into:
 *
 *   GKCOcompCompressGrid (
 *       GKCOcompGridBlock (3,
 *           GKCOcompGen( nbounds, bounds)))
 *
 *  The actual construction of this function is achieved through a
 *  local function dispatch which uses the info in gpukernel_funs.mac
 *  to generate the string comparisons and functional calls as needed.
 *  As a consequence, new pragma functions can be added without 
 *  the need to modify GKCOcompGpuKernelPragma or the dipatch function.
 *  All that is needed is a new entry in gpukernel_funs.mac and an 
 *  implementation of the coresponding GKCOcomp-function.
 *
 *  All GKCOcomp-functions obtain as last argument a threadspace
 *  representation of type gpukernelres_t and they return back a
 *  modified version of it.
 *
 *  The final threadspace representation is then handed back to 
 *  ICMCompileCUDA_THREAD_SPACE, where it is translated into the
 *  actual CUDA code for invoking the kernel.
 *
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
#include "ctinfo.h"
#include "globals.h"
#include "memory.h"
#include "str.h"
#include "str_vec.h"
#include "print.h"

#define CONST_ZERO "0"
#define CONST_ONE "1"

#define CONST_UB_POSTFIX "_ub"

/**
 * An enum of the types of mappings. Most of them correspond to the functions below GKCOcompGpuKernelPragma.
 */
enum GPUKERNELOPTYPE {
    // Types with a corresponding function. Note that permute is unused, as it is not a real operation
    Gen, Permute, ShiftLB, Compress, FoldLast2, SplitLast, PadLast, GridBlock,
    // Types without a corresponding function
    CheckSW,                // Check the index against step/width variables
    CheckPD,                // Check the incex against the upperbound variable, to counter padding
};

/**
 * A general struct to represent a kernel mapping operation. It contains variables for all possible mappings.
 *
 * Note that upon creation, we cannot yet fill in all values, so this has to be done manually. However, we can state
 * some assertions over some of them after creation:
 *
 *   - type is set
 *   - flags are all unset
 *   - all pointers are NULL
 *
 *   Furthermore it is important to know that this structure does NOT own any of the strings.
 *   However, it DOES own the next op.
 */
struct GPUKERNELOP {
    enum GPUKERNELOPTYPE type;                  // The type of operation
    size_t               arg;                   // The numeric argument
    size_t               arg_inverse;           // The inverse argument, if needed

    char              * lb;                     // The lowerbound variable
    char              * ub;                     // The upperbound variable
    char              * nlb_st;                 // The new lowerbound, or step variable
    char              * nub_wi;                 // The new upperbound, or width variable
    strvec            * redefine_from;          // For the generate type, redefine variables
    strvec            * redefine_to;            // For the generate type, redefine variables
    struct GPUKERNELOP* next;                   // The next operation
};

#define GKO_TYPE(gko) gko->type
#define GKO_ARG(gko) gko->arg
#define GKO_ARG_INV(gko) gko->arg_inverse

#define GKO_LB(gko) gko->lb
#define GKO_UB(gko) gko->ub
#define GKO_ST(gko) gko->nlb_st
#define GKO_WI(gko) gko->nub_wi
#define GKO_NLB(gko) gko->nlb_st
#define GKO_NUB(gko) gko->nub_wi

#define GKO_RED_FROM(gko) gko->redefine_from
#define GKO_RED_TO(gko) gko->redefine_to

#define GKO_NEXT(gko) gko->next

/**
 * Make a new GPU kernel operation. Also see the comment above the definition of the struct.
 *
 * @param type The type of the GPU kernel operation
 * @return The GPU kernel operation
 */
static gpukernelop_t*
MakeGPUkernelOp(gpukerneloptype_t type) {
    DBUG_ENTER();

    gpukernelop_t* op = (gpukernelop_t*) MEMmalloc(sizeof(gpukernelop_t));

    GKO_TYPE(op) = type;

    GKO_LB(op)       = NULL;
    GKO_UB(op)       = NULL;
    GKO_ST(op)       = NULL;
    GKO_WI(op)       = NULL;
    GKO_RED_FROM(op) = NULL;
    GKO_RED_TO(op)   = NULL;
    GKO_NEXT(op)     = NULL;

    DBUG_RETURN(op);
}

/**
 * Free a GPU kernel op, according to the ownership conventions stated in the definition of the struct.
 *
 * @param op The operation to be freed
 * @return A null pointer
 */
static gpukernelop_t*
FreeGPUKernelOp(gpukernelop_t* op) {
    DBUG_ENTER();

    if (GKO_NEXT(op) != NULL)
        FreeGPUKernelOp(GKO_NEXT(op));
    MEMfree(op);

    DBUG_RETURN(NULL);
}

/**
 * GPU kernel representation struct. It contains:
 *   - A dimensionality of type size_t
 *   - Four string vectors, containing the variable names of the index space (lowerbound, upperbound, step and width)
 *     for each dimension
 *
 * IMPORTANT NOTE: The GPUKERNELRES struct is assumed to own all strings it makes itself. This means that, when freed or
 * shrunk, those strings are freed as well.
 * GPUKERNELRES also owns all GPUKERNELOPS.
 */
struct GPUKERNELRES {
    size_t dim;
    strvec* lowerbound;
    strvec* upperbound;
    strvec* step;
    strvec* width;

    strvec* to_alloc;

    gpukernelop_t* mappingstart;
    gpukernelop_t** mappingnext;
};

#define GKR_DIM(gkr) gkr->dim

#define GKR_LB(gkr) gkr->lowerbound
#define GKR_UB(gkr) gkr->upperbound
#define GKR_ST(gkr) gkr->step
#define GKR_WI(gkr) gkr->width

#define GKR_LB_D_READ(gkr, i) STRVECsel(GKR_LB(gkr), i)
#define GKR_UB_D_READ(gkr, i) STRVECsel(GKR_UB(gkr), i)
#define GKR_ST_D_READ(gkr, i) STRVECsel(GKR_ST(gkr), i)
#define GKR_WI_D_READ(gkr, i) STRVECsel(GKR_WI(gkr), i)

#define GKR_TA(gkr) gkr->to_alloc

#define GKR_MAP_START(gkr) gkr->mappingstart
#define GKR_MAP_NEXT(gkr) gkr->mappingnext

/**
 * Make a new gpu kernel representation struct. The vectors containing the variable names will be empty at this point.
 * This is because the variables have to be inserted manually at GKCOcompGen.
 *
 * @param dim The dimensionality of the gpu kernel representation
 * @return The newly created gpu kernel representation
 */
static gpukernelres_t*
MakeGPUkernelres(size_t dim) {
    DBUG_ENTER();

    gpukernelres_t* gpukernelres = (gpukernelres_t*) MEMmalloc(sizeof(gpukernelres_t));

    GKR_DIM(gpukernelres) = dim;
    GKR_LB(gpukernelres)  = STRVECempty(dim);
    GKR_UB(gpukernelres)  = STRVECempty(dim);
    GKR_ST(gpukernelres)  = STRVECempty(dim);
    GKR_WI(gpukernelres)  = STRVECempty(dim);

    GKR_MAP_START(gpukernelres) = NULL;
    GKR_MAP_NEXT(gpukernelres)  = &GKR_MAP_START(gpukernelres);

    DBUG_RETURN(gpukernelres);
}

/**
 * Free a gpu kernel representation struct. It will deep free all string vectors and strings, per convention that the
 * struct owns all strings.
 *
 * @param gkr The struct to be freed
 * @return the null pointer
 */
static gpukernelres_t*
FreeGPUkernelres(gpukernelres_t* gkr) {
    DBUG_ENTER();

    STRVECfree(GKR_LB(gkr));
    STRVECfree(GKR_UB(gkr));
    STRVECfree(GKR_ST(gkr));
    STRVECfree(GKR_WI(gkr));

    STRVECfreeDeep(GKR_TA(gkr));
    FreeGPUKernelOp(GKR_MAP_START(gkr));

    MEMfree(gkr);

    DBUG_RETURN(NULL);
}

/**
 * Add a dimension to the GPU kernel representation.
 *
 * Because it is a new dimension, lb will be set to 0, and step and width will be set to 1. The value for ub can be
 * given, but this is not required. If a NULL value is given, a new variable name with postfix "_ub" will be
 * automatically generated.
 *
 * @param gkr The GPU kernel representation
 * @param ub The variable/value for the upper bound. Can be NULL
 */
static void
AddDimension(gpukernelres_t* gkr, char* ub) {
    DBUG_ENTER();

    if (ub == NULL)
        ub = TRAVtmpVarName(CONST_UB_POSTFIX);

    GKR_DIM(gkr)++;
    STRVECappend(GKR_LB(gkr), CONST_ZERO);
    STRVECappend(GKR_UB(gkr), ub);
    STRVECappend(GKR_ST(gkr), CONST_ONE);
    STRVECappend(GKR_WI(gkr), CONST_ONE);

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

    GKR_DIM(gkr)--;
    STRVECpop(GKR_LB(gkr));
    STRVECpop(GKR_UB(gkr));
    STRVECpop(GKR_ST(gkr));
    STRVECpop(GKR_WI(gkr));

    DBUG_RETURN();
}

/**
 * Create the next GPUkernelOp. It will automatically correctly added to the data structures in the GPU kernel
 * representation. The type and flags and next will be set correctly and all pointers will be set to 0, but the rest of
 * the values have to be set manually.
 *
 * @param gkr The current GPU kernel representation
 * @param type The type of the operation
 * @return The newly created operation
 */
static gpukernelop_t*
NextKernelOp(gpukernelres_t* gkr, gpukerneloptype_t type) {
    DBUG_ENTER();

    gpukernelop_t* op = MakeGPUkernelOp(type);

    *GKR_MAP_NEXT(gkr) = op;
    GKR_MAP_NEXT(gkr) = &GKO_NEXT(op);

    DBUG_RETURN(op);
}

/**
 * For some operations the step and width are required to be "1". This function makes sure they are, in one single
 * dimension. It will:
 *
 *   - Check if it already is. If it is, it skips all subsequent steps
 *   - Inserts a check step/width operation
 *   - Sets the step and width variables to the constant "1" in the GPU kernel representation
 *
 * @param gkr The current GPU kernel representation
 * @param dim The dimension that has to be checked
 */
static void
CheckGKRdestroySW(gpukernelres_t* gkr, size_t dim) {
    DBUG_ENTER();

    // The step should be 1 to continue. If it is not, the step/width variables have to be checked at this point.
    if (!STReq(GKR_ST_D_READ(gkr, dim), "1")) {
        gpukernelop_t* op = NextKernelOp(gkr, CheckSW);
        GKO_ARG(op) = dim;
        GKO_ST(op)  = GKR_ST_D_READ(gkr, dim);
        GKO_WI(op)  = GKR_WI_D_READ(gkr, dim);

        STRVECswap(GKR_ST(gkr), dim, CONST_ONE);
        STRVECswap(GKR_WI(gkr), dim, CONST_ONE);
    }

    DBUG_RETURN();
}

static size_t
CheckGKRpadding(gpukernelres_t* gkr, size_t dim, size_t targetlength, bool cansplit) {
    DBUG_ENTER();

    // TODO: implement this stub
    const size_t invertedsize = 0;

    DBUG_RETURN(invertedsize);
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
    fprintf(stream, "GPU kernelres (dim: %zu)\n", GKR_DIM(gkr));
    fprintf(stream, "lb = ");
    STRVECprint(GKR_LB(gkr), stream, linesize);
    fprintf(stream, "ub = ");
    STRVECprint(GKR_UB(gkr), stream, linesize);
    fprintf(stream, "step = ");
    STRVECprint(GKR_ST(gkr), stream, linesize);
    fprintf(stream, "width = ");
    STRVECprint(GKR_WI(gkr), stream, linesize);

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
dispatch(node* spap, unsigned int bnum, char** bounds) {
    gpukernelres_t* res = NULL;

    DBUG_ENTER ();

    if (NODE_TYPE (spap) == N_spid) {
        res = GKCOcompGen(bnum, bounds);

#define ARGS(nargs) ARG##nargs
#define ARG0
#define ARG1 EXPRS_EXPR (SPAP_ARGS (spap)),
#define SKIPS(nargs) SKIP##nargs
#define SKIP0
#define SKIP1 EXPRS_NEXT
#define WLP(fun, nargs)                                                                   \
    } else if (STReq (SPAP_NAME (spap), #fun)) {                                          \
        DBUG_ASSERT ((SPAP_ARGS (spap) != NULL), "missing argument in `%s' ()", #fun);    \
        res = GKCOcomp ## fun ( ARGS( nargs)                                              \
                                dispatch (EXPRS_EXPR ( SKIPS( nargs) (SPAP_ARGS (spap))), \
                                          bnum, bounds));

#include "gpukernel_funs.mac" #undef WLP #undef ARGS #undef ARG0
#undef ARG1
#undef SKIPS
#undef SKIP0
#undef SKIP1

    } else {
        DBUG_ASSERT(0 == 1, "expected gpukernel function, found `%s'", SPAP_NAME(spap));
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompGpuKernelPragma (node *spap, unsigned int bnum, char ** bounds)
 *
 * @brief generates the actual function nesting. It manually calls the 
 *        mandatory outer function GKCOcompGridBlock with its parameter and
 *        it provides the innermost calls through the recursive helper function
 *        dispatch.
 *
 * @param spap   N_spap of the outer function GridBlock(...)
 * @param bnum   number of bound elements (== number of strings in bounds)
 * @param bounds the actual bounds as strings, these are either SAC runtime
 *               variable reads or constants.
 *
 * @return the result of the entire function nesting
 ******************************************************************************/
gpukernelres_t*
GKCOcompGpuKernelPragma(node* spap, unsigned int bnum, char** bounds) {
    gpukernelres_t* res;
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

    res = GKCOcompGridBlock(EXPRS_EXPR(SPAP_ARGS(spap)),
                            dispatch(EXPRS_EXPR(EXPRS_NEXT(SPAP_ARGS(spap))), bnum, bounds));

    DBUG_RETURN (res);
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

    DBUG_RETURN (inner);
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
GKCOcompGen(unsigned int bnum, char** bounds) {
#ifndef DBUG_OFF
    unsigned int i;
#endif
    DBUG_ENTER ();
    DBUG_EXECUTE (
            DBUG_PRINT("compiling Gen:");
            fprintf(stderr, "    Gen ( %u", bnum);
            for (i = 0; i < bnum; i++) {
                fprintf(stderr, ", %s", bounds[i]);
            }
            fprintf(stderr, ")\n");
    );

    // We have four variables per dimension, so the number of dimensions is the number of variables devided by 4
    size_t dim = bnum / 4;
    gpukernelres_t* gpukernelres = MakeGPUkernelres(dim);

    for (size_t i = 0; i < dim; i++) {
        STRVECappend(GKR_LB(gpukernelres), bounds[i + 0 * dim]);
        STRVECappend(GKR_UB(gpukernelres), bounds[i + 1 * dim]);
        STRVECappend(GKR_ST(gpukernelres), bounds[i + 2 * dim]);
        STRVECappend(GKR_WI(gpukernelres), bounds[i + 3 * dim]);
    }

    NextKernelOp(gpukernelres, Gen);

    DBUG_EXECUTE(
            DBUG_PRINT("Generated GPU kernel representation:");
            PrintGPUkernelres(gpukernelres, stderr);
    );

    DBUG_RETURN (gpukernelres);
}


/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompShift (node *array, gpukernelres_t *inner)
 *
 * @brief
 *
 * @param _DEPRICATED  To remove
 * @param inner  the thread space that results from applying all inner
 *               pragma functions
 *
 * @return the resulting thread space from applying Shift (array) to it.
 ******************************************************************************/
gpukernelres_t*
GKCOcompShift(node* _DEPRICATED, gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT("compiling Shift (inner)\n");

    for (size_t i = 0; i < GKR_DIM(inner); i++) {
        // We only have to handle dimensions that aren't already normalized
        if (!STReq(STRVECsel(GKR_LB(inner), i), CONST_ZERO)) {
            // Create new kernel op to normalize the lowerbound
            gpukernelop_t* op = NextKernelOp(inner, ShiftLB);
            GKO_LB(op) = GKR_LB_D_READ(inner, i);
            GKO_UB(op) = GKR_UB_D_READ(inner, i);

            // We do not have to free the old constant, as they are not owned by the LB vector
            STRVECswap(GKR_LB(inner), i, CONST_ZERO);
        }
    }

    DBUG_RETURN (inner);
}


/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompCompressGrid ( gpukernelres_t *inner)
 *
 * @brief
 *
 * @param inner  the thread space that results from applying all inner
 *               pragma functions
 *
 * @return the resulting thread space from applying CompressGrid () to it.
 ******************************************************************************/
gpukernelres_t*
GKCOcompCompressGrid(/*bool* dimensions, */gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling CompressGrid (inner)\n");
    // TODO: replace with arguments
    bool* dimensions = (bool*) MEMmalloc(sizeof(bool) * GKR_DIM(inner));

    for (size_t i = 0; i < GKR_DIM(inner); i++) {
        if (!dimensions[i]) continue;                                           // Check the parameter toggle
        // TODO: generate compiler error
        if (!STReq(GKR_LB_D_READ(inner, i), CONST_ZERO)) continue;              // Check wether the lowerbound is 0
        if (STReq(GKR_ST_D_READ(inner, i), CONST_ONE)) continue;                // Check whether the step already is 1

        gpukernelop_t* op = NextKernelOp(inner, Compress);

        GKO_UB(op) = GKR_UB_D_READ(inner, i);
        GKO_ST(op) = GKR_ST_D_READ(inner, i);
        GKO_WI(op) = GKR_WI_D_READ(inner, i);
    }

    DBUG_RETURN (inner);
}

/**
 * Create a permutation of a strvec. The original vector will be consumed.
 *
 * @param vec The strvec vector to be permuted
 * @param permutation  The new order, given as an array of N_num nodes
 * @return The new permutated strvec
 */
static strvec*
GKCOcompPermuteStrvec(strvec* vec, node* permutation) {
    DBUG_ENTER();

    strvec* newvec = STRVECempty(STRVEClen(vec));
    for (size_t i = 0; i < STRVEClen(vec); i++) {
        size_t old_index = NUM_VAL(permutation[i]);
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
GKCOcompPermute(node* array, gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_EXECUTE (
            DBUG_PRINT("compiling Permute:");
            fprintf(stderr, "    Permute (");
            printNumArray(array);
            fprintf(stderr, ", inner)\n");
    );

    GKR_LB(inner) = GKCOcompPermuteStrvec(GKR_LB(inner), array);
    GKR_UB(inner) = GKCOcompPermuteStrvec(GKR_UB(inner), array);
    GKR_ST(inner) = GKCOcompPermuteStrvec(GKR_ST(inner), array);
    GKR_WI(inner) = GKCOcompPermuteStrvec(GKR_WI(inner), array);

    DBUG_RETURN (inner);
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

    size_t major = GKR_DIM(inner) - 2;
    size_t minor = GKR_DIM(inner) - 1;

    if (GKR_DIM(inner) < 2) {
        // TODO: generate compiler error
    } else if (!STReq(GKR_LB_D_READ(inner, major), CONST_ZERO)) {
        // TODO: generate compile error
    } else if (!STReq(GKR_LB_D_READ(inner, minor), CONST_ZERO)) {
        // TODO: generate compile error
    } else {
        CheckGKRdestroySW(inner, major);
        CheckGKRdestroySW(inner, minor);

        gpukernelop_t* op = NextKernelOp(inner, FoldLast2);

        GKO_UB(op)  = GKR_LB_D_READ(inner, major);
        GKO_NUB(op) = GKR_LB_D_READ(inner, minor);

        RemoveDimension(inner);
    }

    DBUG_RETURN (inner);
}


/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompSplitLast (node *array, gpukernelres_t *inner)
 *
 * @brief
 *
 * @param array  N_array argument containing the two values for splitting 
 *               the last dimension into. The elements can either be
 *               N_num or N_id
 * @param inner  the thread space that results from applying all inner
 *               pragma functions
 *
 * @return the resulting thread space from applying SplitLast (array) to it.
 ******************************************************************************/
gpukernelres_t*
GKCOcompSplitLast(node* _DEPRICATED, /* node* minorlength_node, */ gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_EXECUTE (
            DBUG_PRINT("compiling SplitLast:");
            fprintf(stderr, "    SplitLast (");
            printNumArray(_DEPRICATED);
            fprintf(stderr, ", inner)\n");
    );
    // TODO: change out for argument value
    size_t minorlength = 100;

    AddDimension(inner, NULL);

    size_t major = GKR_DIM(inner) - 2;
    size_t minor = GKR_DIM(inner) - 1;

    if (GKR_DIM(inner) < 2) {
        // TODO: generate compiler error
    } else if (!STReq(GKR_LB_D_READ(inner, major), CONST_ZERO)) {
        // TODO: generate compile error
    } else {
        CheckGKRdestroySW(inner, major);
        size_t majorlength = CheckGKRpadding(inner, major, minorlength, true);

        gpukernelop_t* op = NextKernelOp(inner, FoldLast2);

        GKO_ARG(op) = majorlength;
        GKO_ARG(op) = minorlength;
        GKO_UB(op)  = GKR_UB_D_READ(inner, major);
        GKO_NUB(op) = GKR_UB_D_READ(inner, minor);
    }

    DBUG_RETURN (inner);
}


/** <!-- ****************************************************************** -->
 * @fn gpukernelres_t *
 *     GKCOcompPad (node *array, gpukernelres_t *inner)
 *
 * @brief
 *
 * @param array  N_array argument defining the padding size vector
 *               all elements are either N_num or N_id
 * @param inner  the thread space that results from applying all inner
 *               pragma functions
 *
 * @return the resulting thread space from applying Pad (array) to it.
 ******************************************************************************/
gpukernelres_t*
GKCOcompPad(node* _DEPRICATED, /* node* dimlength, */ gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_EXECUTE (
            DBUG_PRINT("compiling Pad:");
            fprintf(stderr, "    Pad (");
            printNumArray(array);
            fprintf(stderr, ", inner)\n");
    );

    if (GKR_DIM(inner) < 2) {
        // TODO: generate compiler error
    } else if (!STReq(GKR_LB_D_READ(inner, major), CONST_ZERO)) {
        // TODO: generate compile error
    } else {
        CheckGKRdestroySW(inner, major);
        size_t majorlength = CheckGKRpadding(inner, major, minorlength, true);

        gpukernelop_t* op = NextKernelOp(inner, FoldLast2);

        GKO_ARG(op) = majorlength;
        GKO_ARG(op) = minorlength;
        GKO_UB(op)  = GKR_UB_D_READ(inner, major);
        GKO_NUB(op) = GKR_UB_D_READ(inner, minor);
    }

    DBUG_RETURN (inner);
}

#undef DBUG_PREFIX
