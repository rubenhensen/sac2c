/*
 *
 * $Log$
 * Revision 3.6  2002/03/07 20:21:57  dkr
 * ICMs for function arguments modified
 *
 * Revision 3.5  2002/03/07 02:25:48  dkr
 * macros ND_MAKE_UNIQUE_HIDDEN, ND_KS_NO_RC_MAKE_UNIQUE_ARRAY removed
 * some minor changes done
 *
 * Revision 3.4  2001/12/21 13:29:52  dkr
 * no changes done
 *
 * Revision 3.3  2001/05/22 12:14:29  nmw
 * SAC_ND_TYPEDEF_ARRAY terminating ; stripped off
 * due to error message by suncc.
 *
 * Revision 3.2  2000/11/27 21:10:01  cg
 * Added implementation of new ICM ND_ALLOC_ARRAY_PLACE.
 *
 * Revision 3.1  2000/11/20 18:02:20  sacbase
 * new release made
 *
 * Revision 2.13  2000/09/25 15:14:50  dkr
 * ICM ND_TYPEDEF_ARRAY moved from sac_misc.h to sac_std.h
 * ICM ND_TYPEDEF_HIDDEN created
 *
 * Revision 2.12  2000/08/24 11:29:05  dkr
 * macros cat? renamed into CAT?
 *
 * Revision 2.11  2000/08/18 23:04:34  dkr
 * definition of ND_KS_RET_OUT_RC and ND_KS_RET_INOUT_RC modified
 * in order to support renaming of out-paramters in case of name
 * conflicts
 *
 * Revision 2.10  2000/07/19 16:38:16  nmw
 * SAC_INITGLOBALOBJECT macros added
 *
 * Revision 2.9  2000/07/06 17:01:13  dkr
 * SAC_ND_A_SHAPE renamed into SAC_ND_A_SHAPE_AKD
 * SAC_ND_KD_A_SHAPE renamed into SAC_A_SHAPE
 *
 * Revision 2.8  2000/01/17 17:58:45  cg
 * Added support for optimized allocation of refernce counters.
 *
 * Revision 2.7  1999/09/22 13:27:03  cg
 * Converted memory allocation macro to command postion (syntactically)
 * rather than expression position. This allows for more flexibility
 * in code generation as alloc and free operations may now be implemented
 * in a similar way.
 *
 * Revision 2.6  1999/07/16 09:36:57  cg
 * Fixed serious bug in call of FIXED_SIZE memory allocation /
 * de-allocation macros.
 *
 * Revision 2.5  1999/07/08 12:39:25  cg
 * New heap manager is now used for array allocation /
 * de-allocation purposes.
 *
 * Revision 2.4  1999/06/25 14:50:48  jhs
 * Changed some macros. I had problems with multiple occurences of ## during
 * expansion of combinated macros. I introduced a Macro called CAT, this solution
 * is described in Kernighan/Ritchie A.12.3.
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
 *
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
 *  README: The difference between 'type' and 'basetype'
 *
 *  'type' and 'basetype' both specify a string containing a type.
 *
 *  'basetype' is used in all icms dedicated to arrays and simply means
 *  the base type of the array, e.g. 'int'.
 *
 *  'type' is used in all icms not specific to arrays and must
 *  specify the actual type, e.g. 'int *' for an integer array,
 *  'int' for an integer, 'void*' for a hidden, etc.
 *
 */

/*
 * ICMs for array access:
 * ========================
 *
 * ND_A_FIELD      : for accessing elements of the array
 * ND_A_SIZE       : accesses the size of the unrolling (in elements)
 * ND_A_DIM        : accesses the dimension of the array
 * ND_A_SHAPE      : accesses the shape in a specified dimension
 * ND_A_SHAPE_AKD  : accesses one shape component of an array
 * ND_A_SHAPEP_AKD : accesses the shape vector of an array
 *
 * ND_WRITE_ARRAY  : write access at specified index position
 * ND_READ_ARRAY   : read access at specified index position
 *
 * Only the latter two ICMs should be used to access the elements of an array
 * as they selectively enable boundary checking and cache simulation !
 */

