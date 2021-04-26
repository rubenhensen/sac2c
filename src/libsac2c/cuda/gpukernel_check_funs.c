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

static node*
checkNone(node* args, const char* name) {
    DBUG_ENTER ();
    DBUG_RETURN (args);
}

static node*
checkNumArg(node* args, const char* name) {
    node* arg;
    DBUG_ENTER ();

    arg = EXPRS_EXPR(args);
    if (NODE_TYPE (arg) != N_num) {
        CTIerrorLoc(NODE_LOCATION (arg), "wrong first argument to %s;"
                                         " should be `%s ( <num>, <inner>)'", name, name);
    }

    args = EXPRS_NEXT(args);
    DBUG_RETURN (args);
}

static node*
checkNumsArg(node* args, const char* name) {
    node* arg, * exprs;
    DBUG_ENTER ();

    arg = EXPRS_EXPR(args);
    if (NODE_TYPE (arg) != N_array) {
        CTIerrorLoc(NODE_LOCATION (arg), "wrong first argument to %s;"
                                         " should be `%s ( [<nums>], <inner>)'", name, name);
    } else {
        exprs = ARRAY_AELEMS(arg);
        while (exprs != NULL) {
            arg = EXPRS_EXPR(exprs);
            if (NODE_TYPE (arg) != N_num) {
                CTIerrorLoc(NODE_LOCATION (arg), "wrong first argument to %s;"
                                                 " should be `%s ( [<nums>], <inner>)'", name, name);
            }
            exprs = EXPRS_NEXT(exprs);
        }
    }

    args = EXPRS_NEXT(args);
    DBUG_RETURN (args);
}

static node*
checkPermutationArg(node* args, const char* name) {
    node* arg, *exprs;
    DBUG_ENTER();

    checkNumsArg(args, name);

    size_t length = 0;
    arg = EXPRS_EXPR(args);
    exprs = ARRAY_AELEMS(arg);
    while (exprs != NULL){
        length ++;
        exprs = EXPRS_NEXT(exprs);
    }

    bool* perm_hits = (bool*) MEMmalloc(sizeof(bool*)*length);
    for (size_t i=0; i<length; i++)
        perm_hits[i] = false;

    exprs = ARRAY_AELEMS(arg);
    while(exprs != NULL) {
        arg = EXPRS_EXPR(exprs);
        int point_dim = NUM_VAL(arg);
        if (point_dim < 0 || point_dim >= (int) length || perm_hits[point_dim])
            CTIerrorLoc(NODE_LOCATION (arg), "wrong first argument to %s;"
                                             " should be `%s ( [<permutation>], <inner>)'", name, name);
        perm_hits[point_dim] = true;
        exprs = EXPRS_NEXT(exprs);
    }

    MEMfree(perm_hits);

    DBUG_RETURN(args);
}

static node*
checkZONumsArg(node* args, const char* name) {
    node* arg, * exprs;
    DBUG_ENTER ();

    arg = EXPRS_EXPR(args);
    if (NODE_TYPE (arg) != N_array) {
        CTIerrorLoc(NODE_LOCATION (arg), "wrong first argument to %s;"
                                         " should be `%s ( [0/1, ..., 0/1], <inner>)'", name, name);
    } else {
        exprs = ARRAY_AELEMS(arg);
        while (exprs != NULL) {
            arg = EXPRS_EXPR(exprs);
            if ((NODE_TYPE (arg) != N_num) || ((NUM_VAL(arg) != 0) && (NUM_VAL(arg) != 1))) {
                CTIerrorLoc(NODE_LOCATION (arg), "wrong first argument to %s;"
                                                 " should be `%s ( [0/1, ..., 0/1], <inner>)'", name, name);
            }
            exprs = EXPRS_NEXT(exprs);
        }
    }

    args = EXPRS_NEXT(args);
    DBUG_RETURN (args);
}

