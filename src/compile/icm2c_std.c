/*
 *
 * $Log$
 * Revision 3.19  2002/07/10 19:26:21  dkr
 * F_modarray for TAGGED_ARRAYS added
 *
 * Revision 3.18  2002/07/03 15:53:22  dkr
 * more ICMs for TAGGED_ARRAYS added
 *
 * Revision 3.17  2002/06/07 16:11:01  dkr
 * some new ICMs for TAGGED_ARRAYS added
 *
 * Revision 3.16  2002/06/06 18:33:55  dkr
 * works correctly without TAGGED_ARRAYS now
 *
 * Revision 3.15  2002/06/06 18:14:31  dkr
 * some bugs about TAGGED_ARRAYS fixed
 *
 * Revision 3.14  2002/06/02 21:35:54  dkr
 * ICMs for TAGGED_ARRAYS added
 *
 * Revision 3.13  2002/05/31 17:25:28  dkr
 * some ICMs for TAGGED_ARRAYS added
 *
 * Revision 3.12  2002/05/03 14:00:29  dkr
 * some ICM args renamed
 *
 * Revision 3.11  2002/05/03 12:48:36  dkr
 * ND_KD_SET_SHAPE removed
 *
 * Revision 3.10  2002/04/16 21:17:36  dkr
 * ICMCompileND_FUN_DEC:
 * no special treatment for main() function needed anymore
 *
 * Revision 3.9  2002/03/07 20:22:47  dkr
 * - ND_FUN_DEC, ND_FUN_RET, ND_FUN_AP modified
 * - code brushed
 *
 * Revision 3.8  2001/12/21 13:31:19  dkr
 * ALLOC_ARRAY, CHECK_REUSE ICMs seperated
 * (they no longer occur in other ICMs)
 *
 * Revision 3.7  2001/11/21 09:08:19  dkr
 * comment for ICMCompileND_FUN_RET() modified
 *
 * Revision 3.6  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.5  2001/05/18 09:58:03  cg
 * #include <malloc.h> removed.
 *
 * Revision 3.4  2001/02/09 13:35:28  dkr
 * DEC_RC_FREE in ND_KS_VECT2OFFSET removed:
 * the RC is done by Refcount/Compile now :-)
 *
 * Revision 3.3  2001/02/06 01:37:16  dkr
 * some superfluous '\n' removed
 *
 * Revision 3.2  2001/01/24 15:48:28  dkr
 * bug in ND_FUN_RET fixed
 *
 * Revision 3.1  2000/11/20 18:01:17  sacbase
 * new release made
 *
 * [...]
 *
 */

#include <stdio.h>
#include <ctype.h>

#include "icm2c_basic.h"
#include "icm2c_std.h"

#ifdef TAGGED_ARRAYS
#include "icm2c_utils.h"
#endif /* TAGGED_ARRAYS */

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

#define NewBlock(init, body)                                                             \
    fprintf (outfile, "{\n");                                                            \
    indent++;                                                                            \
    init;                                                                                \
    body;                                                                                \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n")

#define InitPtr(src, dest)                                                               \
    INDENT;                                                                              \
    fprintf (outfile, "int SAC_isrc = ");                                                \
    src;                                                                                 \
    fprintf (outfile, ";\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "int SAC_idest = ");                                               \
    dest;                                                                                \
    fprintf (outfile, ";\n")

#define InitVecs(from, to, vn, v_i_str)                                                  \
    {                                                                                    \
        int i;                                                                           \
        for (i = from; i < to; i++) {                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "int %s%d=", vn, i);                                       \
            v_i_str;                                                                     \
            fprintf (outfile, ";\n");                                                    \
        }                                                                                \
    }

#define InitIMaxs(from, to, v_i_str) InitVecs (from, to, "SAC_imax", v_i_str)

#define InitSrcOffs(from, to, v_i_str) InitVecs (from, to, "SAC_srcoff", v_i_str)

#define FillRes(res, body)                                                               \
    INDENT;                                                                              \
    fprintf (outfile, "do {\n");                                                         \
    indent++;                                                                            \
    body;                                                                                \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "while( SAC_idest < SAC_ND_A_SIZE( %s));\n", res)

#define AccessSeg(dim, body)                                                             \
    {                                                                                    \
        int i;                                                                           \
        INDENT;                                                                          \
        fprintf (outfile, "{\n");                                                        \
        indent++;                                                                        \
        for (i = 1; i < dim; i++) {                                                      \
            INDENT;                                                                      \
            fprintf (outfile, "int SAC_i%d;\n", i);                                      \
        }                                                                                \
        for (i = 1; i < dim; i++) {                                                      \
            INDENT;                                                                      \
            fprintf (outfile, "for (SAC_i%d = 0; SAC_i%d < SAC_imax%d; SAC_i%d++) {\n",  \
                     i, i, i, i);                                                        \
            indent++;                                                                    \
        }                                                                                \
        body;                                                                            \
        for (i = dim - 1; i > 0; i--) {                                                  \
            indent--;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "}\n");                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "SAC_isrc += SAC_srcoff%d;\n", i);                         \
        }                                                                                \
        indent--;                                                                        \
        INDENT;                                                                          \
        fprintf (outfile, "}\n");                                                        \
    }

#ifdef TAGGED_ARRAYS
#define CopyBlock(a, offset, res)                                                        \
    NewBlock (InitPtr (offset, fprintf (outfile, "0")),                                  \
              FillRes (res, INDENT; fprintf (outfile,                                    \
                                             "SAC_ND_WRITE( %s, SAC_idest)"              \
                                             " = SAC_ND_READ( %s, SAC_isrc);\n",         \
                                             res, a);                                    \
                       INDENT; fprintf (outfile, "SAC_idest++; SAC_isrc++;\n");))
#else /* TAGGED_ARRAYS */
#define CopyBlock(a, offset, res)                                                        \
    NewBlock (InitPtr (offset, fprintf (outfile, "0")),                                  \
              FillRes (res, INDENT; fprintf (outfile,                                    \
                                             "SAC_ND_WRITE_ARRAY( %s, SAC_idest)"        \
                                             " = SAC_ND_READ_ARRAY( %s, SAC_isrc);\n",   \
                                             res, a);                                    \
                       INDENT; fprintf (outfile, "SAC_idest++; SAC_isrc++;\n");))
#endif /* TAGGED_ARRAYS */

/*
 * TakeSeg( a, dima, offset, dimi, sz_i_str, off_i_str, res)
 *   a        : src-array
 *   dima     : dimension of "a"
 *   offset   : beginning of the segment in the unrolling of "a"
 *   dimi     : length of "sz_i_str" and "off_i_str"
 *   sz_i_str : number of elements to take from the i'th axis
 *   off_i_str: number of elements to skip in the i'th axis
 *   res      : resulting array
 *
 */

