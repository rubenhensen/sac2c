/*
 *
 * $Log$
 * Revision 3.28  2002/10/08 01:53:54  dkr
 * collisions with CAT?? macros removed
 *
 * Revision 3.26  2002/08/05 18:57:20  dkr
 * SAC_ND_A_MIRROR_... added
 *
 * Revision 3.25  2002/08/03 03:17:16  dkr
 * bug in SAC_ND_ALLOC__DATA__AKD fixed
 *
 * Revision 3.24  2002/08/02 20:58:54  dkr
 * bug in SAC_ND_ALLOC__DESC__AKS fixed
 *
 * Revision 3.23  2002/08/02 20:48:56  dkr
 * ..__DIM.. icms added
 *
 * Revision 3.22  2002/08/01 12:08:41  dkr
 * macros DESC_... added
 *
 * Revision 3.21  2002/07/31 16:35:28  dkr
 * tags reorganized: HID/NHD are seperate classes now
 *
 * Revision 3.20  2002/07/24 13:48:46  dkr
 * use of CAT? macros reorganized
 *
 * Revision 3.19  2002/07/23 16:22:04  dkr
 * minor changes done
 *
 * Revision 3.18  2002/07/15 19:44:06  dkr
 * bug with use of CAT? macros fixed
 *
 * Revision 3.17  2002/07/15 12:44:42  dkr
 * SAC_ND_PRF_... macros moved to sac_prf.h
 *
 * Revision 3.16  2002/07/12 20:43:53  dkr
 * bug in ND_PARAM_..., ND_ARG_..., ND_RET_... icms fixed
 *
 * Revision 3.15  2002/07/12 19:32:10  dkr
 * bug in SAC_IS_LASTREF__BLOCK_... fixed
 *
 * Revision 3.14  2002/07/12 19:11:15  dkr
 * ND_PARAM_() added
 *
 * Revision 3.13  2002/07/12 16:58:42  dkr
 * some comments corrected
 *
 * Revision 3.12  2002/07/12 15:50:27  dkr
 * first complete revision 8-))
 *
 * Revision 3.3  2002/05/03 13:57:09  dkr
 * macros updated
 *
 * Revision 3.1  2000/11/20 18:02:21  sacbase
 * new release made
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
 * Positional parameters for name tuples (nt):
 *   name,class,unique
 *
 * Values are:
 *   name:   data object name
 *
 *   shape:  SCL: scalar
 *           AKS: array of known shape
 *           AKD: array of known dimension but unknown shape
 *           AUD: array of unknown dimension
 *
 *   hidden: NUQ: data object is non-hidden
 *           HID: data object is hidden
 *
 *   unique: NUQ: data object is non-unique
 *           UNQ: data object is unique
 */

#define NT_NAME(nt) Item0 nt
#define NT_SHP(nt) Item1 nt
#define NT_HID(nt) Item2 nt
#define NT_UNQ(nt) Item3 nt

/*
 * implementation type for hidden objects
 */

typedef void *SAC_hidden;

/*
 * array descriptor:
 *   [0]     -> reference count
 *   [1]     -> # of dimensions
 *   [2]     -> # of elements
 *   [3,...] -> shape vector
 */
typedef int *SAC_array_descriptor_t;

/* (max.) dim of descriptor for allocation optimization */
#define MAX_DIM_OF_DESC 10

/* size of dimension-independent parts of the descriptor */
#define FIXED_SIZE_OF_DESC 3
/* size of dimension-dependent parts of the descriptor */
#define VAR_SIZE_OF_DESC 1
/* size of the descriptor = (FIXED_SIZE_OF_DESC + dim * VAR_SIZE_OF_DESC) */
#define SIZE_OF_DESC(dim) (FIXED_SIZE_OF_DESC + dim * VAR_SIZE_OF_DESC)

#define DESC_RC(desc) CAT1 (desc, [0])
#define DESC_DIM(desc) CAT1 (desc, [1])
#define DESC_SIZE(desc) CAT1 (desc, [2])
#define DESC_SHAPE(desc, pos) CAT1 (desc, [FIXED_SIZE_OF_DESC + pos])

/**********************************
 **********************************
 ***
 *** CAT2, CAT3, CAT4, CAT5 (desc)
 *** CAT6, CAT7, CAT8
 ***
 ***/

/******************************************************************************
 *
 * ICMs for descriptor access
 * ==========================
 *
 * ND_A_DESC( nt) :
 *   accesses the descriptor of the data object
 *
 * ND_A_DESC_DIM( nt) :
 *   accesses the dimension of the data object via descriptor(!)
 * ND_A_DESC_SIZE( nt) :
 *   accesses the size (number of elements) of the data object via descriptor(!)
 * ND_A_DESC_SHAPE( nt, dim) :
 *   accesses a shape component of the data object via descriptor(!)
 *
 * ND_A_MIRROR_DIM( nt) :
 *   accesses the dimension of the data object via mirror(!)
 * ND_A_MIRROR_SIZE( nt) :
 *   accesses the size of the data object via mirror(!)
 * ND_A_MIRROR_SHAPE( nt, dim) :
 *   accesses a shape component of the data object via mirror(!)
 *
 * ND_A_FIELD( nt) :
 *   accesses the pointer to the data object (array)
 *   or the data object (scalar) respectively
 *
 * ND_A_RC( nt) :
 *   accesses the reference counter
 * ND_A_DIM( nt) :
 *   accesses the dimension (via mirror if exists, via descriptor otherwise)
 * ND_A_SIZE( nt) :
 *   accesses the size (via mirror if exists, via descriptor otherwise)
 * ND_A_SHAPE( nt, dim) :
 *   accesses a shape component (via mirror if exists, via descriptor otherwise)
 *
 ******************************************************************************/

#define SAC_ND_A_DESC(nt) CAT2 (SAC_ND_A_DESC__, CAT2 (NT_SHP (nt), (nt)))

#define SAC_ND_A_DESC_DIM(nt) DESC_DIM (SAC_ND_A_DESC (nt))

#define SAC_ND_A_DESC_SIZE(nt) DESC_SIZE (SAC_ND_A_DESC (nt))

#define SAC_ND_A_DESC_SHAPE(nt, dim) DESC_SHAPE (SAC_ND_A_DESC (nt), dim)

#define SAC_ND_A_MIRROR_DIM(nt) CAT7 (SAC_ND_A_MIRROR_DIM__, CAT7 (NT_SHP (nt), (nt)))

#define SAC_ND_A_MIRROR_SIZE(nt) CAT7 (SAC_ND_A_MIRROR_SIZE__, CAT7 (NT_SHP (nt), (nt)))

#define SAC_ND_A_MIRROR_SHAPE(nt, dim)                                                   \
    CAT7 (SAC_ND_A_MIRROR_SHAPE__, CAT7 (NT_SHP (nt), BuildArgs2 (nt, dim)))

#define SAC_ND_A_FIELD(nt) CAT6 (SAC_ND_A_FIELD__, CAT6 (NT_SHP (nt), (nt)))

#define SAC_ND_A_RC(nt) CAT6 (SAC_ND_A_RC__, CAT6 (NT_UNQ (nt), (nt)))

#define SAC_ND_A_DIM(nt) CAT6 (SAC_ND_A_DIM__, CAT6 (NT_SHP (nt), (nt)))

#define SAC_ND_A_SIZE(nt) CAT6 (SAC_ND_A_SIZE__, CAT6 (NT_SHP (nt), (nt)))

#define SAC_ND_A_SHAPE(nt, dim)                                                          \
    CAT6 (SAC_ND_A_SHAPE__, CAT6 (NT_SHP (nt), BuildArgs2 (nt, dim)))

/*
 * NUQ
 */

#define SAC_ND_A_RC__NUQ(nt) CAT7 (SAC_ND_A_RC__, CAT7 (NT_SHP (nt), CAT7 (_NUQ, (nt))))

/*
 * UNQ
 */

#define SAC_ND_A_RC__UNQ(nt) SAC_ICM_UNDEF ()

/*
 * SCL
 */

#define SAC_ND_A_DESC__SCL(nt) CAT3 (SAC_ND_A_DESC__SCL_, CAT3 (NT_HID (nt), (nt)))
#define SAC_ND_A_DESC__SCL_NHD(nt) SAC_ICM_UNDEF ()
#define SAC_ND_A_DESC__SCL_HID(nt)                                                       \
    CAT4 (SAC_ND_A_DESC__SCL_HID_, CAT4 (NT_UNQ (nt), (nt)))
#define SAC_ND_A_DESC__SCL_HID_NUQ(nt) SAC_ND_A_DESC__AKS (nt)
#define SAC_ND_A_DESC__SCL_HID_UNQ(nt) SAC_ICM_UNDEF ()

#define SAC_ND_A_MIRROR_DIM__SCL(nt) 0

#define SAC_ND_A_MIRROR_SIZE__SCL(nt) 1

#define SAC_ND_A_MIRROR_SHAPE__SCL(nt, dim) SAC_ICM_UNDEF ()

#define SAC_ND_A_FIELD__SCL(nt) NT_NAME (nt)

