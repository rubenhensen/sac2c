/*
 *
 * $Log$
 * Revision 3.115  2005/07/21 14:18:54  sah
 * introduced remove_external_code
 *
 * Revision 3.114  2005/07/03 17:16:07  ktr
 * changed to phase.h
 *
 * Revision 3.113  2005/04/12 15:51:32  ktr
 * Steps are now enumerated automatically
 *
 * Revision 3.112  2005/03/19 23:18:02  sbs
 * initial call to NT2OTdoTransform eliminated. THIS IS ESSENTIAL for AUD wls.
 * Furthermore NT2OTdoTransform should NEVER EVER be called outside of typecheck!!!
 * This phase does much more than just new 2 old types!
 * One should use ToOldTypes instead!!.
 *
 * Revision 3.111  2005/01/07 17:24:50  cg
 * Removed legacy code meanwhile moved to subphases.
 * Some code brushing done.
 *
 * Revision 3.110  2004/12/19 22:34:42  sbs
 * *** empty log message ***
 *
 * Revision 3.106  2004/11/29 16:50:43  sah
 * added another nt2ot traversal
 *
 * Revision 3.105  2004/11/29 14:41:57  sah
 * added setlinksign traversal
 *
 * Revision 3.104  2004/11/27 02:39:27  ktr
 * errorcorrection.
 *
 * Revision 3.103  2004/11/27 02:12:42  ktr
 * string.h
 *
 * Revision 3.102  2004/11/27 00:16:00  ktr
 * New barebones precompile.
 *
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
     * Remove External Code
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rec, syntax_tree);

    /*
     * Set Linksign
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_sls, syntax_tree);

    /*
     * MarkMemVals
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_mmv, syntax_tree);

#if 0
  /*
   * Object precompilation
   */
  syntax_tree = PHrunCompilerSubPhase( SUBPH_opc, syntax_tree);
#endif

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
     * Rename identifiers
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rid, syntax_tree);

    DBUG_RETURN (syntax_tree);
}
