/*
 *
 * $Log$
 * Revision 3.54  2003/11/10 20:22:56  dkrHH
 * debug output: NT objs are converted into strings correctly now
 *
 * Revision 3.53  2003/10/20 18:15:12  dkr
 * some comments added
 *
 * Revision 3.52  2003/10/15 17:25:01  dkrHH
 * some comments corrected
 *
 * Revision 3.51  2003/10/14 14:56:30  cg
 * some superfluous calls of SAC_TR_REF_PRINT_RC removed.
 * some calls of SAC_TR_MEM_PRINT corrected.
 *
 * Revision 3.50  2003/10/08 15:50:23  cg
 * De-allocation of descriptors is now done using tailor-made ICM.
 *
 * Revision 3.49  2003/09/29 23:46:27  dkr
 * SAC_IS_REUSED__BLOCK_BEGIN__SCL corrected
 *
 * Revision 3.48  2003/09/29 22:52:18  dkr
 * some icms removed/renamed/added.
 * some comments corrected
 *
 * Revision 3.47  2003/09/25 14:03:13  dkr
 * ND_WRITE_READ corrected
 *
 * Revision 3.46  2003/09/25 13:45:25  dkr
 * ND_WRITE_READ added.
 * some bugs fixed.
 *
 * Revision 3.45  2003/09/19 12:26:49  dkr
 * postfixes _nt, _any of varnames renamed into _NT, _ANY
 *
 * Revision 3.44  2003/09/18 15:11:38  dkr
 * nesting of CAT?? macros for ALLOC ICMs corrected
 *
 * Revision 3.43  2003/09/18 12:23:37  dkr
 * BYTE_SIZE_OF_DESC added.
 * some bugs fixed.
 *
 * Revision 3.42  2003/09/18 11:38:48  dkr
 * ND_ALLOC__DESC_AND_DATA added (for DAO)
 *
 * Revision 3.41  2003/09/17 18:13:39  dkr
 * ALLOC_FIXED_SIZE, FREE_FIXED_SIZE used for arrays with statically
 * known size only
 *
 * Revision 3.40  2003/09/17 14:18:16  dkr
 * definition of SAC_ND_A_RC__SCL_NUQ corrected
 *
 * Revision 3.39  2003/09/16 15:57:57  dkr
 * some comments modified
 *
 * Revision 3.38  2003/08/04 14:01:59  dkr
 * comment for SAC_ND_PARAM_ added
 *
 * Revision 3.37  2003/06/12 17:21:54  dkr
 * ICMs CREATE__VECT__... renamed into CREATE__ARRAY__...
 *
 * Revision 3.36  2003/04/15 17:57:10  dkr
 * implementation of ALLOC/FREE-ICMs modified: test for SIZE==0 removed
 *
 * Revision 3.35  2003/04/15 14:39:49  dkr
 * workaround for (dim < 0) in ND_ALLOC__DESC__AUD removed
 *
 * Revision 3.34  2003/04/14 15:15:33  dkr
 * IS_LASTREF__THEN, IS_LASTREF__ELSE removed
 * IS_REUSED__BLOCK_... added
 *
 * Revision 3.33  2003/03/09 21:31:31  dkr
 * , moved from SAC_ND_WRITE__AKS to SAC_TR_AA_PRINT
 *
 * Revision 3.32  2003/03/09 21:26:48  dkr
 * SAC_TR_AA_PRINT added
 *
 * Revision 3.31  2002/11/26 00:17:43  dkr
 * several superfluous concat-operators removed in macro definitions
 *
 * Revision 3.30  2002/10/29 19:07:41  dkr
 * definition of ND_A_DESC_... modified
 *
 * Revision 3.29  2002/10/10 23:53:20  dkr
 * syntax error fixed
 *
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

#ifndef _SAC_STD_H_
#define _SAC_STD_H_

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

#define NT_NAME(var_NT) Item0 var_NT
#define NT_SHP(var_NT) Item1 var_NT
#define NT_HID(var_NT) Item2 var_NT
#define NT_UNQ(var_NT) Item3 var_NT

#define NT_STR(var_NT) TO_STR (NT_NAME (var_NT))

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

/* size of dimension-independent parts of the descriptor */
#define FIXED_SIZE_OF_DESC 3
/* size of dimension-dependent parts of the descriptor */
#define VAR_SIZE_OF_DESC 1
/* size of the descriptor = (FIXED_SIZE_OF_DESC + dim * VAR_SIZE_OF_DESC) */
#define SIZE_OF_DESC(dim) (FIXED_SIZE_OF_DESC + dim * VAR_SIZE_OF_DESC)

#define BYTE_SIZE_OF_DESC(dim) (SIZE_OF_DESC (dim) * sizeof (int))

#define DESC_RC(desc) desc[0]
#define DESC_DIM(desc) desc[1]
#define DESC_SIZE(desc) desc[2]
#define DESC_SHAPE(desc, pos) desc[FIXED_SIZE_OF_DESC + pos]

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
 * ND_A_DESC( var_NT) :
 *   accesses the descriptor of the data object
 *
 * ND_A_DESC_DIM( var_NT) :
 *   accesses the dimension of the data object via descriptor(!)
 * ND_A_DESC_SIZE( var_NT) :
 *   accesses the size (number of elements) of the data object via descriptor(!)
 * ND_A_DESC_SHAPE( var_NT, dim) :
 *   accesses a shape component of the data object via descriptor(!)
 *
 * ND_A_MIRROR_DIM( var_NT) :
 *   accesses the dimension of the data object via mirror(!)
 * ND_A_MIRROR_SIZE( var_NT) :
 *   accesses the size of the data object via mirror(!)
 * ND_A_MIRROR_SHAPE( var_NT, dim) :
 *   accesses a shape component of the data object via mirror(!)
 *
 * ND_A_FIELD( var_NT) :
 *   accesses the pointer to the data object (array)
 *   or the data object (scalar) respectively
 *
 * ND_A_RC( var_NT) :
 *   accesses the reference counter
 * ND_A_DIM( var_NT) :
 *   accesses the dimension (via mirror if exists, via descriptor otherwise)
 * ND_A_SIZE( var_NT) :
 *   accesses the size (via mirror if exists, via descriptor otherwise)
 * ND_A_SHAPE( var_NT, dim) :
 *   accesses a shape component (via mirror if exists, via descriptor otherwise)
 *
 ******************************************************************************/

#define SAC_ND_A_DESC(var_NT) CAT2 (SAC_ND_A_DESC__, NT_SHP (var_NT) (var_NT))

#define SAC_ND_A_DESC_DIM(var_NT) CAT7 (SAC_ND_A_DESC_DIM__, NT_SHP (var_NT) (var_NT))

#define SAC_ND_A_DESC_SIZE(var_NT) CAT7 (SAC_ND_A_DESC_SIZE__, NT_SHP (var_NT) (var_NT))

#define SAC_ND_A_DESC_SHAPE(var_NT, dim)                                                 \
    CAT7 (SAC_ND_A_DESC_SHAPE__, NT_SHP (var_NT) BuildArgs2 (var_NT, dim))

#define SAC_ND_A_MIRROR_DIM(var_NT) CAT7 (SAC_ND_A_MIRROR_DIM__, NT_SHP (var_NT) (var_NT))

#define SAC_ND_A_MIRROR_SIZE(var_NT)                                                     \
    CAT7 (SAC_ND_A_MIRROR_SIZE__, NT_SHP (var_NT) (var_NT))

#define SAC_ND_A_MIRROR_SHAPE(var_NT, dim)                                               \
    CAT7 (SAC_ND_A_MIRROR_SHAPE__, NT_SHP (var_NT) BuildArgs2 (var_NT, dim))

#define SAC_ND_A_FIELD(var_NT) CAT6 (SAC_ND_A_FIELD__, NT_SHP (var_NT) (var_NT))

#define SAC_ND_A_RC(var_NT) CAT6 (SAC_ND_A_RC__, NT_UNQ (var_NT) (var_NT))

#define SAC_ND_A_DIM(var_NT) CAT6 (SAC_ND_A_DIM__, NT_SHP (var_NT) (var_NT))

#define SAC_ND_A_SIZE(var_NT) CAT6 (SAC_ND_A_SIZE__, NT_SHP (var_NT) (var_NT))

#define SAC_ND_A_SHAPE(var_NT, dim)                                                      \
    CAT6 (SAC_ND_A_SHAPE__, NT_SHP (var_NT) BuildArgs2 (var_NT, dim))

/*
 * NUQ
 */

#define SAC_ND_A_RC__NUQ(var_NT)                                                         \
    CAT7 (SAC_ND_A_RC__, CAT7 (NT_SHP (var_NT), _NUQ (var_NT)))

/*
 * UNQ
 */

#define SAC_ND_A_RC__UNQ(var_NT) SAC_ICM_UNDEF ()

/*
 * SCL
 */

#define SAC_ND_A_DESC__SCL(var_NT) CAT3 (SAC_ND_A_DESC__SCL_, NT_HID (var_NT) (var_NT))
#define SAC_ND_A_DESC__SCL_NHD(var_NT) SAC_ICM_UNDEF ()
#define SAC_ND_A_DESC__SCL_HID(var_NT)                                                   \
    CAT4 (SAC_ND_A_DESC__SCL_HID_, NT_UNQ (var_NT) (var_NT))
#define SAC_ND_A_DESC__SCL_HID_NUQ(var_NT) SAC_ND_A_DESC__AKS (var_NT)
#define SAC_ND_A_DESC__SCL_HID_UNQ(var_NT) SAC_ICM_UNDEF ()

#define SAC_ND_A_DESC_DIM__SCL(var_NT) SAC_ICM_UNDEF ()

#define SAC_ND_A_DESC_SIZE__SCL(var_NT) SAC_ICM_UNDEF ()

#define SAC_ND_A_DESC_SHAPE__SCL(var_NT, dim) SAC_ICM_UNDEF ()

#define SAC_ND_A_MIRROR_DIM__SCL(var_NT) 0

#define SAC_ND_A_MIRROR_SIZE__SCL(var_NT) 1

#define SAC_ND_A_MIRROR_SHAPE__SCL(var_NT, dim) SAC_ICM_UNDEF ()

