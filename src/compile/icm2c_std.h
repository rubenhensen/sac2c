/*
 *
 * $Log$
 * Revision 3.10  2002/06/07 16:10:49  dkr
 * some new ICMs for TAGGED_ARRAYS added
 *
 * Revision 3.9  2002/06/06 18:22:31  dkr
 * some signatures modified
 *
 * Revision 3.8  2002/06/02 21:35:54  dkr
 * ICMs for TAGGED_ARRAYS added
 *
 * Revision 3.7  2002/05/31 17:25:31  dkr
 * some ICMs for TAGGED_ARRAYS added
 *
 * Revision 3.6  2002/05/03 14:01:18  dkr
 * some ICM args renamed
 *
 * Revision 3.5  2002/05/03 12:48:48  dkr
 * ND_KD_SET_SHAPE removed
 *
 * Revision 3.4  2002/03/07 20:08:33  dkr
 * ND_FUN_RET: parameter 'arg_info' removed
 *
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

extern void ICMCompileND_FUN_RET (char *retname, int narg, char **arg);

#ifdef TAGGED_ARRAYS

extern void ICMCompileND_OBJDEF (char *nt, char *basetype, int sdim, int *shp);

extern void ICMCompileND_OBJDEF_EXTERN (char *nt, char *basetype, int sdim);

extern void ICMCompileND_DECL (char *nt, char *basetype, int sdim, int *shp);

extern void ICMCompileND_DECL_EXTERN (char *nt, char *basetype, int sdim);

extern void ICMCompileND_DECL_MIRROR (char *nt, int sdim, int *shp);

extern void ICMCompileND_DECL_MIRROR_PARAM (char *nt, int sdim, int *shp);

extern void ICMCompileND_DECL_MIRROR_EXTERN (char *nt, int sdim);

extern void ICMCompileND_SET_SHP (char *nt, int sdim, int *shp);

extern void ICMCompileND_REFRESH_MIRROR (char *nt, int sdim);

extern void ICMCompileND_CHECK_REUSE (char *nt);

extern void ICMCompileND_ALLOC (char *nt, char *basetype, int sdim, int *shp, int rc);

extern void ICMCompileND_ALLOC_PLACE (char *nt, char *basetype, int sdim, int *shp,
                                      int rc);

extern void ICMCompileND_ASSIGN (char *to_nt, int to_sdim, char *from_nt);

extern void ICMCompileND_ASSIGN_DATA (char *to_nt, int to_sdim, char *from_nt);

extern void ICMCompileND_ASSIGN_MIRROR (char *to_nt, int to_sdim, char *from_nt);

extern void ICMCompileND_COPY (char *to_nt, char *from_nt);

extern void ICMCompileND_MAKE_UNIQUE (char *to_nt, char *from_nt, char *basetype);

extern void ICMCompileND_ASSIGN_CONST_VECT (char *name, int len, char **s);

extern void ICMCompileND_PRF_SHAPE (char *to_nt, char *from_nt, int from_sdim);

#else /* TAGGED_ARRAYS */

extern void ICMCompileND_KS_DECL_GLOBAL_ARRAY (char *basetype, char *name, int dim,
                                               char **s);

extern void ICMCompileND_KD_DECL_EXTERN_ARRAY (char *basetype, char *name, int dim);

extern void ICMCompileND_KS_DECL_ARRAY (char *basetype, char *name, int dim, char **s);

extern void ICMCompileND_KS_DECL_ARRAY_ARG (char *name, int dim, char **s);

extern void ICMCompileND_CREATE_CONST_ARRAY_S (char *name, int len, char **s);

extern void ICMCompileND_CREATE_CONST_ARRAY_H (char *name, char *copyfun, int len,
                                               char **A);

extern void ICMCompileND_CREATE_CONST_ARRAY_A (char *name, int len2, int len1, char **s);

#endif /* TAGGED_ARRAYS */

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

extern void ICMCompileND_PRF_MODARRAY_AxCxS (char *res_btype, int dimres, char *res,
                                             char *old, char **value, int dimv,
                                             char **vi);

extern void ICMCompileND_PRF_MODARRAY_AxVxS (char *res_btype, int dimres, char *res,
                                             char *old, char **value, int dim, char *v);

extern void ICMCompileND_PRF_MODARRAY_AxCxA (char *res_btype, int dimres, char *res,
                                             char *old, char *val, int dimv, char **vi);

extern void ICMCompileND_PRF_MODARRAY_AxVxA (char *res_btype, int dimres, char *res,
                                             char *old, char *val, int dim, char *v);

extern void ICMCompileND_KS_VECT2OFFSET (char *off_name, char *arr_name, int dim,
                                         int dims, char **s);

#endif /* _icm2c_std_h */
