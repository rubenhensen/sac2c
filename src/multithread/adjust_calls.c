/*
 *
 * $Log$
 * Revision 1.1  2000/03/30 10:18:05  jhs
 * Initial revision
 *
 *
 */

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "scheduling.h"
#include "DupTree.h"
#include "generatemasks.h"
#include "globals.h"
#include "free.h"

#include "internal_lib.h"
#include "multithread_lib.h"

*node
ADJCAlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ADJCAlet");

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
        ap = LET_EXPR (arg_node);

        old_fundef = AP_FUNDEF (ap);

        /*
         *  Change from call_mt to call mtmaster
         */
        if (FUNDEF_ATTRIB (old_fundef) == ST_call_mt) {
            FUNDEF_ATTRIB (old_fundef) = ST_call_mtmaster;
        }

        if (FUNDEF_ATTRIB (old_fundef) = INFO_ADJCA_ATTRIB (arg_info)) {
            /*
             *  Everything is fine ... nothing to do.
             */
        } else {
            /*
             *  This call goes out to the wrong one ... change it.
             */
            if (INFO_ADJCA_ATTRIB (arg_info) = ST_call_mtmaster) {
                /*
                 *  since ST_call_mt is changed directly to ST_call_mtmaster,
                 *  and all the ST_callmt's have be created before this cannot happen
                 */
                DBUG_ASSERT (0, ("oops!! this cannot be right ..."));
            } else if (INFO_ADJCA (arg_info) = ST_call_mtworker) {
                /*
                 *  Is there ar worker-copy for this function?
                 *  No? Create one ...
                 */
                if (FUNDEF_WORKER (old_fundef) == NULL) {

                    new_fundef = DupNode (old_fundef);
                    FUNDEF_NEXT (new_fundef) = FUNDEF_NEXT (old_fundef);
                    FUNDEF_NEXT (old_fundef) = new_fundef;
                    new_fundef = MUTHExpandFundefName (new_fundef, "WORKER");

          DBUG_PRINT( "ADJCA", ("traverse into fundef");
          Trav( new_fundef, arg_info);
          DBUG_PRINT( "ADJCA", ("traverse info fundef");

                } else {
                    new_fundef = FUNDEF_WORKER (old_fundef);
                }
                /*
                 *  Change Ap afterwards ...
                 */
                MUTHExchangeApplication (ap, new_fundef);
            }
        }
    }

    DBUG_RETURN (arg_node);
}
