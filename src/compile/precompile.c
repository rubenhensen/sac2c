/*
 *
 * $Log$
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
#include "markmemvals.h"
#include "traverse.h"
#include "internal_lib.h"
#include "globals.h"
#include "functionprecompile.h"
#include "typeconv_precompile.h"
#include "renameidentifiers.h"
#include "ToOldTypes.h"
#include "setlinksign.h"
#include "internal_lib.h"

#include <string.h>

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
    int step = 1;

    DBUG_ENTER ("Precompile");

    /*
     * Set Linksign
     */
    DBUG_PRINT ("PREC", ("step %d: Set Linksign", step++));
    syntax_tree = SLSdoSetLinksign (syntax_tree);
    if (ILIBstringCompare (global.break_specifier, "sls")) {
        goto DONE;
    }

    /*
     * MarkMemVals
     */
    DBUG_PRINT ("PREC", ("step %d: renaming MemVals", step++));
    syntax_tree = MMVdoMarkMemVals (syntax_tree);
    if (ILIBstringCompare (global.break_specifier, "mmv")) {
        goto DONE;
    }

    /*
     * Object precompilation
     */
    DBUG_PRINT ("PREC", ("step %d: Object precompilation", step++));
    /* FIX ME */
    if (ILIBstringCompare (global.break_specifier, "opc")) {
        goto DONE;
    }

    /*
     * Function precompilation
     */
    DBUG_PRINT ("PREC", ("step %d: function precompilation", step++));
    syntax_tree = FPCdoFunctionPrecompile (syntax_tree);
    if (ILIBstringCompare (global.break_specifier, "fpc")) {
        goto DONE;
    }

    /*
     * Type conversions
     */
    DBUG_PRINT ("PREC", ("step %d: type conversions", step++));
    syntax_tree = TCPdoTypeConversions (syntax_tree);
    if (ILIBstringCompare (global.break_specifier, "tcp")) {
        goto DONE;
    }

    /*
     * Adjusting fold functions ( MT only )
     */
    DBUG_PRINT ("PREC", ("step %d: Adjusting fold functions", step++));
    /* FIX ME */
    if (global.mtmode != MT_none) {
        DBUG_ASSERT ((0), "IMPLEMENT ME!");
    }
    if (ILIBstringCompare (global.break_specifier, "aff")) {
        goto DONE;
    }

    /*
     * Type conversions
     */
    DBUG_PRINT ("PREC", ("step %d: renaming identifiers", step++));
    syntax_tree = RIDdoRenameIdentifiers (syntax_tree);
    if (ILIBstringCompare (global.break_specifier, "RID")) {
        goto DONE;
    }

DONE:
    DBUG_RETURN (syntax_tree);
}
