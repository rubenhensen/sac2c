/*
 *
 * $Log$
 * Revision 3.56  2003/10/02 06:28:49  dkrHH
 * unused variable removedunused variable removed
 *
 * Revision 3.55  2003/09/30 19:29:30  dkr
 * code brushed: Set_Shape() used
 *
 * Revision 3.54  2003/09/30 00:04:33  dkr
 * minor modification in Set_Shape()
 *
 * Revision 3.53  2003/09/29 23:45:48  dkr
 * ND_CHECK_REUSE: unique objects are reused as well now
 *
 * Revision 3.52  2003/09/29 22:54:24  dkr
 * code brushing done.
 * several icms renamed/removed/added.
 *
 * Revision 3.51  2003/09/25 13:43:46  dkr
 * new argument 'copyfun' added to some ICMs.
 * ND_WRITE replaced by ND_WRITE_READ_COPY.
 *
 * Revision 3.50  2003/09/20 14:22:58  dkr
 * prf ICMs moved to icm2c_prf.c
 *
 * Revision 3.49  2003/09/19 15:39:10  dkr
 * postfix _nt of varnames renamed into _NT
 *
 * Revision 3.48  2003/09/17 14:17:20  dkr
 * some function parameters renamed
 *
 * Revision 3.47  2003/09/17 13:03:12  dkr
 * postfixes _nt, _any renamed into _NT, _ANY
 *
 * Revision 3.46  2003/06/12 17:23:12  dkr
 * support for multi-dimensional constant arrays added:
 * ICMs CREATE__VECT__... renamed into CREATE__ARRAY__... and modified
 *
 * Revision 3.45  2003/04/15 19:05:50  dkr
 * macro SET_SHAPES_AUD__XXX used. now it is possible to implement AKS,
 * AKD arrays as AUD arrays (sac2c flag -minarrayrep)
 *
 * Revision 3.44  2003/04/15 14:17:44  dkr
 * ICMCompileND_CHECK_REUSE(): \n added
 *
 * Revision 3.43  2003/04/14 15:16:50  dkr
 * IS_REUSED__BLOCK_... icms used
 *
 * Revision 3.42  2003/03/14 13:23:37  dkr
 * ND_PRF_SEL__SHAPE, ND_PRF_IDX_SEL__SHAPE modified
 *
 * Revision 3.41  2002/10/30 14:20:22  dkr
 * some new macros used
 *
 * Revision 3.40  2002/10/29 19:10:56  dkr
 * several bugs removed,
 * new macros for code generation used.
 *
 * Revision 3.39  2002/10/28 09:24:28  dkr
 * some \n for output added
 *
 * [...]
 *
 */

#include <stdio.h>
#include <ctype.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_std.h"

#include "dbug.h"
#include "my_debug.h"
#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"

#ifdef BEtest
#define Free(x)                                                                          \
    x;                                                                                   \
    free (x)
#define Malloc(x) malloc (x)
#endif /* BEtest */

