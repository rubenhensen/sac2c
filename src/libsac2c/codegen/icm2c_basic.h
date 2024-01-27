#ifndef _ICM2C_BASIC_H_
#define _ICM2C_BASIC_H_

extern int print_comment; /* bool */

/*
 * Wraps default fprintf call with `global.outfile` as a parameter
 * in one short an simple macro called `out`.
 */
#define out(...) (void)fprintf (global.outfile, __VA_ARGS__)

/* Indented out: calls INDENT and outputs a text using fprintf.  */
#define indout(...)                                                                      \
    do {                                                                                 \
        INDENT;                                                                          \
        out (__VA_ARGS__);                                                               \
    } while (0)

#define SCAN_ARG_LIST(cnt, inc, sep_str, sep_code, code)                                 \
    do {                                                                                    \
        size_t i;                                                                        \
        for (i = 0; i < cnt * inc; i += inc) {                                           \
            if (i > 0) {                                                                 \
                out ("%s", sep_str);                                                     \
                sep_code;                                                                \
            }                                                                            \
            code;                                                                        \
        }                                                                                \
    } while (0)

/*
 * ASSURE_TYPE is implemented using helpers ASSURE_COND and
 * ASSURE_TEXT to pass va_args in conditional and message
 * part.  The anticipated usage is:
 *
 *      ASSURE_TYPE (ASSURE_COND ("%s > 0", var),
 *                   ASSURE_TEXT ("too bad"));
 */
#define ASSURE_COND(...) out (__VA_ARGS__)
#define ASSURE_TEXT(...) out (__VA_ARGS__)

#define ASSURE_EXPR(cond_stmt, text_stmt)                                                \
    do {                                                                                 \
        out ("SAC_ASSURE_TYPE_LINE (\"%s\", %zu, %zu, (",                                \
             global.filename, global.linenum, global.colnum);                            \
        cond_stmt;                                                                       \
        out ("), \"");                                                                   \
        text_stmt;                                                                       \
        out ("\")");                                                                     \
    } while (0)

#define ASSURE_TYPE(cond_stmt, text_stmt)                                                \
    do {                                                                                 \
        INDENT;                                                                          \
        ASSURE_EXPR (cond_stmt, text_stmt);                                              \
        out (";\n");                                                                     \
    } while (0)

/* Generic block template.  */
#define BLOCK_BEGIN(...)                                                                 \
    do {                                                                                 \
        indout ("{\n");                                                                  \
        global.indent++;                                                                 \
        indout (__VA_ARGS__); /* block variables.  */                                    \
        out ("\n");                                                                      \
    } while (0)

/* -- block without any variable declaration.  */
#define BLOCK_NOVAR_BEGIN()                                                              \
    do {                                                                                 \
        indout ("{\n");                                                                  \
        global.indent++;                                                                 \
        out ("\n");                                                                      \
    } while (0)

#define BLOCK_END()                                                                      \
    do {                                                                                 \
        global.indent--;                                                                 \
        indout ("}\n");                                                                  \
    } while (0)

/* Generic for loop template.  */
#define FOR_LOOP_BEGIN(...)                                                              \
    do {                                                                                 \
        indout ("for (");                                                                \
        out (__VA_ARGS__);                                                               \
        out (") {\n");                                                                   \
        global.indent++;                                                                 \
    } while (0)

#define FOR_LOOP_END()                                                                   \
    BLOCK_END                                                                            \
    ()

/* Generic if condition.  */
#define IF_BEGIN(...)                                                                    \
    do {                                                                                 \
        indout ("if (");                                                                 \
        out (__VA_ARGS__);                                                               \
        out (") {\n");                                                                   \
        global.indent++;                                                                 \
    } while (0)

#define IF_END()                                                                         \
    BLOCK_END                                                                            \
    ()

#define ELSE_BEGIN()                                                                     \
    do {                                                                                 \
        indout ("else {\n");                                                             \
        global.indent++;                                                                 \
    } while (0)

#define ELSE_BLOCK_BEGIN(...)                                                            \
    do {                                                                                 \
        ELSE_BEGIN ();                                                                   \
        indout (__VA_ARGS__); /* block variables.  */                                    \
    } while (0)

#define ELSE_END()                                                                       \
    BLOCK_END                                                                            \
    ()

#define ELIF_BEGIN(...)                                                                  \
    do {                                                                                 \
        indout ("else if (");                                                            \
        out (__VA_ARGS__);                                                               \
        out (") {\n");                                                                   \
        global.indent++;                                                                 \
    } while (0)

#define ELIF_END()                                                                       \
    BLOCK_END                                                                            \
    ()

/* Shape macro templates.  */
#define SET_SHP_AUD(to_NT, idx_stmt, set_stmt)                                           \
    do {                                                                                 \
        indout ("SAC_ND_A_DESC_SHAPE( %s, ", to_NT);                                     \
        idx_stmt;                                                                        \
        out (") = ");                                                                    \
        set_stmt;                                                                        \
        out (";\n");                                                                     \
    } while (0)

#define SET_SHP_AUD_NUM(to_NT, idx_num, set_stmt)                                        \
    SET_SHP_AUD (to_NT, out ("%d", idx_num), set_stmt)

#define SET_SHP_AKD(to_NT, idx_num, set_stmt)                                            \
    do {                                                                                 \
        indout ("SAC_ND_A_MIRROR_SHAPE( %s, %d) = \n", to_NT, idx_num);                  \
        indout ("SAC_ND_A_DESC_SHAPE( %s, %d) = ", to_NT, idx_num);                      \
        set_stmt;                                                                        \
        out (";\n");                                                                     \
    } while (0)

/*
 * This is the only macro where we have to pass a statement
 * as a parameter.  But unfortunately there is no simple way
 * around it.  Leave it here until the next refactoring.
 */
#define SET_SIZE(to_NT, set_expr)                                                        \
    do {                                                                                 \
        indout ("SAC_ND_A_DESC_SIZE( %s) "                                               \
                "= SAC_ND_A_MIRROR_SIZE( %s) = ",                                        \
                to_NT, to_NT);                                                           \
        set_expr out (";\n");                                                            \
        ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_MIRROR_SIZE( %s) >= 0", to_NT),              \
                     ASSURE_TEXT ("Array with size <0 found!"));                         \
    } while (0)

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
