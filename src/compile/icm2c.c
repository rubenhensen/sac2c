/*
 *
 * $Log$
 * Revision 1.21  1995/05/24 15:31:22  sbs
 * trace.h included
 *
 * Revision 1.20  1995/05/24  13:58:31  sbs
 * ND_KS_DECL_ARRAY_ARG inserted
 *
 * Revision 1.19  1995/05/04  11:43:51  sbs
 * icm_trace.c inserted
 *
 * Revision 1.18  1995/05/02  07:11:31  sbs
 * with-constructs debugged
 *
 * Revision 1.17  1995/04/27  12:57:05  sbs
 * WITH-LOOP ICM s inserted.
 *
 * Revision 1.16  1995/04/18  14:45:55  sbs
 * ND_CREATE_CONST_ARRAY modified; initial basic_type parameter eliminated.
 *
 * Revision 1.15  1995/04/18  11:20:34  sbs
 * ND_KD_ROT_SxSxA_A eliminated & ND_KD_ROT_CxSxA_A modified
 *
 * Revision 1.14  1995/04/13  09:12:59  sbs
 * ND_KD_ROT_SxSxA_A inserted
 *
 * Revision 1.13  1995/04/12  15:14:00  sbs
 * cat & rot inserted.
 *
 * Revision 1.12  1995/04/12  07:02:53  sbs
 * separation of IN and OUT arrays added
 *
 * Revision 1.11  1995/04/11  16:08:48  sbs
 * some minor bugs fixed
 *
 * Revision 1.10  1995/04/11  15:06:35  sbs
 * arg->tyarg in ND_FUN_DEC
 *
 * Revision 1.9  1995/04/11  15:03:13  sbs
 * ND_FUN_[DEC/AP/RET] inserted
 *
 * Revision 1.8  1995/04/07  12:19:48  sbs
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
#include "main.h"
#include "trace.h"

#define RetWithScal(res, val)                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr++]=%s;\n", res, res, val[0]);

#define RetWithArray(res, a)                                                             \
    INDENT;                                                                              \
    fprintf (outfile, "{ int __i;\n\n");                                                 \
    indent++;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "for(__i=0; __i<ND_A_SIZE(%s); __i++)\n", a);                      \
    indent++;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr++]=ND_A_FIELD(%s)[__i];\n", res, res,  \
             a);                                                                         \
    indent -= 2;                                                                         \
    INDENT;                                                                              \
    fprintf (outfile, "}\n")

#define BeginWith(res, dimres, from, to, idx, idxlen, fillstr)                           \
    INDENT;                                                                              \
    fprintf (outfile, "{ int __i;\n");                                                   \
    indent++;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "int %s__destptr=0;\n", res);                                      \
    INDENT;                                                                              \
    fprintf (outfile, "int __start=");                                                   \
    VectToOffset (idxlen, AccessVect (from, i), dimres, res);                            \
    fprintf (outfile, ";\n");                                                            \
    {                                                                                    \
        int i, j;                                                                        \
        for (i = 1; i < idxlen; i++) {                                                   \
            INDENT;                                                                      \
            fprintf (outfile, "int %s__offset%d=", res, i);                              \
            fprintf (outfile, "(ND_KD_A_SHAPE(%s, %d)-ND_A_FIELD(%s)[%d]", res, i, to,   \
                     i);                                                                 \
            fprintf (outfile, "+ND_A_FIELD(%s)[%d]-1)", from, i);                        \
            for (j = (i + 1); j < dimres; j++)                                           \
                fprintf (outfile, " *ND_KD_A_SHAPE(%s, %d)", res, j);                    \
            fprintf (outfile, ";\n");                                                    \
        }                                                                                \
        fprintf (outfile, "\n");                                                         \
        INDENT;                                                                          \
        fprintf (outfile, "while( %s__destptr < __start) {\n", res);                     \
        indent++;                                                                        \
        INDENT;                                                                          \
        fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr]=", res, res);                     \
        fillstr;                                                                         \
        fprintf (outfile, ";\n");                                                        \
        INDENT;                                                                          \
        fprintf (outfile, "%s__destptr++;\n", res);                                      \
        indent--;                                                                        \
        INDENT;                                                                          \
        fprintf (outfile, "}\n");                                                        \
        {                                                                                \
            int i;                                                                       \
            for (i = 0; i < idxlen; i++) {                                               \
                INDENT;                                                                  \
                fprintf (outfile,                                                        \
                         "for( ND_A_FIELD(%s)[%d]=ND_A_FIELD(%s)[%d]; "                  \
                         "ND_A_FIELD(%s)[%d]<=ND_A_FIELD(%s)[%d]; "                      \
                         "ND_A_FIELD(%s)[%d]++) {\n",                                    \
                         idx, i, from, i, idx, i, to, i, idx, i);                        \
                indent++;                                                                \
            }                                                                            \
        }                                                                                \
        fprintf (outfile, "\n");                                                         \
    }

#define EndWith(res, dimres, idxlen, fillstr)                                            \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n");                                                            \
    {                                                                                    \
        int i;                                                                           \
        for (i = idxlen - 1; i > 0; i--) {                                               \
            INDENT;                                                                      \
            fprintf (outfile, "for(__i=0; __i<%s__offset%d; __i++) {\n", res, i);        \
            indent++;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr]=", res, res);                 \
            fillstr;                                                                     \
            fprintf (outfile, ";\n");                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "%s__destptr++;\n", res);                                  \
            indent--;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "}\n");                                                    \
            indent--;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "}\n");                                                    \
        }                                                                                \
    }                                                                                    \
    INDENT;                                                                              \
    fprintf (outfile, "while( %s__destptr< ND_A_SIZE(%s)) {\n", res, res);               \
    indent++;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr]=", res, res);                         \
    fillstr;                                                                             \
    fprintf (outfile, ";\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "%s__destptr++;\n", res);                                          \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n");                                                            \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n\n")

#define FirstOut(arg, n, body, default, step)                                            \
    {                                                                                    \
        int i = 0;                                                                       \
        int out = 1;                                                                     \
        while (out && (i < n)) {                                                         \
            if (strcmp (arg[i++], "out") == 0) {                                         \
                body;                                                                    \
                out = 0;                                                                 \
            }                                                                            \
            i += step;                                                                   \
        }                                                                                \
        if (out)                                                                         \
        default;                                                                         \
    }
#define ScanArglist(arg, n, bin, bfout, bout, binout, bina, bouta, binouta, sepstr)      \
    {                                                                                    \
        int i = 0;                                                                       \
        int out = 0;                                                                     \
        int sep = 0;                                                                     \
        while (i < n) {                                                                  \
            if (sep)                                                                     \
                fprintf (outfile, "%s", sepstr);                                         \
            DBUG_PRINT ("PRINT", ("arg-index: %d, arg-tag : %s", i, arg[i]));            \
            if (strcmp (arg[i], "in") == 0) {                                            \
                i++;                                                                     \
                bin;                                                                     \
            } else if (strcmp (arg[i], "out") == 0) {                                    \
                i++;                                                                     \
                if (out) {                                                               \
                    bout;                                                                \
                } else {                                                                 \
                    out = 1;                                                             \
                    bfout;                                                               \
                }                                                                        \
            } else if (strcmp (arg[i], "inout") == 0) {                                  \
                i++;                                                                     \
                binout;                                                                  \
            } else if (strcmp (arg[i], "in_a") == 0) {                                   \
                i++;                                                                     \
                bina;                                                                    \
            } else if (strcmp (arg[i], "out_a") == 0) {                                  \
                i++;                                                                     \
                bouta;                                                                   \
            } else {                                                                     \
                i++;                                                                     \
                binouta;                                                                 \
            }                                                                            \
        }                                                                                \
    }
#define AccessVect(v, i) fprintf (outfile, "ND_A_FIELD(%s)[%i]", v, i)

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
    init;                                                                                \
    body;                                                                                \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n")

#define InitPtr(src, dest)                                                               \
    INDENT;                                                                              \
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

MAIN
{

    typedef char *string;

    FILE *outfile = stdout;
    char type[] = "double";
    char name[] = "array_name";
    int dim = 3, dima = 4, dimv = 3, dimres = 4;
    char *s[] = {"40", "50", "60"};
    char *vi[] = {"10", "20", "30"};
    char *ar[] = {"firstarray", "secondarray"};
    char *arg[] = {"in", "i", "in_a", "ia", "out", "o1", "out", "o2", "out_a", "oa"};
    char *tyarg[] = {"in", "int", "i",   "in_a", "int",   "ia",  "out", "int",
                     "o1", "out", "int", "o2",   "out_a", "int", "oa"};
    char *rotdimstr[] = {"rotindim"};
    char *numstr[] = {"rotnum"};
    char *valstr[] = {"ret-val"};
    char v[] = "vector";
    char a[] = "arg";
    char reta[] = "ret-array";
    char a1[] = "arg1";
    char from[] = "fromvar";
    char to[] = "tovar";
    char idx[] = "idxvar";
    char a2[] = "arg2";
    char res[] = "result";
    int idxlen = 3;
    int i;
    int narg = 5;
    int indent = 1;
    int catdim = 2;
    int rotdim = 1;

    int traceflag = 0xffff;

    OPT OTHER
    {
    }
    ENDOPT

#else /* TEST_BACKEND */