#define SAC_ND_A_FIELD__SCL(var_NT) NT_NAME (var_NT)

#define SAC_ND_A_RC__SCL_NUQ(var_NT)                                                     \
    CAT8 (SAC_ND_A_RC__SCL_, CAT8 (NT_HID (var_NT), _NUQ (var_NT)))
#define SAC_ND_A_RC__SCL_NHD_NUQ(var_NT) SAC_ICM_UNDEF ()
#define SAC_ND_A_RC__SCL_HID_NUQ(var_NT) SAC_ND_A_RC__AKS_NUQ (var_NT)

#define SAC_ND_A_DIM__SCL(var_NT) SAC_ND_A_MIRROR_DIM (var_NT)

#define SAC_ND_A_SIZE__SCL(var_NT) SAC_ND_A_MIRROR_SIZE (var_NT)

#define SAC_ND_A_SHAPE__SCL(var_NT, dim) (-1) /* SAC_ICM_UNDEF() */
                                              /*
                                               * For convenience reasons, SAC_ND_A_SHAPE__SCL is *not* undefined but
                                               * a dummy value. Now, the following code fragment works even for scalars:
                                               *   for (i = 0; i < SAC_ND_A_DIM( var_NT); i++) {
                                               *     ... SAC_ND_A_SHAPE( var_NT, i) ...
                                               *   }
                                               * Unfortunately, however, things will go wrong if SAC_ND_A_SHAPE is used
                                               * on scalars in non-dead code...
                                               */

/*
 * AKS
 */

#define SAC_ND_A_DESC__AKS(var_NT) CAT5 (NT_NAME (var_NT), __desc)

#define SAC_ND_A_DESC_DIM__AKS(var_NT) SAC_ICM_UNDEF ()

#define SAC_ND_A_DESC_SIZE__AKS(var_NT) SAC_ICM_UNDEF ()

#define SAC_ND_A_DESC_SHAPE__AKS(var_NT, dim) SAC_ICM_UNDEF ()

#define SAC_ND_A_MIRROR_DIM__AKS(var_NT) CAT8 (NT_NAME (var_NT), __dim)

#define SAC_ND_A_MIRROR_SIZE__AKS(var_NT) CAT8 (NT_NAME (var_NT), __sz)

#define SAC_ND_A_MIRROR_SHAPE__AKS(var_NT, dim) CAT8 (NT_NAME (var_NT), CAT8 (__shp, dim))

#define SAC_ND_A_FIELD__AKS(var_NT) NT_NAME (var_NT)

#define SAC_ND_A_RC__AKS_NUQ(var_NT) DESC_RC (SAC_ND_A_DESC (var_NT))

#define SAC_ND_A_DIM__AKS(var_NT) SAC_ND_A_MIRROR_DIM (var_NT)

#define SAC_ND_A_SIZE__AKS(var_NT) SAC_ND_A_MIRROR_SIZE (var_NT)

#define SAC_ND_A_SHAPE__AKS(var_NT, dim) SAC_ND_A_MIRROR_SHAPE (var_NT, dim)

/*
 * AKD
 */

#define SAC_ND_A_DESC__AKD(var_NT) SAC_ND_A_DESC__AKS (var_NT)

#define SAC_ND_A_DESC_DIM__AKD(var_NT) SAC_ICM_UNDEF ()

#define SAC_ND_A_DESC_SIZE__AKD(var_NT) DESC_SIZE (SAC_ND_A_DESC (var_NT))

#define SAC_ND_A_DESC_SHAPE__AKD(var_NT, dim) DESC_SHAPE (SAC_ND_A_DESC (var_NT), dim)

#define SAC_ND_A_MIRROR_DIM__AKD(var_NT) SAC_ND_A_MIRROR_DIM__AKS (var_NT)

#define SAC_ND_A_MIRROR_SIZE__AKD(var_NT) SAC_ND_A_MIRROR_SIZE__AKS (var_NT)

#define SAC_ND_A_MIRROR_SHAPE__AKD(var_NT, dim) SAC_ND_A_MIRROR_SHAPE__AKS (var_NT, dim)

#define SAC_ND_A_FIELD__AKD(var_NT) SAC_ND_A_FIELD__AKS (var_NT)

#define SAC_ND_A_RC__AKD_NUQ(var_NT) SAC_ND_A_RC__AKS_NUQ (var_NT)

#define SAC_ND_A_DIM__AKD(var_NT) SAC_ND_A_MIRROR_DIM (var_NT)

#define SAC_ND_A_SIZE__AKD(var_NT) SAC_ND_A_MIRROR_SIZE (var_NT)

#define SAC_ND_A_SHAPE__AKD(var_NT, dim) SAC_ND_A_MIRROR_SHAPE (var_NT, dim)

/*
 * AUD
 */

#define SAC_ND_A_DESC__AUD(var_NT) SAC_ND_A_DESC__AKS (var_NT)

#define SAC_ND_A_DESC_DIM__AUD(var_NT) DESC_DIM (SAC_ND_A_DESC (var_NT))

#define SAC_ND_A_DESC_SIZE__AUD(var_NT) DESC_SIZE (SAC_ND_A_DESC (var_NT))

#define SAC_ND_A_DESC_SHAPE__AUD(var_NT, dim) DESC_SHAPE (SAC_ND_A_DESC (var_NT), dim)

#define SAC_ND_A_MIRROR_DIM__AUD(var_NT) SAC_ND_A_MIRROR_DIM__AKS (var_NT)

#define SAC_ND_A_MIRROR_SIZE__AUD(var_NT) SAC_ND_A_MIRROR_SIZE__AKS (var_NT)

#define SAC_ND_A_MIRROR_SHAPE__AUD(var_NT, dim) SAC_ICM_UNDEF ()

#define SAC_ND_A_FIELD__AUD(var_NT) SAC_ND_A_FIELD__AKS (var_NT)

#define SAC_ND_A_RC__AUD_NUQ(var_NT) SAC_ND_A_RC__AKS_NUQ (var_NT)

#define SAC_ND_A_DIM__AUD(var_NT) SAC_ND_A_MIRROR_DIM (var_NT)

#define SAC_ND_A_SIZE__AUD(var_NT) SAC_ND_A_MIRROR_SIZE (var_NT)

#define SAC_ND_A_SHAPE__AUD(var_NT, dim) SAC_ND_A_DESC_SHAPE (var_NT, dim)

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
 * ND_READ( from_NT, from_pos) :
 *   read access at specified index position
 * ND_WRITE( to_NT, to_pos) :
 *   write access at specified index position
 * ND_WRITE_READ( to_NT, to_pos, from_NT, from_pos) :
 *   write/read access at specified index positions
 *
 * ND_WRITE_COPY( to_NT, to_pos, expr, copyfun) :
 *   write/copy access at specified index position
 * ND_WRITE_READ_COPY( to_NT, to_pos, from_NT, from_pos, copyfun) :
 *   write/read/copy access at specified index positions
 *
 ******************************************************************************/

#define SAC_ND_READ(from_NT, from_pos)                                                   \
    CAT9 (SAC_ND_READ__, NT_SHP (from_NT) BuildArgs2 (from_NT, from_pos))

#define SAC_ND_WRITE(to_NT, to_pos)                                                      \
    CAT9 (SAC_ND_WRITE__, NT_SHP (to_NT) BuildArgs2 (to_NT, to_pos))

#define SAC_ND_WRITE_READ(to_NT, to_pos, from_NT, from_pos)                              \
    {                                                                                    \
        SAC_ND_WRITE (to_NT, to_pos) = SAC_ND_READ (from_NT, from_pos);                  \
    }

#define SAC_ND_WRITE_COPY(to_NT, to_pos, expr, copyfun)                                  \
    CAT10 (SAC_ND_WRITE_COPY__, NT_HID (to_NT) BuildArgs4 (to_NT, to_pos, expr, copyfun))

#define SAC_ND_WRITE_READ_COPY(to_NT, to_pos, from_NT, from_pos, copyfun)                \
    SAC_ND_WRITE_COPY (to_NT, to_pos, SAC_ND_READ (from_NT, from_pos), copyfun)

/*
 * NHD
 */

#define SAC_ND_WRITE_COPY__NHD(to_NT, to_pos, expr, copyfun)                             \
    {                                                                                    \
        SAC_ND_WRITE (to_NT, to_pos) = expr;                                             \
    }

/*
 * HID
 */

#define SAC_ND_WRITE_COPY__HID(to_NT, to_pos, expr, copyfun)                             \
    {                                                                                    \
        SAC_ND_WRITE (to_NT, to_pos) = copyfun (expr);                                   \
        SAC_TR_MEM_PRINT (                                                               \
          ("new hidden object at addr: %p", SAC_ND_READ (to_NT, to_pos)))                \
        SAC_TR_INC_HIDDEN_MEMCNT (1)                                                     \
    }

/*
 * SCL
 */

#define SAC_ND_WRITE__SCL(to_NT, to_pos) SAC_ND_A_FIELD (to_NT)

#define SAC_ND_READ__SCL(from_NT, from_pos) SAC_ND_A_FIELD (from_NT)

/*
 * AKS
 */

#define SAC_ND_WRITE__AKS(to_NT, to_pos)                                                 \
    SAC_TR_AA_PRINT ("write", to_NT, to_pos)                                             \
    SAC_BC_WRITE (to_NT, to_pos)                                                         \
    SAC_CS_WRITE_ARRAY (to_NT, to_pos)                                                   \
    SAC_ND_A_FIELD (to_NT)[to_pos]

#define SAC_ND_READ__AKS(from_NT, from_pos)                                              \
    (SAC_TR_AA_PRINT ("read", from_NT, from_pos) SAC_BC_READ (from_NT, from_pos)         \
       SAC_CS_READ_ARRAY (from_NT, from_pos) SAC_ND_A_FIELD (from_NT)[from_pos])

/*
 * AKD
 */

#define SAC_ND_WRITE__AKD(to_NT, to_pos) SAC_ND_WRITE__AKS (to_NT, to_pos)

#define SAC_ND_READ__AKD(from_NT, from_pos) SAC_ND_READ__AKS (from_NT, from_pos)

/*
 * AUD
 */

#define SAC_ND_WRITE__AUD(to_NT, to_pos) SAC_ND_WRITE__AKS (to_NT, to_pos)

