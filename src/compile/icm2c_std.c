/*
 *
 * $Log$
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
 * Revision 3.38  2002/10/24 16:03:22  dkr
 * some comments added,
 * bug in ND_ASSIGN__DESC fixed: dimension for scalars is 0 rather than 1
 * now...
 *
 * Revision 3.37  2002/10/10 23:52:09  dkr
 * bugs fixed
 *
 * Revision 3.36  2002/10/07 23:36:10  dkr
 * some bugs with TAGGED_ARRAYS fixed
 *
 * Revision 3.35  2002/09/11 23:18:03  dkr
 * name of GlobalObjInit() function modified
 *
 * Revision 3.34  2002/09/06 09:57:12  dkr
 * ND_IDXS2OFFSET added
 *
 * Revision 3.33  2002/09/06 09:37:11  dkr
 * ND_IDXS2OFFSET
 *
 * Revision 3.32  2002/08/05 20:50:12  dkr
 * DBUG_ASSERT messages modified
 *
 * Revision 3.31  2002/08/05 20:42:44  dkr
 * some bugs fixed
 *
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
 *   void ICMCompileND_FUN_DEC( char *name, char *rettype_NT,
 *                              int narg, char **arg_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEC( name, rettype_NT, narg, [ TAG, basetype, arg_NT ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_DEC (char *name, char *rettype_NT, int narg, char **arg_ANY)
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
        ScanArglist (narg, 3, ",", ,
                     fprintf (outfile, " SAC_ND_PARAM_%s( %s, %s)", arg_ANY[i],
                              arg_ANY[i + 2], arg_ANY[i + 1]));
#else
        ScanArglist (narg, 3, ",", ,
                     fprintf (outfile, " SAC_ND_PARAM_%s( %s, %s)", arg_ANY[i],
                              arg_ANY[i + 1], arg_ANY[i + 2]));
#endif
        fprintf (outfile, ")");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_AP( char *name, char *retname,
 *                             int narg, char **arg_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEC( name, retname, narg, [ TAG, arg_NT ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_AP (char *name, char *retname, int narg, char **arg_ANY)
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
                     fprintf (outfile, " SAC_ND_ARG_%s( %s)", arg_ANY[i],
                              arg_ANY[i + 1]));
        fprintf (outfile, ");");
    }
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_RET( char *retname, int narg, char **arg_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_RET( retname, narg, [ TAG, arg_NT, decl_arg_NT ]* )
 *
 *   where TAG is element in { out, inout }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_RET (char *retname, int narg, char **arg_ANY)
{
    DBUG_ENTER ("ICMCompileND_FUN_RET");

#define ND_FUN_RET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_RET

    INDENT;
    ScanArglist (narg, 3, "\n", INDENT,
                 fprintf (outfile, "SAC_ND_RET_%s( %s, %s)", arg_ANY[i], arg_ANY[i + 1],
                          arg_ANY[i + 2]));
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
 *   ND_OBJDEF( nt, basetype, sdim, [ shp ]* )
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
 *   ND_DECL( nt, basetype, sdim, [ shp ]* )
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
 *   ND_DECL__MIRROR( nt, sdim, [ shp ]* )
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
            fprintf (outfile, "const int SAC_ND_A_MIRROR_SHAPE( %s, %d) = %d;\n", nt, i,
                     shp[i]);
            size *= shp[i];
        }
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_MIRROR_SIZE( %s) = %d;\n", nt, size);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", nt, dim);
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "int SAC_ND_A_MIRROR_SHAPE( %s, %d);\n", nt, i);
        }
        INDENT;
        fprintf (outfile, "int SAC_ND_A_MIRROR_SIZE( %s);\n", nt);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", nt, dim);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile, "int SAC_ND_A_MIRROR_SIZE( %s);\n", nt);
        INDENT;
        fprintf (outfile, "int SAC_ND_A_MIRROR_DIM( %s);\n", nt);
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
 *   ND_DECL__MIRROR_PARAM( nt, sdim, [ shp ]* )
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
            fprintf (outfile, "const int SAC_ND_A_MIRROR_SHAPE( %s, %d) = %d;\n", nt, i,
                     shp[i]);
            size *= shp[i];
        }
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_MIRROR_SIZE( %s) = %d;\n", nt, size);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", nt, dim);
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile,
                     "int SAC_ND_A_MIRROR_SHAPE( %s, %d) "
                     "= SAC_ND_A_DESC_SHAPE( %s, %d);\n",
                     nt, i, nt, i);
        }
        INDENT;
        fprintf (outfile,
                 "int SAC_ND_A_MIRROR_SIZE( %s)"
                 " = SAC_ND_A_DESC_SIZE( %s);\n",
                 nt, nt);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", nt, dim);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile,
                 "int SAC_ND_A_MIRROR_SIZE( %s)"
                 " = SAC_ND_A_DESC_SIZE( %s);\n",
                 nt, nt);
        INDENT;
        fprintf (outfile,
                 "int SAC_ND_A_MIRROR_DIM( %s)"
                 " = SAC_ND_A_DESC_DIM( %s);\n",
                 nt, nt);
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
                     "const int SAC_ND_A_MIRROR_SHAPE( %s, %d);\n",
                     nt, i);
        }
        INDENT;
        fprintf (outfile,
                 "extern "
                 "const int SAC_ND_A_MIRROR_SIZE( %s);\n",
                 nt);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "const int SAC_ND_A_MIRROR_DIM( %s);\n",
                 nt);
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile,
                     "extern "
                     "int SAC_ND_A_MIRROR_SHAPE( %s, %d);\n",
                     nt, i);
        }
        INDENT;
        fprintf (outfile,
                 "extern "
                 "int SAC_ND_A_MIRROR_SIZE( %s);\n",
                 nt);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "const int SAC_ND_A_MIRROR_DIM( %s);\n",
                 nt);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile,
                 "extern "
                 "int SAC_ND_A_MIRROR_SIZE( %s);\n",
                 nt);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "int SAC_ND_A_MIRROR_DIM( %s);\n",
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
    unique_class_t to_uc = ICUGetUniqueClass (to_NT);

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
 *   void ICMCompileND_SET__SHAPE( char *to_NT, int dim, char **shp_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_SET__SHAPE( to_NT, dim, [shp]* )
 *
 ******************************************************************************/

void
ICMCompileND_SET__SHAPE (char *to_NT, int dim, char **shp_ANY)
{
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_NT);

    DBUG_ENTER ("ICMCompileND_SET__SHAPE");

