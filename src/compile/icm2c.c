/*
 *
 * $Log$
 * Revision 1.1  1995/03/07 18:21:02  sbs
 * Initial revision
 *
 *
 */

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

#define ConstVectToOffset(dim, v, a)                                                     \
    {                                                                                    \
        int i;                                                                           \
        for (i = dim - 1; i > 0; i--)                                                    \
            fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)*(", a, i);                          \
        fprintf (outfile, "%s ", v[0]);                                                  \
        for (i = 1; i < dim; i++)                                                        \
            fprintf (outfile, "+%s) ", v[i]);                                            \
    }

#define VectToOffset(dim, v, a)                                                          \
    {                                                                                    \
        int i;                                                                           \
        fprintf (outfile, "[");                                                          \
        for (i = dim - 1; i > 0; i--)                                                    \
            fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)*(", a, i);                          \
        fprintf (outfile, "%s ", v[0]);                                                  \
        for (i = 1; i < dim; i++)                                                        \
            fprintf (outfile, "+ND_A_FIELD(%s)[%i]) ", v, i);                            \
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
        fprintf(outfile, "})

/*

 ND_KS_DECL_ARRAY( basic_type, name, dim, s0,..., sn)   : declares an array

char *type, *name;
int dim;
char **s;

int i;

  GetNextId( type, exprs);
  GetNextId( name, exprs);
  GetNextInt( dim, exprs);
  GetShape(dim, s, exprs);

  INDENT;
  fprintf( outfile, "%s *%s;\n", type, name);
  INDENT;
  fprintf( outfile, "int __%s_rc=1;\n", name);
  INDENT;
  fprintf( outfile, "int __%s_sz=", name);
  fprintf( outfile, "%s", s[0]);
  for(i=1; i<dim; i++)
    fprintf( outfile, "*%s", s[i]);
  fprintf( outfile, ";\n", s[i]);
  INDENT;
  fprintf( outfile, "int __%s_d=%d;\n", name, dim);
  for(i=0; i<dim; i++) {
    INDENT;
    fprintf( outfile, "int __%s_s%d=%d;\n", name, i, s[i]);
  }

*/

/*

ND_KD_SET_SHAPE( name, dim, s0,..., sn)        : sets all shape components of an array

char *name;
int dim;
char **s;

int i;

  GetNextId( name, exprs);
  GetNextInt( dim, exprs);
  GetShape(dim, s, exprs);

  for(i=0; i<dim; i++) {
    INDENT;
    fprintf( outfile, "int __%s_s%d=%d;\n", name, i, s[i]);
  }


*/

/*

ND_KD_PSI_CxA_S( type, a, res, dim, v0,..., vn): selects a single element of the array


char *type, *a, *res;
int dim;
char **v;

  INDENT;
  GetNextId( type, exprs);
  GetNextId( a, exprs);
  GetNextId( res, exprs);
  GetNextInt( dim, exprs);
  GetShape(dim, v, exprs);

  fprintf("%s=ND_A_FIELD(%s)[", res, a);
  ConstVectToOffset(dim, v, a);
  fprintf("];\n");

  free(v);

*/
/*

ND_KD_PSI_VxA_S( type, a, res, dim, v ) : selects a single element of the array

char *type, *a, *res, *v;
int dim;

  INDENT;
  GetNextId( type, exprs);
  GetNextId( a, exprs);
  GetNextId( res, exprs);
  GetNextInt( dim, exprs);
  GetNextId( v, exprs);

  fprintf("%s=ND_A_FIELD(%s)[", res, a);
  VectToOffset(dim, v, a);
  fprintf("];\n");

  free(v);

*/

/*

ND_KD_PSI_CxA_A_NEW( type, a, res, dim, v0,..., vn): selects a sub-array


char *type, *a, *res;
int dim;
char **v;

  INDENT;
  GetNextId( type, exprs);
  GetNextId( a, exprs);
  GetNextId( res, exprs);
  GetNextInt( dim, exprs);
  GetShape(dim, v, exprs);

  INDENT;
  fprintf(outfile, "ND_ALLOC_ARRAY(%s, %s);\n", type, res);
  INDENT;
  CopyBlock(a, ConstVectToOffset(dim, v, a), res)

  free(v);

*/

/*

ND_KD_PSI_CxA_A_OLD( type, a, res, dim, v0,..., vn): selects a sub-array


char *type, *a, *res;
int dim;
char **v;

  INDENT;
  GetNextId( type, exprs);
  GetNextId( a, exprs);
  GetNextId( res, exprs);
  GetNextInt( dim, exprs);
  GetShape(dim, v, exprs);

  INDENT;
  CopyBlock(a, ConstVectToOffset(dim, v, a), res)

  free(v);

*/
/*

 ND_KD_PSI_VxA_A_NEW( type, a, res, dim, v )       : selects a sub-array

char *type, *a, *res, *v;
int dim;

  INDENT;
  GetNextId( type, exprs);
  GetNextId( a, exprs);
  GetNextId( res, exprs);
  GetNextInt( dim, exprs);
  GetNextId( v, exprs);

  INDENT;
  fprintf(outfile, "ND_ALLOC_ARRAY(%s, %s);\n", type, res);
  INDENT;
  CopyBlock(a, VectToOffset(dim, v, a), res)

  free(v);

*/

/*

 ND_KD_PSI_VxA_A_OLD( type, a, res, dim, v )       : selects a sub-array

char *type, *a, *res, *v;
int dim;

  INDENT;
  GetNextId( type, exprs);
  GetNextId( a, exprs);
  GetNextId( res, exprs);
  GetNextInt( dim, exprs);
  GetNextId( v, exprs);

  INDENT;
  CopyBlock(a, VectToOffset(dim, v, a), res)

  free(v);

*/