extern FILE *outfile; /* outputfile for PrintTree defined in main.c*/

#endif /* TEST_BACKEND */

    /*
     * ND_FUN_DEC( name, n, [ {in, in_a, out, out_a, inout, inout_a}, type, arg ] )
     *
     * char *name;
     * int narg;
     * char **tyarg;
     */

#define ND_FUN_DEC

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

    INDENT;
    FirstOut (tyarg, 3 * narg, fprintf (outfile, "%s ", tyarg[i]),
              fprintf (outfile, "void "), 2);
    fprintf (outfile, "%s( ", name);
    ScanArglist (tyarg, 3 * narg, fprintf (outfile, " %s %s", tyarg[i++], tyarg[i++]);
                 sep = 1, i += 2;
                 sep = 0, fprintf (outfile, " %s *%s__p", tyarg[i++], tyarg[i++]);
                 sep = 1, fprintf (outfile, " %s *%s__p", tyarg[i++], tyarg[i++]);
                 sep = 1,
                 fprintf (outfile, " ND_KS_DEC_IN_ARRAY(%s, %s)", tyarg[i++], tyarg[i++]);
                 sep = 1, fprintf (outfile, " ND_KS_DEC_OUT_ARRAY(%s, %s)", tyarg[i++],
                                   tyarg[i++]);
                 sep = 1, fprintf (outfile, " ND_KS_DEC_OUT_ARRAY(%s, %s)", tyarg[i++],
                                   tyarg[i++]);
                 sep = 1, ",");
    fprintf (outfile, ")\n");

