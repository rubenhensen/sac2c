/*
 *
 * $Log$
 * Revision 3.23  2004/11/24 18:25:33  jhb
 * added include "types.h"
 *
 * Revision 3.22  2004/11/24 17:40:59  jhb
 * changend outfile and indent to global
 *
 * Revision 3.21  2004/11/21 22:04:36  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 3.20  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.19  2003/12/11 19:05:09  dkrHH
 * bug in SET_SIZE fixed
 *
 * Revision 3.18  2003/12/01 18:58:26  dkrHH
 * SET_SIZE: runtime check added (size>=0 ?)
 *
 * Revision 3.17  2003/09/30 19:29:41  dkr
 * Set_Shape() added
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
 * access macros are functions now
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

#ifndef _ICM2C_BASIC_H_
#define _ICM2C_BASIC_H_

#include "types.h"

extern int print_comment; /* bool */

#define ASSURE_TYPE_ASS(cond_expr, msg_expr)                                             \
    INDENT;                                                                              \
    ASSURE_TYPE_EXPR (cond_expr, msg_expr);                                              \
    fprintf (global.outfile, ";\n")

#define ASSURE_TYPE_EXPR(cond_expr, msg_expr)                                            \
    fprintf (global.outfile, "SAC_ASSURE_TYPE( (");                                      \
    cond_expr fprintf (global.outfile, "), (\"");                                        \
    msg_expr fprintf (global.outfile, "\"))")

#define BLOCK(ass)                                                                       \
    INDENT;                                                                              \
    BLOCK_VARDECS (, ass)

#define BLOCK_VARDECS(vardecs, ass)                                                      \
    INDENT;                                                                              \
    BLOCK_VARDECS__NOINDENT (vardecs, ass)

#define BLOCK__NOINDENT(ass) BLOCK_VARDECS__NOINDENT (, ass)

#define BLOCK_VARDECS__NOINDENT(vardecs, ass)                                            \
    fprintf (global.outfile, "{ ");                                                      \
    vardecs fprintf (global.outfile, "\n");                                              \
    global.indent++;                                                                     \
    ass global.indent--;                                                                 \
    INDENT;                                                                              \
    fprintf (global.outfile, "}\n")

#define FOR_LOOP(init, cond, post, ass)                                                  \
    INDENT;                                                                              \
    fprintf (global.outfile, "for (");                                                   \
    init fprintf (global.outfile, "; ");                                                 \
    cond fprintf (global.outfile, "; ");                                                 \
    post fprintf (global.outfile, ") ");                                                 \
    BLOCK__NOINDENT (ass)

#define FOR_LOOP_VARDECS(vardecs, init, cond, post, ass)                                 \
    BLOCK_VARDECS (vardecs, FOR_LOOP (init, cond, post, ass);)

#define FOR_LOOP_INC(idx_var, start, stop, ass)                                          \
    FOR_LOOP (idx_var fprintf (global.outfile, " = ");                                   \
              start, idx_var fprintf (global.outfile, " < ");                            \
              stop, idx_var fprintf (global.outfile, "++");, ass)

#define FOR_LOOP_INC_VARDEC(idx_var, start, stop, ass)                                   \
    BLOCK_VARDECS (fprintf (global.outfile, "int ");                                     \
                   idx_var fprintf (global.outfile, ";");                                \
                   , FOR_LOOP_INC (idx_var, start, stop, ass);)

#define COND1(cond, ass)                                                                 \
    INDENT;                                                                              \
    fprintf (global.outfile, "if (");                                                    \
    cond fprintf (global.outfile, ") ");                                                 \
    BLOCK__NOINDENT (ass)

#define COND2(cond, then_ass, else_ass)                                                  \
    INDENT;                                                                              \
    fprintf (global.outfile, "if (");                                                    \
    cond fprintf (global.outfile, ") ");                                                 \
    BLOCK__NOINDENT (then_ass);                                                          \
    INDENT;                                                                              \
    fprintf (global.outfile, "else ");                                                   \
    BLOCK__NOINDENT (else_ass)

#define SET_SIZE(to_NT, set_expr)                                                        \
    INDENT;                                                                              \
    fprintf (global.outfile,                                                             \
             "SAC_ND_A_DESC_SIZE( %s) = SAC_ND_A_MIRROR_SIZE( %s) = ", to_NT, to_NT);    \
    set_expr fprintf (global.outfile, ";\n");                                            \
    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_MIRROR_SIZE( %s) >= 0", to_NT);  \
                     , fprintf (global.outfile, "Array with size <0 found!"););

