/*
 *
 * $Log$
 * Revision 3.2  2002/04/30 08:37:00  dkr
 * no changes done
 *
 * Revision 3.1  2000/11/20 18:02:12  sacbase
 * new release made
 *
 * Revision 1.6  2000/08/04 09:51:23  nmw
 * added documentation of the c interface.
 *
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

/*
 * Using the SAC<->C interface
 * ---------------------------
 *
 * The c interface of SAC allows you to use your SAC-coded module in
 * your c program. When you create your SAC-module you might use all
 * advantages of this powerful language (e.g. arrays, special
 * optimizations, shape invariant functions, the module-system, ...).
 *
 * Your SAC-module is exported as a c library using the '-genlib c'
 * switch of sac2c when comping the module. You will get from your module
 * XXX.sac a c library libXXX.a and a headerfile XXX.h documenting the
 * functions you can use. In this headerfile there is a comment for every
 * function in the library what kind of arguments the function can accept.
 * This deals with the overloading possibility of SAC.
 * All return types of a SAC-function are implemented as reference parameters
 * in the interface function.
 *
 * example:
 *  the SAC-function in myMod.sac:
 *    int, int[], int myTestFun(double[], int[])
 *
 *  will be exported as:
 *    SAC_myMod_myTestFun_3_2(SAC_arg *out1, SAC_arg *out2, SAC_arg *out3,
 *                           SAC_arg in1, SAC_arg_in2)
 *
 * you can call this functions with all arguments matching the specialization
 * documented in the library-header-file.
 *
 *
 * If you want to use any SAC-module in your c program, you have you setup
 * the infrastructure needed by SAC-modules. To do this, you call the
 * SAC_InitRuntimeSystem() funtion as FIRST statement in your main program.
 * The last statement before returning from main() should be a call to
 * SAC_FreeRuntimeSystem(), which cleans up internal data structures.
 * Additionally you have to link your executable with the SAC-library and
 * the necessary parts of the SAC-runtime system. The needed files are
 * listed in the library headerfile with their location and compiler/linker
 * command line. You might adjust your makefile with these options.
 *
 * example:
 *  if you have compiled your program like this
 *    gcc -o myPrg myPrg.c
 *
 *  and you now want to use the SAC-library mySACMod, you call your compiler
 *    gcc -I$SACBASE/runtime -L$SACBASE/runtime -L.
 *        -o myPrg myPrg.c -lmySACMod -lsac
 *
 *
 * Because of the powerful type-system in SAC you cannot directly use
 * your c data in the sac-functions (e.g. arrays). So you have to
 * convert your c data in sac data (SAC_arg). This interface provides the
 * needed convert functions for simple-types and array-types. User defined
 * types from a SAC-module can be used as arguments to call other
 * SAC-functions expecting this type, but they are only an abstract
 * datatype, which is not accessable. The supported basic types are
 * int, long, float, double and char.
 *
 * convert functions: here for type ???
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
 * example: SAC_IntArray2Sac(SAC_COPY_ARG, cvar, 3, 2, 5, 2)
 *  to convert your c integer array of dimension 3 with the shape [2,5,2]
 *  to an SAC_arg.
 *
 * remarks:
 *   When reuseflag is set to SAC_CONSUME_ARG, the array is shared internally,
 *   between c-code and SAC. Therefore you are not allowed to access a c array
 *   anymore when convert it to SAC_arg!!! DO NOT FREE such a array!!!
 *   On the other hand you are not allowed to access the SAC_arg anymore,
 *   when you have converted it to a c-array using SAC_CONSUME_ARG. And you
 *   HAVE TO FREE this array.
 *   When reuseflag is set to SAC_COPY_ARG, then the argument array is not
 *   touched and you might use it in the future. This is valid for both
 *   directions. A c array you have to free on your own.
 *
 *
 * Due to the optimization techniques sac2c uses, only functions
 * with arguments of simple types and arrays with fixed shapes can be
 * exported to a c library. But you do not have to create a function
 * for each array shape you want to use! You can specify a generic
 * SAC-function in your module and additionally you create a
 * specialization file (called XXX.spec in a format similar to the one of
 * SAC declaration files). In this file you can specify all prototypes
 * of the specialized SAC-functions of you generic function sac2c
 * should create for you.
 *
 * Example:
 * Your SAC-module mySACMod.sac contains a generic function
 *   int[] addToArray(int[] a, int value)
 *
 * Your specialization file mySACMod.spec might look like this
 *   ModuleSpec SACMod :
 *   own:
 *   {
 *    functions:
 *     int[] addToArray(int[10] a, int value);
 *     int[] addToArray(int[5,5] a, int value);
 *     int[] addToArray(int[2,5,2] a, int value);
 *   }
 *
 * sac2c will creates the specified specializations and exports them
 * to your c library.
 *
 *
 * A special feature of SAC is the refcounting of data. In a SAC-program
 * the compiler checks when to release a data item. If you use a SAC-module
 * on its own (e.g. from a c program), the initial refcounter of a data item
 * is set to 1. Do every data is valid only for one function call to a
 * SAC-function. If you want to use some data more than one time, you can
 * increase the reference counter of this item by using the
 *   SAC_SetRefcounter(SAC_arg sa, int RC)
 * function to set the refcounter of sa to RC. If you call a SAC-function
 * with arguments containing no valid data, a runtime error will occur.
 * You can check you SAC_arg with the interface functions SAC_isValid() or
 * SAC_GetRefcounter().
 *
 *
 * special features:
 * - private heap manager
 *   The SAC-runtime system supports a special heapmanager optimized for
 *   performance. It is used by default and can be disabled by the '-noPHM'
 *   switch. When using this heapmanager, you have to link with the
 *   '-lsac_heapmgr' option. There is a diagnostic version too, which is
 *   enabled by the switch '-check h' and is linked as '-lsac_heapmgr_diag'.
 * - profiling
 *   the SAC-part of your program can be profiled unsing the
     '-profile' switch.
 *
 * known limitations:
 * - the multithreading feature of SAC is (yet) only available for
 *   SAC-programs and not for single modules. Therefore you cannot use
 *   the '-mt' switch at all when compiling a SAC-module. Maybe this
 *   feature will be implemented in the future.
 * - you have to recompile the library if you need additional specializations
 *   of your SAC-functions.
 *
 * See also: the provided demos using this interface
 */

#ifndef _sac_interface_h
#define _sac_interface_h

/*
 * functions and datatype for c <-> SAC interface
 */

/* constants for mode */
typedef enum { SAC_CONSUME_ARG, SAC_COPY_ARG } SAC_reusetype;

/* data type */
typedef struct SAC_ARG_STRUCT *SAC_arg;

/*
 * interface functions
 */

/* do some init procedures for the SAC runtime system
 * this has to be the first function called in every c programm using
 * this sac-interface
 */
extern void SAC_InitRuntimeSystem ();

/* do some cleanup after running the SAC Module */
extern void SAC_FreeRuntimeSystem ();

/* utilities for SAC_arg data type */
extern int SAC_isValid (SAC_arg sa);
extern int SAC_GetDim (SAC_arg sa);
extern int SAC_GetShape (SAC_arg sa, int pos);

/* functions concerning the refcounting */
extern void SAC_IncRC (SAC_arg sa, int no);
extern void SAC_DecRC (SAC_arg sa, int no);

/* functions to convert SAC_arg from and to c datatypes */
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

#endif /* _sac_interface_h */
