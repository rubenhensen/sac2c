/*
 *
 * $Log$
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
                        fprintf (outfile, "*SAC_ND_A_SHAPE( %s, %d)", a, j);             \
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
                        fprintf (outfile, "*SAC_ND_A_SHAPE( %s, %d)", a, j);             \
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
 *   void ICMCompileND_FUN_DEC( char *name, char *rettype, int narg, char **arg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEC( name, rettype, narg, [ TAG, type, arg ]* )
 *
 *   where TAG is element in {in, in_rc, out, out_rc, inout, inout_rc}.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_DEC (char *name, char *rettype, int narg, char **arg)
{
    DBUG_ENTER ("ICMCompileND_FUN_DEC");

#define ND_FUN_DEC
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_DEC

    INDENT;
    fprintf (outfile, "%s ", rettype);
    if (strcmp (name, "create_TheCommandLine") == 0) {
        fprintf (outfile, "%s( int __argc, char *__argv[])", name);
    } else if (strncmp (name, "SACf_GlobalObjInit_", strlen ("SACf_GlobalObjInit_"))
               == 0) {
        fprintf (outfile, "%s( int __argc, char *__argv[])", name);
    } else {
        fprintf (outfile, "%s(", name);
        ScanArglist (narg, 3, ",", ,
                     fprintf (outfile, " SAC_ND_PARAM_%s( %s, %s)", arg[i], arg[i + 1],
                              arg[i + 2]));
        fprintf (outfile, ")");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_AP( char *name, char *retname, int narg, char **arg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEC( name, retname, narg, [ TAG, arg ]* )
 *
 *   where TAG is element in {in, in_rc, out, out_rc, inout, inout_rc}.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_AP (char *name, char *retname, int narg, char **arg)
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
                     fprintf (outfile, " SAC_ND_ARG_%s(%s)", arg[i], arg[i + 1]));
        fprintf (outfile, ");");
    }
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_RET( char *retname, int narg, char **arg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_RET( retname, narg, [ TAG, arg, decl_arg ]* )
 *
 *   where TAG is element in { out, out_rc, inout, inout_rc}.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_RET (char *retname, int narg, char **arg)
{
    DBUG_ENTER ("ICMCompileND_FUN_RET");

#define ND_FUN_RET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_RET

    INDENT;
    ScanArglist (narg, 3, "\n", INDENT,
                 fprintf (outfile, "SAC_ND_RET_%s( %s, %s)", arg[i], arg[i + 1],
                          arg[i + 2]));
    if (narg > 0) {
        fprintf (outfile, "\n");
        INDENT;
    }

    if (strcmp (retname, "")) {
        fprintf (outfile, "return( %s);", retname);
    } else {
        fprintf (outfile, "return;");
    }
    fprintf (outfile, "\n");

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
        ICMCompileND_OBJDEF_EXTERN (nt, basetype, sdim);
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
 *   declares an array which is an imported global object
 *
 ******************************************************************************/