#define SAC_ND_READ__AUD(from_NT, from_pos) SAC_ND_READ__AKS (from_NT, from_pos)

/******************************************************************************
 *
 * ICMs for types
 * =================
 *
 * ND_DESC_TYPE( var_NT)           : type of descriptor
 *
 * ND_TYPE_NT( basetype_NT)        : type implementation
 *     (basetype must be tagged)
 * ND_TYPE( var_NT, basetype)      : type implementation
 *     (basetype not tagged, separate tagged identifier at hand instead)
 * ND_TYPE__<CLASS>( basetype)     : type implementation
 *
 ******************************************************************************/

#define SAC_ND_DESC_TYPE(var_NT) CAT9 (SAC_ND_DESC_TYPE__, NT_SHP (var_NT) (var_NT))

#define SAC_ND_TYPE_NT(basetype_NT)                                                      \
    CAT9 (SAC_ND_TYPE__, NT_SHP (basetype_NT) (NT_NAME (basetype_NT)))

#define SAC_ND_TYPE(var_NT, basetype) CAT9 (SAC_ND_TYPE__, NT_SHP (var_NT) (basetype))

/*
 * SCL
 */

#define SAC_ND_DESC_TYPE__SCL(var_NT) SAC_ND_DESC_TYPE__AKS (var_NT)

#define SAC_ND_TYPE__SCL(basetype) basetype

/*
 * AKS
 */

#define SAC_ND_DESC_TYPE__AKS(var_NT) SAC_array_descriptor_t

#define SAC_ND_TYPE__AKS(basetype) basetype *

/*
 * AKD
 */

#define SAC_ND_DESC_TYPE__AKD(var_NT) SAC_ND_DESC_TYPE__AKS (var_NT)

#define SAC_ND_TYPE__AKD(basetype) SAC_ND_TYPE__AKS (basetype)

/*
 * AUD
 */

#define SAC_ND_DESC_TYPE__AUD(var_NT) SAC_ND_DESC_TYPE__AKS (var_NT)

#define SAC_ND_TYPE__AUD(basetype) SAC_ND_TYPE__AKS (basetype)

/******************************************************************************
 *
 * ICMs for typedefs
 * =================
 *
 * ND_TYPEDEF( var_NT, basetype) : type definition
 *
 ******************************************************************************/

#define SAC_ND_TYPEDEF(var_NT, basetype)                                                 \
    typedef SAC_ND_TYPE (var_NT, basetype) NT_NAME (var_NT);

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
 * ND_OBJDEF( var_NT, basetype, sdim, ...shp...) :
 *   declaration of an internal object
 * ND_OBJDEF_EXTERN( var_NT, basetype, sdim) :
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
 * ND_DECL( var_NT, basetype, sdim, ...shp...) :
 *   declares a data object
 * ND_DECL_EXTERN( var_NT, basetype, sdim) :
 *   declares an external data object
 *
 * ND_DECL__DESC( var_NT, decoration) :
 *   declares descriptor of a data object
 * ND_DECL__DATA( var_NT, basetype, decoration) :
 *   declares a data object (without mirror and descriptor)
 *
 * ND_DECL__MIRROR( var_NT, sdim, ...shp...) :
 *   declares mirror of a data object
 * ND_DECL__MIRROR_PARAM( var_NT, sdim, ...shp...) :
 *   declares mirror of a function parameter
 * ND_DECL__MIRROR_EXTERN( var_NT, sdim) :
 *   declares mirror of an external data object
 *
 ******************************************************************************/

/* ND_DECL( ...)  is a C-ICM */

/* ND_DECL_EXTERN( ...)  is a C-ICM */

#define SAC_ND_DECL__DESC(var_NT, decoration)                                            \
    CAT11 (SAC_ND_DECL__DESC__, NT_SHP (var_NT) BuildArgs2 (var_NT, decoration))

#define SAC_ND_DECL__DATA(var_NT, basetype, decoration)                                  \
    decoration SAC_ND_TYPE (var_NT, basetype) SAC_ND_A_FIELD (var_NT);

/* ND_DECL__MIRROR( ...)  is a C-ICM */

/* ND_DECL__MIRROR_PARAM( ...)  is a C-ICM */

/* ND_DECL__MIRROR_EXTERN( ...)  is a C-ICM */

/*
 * SCL
 */

#define SAC_ND_DECL__DESC__SCL(var_NT, decoration)                                       \
    CAT12 (SAC_ND_DECL__DESC__SCL_, NT_HID (var_NT) BuildArgs2 (var_NT, decoration))
#define SAC_ND_DECL__DESC__SCL_NHD(var_NT, decoration) SAC_NOTHING ()
#define SAC_ND_DECL__DESC__SCL_HID(var_NT, decoration)                                   \
    CAT13 (SAC_ND_DECL__DESC__SCL_HID_, NT_UNQ (var_NT) BuildArgs2 (var_NT, decoration))
#define SAC_ND_DECL__DESC__SCL_HID_NUQ(var_NT, decoration)                               \
    SAC_ND_DECL__DESC__AKS (var_NT, decoration)
#define SAC_ND_DECL__DESC__SCL_HID_UNQ(var_NT, decoration) SAC_NOTHING ()

/*
 * AKS
 */

#define SAC_ND_DECL__DESC__AKS(var_NT, decoration)                                       \
    decoration SAC_ND_DESC_TYPE (var_NT) SAC_ND_A_DESC (var_NT);

/*
 * AKD
 */

#define SAC_ND_DECL__DESC__AKD(var_NT, decoration)                                       \
    SAC_ND_DECL__DESC__AKS (var_NT, decoration)

/*
 * AUD
 */

#define SAC_ND_DECL__DESC__AUD(var_NT, decoration)                                       \
    SAC_ND_DECL__DESC__AKS (var_NT, decoration)

/******************************************************************************
 *
 * ICMs for descriptor initializing
 * ================================
 *
 * ND_SET__RC( var_NT, rc) :
 *   sets the refcount (in the descriptor) of a data object
 *
 * ND_SET__SHAPE_id( to_NT, to_sdim, shp_NT) :
 * ND_SET__SHAPE_arr( to_NT, dim, ...shp...) :
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
 * ND_PARAM_in( var_NT, basetype) :
 *   macro for prototyping data as "in" parameter
 *
 * ND_PARAM_in_nodesc( var_NT, basetype) :
 *   macro for prototyping data as "in" parameter without descriptor
 *
 * ND_PARAM_out( var_NT, basetype) :
 *   macro for prototyping data as "out" parameter
 *
 * ND_PARAM_out_nodesc( var_NT, basetype) :
 *   macro for prototyping data as "out" parameter without descriptor
 *
 * ND_PARAM_inout( var_NT, basetype) :
 *   macro for prototyping data as "inout" parameter
 *
 * ND_PARAM_inout_nodesc( var_NT, basetype) :
 *   macro for prototyping data as "inout" parameter without descriptor
 *
 * ND_PARAM_inout_nodesc_bx( var_NT, basetype) :
 *   macro for prototyping boxed data as "inout" parameter without descriptor
 *
 * ND_ARG_in( var_NT) :
 *   macro for giving data as "in" parameter
 *
 * ND_ARG_in_nodesc( var_NT) :
 *   macro for giving data as "in" parameter without descriptor
 *
 * ND_ARG_out( var_NT) :
 *   macro for getting data as "out" parameter
 *
 * ND_ARG_out_nodesc( var_NT) :
 *   macro for getting data as "out" parameter without descriptor
 *
 * ND_ARG_inout( var_NT) :
 *   macro for giving data as "inout" parameter
 *
 * ND_ARG_inout_nodesc( var_NT) :
 *   macro for giving data as "inout" argument without descriptor
 *
 * ND_ARG_inout_nodesc_bx( var_NT) :
 *   macro for giving boxed data as "inout" argument without descriptor
 *
 * ND_RET_out( retvar_NT, var_NT) :
 *   macro for returning data
 *
 * ND_RET_inout( retvar_NT, var_NT) :
 *   macro for returning "inout" data
 *
 * ND_DECL_PARAM_inout( var_NT, basetype) :
 *   macro for declaration of "inout" parameter
 *
 ******************************************************************************/

/* creates name for formal function parameter */
#define SAC_NAMEP(name) CAT0 (name, __p)

/* needed for T_dots parameters (ATG_notag) */
#define SAC_ND_PARAM_(dummy, basetype) basetype

#define SAC_ND_PARAM_in(var_NT, basetype)                                                \
    CAT11 (SAC_ND_PARAM_in__, NT_SHP (var_NT) BuildArgs2 (var_NT, basetype))

#define SAC_ND_PARAM_in_nodesc(var_NT, basetype)                                         \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    SAC_ND_A_FIELD (var_NT)

#define SAC_ND_PARAM_out(var_NT, basetype)                                               \
    CAT11 (SAC_ND_PARAM_out__, NT_SHP (var_NT) BuildArgs2 (var_NT, basetype))

#define SAC_ND_PARAM_out_nodesc(var_NT, basetype)                                        \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    *SAC_NAMEP (SAC_ND_A_FIELD (var_NT))

#define SAC_ND_PARAM_inout(var_NT, basetype) SAC_ND_PARAM_out (var_NT, basetype)

#define SAC_ND_PARAM_inout_nodesc(var_NT, basetype)                                      \
    SAC_ND_PARAM_out_nodesc (var_NT, basetype)

#define SAC_ND_PARAM_inout_nodesc_bx(var_NT, basetype)                                   \
    SAC_ND_PARAM_in_nodesc (var_NT, basetype)

#define SAC_ND_ARG_in(var_NT) CAT11 (SAC_ND_ARG_in__, NT_SHP (var_NT) (var_NT))

#define SAC_ND_ARG_in_nodesc(var_NT) SAC_ND_A_FIELD (var_NT)

#define SAC_ND_ARG_out(var_NT) CAT11 (SAC_ND_ARG_out__, NT_SHP (var_NT) (var_NT))

#define SAC_ND_ARG_out_nodesc(var_NT) &SAC_ND_A_FIELD (var_NT)

#define SAC_ND_ARG_inout(var_NT) SAC_ND_ARG_out (var_NT)

#define SAC_ND_ARG_inout_nodesc(var_NT) SAC_ND_ARG_out_nodesc (var_NT)

