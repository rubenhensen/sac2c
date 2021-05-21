#define STR_DUP(buffer, name)                                                            \
    name = (char *)malloc ((STRlen (buffer) + 1) * sizeof (char));                       \
    strcpy (name, buffer)

#define ICM_DEF(prf, trf)                                                                \
    if (strcmp (buffer, #prf) == 0) {                                                    \
        DBUG_PRINT ("BEtest", ("reading args:\n"));

#define ICM_ANY(name)                                                                    \
    scanf ("%s", buffer);                                                                \
    DBUG_PRINT ("BEtest", ("any-arg: %s\n", buffer));                                    \
    STR_DUP (buffer, name);

#define ICM_ICM(name)                                                                    \
    scanf ("%s", buffer);                                                                \
    DBUG_PRINT ("BEtest", ("icm-arg: %s\n", buffer));                                    \
    STR_DUP (buffer, name);

#define ICM_NT(name)                                                                     \
    scanf ("%s", buffer);                                                                \
    DBUG_PRINT ("BEtest", ("nt-arg: %s\n", buffer));                                     \
    STR_DUP (buffer, name);

#define ICM_ID(name)                                                                     \
    scanf ("%s", buffer);                                                                \
    DBUG_PRINT ("BEtest", ("id-arg: %s\n", buffer));                                     \
    STR_DUP (buffer, name);

#define ICM_STR(name)                                                                    \
    scanf ("%s", buffer);                                                                \
    DBUG_PRINT ("BEtest", ("str-arg: %s\n", buffer));                                    \
    STR_DUP (buffer, name);

#define ICM_INT(name)                                                                    \
    scanf ("%i", &name);                                                                 \
    DBUG_PRINT ("BEtest", ("int-arg: %i\n", name));

#define ICM_UINT(name)                                                                    \
    scanf ("%u", &name);                                                                 \
    DBUG_PRINT ("BEtest", ("sizet-arg: %u\n", name));

#define ICM_BOOL(name)                                                                   \
    scanf ("%i", &name);                                                                 \
    DBUG_PRINT ("BEtest", ("bool-arg: %i\n", name));

#define ICM_VARANY(dim, name)                                                            \
    {                                                                                    \
        int i;                                                                           \
        name = (char **)malloc (dim * sizeof (char *));                                  \
        DBUG_PRINT ("BEtest", ("varany-arg with %d elems:\n", dim));                     \
        for (i = 0; i < dim; i++) {                                                      \
            ICM_ANY (name[i])                                                            \
        }                                                                                \
    }

#define ICM_VARNT(dim, name)                                                             \
    {                                                                                    \
        int i;                                                                           \
        name = (char **)malloc (dim * sizeof (char *));                                  \
        DBUG_PRINT ("BEtest", ("varnt-arg with %d elems:\n", dim));                      \
        for (i = 0; i < dim; i++) {                                                      \
            ICM_NT (name[i])                                                             \
        }                                                                                \
    }

#define ICM_VARID(dim, name)                                                             \
    {                                                                                    \
        int i;                                                                           \
        name = (char **)malloc (dim * sizeof (char *));                                  \
        DBUG_PRINT ("BEtest", ("varid-arg with %d elems:\n", dim));                      \
        for (i = 0; i < dim; i++) {                                                      \
            ICM_ID (name[i])                                                             \
        }                                                                                \
    }

#define ICM_VARINT(dim, varint)                                                          \
    {                                                                                    \
        int i;                                                                           \
        varint = (int *)malloc (dim * sizeof (int));                                     \
        DBUG_PRINT ("BEtest", ("varint-arg with %d elems:\n", dim));                     \
        for (i = 0; i < dim; i++) {                                                      \
            ICM_INT (varint[i])                                                          \
        }                                                                                \
    }

#define ICM_END(prf, args)                                                               \
    ICMCompile##prf args;                                                                \
    }                                                                                    \
    else

#include "icm.data"

#undef STR_DUP

#undef ICM_DEF
#undef ICM_ANY
#undef ICM_ICM
#undef ICM_NT
#undef ICM_ID
#undef ICM_STR
#undef ICM_INT
#undef ICM_UINT
#undef ICM_BOOL
#undef ICM_VARANY
#undef ICM_VARNT
#undef ICM_VARID
#undef ICM_VARINT
#undef ICM_END
