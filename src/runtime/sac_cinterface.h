/*
 * $Log$
 * Revision 1.4  2000/07/12 10:14:10  nmw
 * RCS-header added
 *
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
/* for int */
extern SAC_arg SAC_IntArray2Sac (SAC_reusetype reuseflag, int *array, int dim, ...);
extern SAC_arg SAC_Int2Sac (int value);
extern int *SAC_Sac2IntArray (SAC_reusetype reuseflag, SAC_arg sa);
extern int SAC_Sac2Int (SAC_arg sa);

/* for double */
extern SAC_arg SAC_DoubleArray2Sac (SAC_reusetype reuseflag, double *array, int dim, ...);
extern SAC_arg SAC_Double2Sac (double value);
extern double *SAC_Sac2DoubleArray (SAC_reusetype reuseflag, SAC_arg sa);
extern double SAC_Sac2Double (SAC_arg sa);

/* other types -> to be implemented */
#endif
