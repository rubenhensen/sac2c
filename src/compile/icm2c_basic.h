/*
 *
 * $Log$
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

#ifndef _icm2c_basic_h
#define _icm2c_basic_h

extern int print_comment; /* bool */

#ifdef TAGGED_ARRAYS

extern void AccessVect (void *v, char *i_str, int i);

extern void AccessConst (void *v, char *i_str, int i);

extern void AccessDim (void *v);

extern void AccessShape (void *v, char *i_str, int i);

extern void VectToOffset2 (char *offset, void *v, int dimv, void (*v_acc_dim) (void *),
                           void (*v_acc_shp) (void *, char *, int), void *a, int dima,
                           void (*a_acc_dim) (void *),
                           void (*a_acc_shp) (void *, char *, int));

extern void VectToOffset (char *offset, void *v, int dimv, void (*v_acc_dim) (void *),
                          void (*v_acc_shp) (void *, char *, int), void *a, int dima);

#else

#define AccessVect(v, i) fprintf (outfile, "SAC_ND_READ_ARRAY( %s, %i)", v, i)

#define AccessConst(v, i) fprintf (outfile, "%s", v[i])

#define AccessShape(v, i) fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)", v, i)

#define VectToOffset2(dim, v_i_str, dima, a_i_str)                                       \
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

#define VectToOffset(dimv, v_i_str, dima, a)                                             \
    VectToOffset2 (dimv, v_i_str, dima, a, AccessShape (a, i))

#endif

#endif /* _icm2c_basic_h */