#define ScanArglist(cnt, inc, sep_str, sep_code, code)                                   \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < cnt * inc; i += inc) {                                           \
            if (i > 0) {                                                                 \
                fprintf (outfile, "%s", sep_str);                                        \
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
#ifdef TAGGED_ARRAYS
    if (rettype_NT[0] != '\0') {
        fprintf (outfile, "SAC_ND_TYPE_NT( %s) ", rettype_NT);
    } else {
        fprintf (outfile, "void ");
    }
#else
    fprintf (outfile, "%s ", rettype_NT);
#endif
    if (strcmp (name, "create_TheCommandLine") == 0) {
        fprintf (outfile, "%s( int __argc, char *__argv[])", name);
    } else if (strcmp (name, "SACf_GlobalObjInit") == 0) {
        fprintf (outfile, "%s( int __argc, char *__argv[])", name);
    } else {
        fprintf (outfile, "%s(", name);
#ifdef TAGGED_ARRAYS
        ScanArglist (vararg_cnt, 3, ",", ,
                     fprintf (outfile, " SAC_ND_PARAM_%s( %s, %s)", vararg[i],
                              vararg[i + 2], vararg[i + 1]));
#else
        ScanArglist (vararg_cnt, 3, ",", ,
                     fprintf (outfile, " SAC_ND_PARAM_%s( %s, %s)", vararg[i],
                              vararg[i + 1], vararg[i + 2]));
#endif
        fprintf (outfile, ")");
    }

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
    if (0 != strcmp (retname, "")) {
        fprintf (outfile, "%s = ", retname);
    }
    if (strcmp (name, "create_TheCommandLine") == 0) {
        fprintf (outfile, "%s( __argc, __argv);", name);
    } else {
        fprintf (outfile, "%s(", name);
        ScanArglist (vararg_cnt, 2, ",", ,
                     fprintf (outfile, " SAC_ND_ARG_%s( %s)", vararg[i], vararg[i + 1]));
        fprintf (outfile, ");");
    }
    fprintf (outfile, "\n");

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
                 fprintf (outfile, "SAC_ND_RET_%s( %s, %s)", vararg[i], vararg[i + 1],
                          vararg[i + 2]));
    if (vararg_cnt > 0) {
        fprintf (outfile, "\n");
        INDENT;
    }

    if (strcmp (retname, "")) {
        fprintf (outfile, "return( %s);", retname);
    } else {
        fprintf (outfile, "return;");
    }

    DBUG_VOID_RETURN;
}