#define ND_SET__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_SET__SHAPE

    /*
     * CAUTION:
     * 'shp_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    /*
     * set descriptor and non-constant part of mirror
     */
    switch (to_sc) {
    case C_aud:
        /*
         * ND_A_DESC_DIM, ND_A_MIRROR_DIM have already been set by ND_ALLOC__DESC!
         */
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == %d", to_NT, dim);
                         ,
                         fprintf (outfile, "Assignment with incompatible types found!"););
        BLOCK_VARDECS (fprintf (outfile, "int SAC_size = 1;");
                       , SET_SHAPES_AUD__NUM (to_NT, i, 0, dim, INDENT;
                                              fprintf (outfile, "SAC_size *= \n");
                                              , ReadScalar (shp_ANY[i], NULL, 0););

                       SET_SIZE (to_NT, fprintf (outfile, "SAC_size");););
        break;

    case C_akd:
        BLOCK_VARDECS (fprintf (outfile, "int SAC_size = 1;");
                       , SET_SHAPES_AKD (to_NT, i, 0, dim, INDENT;
                                         fprintf (outfile, "SAC_size *= \n");
                                         , ReadScalar (shp_ANY[i], NULL, 0););

                       SET_SIZE (to_NT, fprintf (outfile, "SAC_size");););
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
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d) == ", to_NT, i);
                             ReadScalar (shp_ANY[i], NULL, 0);
                             , fprintf (outfile,
                                        "Assignment with incompatible types found!"););
        }
        /* here is no break missing */

    case C_akd:
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == %d", to_NT, dim);
                         ,
                         fprintf (outfile, "Assignment with incompatible types found!"););
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
                     "SAC_ND_A_MIRROR_SHAPE( %s, %d) "
                     "= SAC_ND_A_DESC_SHAPE( %s, %d);\n",
                     nt, i, nt, i);
        }
        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_MIRROR_SIZE( %s)"
                 " = SAC_ND_A_DESC_SIZE( %s);\n",
                 nt, nt);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_MIRROR_SIZE( %s)"
                 " = SAC_ND_A_DESC_SIZE( %s);\n",
                 nt, nt);
        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_MIRROR_DIM( %s)"
                 " = SAC_ND_A_DESC_DIM( %s);\n",
                 nt, nt);
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
 *   void ICMCompileND_CHECK_MIRROR( char *to_NT, int to_sdim,
 *                                   char *from_NT, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CHECK_MIRROR( to_NT, to_sdim, from_NT, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_CHECK_MIRROR (char *to_NT, int to_sdim, char *from_NT, int from_sdim)
{
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    shape_class_t from_sc = ICUGetShapeClass (from_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_CHECK_MIRROR");

#define ND_CHECK_MIRROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CHECK_MIRROR

#if 0
  if ((runtimecheck & RUNTIMECHECK_TYPE) && (from_sc == C_aud)) {
    INDENT;
    fprintf( outfile, "if (SAC_ND_A_DIM( %s) > 0) {\n", from_NT);
    indent++;
  }
#endif

    /*
     * check constant parts of mirror
     */
    switch (to_sc) {
    case C_scl:
        /* check dimension/size */
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", from_NT);
                         ,
                         fprintf (outfile, "Assignment with incompatible types found!"););
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_SIZE( %s) == 1", from_NT);
                         ,
                         fprintf (outfile, "Assignment with incompatible types found!"););
        break;

    case C_aks:
        switch (from_sc) {
        case C_aud:
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s)",
                                      to_NT, from_NT);
                             , fprintf (outfile,
                                        "Assignment with incompatible types found!"););
            /* here is no break missing! */
        case C_akd:
            DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
            for (i = 0; i < to_dim; i++) {
                ASSURE_TYPE_ASS (fprintf (outfile,
                                          "SAC_ND_A_SHAPE( %s, %d) == SAC_ND_A_SHAPE( "
                                          "%s, %d)",
                                          to_NT, i, from_NT, i);
                                 ,
                                 fprintf (outfile,
                                          "Assignment with incompatible types found!"););
            }
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_SIZE( %s) == SAC_ND_A_SIZE( %s)",
                                      to_NT, from_NT);
                             , fprintf (outfile,
                                        "Assignment with incompatible types found!"););
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
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s)",
                                      to_NT, from_NT);
                             , fprintf (outfile,
                                        "Assignment with incompatible types found!"););
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

#if 0
  if ((runtimecheck & RUNTIMECHECK_TYPE) && (from_sc == C_aud)) {
    indent--;
    INDENT;
    fprintf( outfile, "}\n");
  }
#endif

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

    ICMCompileND_ASSIGN__DESC (to_NT, from_NT);

    ICMCompileND_ASSIGN__DIMSHP (to_NT, to_sdim, from_NT, from_sdim);

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
 *   ND_ASSIGN__DESC( to_NT, to_sdim, from_NT, from_sdim)
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

    DBUG_ENTER ("ICMCompileND_ASSIGN__DESC");