#define SAC_ND_A_RC__SCL_NUQ(nt) SAC_ND_A_RC__AKS_NUQ (nt)

#define SAC_ND_A_DIM__SCL(nt) SAC_ND_A_MIRROR_DIM (nt)

#define SAC_ND_A_SIZE__SCL(nt) SAC_ND_A_MIRROR_SIZE (nt)

#define SAC_ND_A_SHAPE__SCL(nt, dim) (-1) /* SAC_ICM_UNDEF() */
                                          /*
                                           * For convenience reasons, SAC_ND_A_SHAPE__SCL is *not* undefined but
                                           * a dummy value. Now, the following code fragment works even for scalars:
                                           *   for (i = 0; i < SAC_ND_A_DIM( nt); i++) {
                                           *     ... SAC_ND_A_SHAPE( nt, i) ...
                                           *   }
                                           * Unfortunately, however, things will go wrong if SAC_ND_A_SHAPE is used
                                           * on scalars in non-dead code...
                                           */

/*
 * AKS
 */

#define SAC_ND_A_DESC__AKS(nt) CAT5 (NT_NAME (nt), __desc)

#define SAC_ND_A_MIRROR_DIM__AKS(nt) CAT8 (NT_NAME (nt), __dim)

#define SAC_ND_A_MIRROR_SIZE__AKS(nt) CAT8 (NT_NAME (nt), __sz)

#define SAC_ND_A_MIRROR_SHAPE__AKS(nt, dim) CAT8 (NT_NAME (nt), CAT8 (__shp, dim))

#define SAC_ND_A_FIELD__AKS(nt) NT_NAME (nt)

#define SAC_ND_A_RC__AKS_NUQ(nt) DESC_RC (SAC_ND_A_DESC (nt))

#define SAC_ND_A_DIM__AKS(nt) SAC_ND_A_MIRROR_DIM (nt)

#define SAC_ND_A_SIZE__AKS(nt) SAC_ND_A_MIRROR_SIZE (nt)

#define SAC_ND_A_SHAPE__AKS(nt, dim) SAC_ND_A_MIRROR_SHAPE (nt, dim)

/*
 * AKD
 */

#define SAC_ND_A_DESC__AKD(nt) SAC_ND_A_DESC__AKS (nt)

#define SAC_ND_A_MIRROR_DIM__AKD(nt) SAC_ND_A_MIRROR_DIM__AKS (nt)

#define SAC_ND_A_MIRROR_SIZE__AKD(nt) SAC_ND_A_MIRROR_SIZE__AKS (nt)

#define SAC_ND_A_MIRROR_SHAPE__AKD(nt, dim) SAC_ND_A_MIRROR_SHAPE__AKS (nt, dim)

#define SAC_ND_A_FIELD__AKD(nt) SAC_ND_A_FIELD__AKS (nt)

#define SAC_ND_A_RC__AKD_NUQ(nt) SAC_ND_A_RC__AKS_NUQ (nt)

#define SAC_ND_A_DIM__AKD(nt) SAC_ND_A_MIRROR_DIM (nt)

#define SAC_ND_A_SIZE__AKD(nt) SAC_ND_A_MIRROR_SIZE (nt)

#define SAC_ND_A_SHAPE__AKD(nt, dim) SAC_ND_A_MIRROR_SHAPE (nt, dim)

/*
 * AUD
 */

#define SAC_ND_A_DESC__AUD(nt) SAC_ND_A_DESC__AKS (nt)

#define SAC_ND_A_MIRROR_DIM__AUD(nt) SAC_ND_A_MIRROR_DIM__AKS (nt)

#define SAC_ND_A_MIRROR_SIZE__AUD(nt) SAC_ND_A_MIRROR_SIZE__AKS (nt)

#define SAC_ND_A_MIRROR_SHAPE__AUD(nt, dim) SAC_ICM_UNDEF ()

#define SAC_ND_A_FIELD__AUD(nt) SAC_ND_A_FIELD__AKS (nt)

#define SAC_ND_A_RC__AUD_NUQ(nt) SAC_ND_A_RC__AKS_NUQ (nt)

#define SAC_ND_A_DIM__AUD(nt) SAC_ND_A_MIRROR_DIM (nt)

#define SAC_ND_A_SIZE__AUD(nt) SAC_ND_A_MIRROR_SIZE (nt)

#define SAC_ND_A_SHAPE__AUD(nt, dim) SAC_ND_A_DESC_SHAPE (nt, dim)

/****************
 ****************
 ***
 *** CAT9, CAT10
 ***
 ***/

/******************************************************************************
 *
 * ICMs for read/write access
 * ==========================
 *
 * Only these two ICMs should be used to access the elements of an array as
 * they selectively enable boundary checking and cache simulation!
 *
 * ND_READ( from_nt, from_pos) :
 *   read access at specified index position
 * ND_WRITE( to_nt, to_pos) :
 *   write access at specified index position
 *
 * ND_WRITE_COPY( to_nt, to_pos, expr, copyfun) :
 *   write/copy access at specified index position
 * ND_WRITE_READ_COPY( to_nt, to_pos, from_nt, from_pos, copyfun) :
 *   write/read/copy access at specified index positions
 *
 ******************************************************************************/

#define SAC_ND_READ(from_nt, from_pos)                                                   \
    CAT9 (SAC_ND_READ__, CAT9 (NT_SHP (from_nt), BuildArgs2 (from_nt, from_pos)))

#define SAC_ND_WRITE(to_nt, to_pos)                                                      \
    CAT9 (SAC_ND_WRITE__, CAT9 (NT_SHP (to_nt), BuildArgs2 (to_nt, to_pos)))

#define SAC_ND_WRITE_COPY(to_nt, to_pos, expr, copyfun)                                  \
    CAT10 (SAC_ND_WRITE_COPY__,                                                          \
           CAT10 (NT_HID (to_nt), BuildArgs4 (to_nt, to_pos, expr, copyfun)))

#define SAC_ND_WRITE_READ_COPY(to_nt, to_pos, from_nt, from_pos, copyfun)                \
    SAC_ND_WRITE_COPY (to_nt, to_pos, SAC_ND_READ (from_nt, from_pos), copyfun)

/*
 * NHD
 */

#define SAC_ND_WRITE_COPY__NHD(to_nt, to_pos, expr, copyfun)                             \
    SAC_ND_WRITE (to_nt, to_pos) = expr;

/*
 * HID
 */

#define SAC_ND_WRITE_COPY__HID(to_nt, to_pos, expr, copyfun)                             \
    {                                                                                    \
        SAC_ND_WRITE (to_nt, to_pos) = copyfun (expr);                                   \
        SAC_TR_MEM_PRINT (                                                               \
          ("new hidden object at addr: %p", SAC_ND_READ (to_nt, to_pos)))                \
        SAC_TR_INC_HIDDEN_MEMCNT (1)                                                     \
    }

/*
 * SCL
 */

#define SAC_ND_WRITE__SCL(to_nt, to_pos) SAC_ND_A_FIELD (to_nt)

#define SAC_ND_READ__SCL(from_nt, from_pos) SAC_ND_A_FIELD (from_nt)

/*
 * AKS
 */

#define SAC_ND_WRITE__AKS(to_nt, to_pos)                                                 \
    SAC_BC_WRITE (to_nt, to_pos)                                                         \
    SAC_CS_WRITE_ARRAY (to_nt, to_pos)                                                   \
    SAC_ND_A_FIELD (to_nt)[to_pos]

#define SAC_ND_READ__AKS(from_nt, from_pos)                                              \
    (SAC_BC_READ (from_nt, from_pos) SAC_CS_READ_ARRAY (from_nt, from_pos)               \
       SAC_ND_A_FIELD (from_nt)[from_pos])

/*
 * AKD
 */

#define SAC_ND_WRITE__AKD(to_nt, to_pos) SAC_ND_WRITE__AKS (to_nt, to_pos)

#define SAC_ND_READ__AKD(from_nt, from_pos) SAC_ND_READ__AKS (from_nt, from_pos)

/*
 * AUD
 */

#define SAC_ND_WRITE__AUD(to_nt, to_pos) SAC_ND_WRITE__AKS (to_nt, to_pos)

#define SAC_ND_READ__AUD(from_nt, from_pos) SAC_ND_READ__AKS (from_nt, from_pos)

/******************************************************************************
 *
 * ICMs for types
 * =================
 *
 * ND_DESC_TYPE( nt)           : type of descriptor
 *
 * ND_TYPE_NT( basetype_nt)    : type implementation
 *     (basetype must be tagged)
 * ND_TYPE( nt, basetype)      : type implementation
 *     (basetype not tagged, separate tagged identifier at hand instead)
 * ND_TYPE__<CLASS>( basetype) : type implementation
 *
 ******************************************************************************/

#define SAC_ND_DESC_TYPE(nt) CAT9 (SAC_ND_DESC_TYPE__, CAT9 (NT_SHP (nt), (nt)))

#define SAC_ND_TYPE_NT(basetype_nt)                                                      \
    CAT9 (SAC_ND_TYPE__, CAT9 (NT_SHP (basetype_nt), (NT_NAME (basetype_nt))))

