/*
 *
 * $Log$
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
    int rc;          /* reference count */
    int dim;         /* # of dimensions */
    int sz;          /* # of elements   */
    int shp[MAXDIM]; /* shape vector    */
} SAC_array_descriptor;

typedef int SAC_hidden_descriptor; /* reference count */

/*********************
 *********************
 ***
 *** CAT1, CAT2, CAT3
 ***
 ***/

/******************************************************************************
 *
 * ICMs for descriptor access
 * ==========================
 *
 * ND_A_DESC( nt) :
 *   accesses the descriptor of the data object
 * ND_A_DESC_DIM( nt) :
 *   accesses the dimension of the data object via descriptor(!)
 * ND_A_DESC_SIZE( nt) :
 *   accesses the size of the data object via descriptor(!)
 * ND_A_DESC_SHAPE( nt, dim) :
 *   accesses a shape component of the data object via descriptor(!)
 *
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
 *   accesses a shape component
 *
 ******************************************************************************/

#define SAC_ND_A_DESC(nt)                                                                \
    CAT0 (SAC_ND_A_DESC__, CAT0 (NT_DATA (nt), CAT0 (_, CAT0 (NT_UNQ (nt), (nt)))))

#define SAC_ND_A_DESC_DIM(nt) CAT2 (SAC_ND_A_DESC (nt), ->dim)

#define SAC_ND_A_DESC_SIZE(nt) CAT2 (SAC_ND_A_DESC (nt), ->sz)

#define SAC_ND_A_DESC_SHAPE(nt, dim) CAT2 (SAC_ND_A_DESC (nt), ->shp[dim])

#define SAC_ND_A_FIELD(nt) CAT1 (SAC_ND_A_FIELD__, CAT1 (NT_DATA (nt), (nt)))

#define SAC_ND_A_RC(nt) CAT1 (SAC_ND_A_RC__, CAT1 (NT_UNQ (nt), (nt)))

#define SAC_ND_A_DIM(nt) CAT1 (SAC_ND_A_DIM__, CAT1 (NT_DATA (nt), (nt)))

#define SAC_ND_A_SIZE(nt) CAT1 (SAC_ND_A_SIZE__, CAT1 (NT_DATA (nt), (nt)))

#define SAC_ND_A_SHAPE(nt, dim)                                                          \
    CAT1 (SAC_ND_A_SHAPE__, CAT1 (NT_DATA (nt), BuildArgs2 (nt, dim)))

/*
 * NUQ
 */

#define SAC_ND_A_RC__NUQ(nt)                                                             \
    CAT3 (SAC_ND_A_RC__, CAT3 (NT_DATA (nt), CAT3 (_, CAT3 (NT_UNQ (nt), (nt)))))

/*
 * UNQ
 */

#define SAC_ND_A_RC__UNQ(nt) SAC_ICM_UNDEF ()

/*
 * SCL
 */

#define SAC_ND_A_DESC__SCL_NUQ(nt) SAC_ICM_UNDEF ()
#define SAC_ND_A_DESC__SCL_UNQ(nt) SAC_ND_A_DESC__SCL_NUQ (nt)

#define SAC_ND_A_FIELD__SCL(nt) NT_NAME (nt)

#define SAC_ND_A_RC__SCL_NUQ(nt) SAC_ICM_UNDEF ()

#define SAC_ND_A_DIM__SCL(nt) 0

#define SAC_ND_A_SIZE__SCL(nt) 1

#define SAC_ND_A_SHAPE__SCL(nt, dim) SAC_ICM_UNDEF ()

/*
 * AKS
 */

#define SAC_ND_A_DESC__AKS_NUQ(nt) CAT2 (NT_NAME (nt), __desc)
#define SAC_ND_A_DESC__AKS_UNQ(nt) SAC_ND_A_DESC__AKS_NUQ (nt)

#define SAC_ND_A_FIELD__AKS(nt) NT_NAME (nt)

#define SAC_ND_A_RC__AKS_NUQ(nt) CAT2 (SAC_ND_A_DESC (nt), ->rc)

#define SAC_ND_A_DIM__AKS(nt) CAT2 (NT_NAME (nt), __dim)

#define SAC_ND_A_SIZE__AKS(nt) CAT2 (NT_NAME (nt), __sz)

#define SAC_ND_A_SHAPE__AKS(nt, dim) CAT2 (NT_NAME (nt), CAT2 (__shp, dim))

/*
 * AKD
 */

#define SAC_ND_A_DESC__AKD_NUQ(nt) SAC_ND_A_DESC__AKS_NUQ (nt)
#define SAC_ND_A_DESC__AKD_UNQ(nt) SAC_ND_A_DESC__AKD_NUQ (nt)

#define SAC_ND_A_FIELD__AKD(nt) SAC_ND_A_FIELD__AKS (nt)

#define SAC_ND_A_RC__AKD_NUQ(nt) SAC_ND_A_RC__AKS_NUQ (nt)

#define SAC_ND_A_DIM__AKD(nt) SAC_ND_A_DIM__AKS (nt)

#define SAC_ND_A_SIZE__AKD(nt) SAC_ND_A_SIZE__AKS (nt)

#define SAC_ND_A_SHAPE__AKD(nt, dim) SAC_ND_A_SHAPE__AKS (nt, dim)

/*
 * AUD
 */

#define SAC_ND_A_DESC__AUD_NUQ(nt) SAC_ND_A_DESC__AKS_NUQ (nt)
#define SAC_ND_A_DESC__AUD_UNQ(nt) SAC_ND_A_DESC__AUD_NUQ (nt)

#define SAC_ND_A_FIELD__AUD(nt) SAC_ND_A_FIELD__AKS (nt)

#define SAC_ND_A_RC__AUD_NUQ(nt) SAC_ND_A_RC__AKS_NUQ (nt)

#define SAC_ND_A_DIM__AUD(nt) SAC_ND_A_DIM__AKS (nt)

#define SAC_ND_A_SIZE__AUD(nt) SAC_ND_A_SIZE__AKS (nt)

#define SAC_ND_A_SHAPE__AUD(nt, dim) SAC_ND_A_DESC_SHAPE (nt, dim)

/*
 * HID
 */

