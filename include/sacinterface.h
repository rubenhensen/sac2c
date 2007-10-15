/* $Id$ */

#ifndef _SAC_SACINTERFACE_H_
#define _SAC_SACINTERFACE_H_

/**
 * SACarg type
 */
typedef struct SAC_SACARG SACarg;

extern int SACARGgetDim (SACarg *arg);
extern int SACARGgetShape (SACarg *arg, int pos);
extern int SACARGgetBasetype (SACarg *arg);

extern SACarg *SACARGnewReference (SACarg *arg);

extern int *SACARGconvertToIntArray (SACarg *arg);
extern double *SACARGconvertToDoubleArray (SACarg *arg);
extern float *SACARGconvertToFloatArray (SACarg *arg);
extern int *SACARGconvertToBoolArray (SACarg *arg);
extern char *SACARGconvertToCharArray (SACarg *arg);

extern SACarg *SACARGconvertFromIntPointer (int *data, int dim, ...);
extern SACarg *SACARGconvertFromDoublePointer (double *data, int dim, ...);
extern SACarg *SACARGconvertFromFloatPointer (float *data, int dim, ...);
extern SACarg *SACARGconvertFromBoolPointer (int *data, int dim, ...);
extern SACarg *SACARGconvertFromCharPointer (char *data, int dim, ...);

extern SACarg *SACARGconvertFromIntPointerVect (int *data, int dim, int *shape);
extern SACarg *SACARGconvertFromDoublePointerVect (double *data, int dim, int *shape);
extern SACarg *SACARGconvertFromFloatPointerVect (float *data, int dim, int *shape);
extern SACarg *SACARGconvertFromBoolPointerVect (int *data, int dim, int *shape);
extern SACarg *SACARGconvertFromCharPointerVect (char *data, int dim, int *shape);

extern SACarg *SACARGconvertFromIntScalar (int value);
extern SACarg *SACARGconvertFromDoubleScalar (double value);
extern SACarg *SACARGconvertFromFloatScalar (float value);
extern SACarg *SACARGconvertFromBoolScalar (int value);
extern SACarg *SACARGconvertFromCharScalar (char value);

#endif /* _SAC_SACINTERFACE_H_ */
