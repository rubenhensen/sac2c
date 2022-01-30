#include <stdio.h>
#include <ctype.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_error.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "globals.h"
#include "print.h"

/******************************************************************************
 *
 * function:
 *   void ICMCompileDISPATCH_ERROR( unsigned int cnt_to,   char **to_ANY,
 *                                  unsigned int cnt_from, char **from_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   DISPATCH_ERROR( cnt_to,   [ to_sdim,   to_nt   ]* , funname,
 *                   cnt_from, [ from_sdim, from_nt ]* )
 *
 ******************************************************************************/

void
ICMCompileDISPATCH_ERROR (unsigned int cnt_to, char **to_ANY, char *funname, unsigned int cnt_from,
                          char **from_ANY)
{
    unsigned int i;

    DBUG_ENTER ();

#define DISPATCH_ERROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef DISPATCH_ERROR

    INDENT;
    fprintf (global.outfile, "SAC_RuntimeError_Mult( ");
    fprintf (global.outfile, "%i", cnt_from + 2);
    fprintf (global.outfile, ", ");
    fprintf (global.outfile,
             "\"No appropriate instance of function \\\"\" %s"
             " \"\\\" found!\"",
             funname);
    fprintf (global.outfile, ", ");
    fprintf (global.outfile, "\"Shape of arguments:\"");
    fprintf (global.outfile, ", ");
    for (i = 0; i < cnt_from; i++) {
        if (ICUGetShapeClass (from_ANY[i]) == C_scl) {
            fprintf (global.outfile, "\"  []\"");
        } else {
            fprintf (global.outfile, "\"  %%s\", SAC_PrintShape( SAC_ND_A_DESC( %s))",
                     from_ANY[i]);
        }

        if (i < cnt_from - 1) {
            fprintf (global.outfile, ", ");
        }
    }
    fprintf (global.outfile, ");\n");

    /*
     * It would be nice to initialize the return values correctly in order
     * to prevent the cc to generate "... might be used uninitialized" warnings.
     * But unfortunately, this is not that easy.....
     *
     * It seems, that a dummy 'break' instruction prevents such warnings, too.
     * But I am not quite sure whether this is bad or good news. What if this
     * instruction simply prevents some optimizations of the c compiler???
     * 
     * Current attempt: we tell the C compiler that SAC_RuntimeError_Mult
     * does not return. Now, we MUST NOT have a return as we get a warning otherwise!
     */

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