#define SAC_ND_A_DESC__HID_NUQ(nt) CAT2 (NT_NAME (nt), __rc)
#define SAC_ND_A_DESC__HID_UNQ(nt) SAC_ICM_UNDEF ()

#define SAC_ND_A_FIELD__HID(nt) NT_NAME (nt)

#define SAC_ND_A_RC__HID_NUQ(nt) *SAC_ND_A_DESC (nt)

#define SAC_ND_A_DIM__HID(nt) SAC_ICM_UNDEF ()

#define SAC_ND_A_SIZE__HID(nt) SAC_ICM_UNDEF ()

#define SAC_ND_A_SHAPE__HID(nt, dim) SAC_ICM_UNDEF ()

/***************
 ***************
 ***
 *** CAT4
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
 * ND_READ( nt, pos) :  read access at specified index position
 * ND_WRITE( nt, pos) : write access at specified index position
 *
 ******************************************************************************/

#define SAC_ND_READ(nt, pos)                                                             \
    CAT4 (SAC_ND_READ__, CAT4 (NT_DATA (nt), BuildArgs2 (nt, pos)))

#define SAC_ND_WRITE(nt, pos)                                                            \
    CAT4 (SAC_ND_WRITE__, CAT4 (NT_DATA (nt), BuildArgs2 (nt, pos)))

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

#define SAC_ND_READ__HID(nt, pos) SAC_ICM_UNDEF ()

#define SAC_ND_WRITE__HID(nt, pos) SAC_ICM_UNDEF ()

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

#define SAC_ND_DESC_TYPE(nt) CAT4 (SAC_ND_DESC_TYPE__, CAT4 (NT_DATA (nt), (nt)))

#define SAC_ND_TYPE_NT(basetype_nt)                                                      \
    CAT4 (SAC_ND_TYPE__, CAT4 (NT_DATA (basetype_nt), (NT_NAME (basetype_nt))))

#define SAC_ND_TYPE(nt, basetype) CAT4 (SAC_ND_TYPE__, CAT4 (NT_DATA (nt), (basetype)))

/*
 * SCL
 */

#define SAC_ND_DESC_TYPE__SCL(nt) SAC_ICM_UNDEF ()

#define SAC_ND_TYPE__SCL(basetype) basetype

/*
 * AKS
 */

#define SAC_ND_DESC_TYPE__AKS(nt) SAC_array_descriptor *

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

/*
 * HID
 */

#define SAC_ND_DESC_TYPE__HID(nt) SAC_hidden_descriptor *

#define SAC_ND_TYPE__HID(basetype) void *

/******************************************************************************
 *
 * ICMs for typedefs
 * =================
 *
 * ND_TYPEDEF( nt, basetype) : type definition
 *
 ******************************************************************************/

#define SAC_ND_TYPEDEF(nt, basetype) typedef SAC_ND_TYPE (nt, basetype) NT_NAME (nt);

/***************
 ***************
 ***
 *** CAT5, CAT6
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
    CAT5 (SAC_ND_DECL__DESC__,                                                           \
          CAT5 (NT_DATA (nt),                                                            \
                CAT5 (_, CAT5 (NT_UNQ (nt), BuildArgs2 (nt, decoration)))))

/* ND_DECL__MIRROR( ...)  is a C-ICM */

/* ND_DECL__MIRROR_PARAM( ...)  is a C-ICM */

/* ND_DECL__MIRROR_EXTERN( ...)  is a C-ICM */

/*
 * SCL
 */

#define SAC_ND_DECL__DESC__SCL_NUQ(nt, decoration) SAC_NOTHING ()
#define SAC_ND_DECL__DESC__SCL_UNQ(nt, decoration) SAC_NOTHING ()

/*
 * AKS
 */

#define SAC_ND_DECL__DESC__AKS_NUQ(nt, decoration)                                       \
    decoration SAC_ND_DESC_TYPE (nt) SAC_ND_A_DESC (nt);
#define SAC_ND_DECL__DESC__AKS_UNQ(nt, decoration)                                       \
    SAC_ND_DECL__DESC__AKS_NUQ (nt, decoration)

/*
 * AKD
 */

#define SAC_ND_DECL__DESC__AKD_NUQ(nt, decoration)                                       \
    SAC_ND_DECL__DESC__AKS_NUQ (nt, decoration)
#define SAC_ND_DECL__DESC__AKD_UNQ(nt, decoration)                                       \
    SAC_ND_DECL__DESC__AKS_UNQ (nt, decoration)

/*
 * AUD
 */

#define SAC_ND_DECL__DESC__AUD_NUQ(nt, decoration)                                       \
    SAC_ND_DECL__DESC__AKS_NUQ (nt, decoration)
#define SAC_ND_DECL__DESC__AUD_UNQ(nt, decoration)                                       \
    SAC_ND_DECL__DESC__AKS_UNQ (nt, decoration)

/*
 * HID
 */

#define SAC_ND_DECL__DESC__HID_NUQ(nt, decoration)                                       \
    SAC_ND_DECL__DESC__AKS_NUQ (nt, decoration)
#define SAC_ND_DECL__DESC__HID_UNQ(nt, decoration) SAC_NOTHING ()

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
 * ND_PARAM_( dummy, basetype)
 *   macro for ... parameters
 *
 * ND_PARAM_in( nt, basetype)
 *   macro for prototyping data as "in" parameter
 *
 * ND_PARAM_in_nodesc( nt, basetype)
 *   macro for prototyping data as "in" parameter without descriptor
 *
 * ND_PARAM_out( nt, basetype)
 *   macro for prototyping data as "out" parameter
 *
 * ND_PARAM_out_nodesc( nt, basetype)
 *   macro for prototyping data as "out" parameter without descriptor
 *
 * ND_PARAM_inout( nt, basetype)
 *   macro for prototyping data as "inout" parameter
 *
 * ND_PARAM_inout_nodesc( nt, basetype)
 *   macro for prototyping data as "inout" parameter without descriptor
 *
 * ND_PARAM_inout_nodesc_bx( nt, basetype)
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
 * ND_DECL_PARAM_inout( nt, basetype)
 *   macro for declaration of "inout" parameter
 *
 ******************************************************************************/

/* creates name for formal function parameter */
#define SAC_NAMEP(name) CAT1 (name, __p)

