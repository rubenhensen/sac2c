/*
 *
 * $Log$
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
 * Revision 1.3  1998/06/19 18:31:01  dkr
 * *** empty log message ***
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

/*
 * ICMs for array descriptor access:
 * ========================
 *
 * ND_A_FIELD : for accessing elements of the array
 * ND_A_SIZE  : accesses the size of the unrolling (in elements)
 * ND_A_DIM   : accesses the dimension of the array
 * ND_A_SHAPE : accesses one shape component of an array
 * ND_WRITE_ARRAY : write access at specified index position
 * ND_READ_ARRAY  : read access at specified index position
 *
 * Only the latter two ICMs should be used to access the elements of an array
 * as they selectively enable boundary checking and cache simulation !
 * ND_A_RC    : accesses the reference count of the array
 * ND_A_DESC  : accesses the descriptor of the array
 * ND_A_PAA   : accesses the physical (malloc) address of the array
 *
 */

/*
 * Positional parameters for name tuples (nt):
 *  name,class,unique
 *  Values are:
 *    name: object name
 *
 *    class:  AKD: array of known dimension but unknown shape
 *            AKS: array of known shape
 *            HID: hidden object
 *
 *    unique: UNQ: object is unique
 *            NUQ: object is non-unique
 *
 */

#define NAME_NAME(nt) Item0 nt
#define NAME_CLASS(nt) Item1 nt
#define NAME_UNI(nt) Item2 nt

#define SAC_ND_A_FIELD(nt) NAME_NAME (nt)

#define SAC_ND_A_DIM(nt) cat (cat (SAC_ND_A_DIM_, NAME_CLASS (nt)), (nt))

#define SAC_ND_A_SHAPE(nt, dim)                                                          \
    cat (SAC_ND_A_SHAPE_, cat (NAME_CLASS (nt), BuildArgs2 (nt, dim)))

#define SAC_ND_A_SIZE(nt) cat (SAC_ND_A_SIZE_, cat (NAME_CLASS (nt), (nt)))

#define ncat(x, y) nncat (x, y)
#define nncat(x, y) x##y

#define SAC_ND_A_RC(nt) cat (SAC_ND_A_RC_, cat (NAME_CLASS (nt), (nt)))
#define SAC_ND_A_RCP(nt) cat (SAC_ND_A_RCP_, cat (NAME_CLASS (nt), (nt)))
#define SAC_ND_A_DESC(nt) ncat (NAME_NAME (nt), __desc)
#define SAC_ND_A_PAA(nt) cat (SAC_ND_A_DESC (nt), ->paa)

#define SAC_ND_WRITE_ARRAY(nt, pos)                                                      \
    SAC_BC_WRITE (nt, pos) SAC_CS_WRITE_ARRAY (nt, pos) SAC_ND_A_FIELD (nt)[pos]

#define SAC_ND_READ_ARRAY(nt, pos)                                                       \
    (SAC_BC_READ (nt, pos) SAC_CS_READ_ARRAY (nt, pos) SAC_ND_A_FIELD (nt)[pos])

/*
 * ICMs for fixed-shape, non-unique arrays
 * ICMs for fixed-shape, unique arrays
 *
 *  IMPLEMENTOR NOTE: The RC ICMs are not used for unique
 *  arrays, but they WILL generate (incorrect) code if
 *  invoked, rather than generating an error.
 *  rbe 1999-06-15,
 *
 */

#define ycat(x, y) yycat (x, y)
#define yycat(x, y) x##y
#define SAC_ND_A_SIZE_AKS(nt) ycat (NAME_NAME (nt), __sz)
#define SAC_ND_A_DIM_AKS(nt) ycat (NAME_NAME (nt), __d)
#define SAC_ND_A_SHAPE_AKS(nt, dim) ycat (NAME_NAME (nt), __s##dim)
#define SAC_ND_A_RC_AKS(nt) ycat (SAC_ND_A_DESC (nt), ->rc)