#define SAC_ND_TYPE(nt, basetype) CAT9 (SAC_ND_TYPE__, CAT9 (NT_SHP (nt), (basetype)))

/*
 * SCL
 */

#define SAC_ND_DESC_TYPE__SCL(nt) SAC_ND_DESC_TYPE__AKS (nt)

#define SAC_ND_TYPE__SCL(basetype) basetype

/*
 * AKS
 */

#define SAC_ND_DESC_TYPE__AKS(nt) SAC_array_descriptor_t

#define SAC_ND_TYPE__AKS(basetype) basetype *

/*
 * AKD
 */

#define SAC_ND_DESC_TYPE__AKD(nt) SAC_ND_DESC_TYPE__AKS (nt)

#define SAC_ND_TYPE__AKD(basetype) SAC_ND_TYPE__AKS (basetype)

/*
 * AUD
 */

#define SAC_ND_DESC_TYPE__AUD(nt) SAC_ND_DESC_TYPE__AKS (nt)

#define SAC_ND_TYPE__AUD(basetype) SAC_ND_TYPE__AKS (basetype)

/******************************************************************************
 *
 * ICMs for typedefs
 * =================
 *
 * ND_TYPEDEF( nt, basetype) : type definition
 *
 ******************************************************************************/

#define SAC_ND_TYPEDEF(nt, basetype) typedef SAC_ND_TYPE (nt, basetype) NT_NAME (nt);

/************************
 ************************
 ***
 *** CAT11, CAT12, CAT13
 ***
 ***/

/******************************************************************************
 *
 * ICMs for objdefs
 * ================
 *
 * ND_OBJDEF( nt, basetype, sdim, ...shp...) :
 *   declaration of an internal object
 * ND_OBJDEF_EXTERN( nt, basetype, sdim) :
 *   declaration of an external object
 *
 ******************************************************************************/

/* ND_OBJDEF( ...)  is a C-ICM */

/* ND_OBJDEF_EXTERN( ...)  is a C-ICM */

/******************************************************************************
 *
 * ICMs for declaration of data objects
 * ====================================
 *
 * ND_DECL( nt, basetype, sdim, ...shp...) :
 *   declares a data object
 * ND_DECL_EXTERN( nt, basetype, sdim) :
 *   declares an external data object
 *
 * ND_DECL__DATA( nt, basetype, decoration) :
 *   declares a data object (without mirror and descriptor)
 * ND_DECL__DESC( nt, decoration) :
 *   declares descriptor of a data object
 *
 * ND_DECL__MIRROR( nt, sdim, ...shp...) :
 *   declares mirror of a data object
 * ND_DECL__MIRROR_PARAM( nt, sdim, ...shp...) :
 *   declares mirror of a function parameter
 * ND_DECL__MIRROR_EXTERN( nt, sdim) :
 *   declares mirror of an external data object
 *
 ******************************************************************************/

/* ND_DECL( ...)  is a C-ICM */

/* ND_DECL_EXTERN( ...)  is a C-ICM */

#define SAC_ND_DECL__DATA(nt, basetype, decoration)                                      \
    decoration SAC_ND_TYPE (nt, basetype) SAC_ND_A_FIELD (nt);

#define SAC_ND_DECL__DESC(nt, decoration)                                                \
    CAT11 (SAC_ND_DECL__DESC__, CAT11 (NT_SHP (nt), BuildArgs2 (nt, decoration)))

/* ND_DECL__MIRROR( ...)  is a C-ICM */

/* ND_DECL__MIRROR_PARAM( ...)  is a C-ICM */

/* ND_DECL__MIRROR_EXTERN( ...)  is a C-ICM */

/*
 * SCL
 */

#define SAC_ND_DECL__DESC__SCL(nt, decoration)                                           \
    CAT12 (SAC_ND_DECL__DESC__SCL_, CAT12 (NT_HID (nt), BuildArgs2 (nt, decoration)))
#define SAC_ND_DECL__DESC__SCL_NHD(nt, decoration) SAC_NOTHING ()
#define SAC_ND_DECL__DESC__SCL_HID(nt, decoration)                                       \
    CAT13 (SAC_ND_DECL__DESC__SCL_HID_, CAT13 (NT_UNQ (nt), BuildArgs2 (nt, decoration)))
#define SAC_ND_DECL__DESC__SCL_HID_NUQ(nt, decoration)                                   \
    SAC_ND_DECL__DESC__AKS (nt, decoration)
#define SAC_ND_DECL__DESC__SCL_HID_UNQ(nt, decoration) SAC_NOTHING ()

/*
 * AKS
 */

#define SAC_ND_DECL__DESC__AKS(nt, decoration)                                           \
    decoration SAC_ND_DESC_TYPE (nt) SAC_ND_A_DESC (nt);

/*
 * AKD
 */

#define SAC_ND_DECL__DESC__AKD(nt, decoration) SAC_ND_DECL__DESC__AKS (nt, decoration)

/*
 * AUD
 */

#define SAC_ND_DECL__DESC__AUD(nt, decoration) SAC_ND_DECL__DESC__AKS (nt, decoration)

/******************************************************************************
 *
 * ICMs for descriptor initializing
 * ================================
 *
 * ND_SET__RC( nt, rc) :
 *   sets the refcount (in the descriptor) of a data object
 * ND_SET__SHAPE( to_nt, to_sdim, dim, ...shp...) :
 *   sets the shape information (descriptor *and* mirror) of a data object
 *
 ******************************************************************************/

/* ND_SET__RC( ...) is defined in one of the other sections below */

/* ND_SET__SHAPE( ...) is a C-ICM */

/******************************************************************************
 *
 * ICMs for passing data objects to functions
 * ==========================================
 *
 * ND_PARAM_( dummy, basetype) :
 *   macro for ... parameters
 *
 * ND_PARAM_in( nt, basetype) :
 *   macro for prototyping data as "in" parameter
 *
 * ND_PARAM_in_nodesc( nt, basetype) :
 *   macro for prototyping data as "in" parameter without descriptor
 *
 * ND_PARAM_out( nt, basetype) :
 *   macro for prototyping data as "out" parameter
 *
 * ND_PARAM_out_nodesc( nt, basetype) :
 *   macro for prototyping data as "out" parameter without descriptor
 *
 * ND_PARAM_inout( nt, basetype) :
 *   macro for prototyping data as "inout" parameter
 *
 * ND_PARAM_inout_nodesc( nt, basetype) :
 *   macro for prototyping data as "inout" parameter without descriptor
 *
 * ND_PARAM_inout_nodesc_bx( nt, basetype) :
 *   macro for prototyping boxed data as "inout" parameter without descriptor
 *
 * ND_ARG_in( nt) :
 *   macro for giving data as "in" parameter
 *
 * ND_ARG_in_nodesc( nt) :
 *   macro for giving data as "in" parameter without descriptor
 *
 * ND_ARG_out( nt) :
 *   macro for getting data as "out" parameter
 *
 * ND_ARG_out_nodesc( nt) :
 *   macro for getting data as "out" parameter without descriptor
 *
 * ND_ARG_inout( nt) :
 *   macro for giving data as "inout" parameter
 *
 * ND_ARG_inout_nodesc( nt) :
 *   macro for giving data as "inout" argument without descriptor
 *
 * ND_ARG_inout_nodesc_bx( nt) :
 *   macro for giving boxed data as "inout" argument without descriptor
 *
 * ND_RET_out( ntp, nt) :
 *   macro for returning data
 *
 * ND_RET_inout( ntp, nt) :
 *   macro for returning "inout" data
 *
 * ND_DECL_PARAM_inout( nt, basetype) :
 *   macro for declaration of "inout" parameter
 *
 ******************************************************************************/

/* creates name for formal function parameter */
#define SAC_NAMEP(name) CAT0 (name, __p)

#define SAC_ND_PARAM_(dummy, basetype) basetype

#define SAC_ND_PARAM_in(nt, basetype)                                                    \
    CAT11 (SAC_ND_PARAM_in__, CAT11 (NT_SHP (nt), BuildArgs2 (nt, basetype)))

#define SAC_ND_PARAM_in_nodesc(nt, basetype)                                             \
    SAC_ND_TYPE (nt, basetype)                                                           \
    SAC_ND_A_FIELD (nt)

#define SAC_ND_PARAM_out(nt, basetype)                                                   \
    CAT11 (SAC_ND_PARAM_out__, CAT11 (NT_SHP (nt), BuildArgs2 (nt, basetype)))

#define SAC_ND_PARAM_out_nodesc(nt, basetype)                                            \
    SAC_ND_TYPE (nt, basetype)                                                           \
    *SAC_NAMEP (SAC_ND_A_FIELD (nt))

#define SAC_ND_PARAM_inout(nt, basetype) SAC_ND_PARAM_out (nt, basetype)

#define SAC_ND_PARAM_inout_nodesc(nt, basetype) SAC_ND_PARAM_out_nodesc (nt, basetype)

#define SAC_ND_PARAM_inout_nodesc_bx(nt, basetype) SAC_ND_PARAM_in_nodesc (nt, basetype)

#define SAC_ND_ARG_in(nt) CAT11 (SAC_ND_ARG_in__, CAT11 (NT_SHP (nt), (nt)))