#define SAC_ND_PARAM_(dummy, basetype) basetype

#define SAC_ND_PARAM_in(nt, basetype)                                                    \
    CAT5 (SAC_ND_PARAM_in__,                                                             \
          CAT5 (NT_DATA (nt), CAT5 (_, CAT5 (NT_UNQ (nt), BuildArgs2 (nt, basetype)))))

#define SAC_ND_PARAM_in_nodesc(nt, basetype)                                             \
    SAC_ND_TYPE (nt, basetype)                                                           \
    SAC_ND_A_FIELD (nt)

#define SAC_ND_PARAM_out(nt, basetype)                                                   \
    CAT5 (SAC_ND_PARAM_out__,                                                            \
          CAT5 (NT_DATA (nt), CAT5 (_, CAT5 (NT_UNQ (nt), BuildArgs2 (nt, basetype)))))

#define SAC_ND_PARAM_out_nodesc(nt, basetype)                                            \
    SAC_ND_TYPE (nt, basetype)                                                           \
    *SAC_NAMEP (SAC_ND_A_FIELD (nt))

#define SAC_ND_PARAM_inout(nt, basetype) SAC_ND_PARAM_out (nt, basetype)

#define SAC_ND_PARAM_inout_nodesc(nt, basetype) SAC_ND_PARAM_out_nodesc (nt, basetype)

#define SAC_ND_PARAM_inout_nodesc_bx(nt, basetype) SAC_ND_PARAM_in_nodesc (nt, basetype)

#define SAC_ND_ARG_in(nt)                                                                \
    CAT5 (SAC_ND_ARG_in__, CAT5 (NT_DATA (nt), CAT5 (_, CAT5 (NT_UNQ (nt), (nt)))))

#define SAC_ND_ARG_in_nodesc(nt) SAC_ND_A_FIELD (nt)

#define SAC_ND_ARG_out(nt)                                                               \
    CAT5 (SAC_ND_ARG_out__, CAT5 (NT_DATA (nt), CAT5 (_, CAT5 (NT_UNQ (nt), (nt)))))

#define SAC_ND_ARG_out_nodesc(nt) &SAC_ND_A_FIELD (nt)

#define SAC_ND_ARG_inout(nt) SAC_ND_ARG_out (nt)

#define SAC_ND_ARG_inout_nodesc(nt) SAC_ND_ARG_out_nodesc (nt)

#define SAC_ND_ARG_inout_nodesc_bx(nt) SAC_ND_ARG_in_nodesc (nt)

#define SAC_ND_RET_out(nt, ntp)                                                          \
    CAT5 (SAC_ND_RET_out__,                                                              \
          CAT5 (NT_DATA (nt), CAT5 (_, CAT5 (NT_UNQ (nt), BuildArgs2 (nt, ntp)))))

#define SAC_ND_RET_inout(nt, ntp) SAC_ND_RET_out (nt, ntp)

#define SAC_ND_DECL_PARAM_inout(nt, basetype)                                            \
    CAT5 (SAC_ND_DECL_PARAM_inout__,                                                     \
          CAT5 (NT_DATA (nt), CAT5 (_, CAT5 (NT_UNQ (nt), BuildArgs2 (nt, basetype)))))

/*
 * SCL
 */

#define SAC_ND_PARAM_in__SCL_NUQ(nt, basetype) SAC_ND_PARAM_in_nodesc (nt, basetype)
#define SAC_ND_PARAM_in__SCL_UNQ(nt, basetype) SAC_ND_PARAM_in__SCL_NUQ (nt, basetype)

#define SAC_ND_PARAM_out__SCL_NUQ(nt, basetype) SAC_ND_PARAM_out_nodesc (nt, basetype)
#define SAC_ND_PARAM_out__SCL_UNQ(nt, basetype) SAC_ND_PARAM_out__SCL_NUQ (nt, basetype)

#define SAC_ND_ARG_in__SCL_NUQ(nt) SAC_ND_ARG_in_nodesc (nt)
#define SAC_ND_ARG_in__SCL_UNQ(nt) SAC_ND_ARG_in__SCL_NUQ (nt)

#define SAC_ND_ARG_out__SCL_NUQ(nt) SAC_ND_ARG_out_nodesc (nt)
#define SAC_ND_ARG_out__SCL_UNQ(nt) SAC_ND_ARG_out__SCL_NUQ (nt)

#define SAC_ND_RET_out__SCL_NUQ(nt, ntp)                                                 \
    {                                                                                    \
        *SAC_NAMEP (SAC_ND_A_FIELD (ntp)) = SAC_ND_A_FIELD (nt);                         \
    }
#define SAC_ND_RET_out__SCL_UNQ(nt, ntp) SAC_ND_RET_out__SCL_NUQ (nt, ntp)

#define SAC_ND_DECL_PARAM_inout__SCL_NUQ(nt, basetype)                                   \
    SAC_ND_TYPE (nt, basetype)                                                           \
    SAC_ND_A_FIELD (nt) = *SAC_NAMEP (SAC_ND_A_FIELD (nt));
#define SAC_ND_DECL_PARAM_inout__SCL_UNQ(nt, basetype)                                   \
    SAC_ND_DECL_PARAM_inout__SCL_NUQ (nt, basetype)

/*
 * AKS
 */

#define SAC_ND_PARAM_in__AKS_NUQ(nt, basetype)                                           \
    SAC_ND_PARAM_in_nodesc (nt, basetype), SAC_ND_DESC_TYPE (nt) SAC_ND_A_DESC (nt)
#define SAC_ND_PARAM_in__AKS_UNQ(nt, basetype) SAC_ND_PARAM_in__AKS_NUQ (nt, basetype)

#define SAC_ND_PARAM_out__AKS_NUQ(nt, basetype)                                          \
    SAC_ND_PARAM_out_nodesc (nt, basetype),                                              \
      SAC_ND_DESC_TYPE (nt) * SAC_NAMEP (SAC_ND_A_DESC (nt))
#define SAC_ND_PARAM_out__AKS_UNQ(nt, basetype) SAC_ND_PARAM_out__AKS_NUQ (nt, basetype)

