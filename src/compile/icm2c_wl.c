/*
 *
 * $Log$
 * Revision 1.8  1998/05/12 22:29:57  dkr
 * added new macros for fold
 *
 * Revision 1.7  1998/05/12 18:49:56  dkr
 * changed some ICMs
 *
 * Revision 1.6  1998/05/07 16:20:43  dkr
 * changed signature of ICMs
 *
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
 *   void ICMCompileWL_NONFOLD_BEGIN( char *target, char *idx_vec,
 *                                    int dims, char **args)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_NONFOLD_BEGIN( target, idx_vec,
 *                     dims, [ idx_scalar, idx_min, idx_max ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_NONFOLD_BEGIN (char *target, char *idx_vec, int dims, char **args)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_NONFOLD_BEGIN");

#define WL_NONFOLD_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_NONFOLD_BEGIN

    INDENT;
    fprintf (outfile, "{\n");

    indent++;
    INDENT;
    fprintf (outfile, "int %s__destptr = 0;\n", target);

    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int __stop_%s = %s;\n", args[3 * i], args[3 * i + 2]);
    }
    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "%s = %s;\n", args[3 * i], args[3 * i + 1]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_FOLD_BEGIN( char *target, char *idx_vec,
 *                                 int dims, char **args)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_FOLD_BEGIN( target, idx_vec, dims, [ idx_scalar, idx_min, idx_max ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD_BEGIN (char *target, char *idx_vec, int dims, char **args)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_FOLD_BEGIN");

#define WL_FOLD_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD_BEGIN

    INDENT;
    fprintf (outfile, "{\n");

    indent++;

    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int __stop_%s = %s;\n", args[3 * i], args[3 * i + 2]);
    }
    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "%s = %s;\n", args[3 * i], args[3 * i + 1]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_END( char *target, char *idx_vec,
 *                          int dims, char **args)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_END( target, idx_vec, dims, [ idx_scalar, idx_min, idx_max ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_END (char *target, char *idx_vec, int dims, char **args)
{
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
 *   void ICMCompileWL_ASSIGN( char *expr, char *target, char *idx_vec,
 *                             int dims, char **idx_scalar)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_ASSIGN( expr, target, idx_vec, dims, [ idx_scalar ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN (char *expr, char *target, char *idx_vec, int dims, char **idx_scalar)
{
    DBUG_ENTER ("ICMCompileWL_ASSIGN");

#define WL_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN

    INDENT;
    fprintf (outfile, "SAC_ND_A_FIELD(%s)[%s__destptr] = %s;\n", target, target, expr);

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
    fprintf (outfile, "%s__destptr++;\n", target);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_FOLD( char *expr, char *target, char *idx_vec,
 *                           int dims, char **idx_scalar)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_FOLD( expr, target, idx_vec, dims, [ idx_scalar ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD (char *expr, char *target, char *idx_vec, int dims, char **idx_scalar)
{
    DBUG_ENTER ("ICMCompileWL_FOLD");

#define WL_FOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD

    INDENT;
    fprintf (outfile, "%s = %s + %s;\n", target, target, expr);

    DBUG_VOID_RETURN;
}