#define SAC_ND_A_FIELD(name) name
#define SAC_ND_A_SIZE(name) CAT0 (name, __sz)
#define SAC_ND_A_DIM(name) CAT0 (name, __d)
#define SAC_ND_A_SHAPE(name, dim) CAT0 (CAT0 (name, __s), dim)
#define SAC_ND_A_SHAPEP_AKD(name) CAT0 (name, __s)
#define SAC_ND_A_SHAPE_AKD(name, dim) CAT0 (name, __s[dim])

#define SAC_ND_WRITE_ARRAY(name, pos)                                                    \
    SAC_BC_WRITE (name, pos) SAC_CS_WRITE_ARRAY (name, pos) SAC_ND_A_FIELD (name)[pos]

#define SAC_ND_READ_ARRAY(name, pos)                                                     \
    (SAC_BC_READ (name, pos) SAC_CS_READ_ARRAY (name, pos) SAC_ND_A_FIELD (name)[pos])

/*
 * ICMs for refcount access:
 * ===========================
 *
 * ND_A_RC(name)   : accesses the refcnt
 * ND_A_RCP(name)  : accesses the pointer to the refcnt
 *
 */

#define SAC_ND_A_RC(name) *CAT0 (name, __rc)
#define SAC_ND_A_RCP(name) CAT0 (name, __rc)

/*
 * ICMs for typedefs:
 * ====================
 *
 * ND_TYPEDEF_HIDDEN( name)
 * ND_TYPEDEF_ARRAY( basetype, name)
 *
 */

#define SAC_ND_TYPEDEF_HIDDEN(name) typedef void *name

#define SAC_ND_TYPEDEF_ARRAY(basetype, name) typedef basetype *name

/*
 * ICMs for declaring refcounted data:
 * =====================================
 *
 * ND_KD_DECL_ARRAY(basetype, name, dim)
 *   declares an array with dimension but without shape information
 *
 * ND_DECL_ARRAY(basetype, name)
 *   declares an array without dimension and shape information
 *
 * ND_DECL_RC(type, name)
 *   declares a refcounted variable in general
 *
 */

#define SAC_ND_DECL_RC(type, name)                                                       \
    type name;                                                                           \
    int SAC_ND_A_RC (name);

#define SAC_ND_KD_DECL_ARRAY(basetype, name, dim)                                        \
    SAC_ND_DECL_RC (basetype *, name)                                                    \
    int SAC_ND_A_SIZE (name);                                                            \
    const int SAC_ND_A_DIM (name) = dim;                                                 \
    int *SAC_ND_A_SHAPEP_AKD (name);

#define SAC_ND_DECL_ARRAY(basetype, name)                                                \
    SAC_ND_DECL_RC (basetype *, name)                                                    \
    int SAC_ND_A_SIZE (name);                                                            \
    int SAC_ND_A_DIM (name);                                                             \
    int *SAC_ND_A_SHAPEP_AKD (name);

/*
 * ICMs for removing refcounted data :
 * =====================================
 *
 * ND_FREE_HIDDEN(name, freefun)
 *   frees hidden data
 *
 * ND_NO_RC_FREE_HIDDEN(name, freefun)
 *   frees hidden data to whom a refcount is not yet assigned.
 *
 * ND_FREE_ARRAY(name)
 *   removes an array
 *
 * ND_NO_RC_FREE_ARRAY(name)
 *   removes an array to whom a refcount is not yet assigned
 *
 */

