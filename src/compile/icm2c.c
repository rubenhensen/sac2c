/*
 *
 * $Log$
 * Revision 1.8  1995/04/07 12:19:48  sbs
 * psi, take & drop fixed for args of different dimensionality
 *
 * Revision 1.7  1995/04/06  14:23:25  sbs
 * some bugs fixed
 *
 * Revision 1.6  1995/04/03  13:58:57  sbs
 * first "complete" version
 *
 * Revision 1.5  1995/03/31  13:57:34  sbs
 * ND_CREATE_CONST_ARRAY,ND_KS_ARG_ARRAY & ND_KS_RET_ARRAY inserted
 *
 * Revision 1.4  1995/03/15  07:55:11  sbs
 * bug in VectToOffset fixed
 *
 * Revision 1.3  1995/03/10  17:24:37  sbs
 * New macros develloped; psi, take & drop integrated
 *
 * Revision 1.2  1995/03/08  17:13:08  sbs
 * first version with test embedded
 *
 * Revision 1.1  1995/03/07  18:21:02  sbs
 * Initial revision
 *
 *
 */

#include <malloc.h>
#include "stdio.h"
#include "print.h"
#include "dbug.h"
#include "my_debug.h"

#define AccessVect(v, i) fprintf (outfile, "ND_A_FIELD(%s)[%i])", v, i)

#define AccessConst(v, i) fprintf (outfile, "%s", v[i])

#define VectToOffset(dim, v_i_str, dima, a)                                              \
    {                                                                                    \
        int i;                                                                           \
        for (i = dim - 1; i > 0; i--)                                                    \
            fprintf (outfile, "( ND_KD_A_SHAPE(%s, %d)* ", a, i);                        \
        v_i_str;                                                                         \
        for (i = 1; i < dim; i++) {                                                      \
            fprintf (outfile, "+");                                                      \
            v_i_str;                                                                     \
            fprintf (outfile, ") ");                                                     \
        }                                                                                \
        while (i < dima) {                                                               \
            fprintf (outfile, "*ND_KD_A_SHAPE(%s, %d) ", a, i);                          \
            i++;                                                                         \
        }                                                                                \
    }

#define NewBlock(init, body)                                                             \
    fprintf (outfile, "{\n");                                                            \
    indent++;                                                                            \
    INDENT;                                                                              \
    init;                                                                                \
    body;                                                                                \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}")

#define InitPtr(src, dest)                                                               \
    fprintf (outfile, "int __isrc=");                                                    \
    src;                                                                                 \
    fprintf (outfile, ";\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "int __idest=");                                                   \
    dest;                                                                                \
    fprintf (outfile, ";\n\n")

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

#define InitIMaxs(from, to, v_i_str) InitVecs (from, to, "__imax", v_i_str)

#define InitSrcOffs(from, to, v_i_str) InitVecs (from, to, "__srcoff", v_i_str)

#define FillRes(res, body)                                                               \
    INDENT;                                                                              \
    fprintf (outfile, "do {\n");                                                         \
    indent++;                                                                            \
    body;                                                                                \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "while( __idest<ND_A_SIZE(%s));\n", res)

#define AccessSeg(dim, body)                                                             \
    {                                                                                    \
        int i;                                                                           \
        INDENT;                                                                          \
        fprintf (outfile, "{\n");                                                        \
        indent++;                                                                        \
        for (i = 1; i < dim; i++) {                                                      \
            INDENT;                                                                      \
            fprintf (outfile, "int __i%d;\n", i);                                        \
        }                                                                                \
        for (i = 1; i < dim; i++) {                                                      \
            INDENT;                                                                      \
            fprintf (outfile, "for( __i%d=0; __i%d<__imax%d; __i%d++) {\n", i, i, i, i); \
            indent++;                                                                    \
        }                                                                                \
        body;                                                                            \
        for (i = dim - 1; i > 0; i--) {                                                  \
            indent--;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "}\n");                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "__isrc += __srcoff%d;\n", i);                             \
        }                                                                                \
        indent--;                                                                        \
        INDENT;                                                                          \
        fprintf (outfile, "}\n");                                                        \
    }

#define CopyBlock(a, offset, res)                                                        \
    NewBlock (InitPtr (offset, fprintf (outfile, "0")),                                  \
              FillRes (res, INDENT;                                                      \
                       fprintf (outfile,                                                 \
                                "ND_A_FIELD(%s)[__idest++]=ND_A_FIELD(%s)[__isrc++];\n", \
                                res, a);))

/*
 * TakeSeg(a, dima, offset, dimi, sz_i_str, off_i_str, res)
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
              InitIMaxs (dimi, dima, fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)", a, i));  \
              InitSrcOffs (                                                              \
                0, dimi, off_i_str; {                                                    \
                    int j;                                                               \
                    for (j = i + 1; j < dima; j++)                                       \
                        fprintf (outfile, "*ND_KD_A_SHAPE(%s, %d)", a, j);               \
                }) InitSrcOffs (dimi, dima, fprintf (outfile, "0")),                     \
              FillRes (res,                                                              \
                       AccessSeg (dima, INDENT; fprintf (outfile,                        \
                                                         "ND_A_FIELD(%s)[__idest++]=ND_" \
                                                         "A_FIELD(%s)[__isrc++];\n",     \
                                                         res, a);)))

#ifdef TEST_BACKEND

int
main ()
{

    typedef char *string;

    FILE *outfile = stdout;
    char type[] = "double";
    char name[] = "array_name";
    int dim = 3, dima = 4, dimv = 3;
    char *s[] = {"40", "50", "60"};
    char *vi[] = {"10", "20", "30"};
    char v[] = "vector";
    char a[] = "arg";
    char a1[] = "arg1";
    char a2[] = "arg2";
    char res[] = "result";
    int i;
    int indent = 1;

#else /* TEST_BACKEND */

