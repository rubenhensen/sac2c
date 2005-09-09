/*
 *
 * $Log$
 * Revision 1.18  2005/09/09 18:20:06  ktr
 * removed non needed stuff
 *
 * Revision 1.17  2005/09/04 12:52:50  ktr
 * re-engineered the optimization cycle
 *
 * Revision 1.16  2005/09/03 08:47:28  ktr
 * CVP is now performed for all functions
 *
 * Revision 1.15  2005/07/19 17:08:47  ktr
 * replaced SSADeadCodeRemoval with deadcoderemoval
 *
 * Revision 1.14  2005/07/03 16:59:18  ktr
 * Switched to phase.h
 *
 * Revision 1.13  2005/06/30 16:40:52  ktr
 * added AUD SCL distincion as first pass
 *
 * Revision 1.12  2005/06/06 13:29:23  jhb
 * added PHrunCompilerSubPhase
 *
 * Revision 1.11  2004/12/16 14:37:30  ktr
 * added InplaceComputation
 *
 * Revision 1.10  2004/12/09 21:09:26  ktr
 * bugfix roundup
 *
 * Revision 1.9  2004/12/08 21:22:24  ktr
 * Aliasing information is now printed correctly
 *
 * Revision 1.8  2004/12/01 16:33:57  ktr
 * Prefun is now used in order to print ALIAS information
 *
 * Revision 1.7  2004/11/26 23:36:06  jhb
 * ExplicitAllocation to EMAdoAllocation
 *
 * Revision 1.6  2004/11/25 18:04:36  jhb
 * SSAdoSSA change d to SSAdoSsa
 *
 * Revision 1.5  2004/11/23 19:29:35  ktr
 * COMPILES!!!
 *
 * Revision 1.4  2004/11/19 15:44:09  ktr
 * Support for F_alloc_or_reshape added.
 *
 * Revision 1.3  2004/10/11 14:26:09  ktr
 * removed emm.
 *
 * Revision 1.2  2004/08/10 16:14:33  ktr
 * reuse inference in EMM can now be activated using -reuse.
 *
 * Revision 1.1  2004/08/09 14:56:52  ktr
 * Initial revision
 *
 */

#include "allocation.h"

#include "phase.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "globals.h"
#include "dbug.h"
#include "print.h"

/** <!--********************************************************************-->
 *
 * @fn EMAprintPreFun
 *
 *   @brief
 *
 *   @param  node *arg_node
 *   @param  info *arg_info
 *
 *   @return node *           :  the transformed syntax tree
 *
 *****************************************************************************/
node *
EMAprintPreFun (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAprintPreFun");

    switch (NODE_TYPE (arg_node)) {
    case N_arg:
        if (ARG_ISALIASING (arg_node)) {
            fprintf (global.outfile, " /* ALIAS */");
        }
        if (AVIS_ISALIAS (ARG_AVIS (arg_node))) {
            fprintf (global.outfile, " /* alias */");
        }
        break;
    case N_ret:
        if (RET_ISALIASING (arg_node)) {
            fprintf (global.outfile, " /* ALIAS */");
        }
        break;
    case N_vardec:
        if (AVIS_ISALIAS (VARDEC_AVIS (arg_node))) {
            INDENT;
            fprintf (global.outfile, " /* alias */\n");
        }
        break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn EMAdoAllocation
 *
 *   @brief
 *
 *   @param  node *syntax_tree:  the whole syntax tree
 *   @return node *           :  the transformed syntax tree
 *
 *****************************************************************************/
node *
EMAdoAllocation (node *syntax_tree)
{
    DBUG_ENTER ("ExplicitAllocation");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "ExplicitAllocation not started with modul node");

    /*
     * Transformation into ssa form
     *
     * !!! IF THIS BECOMES UNNECESSARY ONE DAY: !!!
     *     CVP and DCR can be removed as well
     */
    DBUG_PRINT ("EMM", ("Transforming syntax tree into SSA form (lac2fun2, ssa2)"));
    syntax_tree = PHrunCompilerSubPhase (SUBPH_lac2fun2, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ssa2, syntax_tree);

    /*
     * Constant and variable propagation
     *
     * !!! Only needed as long we retransform in SSA form
     */
    if (global.optimize.docvp) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_cvpemm, syntax_tree);
    }

    /*
     * Dead code removal
     *
     * !!! Only needed as long we retransform in SSA form
     */
    if (global.optimize.dodcr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_dcremm, syntax_tree);
    }

    /*
     * AUD SCL distinction
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_asd, syntax_tree);

    /*
     * Explicit copy
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_copy, syntax_tree);

    /*
     * Explicit allocation
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_alloc, syntax_tree);

    /*
     * Dead code removal
     */
    if (global.optimize.dodcr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_dcremm2, syntax_tree);
    }

    /*
     * Reuse candidate inference
     */
    if (global.optimize.douip) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_rci, syntax_tree);
    }

    /*
     * Interface analysis
     */
    TRAVsetPreFun (TR_prt, EMAprintPreFun);
    if (global.optimize.dosrf) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_ia, syntax_tree);
    }

    /*
     * Loop reuse optimization
     */
    if ((global.optimize.dosrf) && (global.optimize.dolro)) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_lro, syntax_tree);
    }

    /*
     * Alias analysis
     */
    if (global.optimize.dosrf) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_aa, syntax_tree);
    }

    /*
     * Filter reuse candidates
     */
    if (global.optimize.douip) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_frc, syntax_tree);
    }

    /*
     * Static reuse
     */
    if ((global.optimize.douip) && (global.optimize.dosrf)) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_sr, syntax_tree);
    }

    /*
     * Reuse case dependent branching
     */
    if ((global.optimize.doipc) || ((global.optimize.douip) && (global.optimize.dodr))) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_rb, syntax_tree);
    }

    /*
     * In Place computation
     */
    if (global.optimize.doipc) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_ipc, syntax_tree);
    }

    /*
     * Data reuse
     */
    if ((global.optimize.douip) && (global.optimize.dodr)) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_dr, syntax_tree);
    }

    /*
     * Dead code removal
     */
    if (global.optimize.dodcr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_dcremm3, syntax_tree);
    }

    TRAVsetPreFun (TR_prt, NULL);

    DBUG_RETURN (syntax_tree);
}