#define SAC_ND_FREE_HIDDEN(name, freefun)                                                \
    freefun (name);                                                                      \
    SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_RCP (name), sizeof (int));                          \
    SAC_TR_MEM_PRINT (("ND_FREE_HIDDEN(%s, %s) at addr: %p", #name, #freefun, name));    \
    SAC_TR_DEC_HIDDEN_MEMCNT (1);

#define SAC_ND_NO_RC_FREE_HIDDEN(name, freefun)                                          \
    freefun (name);                                                                      \
    SAC_TR_MEM_PRINT (                                                                   \
      ("ND_NO_RC_FREE_HIDDEN(%s, %s) at addr: %p", #name, #freefun, name));              \
    SAC_TR_DEC_HIDDEN_MEMCNT (1);

/*
 *  Look for comment on SECURE_ALLOC_FREE at SAC_ND_ALLOC_ARRAY.
 */

#ifdef SECURE_ALLOC_FREE

#define SAC_ND_FREE_ARRAY(name)                                                          \
    if (SAC_ND_A_SIZE (name) != 0) {                                                     \
        SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_FIELD (name),                                   \
                                SAC_ND_A_SIZE (name) * sizeof (*SAC_ND_A_FIELD (name))); \
    }                                                                                    \
    SAC_FREE_FIXED_SIZE (SAC_ND_A_RCP (name), sizeof (int));                             \
    SAC_TR_MEM_PRINT (("ND_FREE_ARRAY(%s) at addr: %p", #name, name));                   \
    SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (name));                                      \
    SAC_CS_UNREGISTER_ARRAY (name);

#define SAC_ND_NO_RC_FREE_ARRAY(name)                                                    \
    if (SAC_ND_A_SIZE (name) != 0) {                                                     \
        SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_FIELD (name),                                   \
                                SAC_ND_A_SIZE (name) * sizeof (*SAC_ND_A_FIELD (name))); \
    }                                                                                    \
    SAC_TR_MEM_PRINT (("ND_NO_RC_FREE_ARRAY(%s) at addr: %p", #name, name));             \
    SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (name));                                      \
    SAC_CS_UNREGISTER_ARRAY (name);

#else /* SECURE_ALLOC_FREE */

#define SAC_ND_FREE_ARRAY(name)                                                          \
    SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_FIELD (name),                                       \
                            SAC_ND_A_SIZE (name) * sizeof (*SAC_ND_A_FIELD (name)));     \
    SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_RCP (name), sizeof (int));                          \
    SAC_TR_MEM_PRINT (("ND_FREE_ARRAY(%s) at addr: %p", #name, name));                   \
    SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (name));                                      \
    SAC_CS_UNREGISTER_ARRAY (name);

#define SAC_ND_NO_RC_FREE_ARRAY(name)                                                    \
    SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_FIELD (name),                                       \
                            SAC_ND_A_SIZE (name) * sizeof (*SAC_ND_A_FIELD (name)));     \
    SAC_TR_MEM_PRINT (("ND_NO_RC_FREE_ARRAY(%s) at addr: %p", #name, name));             \
    SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (name));                                      \
    SAC_CS_UNREGISTER_ARRAY (name);
#endif

/*
 * ICMs for assigning refcounted data :
 * ====================================
 *
 * ND_ASSIGN_HIDDEN(old, new)
 *  copies the pointer to a hidden (including refcount)
 *
 * ND_NO_RC_ASSIGN_HIDDEN(old, new)
 *  copies the pointer to a hidden (without refcount)
 *
 * ND_COPY_HIDDEN(old, new, copyfun)
 *  copies hidden data using given function
 *
 * ND_KS_ASSIGN_ARRAY(name, res)
 *   copies pointer to array field (including refcount)
 *
 * ND_KS_NO_RC_ASSIGN_ARRAY(name, res)
 *   copies pointer to array field (without refcount)
 *
 * ND_KS_COPY_ARRAY(old, new, basetypesize)
 *   copies the array, doesn't care about refcount
 *
 */

#define SAC_ND_ASSIGN_HIDDEN(old, new)                                                   \
    {                                                                                    \
        new = old;                                                                       \
        SAC_ND_A_RCP (new) = SAC_ND_A_RCP (old);                                         \
    }

#define SAC_ND_NO_RC_ASSIGN_HIDDEN(old, new) new = old;

#define SAC_ND_COPY_HIDDEN(old, new, copyfun)                                            \
    {                                                                                    \
        new = copyfun (old);                                                             \
        SAC_TR_MEM_PRINT (("ND_COPY_HIDDEN(%s, %s, %s)", #old, #new, #copyfun));         \
        SAC_TR_MEM_PRINT (("new hidden object at addr: %p", new));                       \
        SAC_TR_INC_HIDDEN_MEMCNT (1);                                                    \
    }

#define SAC_ND_KS_ASSIGN_ARRAY(name, res)                                                \
    {                                                                                    \
        SAC_ND_A_RCP (res) = SAC_ND_A_RCP (name);                                        \
        SAC_ND_A_FIELD (res) = SAC_ND_A_FIELD (name);                                    \
    }

#define SAC_ND_KS_NO_RC_ASSIGN_ARRAY(name, res)                                          \
    SAC_ND_A_FIELD (res) = SAC_ND_A_FIELD (name);

#define SAC_ND_KS_COPY_ARRAY(old, new, basetypesize)                                     \
    {                                                                                    \
        int __i;                                                                         \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_FIELD (new),                                  \
                                  basetypesize *SAC_ND_A_SIZE (old));                    \
        SAC_CS_REGISTER_ARRAY (new);                                                     \
        for (__i = 0; __i < SAC_ND_A_SIZE (old); __i++) {                                \
            SAC_ND_WRITE_ARRAY (new, __i) = SAC_ND_READ_ARRAY (old, __i);                \
        }                                                                                \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_KS_COPY_ARRAY(%s, %s) at addr: %p", #old, #new, SAC_ND_A_FIELD (new)));   \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (new));                                   \
    }

/*
 * ICMs for creating refcounted data:
 * ====================================
 *
 * ND_ALLOC_ARRAY(basetype, name, rc)
 *   allocates memory needed
 *
 * ND_SET_SIZE( name, num)
 *   sets the size of the unrolling in elements
 *
 * ND_SET_DIM( name, num)
 *   sets the dimension of the array
 *
 * ND_SET_SHAPE( name, dim, s)
 *   sets one shape component of an array
 *
 * ND_ALLOC_RC(name)
 *   allocates memory for refcount (no initialization)
 *
 * ND_CREATE_CONST_ARRAY_C( name, str)
 *   creates a constant character array (string)
 *   Also see ND_CREATE_CONST_ARRAY_S for the creation of scalar arrays.
 */

#define SAC_ND_ALLOC_RC(name)                                                            \
    SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_RCP (name), sizeof (int));

/*  If SECURE_ALLOC_FREE is defined an extra check wether the requested size is 0
 *  is done. In that case no call for malloc(0) is executed, but directly NULL set for
 *  the array-variable is done.
 *
 *  This is an Option for possible Problems with calls for malloc(0).
 *
 *  This comment is referenced by: SAC_ND_FREE_ARRAY, SAC_ND_NO_RC_FREE_ARRAY.
 */
#ifdef SECURE_ALLOC_FREE
#define SAC_ND_ALLOC_ARRAY(basetype, name, rc)                                           \
    {                                                                                    \
        if (SAC_ND_A_SIZE (name) != 0) {                                                 \
            SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_FIELD (name),                             \
                                      sizeof (basetype) * SAC_ND_A_SIZE (name));         \
        } else {                                                                         \
            SAC_ND_A_FIELD (name) = NULL;                                                \
        }                                                                                \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_RCP (name), sizeof (int));                    \
        SAC_ND_A_RC (name) = rc;                                                         \
        SAC_TR_MEM_PRINT (("ND_ALLOC_ARRAY(%s, %s, %d) at addr: %p", #basetype, #name,   \
                           rc, SAC_ND_A_FIELD (name)));                                  \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (name));                                  \
        SAC_TR_REF_PRINT_RC (name);                                                      \
        SAC_CS_REGISTER_ARRAY (name);                                                    \
    }
#else
#define SAC_ND_ALLOC_ARRAY(basetype, name, rc)                                           \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE_WITH_RC (SAC_ND_A_FIELD (name), SAC_ND_A_RCP (name),    \
                                          sizeof (basetype) * SAC_ND_A_SIZE (name));     \
        SAC_ND_A_RC (name) = rc;                                                         \
        SAC_TR_MEM_PRINT (("ND_ALLOC_ARRAY(%s, %s, %d) at addr: %p", #basetype, #name,   \
                           rc, SAC_ND_A_FIELD (name)));                                  \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (name));                                  \
        SAC_TR_REF_PRINT_RC (name);                                                      \
        SAC_CS_REGISTER_ARRAY (name);                                                    \
    }