#define SAC_ND_ARG_in__AKS_NUQ(nt) SAC_ND_ARG_in_nodesc (nt), SAC_ND_A_DESC (nt)
#define SAC_ND_ARG_in__AKS_UNQ(nt) SAC_ND_ARG_in__AKS_NUQ (nt)

#define SAC_ND_ARG_out__AKS_NUQ(nt) SAC_ND_ARG_out_nodesc (nt), &SAC_ND_A_DESC (nt)
#define SAC_ND_ARG_out__AKS_UNQ(nt) SAC_ND_ARG_out__AKS_NUQ (nt)

#define SAC_ND_RET_out__AKS_NUQ(nt, ntp)                                                 \
    {                                                                                    \
        *SAC_NAMEP (SAC_ND_A_FIELD (ntp)) = SAC_ND_A_FIELD (nt);                         \
        *SAC_NAMEP (SAC_ND_A_DESC (ntp)) = SAC_ND_A_DESC (nt);                           \
    }
#define SAC_ND_RET_out__AKS_UNQ(nt, ntp) SAC_ND_RET_out__AKS_NUQ (nt, ntp)

#define SAC_ND_DECL_PARAM_inout__AKS_NUQ(nt, basetype)                                   \
    SAC_ND_TYPE (nt, basetype)                                                           \
    SAC_ND_A_FIELD (nt) = *SAC_NAMEP (SAC_ND_A_FIELD (nt));                              \
    SAC_ND_DESC_TYPE (nt)                                                                \
    SAC_ND_A_DESC (nt) = *SAC_NAMEP (SAC_ND_A_DESC (nt));
#define SAC_ND_DECL_PARAM_inout__AKS_UNQ(nt, basetype)                                   \
    SAC_ND_DECL_PARAM_inout__AKS_NUQ (nt, basetype)

/*
 * AKD
 */

#define SAC_ND_PARAM_in__AKD_NUQ(nt, basetype) SAC_ND_PARAM_in__AKS_NUQ (nt, basetype)
#define SAC_ND_PARAM_in__AKD_UNQ(nt, basetype) SAC_ND_PARAM_in__AKD_NUQ (nt, basetype)

#define SAC_ND_PARAM_out__AKD_NUQ(nt, basetype) SAC_ND_PARAM_out__AKS_NUQ (nt, basetype)
#define SAC_ND_PARAM_out__AKD_UNQ(nt, basetype) SAC_ND_PARAM_out__AKD_NUQ (nt, basetype)

#define SAC_ND_ARG_in__AKD_NUQ(nt) SAC_ND_ARG_in__AKS_NUQ (nt)
#define SAC_ND_ARG_in__AKD_UNQ(nt) SAC_ND_ARG_in__AKD_NUQ (nt)

#define SAC_ND_ARG_out__AKD_NUQ(nt) SAC_ND_ARG_out__AKS_NUQ (nt)
#define SAC_ND_ARG_out__AKD_UNQ(nt) SAC_ND_ARG_out__AKD_NUQ (nt)

#define SAC_ND_RET_out__AKD_NUQ(nt, ntp) SAC_ND_RET_out__AKS_NUQ (nt, ntp)
#define SAC_ND_RET_out__AKD_UNQ(nt, ntp) SAC_ND_RET_out__AKD_NUQ (nt, ntp)

#define SAC_ND_DECL_PARAM_inout__AKD_NUQ(nt, basetype)                                   \
    SAC_ND_DECL_PARAM_inout__AKS_NUQ (nt, basetype)
#define SAC_ND_DECL_PARAM_inout__AKD_UNQ(nt, basetype)                                   \
    SAC_ND_DECL_PARAM_inout__AKD_NUQ (nt, basetype)

/*
 * AUD
 */

#define SAC_ND_PARAM_in__AUD_NUQ(nt, basetype) SAC_ND_PARAM_in__AKS_NUQ (nt, basetype)
#define SAC_ND_PARAM_in__AUD_UNQ(nt, basetype) SAC_ND_PARAM_in__AUD_NUQ (nt, basetype)

#define SAC_ND_PARAM_out__AUD_NUQ(nt, basetype) SAC_ND_PARAM_out__AKS_NUQ (nt, basetype)
#define SAC_ND_PARAM_out__AUD_UNQ(nt, basetype) SAC_ND_PARAM_out__AUD_NUQ (nt, basetype)

#define SAC_ND_ARG_in__AUD_NUQ(nt) SAC_ND_ARG_in__AKS_NUQ (nt)
#define SAC_ND_ARG_in__AUD_UNQ(nt) SAC_ND_ARG_in__AUD_NUQ (nt)

#define SAC_ND_ARG_out__AUD_NUQ(nt) SAC_ND_ARG_out__AKS_NUQ (nt)
#define SAC_ND_ARG_out__AUD_UNQ(nt) SAC_ND_ARG_out__AUD_NUQ (nt)

#define SAC_ND_RET_out__AUD_NUQ(nt, ntp) SAC_ND_RET_out__AKS_NUQ (nt, ntp)
#define SAC_ND_RET_out__AUD_UNQ(nt, ntp) SAC_ND_RET_out__AUD_NUQ (nt, ntp)

#define SAC_ND_DECL_PARAM_inout__AUD_NUQ(nt, basetype)                                   \
    SAC_ND_DECL_PARAM_inout__AKS_NUQ (nt, basetype)
#define SAC_ND_DECL_PARAM_inout__AUD_UNQ(nt, basetype)                                   \
    SAC_ND_DECL_PARAM_inout__AUD_NUQ (nt, basetype)

/*
 * HID
 */

#define SAC_ND_PARAM_in__HID_NUQ(nt, basetype) SAC_ND_PARAM_in__AKS_NUQ (nt, basetype)
#define SAC_ND_PARAM_in__HID_UNQ(nt, basetype) SAC_ND_PARAM_in__SCL_UNQ (nt, basetype)

#define SAC_ND_PARAM_out__HID_NUQ(nt, basetype) SAC_ND_PARAM_out__AKS_NUQ (nt, basetype)
#define SAC_ND_PARAM_out__HID_UNQ(nt, basetype) SAC_ND_PARAM_out__SCL_UNQ (nt, basetype)

