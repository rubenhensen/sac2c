/*
 *
 * $Id$
 *
 */

/*****************************************************************************
 *
 * file:   multithread_lib.c
 *
 * prefix: MUTHLIB
 *
 * description:
 *   helper functions for multithread-compilation
 *
 *****************************************************************************/

#include "tree_basic.h"
#include "tree_compound.h"
#include "multithread_lib.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

/******************************************************************************
 *
 * function:
 *   node *MUTHLIBexpandFundefName (node *fundef, char *prefix)
 *
 * description:
 *   Changes the name of the fundef. The prefix and original will will be
 *   concatenated to a new string n and set n is set as FUNDEF_NAME.
 *   The original name will be set free automatically.
 *
 * attention:
 *   THIS DOES NOT CHANGE THE NAMES IN N_AP.
 *   N_ap holds a copy of the name (I don't know why, but this is the fact),
 *   so you have to change *all* the names in corresponding N_ap.
 *
 ******************************************************************************/
node *
MUTHLIBexpandFundefName (node *fundef, char *prefix)
{
    char *old_name;

    DBUG_ENTER ();

    old_name = FUNDEF_NAME (fundef);
    FUNDEF_NAME (fundef) = STRcat (prefix, old_name);
    old_name = MEMfree (old_name);

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn node *RenewExecutionmode(node *assign, mtexecmode_t executionmode)
 *
 *   @brief renew the executionmode of the assignment
 *
 *   @param assign an allocation, somehow involved with a with-loop
 *   @param executionmode the executiomode of the with-loop
 *   @return the assignment with the renewed executionmode
 *
 *****************************************************************************/
static node *
RenewExecutionmode (node *assign, mtexecmode_t executionmode)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (assign) == N_assign,
                 "RenewExecutionmode expects a N_assign as #1 arg.");

    /* change the executionmode, if it's no MUTH_EXCLUSIVE already */
    if (ASSIGN_EXECMODE (assign) != MUTH_EXCLUSIVE) {
        if (executionmode == MUTH_EXCLUSIVE) {
            ASSIGN_EXECMODE (assign) = MUTH_EXCLUSIVE;
        } else {
            ASSIGN_EXECMODE (assign) = MUTH_SINGLE;
        }
    }

    DBUG_RETURN (assign);
}

/** <!--********************************************************************-->
 *
 * @fn void MUTHLIBtagAllocs(node *withloop, mtexecmode_t executionmode)
 *
 *   @brief This function tags the executionmode of the allocation-assignments
 *          belonging to the with-loop, i.e. the allocations of the withop an
 *          of the withid
 *   <pre>
 *         There are two miniphases that call TagAllocs: TagExecutionmode and
 *         PropagateExecutionmode. A call by the 1st one means, the with-loop
 *         is tagged to be calculated parallel, which implicits
 *         - allocation of the wlops has to be made in ST- or EX-mode (depends
 *           on their original executionmode)
 *         - allocation of the with-id has to be made in MT-mode
 *         A call by the 2nd one means, the executionmode of the with-loop
 *         differs from the inferred executionmode in TagExecutionmode, so
 *         let's adapt the allocations:
 *         - allocation of the wlops has to be made in ST- or EX-mode (depends
 *           on their original executionmode and on the 2nd parameter)
 *         - allocation of the with-id has to be done in executionmode
 *   </pre>
 *
 *   @param withloop
 *   @param executionmode the actual executionmode of the with-loop
 *   @return nothing at all
 *
 *****************************************************************************/
void
MUTHLIBtagAllocs (node *withloop, mtexecmode_t executionmode)
{
    node *assign;
    node *wlops;
    node *iterator;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (withloop) == N_with2,
                 "MUTHLIBtagAllocs expects a N_with2 as #1 argument");

    /* work on the withop */
    wlops = WITH2_WITHOP (withloop);
    while (wlops != NULL) {
        if ((NODE_TYPE (wlops) == N_genarray) || (NODE_TYPE (wlops) == N_modarray)) {
            assign = AVIS_SSAASSIGN (ID_AVIS (WITHOP_MEM (wlops)));

            DBUG_ASSERT (ASSIGN_EXECMODE (assign) != MUTH_MULTI,
                         "The execmode of the alloc-assign must'n be MUTH_MULTI");

            assign = RenewExecutionmode (assign, executionmode);
        }
        wlops = WITHOP_NEXT (wlops);
    }

    /* handle the with-id vector */
    iterator = WITHID_VEC (WITH2_WITHID (withloop));
    ASSIGN_EXECMODE (AVIS_SSAASSIGN (IDS_AVIS (iterator))) = executionmode;

    /* handle the with-id vector elements */
    iterator = WITHID_IDS (WITH2_WITHID (withloop));
    while (iterator != NULL) {
        ASSIGN_EXECMODE (AVIS_SSAASSIGN (IDS_AVIS (iterator))) = executionmode;

        iterator = IDS_NEXT (iterator);
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn char *MUTHLIBdecodeExecmode(mtexecmode_t execmode)
 *
 *   @brief A small helper function to make debug-output more readable
 *          !It must be adapted if the names of the modes change!
 *
 *   @param execmode the executionmode to decode
 *   @return the name of the executionmode as a string
 *
 *****************************************************************************/
char *
MUTHLIBdecodeExecmode (mtexecmode_t execmode)
{
    char *result;
    DBUG_ENTER ();
    switch (execmode) {
    case MUTH_ANY:
        result = "AT";
        break;
    case MUTH_EXCLUSIVE:
        result = "EX";
        break;
    case MUTH_SINGLE:
        result = "ST";
        break;
    case MUTH_MULTI:
        result = "MT";
        break;
    case MUTH_MULTI_SPECIALIZED:
        result = "MS";
        break;
    default:
        result = "";
        break;
    }
    DBUG_RETURN (result);
}

#undef DBUG_PREFIX
