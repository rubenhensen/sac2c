/*
 *
 * $Log$
 * Revision 3.3  2001/12/21 13:33:02  dkr
 * ICMs ..._CHECK_REUSE removed
 *
 * Revision 3.2  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.1  2000/11/20 18:01:18  sacbase
 * new release made
 *
 * Revision 2.5  2000/07/24 15:07:03  dkr
 * redundant parameter 'line' removed from ICMs for array-prfs
 *
 * Revision 2.4  2000/07/06 12:26:18  dkr
 * prototypes for old with-loop removed
 *
 * Revision 2.3  1999/06/25 14:52:25  rob
 * Introduce definitions and utility infrastructure for tagged array support.
 *
 * Revision 2.2  1999/06/16 17:11:23  rob
 * Add code for C macros for TAGGED ARRAY support.
 * These are intended to eventually supplant the extant
 * ARRAY macros.
 *
 * Revision 2.1  1999/02/23 12:42:42  sacbase
 * new release made
 *
 * Revision 1.3  1999/01/22 14:32:25  sbs
 * ND_PRF_MODARRAY_AxVxA_CHECK_REUSE and
 * ND_PRF_MODARRAY_AxVxA
 * added.
 *
 * Revision 1.2  1998/08/06 17:19:04  dkr
 * changed signature of ICM ND_KS_VECT2OFFSET
 *
 * Revision 1.1  1998/04/25 16:20:35  sbs
 * Initial revision
 *
 */

#ifndef _icm2c_std_h
#define _icm2c_std_h

#include "types.h"

extern void ICMCompileND_FUN_DEC (char *name, char *rettype, int narg, char **tyarg);

extern void ICMCompileND_FUN_AP (char *name, char *retname, int narg, char **arg);

extern void ICMCompileND_FUN_RET (char *retname, int narg, char **arg, node *arg_info);

extern void ICMCompileND_CREATE_CONST_ARRAY_S (char *name, int dim, char **s);

extern void ICMCompileND_CREATE_CONST_ARRAY_H (char *name, char *copyfun, int dim,
                                               char **A);

extern void ICMCompileND_CREATE_CONST_ARRAY_A (char *name, int length, int dim, char **s);

#ifdef TAGGED_ARRAYS
extern void ICMCompileND_DECL_AKS (char *type, char *nt, int dim, char **s);
#else  /* TAGGED_ARRAYS */
extern void ICMCompileND_KS_DECL_ARRAY (char *type, char *name, int dim, char **s);
#endif /* TAGGED_ARRAYS */

extern void ICMCompileND_KS_DECL_GLOBAL_ARRAY (char *type, char *name, int dim, char **s);

extern void ICMCompileND_KD_DECL_EXTERN_ARRAY (char *basetype, char *name, int dim);

extern void ICMCompileND_KS_DECL_ARRAY_ARG (char *name, int dim, char **s);

extern void ICMCompileND_KD_SET_SHAPE (char *name, int dim, char **s);

extern void ICMCompileND_KD_SEL_CxA_S (char *a, char *res, int dim, char **vi);

extern void ICMCompileND_KD_SEL_CxA_A (int dima, char *a, char *res, int dimv, char **vi);

extern void ICMCompileND_KD_SEL_VxA_S (char *a, char *res, int dim, char *v);

extern void ICMCompileND_KD_SEL_VxA_A (int dima, char *a, char *res, int dimv, char *v);

extern void ICMCompileND_KD_TAKE_CxA_A (int dima, char *a, char *res, int dimv,
                                        char **vi);

extern void ICMCompileND_KD_DROP_CxA_A (int dima, char *a, char *res, int dimv,
                                        char **vi);

extern void ICMCompileND_KD_CAT_SxAxA_A (int dima, char **ar, char *res, int catdim);

extern void ICMCompileND_KD_ROT_CxSxA_A (int rotdim, char **numstr, int dima, char *a,
                                         char *res);

extern void ICMCompileND_PRF_MODARRAY_AxCxS (char *res_type, int dimres, char *res,
                                             char *old, char **value, int dimv,
                                             char **vi);

extern void ICMCompileND_PRF_MODARRAY_AxVxS (char *res_type, int dimres, char *res,
                                             char *old, char **value, int dim, char *v);

extern void ICMCompileND_PRF_MODARRAY_AxCxA (char *res_type, int dimres, char *res,
                                             char *old, char *val, int dimv, char **vi);

extern void ICMCompileND_PRF_MODARRAY_AxVxA (char *res_type, int dimres, char *res,
                                             char *old, char *val, int dim, char *v);

extern void ICMCompileND_KS_VECT2OFFSET (char *off_name, char *arr_name, int dim,
                                         int dims, char **s);

#endif /* _icm2c_std_h */