#define SAC_ND_ARG_in__HID_NUQ(nt) SAC_ND_ARG_in__AKS_NUQ (nt)
#define SAC_ND_ARG_in__HID_UNQ(nt) SAC_ND_ARG_in__SCL_UNQ (nt)

#define SAC_ND_ARG_out__HID_NUQ(nt) SAC_ND_ARG_out__AKS_NUQ (nt)
#define SAC_ND_ARG_out__HID_UNQ(nt) SAC_ND_ARG_out__SCL_UNQ (nt)

#define SAC_ND_RET_out__HID_NUQ(nt, ntp) SAC_ND_RET_out__AKS_NUQ (nt, ntp)
#define SAC_ND_RET_out__HID_UNQ(nt, ntp) SAC_ND_RET_out__SCL_UNQ (nt, ntp)

#define SAC_ND_DECL_PARAM_inout__HID_NUQ(nt, basetype)                                   \
    SAC_ND_DECL_PARAM_inout__AKS_NUQ (nt, basetype)
#define SAC_ND_DECL_PARAM_inout__HID_UNQ(nt, basetype)                                   \
    SAC_ND_DECL_PARAM_inout__SCL_UNQ (nt, basetype)

/***************
 ***************
 ***
 *** CAT7
 ***
 ***/

/******************************************************************************
 *
 * ICMs for allocation of data objects
 * ===================================
 *
 * ND_ALLOC( nt, dim, ...shp..., rc) :
 *   allocates a data object (no initialization but descriptor!)
 * ND_FREE( nt, freefun) :
 *   frees a data object
 *
 * ND_ALLOC__DATA( nt) :
 *   allocates memory for a data object (without descriptor, no initialization)
 * ND_FREE__DATA( nt, freefun) :
 *   frees memory for a data object (without descriptor)
 *
 * ND_ALLOC__DESC( nt) :
 *   allocates memory for descriptor (no initialization!)
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
#define SAC_ND_ALLOC(nt, rc, set_shape_icm)                                              \
    {                                                                                    \
        SAC_ND_ALLOC__DESC (nt)                                                          \
        SAC_ND_SET__RC (nt, rc)                                                          \
        set_shape_icm SAC_ND_ALLOC__DATA (nt)                                            \
    }
/*
 * these two macros are used instead:
 */
#define SAC_ND_ALLOC_BEGIN(nt, rc)                                                       \
    {                                                                                    \
        SAC_ND_ALLOC__DESC (nt)                                                          \
        SAC_ND_SET__RC (nt, rc)
#define SAC_ND_ALLOC_END(nt, rc)                                                         \
    SAC_ND_ALLOC__DATA (nt)                                                              \
    }

#define SAC_ND_FREE(nt, freefun)                                                         \
    {                                                                                    \
        SAC_ND_FREE__DATA (nt, freefun)                                                  \
        SAC_ND_FREE__DESC (nt)                                                           \
    }

#define SAC_ND_ALLOC__DATA(nt) CAT7 (SAC_ND_ALLOC__DATA__, CAT7 (NT_DATA (nt), (nt)))

#define SAC_ND_FREE__DATA(nt, freefun)                                                   \
    CAT7 (SAC_ND_FREE__DATA__, CAT7 (NT_DATA (nt), BuildArgs2 (nt, freefun)))

#define SAC_ND_ALLOC__DESC(nt)                                                           \
    CAT7 (SAC_ND_ALLOC__DESC__, CAT7 (NT_DATA (nt), CAT7 (_, CAT7 (NT_UNQ (nt), (nt)))))

#define SAC_ND_FREE__DESC(nt)                                                            \
    CAT7 (SAC_ND_FREE__DESC__, CAT7 (NT_DATA (nt), CAT7 (_, CAT7 (NT_UNQ (nt), (nt)))))

/* ND_CHECK_REUSE( ...)  is a C-ICM */

/*
 * SCL
 */

#define SAC_ND_ALLOC__DATA__SCL(nt) SAC_NOOP ()

#define SAC_ND_FREE__DATA__SCL(nt, freefun) SAC_NOOP ()

#define SAC_ND_ALLOC__DESC__SCL_NUQ(nt) SAC_NOOP ()
#define SAC_ND_ALLOC__DESC__SCL_UNQ(nt) SAC_NOOP ()

#define SAC_ND_FREE__DESC__SCL_NUQ(nt) SAC_NOOP ()
#define SAC_ND_FREE__DESC__SCL_UNQ(nt) SAC_NOOP ()

/*
 * AKS
 */

/*
 * CAUTION:
 * The test (SAC_ND_A_SIZE( nt) != 0) is needed to prevent a malloc(0)
 */
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

/*
 * CAUTION:
 * The test (SAC_ND_A_SIZE( nt) != 0) is needed to prevent a malloc(0)
 * (see ALLOC__DATA-icm)
 */
#define SAC_ND_FREE__DATA__AKS(nt, freefun)                                              \
    {                                                                                    \
        if (SAC_ND_A_SIZE (nt) != 0) {                                                   \
            SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_FIELD (nt),                                 \
                                    SAC_ND_A_SIZE (nt) * sizeof (*SAC_ND_A_FIELD (nt)))  \
        }                                                                                \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_FREE__DATA( %s, %s) at addr: %p", #nt, #freefun, SAC_ND_A_FIELD (nt)))    \
        SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (nt))                                     \
        SAC_CS_UNREGISTER_ARRAY (nt)                                                     \
    }

