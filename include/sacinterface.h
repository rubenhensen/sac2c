/* $Id$ */

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
extern int SACARGgetDim (SACarg *arg);

/** <!-- ****************************************************************** -->
 * @brief Returns the shape of the given argument at the given position.
 *
 * @param arg SACarg (not consumed)
 * @param pos position starting from 0
 *
 * @return shape
 ******************************************************************************/
extern int SACARGgetShape (SACarg *arg, int pos);

/** <!-- ****************************************************************** -->
 * @brief Returns the basetype of the given argument.
 *
 * @param arg SACarg (not consumed)
 *
 * @return basetype
 ******************************************************************************/
extern int SACARGgetBasetype (SACarg *arg);

/** <!-- ****************************************************************** -->
 * @brief Returns a new reference to the given argument. This can be used
 *        to keep a handle to a datastructure passed to a SAC function and
 *        thus preventing it from beeing freed/reused.
 *
 * @param arg SACarg (not consumed)
 *
 * @return new SACarg containing the same data
 ******************************************************************************/
extern SACarg *SACARGnewReference (SACarg *arg);

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
extern int *SACARGconvertToIntArray (SACarg *arg);
extern double *SACARGconvertToDoubleArray (SACarg *arg);
extern float *SACARGconvertToFloatArray (SACarg *arg);
extern int *SACARGconvertToBoolArray (SACarg *arg);
extern char *SACARGconvertToCharArray (SACarg *arg);

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
extern SACarg *SACARGconvertFromIntPointer (int *data, int dim, ...);
extern SACarg *SACARGconvertFromDoublePointer (double *data, int dim, ...);
extern SACarg *SACARGconvertFromFloatPointer (float *data, int dim, ...);
extern SACarg *SACARGconvertFromBoolPointer (int *data, int dim, ...);
extern SACarg *SACARGconvertFromCharPointer (char *data, int dim, ...);

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
extern SACarg *SACARGconvertFromIntPointerVect (int *data, int dim, int *shape);
extern SACarg *SACARGconvertFromDoublePointerVect (double *data, int dim, int *shape);
extern SACarg *SACARGconvertFromFloatPointerVect (float *data, int dim, int *shape);
extern SACarg *SACARGconvertFromBoolPointerVect (int *data, int dim, int *shape);
extern SACarg *SACARGconvertFromCharPointerVect (char *data, int dim, int *shape);

/** <!-- ****************************************************************** -->
 * @brief Converts a scalar C value into an SACarg. The resulting SACarg
 *        has dimensionality 0 and shape [].
 *
 * @param value the value to wrap
 *
 * @return corresponding SACarg structure
 ******************************************************************************/
extern SACarg *SACARGconvertFromIntScalar (int value);
extern SACarg *SACARGconvertFromDoubleScalar (double value);
extern SACarg *SACARGconvertFromFloatScalar (float value);
extern SACarg *SACARGconvertFromBoolScalar (int value);
extern SACarg *SACARGconvertFromCharScalar (char value);

#endif /* _SAC_SACINTERFACE_H_ */