#define SAC_ND_ALLOC_ARRAY_PLACE(basetype, name, rc, other, offset, cachesize)           \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE_WITH_RC (SAC_ND_A_FIELD (name), SAC_ND_A_RCP (name),    \
                                          sizeof (basetype) * SAC_ND_A_SIZE (name)       \
                                            + cachesize + 32);                           \
        SAC_ND_A_RC (name) = rc;                                                         \
                                                                                         \
        SAC_ND_A_FIELD (name)                                                            \
          = SAC_HM_PlaceArray (SAC_ND_A_FIELD (name), SAC_ND_A_FIELD (other),            \
                               offset * sizeof (basetype), cachesize);                   \
                                                                                         \
        SAC_TR_MEM_PRINT (("ND_ALLOC_ARRAY(%s, %s, %d) at addr: %p", #basetype, #name,   \
                           rc, SAC_ND_A_FIELD (name)));                                  \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (name));                                  \
        SAC_TR_REF_PRINT_RC (name);                                                      \
        SAC_CS_REGISTER_ARRAY (name);                                                    \
    }
#endif

#define SAC_ND_SET_SIZE(name, num) SAC_ND_A_SIZE (name) = num;

#define SAC_ND_SET_DIM(name, num) SAC_ND_A_DIM (name) = num;

