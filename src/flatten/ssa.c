/*
 * $Log$
 * Revision 1.1  2004/02/02 15:47:38  skt
 * Initial revision
 *
 */

/**
 * @brief Tool package to create resp. to reverse the ssa-form
 */

/**
 *
 * @file ssa.c
 *
 * Till now we need to call several functions to create the ssa-form,
 * e.g. TansformWhile2Do, Lac2Fun, SSATransform.
 * This module enables the possibility to use just one function call
 * for creation and another for reversion
 */

#include "globals.h"
#include "while2do.h"
#include "lac2fun.h"
#include "CheckAvis.h"
#include "SSATransform.h"
#include "fun2lac.h"
#include "UndoSSATransform.h"
#include "ssa.h"

/** <!--********************************************************************-->
 *
 * @fn node *DoSSA(node *syntax_tree)
 *
 *   @brief  takes the syntax_tree and return it in ssa-form
 *
 *   <pre>
 *   1. Conversion of while-loops in do-loops
 *   2. Conversion of conditionals and loops into their true
 *      functional represantation
 *   3. Check for correct Avis nodes in vardec/arg nodes. All backrefs
 *      from N_id or IDS structures are checked for consistent values.
 *      This traversal is needed for compatiblity with old code without
 *      knowledge of the avis nodes.
 *   4. Transform code in SSA form.
 *      Every variable has exaclty one definition.
 *      After all the valid_ssaform flag is set to TRUE.
 *   </pre>
 *
 *   @param syntax_tree  nomen est omen
 *   @return the whole syntax_tree, now in ssa-form
 *
 *****************************************************************************/
extern node *
DoSSA (node *syntax_tree)
{
    DBUG_ENTER ("DoSSA");

    DBUG_PRINT ("SSA", ("call TransformWhile2Do"));
    syntax_tree = TransformWhile2Do (syntax_tree);
    if ((break_after == compiler_phase) && (0 == strcmp (break_specifier, "w2d"))) {
        goto DONE;
    }

    DBUG_PRINT ("SSA", ("call Lac2Fun"));
    syntax_tree = Lac2Fun (syntax_tree);
    if ((break_after == compiler_phase) && (0 == strcmp (break_specifier, "l2f"))) {
        goto DONE;
    }

    DBUG_PRINT ("SSA", ("call CheckAvis"));
    syntax_tree = CheckAvis (syntax_tree);
    if ((break_after == compiler_phase) && (0 == strcmp (break_specifier, "cha"))) {
        goto DONE;
    }

    DBUG_PRINT ("SSA", ("call SSATransform"));
    syntax_tree = SSATransform (syntax_tree);

DONE:
    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *UndoSSA(node *syntax_tree)
 *
 *   @brief  takes the syntax_tree and return it in nonssa-form
 *
 *   <pre>
 *   1.a Renames all artificial identifier to their original
 *       baseid to avoid problems with multiple global object names in the
 *       compiler backend.
 *     b All result-variables of a multigenerator fold-withloop are
 *       made identical.This adjustment is necessary for the inlining
 *       of the fold-function.
 *   2. Reconversion of conditionals and loops from their true functional
 *      representation into an explicit representation suitable for code
 *      generation.
 *   </pre>
 *
 *   @param syntax_tree  nomen est omen
 *   @return the whole syntax_tree, no longer in ssa-form
 *
 ******************************************************************************/
extern node *
UndoSSA (node *syntax_tree)
{
    DBUG_ENTER ("UndoSSA");

    DBUG_PRINT ("SSA", ("call UndoSSATransform"));
    syntax_tree = UndoSSATransform (syntax_tree);
    if ((break_after == compiler_phase) && (0 == strcmp (break_specifier, "ussa"))) {
        goto DONE;
    }

    DBUG_PRINT ("SSA", ("call Fun2Lac"));
    /* undo lac2fun transformation */
    syntax_tree = Fun2Lac (syntax_tree);

DONE:
    DBUG_RETURN (syntax_tree);
}
