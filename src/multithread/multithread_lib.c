/*
 *
 * $Log$
 * Revision 3.13  2004/11/23 14:38:13  skt
 * SACDevCampDK 2k4
 *
 * Revision 3.12  2004/11/21 14:22:39  skt
 * uncomment some old stuff to make it run this newast
 *
 * Revision 3.11  2004/09/28 13:22:48  ktr
 * Removed generatemasks.
 *
 * Revision 3.10  2004/08/26 17:01:36  skt
 * moved MUTHDecodeExecmode from multithread to multithread_lib
 *
 * Revision 3.9  2004/08/18 13:24:31  skt
 * switch to mtexecmode_t done
 *
 * Revision 3.8  2004/08/05 17:42:19  skt
 * TagAllocs added
 *
 * Revision 3.7  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
 * Revision 3.6  2004/07/28 17:47:40  skt
 * added N_ex into assertion
 *
 * Revision 3.5  2004/07/26 16:53:07  skt
 * added support for exclusive cells
 *
 * Revision 3.4  2004/06/08 14:40:22  skt
 * MUTHGetLastExpression added
 *
 * Revision 3.3  2001/05/17 11:46:31  dkr
 * FREE eliminated
 *
 * Revision 3.2  2001/03/21 18:05:11  dkr
 * INFO_DUP_... no longer used here
 *
 * Revision 3.1  2000/11/20 18:03:12  sacbase
 * new release made
 *
 * Revision 1.8  2000/07/13 08:24:24  jhs
 * Moved DupMask_ InsertBlock, InsertMT and InsertST from blocks_init.[ch]
 * Renamed InsertXX to MUTHInsertXX.
 *
 * Revision 1.7  2000/06/23 15:13:47  dkr
 * signature of DupTree changed
 *
 * Revision 1.6  2000/04/14 17:43:26  jhs
 * Comments ...
 *
 * Revision 1.5  2000/04/10 15:45:08  jhs
 * Added Reduce
 *
 * Revision 1.4  2000/03/09 18:34:40  jhs
 * Additional features.
 *
 * Revision 1.3  2000/03/02 12:59:35  jhs
 * Added MUTHExchangeApplication,
 * added MUTHExpandFundefName,
 * added DBUG_PRINTS and DBUG_ASSERTS.
 *
 * Revision 1.2  2000/02/22 15:48:50  jhs
 * Adapted NODE_TEXT.
 *
 * Revision 1.1  2000/02/21 17:48:22  jhs
 * Initial revision
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

#include "dbug.h"

#include <string.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "multithread_lib.h"
#include "internal_lib.h"

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
    char *new_name;

    DBUG_ENTER ("MUTHLIBexpandFundefName");

    old_name = FUNDEF_NAME (fundef);
    new_name = ILIBmalloc (strlen (old_name) + strlen (prefix) + 1);
    strcpy (new_name, prefix);
    strcat (new_name, old_name);
    FUNDEF_NAME (fundef) = new_name;
    old_name = ILIBfree (old_name);

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
    DBUG_ENTER ("RenewExecutionmode");
    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign),
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
    DBUG_ENTER ("MUTHLIBtagAllocs");
    DBUG_ASSERT ((NODE_TYPE (withloop) == N_with2),
                 "MUTHLIBtagAllocs expects a N_with2 as #1 argument");

    /* work on the withop */
    wlops = WITH2_WITHOP (withloop);
    while (wlops != NULL) {
        if ((WITHOP_TYPE (wlops) == WO_genarray)
            || (WITHOP_TYPE (wlops) == WO_modarray)) {
            assign = AVIS_SSAASSIGN (ID_AVIS (WITHOP_MEM (wlops)));

            DBUG_ASSERT ((ASSIGN_EXECMODE (assign) != MUTH_MULTI),
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

    DBUG_VOID_RETURN;
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
    DBUG_ENTER ("MUTHLIBdecodeExecmode");
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