#undef ND_FUN_DEC

#ifndef TEST_BACKEND
    DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_FUN_AP( name, n, [ {in, in_a, out, out_a, inout, inout_a}, arg ] )
 *
 * char *name;
 * int narg;
 * char **arg;
 */

#define ND_FUN_AP

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
FirstOut (arg, 2 * narg, fprintf (outfile, "%s =", arg[i]), {}, 1);
fprintf (outfile, "%s( ", name);
ScanArglist (arg, 2 * narg, fprintf (outfile, " %s", arg[i++]); sep = 1, i++; sep = 0;
             , fprintf (outfile, " &%s", arg[i++]);
             sep = 1, fprintf (outfile, " &%s", arg[i++]);
             sep = 1, fprintf (outfile, " ND_KS_AP_IN_ARRAY(%s)", arg[i++]);
             sep = 1, fprintf (outfile, " ND_KS_AP_OUT_ARRAY(%s)", arg[i++]);
             sep = 1, fprintf (outfile, " ND_KS_AP_OUT_ARRAY(%s)", arg[i++]);
             sep = 1, ",");
fprintf (outfile, ");\n");

#undef ND_FUN_AP

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_FUN_RET( n, [ { out, out_a, inout, inout_a}, arg ] )
 *
 * char *name;
 * int narg;
 * char **arg;
 */

