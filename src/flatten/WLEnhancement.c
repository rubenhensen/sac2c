/*
 *
 * $Log$
 * Revision 1.3  2004/08/04 12:16:22  khf
 * changed flag eacc to emm
 *
 * Revision 1.2  2004/07/23 15:24:04  khf
 * changed flag for explicit accumulation from ktr to eacc
 *
 * Revision 1.1  2004/07/21 12:35:41  khf
 * Initial revision
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "Error.h"
#include "globals.h"
#include "dbug.h"
#include "ssa.h"
#include "WLEnhancement.h"
#include "ExplicitAccumulate.h"
#include "WLPartitionGeneration.h"

/** <!--********************************************************************-->
 *
 * @fn node *WLEnhancement( node *arg_node)
 *
 *   @brief
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/

node *
WLEnhancement (node *arg_node)
{

    DBUG_ENTER ("WLEnhancement");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "WLEnhancement not started with modul node");

    DBUG_PRINT ("WLE", ("starting WLEnhancement"));

    if (emm || khf) {
        /* transformation in ssa-form */
        arg_node = DoSSA (arg_node);
        /* necessary to guarantee, that the compilation can be stopped
           during the call of DoSSA */
        if ((break_after == PH_wlenhance)
            && ((0 == strcmp (break_specifier, "l2f"))
                || (0 == strcmp (break_specifier, "cha"))
                || (0 == strcmp (break_specifier, "ssa")))) {
            goto DONE;
        }

        if (emm) {
            arg_node = ExplicitAccumulate (arg_node);

            if ((break_after == PH_wlenhance) && (0 == strcmp (break_specifier, "ea"))) {
                goto DONE;
            }
        }

        if (khf) {
            arg_node = WLPartitionGeneration (arg_node);

            if ((break_after == PH_wlenhance)
                && ((0 == strcmp (break_specifier, "wlpg"))
                    || (0 == strcmp (break_specifier, "cf")))) {
                goto DONE;
            }
        }

        /* undo tranformation in ssa-form */
        arg_node = UndoSSA (arg_node);
        /* necessary to guarantee, that the compilation can be stopped
           during the call of UndoSSA */
        if ((break_after == PH_wlenhance)
            && ((0 == strcmp (break_specifier, "ussa"))
                || (0 == strcmp (break_specifier, "f2l")))) {
            goto DONE;
        }
    }

DONE:
    DBUG_RETURN (arg_node);
}
