/*
 *
 * $Log$
 * Revision 1.2  2000/11/20 14:27:54  dkr
 * nothing changed
 *
 * Revision 1.1  1999/06/16 17:37:51  rob
 * Initial revision
 *
 * Revision 2.3  1999/04/14 16:22:03  jhs
 * Option for secure malloc(0) added.
 *
 * Revision 2.2  1999/04/12 09:37:48  cg
 * All accesses to C arrays are now performed through the new ICMs
 * ND_WRITE_ARRAY and ND_READ_ARRAY. This allows for an integration
 * of cache simulation as well as boundary checking.
 *
 * Revision 2.1  1999/02/23 12:43:59  sacbase
 * new release made
 *
 * Revision 1.6  1998/12/08 10:58:21  cg
 * bug fixed: SAC_TR_MEM_PRINT_TRACEHEADER_ALL still used but no
 * longer defined.
 *
 * Revision 1.5  1998/07/10 08:09:21  cg
 * some bugs fixed, appropriate renaming of macros
 *
 * Revision 1.4  1998/06/29 08:52:19  cg
 * streamlined tracing facilities
 * tracing on new with-loop and multi-threading operations implemented
 *
 * Revision 1.2  1998/06/03 14:59:05  cg
 * generation of new identifier names as extensions of old ones
 * by macros made compatible with new renaming scheme
 *
 * Revision 1.1  1998/05/07 08:38:05  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   sac_std.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *   This file contains all those ICMs that deal with refcounted data
 *   objects in any way
 *
 *****************************************************************************/

#ifndef SAC_STD_H
#define SAC_STD_H

/*
 * README: The difference between 'type' and 'basetype'
 *
 * 'type' and 'basetype' both specify a string containing a type.
 *
 * 'basetype' is used in all ICMs dedicated to arrays and simply means the
 * base type of the array, e.g. 'int'.
 *
 * 'type' is used in all ICMs not specific to arrays and must specify the
 * actual type, e.g. 'int*' for an integer array, 'int' for an integer,
 * 'void*' for a hidden, etc.
 */

/*
 * Positional parameters for name tuples (nt):
 *   name,class,unique
 *
 * Values are:
 *   name:   data object name
 *
 *   class:  AKS: array of known shape
 *           AKD: array of known dimension but unknown shape
 *           HID: hidden object
 *
 *   unique: UNQ: data object is unique
 *           NUQ: data object is non-unique
 *
 */

#define NAME_NAME(nt) Item0 nt
#define NAME_CLASS(nt) Item1 nt
#define NAME_UNQ(nt) Item2 nt

/*
 * Array descriptor used by dynamic shape support
 */

#define MAXDIM 15

typedef struct {
    int rc;            /* reference count for array */
    int dim;           /* # of dimensions in array */
    int size;          /* number of elements in array */
    void *paa;         /* physical (malloc) address of array */
    int shape[MAXDIM]; /* shape vector for array */
} SAC_array_descriptor;

/******************************************************************************
 *
 * ICMs for descriptor access
 * ==========================
 *
 * ND_A_DESC( nt)  : accesses the descriptor of the data object
 *
 * ND_A_RCP( nt)   : accesses the pointer to reference counter
 * ND_A_RC( nt)    : accesses the reference counter
 *
 * ND_A_DIM( nt)   : accesses the dimension
 * ND_A_SIZE( nt)  : accesses the size of the unrolling (in elements)
 * ND_A_SHAPE( nt) : accesses one shape component
 * ND_A_FIELD( nt) : accesses the physical (malloc) address of the data object
 *
 ******************************************************************************/

#define SAC_ND_A_DESC(nt) CAT0 (SAC_ND_A_DESC__, CAT0 (NAME_CLASS (nt), (nt)))

#define SAC_ND_A_RCP(nt)                                                                 \
    CAT1 (SAC_ND_A_RCP__, CAT1 (NAME_CLASS (nt), CAT1 (_, CAT1 (NAME_UNQ (nt), (nt)))))

#define SAC_ND_A_RC(nt)                                                                  \
    CAT1 (SAC_ND_A_RC__, CAT1 (NAME_CLASS (nt), CAT1 (_, CAT1 (NAME_UNQ (nt), (nt)))))

#define SAC_ND_A_DIM(nt) CAT1 (SAC_ND_A_DIM__, CAT1 (NAME_CLASS (nt), (nt)))

#define SAC_ND_A_SIZE(nt) CAT1 (SAC_ND_A_SIZE__, CAT1 (NAME_CLASS (nt), (nt)))

#define SAC_ND_A_SHAPE(nt, dim)                                                          \
    CAT1 (SAC_ND_A_SHAPE__, CAT1 (NAME_CLASS (nt), BuildArgs2 (nt, dim)))

