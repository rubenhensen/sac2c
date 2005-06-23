/*
 *
 * $Log$
 * Revision 1.9  2005/06/23 09:04:01  sah
 * made message printed by DISPATCH_ERROR
 * much more descriptive
 *
 * Revision 1.8  2005/06/16 09:48:48  sbs
 * changed TYPE_ERROR into DISPATCH_ERROR
 *
 * Revision 1.7  2004/11/25 10:26:46  jhb
 * compile SACdevCamp 2k4
 *
 * Revision 1.6  2003/09/17 12:56:52  dkr
 * postfix _any renamed into _ANY
 *
 * Revision 1.5  2002/10/29 20:41:21  dkr
 * TYPE_ERROR ICM: 'exit(-1)' replaced by 'return' ...
 *
 * Revision 1.4  2002/10/29 19:08:38  dkr
 * TYPE_ERROR ICM: 'break' replaced by 'exit(-1)'
 *
 * Revision 1.3  2002/10/10 23:52:23  dkr
 * signature of TYPE_ERROR modified
 *
 * Revision 1.2  2002/09/09 14:24:53  dkr
 * signature of ICMCompileTYPE_ERROR modified
 *
 * Revision 1.1  2002/09/09 14:18:52  dkr
 * Initial revision
 *
 */

#include <stdio.h>
#include <ctype.h>

#include "icm2c_basic.h"
#include "icm2c_error.h"

#include "dbug.h"
#include "globals.h"
#include "print.h"

/******************************************************************************
 *
 * function:
 *   void ICMCompileDISPATCH_ERROR( int cnt_to,   char **to_ANY,
 *                                  int cnt_from, char **from_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   DISPATCH_ERROR( cnt_to,   [ to_sdim,   to_nt   ]* , funname,
 *                   cnt_from, [ from_sdim, from_nt ]* )
 *
 ******************************************************************************/

void
ICMCompileDISPATCH_ERROR (int cnt_to, char **to_ANY, char *funname, int cnt_from,
                          char **from_ANY)
{
    int i;

    DBUG_ENTER ("ICMCompileDISPATCH_ERROR");

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
        fprintf (global.outfile, "\"  %%s\", SAC_PrintShape( SAC_ND_A_DESC( %s))",
                 from_ANY[i]);
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
     */
    INDENT;
    fprintf (global.outfile, "return; /* dummy; is this really a good idea??? */\n");

    DBUG_VOID_RETURN;
}