void
ICMCompileND_OBJDEF_EXTERN (char *nt, char *basetype, int sdim)
{
    int i;
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ("ICMCompileND_OBJDEF_EXTERN");

#define ND_OBJDEF_EXTERN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_OBJDEF_EXTERN

    switch (ICUGetDataClass (nt)) {
    case C_scl:
        INDENT;
        fprintf (outfile,
                 "extern "
                 "%s SAC_ND_A_FIELD( %s);\n",
                 basetype, nt);
        break;

    case C_aks:
        INDENT;
        fprintf (outfile,
                 "extern "
                 "%s *SAC_ND_A_FIELD( %s);\n",
                 basetype, nt);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "SAC_array_descriptor *SAC_ND_A_DESC( %s);\n",
                 nt);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "const int SAC_ND_A_DIM( %s);\n",
                 nt);
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
        break;

    case C_akd:
        INDENT;
        fprintf (outfile,
                 "extern "
                 "%s *SAC_ND_A_FIELD( %s);\n",
                 basetype, nt);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "SAC_array_descriptor *SAC_ND_A_DESC( %s);\n",
                 nt);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "const int SAC_ND_A_DIM( %s);\n",
                 nt);
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
        break;

    case C_aud:
        INDENT;
        fprintf (outfile,
                 "extern "
                 "%s *SAC_ND_A_FIELD( %s);\n",
                 basetype, nt);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "SAC_array_descriptor *SAC_ND_A_DESC( %s);\n",
                 nt);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "int SAC_ND_A_DIM( %s);\n",
                 nt);
        break;

    case C_hid:
        INDENT;
        fprintf (outfile,
                 "extern "
                 "%s *SAC_ND_A_FIELD( %s);\n",
                 basetype, nt);
        INDENT;
        fprintf (outfile,
                 "extern "
                 "int *SAC_ND_A_DESC( %s);\n",
                 nt);
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
    int size, i;
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ("ICMCompileND_DECL");

