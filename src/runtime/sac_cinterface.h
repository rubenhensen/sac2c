/*
 * $Log$
 * Revision 1.5  2000/07/13 09:45:25  nmw
 * convert functions for double, int, float, char added
 *
 * Revision 1.4  2000/07/12 10:14:10  nmw
 * RCS-header added
 *
 * Revision 1.3  2000/07/07 15:32:34  nmw
 * utility functions added
 *
 * Revision 1.2  2000/07/06 15:53:36  nmw
 * convert functions changed to var_args parameters
 *
 * Revision 1.1  2000/07/05 12:51:30  nmw
 * Initial revision
 *
 */

#ifndef _sac_interface_h
#define _sac_interface_h

/* functions and datatype for c <-> SAC interface */

/* constants for mode */
typedef enum { SAC_CONSUME_ARG, SAC_COPY_ARG } SAC_reusetype;

/* data type */
typedef struct SAC_ARG_STRUCT *SAC_arg;

/*
 * interface functions
 */

/*
 * do some init procedures for the SAC runtime system
 * this has to be the first function called in every c programm using
 * this sac-interface
 */
extern void SAC_InitRuntimeSystem ();

/* do some cleanup after running the SAC Module         */
extern void SAC_FreeRuntimeSystem ();

/* utilities for SAC_arg data type */
extern int SAC_isValid (SAC_arg sa);
extern int SAC_GetDim (SAC_arg sa);
extern int SAC_GetShape (SAC_arg sa, int pos);
extern int SAC_GetRefcounter (SAC_arg sa);
extern int SAC_SetRefcounter (SAC_arg sa, int newrc);

/* functions to convert SAC_arg from and to c datatypes */

/******************************************************************************
 *
 * functions: here for type ???
 *   SAC_arg  SAC_???Array2Sac(SAC_reusetype reuseflag,??? *array,int dim,...)
 *   SAC_arg  SAC_???2Sac(??? value)
 *   ???     *SAC_Sac2IntArray(SAC_reusetype reuseflag, SAC_arg sa)
 *   ???      SAC_Sac2Int(SAC_arg sa)
 *
 * description:
 *   converts c datatype to SAC_arg datatype and vice versa.
 *   for every basic type there are 4 functions:
 *         simpletype -> SAC_arg
 *         arraytype  -> SAC_arg
 *         SAC_arg    -> simpletype
 *         SAC_arg    -> arraytype
 *
 *   for the arraytype to SAC_arg you have to specify the dimension and its
 *   shape (as varargs after the dimension).
 *
 * remarks:
 *   When reuseflag is set to SAC_CONSUME_ARG, the array is shared internally,
 *   between c-code and SAC. Therefore you are not allowed to access a c array
 *   any more when convert it to SAC_arg!!! DO NOT FREE such a array!!!
 *   On the other hand you are not allowed to access the SAC_arg any more,
 *   when you have converted it to a c-array using SAC_CONSUME_ARG. And you
 *   HAVE TO FREE this array.
 *   When reuseflag is set to SAC_COPY_ARG, then the argument array is not
 *   touched and you might use it in the future. This is valid for both
 *   directions. A c-array you have to free on your own.
 *
 ******************************************************************************/

/* for int */
extern SAC_arg SAC_IntArray2Sac (SAC_reusetype reuseflag, int *array, int dim, ...);
extern SAC_arg SAC_Int2Sac (int value);
extern int *SAC_Sac2IntArray (SAC_reusetype reuseflag, SAC_arg sa);
extern int SAC_Sac2Int (SAC_arg sa);

/* for long */
extern SAC_arg SAC_LongArray2Sac (SAC_reusetype reuseflag, long *array, int dim, ...);
extern SAC_arg SAC_Long2Sac (long value);
extern long *SAC_Sac2LongArray (SAC_reusetype reuseflag, SAC_arg sa);
extern long SAC_Sac2Long (SAC_arg sa);

/* for double */
extern SAC_arg SAC_DoubleArray2Sac (SAC_reusetype reuseflag, double *array, int dim, ...);
extern SAC_arg SAC_Double2Sac (double value);
extern double *SAC_Sac2DoubleArray (SAC_reusetype reuseflag, SAC_arg sa);
extern double SAC_Sac2Double (SAC_arg sa);

/* for float */
extern SAC_arg SAC_FloatArray2Sac (SAC_reusetype reuseflag, float *array, int dim, ...);
extern SAC_arg SAC_Float2Sac (float value);
extern float *SAC_Sac2FloatArray (SAC_reusetype reuseflag, SAC_arg sa);
extern float SAC_Sac2Float (SAC_arg sa);

/* for char */
extern SAC_arg SAC_CharArray2Sac (SAC_reusetype reuseflag, char *array, int dim, ...);
extern SAC_arg SAC_Char2Sac (char value);
extern char *SAC_Sac2CharArray (SAC_reusetype reuseflag, SAC_arg sa);
extern char SAC_Sac2Char (SAC_arg sa);

#endif
