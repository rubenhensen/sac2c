/*
 *
 */

#ifndef _sac_interface_h
#define _sac_interface_h

/* functions and datatype for c <-> SAC interface */

/* constants for mode */
typedef enum { SAC_CONSUME_ARGS, SAC_COPY_ARGS } SAC_reusetype;

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

/* functions to convert SAC_arg from and to c datatypes */
/* int */
extern SAC_arg SAC_IntArray2Sac (SAC_reusetype reuseflag, int *array, int dim, ...);
extern SAC_arg SAC_Int2Sac (int value);
extern int *SAC_Sac2IntArray (SAC_arg sa);
extern int SAC_Sac2Int (SAC_arg sa);

/* double */
extern SAC_arg SAC_DoubleArray2Sac (SAC_reusetype reuseflag, double *array, int dim, ...);
extern SAC_arg SAC_Double2Sac (double value);
extern double *SAC_Sac2DoubleArray (SAC_arg sa);
extern double SAC_Sac2Double (SAC_arg sa);

/* other types -> to be implemented */
#endif
