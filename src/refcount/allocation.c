/*
 *
 * $Log$
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

#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "dbug.h"
#include "ssa.h"
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
#include <string.h>

/** <!--********************************************************************-->
 *
 * @fn ExplicitAllocation
 *
 *   @brief
 *
 *   @param  node *syntax_tree:  the whole syntax tree
 *   @return node *           :  the transformed syntax tree
 *
 *****************************************************************************/
node *
ExplicitAllocation (node *syntax_tree)
{
    node *fundef;
    DBUG_ENTER ("ExplicitAllocation");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "ExplicitAllocation not started with modul node");

    DBUG_PRINT ("EMM", ("Transforming syntax tree into SSA form (l2f, cha, ssa)"));

    /*
     * Transformation into ssa form
     *
     * !!! IF THIS BECOMES UNNECESSARY ONE DAY: !!!
     *     CVP and DCR can be removed as well
     */
    syntax_tree = SSAdoSsa (syntax_tree);
    if ((global.break_after == PH_alloc)
        && ((0 == strcmp (global.break_specifier, "l2f"))
            || (0 == strcmp (global.break_specifier, "cha"))
            || (0 == strcmp (global.break_specifier, "ssa")))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Performing Constant and Varible Propagation (cvp)"));

    /*
     * Constant and variable propagation
     *
     * !!! Only needed as long we retransform in SSA form
     */
    if (global.optimize.docvp) {
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

    DBUG_PRINT ("EMM", ("Applying Dead Code Removal (dcr)"));

    /*
     * Dead code removal
     *
     * !!! Only needed as long we retransform in SSA form
     */
    if (global.optimize.dodcr) {
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

    DBUG_PRINT ("EMM", ("Making copy operations explicit (copy)"));

    /*
     * Explicit copy
     */
    syntax_tree = EMECdoExplicitCopy (syntax_tree);
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "copy"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Introducing ALLOC statements (alloc)"));

    /*
     * Explicit allocation
     */
    syntax_tree = EMALdoAlloc (syntax_tree);
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "alloc"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Applying Dead Code Removal (dcr2)"));

    /*
     * Dead code removal
     */
    if (global.optimize.dodcr) {
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

    DBUG_PRINT ("EMM", ("Inferencing Reuse Candidates (ri)"));

    /*
     * Reuse inference
     */
    if (global.optimize.douip) {
        syntax_tree = EMRIdoReuseInference (syntax_tree);
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "ri"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Interface analysis (ia)"));

    /*
     * Interface analysis
     */
    if (global.optimize.dosrf) {
        syntax_tree = EMIAdoInterfaceAnalysis (syntax_tree);
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "ia"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Loop reuse optimization (lro)"));

    /*
     * Loop reuse optimization
     */
    if ((global.optimize.dolro) && (global.optimize.dosrf)) {
        syntax_tree = EMLRdoLoopReuseOptimization (syntax_tree);
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "lro"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Alias analysis (aa)"));

    /*
     * Alias analysis
     */
    if (global.optimize.dosrf) {
        syntax_tree = EMAAdoAliasAnalysis (syntax_tree);
    }
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "aa"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Filtering reuse candidates (frc)"));

    /*
     * Filter reuse candidates
     */
    syntax_tree = EMFRCdoFilterReuseCandidates (syntax_tree);
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "frc"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Static reuse (sr)"));

    /*
     * Static reuse
     */
    syntax_tree = EMSRdoStaticReuse (syntax_tree);
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "sr"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("In-Place computation (ipc)"));

    /*
     * In Place computation
     */
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "ipc"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Reuse branching (rb)"));

    /*
     * Reuse case dependent branching
     */
    syntax_tree = EMRBdoReuseBranching (syntax_tree);
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "rb"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Data reuse (dr)"));

    /*
     * Data reuse
     */
    syntax_tree = EMDRdoDataReuse (syntax_tree);
    if ((global.break_after == PH_alloc)
        && (0 == strcmp (global.break_specifier, "dr"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Applying Dead Code Removal (dcr3)"));

    /*
     * Dead code removal
     */
    if (global.optimize.dodcr) {
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

DONE:
    DBUG_RETURN (syntax_tree);
}
