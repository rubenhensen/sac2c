/*
 *
 * $Log$
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
 *   void ICMCompileTYPE_ERROR( int cnt, char **args_any)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   TYPE_ERROR( cnt, args_any_0 ... args_any_n)
 *
 ******************************************************************************/

void
ICMCompileTYPE_ERROR (int cnt, char **args_any)
{
    DBUG_ENTER ("ICMCompileTYPE_ERROR");

#define TYPE_ERROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef TYPE_ERROR

    DBUG_ASSERT ((cnt >= 1), "illegal number of arguments for TYPE_ERROR found!");

    INDENT;
#if 0
  fprintf( outfile, "SAC_RuntimeError_Mult( ");
  fprintf( outfile, "%i", cnt + 1);
  fprintf( outfile, ", ");
  fprintf( outfile, "\"No appropriate instance of function \\\"\" %s"
                    " \"\\\" found!\"", args_any[0]);
  fprintf( outfile, ", ");
  fprintf( outfile, "\"Types of arguments:\"");
  fprintf( outfile, ", ");
  for (i = 1; i < cnt; i++) {
    fprintf( outfile, "\"  %%s\", SAC_PrintShape( SAC_ND_A_DESC( %s))",
                      args_any[i]);
    if (i < cnt - 1) {
      fprintf( outfile, ", ");
    }
  }
  fprintf( outfile, ");\n");
#else
    fprintf (outfile,
             "SAC_RuntimeError( "
             "\"No appropriate instance of function \\\"\" %s"
             " \"\\\" found!\");\n",
             args_any[0]);
#endif

    DBUG_VOID_RETURN;
}