#define SAC_ND_ARG_inout_nodesc_bx(var_NT) SAC_ND_ARG_in_nodesc (var_NT)

#define SAC_ND_RET_out(retvar_NT, var_NT)                                                \
    CAT11 (SAC_ND_RET_out__, NT_SHP (var_NT) BuildArgs2 (retvar_NT, var_NT))

#define SAC_ND_RET_inout(retvar_NT, var_NT) SAC_ND_RET_out (retvar_NT, var_NT)

#define SAC_ND_DECL_PARAM_inout(var_NT, basetype)                                        \
    CAT11 (SAC_ND_DECL_PARAM_inout__, NT_SHP (var_NT) BuildArgs2 (var_NT, basetype))

/*
 * SCL
 */

#define SAC_ND_PARAM_in__SCL(var_NT, basetype)                                           \
    CAT12 (SAC_ND_PARAM_in__SCL_, NT_HID (var_NT) BuildArgs2 (var_NT, basetype))
#define SAC_ND_PARAM_in__SCL_NHD(var_NT, basetype)                                       \
    SAC_ND_PARAM_in_nodesc (var_NT, basetype)
#define SAC_ND_PARAM_in__SCL_HID(var_NT, basetype)                                       \
    CAT13 (SAC_ND_PARAM_in__SCL_HID_, NT_UNQ (var_NT) BuildArgs2 (var_NT, basetype))
#define SAC_ND_PARAM_in__SCL_HID_NUQ(var_NT, basetype)                                   \
    SAC_ND_PARAM_in__AKS (var_NT, basetype)
#define SAC_ND_PARAM_in__SCL_HID_UNQ(var_NT, basetype)                                   \
    SAC_ND_PARAM_in_nodesc (var_NT, basetype)

#define SAC_ND_PARAM_out__SCL(var_NT, basetype)                                          \
    CAT12 (SAC_ND_PARAM_out__SCL_, NT_HID (var_NT) BuildArgs2 (var_NT, basetype))
#define SAC_ND_PARAM_out__SCL_NHD(var_NT, basetype)                                      \
    SAC_ND_PARAM_out_nodesc (var_NT, basetype)
#define SAC_ND_PARAM_out__SCL_HID(var_NT, basetype)                                      \
    CAT13 (SAC_ND_PARAM_out__SCL_HID_, NT_UNQ (var_NT) BuildArgs2 (var_NT, basetype))
#define SAC_ND_PARAM_out__SCL_HID_NUQ(var_NT, basetype)                                  \
    SAC_ND_PARAM_out__AKS (var_NT, basetype)
#define SAC_ND_PARAM_out__SCL_HID_UNQ(var_NT, basetype)                                  \
    SAC_ND_PARAM_out_nodesc (var_NT, basetype)

#define SAC_ND_ARG_in__SCL(var_NT) CAT12 (SAC_ND_ARG_in__SCL_, NT_HID (var_NT) (var_NT))
#define SAC_ND_ARG_in__SCL_NHD(var_NT) SAC_ND_ARG_in_nodesc (var_NT)
#define SAC_ND_ARG_in__SCL_HID(var_NT)                                                   \
    CAT13 (SAC_ND_ARG_in__SCL_HID_, NT_UNQ (var_NT) (var_NT))
#define SAC_ND_ARG_in__SCL_HID_NUQ(var_NT) SAC_ND_ARG_in__AKS (var_NT)
#define SAC_ND_ARG_in__SCL_HID_UNQ(var_NT) SAC_ND_ARG_in_nodesc (var_NT)

#define SAC_ND_ARG_out__SCL(var_NT) CAT12 (SAC_ND_ARG_out__SCL_, NT_HID (var_NT) (var_NT))
#define SAC_ND_ARG_out__SCL_NHD(var_NT) SAC_ND_ARG_out_nodesc (var_NT)
#define SAC_ND_ARG_out__SCL_HID(var_NT)                                                  \
    CAT13 (SAC_ND_ARG_out__SCL_HID_, NT_UNQ (var_NT) (var_NT))
#define SAC_ND_ARG_out__SCL_HID_NUQ(var_NT) SAC_ND_ARG_out__AKS (var_NT)
#define SAC_ND_ARG_out__SCL_HID_UNQ(var_NT) SAC_ND_ARG_out_nodesc (var_NT)

#define SAC_ND_RET_out__SCL(retvar_NT, var_NT)                                           \
    CAT12 (SAC_ND_RET_out__SCL_, NT_HID (var_NT) BuildArgs2 (retvar_NT, var_NT))
#define SAC_ND_RET_out__SCL_NHD(retvar_NT, var_NT)                                       \
    {                                                                                    \
        *SAC_NAMEP (SAC_ND_A_FIELD (retvar_NT)) = SAC_ND_A_FIELD (var_NT);               \
    }
#define SAC_ND_RET_out__SCL_HID(retvar_NT, var_NT)                                       \
    CAT13 (SAC_ND_RET_out__SCL_HID_, NT_UNQ (var_NT) BuildArgs2 (retvar_NT, var_NT))
#define SAC_ND_RET_out__SCL_HID_NUQ(retvar_NT, var_NT)                                   \
    SAC_ND_RET_out__AKS (retvar_NT, var_NT)
#define SAC_ND_RET_out__SCL_HID_UNQ(retvar_NT, var_NT)                                   \
    {                                                                                    \
        *SAC_NAMEP (SAC_ND_A_FIELD (retvar_NT)) = SAC_ND_A_FIELD (var_NT);               \
    }

#define SAC_ND_DECL_PARAM_inout__SCL(var_NT, basetype)                                   \
    CAT12 (SAC_ND_DECL_PARAM_inout__SCL_, NT_HID (var_NT) BuildArgs2 (var_NT, basetype))
#define SAC_ND_DECL_PARAM_inout__SCL_NHD(var_NT, basetype)                               \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    SAC_ND_A_FIELD (var_NT) = *SAC_NAMEP (SAC_ND_A_FIELD (var_NT));
#define SAC_ND_DECL_PARAM_inout__SCL_HID(var_NT, basetype)                               \
    CAT13 (SAC_ND_DECL_PARAM_inout__SCL_HID_,                                            \
           NT_UNQ (var_NT) BuildArgs2 (var_NT, basetype))
#define SAC_ND_DECL_PARAM_inout__SCL_HID_NUQ(var_NT, basetype)                           \
    SAC_ND_DECL_PARAM_inout__AKS (var_NT, basetype)
#define SAC_ND_DECL_PARAM_inout__SCL_HID_UNQ(var_NT, basetype)                           \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    SAC_ND_A_FIELD (var_NT) = *SAC_NAMEP (SAC_ND_A_FIELD (var_NT));

/*
 * AKS
 */

#define SAC_ND_PARAM_in__AKS(var_NT, basetype)                                           \
    SAC_ND_PARAM_in_nodesc (var_NT, basetype),                                           \
      SAC_ND_DESC_TYPE (var_NT) SAC_ND_A_DESC (var_NT)

#define SAC_ND_PARAM_out__AKS(var_NT, basetype)                                          \
    SAC_ND_PARAM_out_nodesc (var_NT, basetype),                                          \
      SAC_ND_DESC_TYPE (var_NT) * SAC_NAMEP (SAC_ND_A_DESC (var_NT))

#define SAC_ND_ARG_in__AKS(var_NT) SAC_ND_ARG_in_nodesc (var_NT), SAC_ND_A_DESC (var_NT)

#define SAC_ND_ARG_out__AKS(var_NT)                                                      \
    SAC_ND_ARG_out_nodesc (var_NT), &SAC_ND_A_DESC (var_NT)

#define SAC_ND_RET_out__AKS(retvar_NT, var_NT)                                           \
    {                                                                                    \
        *SAC_NAMEP (SAC_ND_A_FIELD (retvar_NT)) = SAC_ND_A_FIELD (var_NT);               \
        *SAC_NAMEP (SAC_ND_A_DESC (retvar_NT)) = SAC_ND_A_DESC (var_NT);                 \
    }

#define SAC_ND_DECL_PARAM_inout__AKS(var_NT, basetype)                                   \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    SAC_ND_A_FIELD (var_NT) = *SAC_NAMEP (SAC_ND_A_FIELD (var_NT));                      \
    SAC_ND_DESC_TYPE (var_NT)                                                            \
    SAC_ND_A_DESC (var_NT) = *SAC_NAMEP (SAC_ND_A_DESC (var_NT));

/*
 * AKD
 */

#define SAC_ND_PARAM_in__AKD(var_NT, basetype) SAC_ND_PARAM_in__AKS (var_NT, basetype)

#define SAC_ND_PARAM_out__AKD(var_NT, basetype) SAC_ND_PARAM_out__AKS (var_NT, basetype)

#define SAC_ND_ARG_in__AKD(var_NT) SAC_ND_ARG_in__AKS (var_NT)

#define SAC_ND_ARG_out__AKD(var_NT) SAC_ND_ARG_out__AKS (var_NT)

#define SAC_ND_RET_out__AKD(retvar_NT, var_NT) SAC_ND_RET_out__AKS (retvar_NT, var_NT)

#define SAC_ND_DECL_PARAM_inout__AKD(var_NT, basetype)                                   \
    SAC_ND_DECL_PARAM_inout__AKS (var_NT, basetype)

/*
 * AUD
 */

#define SAC_ND_PARAM_in__AUD(var_NT, basetype) SAC_ND_PARAM_in__AKS (var_NT, basetype)

#define SAC_ND_PARAM_out__AUD(var_NT, basetype) SAC_ND_PARAM_out__AKS (var_NT, basetype)

#define SAC_ND_ARG_in__AUD(var_NT) SAC_ND_ARG_in__AKS (var_NT)

#define SAC_ND_ARG_out__AUD(var_NT) SAC_ND_ARG_out__AKS (var_NT)

#define SAC_ND_RET_out__AUD(retvar_NT, var_NT) SAC_ND_RET_out__AKS (retvar_NT, var_NT)

#define SAC_ND_DECL_PARAM_inout__AUD(var_NT, basetype)                                   \
    SAC_ND_DECL_PARAM_inout__AKS (var_NT, basetype)

