/*
 *
 * $Log$
 * Revision 1.5  1998/05/07 08:10:02  cg
 * C implemented ICMs converted to new naming conventions.
 *
 * Revision 1.4  1998/05/06 14:49:08  dkr
 * changed WL_ASSIGN
 *
 * Revision 1.3  1998/05/04 15:35:29  dkr
 * added WL_ASSIGN
 *
 * Revision 1.2  1998/05/04 09:36:19  dkr
 * changed WL_BEGIN
 *
 * Revision 1.1  1998/05/03 14:06:09  dkr
 * Initial revision
 *
 *
 *
 */

#include <malloc.h>
#include <stdio.h>

#include "icm2c_basic.h"

#include "dbug.h"
#include "my_debug.h"
#include "main.h"
#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_BEGIN( char *array, char *offset, char *idx_vec,
 *                            int dims, char **idx_scalar)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_BEGIN( array, offset, idx_vec, dims, [ idx_scalar ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_BEGIN (char *array, char *offset, char *idx_vec, int dims, char **idx_scalar)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_BEGIN");

#define WL_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_BEGIN

    INDENT;
    fprintf (outfile, "{\n");

    indent++;
    INDENT;
    fprintf (outfile, "int %s = 0;\n", offset);

    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int __stop_%s = ", idx_scalar[i]);
        AccessShape (array, i);
        fprintf (outfile, ";\n");
    }
    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "%s = ", idx_scalar[i]);
        fprintf (outfile, "0");
        fprintf (outfile, ";\n");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_END( char *offset, int dims, char *idx_vec,
 *                          int dims, char **idx_scalar)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_END( array, offset, idx_vec, dims, [ idx_scalar ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_END (char *array, char *offset, char *idx_vec, int dims, char **idx_scalar)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_END");

#define WL_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_END

    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_ASSIGN( char *expr,
 *                             char *array, char *offset, char *idx_vec,
 *                             int dims, char **idx_scalar)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_ASSIGN( expr, array, offset, idx_vec, dims, [ idx_scalar ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN (char *expr, char *array, char *offset, char *idx_vec, int dims,
                     char **idx_scalar)
{
    DBUG_ENTER ("ICMCompileWL_ASSIGN");

#define WL_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN

    INDENT;
    fprintf (outfile, "SAC_ND_A_FIELD(%s)[%s] = %s;\n", array, offset, expr);

#if 0
  {
    int i;

    fprintf( outfile, "fprintf( stderr, \"");
    for (i = 0; i < dims; i++) {
      fprintf( outfile, "idx%d=%%d ", i);
    }
    fprintf( outfile, "\\n\"");
    for (i = 0; i < dims; i++) {
      fprintf( outfile, ", %s", idx_scalar[ i]);
    }
    fprintf( outfile, ");\n");
  }
#endif

    INDENT;
    fprintf (outfile, "%s++;\n", offset);

    DBUG_VOID_RETURN;
}