#define SAC_ND_A_FIELD(nt) CAT1 (SAC_ND_A_FIELD__, CAT1 (NAME_CLASS (nt), (nt)))

/*
 * ICMs for AKS
 */

#define SAC_ND_A_DESC__AKS(nt) ICM_UNDEF

#define SAC_ND_A_RCP__AKS_NUQ(nt) CAT2 (NAME_NAME (nt), __rc)
#define SAC_ND_A_RCP__AKS_UNQ(nt) ICM_UNDEF

#define SAC_ND_A_RC__AKS_NUQ(nt) CAT2 (*, CAT2 (NAME_NAME (nt), __rc))
#define SAC_ND_A_RC__AKS_UNQ(nt) ICM_UNDEF

#define SAC_ND_A_DIM__AKS(nt) CAT2 (NAME_NAME (nt), __d)

#define SAC_ND_A_SIZE__AKS(nt) CAT2 (NAME_NAME (nt), __sz)

#define SAC_ND_A_SHAPE__AKS(nt, dim) CAT2 (NAME_NAME (nt), __s##dim)

#define SAC_ND_A_FIELD__AKS(nt) NAME_NAME (nt)

/*
 * ICMs for AKD
 */

#define SAC_ND_A_DESC__AKD(nt) CAT2 (NAME_NAME (nt), __desc)

#define SAC_ND_A_RCP__AKD_NUQ(nt) ICM_UNDEF
#define SAC_ND_A_RCP__AKD_UNQ(nt) ICM_UNDEF

#define SAC_ND_A_RC__AKD_NUQ(nt) CAT2 (SAC_ND_A_DESC (nt), ->rc)
#define SAC_ND_A_RC__AKD_UNQ(nt) ICM_UNDEF

#define SAC_ND_A_DIM__AKD(nt) CAT2 (SAC_ND_A_DESC (nt), ->dim)

#define SAC_ND_A_SIZE__AKD(nt) CAT2 (SAC_ND_A_DESC (nt), ->size)

#define SAC_ND_A_SHAPE__AKD(nt, dim) CAT2 (SAC_ND_A_DESC (nt), ->shape[dim])

#define SAC_ND_A_FIELD__AKD(nt) CAT2 (SAC_ND_A_DESC (nt), ->paa)

/*
 * ICMs for HID
 */

#define SAC_ND_A_DESC__HID(nt) ICM_UNDEF

#define SAC_ND_A_RCP__HID_NUQ(nt) CAT2 (NAME_NAME (nt), __rc)
#define SAC_ND_A_RCP__HID_UNQ(nt) ICM_UNDEF

#define SAC_ND_A_RC__HID_NUQ(nt) CAT2 (*, CAT2 (NAME_NAME (nt), __rc))
#define SAC_ND_A_RC__HID_UNQ(nt) ICM_UNDEF

#define SAC_ND_A_DIM__HID(nt) ICM_UNDEF

#define SAC_ND_A_SIZE__HID(nt) ICM_UNDEF

#define SAC_ND_A_SHAPE__HID(nt, dim) ICM_UNDEF

#define SAC_ND_A_FIELD__HID(nt) NAME_NAME (nt)

/******************************************************************************
 *
 * ICMs for read/write access
 * ==========================
 *
 * Only these two ICMs should be used to access the elements of an array as
 * they selectively enable boundary checking and cache simulation!
 *
 * ND_READ( nt, pos)  : read access at specified index position
 * ND_WRITE( nt, pos) : write access at specified index position
 *
 ******************************************************************************/

#define SAC_ND_READ(nt, pos)                                                             \
    CAT2 (SAC_ND_READ__, CAT2 (NAME_CLASS (nt), BuildArgs2 (nt, pos)))

#define SAC_ND_WRITE(nt, pos)                                                            \
    CAT2 (SAC_ND_WRITE__, CAT2 (NAME_CLASS (nt), BuildArgs2 (nt, pos)))

/*
 * ICMs for AKS
 */

#define SAC_ND_READ__AKS(nt, pos)                                                        \
    (SAC_BC_READ (nt, pos) SAC_CS_READ_ARRAY (nt, pos) SAC_ND_A_FIELD (nt)[pos])

#define SAC_ND_WRITE__AKS(nt, pos)                                                       \
    SAC_BC_WRITE (nt, pos) SAC_CS_WRITE_ARRAY (nt, pos) SAC_ND_A_FIELD (nt)[pos]

/*
 * ICMs for AKD
 */

#define SAC_ND_READ__AKD(nt, pos) SAC_ND_READ__AKS (nt, pos)

#define SAC_ND_WRITE__AKD(nt, pos) SAC_ND_WRITE__AKS (nt, pos)

/*
 * ICMs for HID
 */

#define SAC_ND_READ__HID(nt) ICM_UNDEF

#define SAC_ND_WRITE__HID(nt) ICM_UNDEF