#define SET_SHAPE_AUD(to_NT, idx_expr, set_expr)                                         \
    INDENT;                                                                              \
    fprintf (global.outfile, "SAC_ND_A_DESC_SHAPE( %s, ", to_NT);                        \
    idx_expr fprintf (global.outfile, ") = ");                                           \
    set_expr fprintf (global.outfile, ";\n")

#define SET_SHAPE_AUD__NUM(to_NT, idx_num, set_expr)                                     \
    INDENT;                                                                              \
    fprintf (global.outfile, "SAC_ND_A_DESC_SHAPE( %s, %d) = ", to_NT, idx_num);         \
    set_expr fprintf (global.outfile, ";\n")

#define SET_SHAPE_AKD(to_NT, idx_num, set_expr)                                          \
    INDENT;                                                                              \
    fprintf (global.outfile, "SAC_ND_A_MIRROR_SHAPE( %s, %d) = \n", to_NT, idx_num);     \
    INDENT;                                                                              \
    fprintf (global.outfile, "SAC_ND_A_DESC_SHAPE( %s, %d) = ", to_NT, idx_num);         \
    set_expr fprintf (global.outfile, ";\n")

#define SET_SHAPES_AUD(to_NT, idx_var, idx_start_expr, idx_stop_expr, prolog_ass,        \
                       set_expr)                                                         \
    FOR_LOOP_INC (idx_var, idx_start_expr, idx_stop_expr,                                \
                  prolog_ass SET_SHAPE_AUD (to_NT, idx_var, set_expr);)

#define SET_SHAPES_AUD__NUM(to_NT, idx_var, idx_start, idx_stop, prolog_ass, set_expr)   \
    DBUG_ASSERT ((idx_start >= 0), "illegal dimension found!");                          \
    DBUG_ASSERT ((idx_stop >= 0), "illegal dimension found!");                           \
    for (idx_var = idx_start; idx_var < idx_stop; i++) {                                 \
        prolog_ass SET_SHAPE_AUD__NUM (to_NT, idx_var, set_expr);                        \
    }

#define SET_SHAPES_AUD__XXX(to_NT, idx_var, idx_var2, idx_start, idx_start_expr2,        \
                            idx_stop, idx_stop_expr2, prolog_ass, set_expr, set_expr2)   \
    if ((idx_start >= 0) && (idx_stop >= 0)) {                                           \
        SET_SHAPES_AUD__NUM (to_NT, idx_var, idx_start, idx_stop, prolog_ass, set_expr); \
    } else {                                                                             \
        SET_SHAPES_AUD (to_NT, idx_var2, idx_start_expr2, idx_stop_expr2, prolog_ass,    \
                        set_expr2);                                                      \
    }

#define SET_SHAPES_AKD(to_NT, idx_var, idx_start, idx_stop, prolog_ass, set_expr)        \
    DBUG_ASSERT ((idx_stop >= 0), "illegal dimension found!");                           \
    for (idx_var = idx_start; idx_var < idx_stop; i++) {                                 \
        prolog_ass SET_SHAPE_AKD (to_NT, idx_var, set_expr);                             \
    }

extern void Check_Mirror (char *to_NT, int to_sdim, void *shp1, int shp1_size,
                          void (*shp1_size_fun) (void *),
                          void (*shp1_read_fun) (void *, char *, int), void *shp2,
                          int shp2_size, void (*shp2_size_fun) (void *),
                          void (*shp2_read_fun) (void *, char *, int));

extern void Set_Shape (char *to_NT, int to_sdim, void *shp1, int shp1_size,
                       void (*shp1_size_fun) (void *), void (*shp1_prod_fun) (void *),
                       void (*shp1_read_fun) (void *, char *, int), void *shp2,
                       int shp2_size, void (*shp2_size_fun) (void *),
                       void (*shp2_prod_fun) (void *),
                       void (*shp2_read_fun) (void *, char *, int));

extern void ReadId (void *var_NT, char *idx_str, int idx);

extern void ReadScalar (void *scl, char *idx_str, int idx);

extern void ReadScalar_Check (void *scl, char *idx_str, int idx);

extern void ReadConstArray_Str (void *v, char *idx_str, int idx);

extern void ReadConstArray_Num (void *v, char *idx_str, int idx);

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

#endif /* _ICM2C_BASIC_H_ */