#ifdef TAGGED_ARRAYS
#define TakeSeg(a, dima, offset, dimi, sz_i_str, off_i_str, res)                         \
    NewBlock (InitPtr (offset, fprintf (outfile, "0")); InitIMaxs (0, dimi, sz_i_str);   \
              InitIMaxs (dimi, dima,                                                     \
                         fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)", a, i));            \
              InitSrcOffs (                                                              \
                0, dimi, off_i_str; {                                                    \
                    int j;                                                               \
                    for (j = i + 1; j < dima; j++)                                       \
                        fprintf (outfile, "* SAC_ND_A_SHAPE( %s, %d)", a, j);            \
                }) InitSrcOffs (dimi, dima, fprintf (outfile, "0")),                     \
              FillRes (res,                                                              \
                       AccessSeg (dima, INDENT;                                          \
                                  fprintf (outfile,                                      \
                                           "SAC_ND_WRITE( %s, SAC_idest) "               \
                                           "= SAC_ND_READ( %s, SAC_isrc);\n",            \
                                           res, a);                                      \
                                  INDENT;                                                \
                                  fprintf (outfile, "SAC_idest++; SAC_isrc++;\n");)))
#else /* TAGGED_ARRAYS */
#define TakeSeg(a, dima, offset, dimi, sz_i_str, off_i_str, res)                         \
    NewBlock (InitPtr (offset, fprintf (outfile, "0")); InitIMaxs (0, dimi, sz_i_str);   \
              InitIMaxs (dimi, dima,                                                     \
                         fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)", a, i));            \
              InitSrcOffs (                                                              \
                0, dimi, off_i_str; {                                                    \
                    int j;                                                               \
                    for (j = i + 1; j < dima; j++)                                       \
                        fprintf (outfile, "* SAC_ND_A_SHAPE( %s, %d)", a, j);            \
                }) InitSrcOffs (dimi, dima, fprintf (outfile, "0")),                     \
              FillRes (res,                                                              \
                       AccessSeg (dima, INDENT;                                          \
                                  fprintf (outfile,                                      \
                                           "SAC_ND_WRITE_ARRAY( %s, SAC_idest) "         \
                                           "= SAC_ND_READ_ARRAY( %s, SAC_isrc);\n",      \
                                           res, a);                                      \
                                  INDENT;                                                \
                                  fprintf (outfile, "SAC_idest++; SAC_isrc++;\n");)))
#endif /* TAGGED_ARRAYS */

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_DEC( char *name, char *rettype,
 *                              int narg, char **arg_any)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEC( name, rettype, narg, [ TAG, basetype, arg_nt ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_DEC (char *name, char *rettype, int narg, char **arg_any)
{
    DBUG_ENTER ("ICMCompileND_FUN_DEC");

#define ND_FUN_DEC
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_DEC

    INDENT;
#ifdef TAGGED_ARRAYS
    if (rettype[0] != '\0') {
        fprintf (outfile, "SAC_ND_TYPE_NT( %s) ", rettype);
    } else {
        fprintf (outfile, "void ");
    }
#else
    fprintf (outfile, "%s ", rettype);
#endif
    if (strcmp (name, "create_TheCommandLine") == 0) {
        fprintf (outfile, "%s( int __argc, char *__argv[])", name);
    } else if (strncmp (name, "SACf_GlobalObjInit_", strlen ("SACf_GlobalObjInit_"))
               == 0) {
        fprintf (outfile, "%s( int __argc, char *__argv[])", name);
    } else {
        fprintf (outfile, "%s(", name);
#ifdef TAGGED_ARRAYS
        ScanArglist (narg, 3, ",", ,
                     fprintf (outfile, " SAC_ND_PARAM_%s( %s, %s)", arg_any[i],
                              arg_any[i + 2], arg_any[i + 1]));
#else
        ScanArglist (narg, 3, ",", ,
                     fprintf (outfile, " SAC_ND_PARAM_%s( %s, %s)", arg_any[i],
                              arg_any[i + 1], arg_any[i + 2]));
#endif
        fprintf (outfile, ")");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_AP( char *name, char *retname,
 *                             int narg, char **arg_any)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEC( name, retname, narg, [ TAG, arg_nt ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_AP (char *name, char *retname, int narg, char **arg_any)
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
        ScanArglist (narg, 2, ",", ,
                     fprintf (outfile, " SAC_ND_ARG_%s( %s)", arg_any[i],
                              arg_any[i + 1]));
        fprintf (outfile, ");");
    }
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_RET( char *retname, int narg, char **arg_any)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_RET( retname, narg, [ TAG, arg_nt, decl_arg_nt ]* )
 *
 *   where TAG is element in { out, inout }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_RET (char *retname, int narg, char **arg_any)
{
    DBUG_ENTER ("ICMCompileND_FUN_RET");

#define ND_FUN_RET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_RET

    INDENT;
    ScanArglist (narg, 3, "\n", INDENT,
                 fprintf (outfile, "SAC_ND_RET_%s( %s, %s)", arg_any[i], arg_any[i + 1],
                          arg_any[i + 2]));
    if (narg > 0) {
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
 *   void ICMCompileND_OBJDEF( char *nt, char *basetype, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_OBJDEF( nt, basetype, sdim, shp_0 ... shp_n)
 *
 ******************************************************************************/

void
ICMCompileND_OBJDEF (char *nt, char *basetype, int sdim, int *shp)
{
    DBUG_ENTER ("ICMCompileND_OBJDEF");

#define ND_OBJDEF
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_OBJDEF

    if (print_objdef_for_header_file) {
        ICMCompileND_DECL_EXTERN (nt, basetype, sdim);
    } else {
        ICMCompileND_DECL (nt, basetype, sdim, shp);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_OBJDEF_EXTERN( char *nt, char *basetype, int sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_OBJDEF_EXTERN( nt, basetype, sdim)
 *
 ******************************************************************************/

void
ICMCompileND_OBJDEF_EXTERN (char *nt, char *basetype, int sdim)
{
    DBUG_ENTER ("ICMCompileND_OBJDEF_EXTERN");

#define ND_OBJDEF_EXTERN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_OBJDEF_EXTERN

    ICMCompileND_DECL_EXTERN (nt, basetype, sdim);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL( char *nt, char *basetype, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL( nt, basetype, sdim, shp_0 ... shp_n)
 *
 ******************************************************************************/

void
ICMCompileND_DECL (char *nt, char *basetype, int sdim, int *shp)
{
    DBUG_ENTER ("ICMCompileND_DECL");

#define ND_DECL
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL

    INDENT;
    fprintf (outfile, "SAC_ND_DECL__DATA( %s, %s, )\n", nt, basetype);

    INDENT;
    fprintf (outfile, "SAC_ND_DECL__DESC( %s, )\n", nt);

    ICMCompileND_DECL__MIRROR (nt, sdim, shp);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL_EXTERN( char *nt, char *basetype, int sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL_EXTERN( nt, basetype, sdim)
 *
 ******************************************************************************/

void
ICMCompileND_DECL_EXTERN (char *nt, char *basetype, int sdim)
{
    DBUG_ENTER ("ICMCompileND_DECL_EXTERN");

#define ND_DECL_EXTERN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL_EXTERN

    INDENT;
    fprintf (outfile, "SAC_ND_DECL__DATA( %s, %s, extern)\n", nt, basetype);

    INDENT;
    fprintf (outfile, "SAC_ND_DECL__DESC( %s, extern)\n", nt);

    ICMCompileND_DECL__MIRROR_EXTERN (nt, sdim);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL__MIRROR( char *nt, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL__MIRROR( nt, sdim, shp_0 ... shp_n)
 *
 ******************************************************************************/

void
ICMCompileND_DECL__MIRROR (char *nt, int sdim, int *shp)
{
    int size, i;
    data_class_t dc = ICUGetDataClass (nt);
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ("ICMCompileND_DECL__MIRROR");

#define ND_DECL__MIRROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL__MIRROR

    switch (dc) {
    case C_scl:
        INDENT;
        fprintf (outfile, "SAC_NOTHING()\n");
        break;

    case C_aks:
        size = 1;
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "const int SAC_ND_A_SHAPE( %s, %d) = %d;\n", nt, i, shp[i]);
            size *= shp[i];
        }
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_SIZE( %s) = %d;\n", nt, size);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_DIM( %s) = %d;\n", nt, dim);
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "int SAC_ND_A_SHAPE( %s, %d);\n", nt, i);
        }
        INDENT;
        fprintf (outfile, "int SAC_ND_A_SIZE( %s);\n", nt);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_DIM( %s) = %d;\n", nt, dim);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile, "int SAC_ND_A_SIZE( %s);\n", nt);
        INDENT;
        fprintf (outfile, "int SAC_ND_A_DIM( %s);\n", nt);
        break;

    case C_hid:
        INDENT;
        fprintf (outfile, "SAC_NOTHING()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown data class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL__MIRROR_PARAM( char *nt, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL__MIRROR_PARAM( nt, sdim, shp_0 ... shp_n)
 *
 ******************************************************************************/

void
ICMCompileND_DECL__MIRROR_PARAM (char *nt, int sdim, int *shp)
{
    int size, i;
    data_class_t dc = ICUGetDataClass (nt);
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ("ICMCompileND_DECL__MIRROR_PARAM");

#define ND_DECL__MIRROR_PARAM
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL__MIRROR_PARAM

    switch (dc) {
    case C_scl:
        INDENT;
        fprintf (outfile, "SAC_NOTHING()\n");
        break;

    case C_aks:
        size = 1;
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "const int SAC_ND_A_SHAPE( %s, %d) = %d;\n", nt, i, shp[i]);
            size *= shp[i];
        }
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_SIZE( %s) = %d;\n", nt, size);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_DIM( %s) = %d;\n", nt, dim);
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile,
                     "int SAC_ND_A_SHAPE( %s, %d) "
                     "= SAC_ND_A_DESC_SHAPE( %s, %d);\n",
                     nt, i, nt, i);
        }
        INDENT;
        fprintf (outfile, "int SAC_ND_A_SIZE( %s) = SAC_ND_A_DESC_SIZE( %s);\n", nt, nt);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_DIM( %s) = %d;\n", nt, dim);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile, "int SAC_ND_A_SIZE( %s) = SAC_ND_A_DESC_SIZE( %s);\n", nt, nt);
        INDENT;
        fprintf (outfile, "int SAC_ND_A_DIM( %s) = SAC_ND_A_DESC_DIM( %s);\n", nt, nt);
        break;

    case C_hid:
        INDENT;
        fprintf (outfile, "SAC_NOTHING()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown data class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL__MIRROR_EXTERN( char *nt, int sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL__MIRROR_EXTERN( nt, sdim)
 *
 ******************************************************************************/

void
ICMCompileND_DECL__MIRROR_EXTERN (char *nt, int sdim)
{
    int i;
    data_class_t dc = ICUGetDataClass (nt);
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ("ICMCompileND_DECL__MIRROR_EXTERN");

#define ND_DECL__MIRROR_EXTERN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL__MIRROR_EXTERN

    switch (dc) {
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
                     "const int SAC_ND_A_SHAPE( %s, %d);\n",
                     nt, i);
        }
        INDENT;
        fprintf (outfile,
                 "extern "
                 "const int SAC_ND_A_SIZE( %s);\n",
                 nt);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "const int SAC_ND_A_DIM( %s);\n",
                 nt);
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile,
                     "extern "
                     "int SAC_ND_A_SHAPE( %s, %d);\n",
                     nt, i);
        }
        INDENT;
        fprintf (outfile,
                 "extern "
                 "int SAC_ND_A_SIZE( %s);\n",
                 nt);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "const int SAC_ND_A_DIM( %s);\n",
                 nt);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile,
                 "extern "
                 "int SAC_ND_A_SIZE( %s);\n",
                 nt);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "int SAC_ND_A_DIM( %s);\n",
                 nt);
        break;

    case C_hid:
        INDENT;
        fprintf (outfile, "SAC_NOTHING()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown data class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CHECK_REUSE( char *to_nt, int to_sdim,
 *                                  char *from_nt, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CHECK_REUSE( to_nt, to_sdim, from_nt, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_CHECK_REUSE (char *to_nt, int to_sdim, char *from_nt, int from_sdim)
{
    data_class_t to_dc = ICUGetDataClass (to_nt);
    unq_class_t to_uc = ICUGetUnqClass (to_nt);

    DBUG_ENTER ("ICMCompileND_CHECK_REUSE");

#define ND_CHECK_REUSE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CHECK_REUSE

    if ((to_uc == C_unq) || (to_dc == C_scl)) {
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
    } else {
        INDENT;
        fprintf (outfile, "SAC_IS_LASTREF__THEN( %s) {\n", from_nt);
        indent++;

        ICMCompileND_ASSIGN (to_nt, to_sdim, from_nt, from_sdim);

        INDENT;
        fprintf (outfile,
                 "SAC_TR_MEM_PRINT("
                 " (\"reuse memory of %s at %%p for %s\","
                 " ND_A_FIELD( %s)));\n",
                 from_nt, to_nt, from_nt);

        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        INDENT;
        fprintf (outfile, "SAC_IS_LASTREF__ELSE( %s)\n", from_nt);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_SET__SHAPE( char *to_nt, int to_sdim, int dim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_SET__SHAPE( to_nt, to_sdim, dim, shp_0 ... shp_n)
 *
 ******************************************************************************/

void
ICMCompileND_SET__SHAPE (char *to_nt, int to_sdim, int dim, int *shp)
{
    int i;
    data_class_t to_dc = ICUGetDataClass (to_nt);
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_SET__SHAPE");

#define ND_SET__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_SET__SHAPE

    DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");

    /*
     * set descriptor and non-constant part of mirror
     */
    switch (to_dc) {
    case C_aud:
        INDENT;
        fprintf (outfile, "SAC_ND_A_DESC_DIM( %s) = SAC_ND_A_DIM( %s) = %d;\n", to_nt,
                 to_nt, dim);
        /* here is no break missing! */

    case C_akd:
        INDENT;
        fprintf (outfile, "{ int SAC__size = 1;\n");
        indent++;
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile,
                     "SAC__size *= SAC_ND_A_DESC_SHAPE( %s, %d) ="
                     " SAC_ND_A_SHAPE( %s, %d) = %d;\n",
                     to_nt, i, to_nt, i, shp[i]);
        }
        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_DESC_SIZE( %s) ="
                 " SAC_ND_A_SIZE( %s) = SAC__size;\n",
                 to_nt, to_nt);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        break;

    case C_aks:
        /* here is no break missing */
    case C_scl:
        /* noop */
        break;

    case C_hid:
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown data class found!");
        break;
    }

    /*
     * check constant parts of mirror
     */
    switch (to_dc) {
    case C_scl:
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == 0),"
                 " (\"Assignment with incompatible types found!\"))\n",
                 to_nt);
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE( (SAC_ND_A_SIZE( %s) == 1),"
                 " (\"Assignment with incompatible types found!\"))\n",
                 to_nt);
        break;

    case C_aks:
        DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
        for (i = 0; i < to_dim; i++) {
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE( (SAC_ND_A_SHAPE( %s, %d) == %d),"
                     " (\"Assignment with incompatible types found!\"))\n",
                     to_nt, i, shp[i]);
        }
        /* here is no break missing */

    case C_akd:
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == %d),"
                 " (\"Assignment with incompatible types found!\"))\n",
                 to_nt, dim);
        break;

    case C_aud:
        /* noop */
        break;

    case C_hid:
        /* noop */
        break;

    default:
        DBUG_ASSERT ((0), "Unknown data class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_REFRESH_MIRROR( char *nt, int sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_REFRESH_MIRROR( nt, sdim)
 *
 ******************************************************************************/

void
ICMCompileND_REFRESH_MIRROR (char *nt, int sdim)
{
    int i;
    data_class_t dc = ICUGetDataClass (nt);
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ("ICMCompileND_REFRESH_MIRROR");

#define ND_REFRESH_MIRROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_REFRESH_MIRROR

    switch (dc) {
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
                     "SAC_ND_A_SHAPE( %s, %d) "
                     "= SAC_ND_A_DESC_SHAPE( %s, %d);\n",
                     nt, i, nt, i);
        }
        INDENT;
        fprintf (outfile, "SAC_ND_A_SIZE( %s) = SAC_ND_A_DESC_SIZE( %s);\n", nt, nt);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile, "SAC_ND_A_SIZE( %s) = SAC_ND_A_DESC_SIZE( %s);\n", nt, nt);
        INDENT;
        fprintf (outfile, "SAC_ND_A_DIM( %s) = SAC_ND_A_DESC_DIM( %s);\n", nt, nt);
        break;

    case C_hid:
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown data class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CHECK_MIRROR( char *to_nt, int to_sdim,
 *                                   char *from_nt, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CHECK_MIRROR( to_nt, to_sdim, from_nt, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_CHECK_MIRROR (char *to_nt, int to_sdim, char *from_nt, int from_sdim)
{
    int i;
    data_class_t to_dc = ICUGetDataClass (to_nt);
    data_class_t from_dc = ICUGetDataClass (from_nt);
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_CHECK_MIRROR");

#define ND_CHECK_MIRROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CHECK_MIRROR

    /*
     * check constant parts of mirror
     */
    switch (to_dc) {
    case C_scl:
        /* check dimension/size */
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE("
                 " (SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s)),"
                 " (\"Assignment with incompatible types found!\"))\n",
                 to_nt, from_nt);
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE("
                 " (SAC_ND_A_SIZE( %s) == SAC_ND_A_SIZE( %s)),"
                 " (\"Assignment with incompatible types found!\"))\n",
                 to_nt, from_nt);
        break;

    case C_aks:
        switch (from_dc) {
        case C_aud:
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE("
                     " (SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s)),"
                     " (\"Assignment with incompatible types found!\"))\n",
                     to_nt, from_nt);
            /* here is no break missing! */
        case C_akd:
            DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
            for (i = 0; i < to_dim; i++) {
                INDENT;
                fprintf (outfile,
                         "SAC_ASSURE_TYPE("
                         " (SAC_ND_A_SHAPE( %s, %d) == SAC_ND_A_SHAPE( %s, %d)),"
                         " (\"Assignment with incompatible types found!\"))\n",
                         to_nt, i, from_nt, i);
            }
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE("
                     " (SAC_ND_A_SIZE( %s) == SAC_ND_A_SIZE( %s)),"
                     " (\"Assignment with incompatible types found!\"))\n",
                     to_nt, from_nt);
            break;
        case C_aks:
            INDENT;
            fprintf (outfile, "SAC_NOOP()\n");
            break;
        default:
            DBUG_ASSERT ((0), ("Illegal assignment found!"));
            break;
        }
        break;

    case C_akd:
        switch (from_dc) {
        case C_aud:
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE("
                     " (SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s)),"
                     " (\"Assignment with incompatible types found!\"))\n",
                     to_nt, from_nt);
            break;
        case C_akd:
            /* here is no break missing */
        case C_aks:
            INDENT;
            fprintf (outfile, "SAC_NOOP()\n");
            break;
        default:
            DBUG_ASSERT ((0), ("Illegal assignment found!"));
            break;
        }
        break;

    case C_aud:
        /* here is no break missing */
    case C_hid:
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown data class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ASSIGN( char *to_nt, int to_sdim,
 *                             char *from_nt, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN( to_nt, to_sdim, from_nt, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN (char *to_nt, int to_sdim, char *from_nt, int from_sdim)
{
    DBUG_ENTER ("ICMCompileND_ASSIGN");

#define ND_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN

    ICMCompileND_ASSIGN__DESC (to_nt, from_nt);

    ICMCompileND_ASSIGN__SHAPE (to_nt, to_sdim, from_nt, from_sdim);

    INDENT;
    fprintf (outfile, "SAC_ND_ASSIGN__DATA( %s, %d, %s, %d)\n", to_nt, to_sdim, from_nt,
             from_sdim);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ASSIGN__DESC( char *to_nt, char *from_nt)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN__DESC( to_nt, to_sdim, from_nt, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN__DESC (char *to_nt, char *from_nt)
{
    data_class_t to_dc = ICUGetDataClass (to_nt);
    unq_class_t to_uc = ICUGetUnqClass (to_nt);
    data_class_t from_dc = ICUGetDataClass (from_nt);
    unq_class_t from_uc = ICUGetUnqClass (from_nt);

    DBUG_ENTER ("ICMCompileND_ASSIGN__DESC");

#define ND_ASSIGN__DESC
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN__DESC

    if ((to_dc == C_scl) && (from_dc == C_scl)) {
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
    } else if ((to_dc == C_scl) && (from_dc == C_aud)) {
        INDENT;
        fprintf (outfile, "SAC_ND_DEC_RC_FREE( %s, 1, )\n", from_nt);
    } else if ((to_dc == C_aud) && (from_dc == C_scl)) {
        INDENT;
        fprintf (outfile, "SAC_ND_ALLOC__DESC( %s)\n", to_nt);
        INDENT;
        fprintf (outfile, "SAC_ND_SET__RC( %s, 1)\n", to_nt);
    } else if (((to_dc == C_aks) || (to_dc == C_akd) || (to_dc == C_aud))
               && ((from_dc == C_aks) || (from_dc == C_akd) || (from_dc == C_aud))) {
        INDENT;
        fprintf (outfile, "SAC_ND_A_DESC( %s) = SAC_ND_A_DESC( %s);\n", to_nt, from_nt);
    } else if ((to_dc == C_hid) && (from_dc == C_hid)) {
        if ((to_uc == C_nuq) && (from_uc == C_nuq)) {
            INDENT;
            fprintf (outfile, "SAC_ND_A_DESC( %s) = SAC_ND_A_DESC( %s);\n", to_nt,
                     from_nt);
        } else if ((to_uc == C_nuq) && (from_uc == C_unq)) {
            INDENT;
            fprintf (outfile, "SAC_ND_ALLOC__DESC( %s)\n", to_nt);
            INDENT;
            fprintf (outfile, "SAC_ND_SET__RC( %s, 1)\n", to_nt);
        } else if ((to_uc == C_unq) && (from_uc == C_nuq)) {
            INDENT;
            fprintf (outfile, "SAC_ND_FREE__DESC( %s)\n", from_nt);
        } else if ((to_uc == C_unq) && (from_uc == C_unq)) {
            INDENT;
            fprintf (outfile, "SAC_NOOP()\n");
        } else {
            DBUG_ASSERT ((0), ("Illegal uniqueness class found!"));
        }
    } else {
        DBUG_ASSERT ((0), ("Illegal assignment found!"));
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ASSIGN__SHAPE( char *to_nt, int to_sdim,
 *                                    char *from_nt, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN__SHAPE( to_nt, to_sdim, from_nt, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN__SHAPE (char *to_nt, int to_sdim, char *from_nt, int from_sdim)
{
    int i;
    data_class_t to_dc = ICUGetDataClass (to_nt);
    data_class_t from_dc = ICUGetDataClass (from_nt);
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("ICMCompileND_ASSIGN__SHAPE");

#define ND_ASSIGN__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN__SHAPE

    ICMCompileND_CHECK_MIRROR (to_nt, to_sdim, from_nt, from_sdim);

    /*
     * assign missing descriptor entries
     */
    switch (to_dc) {
    case C_scl:
        /* here is no break missing */
    case C_aks:
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    case C_akd:
        switch (from_dc) {
        case C_aks:
            DBUG_ASSERT ((from_dim >= 0), "illegal dimension found!");
            for (i = 0; i < from_dim; i++) {
                INDENT;
                fprintf (outfile,
                         "SAC_ND_A_DESC_SHAPE( %s, %d)"
                         " = SAC_ND_A_SHAPE( %s, %d);\n",
                         to_nt, i, from_nt, i);
            }
            INDENT;
            fprintf (outfile, "SAC_ND_A_DESC_SIZE( %s) = SAC_ND_A_SIZE( %s);\n", to_nt,
                     from_nt);
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
        switch (from_dc) {
        case C_scl:
            /* here is no break missing! */
        case C_aks:
            DBUG_ASSERT ((from_dim >= 0), "illegal dimension found!");
            for (i = 0; i < from_dim; i++) {
                INDENT;
                fprintf (outfile,
                         "SAC_ND_A_DESC_SHAPE( %s, %d)"
                         " = SAC_ND_A_SHAPE( %s, %d);\n",
                         to_nt, i, from_nt, i);
            }
            INDENT;
            fprintf (outfile, "SAC_ND_A_DESC_SIZE( %s) = SAC_ND_A_SIZE( %s);\n", to_nt,
                     from_nt);
            /* here is no break missing */
        case C_akd:
            INDENT;
            fprintf (outfile, "SAC_ND_A_DESC_DIM( %s) = SAC_ND_A_DIM( %s);\n", to_nt,
                     from_nt);
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

    case C_hid:
        DBUG_ASSERT ((from_dc == C_hid), ("Illegal assignment found!"));

        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown data class found!");
        break;
    }

    /*
     * assign mirror
     */
    switch (to_dc) {
    case C_scl:
        /* here is no break missing */
    case C_aks:
        /* noop */
        break;

    case C_akd:
        DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
        for (i = 0; i < to_dim; i++) {
            INDENT;
            fprintf (outfile,
                     "SAC_ND_A_SHAPE( %s, %d) = "
                     "SAC_ND_A_SHAPE( %s, %d);\n",
                     to_nt, i, from_nt, i);
        }
        INDENT;
        fprintf (outfile, "SAC_ND_A_SIZE( %s) = SAC_ND_A_SIZE( %s);\n", to_nt, from_nt);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile, "SAC_ND_A_SIZE( %s) = SAC_ND_A_SIZE( %s);\n", to_nt, from_nt);
        INDENT;
        fprintf (outfile, "SAC_ND_A_DIM( %s) = SAC_ND_A_DIM( %s);\n", to_nt, from_nt);
        break;

    case C_hid:
        /* noop */
        break;

    default:
        DBUG_ASSERT ((0), "Unknown data class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_COPY( char *to_nt, int to_sdim,
 *                           char *from_nt, int from_sdim, char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_COPY( to_nt, to_sdim, from_nt, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_COPY (char *to_nt, int to_sdim, char *from_nt, int from_sdim, char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_COPY");

#define ND_COPY
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_COPY

    /* allocate descriptor */
    INDENT;
    fprintf (outfile, "SAC_ALLOC__DESC( %s)\n", to_nt);

    INDENT;
    fprintf (outfile, "SAC_ND_SET__RC( %s, 1)\n", to_nt);

    /* copy descriptor entries and mirror */
    ICMCompileND_COPY__SHAPE (to_nt, to_sdim, from_nt, from_sdim);

    INDENT;
    fprintf (outfile, "SAC_ND_ALLOC__DATA( %s)\n", to_nt);

    INDENT;
    fprintf (outfile, "SAC_ND_COPY__DATA( %s, %s, %s)\n", to_nt, from_nt, copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_COPY__SHAPE( char *to_nt, int to_sdim,
 *                                  char *from_nt, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_COPY__SHAPE( to_nt, to_sdim, from_nt, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_COPY__SHAPE (char *to_nt, int to_sdim, char *from_nt, int from_sdim)
{
    int i;
    data_class_t to_dc = ICUGetDataClass (to_nt);
    data_class_t from_dc = ICUGetDataClass (from_nt);
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("ICMCompileND_COPY__SHAPE");

#define ND_COPY__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_COPY__SHAPE

    ICMCompileND_CHECK_MIRROR (to_nt, to_sdim, from_nt, from_sdim);

    /*
     * copy descriptor entries and mirror
     */
    switch (to_dc) {
    case C_aud:
        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_DESC_DIM( %s) ="
                 " SAC_ND_A_DIM( %s) = SAC_ND_A_DIM( %s);\n",
                 to_nt, to_nt, from_nt);
        /* here is no break missing */
    case C_akd:
        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_DESC_SIZE( %s) ="
                 " SAC_ND_A_SIZE( %s) = SAC_ND_A_SIZE( %s);\n",
                 to_nt, to_nt, from_nt);
        break;

    case C_aks:
        /* here is no break missing */
    case C_scl:
        /* here is no break missing */
    case C_hid:
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown data class found!");
        break;
    }
    if ((to_dc == C_aud) && (from_dc == C_aud)) {
        INDENT;
        fprintf (outfile, "{ int SAC__i;\n");
        indent++;
        INDENT;
        fprintf (outfile,
                 "for (SAC__i = 0; SAC__i < SAC_ND_A_DIM( %s);"
                 " SAC__i++) {\n",
                 from_nt);
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_DESC_SHAPE( %s, SAC__i)"
                 " = SAC_ND_A_SHAPE( %s, SAC__i);\n",
                 to_nt, from_nt);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    } else if (to_dc == C_aud) {
        DBUG_ASSERT ((from_dim >= 0), "illegal dimension found!");
        for (i = 0; i < from_dim; i++) {
            INDENT;
            fprintf (outfile,
                     "SAC_ND_A_DESC_SHAPE( %s, %d)"
                     " = SAC_ND_A_SHAPE( %s, %d);\n",
                     to_nt, i, from_nt, i);
        }
    } else if (to_dc == C_akd) {
        DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
        for (i = 0; i < to_dim; i++) {
            INDENT;
            fprintf (outfile,
                     "SAC_ND_A_DESC_SHAPE( %s, %d) ="
                     " SAC_ND_A_SHAPE( %s, %d) = SAC_ND_A_SHAPE( %s, %d);\n",
                     to_nt, i, to_nt, i, from_nt, i);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_MAKE_UNIQUE( char *to_nt, int to_sdim,
 *                                  char *from_nt, int from_sdim, char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_MAKE_UNIQUE( to_nt, to_sdim, from_nt, from_sdim, basetype)
 *
 ******************************************************************************/

void
ICMCompileND_MAKE_UNIQUE (char *to_nt, int to_sdim, char *from_nt, int from_sdim,
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
             to_nt, to_sdim, from_nt, from_sdim, copyfun);
    INDENT;
    fprintf (outfile, "SAC_TR_REF_PRINT_RC( %s)\n", from_nt);
    INDENT;
    fprintf (outfile, "SAC_IS_LASTREF__BLOCK_BEGIN( %s)\n", from_nt);
    indent++;
    INDENT;
    fprintf (outfile, "SAC_TR_MEM_PRINT( (\"%s is already unique.\"))\n", from_nt);
    ICMCompileND_ASSIGN (to_nt, to_sdim, from_nt, from_sdim);
    indent--;
    INDENT;
    fprintf (outfile, "SAC_IS_LASTREF__BLOCK_ELSE( %s)\n", from_nt);
    indent++;
    ICMCompileND_COPY (to_nt, to_sdim, from_nt, from_sdim, copyfun);
    INDENT;
    fprintf (outfile, "SAC_ND_DEC_RC( %s, 1)\n", from_nt);
    indent--;
    INDENT;
    fprintf (outfile, "SAC_IS_LASTREF__BLOCK_END( %s)\n", from_nt);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE__VECT__SHAPE( char *nt, int sdim,
 *                                          int len, char **vala_any)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE__VECT__SHAPE( nt, sdim, len, vala_0 ... vala_n)
 *
 ******************************************************************************/

void
ICMCompileND_CREATE__VECT__SHAPE (char *nt, int sdim, int len, char **vala_any)
{
    bool entries_are_scalars;
    int i;

    DBUG_ENTER ("ICMCompileND_CREATE__VECT__SHAPE");

#define ND_CREATE__VECT__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE__VECT__SHAPE

    /*
     * CAUTION:
     * vala_any[i] is either a tagged identifier or a constant scalar!!!
     */

    entries_are_scalars = FALSE;
    for (i = 0; i < len; i++) {
        if ((vala_any[i][0] != '(') ||
            /* not a tagged id -> is a constant scalar! */
            (ICUGetDataClass (vala_any[i]) == C_scl)) {
            entries_are_scalars = TRUE;
        }
    }

    if (entries_are_scalars) {
        ICMCompileND_SET__SHAPE (nt, sdim, 1, &len);
    } else {
        DBUG_ASSERT ((0), "not yet implemented!");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE__VECT__DATA( char *nt, int sdim,
 *                                         int len, char **vala_any
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE__VEC__DATAT( nt, sdim, len, vala_0 ... vala_n)
 *
 ******************************************************************************/

void
ICMCompileND_CREATE__VECT__DATA (char *nt, int sdim, int len, char **vala_any)
{
    bool entries_are_scalars;
    int i;

    DBUG_ENTER ("ICMCompileND_CREATE__VECT__DATA");

#define ND_CREATE__VECT__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE__VECT__DATA

    /*
     * CAUTION:
     * vala_any[i] is either a tagged identifier or a constant scalar!!
     */

    entries_are_scalars = FALSE;
    for (i = 0; i < len; i++) {
        if ((vala_any[i][0] != '(') ||
            /* not a tagged id -> is a constant scalar! */
            (ICUGetDataClass (vala_any[i]) == C_scl)) {
            entries_are_scalars = TRUE;
        }
    }

    if (entries_are_scalars) {
        for (i = 0; i < len; i++) {
            if (vala_any[i][0] == '(') {
                /* entry is a tagged identifier */

                /* check whether entry is indeed a scalar */
                INDENT;
                fprintf (outfile,
                         "SAC_ASSURE_TYPE( (SAC_ND_A_SIZE( %s) == 0),"
                         " (\"Inconsistent vector found:"
                         " Entry at position %d is not a scalar!\"))\n",
                         vala_any[i], i);

                /* assign value of entry */
                INDENT;
                fprintf (outfile, "SAC_ND_WRITE( %s, %d) = SAC_ND_READ( %s, 0);\n", nt, i,
                         vala_any[i]);
            } else {
                /* entry is a constant */
                INDENT;
                fprintf (outfile, "SAC_ND_WRITE( %s, %d) = %s;\n", nt, i, vala_any[i]);
            }
        }
    } else {
        if (len > 0) {
            INDENT;
            fprintf (outfile, "{ int SAC__i = 0;\n");
            indent++;
            for (i = 0; i < len; i++) {
                /* check whether all entries have identical size */
                if (i > 0) {
                    INDENT;
                    fprintf (outfile,
                             "SAC_ASSURE_TYPE("
                             " (SAC_ND_A_SIZE( %s) == SAC_ND_A_SIZE( %s)),"
                             " (\"Inconsistent vector found:"
                             " First entry and entry at position %d have"
                             " different sizes!\"))\n",
                             vala_any[i], vala_any[0], i);
                }

                /* assign values of entry */
                INDENT;
                fprintf (outfile,
                         "for ( ; SAC__i < %d * SAC_ND_A_SIZE( %s); SAC__i++) "
                         "{\n",
                         i + 1, vala_any[i]);
                indent++;
                INDENT;
                fprintf (outfile,
                         "SAC_ND_WRITE( %s, SAC__i) = SAC_ND_READ( %s, SAC__i)"
                         ";\n",
                         nt, vala_any[i]);
                indent--;
                INDENT;
                fprintf (outfile, "}\n");
            }
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE( (SAC__i == SAC_ND_A_SIZE( %s)),"
                     " (\"Assignment with incompatible types found!\"))\n",
                     nt);
            indent--;
            INDENT;
            fprintf (outfile, "}\n");
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_SHAPE__DATA( char *to_nt, int to_sdim,
 *                                      char *from_nt, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SHAPE__DATA( to_nt, to_sdim, from_nt, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SHAPE__DATA (char *to_nt, int to_sdim, char *from_nt, int from_sdim)
{
    int i;
    data_class_t from_dc = ICUGetDataClass (from_nt);
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("ICMCompileND_PRF_SHAPE__DATA");

#define ND_PRF_SHAPE__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SHAPE__DATA

    switch (from_dc) {
    case C_scl:
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    case C_aks:
        /* here is no break missing */
    case C_akd:
        DBUG_ASSERT ((from_dim >= 0), "illegal dimension found!");
        for (i = 0; i < from_dim; i++) {
            INDENT;
            fprintf (outfile,
                     "SAC_ND_WRITE( %s, %d) = "
                     "SAC_ND_A_SHAPE( %s, %d);\n",
                     to_nt, i, from_nt, i);
        }
        break;

    case C_aud:
        INDENT;
        fprintf (outfile, "{ int SAC__i;\n");
        indent++;
        INDENT;
        fprintf (outfile,
                 "for ( SAC__i = 0; SAC__i < SAC_ND_A_DIM( %s);"
                 " SAC__i++) {\n",
                 from_nt);
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_ND_WRITE( %s, SAC__i) = "
                 "SAC_ND_A_SHAPE( %s, SAC__i);\n",
                 to_nt, from_nt);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        break;

    case C_hid:
        DBUG_ASSERT ((0), "ND_PRF_SHAPE__DATA() is undefined for C_hid!");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown data class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY__DATA_id( char *to_nt, int to_sdim,
 *                                            char *from_nt, int from_sdim,
 *                                            char *idx_any, char *val_any)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY__DATA_id( to_nt, to_sdim, from_nt, from_sdim,
 *                             idx_any, val_any)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY__DATA_id (char *to_nt, int to_sdim, char *from_nt,
                                    int from_sdim, char *idx_any, char *val_any)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY__DATA_id");

#define ND_PRF_MODARRAY__DATA_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY__DATA_id

    if (idx_any[0] == '(') {
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == 1),"
                 " (\"2nd argument of F_modarray has (dim != 1)!\"))\n",
                 idx_any);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY__DATA_arr( char *to_nt, int to_sdim,
 *                                             char *from_nt, int from_sdim,
 *                                             int idx_len, char **idx_arr,
 *                                             char *val_any)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY__DATA_arr( to_nt, to_sdim, from_nt, from_sdim,
 *                              idx_len, ...idxa_any..., val_any)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY__DATA_arr (char *to_nt, int to_sdim, char *from_nt,
                                     int from_sdim, int idx_len, char **idxa_any,
                                     char *val_any)
{
    int i;
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY__DATA_arr");

#define ND_PRF_MODARRAY__DATA_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY__DATA_arr

    for (i = 0; i < idx_len; i++) {
        if (idxa_any[i][0] == '(') {
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == 0),"
                     " (\"2nd argument of F_modarray has (dim != 1)!\"))\n",
                     idxa_any[i]);
        }
    }
    INDENT;
    fprintf (outfile,
             "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) >= %d),"
             " (\"2nd argument of F_modarray is too large!\"))\n",
             from_nt, idx_len);

    if ((val_any[0] != '(') ||
        /* not a tagged id -> is a constant scalar! */
        (ICUGetDataClass (val_any) == C_scl)) {
        INDENT;
        fprintf (outfile, "if (SAC_ND_A_FIELD( %s) != SAC_ND_A_FIELD( %s)) {\n", to_nt,
                 from_nt);
        indent++;
        INDENT;
        fprintf (outfile, "int SAC_i;\n");
        INDENT;
        fprintf (outfile,
                 "for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++)"
                 " {\n",
                 to_nt);
        indent++;
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE( %s, SAC_i) = SAC_ND_READ( %s, SAC_i);\n", to_nt,
                 from_nt);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        INDENT;
        fprintf (outfile, "{ int SAC_off;\n");
        indent++;
        VectToOffset ("SAC_off", idxa_any, idx_len, NULL, AccessConst, to_nt, to_dim);
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE( %s, SAC_off) = %s;\n", to_nt, val_any);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    } else {
        INDENT;
        fprintf (outfile, "{\n");
        indent++;
        INDENT;
        fprintf (outfile, "int SAC_i, SAC_j;\n");
        INDENT;
        fprintf (outfile, "int SAC_off;\n");
        VectToOffset ("SAC_off", idxa_any, idx_len, NULL, AccessConst, to_nt, to_dim);
        INDENT;
        fprintf (outfile, "if (SAC_ND_A_FIELD( %s) == SAC_ND_A_FIELD( %s)) {\n", to_nt,
                 from_nt);
        indent++;
        INDENT;
        fprintf (outfile,
                 "for (SAC_i = SAC_off, SAC_j = 0;"
                 " SAC_j < SAC_ND_A_SIZE( %s); SAC_i++, SAC_j++) {\n",
                 val_any);
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
                 " SAC_ND_READ_ARRAY( %s, SAC_j);\n",
                 to_nt, val_any);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        indent--;
        INDENT;
        fprintf (outfile, "} else {\n");
        indent++;
        INDENT;
        fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_off - 1; SAC_i++) {\n");
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
                 " SAC_ND_READ_ARRAY( %s, SAC_i);\n",
                 to_nt, from_nt);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        INDENT;
        fprintf (outfile,
                 "for (SAC_i = SAC_off, SAC_j = 0;"
                 " SAC_j < SAC_ND_A_SIZE( %s); SAC_i++, SAC_j++) {\n",
                 val_any);
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
                 " SAC_ND_READ_ARRAY( %s, SAC_j);\n",
                 to_nt, val_any);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        INDENT;
        fprintf (outfile, "for (; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++) {\n", to_nt);
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
                 " SAC_ND_READ_ARRAY( %s, SAC_i);\n",
                 to_nt, from_nt);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
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
 *   void ICMCompileND_PRF_IDX_MODARRAY__DATA( char *to_nt, int to_sdim,
 *                                             char *from_nt, int from_sdim,
 *                                             char *idx, char *val)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_IDX_MODARRAY__DATA( to_nt, to_sdim, from_nt, from_sdim, idx, val)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_IDX_MODARRAY__DATA (char *to_nt, int to_sdim, char *from_nt,
                                     int from_sdim, char *idx, char *val)
{
    DBUG_ENTER ("ICMCompileND_PRF_IDX_MODARRAY__DATA");

#define ND_PRF_IDX_MODARRAY__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_IDX_MODARRAY__DATA

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
 *   ND_KS_DECL_GLOBAL_ARRAY( basetype, name, dim, s0 ... sn)
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
        fprintf (outfile, "extern int SAC_ND_A_RC(%s);\n", name);
        INDENT;
        fprintf (outfile, "extern int const SAC_ND_A_SIZE(%s);\n", name);
        INDENT;
        fprintf (outfile, "extern int const SAC_ND_A_DIM(%s);\n", name);
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "extern int const SAC_ND_A_SHAPE( %s, %d);\n", name, i);
        }
    } else {
        INDENT;
        fprintf (outfile, "%s *%s;\n", basetype, name);
        INDENT;
        fprintf (outfile, "int SAC_ND_A_RC(%s);\n", name);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_SIZE(%s)=", name);
        fprintf (outfile, "%s", s[0]);
        for (i = 1; i < dim; i++)
            fprintf (outfile, "*%s", s[i]);
        fprintf (outfile, ";\n");
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_DIM(%s)=%d;\n", name, dim);
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "const int SAC_ND_A_SHAPE(%s, %d)=%s;\n", name, i, s[i]);
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
    fprintf (outfile, "extern int SAC_ND_A_RC(%s);\n", name);
    INDENT;
    fprintf (outfile, "extern int SAC_ND_A_SIZE(%s);\n", name);
    INDENT;
    fprintf (outfile, "extern int SAC_ND_A_DIM(%s);\n", name);
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "extern int SAC_ND_A_SHAPE(%s, %d);\n", name, i);
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
 *   ND_KS_DECL_ARRAY( basetype, name, dim, s0 ... sn)
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
 *   ND_KS_DECL_ARRAY_ARG( name, dim, s0 ... sn)
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
    fprintf (outfile, "const int SAC_ND_A_SIZE(%s)=", name);
    fprintf (outfile, "%s", s[0]);
    for (i = 1; i < dim; i++) {
        fprintf (outfile, "*%s", s[i]);
    }
    fprintf (outfile, ";\n");
    INDENT;
    fprintf (outfile, "const int SAC_ND_A_DIM(%s)=%d;\n", name, dim);
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_SHAPE(%s, %d)=%s;\n", name, i, s[i]);
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
 *   ND_CREATE_CONST_ARRAY_S( name, len, s0 ... sn)
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
 *   ND_CREATE_CONST_ARRAY_H( name, copyfun, len, A0 ... An)
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
 *   ND_CREATE_CONST_ARRAY_A( name, len2, len1, A0 ... An)
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
 *   void ICMCompileND_KD_SEL_CxA_S( char *a, char *res, int dim, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_SEL_CxA_S( a, res, dim, v0 ... vn)
 *   selects a single element of the array
 *
 ******************************************************************************/

void
ICMCompileND_KD_SEL_CxA_S (char *a, char *res, int dim, char **vi)
{
    DBUG_ENTER ("ICMCompileND_KD_SEL_CxA_S");

#define ND_KD_SEL_CxA_S
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_SEL_CxA_S

    INDENT;
    fprintf (outfile, "%s = SAC_ND_READ_ARRAY(%s, ", res, a);
    VectToOffset (dim, AccessConst (vi, i), dim, a);
    fprintf (outfile, ");\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_SEL_VxA_S( char *a, char *res, int dim, char *v)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_SEL_VxA_S( a, res, dim, v )
 *   selects a single element of the array
 *
 ******************************************************************************/

void
ICMCompileND_KD_SEL_VxA_S (char *a, char *res, int dim, char *v)
{
    DBUG_ENTER ("ICMCompileND_KD_SEL_VxA_S");

#define ND_KD_SEL_VxA_S
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_SEL_VxA_S

    INDENT;
    fprintf (outfile, "%s = SAC_ND_READ_ARRAY(%s, ", res, a);
    VectToOffset (dim, AccessVect (v, i), dim, a);
    fprintf (outfile, ");\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_SEL_CxA_A( int dima, char *a,
 *                                   char *res, int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_SEL_CxA_A( dima, a, res, dimv, v0 ... vn)
 *   selects a sub-array
 *
 ******************************************************************************/

void
ICMCompileND_KD_SEL_CxA_A (int dima, char *a, char *res, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_KD_SEL_CxA_A");

#define ND_KD_SEL_CxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_SEL_CxA_A

    INDENT;
    CopyBlock (a, VectToOffset (dimv, AccessConst (vi, i), dima, a), res);
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_SEL_VxA_A( int dima, char *a,
 *                                   char *res, int dimv, char *v)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_SEL_VxA_A( dima, a, res, dimv, v )
 *   selects a sub-array
 *
 ******************************************************************************/

void
ICMCompileND_KD_SEL_VxA_A (int dima, char *a, char *res, int dimv, char *v)
{
    DBUG_ENTER ("ICMCompileND_KD_SEL_VxA_A");

#define ND_KD_SEL_VxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_SEL_VxA_A

    INDENT;
    CopyBlock (a, VectToOffset (dimv, AccessVect (v, i), dima, a), res);
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_TAKE_CxA_A( int dima, char *a, char *res,
 *                                    int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_TAKE_CxA_A( dima, a, res, dimv, v0 ... vn)
 *
 ******************************************************************************/

void
ICMCompileND_KD_TAKE_CxA_A (int dima, char *a, char *res, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_KD_TAKE_CxA_A");

#define ND_KD_TAKE_CxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_TAKE_CxA_A

    INDENT;
    TakeSeg (a, dima, fprintf (outfile, "0"), /* offset */
             dimv,                            /* dim of sizes & offsets */
             AccessConst (vi, i),             /* sizes */
             fprintf (outfile, "(SAC_ND_A_SHAPE(%s, %d) - ", a, i);
             AccessConst (vi, i); fprintf (outfile, ")"), /* offsets */
                                  res);

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_DROP_CxA_A( int dima, char *a, char *res,
 *                                    int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_DROP_CxA_A( dima, a, res, dimv, v0 ... vn)
 *
 ******************************************************************************/

void
ICMCompileND_KD_DROP_CxA_A (int dima, char *a, char *res, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_KD_DROP_CxA_A");

#define ND_KD_DROP_CxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_DROP_CxA_A

    INDENT;
    TakeSeg (a, dima, VectToOffset (dimv, AccessConst (vi, i), dima, a), /* offset */
             dimv, /* dim of sizes & offsets */
             fprintf (outfile, "SAC_ND_A_SHAPE(%s, %d) - ", a, i);
             AccessConst (vi, i), /* sizes */
             AccessConst (vi, i), /* offsets */
             res);

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_CAT_SxAxA_A( int dima, char **ar, char *res,
 *                                     int catdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_CAT_SxAxA_A( dima, ar0, ar1, res, catdim)
 *
 ******************************************************************************/

void
ICMCompileND_KD_CAT_SxAxA_A (int dima, char **ar, char *res, int catdim)
{
    DBUG_ENTER ("ICMCompileND_KD_CAT_SxAxA_A");

#define ND_KD_CAT_SxAxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_CAT_SxAxA_A

    INDENT;
    NewBlock (InitPtr (fprintf (outfile, "0"), fprintf (outfile, "0"));
              InitVecs (1, 2, "SAC_isrc", fprintf (outfile, "0"));
              InitVecs (0, 2, "SAC_i", fprintf (outfile, "0"));
              InitVecs (0, 2, "SAC_bl",
                        {
                            int j;
                            for (j = catdim; j < dima; j++) {
                                fprintf (outfile, "SAC_ND_A_SHAPE(%s, %d)*", ar[i], j);
                            }
                            fprintf (outfile, "1");
                        }),
              FillRes (res, INDENT;
                       fprintf (outfile, "for (SAC_i0 = 0; SAC_i0 < SAC_bl0;"
                                         " SAC_i0++, SAC_idest++, SAC_isrc++)\n");
                       indent++; INDENT; fprintf (outfile,
                                                  "SAC_ND_WRITE_ARRAY(%s, SAC_idest) ="
                                                  " SAC_ND_READ_ARRAY(%s, SAC_isrc);\n",
                                                  res, ar[0]);
                       indent--; INDENT;
                       fprintf (outfile, "for (SAC_i1 = 0; SAC_i1 < SAC_bl1;"
                                         " SAC_i1++, SAC_idest++, SAC_isrc1++)\n");
                       indent++; INDENT; fprintf (outfile,
                                                  "SAC_ND_WRITE_ARRAY(%s, SAC_idest) ="
                                                  " SAC_ND_READ_ARRAY(%s, SAC_isrc1);\n",
                                                  res, ar[1]);
                       indent--; INDENT;));
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_ROT_CxSxA_A( int rotdim, char **numstr, int dima,
 *                                     char *a, char *res)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_ROT_CxSxA_A( rotdim, numstr, dima, a, res)
 *
 ******************************************************************************/

void
ICMCompileND_KD_ROT_CxSxA_A (int rotdim, char **numstr, int dima, char *a, char *res)
{
    DBUG_ENTER ("ICMCompileND_KD_ROT_CxSxA_A");

#define ND_KD_ROT_CxSxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_ROT_CxSxA_A

    INDENT;
    NewBlock (InitVecs (0, 1, "SAC_shift",
                        {
                            int j;
                            for (j = rotdim + 1; j < dima; j++) {
                                fprintf (outfile, "SAC_ND_A_SHAPE(%s, %d)*", a, j);
                            }
                            fprintf (outfile, "(%s<0 ? ", numstr[0]);
                            fprintf (outfile,
                                     "( SAC_ND_A_SHAPE( %s, %d)==0 ? 0 :"
                                     " SAC_ND_A_SHAPE( %s, %d)+"
                                     " (%s %% SAC_ND_A_SHAPE( %s, %d))) : ",
                                     a, rotdim, a, rotdim, numstr[0], a, rotdim);
                            fprintf (outfile,
                                     "( SAC_ND_A_SHAPE( %s, %d)==0 ? 0 :"
                                     " %s %% SAC_ND_A_SHAPE( %s, %d)))",
                                     a, rotdim, numstr[0], a, rotdim);
                        });
              InitVecs (0, 1, "SAC_bl",
                        {
                            int j;
                            for (j = rotdim + 1; j < dima; j++) {
                                fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)*", a, j);
                            }
                            fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)", a, rotdim);
                        });
              InitVecs (0, 1, "SAC_i", fprintf (outfile, "0"));
              InitPtr (fprintf (outfile, "-SAC_shift0"), fprintf (outfile, "0")),
              FillRes (res, INDENT; fprintf (outfile, "SAC_isrc+=SAC_bl0;\n"); INDENT;
                       fprintf (outfile, "for (SAC_i0 = 0; SAC_i0 < SAC_shift0;"
                                         " SAC_i0++, SAC_idest++,SAC_isrc++)\n");
                       indent++; INDENT; fprintf (outfile,
                                                  "SAC_ND_WRITE_ARRAY( %s, SAC_idest) ="
                                                  " SAC_ND_READ_ARRAY( %s, SAC_isrc);\n",
                                                  res, a);
                       indent--; INDENT; fprintf (outfile, "SAC_isrc -= SAC_bl0;\n");
                       INDENT; fprintf (outfile, "for (; SAC_i0 < SAC_bl0;"
                                                 " SAC_i0++, SAC_idest++, SAC_isrc++)\n");
                       indent++; INDENT; fprintf (outfile,
                                                  "SAC_ND_WRITE_ARRAY( %s, SAC_idest) ="
                                                  " SAC_ND_READ_ARRAY( %s, SAC_isrc);\n",
                                                  res, a);
                       indent--;));
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxCxS( char *res_btype, int dimres,
 *                                         char *res, char *old, char **value,
 *                                         int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxCxS( res_btype, dimres, res, old, value, dimv, v0 ... vn)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxCxS (char *res_btype, int dimres, char *res, char *old,
                                 char **value, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY_AxCxS");

#define ND_PRF_MODARRAY_AxCxS
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxCxS

    INDENT;
    fprintf (outfile, "if (SAC_ND_A_FIELD( %s) != SAC_ND_A_FIELD( %s)) {\n", old, res);
    indent++;
    INDENT;
    fprintf (outfile, "int SAC_i;\n");
    INDENT;
    fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++)\n", res);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_i);\n",
             res, old);
    indent--;
    INDENT;
    fprintf (outfile, "}\n");
    indent--;
    INDENT;
    fprintf (outfile, "SAC_ND_WRITE_ARRAY( %s, ", res);
    VectToOffset (dimv, AccessConst (vi, i), dimres, res);
    fprintf (outfile, ") = %s;\n", value[0]);
    INDENT;
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxVxS( char *res_btype, int dimres,
 *                                         char *res, char *old, char **value,
 *                                         int dim, char *v)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxVxS( res_btype, dimres, res, old, value, dimv, v)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxVxS (char *res_btype, int dimres, char *res, char *old,
                                 char **value, int dim, char *v)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY_AxVxS");

#define ND_PRF_MODARRAY_AxVxS
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxVxS

    INDENT;
    fprintf (outfile, "if (SAC_ND_A_FIELD( %s) != SAC_ND_A_FIELD( %s)) {\n", old, res);
    indent++;
    INDENT;
    fprintf (outfile, "int SAC_i;\n");
    INDENT;
    fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++)\n", res);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_i);\n",
             res, old);
    indent -= 2;
    INDENT;
    fprintf (outfile, "}\n");
    INDENT;
    fprintf (outfile, "SAC_ND_WRITE_ARRAY( %s, ", res);
    VectToOffset (dim, AccessVect (v, i), dimres, res);
    fprintf (outfile, ") = %s;\n", value[0]);
    INDENT;
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxCxA( char *res_btype, int dimres,
 *                                         char *res, char *old, char *val,
 *                                         int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxCxA( res_btype, dimres, res, old, val, dimv, v0 ... vn)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxCxA (char *res_btype, int dimres, char *res, char *old,
                                 char *val, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY_AxCxA");

#define ND_PRF_MODARRAY_AxCxA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxCxA

    INDENT;
    fprintf (outfile, "{\n");
    indent++;
    INDENT;
    fprintf (outfile, "int SAC_i, SAC_j;\n");
    INDENT;
    fprintf (outfile, "int SAC_idx = ");
    VectToOffset (dimv, AccessConst (vi, i), dimres, res);
    fprintf (outfile, ";\n");
    INDENT;
    fprintf (outfile, "if (SAC_ND_A_FIELD( %s) == SAC_ND_A_FIELD( %s)) {\n", old, res);
    indent++;
    INDENT;
    fprintf (outfile,
             "for (SAC_i = SAC_idx, SAC_j = 0;"
             " SAC_j < SAC_ND_A_SIZE( %s); SAC_i++, SAC_j++)\n",
             val);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_j);\n",
             res, val);
    indent -= 2;
    INDENT;
    fprintf (outfile, "} else {\n");
    indent++;
    INDENT;
    fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_idx - 1; SAC_i++)\n");
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_i);\n",
             res, old);
    indent--;
    INDENT;
    fprintf (outfile,
             "for (SAC_i = SAC_idx, SAC_j = 0;"
             " SAC_j < SAC_ND_A_SIZE( %s); SAC_i++, SAC_j++)\n",
             val);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_j);\n",
             res, val);
    indent--;
    INDENT;
    fprintf (outfile, "for (; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++)\n", res);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_i);\n",
             res, old);
    indent -= 2;
    INDENT;
    fprintf (outfile, "}\n");
    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxVxA( char *res_btype, int dimres,
 *                                         char *res, char *old, char *val,
 *                                         int dim, char *v)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxVxA( res_btype, dimres, res, old, val, dim, v)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxVxA (char *res_btype, int dimres, char *res, char *old,
                                 char *val, int dim, char *v)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY_AxVxA");

#define ND_PRF_MODARRAY_AxVxA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxVxA

    INDENT;
    fprintf (outfile, "{\n");
    indent++;
    INDENT;
    fprintf (outfile, "int SAC_i, SAC_j;\n");
    INDENT;
    fprintf (outfile, "int SAC_idx=");
    VectToOffset (dim, AccessVect (v, i), dimres, res);
    fprintf (outfile, ";\n");
    INDENT;
    fprintf (outfile, "if (SAC_ND_A_FIELD( %s) == SAC_ND_A_FIELD( %s)) {\n", old, res);
    indent++;
    INDENT;
    fprintf (outfile,
             "for (SAC_i = SAC_idx, SAC_j = 0;"
             " SAC_j < SAC_ND_A_SIZE( %s); SAC_i++, SAC_j++)\n",
             val);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_j);\n",
             res, val);
    indent -= 2;
    INDENT;
    fprintf (outfile, "} else {\n");
    indent++;
    INDENT;
    fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_idx - 1; SAC_i++)\n");
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY(%s, SAC_i) ="
             " SAC_ND_READ_ARRAY(%s, SAC_i);\n",
             res, old);
    indent--;
    INDENT;
    fprintf (outfile,
             "for (SAC_i = SAC_idx, SAC_j = 0;"
             " SAC_j < SAC_ND_A_SIZE( %s); SAC_i++, SAC_j++)\n",
             val);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_j);\n",
             res, val);
    indent--;
    INDENT;
    fprintf (outfile, "for (; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++)\n", res);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_i);\n",
             res, old);
    indent -= 2;
    INDENT;
    fprintf (outfile, "}\n");
    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    DBUG_VOID_RETURN;
}

#endif /* TAGGED_ARRAYS */

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KS_VECT2OFFSET( char *off_name, char *arr_name,
 *                                     int dim, int dims, char **shp_any)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KS_VECT2OFFSET( off_name, arr_name, dim, dims, shp_any )
 *
 ******************************************************************************/

void
ICMCompileND_KS_VECT2OFFSET (char *off_name, char *arr_name, int dim, int dims,
                             char **shp_any)
{
    DBUG_ENTER ("ICMCompileND_KS_VECT2OFFSET");

#define ND_KS_VECT2OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KS_VECT2OFFSET

#ifdef TAGGED_ARRAYS
    VectToOffset2 (off_name, arr_name, dim, NULL, AccessVect, shp_any, dims, NULL,
                   AccessConst);
#else
    INDENT;
    fprintf (outfile, "%s = ", off_name);
    VectToOffset2 (dim, AccessVect (arr_name, i), dims, AccessConst (shp_any, i));
    fprintf (outfile, ";\n");
#endif

    DBUG_VOID_RETURN;
}