#define ND_FUN_RET

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
ScanArglist (arg, 2 * narg, i++; sep = 0, i++;
             sep = 0, fprintf (outfile, "*%s__p = %s;\n", arg[i], arg[i++]); INDENT;
             sep = 1, fprintf (outfile, "*%s__p = %s;\n", arg[i], arg[i++]); INDENT;
             sep = 1, i += 1;
             sep = 0, fprintf (outfile, "ND_KS_RET_OUT_ARRAY(%s);\n", arg[i++]); INDENT;
             sep = 1, fprintf (outfile, "ND_KS_RET_OUT_ARRAY(%s);\n", arg[i++]); INDENT;
             sep = 1, "");
FirstOut (arg, 2 * narg, fprintf (outfile, "return(%s);\n", arg[i]), {}, 1);

#undef ND_FUN_RET

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_CREATE_CONST_ARRAY( name, dim, s0,..., sn)   : generates a constant array
 *
 * char *name;
 * int dim;
 * char **s;
 */

#define ND_CREATE_CONST_ARRAY

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

{
    int i;
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "%s[%d]=%s;\n", name, i, s[i]);
    }
}

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
#include "icm_trace.c"

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
 * ND_KS_DECL_ARRAY_ARG( name, dim, s0,..., sn)   : declares an array given as arg
 *
 * char *name;
 * int dim;
 * char **s;
 */

#define ND_KS_DECL_ARRAY_ARG

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

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

#undef ND_KS_DECL_ARRAY_ARG

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
#include "icm_trace.c"

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
#include "icm_trace.c"

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
#include "icm_trace.c"

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
#include "icm_trace.c"

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
#include "icm_trace.c"

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
#include "icm_trace.c"

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
#include "icm_trace.c"

INDENT;
TakeSeg (a, dima, VectToOffset (dimv, AccessConst (vi, i), dima, a), /* offset */
         dimv, /* dim of sizes & offsets */
         fprintf (outfile, "ND_KD_A_SHAPE(%s, %d) - ", a, i);
         AccessConst (vi, i), /* sizes */
         AccessConst (vi, i), /* offsets */
         res);

fprintf (outfile, "\n\n");

#undef ND_KD_DROP_CxA_A

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_CAT_SxAxA_A( dima, ar0, ar1, res, catdim):
 */

#define ND_KD_CAT_SxAxA_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
NewBlock (
  InitPtr (fprintf (outfile, "0"), fprintf (outfile, "0"));
  InitVecs (1, 2, "__isrc", fprintf (outfile, "0"));
  InitVecs (0, 2, "__i", fprintf (outfile, "0"));
  InitVecs (0, 2, "__bl",
            {
                int j;
                for (j = catdim; j < dima; j++)
                    fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)*", ar[i], j);
                fprintf (outfile, "1");
            }),
  FillRes (res, INDENT; fprintf (outfile, "for(__i0=0; __i0<__bl0; __i0++)\n"); indent++;
           INDENT;
           fprintf (outfile, "ND_A_FIELD(%s)[__idest++] = ND_A_FIELD(%s)[__isrc++];\n",
                    res, ar[0]);
           indent--; INDENT; fprintf (outfile, "for(__i1=0; __i1<__bl1; __i1++)\n");
           indent++; INDENT;
           fprintf (outfile, "ND_A_FIELD(%s)[__idest++] = ND_A_FIELD(%s)[__isrc1++];\n",
                    res, ar[1]);
           indent--; INDENT));
fprintf (outfile, "\n\n");

#undef ND_KD_CAT_SxAxA_A

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_ROT_CxSxA_A( rotdim, numstr, dima, a, res):
 */

