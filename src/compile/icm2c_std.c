/*
 *
 * $Log$
 * Revision 3.30  2002/08/05 18:48:54  dkr
 * comments corrected
 *
 * Revision 3.29  2002/08/05 18:21:33  dkr
 * - some shape calcualtions for AKD, AUD implemented
 * - ND_ASSIGN__SHAPE renamed into ND_ASSIGN__DIMSHP
 *
 * Revision 3.28  2002/08/03 03:16:20  dkr
 * ND_PRF_SEL__DIM icms removed
 *
 * Revision 3.27  2002/08/02 20:59:21  dkr
 * ND_CREATE__VECT__DIM: comment added
 *
 * Revision 3.26  2002/08/02 20:48:50  dkr
 * ..__DIM.. icms added
 *
 * Revision 3.25  2002/07/31 16:35:48  dkr
 * - tags reorganized: HID/NHD are seperate classes now
 * - support for arrays of hidden added
 *
 * Revision 3.24  2002/07/24 15:04:03  dkr
 * ND_VECT2OFFSET modified
 *
 * Revision 3.23  2002/07/15 18:40:03  dkr
 * some bugs fixed
 *
 * Revision 3.22  2002/07/12 18:54:46  dkr
 * first (almost) complete TAGGED_ARRAYS revision.
 * some shape computations are missing yet (but SCL, AKS should be
 * complete)
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

#ifndef TAGGED_ARRAYS

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

#define CopyBlock(a, offset, res)                                                        \
    NewBlock (InitPtr (offset, fprintf (outfile, "0")),                                  \
              FillRes (res, INDENT; fprintf (outfile,                                    \
                                             "SAC_ND_WRITE_ARRAY( %s, SAC_idest)"        \
                                             " = SAC_ND_READ_ARRAY( %s, SAC_isrc);\n",   \
                                             res, a);                                    \
                       INDENT; fprintf (outfile, "SAC_idest++; SAC_isrc++;\n");))

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
 *   void ICMCompileND_FUN_DEC( char *name, char *rettype_nt,
 *                              int narg, char **arg_any)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEC( name, rettype_nt, narg, [ TAG, basetype, arg_nt ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_DEC (char *name, char *rettype_nt, int narg, char **arg_any)
{
    DBUG_ENTER ("ICMCompileND_FUN_DEC");

#define ND_FUN_DEC
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_DEC

    INDENT;
#ifdef TAGGED_ARRAYS
    if (rettype_nt[0] != '\0') {
        fprintf (outfile, "SAC_ND_TYPE_NT( %s) ", rettype_nt);
    } else {
        fprintf (outfile, "void ");
    }
#else
    fprintf (outfile, "%s ", rettype_nt);
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
    shape_class_t sc = ICUGetShapeClass (nt);
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

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
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
    shape_class_t sc = ICUGetShapeClass (nt);
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

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
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
    shape_class_t sc = ICUGetShapeClass (nt);
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

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CHECK_REUSE( char *to_nt, int to_sdim,
 *                                  char *from_nt, int from_sdim,
 *                                  char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CHECK_REUSE( to_nt, to_sdim, from_nt, from_sdim, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_CHECK_REUSE (char *to_nt, int to_sdim, char *from_nt, int from_sdim,
                          char *copyfun)
{
    shape_class_t to_sc = ICUGetShapeClass (to_nt);
    unique_class_t to_uc = ICUGetUniqueClass (to_nt);

    DBUG_ENTER ("ICMCompileND_CHECK_REUSE");

#define ND_CHECK_REUSE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CHECK_REUSE

    if ((to_uc == C_unq) || (to_sc == C_scl)) {
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
    } else {
        INDENT;
        fprintf (outfile, "SAC_IS_LASTREF__THEN( %s) {\n", from_nt);
        indent++;

        ICMCompileND_ASSIGN (to_nt, to_sdim, from_nt, from_sdim, copyfun);

        INDENT;
        fprintf (outfile,
                 "SAC_TR_MEM_PRINT( (\"reuse memory of %s at %%p for %s\","
                 " ND_A_FIELD( %s)))\n",
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
 *   void ICMCompileND_SET__SHAPE( char *to_nt, int dim, char **shp_any)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_SET__SHAPE( to_nt, dim, shp_0 ... shp_n)
 *
 ******************************************************************************/

void
ICMCompileND_SET__SHAPE (char *to_nt, int dim, char **shp_any)
{
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_nt);

    DBUG_ENTER ("ICMCompileND_SET__SHAPE");

#define ND_SET__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_SET__SHAPE

    /*
     * CAUTION:
     * 'shp_any[i]' is either a tagged identifier or a constant scalar!!
     */

    /*
     * set descriptor and non-constant part of mirror
     */
    switch (to_sc) {
    case C_aud:
        /*
         * ND_A_DESC_DIM, ND_A_DIM have already been set by ND_ALLOC__DESC!
         */
#if 0
      INDENT;
      fprintf( outfile, "SAC_ND_A_DESC_DIM( %s) = SAC_ND_A_DIM( %s) = %d;\n",
                        to_nt, to_nt, dim);
#else
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == %d),"
                 " (\"Assignment with incompatible types found!\"));\n",
                 to_nt, dim);
#endif
        /* here is no break missing! */

    case C_akd:
        INDENT;
        fprintf (outfile, "{ int SAC_size = 1;\n");
        indent++;
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile,
                     "SAC_size *= SAC_ND_A_DESC_SHAPE( %s, %d) ="
                     " SAC_ND_A_SHAPE( %s, %d) = ",
                     to_nt, i, to_nt, i);
            ReadScalar (shp_any[i], NULL, 0);
            fprintf (outfile, ";\n");
        }
        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_DESC_SIZE( %s) ="
                 " SAC_ND_A_SIZE( %s) = SAC_size;\n",
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

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    /*
     * check constant parts of mirror
     */
    switch (to_sc) {
    case C_scl:
        DBUG_ASSERT ((dim == 0), "illegal dimension found!");
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    case C_aks:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "SAC_ASSURE_TYPE( (SAC_ND_A_SHAPE( %s, %d) == ", to_nt, i);
            ReadScalar (shp_any[i], NULL, 0);
            fprintf (outfile, "), (\"Assignment with incompatible types"
                              " found!\"));\n");
        }
        /* here is no break missing */

    case C_akd:
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == %d),"
                 " (\"Assignment with incompatible types found!\"));\n",
                 to_nt, dim);
        break;

    case C_aud:
        /* noop */
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
    shape_class_t sc = ICUGetShapeClass (nt);
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ("ICMCompileND_REFRESH_MIRROR");

#define ND_REFRESH_MIRROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_REFRESH_MIRROR

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

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
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
    shape_class_t to_sc = ICUGetShapeClass (to_nt);
    shape_class_t from_sc = ICUGetShapeClass (from_nt);
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_CHECK_MIRROR");

