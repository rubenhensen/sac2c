/*
 *
 * $Log$
 * Revision 1.2  1995/03/08 17:13:08  sbs
 * first version with test embedded
 *
 * Revision 1.1  1995/03/07  18:21:02  sbs
 * Initial revision
 *
 *
 */

#include "stdio.h"
#include "print.h"
#include "dbug.h"

#define GetNextId(res, ex)                                                               \
    {                                                                                    \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
        DBUG_ASSERT ((ex->node[1]->nodetype == N_id), "wrong icm-arg: N_id expected");   \
        res = ex->node[1]->info.id;                                                      \
        exprsp = ex->node[0];                                                            \
    }

#define GetNextInt(res, ex)                                                              \
    {                                                                                    \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
        DBUG_ASSERT ((ex->node[1]->nodetype == N_num), "wrong icm-arg: N_num expected"); \
        res = ex->node[1]->info.cint;                                                    \
        exprsp = ex->node[0];                                                            \
    }

#define GetNextFloat(res, ex)                                                            \
    {                                                                                    \
        DBUG_ASSERT ((ex->nodetype == N_exprs), "wrong icm-arg: N_exprs expected");      \
        DBUG_ASSERT ((ex->node[1]->nodetype == N_float),                                 \
                     "wrong icm-arg: N_float expected");                                 \
        res = ex->node[1]->info.cfloat;                                                  \
        exprsp = ex->node[0];                                                            \
    }

#define GetShape(dim, v, ex)                                                             \
    {                                                                                    \
        int i;                                                                           \
        v = (char **)malloc (size (char *) * dim);                                       \
        for (i = 0; i < dim; i++)                                                        \
            GetNextId (v[i], ex);                                                        \
    }

#define BEGIN_COMMENT fprintf (outfile, "/*\n *")
#define END_COMMENT fprintf (outfile, " */\n")

#define AccessVect(v, i) fprintf (outfile, "ND_A_FIELD(%s)[%i])", v, i)

#define AccessConst(v, i) fprintf (outfile, "%s", v[i])

#define VectToOffset(dim, v_i_str, a)                                                    \
    {                                                                                    \
        int i;                                                                           \
        for (i = dim - 1; i > 0; i--)                                                    \
            fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)* (", a, i);                         \
        v_i_str;                                                                         \
        for (i = 1; i < dim; i++) {                                                      \
            fprintf (outfile, "+");                                                      \
            v_i_str;                                                                     \
            fprintf (outfile, ") ");                                                     \
        }                                                                                \
    }

#define CopyBlock(a, offset, res)                                                        \
    fprintf (outfile, "{\n");                                                            \
    indent++;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "int __isrc=");                                                    \
    offset;                                                                              \
    fprintf (outfile, ";\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "int __idest=0;\n\n");                                             \
    INDENT;                                                                              \
    fprintf (outfile, "while( __idest<ND_A_SIZE(%s))\n", res);                           \
    indent++;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "ND_A_FIELD(%s)[__idest++]=ND_A_FIELD(%s)[__isrc++];\n", res, a);  \
    indent -= 2;                                                                         \
    INDENT;                                                                              \
    fprintf (outfile, "}")

#ifdef TEST_BACKEND

int
main ()
{

    typedef char *string;

    FILE *outfile = stdout;
    char type[] = "double";
    char name[] = "array_name";
    int dim = 3;
    char *s[] = {"40", "50", "60"};
    char *vi[] = {"10", "20", "30"};
    char v[] = "vector";
    char a[] = "arg";
    char a1[] = "arg1";
    char a2[] = "arg2";
    char res[] = "result";
    int i;
    int indent = 1;

#endif /* TEST_BACKEND */

    /*
     * ND_KS_DECL_ARRAY( basic_type, name, dim, s0,..., sn)   : declares an array
     *
     * char *type, *name;
     * int dim;
     * char **s;
     */

#ifndef TEST_BACKEND
    int i;

    GetNextId (type, exprs);
    GetNextId (name, exprs);
    GetNextInt (dim, exprs);
    GetShape (dim, s, exprs);
#endif /* no TEST_BACKEND */

    BEGIN_COMMENT;
    fprintf (outfile, " ND_KS_DECL_ARRAY(%s, %s, %d", type, name, dim);
    for (i = 0; i < dim; i++) {
        fprintf (outfile, ", ");
        AccessConst (s, i);
    }
    fprintf (outfile, ")\n");
    END_COMMENT;

    INDENT;
    fprintf (outfile, "%s *%s;\n", type, name);
    INDENT;
    fprintf (outfile, "int *__%s_rc;\n", name);
    INDENT;
    fprintf (outfile, "int __%s_sz=", name);
    fprintf (outfile, "%s", s[0]);
    for (i = 1; i < dim; i++)
        fprintf (outfile, "*%s", s[i]);
    fprintf (outfile, ";\n", s[i]);
    INDENT;
    fprintf (outfile, "int __%s_d=%d;\n", name, dim);
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "int __%s_s%d=%s;\n", name, i, s[i]);
    }
    fprintf (outfile, "\n");

    /*
     * ND_KD_SET_SHAPE( name, dim, s0,..., sn)        : sets all shape components of an
     * array
     *
     * char *name;
     * int dim;
     * char **s;
     */

