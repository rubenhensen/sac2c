/*
 *
 * $Log$
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
#include "alloc.h"
#include "ConstVarPropagation.h"
#include "SSADeadCodeRemoval.h"
#include "reuse.h"
#include "aliasanalysis.h"
#include "interfaceanalysis.h"
#include "staticreuse.h"
#include "filterrc.h"
#include "loopreuseopt.h"
#include "datareuse.h"
#include "explicitcopy.h"
#include "reusebranching.h"
#include "print.h"
#include "inplacecomp.h"
#include "audscldist.h"
#include <string.h>
#include <stdio.h>

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
    node *fundef;
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
        DBUG_PRINT ("EMM", ("Performing Constant and Varible Propagation (cvp)"));
        fundef = MODULE_FUNS (syntax_tree);
        while (fundef != NULL) {
            if (!(FUNDEF_ISLACFUN (fundef))) {
                fundef = CVPdoConstVarPropagation (fundef);
            }

            fundef = FUNDEF_NEXT (fundef);
        }
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "cvp"))) {
        goto DONE;
    }

    /*
     * Dead code removal
     *
     * !!! Only needed as long we retransform in SSA form
     */
    if (global.optimize.dodcr) {
        DBUG_PRINT ("EMM", ("Applying Dead Code Removal (dcr)"));
        fundef = MODULE_FUNS (syntax_tree);
        while (fundef != NULL) {
            fundef = DCRdoDeadCodeRemoval (fundef, syntax_tree);

            fundef = FUNDEF_NEXT (fundef);
        }
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "dcr"))) {
        goto DONE;
    }

    /*
     * AUD SCL distinction
     */
    DBUG_PRINT ("EMM", ("Seperating AUD and SCL variables (asd)"));
    syntax_tree = ASDdoAudSclDistinction (syntax_tree);
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "asd"))) {
        goto DONE;
    }

    /*
     * Explicit copy
     */
    DBUG_PRINT ("EMM", ("Making copy operations explicit (copy)"));
    syntax_tree = EMECdoExplicitCopy (syntax_tree);
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "copy"))) {
        goto DONE;
    }

    /*
     * Explicit allocation
     */
    DBUG_PRINT ("EMM", ("Introducing ALLOC statements (alloc)"));
    syntax_tree = EMALdoAlloc (syntax_tree);
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "alloc"))) {
        goto DONE;
    }

    /*
     * Dead code removal
     */
    if (global.optimize.dodcr) {
        DBUG_PRINT ("EMM", ("Applying Dead Code Removal (dcr2)"));
        fundef = MODULE_FUNS (syntax_tree);
        while (fundef != NULL) {
            fundef = DCRdoDeadCodeRemoval (fundef, syntax_tree);

            fundef = FUNDEF_NEXT (fundef);
        }
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "dcr2"))) {
        goto DONE;
    }

    /*
     * Reuse inference
     */
    if (global.optimize.douip) {
        DBUG_PRINT ("EMM", ("Inferencing Reuse Candidates (ri)"));
        syntax_tree = EMRIdoReuseInference (syntax_tree);
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "ri"))) {
        goto DONE;
    }

    TRAVsetPreFun (TR_prt, EMAprintPreFun);

    /*
     * Interface analysis
     */
    if (global.optimize.dosrf) {
        DBUG_PRINT ("EMM", ("Interface analysis (ia)"));
        syntax_tree = EMIAdoInterfaceAnalysis (syntax_tree);
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "ia"))) {
        goto DONE;
    }

    /*
     * Loop reuse optimization
     */
    if ((global.optimize.dolro) && (global.optimize.dosrf)) {
        DBUG_PRINT ("EMM", ("Loop reuse optimization (lro)"));
        syntax_tree = EMLRdoLoopReuseOptimization (syntax_tree);
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "lro"))) {
        goto DONE;
    }

    /*
     * Alias analysis
     */
    if (global.optimize.dosrf) {
        DBUG_PRINT ("EMM", ("Alias analysis (aa)"));
        syntax_tree = EMAAdoAliasAnalysis (syntax_tree);
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "aa"))) {
        goto DONE;
    }

    /*
     * Filter reuse candidates
     */
    DBUG_PRINT ("EMM", ("Filtering reuse candidates (frc)"));
    syntax_tree = EMFRCdoFilterReuseCandidates (syntax_tree);
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "frc"))) {
        goto DONE;
    }

    /*
     * Static reuse
     */
    if (global.optimize.dosrf) {
        DBUG_PRINT ("EMM", ("Static reuse (sr)"));
        syntax_tree = EMSRdoStaticReuse (syntax_tree);
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "sr"))) {
        goto DONE;
    }

    /*
     * Reuse case dependent branching
     */
    if ((global.optimize.doipc) || (global.optimize.dodr)) {
        DBUG_PRINT ("EMM", ("Reuse branching (rb)"));
        syntax_tree = EMRBdoReuseBranching (syntax_tree);
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "rb"))) {
        goto DONE;
    }

    /*
     * In Place computation
     */
    if (global.optimize.doipc) {
        DBUG_PRINT ("EMM", ("In-Place computation (ipc)"));
        syntax_tree = EMIPdoInplaceComputation (syntax_tree);
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "ipc"))) {
        goto DONE;
    }

    /*
     * Data reuse
     */
    if (global.optimize.dodr) {
        DBUG_PRINT ("EMM", ("Data reuse (dr)"));
        syntax_tree = EMDRdoDataReuse (syntax_tree);
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "dr"))) {
        goto DONE;
    }

    /*
     * Dead code removal
     */
    if (global.optimize.dodcr) {
        DBUG_PRINT ("EMM", ("Applying Dead Code Removal (dcr3)"));
        fundef = MODULE_FUNS (syntax_tree);
        while (fundef != NULL) {
            fundef = DCRdoDeadCodeRemoval (fundef, syntax_tree);

            fundef = FUNDEF_NEXT (fundef);
        }
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "dcr3"))) {
        goto DONE;
    }

    TRAVsetPreFun (TR_prt, NULL);

DONE:
    DBUG_RETURN (syntax_tree);
}
