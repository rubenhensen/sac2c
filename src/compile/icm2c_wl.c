/*
 *
 * $Log$
 * Revision 1.10  1998/05/16 00:09:24  dkr
 * changed some macros
 * added PRINT_INDEX_CODE, PRINT_INDEX macros, to log the values of the
 * index-vector
 *
 * Revision 1.9  1998/05/14 21:37:55  dkr
 * changed some ICMs
 *
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

#include <limits.h> /* MAX_INT */

#define PRINT_INDEX 0

#define PRINT_INDEX_CODE                                                                 \
    {                                                                                    \
        int i;                                                                           \
        fprintf (outfile, "fprintf( SACstderr, \"");                                     \
        for (i = 0; i < dims; i++) {                                                     \
            fprintf (outfile, "idx%d=%%d ", i);                                          \
        }                                                                                \
        fprintf (outfile, "\\n\"");                                                      \
        for (i = 0; i < dims; i++) {                                                     \
            fprintf (outfile, ", %s", idx_scalars[i]);                                   \
        }                                                                                \
        fprintf (outfile, ");\n");                                                       \
    }

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
 *                      dims, [ idx_scalars, idx_min, idx_max ]* )
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
        fprintf (outfile, "int __start_%s = %s;\n", args[3 * i], args[3 * i + 1]);
    }
    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int __stop_%s = %s;\n", args[3 * i], args[3 * i + 2]);
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
 *   WL_FOLD_BEGIN( target, idx_vec, dims, [ idx_scalars, idx_min, idx_max ]* )
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
        fprintf (outfile, "int __start_%s = %s;\n", args[3 * i], args[3 * i + 1]);
    }
    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int __stop_%s = %s;\n", args[3 * i], args[3 * i + 2]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_FOLDVAR_BEGIN( char *target, char *idx_vec,
 *                                    int dims, char **args)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_FOLDVAR_BEGIN( target, idx_vec, dims, [ idx_scalars ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_FOLDVAR_BEGIN (char *target, char *idx_vec, int dims, char **args)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_FOLDVAR_BEGIN");

#define WL_FOLDVAR_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLDVAR_BEGIN

    INDENT;
    fprintf (outfile, "{\n");
    indent++;

    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int __start_%s = 0;\n", args[i]);
    }
    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int __stop_%s = %d;\n", args[i], INT_MAX);
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
 *   WL_END( target, idx_vec, dims, [ idx_scalars, idx_min, idx_max ]* )
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
 *   void ICMCompileWL_ASSIGN( char *target, char *idx_vec,
 *                             int dims, char **idx_scalars,
 *                             int dim_expr, char *expr)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_ASSIGN( target, idx_vec, dims, [ idx_scalars ]*, dim_expr, expr)
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN (char *target, char *idx_vec, int dims, char **idx_scalars,
                     int dim_expr, char *expr)
{
    DBUG_ENTER ("ICMCompileWL_ASSIGN");

#define WL_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN

#if PRINT_INDEX
    PRINT_INDEX_CODE
#endif

    if (dim_expr > 0) {

        INDENT;
        fprintf (outfile, "{\n");
        indent++;

        INDENT;
        fprintf (outfile, "int __i;\n");

        INDENT;
        fprintf (outfile, "for (__i = 0; __i < SAC_ND_A_SIZE( %s); __i++) {\n", expr);
        indent++;

        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_FIELD( %s)[ %s__destptr++] = "
                 "SAC_ND_A_FIELD( %s)[ __i];\n",
                 target, target, expr);

        indent--;
        INDENT;
        fprintf (outfile, "}\n");

        indent--;
        INDENT;
        fprintf (outfile, "}\n");

    } else {

        INDENT;
        fprintf (outfile, "SAC_ND_A_FIELD( %s)[ %s__destptr++] = %s;\n", target, target,
                 expr);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_ASSIGN_INIT( char *target, char *idx_vec,
 *                                  int dims, char **idx_scalars, char *templ)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_ASSIGN_INIT( target, idx_vec, dims, [ idx_scalars ]*, templ)
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN_INIT (char *target, char *idx_vec, int dims, char **idx_scalars,
                          int dim_templ, char *templ)
{
    DBUG_ENTER ("ICMCompileWL_ASSIGN_INIT");

#define WL_ASSIGN_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN_INIT

#if PRINT_INDEX
    PRINT_INDEX_CODE
#endif

    if (dim_templ > 0) {

        INDENT;
        fprintf (outfile, "{\n");
        indent++;

        INDENT;
        fprintf (outfile, "int __i;\n");

        INDENT;
        fprintf (outfile, "for (__i = 0; __i < SAC_ND_A_SIZE( %s); __i++) {\n", templ);
        indent++;
    }

    INDENT;
    fprintf (outfile, "SAC_ND_A_FIELD( %s)[ %s__destptr++] = 0;\n", target, target);

    if (dim_templ > 0) {

        indent--;
        INDENT;
        fprintf (outfile, "}\n");

        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_ASSIGN_COPY( char *source, char *target, char *idx_vec,
 *                                  int dims, char **idx_scalars, char *templ)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_ASSIGN_COPY( source, target, idx_vec, dims, [ idx_scalars ]*, templ)
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN_COPY (char *source, char *target, char *idx_vec, int dims,
                          char **idx_scalars, int dim_templ, char *templ)
{
    DBUG_ENTER ("ICMCompileWL_ASSIGN_COPY");

#define WL_ASSIGN_COPY
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN_COPY

#if PRINT_INDEX
    PRINT_INDEX_CODE
#endif

    if (dim_templ > 0) {

        INDENT;
        fprintf (outfile, "{\n");
        indent++;

        INDENT;
        fprintf (outfile, "int __i;\n");

        INDENT;
        fprintf (outfile, "for (__i = 0; __i < SAC_ND_A_SIZE( %s); __i++) {\n", templ);
        indent++;
    }

    INDENT;
    fprintf (outfile,
             "SAC_ND_A_FIELD(%s)[ %s__destptr] = "
             "SAC_ND_A_FIELD( %s)[ %s__destptr];\n",
             target, target, source, target);

    INDENT;
    fprintf (outfile, "%s__destptr++;\n", target);

    if (dim_templ > 0) {

        indent--;
        INDENT;
        fprintf (outfile, "}\n");

        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_FOLD( char *target, char *idx_vec,
 *                           int dims, char **idx_scalars,
 *                           int dim_expr, char *expr)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_FOLD( target, idx_vec, dims, [ idx_scalars ]*, dim_expr, expr )
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD (char *target, char *idx_vec, int dims, char **idx_scalars,
                   int dim_expr, char *expr)
{
    DBUG_ENTER ("ICMCompileWL_FOLD");

#define WL_FOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD

#if PRINT_INDEX
    PRINT_INDEX_CODE
#endif

    INDENT;
    fprintf (outfile, "%s = %s + %s;\n", target, target, expr);

    DBUG_VOID_RETURN;
}