#ifndef TEST_BACKEND
    int i;

    GetNextId (name, exprs);
    GetNextInt (dim, exprs);
    GetShape (dim, s, exprs);
#endif /* no TEST_BACKEND */

    BEGIN_COMMENT;
    fprintf (outfile, " ND_KD_SET_SHAPE( %s, %d", name, dim);
    for (i = 0; i < dim; i++) {
        fprintf (outfile, ", ");
        AccessConst (s, i);
    }
    fprintf (outfile, ")\n");
    END_COMMENT;

    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)=%s;\n", name, i, s[i]);
    }
    fprintf (outfile, "\n");

#ifndef TEST_BACKEND
    free (s);
#endif /* no TEST_BACKEND */

    /*
     * ND_KD_PSI_CxA_S( a, res, dim, v0,..., vn): selects a single element of the array
     *
     *
     * char *type, *a, *res;
     * int dim;
     * char **vi;
     */

#ifndef TEST_BACKEND
    fprintf (outfile, "\n--> ND_KD_PSI_CxA_S( %s, %s, %d, %s, %s, %s)\n", a, res, dim,
             vi[0], vi[1], vi[2]);
#else  /* TEST_BACKEND */
GetNextId (type, exprs);
GetNextId (a, exprs);
GetNextId (res, exprs);
GetNextInt (dim, exprs);
GetShape (dim, vi, exprs);
#endif /* TEST_BACKEND */

    INDENT;
    fprintf (outfile, "%s=ND_A_FIELD(%s)[", res, a);
    VectToOffset (dim, AccessConst (vi, i), a);
    fprintf (outfile, "];\n");

#ifndef TEST_BACKEND
    free (v);
#endif /* no TEST_BACKEND */

    /*
     * ND_KD_PSI_VxA_S( type, a, res, dim, v ) : selects a single element of the array
     *
     * char *type, *a, *res, *v;
     * int dim;
     */

#ifdef TEST_BACKEND
    fprintf (outfile, "\n--> ND_KD_PSI_VxA_S( %s, %s, %s, %d, %s)\n", type, a, res, dim,
             v);
#else  /* TEST_BACKEND */
GetNextId (type, exprs);
GetNextId (a, exprs);
GetNextId (res, exprs);
GetNextInt (dim, exprs);
GetNextId (v, exprs);
#endif /* TEST_BACKEND */

    INDENT;
    fprintf (outfile, "%s=ND_A_FIELD(%s)[", res, a);
    VectToOffset (dim, AccessVect (v, i), a);
    fprintf (outfile, "];\n");

#ifndef TEST_BACKEND
    free (v);
#endif /* no TEST_BACKEND */

    /*
     * ND_KD_PSI_CxA_A( type, a, res, dim, v0,..., vn): selects a sub-array
     *
     * char *type, *a, *res;
     * int dim;
     * char **vi;
     */

#ifdef TEST_BACKEND
    fprintf (outfile, "\n--> ND_KD_PSI_CxA_A( %s, %s, %s, %d, %s, %s, %s)\n", type, a,
             res, dim, vi[0], vi[1], vi[2]);
#else  /* TEST_BACKEND */
GetNextId (type, exprs);
GetNextId (a, exprs);
GetNextId (res, exprs);
GetNextInt (dim, exprs);
GetShape (dim, vi, exprs);
#endif /* TEST_BACKEND */

    INDENT;
    CopyBlock (a, VectToOffset (dim, AccessConst (vi, i), a), res);

#ifndef TEST_BACKEND
    free (v);
#endif /* no TEST_BACKEND */

    /*
     * ND_KD_PSI_VxA_A( type, a, res, dim, v )       : selects a sub-array
     *
     * char *type, *a, *res, *v;
     * int dim;
     */

#ifdef TEST_BACKEND
    fprintf (outfile, "\n--> ND_KD_PSI_VxA_A( %s, %s, %s, %d, %s)\n", type, a, res, dim,
             v);
#else  /* TEST_BACKEND */
GetNextId (type, exprs);
GetNextId (a, exprs);
GetNextId (res, exprs);
GetNextInt (dim, exprs);
GetNextId (v, exprs);
#endif /* TEST_BACKEND */

    INDENT;
    CopyBlock (a, VectToOffset (dim, AccessVect (v, i), a), res);

#ifndef TEST_BACKEND
    free (v);
#endif /* no TEST_BACKEND */

#ifdef TEST_BACKEND
    return (0);
}
#endif /* TEST_BACKEND */