#define ND_ASSIGN__DESC
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN__DESC

    DBUG_ASSERT ((to_hc == from_hc), "Illegal assignment found!");

    if (((to_sc == C_scl) && ((to_hc == C_nhd) || (to_uc == C_unq)))
        && ((from_sc == C_scl) && ((from_hc == C_nhd) || (from_uc == C_unq)))) {
        /* 'to_NT' has no desc, 'from_NT' has no desc */
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
    } else if (((to_sc == C_scl) && ((to_hc == C_nhd) || (to_uc == C_unq)))
               && ((from_sc == C_aud) || ((from_hc == C_hid) && (from_uc == C_nuq)))) {
        /* 'to_NT' has no desc, 'from_NT' has a desc */
        INDENT;
        if (to_hc == C_hid) {
            DBUG_ASSERT ((to_uc == C_unq), "Illegal assignment found!");
            /*
             * data vector of 'from_NT' is reused by 'to_NT'
             * (and 'to_NT' is unique, i.e. RC of 'from_NT' is 1)
             *   -> ND_FREE__DESC( from_NT)
             */
            fprintf (outfile, "SAC_ND_FREE__DESC( %s)\n", from_NT);
        } else {
            /*
             * data vector of 'from_NT' is not reused by 'to_NT'
             *   -> ND_DEC_RC_FREE( from_NT) in ND_ASSIGN__DATA
             */
        }
    } else if (((to_sc == C_aud) || ((to_hc == C_hid) && (to_uc == C_nuq)))
               && ((from_sc == C_scl) && ((from_hc == C_nhd) || (from_uc == C_unq)))) {
        /* 'to_NT' has a desc, 'from_NT' has no desc */
        INDENT;
        fprintf (outfile, "SAC_ND_ALLOC__DESC( %s, 0)\n", to_NT);
        INDENT;
        fprintf (outfile, "SAC_ND_SET__RC( %s, 1)\n", to_NT);
    } else if (((to_sc == C_aks) || (to_sc == C_akd) || (to_sc == C_aud))
               && ((from_sc == C_aks) || (from_sc == C_akd) || (from_sc == C_aud))) {
        /* 'to_NT' has a desc, 'from_NT' has a desc, no scalar involved */
        INDENT;
        fprintf (outfile, "SAC_ND_A_DESC( %s) = SAC_ND_A_DESC( %s);\n", to_NT, from_NT);
    } else if (((to_sc == C_aud) || ((to_hc == C_hid) && (to_uc == C_nuq)))
               && ((from_sc == C_aud) || ((from_hc == C_hid) && (from_uc == C_nuq)))) {
        /* 'to_NT' has a desc, 'from_NT' has a desc, scalar involved */
        if ((to_sc == C_aud) || ((from_sc == C_aud) && (from_uc == C_nuq))) {
            /*
             * 'from_NT' is a non-unique hidden and cannot be reused by 'to_NT'
             *   -> ND_ALLOC__DESC( to_NT, 0)
             *   -> ND_DEC_RC_FREE( from_NT) in ND_ASSIGN__DATA
             */
            INDENT;
            fprintf (outfile, "SAC_ND_ALLOC__DESC( %s, 0)\n", to_NT);
        } else {
            INDENT;
            fprintf (outfile, "SAC_ND_A_DESC( %s) = SAC_ND_A_DESC( %s);\n", to_NT,
                     from_NT);
        }
    } else {
        DBUG_ASSERT ((0), "Illegal assignment found!");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ASSIGN__DIMSHP( char *to_NT, int to_sdim,
 *                                     char *from_NT, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN__DIMSHP( to_NT, to_sdim, from_NT, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN__DIMSHP (char *to_NT, int to_sdim, char *from_NT, int from_sdim)
{
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    shape_class_t from_sc = ICUGetShapeClass (from_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("ICMCompileND_ASSIGN__DIMSHP");

#define ND_ASSIGN__DIMSHP
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN__DIMSHP

    ICMCompileND_CHECK_MIRROR (to_NT, to_sdim, from_NT, from_sdim);

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
 *   ND_COPY( to_NT, to_sdim, from_NT, copyfun)
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

    /* copy descriptor entries and mirror */
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
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("ICMCompileND_COPY__SHAPE");

#define ND_COPY__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_COPY__SHAPE

    ICMCompileND_CHECK_MIRROR (to_NT, to_sdim, from_NT, from_sdim);

    /*
     * copy descriptor entries and mirror
     */
    switch (to_sc) {
    case C_aud:
        /*
         * ND_A_DESC_DIM, ND_A_MIRROR_DIM have already been set by ND_ALLOC__DESC!
         */
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s)",
                                  to_NT, from_NT);
                         ,
                         fprintf (outfile, "Assignment with incompatible types found!"););
        /* here is no break missing */
    case C_akd:
        SET_SIZE (to_NT, fprintf (outfile, "SAC_ND_A_SIZE( %s)", from_NT););
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

    switch (to_sc) {
    case C_aud:
        SET_SHAPES_AUD__XXX (to_NT, i, fprintf (outfile, "SAC_i");
                             , 0, fprintf (outfile, "0");
                             , from_dim, fprintf (outfile, "SAC_ND_A_DIM( %s)", from_NT);
                             ,
                             /* noop */
                             , fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)", from_NT, i);
                             , fprintf (outfile, "SAC_ND_A_SHAPE( %s, SAC_i)", from_NT););
        break;

    case C_akd:
        SET_SHAPES_AKD (to_NT, i, 0, to_dim, ,
                        fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)", from_NT, i););
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
 *   void ICMCompileND_CREATE__ARRAY__DIM( int dim,
 *                                         int val_size, char **vala_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE__ARRAY__DIM( dim, val_size, [ vala_ANY ]* )
 *
 *   dim: top-level dimension
 *           [a,b,c,d]       ->   dim == 1
 *           [[a,b],[c,d]]   ->   dim == 2
 *   val_size: size of data vector
 *   vala_ANY: data vector
 *
 ******************************************************************************/

void
ICMCompileND_CREATE__ARRAY__DIM (int dim, int val_size, char **vala_ANY)
{
    bool entries_are_scalars;
    int i;

    DBUG_ENTER ("ICMCompileND_CREATE__ARRAY__DIM");

#define ND_CREATE__ARRAY__DIM
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE__ARRAY__DIM

    /*
     * CAUTION:
     * 'vala_ANY[i]' is either a tagged identifier (scalar or non-scalar)
     * or a constant scalar!
     */

    entries_are_scalars = FALSE;
    for (i = 0; i < val_size; i++) {
        if ((vala_ANY[i][0] != '(') || /* not a tagged id -> is a const scalar! */
            (ICUGetShapeClass (vala_ANY[i]) == C_scl)) {
            entries_are_scalars = TRUE;
        }
    }

    if (val_size <= 0) {
        /*
         * 'A = []' works only for arrays with known dimension/shape!
         */
        fprintf (outfile, "SAC_ICM_UNDEF()");
    } else if (entries_are_scalars) {
        fprintf (outfile, "%d", dim);
    } else {
        /* 'vala_ANY[i]' is a tagged identifier! */
        fprintf (outfile, "SAC_ND_A_DIM( %s) + %d", vala_ANY[0], dim);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE__ARRAY__SHAPE( char *to_NT, int to_sdim,
 *                                           int dim, int *shp,
 *                                           int val_size, char **vala_ANY,
 *                                           int val0_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE__ARRAY__SHAPE( to_NT, to_sdim, dim, [ shp ]* ,
 *                            val_size, [ vala_ANY ]* , val0_sdim )
 *
 *   dim: top-level dimension
 *   shape: top-level shape
 *           [a,b,c,d]       ->   dim == 1, shape == [4]
 *           [[a,b],[c,d]]   ->   dim == 2, shape == [2,2]
 *   val_size: size of data vector
 *   vala_ANY: data vector
 *   val0_sdim: shape-encoded dimension of the data vector elements
 *
 ******************************************************************************/

void
ICMCompileND_CREATE__ARRAY__SHAPE (char *to_NT, int to_sdim, int dim, int *shp,
                                   int val_size, char **vala_ANY, int val0_sdim)
{
    bool entries_are_scalars;
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int val0_dim = DIM_NO_OFFSET (val0_sdim);

    DBUG_ENTER ("ICMCompileND_CREATE__ARRAY__SHAPE");

#define ND_CREATE__ARRAY__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE__ARRAY__SHAPE

    /*
     * CAUTION:
     * 'vala_ANY[i]' is either a tagged identifier (scalar or non-scalar)
     * or a constant scalar!
     */

    entries_are_scalars = FALSE;
    for (i = 0; i < val_size; i++) {
        if ((vala_ANY[i][0] != '(') || /* not a tagged id -> is a const scalar! */
            (ICUGetShapeClass (vala_ANY[i]) == C_scl)) {
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
        ICMCompileND_SET__SHAPE (to_NT, dim, shp_str);
        for (i = 0; i < dim; i++) {
            shp_str[i] = Free (shp_str[i]);
        }
        shp_str = Free (shp_str);
    } else {
        /* 'vala_ANY[i]' is a tagged identifier */

        /*
         * set descriptor and non-constant part of mirror
         */
        switch (to_sc) {
        case C_aud:
            /* check whether all entries have identical dimension */
            for (i = 1; i < val_size; i++) {
                ASSURE_TYPE_ASS (fprintf (outfile,
                                          "SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s)",
                                          vala_ANY[i], vala_ANY[0]);
                                 , fprintf (outfile,
                                            "Inconsistent vector found:"
                                            " First entry and entry at position %d have"
                                            " different dimension!",
                                            i););
                /* check whether all entries have identical size */
                ASSURE_TYPE_ASS (fprintf (outfile,
                                          "SAC_ND_A_SIZE( %s) == SAC_ND_A_SIZE( %s)",
                                          vala_ANY[i], vala_ANY[0]);
                                 , fprintf (outfile,
                                            "Inconsistent vector found:"
                                            " First entry and entry at position %d have"
                                            " different size!",
                                            i););
            }

            /*
             * ND_A_DESC_DIM, ND_A_MIRROR_DIM have already been set by
             *                                                      ND_ALLOC__DESC!
             */
            ASSURE_TYPE_ASS (fprintf (outfile,
                                      "SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s) + %d",
                                      to_NT, vala_ANY[0], dim);
                             , fprintf (outfile,
                                        "Assignment with incompatible types found!"););

            for (i = 0; i < dim; i++) {
                SET_SHAPE_AUD__NUM (to_NT, i, fprintf (outfile, "%d", shp[i]););
            }
            SET_SHAPES_AUD__XXX (to_NT, i, fprintf (outfile, "SAC_i");
                                 , dim, fprintf (outfile, "%d", dim);
                                 , dim + val0_dim,
                                 fprintf (outfile, "SAC_ND_A_DIM( %s)", to_NT);
                                 ,
                                 /* noop */
                                 , fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)",
                                            vala_ANY[0], i - dim);
                                 , fprintf (outfile, "SAC_ND_A_SHAPE( %s, SAC_i-%d)",
                                            vala_ANY[0], dim);
                                 /* SAC_ND_A_SHAPE() with variable index works for AUD
                                    only! */
            );

            SET_SIZE (to_NT, fprintf (outfile, "%d * SAC_ND_A_SIZE( %s)", val_size,
                                      vala_ANY[0]););
            break;

        case C_akd:
            /* check whether all entries have identical shape */
            for (i = 1; i < val_size; i++) {
                int d;
                for (d = 0; d < to_dim - 1; d++) {
                    ASSURE_TYPE_ASS (fprintf (outfile,
                                              "SAC_ND_A_SHAPE( %s, %d) == "
                                              "SAC_ND_A_SHAPE( %s, %d)",
                                              vala_ANY[i], d, vala_ANY[0], d);
                                     ,
                                     fprintf (outfile,
                                              "Inconsistent vector found:"
                                              " First entry and entry at position %d have"
                                              " different shape!",
                                              i););
                }
            }

            for (i = 0; i < dim; i++) {
                SET_SHAPE_AKD (to_NT, i, fprintf (outfile, "%d", shp[i]););
            }
            SET_SHAPES_AKD (to_NT, i, dim, to_dim, ,
                            fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)", vala_ANY[0],
                                     i - dim););

            SET_SIZE (to_NT, fprintf (outfile, "%d * SAC_ND_A_SIZE( %s)", val_size,
                                      vala_ANY[0]););
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
            for (i = 0; i < dim; i++) {
                ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d) == %d", to_NT,
                                          i, shp[i]);
                                 ,
                                 fprintf (outfile,
                                          "Assignment with incompatible types found!"););
            }
            DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
            for (i = dim; i < to_dim; i++) {
                ASSURE_TYPE_ASS (fprintf (outfile,
                                          "SAC_ND_A_SHAPE( %s, %d) == SAC_ND_A_SHAPE( "
                                          "%s, %d)",
                                          to_NT, i, vala_ANY[0], i - dim);
                                 ,
                                 fprintf (outfile,
                                          "Assignment with incompatible types found!"););
            }
            /* here is no break missing */

        case C_akd:
            ASSURE_TYPE_ASS (fprintf (outfile,
                                      "SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s) + %d",
                                      to_NT, vala_ANY[0], dim);
                             , fprintf (outfile,
                                        "Assignment with incompatible types found!"););
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
 *   void ICMCompileND_CREATE__ARRAY__DATA( char *to_NT, int to_sdim,
 *                                          int val_size, char **vala_ANY,
 *                                          char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE__VEC__DATAT( to_NT, to_sdim, val_size, [ vala_ANY ]* , copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_CREATE__ARRAY__DATA (char *to_NT, int to_sdim, int val_size, char **vala_ANY,
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
     * 'vala_ANY[i]' is either a tagged identifier (scalar or non-scalar)
     * or a constant scalar!
     */

    entries_are_scalars = FALSE;
    for (i = 0; i < val_size; i++) {
        if ((vala_ANY[i][0] != '(') || /* not a tagged id -> is a const scalar! */
            (ICUGetShapeClass (vala_ANY[i]) == C_scl)) {
            entries_are_scalars = TRUE;
        }
    }

    if (entries_are_scalars) {
        for (i = 0; i < val_size; i++) {
            INDENT;
            fprintf (outfile, "SAC_ND_WRITE_COPY( %s, %d, ", to_NT, i);
            ReadScalar_Check (vala_ANY[i], NULL, 0);
            fprintf (outfile, ", %s)\n", copyfun);
        }
    } else {
        /* 'vala_ANY[i]' is a tagged identifier */

        if (val_size > 0) {
            BLOCK_VARDECS (fprintf (outfile, "int SAC_j, SAC_i = 0;");
                           ,
                           for (i = 0; i < val_size; i++) {
                               /* check whether all entries have identical size */
                               if (i > 0) {
                                   ASSURE_TYPE_ASS (fprintf (outfile,
                                                             "SAC_ND_A_SIZE( %s) == "
                                                             "SAC_ND_A_SIZE( %s)",
                                                             vala_ANY[i], vala_ANY[0]);
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
                                                        vala_ANY[i]);
                                             , INDENT;
                                             fprintf (outfile,
                                                      "SAC_ND_WRITE_READ_COPY("
                                                      " %s, SAC_i, %s, SAC_j, %s)\n",
                                                      to_NT, vala_ANY[i], copyfun);
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
 *   void ICMCompileND_PRF_SHAPE__DATA( char *to_NT, int to_sdim,
 *                                      char *from_NT, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SHAPE__DATA( to_NT, to_sdim, from_NT, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SHAPE__DATA (char *to_NT, int to_sdim, char *from_NT, int from_sdim)
{
    int i;
    shape_class_t from_sc = ICUGetShapeClass (from_NT);
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
             to_NT, to_sdim, from_NT, from_sdim);

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
            fprintf (outfile, "SAC_ND_WRITE( %s, %d) = SAC_ND_A_SHAPE( %s, %d);\n", to_NT,
                     i, from_NT, i);
        }
        break;

    case C_aud:
        FOR_LOOP_INC_VARDEC (fprintf (outfile, "SAC_i");, fprintf (outfile, "0");
                             , fprintf (outfile, "SAC_ND_A_DIM( %s)", from_NT);, INDENT;
                             fprintf (outfile,
                                      "SAC_ND_WRITE( %s, SAC_i) = "
                                      "SAC_ND_A_SHAPE( %s, SAC_i);\n",
                                      to_NT, from_NT););
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
 *   void PrfReshape_Shape( char *to_NT, int to_sdim,
 *                          void *shp, int shp_size,
 *                          void (*shp_size_fun)( void *),
 *                          void (*shp_read_fun)( void *, char *, int))
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
PrfReshape_Shape (char *to_NT, int to_sdim, void *shp, int shp_size,
                  void (*shp_size_fun) (void *),
                  void (*shp_read_fun) (void *, char *, int))
{
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("PrfReshape_Shape");

    /*
     * set descriptor and non-constant part of mirror
     */
    switch (to_sc) {
    case C_aud:
        /*
         * ND_A_DESC_DIM, ND_A_MIRROR_DIM have already been set by ND_ALLOC__DESC!
         */
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == ", to_NT);
                         GetAttr (shp, shp_size, shp_size_fun);
                         ,
                         fprintf (outfile, "Assignment with incompatible types found!"););
        BLOCK_VARDECS (fprintf (outfile, "int SAC_size = 1;");
                       ,
                       /*
                        * although 'to_NT' is AUD, 'to_dim' may indeed be >=0 if the sac2c
                        * flag -minarrayrep has been used, i.e. 'to_NT' may be implemented
                        * as AUD although it is AKD!!!
                        */
                       SET_SHAPES_AUD__XXX (to_NT, i, fprintf (outfile, "SAC_i");
                                            , 0, fprintf (outfile, "0");
                                            , to_dim,
                                            fprintf (outfile, "SAC_ND_A_DIM( %s)", to_NT);
                                            , INDENT; fprintf (outfile, "SAC_size *= \n");
                                            , shp_read_fun (shp, NULL, i);
                                            , shp_read_fun (shp, "SAC_i", -1););

                       SET_SIZE (to_NT, fprintf (outfile, "SAC_size");););
        break;

    case C_akd:
        BLOCK_VARDECS (fprintf (outfile, "int SAC_size = 1;");
                       , SET_SHAPES_AKD (to_NT, i, 0, to_dim, INDENT;
                                         fprintf (outfile, "SAC_size *= \n");
                                         , shp_read_fun (shp, NULL, i););

                       SET_SIZE (to_NT, fprintf (outfile, "SAC_size");););
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
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d) == ", to_NT, i);
                             shp_read_fun (shp, NULL, i);
                             , fprintf (outfile,
                                        "Assignment with incompatible types found!"););
        }
        /* here is no break missing */

    case C_akd:
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == ", to_NT);
                         GetAttr (shp, shp_size, shp_size_fun);
                         ,
                         fprintf (outfile, "Assignment with incompatible types found!"););
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
 *   void ICMCompileND_PRF_RESHAPE__SHAPE_id( char *to_NT, int to_sdim,
 *                                            char *shp_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_RESHAPE__SHAPE_id( to_NT, to_sdim, shp_NT)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_RESHAPE__SHAPE_id (char *to_NT, int to_sdim, char *shp_NT)
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
             to_NT, to_sdim);

    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 1", shp_NT);
                     , fprintf (outfile, "1st argument of F_reshape has (dim != 1)!"););

    PrfReshape_Shape (to_NT, to_sdim, shp_NT, -1, SizeId, ReadId);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_RESHAPE__SHAPE_arr( char *to_NT, int to_sdim,
 *                                             int shp_size, char **shpa_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_RESHAPE__SHAPE_arr( to_NT, to_sdim, shp_size, shpa_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_RESHAPE__SHAPE_arr (char *to_NT, int to_sdim, int shp_size,
                                     char **shpa_ANY)
{
    int i;

    DBUG_ENTER ("ICMCompileND_PRF_RESHAPE__SHAPE_arr");

#define ND_PRF_RESHAPE__SHAPE_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_RESHAPE__SHAPE_arr

    /*
     * CAUTION:
     * 'shpa_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_RESHAPE__SHAPE( %s, %d, ...)\"))\n",
             to_NT, to_sdim);

    for (i = 0; i < shp_size; i++) {
        if (shpa_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", shpa_ANY[i]);
                             , fprintf (outfile,
                                        "1st argument of F_reshape has (dim != 1)!"););
        }
    }

    PrfReshape_Shape (to_NT, to_sdim, shpa_ANY, shp_size, NULL, ReadConstArray);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_SEL__SHAPE_id( char *to_NT, int to_sdim,
 *                                        char *from_NT, int from_sdim,
 *                                        char *idx_NT)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL__SHAPE_id( to_NT, to_sdim, from_NT, from_sdim, idx_NT)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL__SHAPE_id (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                char *idx_NT)
{
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_PRF_SEL__SHAPE_id");

#define ND_PRF_SEL__SHAPE_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL__SHAPE_id

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__SHAPE( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    switch (to_sc) {
    case C_aud:
        /*
         * for the time being implemented for scalar results only!
         */
        if (to_dim != 0) {
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == SAC_ND_A_SIZE( %s)",
                                      from_NT, idx_NT);
                             , fprintf (outfile, "Result of F_sel has (dim != 0)!"););
        }
        ICMCompileND_SET__SHAPE (to_NT, 0, NULL);
        break;

    case C_akd:
        /* here is no break missing */
    case C_aks:
        DBUG_ASSERT ((0), "sel() with non-scalar result not yet implemented");
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
 *   void ICMCompileND_PRF_SEL__SHAPE_arr( char *to_NT, int to_sdim,
 *                                         char *from_NT, int from_sdim,
 *                                         int idx_size, char **idxa_ANY)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL__SHAPE_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                          idx_size, [ idxa_ANY ]* )
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL__SHAPE_arr (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                 int idx_size, char **idxa_ANY)
{
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_PRF_SEL__SHAPE_arr");

#define ND_PRF_SEL__SHAPE_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL__SHAPE_arr

    /*
     * CAUTION:
     * 'idxa_ANY[i]' is either a tagged identifier or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__SHAPE( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    switch (to_sc) {
    case C_aud:
        /*
         * for the time being implemented for scalar results only!
         */
        if (to_dim != 0) {
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == %d", from_NT,
                                      idx_size);
                             , fprintf (outfile, "Result of F_sel has (dim != 0)!"););
        }
        ICMCompileND_SET__SHAPE (to_NT, 0, NULL);
        break;

    case C_akd:
        /* here is no break missing */
    case C_aks:
        DBUG_ASSERT ((0), "sel() with non-scalar result not yet implemented");
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
 *   void PrfSel_Data( char *to_NT, int to_sdim,
 *                     char *from_NT, int from_sdim,
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
PrfSel_Data (char *to_NT, int to_sdim, char *from_NT, int from_sdim, void *idx,
             int idx_size, void (*idx_size_fun) (void *),
             void (*idx_read_fun) (void *, char *, int), char *copyfun)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("PrfSel_Data");

    if (to_dim == 0) {
        BLOCK_VARDECS (fprintf (outfile, "int SAC_idx;");
                       , Vect2Offset ("SAC_idx", idx, idx_size, idx_size_fun,
                                      idx_read_fun, from_NT, from_dim);
                       INDENT;
                       fprintf (outfile,
                                "SAC_ND_WRITE_READ_COPY( %s, 0, %s, SAC_idx, %s)\n",
                                to_NT, from_NT, copyfun););
    } else {
        BLOCK_VARDECS (fprintf (outfile, "int SAC_idx, SAC_i;");
                       , Vect2Offset ("SAC_idx", idx, idx_size, idx_size_fun,
                                      idx_read_fun, from_NT, from_dim);
                       FOR_LOOP_INC (fprintf (outfile, "SAC_i");, fprintf (outfile, "0");
                                     , fprintf (outfile, "SAC_ND_A_SIZE( %s)", to_NT);
                                     , INDENT; fprintf (outfile,
                                                        "SAC_ND_WRITE_READ_COPY( %s, "
                                                        "SAC_i, %s, SAC_idx, %s)\n",
                                                        to_NT, from_NT, copyfun);
                                     INDENT; fprintf (outfile, "SAC_idx++;\n");););
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_SEL__DATA_id( char *to_NT, int to_sdim,
 *                                       char *from_NT, int from_sdim,
 *                                       char *idx_NT, int idx_size,
 *                                       char *copyfun)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL__DATA_id( to_NT, to_sdim, from_NT, from_sdim, idx_NT, idx_size,
 *                        copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL__DATA_id (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                               char *idx_NT, int idx_size, char *copyfun)
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
             to_NT, to_sdim, from_NT, from_sdim);

    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 1", idx_NT);
                     , fprintf (outfile, "1st argument of F_sel has (dim != 1)!"););
    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) >= SAC_ND_A_SIZE( %s)", from_NT,
                              idx_NT);
                     , fprintf (outfile, "1st argument of F_sel has illegal size!"););

    PrfSel_Data (to_NT, to_sdim, from_NT, from_sdim, idx_NT, idx_size, SizeId, ReadId,
                 copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_SEL__DATA_arr( char *to_NT, int to_sdim,
 *                                        char *from_NT, int from_sdim,
 *                                        int idx_size, char **idxa_ANY,
 *                                        char *copyfun)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL__DATA_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                         idx_size, [ idxa_ANY ]* , copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL__DATA_arr (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                int idx_size, char **idxa_ANY, char *copyfun)
{
    int i;

    DBUG_ENTER ("ICMCompileND_PRF_SEL__DATA_arr");

#define ND_PRF_SEL__DATA_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL__DATA_arr

    /*
     * CAUTION:
     * 'idxa_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__DATA( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    for (i = 0; i < idx_size; i++) {
        if (idxa_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", idxa_ANY[i]);
                             ,
                             fprintf (outfile, "1st argument of F_sel has (dim != 1)!"););
        }
    }
    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) >= %d", from_NT, idx_size);
                     , fprintf (outfile, "1st argument of F_sel has illegal size!"););

    PrfSel_Data (to_NT, to_sdim, from_NT, from_sdim, idxa_ANY, idx_size, NULL,
                 ReadConstArray, copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void PrfModarray_Data( char *to_NT, int to_sdim,
 *                          char *from_NT, int from_sdim,
 *                          bool idx_unrolled, void *idx, int idx_size,
 *                          void (*idx_size_fun)( void *),
 *                          void (*idx_read_fun)( void *, char *, int),
 *                          char *val_ANY,
 *                          char *copyfun)
 *
 * Description:
 *   implements all the ND_PRF_..._MODARRAY__DATA_... ICMs.
 *
 ******************************************************************************/

static void
PrfModarray_Data (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                  bool idx_unrolled, void *idx, int idx_size,
                  void (*idx_size_fun) (void *),
                  void (*idx_read_fun) (void *, char *, int), char *val_ANY,
                  char *copyfun)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("PrfModarray_Data");

    /*
     * CAUTION:
     * 'val_ANY' is either a tagged identifier or a constant scalar!
     */

    if ((val_ANY[0] != '(') || /* not a tagged id -> is a const scalar! */
        (ICUGetShapeClass (val_ANY) == C_scl)) {
        /* 'val_ANY' is scalar */
        INDENT;
        fprintf (outfile, "SAC_IS_REUSED__BLOCK_BEGIN( %s, %s)\n", to_NT, from_NT);
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_TR_MEM_PRINT("
                 " (\"reuse memory of %s at %%p for %s\","
                 " SAC_ND_A_FIELD( %s)))\n",
                 from_NT, to_NT, from_NT);
        indent--;
        INDENT;
        fprintf (outfile, "SAC_IS_REUSED__BLOCK_ELSE( %s, %s)\n", to_NT, from_NT);
        indent++;
        INDENT;
        fprintf (outfile, "int SAC_i;\n");
        FOR_LOOP_INC (fprintf (outfile, "SAC_i");, fprintf (outfile, "0");
                      , fprintf (outfile, "SAC_ND_A_SIZE( %s)", to_NT);, INDENT;
                      fprintf (outfile,
                               "SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_i, %s)\n",
                               to_NT, from_NT, copyfun););
        indent--;
        INDENT;
        fprintf (outfile, "SAC_IS_REUSED__BLOCK_END( %s, %s)\n", to_NT, from_NT);

        BLOCK_VARDECS (fprintf (outfile, "int SAC_idx;");
                       ,
                       if (idx_unrolled) {
                           INDENT;
                           fprintf (outfile, "SAC_idx = ");
                           idx_read_fun (idx, NULL, 0);
                           fprintf (outfile, ";\n");
                       } else {
                           Vect2Offset ("SAC_idx", idx, idx_size, idx_size_fun,
                                        idx_read_fun, to_NT, to_dim);
                       } INDENT;
                       fprintf (outfile, "SAC_ND_WRITE_COPY( %s, SAC_idx, ", to_NT);
                       ReadScalar (val_ANY, NULL, 0);
                       fprintf (outfile, " , %s)\n", copyfun););
    } else {
        /* 'val_ANY' is a tagged identifier representing a non-scalar array */
        BLOCK_VARDECS (fprintf (outfile, "int SAC_i, SAC_j, SAC_idx;");
                       ,
                       if (idx_unrolled) {
                           INDENT;
                           fprintf (outfile, "SAC_idx = ");
                           idx_read_fun (idx, NULL, 0);
                           fprintf (outfile, ";\n");
                       } else {
                           Vect2Offset ("SAC_idx", idx, idx_size, idx_size_fun,
                                        idx_read_fun, to_NT, to_dim);
                       } INDENT;
                       fprintf (outfile, "SAC_IS_REUSED__BLOCK_BEGIN( %s, %s)\n", to_NT,
                                from_NT);
                       indent++; INDENT;
                       fprintf (outfile,
                                "SAC_TR_MEM_PRINT("
                                " (\"reuse memory of %s at %%p for %s\","
                                " SAC_ND_A_FIELD( %s)))\n",
                                from_NT, to_NT, from_NT);
                       FOR_LOOP (fprintf (outfile, "SAC_i = SAC_idx, SAC_j = 0");
                                 ,
                                 fprintf (outfile, "SAC_j < SAC_ND_A_SIZE( %s)", val_ANY);
                                 , fprintf (outfile, "SAC_i++, SAC_j++");, INDENT;
                                 fprintf (outfile,
                                          "SAC_ND_WRITE_READ_COPY("
                                          " %s, SAC_i, %s, SAC_j, %s)\n",
                                          to_NT, val_ANY, copyfun););
                       indent--; INDENT;
                       fprintf (outfile, "SAC_IS_REUSED__BLOCK_ELSE( %s, %s)\n", to_NT,
                                from_NT);
                       indent++;
                       FOR_LOOP_INC (fprintf (outfile, "SAC_i");, fprintf (outfile, "0");
                                     , fprintf (outfile, "SAC_idx");, INDENT;
                                     fprintf (outfile,
                                              "SAC_ND_WRITE_READ_COPY("
                                              " %s, SAC_i, %s, SAC_i, %s)\n",
                                              to_NT, from_NT, copyfun););
                       FOR_LOOP_INC (fprintf (outfile, "SAC_j");, fprintf (outfile, "0");
                                     , fprintf (outfile, "SAC_ND_A_SIZE( %s)", val_ANY);
                                     , INDENT; fprintf (outfile,
                                                        "SAC_ND_WRITE_READ_COPY("
                                                        " %s, SAC_i, %s, SAC_j, %s)\n",
                                                        to_NT, val_ANY, copyfun);
                                     INDENT; fprintf (outfile, "SAC_i++;\n"););
                       FOR_LOOP (fprintf (outfile, " ");
                                 , fprintf (outfile, "SAC_i < SAC_ND_A_SIZE( %s)", to_NT);
                                 , fprintf (outfile, "SAC_i++");, INDENT;
                                 fprintf (outfile,
                                          "SAC_ND_WRITE_READ_COPY("
                                          " %s, SAC_i, %s, SAC_i, %s)\n",
                                          to_NT, from_NT, copyfun););
                       indent--; INDENT;
                       fprintf (outfile, "SAC_IS_REUSED__BLOCK_END( %s, %s)\n", to_NT,
                                from_NT););
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY__DATA_id( char *to_NT, int to_sdim,
 *                                            char *from_NT, int from_sdim,
 *                                            char *idx_NT, int idx_size,
 *                                            char *val_ANY, char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY__DATA_id( to_NT, to_sdim, from_NT, from_sdim,
 *                             idx_NT, idx_size, val_ANY, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY__DATA_id (char *to_NT, int to_sdim, char *from_NT,
                                    int from_sdim, char *idx_NT, int idx_size,
                                    char *val_ANY, char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY__DATA_id");

#define ND_PRF_MODARRAY__DATA_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY__DATA_id

    /*
     * CAUTION:
     * 'val_ANY' is either a tagged identifier or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_MODARRAY__DATA( %s, %d, %s, %d, ..., %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, val_ANY);

    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 1", idx_NT);
                     , fprintf (outfile, "2nd argument of F_modarray has (dim != 1)!"););
    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) >= SAC_ND_A_SIZE( %s)", from_NT,
                              idx_NT);
                     ,
                     fprintf (outfile, "2nd argument of F_modarray has illegal size!"););

    PrfModarray_Data (to_NT, to_sdim, from_NT, from_sdim, FALSE, idx_NT, idx_size, SizeId,
                      ReadId, val_ANY, copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY__DATA_arr( char *to_NT, int to_sdim,
 *                                             char *from_NT, int from_sdim,
 *                                             int idx_size, char **idxa_ANY,
 *                                             char *val_ANY,
 *                                             char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY__DATA_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                              idx_size, [ idxa_ANY ]* , val_ANY, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY__DATA_arr (char *to_NT, int to_sdim, char *from_NT,
                                     int from_sdim, int idx_size, char **idxa_ANY,
                                     char *val_ANY, char *copyfun)
{
    int i;

    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY__DATA_arr");

#define ND_PRF_MODARRAY__DATA_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY__DATA_arr

    /*
     * CAUTION:
     * 'idxa_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     * 'val_ANY' is either a tagged identifier (scalar or non-scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_MODARRAY__DATA( %s, %d, %s, %d, ..., %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, val_ANY);

    for (i = 0; i < idx_size; i++) {
        if (idxa_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", idxa_ANY[i]);
                             , fprintf (outfile,
                                        "2nd argument of F_modarray has (dim != 1)"););
        }
    }
    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) >= %d", from_NT, idx_size);
                     ,
                     fprintf (outfile, "2nd argument of F_modarray has illegal size!"););

    PrfModarray_Data (to_NT, to_sdim, from_NT, from_sdim, FALSE, idxa_ANY, idx_size, NULL,
                      ReadConstArray, val_ANY, copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_IDX_SEL__SHAPE( char *to_NT, int to_sdim,
 *                                         char *from_NT, int from_sdim,
 *                                         char *idx_ANY)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_IDX_SEL__SHAPE( to_NT, to_sdim, from_NT, from_sdim, idx_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_IDX_SEL__SHAPE (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                 char *idx_ANY)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_PRF_IDX_SEL__SHAPE");

#define ND_PRF_IDX_SEL__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_IDX_SEL__SHAPE

    /*
     * CAUTION:
     * 'idx_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_IDX_SEL__SHAPE( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, idx_ANY);

    if (to_dim == 0) {
        ICMCompileND_SET__SHAPE (to_NT, 0, NULL);
    } else {
        DBUG_ASSERT ((0), "idx_sel() with non-scalar result not yet implemented");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_IDX_SEL__DATA( char *to_NT, int to_sdim,
 *                                        char *from_NT, int from_sdim,
 *                                        char *idx_ANY,
 *                                        char *copyfun)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_IDX_SEL__DATA( to_NT, to_sdim, from_NT, from_sdim, idx_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_IDX_SEL__DATA (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                char *idx_ANY, char *copyfun)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_PRF_IDX_SEL__DATA");

#define ND_PRF_IDX_SEL__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_IDX_SEL__DATA

    /*
     * CAUTION:
     * 'idx_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_IDX_SEL__DATA( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, idx_ANY);

    if (idx_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", idx_ANY);
                         ,
                         fprintf (outfile, "1st argument of F_idx_sel has (dim != 0)!"););
    }

    /*
     * idx_sel() works only for arrays with known dimension!
     */
    DBUG_ASSERT ((to_dim >= 0), "idx_sel() with unknown dimension found!");

    if (to_dim == 0) {
        /*
         * 'to_NT' is scalar
         */
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE_READ_COPY( %s, 0, %s, ", to_NT, from_NT);
        ReadScalar (idx_ANY, NULL, 0);
        fprintf (outfile, ", %s)\n", copyfun);
    } else {
        /*
         * 'to_NT' is array
         */
        FOR_LOOP_VARDECS (fprintf (outfile, "int SAC_i, SAC_j;");
                          , fprintf (outfile, "SAC_i = 0, SAC_j = ");
                          ReadScalar (idx_ANY, NULL, 0);
                          , fprintf (outfile, "SAC_i < SAC_ND_A_SIZE( %s)", to_NT);
                          , fprintf (outfile, "SAC_i++, SAC_j++");, INDENT;
                          fprintf (outfile,
                                   "SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_j, %s)\n",
                                   to_NT, from_NT, copyfun););
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_IDX_MODARRAY__DATA( char *to_NT, int to_sdim,
 *                                             char *from_NT, int from_sdim,
 *                                             char *idx_ANY, char *val_ANY,
 *                                             char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_IDX_MODARRAY__DATA( to_NT, to_sdim, from_NT, from_sdim, idx, val,
 *                              copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_IDX_MODARRAY__DATA (char *to_NT, int to_sdim, char *from_NT,
                                     int from_sdim, char *idx_ANY, char *val_ANY,
                                     char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_PRF_IDX_MODARRAY__DATA");

#define ND_PRF_IDX_MODARRAY__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_IDX_MODARRAY__DATA

    /*
     * CAUTION:
     * 'idx_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     * 'val_ANY' is either a tagged identifier (scalar or non-scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_IDX_MODARRAY__DATA( %s, %d, %s, %d, %s, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, idx_ANY, val_ANY);

    if (idx_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", idx_ANY);
                         , fprintf (outfile,
                                    "2nd argument of F_modarray has (dim != 0)!"););
    }

    PrfModarray_Data (to_NT, to_sdim, from_NT, from_sdim, TRUE, idx_ANY, 1, NULL,
                      ReadScalar, val_ANY, copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_VECT2OFFSET( char *off_NT, int from_size, char *from_NT,
 *                                  int shp_size, char **shpa_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_VECT2OFFSET( off_NT, from_size, from_NT, shp_size, shpa_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_VECT2OFFSET (char *off_NT, int from_size, char *from_NT, int shp_size,
                          char **shpa_ANY)
{
    DBUG_ENTER ("ICMCompileND_VECT2OFFSET");

#define ND_VECT2OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_VECT2OFFSET

    /*
     * CAUTION:
     * 'shpa_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    DBUG_ASSERT ((from_size >= 0), "Illegal size found!");

    Vect2Offset2 (off_NT, from_NT, from_size, NULL, ReadId, shpa_ANY, shp_size, NULL,
                  ReadConstArray);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_IDXS2OFFSET( char *off_NT, int idxs_size, char **idxs_NT,
 *                                  int shp_size, char **shpa_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_IDXS2OFFSET( off_NT, idxs_size, idxs_NT, shp_size, shpa_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_IDXS2OFFSET (char *off_NT, int idxs_size, char **idxs_NT, int shp_size,
                          char **shpa_ANY)
{
    DBUG_ENTER ("ICMCompileND_IDXS2OFFSET");

#define ND_IDXS2OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_IDXS2OFFSET

    /*
     * CAUTION:
     * 'shpa_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    DBUG_ASSERT ((idxs_size >= 0), "Illegal size found!");

    Vect2Offset2 (off_NT, idxs_NT, idxs_size, NULL, ReadConstArray, shpa_ANY, shp_size,
                  NULL, ReadConstArray);

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
 *   void ICMCompileND_KD_SEL_CxA_S( char *a, char *res, int dim, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_SEL_CxA_S( a, res, dim, [ v ]* )
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
    fprintf (outfile, "%s = SAC_ND_READ_ARRAY( %s, ", res, a);
    Vect2Offset (dim, AccessConst (vi, i), dim, a);
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
    fprintf (outfile, "%s = SAC_ND_READ_ARRAY( %s, ", res, a);
    Vect2Offset (dim, AccessVect (v, i), dim, a);
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
 *   ND_KD_SEL_CxA_A( dima, a, res, dimv, [ v ]* )
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
    CopyBlock (a, Vect2Offset (dimv, AccessConst (vi, i), dima, a), res);
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
    CopyBlock (a, Vect2Offset (dimv, AccessVect (v, i), dima, a), res);
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
 *   ND_KD_TAKE_CxA_A( dima, a, res, dimv, [ v ]* )
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
             fprintf (outfile, "(SAC_ND_A_SHAPE( %s, %d) - ", a, i);
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
 *   ND_KD_DROP_CxA_A( dima, a, res, dimv, [ v ]* )
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
    TakeSeg (a, dima, Vect2Offset (dimv, AccessConst (vi, i), dima, a), /* offset */
             dimv, /* dim of sizes & offsets */
             fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d) - ", a, i);
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
                                fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d) *", ar[i], j);
                            }
                            fprintf (outfile, "1");
                        }),
              FillRes (res, INDENT;
                       fprintf (outfile, "for (SAC_i0 = 0; SAC_i0 < SAC_bl0;"
                                         " SAC_i0++, SAC_idest++, SAC_isrc++)\n");
                       indent++; INDENT; fprintf (outfile,
                                                  "SAC_ND_WRITE_ARRAY( %s, SAC_idest) ="
                                                  " SAC_ND_READ_ARRAY( %s, SAC_isrc);\n",
                                                  res, ar[0]);
                       indent--; INDENT;
                       fprintf (outfile, "for (SAC_i1 = 0; SAC_i1 < SAC_bl1;"
                                         " SAC_i1++, SAC_idest++, SAC_isrc1++)\n");
                       indent++; INDENT; fprintf (outfile,
                                                  "SAC_ND_WRITE_ARRAY( %s, SAC_idest) ="
                                                  " SAC_ND_READ_ARRAY( %s, SAC_isrc1);\n",
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
                                fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d) * ", a, j);
                            }
                            fprintf (outfile, "(%s<0 ? ", numstr[0]);
                            fprintf (outfile,
                                     "( SAC_ND_A_SHAPE( %s, %d) == 0 ? 0 :"
                                     " SAC_ND_A_SHAPE( %s, %d) + "
                                     " (%s %% SAC_ND_A_SHAPE( %s, %d))) : ",
                                     a, rotdim, a, rotdim, numstr[0], a, rotdim);
                            fprintf (outfile,
                                     "( SAC_ND_A_SHAPE( %s, %d) == 0 ? 0 :"
                                     " %s %% SAC_ND_A_SHAPE( %s, %d)))",
                                     a, rotdim, numstr[0], a, rotdim);
                        });
              InitVecs (0, 1, "SAC_bl",
                        {
                            int j;
                            for (j = rotdim + 1; j < dima; j++) {
                                fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d) * ", a, j);
                            }
                            fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)", a, rotdim);
                        });
              InitVecs (0, 1, "SAC_i", fprintf (outfile, "0"));
              InitPtr (fprintf (outfile, "- SAC_shift0"), fprintf (outfile, "0")),
              FillRes (res, INDENT; fprintf (outfile, "SAC_isrc += SAC_bl0;\n"); INDENT;
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
 *   ND_PRF_MODARRAY_AxCxS( res_btype, dimres, res, old, value, dimv, [ v ]* )
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
    Vect2Offset (dimv, AccessConst (vi, i), dimres, res);
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
    Vect2Offset (dim, AccessVect (v, i), dimres, res);
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
 *   ND_PRF_MODARRAY_AxCxA( res_btype, dimres, res, old, val, dimv, [ v ]* )
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
    Vect2Offset (dimv, AccessConst (vi, i), dimres, res);
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
    fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_idx-1; SAC_i++)\n");
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
    fprintf (outfile, "int SAC_idx = ");
    Vect2Offset (dim, AccessVect (v, i), dimres, res);
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
    fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_idx-1; SAC_i++)\n");
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
 *   void ICMCompileND_KS_VECT2OFFSET( char *off_name, char *arr_name,
 *                                     int dim, int dims, char **shp_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KS_VECT2OFFSET( off_name, arr_name, dim, dims, shp_ANY )
 *
 ******************************************************************************/

void
ICMCompileND_KS_VECT2OFFSET (char *off_name, char *arr_name, int dim, int dims,
                             char **shp_ANY)
{
    DBUG_ENTER ("ICMCompileND_KS_VECT2OFFSET");

#define ND_KS_VECT2OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KS_VECT2OFFSET

    INDENT;
    fprintf (outfile, "%s = ", off_name);
    Vect2Offset2 (dim, AccessVect (arr_name, i), dims, AccessConst (shp_ANY, i));
    fprintf (outfile, ";\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_IDXS2OFFSET( char *off, int idxs_size, char **idxs,
 *                                  int shp_size, char **shpa_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_IDXS2OFFSET( off, idxs_size, idxs, shp_size, shpa_ANY )
 *
 ******************************************************************************/

void
ICMCompileND_IDXS2OFFSET (char *off, int idxs_size, char **idxs, int shp_size,
                          char **shpa_ANY)
{
    DBUG_ENTER ("ICMCompileND_IDXS2OFFSET");

#define ND_IDXS2OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_IDXS2OFFSET

    INDENT;
    fprintf (outfile, "%s = ", off);
    Vect2Offset2 (idxs_size, AccessConst (idxs, i), shp_size, AccessConst (shpa_ANY, i));
    fprintf (outfile, ";\n");

    DBUG_VOID_RETURN;
}

#endif /* TAGGED_ARRAYS */
