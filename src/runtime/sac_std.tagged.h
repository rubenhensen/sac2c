/*
 *
 * $Log$
 * Revision 3.4  2002/06/02 21:50:04  dkr
 * some bugs fixed
 *
 * Revision 3.3  2002/05/03 13:57:09  dkr
 * macros updated
 *
 * Revision 3.2  2002/03/07 20:15:28  dkr
 * code brushed
 *
 * Revision 3.1  2000/11/20 18:02:21  sacbase
 * new release made
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

#ifndef _SAC_STD_H
#define _SAC_STD_H

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
 *   class:  SCL: scalar
 *           AKS: array of known shape
 *           AKD: array of known dimension but unknown shape
 *           AUD: array of unknown dimension
 *           HID: hidden object
 *
 *   unique: NUQ: data object is non-unique
 *           UNQ: data object is unique
 */

#define NT_NAME(nt) Item0 nt
#define NT_DATA(nt) Item1 nt
#define NT_UNQ(nt) Item2 nt

/*
 * Array descriptor used by dynamic shape support
 */

#define MAXDIM 10

typedef struct {
    int rc;          /* reference count for array */
    int dim;         /* # of dimensions in array */
    int sz;          /* number of elements in array */
    int shp[MAXDIM]; /* shape vector for array */
} SAC_array_descriptor;

/******************************************************************************
 *
 * ICMs for descriptor access
 * ==========================
 *
 * ND_A_DESC( nt) :
 *   accesses the descriptor of the data object
 * ND_A_FIELD( nt) :
 *   accesses the pointer to the data object (array)
 *   or the data object (scalar) respectively
 *
 * ND_A_RC( nt) :
 *   accesses the reference counter
 * ND_A_DIM( nt) :
 *   accesses the dimension
 * ND_A_SIZE( nt) :
 *   accesses the size of the unrolling (in elements)
 * ND_A_SHAPE( nt, dim) :
 *   accesses one shape component
 *
 ******************************************************************************/

#define SAC_ND_A_DESC(nt) CAT0 (SAC_ND_A_DESC__, CAT0 (NT_DATA (nt), (nt)))

#define SAC_ND_A_FIELD(nt) CAT1 (SAC_ND_A_FIELD__, CAT1 (NT_DATA (nt), (nt)))

#define SAC_ND_A_RC(nt)                                                                  \
    CAT1 (SAC_ND_A_RC__, CAT1 (NT_DATA (nt), CAT1 (_, CAT1 (NT_UNQ (nt), (nt)))))

#define SAC_ND_A_DIM(nt) CAT1 (SAC_ND_A_DIM__, CAT1 (NT_DATA (nt), (nt)))

#define SAC_ND_A_SIZE(nt) CAT1 (SAC_ND_A_SIZE__, CAT1 (NT_DATA (nt), (nt)))

#define SAC_ND_A_SHAPE(nt, dim)                                                          \
    CAT1 (SAC_ND_A_SHAPE__, CAT1 (NT_DATA (nt), BuildArgs2 (nt, dim)))

/*
 * SCL
 */

#define SAC_ND_A_DESC__SCL(nt) ICM_UNDEF

#define SAC_ND_A_FIELD__SCL(nt) NT_NAME (nt)

#define SAC_ND_A_RC__SCL_NUQ(nt) ICM_UNDEF
#define SAC_ND_A_RC__SCL_UNQ(nt) ICM_UNDEF

#define SAC_ND_A_DIM__SCL(nt) 0

#define SAC_ND_A_SIZE__SCL(nt) 1

#define SAC_ND_A_SHAPE__SCL(nt, dim) ICM_UNDEF

/*
 * AKS
 */

#define SAC_ND_A_DESC__AKS(nt) CAT2 (NT_NAME (nt), __desc)

#define SAC_ND_A_FIELD__AKS(nt) NT_NAME (nt)

#define SAC_ND_A_RC__AKS_NUQ(nt) CAT2 (SAC_ND_A_DESC (nt), ->rc)
#define SAC_ND_A_RC__AKS_UNQ(nt) ICM_UNDEF

#define SAC_ND_A_DIM__AKS(nt) CAT2 (NT_NAME (nt), __dim)

#define SAC_ND_A_SIZE__AKS(nt) CAT2 (NT_NAME (nt), __sz)

#define SAC_ND_A_SHAPE__AKS(nt, dim) CAT2 (NT_NAME (nt), CAT0 (__shp, dim))

/*
 * AKD
 */

#define SAC_ND_A_DESC__AKD(nt) SAC_ND_A_DESC__AKS (nt)

#define SAC_ND_A_FIELD__AKD(nt) SAC_ND_A_FIELD__AKS (nt)