extern FILE *outfile; /* outputfile for PrintTree defined in main.c*/

#endif /* TEST_BACKEND */

    /*
     * ND_CREATE_CONST_ARRAY( basic_type, name, dim, s0,..., sn)   : generates a constant
     * array
     *
     * char *type, *name;
     * int dim;
     * char **s;
     */

#define ND_CREATE_CONST_ARRAY

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"

    INDENT;
    NewBlock (fprintf (outfile, "static %s __dummy[]={", type), {
        int i;
        for (i = 0; i < dim - 1; i++)
            fprintf (outfile, "%s,", s[i]);
        fprintf (outfile, "%s};\n", s[i]);
        INDENT;
        fprintf (outfile, "%s=__dummy;\n", name);
    });
    fprintf (outfile, "\n");

#undef ND_CREATE_CONST_ARRAY

#ifndef TEST_BACKEND
    DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KS_DECL_ARRAY( basic_type, name, dim, s0,..., sn)   : declares an array
 *
 * char *type, *name;
 * int dim;
 * char **s;
 */

#define ND_KS_DECL_ARRAY

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"

INDENT;
fprintf (outfile, "%s *%s;\n", type, name);
INDENT;
fprintf (outfile, "int *__%s_rc;\n", name);
INDENT;
fprintf (outfile, "int __%s_sz=", name);
fprintf (outfile, "%s", s[0]);
{
    int i;
    for (i = 1; i < dim; i++)
        fprintf (outfile, "*%s", s[i]);
    fprintf (outfile, ";\n");
    INDENT;
    fprintf (outfile, "int __%s_d=%d;\n", name, dim);
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "int __%s_s%d=%s;\n", name, i, s[i]);
    }
}
fprintf (outfile, "\n");

#undef ND_KS_DECL_ARRAY

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_SET_SHAPE( name, dim, s0,..., sn)        : sets all shape components of an array
 *
 * char *name;
 * int dim;
 * char **s;
 */

#define ND_KD_SET_SHAPE

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"

{
    int i;
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)=%s;\n", name, i, s[i]);
    }
}
fprintf (outfile, "\n");

#undef ND_KD_SET_SHAPE

#ifndef TEST_BACKEND
free (s);
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_PSI_CxA_S( a, res, dim, v0,..., vn): selects a single element of the array
 *
 *
 * char *type, *a, *res;
 * int dim;
 * char **vi;
 */

#define ND_KD_PSI_CxA_S

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"

INDENT;
fprintf (outfile, "%s=ND_A_FIELD(%s)[", res, a);
VectToOffset (dim, AccessConst (vi, i), dim, a);
fprintf (outfile, "];\n\n");

#undef ND_KD_PSI_CxA_S

#ifndef TEST_BACKEND
free (vi);
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_PSI_VxA_S( a, res, dim, v ) : selects a single element of the array
 *
 * char *a, *res, *v;
 * int dim;
 */

#define ND_KD_PSI_VxA_S

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"

INDENT;
fprintf (outfile, "%s=ND_A_FIELD(%s)[", res, a);
VectToOffset (dim, AccessVect (v, i), dim, a);
fprintf (outfile, "];\n\n");

#undef ND_KD_PSI_VxA_S

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_PSI_CxA_A( dima, a, res, dimv, v0,..., vn): selects a sub-array
 *
 * int dima;
 * char *a, *res;
 * int dimv;
 * char **vi;
 */

#define ND_KD_PSI_CxA_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"

INDENT;
CopyBlock (a, VectToOffset (dimv, AccessConst (vi, i), dima, a), res);
fprintf (outfile, "\n\n");

#undef ND_KD_PSI_CxA_A

#ifndef TEST_BACKEND
free (vi);
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_PSI_VxA_A( dima, a, res, dimv, v )       : selects a sub-array
 *
 * int dima;
 * char *a, *res, *v;
 * int dimv;
 */

#define ND_KD_PSI_VxA_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"

INDENT;
CopyBlock (a, VectToOffset (dimv, AccessVect (v, i), dima, a), res);
fprintf (outfile, "\n\n");

#undef ND_KD_PSI_VxA_A

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_TAKE_CxA_A( dima, a, res, dimv, v0,..., vn):
 */

#define ND_KD_TAKE_CxA_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"

INDENT;
TakeSeg (a, dima, fprintf (outfile, "0"), /* offset */
         dimv,                            /* dim of sizes & offsets */
         AccessConst (vi, i),             /* sizes */
         fprintf (outfile, "(ND_KD_A_SHAPE(%s, %d) - ", a, i);
         AccessConst (vi, i); fprintf (outfile, ")"), /* offsets */
                              res);

fprintf (outfile, "\n\n");

#undef ND_KD_TAKE_CxA_A

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_DROP_CxA_A( dima, a, res, dimv, v0,..., vn):
 */

#define ND_KD_DROP_CxA_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"

INDENT;
TakeSeg (a, dima, VectToOffset (dimv, AccessConst (vi, i), dima, a), /* offset */
         dimv, /* dim of sizes & offsets */
         fprintf (outfile, "ND_KD_A_SHAPE(%s, %d) - ", a, i);
         AccessConst (vi, i), /* sizes */
         AccessConst (vi, i), /* offsets */
         res);

fprintf (outfile, "\n\n");

#undef ND_KD_DROP_CxA_A

#ifdef TEST_BACKEND
return (0);
}
#else  /* TEST_BACKEND */
DBUG_VOID_RETURN;
}
#endif /* TEST_BACKEND */
