/*
 *
 * $Log$
 * Revision 1.13  2005/07/17 20:13:33  sbs
 * DFC and RC moved into separate phase now.
 *
 * Revision 1.12  2005/07/16 21:13:51  sbs
 * moved dispatch and rmcasts here
 *
 * Revision 1.11  2005/07/03 17:06:32  ktr
 * switched to phase.h
 *
 * Revision 1.10  2005/06/14 08:52:04  khf
 * added traversal in subphase WLDP
 *
 * Revision 1.9  2005/04/19 18:01:23  khf
 * removed transformation in and out ssa-form
 *
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

#include "WLEnhancement.h"

#include "phase.h"
#include "tree_basic.h"
#include "dbug.h"
#include "dispatchfuncalls.h"
#include "rmcasts.h"

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
WLEdoWlEnhancement (node *syntax_tree)
{

    DBUG_ENTER ("WLEdoWlEnhancement");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "WLEdoWlEnhancement not started with modul node");

    DBUG_PRINT ("WLE", ("starting WLEdoWlEnhancement"));

    /*
     * Explicit accumulation
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_accu, syntax_tree);

    /*
     * Default partition generation
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wldp, syntax_tree);

    /*
     * With-loop partition generation
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wlpg, syntax_tree);

    DBUG_RETURN (syntax_tree);
}