#define SAC_ND_A_RC__AKD_NUQ(nt) SAC_ND_A_RC__AKS_NUQ (nt)
#define SAC_ND_A_RC__AKD_UNQ(nt) SAC_ND_A_RC__AKS_UNQ (nt)

#define SAC_ND_A_DIM__AKD(nt) SAC_ND_A_DIM__AKS (nt)

#define SAC_ND_A_SIZE__AKD(nt) SAC_ND_A_SIZE__AKS (nt)

#define SAC_ND_A_SHAPE__AKD(nt, dim) SAC_ND_A_SHAPE__AKS (nt, dim)

/*
 * AUD
 */

#define SAC_ND_A_DESC__AUD(nt) SAC_ND_A_DESC__AKS (nt)

#define SAC_ND_A_FIELD__AUD(nt) SAC_ND_A_FIELD__AKS (nt)

#define SAC_ND_A_RC__AUD_NUQ(nt) SAC_ND_A_RC__AKS_NUQ (nt)
#define SAC_ND_A_RC__AUD_UNQ(nt) SAC_ND_A_RC__AKS_UNQ (nt)

#define SAC_ND_A_DIM__AUD(nt) SAC_ND_A_DIM__AKS (nt)

#define SAC_ND_A_SIZE__AUD(nt) SAC_ND_A_SIZE__AKS (nt)

#define SAC_ND_A_SHAPE__AUD(nt, dim) CAT2 (SAC_ND_A_DESC (nt), ->shp[dim])

/*
 * HID
 */

#define SAC_ND_A_DESC__HID(nt) CAT2 (NT_NAME (nt), __rc)

#define SAC_ND_A_FIELD__HID(nt) NT_NAME (nt)

#define SAC_ND_A_RC__HID_NUQ(nt) CAT2 (*, SAC_ND_A_DESC (nt))
#define SAC_ND_A_RC__HID_UNQ(nt) ICM_UNDEF

#define SAC_ND_A_DIM__HID(nt) ICM_UNDEF

#define SAC_ND_A_SIZE__HID(nt) ICM_UNDEF

#define SAC_ND_A_SHAPE__HID(nt, dim) ICM_UNDEF

/******************************************************************************
 *
 * ICMs for read/write access
 * ==========================
 *
 * Only these two ICMs should be used to access the elements of an array as
 * they selectively enable boundary checking and cache simulation!
 *
 * ND_READ( nt, pos) :  read access at specified index position
 * ND_WRITE( nt, pos) : write access at specified index position
 *
 ******************************************************************************/

#define SAC_ND_READ(nt, pos)                                                             \
    CAT2 (SAC_ND_READ__, CAT2 (NT_DATA (nt), BuildArgs2 (nt, pos)))

#define SAC_ND_WRITE(nt, pos)                                                            \
    CAT2 (SAC_ND_WRITE__, CAT2 (NT_DATA (nt), BuildArgs2 (nt, pos)))

/*
 * SCL
 */

#define SAC_ND_READ__SCL(nt, pos) SAC_ND_A_FIELD (nt)

#define SAC_ND_WRITE__SCL(nt, pos) SAC_ND_A_FIELD (nt)

/*
 * AKS
 */

#define SAC_ND_READ__AKS(nt, pos)                                                        \
    (SAC_BC_READ (nt, pos) SAC_CS_READ_ARRAY (nt, pos) SAC_ND_A_FIELD (nt)[pos])

#define SAC_ND_WRITE__AKS(nt, pos)                                                       \
    SAC_BC_WRITE (nt, pos)                                                               \
    SAC_CS_WRITE_ARRAY (nt, pos)                                                         \
    SAC_ND_A_FIELD (nt)[pos]

/*
 * AKD
 */

#define SAC_ND_READ__AKD(nt, pos) SAC_ND_READ__AKS (nt, pos)

#define SAC_ND_WRITE__AKD(nt, pos) SAC_ND_WRITE__AKS (nt, pos)

/*
 * AUD
 */

#define SAC_ND_READ__AUD(nt, pos) SAC_ND_READ__AKS (nt, pos)

#define SAC_ND_WRITE__AUD(nt, pos) SAC_ND_WRITE__AKS (nt, pos)

/*
 * HID
 */

#define SAC_ND_READ__HID(nt, pos) ICM_UNDEF

#define SAC_ND_WRITE__HID(nt, pos) ICM_UNDEF

/******************************************************************************
 *
 * ICMs for types
 * =================
 *
 * ND_TYPE__<CLASS>( basetype) : type implementation
 *
 ******************************************************************************/

#define SAC_ND_TYPE__SCL(basetype) basetype

#define SAC_ND_TYPE__AKS(basetype) basetype *

#define SAC_ND_TYPE__AKD(basetype) SAC_ND_TYPE__AKS (basetype)

