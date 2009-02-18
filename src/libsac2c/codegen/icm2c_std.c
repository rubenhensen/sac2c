/*
 * $Id$
 */

#include <stdio.h>
#include <ctype.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_std.h"

#include "dbug.h"
#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"
#include "free.h"
#include "str.h"
#include "memory.h"

#ifdef BEtest
#define MEMfree(x)                                                                       \
    x;                                                                                   \
    free (x)
#define MEMmalloc(x) malloc (x)
#endif /* BEtest */

#define ScanArglist(cnt, inc, sep_str, sep_code, code)                                   \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < cnt * inc; i += inc) {                                           \
            if (i > 0) {                                                                 \
                fprintf (global.outfile, "%s", sep_str);                                 \
                sep_code;                                                                \
            }                                                                            \
            code;                                                                        \
        }                                                                                \
    }

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_DEC( char *name, char *rettype_NT,
 *                              int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEC( name, rettype_NT, vararg_cnt, [ TAG, basetype, arg_NT ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_DEC (char *name, char *rettype_NT, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileND_FUN_DEC");

#define ND_FUN_DEC
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_DEC

    INDENT;

    fprintf (global.outfile, "SAC_MUTC_DECL_FUN(");

    if (rettype_NT[0] != '\0') {
        fprintf (global.outfile, "SAC_ND_TYPE_NT( %s) ", rettype_NT);
    } else {
        fprintf (global.outfile, "void ");
    }

    fprintf (global.outfile, "%s(", name);
    ScanArglist (vararg_cnt, 3, ",", ,
                 fprintf (global.outfile, " SAC_ND_PARAM_%s( %s, %s)", vararg[i],
                          vararg[i + 2], vararg[i + 1]));
    fprintf (global.outfile, ")");

    fprintf (global.outfile, ")"); /* SAC_MUTC_DECL_FUN */

    DBUG_VOID_RETURN;
}

/**<!--*********************************************************************-->
 *
 * @fn void ICMCompileND_THREAD_FUN_DEC( char *name, char *rettype_NT,
 *                                       int vararg_cnt, char **vararg)
 *
 * @brief
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEC( name, rettype_NT, vararg_cnt, [ TAG, basetype, arg_NT ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileND_THREAD_FUN_DEC (char *name, char *rettype_NT, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileND_THREAD_FUN_DEC");

#define ND_FUN_DEC
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_DEC

    DBUG_ASSERT (rettype_NT[0] == '\0', "Thread funs must have a return type of void");

    INDENT;
    fprintf (global.outfile, "thread ");

    fprintf (global.outfile, "%s(", name);
    ScanArglist (vararg_cnt, 3, ",", ,
                 fprintf (global.outfile, " SAC_ND_PARAM_%s( %s, %s)", vararg[i],
                          vararg[i + 2], vararg[i + 1]));
    fprintf (global.outfile, ")");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_AP( char *name, char *retname,
 *                             int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEC( name, retname, vararg_cnt, [ TAG, arg_NT ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_AP (char *name, char *retname, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileND_FUN_AP");

#define ND_FUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_AP

    INDENT;
    if (!STReq (retname, "")) {
        fprintf (global.outfile, "%s = ", retname);
    } else {
        fprintf (global.outfile, "SAC_MUTC_FUN_AP(");
    }

    fprintf (global.outfile, "%s(", name);
    ScanArglist (vararg_cnt, 2, ",", ,
                 fprintf (global.outfile, " SAC_ND_ARG_%s( %s)", vararg[i],
                          vararg[i + 1]));
    fprintf (global.outfile, ");");

    if (STReq (retname, "")) {
        fprintf (global.outfile, ")"); /* SAC_MUTC_FUN_AP */
    }

    fprintf (global.outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_RET( char *retname, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_RET( retname, vararg_cnt, [ TAG, arg_NT, decl_arg_NT ]* )
 *
 *   where TAG is element in { out, inout }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_RET (char *retname, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileND_FUN_RET");

#define ND_FUN_RET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_RET

    INDENT;
    ScanArglist (vararg_cnt, 3, "\n", INDENT,
                 fprintf (global.outfile, "SAC_ND_RET_%s( %s, %s)", vararg[i],
                          vararg[i + 1], vararg[i + 2]));
    if (vararg_cnt > 0) {
        fprintf (global.outfile, "\n");
        INDENT;
    }

    if (!STReq (retname, "")) {
        fprintf (global.outfile, "return( %s);", retname);
    } else {
        fprintf (global.outfile, "return;");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_OBJDEF( char *var_NT, char *basetype, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_OBJDEF( var_NT, basetype, sdim, [ shp ]* )
 *
 ******************************************************************************/

void
ICMCompileND_OBJDEF (char *var_NT, char *basetype, int sdim, int *shp)
{
    DBUG_ENTER ("ICMCompileND_OBJDEF");

#define ND_OBJDEF
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_OBJDEF

    if (global.print_objdef_for_header_file) {
        ICMCompileND_DECL_EXTERN (var_NT, basetype, sdim);
    } else {
        ICMCompileND_DECL (var_NT, basetype, sdim, shp);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_OBJDEF_EXTERN( char *var_NT, char *basetype, int sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_OBJDEF_EXTERN( var_NT, basetype, sdim)
 *
 ******************************************************************************/

void
ICMCompileND_OBJDEF_EXTERN (char *var_NT, char *basetype, int sdim)
{
    DBUG_ENTER ("ICMCompileND_OBJDEF_EXTERN");

#define ND_OBJDEF_EXTERN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_OBJDEF_EXTERN

    ICMCompileND_DECL_EXTERN (var_NT, basetype, sdim);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL( char *var_NT, char *basetype, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL( var_NT, basetype, sdim, [ shp ]* )
 *
 ******************************************************************************/

void
ICMCompileND_DECL (char *var_NT, char *basetype, int sdim, int *shp)
{
    DBUG_ENTER ("ICMCompileND_DECL");

#define ND_DECL
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL

    INDENT;
    fprintf (global.outfile, "SAC_ND_DECL__DATA( %s, %s, )\n", var_NT, basetype);

    INDENT;
    fprintf (global.outfile, "SAC_ND_DECL__DESC( %s, )\n", var_NT);

    ICMCompileND_DECL__MIRROR (var_NT, sdim, shp);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL_EXTERN( char *var_NT, char *basetype, int sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL_EXTERN( var_NT, basetype, sdim)
 *
 ******************************************************************************/

void
ICMCompileND_DECL_EXTERN (char *var_NT, char *basetype, int sdim)
{
    DBUG_ENTER ("ICMCompileND_DECL_EXTERN");

#define ND_DECL_EXTERN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL_EXTERN

    INDENT;
    fprintf (global.outfile, "SAC_ND_DECL__DATA( %s, %s, extern)\n", var_NT, basetype);

    INDENT;
    fprintf (global.outfile, "SAC_ND_DECL__DESC( %s, extern)\n", var_NT);

    ICMCompileND_DECL__MIRROR_EXTERN (var_NT, sdim);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL__MIRROR( char *var_NT, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL__MIRROR( var_NT, sdim, [ shp ]* )
 *
 ******************************************************************************/

void
ICMCompileND_DECL__MIRROR (char *var_NT, int sdim, int *shp)
{
    int size, i;
    shape_class_t sc = ICUGetShapeClass (var_NT);
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ("ICMCompileND_DECL__MIRROR");

#define ND_DECL__MIRROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL__MIRROR

    switch (sc) {
    case C_scl:
        INDENT;
        fprintf (global.outfile, "SAC_NOTHING()\n");
        break;

    case C_aks:
        size = 1;
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (global.outfile, "const int SAC_ND_A_MIRROR_SHAPE( %s, %d) = %d;\n",
                     var_NT, i, shp[i]);
            size *= shp[i];
            DBUG_ASSERT ((size >= 0), "array with size <0 found!");
        }
        INDENT;
        fprintf (global.outfile, "const int SAC_ND_A_MIRROR_SIZE( %s) = %d;\n", var_NT,
                 size);
        INDENT;
        fprintf (global.outfile, "const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", var_NT,
                 dim);
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (global.outfile, "int SAC_ND_A_MIRROR_SHAPE( %s, %d);\n", var_NT, i);
        }
        INDENT;
        fprintf (global.outfile, "int SAC_ND_A_MIRROR_SIZE( %s);\n", var_NT);
        INDENT;
        fprintf (global.outfile, "const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", var_NT,
                 dim);
        break;

    case C_aud:
        INDENT;
        fprintf (global.outfile, "int SAC_ND_A_MIRROR_SIZE( %s);\n", var_NT);
        INDENT;
        fprintf (global.outfile, "int SAC_ND_A_MIRROR_DIM( %s);\n", var_NT);
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL__MIRROR_PARAM( char *var_NT, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL__MIRROR_PARAM( var_NT, sdim, [ shp ]* )
 *
 ******************************************************************************/

void
ICMCompileND_DECL__MIRROR_PARAM (char *var_NT, int sdim, int *shp)
{
    int size, i;
    shape_class_t sc = ICUGetShapeClass (var_NT);
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ("ICMCompileND_DECL__MIRROR_PARAM");

#define ND_DECL__MIRROR_PARAM
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL__MIRROR_PARAM

    switch (sc) {
    case C_scl:
        INDENT;
        fprintf (global.outfile, "SAC_NOTHING()\n");
        break;

    case C_aks:
        size = 1;
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (global.outfile, "const int SAC_ND_A_MIRROR_SHAPE( %s, %d) = %d;\n",
                     var_NT, i, shp[i]);
            size *= shp[i];
            DBUG_ASSERT ((size >= 0), "array with size <0 found!");
        }
        INDENT;
        fprintf (global.outfile, "const int SAC_ND_A_MIRROR_SIZE( %s) = %d;\n", var_NT,
                 size);
        INDENT;
        fprintf (global.outfile, "const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", var_NT,
                 dim);
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (global.outfile,
                     "int SAC_ND_A_MIRROR_SHAPE( %s, %d) "
                     "= SAC_ND_A_DESC_SHAPE( %s, %d);\n",
                     var_NT, i, var_NT, i);
        }
        INDENT;
        fprintf (global.outfile,
                 "int SAC_ND_A_MIRROR_SIZE( %s)"
                 " = SAC_ND_A_DESC_SIZE( %s);\n",
                 var_NT, var_NT);
        INDENT;
        fprintf (global.outfile, "const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", var_NT,
                 dim);
        break;

    case C_aud:
        INDENT;
        fprintf (global.outfile,
                 "int SAC_ND_A_MIRROR_SIZE( %s)"
                 " = SAC_ND_A_DESC_SIZE( %s);\n",
                 var_NT, var_NT);
        INDENT;
        fprintf (global.outfile,
                 "int SAC_ND_A_MIRROR_DIM( %s)"
                 " = SAC_ND_A_DESC_DIM( %s);\n",
                 var_NT, var_NT);
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL__MIRROR_EXTERN( char *var_NT, int sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL__MIRROR_EXTERN( var_NT, sdim)
 *
 ******************************************************************************/

void
ICMCompileND_DECL__MIRROR_EXTERN (char *var_NT, int sdim)
{
    int i;
    shape_class_t sc = ICUGetShapeClass (var_NT);
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ("ICMCompileND_DECL__MIRROR_EXTERN");

#define ND_DECL__MIRROR_EXTERN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL__MIRROR_EXTERN

    switch (sc) {
    case C_scl:
        INDENT;
        fprintf (global.outfile, "SAC_NOTHING()\n");
        break;

    case C_aks:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (global.outfile,
                     "extern "
                     "const int SAC_ND_A_MIRROR_SHAPE( %s, %d);\n",
                     var_NT, i);
        }
        INDENT;
        fprintf (global.outfile,
                 "extern "
                 "const int SAC_ND_A_MIRROR_SIZE( %s);\n",
                 var_NT);
        INDENT;
        fprintf (global.outfile,
                 "extern "
                 "const int SAC_ND_A_MIRROR_DIM( %s);\n",
                 var_NT);
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (global.outfile,
                     "extern "
                     "int SAC_ND_A_MIRROR_SHAPE( %s, %d);\n",
                     var_NT, i);
        }
        INDENT;
        fprintf (global.outfile,
                 "extern "
                 "int SAC_ND_A_MIRROR_SIZE( %s);\n",
                 var_NT);
        INDENT;
        fprintf (global.outfile,
                 "extern "
                 "const int SAC_ND_A_MIRROR_DIM( %s);\n",
                 var_NT);
        break;

    case C_aud:
        INDENT;
        fprintf (global.outfile,
                 "extern "
                 "int SAC_ND_A_MIRROR_SIZE( %s);\n",
                 var_NT);
        INDENT;
        fprintf (global.outfile,
                 "extern "
                 "int SAC_ND_A_MIRROR_DIM( %s);\n",
                 var_NT);
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CHECK_REUSE( char *to_NT, int to_sdim,
 *                                  char *from_NT, int from_sdim,
 *                                  char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CHECK_REUSE( to_NT, to_sdim, from_NT, from_sdim, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_CHECK_REUSE (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                          char *copyfun)
{
    shape_class_t to_sc = ICUGetShapeClass (to_NT);

    DBUG_ENTER ("ICMCompileND_CHECK_REUSE");

#define ND_CHECK_REUSE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CHECK_REUSE

    if (to_sc == C_scl) {
        INDENT;
        fprintf (global.outfile, "SAC_NOOP()\n");
    } else {
        INDENT;
        fprintf (global.outfile, "SAC_IS_LASTREF__BLOCK_BEGIN( %s)\n", from_NT);
        global.indent++;
        ICMCompileND_ASSIGN (to_NT, to_sdim, from_NT, from_sdim, copyfun);

        INDENT;
        fprintf (global.outfile,
                 "SAC_TR_MEM_PRINT("
                 " (\"reuse memory of %s at %%p for %s\","
                 " SAC_ND_A_FIELD( %s)))\n",
                 from_NT, to_NT, from_NT);
        global.indent--;
        INDENT;
        fprintf (global.outfile, "SAC_IS_LASTREF__BLOCK_END( %s)\n", from_NT);
        INDENT;
        fprintf (global.outfile, "else\n");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_SET__SHAPE_id( char *to_NT, int to_sdim, char *shp_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_SET__SHAPE_id( to_NT, to_sdim, shp_NT )
 *
 ******************************************************************************/

void
ICMCompileND_SET__SHAPE_id (char *to_NT, int to_sdim, char *shp_NT)
{
    DBUG_ENTER ("ICMCompileND_SET__SHAPE_id");

#define ND_SET__SHAPE_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_SET__SHAPE_id

    Set_Shape (to_NT, to_sdim, shp_NT, -1, SizeId, NULL, ReadId, NULL, 0, NULL, NULL,
               NULL);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_SET__SHAPE_arr( char *to_NT, int dim, char **shp_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_SET__SHAPE_arr( to_NT, dim, [ shp_ANY ]* )
 *
 ******************************************************************************/

void
ICMCompileND_SET__SHAPE_arr (char *to_NT, int dim, char **shp_ANY)
{
    DBUG_ENTER ("ICMCompileND_SET__SHAPE_arr");

#define ND_SET__SHAPE_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_SET__SHAPE_arr

    /*
     * CAUTION:
     * 'shp_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    Set_Shape (to_NT, dim, shp_ANY, dim, NULL, NULL, ReadConstArray_Str, NULL, 0, NULL,
               NULL, NULL);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_REFRESH__MIRROR( char *var_NT, int sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_REFRESH__MIRROR( var_NT, sdim)
 *
 ******************************************************************************/

void
ICMCompileND_REFRESH__MIRROR (char *var_NT, int sdim)
{
    int i;
    shape_class_t sc = ICUGetShapeClass (var_NT);
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ("ICMCompileND_REFRESH__MIRROR");

#define ND_REFRESH__MIRROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_REFRESH__MIRROR

    switch (sc) {
    case C_scl:
        INDENT;
        fprintf (global.outfile, "SAC_NOOP()\n");
        break;

    case C_aks:
        INDENT;
        fprintf (global.outfile, "SAC_NOOP()\n");
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (global.outfile,
                     "SAC_ND_A_MIRROR_SHAPE( %s, %d) "
                     "= SAC_ND_A_DESC_SHAPE( %s, %d);\n",
                     var_NT, i, var_NT, i);
        }
        INDENT;
        fprintf (global.outfile,
                 "SAC_ND_A_MIRROR_SIZE( %s)"
                 " = SAC_ND_A_DESC_SIZE( %s);\n",
                 var_NT, var_NT);
        break;

    case C_aud:
        INDENT;
        fprintf (global.outfile,
                 "SAC_ND_A_MIRROR_SIZE( %s)"
                 " = SAC_ND_A_DESC_SIZE( %s);\n",
                 var_NT, var_NT);
        INDENT;
        fprintf (global.outfile,
                 "SAC_ND_A_MIRROR_DIM( %s)"
                 " = SAC_ND_A_DESC_DIM( %s);\n",
                 var_NT, var_NT);
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ASSIGN( char *to_NT, int to_sdim,
 *                             char *from_NT, int from_sdim,
 *                             char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN( to_NT, to_sdim, from_NT, from_sdim, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                     char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_ASSIGN");

#define ND_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN

    ICMCompileND_ASSIGN__SHAPE (to_NT, to_sdim, from_NT, from_sdim);

    INDENT;
    fprintf (global.outfile, "SAC_ND_ASSIGN__DATA( %s, %s, %s)\n", to_NT, from_NT,
             copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ASSIGN__DESC( char *to_NT, char *from_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN__DESC( to_NT, from_NT)
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN__DESC (char *to_NT, char *from_NT)
{
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    hidden_class_t to_hc = ICUGetHiddenClass (to_NT);
    unique_class_t to_uc = ICUGetUniqueClass (to_NT);
    shape_class_t from_sc = ICUGetShapeClass (from_NT);
    hidden_class_t from_hc = ICUGetHiddenClass (from_NT);
    unique_class_t from_uc = ICUGetUniqueClass (from_NT);

    bool to_has_desc, from_has_desc;

    DBUG_ENTER ("ICMCompileND_ASSIGN__DESC");

#define ND_ASSIGN__DESC
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN__DESC

    DBUG_ASSERT ((to_hc == from_hc), "Illegal assignment found!");

    to_has_desc = ((to_sc != C_scl) || ((to_hc == C_hid) && (to_uc != C_unq)));
    from_has_desc = ((from_sc != C_scl) || ((from_hc == C_hid) && (from_uc != C_unq)));

    if ((!to_has_desc) && (!from_has_desc)) {
        /* 'to_NT' has no desc, 'from_NT' has no desc */
        INDENT;
        fprintf (global.outfile, "SAC_NOOP()\n");
    } else if ((!to_has_desc) && from_has_desc) {
        /* 'to_NT' has no desc, 'from_NT' has a desc */
        if (to_hc != C_hid) {
            /*
             * -> 'to_NT' is a non-hidden scalar
             * -> 'from_NT' is a non-hidden array
             *     -> descriptor / data vector of 'from_NT' are not reused by 'to_NT'
             *         -> ND_DEC_RC_FREE( from_NT) in ND_ASSIGN__DATA
             */
            INDENT;
            fprintf (global.outfile, "SAC_NOOP()\n");
        } else {
            /*
             * -> 'to_NT' is a unique hidden scalar
             * -> 'from_NT' is hidden
             * -> RC of 'from_NT' is 1 (otherwise ND_COPY is used)
             *     -> descriptor of 'from_NT' is not reused by 'to_NT'
             *     -> content of data vector of 'from_NT' is reused by 'to_NT'
             *         -> data vector of 'from_NT' (without content) is removed
             *            in ND_ASSIGN_DATA
             *         -> descriptor is removed here
             */
            INDENT;
            fprintf (global.outfile, "SAC_ND_FREE__DESC( %s)\n", from_NT);
        }
    } else if (to_has_desc && (!from_has_desc)) {
        /* 'to_NT' has a desc, 'from_NT' has no desc */
        INDENT;
        fprintf (global.outfile, "SAC_ND_ALLOC__DESC( %s, 0)\n", to_NT);
        INDENT;
        fprintf (global.outfile, "SAC_ND_SET__RC( %s, 1)\n", to_NT);
    } else {
        /* 'to_NT' has a desc, 'from_NT' has a desc */
        if ((((to_sc == C_scl) && (from_sc != C_scl))
             || ((to_sc != C_scl) && (from_sc == C_scl)))
            && (from_uc == C_nuq)) {
            /*
             * -> 'to_NT' and 'from_NT' are neither both scalars nor both arrays
             * -> 'from_NT' is a non-unique hidden
             *     -> descriptor / data vector of 'from_NT' cannot be reused by 'to_NT'
             *         -> ND_ALLOC__DESC( to_NT, 0)
             *         -> ND_DEC_RC_FREE( from_NT) in ND_ASSIGN__DATA
             */
            INDENT;
            fprintf (global.outfile, "SAC_ND_ALLOC__DESC( %s, 0)\n", to_NT);
        } else {
            INDENT;
            fprintf (global.outfile, "SAC_ND_A_DESC( %s) = SAC_ND_A_DESC( %s);\n", to_NT,
                     from_NT);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ASSIGN__SHAPE( char *to_NT, int to_sdim,
 *                                    char *from_NT, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN__SHAPE( to_NT, to_sdim, from_NT, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN__SHAPE (char *to_NT, int to_sdim, char *from_NT, int from_sdim)
{
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("ICMCompileND_ASSIGN__SHAPE");

#define ND_ASSIGN__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN__SHAPE

    /* check constant part of mirror */
    Check_Mirror (to_NT, to_sdim, from_NT, from_dim, DimId, ShapeId, NULL, 0, NULL, NULL);

    ICMCompileND_ASSIGN__DESC (to_NT, from_NT);

    /* assign non-constant part of mirror */
    ICMCompileND_UPDATE__MIRROR (to_NT, to_sdim, from_NT, from_sdim);

    /* assign missing descriptor entries */
    ICMCompileND_UPDATE__DESC (to_NT, to_sdim, from_NT, from_sdim);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_UPDATE__DESC( char *to_NT, int to_sdim,
 *                                   char *from_NT, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_UPDATE__DESC( to_NT, to_sdim, from_NT, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_UPDATE__DESC (char *to_NT, int to_sdim, char *from_NT, int from_sdim)
{
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    shape_class_t from_sc = ICUGetShapeClass (from_NT);
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("ICMCompileND_UPDATE__DESC");

#define ND_UPDATE__DESC
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_UPDATE__DESC

    /*
     * assign missing descriptor entries
     */
    switch (to_sc) {
    case C_scl:
        /* here is no break missing */
    case C_aks:
        INDENT;
        fprintf (global.outfile, "SAC_NOOP()\n");
        break;

    case C_akd:
        switch (from_sc) {
        case C_aks:
            DBUG_ASSERT ((from_dim >= 0), "illegal dimension found!");
            for (i = 0; i < from_dim; i++) {
                INDENT;
                fprintf (global.outfile,
                         "SAC_ND_A_DESC_SHAPE( %s, %d)"
                         " = SAC_ND_A_SHAPE( %s, %d);\n",
                         to_NT, i, from_NT, i);
            }
            INDENT;
            fprintf (global.outfile, "SAC_ND_A_DESC_SIZE( %s) = SAC_ND_A_SIZE( %s);\n",
                     to_NT, from_NT);
            break;
        case C_akd:
            /* here is no break missing */
        case C_aud:
            INDENT;
            fprintf (global.outfile, "SAC_NOOP()\n");
            break;
        default:
            DBUG_ASSERT ((0), ("Illegal assignment found!"));
            break;
        }
        break;

    case C_aud:
        switch (from_sc) {
        case C_scl:
            /* here is no break missing! */
        case C_aks:
            DBUG_ASSERT ((from_dim >= 0), "illegal dimension found!");
            for (i = 0; i < from_dim; i++) {
                INDENT;
                fprintf (global.outfile,
                         "SAC_ND_A_DESC_SHAPE( %s, %d)"
                         " = SAC_ND_A_SHAPE( %s, %d);\n",
                         to_NT, i, from_NT, i);
            }
            INDENT;
            fprintf (global.outfile, "SAC_ND_A_DESC_SIZE( %s) = SAC_ND_A_SIZE( %s);\n",
                     to_NT, from_NT);
            /* here is no break missing */
        case C_akd:
            INDENT;
            fprintf (global.outfile, "SAC_ND_A_DESC_DIM( %s) = SAC_ND_A_DIM( %s);\n",
                     to_NT, from_NT);
            break;
        case C_aud:
            INDENT;
            fprintf (global.outfile, "SAC_NOOP()\n");
            break;
        default:
            DBUG_ASSERT ((0), ("Illegal assignment found!"));
            break;
        }
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_UPDATE__MIRROR( char *to_NT, int to_sdim,
 *                                     char *from_NT, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_UPDATE__MIRROR( to_NT, to_sdim, from_NT, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_UPDATE__MIRROR (char *to_NT, int to_sdim, char *from_NT, int from_sdim)
{
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_UPDATE__MIRROR");

#define ND_UPDATE__MIRROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_UPDATE__MIRROR

    /*
     * initialize non-constant mirror variables
     */
    switch (to_sc) {
    case C_scl:
    case C_aks:
        INDENT;
        fprintf (global.outfile, "SAC_NOOP()\n");
        break;

    case C_akd:
        DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
        for (i = 0; i < to_dim; i++) {
            INDENT;
            fprintf (global.outfile,
                     "SAC_ND_A_MIRROR_SHAPE( %s, %d) = "
                     "SAC_ND_A_SHAPE( %s, %d);\n",
                     to_NT, i, from_NT, i);
        }
        INDENT;
        fprintf (global.outfile, "SAC_ND_A_MIRROR_SIZE( %s) = SAC_ND_A_SIZE( %s);\n",
                 to_NT, from_NT);
        break;

    case C_aud:
        INDENT;
        fprintf (global.outfile, "SAC_ND_A_MIRROR_SIZE( %s) = SAC_ND_A_SIZE( %s);\n",
                 to_NT, from_NT);
        INDENT;
        fprintf (global.outfile, "SAC_ND_A_MIRROR_DIM( %s) = SAC_ND_A_DIM( %s);\n", to_NT,
                 from_NT);
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_COPY( char *to_NT, int to_sdim,
 *                           char *from_NT, int from_sdim,
 *                           char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_COPY( to_NT, to_sdim, from_NT, from_sdim, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_COPY (char *to_NT, int to_sdim, char *from_NT, int from_sdim, char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_COPY");

#define ND_COPY
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_COPY

    /* allocate descriptor */
    INDENT;
    fprintf (global.outfile, "SAC_ND_ALLOC_BEGIN( %s, 1, SAC_ND_A_DIM( %s))\n", to_NT,
             from_NT);

    /* copy descriptor entries and non-constant part of mirror */
    ICMCompileND_COPY__SHAPE (to_NT, to_sdim, from_NT, from_sdim);

    INDENT;
    fprintf (global.outfile, "SAC_ND_ALLOC_END( %s, 1, SAC_ND_A_DIM( %s))\n", to_NT,
             from_NT);

    INDENT;
    fprintf (global.outfile, "SAC_ND_COPY__DATA( %s, %s, %s)\n", to_NT, from_NT, copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_COPY__SHAPE( char *to_NT, int to_sdim,
 *                                  char *from_NT, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_COPY__SHAPE( to_NT, to_sdim, from_NT, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_COPY__SHAPE (char *to_NT, int to_sdim, char *from_NT, int from_sdim)
{
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("ICMCompileND_COPY__SHAPE");

#define ND_COPY__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_COPY__SHAPE

    Set_Shape (to_NT, to_sdim, from_NT, from_dim, DimId, SizeId, ShapeId, NULL, 0, NULL,
               NULL, NULL);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_MAKE_UNIQUE( char *to_NT, int to_sdim,
 *                                  char *from_NT, int from_sdim,
 *                                  char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_MAKE_UNIQUE( to_NT, to_sdim, from_NT, from_sdim, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_MAKE_UNIQUE (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                          char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_MAKE_UNIQUE");

#define ND_MAKE_UNIQUE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_MAKE_UNIQUE

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_MEM_PRINT("
             " (\"ND_MAKE_UNIQUE( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, copyfun);
    INDENT;
    fprintf (global.outfile, "SAC_TR_REF_PRINT_RC( %s)\n", from_NT);
    INDENT;
    fprintf (global.outfile, "SAC_IS_LASTREF__BLOCK_BEGIN( %s)\n", from_NT);
    global.indent++;
    INDENT;
    fprintf (global.outfile, "SAC_TR_MEM_PRINT( (\"%s is already unique.\"))\n", from_NT);
    ICMCompileND_ASSIGN (to_NT, to_sdim, from_NT, from_sdim, copyfun);
    global.indent--;
    INDENT;
    fprintf (global.outfile, "SAC_IS_LASTREF__BLOCK_ELSE( %s)\n", from_NT);
    global.indent++;
    ICMCompileND_COPY (to_NT, to_sdim, from_NT, from_sdim, copyfun);
    INDENT;
    fprintf (global.outfile, "SAC_ND_DEC_RC( %s, 1)\n", from_NT);
    global.indent--;
    INDENT;
    fprintf (global.outfile, "SAC_IS_LASTREF__BLOCK_END( %s)\n", from_NT);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE__ARRAY__SHAPE( char *to_NT, int to_sdim,
 *                                           int dim, int *shp,
 *                                           int val_size, char **vals_ANY,
 *                                           int val0_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE__ARRAY__SHAPE( to_NT, to_sdim, dim, [ shp ]* ,
 *                            val_size, [ vals_ANY ]* , val0_sdim )
 *
 *   dim: top-level dimension
 *   shp: top-level shape
 *           [a,b,c,d]       ->   dim == 1, shp == [4]
 *           [[a,b],[c,d]]   ->   dim == 2, shp == [2,2]
 *   val_size: size of data vector
 *   vals_ANY: data vector
 *   val0_sdim: shape-encoded dimension of the (first) data vector element(s)
 *
 ******************************************************************************/

void
ICMCompileND_CREATE__ARRAY__SHAPE (char *to_NT, int to_sdim, int dim, int *shp,
                                   int val_size, char **vals_ANY, int val0_sdim)
{
    bool entries_are_scalars;
    int i;
#ifndef DBUG_OFF
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
#endif
    int val0_dim = DIM_NO_OFFSET (val0_sdim);

    DBUG_ENTER ("ICMCompileND_CREATE__ARRAY__SHAPE");

#define ND_CREATE__ARRAY__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE__ARRAY__SHAPE

    /*
     * CAUTION:
     * 'vals_ANY[i]' is either a tagged identifier (scalar or non-scalar)
     * or a constant scalar!
     */

    entries_are_scalars = FALSE;
    for (i = 0; i < val_size; i++) {
        if ((vals_ANY[i][0] != '(') || /* not a tagged id -> is a const scalar! */
            (ICUGetShapeClass (vals_ANY[i]) == C_scl)) {
            entries_are_scalars = TRUE;
        }
    }

    if (val_size <= 0) {
        /*
         * 'A = []' works only for arrays with known dimension/shape!
         */
        DBUG_ASSERT ((to_sc == C_aks), "[] with unknown shape found!");
    } else if (entries_are_scalars) {
        char **shp_str = (char **)MEMmalloc (dim * sizeof (char *));
        for (i = 0; i < dim; i++) {
            shp_str[i] = (char *)MEMmalloc (20 * sizeof (char));
            sprintf (shp_str[i], "%d", shp[i]);
        }
        ICMCompileND_SET__SHAPE_arr (to_NT, dim, shp_str);
        for (i = 0; i < dim; i++) {
            shp_str[i] = MEMfree (shp_str[i]);
        }
        shp_str = MEMfree (shp_str);
    } else {
        /* 'vals_ANY[i]' is a tagged identifier */

        /*
         * check whether all entries have identical shape
         */
        for (i = 1; i < val_size; i++) {
            ASSURE_TYPE_ASS (fprintf (global.outfile,
                                      "SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s)",
                                      vals_ANY[i], vals_ANY[0]);
                             , fprintf (global.outfile,
                                        "Inconsistent vector found:"
                                        " First entry and entry at position %d have"
                                        " different dimension!",
                                        i););
            ASSURE_TYPE_ASS (fprintf (global.outfile,
                                      "SAC_ND_A_SIZE( %s) == SAC_ND_A_SIZE( %s)",
                                      vals_ANY[i], vals_ANY[0]);
                             , fprintf (global.outfile,
                                        "Inconsistent vector found:"
                                        " First entry and entry at position %d have"
                                        " different size!",
                                        i););
            if (val0_dim >= 0) {
                int d;
                for (d = 0; d < val0_dim; d++) {
                    ASSURE_TYPE_ASS (fprintf (global.outfile,
                                              "SAC_ND_A_SHAPE( %s, %d) == "
                                              "SAC_ND_A_SHAPE( %s, %d)",
                                              vals_ANY[i], d, vals_ANY[0], d);
                                     ,
                                     fprintf (global.outfile,
                                              "Inconsistent vector found:"
                                              " First entry and entry at position %d have"
                                              " different shape component %d!",
                                              i, d););
                }
            } else {
                FOR_LOOP_INC_VARDEC (fprintf (global.outfile, "SAC_d");
                                     , fprintf (global.outfile, "0");
                                     , fprintf (global.outfile, "SAC_ND_A_DIM( %s)",
                                                vals_ANY[0]);
                                     ,
                                     ASSURE_TYPE_ASS (fprintf (global.outfile,
                                                               "SAC_ND_A_SHAPE( %s, "
                                                               "SAC_d) == "
                                                               "SAC_ND_A_SHAPE( %s, "
                                                               "SAC_d)",
                                                               vals_ANY[i], vals_ANY[0]);
                                                      , fprintf (global.outfile,
                                                                 "Inconsistent vector "
                                                                 "found:"
                                                                 " First entry and entry "
                                                                 "at position %d have"
                                                                 " different shape!",
                                                                 i);););
            }
        }

        Set_Shape (to_NT, to_sdim, shp, dim, NULL, NULL, ReadConstArray_Num, vals_ANY[0],
                   val0_dim, DimId, SizeId, ShapeId);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE__ARRAY__DATA( char *to_NT, int to_sdim,
 *                                          int val_size, char **vals_ANY,
 *                                          char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE__VEC__DATAT( to_NT, to_sdim, val_size, [ vals_ANY ]* , copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_CREATE__ARRAY__DATA (char *to_NT, int to_sdim, int val_size, char **vals_ANY,
                                  char *copyfun)
{
    bool entries_are_scalars;
    int i;

    DBUG_ENTER ("ICMCompileND_CREATE__ARRAY__DATA");

#define ND_CREATE__ARRAY__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE__ARRAY__DATA

    /*
     * CAUTION:
     * 'vals_ANY[i]' is either a tagged identifier (scalar or non-scalar)
     * or a constant scalar!
     */

    entries_are_scalars = FALSE;
    for (i = 0; i < val_size; i++) {
        if ((vals_ANY[i][0] != '(') || /* not a tagged id -> is a const scalar! */
            (ICUGetShapeClass (vals_ANY[i]) == C_scl)) {
            entries_are_scalars = TRUE;
        }
    }

    if (entries_are_scalars) {

        /* ensure all entries are scalar */
        for (i = 0; i < val_size; i++) {
            if ((vals_ANY[i][0] == '(') && (ICUGetShapeClass (vals_ANY[i]) != C_scl)) {
                DBUG_ASSERT ((ICUGetShapeClass (vals_ANY[i]) == C_aud),
                             "tagged id is no scalar!");
                INDENT;
                ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0",
                                          vals_ANY[i]);
                                 , fprintf (global.outfile, "Scalar expected but array "
                                                            "with (dim > 0) found!"););
                fprintf (global.outfile, "\n");
            }
        }

        for (i = 0; i < val_size; i++) {
            INDENT;
            fprintf (global.outfile, "SAC_ND_WRITE_COPY( %s, %d, ", to_NT, i);
            ReadScalar (vals_ANY[i], NULL, 0);
            fprintf (global.outfile, ", %s)\n", copyfun);
        }
    } else {
        /* 'vals_ANY[i]' is a tagged identifier */

        if (val_size > 0) {
            BLOCK_VARDECS (fprintf (global.outfile, "int SAC_j, SAC_i = 0;");
                           ,
                           for (i = 0; i < val_size; i++) {
                               /* check whether all entries have identical size */
                               if (i > 0) {
                                   ASSURE_TYPE_ASS (fprintf (global.outfile,
                                                             "SAC_ND_A_SIZE( %s) == "
                                                             "SAC_ND_A_SIZE( %s)",
                                                             vals_ANY[i], vals_ANY[0]);
                                                    ,
                                                    fprintf (global.outfile,
                                                             "Inconsistent vector found:"
                                                             " First entry and entry at "
                                                             "position %d have"
                                                             " different size!",
                                                             i););
                               }

                               /* assign values of entry */
                               FOR_LOOP_INC (fprintf (global.outfile, "SAC_j");
                                             , fprintf (global.outfile, "0");
                                             ,
                                             fprintf (global.outfile,
                                                      "SAC_ND_A_SIZE( %s)", vals_ANY[i]);
                                             , INDENT;
                                             fprintf (global.outfile,
                                                      "SAC_ND_WRITE_READ_COPY("
                                                      " %s, SAC_i, %s, SAC_j, %s)\n",
                                                      to_NT, vals_ANY[i], copyfun);
                                             INDENT;
                                             fprintf (global.outfile, "SAC_i++;\n"););
                           }

                           ASSURE_TYPE_ASS (fprintf (global.outfile,
                                                     "SAC_ND_A_SIZE( %s) == SAC_i",
                                                     to_NT);
                                            , fprintf (global.outfile,
                                                       "Assignment with incompatible "
                                                       "types found!");););
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_VECT2OFFSET_arr( char *off_NT, int from_size,
 *                                      char *from_NT,
 *                                      int shp_size, char **shp_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_VECT2OFFSET_arr( off_NT, from_size, from_NT, shp_size, shp_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_VECT2OFFSET_arr (char *off_NT, int from_size, char *from_NT, int shp_size,
                              char **shp_ANY)
{
    DBUG_ENTER ("ICMCompileND_VECT2OFFSET_arr");

#define ND_VECT2OFFSET_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_VECT2OFFSET_arr

    /*
     * CAUTION:
     * 'shp_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar.
     */

    DBUG_ASSERT ((shp_size >= 0), "invalid size found!");

    Vect2Offset2 (off_NT, from_NT, from_size, SizeId, ReadId, shp_ANY, shp_size, NULL,
                  ReadConstArray_Str);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_VECT2OFFSET_id( char *off_NT, int from_size,
 *                                     char *from_NT,
 *                                     int shp_size, char *shp_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_VECT2OFFSET_id( off_NT, from_size, from_NT, shp_size, shp_NT)
 *
 ******************************************************************************/

void
ICMCompileND_VECT2OFFSET_id (char *off_NT, int from_size, char *from_NT, int shp_size,
                             char *shp_NT)
{
    DBUG_ENTER ("ICMCompileND_VECT2OFFSET_id");

#define ND_VECT2OFFSET_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_VECT2OFFSET_id

    /*
     * CAUTION:
     * shp_NT is a tagged identifier (representing a vector).
     */

    Vect2Offset2 (off_NT, from_NT, from_size, SizeId, ReadId, shp_NT, shp_size, SizeId,
                  ReadId);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_IDXS2OFFSET_arr( char *off_NT, int idxs_size,
 *                                      char **idxs_ANY,
 *                                      int shp_size, char **shp_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_IDXS2OFFSET_arr( off_NT, idxs_size, idxs_ANY, shp_size, shp_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_IDXS2OFFSET_arr (char *off_NT, int idxs_size, char **idxs_ANY, int shp_size,
                              char **shp_ANY)
{
    DBUG_ENTER ("ICMCompileND_IDXS2OFFSET_arr");

#define ND_IDXS2OFFSET_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_IDXS2OFFSET_arr

    /*
     * CAUTION:
     * 'shp_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar.
     *
     * 'idxs_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    DBUG_ASSERT ((idxs_size >= 0), "illegal index size");
    DBUG_ASSERT ((shp_size >= 0), "illegal shape size");

    Vect2Offset2 (off_NT, idxs_ANY, idxs_size, NULL, ReadConstArray_Str, shp_ANY,
                  shp_size, NULL, ReadConstArray_Str);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_IDXS2OFFSET_id( char *off_NT, int idxs_size,
 *                                     char **idxs_ANY,
 *                                     int shp_size, char *shp_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_IDXS2OFFSET_id( off_NT, idxs_size, idxs_ANY, shp_size, shp_NT)
 *
 ******************************************************************************/

void
ICMCompileND_IDXS2OFFSET_id (char *off_NT, int idxs_size, char **idxs_ANY, int shp_size,
                             char *shp_NT)
{
    DBUG_ENTER ("ICMCompileND_IDXS2OFFSET_id");

#define ND_IDXS2OFFSET_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_IDXS2OFFSET_id

    /*
     * CAUTION:
     * shp_NT is a tagged identifier (representing a vector).
     *
     * 'idxs_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    DBUG_ASSERT ((idxs_size >= 0), "illegal index size");

    Vect2Offset2 (off_NT, idxs_ANY, idxs_size, NULL, ReadConstArray_Str, shp_NT, shp_size,
                  SizeId, ReadId);

    DBUG_VOID_RETURN;
}