#define SAC_ND_SET_SHAPE(name, dim, s) SAC_ND_A_SHAPE_AKD (name, dim) = s;

#define SAC_ND_CREATE_CONST_ARRAY_C(name, str) SAC_String2Array (name, str);

/*
 * ICMs for reference counting :
 * ===============================
 *
 * ND_SET_RC(name, num)
 *   sets the refcnt
 *
 * ND_INC_RC(name, num)
 *   increments the refcnt
 *
 * ND_DEC_RC(name, num)
 *   decrements the refcnt
 *
 * ND_DEC_RC_FREE_ARRAY(name, num)
 *   decrements the refcnt AND frees the array if refcnt becomes zero!
 *
 * ND_DEC_RC_FREE_HIDDEN(name, num, freefun)
 *   decrements the refcnt AND frees the hidden object if refcnt becomes zero!
 *
 * ND_CHECK_REUSE_ARRAY(old, new)
 *   tries to reuse old array for new
 *
 * ND_CHECK_REUSE_HIDDEN(old, new, copyfun)
 *   tries to reuse old hidden data for new, copies if impossible
 *
 * ND_KS_MAKE_UNIQUE_ARRAY(old, new, basetypesize)
 *   assigns old to new if refcount is zero and copies the array otherwise
 *   A new refcount is allocated if necessary
 *
 * ND_NO_RC_MAKE_UNIQUE_HIDDEN(old, new, copyfun)
 *   assigns old to new if refcount is zero and copies the hidden otherwise
 *   No new refcount is allocated, the old one is freed when not copying
 *
 *
 */