#define SAC_ND_ARG_in_nodesc(nt) SAC_ND_A_FIELD (nt)

#define SAC_ND_ARG_out(nt) CAT11 (SAC_ND_ARG_out__, CAT11 (NT_SHP (nt), (nt)))

#define SAC_ND_ARG_out_nodesc(nt) &SAC_ND_A_FIELD (nt)

#define SAC_ND_ARG_inout(nt) SAC_ND_ARG_out (nt)

#define SAC_ND_ARG_inout_nodesc(nt) SAC_ND_ARG_out_nodesc (nt)

#define SAC_ND_ARG_inout_nodesc_bx(nt) SAC_ND_ARG_in_nodesc (nt)

#define SAC_ND_RET_out(ntp, nt)                                                          \
    CAT11 (SAC_ND_RET_out__, CAT11 (NT_SHP (nt), BuildArgs2 (ntp, nt)))

#define SAC_ND_RET_inout(ntp, nt) SAC_ND_RET_out (ntp, nt)

#define SAC_ND_DECL_PARAM_inout(nt, basetype)                                            \
    CAT11 (SAC_ND_DECL_PARAM_inout__, CAT11 (NT_SHP (nt), BuildArgs2 (nt, basetype)))

/*
 * SCL
 */

#define SAC_ND_PARAM_in__SCL(nt, basetype)                                               \
    CAT12 (SAC_ND_PARAM_in__SCL_, CAT12 (NT_HID (nt), BuildArgs2 (nt, basetype)))
#define SAC_ND_PARAM_in__SCL_NHD(nt, basetype) SAC_ND_PARAM_in_nodesc (nt, basetype)
#define SAC_ND_PARAM_in__SCL_HID(nt, basetype)                                           \
    CAT13 (SAC_ND_PARAM_in__SCL_HID_, CAT13 (NT_UNQ (nt), BuildArgs2 (nt, basetype)))
#define SAC_ND_PARAM_in__SCL_HID_NUQ(nt, basetype) SAC_ND_PARAM_in__AKS (nt, basetype)
#define SAC_ND_PARAM_in__SCL_HID_UNQ(nt, basetype) SAC_ND_PARAM_in_nodesc (nt, basetype)

#define SAC_ND_PARAM_out__SCL(nt, basetype)                                              \
    CAT12 (SAC_ND_PARAM_out__SCL_, CAT12 (NT_HID (nt), BuildArgs2 (nt, basetype)))
#define SAC_ND_PARAM_out__SCL_NHD(nt, basetype) SAC_ND_PARAM_out_nodesc (nt, basetype)
#define SAC_ND_PARAM_out__SCL_HID(nt, basetype)                                          \
    CAT13 (SAC_ND_PARAM_out__SCL_HID_, CAT13 (NT_UNQ (nt), BuildArgs2 (nt, basetype)))
#define SAC_ND_PARAM_out__SCL_HID_NUQ(nt, basetype) SAC_ND_PARAM_out__AKS (nt, basetype)
#define SAC_ND_PARAM_out__SCL_HID_UNQ(nt, basetype) SAC_ND_PARAM_out_nodesc (nt, basetype)

#define SAC_ND_ARG_in__SCL(nt) CAT12 (SAC_ND_ARG_in__SCL_, CAT12 (NT_HID (nt), (nt)))
#define SAC_ND_ARG_in__SCL_NHD(nt) SAC_ND_ARG_in_nodesc (nt)
#define SAC_ND_ARG_in__SCL_HID(nt)                                                       \
    CAT13 (SAC_ND_ARG_in__SCL_HID_, CAT13 (NT_UNQ (nt), (nt)))
#define SAC_ND_ARG_in__SCL_HID_NUQ(nt) SAC_ND_ARG_in__AKS (nt)
#define SAC_ND_ARG_in__SCL_HID_UNQ(nt) SAC_ND_ARG_in_nodesc (nt)

#define SAC_ND_ARG_out__SCL(nt) CAT12 (SAC_ND_ARG_out__SCL_, CAT12 (NT_HID (nt), (nt)))
#define SAC_ND_ARG_out__SCL_NHD(nt) SAC_ND_ARG_out_nodesc (nt)
#define SAC_ND_ARG_out__SCL_HID(nt)                                                      \
    CAT13 (SAC_ND_ARG_out__SCL_HID_, CAT13 (NT_UNQ (nt), (nt)))
#define SAC_ND_ARG_out__SCL_HID_NUQ(nt) SAC_ND_ARG_out__AKS (nt)
#define SAC_ND_ARG_out__SCL_HID_UNQ(nt) SAC_ND_ARG_out_nodesc (nt)

#define SAC_ND_RET_out__SCL(ntp, nt)                                                     \
    CAT12 (SAC_ND_RET_out__SCL_, CAT12 (NT_HID (nt), BuildArgs2 (ntp, nt)))
#define SAC_ND_RET_out__SCL_NHD(ntp, nt)                                                 \
    {                                                                                    \
        *SAC_NAMEP (SAC_ND_A_FIELD (ntp)) = SAC_ND_A_FIELD (nt);                         \
    }
#define SAC_ND_RET_out__SCL_HID(ntp, nt)                                                 \
    CAT13 (SAC_ND_RET_out__SCL_HID_, CAT13 (NT_UNQ (nt), BuildArgs2 (ntp, nt)))
#define SAC_ND_RET_out__SCL_HID_NUQ(ntp, nt) SAC_ND_RET_out__AKS (ntp, nt)
#define SAC_ND_RET_out__SCL_HID_UNQ(ntp, nt)                                             \
    {                                                                                    \
        *SAC_NAMEP (SAC_ND_A_FIELD (ntp)) = SAC_ND_A_FIELD (nt);                         \
    }

#define SAC_ND_DECL_PARAM_inout__SCL(nt, basetype)                                       \
    CAT12 (SAC_ND_DECL_PARAM_inout__SCL_, CAT12 (NT_HID (nt), BuildArgs2 (nt, basetype)))
#define SAC_ND_DECL_PARAM_inout__SCL_NHD(nt, basetype)                                   \
    SAC_ND_TYPE (nt, basetype)                                                           \
    SAC_ND_A_FIELD (nt) = *SAC_NAMEP (SAC_ND_A_FIELD (nt));
#define SAC_ND_DECL_PARAM_inout__SCL_HID(nt, basetype)                                   \
    CAT13 (SAC_ND_DECL_PARAM_inout__SCL_HID_,                                            \
           CAT13 (NT_UNQ (nt), BuildArgs2 (nt, basetype)))
#define SAC_ND_DECL_PARAM_inout__SCL_HID_NUQ(nt, basetype)                               \
    SAC_ND_DECL_PARAM_inout__AKS (nt, basetype)
#define SAC_ND_DECL_PARAM_inout__SCL_HID_UNQ(nt, basetype)                               \
    SAC_ND_TYPE (nt, basetype)                                                           \
    SAC_ND_A_FIELD (nt) = *SAC_NAMEP (SAC_ND_A_FIELD (nt));

/*
 * AKS
 */

#define SAC_ND_PARAM_in__AKS(nt, basetype)                                               \
    SAC_ND_PARAM_in_nodesc (nt, basetype), SAC_ND_DESC_TYPE (nt) SAC_ND_A_DESC (nt)

#define SAC_ND_PARAM_out__AKS(nt, basetype)                                              \
    SAC_ND_PARAM_out_nodesc (nt, basetype),                                              \
      SAC_ND_DESC_TYPE (nt) * SAC_NAMEP (SAC_ND_A_DESC (nt))

#define SAC_ND_ARG_in__AKS(nt) SAC_ND_ARG_in_nodesc (nt), SAC_ND_A_DESC (nt)

#define SAC_ND_ARG_out__AKS(nt) SAC_ND_ARG_out_nodesc (nt), &SAC_ND_A_DESC (nt)

#define SAC_ND_RET_out__AKS(ntp, nt)                                                     \
    {                                                                                    \
        *SAC_NAMEP (SAC_ND_A_FIELD (ntp)) = SAC_ND_A_FIELD (nt);                         \
        *SAC_NAMEP (SAC_ND_A_DESC (ntp)) = SAC_ND_A_DESC (nt);                           \
    }

#define SAC_ND_DECL_PARAM_inout__AKS(nt, basetype)                                       \
    SAC_ND_TYPE (nt, basetype)                                                           \
    SAC_ND_A_FIELD (nt) = *SAC_NAMEP (SAC_ND_A_FIELD (nt));                              \
    SAC_ND_DESC_TYPE (nt)                                                                \
    SAC_ND_A_DESC (nt) = *SAC_NAMEP (SAC_ND_A_DESC (nt));

/*
 * AKD
 */

#define SAC_ND_PARAM_in__AKD(nt, basetype) SAC_ND_PARAM_in__AKS (nt, basetype)

#define SAC_ND_PARAM_out__AKD(nt, basetype) SAC_ND_PARAM_out__AKS (nt, basetype)

#define SAC_ND_ARG_in__AKD(nt) SAC_ND_ARG_in__AKS (nt)

