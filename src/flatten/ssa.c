/*
 * $Log$
 * Revision 1.8  2004/11/24 19:15:29  mwe
 * SacDevCamp: compiles!!\
 *
 * Revision 1.7  2004/11/18 14:34:31  mwe
 * changed CheckAvis and chkavis to ToNewTypes and to tonewtypes
 *
 * Revision 1.6  2004/09/18 15:58:34  ktr
 * added RestoreSSAExplicitAllocs
 *
 * Revision 1.5  2004/08/08 16:05:08  sah
 * fixed some includes.
 *
 * Revision 1.4  2004/02/25 15:53:06  cg
 * New functions RestoreSSAOneFunction and RestoreSSAOneFundef
 * now provide access to SSA transformations on a per function
 * basis.
 * Only functions from ssa.[ch] should be used to initiate the
 * transformation process in either direction!
 *
 * Revision 1.3  2004/02/25 08:22:32  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 *
 * Revision 1.2  2004/02/04 12:32:28  skt
 * some comments added
 *
 * Revision 1.1  2004/02/02 15:47:38  skt
 * Initial revision
 *
 */

/**
 * @defgroup ssa Static Single Assignment
 *
 * This group contains all those files/ modules that deal with administrative
 * tasks of the SSA-form
 * @{
 */

/**
 *
 * @file ssa.c
 * @brief Tool package to create resp. to reverse the ssa-form
 *
 * Until now we need to call several functions to create the ssa-form,
 * e.g. Lac2Fun, SSATransform.
 * This module allows to use just one function call
 * for creation and another for reversion
 */

#include "globals.h"
#include "lac2fun.h"
#include "ToNewTypes.h"
#include "SSATransform.h"
#include "fun2lac.h"
#include "node_basic.h"
#include "internal_lib.h"
#include "UndoSSATransform.h"
#include "string.h"
#include "tree_basic.h"
#include "ssa.h"

/** <!--********************************************************************-->
 *
 * @fn node *SSAdoSsa(node *syntax_tree)
 *
 *   @brief  takes the syntax_tree and return it in ssa-form
 *
 *   <pre>
 *   1. Conversion of conditionals and loops into their true
 *      functional represantation
 *   2. Check for correct Avis nodes in vardec/arg nodes. All backrefs
 *      from N_id or IDS structures are checked for consistent values.
 *      This traversal is needed for compatiblity with old code without
 *      knowledge of the avis nodes.
 *   3. Transform code in SSA form.
 *      Every variable has exaclty one definition.
 *      After all the valid_ssaform flag is set to TRUE.
 *   </pre>
 *
 *   @param syntax_tree  nomen est omen
 *   @return the whole syntax_tree, now in ssa-form
 *
 *****************************************************************************/