#define SAC_ND_TYPE__AUD(basetype) SAC_ND_TYPE__AKS (basetype)

#define SAC_ND_TYPE__HID(basetype) void *

/******************************************************************************
 *
 * ICMs for typedefs
 * =================
 *
 * ND_TYPEDEF( nt, basetype) : type definition
 *
 ******************************************************************************/

#define SAC_ND_TYPEDEF(nt, basetype)                                                     \
    typedef CAT2 (SAC_ND_TYPE__, CAT2 (NT_DATA (nt), (basetype))) NT_NAME (nt)

/******************************************************************************
 *
 * ICMs for objdefs
 * ================
 *
 * ND_OBJDEF( nt, basetype, dim, ...shp...) :
 *   declaration of an internal object
 * ND_OBJDEF_EXTERN( nt, basetype, dim, ...shp...) :
 *   declaration of an external object
 *
 ******************************************************************************/

/* ND_OBJDEF( ...)  is a C-ICM */

/* ND_OBJDEF_EXTERN( ...)  is a C-ICM */

/******************************************************************************
 *
 * ICMs for declaration and allocation of data objects
 * ===================================================
 *
 * ND_DECL( nt, basetype, dim, ...shp...) :
 *   declares a data object
 *
 * ND_ALLOC_DESC( nt) :
 *   allocates memory for descriptor (no initialization!)
 * ND_ALLOC( nt, rc, basetype, dim, ...shp...) :
 *   allocates a data object (no initialization!)
 *
 * ND_FREE_DESC( nt) :
 *   frees memory for descriptor
 * ND_FREE( nt, freefun) :
 *   frees a data object
 *
 *************
 *
 * NOTE:
 * SECURE_ALLOC_FREE defined -> an extra check whether the requested size is 0
 * is inserted into the code. In that case no call for malloc(0) is executed,
 * but the variable is directly set to NULL.
 *
 ******************************************************************************/

/* ND_DECL( ...)  is a C-ICM */

#define SAC_ND_ALLOC_DESC(nt)                                                            \
    CAT3 (SAC_ND_ALLOC_DESC__, CAT3 (NT_DATA (nt), CAT3 (_, CAT3 (NT_UNQ (nt), (nt)))))

/* ND_ALLOC( ...)  is a C-ICM */

#define SAC_ND_FREE_DESC(nt)                                                             \
    CAT3 (SAC_ND_FREE_DESC__, CAT3 (NT_DATA (nt), CAT3 (_, CAT3 (NT_UNQ (nt), (nt)))))

#define SAC_ND_FREE(nt, freefun)                                                         \
    CAT4 (SAC_ND_FREE__,                                                                 \
          CAT4 (NT_DATA (nt), CAT4 (_, CAT4 (NT_UNQ (nt), BuildArgs2 (nt, freefun)))))

/*
 * misc
 */

#ifdef SECURE_ALLOC_FREE
#define _SAC_ND_FREE_1_(nt)                                                              \
    if (SAC_ND_A_SIZE (nt) != 0) {                                                       \
        SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_FIELD (nt),                                     \
                                SAC_ND_A_SIZE (nt) * sizeof (*SAC_ND_A_FIELD (nt)));     \
    }
#else
#define _SAC_ND_FREE_1_(nt)                                                              \
    SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_FIELD (nt),                                         \
                            SAC_ND_A_SIZE (nt) * sizeof (*SAC_ND_A_FIELD (nt)))
#endif

#define _SAC_ND_FREE_2_(nt)                                                              \
    SAC_TR_MEM_PRINT (("ND_FREE_ARRAY( %s) at addr: %p", #nt, NT_NAME (nt)));            \
    SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (nt));                                        \
    SAC_CS_UNREGISTER_ARRAY (nt);

/*
 * SCL
 */

#define SAC_ND_ALLOC_DESC__SCL_NUQ(nt)
#define SAC_ND_ALLOC_DESC__SCL_UNQ(nt)

#define SAC_ND_FREE_DESC__SCL_NUQ(nt)
#define SAC_ND_FREE_DESC__SCL_UNQ(nt)

#define SAC_ND_FREE__SCL_NUQ(nt, freefun)
#define SAC_ND_FREE__SCL_UNQ(nt, freefun)

/*
 * AKS
 */

#define SAC_ND_ALLOC_DESC__AKS_NUQ(nt)                                                   \
    SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_DESC (nt), sizeof (*SAC_ND_A_DESC (nt)));
#define SAC_ND_ALLOC_DESC__AKS_UNQ(nt)

#define SAC_ND_FREE_DESC__AKS_NUQ(nt)                                                    \
    SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_DESC (nt), sizeof (*SAC_ND_A_DESC (nt)));