#define SAC_ND_ARG_out__AKD(nt) SAC_ND_ARG_out__AKS (nt)

#define SAC_ND_RET_out__AKD(ntp, nt) SAC_ND_RET_out__AKS (ntp, nt)

#define SAC_ND_DECL_PARAM_inout__AKD(nt, basetype)                                       \
    SAC_ND_DECL_PARAM_inout__AKS (nt, basetype)

/*
 * AUD
 */

#define SAC_ND_PARAM_in__AUD(nt, basetype) SAC_ND_PARAM_in__AKS (nt, basetype)

#define SAC_ND_PARAM_out__AUD(nt, basetype) SAC_ND_PARAM_out__AKS (nt, basetype)

#define SAC_ND_ARG_in__AUD(nt) SAC_ND_ARG_in__AKS (nt)

#define SAC_ND_ARG_out__AUD(nt) SAC_ND_ARG_out__AKS (nt)

#define SAC_ND_RET_out__AUD(ntp, nt) SAC_ND_RET_out__AKS (ntp, nt)

#define SAC_ND_DECL_PARAM_inout__AUD(nt, basetype)                                       \
    SAC_ND_DECL_PARAM_inout__AKS (nt, basetype)

/******************************************************************************
 *
 * ICMs for allocation of data objects
 * ===================================
 *
 * ND_ALLOC( nt, rc, dim, set_shape_icm) :
 *   allocates a data object (no initialization but descriptor!)
 * ND_FREE( nt, freefun) :
 *   frees a data object
 *
 * ND_ALLOC__DATA( nt) :
 *   allocates memory for a data object (without descriptor, no initialization)
 * ND_FREE__DATA( nt, freefun) :
 *   frees memory for a data object (without descriptor)
 *
 * ND_ALLOC__DESC( nt, dim) :
 *   allocates memory for descriptor (no initialization but dimension!)
 * ND_FREE__DESC( nt) :
 *   frees memory for descriptor
 *
 * ND_CHECK_REUSE( to_nt, to_sdim, from_nt, copyfun) :
 *   tries to reuse old data object for new, copies if impossible
 *
 ******************************************************************************/

/*
 * for the time being this ICM is not used :-( because 'set_shape_icm' is
 * usually a C-ICM but this macro support H-ICMs only:
 */
#define SAC_ND_ALLOC(nt, rc, dim, set_shape_icm)                                         \
    {                                                                                    \
        SAC_ND_ALLOC__DESC (nt, dim)                                                     \
        SAC_ND_SET__RC (nt, rc)                                                          \
        set_shape_icm SAC_ND_ALLOC__DATA (nt)                                            \
    }
/*
 * these two macros are used instead:
 */
#define SAC_ND_ALLOC_BEGIN(nt, rc, dim)                                                  \
    {                                                                                    \
        SAC_ND_ALLOC__DESC (nt, dim)                                                     \
        SAC_ND_SET__RC (nt, rc)
#define SAC_ND_ALLOC_END(nt, rc, dim)                                                    \
    SAC_ND_ALLOC__DATA (nt)                                                              \
    }

#define SAC_ND_FREE(nt, freefun)                                                         \
    {                                                                                    \
        SAC_ND_FREE__DATA (nt, freefun)                                                  \
        SAC_ND_FREE__DESC (nt)                                                           \
    }

#define SAC_ND_ALLOC__DATA(nt) CAT11 (SAC_ND_ALLOC__DATA__, CAT11 (NT_SHP (nt), (nt)))

#define SAC_ND_FREE__DATA(nt, freefun)                                                   \
    CAT11 (SAC_ND_FREE__DATA__,                                                          \
           CAT11 (NT_SHP (nt),                                                           \
                  CAT11 (_, CAT11 (NT_HID (nt), BuildArgs2 (nt, freefun)))))

#define SAC_ND_ALLOC__DESC(nt, dim)                                                      \
    CAT11 (SAC_ND_ALLOC__DESC__, CAT11 (NT_SHP (nt), BuildArgs2 (nt, dim)))

#define SAC_ND_FREE__DESC(nt) CAT11 (SAC_ND_FREE__DESC__, CAT11 (NT_SHP (nt), (nt)))

/* ND_CHECK_REUSE( ...)  is a C-ICM */

/*
 * SCL
 */

#define SAC_ND_ALLOC__DATA__SCL(nt) SAC_NOOP ()

#define SAC_ND_FREE__DATA__SCL_NHD(nt, freefun) SAC_NOOP ()
#define SAC_ND_FREE__DATA__SCL_HID(nt, freefun)                                          \
    {                                                                                    \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_FREE__DATA( %s, %s) at addr: %p", #nt, #freefun, SAC_ND_A_FIELD (nt)))    \
        freefun (SAC_ND_A_FIELD (nt));                                                   \
        SAC_TR_DEC_HIDDEN_MEMCNT (1)                                                     \
    }

#define SAC_ND_ALLOC__DESC__SCL(nt, dim)                                                 \
    CAT12 (SAC_ND_ALLOC__DESC__SCL_, CAT12 (NT_HID (nt), BuildArgs2 (nt, dim)))
#define SAC_ND_ALLOC__DESC__SCL_NHD(nt, dim) SAC_NOOP ()
#define SAC_ND_ALLOC__DESC__SCL_HID(nt, dim)                                             \
    CAT13 (SAC_ND_ALLOC__DESC__SCL_HID_, CAT13 (NT_UNQ (nt), BuildArgs2 (nt, dim)))
#define SAC_ND_ALLOC__DESC__SCL_HID_NUQ(nt, dim) SAC_ND_ALLOC__DESC__AKS (nt, dim)
#define SAC_ND_ALLOC__DESC__SCL_HID_UNQ(nt, dim) SAC_NOOP ()

#define SAC_ND_FREE__DESC__SCL(nt)                                                       \
    CAT12 (SAC_ND_FREE__DESC__SCL_, CAT12 (NT_HID (nt), (nt)))
#define SAC_ND_FREE__DESC__SCL_NHD(nt) SAC_NOOP ()
#define SAC_ND_FREE__DESC__SCL_HID(nt)                                                   \
    CAT13 (SAC_ND_FREE__DESC__SCL_HID_, CAT13 (NT_UNQ (nt), (nt)))
#define SAC_ND_FREE__DESC__SCL_HID_NUQ(nt) SAC_ND_FREE__DESC__AKS (nt)
#define SAC_ND_FREE__DESC__SCL_HID_UNQ(nt) SAC_NOOP ()

/*
 * AKS
 */

/* the test (SAC_ND_A_SIZE( nt) != 0) is needed to prevent a malloc(0) */
#define SAC_ND_ALLOC__DATA__AKS(nt)                                                      \
    {                                                                                    \
        if (SAC_ND_A_SIZE (nt) != 0) {                                                   \
            SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_FIELD (nt),                               \
                                      SAC_ND_A_SIZE (nt)                                 \
                                        * sizeof (*SAC_ND_A_FIELD (nt)))                 \
        } else {                                                                         \
            SAC_ND_A_FIELD (nt) = NULL;                                                  \
        }                                                                                \
        SAC_TR_MEM_PRINT (("ND_ALLOC__DATA( %s) at addr: %p", #nt, SAC_ND_A_FIELD (nt))) \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (nt))                                     \
        SAC_TR_REF_PRINT_RC (nt)                                                         \
        SAC_CS_REGISTER_ARRAY (nt)                                                       \
    }

/* the test (SAC_ND_A_SIZE( nt) != 0) is needed to prevent a malloc(0) */
#define SAC_ND_FREE__DATA__AKS_NHD(nt, freefun)                                          \
    {                                                                                    \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_FREE__DATA( %s, %s) at addr: %p", #nt, #freefun, SAC_ND_A_FIELD (nt)))    \
        if (SAC_ND_A_SIZE (nt) != 0) {                                                   \
            SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_FIELD (nt),                                 \
                                    SAC_ND_A_SIZE (nt) * sizeof (*SAC_ND_A_FIELD (nt)))  \
        }                                                                                \
        SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (nt))                                     \
        SAC_CS_UNREGISTER_ARRAY (nt)                                                     \
    }
#define SAC_ND_FREE__DATA__AKS_HID(nt, freefun)                                          \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (nt); SAC_i++) {                           \
            freefun (SAC_ND_READ (nt, SAC_i));                                           \
        }                                                                                \
        SAC_ND_FREE__DATA__AKS_NHD (nt, freefun)                                         \
    }

#define SAC_ND_ALLOC__DESC__AKS(nt, dim)                                                 \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_DESC (nt),                                    \
                                  SIZE_OF_DESC (SAC_ND_A_MIRROR_DIM (nt))                \
                                    * sizeof (*SAC_ND_A_DESC (nt)))                      \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_ALLOC__DESC( %s, %d) at addr: %p", #nt, #dim, SAC_ND_A_DESC (nt)))        \
    }

