/*
 *
 * $Log$
 * Revision 1.2  1998/05/07 11:27:08  dkr
 * ICM names converted to new naming conventions.
 *
 * Revision 1.1  1998/05/03 13:07:29  dkr
 * Initial revision
 *
 *
 *
 */

#ifndef _icm2c_basic_h
#define _icm2c_basic_h

#define AccessVect(v, i) fprintf (outfile, "SAC_ND_A_FIELD(%s)[%i]", v, i)

#define AccessConst(v, i) fprintf (outfile, "%s", v[i])

#define AccessShape(v, i) fprintf (outfile, "SAC_ND_KD_A_SHAPE(%s, %d)", v, i)

#define VectToOffset2(dim, v_i_str, dima, a_i_str)                                       \
    {                                                                                    \
        int i;                                                                           \
        for (i = dim - 1; i > 0; i--) {                                                  \
            fprintf (outfile, "( ");                                                     \
            a_i_str;                                                                     \
            fprintf (outfile, "* ");                                                     \
        }                                                                                \
        v_i_str;                                                                         \
        for (i = 1; i < dim; i++) {                                                      \
            fprintf (outfile, "+");                                                      \
            v_i_str;                                                                     \
            fprintf (outfile, ") ");                                                     \
        }                                                                                \
        while (i < dima) {                                                               \
            fprintf (outfile, "*");                                                      \
            a_i_str;                                                                     \
            fprintf (outfile, " ");                                                      \
            i++;                                                                         \
        }                                                                                \
    }

#define VectToOffset(dim, v_i_str, dima, a)                                              \
    VectToOffset2 (dim, v_i_str, dima, AccessShape (a, i))

#endif /* _icm2c_basic_h */
