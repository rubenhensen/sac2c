/*
 *
 * $Log$
 * Revision 3.11  2002/10/29 19:10:02  dkr
 * some macros for code generation added
 *
 * Revision 3.10  2002/10/07 23:36:39  dkr
 * GetAttr() added
 *
 * Revision 3.9  2002/07/24 14:34:48  dkr
 * signature of VectToOffset?() modified
 *
 * Revision 3.8  2002/07/11 17:15:38  dkr
 * SizeId() added
 *
 * Revision 3.7  2002/07/11 13:37:27  dkr
 * function renamed
 *
 * Revision 3.6  2002/07/11 08:42:59  dkr
 * bug in VectToOffset() fixed
 *
 * Revision 3.5  2002/07/10 19:28:26  dkr
 * TAGGED_ARRAYS: access macros are functions now
 *
 * Revision 3.3  2002/07/02 13:05:00  dkr
 * global var 'print_comment' added
 *
 * Revision 3.1  2000/11/20 18:01:14  sacbase
 * new release made
 *
 * Revision 2.4  2000/07/06 16:27:03  dkr
 * ICM ND_KD_A_SHAPE renamed into ND_A_SHAPE
 *
 * Revision 2.3  1999/05/05 09:14:47  jhs
 * VectToOffset2 changed, it prints "0" for empty arrays now.
 * So it is able to handle empty arays now.
 *
 * Revision 2.2  1999/04/12 09:37:48  cg
 * All accesses to C arrays are now performed through the new ICMs
 * ND_WRITE_ARRAY and ND_READ_ARRAY. This allows for an integration
 * of cache simulation as well as boundary checking.
 *
 * Revision 2.1  1999/02/23 12:42:37  sacbase
 * new release made
 *
 * Revision 1.2  1998/05/07 11:27:08  dkr
 * ICM names converted to new naming conventions.
 *
 * Revision 1.1  1998/05/03 13:07:29  dkr
 * Initial revision
 *
 */

#ifndef _icm2c_basic_h_
#define _icm2c_basic_h_

extern int print_comment; /* bool */

#define ASSURE_TYPE_ASS(cond_ass, msg_ass)                                               \
    INDENT;                                                                              \
    ASSURE_TYPE_EXPR (cond_ass, msg_ass);                                                \
    fprintf (outfile, ";\n")

#define ASSURE_TYPE_EXPR(cond_ass, msg_ass)                                              \
    fprintf (outfile, "SAC_ASSURE_TYPE( (");                                             \
    cond_ass fprintf (outfile, "), (\"");                                                \
    msg_ass fprintf (outfile, "\"))")

#define BLOCK(ass)                                                                       \
    INDENT;                                                                              \
    BLOCK_VARDECS (, ass)

#define BLOCK_VARDECS(vardecs, ass)                                                      \
    INDENT;                                                                              \
    BLOCK_VARDECS__NOINDENT (vardecs, ass)

#define BLOCK__NOINDENT(ass) BLOCK_VARDECS__NOINDENT (, ass)

#define BLOCK_VARDECS__NOINDENT(vardecs, ass)                                            \
    fprintf (outfile, "{ ");                                                             \
    vardecs fprintf (outfile, "\n");                                                     \
    indent++;                                                                            \
    ass indent--;                                                                        \
    INDENT;                                                                              \
    fprintf (outfile, "}\n")

#define FOR_LOOP(init, cond, post, ass)                                                  \
    INDENT;                                                                              \
    fprintf (outfile, "for (");                                                          \
    init fprintf (outfile, "; ");                                                        \
    cond fprintf (outfile, "; ");                                                        \
    post fprintf (outfile, ") ");                                                        \
    BLOCK__NOINDENT (ass)

#define COND1(cond, ass)                                                                 \
    INDENT;                                                                              \
    fprintf (outfile, "if (");                                                           \
    cond fprintf (outfile, ") ");                                                        \
    BLOCK__NOINDENT (ass)

#define COND2(cond, then_ass, else_ass)                                                  \
    INDENT;                                                                              \
    fprintf (outfile, "if (");                                                           \
    cond fprintf (outfile, ") ");                                                        \
    BLOCK__NOINDENT (then_ass);                                                          \
    INDENT;                                                                              \
    fprintf (outfile, "else ");                                                          \
    BLOCK__NOINDENT (else_ass)

#ifdef TAGGED_ARRAYS

extern void ReadId (void *nt, char *idx_str, int idx);

extern void ReadScalar (void *scl, char *idx_str, int idx);

extern void ReadScalar_Check (void *scl, char *idx_str, int idx);

extern void ReadConstArray (void *v, char *idx_str, int idx);

extern void DimId (void *nt);

extern void ShapeId (void *nt, char *idx_str, int idx);

extern void SizeId (void *nt);

extern void GetAttr (void *v, int v_attr, void (*v_attr_fun) (void *));

extern void Vect2Offset2 (char *off_any, void *v_any, int v_dim,
                          void (*v_size_fun) (void *),
                          void (*v_read_fun) (void *, char *, int), void *a_any,
                          int a_dim, void (*a_dim_fun) (void *),
                          void (*a_shape_fun) (void *, char *, int));

extern void Vect2Offset (char *off_any, void *v_any, int v_dim,
                         void (*v_size_fun) (void *),
                         void (*v_read_fun) (void *, char *, int), void *a_nt, int a_dim);

#else

#define AccessVect(v, i) fprintf (outfile, "SAC_ND_READ_ARRAY( %s, %i)", v, i)

#define AccessConst(v, i) fprintf (outfile, "%s", v[i])

#define AccessShape(v, i) fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)", v, i)

#define Vect2Offset2(dim, v_i_str, dima, a_i_str)                                        \
    {                                                                                    \
        int i;                                                                           \
        if (dim > 0) {                                                                   \
            for (i = dim - 1; i > 0; i--) {                                              \
                fprintf (outfile, "( ");                                                 \
                a_i_str;                                                                 \
                fprintf (outfile, "* ");                                                 \
            }                                                                            \
            v_i_str;                                                                     \
            for (i = 1; i < dim; i++) {                                                  \
                fprintf (outfile, "+");                                                  \
                v_i_str;                                                                 \
                fprintf (outfile, ") ");                                                 \
            }                                                                            \
            while (i < dima) {                                                           \
                fprintf (outfile, "*");                                                  \
                a_i_str;                                                                 \
                fprintf (outfile, " ");                                                  \
                i++;                                                                     \
            }                                                                            \
        } else {                                                                         \
            fprintf (outfile, "0");                                                      \
        }                                                                                \
    }

#define Vect2Offset(dimv, v_i_str, dima, a)                                              \
    Vect2Offset2 (dimv, v_i_str, dima, AccessShape (a, i))

#endif

#endif /* _icm2c_basic_h_ */
