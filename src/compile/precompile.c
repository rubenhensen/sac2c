/*
 *
 * $Log$
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
#include "new2old.h"
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
    DBUG_ENTER ("Precompile");

    /*
     * Fix Oldtypes in ast
     */
    DBUG_PRINT ("PREC", ("step -1: fix oldtypes"));
    syntax_tree = NT2OTdoTransform (syntax_tree);

    /*
     * Set Linksign
     */
    DBUG_PRINT ("PREC", ("step 0: Set Linksign"));
    syntax_tree = SLSdoSetLinksign (syntax_tree);
    if (ILIBstringCompare (global.break_specifier, "sls")) {
        goto DONE;
    }

    /*
     * MarkMemVals
     */
    DBUG_PRINT ("PREC", ("step 1: renaming MemVals"));
    syntax_tree = MMVdoMarkMemVals (syntax_tree);
    if (ILIBstringCompare (global.break_specifier, "mmv")) {
        goto DONE;
    }

    /*
     * Object precompilation
     */
    DBUG_PRINT ("PREC", ("step 2: Object precompilation"));
    /* FIX ME */
    if (ILIBstringCompare (global.break_specifier, "opc")) {
        goto DONE;
    }

    /*
     * Function precompilation
     */
    DBUG_PRINT ("PREC", ("step 3: function precompilation"));
    syntax_tree = FPCdoFunctionPrecompile (syntax_tree);
    if (ILIBstringCompare (global.break_specifier, "fpc")) {
        goto DONE;
    }

    /*
     * Type conversions
     */
    DBUG_PRINT ("PREC", ("step 4: type conversions"));
    syntax_tree = TCPdoTypeConversions (syntax_tree);
    if (ILIBstringCompare (global.break_specifier, "tcp")) {
        goto DONE;
    }

    /*
     * Adjusting fold functions ( MT only )
     */
    DBUG_PRINT ("PREC", ("step 5: Adjusting fold functions"));
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
    DBUG_PRINT ("PREC", ("step 6: renaming identifiers"));
    syntax_tree = RIDdoRenameIdentifiers (syntax_tree);
    if (ILIBstringCompare (global.break_specifier, "RID")) {
        goto DONE;
    }

DONE:
    DBUG_RETURN (syntax_tree);
}
