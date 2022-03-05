/** <!--********************************************************************-->
 *
 * @defgroup gpukernel pragma functions
 *
 *   This module implements the functions that are needed for implementing the
 *   #pragma gpukernel. It contains the following main parts:
 *
 *   1) Parser checker for the gpukernel pragma
 *
 *   The parser checks for #pragma gpukernel <funcall>
 *   Once that is found, the parser calls     GKCHcheckGpuKernelPragma (spap, loc)
 *   from this module. GKCHcheckGpuKernelPragma expects two parameters: an
 *   N_spap node that represents the just parsed funcall and a location containing
 *   the location right after the last symbol parsed so far.
 *
 *   GKCHcheckGpuKernelPragma checks whether this is a legitimate nesting of
 *   gpukernel function calls. The nesting has to adhere to the following bnf:
 *
 *   GridBlock( <num>, <gpukernel_fun_ap>)
 *
 *   <gpukernel_fun_ap> -> Gen
 *                         | ShiftLB ( <gpukernel_fun_ap>)
 *                         | CompressGrid ( <vect>, <gpukernel_fun_ap>)
 *                         | Permute ( <vect>, <gpukernel_fun_ap>)
 *                         | FoldLast2 ( <gpukernel_fun_ap>)
 *                         | SplitLast ( <num>, <gpukernel_fun_ap>)
 *                         | PadLast ( <num>, <gpukernel_fun_ap>)
 *
 *  any non-adherence will issue CTIerrors explaining which arguments are
 *  missing or expected in a different form.
 *
 *  The function specific arguments (typically the first parameters) are being
 *  checked through function-specific check functions named GKCHcheckSplit,....
 *
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

#define DBUG_PREFIX "GKCH"

#include "debug.h"

#include "free.h"
#include "DupTree.h"
#include "gpukernel_check_funs.h"
#include "ctinfo.h"
#include "globals.h"
#include "memory.h"
#include "str.h"

/******************************************************************************
 ******************************************************************************
 **
 **  helper functions
 **/

static node *
checkNone (node *args, const char *name)
{
    DBUG_ENTER ();
    DBUG_RETURN (args);
}

static node *
checkNumArg (node *args, const char *name)
{
    node *arg;
    DBUG_ENTER ();

    arg = EXPRS_EXPR (args);
    if (NODE_TYPE (arg) != N_num) {
        CTIabort (NODE_LOCATION (arg),
                  "Wrong first argument to %s; "
                  "should be `%s ( <num>, <inner>)'",
                  name, name);
    }

    args = EXPRS_NEXT (args);
    DBUG_RETURN (args);
}

static node *
checkNumsArg (node *args, const char *name)
{
    node *arg, *exprs;
    DBUG_ENTER ();

    arg = EXPRS_EXPR (args);
    if (NODE_TYPE (arg) != N_array) {
        CTIabort (NODE_LOCATION (arg),
                  "Wrong first argument to %s; "
                  "should be `%s ( [<nums>], <inner>)'",
                  name, name);
    } else {
        exprs = ARRAY_AELEMS (arg);
        while (exprs != NULL) {
            arg = EXPRS_EXPR (exprs);
            if (NODE_TYPE (arg) != N_num) {
                CTIabort (NODE_LOCATION (arg),
                          "Wrong first argument to %s; "
                          "should be `%s ( [<nums>], <inner>)'",
                          name, name);
            }
            exprs = EXPRS_NEXT (exprs);
        }
    }

    args = EXPRS_NEXT (args);
    DBUG_RETURN (args);
}