/*
 *  ICMs for unknown-shape, non-unique arrays
 *  ICMs for unknown-shape, unique arrays
 */

#define SAC_ND_A_SIZE_AKD(nt) ycat (SAC_ND_A_DESC (nt), ->size)
#define SAC_ND_A_DIM_AKD(nt) ycat (SAC_ND_A_DESC (nt), ->dim)
#define SAC_ND_A_SHAPE_AKD(nt, dim) ycat (SAC_ND_A_DESC (nt), ->shape[dim])
#define SAC_ND_A_RC_AKD(nt) SAC_ND_A_RC_AKS (nt)

/*
 * ICMs for refcount access by hidden objects:
 * ===========================
 *
 * SAC_ND_A_RC_HID(nt)   : accesses the refcnt
 * SAC_ND_A_RCP_HID(nt)  : accesses the pointer to the refcnt
 *
 * The above are used only for abstract data types
 *
 */

#define SAC_ND_A_RC_HID(nt) ycat (*, ycat (NAME_NAME (nt), __rc))
#define SAC_ND_A_RCP_HID(nt) ycat (NAME_NAME (nt), __rc)

/*
 * ICMs for declaring refcounted data:
 * =====================================
 *
 * ND_DECL_RC(type, name)
 *   declares a refcounted variable in general
 *   As of 1999-06-16, this is only used for hidden, ref-counted data.
 *   This is moribund, being replaced in time by a call to
 *    SAC_ND_DECL_DATA(type,nt)
 *
 * SAC_DECL_DESC
 *   declares an array descriptor
 */

#define SAC_ND_DECL_RC(type, name)                                                       \
    type name;                                                                           \
    int name##__rc;

#define SAC_ND_DECL_DATA_AKD_UNQ(type, nt, dim) SAC_ND_DECL_DATA_AKD_NUQ (type, nt, dim)

#define SAC_ND_DECL_DATA_AKD_NUQ(type, nt, dim)                                          \
    type SAC_ND_A_FIELD (nt);                                                            \
    SAC_ND_DECL_DESC (nt)

#define SAC_ND_DECL_DESC(nt, dim) SAC_array_descriptor *SAC_ND_A_DESC (nt);

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
 *   removes an array that has has a descriptor
 *
 * ND_NO_RC_FREE_ARRAY(name)
 *   removes an array to which a refcount is not yet assigned
 *
 */