#define ND_CHECK_MIRROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CHECK_MIRROR

#if 1
    if ((runtimecheck & RUNTIMECHECK_TYPE) && (from_sc == C_aud)) {
        INDENT;
        fprintf (outfile, "if (SAC_ND_A_DIM( %s) > 0) {\n", from_nt);
        indent++;
    }
#endif

    /*
     * check constant parts of mirror
     */
    switch (to_sc) {
    case C_scl:
        /* check dimension/size */
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE("
                 " (SAC_ND_A_DIM( %s) == 0),"
                 " (\"Assignment with incompatible types found!\"));\n",
                 from_nt);
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE("
                 " (SAC_ND_A_SIZE( %s) == 1),"
                 " (\"Assignment with incompatible types found!\"));\n",
                 from_nt);
        break;

    case C_aks:
        switch (from_sc) {
        case C_aud:
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE("
                     " (SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s)),"
                     " (\"Assignment with incompatible types found!\"));\n",
                     to_nt, from_nt);
            /* here is no break missing! */
        case C_akd:
            DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
            for (i = 0; i < to_dim; i++) {
                INDENT;
                fprintf (outfile,
                         "SAC_ASSURE_TYPE("
                         " (SAC_ND_A_SHAPE( %s, %d) == SAC_ND_A_SHAPE( %s, %d)),"
                         " (\"Assignment with incompatible types found!\"));\n",
                         to_nt, i, from_nt, i);
            }
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE("
                     " (SAC_ND_A_SIZE( %s) == SAC_ND_A_SIZE( %s)),"
                     " (\"Assignment with incompatible types found!\"));\n",
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
        switch (from_sc) {
        case C_aud:
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE("
                     " (SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s)),"
                     " (\"Assignment with incompatible types found!\"));\n",
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
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

#if 1
    if ((runtimecheck & RUNTIMECHECK_TYPE) && (from_sc == C_aud)) {
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    }
#endif

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ASSIGN( char *to_nt, int to_sdim,
 *                             char *from_nt, int from_sdim,
 *                             char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN( to_nt, to_sdim, from_nt, from_sdim, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN (char *to_nt, int to_sdim, char *from_nt, int from_sdim,
                     char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_ASSIGN");

#define ND_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN

    ICMCompileND_ASSIGN__DESC (to_nt, from_nt);

    ICMCompileND_ASSIGN__DIMSHP (to_nt, to_sdim, from_nt, from_sdim);

    INDENT;
    fprintf (outfile, "SAC_ND_ASSIGN__DATA( %s, %s, %s)\n", to_nt, from_nt, copyfun);

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
    shape_class_t to_sc = ICUGetShapeClass (to_nt);
    hidden_class_t to_hc = ICUGetHiddenClass (to_nt);
    unique_class_t to_uc = ICUGetUniqueClass (to_nt);
    shape_class_t from_sc = ICUGetShapeClass (from_nt);
    hidden_class_t from_hc = ICUGetHiddenClass (from_nt);
    unique_class_t from_uc = ICUGetUniqueClass (from_nt);

    DBUG_ENTER ("ICMCompileND_ASSIGN__DESC");

#define ND_ASSIGN__DESC
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN__DESC

    DBUG_ASSERT ((to_hc == from_hc), "Illegal assignment found!");

    if (((to_sc == C_scl) && ((to_hc == C_nhd) || (to_uc == C_unq)))
        && ((from_sc == C_scl) && ((from_hc == C_nhd) || (from_uc == C_unq)))) {
        /* 'to_nt' has not a desc, 'from_nt' has not a desc */
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
    } else if (((to_sc == C_scl) && ((to_hc == C_nhd) || (to_uc == C_unq)))
               && ((from_sc == C_aud) || ((from_hc == C_hid) && (from_uc == C_nuq)))) {
        /* 'to_nt' has not a desc, 'from_nt' has a desc */
        INDENT;
        if (to_hc == C_hid) {
            DBUG_ASSERT ((to_uc == C_unq), "Illegal assignment found!");
            /*
             * data vector of 'from_nt' is reused by 'to_nt'
             * (and 'to_nt' is unique, i.e. RC of 'from_nt' is 1)
             *   -> ND_FREE__DESC( from_nt)
             */
            fprintf (outfile, "SAC_ND_FREE__DESC( %s)\n", from_nt);
        } else {
            /*
             * data vector of 'from_nt' is not reused by 'to_nt'
             *   -> ND_DEC_RC_FREE( from_nt) in ND_ASSIGN__DATA
             */
        }
    } else if (((to_sc == C_aud) || ((to_hc == C_hid) && (to_uc == C_nuq)))
               && ((from_sc == C_scl) && ((from_hc == C_nhd) || (from_uc == C_unq)))) {
        /* 'to_nt' has a desc, 'from_nt' has not a desc */
        INDENT;
        fprintf (outfile, "SAC_ND_ALLOC__DESC( %s, 1)\n", to_nt);
        INDENT;
        fprintf (outfile, "SAC_ND_SET__RC( %s, 1)\n", to_nt);
    } else if (((to_sc == C_aks) || (to_sc == C_akd) || (to_sc == C_aud))
               && ((from_sc == C_aks) || (from_sc == C_akd) || (from_sc == C_aud))) {
        /* 'to_nt' has a desc, 'from_nt' has a desc, no scalar involved */
        INDENT;
        fprintf (outfile, "SAC_ND_A_DESC( %s) = SAC_ND_A_DESC( %s);\n", to_nt, from_nt);
    } else if (((to_sc == C_aud) || ((to_hc == C_hid) && (to_uc == C_nuq)))
               && ((from_sc == C_aud) || ((from_hc == C_hid) && (from_uc == C_nuq)))) {
        /* 'to_nt' has a desc, 'from_nt' has a desc, scalar involved */
        if ((to_sc == C_aud) || ((from_sc == C_aud) && (from_uc == C_nuq))) {
            /*
             * 'from_nt' is a non-unique hidden and cannot be reused by 'to_nt'
             *   -> ND_ALLOC__DESC( to_nt, 1)
             *   -> ND_DEC_RC_FREE( from_nt) in ND_ASSIGN__DATA
             */
            INDENT;
            fprintf (outfile, "SAC_ND_ALLOC__DESC( %s, 1)\n", to_nt);
        } else {
            INDENT;
            fprintf (outfile, "SAC_ND_A_DESC( %s) = SAC_ND_A_DESC( %s);\n", to_nt,
                     from_nt);
        }
    } else {
        DBUG_ASSERT ((0), "Illegal assignment found!");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ASSIGN__DIMSHP( char *to_nt, int to_sdim,
 *                                     char *from_nt, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN__DIMSHP( to_nt, to_sdim, from_nt, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN__DIMSHP (char *to_nt, int to_sdim, char *from_nt, int from_sdim)
{
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_nt);
    shape_class_t from_sc = ICUGetShapeClass (from_nt);
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("ICMCompileND_ASSIGN__DIMSHP");

#define ND_ASSIGN__DIMSHP
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN__DIMSHP

    ICMCompileND_CHECK_MIRROR (to_nt, to_sdim, from_nt, from_sdim);

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

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    /*
     * assign mirror
     */
    switch (to_sc) {
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

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_COPY( char *to_nt, int to_sdim,
 *                           char *from_nt, int from_sdim,
 *                           char *copyfun)
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
    fprintf (outfile, "SAC_ND_ALLOC_BEGIN( %s, 1, SAC_ND_A_DIM( %s))\n", to_nt, from_nt);

    /* copy descriptor entries and mirror */
    ICMCompileND_COPY__SHAPE (to_nt, to_sdim, from_nt, from_sdim);

    INDENT;
    fprintf (outfile, "SAC_ND_ALLOC_END( %s, 1, SAC_ND_A_DIM( %s))\n", to_nt, from_nt);

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
    shape_class_t to_sc = ICUGetShapeClass (to_nt);
    shape_class_t from_sc = ICUGetShapeClass (from_nt);
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
    switch (to_sc) {
    case C_aud:
        /*
         * ND_A_DESC_DIM, ND_A_DIM have already been set by ND_ALLOC__DESC!
         */
#if 0
      INDENT;
      fprintf( outfile, "SAC_ND_A_DESC_DIM( %s)"
                        " = SAC_ND_A_DIM( %s) = SAC_ND_A_DIM( %s);\n",
                        to_nt, to_nt, from_nt);
#else
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE("
                 " (SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s)),"
                 " (\"Assignment with incompatible types found!\"));\n",
                 to_nt, from_nt);
#endif
        /* here is no break missing */
    case C_akd:
        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_DESC_SIZE( %s)"
                 " = SAC_ND_A_SIZE( %s) = SAC_ND_A_SIZE( %s);\n",
                 to_nt, to_nt, from_nt);
        break;

    case C_aks:
        /* here is no break missing */
    case C_scl:
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }
    if ((to_sc == C_aud) && (from_sc == C_aud)) {
        INDENT;
        fprintf (outfile, "{ int SAC_i;\n");
        indent++;
        INDENT;
        fprintf (outfile,
                 "for (SAC_i = 0; SAC_i < SAC_ND_A_DIM( %s);"
                 " SAC_i++) {\n",
                 from_nt);
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_DESC_SHAPE( %s, SAC_i)"
                 " = SAC_ND_A_SHAPE( %s, SAC_i);\n",
                 to_nt, from_nt);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    } else if (to_sc == C_aud) {
        DBUG_ASSERT ((from_dim >= 0), "illegal dimension found!");
        for (i = 0; i < from_dim; i++) {
            INDENT;
            fprintf (outfile,
                     "SAC_ND_A_DESC_SHAPE( %s, %d)"
                     " = SAC_ND_A_SHAPE( %s, %d);\n",
                     to_nt, i, from_nt, i);
        }
    } else if (to_sc == C_akd) {
        DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
        for (i = 0; i < to_dim; i++) {
            INDENT;
            fprintf (outfile,
                     "SAC_ND_A_DESC_SHAPE( %s, %d) = SAC_ND_A_SHAPE( %s, %d)"
                     " = SAC_ND_A_SHAPE( %s, %d);\n",
                     to_nt, i, to_nt, i, from_nt, i);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_MAKE_UNIQUE( char *to_nt, int to_sdim,
 *                                  char *from_nt, int from_sdim,
 *                                  char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_MAKE_UNIQUE( to_nt, to_sdim, from_nt, from_sdim, copyfun)
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
    ICMCompileND_ASSIGN (to_nt, to_sdim, from_nt, from_sdim, copyfun);
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
 *C
 * function:
 *   void ICMCompileND_CREATE__VECT__DIM( int val_size, char **vala_any)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE__VECT__DIM( val_size, vala_0 ... vala_n)
 *
 ******************************************************************************/

void
ICMCompileND_CREATE__VECT__DIM (int val_size, char **vala_any)
{
    bool entries_are_scalars;
    int i;

    DBUG_ENTER ("ICMCompileND_CREATE__VECT__DIM");

#define ND_CREATE__VECT__DIM
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE__VECT__DIM

    /*
     * CAUTION:
     * 'vala_any[i]' is either a tagged identifier or a constant scalar!!!
     */

    entries_are_scalars = FALSE;
    for (i = 0; i < val_size; i++) {
        if ((vala_any[i][0] != '(') ||
            /* not a tagged id -> is a constant scalar! */
            (ICUGetShapeClass (vala_any[i]) == C_scl)) {
            entries_are_scalars = TRUE;
        }
    }

    if (val_size > 0) {
        /*
         * 'A = []' works only for arrays with known dimension/shape!!!
         */
        fprintf (outfile, "SAC_ICM_UNDEF()");
    } else if (entries_are_scalars) {
        fprintf (outfile, "1");
    } else {
        fprintf (outfile, "SAC_ND_A_DIM( %s) + 1", vala_any[0]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE__VECT__SHAPE( char *to_nt, int to_sdim,
 *                                          int val_size, char **vala_any)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE__VECT__SHAPE( nt, sdim, val_size, vala_0 ... vala_n)
 *
 ******************************************************************************/

void
ICMCompileND_CREATE__VECT__SHAPE (char *to_nt, int to_sdim, int val_size, char **vala_any)
{
    bool entries_are_scalars;
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_nt);
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_CREATE__VECT__SHAPE");

#define ND_CREATE__VECT__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE__VECT__SHAPE

    /*
     * CAUTION:
     * 'vala_any[i]' is either a tagged identifier or a constant scalar!!!
     */

    entries_are_scalars = FALSE;
    for (i = 0; i < val_size; i++) {
        if ((vala_any[i][0] != '(') ||
            /* not a tagged id -> is a constant scalar! */
            (ICUGetShapeClass (vala_any[i]) == C_scl)) {
            entries_are_scalars = TRUE;
        }
    }

    if (val_size == 0) {
        /*
         * 'A = []' works only for arrays with known dimension/shape!!!
         */
        DBUG_ASSERT ((to_sc == C_aks), "[] with unknown shape found!");
    } else if (entries_are_scalars) {
        char *val_size_str = Malloc (20 * sizeof (char));
        sprintf (val_size_str, "%d", val_size);
        ICMCompileND_SET__SHAPE (to_nt, 1, &val_size_str);
        val_size_str = Free (val_size_str);
    } else {
        /*
         * set descriptor and non-constant part of mirror
         */
        switch (to_sc) {
        case C_aud:
            /* check whether all entries have identical dimension */
            for (i = 0; i < val_size; i++) {
                if (i > 0) {
                    INDENT;
                    fprintf (outfile,
                             "SAC_ASSURE_TYPE("
                             " (SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s)),"
                             " (\"Inconsistent vector found:"
                             " First entry and entry at position %d have"
                             " different dimensions!\"));\n",
                             vala_any[i], vala_any[0], i);
                }
            }

            /*
             * ND_A_DESC_DIM, ND_A_DIM have already been set by ND_ALLOC__DESC!
             */
#if 0
        INDENT;
        fprintf( outfile, "SAC_ND_A_DESC_DIM( %s) = SAC_ND_A_DIM( %s)"
                          " = SAC_ND_A_DIM( %s) + 1;\n",
                          to_nt, to_nt, vala_any[0]);
#else
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE("
                     " (SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s) + 1),"
                     " (\"Assignment with incompatible types found!\"));\n",
                     to_nt, vala_any[0]);
#endif
            INDENT;
            fprintf (outfile, "{ int SAC_size = 1;\n");
            indent++;
            INDENT;
            fprintf (outfile,
                     "SAC_size *= SAC_ND_A_DESC_SHAPE( %s, 0)"
                     " = SAC_ND_A_SHAPE( %s, 0) = %d;\n",
                     to_nt, to_nt, val_size);
            INDENT;
            fprintf (outfile,
                     "for (SAC_i = 1; SAC_i < SAC_ND_A_DIM( %s); SAC_i++)"
                     " {\n",
                     to_nt);
            indent++;
            INDENT;
            fprintf (outfile,
                     "SAC_size *= SAC_ND_A_DESC_SHAPE( %s, SAC_i)"
                     " = SAC_ND_A_SHAPE( %s, SAC_i)"
                     " = SAC_ND_A_SHAPE( %s, SAC_i - 1);\n",
                     to_nt, to_nt, vala_any[0]);
            indent--;
            INDENT;
            fprintf (outfile, "}\n");
            INDENT;
            fprintf (outfile,
                     "SAC_ND_A_DESC_SIZE( %s)"
                     " = SAC_ND_A_SIZE( %s) = SAC_size;\n",
                     to_nt, to_nt);
            indent--;
            INDENT;
            fprintf (outfile, "}\n");
            break;

        case C_akd:
            /* check whether all entries have identical shape */
            for (i = 0; i < val_size; i++) {
                if (i > 0) {
                    int d;
                    for (d = 0; d < to_dim - 1; d++) {
                        INDENT;
                        fprintf (outfile,
                                 "SAC_ASSURE_TYPE("
                                 " (SAC_ND_A_SHAPE( %s, %d) == SAC_ND_A_SHAPE( %s, %d)),"
                                 " (\"Inconsistent vector found:"
                                 " First entry and entry at position %d have"
                                 " different shape!\"));\n",
                                 vala_any[i], d, vala_any[0], d, i);
                    }
                }
            }

            INDENT;
            fprintf (outfile, "{ int SAC_size = 1;\n");
            indent++;
            INDENT;
            fprintf (outfile,
                     "SAC_size *= SAC_ND_A_DESC_SHAPE( %s, 0)"
                     " = SAC_ND_A_SHAPE( %s, 0) = %d;\n",
                     to_nt, to_nt, val_size);
            DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
            for (i = 1; i < to_dim; i++) {
                INDENT;
                fprintf (outfile,
                         "SAC_size *= SAC_ND_A_DESC_SHAPE( %s, %d)"
                         " = SAC_ND_A_SHAPE( %s, %d)"
                         " = SAC_ND_A_SHAPE( %s, %d);\n",
                         to_nt, i, to_nt, i, vala_any[0], i - 1);
            }
            INDENT;
            fprintf (outfile,
                     "SAC_ND_A_DESC_SIZE( %s)"
                     " = SAC_ND_A_SIZE( %s) = SAC_size;\n",
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

        default:
            DBUG_ASSERT ((0), "Unknown shape class found!");
            break;
        }

        /*
         * check constant parts of mirror
         */
        switch (to_sc) {
        case C_scl:
            DBUG_ASSERT ((0), "illegal dimension found!");
            break;

        case C_aks:
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE( (SAC_ND_A_SHAPE( %s, 0) == %d),"
                     " (\"Assignment with incompatible types"
                     " found!\"));\n",
                     to_nt, val_size);
            DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
            for (i = 1; i < to_dim; i++) {
                INDENT;
                fprintf (outfile,
                         "SAC_ASSURE_TYPE("
                         " (SAC_ND_A_SHAPE( %s, %d) == SAC_ND_A_SHAPE( %s, %d)),"
                         " (\"Assignment with incompatible types found!\"));\n",
                         to_nt, i, vala_any[0], i - 1);
            }
            /* here is no break missing */

        case C_akd:
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE( "
                     "(SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s) + 1),"
                     " (\"Assignment with incompatible types found!\"));\n",
                     to_nt, vala_any[0]);
            break;

        case C_aud:
            /* noop */
            break;

        default:
            DBUG_ASSERT ((0), "Unknown shape class found!");
            break;
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE__VECT__DATA( char *to_nt, int to_sdim,
 *                                         int val_size, char **vala_any,
 *                                         char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE__VEC__DATAT( nt, sdim, val_size, vala_0 ... vala_n, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_CREATE__VECT__DATA (char *to_nt, int to_sdim, int val_size, char **vala_any,
                                 char *copyfun)
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
     * 'vala_any[i]' is either a tagged identifier or a constant scalar!!
     */

    entries_are_scalars = FALSE;
    for (i = 0; i < val_size; i++) {
        if ((vala_any[i][0] != '(') ||
            /* not a tagged id -> is a constant scalar! */
            (ICUGetShapeClass (vala_any[i]) == C_scl)) {
            entries_are_scalars = TRUE;
        }
    }

    if (entries_are_scalars) {
        for (i = 0; i < val_size; i++) {
            INDENT;
            fprintf (outfile, "SAC_ND_WRITE_COPY( %s, %d, ", to_nt, i);
            ReadScalar_Check (vala_any[i], NULL, 0);
            fprintf (outfile, ", %s)\n", copyfun);
        }
    } else {
        if (val_size > 0) {
            INDENT;
            fprintf (outfile, "{ int SAC_i = 0; int SAC_j;\n");
            indent++;
            for (i = 0; i < val_size; i++) {
                /* check whether all entries have identical size */
                if (i > 0) {
                    INDENT;
                    fprintf (outfile,
                             "SAC_ASSURE_TYPE("
                             " (SAC_ND_A_SIZE( %s) == SAC_ND_A_SIZE( %s)),"
                             " (\"Inconsistent vector found:"
                             " First entry and entry at position %d have"
                             " different sizes!\"));\n",
                             vala_any[i], vala_any[0], i);
                }

                /* assign values of entry */
                INDENT;
                fprintf (outfile,
                         "for (SAC_j = 0; SAC_j < SAC_ND_A_SIZE( %s);"
                         " SAC_i++, SAC_j++) {\n",
                         vala_any[i]);
                indent++;
                INDENT;
                fprintf (outfile, "SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_j, %s)\n",
                         to_nt, vala_any[i], copyfun);
                indent--;
                INDENT;
                fprintf (outfile, "}\n");
            }
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE( (SAC_i == SAC_ND_A_SIZE( %s)),"
                     " (\"Assignment with incompatible types found!\"));\n",
                     to_nt);
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
    shape_class_t from_sc = ICUGetShapeClass (from_nt);
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("ICMCompileND_PRF_SHAPE__DATA");

#define ND_PRF_SHAPE__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SHAPE__DATA

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SHAPE__DATA( %s, %d, %s, %d)\"))\n",
             to_nt, to_sdim, from_nt, from_sdim);

    switch (from_sc) {
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
            fprintf (outfile, "SAC_ND_WRITE( %s, %d) = SAC_ND_A_SHAPE( %s, %d);\n", to_nt,
                     i, from_nt, i);
        }
        break;

    case C_aud:
        INDENT;
        fprintf (outfile, "{ int SAC_i;\n");
        indent++;
        INDENT;
        fprintf (outfile,
                 "for ( SAC_i = 0; SAC_i < SAC_ND_A_DIM( %s);"
                 " SAC_i++) {\n",
                 from_nt);
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_ND_WRITE( %s, SAC_i) = "
                 "SAC_ND_A_SHAPE( %s, SAC_i);\n",
                 to_nt, from_nt);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void PrfReshape_Shape( char *to_nt, int to_sdim,
 *                          void *shp, int shp_size,
 *                          void (*shp_size_fun)( void *),
 *                          void (*shp_read_fun)( void *, char *, int))
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
PrfReshape_Shape (char *to_nt, int to_sdim, void *shp, int shp_size,
                  void (*shp_size_fun) (void *),
                  void (*shp_read_fun) (void *, char *, int))
{
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_nt);
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("PrfReshape_Shape");

    /*
     * set descriptor and non-constant part of mirror
     */
    switch (to_sc) {
    case C_aud:
        /*
         * ND_A_DESC_DIM, ND_A_DIM have already been set by ND_ALLOC__DESC!
         */
#if 0
      INDENT;
      fprintf( outfile, "SAC_ND_A_DESC_DIM( %s) = SAC_ND_A_DIM( %s) = ",
                        to_nt, to_nt);
      if (shp_size < 0) {
        shp_size_fun( shp);
      }
      else {
        fprintf( outfile, "%d", shp_size);
      }
      fprintf( outfile, ";\n");
#else
        INDENT;
        fprintf (outfile, "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == ", to_nt);
        if (shp_size < 0) {
            shp_size_fun (shp);
        } else {
            fprintf (outfile, "%d", shp_size);
        }
        fprintf (outfile, "),"
                          " (\"Assignment with incompatible types found!\"));\n");
#endif
        INDENT;
        fprintf (outfile, "{ int SAC_size = 1;\n");
        indent++;
        INDENT;
        fprintf (outfile,
                 "for (SAC_i = 0; SAC_i < SAC_ND_A_DIM( %s);"
                 " SAC_i++) {\n",
                 to_nt);
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_size *= SAC_ND_A_DESC_SHAPE( %s, SAC_i)"
                 " = SAC_ND_A_SHAPE( %s, SAC_i) = ",
                 to_nt, to_nt);
        shp_read_fun (shp, "SAC_i", -1);
        fprintf (outfile, ";\n");
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_DESC_SIZE( %s)"
                 " = SAC_ND_A_SIZE( %s) = SAC_size;\n",
                 to_nt, to_nt);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        break;

    case C_akd:
        INDENT;
        fprintf (outfile, "{ int SAC_size = 1;\n");
        indent++;
        DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
        for (i = 1; i < to_dim; i++) {
            INDENT;
            fprintf (outfile,
                     "SAC_size *= SAC_ND_A_DESC_SHAPE( %s, %d)"
                     " = SAC_ND_A_SHAPE( %s, %d) = ",
                     to_nt, i, to_nt, i);
            shp_read_fun (shp, NULL, i);
            fprintf (outfile, ";\n");
        }
        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_DESC_SIZE( %s)"
                 " = SAC_ND_A_SIZE( %s) = SAC_size;\n",
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

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    /*
     * check constant parts of mirror
     */
    switch (to_sc) {
    case C_scl:
        DBUG_ASSERT ((0), "illegal dimension found!");
        break;

    case C_aks:
        DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
        for (i = 0; i < to_dim; i++) {
            INDENT;
            fprintf (outfile, "SAC_ASSURE_TYPE( (SAC_ND_A_SHAPE( %s, %d) == ", to_nt, i);
            shp_read_fun (shp, NULL, i);
            fprintf (outfile, "), (\"Assignment with incompatible types"
                              " found!\"));\n");
        }
        /* here is no break missing */

    case C_akd:
        INDENT;
        fprintf (outfile, "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == ", to_nt);
        if (shp_size < 0) {
            shp_size_fun (shp);
        } else {
            fprintf (outfile, "%d", shp_size);
        }
        fprintf (outfile, "),"
                          " (\"Assignment with incompatible types found!\"));\n");
        break;

    case C_aud:
        /* noop */
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
 *   void ICMCompileND_PRF_RESHAPE__SHAPE_id( char *to_nt, int to_sdim,
 *                                            char *shp_nt)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_RESHAPE__SHAPE_id( to_nt, to_sdim, shp_nt)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_RESHAPE__SHAPE_id (char *to_nt, int to_sdim, char *shp_nt)
{
    DBUG_ENTER ("ICMCompileND_PRF_RESHAPE__SHAPE_id");

#define ND_PRF_RESHAPE__SHAPE_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_RESHAPE__SHAPE_id

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_RESHAPE__SHAPE( %s, %d, ...)\"))\n",
             to_nt, to_sdim);

    INDENT;
    fprintf (outfile,
             "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == 1),"
             " (\"1st argument of F_reshape has (dim != 1)!\"));\n",
             shp_nt);

    PrfReshape_Shape (to_nt, to_sdim, shp_nt, -1, SizeId, ReadId);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_RESHAPE__SHAPE_arr( char *to_nt, int to_sdim,
 *                                             int shp_size, char **shpa_any)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_RESHAPE__SHAPE_arr( to_nt, to_sdim, shp_size, shpa_any)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_RESHAPE__SHAPE_arr (char *to_nt, int to_sdim, int shp_size,
                                     char **shpa_any)
{
    int i;

    DBUG_ENTER ("ICMCompileND_PRF_RESHAPE__SHAPE_arr");

#define ND_PRF_RESHAPE__SHAPE_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_RESHAPE__SHAPE_arr

    /*
     * CAUTION:
     * 'shpa_any[i]' is either a tagged identifier or a constant scalar!!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_RESHAPE__SHAPE( %s, %d, ...)\"))\n",
             to_nt, to_sdim);

    for (i = 0; i < shp_size; i++) {
        if (shpa_any[i][0] == '(') {
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == 0),"
                     " (\"1st argument of F_reshape has (dim != 1)!\"))\n",
                     shpa_any[i]);
        }
    }

    PrfReshape_Shape (to_nt, to_sdim, shpa_any, shp_size, NULL, ReadConstArray);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_SEL__SHAPE_id( char *to_nt, int to_sdim,
 *                                        char *from_nt, int from_sdim,
 *                                        int idx_size, char *idx_nt)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL__SHAPE_id( to_nt, to_sdim, from_nt, from_sdim, idx_size, idx_nt)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL__SHAPE_id (char *to_nt, int to_sdim, char *from_nt, int from_sdim,
                                int idx_size, char *idx_nt)
{
    shape_class_t to_sc = ICUGetShapeClass (to_nt);

    DBUG_ENTER ("ICMCompileND_PRF_SEL__SHAPE_id");

#define ND_PRF_SEL__SHAPE_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL__SHAPE_id

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__SHAPE( %s, %d, %s, %d, ...)\"))\n",
             to_nt, to_sdim, from_nt, from_sdim);

    switch (to_sc) {
    case C_aud:
        /*
         * for the time being implemented for scalar results only!!!
         */
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == %d),"
                 " (\"Result of F_sel has (dim != 0)!\"))\n",
                 from_nt, idx_size);
        ICMCompileND_SET__SHAPE (to_nt, 0, NULL);
        break;

    case C_akd:
        /* here is no break missing */
    case C_aks:
        DBUG_ASSERT ((0), "not yet implemented");
        break;

    case C_scl:
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_SEL__SHAPE_arr( char *to_nt, int to_sdim,
 *                                         char *from_nt, int from_sdim,
 *                                         int idx_size, char **idxa_any)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL__SHAPE_arr( to_nt, to_sdim, from_nt, from_sdim,
 *                          idx_size, ...idxa_any...)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL__SHAPE_arr (char *to_nt, int to_sdim, char *from_nt, int from_sdim,
                                 int idx_size, char **idxa_any)
{
    shape_class_t to_sc = ICUGetShapeClass (to_nt);

    DBUG_ENTER ("ICMCompileND_PRF_SEL__SHAPE_arr");

#define ND_PRF_SEL__SHAPE_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL__SHAPE_arr

    /*
     * CAUTION:
     * 'idxa_any[i]' is either a tagged identifier or a constant scalar!!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__SHAPE( %s, %d, %s, %d, ...)\"))\n",
             to_nt, to_sdim, from_nt, from_sdim);

    switch (to_sc) {
    case C_aud:
        /*
         * for the time being implemented for scalar results only!!!
         */
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == %d),"
                 " (\"Result of F_sel has (dim != 0)!\"))\n",
                 from_nt, idx_size);
        ICMCompileND_SET__SHAPE (to_nt, 0, NULL);
        break;

    case C_akd:
        /* here is no break missing */
    case C_aks:
        DBUG_ASSERT ((0), "not yet implemented");
        break;

    case C_scl:
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void PrfSel_Data( char *to_nt, int to_sdim,
 *                     char *from_nt, int from_sdim,
 *                     void *idx, int idx_size,
 *                     void (*idx_size_fun)( void *),
 *                     void (*idx_read_fun)( void *, char *, int),
 *                     char *copyfun)
 *
 * Description:
 *   implements all the ND_PRF_..._SEL__DATA_... ICMs.
 *
 ******************************************************************************/

static void
PrfSel_Data (char *to_nt, int to_sdim, char *from_nt, int from_sdim, void *idx,
             int idx_size, void (*idx_size_fun) (void *),
             void (*idx_read_fun) (void *, char *, int), char *copyfun)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("PrfSel_Data");

    if (to_dim == 0) {
        INDENT;
        fprintf (outfile, "{ int SAC_idx;\n");
        indent++;
        VectToOffset ("SAC_idx", idx, idx_size, idx_size_fun, idx_read_fun, from_nt,
                      from_dim);
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE_READ_COPY( %s, 0, %s, SAC_idx, %s)\n", to_nt,
                 from_nt, copyfun);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    } else {
        INDENT;
        fprintf (outfile, "{ int SAC_idx;\n");
        indent++;
        INDENT;
        fprintf (outfile, "int SAC_i;\n");
        VectToOffset ("SAC_idx", idx, idx_size, idx_size_fun, idx_read_fun, from_nt,
                      from_dim);
        INDENT;
        fprintf (outfile,
                 "for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE( %s);"
                 " SAC_i++, SAC_idx++) {\n",
                 to_nt);
        indent++;
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_idx, %s)\n", to_nt,
                 from_nt, copyfun);
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
 * Function:
 *   void ICMCompileND_PRF_SEL__DATA_id( char *to_nt, int to_sdim,
 *                                       char *from_nt, int from_sdim,
 *                                       int idx_size, char *idx_nt,
 *                                       char *copyfun)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL__DATA_id( to_nt, to_sdim, from_nt, from_sdim, idx_size, idx_nt,
 *                        copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL__DATA_id (char *to_nt, int to_sdim, char *from_nt, int from_sdim,
                               int idx_size, char *idx_nt, char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_PRF_SEL__DATA_id");

#define ND_PRF_SEL__DATA_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL__DATA_id

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__DATA( %s, %d, %s, %d, ...)\"))\n",
             to_nt, to_sdim, from_nt, from_sdim);

    INDENT;
    fprintf (outfile,
             "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == 0),"
             " (\"1st argument of F_sel has (dim != 1)!\"))\n",
             idx_nt);
    INDENT;
    fprintf (outfile,
             "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) >= %d),"
             " (\"1st argument of F_sel has illegal size!\"));\n",
             from_nt, idx_size);

    PrfSel_Data (to_nt, to_sdim, from_nt, from_sdim, idx_nt, idx_size, SizeId, ReadId,
                 copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_SEL__DATA_arr( char *to_nt, int to_sdim,
 *                                        char *from_nt, int from_sdim,
 *                                        int idx_size, char **idxa_any,
 *                                        char *copyfun)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL__DATA_arr( to_nt, to_sdim, from_nt, from_sdim,
 *                         idx_size, ...idxa_any..., copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL__DATA_arr (char *to_nt, int to_sdim, char *from_nt, int from_sdim,
                                int idx_size, char **idxa_any, char *copyfun)
{
    int i;

    DBUG_ENTER ("ICMCompileND_PRF_SEL__DATA_arr");

#define ND_PRF_SEL__DATA_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL__DATA_arr

    /*
     * CAUTION:
     * 'idxa_any[i]' is either a tagged identifier or a constant scalar!!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__DATA( %s, %d, %s, %d, ...)\"))\n",
             to_nt, to_sdim, from_nt, from_sdim);

    for (i = 0; i < idx_size; i++) {
        if (idxa_any[i][0] == '(') {
            INDENT;
            fprintf (outfile,
                     "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == 0),"
                     " (\"1st argument of F_sel has (dim != 1)!\"))\n",
                     idxa_any[i]);
        }
    }
    INDENT;
    fprintf (outfile,
             "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) >= %d),"
             " (\"1st argument of F_sel has illegal size!\"));\n",
             from_nt, idx_size);

    PrfSel_Data (to_nt, to_sdim, from_nt, from_sdim, idxa_any, idx_size, NULL,
                 ReadConstArray, copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void PrfModarray_Data( char *to_nt, int to_sdim,
 *                          char *from_nt, int from_sdim,
 *                          bool idx_unrolled, void *idx, int idx_size,
 *                          void (*idx_size_fun)( void *),
 *                          void (*idx_read_fun)( void *, char *, int),
 *                          char *val_any,
 *                          char *copyfun)
 *
 * Description:
 *   implements all the ND_PRF_..._MODARRAY__DATA_... ICMs.
 *
 ******************************************************************************/

static void
PrfModarray_Data (char *to_nt, int to_sdim, char *from_nt, int from_sdim,
                  bool idx_unrolled, void *idx, int idx_size,
                  void (*idx_size_fun) (void *),
                  void (*idx_read_fun) (void *, char *, int), char *val_any,
                  char *copyfun)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("PrfModarray_Data");

    /*
     * CAUTION:
     * 'val_any' is either a tagged identifier or a constant scalar!!
     */

    if ((val_any[0] != '(') ||
        /* not a tagged id -> is a constant scalar! */
        (ICUGetShapeClass (val_any) == C_scl)) {
        /*
         * 'val_any' is scalar
         */
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
        fprintf (outfile, "SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_i, %s)\n", to_nt,
                 from_nt, copyfun);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        indent--;
        INDENT;
        fprintf (outfile, "} else {\n");
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_TR_MEM_PRINT( (\"reuse memory of %s at %%p for %s\","
                 " ND_A_FIELD( %s)))\n",
                 from_nt, to_nt, from_nt);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");

        INDENT;
        fprintf (outfile, "{ int SAC_idx;\n");
        indent++;
        if (idx_unrolled) {
            INDENT;
            fprintf (outfile, "SAC_idx = ");
            idx_read_fun (idx, NULL, 0);
            fprintf (outfile, ";\n");
        } else {
            VectToOffset ("SAC_idx", idx, idx_size, idx_size_fun, idx_read_fun, to_nt,
                          to_dim);
        }
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE_COPY( %s, SAC_idx, ", to_nt);
        ReadScalar (val_any, NULL, 0);
        fprintf (outfile, " , %s)\n", copyfun);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    } else {
        /*
         * 'val_any' is array
         */
        INDENT;
        fprintf (outfile, "{ int SAC_i, SAC_j, SAC_idx;\n");
        indent++;
        if (idx_unrolled) {
            INDENT;
            fprintf (outfile, "SAC_idx = ");
            idx_read_fun (idx, NULL, 0);
            fprintf (outfile, ";\n");
        } else {
            VectToOffset ("SAC_idx", idx, idx_size, idx_size_fun, idx_read_fun, to_nt,
                          to_dim);
        }
        INDENT;
        fprintf (outfile, "if (SAC_ND_A_FIELD( %s) != SAC_ND_A_FIELD( %s)) {\n", to_nt,
                 from_nt);
        indent++;
        INDENT;
        fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_idx; SAC_i++) {\n");
        indent++;
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_i, %s)\n", to_nt,
                 from_nt, copyfun);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        INDENT;
        fprintf (outfile,
                 "for (SAC_j = 0; SAC_j < SAC_ND_A_SIZE( %s);"
                 " SAC_i++, SAC_j++) {\n",
                 val_any);
        indent++;
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_j, %s)\n", to_nt,
                 val_any, copyfun);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        INDENT;
        fprintf (outfile, "for (; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++) {\n", to_nt);
        indent++;
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_i, %s)\n", to_nt,
                 from_nt, copyfun);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        indent--;
        INDENT;
        fprintf (outfile, "} else {\n");
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_TR_MEM_PRINT( (\"reuse memory of %s at %%p for %s\","
                 " ND_A_FIELD( %s)))\n",
                 from_nt, to_nt, from_nt);
        INDENT;
        fprintf (outfile,
                 "for (SAC_i = SAC_idx, SAC_j = 0;"
                 " SAC_j < SAC_ND_A_SIZE( %s); SAC_i++, SAC_j++) {\n",
                 val_any);
        indent++;
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_j, %s)\n", to_nt,
                 val_any, copyfun);
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
 *   void ICMCompileND_PRF_MODARRAY__DATA_id( char *to_nt, int to_sdim,
 *                                            char *from_nt, int from_sdim,
 *                                            int idx_size, char *idx_nt,
 *                                            char *val_any,
 *                                            char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY__DATA_id( to_nt, to_sdim, from_nt, from_sdim,
 *                             idx_size, idx_nt, val_any, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY__DATA_id (char *to_nt, int to_sdim, char *from_nt,
                                    int from_sdim, int idx_size, char *idx_nt,
                                    char *val_any, char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY__DATA_id");

#define ND_PRF_MODARRAY__DATA_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY__DATA_id

    /*
     * CAUTION:
     * 'val_any' is either a tagged identifier or a constant scalar!!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_MODARRAY__DATA( %s, %d, %s, %d, ..., %s)\"))\n",
             to_nt, to_sdim, from_nt, from_sdim, val_any);

    INDENT;
    fprintf (outfile,
             "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == 1),"
             " (\"2nd argument of F_modarray has (dim != 1)!\"));\n",
             idx_nt);
    INDENT;
    fprintf (outfile,
             "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) >= %d),"
             " (\"2nd argument of F_modarray has illegal size!\"));\n",
             from_nt, idx_size);

    PrfModarray_Data (to_nt, to_sdim, from_nt, from_sdim, FALSE, idx_nt, idx_size, SizeId,
                      ReadId, val_any, copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY__DATA_arr( char *to_nt, int to_sdim,
 *                                             char *from_nt, int from_sdim,
 *                                             int idx_size, char **idxa_any,
 *                                             char *val_any,
 *                                             char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY__DATA_arr( to_nt, to_sdim, from_nt, from_sdim,
 *                              idx_size, ...idxa_any..., val_any, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY__DATA_arr (char *to_nt, int to_sdim, char *from_nt,
                                     int from_sdim, int idx_size, char **idxa_any,
                                     char *val_any, char *copyfun)
{
    int i;

    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY__DATA_arr");

#define ND_PRF_MODARRAY__DATA_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY__DATA_arr

    /*
     * CAUTION:
     * 'idxa_any[i]', 'val_any' are either tagged identifiers or constant
     * scalars!!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_MODARRAY__DATA( %s, %d, %s, %d, ..., %s)\"))\n",
             to_nt, to_sdim, from_nt, from_sdim, val_any);

    for (i = 0; i < idx_size; i++) {
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
             " (\"2nd argument of F_modarray has illegal size!\"));\n",
             from_nt, idx_size);

    PrfModarray_Data (to_nt, to_sdim, from_nt, from_sdim, FALSE, idxa_any, idx_size, NULL,
                      ReadConstArray, val_any, copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_IDX_SEL__SHAPE( char *to_nt, int to_sdim,
 *                                         char *from_nt, int from_sdim,
 *                                         char *idx_any)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_IDX_SEL__SHAPE( to_nt, to_sdim, from_nt, from_sdim, idx_any)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_IDX_SEL__SHAPE (char *to_nt, int to_sdim, char *from_nt, int from_sdim,
                                 char *idx_any)
{
    shape_class_t to_sc = ICUGetShapeClass (to_nt);

    DBUG_ENTER ("ICMCompileND_PRF_IDX_SEL__SHAPE");

#define ND_PRF_IDX_SEL__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_IDX_SEL__SHAPE

    /*
     * CAUTION:
     * 'idx_any' is either a tagged identifier or a constant scalar!!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_IDX_SEL__SHAPE( %s, %d, %s, %d, %s)\"))\n",
             to_nt, to_sdim, from_nt, from_sdim, idx_any);

    switch (to_sc) {
    case C_aud:
        DBUG_ASSERT ((0), "idx_sel() with unknown dimension found!");
        break;

    case C_akd:
        /* here is no break missing */
    case C_aks:
        DBUG_ASSERT ((0), "not yet implemented");
        break;

    case C_scl:
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_IDX_SEL__DATA( char *to_nt, int to_sdim,
 *                                        char *from_nt, int from_sdim,
 *                                        char *idx_any,
 *                                        char *copyfun)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_IDX_SEL__DATA( to_nt, to_sdim, from_nt, from_sdim, idx_any)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_IDX_SEL__DATA (char *to_nt, int to_sdim, char *from_nt, int from_sdim,
                                char *idx_any, char *copyfun)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_PRF_IDX_SEL__DATA");

#define ND_PRF_IDX_SEL__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_IDX_SEL__DATA

    /*
     * CAUTION:
     * 'idx_any' is either a tagged identifier or a constant scalar!!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_IDX_SEL__DATA( %s, %d, %s, %d, %s)\"))\n",
             to_nt, to_sdim, from_nt, from_sdim, idx_any);

    if (idx_any[0] == '(') {
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == 0),"
                 " (\"1st argument of F_idx_sel has (dim != 0)!\"));\n",
                 idx_any);
    }

    /*
     * idx_sel() works only for arrays with known dimension!!!
     */
    DBUG_ASSERT ((to_dim >= 0), "idx_sel() with unknown dimension found!");

    if (to_dim == 0) {
        /*
         * 'to_nt' is scalar
         */
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE_READ_COPY( %s, 0, %s, ", to_nt, from_nt);
        ReadScalar (idx_any, NULL, 0);
        fprintf (outfile, ", %s)\n", copyfun);
    } else {
        /*
         * 'to_nt' is array
         */
        INDENT;
        fprintf (outfile, "{ int SAC_i, SAC_j;\n");
        indent++;
        INDENT;
        fprintf (outfile, "for (SAC_i = 0, SAC_j = ");
        ReadScalar (idx_any, NULL, 0);
        fprintf (outfile, "; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++, SAC_j++) {\n", to_nt);
        indent++;
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_j, %s)\n", to_nt,
                 from_nt, copyfun);
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
 *                                             char *idx_any, char *val_any,
 *                                             char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_IDX_MODARRAY__DATA( to_nt, to_sdim, from_nt, from_sdim, idx, val,
 *                              copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_IDX_MODARRAY__DATA (char *to_nt, int to_sdim, char *from_nt,
                                     int from_sdim, char *idx_any, char *val_any,
                                     char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_PRF_IDX_MODARRAY__DATA");

#define ND_PRF_IDX_MODARRAY__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_IDX_MODARRAY__DATA

    /*
     * CAUTION:
     * 'idx_any', 'val_any' are either tagged identifiers or constant scalars!!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_IDX_MODARRAY__DATA( %s, %d, %s, %d, %s, %s)\"))\n",
             to_nt, to_sdim, from_nt, from_sdim, idx_any, val_any);

    if (idx_any[0] == '(') {
        INDENT;
        fprintf (outfile,
                 "SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == 0),"
                 " (\"2nd argument of F_modarray has (dim != 0)!\"));\n",
                 idx_any);
    }

    PrfModarray_Data (to_nt, to_sdim, from_nt, from_sdim, TRUE, idx_any, 1, NULL,
                      ReadScalar, val_any, copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_VECT2OFFSET( char *off_nt, int from_size, char *from_nt,
 *                                  int shp_size, char **shpa_any)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_VECT2OFFSET( off_nt, from_size, from_nt, shp_size, shpa_any)
 *
 ******************************************************************************/

void
ICMCompileND_VECT2OFFSET (char *off_nt, int from_size, char *from_nt, int shp_size,
                          char **shpa_any)
{
    DBUG_ENTER ("ICMCompileND_VECT2OFFSET");

#define ND_VECT2OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_VECT2OFFSET

    /*
     * CAUTION:
     * 'shpa_any[i]' is either a tagged identifier or a constant scalar!!
     */

    DBUG_ASSERT ((from_size >= 0), "Illegal size found!");

    VectToOffset2 (off_nt, from_nt, from_size, NULL, ReadId, shpa_any, shp_size, NULL,
                   ReadConstArray);

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
                                         " SAC_i0++, SAC_idest++, SAC_isrc++)\n");
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

    INDENT;
    fprintf (outfile, "%s = ", off_name);
    VectToOffset2 (dim, AccessVect (arr_name, i), dims, AccessConst (shp_any, i));
    fprintf (outfile, ";\n");

    DBUG_VOID_RETURN;
}

#endif /* TAGGED_ARRAYS */
