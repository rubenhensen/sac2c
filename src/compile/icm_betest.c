/*
 *
 * $Log$
 * Revision 3.2  2001/01/22 13:38:51  dkr
 * tabs removed
 *
 * Revision 3.1  2000/11/20 18:01:22  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:42:46  sacbase
 * new release made
 *
 * Revision 1.2  1998/06/29 08:47:50  cg
 * implemented new icm-argument type VARINT
 *
 * Revision 1.1  1998/04/25 16:21:06  sbs
 * Initial revision
 *
 */

#define STR_DUP(buffer, name)                                                            \
    name = (char *)malloc (sizeof (char) * strlen (buffer) + 1);                         \
    strcpy (name, buffer)

#define ICM_DEF(prf, trf)                                                                \
    if (strcmp (buffer, #prf) == 0) {                                                    \
        DBUG_PRINT ("BEtest", ("reading args:\n"));

#define ICM_STR(name)                                                                    \
    scanf ("%s", buffer);                                                                \
    DBUG_PRINT ("BEtest", ("string-arg: %s\n", buffer));                                 \
    STR_DUP (buffer, name);

#define ICM_INT(name)                                                                    \
    scanf ("%i", &name);                                                                 \
    DBUG_PRINT ("BEtest", ("int-arg: %i\n", name));

#define ICM_VAR(dim, name)                                                               \
    {                                                                                    \
        int i;                                                                           \
        name = (char **)malloc (dim * sizeof (char *));                                  \
        DBUG_PRINT ("BEtest", ("var-arg with %d elems:\n", dim));                        \
        for (i = 0; i < dim; i++) {                                                      \
            scanf ("%s", buffer);                                                        \
            STR_DUP (buffer, name[i]);                                                   \
            DBUG_PRINT ("BEtest", ("  string-arg: %s\n", name[i]));                      \
        }                                                                                \
    }

#define ICM_VARINT(dim, varint)                                                          \
    {                                                                                    \
        int i;                                                                           \
        varint = (int *)malloc (dim * sizeof (int));                                     \
        DBUG_PRINT ("BEtest", ("varint-arg with %d elems:\n", dim));                     \
        for (i = 0; i < dim; i++) {                                                      \
            scanf ("%d", &(varint[i]));                                                  \
            DBUG_PRINT ("BEtest", ("  varint-arg: %d\n", varint[i]));                    \
        }                                                                                \
    }

#define ICM_END(prf, args)                                                               \
    ICMCompile##prf args;                                                                \
    }                                                                                    \
    else

#include "icm.data"

#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_VARINT
#undef ICM_END