/******************************************************************************
 *
 * ICMs for allocation of data objects
 * ===================================
 *
 * ND_ALLOC( var_NT, rc, dim, set_shape_icm) :
 *   allocates a data object (no initialization apart from descriptor!)
 *
 * ND_ALLOC__DESC( var_NT, dim) :
 *   allocates memory for descriptor (no initialization apart from dimension!)
 *
 * ND_ALLOC__DATA( var_NT) :
 *   allocates memory for a data object (without descriptor, no initialization)
 *
 * ND_ALLOC__DESC_AND_DATA( var_NT, dim) :
 *   allocates memory for descriptor and data object (no init. apart from dim.!)
 *   This ICM is needed for decriptor allocation optimization (DAO)
 *
 * ND_CHECK_REUSE( to_NT, to_sdim, from_NT, copyfun) :
 *   tries to reuse old data object for new, copies if impossible
 *
 ******************************************************************************/

/*
 * for the time being this ICM is not used :-( because 'set_shape_icm' is
 * usually a C-ICM but this macro support H-ICMs only:
 */
#define SAC_ND_ALLOC(var_NT, rc, dim, set_shape_icm)                                     \
    CAT10 (SAC_ND_ALLOC__, NT_SHP (var_NT) BuildArgs4 (var_NT, rc, dim, set_shape_icm))

/*
 * these two macros are used instead:
 */
#define SAC_ND_ALLOC_BEGIN(var_NT, rc, dim)                                              \
    CAT10 (SAC_ND_ALLOC_BEGIN__, NT_SHP (var_NT) BuildArgs3 (var_NT, rc, dim))
#define SAC_ND_ALLOC_END(var_NT, rc, dim)                                                \
    CAT10 (SAC_ND_ALLOC_END__, NT_SHP (var_NT) BuildArgs3 (var_NT, rc, dim))

#define SAC_ND_ALLOC__DESC(var_NT, dim)                                                  \
    CAT11 (SAC_ND_ALLOC__DESC__, NT_SHP (var_NT) BuildArgs2 (var_NT, dim))

#define SAC_ND_ALLOC__DATA(var_NT) CAT11 (SAC_ND_ALLOC__DATA__, NT_SHP (var_NT) (var_NT))

#define SAC_ND_ALLOC__DESC_AND_DATA(var_NT, dim)                                         \
    CAT11 (SAC_ND_ALLOC__DESC_AND_DATA__, NT_SHP (var_NT) BuildArgs2 (var_NT, dim))

/* ND_CHECK_REUSE( ...)  is a C-ICM */

/*
 * SCL
 */

/* DAO: descriptor and data vector are allocated together */
#define SAC_ND_ALLOC__SCL(var_NT, rc, dim, set_shape_icm)                                \
    {                                                                                    \
        SAC_ND_ALLOC__DESC_AND_DATA (var_NT, dim)                                        \
        SAC_ND_SET__RC (var_NT, rc)                                                      \
        set_shape_icm                                                                    \
    }

#define SAC_ND_ALLOC_BEGIN__SCL(var_NT, rc, dim)                                         \
    {                                                                                    \
        SAC_ND_ALLOC__DESC_AND_DATA (var_NT, dim)                                        \
        SAC_ND_SET__RC (var_NT, rc)
#define SAC_ND_ALLOC_END__SCL(var_NT, rc, dim) }

#define SAC_ND_ALLOC__DESC__SCL(var_NT, dim)                                             \
    CAT12 (SAC_ND_ALLOC__DESC__SCL_, NT_HID (var_NT) BuildArgs2 (var_NT, dim))
#define SAC_ND_ALLOC__DESC__SCL_NHD(var_NT, dim) SAC_NOOP ()
#define SAC_ND_ALLOC__DESC__SCL_HID(var_NT, dim)                                         \
    CAT13 (SAC_ND_ALLOC__DESC__SCL_HID_, NT_UNQ (var_NT) BuildArgs2 (var_NT, dim))
#define SAC_ND_ALLOC__DESC__SCL_HID_NUQ(var_NT, dim) SAC_ND_ALLOC__DESC__AKS (var_NT, dim)
#define SAC_ND_ALLOC__DESC__SCL_HID_UNQ(var_NT, dim) SAC_NOOP ()

#define SAC_ND_ALLOC__DATA__SCL(var_NT) SAC_NOOP ()

#define SAC_ND_ALLOC__DESC_AND_DATA__SCL(var_NT, dim)                                    \
    SAC_ND_ALLOC__DESC__SCL (var_NT, dim)

/*
 * AKS
 */

#define SAC_ND_ALLOC__AKS(var_NT, rc, dim, set_shape_icm)                                \
    SAC_ND_ALLOC__SCL (var_NT, rc, dim, set_shape_icm)

#define SAC_ND_ALLOC_BEGIN__AKS(var_NT, rc, dim) SAC_ND_ALLOC_BEGIN__SCL (var_NT, rc, dim)
#define SAC_ND_ALLOC_END__AKS(var_NT, rc, dim) SAC_ND_ALLOC_END__SCL (var_NT, rc, dim)

#define SAC_ND_ALLOC__DESC__AKS(var_NT, dim)                                             \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim == SAC_ND_A_MIRROR_DIM (var_NT)),                          \
                         ("Inconsistant dimension for array %s found!",                  \
                          NT_STR (var_NT)));                                             \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_DESC (var_NT),                                \
                                  BYTE_SIZE_OF_DESC (SAC_ND_A_MIRROR_DIM (var_NT)))      \
        SAC_TR_MEM_PRINT (("ND_ALLOC__DESC( %s, %s) at addr: %p", NT_STR (var_NT), #dim, \
                           SAC_ND_A_DESC (var_NT)))                                      \
    }

#define SAC_ND_ALLOC__DATA__AKS(var_NT)                                                  \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_FIELD (var_NT),                               \
                                  SAC_ND_A_SIZE (var_NT)                                 \
                                    * sizeof (*SAC_ND_A_FIELD (var_NT)))                 \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_ALLOC__DATA( %s) at addr: %p", NT_STR (var_NT), SAC_ND_A_FIELD (var_NT))) \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_CS_REGISTER_ARRAY (var_NT)                                                   \
    }

#define SAC_ND_ALLOC__DESC_AND_DATA__AKS(var_NT, dim)                                    \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim == SAC_ND_A_MIRROR_DIM (var_NT)),                          \
                         ("Inconsistant dimension for array %s found!",                  \
                          NT_STR (var_NT)));                                             \
        SAC_HM_MALLOC_FIXED_SIZE_WITH_DESC (SAC_ND_A_FIELD (var_NT),                     \
                                            SAC_ND_A_DESC (var_NT),                      \
                                            SAC_ND_A_MIRROR_SIZE (var_NT)                \
                                              * sizeof (*SAC_ND_A_FIELD (var_NT)),       \
                                            SAC_ND_A_MIRROR_DIM (var_NT))                \
        SAC_TR_MEM_PRINT (("ND_ALLOC__DESC( %s, %s) at addr: %p", NT_STR (var_NT), #dim, \
                           SAC_ND_A_DESC (var_NT)))                                      \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_ALLOC__DATA( %s) at addr: %p", NT_STR (var_NT), SAC_ND_A_FIELD (var_NT))) \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_CS_REGISTER_ARRAY (var_NT)                                                   \
    }

/*
 * AKD
 */

/* DAO is possible for SCL and AKS only!                                    */
/* In order to allocate the data vector, its size / the shape of the array  */
/* is needed. If the shape is not statically, known it must be computed and */
/* stored in the decriptor first!                                           */
#define SAC_ND_ALLOC__AKD(var_NT, rc, dim, set_shape_icm)                                \
    {                                                                                    \
        SAC_ND_ALLOC__DESC (var_NT, dim)                                                 \
        SAC_ND_SET__RC (var_NT, rc)                                                      \
        set_shape_icm SAC_ND_ALLOC__DATA (var_NT)                                        \
    }

#define SAC_ND_ALLOC_BEGIN__AKD(var_NT, rc, dim)                                         \
    {                                                                                    \
        SAC_ND_ALLOC__DESC (var_NT, dim)                                                 \
        SAC_ND_SET__RC (var_NT, rc)
#define SAC_ND_ALLOC_END__AKD(var_NT, rc, dim)                                           \
    SAC_ND_ALLOC__DATA (var_NT)                                                          \
    }

#define SAC_ND_ALLOC__DESC__AKD(var_NT, dim) SAC_ND_ALLOC__DESC__AKS (var_NT, dim)

#define SAC_ND_ALLOC__DATA__AKD(var_NT)                                                  \
    {                                                                                    \
        SAC_HM_MALLOC (SAC_ND_A_FIELD (var_NT),                                          \
                       SAC_ND_A_SIZE (var_NT) * sizeof (*SAC_ND_A_FIELD (var_NT)))       \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_ALLOC__DATA( %s) at addr: %p", NT_STR (var_NT), SAC_ND_A_FIELD (var_NT))) \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_CS_REGISTER_ARRAY (var_NT)                                                   \
    }

#define SAC_ND_ALLOC__DESC_AND_DATA__AKD(var_NT, dim) SAC_ICM_UNDEF ();

/*
 * AUD
 */

#define SAC_ND_ALLOC__AUD(var_NT, rc, dim, set_shape_icm)                                \
    SAC_ND_ALLOC__AKD (var_NT, rc, dim, set_shape_icm)

#define SAC_ND_ALLOC_BEGIN__AUD(var_NT, rc, dim) SAC_ND_ALLOC_BEGIN__AKD (var_NT, rc, dim)
#define SAC_ND_ALLOC_END__AUD(var_NT, rc, dim) SAC_ND_ALLOC_END__AKD (var_NT, rc, dim)

#define SAC_ND_ALLOC__DESC__AUD(var_NT, dim)                                             \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim >= 0),                                                     \
                         ("Illegal dimension for array %s found!", NT_STR (var_NT)));    \
        SAC_HM_MALLOC (SAC_ND_A_DESC (var_NT), BYTE_SIZE_OF_DESC (dim))                  \
        SAC_TR_MEM_PRINT (("ND_ALLOC__DESC( %s, %s) at addr: %p", NT_STR (var_NT), #dim, \
                           SAC_ND_A_DESC (var_NT)))                                      \
        SAC_ND_A_DESC_DIM (var_NT) = SAC_ND_A_MIRROR_DIM (var_NT) = dim;                 \
    }