#define SAC_ND_ALLOC__DESC__AKS_NUQ(nt)                                                  \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_DESC (nt), sizeof (*SAC_ND_A_DESC (nt)))      \
        SAC_TR_MEM_PRINT (("ND_ALLOC__DESC( %s) at addr: %p", #nt, SAC_ND_A_DESC (nt)))  \
    }
#define SAC_ND_ALLOC__DESC__AKS_UNQ(nt) SAC_ND_ALLOC__DESC__AKS_NUQ (nt)

#define SAC_ND_FREE__DESC__AKS_NUQ(nt)                                                   \
    {                                                                                    \
        SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_DESC (nt), sizeof (*SAC_ND_A_DESC (nt)))        \
        SAC_TR_MEM_PRINT (("ND_FREE__DESC( %s) at addr: %p", #nt, SAC_ND_A_DESC (nt)))   \
    }
#define SAC_ND_FREE__DESC__AKS_UNQ(nt) SAC_ND_FREE__DESC__AKS_NUQ (nt)

/*
 * AKD
 */

#define SAC_ND_ALLOC__DATA__AKD(nt) SAC_ND_ALLOC__DATA__AKS (nt)

#define SAC_ND_FREE__DATA__AKD(nt, freefun) SAC_ND_FREE__DATA__AKS (nt, freefun)

#define SAC_ND_ALLOC__DESC__AKD_NUQ(nt) SAC_ND_ALLOC__DESC__AKS_NUQ (nt)
#define SAC_ND_ALLOC__DESC__AKD_UNQ(nt) SAC_ND_ALLOC__DESC__AKS_UNQ (nt)

#define SAC_ND_FREE__DESC__AKD_NUQ(nt) SAC_ND_FREE__DESC__AKS_NUQ (nt)
#define SAC_ND_FREE__DESC__AKD_UNQ(nt) SAC_ND_FREE__DESC__AKS_UNQ (nt)

/*
 * AUD
 */

#define SAC_ND_ALLOC__DATA__AUD(nt) SAC_ND_ALLOC__DATA__AKS (nt)

#define SAC_ND_FREE__DATA__AUD(nt, freefun) SAC_ND_FREE__DATA__AKS (nt, freefun)

#define SAC_ND_ALLOC__DESC__AUD_NUQ(nt) SAC_ND_ALLOC__DESC__AKS_NUQ (nt)
#define SAC_ND_ALLOC__DESC__AUD_UNQ(nt) SAC_ND_ALLOC__DESC__AKS_UNQ (nt)

#define SAC_ND_FREE__DESC__AUD_NUQ(nt) SAC_ND_FREE__DESC__AKS_NUQ (nt)
#define SAC_ND_FREE__DESC__AUD_UNQ(nt) SAC_ND_FREE__DESC__AKS_UNQ (nt)

/*
 * HID
 */

#define SAC_ND_ALLOC__DATA__HID(nt) SAC_NOOP ()

#define SAC_ND_FREE__DATA__HID(nt, freefun)                                              \
    {                                                                                    \
        freefun (SAC_ND_A_FIELD (nt));                                                   \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_FREE__DATA( %s, %s) at addr: %p", #nt, #freefun, SAC_ND_A_FIELD (nt)))    \
        SAC_TR_DEC_HIDDEN_MEMCNT (1)                                                     \
    }

#define SAC_ND_ALLOC__DESC__HID_NUQ(nt) SAC_ND_ALLOC__DESC__AKS_NUQ (nt)
#define SAC_ND_ALLOC__DESC__HID_UNQ(nt) SAC_NOOP ()

#define SAC_ND_FREE__DESC__HID_NUQ(nt) SAC_ND_FREE__DESC__AKS_NUQ (nt)
#define SAC_ND_FREE__DESC__HID_UNQ(nt) SAC_NOOP ()

/***************
 ***************
 ***
 *** CAT8, CAT9
 ***
 ***/

/******************************************************************************
 *
 * ICMs for assigning data objects
 * ===============================
 *
 * ND_ASSIGN( to_nt, to_sdim, from_nt) :
 *   assigns a data object to another one
 * ND_ASSIGN__DATA( to_nt, from_nt) :
 *   assigns a data object to another one (without mirror)
 * ND_ASSIGN__DESC( to_nt, from_nt) :
 *   assigns a descriptor to another one
 * ND_ASSIGN__SHAPE( to_nt, to_sdim, from_nt, from_sdim) :
 *   assigns a shape information (descriptor *and* mirror) to another one
 *
 * ND_COPY( to_nt, to_sdim, from_nt, from_sdim, copyfun) :
 *   copies a data object to another one
 * ND_COPY__DATA( to_nt, from_nt, copyfun) :
 *   copies a data object to another one (without mirror)
 * ND_COPY__SHAPE( to_nt, to_sdim, from_nt, from_sdim) :
 *   copies a shape information (descriptor *and* mirror) to another one
 *
 * ND_MAKE_UNIQUE( to_nt, to_sdim, from_nt, copyfun) :
 *   assigns a data object to another one iff RC is zero, copies it otherwise.
 *
 ******************************************************************************/

/* ND_ASSIGN( ...)  is a C-ICM */

#define SAC_ND_ASSIGN__DATA(to_nt, from_nt)                                              \
    CAT8 (SAC_ND_ASSIGN__DATA__,                                                         \
          CAT8 (NT_DATA (to_nt),                                                         \
                CAT8 (_, CAT8 (NT_DATA (from_nt), BuildArgs2 (to_nt, from_nt)))))

/* ND_ASSIGN__DESC( ...)  is a C-ICM */

/* ND_ASSIGN__SHAPE( ...)  is a C-ICM */

/* ND_COPY( ...)  is a C-ICM */

#define SAC_ND_COPY__DATA(to_nt, from_nt, copyfun)                                       \
    CAT8 (SAC_ND_COPY__DATA__,                                                           \
          CAT8 (NT_DATA (to_nt), CAT8 (_, CAT8 (NT_DATA (from_nt),                       \
                                                BuildArgs3 (to_nt, from_nt, copyfun)))))

/* ND_COPY__SHAPE( ...)  is a C-ICM */

/* ND_MAKE_UNIQUE( ...)  is a C-ICM */

/*
 * SCL
 */

#define SAC_ND_ASSIGN__DATA__SCL_SCL(to_nt, from_nt)                                     \
    {                                                                                    \
        SAC_ND_A_FIELD (to_nt) = SAC_ND_A_FIELD (from_nt);                               \
    }
#define SAC_ND_ASSIGN__DATA__SCL_AKS(to_nt, from_nt) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__SCL_AKD(to_nt, from_nt) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__SCL_AUD(to_nt, from_nt)                                     \
    {                                                                                    \
        SAC_ND_A_FIELD (to_nt) = SAC_ND_READ (from_nt, 0);                               \
    }
#define SAC_ND_ASSIGN__DATA__SCL_HID(to_nt, from_nt) SAC_ICM_UNDEF ();

#define SAC_ND_COPY__DATA__SCL_SCL(to_nt, from_nt, copyfun)                              \
    {                                                                                    \
        SAC_ND_WRITE (to_nt, 0) = SAC_ND_READ (from_nt, 0);                              \
    }
#define SAC_ND_COPY__DATA__SCL_AKS(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__SCL_AKD(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__SCL_AUD(to_nt, from_nt, copyfun)                              \
    {                                                                                    \
        SAC_ND_WRITE (to_nt, 0) = SAC_ND_READ (from_nt, 0);                              \
    }
#define SAC_ND_COPY__DATA__SCL_HID(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();

/*
 * AKS
 */

#define SAC_ND_ASSIGN__DATA__AKS_SCL(to_nt, from_nt) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__AKS_AKS(to_nt, from_nt)                                     \
    {                                                                                    \
        SAC_ND_A_FIELD (to_nt) = SAC_ND_A_FIELD (from_nt);                               \
    }
#define SAC_ND_ASSIGN__DATA__AKS_AKD(to_nt, from_nt)                                     \
    SAC_ND_ASSIGN__DATA__AKS_AKS (to_nt, from_nt)
#define SAC_ND_ASSIGN__DATA__AKS_AUD(to_nt, from_nt)                                     \
    SAC_ND_ASSIGN__DATA__AKS_AKS (to_nt, from_nt)
#define SAC_ND_ASSIGN__DATA__AKS_HID(to_nt, from_nt) SAC_ICM_UNDEF ();

#define SAC_ND_COPY__DATA__AKS_SCL(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__AKS_AKS(to_nt, from_nt, copyfun)                              \
    {                                                                                    \
        int SAC__i;                                                                      \
        for (SAC__i = 0; SAC__i < SAC_ND_A_SIZE (from_nt); SAC__i++) {                   \
            SAC_ND_WRITE (to_nt, SAC__i) = SAC_ND_READ (from_nt, SAC__i);                \
        }                                                                                \
        SAC_ASSURE_TYPE ((SAC_ND_A_SIZE (to_nt) == SAC_ND_A_SIZE (from_nt)),             \
                         "Assignment with incompatible types found!");                   \
        SAC_TR_MEM_PRINT (("ND_COPY__DATA( %s, %s, %s) at addr: %p", #from_nt, #to_nt,   \
                           #copyfun, SAC_ND_A_FIELD (to_nt)))                            \
    }
#define SAC_ND_COPY__DATA__AKS_AKD(to_nt, from_nt, copyfun)                              \
    SAC_ND_COPY__DATA__AKS_AKS (to_nt, from_nt, copyfun)
#define SAC_ND_COPY__DATA__AKS_AUD(to_nt, from_nt, copyfun)                              \
    SAC_ND_COPY__DATA__AKS_AKS (to_nt, from_nt, copyfun)
#define SAC_ND_COPY__DATA__AKS_HID(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();

/*
 * AKD
 */

#define SAC_ND_ASSIGN__DATA__AKD_SCL(to_nt, from_nt) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__AKD_AKS(to_nt, from_nt)                                     \
    SAC_ND_ASSIGN__DATA__AKS_AKS (to_nt, from_nt)
#define SAC_ND_ASSIGN__DATA__AKD_AKD(to_nt, from_nt)                                     \
    SAC_ND_ASSIGN__DATA__AKS_AKS (to_nt, from_nt)
#define SAC_ND_ASSIGN__DATA__AKD_AUD(to_nt, from_nt)                                     \
    SAC_ND_ASSIGN__DATA__AKS_AKS (to_nt, from_nt)
#define SAC_ND_ASSIGN__DATA__AKD_HID(to_nt, from_nt) SAC_ICM_UNDEF ();

#define SAC_ND_COPY__DATA__AKD_SCL(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__AKD_AKS(to_nt, from_nt, copyfun)                              \
    SAC_ND_COPY__DATA__AKS_AKS (to_nt, from_nt, copyfun)
#define SAC_ND_COPY__DATA__AKD_AKD(to_nt, from_nt, copyfun)                              \
    SAC_ND_COPY__DATA__AKS_AKS (to_nt, from_nt, copyfun)
#define SAC_ND_COPY__DATA__AKD_AUD(to_nt, from_nt, copyfun)                              \
    SAC_ND_COPY__DATA__AKS_AKS (to_nt, from_nt, copyfun)
#define SAC_ND_COPY__DATA__AKD_HID(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();

/*
 * AUD
 */

#define SAC_ND_ASSIGN__DATA__AUD_SCL(to_nt, from_nt)                                     \
    {                                                                                    \
        SAC_ND_WRITE (to_nt, 0) = SAC_ND_A_FIELD (from_nt);                              \
    }
#define SAC_ND_ASSIGN__DATA__AUD_AKS(to_nt, from_nt)                                     \
    SAC_ND_ASSIGN__DATA__AKS_AKS (to_nt, from_nt)
#define SAC_ND_ASSIGN__DATA__AUD_AKD(to_nt, from_nt)                                     \
    SAC_ND_ASSIGN__DATA__AKS_AKS (to_nt, from_nt)
#define SAC_ND_ASSIGN__DATA__AUD_AUD(to_nt, from_nt)                                     \
    SAC_ND_ASSIGN__DATA__AKS_AKS (to_nt, from_nt)
#define SAC_ND_ASSIGN__DATA__AUD_HID(to_nt, from_nt) SAC_ICM_UNDEF ();

#define SAC_ND_COPY__DATA__AUD_SCL(to_nt, from_nt, copyfun)                              \
    {                                                                                    \
        SAC_ND_WRITE (to_nt, 0) = SAC_ND_READ (from_nt, 0);                              \
    }
#define SAC_ND_COPY__DATA__AUD_AKS(to_nt, from_nt, copyfun)                              \
    SAC_ND_COPY__DATA__AKS_AKS (to_nt, from_nt, copyfun)
#define SAC_ND_COPY__DATA__AUD_AKD(to_nt, from_nt, copyfun)                              \
    SAC_ND_COPY__DATA__AKS_AKS (to_nt, from_nt, copyfun)
#define SAC_ND_COPY__DATA__AUD_AUD(to_nt, from_nt, copyfun)                              \
    SAC_ND_COPY__DATA__AKS_AKS (to_nt, from_nt, copyfun)
#define SAC_ND_COPY__DATA__AUD_HID(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();

/*
 * HID
 */

#define SAC_ND_ASSIGN__DATA__HID_SCL(to_nt, from_nt) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__HID_AKS(to_nt, from_nt) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__HID_AKD(to_nt, from_nt) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__HID_AUD(to_nt, from_nt) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__HID_HID(to_nt, from_nt)                                     \
    {                                                                                    \
        SAC_ND_A_FIELD (to_nt) = SAC_ND_A_FIELD (from_nt);                               \
    }

#define SAC_ND_COPY__DATA__HID_SCL(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__HID_AKS(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__HID_AKD(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__HID_AUD(to_nt, from_nt, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__HID_HID(to_nt, from_nt, copyfun)                              \
    {                                                                                    \
        SAC_ND_A_FIELD (to_nt) = copyfun (SAC_ND_A_FIELD (from_nt));                     \
        SAC_TR_MEM_PRINT (("ND_COPY__DATA( %s, %s, %s)", #to_nt, #from_nt, #copyfun))    \
        SAC_TR_MEM_PRINT (("new hidden object at addr: %p", SAC_ND_A_FIELD (to_nt)))     \
        SAC_TR_INC_HIDDEN_MEMCNT (1)                                                     \
    }

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
    CAT8 (SAC_ND_SET__RC__, CAT8 (NT_UNQ (nt), BuildArgs2 (nt, rc)))

#define SAC_ND_INC_RC(nt, rc)                                                            \
    CAT8 (SAC_ND_INC_RC__, CAT8 (NT_UNQ (nt), BuildArgs2 (nt, rc)))

#define SAC_ND_DEC_RC(nt, rc)                                                            \
    CAT8 (SAC_ND_DEC_RC__, CAT8 (NT_UNQ (nt), BuildArgs2 (nt, rc)))

#define SAC_ND_DEC_RC_FREE(nt, rc, freefun)                                              \
    CAT8 (SAC_ND_DEC_RC_FREE__, CAT8 (NT_UNQ (nt), BuildArgs3 (nt, rc, freefun)))

/*
 * NUQ
 */

#define SAC_ND_SET__RC__NUQ(nt, rc)                                                      \
    CAT9 (SAC_ND_SET__RC__,                                                              \
          CAT9 (NT_DATA (nt), CAT9 (_, CAT9 (NT_UNQ (nt), BuildArgs2 (nt, rc)))))

#define SAC_ND_INC_RC__NUQ(nt, rc)                                                       \
    CAT9 (SAC_ND_INC_RC__,                                                               \
          CAT9 (NT_DATA (nt), CAT9 (_, CAT9 (NT_UNQ (nt), BuildArgs2 (nt, rc)))))

#define SAC_ND_DEC_RC__NUQ(nt, rc)                                                       \
    CAT9 (SAC_ND_DEC_RC__,                                                               \
          CAT9 (NT_DATA (nt), CAT9 (_, CAT9 (NT_UNQ (nt), BuildArgs2 (nt, rc)))))

#define SAC_ND_DEC_RC_FREE__NUQ(nt, rc, freefun)                                         \
    CAT9 (SAC_ND_DEC_RC_FREE__,                                                          \
          CAT9 (NT_DATA (nt),                                                            \
                CAT9 (_, CAT9 (NT_UNQ (nt), BuildArgs3 (nt, rc, freefun)))))

/*
 * UNQ
 */

#define SAC_ND_SET__RC__UNQ(nt, rc) SAC_NOOP ()

#define SAC_ND_INC_RC__UNQ(nt, rc) SAC_NOOP ()

#define SAC_ND_DEC_RC__UNQ(nt, rc) SAC_NOOP ()

#define SAC_ND_DEC_RC_FREE__UNQ(nt, rc, freefun) SAC_NOOP ()

/*
 * SCL, NUQ
 */

#define SAC_ND_SET__RC__SCL_NUQ(nt, rc) SAC_NOOP ()

#define SAC_ND_INC_RC__SCL_NUQ(nt, rc) SAC_NOOP ()

#define SAC_ND_DEC_RC__SCL_NUQ(nt, rc) SAC_NOOP ()

#define SAC_ND_DEC_RC_FREE__SCL_NUQ(nt, rc, freefun) SAC_NOOP ()

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

/*
 * HID, NUQ
 */

#define SAC_ND_SET__RC__HID_NUQ(nt, rc) SAC_ND_SET__RC__AKS_NUQ (nt, rc)

#define SAC_ND_INC_RC__HID_NUQ(nt, rc) SAC_ND_INC_RC__AKS_NUQ (nt, rc)

#define SAC_ND_DEC_RC__HID_NUQ(nt, rc) SAC_ND_DEC_RC__AKS_NUQ (nt, rc)

#define SAC_ND_DEC_RC_FREE__HID_NUQ(nt, rc, freefun)                                     \
    SAC_ND_DEC_RC_FREE__AKS_NUQ (nt, rc, freefun)

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
 * ND_CREATE__VECT__SHAPE( nt, sdim, len, ...elem..., copyfun) :
 *   creates shape of a constant vector
 * ND_CREATE__VECT__DATA( nt, sdim, len, ...elem..., copyfun) :
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

/* ND_CREATE__VECT__SHAPE( ...) is a C-ICM */
/* ND_CREATE__VECT__DATA( ...) is a C-ICM */

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

#define SAC_IS_LASTREF__THEN(nt) CAT8 (SAC_IS_LASTREF__THEN__, CAT8 (NT_UNQ (nt), (nt)))
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
    CAT9 (SAC_IS_LASTREF__THEN__, CAT9 (NT_DATA (nt), CAT9 (_, CAT9 (NT_UNQ (nt), (nt)))))
/*
 * UNQ
 */

#define SAC_IS_LASTREF__THEN__UNQ(nt) if (0)

/*
 * SCL, NUQ
 */

#define SAC_IS_LASTREF__THEN__SCL_NUQ(nt) if (0)

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

/*
 * HID, NUQ
 */

#define SAC_IS_LASTREF__THEN__HID_NUQ(nt) SAC_IS_LASTREF__THEN__AKS_NUQ (nt)

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