#define SAC_ND_FREE_HIDDEN(name, freefun)                                                \
    freefun (name);                                                                      \
    SAC_FREE (SAC_ND_A_RCP (name));                                                      \
    SAC_TR_MEM_PRINT (("ND_FREE_HIDDEN(%s, %s) at addr: %p", #name, #freefun, name));    \
    SAC_TR_DEC_HIDDEN_MEMCNT (1);

#define SAC_ND_NO_RC_FREE_HIDDEN(name, freefun)                                          \
    freefun (name);                                                                      \
    SAC_TR_MEM_PRINT (                                                                   \
      ("ND_NO_RC_FREE_HIDDEN(%s, %s) at addr: %p", #name, #freefun, name));              \
    SAC_TR_DEC_HIDDEN_MEMCNT (1);

#define SAC_ND_FREE_ARRAY(nt)                                                            \
    cat (SAC_ND_FREE_ARRAY_, cat (NAME_CLASS (nt), cat (_, cat (NAME_UNI (nt), (nt)))))

#define SAC_ND_FREE_ARRAY_AKS_NUQ(nt)                                                    \
    SAC_FREE (SAC_ND_A_FIELD (nt));                                                      \
    SAC_FREE (SAC_ND_A_DESC (nt));                                                       \
    SAC_TR_MEM_PRINT (("ND_FREE_ARRAY(%s) at addr: %p", #nt, nt));                       \
    SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (nt));

#define SAC_ND_FREE_ARRAY_AKD_UNQ(nt) SAC_ND_FREE_ARRAY_AKS_NUQ (nt)
#define SAC_ND_FREE_ARRAY_AKD_NUQ(nt) SAC_ND_FREE_ARRAY_AKS_NUQ (nt)
#define SAC_ND_FREE_ARRAY_AKS_UNQ(nt)                                                    \
    SAC_FREE (SAC_ND_A_FIELD (nt));                                                      \
    SAC_TR_MEM_PRINT (("ND_FREE_ARRAY(%s) at addr: %p", #nt, nt));                       \
    SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (nt));

#define SAC_ND_NO_RC_FREE_ARRAY(nt)                                                      \
    SAC_FREE (SAC_ND_A_FIELD (nt));                                                      \
    SAC_TR_MEM_PRINT (("ND_NO_RC_FREE_ARRAY(%s) at addr: %p", #nt, NAME_NAME (nt)));     \
    SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (NAME_NAME (nt)));

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
 * ND_AKS_ASSIGN_ARRAY(name, res)
 *   copies pointer to array field (including refcount)
 *
 * ND_AKS_NO_RC_ASSIGN_ARRAY(name, res)
 *   copies pointer to array field (without refcount)
 *
 * ND_AKS_COPY_ARRAY(old, new, basetypesize)
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
        SAC_TR_REF_PRINT_RC (new);                                                       \
    }

#define SAC_ND_ASSIGN_DATA(nt, resnt)                                                    \
    cat (SAC_ND_ASSIGN_DATA_, cat (NAME_CLASS (nt), cat (_, NAME_UNI (nt) (nt, resnt))))

#define SAC_ND_ASSIGN_DATA_AKS_UNQ(nt, resnt)                                            \
    {                                                                                    \
        SAC_ND_A_FIELD (resnt) = SAC_ND_A_FIELD (nt);                                    \
    }

#define SAC_ND_ASSIGN_DATA_AKS_NUQ(nt, resnt)                                            \
    {                                                                                    \
        SAC_ND_A_DESC (resnt) = SAC_ND_A_DESC (nt);                                      \
        SAC_ND_A_FIELD (resnt) = SAC_ND_A_FIELD (nt);                                    \
    }

#define SAC_ND_ASSIGN_DATA_AKD_UNQ(nt, resnt)                                            \
    {                                                                                    \
        SAC_ND_A_DESC (resnt) = SAC_ND_A_DESC (nt);                                      \
        SAC_ND_A_FIELD (resnt) = SAC_ND_A_FIELD (nt);                                    \
    }

#define SAC_ND_ASSIGN_DATA_AKD_NUQ(nt, resnt)                                            \
    {                                                                                    \
        SAC_ND_A_DESC (resnt) = SAC_ND_A_DESC (nt);                                      \
        SAC_ND_A_FIELD (resnt) = SAC_ND_A_FIELD (nt);                                    \
    }

/* Side entrances to assignment macros to ease transition to new macros */
#define SAC_ND_AKS_ASSIGN_ARRAY(name, res)                                               \
    SAC_ND_ASSIGN_DATA ((name, (AKS, (NUQ, ))), (res, (AKS, (NUQ, ))));
#define SAC_ND_AKS_NO_RC_ASSIGN_ARRAY(name, res)                                         \
    SAC_ND_ASSIGN_DATA ((name, (AKS, (UNQ, ))), (res, (AKS, (UNQ, ))));

/* Array copy macros when no class changing is going on */
#define SAC_ND_COPY_DATA(oldnt, newnt)                                                   \
    cat (SAC_ND_COPY_DATA,                                                               \
         cat (NAME_CLASS (newnt), cat (_, NAME_UNI (newnt) (oldnt, newnt))))

/* Another side entrance for transition to new macros */
#define SAC_ND_AKS_COPY_ARRAY(old, new, basetypesize)                                    \
    SAC_ND_COPY_DATA ((old, (AKS, (NUQ, ))), (new, (AKS, (NUQ, ))))

#define SAC_ND_COPY_DATA_AKS_NUQ(oldnt, newnt)                                           \
    {                                                                                    \
        int __i;                                                                         \
  SAC_ND_A_FIELD(newnt)= \
   SAC_MALLOC((sizeof(*SAC_ND_A_FIELD(oldnt))*SAC_ND_A_SIZE(oldnt));      \
  for(__i=0; __i<SAC_ND_A_SIZE(oldnt); __i++)  {                          \
    SAC_ND_WRITE_ARRAY(newnt,__i)= SAC_ND_READ_ARRAY(oldnt,__i);          \
  }                                                                       \
  SAC_TR_MEM_PRINT(("ND_AKS_COPY_ARRAY(%s, %s) at addr: %p",               \
                    #oldnt, #newnt, SAC_ND_A_FIELD(newnt)));              \
  SAC_TR_REF_PRINT_RC(oldnt);                                             \
  SAC_TR_INC_ARRAY_MEMCNT(SAC_ND_A_SIZE(newnt));                                         \
    }

/*
 * ICMs for creating refcounted data:
 * ====================================
 *
 * ND_ALLOC_ARRAY(basetype, nt, rc)
 *   allocates memory needed
 *
 * ND_SET_SIZE( nt, num)
 *   sets the size of the unrolling in elements
 *
 * ND_SET_DIM( nt, num)
 *   sets the dimension of the array
 *
 * ND_SET_SHAPE( nt, dim, s)
 *   sets one shape component of an array
 *
 * ND_ALLOC_RC(nt)
 *   allocates memory for refcount (no initialization)
 *
 * ND_CREATE_CONST_ARRAY_C( nt, str)
 *   creates a constant character array (string)
 *   Also see ND_CREATE_CONST_ARRAY_S for the creation of scalar arrays.
 */

#define SAC_ND_ALLOC_RC(nt) SAC_ND_A_RCP (nt) = (int *)SAC_MALLOC (sizeof (int));

#define SAC_ND_ALLOC_ARRAY(basetype, nt, rc)                                             \
    {                                                                                    \
        SAC_ND_A_FIELD (nt)                                                              \
          = (basetype *)SAC_MALLOC (sizeof (basetype) * SAC_ND_A_SIZE (nt));             \
        SAC_ND_A_RCP (nt) = (int *)SAC_MALLOC (sizeof (int));                            \
        SAC_ND_A_RC (nt) = rc;                                                           \
        \ 
  SAC_TR_MEM_PRINT (("ND_ALLOC_ARRAY(%s, %s, %d) at addr: %p", #basetype, #nt, rc,       \
                     SAC_ND_A_FIELD (nt)));                                              \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (nt));                                    \
        SAC_TR_REF_PRINT_RC (nt);                                                        \
    }

#define SAC_ND_SET_SIZE(nt, num) SAC_ND_A_SIZE (nt) = num;

#define SAC_ND_SET_DIM(nt, num) SAC_ND_A_DIM (nt) = num;

#define SAC_ND_SET_SHAPE(nt, dim, s) SAC_ND_A_SHAPE (nt, dim) = s;

#define SAC_ND_CREATE_CONST_ARRAY_C(nt, str) SAC_String2Array (nt, str);

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
 *
 *
 */

#define SAC_ND_SET_RC(nt, num)                                                           \
    {                                                                                    \
        SAC_ND_A_RC (nt) = num;                                                          \
        SAC_TR_REF_PRINT (("ND_SET_RC(%s, %d)", #nt, num));                              \
        SAC_TR_REF_PRINT_RC (nt)                                                         \
    \ 
}

#define SAC_ND_INC_RC(nt, num)                                                           \
    {                                                                                    \
        SAC_ND_A_RC (nt) += num;                                                         \
        SAC_TR_REF_PRINT (("ND_INC_RC(%s, %d)", #nt, num));                              \
        SAC_TR_REF_PRINT_RC (nt);                                                        \
    }

#define SAC_ND_DEC_RC(nt, num)                                                           \
    {                                                                                    \
        SAC_ND_A_RC (nt) -= num;                                                         \
        SAC_TR_REF_PRINT (("ND_DEC_RC(%s, %d)", #nt, num));                              \
        SAC_TR_REF_PRINT_RC (nt);                                                        \
    }

#define SAC_ND_DEC_RC_FREE_ARRAY(nt, num)                                                \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_DEC_RC_FREE(%s, %d)", #nt, num));                         \
        if ((SAC_ND_A_RC (nt) -= num) == 0) {                                            \
            SAC_TR_REF_PRINT_RC (nt);                                                    \
            SAC_ND_FREE_ARRAY (nt);                                                      \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (nt);                                                    \
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
        SAC_ND_AKS_ASSIGN_ARRAY (old, new);                                              \
        SAC_TR_MEM_PRINT (("reuse memory of %s at %p for %s", #old, old, #new));         \
    } else

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

/*
 * ICMs for passing refcounted data to functions :
 * =================================================
 *
 *
 * ND_KS_DEC_IN_RC( type, name)
 *   macro for prototyping refcounted data as "in" parameter
 *
 * ND_KS_DEC_OUT_RC( type, name)
 *   macro for prototyping refcounted data as "out" parameter
 *
 * ND_KS_DEC_INOUT_RC( type, name)
 *   macro for prototyping refcounted data as "inout" parameter
 *
 * ND_KS_DEC_IMPORT_IN_RC( type)
 *   macro for prototyping refcounted data as "in" parameter
 *   (imported functions only )
 *
 * ND_KS_DEC_IMPORT_OUT_RC( type)
 *   macro for prototyping refcounted data as "out" parameter
 *   (imported functions only )
 *
 * ND_KS_DEC_IMPORT_INOUT_RC( type)
 *   macro for prototyping refcounted data as "inout" parameter
 *   (imported functions only )
 *
 * ND_KS_AP_IN_RC( name)
 *   macro for giving refcounted data as argument
 *
 * ND_KS_AP_OUT_RC( name)
 *   macro for getting refcounted data as result
 *
 * ND_KS_AP_INOUT_RC( name)
 *   macro for giving refcounted data as "inout" argument
 *
 * ND_KS_RET_OUT_RC( name)
 *   macro for returning refcounted data
 *
 * ND_KS_RET_INOUT_RC( name)
 *   macro for returning "inout" refcounted data
 *
 */

#define SAC_ND_KS_DEC_IN_RC(type, name) type name, int *name##__rc

#define SAC_ND_KS_DEC_OUT_RC(type, name) type *name##__p, int **name##__rc__p

#define SAC_ND_KS_DEC_INOUT_RC(type, name) type *name##__p, int **name##__rc__p

#define SAC_ND_KS_DEC_IMPORT_IN_RC(type) type, int *

#define SAC_ND_KS_DEC_IMPORT_OUT_RC(type) type *, int **

#define SAC_ND_KS_DEC_IMPORT_INOUT_RC(type) type *, int **

#define SAC_ND_KS_AP_IN_RC(name) name, name##__rc

#define SAC_ND_KS_AP_OUT_RC(name) &name, &name##__rc

#define SAC_ND_KS_AP_INOUT_RC(name) &name, &name##__rc

#define SAC_ND_KS_RET_OUT_RC(name)                                                       \
    *##name##__p = name;                                                                 \
    *name##__rc__p = name##__rc

#define SAC_ND_KS_RET_INOUT_RC(name)                                                     \
    *##name##__p = name;                                                                 \
    *name##__rc__p = name##__rc

#define SAC_ND_DECL_INOUT_PARAM(type, name) type name = *##name##__p;

#define SAC_ND_DECL_INOUT_PARAM_RC(type, name)                                           \
    type name = *##name##__p;                                                            \
    int *name##__rc = *name##__rc__p;

#endif /* SAC_STD_H */