#define SAC_ND_FREE__DESC__AKS(nt)                                                       \
    {                                                                                    \
        SAC_TR_MEM_PRINT (("ND_FREE__DESC( %s) at addr: %p", #nt, SAC_ND_A_DESC (nt)))   \
        SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_DESC (nt),                                      \
                                SIZE_OF_DESC (SAC_ND_A_MIRROR_DIM (nt))                  \
                                  * sizeof (*SAC_ND_A_DESC (nt)))                        \
    }

/*
 * AKD
 */

#define SAC_ND_ALLOC__DATA__AKD(nt) SAC_ND_ALLOC__DATA__AKS (nt)

#define SAC_ND_FREE__DATA__AKD_NHD(nt, freefun) SAC_ND_FREE__DATA__AKS_NHD (nt, freefun)
#define SAC_ND_FREE__DATA__AKD_HID(nt, freefun) SAC_ND_FREE__DATA__AKS_HID (nt, freefun)

#define SAC_ND_ALLOC__DESC__AKD(nt, dim) SAC_ND_ALLOC__DESC__AKS (nt, dim)

#define SAC_ND_FREE__DESC__AKD(nt) SAC_ND_FREE__DESC__AKS (nt)

/*
 * AUD
 */

#define SAC_ND_ALLOC__DATA__AUD(nt) SAC_ND_ALLOC__DATA__AKS (nt)

#define SAC_ND_FREE__DATA__AUD_NHD(nt, freefun) SAC_ND_FREE__DATA__AKS_NHD (nt, freefun)
#define SAC_ND_FREE__DATA__AUD_HID(nt, freefun) SAC_ND_FREE__DATA__AKS_HID (nt, freefun)

/* CAUTION: for the time being the compiler tolerates (dim < 0)!! */
#if 0
#define SAC_ND_ALLOC__DESC__AUD(nt, dim)                                                 \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim >= 0),                                                     \
                         ("Illegal dimension for array %s found!", NT_NAME (nt)));       \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_DESC (nt),                                    \
                                  SIZE_OF_DESC (dim) * sizeof (*SAC_ND_A_DESC (nt)))     \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_ALLOC__DESC( %s, %d) at addr: %p", #nt, #dim, SAC_ND_A_DESC (nt)))        \
        SAC_ND_A_DESC_DIM (nt) = SAC_ND_A_MIRROR_DIM (nt) = dim;                         \
    }
#else
#define SAC_ND_ALLOC__DESC__AUD(nt, dim)                                                 \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_DESC (nt), SIZE_OF_DESC (MAX_DIM_OF_DESC)     \
                                                        * sizeof (*SAC_ND_A_DESC (nt)))  \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_ALLOC__DESC( %s, %d) at addr: %p", #nt, #dim, SAC_ND_A_DESC (nt)))        \
        SAC_ND_A_DESC_DIM (nt) = SAC_ND_A_MIRROR_DIM (nt) = dim;                         \
    }
#endif