static node *
checkPermutationArg (node *args, const char *name)
{
    node *arg, *exprs;
    DBUG_ENTER ();

    checkNumsArg (args, name);

    size_t length = 0;
    arg = EXPRS_EXPR (args);
    exprs = ARRAY_AELEMS (arg);
    while (exprs != NULL) {
        length++;
        exprs = EXPRS_NEXT (exprs);
    }

    bool *perm_hits = (bool *)MEMmalloc (sizeof (bool *) * length);
    for (size_t i = 0; i < length; i++)
        perm_hits[i] = false;

    exprs = ARRAY_AELEMS (arg);
    while (exprs != NULL) {
        arg = EXPRS_EXPR (exprs);
        int point_dim = NUM_VAL (arg);
        if (point_dim < 0 || point_dim >= (int)length || perm_hits[point_dim])
            CTIabort (NODE_LOCATION (arg),
                      "Wrong first argument to %s; "
                      "should be `%s ( [<permutation>], <inner>)'",
                      name, name);
        perm_hits[point_dim] = true;
        exprs = EXPRS_NEXT (exprs);
    }

    MEMfree (perm_hits);

    args = EXPRS_NEXT (args);
    DBUG_RETURN (args);
}

static node *
checkZONumsArg (node *args, const char *name)
{
    node *arg, *exprs;
    DBUG_ENTER ();

    arg = EXPRS_EXPR (args);
    if (NODE_TYPE (arg) != N_array) {
        CTIabort (NODE_LOCATION (arg),
                  "Wrong first argument to %s; "
                  "should be `%s ( [0/1, ..., 0/1], <inner>)'",
                  name, name);
    } else {
        exprs = ARRAY_AELEMS (arg);
        while (exprs != NULL) {
            arg = EXPRS_EXPR (exprs);
            if ((NODE_TYPE (arg) != N_num)
                || ((NUM_VAL (arg) != 0) && (NUM_VAL (arg) != 1))) {
                CTIabort (NODE_LOCATION (arg),
                          "Wrong first argument to %s; "
                          "should be `%s ( [0/1, ..., 0/1], <inner>)'",
                          name, name);
            }
            exprs = EXPRS_NEXT (exprs);
        }
    }

    args = EXPRS_NEXT (args);
    DBUG_RETURN (args);
}

void
checkArgsLength (node *arg, const size_t length, const char *name)
{
    node *exprs;
    DBUG_ENTER ();

    size_t computedlen = 0;
    exprs = ARRAY_AELEMS (arg);
    while (exprs != NULL) {
        exprs = EXPRS_NEXT (exprs);
        computedlen++;
    }

    if (computedlen < length)
        CTIabort (NODE_LOCATION (arg),
                  "Wrong first argument to %s: too short (%zu); "
                  "The list should be of length %zu",
                  name, computedlen, length);

    if (computedlen > length)
        CTIabort (NODE_LOCATION (arg),
                  "Wrong first argument to %s: too long (%zu); "
                  "The list should be of length %zu",
                  name, computedlen, length);

    DBUG_RETURN ();
}

void
checkDimensionSettings (node *gridDims_node, size_t dims)
{
    DBUG_ENTER ();

    size_t gridDims = (size_t)NUM_VAL (gridDims_node);
    size_t blockDims = dims - gridDims;

    // < 0 check is not necessary, as we have unsigned numbers
    if (gridDims > (size_t)global.config.cuda_dim_grid)
        CTIabort (NODE_LOCATION (gridDims_node),
                  "Number of grid dimensions too high! Should be 0-%i, currently %zu",
                  global.config.cuda_dim_grid, gridDims);
    // < 0 check is not necessary, as we have unsigned numbers
    if (blockDims > (size_t)global.config.cuda_dim_block)
        CTIabort (NODE_LOCATION (gridDims_node),
                  "Number of block dimensions too high! Should be 0-%i, currently %zu "
                  "(%zu - %zu)",
                  global.config.cuda_dim_block, blockDims, dims, gridDims);

    DBUG_RETURN ();
}

/**<!--*********************************************************************-->
 *
 * @fn void GKCHcheckGpuKernelPragma (node *spap, struct location loc)
 *
 * @param spap - N_spap node representing scanned/parsed function call
 * @param loc - the location of the scanner/parser wight after the funcall
 *
 * @brief expects an N_spap node which contains a nesting of function calls
 *        according to the syntax of the pragma gpukernel. It checks validity
 *        up to the legitimate ordering (see top of this file or SaC BNF document)
 *        and legitimate argument nodes of the individual functions.
 *        If errors are being detected CTIerrors are being issued.
 *
 ******************************************************************************/

