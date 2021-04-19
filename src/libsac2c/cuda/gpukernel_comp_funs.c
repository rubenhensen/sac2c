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

/**
 * An enum of the types of mappings. These correspond to the functions below GKCOcompGpuKernelPragma.
 */
enum GPUKERNELOPTYPE {
    Gen, Permute, ShiftLB, Compress, FoldLast2, SplitLast, PadLast, GridBlock
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
    int                  flags;                 // The flags, stored as bitmap
    int                  arg;                   // The numeric argument
    int                  arg_inverse;           // The inverse argument, if needed
    char              * lb;                     // The lowerbound variable
    char              * ub;                     // The upperbound variable
    char              * nlb_st;                 // The new lowerbound, or step variable
    char              * nub_wi;                 // The new upperbound, or width variable
    struct GPUKERNELOP* next;                   // The next operation
};

#define GKO_TYPE(gkr) gkr->type
#define GKO_FLAGS(gkr) gkr->flags
#define GKO_ARG(gkr) gkr->arg
#define GKO_ARG_INV(gkr) gkr->arg_inverse

#define GKO_LB(gkr) gkr->lb
#define GKO_UB(gkr) gkr->ub
#define GKO_ST(gkr) gkr->nlb_st
#define GKO_WI(gkr) gkr->nub_wi
#define GKO_NLB(gkr) gkr->nlb_st
#define GKO_NUB(gkr) gkr->nub_wi

#define GKO_NEXT(gkr) gkr->next

#define FLG_CHK_PD 1            /* Flag to check the padding */
#define FLG_CHK_SW 2            /* Flag to check the step/width, because this cannot be done anymore in later stages */

#define FLG_GET(gko, flg) GKO_FLAGS(gko) & flg
#define FLG_SET(gko, flg) GKO_FLAGS(gko) = GKO_FLAGS(gko) | flg
#define FLG_UNSET(gko, flg) GKO_FLAGS(gko) = GKO_FLAGS(gko) & ~flg

/**
 * Make a new GPU kernel operation. Also see the comment above the definition of the struct.
 *
 * @param type The type of the GPU kernel operation
 * @return The GPU kernel operation
 */
static gpukernelop_t *
MakeGPUkernelOp (gpukerneloptype_t type){
    DBUG_ENTER();

    gpukernelop_t* op = (gpukernelop_t*) MEMmalloc(sizeof(gpukernelop_t));

    GKO_TYPE(op)  = type;
    GKO_FLAGS(op) = 0;

    GKO_LB(op) = NULL;
    GKO_UB(op) = NULL;
    GKO_ST(op) = NULL;
    GKO_WI(op) = NULL;
    GKO_NEXT(op)  = NULL;

    DBUG_RETURN(op);
}

/**
 * Free a GPU kernel op, according to the ownership conventions stated in the definition of the struct.
 *
 * @param op The operation to be freed
 * @return A null pointer
 */
static gpukernelop_t *
FreeGPUKernelOp (gpukernelop_t* op) {
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
#define GKR_WD(gkr) gkr->width

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
    GKR_WD(gpukernelres)  = STRVECempty(dim);

    GKR_MAP_START(gpukernelres) = NULL;
    GKR_MAP_NEXT(gpukernelres) = &GKR_MAP_START(gpukernelres);

    DBUG_RETURN(gpukernelres);
}

/**
 * Free a gpu kernel representation struct. It will deep free all string vectors and strings, per convention that the
 * struct owns all strings.
 *
 * @param gpukernelres The struct to be freed
 * @return the null pointer
 */
static gpukernelres_t*
FreeGPUkernelres(gpukernelres_t* gpukernelres) {
    DBUG_ENTER();

    STRVECfree(GKR_LB(gpukernelres));
    STRVECfree(GKR_UB(gpukernelres));
    STRVECfree(GKR_ST(gpukernelres));
    STRVECfree(GKR_WD(gpukernelres));

    STRVECfreeDeep(GKR_TA(gpukernelres));
    FreeGPUKernelOp(GKR_MAP_START(gpukernelres));

    MEMfree(gpukernelres);

    DBUG_RETURN(NULL);
}

// TODO: Replace with actual variable name generator function
// Used in function below
static void
GrowGPUkernelresDim(gpukernelres_t* gpukernelres, strvec* vec, char* postfix, size_t new_dim) {
    DBUG_ENTER();

    for (size_t i = STRVEClen(vec); i < new_dim; i++) {
        char* var = TRAVtmpVarName(postfix);
        STRVECappend(GKR_TA(gpukernelres), var);
        STRVECappend(vec, var);
    }
    STRVECresize(vec, new_dim, NULL);

    DBUG_RETURN();
}