#if 0
#define SAC_ND_FREE__DESC__AUD(nt) SAC_ND_FREE__DESC__AKS (nt)
#else
#define SAC_ND_FREE__DESC__AUD(nt)                                                       \
    {                                                                                    \
        SAC_TR_MEM_PRINT (("ND_FREE__DESC( %s) at addr: %p", #nt, SAC_ND_A_DESC (nt)))   \
        SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_DESC (nt), SIZE_OF_DESC (MAX_DIM_OF_DESC)       \
                                                      * sizeof (*SAC_ND_A_DESC (nt)))    \
    }
#endif

/************************
 ************************
 ***
 *** CAT14, CAT15, CAT16
 ***
 ***/

/******************************************************************************
 *
 * ICMs for assigning data objects
 * ===============================
 *
 * ND_ASSIGN( to_nt, to_sdim, from_nt, copyfun) :
 *   assigns a data object to another one
 * ND_ASSIGN__DATA( to_nt, from_nt, copyfun) :
 *   assigns a data object to another one (without mirror)
 * ND_ASSIGN__DESC( to_nt, from_nt) :
 *   assigns a descriptor to another one
 * ND_ASSIGN__SHPDIM( to_nt, to_sdim, from_nt, from_sdim) :
 *   assigns a shape information (descriptor *and* mirror) to another one
 *
 * ND_COPY( to_nt, to_sdim, from_nt, from_sdim, copyfun) :
 *   copies a data object to another one
 * ND_COPY__DATA( to_nt, from_nt, copyfun) :
 *   copies a data object to another one (without mirror)
 * ND_COPY__SHPDIM( to_nt, to_sdim, from_nt, from_sdim) :
 *   copies a shape information (descriptor *and* mirror) to another one
 *
 * ND_MAKE_UNIQUE( to_nt, to_sdim, from_nt, copyfun) :
 *   assigns a data object to another one iff RC is zero, copies it otherwise.
 *
 ******************************************************************************/

/* ND_ASSIGN( ...)  is a C-ICM */

#define SAC_ND_ASSIGN__DATA(to_nt, from_nt, copyfun)                                     \
    CAT14 (SAC_ND_ASSIGN__DATA__,                                                        \
           CAT14 (NT_SHP (to_nt),                                                        \
                  CAT14 (__, CAT14 (NT_SHP (from_nt),                                    \
                                    BuildArgs3 (to_nt, from_nt, copyfun)))))

/* ND_ASSIGN__DESC( ...)  is a C-ICM */

/* ND_ASSIGN__SHPDIM( ...)  is a C-ICM */

/* ND_COPY( ...)  is a C-ICM */

#define SAC_ND_COPY__DATA(to_nt, from_nt, copyfun)                                       \
    CAT14 (SAC_ND_COPY__DATA__,                                                          \
           CAT14 (NT_SHP (to_nt),                                                        \
                  CAT14 (__, CAT14 (NT_SHP (from_nt),                                    \
                                    BuildArgs3 (to_nt, from_nt, copyfun)))))

/* ND_COPY__SHPDIM( ...)  is a C-ICM */

/* ND_MAKE_UNIQUE( ...)  is a C-ICM */

/*
 * SCL
 */

#define SAC_ND_ASSIGN__DATA__SCL__SCL(to_nt, from_nt, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_nt, from_nt, copyfun)
#define SAC_ND_ASSIGN__DATA__SCL__AKS(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__SCL__AKD(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__SCL__AUD(to_nt, from_nt, copyfun)                           \
    CAT15 (SAC_ND_ASSIGN__DATA__SCL_,                                                    \
           CAT15 (NT_HID (to_nt),                                                        \
                  CAT15 (__AUD_, CAT15 (NT_HID (from_nt),                                \
                                        BuildArgs3 (to_nt, from_nt, copyfun)))))
#define SAC_ND_ASSIGN__DATA__SCL_NHD__AUD_NHD(to_nt, from_nt, copyfun)                   \
    {                                                                                    \
        SAC_ND_WRITE_READ_COPY (to_nt, 0, from_nt, 0, copyfun)                           \
        SAC_ND_DEC_RC_FREE (from_nt, 1, )                                                \
    }
#define SAC_ND_ASSIGN__DATA__SCL_HID__AUD_HID(to_nt, from_nt, copyfun)                   \
    CAT16 (SAC_ND_ASSIGN__DATA__SCL_HID_,                                                \
           CAT16 (NT_UNQ (to_nt),                                                        \
                  CAT16 (__AUD_HID_, CAT16 (NT_UNQ (from_nt),                            \
                                            BuildArgs3 (to_nt, from_nt, copyfun)))))
#define SAC_ND_ASSIGN__DATA__SCL_HID_NUQ__AUD_HID_NUQ(to_nt, from_nt, copyfun)           \
    SAC_ND_ASSIGN__DATA__SCL_NHD__AUD_NHD (to_nt, from_nt, copyfun)
#define SAC_ND_ASSIGN__DATA__SCL_HID_NUQ__AUD_HID_UNQ(to_nt, from_nt, copyfun)           \
    {                                                                                    \
        SAC_ND_WRITE (to_nt, 0) = SAC_ND_READ (from_nt, 0);                              \
        /* free data vector but keep its content (i.e. the hidden object itself)! */     \
        SAC_ND_FREE__DATA__AUD_NHD (from_nt, freefun)                                    \
    }
#define SAC_ND_ASSIGN__DATA__SCL_HID_UNQ__AUD_HID_NUQ(to_nt, from_nt, copyfun)           \
    SAC_ND_ASSIGN__DATA__SCL_HID_NUQ__AUD_HID_UNQ (to_nt, from_nt, copyfun)
#define SAC_ND_ASSIGN__DATA__SCL_HID_UNQ__AUD_HID_UNQ(to_nt, from_nt, copyfun)           \
    SAC_ND_ASSIGN__DATA__SCL_HID_NUQ__AUD_HID_UNQ (to_nt, from_nt, copyfun)

#define SAC_ND_COPY__DATA__SCL__SCL(to_nt, from_nt, copyfun)                             \
    {                                                                                    \
        SAC_TR_MEM_PRINT (("ND_COPY__DATA( %s, %s, %s)" #to_nt, #from_nt, #copyfun))     \
        SAC_ND_WRITE_READ_COPY (to_nt, 0, from_nt, 0, copyfun)                           \
    }
#define SAC_ND_COPY__DATA__SCL__AKS(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__SCL__AKD(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__SCL__AUD(to_nt, from_nt, copyfun)                             \
    SAC_ND_COPY__DATA__SCL__SCL (to_nt, from_nt, copyfun)

/*
 * AKS
 */

#define SAC_ND_ASSIGN__DATA__AKS__SCL(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__AKS__AKS(to_nt, from_nt, copyfun)                           \
    {                                                                                    \
        SAC_ND_A_FIELD (to_nt) = SAC_ND_A_FIELD (from_nt);                               \
    }
#define SAC_ND_ASSIGN__DATA__AKS__AKD(to_nt, from_nt, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_nt, from_nt, copyfun)
#define SAC_ND_ASSIGN__DATA__AKS__AUD(to_nt, from_nt, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_nt, from_nt, copyfun)

#define SAC_ND_COPY__DATA__AKS__SCL(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__AKS__AKS(to_nt, from_nt, copyfun)                             \
    {                                                                                    \
        int SAC_i;                                                                       \
        SAC_TR_MEM_PRINT (("ND_COPY__DATA( %s, %s, %s) at addr: %p", #from_nt, #to_nt,   \
                           #copyfun, SAC_ND_A_FIELD (to_nt)))                            \
        SAC_ASSURE_TYPE ((SAC_ND_A_SIZE (to_nt) == SAC_ND_A_SIZE (from_nt)),             \
                         ("Assignment with incompatible types found!"));                 \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (from_nt); SAC_i++) {                      \
            SAC_ND_WRITE_READ_COPY (to_nt, SAC_i, from_nt, SAC_i, copyfun)               \
        }                                                                                \
    }
#define SAC_ND_COPY__DATA__AKS__AKD(to_nt, from_nt, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_nt, from_nt, copyfun)
#define SAC_ND_COPY__DATA__AKS__AUD(to_nt, from_nt, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_nt, from_nt, copyfun)

/*
 * AKD
 */

#define SAC_ND_ASSIGN__DATA__AKD__SCL(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__AKD__AKS(to_nt, from_nt, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_nt, from_nt, copyfun)
#define SAC_ND_ASSIGN__DATA__AKD__AKD(to_nt, from_nt, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_nt, from_nt, copyfun)
#define SAC_ND_ASSIGN__DATA__AKD__AUD(to_nt, from_nt, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_nt, from_nt, copyfun)

#define SAC_ND_COPY__DATA__AKD__SCL(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__AKD__AKS(to_nt, from_nt, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_nt, from_nt, copyfun)
#define SAC_ND_COPY__DATA__AKD__AKD(to_nt, from_nt, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_nt, from_nt, copyfun)
#define SAC_ND_COPY__DATA__AKD__AUD(to_nt, from_nt, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_nt, from_nt, copyfun)

/*
 * AUD
 */

#define SAC_ND_ASSIGN__DATA__AUD__SCL(to_nt, from_nt, copyfun)                           \
  CAT15( SAC_ND_ASSIGN__DATA__AUD_, CAT15( NT_HID( to_nt), CAT15( __SCL_, \
                                    CAT15( NT_HID( from_nt), CAT15( _,    \
                                    CAT15( NT_UNQ( from_nt),              \
                                    BuildArgs3( to_nt, from_nt, copyfun)))))
#define SAC_ND_ASSIGN__DATA__AUD_NHD__SCL_NHD_NUQ(to_nt, from_nt, copyfun)               \
    SAC_ND_WRITE_READ_COPY (to_nt, 0, from_nt, 0, copyfun)
#define SAC_ND_ASSIGN__DATA__AUD_NHD__SCL_NHD_UNQ(to_nt, from_nt, copyfun)               \
    SAC_ND_ASSIGN__DATA__AUD_NHD__SCL_NHD_NUQ (to_nt, from_nt, copyfun)
#define SAC_ND_ASSIGN__DATA__AUD_HID__SCL_HID_NUQ(to_nt, from_nt, copyfun)               \
    {                                                                                    \
        SAC_ND_WRITE_READ_COPY (to_nt, 0 from_nt, 0, copyfun)                            \
        SAC_ND_DEC_RC_FREE (from_nt, 1, )                                                \
    }
#define SAC_ND_ASSIGN__DATA__AUD_HID__SCL_HID_UNQ(to_nt, from_nt, copyfun)               \
    SAC_ND_ASSIGN__DATA__AUD_NHD__SCL_NHD_NUQ (to_nt, from_nt, copyfun)

#define SAC_ND_ASSIGN__DATA__AUD__AKS(to_nt, from_nt, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_nt, from_nt, copyfun)
#define SAC_ND_ASSIGN__DATA__AUD__AKD(to_nt, from_nt, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_nt, from_nt, copyfun)
#define SAC_ND_ASSIGN__DATA__AUD__AUD(to_nt, from_nt, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_nt, from_nt, copyfun)

#define SAC_ND_COPY__DATA__AUD__SCL(to_nt, from_nt, copyfun)                             \
    SAC_ND_WRITE_READ_COPY (to_nt, 0, from_nt, 0, copyfun)
#define SAC_ND_COPY__DATA__AUD__AKS(to_nt, from_nt, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_nt, from_nt, copyfun)
#define SAC_ND_COPY__DATA__AUD__AKD(to_nt, from_nt, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_nt, from_nt, copyfun)
#define SAC_ND_COPY__DATA__AUD__AUD(to_nt, from_nt, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_nt, from_nt, copyfun)

/******************************************************************************
 *
 * ICMs for creating refcounted objects:
 * =====================================
 *
 * ND_CREATE__SCALAR__DATA( nt, val) :
 *   creates data of a constant scalar
 *
 * ND_CREATE__STRING__DATA( nt, str) :
 *   creates data of a constant character array (string)
 *
 * ND_CREATE__VECT__DIM( val_size, ...val...) :
 *   computes dim of a constant vector
 * ND_CREATE__VECT__SHAPE( nt, sdim, val_size, ...val...) :
 *   computes shape of a constant vector
 * ND_CREATE__VECT__DATA( nt, sdim, val_size, ...val..., copyfun) :
 *   creates data of a constant vector
 *
 ******************************************************************************/

#define SAC_ND_CREATE__SCALAR__DATA(nt, val)                                             \
    {                                                                                    \
        SAC_ND_WRITE (nt, 0) = val;                                                      \
    }

#define SAC_ND_CREATE__STRING__DATA(nt, str)                                             \
    {                                                                                    \
        SAC_String2Array (SAC_ND_A_FIELD (nt), str);                                     \
    }

/* ND_CREATE__VECT__DIM( ...) is a C-ICM */
/* ND_CREATE__VECT__SHAPE( ...) is a C-ICM */
/* ND_CREATE__VECT__DATA( ...) is a C-ICM */

/************************
 ************************
 ***
 *** CAT17, CAT18, CAT19
 ***
 ***/

/******************************************************************************
 *
 * ICMs for refcounting data objects
 * =================================
 *
 * ND_SET__RC( nt, rc) :
 *   sets the refcount (in the descriptor) of a data object
 * ND_INC_RC( nt, rc) :
 *   increments the refcount of a data object
 * ND_DEC_RC( nt, rc) :
 *   decrements the refcount of a data object
 * ND_DEC_RC_FREE( nt, rc, freefun) :
 *   decrements the refcount and frees the data object if refcount becomes 0
 *
 ******************************************************************************/

#define SAC_ND_SET__RC(nt, rc)                                                           \
    CAT17 (SAC_ND_SET__RC__, CAT17 (NT_UNQ (nt), BuildArgs2 (nt, rc)))

#define SAC_ND_INC_RC(nt, rc)                                                            \
    CAT17 (SAC_ND_INC_RC__, CAT17 (NT_UNQ (nt), BuildArgs2 (nt, rc)))

#define SAC_ND_DEC_RC(nt, rc)                                                            \
    CAT17 (SAC_ND_DEC_RC__, CAT17 (NT_UNQ (nt), BuildArgs2 (nt, rc)))

#define SAC_ND_DEC_RC_FREE(nt, rc, freefun)                                              \
    CAT17 (SAC_ND_DEC_RC_FREE__, CAT17 (NT_UNQ (nt), BuildArgs3 (nt, rc, freefun)))

/*
 * NUQ
 */

#define SAC_ND_SET__RC__NUQ(nt, rc)                                                      \
    CAT18 (SAC_ND_SET__RC__, CAT18 (NT_SHP (nt), CAT18 (_NUQ, BuildArgs2 (nt, rc))))

#define SAC_ND_INC_RC__NUQ(nt, rc)                                                       \
    CAT18 (SAC_ND_INC_RC__, CAT18 (NT_SHP (nt), CAT18 (_NUQ, BuildArgs2 (nt, rc))))

#define SAC_ND_DEC_RC__NUQ(nt, rc)                                                       \
    CAT18 (SAC_ND_DEC_RC__, CAT18 (NT_SHP (nt), CAT18 (_NUQ, BuildArgs2 (nt, rc))))

#define SAC_ND_DEC_RC_FREE__NUQ(nt, rc, freefun)                                         \
    CAT18 (SAC_ND_DEC_RC_FREE__,                                                         \
           CAT18 (NT_SHP (nt), CAT18 (_NUQ, BuildArgs3 (nt, rc, freefun))))

/*
 * UNQ
 */

#define SAC_ND_SET__RC__UNQ(nt, rc) SAC_NOOP ()

#define SAC_ND_INC_RC__UNQ(nt, rc) SAC_NOOP ()

#define SAC_ND_DEC_RC__UNQ(nt, rc) SAC_NOOP ()

/* 'nt' is unique -> 'nt' has been consumed -> free 'nt' */
#define SAC_ND_DEC_RC_FREE__UNQ(nt, rc, freefun) SAC_ND_FREE (nt, freefun)

/*
 * SCL, NUQ
 */

#define SAC_ND_SET__RC__SCL_NUQ(nt, rc)                                                  \
    CAT19 (SAC_ND_SET__RC__SCL_, CAT19 (NT_HID (nt), CAT19 (_NUQ, BuildArgs2 (nt, rc))))
#define SAC_ND_SET__RC__SCL_NHD_NUQ(nt, rc) SAC_NOOP ()
#define SAC_ND_SET__RC__SCL_HID_NUQ(nt, rc) SAC_ND_SET__RC__AKS_NUQ (nt, rc)

#define SAC_ND_INC_RC__SCL_NUQ(nt, rc)                                                   \
    CAT19 (SAC_ND_INC_RC__SCL_, CAT19 (NT_HID (nt), CAT19 (_NUQ, BuildArgs2 (nt, rc))))
#define SAC_ND_INC_RC__SCL_NHD_NUQ(nt, rc) SAC_NOOP ()
#define SAC_ND_INC_RC__SCL_HID_NUQ(nt, rc) SAC_ND_INC_RC__AKS_NUQ (nt, rc)

#define SAC_ND_DEC_RC__SCL_NUQ(nt, rc)                                                   \
    CAT19 (SAC_ND_DEC_RC__SCL_, CAT19 (NT_HID (nt), CAT19 (_NUQ, BuildArgs2 (nt, rc))))
#define SAC_ND_DEC_RC__SCL_NHD_NUQ(nt, rc) SAC_NOOP ()
#define SAC_ND_DEC_RC__SCL_HID_NUQ(nt, rc) SAC_ND_DEC_RC__AKS_NUQ (nt, rc)

#define SAC_ND_DEC_RC_FREE__SCL_NUQ(nt, rc, freefun)                                     \
    CAT19 (SAC_ND_DEC_RC_FREE__SCL_,                                                     \
           CAT19 (NT_HID (nt), CAT19 (_NUQ, BuildArgs3 (nt, rc, freefun))))
#define SAC_ND_DEC_RC_FREE__SCL_NHD_NUQ(nt, rc, freefun) SAC_NOOP ()
#define SAC_ND_DEC_RC_FREE__SCL_HID_NUQ(nt, rc, freefun)                                 \
    SAC_ND_DEC_RC_FREE__AKS_NUQ (nt, rc, freefun)

/*
 * AKS, NUQ
 */

#define SAC_ND_SET__RC__AKS_NUQ(nt, rc)                                                  \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_SET__RC( %s, %d)", #nt, rc))                              \
        SAC_ND_A_RC (nt) = rc;                                                           \
        SAC_TR_REF_PRINT_RC (nt)                                                         \
    }

#define SAC_ND_INC_RC__AKS_NUQ(nt, rc)                                                   \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_INC_RC( %s, %d)", #nt, rc))                               \
        SAC_ND_A_RC (nt) += rc;                                                          \
        SAC_TR_REF_PRINT_RC (nt)                                                         \
    }

#define SAC_ND_DEC_RC__AKS_NUQ(nt, rc)                                                   \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_DEC_RC( %s, %d)", #nt, rc))                               \
        SAC_ND_A_RC (nt) -= rc;                                                          \
        SAC_TR_REF_PRINT_RC (nt)                                                         \
    }

#define SAC_ND_DEC_RC_FREE__AKS_NUQ(nt, rc, freefun)                                     \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_DEC_RC_FREE( %s, %d, %s)", #nt, rc, #freefun))            \
        if ((SAC_ND_A_RC (nt) -= rc) == 0) {                                             \
            SAC_TR_REF_PRINT_RC (nt)                                                     \
            SAC_ND_FREE (nt, freefun)                                                    \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (nt)                                                     \
        }                                                                                \
    }

/*
 * AKD, NUQ
 */

#define SAC_ND_SET__RC__AKD_NUQ(nt, rc) SAC_ND_SET__RC__AKS_NUQ (nt, rc)

#define SAC_ND_INC_RC__AKD_NUQ(nt, rc) SAC_ND_INC_RC__AKS_NUQ (nt, rc)

#define SAC_ND_DEC_RC__AKD_NUQ(nt, rc) SAC_ND_DEC_RC__AKS_NUQ (nt, rc)

#define SAC_ND_DEC_RC_FREE__AKD_NUQ(nt, rc, freefun)                                     \
    SAC_ND_DEC_RC_FREE__AKS_NUQ (nt, rc, freefun)

/*
 * AUD, NUQ
 */

#define SAC_ND_SET__RC__AUD_NUQ(nt, rc) SAC_ND_SET__RC__AKS_NUQ (nt, rc)

#define SAC_ND_INC_RC__AUD_NUQ(nt, rc) SAC_ND_INC_RC__AKS_NUQ (nt, rc)

#define SAC_ND_DEC_RC__AUD_NUQ(nt, rc) SAC_ND_DEC_RC__AKS_NUQ (nt, rc)

#define SAC_ND_DEC_RC_FREE__AUD_NUQ(nt, rc, freefun)                                     \
    SAC_ND_DEC_RC_FREE__AKS_NUQ (nt, rc, freefun)

/******************************************************************************
 *
 * ICMs for refcount-sensitive operations:
 * =======================================
 *
 * IS_LASTREF__THEN( nt)
 * IS_LASTREF__ELSE( nt)
 *
 * IS_LASTREF__BLOCK_BEGIN( nt)
 * IS_LASTREF__BLOCK_ELSE( nt)
 * IS_LASTREF__BLOCK_END( nt)
 *
 ******************************************************************************/

#define SAC_IS_LASTREF__THEN(nt) CAT17 (SAC_IS_LASTREF__THEN__, CAT17 (NT_UNQ (nt), (nt)))
#define SAC_IS_LASTREF__ELSE(nt) else

#define SAC_IS_LASTREF__BLOCK_BEGIN(nt)                                                  \
    SAC_IS_LASTREF__THEN (nt)                                                            \
    {
#define SAC_IS_LASTREF__BLOCK_ELSE(nt)                                                   \
    }                                                                                    \
    SAC_IS_LASTREF__ELSE (nt)                                                            \
    {
#define SAC_IS_LASTREF__BLOCK_END(nt) }

/*
 * NUQ
 */

#define SAC_IS_LASTREF__THEN__NUQ(nt)                                                    \
    CAT18 (SAC_IS_LASTREF__THEN__, CAT18 (NT_SHP (nt), CAT18 (_NUQ, (nt))))

/*
 * UNQ
 */

#define SAC_IS_LASTREF__THEN__UNQ(nt) if (0)

/*
 * SCL, NUQ
 */

#define SAC_IS_LASTREF__THEN__SCL_NUQ(nt)                                                \
    CAT19 (SAC_IS_LASTREF__THEN__SCL_, CAT19 (NT_HID (nt), CAT19 (_NUQ, (nt))))
#define SAC_IS_LASTREF__THEN__SCL_NHD_NUQ(nt) if (0)
#define SAC_IS_LASTREF__THEN__SCL_HID_NUQ(nt) SAC_IS_LASTREF__THEN__AKS_NUQ (nt)

/*
 * AKS, NUQ
 */

#define SAC_IS_LASTREF__THEN__AKS_NUQ(nt) if (SAC_ND_A_RC (nt) == 1)

/*
 * AKD, NUQ
 */

#define SAC_IS_LASTREF__THEN__AKD_NUQ(nt) SAC_IS_LASTREF__THEN__AKS_NUQ (nt)

/*
 * AUD, NUQ
 */

#define SAC_IS_LASTREF__THEN__AUD_NUQ(nt) SAC_IS_LASTREF__THEN__AKS_NUQ (nt)

/******************************************************************************
 *
 * ICMs for initializing global objects
 * ====================================
 *
 * INITGLOBALOBJECT_BEGIN( varname)
 * INITGLOBALOBJECT_END()
 *
 ******************************************************************************/

#ifdef SAC_GENERATE_CLIBRARY
/*
 * If used in a c library it has to check if the global object has been
 * initialized by another module (uses varname as flag).
 */
#define SAC_INITGLOBALOBJECT_BEGIN(varname)                                              \
    if (!CAT1 (SAC_INIT_FLAG_, varname)) {                                               \
        CAT1 (SAC_INIT_FLAG_, varname) = true;
#define SAC_INITGLOBALOBJECT_END() }
#else
/*
 * without check -> nothing
 */
#define SAC_INITGLOBALOBJECT_BEGIN(varname) SAC_NOTHING ()
#define SAC_INITGLOBALOBJECT_END() SAC_NOTHING ()
#endif

#endif /* _SAC_STD_H */