#define ND_KD_ROT_CxSxA_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
NewBlock (
  InitVecs (0, 1, "__shift",
            {
                int j;
                for (j = rotdim + 1; j < dima; j++)
                    fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)*", a, j);
                fprintf (outfile, "(%s<0 ? ", numstr[0]);
                fprintf (outfile,
                         "ND_KD_A_SHAPE(%s, %d)+ (%s %% ND_KD_A_SHAPE(%s, %d)) : ", a,
                         rotdim, numstr[0], a, rotdim);
                fprintf (outfile, "%s %% ND_KD_A_SHAPE(%s, %d))", numstr[0], a, rotdim);
            });
  InitVecs (0, 1, "__bl",
            {
                int j;
                for (j = rotdim + 1; j < dima; j++)
                    fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)*", a, j);
                fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)", a, rotdim);
            });
  InitVecs (0, 1, "__i", fprintf (outfile, "0"));
  InitPtr (fprintf (outfile, "-__shift0"), fprintf (outfile, "0")),
  FillRes (
    res, INDENT; fprintf (outfile, "__isrc+=__bl0;\n"); INDENT;
    fprintf (outfile, "for(__i0=0; __i0<__shift0; __i0++)\n"); indent++; INDENT;
    fprintf (outfile, "ND_A_FIELD(%s)[__idest++] = ND_A_FIELD(%s)[__isrc++];\n", res, a);
    indent--; INDENT; fprintf (outfile, "__isrc-=__bl0;\n"); INDENT;
    fprintf (outfile, "for(; __i0<__bl0; __i0++)\n"); indent++; INDENT;
    fprintf (outfile, "ND_A_FIELD(%s)[__idest++] = ND_A_FIELD(%s)[__isrc++];\n", res, a);
    indent--));
fprintf (outfile, "\n\n");

#undef ND_KD_ROT_CxSxA_A

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_BEGIN_GENARRAY( res, dimres, from, to, idx, idxlen)
 */

#define ND_BEGIN_GENARRAY

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

BeginWith (res, dimres, from, to, idx, idxlen, fprintf (outfile, "0"));

#ifdef TEST_BACKEND
indent -= idxlen + 1;
#endif /* TEST_BACKEND */

#undef ND_BEGIN_GENARRAY

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_BEGIN_MODARRAY( res, dimres, a, from, to, idx, idxlen)
 */

#define ND_BEGIN_MODARRAY

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

BeginWith (res, dimres, from, to, idx, idxlen,
           fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr]", a, res));

#ifdef TEST_BACKEND
indent -= idxlen + 1;
#endif /* TEST_BACKEND */

#undef ND_BEGIN_MODARRAY

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_END_GENARRAY_S( res, dimres, valstr):
 */

#define ND_END_GENARRAY_S

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

#ifdef TEST_BACKEND
indent += dimres + 1;
#endif /* TEST_BACKEND */

RetWithScal (res, valstr);
EndWith (res, dimres, dimres, fprintf (outfile, "0"));

#undef ND_END_GENARRAY_S

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_END_GENARRAY_A( res, dimres, reta, idxlen):
 */

#define ND_END_GENARRAY_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

#ifdef TEST_BACKEND
indent += idxlen + 1;
#endif /* TEST_BACKEND */

RetWithArray (res, reta);
EndWith (res, dimres, idxlen, fprintf (outfile, "0"));

#undef ND_END_GENARRAY_A

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_END_MODARRAY_S( res, dimres, a, valstr):
 */

#define ND_END_MODARRAY_S

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

#ifdef TEST_BACKEND
indent += dimres + 1;
#endif /* TEST_BACKEND */

RetWithScal (res, valstr);
EndWith (res, dimres, dimres, fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr]", a, res));

#undef ND_END_MODARRAY_S

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_END_MODARRAY_A( res, dimres, a, reta, idxlen):
 */

#define ND_END_MODARRAY_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

#ifdef TEST_BACKEND
indent += idxlen + 1;
#endif /* TEST_BACKEND */

RetWithArray (res, reta);
EndWith (res, dimres, idxlen, fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr]", a, res));

#undef ND_END_MODARRAY_A

#ifdef TEST_BACKEND
return (0);
}
#else  /* TEST_BACKEND */
DBUG_VOID_RETURN;
}
#endif /* TEST_BACKEND */