void
GKCHcheckGpuKernelPragma (node *spap, struct location loc)
{
    DBUG_ENTER ();

    DBUG_ASSERT (spap != NULL, "NULL pointer for funcall in gpukernel pragma!");
    DBUG_ASSERT (NODE_TYPE (spap) == N_spap, "non N_spap funcall in gpukernel pragma!");

    if (!STReq (SPAP_NAME (spap), "GridBlock")) {
        CTIabort (NODE_LOCATION (spap), "Expected `GridBlock' but found `%s'",
                  SPAP_NAME (spap));
    }

    while (spap != NULL) {
        if (NODE_TYPE (spap) == N_spid) {
            if (!STReq (SPID_NAME (spap), "Gen")) {
                CTIabort (NODE_LOCATION (spap),
                          "Expected 'Gen` but found `%s'",
                          SPID_NAME (spap));
            }
            spap = NULL;
#define WLP(fun, args, checkfun)                                                         \
    }                                                                                    \
    else if (STReq (SPAP_NAME (spap), #fun))                                             \
    {                                                                                    \
        if (SPAP_ARGS (spap) == NULL) {                                                  \
            CTIabort (loc, "Missing argument in `%s' ()", #fun);                         \
            spap = NULL;                                                                 \
        } else {                                                                         \
            spap = GKCHcheck##fun (SPAP_ARGS (spap));                                    \
            if (spap == NULL) {                                                          \
                CTIabort (loc, "Missing inner gpukernel within `%s'", #fun);             \
            } else {                                                                     \
                if (EXPRS_NEXT (spap) != NULL) {                                         \
                    CTIabort (NODE_LOCATION (EXPRS_EXPR (EXPRS_NEXT (spap))),            \
                              "Superfluous argument within `%s'", #fun);                 \
                }                                                                        \
                spap = EXPRS_EXPR (spap);                                                \
                if ((NODE_TYPE (spap) != N_spap) && (NODE_TYPE (spap) != N_spid)) {      \
                    CTIabort (NODE_LOCATION (spap),                                      \
                              "Missing inner gpukernel within `%s'",                     \
                              #fun);                                                     \
                }                                                                        \
            }                                                                            \
        }

#include "gpukernel_funs.mac"
#undef WLP
        } else {
            CTIabort (NODE_LOCATION (spap),
                      "Expected gpukernel function but found `%s'",
                      SPAP_NAME (spap));
            spap = NULL;
        }
    }

    DBUG_RETURN ();
}

/******************************************************************************
 ******************************************************************************
 **
 **  Here the funs for the generic gpukernel-pragmas are defined.
 **
 **  All gpukernel-check-funs have the signature
 **    node *GKCHcheck<name>( node *args)
 **
 **  they obtain the pointer to an N_exprs node "args" that needs to contain
 **  the first argument of the given gpukernel function.
 **  All they do is to check the gpukernel function specific arguments and
 **  to progress the args pointer behind the function specific arguments.
 **
 **  So the returned pointer *should* be an N_exprs node containing the
 **  inner gpu-kernel function (either N_spap or N_spid for "Gen").
 **  However, these properties are not checked here but from the generic
 **  calling context in GKCHcheckGpuKernelPragma.
 **
 **  Since all these functions are identically constructed, we auto-generate
 **  them from our spec in gpukernel_funs.mac.
 **/

// TODO: check inner Pragma as well?
#define WLP(fun, nargs, checkfun)                                                        \
    node *GKCHcheck##fun (node *args)                                                    \
    {                                                                                    \
        DBUG_ENTER ();                                                                   \
        args = checkfun (args, #fun);                                                    \
        DBUG_RETURN (args);                                                              \
    }

#include "gpukernel_funs.mac"

#undef WLP

#undef DBUG_PREFIX
