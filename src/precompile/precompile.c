/*
 * $Id$
 */

#include "precompile.h"

#include "dbug.h"
#include "phase.h"

/******************************************************************************
 *
 * function:
 *   node *PRECdoPrecompile( node *syntax_tree)
 *
 * description:
 *   Prepares syntax tree for code generation.
 *   (For more information see the comments at the beginning of this file)
 *
 *   Optional traversal of AST when generating c-library:
 *     - Look for overloaded functions and build up a list of wrappers
 *   (This has to be done before the following steps, because of the renaming.)
 *
 ******************************************************************************/
node *
PRECdoPrecompile (node *syntax_tree)
{
    DBUG_ENTER ("Precompile");

    /*
     * restore non-ssa form
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ussa, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_fun2lac, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_lacinl, syntax_tree);

    /*
     * Remove External Code
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rec, syntax_tree);

    syntax_tree = PHrunCompilerSubPhase (SUBPH_spmdfunfix, syntax_tree);

    /*
     * Restore Reference Args
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rera, syntax_tree);

    /*
     * Restore Global Objects
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_reso, syntax_tree);

#ifdef BEMT
    /*
     * Create Multithreaded Code
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_spmdinit, syntax_tree);

    /*
     * Create MT-funs for exported and provided functions in modules
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_createmtfuns, syntax_tree);
#endif

    /*
     * Set Linksign
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_sls, syntax_tree);

    /*
     * MarkMemVals
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_mmv, syntax_tree);

    /*
     * Manage Object Initialisers
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_moi, syntax_tree);

    /*
     * Resolve Code Sharing in With-Loops
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rcs, syntax_tree);

    /*
     * WARNING: no phases that duplicate code below this line!
     *          FPC builds the argtabs which CANNOT be maintained
     *          by duptree!
     */

    /*
     * Function precompilation
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_fpc, syntax_tree);

    /*
     * Type conversions
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_tcp, syntax_tree);

#if 0  
  /*
   * Adjusting fold functions ( MT only )
   */
  if ( global.mtmode != MT_none) {
    syntax_tree = PHrunCompilerSubPhase( SUBPH_aff, syntax_tree);
  }
#endif

    /*
     * Mark Noop Grids
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_mng, syntax_tree);

    /*
     * Rename identifiers
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rid, syntax_tree);

    DBUG_RETURN (syntax_tree);
}