/******************************************************************************
 *
 * ICMs for declaration and allocation of data objects
 * ===================================================
 *
 * ND_DECL( basetype, nt, dim) : declares a data object
 * ND_ALLOC( basetype, nt, rc) : allocates a data object
 * ND_FREE( nt, freefun)       : frees a data object
 *
 *************
 *
 * NOTE:
 * If SECURE_ALLOC_FREE is defined an extra check wether the requested size is
 * 0 is inserted into the code. In that case no call for malloc(0) is executed,
 * but the variable is directly set to NULL.
 *
 ******************************************************************************/

#define SAC_ND_DECL(basetype, nt, dim)                                                   \
    CAT3 (SAC_ND_DECL__,                                                                 \
          CAT3 (NAME_CLASS (nt),                                                         \
                CAT3 (_, CAT3 (NAME_UNQ (nt), BuildArgs3 (basetype, nt, dim)))))

#define SAC_ND_ALLOC(basetype, nt, rc)                                                   \
    CAT3 (SAC_ND_ALLOC__,                                                                \
          CAT3 (NAME_CLASS (nt),                                                         \
                CAT3 (_, CAT3 (NAME_UNQ (nt), BuildArgs3 (basetype, nt, rc)))))

#define SAC_ND_FREE(nt, freefun)                                                         \
    CAT3 (SAC_ND_FREE__,                                                                 \
          CAT3 (NAME_CLASS (nt),                                                         \
                CAT3 (_, CAT3 (NAME_UNQ (nt), BuildArgs2 (nt, freefun)))))

/*
 * helper macros
 */

#ifdef SECURE_ALLOC_FREE
#define _SAC_ND_ALLOC_ARRAY_1_(basetype, nt)                                             \
    if (SAC_ND_A_SIZE (nt) != 0) {                                                       \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_FIELD (nt),                                   \
                                  sizeof (basetype) * SAC_ND_A_SIZE (nt));               \
    } else {                                                                             \
        SAC_ND_A_FIELD (nt) = NULL;                                                      \
    }
#else /* SECURE_ALLOC_FREE */
#define _SAC_ND_ALLOC_ARRAY_1_(basetype, nt)                                             \
    SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_FIELD (nt),                                       \
                              sizeof (basetype) * SAC_ND_A_SIZE (nt));
#endif /* SECURE_ALLOC_FREE */

#define _SAC_ND_ALLOC_ARRAY_2_(nt)                                                       \
    SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (nt));                                        \
    SAC_TR_REF_PRINT_RC (nt);                                                            \
    SAC_CS_REGISTER_ARRAY (nt);

#ifdef SECURE_ALLOC_FREE
#define _SAC_ND_FREE_ARRAY_1_(nt)                                                        \
    if (SAC_ND_A_SIZE (nt) != 0) {                                                       \
        SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_FIELD (nt),                                     \
                                SAC_ND_A_SIZE (nt) * sizeof (*SAC_ND_A_FIELD (nt)));     \
    }
#else /* SECURE_ALLOC_FREE */
#define _SAC_ND_FREE_ARRAY_1_(nt)                                                        \
    SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_FIELD (nt),                                         \
                            SAC_ND_A_SIZE (nt) * sizeof (*SAC_ND_A_FIELD (nt)));
#endif /* SECURE_ALLOC_FREE */

#define _SAC_ND_FREE_ARRAY_2_(nt)                                                        \
    SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (nt));                                        \
    SAC_CS_UNREGISTER_ARRAY (nt);

/*
 * ICMs for AKS
 */

/*
 * SAC_ND_DECL__AKS_NUQ( basetype, nt, dim, ... shape ...)  is a C-ICM
 * SAC_ND_DECL__AKS_UNQ( basetype, nt, dim, ... shape ...)  is a C-ICM
 */

#define SAC_ND_ALLOC__AKS_NUQ(basetype, nt, rc)                                          \
    {                                                                                    \
        _SAC_ND_ALLOC_ARRAY_1_ (basetype, nt);                                           \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_RCP (nt), sizeof (int));                      \
        SAC_ND_A_RC (nt) = rc;                                                           \
        SAC_TR_MEM_PRINT (("ND_ALLOC( %s, %s, %d) at addr: %p", #basetype, #nt, rc,      \
                           SAC_ND_A_FIELD (nt)));                                        \
        _SAC_ND_ALLOC_ARRAY_2_ (ND_ALLOC, nt);                                           \
    }
#define SAC_ND_ALLOC__AKS_UNQ(basetype, nt, rc)                                          \
    {                                                                                    \
        _SAC_ND_ALLOC_ARRAY_1_ (basetype, nt);                                           \
        SAC_TR_MEM_PRINT (("ND_ALLOC( %s, %s, %d) at addr: %p", #basetype, #nt, rc,      \
                           SAC_ND_A_FIELD (nt)));                                        \
        _SAC_ND_ALLOC_ARRAY_2_ (nt);                                                     \
    }