#define SAC_ND_SET_RC(name, num)                                                         \
    {                                                                                    \
        SAC_ND_A_RC (name) = num;                                                        \
        SAC_TR_REF_PRINT (("ND_SET_RC(%s, %d)", #name, num));                            \
        SAC_TR_REF_PRINT_RC (name);                                                      \
    }

#define SAC_ND_INC_RC(name, num)                                                         \
    {                                                                                    \
        SAC_ND_A_RC (name) += num;                                                       \
        SAC_TR_REF_PRINT (("ND_INC_RC(%s, %d)", #name, num));                            \
        SAC_TR_REF_PRINT_RC (name);                                                      \
    }

#define SAC_ND_DEC_RC(name, num)                                                         \
    {                                                                                    \
        SAC_ND_A_RC (name) -= num;                                                       \
        SAC_TR_REF_PRINT (("ND_DEC_RC(%s, %d)", #name, num));                            \
        SAC_TR_REF_PRINT_RC (name);                                                      \
    }

#define SAC_ND_DEC_RC_FREE_ARRAY(name, num)                                              \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_DEC_RC_FREE(%s, %d)", #name, num));                       \
        if ((SAC_ND_A_RC (name) -= num) == 0) {                                          \
            SAC_TR_REF_PRINT_RC (name);                                                  \
            SAC_ND_FREE_ARRAY (name);                                                    \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (name);                                                  \
        }                                                                                \
    }

#define SAC_ND_DEC_RC_FREE_HIDDEN(name, num, freefun)                                    \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_DEC_RC_FREE(%s, %d)", #name, num));                       \
        if ((SAC_ND_A_RC (name) -= num) == 0) {                                          \
            SAC_TR_REF_PRINT_RC (name);                                                  \
            SAC_ND_FREE_HIDDEN (name, freefun);                                          \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (name);                                                  \
        }                                                                                \
    }

#define SAC_ND_CHECK_REUSE_ARRAY(old, new)                                               \
    if (SAC_ND_A_RC (old) == 1) {                                                        \
        SAC_ND_KS_ASSIGN_ARRAY (old, new);                                               \
        SAC_TR_MEM_PRINT (("reuse memory of %s at %p for %s", #old, old, #new));         \
    } else

#define SAC_ND_KS_MAKE_UNIQUE_ARRAY(old, new, basetypesize)                              \
    {                                                                                    \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_KS_MAKE_UNIQUE_ARRAY(%s, %s, %d)", #old, #new, basetypesize));            \
        SAC_TR_REF_PRINT_RC (old);                                                       \
        if ((SAC_ND_A_RC (old)) == 1) {                                                  \
            SAC_ND_KS_ASSIGN_ARRAY (old, new);                                           \
            SAC_TR_MEM_PRINT (("%s is already unique.", old));                           \
        } else {                                                                         \
            SAC_ND_KS_COPY_ARRAY (old, new, basetypesize);                               \
            SAC_ND_DEC_RC (old, 1);                                                      \
            SAC_ND_ALLOC_RC (new);                                                       \
        }                                                                                \
    }

#define SAC_ND_NO_RC_MAKE_UNIQUE_HIDDEN(old, new, copyfun)                               \
    {                                                                                    \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_NO_RC_MAKE_UNIQUE_HIDDEN(%s, %s, %s)", #old, #new, #copyfun));            \
        SAC_TR_REF_PRINT_RC (old);                                                       \
        if (SAC_ND_A_RC (old) == 1) {                                                    \
            SAC_ND_NO_RC_ASSIGN_HIDDEN (old, new);                                       \
            SAC_HM_FREE_FIXED_SIZE (SAC_ND_A_RCP (old), sizeof (int));                   \
            SAC_TR_MEM_PRINT (("%s is already unique.", old));                           \
        } else {                                                                         \
            SAC_ND_COPY_HIDDEN (old, new, copyfun);                                      \
            SAC_ND_DEC_RC (old, 1);                                                      \
        }                                                                                \
    }

/*
 * ICMs for arguments to functions :
 * =================================================
 *
 *
 * ND_PARAM_in( type, name)
 *   macro for prototyping non-refcounted data as "in" parameter
 *
 * ND_PARAM_in_rc( type, name)
 *   macro for prototyping refcounted data as "in" parameter
 *
 * ND_PARAM_out( type, name)
 *   macro for prototyping non-refcounted data as "out" parameter
 *
 * ND_PARAM_out_rc( type, name)
 *   macro for prototyping refcounted data as "out" parameter
 *
 * ND_PARAM_inout( type, name)
 *   macro for prototyping non-refcounted data as "inout" parameter
 *
 * ND_PARAM_inout_rc( type, name)
 *   macro for prototyping refcounted data as "inout" parameter
 *
 * ND_PARAM_upd( type, name)
 *   macro for prototyping unboxed data as "upd" parameter
 *
 * ND_PARAM_upd_bx( type, name)
 *   macro for prototyping boxed data as "upd" parameter
 *
 * ND_ARG_in( name)
 *   macro for giving non-refcounted data as argument
 *
 * ND_ARG_in_rc( name)
 *   macro for giving refcounted data as argument
 *
 * ND_ARG_out( name)
 *   macro for getting non-refcounted data as result
 *
 * ND_ARG_out_rc( name)
 *   macro for getting refcounted data as result
 *
 * ND_ARG_inout( name)
 *   macro for giving non-refcounted data as "inout" argument
 *
 * ND_ARG_inout_rc( name)
 *   macro for giving refcounted data as "inout" argument
 *
 * ND_ARG_upd( name)
 *   macro for giving unboxed data as "upd" argument
 *
 * ND_ARG_upd_bx( name)
 *   macro for giving boxed data as "upd" argument
 *
 * ND_RET_out( name, namep)
 *   macro for returning non-refcounted data
 *
 * ND_RET_out_rc( name, namep)
 *   macro for returning refcounted data
 *
 * ND_RET_inout( name, namep)
 *   macro for returning "inout" non-refcounted data
 *
 * ND_RET_inout_rc( name, namep)
 *   macro for returning "inout" refcounted data
 *
 */

/*
 * creates name for formal function parameter
 */
#define SAC_NAMEP(name) CAT0 (name, __p)

#define SAC_ND_PARAM_in(type, name) type SAC_ND_A_FIELD (name)

#define SAC_ND_PARAM_in_rc(type, name)                                                   \
    type SAC_ND_A_FIELD (name), int *SAC_ND_A_RCP (name)

#define SAC_ND_PARAM_out(type, name) type *SAC_NAMEP (SAC_ND_A_FIELD (name))

#define SAC_ND_PARAM_out_rc(type, name)                                                  \
    type *SAC_NAMEP (SAC_ND_A_FIELD (name)), int **SAC_NAMEP (SAC_ND_A_RCP (name))

#define SAC_ND_PARAM_inout(type, name) type *SAC_NAMEP (SAC_ND_A_FIELD (name))

#define SAC_ND_PARAM_inout_rc(type, name)                                                \
    type *SAC_NAMEP (SAC_ND_A_FIELD (name)), int **SAC_NAMEP (SAC_ND_A_RCP (name))

#define SAC_ND_PARAM_upd(type, name) type *SAC_ND_A_FIELD (name)

#define SAC_ND_PARAM_upd_bx(type, name) type SAC_ND_A_FIELD (name)

#define SAC_ND_ARG_in(name) SAC_ND_A_FIELD (name)

#define SAC_ND_ARG_in_rc(name) SAC_ND_A_FIELD (name), SAC_ND_A_RCP (name)

#define SAC_ND_ARG_out(name) &SAC_ND_A_FIELD (name)

#define SAC_ND_ARG_out_rc(name) &SAC_ND_A_FIELD (name), &SAC_ND_A_RCP (name)

#define SAC_ND_ARG_inout(name) &SAC_ND_A_FIELD (name)

#define SAC_ND_ARG_inout_rc(name) &SAC_ND_A_FIELD (name), &SAC_ND_A_RCP (name)

#define SAC_ND_ARG_upd(name) &SAC_ND_A_FIELD (name)

#define SAC_ND_ARG_upd_bx(name) SAC_ND_A_FIELD (name)

#define SAC_ND_RET_out(name, namep)                                                      \
    *SAC_NAMEP (SAC_ND_A_FIELD (namep)) = SAC_ND_A_FIELD (name);

#define SAC_ND_RET_out_rc(name, namep)                                                   \
    *SAC_NAMEP (SAC_ND_A_FIELD (namep)) = SAC_ND_A_FIELD (name);                         \
    *SAC_NAMEP (SAC_ND_A_RCP (namep)) = SAC_ND_A_RCP (name);

#define SAC_ND_RET_inout(name, namep)                                                    \
    *SAC_NAMEP (SAC_ND_A_FIELD (namep)) = SAC_ND_A_FIELD (name);

#define SAC_ND_RET_inout_rc(name, namep)                                                 \
    *SAC_NAMEP (SAC_ND_A_FIELD (namep)) = SAC_ND_A_FIELD (name);                         \
    *SAC_NAMEP (SAC_ND_A_RCP (namep)) = SAC_ND_A_RCP (name);

#define SAC_ND_DECL_PARAM_inout(type, name)                                              \
    type SAC_ND_A_FIELD (name) = *SAC_NAMEP (SAC_ND_A_FIELD (name));

#define SAC_ND_DECL_PARAM_inout_rc(type, name)                                           \
    type SAC_ND_A_FIELD (name) = *SAC_NAMEP (SAC_ND_A_FIELD (name));                     \
    int *SAC_ND_A_RCP (name) = *SAC_NAMEP (SAC_ND_A_RCP (name));

/*
 * SAC_INITGLOBALOBJECT( initfun, varname )
 *
 * ICM for initializing global objects.
 * when compiling for program or ususal sac module it only calls
 * the objinit function (initfun). if used in a c library it has to check
 * if the global obj has been initialized by another module (uses varname as flag)
 *
 */
#ifdef SAC_GENERATE_CLIBRARY
/* with check */
#define SAC_INITGLOBALOBJECT_BEGIN(varname)                                              \
    if (!varname) {                                                                      \
        varname = true;
#define SAC_INITGLOBALOBJECT_END() }

#else
/* without check -> NOP */
#define SAC_INITGLOBALOBJECT_BEGIN(varname)
#define SAC_INITGLOBALOBJECT_END()
#endif

#endif /* SAC_STD_H */
