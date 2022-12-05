/** <!-- ****************************************************************** -->
 * @file sacinterface.h
 *
 * @brief Implementation of the SAC C Interface.
 *
 *        For examples see sac/demos/sac_from_c
 *
 ******************************************************************************/

#ifndef _SAC_SACINTERFACE_H_
#define _SAC_SACINTERFACE_H_

/**
 * SACarg type
 */
typedef struct SAC_SACARG SACarg;

/** <!-- ****************************************************************** -->
 * @brief Returns the dimensionality of the given argument.
 *
 * @param arg SACarg (not consumed)
 *
 * @return dimensionality
 ******************************************************************************/
SAC_C_EXTERN int SACARGgetDim (SACarg *arg);

/** <!-- ****************************************************************** -->
 * @brief Returns the shape of the given argument at the given position.
 *
 * @param arg SACarg (not consumed)
 * @param pos position starting from 0
 *
 * @return shape
 ******************************************************************************/
SAC_C_EXTERN int SACARGgetShape (SACarg *arg, int pos);

/** <!-- ****************************************************************** -->
 * @brief Returns the basetype of the given argument.
 *
 * @param arg SACarg (not consumed)
 *
 * @return basetype
 ******************************************************************************/
SAC_C_EXTERN int SACARGgetBasetype (SACarg *arg);

/** <!-- ****************************************************************** -->
 * @brief Returns a new reference to the given argument. This can be used
 *        to keep a handle to a datastructure passed to a SAC function and
 *        thus preventing it from beeing freed/reused.
 *
 * @param arg SACarg (not consumed)
 *
 * @return new SACarg containing the same data
 ******************************************************************************/
SAC_C_EXTERN SACarg *SACARGnewReference (SACarg *arg);

/** <!-- ****************************************************************** -->
 * @brief Converts a given SACarg datastructure into a C vector of the
 *        corresponding basetype and returns a pointer to it.
 *
 *        THE ARGUMENT IS CONSUMED AND THE SACARG STRUCTURE IS FREED!
 *
 * @param arg SACarg structure
 *
 * @return pointer to contained data
 ******************************************************************************/
SAC_C_EXTERN int *SACARGconvertToIntArray (SACarg *arg);
SAC_C_EXTERN double *SACARGconvertToDoubleArray (SACarg *arg);
SAC_C_EXTERN float *SACARGconvertToFloatArray (SACarg *arg);
SAC_C_EXTERN int *SACARGconvertToBoolArray (SACarg *arg);
SAC_C_EXTERN char *SACARGconvertToCharArray (SACarg *arg);

/** <!-- ****************************************************************** -->
 * @brief Converts a given C data vector into a SACarg structure.
 *
 *        THE DATA VECTOR IS CONSUMED AND MAY BE FREED ONCE THE LAST
 *        SACARG STRUCTURE REFERENCING IT IS FREED!
 *
 *        The dim argument gives the dimensionality of the array, and is
 *        followed by the extent along each axis.
 *
 * @param data C data vector
 * @param dim  dimensionality of resulting array
 * @param ...  corresponding shape components
 *
 * @return SACarg structure
 ******************************************************************************/
SAC_C_EXTERN SACarg *SACARGconvertFromIntPointer (int *data, int dim, ...);
SAC_C_EXTERN SACarg *SACARGconvertFromDoublePointer (double *data, int dim, ...);
SAC_C_EXTERN SACarg *SACARGconvertFromFloatPointer (float *data, int dim, ...);
SAC_C_EXTERN SACarg *SACARGconvertFromBoolPointer (int *data, int dim, ...);
SAC_C_EXTERN SACarg *SACARGconvertFromCharPointer (char *data, int dim, ...);

