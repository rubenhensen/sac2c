/*
 *
 * $Log$
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

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
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

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_modul),
                 "ExplicitAllocation not started with modul node");

    DBUG_PRINT ("EMM", ("Transforming syntax tree into SSA form (l2f, cha, ssa)"));

    /*
     * Transformation into ssa form
     *
     * !!! IF THIS BECOMES UNNECESSARY ONE DAY: !!!
     *     CVP and DCR can be removed as well
     */
    syntax_tree = DoSSA (syntax_tree);
    if ((break_after == PH_alloc)
        && ((0 == strcmp (break_specifier, "l2f"))
            || (0 == strcmp (break_specifier, "cha"))
            || (0 == strcmp (break_specifier, "ssa")))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Performing Constant and Varible Propagation (cvp)"));

    /*
     * Constant and variable propagation
     *
     * !!! Only needed as long we retransform in SSA form
     */
    if (optimize & OPT_CVP) {
        fundef = MODUL_FUNS (syntax_tree);
        while (fundef != NULL) {
            if (!(FUNDEF_IS_LACFUN (fundef))) {
                fundef = ConstVarPropagation (fundef);
            }

            fundef = FUNDEF_NEXT (fundef);
        }
    }
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "cvp"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Applying Dead Code Removal (dcr)"));

    /*
     * Dead code removal
     *
     * !!! Only needed as long we retransform in SSA form
     */
    if (optimize & OPT_DCR) {
        fundef = MODUL_FUNS (syntax_tree);
        while (fundef != NULL) {
            fundef = SSADeadCodeRemoval (fundef, syntax_tree);

            fundef = FUNDEF_NEXT (fundef);
        }
    }
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "dcr"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Making copy operations explicit (copy)"));

    /*
     * Explicit copy
     */
    syntax_tree = EMECExplicitCopy (syntax_tree);
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "copy"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Introducing ALLOC statements (alloc)"));

    /*
     * Explicit allocation
     */
    syntax_tree = EMAllocateFill (syntax_tree);
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "alloc"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Applying Dead Code Removal (dcr2)"));

    /*
     * Dead code removal
     */
    if (optimize & OPT_DCR) {
        fundef = MODUL_FUNS (syntax_tree);
        while (fundef != NULL) {
            fundef = SSADeadCodeRemoval (fundef, syntax_tree);

            fundef = FUNDEF_NEXT (fundef);
        }
    }
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "dcr2"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Inferencing Reuse Candidates (ri)"));

    /*
     * Reuse inference
     */
    if (reuse) {
        syntax_tree = ReuseInference (syntax_tree);
    }
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "ri"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Interface analysis (ia)"));

    /*
     * Interface analysis
     */
    if (optimize & OPT_SRF) {
        syntax_tree = EMIAInterfaceAnalysis (syntax_tree);
    }
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "ia"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Loop reuse optimization (lro)"));

    /*
     * Loop reuse optimization
     */
    if ((optimize & OPT_LRO) && (optimize & OPT_SRF)) {
        syntax_tree = EMLRLoopReuseOptimization (syntax_tree);
    }
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "lro"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Alias analysis (aa)"));

    /*
     * Alias analysis
     */
    if (optimize & OPT_SRF) {
        syntax_tree = EMAAAliasAnalysis (syntax_tree);
    }
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "aa"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Removing FUNDEF_RETALIAS"));

    /*
     * Remove FUNDEF_RETALIAS which are no longer needed
     */
    fundef = MODUL_FUNS (syntax_tree);
    while (fundef != NULL) {
        FUNDEF_RETALIAS (fundef) = FreeNodelist (FUNDEF_RETALIAS (fundef));
        fundef = FUNDEF_NEXT (fundef);
    }

    DBUG_PRINT ("EMM", ("Filtering reuse candidates (frc)"));

    /*
     * Filter reuse candidates
     */
    syntax_tree = EMFRCFilterReuseCandidates (syntax_tree);
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "frc"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Static reuse (sr)"));

    /*
     * Static reuse
     */
    syntax_tree = EMSRStaticReuse (syntax_tree);
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "sr"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("In-Place computation (ipc)"));

    /*
     * In Place computation
     */
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "ipc"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Reuse branching (rb)"));

    /*
     * Reuse case dependent branching
     */
    syntax_tree = EMRBReuseBranching (syntax_tree);
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "rb"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Data reuse (dr)"));

    /*
     * Data reuse
     */
    syntax_tree = EMDRDataReuse (syntax_tree);
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "dr"))) {
        goto DONE;
    }

    DBUG_PRINT ("EMM", ("Applying Dead Code Removal (dcr3)"));

    /*
     * Dead code removal
     */
    if (optimize & OPT_DCR) {
        fundef = MODUL_FUNS (syntax_tree);
        while (fundef != NULL) {
            fundef = SSADeadCodeRemoval (fundef, syntax_tree);

            fundef = FUNDEF_NEXT (fundef);
        }
    }
    if ((break_after == PH_alloc) && (0 == strcmp (break_specifier, "dcr3"))) {
        goto DONE;
    }

DONE:
    DBUG_RETURN (syntax_tree);
}
