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
#include "print.h"


/*
 * implementation for the type gpukernelres_t:
 */
struct GPUKERNELRES {
   int dummy;
};





#ifndef DBUG_OFF
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
printNumArray (node *array)
{
    node *elems;

    elems = ARRAY_AELEMS (array);
    if (NODE_TYPE (EXPRS_EXPR (elems)) == N_num) {
       fprintf (stderr, "[ %d", NUM_VAL (EXPRS_EXPR (elems)));
    } else {
        fprintf (stderr, "[ %s", ID_NAME (EXPRS_EXPR (elems)));
    }
    elems = EXPRS_NEXT (elems);
    while (elems!=NULL) {
        if (NODE_TYPE (EXPRS_EXPR (elems)) == N_num) {
            fprintf (stderr, ", %d", NUM_VAL (EXPRS_EXPR (elems)));
        } else {
            fprintf (stderr, ", %s", ID_NAME (EXPRS_EXPR (elems)));
        }
        elems = EXPRS_NEXT (elems);
    }
    fprintf (stderr, "]");
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
gpukernelres_t *
dispatch (node *spap, unsigned int bnum, char **bounds)
{
    gpukernelres_t *res=NULL;

    DBUG_ENTER ();

    if (NODE_TYPE (spap) == N_spid) {
        res = GKCOcompGen (bnum, bounds);

#define ARGS( nargs) ARG##nargs
#define ARG0
#define ARG1 EXPRS_EXPR (SPAP_ARGS (spap)),
#define SKIPS( nargs) SKIP##nargs
#define SKIP0
#define SKIP1 EXPRS_NEXT
#define WLP(fun, nargs)                                                                   \
    } else if (STReq (SPAP_NAME (spap), #fun)) {                                          \
        DBUG_ASSERT ((SPAP_ARGS (spap) != NULL), "missing argument in `%s' ()", #fun);    \
        res = GKCOcomp ## fun ( ARGS( nargs)                                              \
                                dispatch (EXPRS_EXPR ( SKIPS( nargs) (SPAP_ARGS (spap))), \
                                          bnum, bounds));
#include "gpukernel_funs.mac"
#undef WLP
#undef ARGS
#undef ARG0
#undef ARG1
#undef SKIPS
#undef SKIP0
#undef SKIP1

    } else {
        DBUG_ASSERT( 0==1, "expected gpukernel function, found `%s'", SPAP_NAME (spap));
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
gpukernelres_t *
GKCOcompGpuKernelPragma (node *spap, unsigned int bnum, char ** bounds)
{
    gpukernelres_t *res;
    DBUG_ENTER ();

    DBUG_ASSERT (spap != NULL, "NULL pointer for funcall in gpukernel pragma!");
    DBUG_ASSERT (NODE_TYPE (spap) == N_spap, "non N_spap funcall in gpukernel pragma!");
    DBUG_ASSERT (STReq (SPAP_NAME (spap), "GridBlock"), "expected `GridBlock' found `%s'",
                                                                        SPAP_NAME (spap));
    /*
     * we could dbug assert many more things here but the asssumption is that 
     * pragmas themselves are rather likely left untouched and since we have 
     * checked the pragma correctness in the parser already, this islikely
     * overkill. Moreover, the debug version provides helpful error messages
     * in case accessors yield unexpected things.
     */

    res = GKCOcompGridBlock (EXPRS_EXPR (SPAP_ARGS (spap)),
                            dispatch (EXPRS_EXPR (EXPRS_NEXT (SPAP_ARGS (spap))), bnum, bounds));

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
gpukernelres_t *
GKCOcompGridBlock (node *num, gpukernelres_t *inner)
{
    DBUG_ENTER ();
    DBUG_PRINT ("compiling GridBlock ( %i, inner)", NUM_VAL (num));

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
gpukernelres_t *
GKCOcompGen ( unsigned int bnum, char **bounds)
{
#ifndef DBUG_OFF
    unsigned int i;
#endif
    DBUG_ENTER ();
    DBUG_EXECUTE (
        DBUG_PRINT ("compiling Gen:");
        fprintf (stderr, "    Gen ( %u", bnum);
        for (i=0; i<bnum; i++) {
            fprintf (stderr, ", %s", bounds[i]);
        }
        fprintf (stderr, ")\n");
    );

    DBUG_RETURN (NULL);
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
gpukernelres_t *
GKCOcompShift (node *array, gpukernelres_t *inner)
{
    DBUG_ENTER ();
    DBUG_EXECUTE (
        DBUG_PRINT ("compiling Shift:");
        fprintf (stderr, "    Shift (");
        printNumArray (array);
        fprintf (stderr, ", inner)\n");
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
gpukernelres_t *
GKCOcompCompressGrid ( gpukernelres_t *inner)
{
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
gpukernelres_t *
GKCOcompPermute (node *array, gpukernelres_t *inner)
{
    DBUG_ENTER ();
    DBUG_EXECUTE (
        DBUG_PRINT ("compiling Permute:");
        fprintf (stderr, "    Permute (");
        printNumArray (array);
        fprintf (stderr, ", inner)\n");
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
gpukernelres_t *
GKCOcompFoldLast2 (gpukernelres_t *inner)
{
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
gpukernelres_t *
GKCOcompSplitLast (node *array, gpukernelres_t *inner)
{
    DBUG_ENTER ();
    DBUG_EXECUTE (
        DBUG_PRINT ("compiling SplitLast:");
        fprintf (stderr, "    SplitLast (");
        printNumArray (array);
        fprintf (stderr, ", inner)\n");
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
gpukernelres_t *
GKCOcompPad (node *array, gpukernelres_t *inner)
{
    DBUG_ENTER ();
    DBUG_EXECUTE (
        DBUG_PRINT ("compiling Pad:");
        fprintf (stderr, "    Pad (");
        printNumArray (array);
        fprintf (stderr, ", inner)\n");
    );

    DBUG_RETURN (inner);
}

#undef DBUG_PREFIX