#define SAC_ND_FREE_DESC__AKS_UNQ(nt)

#define SAC_ND_FREE__AKS_NUQ(nt, freefun)                                                \
    {                                                                                    \
        _SAC_ND_FREE_1_ (nt);                                                            \
        SAC_ND_FREE_DESC (nt);                                                           \
        SAC_TR_MEM_PRINT (("ND_FREE( %s, ) at addr: %p", #nt, NT_NAME (nt)));            \
        _SAC_ND_FREE_2_ (nt);                                                            \
    }
#define SAC_ND_FREE__AKS_UNQ(nt, freefun)                                                \
    {                                                                                    \
        _SAC_ND_FREE_1_ (nt);                                                            \
        SAC_TR_MEM_PRINT (("ND_FREE( %s, ) at addr: %p", #nt, NT_NAME (nt)));            \
        _SAC_ND_FREE_2_ (nt);                                                            \
    }

/*
 * AKD
 */

#define SAC_ND_ALLOC_DESC__AKD_NUQ(nt) SAC_ND_ALLOC_DESC__AKS_NUQ (nt)
#define SAC_ND_ALLOC_DESC__AKD_UNQ(nt) SAC_ND_ALLOC_DESC__AKS_UNQ (nt)

#define SAC_ND_FREE_DESC__AKD_NUQ(nt) SAC_ND_FREE_DESC__AKS_NUQ (nt)
#define SAC_ND_FREE_DESC__AKD_UNQ(nt) SAC_ND_FREE_DESC__AKS_UNQ (nt)

#define SAC_ND_FREE__AKD_NUQ(nt, freefun) SAC_ND_FREE__AKS_NUQ (nt, freefun)
#define SAC_ND_FREE__AKD_UNQ(nt, freefun) SAC_ND_FREE__AKS_UNQ (nt, freefun)

/*
 * AUD
 */

#define SAC_ND_ALLOC_DESC__AUD_NUQ(nt) SAC_ND_ALLOC_DESC__AKS_NUQ (nt)
#define SAC_ND_ALLOC_DESC__AUD_UNQ(nt) SAC_ND_ALLOC_DESC__AKS_UNQ (nt)

#define SAC_ND_FREE_DESC__AUD_NUQ(nt) SAC_ND_FREE_DESC__AKS_NUQ (nt)
#define SAC_ND_FREE_DESC__AUD_UNQ(nt) SAC_ND_FREE_DESC__AKS_UNQ (nt)

#define SAC_ND_FREE__AUD_NUQ(nt, freefun) SAC_ND_FREE__AKS_NUQ (nt, freefun)
#define SAC_ND_FREE__AUD_UNQ(nt, freefun) SAC_ND_FREE__AKS_UNQ (nt, freefun)

/*
 * HID
 */

#define SAC_ND_ALLOC_DESC__HID_NUQ(nt) SAC_ND_ALLOC_DESC__AKS_NUQ (nt)
#define SAC_ND_ALLOC_DESC__HID_UNQ(nt)
SAC_ND_ALLOC_DESC__AKS_UNQ (nt)

#define SAC_ND_FREE_DESC__HID_NUQ(nt) SAC_ND_FREE_DESC__AKS_NUQ (nt)
#define SAC_ND_FREE_DESC__HID_UNQ(nt) SAC_ND_FREE_DESC__AKS_UNQ (nt)

#define SAC_ND_FREE__HID_NUQ(nt, freefun)                                                \
    {                                                                                    \
        freefun (NT_NAME (nt));                                                          \
        SAC_ND_FREE_DESC (nt);                                                           \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_FREE( %s, %s) at addr: %p", #nt, #freefun, NT_NAME (nt)));                \
        SAC_TR_DEC_HIDDEN_MEMCNT (1);                                                    \
    }
#define SAC_ND_FREE__HID_UNQ(nt, freefun)                                                \
    {                                                                                    \
        freefun (NT_NAME (nt));                                                          \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_FREE( %s, %s) at addr: %p", #nt, #freefun, NT_NAME (nt)));                \
        SAC_TR_DEC_HIDDEN_MEMCNT (1);                                                    \
    }

/******************************************************************************
 *
 * ICMs for passing data objects to functions
 * ==========================================
 *
 * ND_PARAM_in( type, nt)
 *   macro for prototyping data as "in" parameter
 *
 * ND_PARAM_in_nodesc( type, nt)
 *   macro for prototyping data as "in" parameter without descriptor
 *
 * ND_PARAM_out( type, nt)
 *   macro for prototyping data as "out" parameter
 *
 * ND_PARAM_out_nodesc( type, nt)
 *   macro for prototyping data as "out" parameter without descriptor
 *
 * ND_PARAM_inout( type, nt)
 *   macro for prototyping data as "inout" parameter
 *
 * ND_PARAM_inout_nodesc( type, nt)
 *   macro for prototyping data as "inout" parameter without descriptor
 *
 * ND_PARAM_inout_nodesc_bx( type, nt)
 *   macro for prototyping boxed data as "inout" parameter without descriptor
 *
 * ND_ARG_in( nt)
 *   macro for giving data as "in" parameter
 *
 * ND_ARG_in_nodesc( nt)
 *   macro for giving data as "in" parameter without descriptor
 *
 * ND_ARG_out( nt)
 *   macro for getting data as "out" parameter
 *
 * ND_ARG_out_nodesc( nt)
 *   macro for getting data as "out" parameter without descriptor
 *
 * ND_ARG_inout( nt)
 *   macro for giving data as "inout" parameter
 *
 * ND_ARG_inout_nodesc( nt)
 *   macro for giving data as "inout" argument without descriptor
 *
 * ND_ARG_inout_nodesc_bx( nt)
 *   macro for giving boxed data as "inout" argument without descriptor
 *
 * ND_RET_out( nt, ntp)
 *   macro for returning data
 *
 * ND_RET_inout( nt, ntp)
 *   macro for returning "inout" data
 *
 * ND_DECL_PARAM_inout( type, nt)
 *   macro for declaration of "inout" parameter
 *
 ******************************************************************************/

/* creates name for formal function parameter */
#define SAC_NAMEP(name) CAT0 (name, __p)

#define SAC_ND_PARAM_in(type, nt)                                                        \
    CAT3 (SAC_ND_PARAM_in__, CAT3 (NT_DATA (nt), BuildArgs2 (type, nt)))

#define SAC_ND_PARAM_in_nodesc(type, nt) type SAC_ND_A_FIELD (nt)

#define SAC_ND_PARAM_out(type, nt)                                                       \
    CAT3 (SAC_ND_PARAM_out__, CAT3 (NT_DATA (nt), BuildArgs2 (type, nt)))

#define SAC_ND_PARAM_out_nodesc(type, nt) type *SAC_NAMEP (SAC_ND_A_FIELD (nt))

#define SAC_ND_PARAM_inout(type, nt) SAC_ND_PARAM_out (type, nt)

#define SAC_ND_PARAM_inout_nodesc(type, nt) SAC_ND_PARAM_out_nodesc (type, nt)

#define SAC_ND_PARAM_inout_nodesc_bx(type, nt) SAC_ND_PARAM_in_nodesc (type, nt)

#define SAC_ND_ARG_in(nt) CAT3 (SAC_ND_ARG_in__, CAT3 (NT_DATA (nt), (nt)))

#define SAC_ND_ARG_in_nodesc(nt) SAC_ND_A_FIELD (nt)

#define SAC_ND_ARG_out(nt) CAT3 (SAC_ND_ARG_out__, CAT3 (NT_DATA (nt), (nt)))

#define SAC_ND_ARG_out_nodesc(nt) &SAC_ND_A_FIELD (nt)

#define SAC_ND_ARG_inout(nt) SAC_ND_ARG_out (nt)

#define SAC_ND_ARG_inout_nodesc(nt) SAC_ND_ARG_out_nodesc (nt)

#define SAC_ND_ARG_inout_nodesc_bx(nt) SAC_ND_ARG_in_nodesc (nt)

#define SAC_ND_RET_out(nt, ntp)                                                          \
    CAT3 (SAC_ND_RET_out__, CAT3 (NT_DATA (nt), BuildArgs2 (nt, ntp)))

#define SAC_ND_RET_inout(nt, ntp) SAC_ND_RET_out (nt, ntp)

#define SAC_ND_DECL_PARAM_inout(type, nt)                                                \
    CAT3 (SAC_ND_DECL_PARAM_inout__, CAT3 (NT_DATA (nt), BuildArgs2 (type, nt)))

/*
 * SCL
 */

#define SAC_ND_PARAM_in__SCL(type, nt) SAC_ND_PARAM_in_nodesc (type, nt)

#define SAC_ND_PARAM_out__SCL(type, nt) SAC_ND_PARAM_out_nodesc (type, nt)

#define SAC_ND_ARG_in__SCL(nt) SAC_ND_ARG_in_nodesc (nt)

#define SAC_ND_ARG_out__SCL(nt) SAC_ND_ARG_out_nodesc (nt)

#define SAC_ND_RET_out__SCL(nt, ntp)                                                     \
    *SAC_NAMEP (SAC_ND_A_FIELD (ntp)) = SAC_ND_A_FIELD (nt);

#define SAC_ND_DECL_PARAM_inout__SCL(type, nt)                                           \
    type SAC_ND_A_FIELD (nt) = *SAC_NAMEP (SAC_ND_A_FIELD (nt));

/*
 * AKS
 */

#define SAC_ND_PARAM_in__AKS(type, nt)                                                   \
    SAC_ND_PARAM_in_nodesc (type, nt), SAC_array_descriptor *SAC_ND_A_DESC (nt)

#define SAC_ND_PARAM_out__AKS(type, nt)                                                  \
    SAC_ND_PARAM_out_nodesc (type, nt),                                                  \
      SAC_array_descriptor **SAC_NAMEP (SAC_ND_A_DESC (nt))

#define SAC_ND_ARG_in__AKS(nt) SAC_ND_ARG_in_nodesc (nt), SAC_ND_A_DESC (nt)

#define SAC_ND_ARG_out__AKS(nt) SAC_ND_ARG_out_nodesc (nt), &SAC_ND_A_DESC (nt)

#define SAC_ND_RET_out__AKS(nt, ntp)                                                     \
    {                                                                                    \
        *SAC_NAMEP (SAC_ND_A_FIELD (ntp)) = SAC_ND_A_FIELD (nt);                         \
        *SAC_NAMEP (SAC_ND_A_DESC (ntp)) = SAC_ND_A_DESC (nt);                           \
    }

#define SAC_ND_DECL_PARAM_inout__AKS(type, nt)                                           \
    type SAC_ND_A_FIELD (nt) = *SAC_NAMEP (SAC_ND_A_FIELD (nt));                         \
    SAC_array_descriptor *SAC_ND_A_DESC (nt) = *SAC_NAMEP (SAC_ND_A_DESC (nt));

/*
 * AKD
 */

#define SAC_ND_PARAM_in__AKD(type, nt) SAC_ND_PARAM_in__AKS (type, nt)

#define SAC_ND_PARAM_out__AKD(type, nt) SAC_ND_PARAM_out__AKS (type, nt)

#define SAC_ND_ARG_in__AKD(nt) SAC_ND_ARG_in__AKS (nt)

#define SAC_ND_ARG_out__AKD(nt) SAC_ND_ARG_out__AKS (nt)

#define SAC_ND_RET_out__AKD(nt, ntp) SAC_ND_RET_out__AKS (nt, ntp)

#define SAC_ND_DECL_PARAM_inout__AKD(type, nt) SAC_ND_DECL_PARAM_inout__AKS (type, nt)

/*
 * AUD
 */

#define SAC_ND_PARAM_in__AUD(type, nt) SAC_ND_PARAM_in__AKS (type, nt)

#define SAC_ND_PARAM_out__AUD(type, nt) SAC_ND_PARAM_out__AKS (type, nt)

#define SAC_ND_ARG_in__AUD(nt) SAC_ND_ARG_in__AKS (nt)

#define SAC_ND_ARG_out__AUD(nt) SAC_ND_ARG_out__AKS (nt)

#define SAC_ND_RET_out__AUD(nt, ntp) SAC_ND_RET_out__AKS (nt, ntp)

#define SAC_ND_DECL_PARAM_inout__AUD(type, nt) SAC_ND_DECL_PARAM_inout__AKS (type, nt)

/*
 * HID
 */

#define SAC_ND_PARAM_in__HID(type, nt) SAC_ND_PARAM_in__AKS (type, nt)

#define SAC_ND_PARAM_out__HID(type, nt) SAC_ND_PARAM_out__AKS (type, nt)

#define SAC_ND_ARG_in__HID(nt) SAC_ND_ARG_in__AKS (nt)

#define SAC_ND_ARG_out__HID(nt) SAC_ND_ARG_out__AKS (nt)

#define SAC_ND_RET_out__HID(nt, ntp) SAC_ND_RET_out__AKS (nt, ntp)

#define SAC_ND_DECL_PARAM_inout__HID(type, nt) SAC_ND_DECL_PARAM_inout__AKS (type, nt)

/******************************************************************************
 *
 * ICMs for assigning data objects
 * ===============================
 *
 * ND_ASSIGN( to_nt, from_nt) :
 *   assigns a data object to another one
 * ND_COPY( to_nt, from_nt, copyfun/basetype) :
 *   copies a data object to another one
 *
 * ND_CHECK_REUSE( to_nt, from_nt, copyfun) :
 *   tries to reuse old data object for new, copies if impossible
 *
 * ND_MAKE_UNIQUE( new_nt, old_nt, basetypesize) :
 *   assigns a data object to another one iff RC is zero, copies it otherwise.
 *   A new refcount is allocated if necessary.
 *
 ******************************************************************************/

/* ND_ASSIGN( ...)  is a C-ICM */

/* ND_COPY( ...)  is a C-ICM */

/* ND_CHECK_REUSE( ...)  is a C-ICM */

/* ND_MAKE_UNIQUE( ...)  is a C-ICM */

/******************************************************************************
 *
 * ICMs for creating refcounted objects:
 * ====================================
 *
 * ND_ASSIGN_CONST_STR( nt, str) :
 *   creates a constant character array (string)
 *
 * ND_ASSIGN_CONST_SCALAR( nt, val) :
 *   creates a constant scalar
 *
 * ND_ASSIGN_CONST_VECT( nt, copyfun, shp0, ...elem...) :
 *   creates a constant vector
 *
 ******************************************************************************/

#define SAC_ND_ASSIGN_CONST_STR(nt, str) SAC_String2Array (NT_NAME (nt), str);

#define SAC_ND_ASSIGN_CONST_SCALAR(nt, val) SAC_ND_WRITE (nt, 0) = val;

/* ND_ASSIGN_CONST_VECT( ...) is a C-ICM */

/******************************************************************************
 *
 * ICMs for refcounting data objects
 * =================================
 *
 * ND_SET_RC( nt, rc) :
 *   sets the refcount of a data object
 * ND_INC_RC( nt, rc) :
 *   increments the refcount of a data object
 * ND_DEC_RC( nt, rc) :
 *   decrements the refcount of a data object
 * ND_DEC_RC_FREE( nt, rc, freefun) :
 *   decrements the refcount and frees the data object if refcount becomes 0
 *
 ******************************************************************************/

#define SAC_ND_SET_RC(nt, rc)                                                            \
    CAT5 (SAC_ND_SET_RC__,                                                               \
          CAT5 (NT_DATA (nt), CAT5 (_, CAT5 (NT_UNQ (nt), BuildArgs2 (nt, rc)))))

#define SAC_ND_INC_RC(nt, rc)                                                            \
    CAT5 (SAC_ND_INC_RC__,                                                               \
          CAT5 (NT_DATA (nt), CAT5 (_, CAT5 (NT_UNQ (nt), BuildArgs2 (nt, rc)))))

#define SAC_ND_DEC_RC(nt, rc)                                                            \
    CAT5 (SAC_ND_DEC_RC__,                                                               \
          CAT5 (NT_DATA (nt), CAT5 (_, CAT5 (NT_UNQ (nt), BuildArgs2 (nt, rc)))))

#define SAC_ND_DEC_RC_FREE(nt, rc, freefun)                                              \
    CAT5 (SAC_ND_DEC_RC_FREE__,                                                          \
          CAT5 (NT_DATA (nt),                                                            \
                CAT5 (_, CAT5 (NT_UNQ (nt), BuildArgs3 (nt, rc, freefun)))))

/*
 * SCL
 */

#define SAC_ND_SET_RC__SCL_NUQ(nt, rc)
#define SAC_ND_SET_RC__SCL_UNQ(nt, rc)

#define SAC_ND_INC_RC__SCL_NUQ(nt, rc)
#define SAC_ND_INC_RC__SCL_UNQ(nt, rc)

#define SAC_ND_DEC_RC__SCL_NUQ(nt, rc)
#define SAC_ND_DEC_RC__SCL_UNQ(nt, rc)

#define SAC_ND_DEC_RC_FREE__SCL_NUQ(nt, rc, freefun)
#define SAC_ND_DEC_RC_FREE__SCL_UNQ(nt, rc, freefun)

/*
 * AKS
 */

#define SAC_ND_SET_RC__AKS_NUQ(nt, rc)                                                   \
    {                                                                                    \
        SAC_ND_A_RC (nt) = rc;                                                           \
        SAC_TR_REF_PRINT (("ND_SET_RC( %s, %d)", #nt, rc));                              \
        SAC_TR_REF_PRINT_RC (nt)                                                         \
    }
#define SAC_ND_SET_RC__AKS_UNQ(nt, rc)

#define SAC_ND_INC_RC__AKS_NUQ(nt, rc)                                                   \
    {                                                                                    \
        SAC_ND_A_RC (nt) += rc;                                                          \
        SAC_TR_REF_PRINT (("ND_INC_RC( %s, %d)", #nt, rc));                              \
        SAC_TR_REF_PRINT_RC (nt)                                                         \
    }
#define SAC_ND_INC_RC__AKS_UNQ(nt, rc)

#define SAC_ND_DEC_RC__AKS_NUQ(nt, rc)                                                   \
    {                                                                                    \
        SAC_ND_A_RC (nt) -= rc;                                                          \
        SAC_TR_REF_PRINT (("ND_DEC_RC( %s, %d)", #nt, rc));                              \
        SAC_TR_REF_PRINT_RC (nt)                                                         \
    }
#define SAC_ND_DEC_RC__AKS_UNQ(nt, rc)

#define SAC_ND_DEC_RC_FREE__AKS_NUQ(nt, rc, freefun)                                     \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_DEC_RC_FREE( %s, %d, %s)", #nt, rc, #freefun));           \
        if ((SAC_ND_A_RC (nt) -= rc) == 0) {                                             \
            SAC_TR_REF_PRINT_RC (nt);                                                    \
            SAC_ND_FREE (nt, freefun);                                                   \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (nt);                                                    \
        }                                                                                \
    }
#define SAC_ND_DEC_RC_FREE__AKS_UNQ(nt, rc, freefun)

/*
 * AKD
 */

#define SAC_ND_SET_RC__AKD_NUQ(nt, rc) SAC_ND_SET_RC__AKS_NUQ (nt, rc)
#define SAC_ND_SET_RC__AKD_UNQ(nt, rc) SAC_ND_SET_RC__AKS_UNQ (nt, rc)

#define SAC_ND_INC_RC__AKD_NUQ(nt, rc) SAC_ND_INC_RC__AKS_NUQ (nt, rc)
#define SAC_ND_INC_RC__AKD_UNQ(nt, rc) SAC_ND_INC_RC__AKS_UNQ (nt, rc)

#define SAC_ND_DEC_RC__AKD_NUQ(nt, rc) SAC_ND_DEC_RC__AKS_NUQ (nt, rc)
#define SAC_ND_DEC_RC__AKD_UNQ(nt, rc) SAC_ND_DEC_RC__AKS_UNQ (nt, rc)

#define SAC_ND_DEC_RC_FREE__AKD_NUQ(nt, rc, freefun)                                     \
    SAC_ND_DEC_RC_FREE__AKS_NUQ (nt, rc, freefun)
#define SAC_ND_DEC_RC_FREE__AKD_UNQ(nt, rc, freefun)                                     \
    SAC_ND_DEC_RC_FREE__AKS_UNQ (nt, rc, freefun)

/*
 * AUD
 */

#define SAC_ND_SET_RC__AUD_NUQ(nt, rc) SAC_ND_SET_RC__AKS_NUQ (nt, rc)
#define SAC_ND_SET_RC__AUD_UNQ(nt, rc) SAC_ND_SET_RC__AKS_UNQ (nt, rc)

#define SAC_ND_INC_RC__AUD_NUQ(nt, rc) SAC_ND_INC_RC__AKS_NUQ (nt, rc)
#define SAC_ND_INC_RC__AUD_UNQ(nt, rc) SAC_ND_INC_RC__AKS_UNQ (nt, rc)

#define SAC_ND_DEC_RC__AUD_NUQ(nt, rc) SAC_ND_DEC_RC__AKS_NUQ (nt, rc)
#define SAC_ND_DEC_RC__AUD_UNQ(nt, rc) SAC_ND_DEC_RC__AKS_UNQ (nt, rc)

#define SAC_ND_DEC_RC_FREE__AUD_NUQ(nt, rc, freefun)                                     \
    SAC_ND_DEC_RC_FREE__AKS_NUQ (nt, rc, freefun)
#define SAC_ND_DEC_RC_FREE__AUD_UNQ(nt, rc, freefun)                                     \
    SAC_ND_DEC_RC_FREE__AKS_UNQ (nt, rc, freefun)

/*
 * HID
 */

#define SAC_ND_SET_RC__HID_NUQ(nt, rc) SAC_ND_SET_RC__AKS_NUQ (nt, rc)
#define SAC_ND_SET_RC__HID_UNQ(nt, rc) SAC_ND_SET_RC__AKS_UNQ (nt, rc)

#define SAC_ND_INC_RC__HID_NUQ(nt, rc) SAC_ND_INC_RC__AKS_NUQ (nt, rc)
#define SAC_ND_INC_RC__HID_UNQ(nt, rc) SAC_ND_INC_RC__AKS_UNQ (nt, rc)

#define SAC_ND_DEC_RC__HID_NUQ(nt, rc) SAC_ND_DEC_RC__AKS_NUQ (nt, rc)
#define SAC_ND_DEC_RC__HID_UNQ(nt, rc) SAC_ND_DEC_RC__AKS_UNQ (nt, rc)

#define SAC_ND_DEC_RC_FREE__HID_NUQ(nt, rc, freefun)                                     \
    SAC_ND_DEC_RC_FREE__AKS_NUQ (nt, rc, freefun)
#define SAC_ND_DEC_RC_FREE__HID_UNQ(nt, rc, freefun)                                     \
    SAC_ND_DEC_RC_FREE__AKS_UNQ (nt, rc, freefun)

#endif /* _SAC_STD_H */
