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

/******************************************************************************
 *
 * Function:
 *   bool GKFcheckGpuKernelPragma (node *spap)
 *
 * Description:
 *
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


/******************************************************************************
 ******************************************************************************
 **
 **  Here the funs for gpukernel-pragmas are defined.
 **
 **  All gpukernel-check-funs have the signature
 **    node *GKFcheck<name>( node *args)
 **
 **  they return the last argument which either needs to be anothe N_spap, or
 **  an N_spid (in case of the inner "Gen").
 **
 **/

/******************************************************************************
 *
 * Function:
 *   node *GKFcheckGridBlock (node *args, struct location loc)
 *
 * Description:
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
 *
 * Function:
 *   node *GKFcheckShift (node *args)
 *
 * Description:
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
 * Function:
 *   node *GKFcheckCompressGrid (node *args)
 *
 * Description:
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
 * Function:
 *   node *GKFcheckPermute (node *args)
 *
 * Description:
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
 * Function:
 *   node *GKFcheckFoldLast2 (node *args)
 *
 * Description:
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
 * Function:
 *   node *GKFcheckSplitLast (node *args)
 *
 * Description:
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
 * Function:
 *   node *GKFcheckPad (node *args)
 *
 * Description:
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
