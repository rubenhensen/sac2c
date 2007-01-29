/*
 *
 * $Log$
 * Revision 3.34  2005/09/14 19:57:20  sah
 * extended ND_IDXS2OFFSET to work with scalar offsets, as well.
 *
 * Revision 3.33  2004/11/21 22:04:36  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 3.32  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.31  2003/09/30 19:30:28  dkr
 * ND_CHECK__MIRROR removed
 *
 * Revision 3.30  2003/09/29 22:51:38  dkr
 * some icms removed/renamed/added
 *
 * Revision 3.29  2003/09/25 13:47:08  dkr
 * ASSIGN__DIMSHP replaced by UPDATE__DESC, UPDATE__MIRROR
 *
 * Revision 3.28  2003/09/20 14:23:15  dkr
 * prf ICMs moved to icm2c_prf.h
 *
 * Revision 3.27  2003/09/19 15:38:50  dkr
 * postfix _nt of varnames renamed into _NT
 *
 * Revision 3.26  2003/09/17 14:17:18  dkr
 * some function parameters renamed
 *
 * Revision 3.25  2003/09/17 12:58:18  dkr
 * postfixes _nt, _any renamed into _NT, _ANY
 *
 * Revision 3.24  2003/06/12 17:22:00  dkr
 * ICMs CREATE__VECT__... renamed into CREATE__ARRAY__...
 *
 * Revision 3.23  2002/10/29 19:10:28  dkr
 * signature of some ICMs modified
 *
 * Revision 3.22  2002/09/06 09:57:54  dkr
 * ifdef-instruction corrected
 *
 * Revision 3.21  2002/09/06 09:37:19  dkr
 * ND_IDXS2OFFSET added
 *
 * Revision 3.20  2002/08/05 18:22:02  dkr
 * ND_ASSIGN__SHAPE renamed into ND_ASSIGN__DIMSHP
 *
 * Revision 3.19  2002/08/03 03:16:30  dkr
 * ND_PRF_SEL__DIM icms removed
 *
 * Revision 3.18  2002/08/02 20:48:41  dkr
 * ..__DIM.. icms added
 *
 * Revision 3.17  2002/07/31 16:34:12  dkr
 * parameter 'copyfun' added for several ICMs
 *
 * Revision 3.16  2002/07/24 15:04:11  dkr
 * ND_VECT2OFFSET modified
 *
 * Revision 3.15  2002/07/12 18:55:09  dkr
 * first (almost) complete revision for new backend.
 * some shape computations are missing yet (but SCL, AKS should be
 * complete)
 *
 * Revision 3.14  2002/07/11 17:44:24  dkr
 * F_modarray completed
 *
 * Revision 3.13  2002/07/10 20:06:34  dkr
 * some bugs modified
 *
 * Revision 3.12  2002/07/10 19:26:32  dkr
 * F_modarray for new backend added
 *
 * Revision 3.7  2002/05/31 17:25:31  dkr
 * ICMs for new backend added
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
 * Add code for C macros for new backend support.
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

#ifndef _SAC_ICM2C_STD_H_
#define _SAC_ICM2C_STD_H_

extern void ICMCompileND_FUN_DEC (char *name, char *rettype_NT, int vararg_cnt,
                                  char **vararg);

extern void ICMCompileND_FUN_AP (char *name, char *retname, int vararg_cnt,
                                 char **vararg);

extern void ICMCompileND_FUN_RET (char *retname, int vararg_cnt, char **vararg);

extern void ICMCompileND_OBJDEF (char *var_NT, char *basetype, int sdim, int *shp);

extern void ICMCompileND_OBJDEF_EXTERN (char *var_NT, char *basetype, int sdim);

extern void ICMCompileND_DECL (char *var_NT, char *basetype, int sdim, int *shp);

extern void ICMCompileND_DECL_EXTERN (char *var_NT, char *basetype, int sdim);

extern void ICMCompileND_DECL__MIRROR (char *var_NT, int sdim, int *shp);

extern void ICMCompileND_DECL__MIRROR_PARAM (char *var_NT, int sdim, int *shp);

extern void ICMCompileND_DECL__MIRROR_EXTERN (char *var_NT, int sdim);

extern void ICMCompileND_CHECK_REUSE (char *to_NT, int to_sdim, char *from_NT,
                                      int from_sdim, char *copyfun);

extern void ICMCompileND_SET__SHAPE_id (char *to_NT, int to_sdim, char *shp_NT);

extern void ICMCompileND_SET__SHAPE_arr (char *to_NT, int dim, char **shp_ANY);

extern void ICMCompileND_REFRESH__MIRROR (char *var_NT, int sdim);

extern void ICMCompileND_ASSIGN (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                 char *copyfun);

extern void ICMCompileND_ASSIGN__DESC (char *to_NT, char *from_NT);

extern void ICMCompileND_ASSIGN__SHAPE (char *to_NT, int to_sdim, char *from_NT,
                                        int from_sdim);

extern void ICMCompileND_UPDATE__DESC (char *to_NT, int to_sdim, char *from_NT,
                                       int from_sdim);

extern void ICMCompileND_UPDATE__MIRROR (char *to_NT, int to_sdim, char *from_NT,
                                         int from_sdim);

extern void ICMCompileND_COPY (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                               char *copyfun);

extern void ICMCompileND_COPY__SHAPE (char *to_NT, int to_sdim, char *from_NT,
                                      int from_sdim);

extern void ICMCompileND_MAKE_UNIQUE (char *to_NT, int to_sdim, char *from_NT,
                                      int from_sdim, char *copyfun);

extern void ICMCompileND_CREATE__ARRAY__SHAPE (char *to_NT, int to_sdim, int dim,
                                               int *shp, int val_size, char **vals_ANY,
                                               int val0_sdim);

extern void ICMCompileND_CREATE__ARRAY__DATA (char *to_NT, int to_sdim, int val_size,
                                              char **vals_ANY, char *copyfun);

extern void ICMCompileND_VECT2OFFSET (char *off_NT, int from_size, char *from_NT,
                                      int shp_size, char **shp_ANY);

extern void ICMCompileND_IDXS2OFFSET (char *off_NT, int idxs_size, char **idxs_ANY,
                                      int shp_size, char **shpa_ANY);

#endif /* _SAC_ICM2C_STD_H_ */