void
checkArgsLength(node* arg, const size_t length, const char* name) {
    node* exprs;
    DBUG_ENTER ();

    exprs = ARRAY_AELEMS(arg);
    while (exprs != NULL) {
        arg = EXPRS_EXPR(exprs);
        if (exprs == NULL)
            CTIerrorLoc(NODE_LOCATION(arg), "wrong first argument to %s: too short; "
                                             "The list should be of length %zu", name, length);
        exprs = EXPRS_NEXT(exprs);
    }
    if (exprs != NULL)
        CTIerrorLoc(NODE_LOCATION(arg), "wrong first argument to %s: too long; "
                                         "The list should be of length %zu", name, length);

    DBUG_RETURN ();
}

void
checkNumLesseqDim(node* arg, const size_t length, const char* name) {
    DBUG_ENTER();

    int num = NUM_VAL(arg);

    if ((size_t) num > length)
        CTIerrorLoc(NODE_LOCATION(arg), "wrong first argument to %s; "
                                        "should be a positive integer less then %zu", name, length);

    DBUG_RETURN();
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
GKCHcheckGpuKernelPragma(node* spap, struct location loc) {
    DBUG_ENTER ();

    DBUG_ASSERT (spap != NULL, "NULL pointer for funcall in gpukernel pragma!");
    DBUG_ASSERT (NODE_TYPE(spap) == N_spap, "non N_spap funcall in gpukernel pragma!");

    if (!STReq(SPAP_NAME (spap), "GridBlock")) {
        CTIerrorLoc(NODE_LOCATION (spap), "expected `GridBlock' found `%s'",
                    SPAP_NAME (spap));
    } else {
        /*
         * check validity of GridBlock arguments; return pointer to nested
         * N_spap / N_spid or NULL in case of an error.
         */
        spap = GKCHcheckGridBlock(SPAP_ARGS(spap), loc);
    }

    while (spap != NULL) {
        if (NODE_TYPE (spap) == N_spid) {
            if (!STReq(SPID_NAME(spap), "Gen")) {
                CTIerrorLoc(NODE_LOCATION (spap), "expected 'Gen`"
                                                  " found `%s'", SPID_NAME(spap));
            }
            spap = NULL;
#define WLP(fun, args, checkfun)                                                     \
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

// @formatter:off
#include "gpukernel_funs.mac"
#undef WLP
// @formatter:on
        } else {
            CTIerrorLoc(NODE_LOCATION (spap), "expected gpukernel function,"
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

node*
GKCHcheckGridBlock(node* args, struct location loc) {
    DBUG_ENTER ();

    if (args == NULL) {
        CTIerrorLoc(loc, "missing argument in `GridBlock ()'");
    } else {
        if (NODE_TYPE (EXPRS_EXPR(args)) != N_num) {
            CTIerrorLoc(NODE_LOCATION (EXPRS_EXPR(args)),
                        "wrong first argument; should be `GridBlock ( <num>, <inner>)'");
        }
        args = EXPRS_NEXT(args);
        if (args == NULL) {
            CTIerrorLoc(loc, "missing inner gpukernel; should be"
                             " `GridBlock ( <num>, <inner>)'");
        } else {
            if (EXPRS_NEXT(args) != NULL) {
                CTIerrorLoc(NODE_LOCATION (EXPRS_EXPR(EXPRS_NEXT(args))),
                            "superfluous argument; should be"
                            " `GridBlock ( <num>, <inner>)'");
            }
            args = EXPRS_EXPR(args);
            if ((NODE_TYPE (args) != N_spap)
                && (NODE_TYPE (args) != N_spid)) {
                CTIerrorLoc(NODE_LOCATION (args), "wrong second argument; should be"
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
 **  Since all these functions are identically constructed, we auto-generate
 **  them from our spec in gpukernel_funs.mac.
 **/

#define WLP(fun, nargs, checkfun)   \
node *                              \
GKCHcheck ## fun (node *args)       \
{                                   \
    DBUG_ENTER ();                  \
    args = checkfun (args, #fun);   \
    DBUG_RETURN (args);             \
}

#include "gpukernel_funs.mac"

#undef WLP

#undef DBUG_PREFIX