#define SAC_ND_ALLOC__DATA__AUD(var_NT) SAC_ND_ALLOC__DATA__AKD (var_NT)

#define SAC_ND_ALLOC__DESC_AND_DATA__AUD(var_NT, dim) SAC_ICM_UNDEF ();

/******************************************************************************
 *
 * ICMs for freeing data objects
 * =============================
 *
 * ND_FREE( var_NT, freefun) :
 *   frees a data object
 *
 * ND_FREE__DESC( var_NT) :
 *   frees memory for descriptor
 *
 * ND_FREE__DATA( var_NT, freefun) :
 *   frees memory for a data object (without descriptor)
 *
 ******************************************************************************/

#define SAC_ND_FREE(var_NT, freefun)                                                     \
    {                                                                                    \
        SAC_ND_FREE__DATA (var_NT, freefun)                                              \
        SAC_ND_FREE__DESC (var_NT)                                                       \
    }

#define SAC_ND_FREE__DESC(var_NT) CAT11 (SAC_ND_FREE__DESC__, NT_SHP (var_NT) (var_NT))

#define SAC_ND_FREE__DATA(var_NT, freefun)                                               \
    CAT11 (SAC_ND_FREE__DATA__,                                                          \
           CAT11 (NT_SHP (var_NT),                                                       \
                  CAT11 (_, NT_HID (var_NT) BuildArgs2 (var_NT, freefun))))

/*
 * SCL
 */

#define SAC_ND_FREE__DESC__SCL(var_NT)                                                   \
    CAT12 (SAC_ND_FREE__DESC__SCL_, NT_HID (var_NT) (var_NT))

#define SAC_ND_FREE__DESC__SCL_NHD(var_NT) SAC_NOOP ()

#define SAC_ND_FREE__DESC__SCL_HID(var_NT)                                               \
    CAT13 (SAC_ND_FREE__DESC__SCL_HID_, NT_UNQ (var_NT) (var_NT))

#define SAC_ND_FREE__DESC__SCL_HID_NUQ(var_NT) SAC_ND_FREE__DESC__AKS (var_NT)

#define SAC_ND_FREE__DESC__SCL_HID_UNQ(var_NT) SAC_NOOP ()

#define SAC_ND_FREE__DATA__SCL_NHD(var_NT, freefun) SAC_NOOP ()