/** <!-- ****************************************************************** -->
 * @brief Converts a given C vector into a SACarg data structure.
 *
 *        THE DATA VECTOR IS CONSUMED AND MAY BE FREED ONCE THE LAST
 *        SACARG STRUCTURE REFERENCING IT IS FREED!
 *
 *        The dim argument gives the dimensionality of the resulting array
 *        and is followed by shape, a C vector of corresponding length.
 *        The shape vector IS NOT CONSUMED and can be safely freed
 *        afterwards.
 *
 * @param data C data vector
 * @param dim  dimensionality of resulting array
 * @param shape corresponding shape vector
 *
 * @return corresponding SACarg structure
 ******************************************************************************/
SAC_C_EXTERN SACarg *SACARGconvertFromIntPointerVect (int *data, int dim, int *shape);
SAC_C_EXTERN SACarg *SACARGconvertFromDoublePointerVect (double *data, int dim, int *shape);
SAC_C_EXTERN SACarg *SACARGconvertFromFloatPointerVect (float *data, int dim, int *shape);
SAC_C_EXTERN SACarg *SACARGconvertFromBoolPointerVect (int *data, int dim, int *shape);
SAC_C_EXTERN SACarg *SACARGconvertFromCharPointerVect (char *data, int dim, int *shape);

/** <!-- ****************************************************************** -->
 * @brief Converts a scalar C value into an SACarg. The resulting SACarg
 *        has dimensionality 0 and shape [].
 *
 * @param value the value to wrap
 *
 * @return corresponding SACarg structure
 ******************************************************************************/
SAC_C_EXTERN SACarg *SACARGconvertFromIntScalar (int value);
SAC_C_EXTERN SACarg *SACARGconvertFromDoubleScalar (double value);
SAC_C_EXTERN SACarg *SACARGconvertFromFloatScalar (float value);
SAC_C_EXTERN SACarg *SACARGconvertFromBoolScalar (int value);
SAC_C_EXTERN SACarg *SACARGconvertFromCharScalar (char value);

/** <!-- ****************************************************************** -->
 * @brief Converts the given void pointer into an external SACarg with
 *        the given basetype. The valid basetypes are defined in the
 *        corresponding header file.
 *
 * @param basetype basetype of SACarg
 * @param data     actual data
 *
 * @return
 ******************************************************************************/
SAC_C_EXTERN SACarg *SACARGconvertFromVoidPointer (int basetype, void *data);

/** <!-- ****************************************************************** -->
 * @brief Converts an external SACarg with the given basetype into a
 *        void pointer.
 *
 * @param basetype the basetype of the contained data
 * @param arg      an external SACarg
 *
 * @return
 ******************************************************************************/
SAC_C_EXTERN void *SACARGconvertToVoidPointer (int basetype, SACarg *arg);

#define EXTERN_DECL(type, alias)                                                         \
    SAC_C_EXTERN type *SACARGconvertTo##alias##Array (SACarg *arg);                      \
    SAC_C_EXTERN SACarg *SACARGconvertFrom##alias##Pointer (type *data, int dim, ...);   \
    SAC_C_EXTERN SACarg *SACARGconvertFrom##alias##PointerVect (type *data, int dim,     \
                                                          int *shape);                   \
    SAC_C_EXTERN SACarg *SACARGconvertFrom##alias##Scalar (type value);

EXTERN_DECL (char, Byte)
EXTERN_DECL (short, Short)
EXTERN_DECL (long, Long)
EXTERN_DECL (long long, Longlong)
EXTERN_DECL (unsigned char, Ubyte)
EXTERN_DECL (unsigned short, Ushort)
EXTERN_DECL (unsigned int, Uint)
EXTERN_DECL (unsigned long, Ulong)
EXTERN_DECL (unsigned long long, Ulonglong)

/** <!--********************************************************************-->
 *
 * @fn void SAC_InitRuntimeSystem ( void);
 *
 * @brief Runtime initialization. Currently implemented only for MT.
 *
 *****************************************************************************/
SAC_C_EXTERN void SAC_InitRuntimeSystem (void);