/**
 * Change the dimensionality of a gpu kernel representation. New variable names will automatically be generated on
 * growth, and old variable names will automatically be freed on shrinkage.
 *
 * @param gpukernelres The gpu kernel representation
 * @param new_dim The new dimensionality
 */
static void
RedimGPUkernelres(gpukernelres_t* gpukernelres, size_t new_dim) {
    DBUG_ENTER();

    char* (* closure)(void);

    GKR_DIM(gpukernelres) = new_dim;
    GrowGPUkernelresDim(gpukernelres, GKR_LB(gpukernelres), "_lb", new_dim);
    GrowGPUkernelresDim(gpukernelres, GKR_UB(gpukernelres), "_ub", new_dim);
    GrowGPUkernelresDim(gpukernelres, GKR_ST(gpukernelres), "_step", new_dim);
    GrowGPUkernelresDim(gpukernelres, GKR_WD(gpukernelres), "_width", new_dim);

    DBUG_RETURN();
}

/**
 * Create the next GPUkernelOp. It will automatically correctly added to the data structures in the GPU kernel
 * representation. The type and flags and next will be set correctly and all pointers will be set to 0, but the rest of
 * the values have to be set manually.
 *
 * @param gpukernelres The current GPU kernel representation
 * @param type The type of the operation
 * @return The newly created operation
 */
static gpukernelop_t*
NextKernelOp(gpukernelres_t* gpukernelres, gpukerneloptype_t type) {
    DBUG_ENTER();

    gpukernelop_t *op = MakeGPUkernelOp(type);

    *GKR_MAP_NEXT(gpukernelres) = op;
    GKR_MAP_NEXT(gpukernelres) = &GKO_NEXT(op);

    DBUG_RETURN(op);
}

#ifndef DBUG_OFF

/**
 * Print a GPU kernel representation. For debugging purposes only.
 *
 * @param gpukernelres The GPU kernelres to be printed
 * @param stream The stream to print to
 */
static void
PrintGPUkernelres(gpukernelres_t* gpukernelres, FILE* stream) {
    DBUG_ENTER();

    size_t linesize = 80;
    fprintf(stream, "GPU kernelres (dim: %zu)\n", GKR_DIM(gpukernelres));
    fprintf(stream, "lb = ");
    STRVECprint(GKR_LB(gpukernelres), stream, linesize);
    fprintf(stream, "ub = ");
    STRVECprint(GKR_UB(gpukernelres), stream, linesize);
    fprintf(stream, "step = ");
    STRVECprint(GKR_ST(gpukernelres), stream, linesize);
    fprintf(stream, "width = ");
    STRVECprint(GKR_WD(gpukernelres), stream, linesize);

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
        STRVECappend(GKR_LB(gpukernelres), STRcpy(bounds[i + 0 * dim]));
        STRVECappend(GKR_UB(gpukernelres), STRcpy(bounds[i + 1 * dim]));
        STRVECappend(GKR_ST(gpukernelres), STRcpy(bounds[i + 2 * dim]));
        STRVECappend(GKR_WD(gpukernelres), STRcpy(bounds[i + 3 * dim]));
    }

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
 * @param array  N_array argument for Shift
 * @param inner  the thread space that results from applying all inner
 *               pragma functions
 *
 * @return the resulting thread space from applying Shift (array) to it.
 ******************************************************************************/
gpukernelres_t*
GKCOcompShift(node* array, gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_EXECUTE (
            DBUG_PRINT("compiling Shift:");
            fprintf(stderr, "    Shift (");
            printNumArray(array);
            fprintf(stderr, ", inner)\n");
    );

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
GKCOcompCompressGrid(gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_PRINT ("compiling CompressGrid ( inner)");

    DBUG_RETURN (inner);
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
GKCOcompSplitLast(node* array, gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_EXECUTE (
            DBUG_PRINT("compiling SplitLast:");
            fprintf(stderr, "    SplitLast (");
            printNumArray(array);
            fprintf(stderr, ", inner)\n");
    );

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
GKCOcompPad(node* array, gpukernelres_t* inner) {
    DBUG_ENTER ();
    DBUG_EXECUTE (
            DBUG_PRINT("compiling Pad:");
            fprintf(stderr, "    Pad (");
            printNumArray(array);
            fprintf(stderr, ", inner)\n");
    );

    DBUG_RETURN (inner);
}

#undef DBUG_PREFIX
