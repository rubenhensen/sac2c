/*
 *
 * $Log$
 * Revision 1.1  2004/08/09 14:56:52  ktr
 * Initial revision
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "dbug.h"
#include "ssa.h"
#include "alloc.h"
#include "ConstVarPropagation.h"
#include "SSADeadCodeRemoval.h"

/** <!--********************************************************************-->
 *
 * @fn ExplicitAllocation
 *
 *   @brief
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/
node *
ExplicitAllocation (node *arg_node)
{
    node *fundef;
    DBUG_ENTER ("ExplicitAllocation");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "ExplicitAllocation not started with modul node");

    if (emm) {
        /*
         * Transformation into ssa form
         *
         * !!! IF THIS BECOMES UNNECESSARY ONE DAY: !!!
         *     CVP and DCR can be removed as well
         */
        arg_node = DoSSA (arg_node);
        if ((break_after == PH_alloc)
            && ((0 == strcmp (break_specifier, "l2f"))
                || (0 == strcmp (break_specifier, "cha"))
                || (0 == strcmp (break_specifier, "ssa")))) {
            goto DONE;
        }

        /*
         * Constant and variable propagation
         *
         * !!! Only needed as long we retransform in SSA form
         */
        fundef = MODUL_FUNS (arg_node);
        while (fundef != NULL) {
            if (!(FUNDEF_IS_LACFUN (fundef))) {
                fundef = ConstVarPropagation (fundef);
            }

            fundef = FUNDEF_NEXT (fundef);
        }
        if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "cvp"))) {
            goto DONE;
        }

        /*
         * Dead code removal
         *
         * !!! Only needed as long we retransform in SSA form
         */
        fundef = MODUL_FUNS (arg_node);
        while (fundef != NULL) {
            fundef = SSADeadCodeRemoval (fundef, arg_node);

            fundef = FUNDEF_NEXT (fundef);
        }
        if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "dcr"))) {
            goto DONE;
        }

        /*
         * Explicit allocation
         */
        arg_node = EMAllocateFill (arg_node);
        if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "emal"))) {
            goto DONE;
        }

        /*
         * In Place computation
         */
        /*
         * ...to be implemented...
         */
        if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "ipc"))) {
            goto DONE;
        }

        /*
         * Dead code removal
         */
        fundef = MODUL_FUNS (arg_node);
        while (fundef != NULL) {
            fundef = SSADeadCodeRemoval (fundef, arg_node);

            fundef = FUNDEF_NEXT (fundef);
        }
        if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "dcr2"))) {
            goto DONE;
        }

        /*
         * Reuse inference
         */
        /*
         * ...to be implemented...
         */
        if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "reuse"))) {
            goto DONE;
        }
    }

DONE:
    DBUG_RETURN (arg_node);
}
