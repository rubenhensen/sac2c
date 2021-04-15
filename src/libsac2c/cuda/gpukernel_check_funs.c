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
 *                         | Shift ( <vect>, <gpukernel_fun_ap>)
 *                         | CompressGrid ( <gpukernel_fun_ap>)
 *                         | Permute ( <vect>, <gpukernel_fun_ap>)
 *                         | FoldLast2 ( <gpukernel_fun_ap>)
 *                         | SplitLast ( [<num>,<num>], <gpukernel_fun_ap>)
 *                         | Pad ( <vect>, <gpukernel_fun_ap>)
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
 **  Functions for naive-compilation pragma
 **/

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
        CTIerrorLoc (NODE_LOCATION (spap), "expected `GridBlock' found `%s'",
                                           SPAP_NAME (spap));
    } else {
        /*
         * check validity of GridBlock arguments; return pointer to nested
         * N_spap / N_spid or NULL in case of an error.
         */
        spap = GKCHcheckGridBlock (SPAP_ARGS (spap), loc);
    }

    while (spap != NULL) {
        if (NODE_TYPE (spap) == N_spid) {
            if (!STReq (SPID_NAME (spap), "Gen")) {
                CTIerrorLoc (NODE_LOCATION (spap), "expected 'Gen`"
                                         " found `%s'", SPID_NAME (spap));
            }
            spap = NULL;
#define WLP(fun, args)                                                               \
        } else if (STReq (SPAP_NAME (spap), #fun)) {                                 \
            if (SPAP_ARGS (spap) == NULL) {                                          \
                CTIerrorLoc (loc,"missing argument in `%s' ()", #fun);               \
                spap = NULL;                                                         \
            } else {                                                                 \
                spap = GKCHcheck ## fun (SPAP_ARGS (spap));                          \
                if (spap == NULL) {                                                  \
                    CTIerrorLoc (loc,"missing inner gpukernel within `%s'", #fun);   \
                } else {                                                             \
                    if (EXPRS_NEXT (spap) != NULL) {                                 \
                        CTIerrorLoc (NODE_LOCATION (EXPRS_EXPR (EXPRS_NEXT (spap))), \
                                     "superfluous argument within `%s'", #fun);      \
                    }                                                                \
                    spap = EXPRS_EXPR (spap);                                        \
                    if ((NODE_TYPE (spap) != N_spap)                                 \
                       && (NODE_TYPE (spap) != N_spid)) {                            \
                    CTIerrorLoc (NODE_LOCATION (spap), "missing inner gpukernel"     \
                                                       " within `%s'", #fun);        \
                    }                                                                \
                }                                                                    \
            }
#include "gpukernel_funs.mac"
#undef WLP
        } else {
            CTIerrorLoc (NODE_LOCATION (spap), "expected gpukernel function,"
                                               " found `%s'", SPAP_NAME (spap));
            spap = NULL;
        }
    }

    DBUG_RETURN ();
}


/** <!--********************************************************************-->
 *
 * @fn  node *GKCHcheckGridBlock (node *args, struct location loc)
 *
 * @param args - N_exprs node containing the first argument
 * @param loc - location of the parser *after* the funcall has been parsed
 *
 * @brief checks for args GridBlock ( <num>, <inner>) where
 *          <num> needs to be an N_num node and
 *          <inner> needs to be either N_spap or N_spid
 *
 *        returns <inner> (N_spap/N_spid) if check succeeds, NULL otherwise.
 *
 ******************************************************************************/

node *
GKCHcheckGridBlock (node *args, struct location loc)
{
    DBUG_ENTER ();

    if (args == NULL) {
        CTIerrorLoc (loc, "missing argument in `GridBlock ()'");
    } else {
        if (NODE_TYPE (EXPRS_EXPR (args)) != N_num) {
            CTIerrorLoc (NODE_LOCATION (EXPRS_EXPR (args)),
                         "wrong first argument; should be `GridBlock ( <num>, <inner>)'");
        }
        args = EXPRS_NEXT (args);
        if (args == NULL) {
            CTIerrorLoc (loc,"missing inner gpukernel; should be"
                             " `GridBlock ( <num>, <inner>)'");
        } else {
            if (EXPRS_NEXT (args) != NULL) {
                CTIerrorLoc (NODE_LOCATION (EXPRS_EXPR (EXPRS_NEXT (args))),
                             "superfluous argument; should be"
                             " `GridBlock ( <num>, <inner>)'");
            }
            args = EXPRS_EXPR (args);
            if ((NODE_TYPE (args) != N_spap)
                 && (NODE_TYPE (args) != N_spid)) {
                CTIerrorLoc (NODE_LOCATION (args), "wrong second argument; should be"
                                                   " `GridBlock ( <num>, <inner>)'");
                args = NULL;
            }
        }
    }

    DBUG_RETURN (args);
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
 **/

/******************************************************************************
 *
 * @fn node *GKCHcheckShift (node *args)
 *
 * @param args - N_exprs node containing the first argument to Shift
 *
 * @brief:
 *
 ******************************************************************************/

node *
GKCHcheckShift (node *args)
{
    DBUG_ENTER ();
    args = EXPRS_NEXT (args);
    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * @fn node *GKCHcheckCompressGrid (node *args)
 *
 * @param args - N_exprs node containing the first argument to CompressGrid
 *
 * @brief: as CompressGrid has no function specific arg, it simply returns its
 *         argument!
 *
 ******************************************************************************/

node *
GKCHcheckCompressGrid (node *args)
{
    DBUG_ENTER ();
    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * @fn node *GKCHcheckPermute (node *args)
 *
 * @param args - N_exprs node containing the first argument to Permute
 *
 * @brief:
 *
 ******************************************************************************/

node *
GKCHcheckPermute (node *args)
{
    DBUG_ENTER ();
    args = EXPRS_NEXT (args);
    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * @fn node *GKCHcheckFoldLast2 (node *args)
 *
 * @param args - N_exprs node containing the first argument to FoldLast2
 *
 * @brief: as FoldLast2 has no function specific arg, it simply returns its 
 *         argument!
 *
 ******************************************************************************/

node *
GKCHcheckFoldLast2 (node *args)
{
    DBUG_ENTER ();
    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * @fn node *GKCHcheckSplitLast (node *args)
 *
 * @param args - N_exprs node containing the first argument to SplitLast
 *
 * @brief:
 *
 ******************************************************************************/

node *
GKCHcheckSplitLast (node *args)
{
    DBUG_ENTER ();
    args = EXPRS_NEXT (args);
    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * @fn  node *GKCHcheckPad (node *args)
 *
 * @param args - N_exprs node containing the first argument to Pad
 *
 * @brief:
 *
 ******************************************************************************/

node *
GKCHcheckPad (node *args)
{
    DBUG_ENTER ();
    args = EXPRS_NEXT (args);
    DBUG_RETURN (args);
}





#undef DBUG_PREFIX