#define SAC_ND_FREE__DATA__SCL_HID(var_NT, freefun)                                      \
    {                                                                                    \
        SAC_TR_MEM_PRINT (("ND_FREE__DATA( %s, %s) at addr: %p", NT_STR (var_NT),        \
                           #freefun, SAC_ND_A_FIELD (var_NT)))                           \
        freefun (SAC_ND_A_FIELD (var_NT));                                               \
        SAC_TR_DEC_HIDDEN_MEMCNT (1)                                                     \
    }

/*
 * AKS
 */

#define SAC_ND_FREE__DESC__AKS(var_NT)                                                   \
    {                                                                                    \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_FREE__DESC( %s) at addr: %p", NT_STR (var_NT), SAC_ND_A_DESC (var_NT)))   \
        SAC_HM_FREE_DESC (SAC_ND_A_DESC (var_NT))                                        \
    }

#define SAC_ND_FREE__DATA__AKS_NHD(var_NT, freefun)                                      \
    {                                                                                    \
        SAC_TR_MEM_PRINT (("ND_FREE__DATA( %s, %s) at addr: %p", NT_STR (var_NT),        \
                           #freefun, SAC_ND_A_FIELD (var_NT)))                           \
        SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_FIELD (var_NT),                                 \
                                SAC_ND_A_SIZE (var_NT)                                   \
                                  * sizeof (*SAC_ND_A_FIELD (var_NT)))                   \
        SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_CS_UNREGISTER_ARRAY (var_NT)                                                 \
    }

#define SAC_ND_FREE__DATA__AKS_HID(var_NT, freefun)                                      \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (var_NT); SAC_i++) {                       \
            freefun (SAC_ND_READ (var_NT, SAC_i));                                       \
        }                                                                                \
        SAC_ND_FREE__DATA__AKS_NHD (var_NT, freefun)                                     \
    }

/*
 * AKD
 */

#define SAC_ND_FREE__DESC__AKD(var_NT) SAC_ND_FREE__DESC__AKS (var_NT)

#define SAC_ND_FREE__DATA__AKD_NHD(var_NT, freefun)                                      \
    {                                                                                    \
        SAC_TR_MEM_PRINT (("ND_FREE__DATA( %s, %s) at addr: %p", NT_STR (var_NT),        \
                           #freefun, SAC_ND_A_FIELD (var_NT)))                           \
        SAC_HM_FREE (SAC_ND_A_FIELD (var_NT))                                            \
        SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_CS_UNREGISTER_ARRAY (var_NT)                                                 \
    }

#define SAC_ND_FREE__DATA__AKD_HID(var_NT, freefun)                                      \
    SAC_ND_FREE__DATA__AKS_HID (var_NT, freefun)

/*
 * AUD
 */

#define SAC_ND_FREE__DESC__AUD(var_NT) SAC_ND_FREE__DESC__AKD (var_NT)

#define SAC_ND_FREE__DATA__AUD_NHD(var_NT, freefun)                                      \
    SAC_ND_FREE__DATA__AKD_NHD (var_NT, freefun)

#define SAC_ND_FREE__DATA__AUD_HID(var_NT, freefun)                                      \
    SAC_ND_FREE__DATA__AKS_HID (var_NT, freefun)

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
 * ND_ASSIGN( to_NT, to_sdim, from_NT, from_sdim, copyfun) :
 *   assigns a data object to another one
 * ND_ASSIGN__DESC( to_NT, from_NT) :
 *   assigns a descriptor to another one
 * ND_ASSIGN__SHAPE( to_NT, to_sdim, from_NT, from_sdim) :
 *   assigns a shape information (descriptor *and* mirror) to another one
 * ND_ASSIGN__DATA( to_NT, to_sdim, from_NT, from_sdim, copyfun) :
 *   assigns a data object to another one (without mirror)
 *
 * ND_COPY( to_NT, to_sdim, from_NT, from_sdim, copyfun) :
 *   copies a data object to another one
 * ND_COPY__SHAPE( to_NT, to_sdim, from_NT, from_sdim) :
 *   copies a shape information (descriptor *and* mirror) to another one
 * ND_COPY__DATA( to_NT, from_NT, copyfun) :
 *   copies a data object to another one (without mirror)
 *
 * ND_MAKE_UNIQUE( to_NT, to_sdim, from_NT, copyfun) :
 *   assigns a data object to another one iff RC is zero, copies it otherwise.
 *
 ******************************************************************************/

/* ND_ASSIGN( ...)  is a C-ICM */

/* ND_ASSIGN__DESC( ...)  is a C-ICM */

/* ND_ASSIGN__SHAPE( ...)  is a C-ICM */

#define SAC_ND_ASSIGN__DATA(to_NT, from_NT, copyfun)                                     \
    CAT14 (SAC_ND_ASSIGN__DATA__,                                                        \
           CAT14 (NT_SHP (to_NT),                                                        \
                  CAT14 (__, NT_SHP (from_NT) BuildArgs3 (to_NT, from_NT, copyfun))))

/* ND_COPY( ...)  is a C-ICM */

/* ND_COPY__SHAPE( ...)  is a C-ICM */

#define SAC_ND_COPY__DATA(to_NT, from_NT, copyfun)                                       \
    CAT14 (SAC_ND_COPY__DATA__,                                                          \
           CAT14 (NT_SHP (to_NT),                                                        \
                  CAT14 (__, NT_SHP (from_NT) BuildArgs3 (to_NT, from_NT, copyfun))))

/* ND_MAKE_UNIQUE( ...)  is a C-ICM */

/*
 * SCL
 */

#define SAC_ND_ASSIGN__DATA__SCL__SCL(to_NT, from_NT, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_NT, from_NT, copyfun)
#define SAC_ND_ASSIGN__DATA__SCL__AKS(to_NT, from_NT, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__SCL__AKD(to_NT, from_NT, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__SCL__AUD(to_NT, from_NT, copyfun)                           \
    CAT15 (SAC_ND_ASSIGN__DATA__SCL_,                                                    \
           CAT15 (NT_HID (to_NT),                                                        \
                  CAT15 (__AUD_,                                                         \
                         NT_HID (from_NT) BuildArgs3 (to_NT, from_NT, copyfun))))
#define SAC_ND_ASSIGN__DATA__SCL_NHD__AUD_NHD(to_NT, from_NT, copyfun)                   \
    {                                                                                    \
        SAC_ND_WRITE_READ_COPY (to_NT, 0, from_NT, 0, copyfun)                           \
        SAC_ND_DEC_RC_FREE (from_NT, 1, )                                                \
    }
#define SAC_ND_ASSIGN__DATA__SCL_HID__AUD_HID(to_NT, from_NT, copyfun)                   \
    CAT16 (SAC_ND_ASSIGN__DATA__SCL_HID_,                                                \
           CAT16 (NT_UNQ (to_NT),                                                        \
                  CAT16 (__AUD_HID_,                                                     \
                         NT_UNQ (from_NT) BuildArgs3 (to_NT, from_NT, copyfun))))
#define SAC_ND_ASSIGN__DATA__SCL_HID_NUQ__AUD_HID_NUQ(to_NT, from_NT, copyfun)           \
    SAC_ND_ASSIGN__DATA__SCL_NHD__AUD_NHD (to_NT, from_NT, copyfun)
#define SAC_ND_ASSIGN__DATA__SCL_HID_NUQ__AUD_HID_UNQ(to_NT, from_NT, copyfun)           \
    {                                                                                    \
        SAC_ND_WRITE_READ (to_NT, 0, from_NT, 0)                                         \
        /* free data vector but keep its content (i.e. the hidden object itself)! */     \
        SAC_ND_FREE__DATA__AUD_NHD (from_NT, freefun)                                    \
    }
#define SAC_ND_ASSIGN__DATA__SCL_HID_UNQ__AUD_HID_NUQ(to_NT, from_NT, copyfun)           \
    SAC_ND_ASSIGN__DATA__SCL_HID_NUQ__AUD_HID_UNQ (to_NT, from_NT, copyfun)
#define SAC_ND_ASSIGN__DATA__SCL_HID_UNQ__AUD_HID_UNQ(to_NT, from_NT, copyfun)           \
    SAC_ND_ASSIGN__DATA__SCL_HID_NUQ__AUD_HID_UNQ (to_NT, from_NT, copyfun)

#define SAC_ND_COPY__DATA__SCL__SCL(to_NT, from_NT, copyfun)                             \
    {                                                                                    \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_COPY__DATA( %s, %s, %s)" NT_STR (to_NT), #from_NT, #copyfun))             \
        SAC_ND_WRITE_READ_COPY (to_NT, 0, from_NT, 0, copyfun)                           \
    }
#define SAC_ND_COPY__DATA__SCL__AKS(to_NT, from_NT, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__SCL__AKD(to_NT, from_NT, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__SCL__AUD(to_NT, from_NT, copyfun)                             \
    SAC_ND_COPY__DATA__SCL__SCL (to_NT, from_NT, copyfun)

/*
 * AKS
 */

#define SAC_ND_ASSIGN__DATA__AKS__SCL(to_NT, from_NT, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__AKS__AKS(to_NT, from_NT, copyfun)                           \
    {                                                                                    \
        SAC_ND_A_FIELD (to_NT) = SAC_ND_A_FIELD (from_NT);                               \
    }
#define SAC_ND_ASSIGN__DATA__AKS__AKD(to_NT, from_NT, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_NT, from_NT, copyfun)
#define SAC_ND_ASSIGN__DATA__AKS__AUD(to_NT, from_NT, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_NT, from_NT, copyfun)

#define SAC_ND_COPY__DATA__AKS__SCL(to_NT, from_NT, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__AKS__AKS(to_NT, from_NT, copyfun)                             \
    {                                                                                    \
        int SAC_i;                                                                       \
        SAC_TR_MEM_PRINT (("ND_COPY__DATA( %s, %s, %s) at addr: %p", NT_STR (from_NT),   \
                           #to_NT, #copyfun, SAC_ND_A_FIELD (to_NT)))                    \
        SAC_ASSURE_TYPE ((SAC_ND_A_SIZE (to_NT) == SAC_ND_A_SIZE (from_NT)),             \
                         ("Assignment with incompatible types found!"));                 \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (from_NT); SAC_i++) {                      \
            SAC_ND_WRITE_READ_COPY (to_NT, SAC_i, from_NT, SAC_i, copyfun)               \
        }                                                                                \
    }
#define SAC_ND_COPY__DATA__AKS__AKD(to_NT, from_NT, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_NT, from_NT, copyfun)
#define SAC_ND_COPY__DATA__AKS__AUD(to_NT, from_NT, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_NT, from_NT, copyfun)

/*
 * AKD
 */

#define SAC_ND_ASSIGN__DATA__AKD__SCL(to_NT, from_NT, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_ASSIGN__DATA__AKD__AKS(to_NT, from_NT, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_NT, from_NT, copyfun)
#define SAC_ND_ASSIGN__DATA__AKD__AKD(to_NT, from_NT, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_NT, from_NT, copyfun)
#define SAC_ND_ASSIGN__DATA__AKD__AUD(to_NT, from_NT, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_NT, from_NT, copyfun)

#define SAC_ND_COPY__DATA__AKD__SCL(to_NT, from_NT, copyfun) SAC_ICM_UNDEF ();
#define SAC_ND_COPY__DATA__AKD__AKS(to_NT, from_NT, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_NT, from_NT, copyfun)
#define SAC_ND_COPY__DATA__AKD__AKD(to_NT, from_NT, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_NT, from_NT, copyfun)
#define SAC_ND_COPY__DATA__AKD__AUD(to_NT, from_NT, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_NT, from_NT, copyfun)

/*
 * AUD
 */

#define SAC_ND_ASSIGN__DATA__AUD__SCL(to_NT, from_NT, copyfun)                           \
    {                                                                                    \
        SAC_ND_ALLOC__DATA (to_NT)                                                       \
        CAT15 (SAC_ND_ASSIGN__DATA__AUD_,                                                \
               CAT15 (NT_HID (to_NT),                                                    \
                      CAT15 (__SCL_, CAT15 (NT_HID (from_NT),                            \
                                            CAT15 (_, NT_UNQ (from_NT)                   \
                                                        BuildArgs3 (to_NT, from_NT,      \
                                                                    copyfun))))))        \
    }
#define SAC_ND_ASSIGN__DATA__AUD_NHD__SCL_NHD_NUQ(to_NT, from_NT, copyfun)               \
    {                                                                                    \
        SAC_ND_WRITE_READ_COPY (to_NT, 0, from_NT, 0, copyfun)                           \
    }
#define SAC_ND_ASSIGN__DATA__AUD_NHD__SCL_NHD_UNQ(to_NT, from_NT, copyfun)               \
    SAC_ND_ASSIGN__DATA__AUD_NHD__SCL_NHD_NUQ (to_NT, from_NT, copyfun)
#define SAC_ND_ASSIGN__DATA__AUD_HID__SCL_HID_NUQ(to_NT, from_NT, copyfun)               \
    SAC_ND_ASSIGN__DATA__SCL_NHD__AUD_NHD (to_NT, from_NT, copyfun)
#define SAC_ND_ASSIGN__DATA__AUD_HID__SCL_HID_UNQ(to_NT, from_NT, copyfun)               \
    {                                                                                    \
        SAC_ND_WRITE_READ (to_NT, 0, from_NT, 0);                                        \
    }

#define SAC_ND_ASSIGN__DATA__AUD__AKS(to_NT, from_NT, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_NT, from_NT, copyfun)
#define SAC_ND_ASSIGN__DATA__AUD__AKD(to_NT, from_NT, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_NT, from_NT, copyfun)
#define SAC_ND_ASSIGN__DATA__AUD__AUD(to_NT, from_NT, copyfun)                           \
    SAC_ND_ASSIGN__DATA__AKS__AKS (to_NT, from_NT, copyfun)

#define SAC_ND_COPY__DATA__AUD__SCL(to_NT, from_NT, copyfun)                             \
    {                                                                                    \
        SAC_ND_WRITE_READ_COPY (to_NT, 0, from_NT, 0, copyfun)                           \
    }
#define SAC_ND_COPY__DATA__AUD__AKS(to_NT, from_NT, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_NT, from_NT, copyfun)
#define SAC_ND_COPY__DATA__AUD__AKD(to_NT, from_NT, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_NT, from_NT, copyfun)
#define SAC_ND_COPY__DATA__AUD__AUD(to_NT, from_NT, copyfun)                             \
    SAC_ND_COPY__DATA__AKS__AKS (to_NT, from_NT, copyfun)

/******************************************************************************
 *
 * ICMs for creating refcounted objects:
 * =====================================
 *
 * ND_CREATE__SCALAR__DATA( var_NT, val) :
 *   sets data of a constant scalar
 *
 * ND_CREATE__STRING__DATA( var_NT, str) :
 *   sets data of a constant character array (string)
 *
 * ND_CREATE__ARRAY__SHAPE( var_NT, sdim, val_size, ...val...) :
 *   sets shape of a constant non-scalar array
 * ND_CREATE__ARRAY__DATA( var_NT, sdim, val_size, ...val..., copyfun) :
 *   sets data of a constant non-scalar array
 *
 ******************************************************************************/

#define SAC_ND_CREATE__SCALAR__DATA(var_NT, val)                                         \
    {                                                                                    \
        SAC_ND_WRITE_COPY (var_NT, 0, val, );                                            \
    }

#define SAC_ND_CREATE__STRING__DATA(var_NT, str)                                         \
    {                                                                                    \
        SAC_String2Array (SAC_ND_A_FIELD (var_NT), str);                                 \
    }

/* ND_CREATE__ARRAY__DIM( ...) is a C-ICM */
/* ND_CREATE__ARRAY__SHAPE( ...) is a C-ICM */
/* ND_CREATE__ARRAY__DATA( ...) is a C-ICM */

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
 * ND_SET__RC( var_NT, rc) :
 *   sets the refcount (in the descriptor) of a data object
 * ND_INC_RC( var_NT, rc) :
 *   increments the refcount of a data object
 * ND_DEC_RC( var_NT, rc) :
 *   decrements the refcount of a data object
 * ND_DEC_RC_FREE( var_NT, rc, freefun) :
 *   decrements the refcount and frees the data object if refcount becomes 0
 *
 ******************************************************************************/

#define SAC_ND_SET__RC(var_NT, rc)                                                       \
    CAT17 (SAC_ND_SET__RC__, NT_UNQ (var_NT) BuildArgs2 (var_NT, rc))

#define SAC_ND_INC_RC(var_NT, rc)                                                        \
    CAT17 (SAC_ND_INC_RC__, NT_UNQ (var_NT) BuildArgs2 (var_NT, rc))

#define SAC_ND_DEC_RC(var_NT, rc)                                                        \
    CAT17 (SAC_ND_DEC_RC__, NT_UNQ (var_NT) BuildArgs2 (var_NT, rc))

#define SAC_ND_DEC_RC_FREE(var_NT, rc, freefun)                                          \
    CAT17 (SAC_ND_DEC_RC_FREE__, NT_UNQ (var_NT) BuildArgs3 (var_NT, rc, freefun))

/*
 * NUQ
 */

#define SAC_ND_SET__RC__NUQ(var_NT, rc)                                                  \
    CAT18 (SAC_ND_SET__RC__, CAT18 (NT_SHP (var_NT), _NUQ BuildArgs2 (var_NT, rc)))

#define SAC_ND_INC_RC__NUQ(var_NT, rc)                                                   \
    CAT18 (SAC_ND_INC_RC__, CAT18 (NT_SHP (var_NT), _NUQ BuildArgs2 (var_NT, rc)))

#define SAC_ND_DEC_RC__NUQ(var_NT, rc)                                                   \
    CAT18 (SAC_ND_DEC_RC__, CAT18 (NT_SHP (var_NT), _NUQ BuildArgs2 (var_NT, rc)))

#define SAC_ND_DEC_RC_FREE__NUQ(var_NT, rc, freefun)                                     \
    CAT18 (SAC_ND_DEC_RC_FREE__,                                                         \
           CAT18 (NT_SHP (var_NT), _NUQ BuildArgs3 (var_NT, rc, freefun)))

/*
 * UNQ
 */

#define SAC_ND_SET__RC__UNQ(var_NT, rc)                                                  \
    SAC_NOOP () /* (rc != 1)  -->  uniqueness violation!!! */

#define SAC_ND_INC_RC__UNQ(var_NT, rc)                                                   \
    SAC_NOOP () /* Most likely a uniqueness violation!!! */

#define SAC_ND_DEC_RC__UNQ(var_NT, rc)                                                   \
    SAC_NOOP () /* Most likely a uniqueness violation!!! */

/* 'nt' is unique -> 'nt' has been consumed -> free 'nt' */
#define SAC_ND_DEC_RC_FREE__UNQ(var_NT, rc, freefun) SAC_ND_FREE (var_NT, freefun)

/*
 * SCL, NUQ
 */

#define SAC_ND_SET__RC__SCL_NUQ(var_NT, rc)                                              \
    CAT19 (SAC_ND_SET__RC__SCL_, CAT19 (NT_HID (var_NT), _NUQ BuildArgs2 (var_NT, rc)))
#define SAC_ND_SET__RC__SCL_NHD_NUQ(var_NT, rc) SAC_NOOP ()
#define SAC_ND_SET__RC__SCL_HID_NUQ(var_NT, rc) SAC_ND_SET__RC__AKS_NUQ (var_NT, rc)

#define SAC_ND_INC_RC__SCL_NUQ(var_NT, rc)                                               \
    CAT19 (SAC_ND_INC_RC__SCL_, CAT19 (NT_HID (var_NT), _NUQ BuildArgs2 (var_NT, rc)))
#define SAC_ND_INC_RC__SCL_NHD_NUQ(var_NT, rc) SAC_NOOP ()
#define SAC_ND_INC_RC__SCL_HID_NUQ(var_NT, rc) SAC_ND_INC_RC__AKS_NUQ (var_NT, rc)

#define SAC_ND_DEC_RC__SCL_NUQ(var_NT, rc)                                               \
    CAT19 (SAC_ND_DEC_RC__SCL_, CAT19 (NT_HID (var_NT), _NUQ BuildArgs2 (var_NT, rc)))
#define SAC_ND_DEC_RC__SCL_NHD_NUQ(var_NT, rc) SAC_NOOP ()
#define SAC_ND_DEC_RC__SCL_HID_NUQ(var_NT, rc) SAC_ND_DEC_RC__AKS_NUQ (var_NT, rc)

#define SAC_ND_DEC_RC_FREE__SCL_NUQ(var_NT, rc, freefun)                                 \
    CAT19 (SAC_ND_DEC_RC_FREE__SCL_,                                                     \
           CAT19 (NT_HID (var_NT), _NUQ BuildArgs3 (var_NT, rc, freefun)))
#define SAC_ND_DEC_RC_FREE__SCL_NHD_NUQ(var_NT, rc, freefun) SAC_NOOP ()
#define SAC_ND_DEC_RC_FREE__SCL_HID_NUQ(var_NT, rc, freefun)                             \
    SAC_ND_DEC_RC_FREE__AKS_NUQ (var_NT, rc, freefun)

/*
 * AKS, NUQ
 */

#define SAC_ND_SET__RC__AKS_NUQ(var_NT, rc)                                              \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_SET__RC( %s, %d)", NT_STR (var_NT), rc))                  \
        SAC_ND_A_RC (var_NT) = rc;                                                       \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

#define SAC_ND_INC_RC__AKS_NUQ(var_NT, rc)                                               \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_INC_RC( %s, %d)", NT_STR (var_NT), rc))                   \
        SAC_ND_A_RC (var_NT) += rc;                                                      \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

#define SAC_ND_DEC_RC__AKS_NUQ(var_NT, rc)                                               \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_DEC_RC( %s, %d)", NT_STR (var_NT), rc))                   \
        SAC_ND_A_RC (var_NT) -= rc;                                                      \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

#define SAC_ND_DEC_RC_FREE__AKS_NUQ(var_NT, rc, freefun)                                 \
    {                                                                                    \
        SAC_TR_REF_PRINT (                                                               \
          ("ND_DEC_RC_FREE( %s, %d, %s)", NT_STR (var_NT), rc, #freefun))                \
        if ((SAC_ND_A_RC (var_NT) -= rc) == 0) {                                         \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
            SAC_ND_FREE (var_NT, freefun)                                                \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
        }                                                                                \
    }

/*
 * AKD, NUQ
 */

#define SAC_ND_SET__RC__AKD_NUQ(var_NT, rc) SAC_ND_SET__RC__AKS_NUQ (var_NT, rc)

#define SAC_ND_INC_RC__AKD_NUQ(var_NT, rc) SAC_ND_INC_RC__AKS_NUQ (var_NT, rc)

#define SAC_ND_DEC_RC__AKD_NUQ(var_NT, rc) SAC_ND_DEC_RC__AKS_NUQ (var_NT, rc)

#define SAC_ND_DEC_RC_FREE__AKD_NUQ(var_NT, rc, freefun)                                 \
    SAC_ND_DEC_RC_FREE__AKS_NUQ (var_NT, rc, freefun)

/*
 * AUD, NUQ
 */

#define SAC_ND_SET__RC__AUD_NUQ(var_NT, rc) SAC_ND_SET__RC__AKS_NUQ (var_NT, rc)

#define SAC_ND_INC_RC__AUD_NUQ(var_NT, rc) SAC_ND_INC_RC__AKS_NUQ (var_NT, rc)

#define SAC_ND_DEC_RC__AUD_NUQ(var_NT, rc) SAC_ND_DEC_RC__AKS_NUQ (var_NT, rc)

#define SAC_ND_DEC_RC_FREE__AUD_NUQ(var_NT, rc, freefun)                                 \
    SAC_ND_DEC_RC_FREE__AKS_NUQ (var_NT, rc, freefun)

/******************************************************************************
 *
 * ICMs for predicate-sensitive operations:
 * ========================================
 *
 * IS_LASTREF__BLOCK_BEGIN( var_NT)
 * IS_LASTREF__BLOCK_ELSE( var_NT)
 * IS_LASTREF__BLOCK_END( var_NT)
 *
 * IS_REUSED__BLOCK_BEGIN( to_NT, from_NT)
 * IS_REUSED__BLOCK_ELSE( to_NT, from_NT)
 * IS_REUSED__BLOCK_END( to_NT, from_NT)
 *
 ******************************************************************************/

#define SAC_IS_LASTREF__BLOCK_BEGIN(var_NT)                                              \
    CAT17 (SAC_IS_LASTREF__BLOCK_BEGIN__, NT_UNQ (var_NT) (var_NT))
#define SAC_IS_LASTREF__BLOCK_ELSE(var_NT)                                               \
    }                                                                                    \
    else                                                                                 \
    {
#define SAC_IS_LASTREF__BLOCK_END(var_NT) }

#define SAC_IS_REUSED__BLOCK_BEGIN(to_NT, from_NT)                                       \
    CAT17 (SAC_IS_REUSED__BLOCK_BEGIN__, NT_SHP (to_NT) BuildArgs2 (to_NT, from_NT))
#define SAC_IS_REUSED__BLOCK_ELSE(to_NT, from_NT)                                        \
    }                                                                                    \
    else                                                                                 \
    {
#define SAC_IS_REUSED__BLOCK_END(to_NT, from_NT) }

/*
 * NUQ
 */

#define SAC_IS_LASTREF__BLOCK_BEGIN__NUQ(var_NT)                                         \
    CAT18 (SAC_IS_LASTREF__BLOCK_BEGIN__, CAT18 (NT_SHP (var_NT), _NUQ (var_NT)))

/*
 * UNQ
 */

#define SAC_IS_LASTREF__BLOCK_BEGIN__UNQ(var_NT) if (1) {

/*
 * SCL, NUQ
 */

#define SAC_IS_LASTREF__BLOCK_BEGIN__SCL_NUQ(var_NT)                                     \
    CAT19 (SAC_IS_LASTREF__BLOCK_BEGIN__SCL_, CAT19 (NT_HID (var_NT), _NUQ (var_NT)))
#define SAC_IS_LASTREF__BLOCK_BEGIN__SCL_NHD_NUQ(var_NT) if (1) {
#define SAC_IS_LASTREF__BLOCK_BEGIN__SCL_HID_NUQ(var_NT) if (SAC_ND_A_RC (var_NT) == 1) {

#define SAC_IS_REUSED__BLOCK_BEGIN__SCL(to_NT, from_NT) if (0) {

/*
 * AKS, NUQ
 */

#define SAC_IS_LASTREF__BLOCK_BEGIN__AKS_NUQ(var_NT)                                     \
    SAC_IS_LASTREF__BLOCK_BEGIN__SCL_HID_NUQ (var_NT)

#define SAC_IS_REUSED__BLOCK_BEGIN__AKS(to_NT, from_NT)                                  \
    if (SAC_ND_A_FIELD (to_NT) == SAC_ND_A_FIELD (from_NT)) {

/*
 * AKD, NUQ
 */

#define SAC_IS_LASTREF__BLOCK_BEGIN__AKD_NUQ(var_NT)                                     \
    SAC_IS_LASTREF__BLOCK_BEGIN__AKS_NUQ (var_NT)

#define SAC_IS_REUSED__BLOCK_BEGIN__AKD(to_NT, from_NT)                                  \
    SAC_IS_REUSED__BLOCK_BEGIN__AKS (to_NT, from_NT)

/*
 * AUD, NUQ
 */

#define SAC_IS_LASTREF__BLOCK_BEGIN__AUD_NUQ(var_NT)                                     \
    SAC_IS_LASTREF__BLOCK_BEGIN__AKS_NUQ (var_NT)

#define SAC_IS_REUSED__BLOCK_BEGIN__AUD(to_NT, from_NT)                                  \
    SAC_IS_REUSED__BLOCK_BEGIN__AKS (to_NT, from_NT)

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

#endif /* _SAC_STD_H_ */