#define ND_DECL
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL

    switch (ICUGetDataClass (nt)) {
    case C_scl:
        INDENT;
        fprintf (outfile, "%s SAC_ND_A_FIELD( %s);\n", basetype, nt);
        break;

    case C_aks:
        INDENT;
        fprintf (outfile, "%s *SAC_ND_A_FIELD( %s);\n", basetype, nt);
        INDENT;
        fprintf (outfile, "SAC_array_descriptor *SAC_ND_A_DESC( %s);\n", nt);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_DIM( %s) = %d;\n", nt, dim);
        size = 1;
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "const int SAC_ND_A_SHAPE( %s, %d) = %d;\n", nt, i, shp[i]);
            size *= shp[i];
        }
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_SIZE( %s) = %d;\n", nt, size);
        break;

    case C_akd:
        INDENT;
        fprintf (outfile, "%s *SAC_ND_A_FIELD( %s);\n", basetype, nt);
        INDENT;
        fprintf (outfile, "SAC_array_descriptor *SAC_ND_A_DESC( %s);\n", nt);
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_DIM( %s) = %d;\n", nt, dim);
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "int SAC_ND_A_SHAPE( %s, %d);\n", nt, i);
        }
        INDENT;
        fprintf (outfile, "int SAC_ND_A_SIZE( %s);\n", nt);
        break;

    case C_aud:
        INDENT;
        fprintf (outfile, "%s *SAC_ND_A_FIELD( %s);\n", basetype, nt);
        INDENT;
        fprintf (outfile, "SAC_array_descriptor *SAC_ND_A_DESC( %s);\n", nt);
        INDENT;
        fprintf (outfile, "int SAC_ND_A_DIM( %s);\n", nt);
        break;

    case C_hid:
        INDENT;
        fprintf (outfile, "%s *SAC_ND_A_FIELD( %s);\n", basetype, nt);
        INDENT;
        fprintf (outfile, "int *SAC_ND_A_DESC( %s);\n", nt);
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
 *   void ICMCompileND_DECL_ARG( char *nt, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL_ARG( nt, sdim, shp_0 ... shp_n)
 *   declares an array given as arg
 *
 ******************************************************************************/

void
ICMCompileND_DECL_ARG (char *nt, int sdim, int *shp)
{
    int size, i;
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ("ICMCompileND_DECL_ARG");

#define ND_DECL_ARG
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL_ARG

    switch (ICUGetDataClass (nt)) {
    case C_scl:
        /* NOOP */
        break;

    case C_aks:
        fprintf (outfile, "const int SAC_ND_A_DIM( %s) = %d;\n", nt, dim);
        size = 1;
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "const int SAC_ND_A_SHAPE( %s, %d) = %d;\n", nt, i, shp[i]);
            size *= shp[i];
        }
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_SIZE( %s) = %d;\n", nt, size);
        break;

    case C_akd:
        INDENT;
        fprintf (outfile, "const int SAC_ND_A_DIM( %s) = %d;\n", nt, dim);
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile,
                     "int SAC_ND_A_SHAPE( %s, %d) "
                     "= SAC_ND_A_DESC( %s)->shp[%d];\n",
                     nt, i, nt, i);
        }
        INDENT;
        fprintf (outfile, "int SAC_ND_A_SIZE( %s) = SAC_ND_A_DESC( %s)->shp[0]", nt, nt);
        for (i = 1; i < dim; i++) {
            fprintf (outfile, " * SAC_ND_A_DESC( %s)->shp[%d]", nt, i);
        }
        fprintf (outfile, ";\n");
        break;

    case C_aud:
        INDENT;
        fprintf (outfile, "int SAC_ND_A_DIM( %s) = SAC_ND_A_DESC( %s)->dim;\n", nt, nt);
        break;

    case C_hid:
        /* NOOP */
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
 *   void ICMCompileND_ALLOC( int rc, char *nt, char *basetype,
 *                            int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ALLOC( rc, nt, basetype, sdim, shp_0 ... shp_n)
 *
 ******************************************************************************/

void
ICMCompileND_ALLOC (int rc, char *nt, char *basetype, int sdim, int *shp)
{
    int i;
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ("ICMCompileND_ALLOC");

#define ND_ALLOC
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ALLOC

    switch (ICUGetDataClass (nt)) {
    case C_scl:
        /* NOOP */
        break;

    case C_aks:
        break;

    case C_akd:
        break;

    case C_aud:
        break;

    case C_hid:
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
 *   void ICMCompileND_ASSIGN( char *dest_nt, char *src_nt)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN( dest_nt, src_nt)
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN (char *dest_nt, char *src_nt)
{
    DBUG_ENTER ("ICMCompileND_ASSIGN");

#define ND_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN

    switch (ICUGetDataClass (dest_nt)) {
    case C_scl:
        break;

    case C_aks:
        break;

    case C_akd:
        break;

    case C_aud:
        break;

    case C_hid:
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
 *   void ICMCompileND_COPY( char *dest_nt, char *src_nt)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_COPY( dest_nt, src_nt)
 *
 ******************************************************************************/

void
ICMCompileND_COPY (char *dest_nt, char *src_nt)
{
    DBUG_ENTER ("ICMCompileND_COPY");

#define ND_COPY
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_COPY

    switch (ICUGetDataClass (dest_nt)) {
    case C_scl:
        break;

    case C_aks:
        break;

    case C_akd:
        break;

    case C_aud:
        break;

    case C_hid:
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
 *   void ICMCompileND_MAKE_UNIQUE( char *dest_nt, char *src_nt, char *basetype)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_MAKE_UNIQUE( dest_nt, src_nt, basetype)
 *
 ******************************************************************************/

void
ICMCompileND_MAKE_UNIQUE (char *dest_nt, char *src_nt, char *basetype)
{
    DBUG_ENTER ("ICMCompileND_MAKE_UNIQUE");

#define ND_MAKE_UNIQUE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_MAKE_UNIQUE

    switch (ICUGetDataClass (dest_nt)) {
    case C_scl:
        break;

    case C_aks:
        break;

    case C_akd:
        break;

    case C_aud:
        break;

    case C_hid:
        break;

    default:
        DBUG_ASSERT ((0), "Unknown data class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

#if 0
/*
 *  If SECURE_ALLOC_FREE is defined an extra check wether the requested size
 *  is 0 is done. In that case no call for malloc(0) is executed, but directly
 *  NULL set for the array-variable is done.
 *  This is an Option for possible Problems with calls for malloc(0).
 *  This comment is referenced by: SAC_ND_FREE
 */
#ifdef SECURE_ALLOC_FREE
#define SAC_ND_ALLOC(rc, nt, basetype, sdim, shp)
{
  if (SAC_ND_A_SIZE(nt) != 0) {
    SAC_HM_MALLOC_FIXED_SIZE(SAC_ND_A_FIELD(nt),
                             sizeof(basetype)*SAC_ND_A_SIZE(nt));
  }
  else {
    SAC_ND_A_FIELD(nt)=NULL;
  }
  SAC_HM_MALLOC_FIXED_SIZE(SAC_ND_A_RCP(nt), sizeof(int));
  SAC_ND_A_RC(nt)=rc;
  SAC_TR_MEM_PRINT(("ND_ALLOC( %s, %s, %d) at addr: %p",
#basetype, #nt, rc, SAC_ND_A_FIELD(nt)));
  SAC_TR_INC_ARRAY_MEMCNT(SAC_ND_A_SIZE(nt));
  SAC_TR_REF_PRINT_RC(nt);
  SAC_CS_REGISTER_ARRAY(nt);
}
#else
#define SAC_ND_ALLOC(basetype, nt, rc)
{
  SAC_HM_MALLOC_FIXED_SIZE_WITH_RC(SAC_ND_A_FIELD(nt),
                                   SAC_ND_A_RCP(nt),
                                   sizeof(basetype)*SAC_ND_A_SIZE(nt));
  SAC_ND_A_RC(nt)=rc;
  SAC_TR_MEM_PRINT(("ND_ALLOC( %s, %s, %d) at addr: %p",
#basetype, #nt, rc, SAC_ND_A_FIELD(nt)));
  SAC_TR_INC_ARRAY_MEMCNT(SAC_ND_A_SIZE(nt));
  SAC_TR_REF_PRINT_RC(nt);
  SAC_CS_REGISTER_ARRAY(nt);
}
#endif
#endif

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ASSIGN_CONST_VECT( char *nt, int len, char **s)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN_CONST_VECT( nt, len, s_0 ... s_n)
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN_CONST_VECT (char *nt, int len, char **s)
{
    int i;

    DBUG_ENTER ("ICMCompileND_ASSIGN_CONST_VECT");

#define ND_ASSIGN_CONST_VECT
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN_CONST_VECT

    for (i = 0; i < len; i++) {
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE( %s, %d) = %s;\n", nt, i, s[i]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ASSIGN_CONST_H( char *nt, char * copyfun,
 *                                     int len, char **A)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN_CONST_H( nt, copyfun, len, A_0 ... A_n)
 *   generates a constant array of refcounted hidden values
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN_CONST_H (char *nt, char *copyfun, int len, char **A)
{
    int i;

    DBUG_ENTER ("ICMCompileND_ASSIGN_CONST_H");

#define ND_ASSIGN_CONST_H
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN_CONST_H

    for (i = 0; i < len; i++) {
        INDENT;
        fprintf (outfile, "SAC_ND_NO_RC_MAKE_UNIQUE_HIDDEN( %s, %s[%d], %s);\n", A[i], nt,
                 i, copyfun);
    }

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

#endif /* TAGGED_ARRAYS */

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
    fprintf (outfile, "}\n");
    INDENT;
    fprintf (outfile, "else {\n");
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
    fprintf (outfile, "}\n");
    INDENT;
    fprintf (outfile, "else {\n");
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
 *                                     int dim, int dims, char **s)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KS_VECT2OFFSET( off_name, arr_name, dim, dims, s )
 *
 ******************************************************************************/

void
ICMCompileND_KS_VECT2OFFSET (char *off_name, char *arr_name, int dim, int dims, char **s)
{
    DBUG_ENTER ("ICMCompileND_KS_VECT2OFFSET");

#define ND_KS_VECT2OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KS_VECT2OFFSET

    INDENT;
    fprintf (outfile, "%s = ", off_name);
    VectToOffset2 (dim, AccessVect (arr_name, i), dims, AccessConst (s, i));
    fprintf (outfile, ";\n");

    DBUG_VOID_RETURN;
}
