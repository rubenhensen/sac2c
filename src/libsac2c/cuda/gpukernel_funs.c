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
 *   Once that is found, the parser calls     GKFcheckGpuKernelPragma( ...) from
 *   this module. GKFcheckGpuKernelPragma expects two parameters: an
 *   N_spap node that represents the just parsed funcall and a location containing
 *   the location right after the last symbol parsed so far.
 *
 *   GKFcheckGpuKernelPragma checks whether this is a legitimate nesting of 
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
 *  checked through function-specific check functions named GKFcheckSplit,....
 *
 *  IF a function <myfun> needs to be added, this requires 
 *    1) an additional entry in the file gpukernel_funs.mac of the form:
 *
 *       WLP (GKFcheck<myfun>, "<myfun>")
 *
 *    2) an additional function definition in this file of the function
 *       
 *       node *GKFcheck<myfun>( node *args) 
 *
 *       which checks any arguments specific to <myfun>.
 *  These two extension suffice to implement the whole extension from the
 *  perspective of the scanner / parser.
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

#define DBUG_PREFIX "GKF"
#include "debug.h"

#include "free.h"
#include "DupTree.h"
#include "gpukernel_funs.h"
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
 * @fn void GKFcheckGpuKernelPragma (node *spap, struct location loc)
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
GKFcheckGpuKernelPragma (node *spap, struct location loc)
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
        spap = GKFcheckGridBlock (SPAP_ARGS (spap), loc);
    }

    while (spap != NULL) {
        if (NODE_TYPE (spap) == N_spid) {
            if (!STReq (SPID_NAME (spap), "Gen")) {
                CTIerrorLoc (NODE_LOCATION (spap), "expected 'Gen`"
                                         " found `%s'", SPID_NAME (spap));
            }
            spap = NULL;
#define WLP(checkfun, str)                                                           \
        } else if (STReq (SPAP_NAME (spap), str)) {                                  \
            if (SPAP_ARGS (spap) == NULL) {                                          \
                CTIerrorLoc (loc,"missing argument in `%s' ()", str);                \
                spap = NULL;                                                         \
            } else {                                                                 \
                spap = checkfun (SPAP_ARGS (spap));                                  \
                if (spap == NULL) {                                                  \
                    CTIerrorLoc (loc,"missing inner gpukernel within `%s'", str);    \
                } else {                                                             \
                    if (EXPRS_NEXT (spap) != NULL) {                                 \
                        CTIerrorLoc (NODE_LOCATION (EXPRS_EXPR (EXPRS_NEXT (spap))), \
                                     "superfluous argument within `%s'", str);       \
                    }                                                                \
                    spap = EXPRS_EXPR (spap);                                        \
                    if ((NODE_TYPE (spap) != N_spap)                                 \
                       && (NODE_TYPE (spap) != N_spid)) {                            \
                    CTIerrorLoc (NODE_LOCATION (spap), "missing inner gpukernel"     \
                                                       " within `%s'", str);         \
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
 * @fn  node *GKFcheckGridBlock (node *args, struct location loc)
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
GKFcheckGridBlock (node *args, struct location loc)
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
 **    node *GKFcheck<name>( node *args)
 **
 **  they obtain the pointer to an N_exprs node "args" that needs to contain
 **  the first argument of the given gpukernel function.
 **  All they do is to check the gpukernel function specific arguments and
 **  to progress the args pointer behind the function specific arguments.
 **
 **  So the returned pointer *should* be an N_exprs node containing the
 **  inner gpu-kernel function (either N_spap or N_spid for "Gen").
 **  However, these properties are not checked here but from the generic
 **  calling context in GKFcheckGpuKernelPragma.
 **
 **/

/******************************************************************************
 *
 * @fn node *GKFcheckShift (node *args)
 *
 * @param args - N_exprs node containing the first argument to Shift
 *
 * @brief:
 *
 ******************************************************************************/

node *
GKFcheckShift (node *args)
{
    DBUG_ENTER ();
    args = EXPRS_NEXT (args);
    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * @fn node *GKFcheckCompressGrid (node *args)
 *
 * @param args - N_exprs node containing the first argument to CompressGrid
 *
 * @brief: as CompressGrid has no function specific arg, it simply returns its
 *         argument!
 *
 ******************************************************************************/

node *
GKFcheckCompressGrid (node *args)
{
    DBUG_ENTER ();
    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * @fn node *GKFcheckPermute (node *args)
 *
 * @param args - N_exprs node containing the first argument to Permute
 *
 * @brief:
 *
 ******************************************************************************/

node *
GKFcheckPermute (node *args)
{
    DBUG_ENTER ();
    args = EXPRS_NEXT (args);
    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * @fn node *GKFcheckFoldLast2 (node *args)
 *
 * @param args - N_exprs node containing the first argument to FoldLast2
 *
 * @brief: as FoldLast2 has no function specific arg, it simply returns its 
 *         argument!
 *
 ******************************************************************************/

node *
GKFcheckFoldLast2 (node *args)
{
    DBUG_ENTER ();
    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * @fn node *GKFcheckSplitLast (node *args)
 *
 * @param args - N_exprs node containing the first argument to SplitLast
 *
 * @brief:
 *
 ******************************************************************************/

node *
GKFcheckSplitLast (node *args)
{
    DBUG_ENTER ();
    args = EXPRS_NEXT (args);
    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * @fn  node *GKFcheckPad (node *args)
 *
 * @param args - N_exprs node containing the first argument to Pad
 *
 * @brief:
 *
 ******************************************************************************/

node *
GKFcheckPad (node *args)
{
    DBUG_ENTER ();
    args = EXPRS_NEXT (args);
    DBUG_RETURN (args);
}


#undef DBUG_PREFIX