#ifdef TAGGED_ARRAYS

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

    if (print_objdef_for_header_file) {
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
    fprintf (outfile, "SAC_ND_DECL__DATA( %s, %s, )\n", var_NT, basetype);

    INDENT;
    fprintf (outfile, "SAC_ND_DECL__DESC( %s, )\n", var_NT);

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
    fprintf (outfile, "SAC_ND_DECL__DATA( %s, %s, extern)\n", var_NT, basetype);

    INDENT;
    fprintf (outfile, "SAC_ND_DECL__DESC( %s, extern)\n", var_NT);

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
        fprintf (outfile, "SAC_NOTHING()\n");
        break;

    case C_aks:
        size = 1;
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "const int SAC_ND_A_MIRROR_SHAPE( %s, %d) = %d;\n", var_NT,
                     i, shp[i]);
            size *= shp[i];
        }
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_MIRROR_SIZE( %s) = %d;\n", var_NT, size);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", var_NT, dim);
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "int SAC_ND_A_MIRROR_SHAPE( %s, %d);\n", var_NT, i);
        }
        INDENT;
        fprintf (outfile, "int SAC_ND_A_MIRROR_SIZE( %s);\n", var_NT);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", var_NT, dim);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile, "int SAC_ND_A_MIRROR_SIZE( %s);\n", var_NT);
        INDENT;
        fprintf (outfile, "int SAC_ND_A_MIRROR_DIM( %s);\n", var_NT);
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
        fprintf (outfile, "SAC_NOTHING()\n");
        break;

    case C_aks:
        size = 1;
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "const int SAC_ND_A_MIRROR_SHAPE( %s, %d) = %d;\n", var_NT,
                     i, shp[i]);
            size *= shp[i];
        }
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_MIRROR_SIZE( %s) = %d;\n", var_NT, size);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", var_NT, dim);
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile,
                     "int SAC_ND_A_MIRROR_SHAPE( %s, %d) "
                     "= SAC_ND_A_DESC_SHAPE( %s, %d);\n",
                     var_NT, i, var_NT, i);
        }
        INDENT;
        fprintf (outfile,
                 "int SAC_ND_A_MIRROR_SIZE( %s)"
                 " = SAC_ND_A_DESC_SIZE( %s);\n",
                 var_NT, var_NT);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", var_NT, dim);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile,
                 "int SAC_ND_A_MIRROR_SIZE( %s)"
                 " = SAC_ND_A_DESC_SIZE( %s);\n",
                 var_NT, var_NT);
        INDENT;
        fprintf (outfile,
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
        fprintf (outfile, "SAC_NOTHING()\n");
        break;

    case C_aks:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile,
                     "extern "
                     "const int SAC_ND_A_MIRROR_SHAPE( %s, %d);\n",
                     var_NT, i);
        }
        INDENT;
        fprintf (outfile,
                 "extern "
                 "const int SAC_ND_A_MIRROR_SIZE( %s);\n",
                 var_NT);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "const int SAC_ND_A_MIRROR_DIM( %s);\n",
                 var_NT);
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile,
                     "extern "
                     "int SAC_ND_A_MIRROR_SHAPE( %s, %d);\n",
                     var_NT, i);
        }
        INDENT;
        fprintf (outfile,
                 "extern "
                 "int SAC_ND_A_MIRROR_SIZE( %s);\n",
                 var_NT);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "const int SAC_ND_A_MIRROR_DIM( %s);\n",
                 var_NT);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile,
                 "extern "
                 "int SAC_ND_A_MIRROR_SIZE( %s);\n",
                 var_NT);
        INDENT;
        fprintf (outfile,
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
        fprintf (outfile, "SAC_NOOP()\n");
    } else {
        INDENT;
        fprintf (outfile, "SAC_IS_LASTREF__BLOCK_BEGIN( %s)\n", from_NT);
        indent++;
        ICMCompileND_ASSIGN (to_NT, to_sdim, from_NT, from_sdim, copyfun);

        INDENT;
        fprintf (outfile,
                 "SAC_TR_MEM_PRINT("
                 " (\"reuse memory of %s at %%p for %s\","
                 " SAC_ND_A_FIELD( %s)))\n",
                 from_NT, to_NT, from_NT);
        indent--;
        INDENT;
        fprintf (outfile, "SAC_IS_LASTREF__BLOCK_END( %s)\n", from_NT);
        INDENT;
        fprintf (outfile, "else\n");
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
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    case C_aks:
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile,
                     "SAC_ND_A_MIRROR_SHAPE( %s, %d) "
                     "= SAC_ND_A_DESC_SHAPE( %s, %d);\n",
                     var_NT, i, var_NT, i);
        }
        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_MIRROR_SIZE( %s)"
                 " = SAC_ND_A_DESC_SIZE( %s);\n",
                 var_NT, var_NT);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_MIRROR_SIZE( %s)"
                 " = SAC_ND_A_DESC_SIZE( %s);\n",
                 var_NT, var_NT);
        INDENT;
        fprintf (outfile,
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
    fprintf (outfile, "SAC_ND_ASSIGN__DATA( %s, %s, %s)\n", to_NT, from_NT, copyfun);

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
        fprintf (outfile, "SAC_NOOP()\n");
    } else if ((!to_has_desc) && from_has_desc) {
        /* 'to_NT' has no desc, 'from_NT' has a desc */
        if (to_hc != C_hid) {
            /*
             * -> 'to_NT' is a non-hidden scalar
             * -> 'from_NT' is a non-hidden array
             *     -> descriptor / data vector of 'from_NT' are not reused by 'to_NT'
             *         -> ND_DEC_RC_FREE( from_NT) in ND_ASSIGN__DATA
             */
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
            fprintf (outfile, "SAC_ND_FREE__DESC( %s)\n", from_NT);
        }
    } else if (to_has_desc && (!from_has_desc)) {
        /* 'to_NT' has a desc, 'from_NT' has no desc */
        INDENT;
        fprintf (outfile, "SAC_ND_ALLOC__DESC( %s, 0)\n", to_NT);
        INDENT;
        fprintf (outfile, "SAC_ND_SET__RC( %s, 1)\n", to_NT);
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
            fprintf (outfile, "SAC_ND_ALLOC__DESC( %s, 0)\n", to_NT);
        } else {
            INDENT;
            fprintf (outfile, "SAC_ND_A_DESC( %s) = SAC_ND_A_DESC( %s);\n", to_NT,
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
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    case C_akd:
        switch (from_sc) {
        case C_aks:
            DBUG_ASSERT ((from_dim >= 0), "illegal dimension found!");
            for (i = 0; i < from_dim; i++) {
                INDENT;
                fprintf (outfile,
                         "SAC_ND_A_DESC_SHAPE( %s, %d)"
                         " = SAC_ND_A_SHAPE( %s, %d);\n",
                         to_NT, i, from_NT, i);
            }
            INDENT;
            fprintf (outfile, "SAC_ND_A_DESC_SIZE( %s) = SAC_ND_A_SIZE( %s);\n", to_NT,
                     from_NT);
            break;
        case C_akd:
            /* here is no break missing */
        case C_aud:
            INDENT;
            fprintf (outfile, "SAC_NOOP()\n");
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
                fprintf (outfile,
                         "SAC_ND_A_DESC_SHAPE( %s, %d)"
                         " = SAC_ND_A_SHAPE( %s, %d);\n",
                         to_NT, i, from_NT, i);
            }
            INDENT;
            fprintf (outfile, "SAC_ND_A_DESC_SIZE( %s) = SAC_ND_A_SIZE( %s);\n", to_NT,
                     from_NT);
            /* here is no break missing */
        case C_akd:
            INDENT;
            fprintf (outfile, "SAC_ND_A_DESC_DIM( %s) = SAC_ND_A_DIM( %s);\n", to_NT,
                     from_NT);
            break;
        case C_aud:
            INDENT;
            fprintf (outfile, "SAC_NOOP()\n");
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
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    case C_akd:
        DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
        for (i = 0; i < to_dim; i++) {
            INDENT;
            fprintf (outfile,
                     "SAC_ND_A_MIRROR_SHAPE( %s, %d) = "
                     "SAC_ND_A_SHAPE( %s, %d);\n",
                     to_NT, i, from_NT, i);
        }
        INDENT;
        fprintf (outfile, "SAC_ND_A_MIRROR_SIZE( %s) = SAC_ND_A_SIZE( %s);\n", to_NT,
                 from_NT);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile, "SAC_ND_A_MIRROR_SIZE( %s) = SAC_ND_A_SIZE( %s);\n", to_NT,
                 from_NT);
        INDENT;
        fprintf (outfile, "SAC_ND_A_MIRROR_DIM( %s) = SAC_ND_A_DIM( %s);\n", to_NT,
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
    fprintf (outfile, "SAC_ND_ALLOC_BEGIN( %s, 1, SAC_ND_A_DIM( %s))\n", to_NT, from_NT);

    /* copy descriptor entries and non-constant part of mirror */
    ICMCompileND_COPY__SHAPE (to_NT, to_sdim, from_NT, from_sdim);

    INDENT;
    fprintf (outfile, "SAC_ND_ALLOC_END( %s, 1, SAC_ND_A_DIM( %s))\n", to_NT, from_NT);

    INDENT;
    fprintf (outfile, "SAC_ND_COPY__DATA( %s, %s, %s)\n", to_NT, from_NT, copyfun);

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
    fprintf (outfile,
             "SAC_TR_MEM_PRINT("
             " (\"ND_MAKE_UNIQUE( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, copyfun);
    INDENT;
    fprintf (outfile, "SAC_TR_REF_PRINT_RC( %s)\n", from_NT);
    INDENT;
    fprintf (outfile, "SAC_IS_LASTREF__BLOCK_BEGIN( %s)\n", from_NT);
    indent++;
    INDENT;
    fprintf (outfile, "SAC_TR_MEM_PRINT( (\"%s is already unique.\"))\n", from_NT);
    ICMCompileND_ASSIGN (to_NT, to_sdim, from_NT, from_sdim, copyfun);
    indent--;
    INDENT;
    fprintf (outfile, "SAC_IS_LASTREF__BLOCK_ELSE( %s)\n", from_NT);
    indent++;
    ICMCompileND_COPY (to_NT, to_sdim, from_NT, from_sdim, copyfun);
    INDENT;
    fprintf (outfile, "SAC_ND_DEC_RC( %s, 1)\n", from_NT);
    indent--;
    INDENT;
    fprintf (outfile, "SAC_IS_LASTREF__BLOCK_END( %s)\n", from_NT);

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
        char **shp_str = (char **)Malloc (dim * sizeof (char *));
        for (i = 0; i < dim; i++) {
            shp_str[i] = (char *)Malloc (20 * sizeof (char));
            sprintf (shp_str[i], "%d", shp[i]);
        }
        ICMCompileND_SET__SHAPE_arr (to_NT, dim, shp_str);
        for (i = 0; i < dim; i++) {
            shp_str[i] = Free (shp_str[i]);
        }
        shp_str = Free (shp_str);
    } else {
        /* 'vals_ANY[i]' is a tagged identifier */

        /*
         * check whether all entries have identical shape
         */
        for (i = 1; i < val_size; i++) {
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s)",
                                      vals_ANY[i], vals_ANY[0]);
                             , fprintf (outfile,
                                        "Inconsistent vector found:"
                                        " First entry and entry at position %d have"
                                        " different dimension!",
                                        i););
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_SIZE( %s) == SAC_ND_A_SIZE( %s)",
                                      vals_ANY[i], vals_ANY[0]);
                             , fprintf (outfile,
                                        "Inconsistent vector found:"
                                        " First entry and entry at position %d have"
                                        " different size!",
                                        i););
            if (val0_dim >= 0) {
                int d;
                for (d = 0; d < val0_dim; d++) {
                    ASSURE_TYPE_ASS (fprintf (outfile,
                                              "SAC_ND_A_SHAPE( %s, %d) == "
                                              "SAC_ND_A_SHAPE( %s, %d)",
                                              vals_ANY[i], d, vals_ANY[0], d);
                                     ,
                                     fprintf (outfile,
                                              "Inconsistent vector found:"
                                              " First entry and entry at position %d have"
                                              " different shape component %d!",
                                              i, d););
                }
            } else {
                FOR_LOOP_INC_VARDEC (fprintf (outfile, "SAC_d");, fprintf (outfile, "0");
                                     ,
                                     fprintf (outfile, "SAC_ND_A_DIM( %s)", vals_ANY[0]);
                                     ,
                                     ASSURE_TYPE_ASS (fprintf (outfile,
                                                               "SAC_ND_A_SHAPE( %s, "
                                                               "SAC_d) == "
                                                               "SAC_ND_A_SHAPE( %s, "
                                                               "SAC_d)",
                                                               vals_ANY[i], vals_ANY[0]);
                                                      , fprintf (outfile,
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
        for (i = 0; i < val_size; i++) {
            INDENT;
            fprintf (outfile, "SAC_ND_WRITE_COPY( %s, %d, ", to_NT, i);
            ReadScalar_Check (vals_ANY[i], NULL, 0);
            fprintf (outfile, ", %s)\n", copyfun);
        }
    } else {
        /* 'vals_ANY[i]' is a tagged identifier */

        if (val_size > 0) {
            BLOCK_VARDECS (fprintf (outfile, "int SAC_j, SAC_i = 0;");
                           ,
                           for (i = 0; i < val_size; i++) {
                               /* check whether all entries have identical size */
                               if (i > 0) {
                                   ASSURE_TYPE_ASS (fprintf (outfile,
                                                             "SAC_ND_A_SIZE( %s) == "
                                                             "SAC_ND_A_SIZE( %s)",
                                                             vals_ANY[i], vals_ANY[0]);
                                                    ,
                                                    fprintf (outfile,
                                                             "Inconsistent vector found:"
                                                             " First entry and entry at "
                                                             "position %d have"
                                                             " different size!",
                                                             i););
                               }

                               /* assign values of entry */
                               FOR_LOOP_INC (fprintf (outfile, "SAC_j");
                                             , fprintf (outfile, "0");
                                             , fprintf (outfile, "SAC_ND_A_SIZE( %s)",
                                                        vals_ANY[i]);
                                             , INDENT;
                                             fprintf (outfile,
                                                      "SAC_ND_WRITE_READ_COPY("
                                                      " %s, SAC_i, %s, SAC_j, %s)\n",
                                                      to_NT, vals_ANY[i], copyfun);
                                             INDENT; fprintf (outfile, "SAC_i++;\n"););
                           }

                           ASSURE_TYPE_ASS (fprintf (outfile,
                                                     "SAC_ND_A_SIZE( %s) == SAC_i",
                                                     to_NT);
                                            , fprintf (outfile, "Assignment with "
                                                                "incompatible types "
                                                                "found!");););
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_VECT2OFFSET( char *off_NT, int from_size, char *from_NT,
 *                                  int shp_size, char **shp_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_VECT2OFFSET( off_NT, from_size, from_NT, shp_size, shp_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_VECT2OFFSET (char *off_NT, int from_size, char *from_NT, int shp_size,
                          char **shp_ANY)
{
    DBUG_ENTER ("ICMCompileND_VECT2OFFSET");

#define ND_VECT2OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_VECT2OFFSET

    /*
     * CAUTION:
     * 'shp_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    DBUG_ASSERT ((from_size >= 0), "Illegal size found!");

    Vect2Offset2 (off_NT, from_NT, from_size, NULL, ReadId, shp_ANY, shp_size, NULL,
                  ReadConstArray_Str);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_IDXS2OFFSET( char *off_NT, int idxs_size, char **idxs_NT,
 *                                  int shp_size, char **shp_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_IDXS2OFFSET( off_NT, idxs_size, idxs_NT, shp_size, shp_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_IDXS2OFFSET (char *off_NT, int idxs_size, char **idxs_NT, int shp_size,
                          char **shp_ANY)
{
    DBUG_ENTER ("ICMCompileND_IDXS2OFFSET");

#define ND_IDXS2OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_IDXS2OFFSET

    /*
     * CAUTION:
     * 'shp_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    DBUG_ASSERT ((idxs_size >= 0), "Illegal size found!");

    Vect2Offset2 (off_NT, idxs_NT, idxs_size, NULL, ReadConstArray_Str, shp_ANY, shp_size,
                  NULL, ReadConstArray_Str);

    DBUG_VOID_RETURN;
}

#else /* TAGGED_ARRAYS */

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KS_DECL_GLOBAL_ARRAY( char *basetype, char *name,
 *                                           int dim, char **s)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KS_DECL_GLOBAL_ARRAY( basetype, name, dim, [ s ]* )
 *
 ******************************************************************************/

void
ICMCompileND_KS_DECL_GLOBAL_ARRAY (char *basetype, char *name, int dim, char **s)
{
    int i;

    DBUG_ENTER ("ICMCompileND_KS_DECL_GLOBAL_ARRAY");

#define ND_KS_DECL_GLOBAL_ARRAY
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KS_DECL_GLOBAL_ARRAY

    if (print_objdef_for_header_file) {
        INDENT;
        fprintf (outfile, "extern %s *%s;\n", basetype, name);
        INDENT;
        fprintf (outfile, "extern int SAC_ND_A_RC( %s);\n", name);
        INDENT;
        fprintf (outfile, "extern int const SAC_ND_A_SIZE( %s);\n", name);
        INDENT;
        fprintf (outfile, "extern int const SAC_ND_A_DIM( %s);\n", name);
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "extern int const SAC_ND_A_SHAPE( %s, %d);\n", name, i);
        }
    } else {
        INDENT;
        fprintf (outfile, "%s *%s;\n", basetype, name);
        INDENT;
        fprintf (outfile, "int SAC_ND_A_RC( %s);\n", name);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_SIZE( %s) = ", name);
        fprintf (outfile, "%s", s[0]);
        for (i = 1; i < dim; i++) {
            fprintf (outfile, "*%s", s[i]);
        }
        fprintf (outfile, ";\n");
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_DIM( %s) = %d;\n", name, dim);
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "const int SAC_ND_A_SHAPE( %s, %d) = %s;\n", name, i, s[i]);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_DECL_EXTERN_ARRAY( char *basetype, char *name,
 *                                           int dim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_DECL_EXTERN_ARRAY( basetype, name, dim)
 *   declares an array which is an imported global object
 *
 ******************************************************************************/

void
ICMCompileND_KD_DECL_EXTERN_ARRAY (char *basetype, char *name, int dim)
{
    int i;

    DBUG_ENTER ("ICMCompileND_KD_DECL_EXTERN_ARRAY");

#define ND_KD_DECL_EXTERN_ARRAY
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_DECL_EXTERN_ARRAY

    INDENT;
    fprintf (outfile, "extern %s *%s;\n", basetype, name);
    INDENT;
    fprintf (outfile, "extern int SAC_ND_A_RC( %s);\n", name);
    INDENT;
    fprintf (outfile, "extern int SAC_ND_A_SIZE( %s);\n", name);
    INDENT;
    fprintf (outfile, "extern int SAC_ND_A_DIM( %s);\n", name);
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "extern int SAC_ND_A_SHAPE( %s, %d);\n", name, i);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KS_DECL_ARRAY( char *basetype, char *name,
 *                                    int dim, char **s)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KS_DECL_ARRAY( basetype, name, dim, [ s ]* )
 *
 ******************************************************************************/

void
ICMCompileND_KS_DECL_ARRAY (char *basetype, char *name, int dim, char **s)
{
    int i;

    DBUG_ENTER ("ICMCompileND_KS_DECL_ARRAY");

#define ND_KS_DECL_ARRAY
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KS_DECL_ARRAY

    INDENT;
    fprintf (outfile, "%s *%s;\n", basetype, name);
    INDENT;
    fprintf (outfile, "int SAC_ND_A_RC( %s);\n", name);
    INDENT;
    fprintf (outfile, "const int SAC_ND_A_SIZE( %s) = ", name);
    fprintf (outfile, "%s", s[0]);
    for (i = 1; i < dim; i++) {
        fprintf (outfile, "*%s", s[i]);
    }
    fprintf (outfile, ";\n");
    INDENT;
    fprintf (outfile, "const int SAC_ND_A_DIM( %s) = %d;\n", name, dim);
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_SHAPE( %s, %d) = %s;\n", name, i, s[i]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KS_DECL_ARRAY_ARG( char *name, int dim, char **s)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KS_DECL_ARRAY_ARG( name, dim, [ s ]* )
 *   declares an array given as arg
 *
 ******************************************************************************/

void
ICMCompileND_KS_DECL_ARRAY_ARG (char *name, int dim, char **s)
{
    int i;

    DBUG_ENTER ("ICMCompileND_KS_DECL_ARRAY_ARG");

#define ND_KS_DECL_ARRAY_ARG
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KS_DECL_ARRAY_ARG

    INDENT;
    fprintf (outfile, "const int SAC_ND_A_SIZE( %s) = ", name);
    fprintf (outfile, "%s", s[0]);
    for (i = 1; i < dim; i++) {
        fprintf (outfile, "*%s", s[i]);
    }
    fprintf (outfile, ";\n");
    INDENT;
    fprintf (outfile, "const int SAC_ND_A_DIM( %s) = %d;\n", name, dim);
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_SHAPE( %s, %d) = %s;\n", name, i, s[i]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE_CONST_ARRAY_S( char *name, int len, char **s)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE_CONST_ARRAY_S( name, len, [ s ]* )
 *
 ******************************************************************************/

void
ICMCompileND_CREATE_CONST_ARRAY_S (char *name, int len, char **s)
{
    int i;

    DBUG_ENTER ("ICMCompileND_CREATE_CONST_ARRAY_S");

#define ND_CREATE_CONST_ARRAY_S
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE_CONST_ARRAY_S

    for (i = 0; i < len; i++) {
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE_ARRAY( %s, %d) = %s;\n", name, i, s[i]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE_CONST_ARRAY_H( char *name, char * copyfun,
 *                                           int len, char **A)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE_CONST_ARRAY_H( name, copyfun, len, [ A ]* )
 *   generates a constant array of refcounted hidden values
 *
 ******************************************************************************/

void
ICMCompileND_CREATE_CONST_ARRAY_H (char *name, char *copyfun, int len, char **A)
{
    int i;

    DBUG_ENTER ("ICMCompileND_CREATE_CONST_ARRAY_H");

#define ND_CREATE_CONST_ARRAY_H
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE_CONST_ARRAY_H

    for (i = 0; i < len; i++) {
        INDENT;
        fprintf (outfile, "SAC_ND_NO_RC_MAKE_UNIQUE_HIDDEN( %s, %s[%d], %s);\n", A[i],
                 name, i, copyfun);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE_CONST_ARRAY_A( char *name, int len2, int len1,
 *                                           char **A)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE_CONST_ARRAY_A( name, len2, len1, [ A ]* )
 *   generates a constant array out of arrays
 *   where len2 is the number of elements of the argument array A
 *
 ******************************************************************************/

void
ICMCompileND_CREATE_CONST_ARRAY_A (char *name, int len2, int len1, char **A)
{
    int i;

    DBUG_ENTER ("ICMCompileND_CREATE_CONST_ARRAY_A");

#define ND_CREATE_CONST_ARRAY_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE_CONST_ARRAY_A

    for (i = 0; i < len1 * len2; i++) {
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE_ARRAY( %s, %d) = SAC_ND_READ_ARRAY( %s, %d);\n",
                 name, i, A[i / len2], i % len2);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KS_VECT2OFFSET( char *off_name, char *arr_name,
 *                                     int dim, int dims, char **shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KS_VECT2OFFSET( off_name, arr_name, dim, dims, shp )
 *
 ******************************************************************************/

void
ICMCompileND_KS_VECT2OFFSET (char *off_name, char *arr_name, int dim, int dims,
                             char **shp)
{
    DBUG_ENTER ("ICMCompileND_KS_VECT2OFFSET");

#define ND_KS_VECT2OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KS_VECT2OFFSET

    INDENT;
    fprintf (outfile, "%s = ", off_name);
    Vect2Offset2 (dim, AccessVect (arr_name, i), dims, AccessConst (shp, i));
    fprintf (outfile, ";\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_IDXS2OFFSET( char *off, int idxs_size, char **idxs,
 *                                  int shp_size, char **shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_IDXS2OFFSET( off, idxs_size, idxs, shp_size, shp )
 *
 ******************************************************************************/

void
ICMCompileND_IDXS2OFFSET (char *off, int idxs_size, char **idxs, int shp_size, char **shp)
{
    DBUG_ENTER ("ICMCompileND_IDXS2OFFSET");

#define ND_IDXS2OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_IDXS2OFFSET

    INDENT;
    fprintf (outfile, "%s = ", off);
    Vect2Offset2 (idxs_size, AccessConst (idxs, i), shp_size, AccessConst (shp, i));
    fprintf (outfile, ";\n");

    DBUG_VOID_RETURN;
}

#endif /* TAGGED_ARRAYS */
