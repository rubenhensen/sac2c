/*
 *
 * $Log$
 * Revision 1.1  1998/04/25 16:20:35  sbs
 * Initial revision
 *
 *
 */

#ifndef _icm2c_std_h
#define _icm2c_std_h

extern void ICMCompileND_FUN_DEC (char *name, char *rettype, int narg, char **tyarg);
extern void ICMCompileND_FUN_AP (char *name, char *retname, int narg, char **arg);
extern void ICMCompileND_FUN_RET (char *retname, int narg, char **arg, node *arg_info);
extern void ICMCompileND_CREATE_CONST_ARRAY_S (char *name, int dim, char **s);
extern void ICMCompileND_CREATE_CONST_ARRAY_H (char *name, char *copyfun, int dim,
                                               char **A);
extern void ICMCompileND_CREATE_CONST_ARRAY_A (char *name, int length, int dim, char **s);
extern void ICMCompileND_KS_DECL_ARRAY (char *type, char *name, int dim, char **s);
extern void ICMCompileND_KS_DECL_GLOBAL_ARRAY (char *type, char *name, int dim, char **s);
extern void ICMCompileND_KD_DECL_EXTERN_ARRAY (char *basetype, char *name, int dim);
extern void ICMCompileND_KS_DECL_ARRAY_ARG (char *name, int dim, char **s);
extern void ICMCompileND_KD_SET_SHAPE (char *name, int dim, char **s);
extern void ICMCompileND_KD_PSI_CxA_S (int line, char *a, char *res, int dim, char **vi);
extern void ICMCompileND_KD_PSI_VxA_S (int line, char *a, char *res, int dim, char *v);
extern void ICMCompileND_KD_PSI_CxA_A (int line, int dima, char *a, char *res, int dimv,
                                       char **vi);
extern void ICMCompileND_KD_PSI_VxA_A (int line, int dima, char *a, char *res, int dimv,
                                       char *v);
extern void ICMCompileND_KD_TAKE_CxA_A (int dima, char *a, char *res, int dimv,
                                        char **vi);
extern void ICMCompileND_KD_DROP_CxA_A (int dima, char *a, char *res, int dimv,
                                        char **vi);
extern void ICMCompileND_KD_CAT_SxAxA_A (int dima, char **ar, char *res, int catdim);
extern void ICMCompileND_KD_ROT_CxSxA_A (int rotdim, char **numstr, int dima, char *a,
                                         char *res);
extern void ICMCompileND_PRF_MODARRAY_AxCxS_CHECK_REUSE (int line, char *res_type,
                                                         int dimres, char *res, char *old,
                                                         char **value, int dimv,
                                                         char **vi);
extern void ICMCompileND_PRF_MODARRAY_AxCxS (int line, char *res_type, int dimres,
                                             char *res, char *old, char **value, int dimv,
                                             char **vi);
extern void ICMCompileND_PRF_MODARRAY_AxVxS_CHECK_REUSE (int line, char *res_type,
                                                         int dimres, char *res, char *old,
                                                         char **value, int dim, char *v);
extern void ICMCompileND_PRF_MODARRAY_AxVxS (int line, char *res_type, int dimres,
                                             char *res, char *old, char **value, int dim,
                                             char *v);
extern void ICMCompileND_PRF_MODARRAY_AxCxA (int line, char *res_type, int dimres,
                                             char *res, char *old, char *val, int dimv,
                                             char **vi);
extern void ICMCompileND_PRF_MODARRAY_AxCxA_CHECK_REUSE (int line, char *res_type,
                                                         int dimres, char *res, char *old,
                                                         char *val, int dimv, char **vi);
extern void ICMCompileND_BEGIN_GENARRAY (char *res, int dimres, char *from, char *to,
                                         char *idx, int idxlen);
extern void ICMCompileND_BEGIN_MODARRAY (char *res, int dimres, char *a, char *from,
                                         char *to, char *idx, int idxlen);
extern void ICMCompileND_BEGIN_FOLDPRF (char *res, int dimres, char *from, char *to,
                                        char *idx, int idxlen, int n_neutral,
                                        char **neutral);
extern void ICMCompileND_BEGIN_FOLDFUN (char *res, int dimres, char *from, char *to,
                                        char *idx, int idxlen, int n_neutral,
                                        char **neutral);
extern void ICMCompileND_END_GENARRAY_S (char *res, int dimres, char **valstr);
extern void ICMCompileND_END_GENARRAY_A (char *res, int dimres, char *reta, int idxlen);
extern void ICMCompileND_END_MODARRAY_S (char *res, int dimres, char *a, char **valstr);
extern void ICMCompileND_END_FOLD (int idxlen);
extern void ICMCompileND_KS_VECT2OFFSET (char *name, int dim, int dims, char **s);
extern void ICMCompileND_END_MODARRAY_A (char *res, int dimres, char *a, char *reta,
                                         int idxlen);

#endif /* _icm2c_std_h */
