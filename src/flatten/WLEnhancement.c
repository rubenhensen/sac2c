/*
 *
 * $Log$
 * Revision 1.8  2005/01/11 11:19:19  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 1.7  2004/11/25 14:12:29  khf
 * SacDevCamp04
 *
 * Revision 1.6  2004/11/24 20:42:26  khf
 * SacDevCamp04: COMPILES!
 *
 * Revision 1.5  2004/09/23 12:01:20  khf
 * some DBUG_PRINTs inserted
 *
 * Revision 1.4  2004/08/09 13:13:31  khf
 * some comments added
 *
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
#include <string.h>

#include "tree_basic.h"
#include "node_basic.h"
#include "globals.h"
#include "dbug.h"
#include "ssa.h"
#include "WLEnhancement.h"
#include "ExplicitAccumulate.h"
#include "WLPartitionGeneration.h"

/** <!--********************************************************************-->
 *
 * @fn node *WLEdoWlEnhancement( node *arg_node)
 *
 *   @brief  this function applies ExplicitAccumulate and
 *           WLPartitionGeneration on the syntax tree.
 *           Both works only in ssa-form, so we have to transform
 *           the syntax tree in ssa-form.
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/

node *
WLEdoWlEnhancement (node *arg_node)
{

    DBUG_ENTER ("WLEdoWlEnhancement");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "WLEdoWlEnhancement not started with modul node");

    DBUG_PRINT ("WLE", ("starting WLEdoWlEnhancement"));

    DBUG_PRINT ("WLE", ("call SSAdoSSA"));
    /* transformation in ssa-form */
    arg_node = SSAdoSsa (arg_node);
    /* necessary to guarantee, that the compilation can be stopped
       during the call of SSAdoSsa */
    if ((global.break_after == PH_wlenhance)
        && ((0 == strcmp (global.break_specifier, "l2f"))
            || (0 == strcmp (global.break_specifier, "cha"))
            || (0 == strcmp (global.break_specifier, "ssa")))) {
        goto DONE;
    }

    DBUG_PRINT ("WLE", ("call EAdoExplicitAccumulate"));
    arg_node = EAdoExplicitAccumulate (arg_node);

    if ((global.break_after == PH_wlenhance)
        && (0 == strcmp (global.break_specifier, "ea"))) {
        goto DONE;
    }

    DBUG_PRINT ("WLE", ("call WLPGdoWlPartitionGeneration"));
    arg_node = WLPGdoWlPartitionGeneration (arg_node);

    if ((global.break_after == PH_wlenhance)
        && ((0 == strcmp (global.break_specifier, "wlpg"))
            || (0 == strcmp (global.break_specifier, "cf")))) {
        goto DONE;
    }

    DBUG_PRINT ("WLE", ("call SSAundoSSA"));
    /* undo tranformation in ssa-form */
    arg_node = SSAundoSsa (arg_node);
    /* necessary to guarantee, that the compilation can be stopped
       during the call of SSAundoSsa */
    if ((global.break_after == PH_wlenhance)
        && ((0 == strcmp (global.break_specifier, "ussa"))
            || (0 == strcmp (global.break_specifier, "f2l")))) {
        goto DONE;
    }

DONE:
    DBUG_RETURN (arg_node);
}