node *
SSAdoSsa (node *syntax_tree)
{
    DBUG_ENTER ("SSAdoSsa");

    DBUG_PRINT ("SSA", ("call Lac2Fun"));
    syntax_tree = L2FdoLac2Fun (syntax_tree);
    if ((global.break_after == global.compiler_phase)
        && (0 == strcmp (global.break_specifier, "l2f"))) {
        goto DONE;
    }

    DBUG_PRINT ("SSA", ("call ToNewTypes"));
    syntax_tree = TNTdoToNewTypes (syntax_tree);
    if ((global.break_after == global.compiler_phase)
        && (0 == strcmp (global.break_specifier, "cha"))) {
        goto DONE;
    }

    DBUG_PRINT ("SSA", ("call SSATransform"));
    syntax_tree = SSATdoTransform (syntax_tree);

DONE:

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSAundoSsa(node *syntax_tree)
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
 *****************************************************************************/

node *
SSAundoSsa (node *syntax_tree)
{
    DBUG_ENTER ("SSAundoSsa");

    DBUG_PRINT ("SSA", ("call SSAundoSsa"));
    syntax_tree = USSATdoUndoSsaTransform (syntax_tree);
    if ((global.break_after == global.compiler_phase)
        && (0 == strcmp (global.break_specifier, "ussa"))) {
        goto DONE;
    }

    DBUG_PRINT ("SSA", ("call Fun2Lac"));
    /* undo lac2fun transformation */
    syntax_tree = F2LdoFun2Lac (syntax_tree);

DONE:
    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSArestoreSsaOneFunction(node *syntax_tree)
 *
 *   @brief  takes an N_fundef node and restores ssa-form
 *
 *   <pre>
 *   1. Check for correct Avis nodes in vardec/arg nodes. All backrefs
 *      from N_id or IDS structures are checked for consistent values.
 *      This traversal is needed for compatiblity with old code without
 *      knowledge of the avis nodes.
 *   2. Transform code in SSA form.
 *      Every variable has exaclty one definition.
 *      After all the valid_ssaform flag is set to TRUE.
 *
 *   Unlike RestoreSSAOneFundef, this function implicitly traverses applied
 *   loop and conditional special functions.
 *   </pre>
 *
 *   @param fundef  nomen est omen
 *   @return fundef in restored ssa-form
 *
 *****************************************************************************/

node *
SSArestoreSsaOneFunction (node *fundef)
{
    DBUG_ENTER ("SSArestoreSsaOneFunction");

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "SSArestoreSsaOneFunction called without N_fundef node.");

    fundef = TNTdoToNewTypesOneFunction (fundef);

    fundef = SSATdoTransformOneFunction (fundef);

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSArestoreSsaOneFundef(node *syntax_tree)
 *
 *   @brief  takes an N_fundef node and restores ssa-form
 *
 *   <pre>
 *   1. Check for correct Avis nodes in vardec/arg nodes. All backrefs
 *      from N_id or IDS structures are checked for consistent values.
 *      This traversal is needed for compatiblity with old code without
 *      knowledge of the avis nodes.
 *   2. Transform code in SSA form.
 *      Every variable has exaclty one definition.
 *      After all the valid_ssaform flag is set to TRUE.
 *
 *   Unlike RestoreSSAOneFunction, this function does NOT traverse nested
 *   loop and conditional special functions.
 *   </pre>
 *
 *   @param fundef  nomen est omen
 *   @return fundef in restored ssa-form
 *
 *****************************************************************************/

node *
SSArestoreSsaOneFundef (node *fundef)
{
    DBUG_ENTER ("SSArestoreSsaOneFundef");

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "SSArestoreSsaOneFundef called without N_fundef node.");

    fundef = TNTdoToNewTypesOneFundef (fundef);

    fundef = SSATdoTransformOneFundef (fundef);

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSArestoreSsaExplicitAllocs(node *syntax_tree)
 *
 *   @brief  takes an Nmodul node and restores ssa-form
 *
 *   <pre>
 *   1. Check for correct Avis nodes in vardec/arg nodes. All backrefs
 *      from N_id or IDS structures are checked for consistent values.
 *      This traversal is needed for compatiblity with old code without
 *      knowledge of the avis nodes.
 *   2. Transform code in SSA form.
 *      Every variable has exaclty one definition.
 *      After all the valid_ssaform flag is set to TRUE.
 *   3. A difference is that it assumes the withid elements to be initialized
 *      explicitlt BEFORE each WL. Therefore, SSAWithid has to behave
 *      slightly different.
 *
 *   </pre>
 *
 *   @param syntax_tree  N_modul
 *   @return syntax_tree in restored ssa-form
 *
 *****************************************************************************/

node *
SSArestoreSsaExplicitAllocs (node *syntax_tree)
{
    DBUG_ENTER ("SSArestoreSsaExplicitAllocs");

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module,
                 "SSArestoreSsaExplicitAllocs called without N_module node.");

    syntax_tree = TNTdoToNewTypes (syntax_tree);

    syntax_tree = SSATdoTransformExplicitAllocs (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/**
 * @}
 */