/** <!--********************************************************************-->
 *
 * @fn void SAC_FreeRuntimeSystem (void);
 *
 * @brief Runtime freeing.
 *
 *****************************************************************************/
SAC_C_EXTERN void SAC_FreeRuntimeSystem (void);

/**
 * SAChive is an opaque type used to construct typed pointers.
 * A *hive* is a collection of bees, i.e. PTH threads or LPEL tasks,
 * depending on the threading backend.
 */
typedef struct SAC_SAChive SAChive;

/** <!--********************************************************************-->
 *
 * @fn SAChive *SAC_AllocHive( unsigned int num_bees, int num_schedulers,
 *                            const int *places)
 *
 * @brief Allocates a new hive and returns a handle to it.
 *        The hive is initially detached.
 *        The hive will contain new (num_bees-1) bees, because the calling context
 *        is used in the computations as well.
 *
 * @param num_bees        Number of bees in a hive, including the calling bee.
 * @param num_schedulers  Number of WL-task schedulers.
 *                        FIXME: Task schedulers are currently broken, hence
 *                        this may change....
 * @param places          An optional array of int[num_bees] size, specifying
 *                        placement for the bees. If NULL, a default placement
 *                        is used.
 *                        Note that places[0] corresponds to the queen bee
 *                        and hence probably won't be used.
 *                        PTH backend ignores the placement information.
 *                        LPEL backed uses it as a worker ID for the new tasks.
 * @param thdata          Backend-specific data.
 *
 * @return A handle to a new hive.
 *
 *****************************************************************************/
SAC_C_EXTERN SAChive *SAC_AllocHive (unsigned int num_bees, int num_schedulers,
                               const int *places, void *thdata);

/** <!--********************************************************************-->
 *
 * @fn void SAC_ReleaseHive(SAChive *h);
 *
 * @brief Releases a hive, terminating all its bees and freeing all memory.
 *
 * @param hive            Handle to a *detached* hive to be freed.
 *                        The handle is invalid after the call.
 *
 *****************************************************************************/
SAC_C_EXTERN void SAC_ReleaseHive (SAChive *hive);

/** <!--********************************************************************-->
 *
 * @fn void SAC_AttachHive(SAChive *h);
 *
 * @brief Attach a hive to the calling bee (thread/task).
 * If this is the first attach in this thread or task context a queen-bee
 * stub for the context is automatically allocated and stored in the corresponding
 * Thread Local Storage (PTH) or Task User Data (LPEL) facility.
 * Before terminating this thread/task the stub context has to be released
 * using SAC_ReleaseQueen().
 * Only a single hive can be attached to a given context at a time.
 *
 * @param hive            Handle to a hive that shall be attached to the calling
 *                        context (thread or task).
 *
 *****************************************************************************/
SAC_C_EXTERN void SAC_AttachHive (SAChive *hive);

/** <!--********************************************************************-->
 *
 * @fn SAChive *SAC_DetachHive(void);
 *
 * @brief Detach a hive from the calling context (thread or task) and return
 *        a handle to it. This is an inverse of SAC_AttachHive(), but the
 *        calling context's queen-bee stub is *not* released. That must
 *        be done using SAC_ReleaseQueen().
 *
 * @return Handle to a hive originally attached to the calling context.
 *         The hive can be re-attached to a different context or released.
 *
 *****************************************************************************/
SAC_C_EXTERN SAChive *SAC_DetachHive (void);

/** <!--********************************************************************-->
 *
 * @fn void SAC_ReleaseQueen(void);
 *
 * @brief Release the queen-bee stub of the calling context (thread or task).
 *        A hive still attached to the context will be detached and released
 *        as well.
 *        The Thread Local Storage / Task User Data slot of the calling context
 *        will be cleared. Subsequent call to SAC_AttachHive() in the context
 *        will create a new stub.
 *
 *****************************************************************************/
SAC_C_EXTERN void SAC_ReleaseQueen (void);

#endif /* _SAC_SACINTERFACE_H_ */
