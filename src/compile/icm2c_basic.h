/*
 *
 * $Log$
 * Revision 3.16  2003/09/25 10:58:18  dkr
 * no changes done
 *
 * Revision 3.15  2003/09/19 15:39:21  dkr
 * postfix _nt of varnames renamed into _NT
 *
 * Revision 3.14  2003/09/17 12:57:54  dkr
 * postfixes _nt, _any renamed into _NT, _ANY
 *
 * Revision 3.13  2003/04/15 18:59:54  dkr
 * macro SET_SHAPES_AUD__XXX added
 *
 * Revision 3.12  2002/10/30 14:20:30  dkr
 * some new macros added
 *
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
 * [...]
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

#define FOR_LOOP_VARDECS(vardecs, init, cond, post, ass)                                 \
    BLOCK_VARDECS (vardecs, FOR_LOOP (init, cond, post, ass);)

#define FOR_LOOP_INC(idx, start, stop, ass)                                              \
    FOR_LOOP (idx fprintf (outfile, " = "); start, idx fprintf (outfile, " < ");         \
              stop, idx fprintf (outfile, "++");, ass)

#define FOR_LOOP_INC_VARDEC(idx, start, stop, ass)                                       \
    FOR_LOOP_VARDECS (fprintf (outfile, "int "); idx fprintf (outfile, ";");             \
                      , idx fprintf (outfile, " = ");                                    \
                      start, idx fprintf (outfile, " < ");                               \
                      stop, idx fprintf (outfile, "++");, ass)

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

#define SET_SIZE(to_NT, set_ass)                                                         \
    INDENT;                                                                              \
    fprintf (outfile, "SAC_ND_A_DESC_SIZE( %s) = SAC_ND_A_MIRROR_SIZE( %s) = ", to_NT,   \
             to_NT);                                                                     \
    set_ass fprintf (outfile, ";\n")

#define SET_SHAPE_AUD(to_NT, idx_ass, set_ass)                                           \
    INDENT;                                                                              \
    fprintf (outfile, "SAC_ND_A_DESC_SHAPE( %s, ", to_NT);                               \
    idx_ass fprintf (outfile, ") = ");                                                   \
    set_ass fprintf (outfile, ";\n")

#define SET_SHAPE_AUD__NUM(to_NT, idx_num, set_ass)                                      \
    INDENT;                                                                              \
    fprintf (outfile, "SAC_ND_A_DESC_SHAPE( %s, %d) = ", to_NT, idx_num);                \
    set_ass fprintf (outfile, ";\n")

#define SET_SHAPE_AKD(to_NT, idx_num, set_ass)                                           \
    INDENT;                                                                              \
    fprintf (outfile, "SAC_ND_A_MIRROR_SHAPE( %s, %d) = \n", to_NT, idx_num);            \
    INDENT;                                                                              \
    fprintf (outfile, "SAC_ND_A_DESC_SHAPE( %s, %d) = ", to_NT, idx_num);                \
    set_ass fprintf (outfile, ";\n")

#define SET_SHAPES_AUD(to_NT, idx_ass, idx_start_ass, idx_stop_ass, prolog_ass, set_ass) \
    FOR_LOOP_INC_VARDEC (idx_ass, idx_start_ass, idx_stop_ass,                           \
                         prolog_ass SET_SHAPE_AUD (to_NT, idx_ass, set_ass);)

#define SET_SHAPES_AUD__NUM(to_NT, idx, idx_start, idx_stop, prolog_ass, set_ass)        \
    DBUG_ASSERT ((idx_start >= 0), "illegal dimension found!");                          \
    DBUG_ASSERT ((idx_stop >= 0), "illegal dimension found!");                           \
    for (idx = idx_start; idx < idx_stop; i++) {                                         \
        prolog_ass SET_SHAPE_AUD__NUM (to_NT, idx, set_ass);                             \
    }

#define SET_SHAPES_AUD__XXX(to_NT, idx, idx_ass2, idx_start, idx_start_ass2, idx_stop,   \
                            idx_stop_ass2, prolog_ass, set_ass, set_ass2)                \
    if ((idx_start >= 0) && (idx_stop >= 0)) {                                           \
        for (idx = idx_start; idx < idx_stop; i++) {                                     \
            prolog_ass SET_SHAPE_AUD__NUM (to_NT, idx, set_ass);                         \
        }                                                                                \
    } else {                                                                             \
        FOR_LOOP_INC_VARDEC (idx_ass2, idx_start_ass2, idx_stop_ass2,                    \
                             prolog_ass SET_SHAPE_AUD (to_NT, idx_ass2, set_ass2););     \
    }

#define SET_SHAPES_AKD(to_NT, idx, idx_start, idx_stop, prolog_ass, set_ass)             \
    DBUG_ASSERT ((idx_stop >= 0), "illegal dimension found!");                           \
    for (idx = idx_start; idx < idx_stop; i++) {                                         \
        prolog_ass SET_SHAPE_AKD (to_NT, idx, set_ass);                                  \
    }

#ifdef TAGGED_ARRAYS

extern void ReadId (void *var_NT, char *idx_str, int idx);

extern void ReadScalar (void *scl, char *idx_str, int idx);

extern void ReadScalar_Check (void *scl, char *idx_str, int idx);

extern void ReadConstArray (void *v, char *idx_str, int idx);

extern void DimId (void *var_NT);

extern void ShapeId (void *var_NT, char *idx_str, int idx);

extern void SizeId (void *var_NT);

extern void GetAttr (void *v, int v_attr, void (*v_attr_fun) (void *));

extern void Vect2Offset2 (char *off_ANY, void *v_ANY, int v_dim,
                          void (*v_size_fun) (void *),
                          void (*v_read_fun) (void *, char *, int), void *a_ANY,
                          int a_dim, void (*a_dim_fun) (void *),
                          void (*a_shape_fun) (void *, char *, int));

extern void Vect2Offset (char *off_ANY, void *v_ANY, int v_dim,
                         void (*v_size_fun) (void *),
                         void (*v_read_fun) (void *, char *, int), void *a_NT, int a_dim);

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