#define SAC_ND_FREE__AKS_NUQ(nt, freefun)                                                \
    {                                                                                    \
        _SAC_ND_FREE_ARRAY_1_ (nt);                                                      \
        SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_RCP (nt), sizeof (int));                        \
        SAC_TR_MEM_PRINT (("ND_FREE( %s, ) at addr: %p", #nt, nt));                      \
        _SAC_ND_FREE_ARRAY_2_ (nt);                                                      \
    }
#define SAC_ND_FREE__AKS_UNQ(nt, freefun)                                                \
    {                                                                                    \
        _SAC_ND_FREE_ARRAY_1_ (nt);                                                      \
        SAC_TR_MEM_PRINT (("ND_FREE( %s, ) at addr: %p", #nt, nt));                      \
        _SAC_ND_FREE_ARRAY_2_ (nt);                                                      \
    }

/*
 * ICMs for AKD
 */

#define SAC_ND_DECL__AKD_NUQ(basetype, nt, dim) SAC_array_descriptor *SAC_ND_A_DESC (nt);
#define SAC_ND_DECL__AKD_UNQ(basetype, nt, dim) SAC_ND_DECL__AKD_NUQ (basetype, nt, dim)

#define SAC_ND_ALLOC__AKD_NUQ(basetype, nt, rc)                                          \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_DESC (nt), sizeof (*SAC_ND_A_DESC (nt)));     \
        _SAC_ND_ALLOC_ARRAY_1_ (basetype, nt);                                           \
        SAC_ND_A_RC (nt) = rc;                                                           \
        SAC_TR_MEM_PRINT (("ND_ALLOC( %s, %s, %d) at addr: %p", #basetype, #nt, rc,      \
                           SAC_ND_A_FIELD (nt)));                                        \
        _SAC_ND_ALLOC_ARRAY_2_ (nt);                                                     \
    }
#define SAC_ND_ALLOC__AKD_UNQ(basetype, nt, rc)                                          \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_DESC (nt), sizeof (*SAC_ND_A_DESC (nt)));     \
        _SAC_ND_ALLOC_ARRAY_1_ (basetype, nt);                                           \
        SAC_TR_MEM_PRINT (("ND_ALLOC( %s, %s, %d) at addr: %p", #basetype, #nt, rc,      \
                           SAC_ND_A_FIELD (nt)));                                        \
        _SAC_ND_ALLOC_ARRAY_2_ (nt);                                                     \
    }

#define SAC_ND_FREE__AKD_NUQ(nt, freefun)                                                \
    {                                                                                    \
        _SAC_ND_FREE_ARRAY_1_ (nt);                                                      \
        SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_DESC (nt), sizeof (*SAC_ND_A_DESC (nt)));       \
        SAC_TR_MEM_PRINT (("ND_FREE( %s, ) at addr: %p", #nt, nt));                      \
        _SAC_ND_FREE_ARRAY_2_ (nt);                                                      \
    }
#define SAC_ND_FREE__AKD_UNQ(nt, freefun)                                                \
    {                                                                                    \
        _SAC_ND_FREE_ARRAY_1_ (nt);                                                      \
        SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_DESC (nt), sizeof (*SAC_ND_A_DESC (nt)));       \
        SAC_TR_MEM_PRINT (("ND_FREE( %s, ) at addr: %p", #nt, nt));                      \
        _SAC_ND_FREE_ARRAY_2_ (nt);                                                      \
    }

/*
 * ICMs for HID
 */

#define SAC_ND_DECL__HID_NUQ(type, nt, dim)                                              \
    type SAC_ND_A_FIELD (nt);                                                            \
    int SAC_ND_A_RC (nt);
#define SAC_ND_DECL__HID_UNQ(type, nt, dim) type SAC_ND_A_FIELD (nt);

#define SAC_ND_ALLOC__HID_NUQ(type, nt, dim) ICM_UNDEF
#define SAC_ND_ALLOC__HID_UNQ(type, nt, dim) ICM_UNDEF

#define SAC_ND_FREE__HID_NUQ(nt, freefun)                                                \
    {                                                                                    \
        freefun (nt);                                                                    \
        SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_RCP (nt), sizeof (int));                        \
        SAC_TR_MEM_PRINT (("ND_FREE( %s, %s) at addr: %p", #nt, #freefun, nt));          \
        SAC_TR_DEC_HIDDEN_MEMCNT (1);                                                    \
    }
#define SAC_ND_FREE__HID_UNQ(nt, freefun)                                                \
    {                                                                                    \
        freefun (nt);                                                                    \
        SAC_TR_MEM_PRINT (("ND_FREE( %s, %s) at addr: %p", #nt, #freefun, nt));          \
        SAC_TR_DEC_HIDDEN_MEMCNT (1);                                                    \
    }

/******************************************************************************
 *
 * ICMs for assigning data objects
 * ===============================
 *
 * ND_ASSIGN( from_nt, to_nt) :
 *   assigns a data object to another one
 * ND_COPY( from_nt, to_nt, copyfun/basetype) :
 *   copies a data object to another one
 *
 * ND_CHECK_REUSE( from_nt, to_nt, copyfun) :
 *   tries to reuse old data object for new, copies if impossible
 *
 ******************************************************************************/

#define SAC_ND_ASSIGN(from_nt, to_nt)                                                    \
    CAT4 (SAC_ND_ASSIGN__,                                                               \
          CAT4 (NAME_CLASS (from_nt),                                                    \
                CAT4 (_, CAT4 (NAME_UNQ (from_nt),                                       \
                               CAT4 (__, CAT4 (NAME_CLASS (to_nt),                       \
                                               CAT4 (_, CAT4 (NAME_UNQ (to_nt),          \
                                                              BuildArgs2 (from_nt,       \
                                                                          to_nt)))))))))

#define SAC_ND_COPY(from_nt, to_nt, arg3)                                                \
    CAT4 (SAC_ND_COPY__,                                                                 \
          CAT4 (NAME_CLASS (from_nt),                                                    \
                CAT4 (_, CAT4 (NAME_UNQ (from_nt), BuildArgs2 (from_nt, to_nt)))))

#define SAC_ND_CHECK_REUSE(from_nt, to_nt, copyfun)                                      \
    CAT3 (SAC_ND_CHECK_REUSE__,                                                          \
          CAT3 (NAME_UNQ (from_nt), BuildArgs3 (from_nt, to_nt, copyfun)))

/*
 * ICMs for AKS x AKS
 */

#define SAC_ND_ASSIGN__AKS_NUQ__AKS_NUQ(from_nt, to_nt)                                  \
    {                                                                                    \
        SAC_ND_A_RCP (to_nt) = SAC_ND_A_RCP (from_nt);                                   \
        SAC_ND_A_FIELD (to_nt) = SAC_ND_A_FIELD (from_nt);                               \
    }
#define SAC_ND_ASSIGN__AKS_NUQ__AKS_UNQ(from_nt, to_nt) ICM_UNDEF
#define SAC_ND_ASSIGN__AKS_UNQ__AKS_NUQ(from_nt, to_nt) ICM_UNDEF
#define SAC_ND_ASSIGN__AKS_UNQ__AKS_UNQ(from_nt, to_nt)                                  \
    {                                                                                    \
        SAC_ND_A_FIELD (to_nt) = SAC_ND_A_FIELD (from_nt);                               \
    }

/*
 * ICMs for AKS x AKD
 */

#define SAC_ND_ASSIGN__AKS_NUQ(from_nt, to_nt)                                           \
    {                                                                                    \
        SAC_ND_A_RCP (to_nt) = SAC_ND_A_RCP (from_nt);                                   \
        SAC_ND_A_FIELD (to_nt) = SAC_ND_A_FIELD (from_nt);                               \
    }
#define SAC_ND_ASSIGN__AKS_UNQ(from_nt, to_nt)                                           \
    {                                                                                    \
        SAC_ND_A_FIELD (to_nt) = SAC_ND_A_FIELD (from_nt);                               \
    }

/*
 * ICMs for AKS x HID
 */

#define SAC_ND_ASSIGN__AKS_NUQ(from_nt, to_nt)                                           \
    {                                                                                    \
        SAC_ND_A_RCP (to_nt) = SAC_ND_A_RCP (from_nt);                                   \
        SAC_ND_A_FIELD (to_nt) = SAC_ND_A_FIELD (from_nt);                               \
    }
#define SAC_ND_ASSIGN__AKS_UNQ(from_nt, to_nt)                                           \
    {                                                                                    \
        SAC_ND_A_FIELD (to_nt) = SAC_ND_A_FIELD (from_nt);                               \
    }

/*
 * ICMs for AKD x ...
 */

#define SAC_ND_ASSIGN__AKD_NUQ(from_nt, to_nt)                                           \
    SAC_ND_A_DESC (to_nt) = SAC_ND_A_DESC (from_nt);
#define SAC_ND_ASSIGN__AKD_UNQ(from_nt, to_nt) SAC_ND_ASSIGN__AKD_NUQ (from_nt, to_nt)

/*
 * ICMs for HID x ...
 */

#define SAC_ND_ASSIGN__HID_NUQ(from_nt, to_nt)                                           \
    {                                                                                    \
        SAC_ND_A_FIELD (to_nt) = SAC_ND_A_FIELD (from_nt);                               \
        SAC_ND_A_RCP (to_nt) = SAC_ND_A_RCP (from_nt);                                   \
    }
#define SAC_ND_ASSIGN__HID_UNQ(from_nt, to_nt)                                           \
    SAC_ND_A_FIELD (to_nt) = SAC_ND_A_FIELD (from_nt);

/*
 * ICMs for AKS
 */

#define SAC_ND_COPY__AKS_NUQ(from_nt, to_nt, basetype)                                   \
    {                                                                                    \
        int __i;                                                                         \
        _SAC_ND_ALLOC_ARRAY_1_ (basetype, to_nt)                                         \
        SAC_TR_MEM_PRINT (("SAC_ND_COPY( %s, %s, %s) at addr: %p", #from_nt, #to_nt,     \
                           #basetype, SAC_ND_A_FIELD (to_nt)));                          \
        _SAC_ND_ALLOC_ARRAY_2_ (to_nt)                                                   \
        SAC_TR_REF_PRINT_RC (from_nt);                                                   \
        for (__i = 0; __i < SAC_ND_A_SIZE (from_nt); __i++) {                            \
            SAC_ND_WRITE (to_nt, __i) = SAC_ND_READ (from_nt, __i);                      \
        }                                                                                \
    }
#define SAC_ND_COPY__AKS_UNQ(from_nt, to_nt, copyfun) ICM_UNDEF

/*
 * ICMs for AKD
 */

#define SAC_ND_COPY__AKD_NUQ(from_nt, to_nt, basetype)                                   \
    SAC_ND_COPY__AKS_NUQ (from_nt, to_nt, basetype)
#define SAC_ND_COPY__AKD_UNQ(from_nt, to_nt, copyfun) ICM_UNDEF

/*
 * ICMs for HID
 */

#define SAC_ND_COPY__HID_NUQ(from_nt, to_nt, copyfun)                                    \
    {                                                                                    \
        SAC_ND_A_FIELD (to_nt) = copyfun (SAC_ND_A_FIELD (from_nt));                     \
        SAC_TR_MEM_PRINT (("ND_COPY( %s, %s, %s)", #from_nt, #to_nt, #copyfun));         \
        SAC_TR_MEM_PRINT (("new hidden object at addr: %p", to_nt));                     \
        SAC_TR_INC_HIDDEN_MEMCNT (1);                                                    \
        SAC_TR_REF_PRINT_RC (to_nt);                                                     \
    }
#define SAC_ND_COPY__HID_UNQ(from_nt, to_nt, copyfun) ICM_UNDEF

/*
 * ICMs for AKS, AKD, HID
 */

#define SAC_ND_CHECK_REUSE__NUQ(from_nt, to_nt, copyfun)                                 \
    if (SAC_ND_A_RC (from_nt) == 1) {                                                    \
        SAC_ND_ASSIGN (from_nt, to_nt);                                                  \
        SAC_TR_MEM_PRINT (                                                               \
          ("reuse memory of %s at %p for %s", #from_nt, from_nt, #to_nt));               \
    } else
#define SAC_ND_CHECK_REUSE__UNQ(from_nt, to_nt, copyfun) ICM_UNDEF

/******************************************************************************
 *
 * ICMs for passing data objects to functions
 * ==========================================
 *
 * ND_DEC_IN( type, nt) :
 *   macro for prototyping data object as "in" parameter
 *
 * ND_DEC_OUT( type, nt) :
 *   macro for prototyping data object as "out" parameter
 *
 * ND_DEC_INOUT( type, nt) :
 *   macro for prototyping data object as "inout" parameter
 *
 * ND_DEC_IMPORT_IN( type) :
 *   macro for prototyping data object as "in" parameter
 *   (imported functions only)
 *
 * ND_DEC_IMPORT_OUT( type) :
 *   macro for prototyping data object as "out" parameter
 *   (imported functions only)
 *
 * ND_DEC_IMPORT_INOUT( type) :
 *   macro for prototyping data object as "inout" parameter
 *   (imported functions only)
 *
 * ND_AP_IN( nt) :
 *   macro for giving data object as argument
 *
 * ND_AP_OUT( nt) :
 *   macro for getting data object as result
 *
 * ND_AP_INOUT( nt) :
 *   macro for giving data object as "inout" argument
 *
 * ND_RET_OUT( nt) :
 *   macro for returning data object
 *
 * ND_RET_INOUT( nt) :
 *   macro for returning "inout" data object
 *
 ******************************************************************************/

/*
 * ICMs for AKS
 */

/*
 * ICMs for AKD
 */

/*
 * ICMs for HID
 */

#define SAC_ND_DEC_IN__AKS_NUQ(type, nt) type SAC_ND_A_FIELD (nt), int *SAC_ND_A_RCP (nt)

#define SAC_ND_DEC_OUT_RC(type, nt) type *nt##__p, int **nt##__rc__p

#define SAC_ND_DEC_INOUT_RC(type, nt) type *nt##__p, int **nt##__rc__p

#define SAC_ND_DEC_IMPORT_IN_RC(type) type, int *

#define SAC_ND_DEC_IMPORT_OUT_RC(type) type *, int **

#define SAC_ND_DEC_IMPORT_INOUT_RC(type) type *, int **

#define SAC_ND_AP_IN_RC(nt) nt, nt##__rc

#define SAC_ND_AP_OUT_RC(nt) &nt, &nt##__rc

#define SAC_ND_AP_INOUT_RC(nt) &nt, &nt##__rc

#define SAC_ND_RET_OUT_RC(nt)                                                            \
    *##nt##__p = nt;                                                                     \
    *nt##__rc__p = nt##__rc

#define SAC_ND_RET_INOUT_RC(nt)                                                          \
    *##nt##__p = nt;                                                                     \
    *nt##__rc__p = nt##__rc

#define SAC_ND_DECL_INOUT_PARAM(type, nt) type nt = *##nt##__p;

#define SAC_ND_DECL_INOUT_PARAM_RC(type, nt)                                             \
    type nt = *##nt##__p;                                                                \
    int *nt##__rc = *nt##__rc__p;

/******************************************************************************
 *
 * ICMs for refcounting data objects
 * =================================
 *
 * ND_SET_RC( nt, rc) : sets the refcount of a data object
 * ND_INC_RC( nt, rc) : increments the refcount of a data object
 * ND_DEC_RC( nt, rc) : decrements the refcount of a data object
 * ND_DEC_RC_FREE( nt, rc, freefun) :
 *   decrements the refcount and frees the data object if refcount becomes 0
 *
 ******************************************************************************/

#define SAC_ND_SET_RC(nt, rc)                                                            \
    CAT4 (SAC_ND_SET_RC__, CAT4 (NAME_UNQ (nt), BuildArgs2 (nt, rc)))

#define SAC_ND_INC_RC(nt, rc)                                                            \
    CAT4 (SAC_ND_INC_RC__, CAT4 (NAME_UNQ (nt), BuildArgs2 (nt, rc)))

#define SAC_ND_DEC_RC(nt, rc)                                                            \
    CAT4 (SAC_ND_DEC_RC__, CAT4 (NAME_UNQ (nt), BuildArgs2 (nt, rc)))

#define SAC_ND_DEC_RC_FREE(nt, rc, freefun)                                              \
    CAT4 (SAC_ND_DEC_RC_FREE__, CAT4 (NAME_UNQ (nt), BuildArgs3 (nt, rc, freefun)))

/*
 * ICMs for AKS, AKD, HID
 */

#define SAC_ND_SET_RC_NUQ(nt, rc)                                                        \
    {                                                                                    \
        SAC_ND_A_RC (nt) = rc;                                                           \
        SAC_TR_REF_PRINT (("ND_SET_RC( %s, %d)", #nt, rc));                              \
        SAC_TR_REF_PRINT_RC (nt)                                                         \
    }
#define SAC_ND_SET_RC_UNQ(nt) ICM_UNDEF

#define SAC_ND_INC_RC_NUQ(nt, rc)                                                        \
    {                                                                                    \
        SAC_ND_A_RC (nt) += rc;                                                          \
        SAC_TR_REF_PRINT (("ND_INC_RC( %s, %d)", #nt, rc));                              \
        SAC_TR_REF_PRINT_RC (nt)                                                         \
    }
#define SAC_ND_INC_RC_UNQ(nt) ICM_UNDEF

#define SAC_ND_DEC_RC_NUQ(nt, rc)                                                        \
    {                                                                                    \
        SAC_ND_A_RC (nt) -= rc;                                                          \
        SAC_TR_REF_PRINT (("ND_DEC_RC( %s, %d)", #nt, rc));                              \
        SAC_TR_REF_PRINT_RC (nt)                                                         \
    }
#define SAC_ND_DEC_RC_UNQ(nt) ICM_UNDEF

#define SAC_ND_DEC_RC_FREE_NUQ(nt, rc, freefun)                                          \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_DEC_RC_FREE( %s, %d, %s)", #nt, rc, #freefun));           \
        if ((SAC_ND_A_RC (nt) -= rc) == 0) {                                             \
            SAC_TR_REF_PRINT_RC (nt);                                                    \
            SAC_ND_FREE (nt, freefun);                                                   \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (nt);                                                    \
        }                                                                                \
    }
#define SAC_ND_DEC_RC_FREE_UNQ(nt) ICM_UNDEF

#endif /* SAC_STD_H */

#if 0

/*
 * ICMs for creating refcounted data:
 * ====================================
 *
 * ND_ALLOC_RC(nt)
 *   allocates memory for refcount (no initialization)
 *
 * ND_CREATE_CONST_ARRAY_C( nt, str)
 *   creates a constant character array (string)
 *   Also see ND_CREATE_CONST_ARRAY_S for the creation of scalar arrays.
 */

#define SAC_ND_ALLOC_RC(nt) SAC_ND_A_RCP (nt) = (int *)SAC_MALLOC (sizeof (int));

#define SAC_ND_CREATE_CONST_ARRAY_C(nt, str) SAC_String2Array (nt, str);




  

/*
 * ICMs for reference counting :
 * ===============================
 *
 * ND_AKS_MAKE_UNIQUE_ARRAY(old, new, basetypesize)
 *   assigns old to new if refcount is zero and copies the array otherwise
 *   A new refcount is allocated if necessary
 *
 * ND_MAKE_UNIQUE_HIDDEN(old, new, copyfun)
 *   assigns old to new if refcount is zero and copies the hidden otherwise
 *   A new refcount is allocated if necessary
 *
 * ND_AKS_NO_RC_MAKE_UNIQUE_ARRAY(old, new, basetypesize)
 *   assigns old to new if refcount is zero and copies the array otherwise
 *   No new refcount is allocated, the old one is freed when not copying
 *
 * ND_NO_RC_MAKE_UNIQUE_HIDDEN(old, new, copyfun)
 *   assigns old to new if refcount is zero and copies the hidden otherwise
 *   No new refcount is allocated, the old one is freed when not copying
 */

#define SAC_ND_MAKE_UNIQUE_HIDDEN(old, new, copyfun)                                     \
    {                                                                                    \
        SAC_TR_MEM_PRINT (("ND_MAKE_UNIQUE_HIDDEN(%s, %s, %s)", #old, #new, #copyfun));  \
        SAC_TR_REF_PRINT_RC (old);                                                       \
        if (SAC_ND_A_RC (old) == 1) {                                                    \
            SAC_ND_ASSIGN_HIDDEN (old, new);                                             \
            SAC_TR_MEM_PRINT (("%s is already unique.", old));                           \
        } else {                                                                         \
            SAC_ND_ALLOC_RC (new);                                                       \
            SAC_ND_COPY_HIDDEN (old, new, copyfun);                                      \
            SAC_ND_DEC_RC (old, 1);                                                      \
        }                                                                                \
    }

#define SAC_ND_AKS_MAKE_UNIQUE_ARRAY(old, new, basetypesize)                             \
    {                                                                                    \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_AKS_MAKE_UNIQUE_ARRAY(%s, %s, %d)", #old, #new, basetypesize));           \
        SAC_TR_REF_PRINT_RC (old);                                                       \
        if ((SAC_ND_A_RC (old)) == 1) {                                                  \
            SAC_ND_AKS_ASSIGN_ARRAY (old, new);                                          \
            SAC_TR_MEM_PRINT (("%s is already unique.", old));                           \
        } else {                                                                         \
            SAC_ND_AKS_COPY_ARRAY (old, new, basetypesize);                              \
            SAC_ND_ALLOC_RC (new);                                                       \
            SAC_ND_DEC_RC (old, 1);                                                      \
        }                                                                                \
    }

#define SAC_ND_NO_RC_MAKE_UNIQUE_HIDDEN(old, new, copyfun)                               \
    {                                                                                    \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_NO_RC_MAKE_UNIQUE_HIDDEN(%s, %s, %s)", #old, #new, #copyfun));            \
        SAC_TR_REF_PRINT_RC (old);                                                       \
        if (SAC_ND_A_RC (old) == 1) {                                                    \
            SAC_ND_NO_RC_ASSIGN_HIDDEN (old, new);                                       \
            SAC_FREE (SAC_ND_A_RCP (old));                                               \
            SAC_TR_MEM_PRINT (("%s is already unique.", old));                           \
        } else {                                                                         \
            SAC_ND_COPY_HIDDEN (old, new, copyfun);                                      \
            SAC_ND_DEC_RC (old, 1);                                                      \
        }                                                                                \
    }

#define SAC_ND_KS_NO_RC_MAKE_UNIQUE_ARRAY(old, new, basetypesize)                        \
    {                                                                                    \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_KS_NO_RC_MAKE_UNIQUE_ARRAY(%s, %s, %d)", #old, #new, basetypesize));      \
        SAC_TR_REF_PRINT_RC (old);                                                       \
        if ((SAC_ND_A_RC (old)) == 1) {                                                  \
            SAC_ND_KS_NO_RC_ASSIGN_ARRAY (old, new);                                     \
            SAC_FREE (SAC_ND_A_RCP (old));                                               \
            SAC_TR_MEM_PRINT (("%s is already unique.", old));                           \
        } else {                                                                         \
            SAC_ND_AKS_COPY_ARRAY (old, new, basetypesize);                              \
            SAC_ND_DEC_RC (old, 1);                                                      \
        }                                                                                \
    }

#endif /* 0 */
