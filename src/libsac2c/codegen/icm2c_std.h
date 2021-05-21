#ifndef _SAC_ICM2C_STD_H_
#define _SAC_ICM2C_STD_H_

extern void ICMCompileND_FUN_DECL (char *name, char *rettype_NT, unsigned int vararg_cnt,
                                   char **vararg);

extern void ICMCompileND_DISTMEM_FUN_DECL_WITH_SIDE_EFFECTS (char *name, char *rettype_NT,
                                                             unsigned int vararg_cnt,
                                                             char **vararg);

extern void ICMCompileND_FUN_DEF_BEGIN (char *name, char *rettype_NT, unsigned int vararg_cnt,
                                        char **vararg);

extern void ICMCompileND_FUN_DEF_END (char *name, char *rettype_NT, unsigned int vararg_cnt,
                                      char **vararg);
extern void ICMCompileMUTC_THREADFUN_DECL (char *name, char *rettype_NT, unsigned int vararg_cnt,
                                           char **vararg);
extern void ICMCompileMUTC_SPAWNFUN_DECL (char *name, char *rettype_NT, unsigned int vararg_cnt,
                                          char **vararg);
extern void ICMCompileMUTC_THREADFUN_DEF_BEGIN (char *name, char *rettype_NT,
                                                unsigned int vararg_cnt, char **vararg);
extern void ICMCompileMUTC_SPAWNFUN_DEF_BEGIN (char *name, char *rettype_NT,
                                               unsigned int vararg_cnt, char **vararg);
extern void ICMCompileND_FUN_AP (char *name, char *retname, unsigned int vararg_cnt,
                                 char **vararg);

extern void ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS (unsigned int vararg_NT_cnt,
                                                           char **vararg_NT,
                                                           char *rettype, char *ret_NT,
                                                           char *name, char *retname,
                                                           unsigned int vararg_cnt, char **vararg);
extern void ICMCompileMUTC_THREADFUN_AP (char *name, char *retname, unsigned int vararg_cnt,
                                         char **vararg);
extern void ICMCompileMUTC_SPAWNFUN_AP (char *syncid, char *place, char *name,
                                        char *retname, unsigned int vararg_cnt, char **vararg);
extern void ICMCompileMUTC_FUNTHREADFUN_AP (char *name, char *retname, unsigned int vararg_cnt,
                                            char **vararg);
extern void ICMCompileND_FUN_RET (char *retname, unsigned int vararg_cnt, char **vararg);

extern void ICMCompileMUTC_THREADFUN_RET (char *retname, unsigned int vararg_cnt, char **vararg);

extern void ICMCompileND_OBJDEF (char *var_NT, char *basetype, int sdim, int *shp);

extern void ICMCompileND_OBJDEF_EXTERN (char *var_NT, char *basetype, int sdim);

extern void ICMCompileND_DECL (char *var_NT, char *basetype, int sdim, int *shp);

extern void ICMCompileND_DSM_DECL (char *var_NT, char *basetype, int sdim, int *shp);

extern void ICMCompileND_DECL_EXTERN (char *var_NT, char *basetype, int sdim);

extern void ICMCompileND_DECL__MIRROR (char *var_NT, int sdim, int *shp);

extern void ICMCompileND_DECL__MIRROR_PARAM (char *var_NT, int sdim, int *shp);

extern void ICMCompileND_DECL__MIRROR_EXTERN (char *var_NT, int sdim);

extern void ICMCompileND_CHECK_REUSE (char *to_NT, int to_sdim, char *from_NT,
                                      int from_sdim, char *copyfun);

extern void ICMCompileND_CHECK_RESIZE (char *to_NT, int to_sdim, char *from_NT,
                                       int from_sdim, char *copyfun);

extern void ICMCompileND_SET__SHAPE_id (char *to_NT, int to_sdim, char *shp_NT);

extern void ICMCompileND_SET__SHAPE_arr (char *to_NT, int dim, char **shp_ANY);

extern void ICMCompileND_REFRESH__MIRROR (char *var_NT, int sdim);

extern void ICMCompileND_ASSIGN (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                 char *copyfun);

extern void ICMCompileND_COPY__DESC_DIS_FIELDS (char *to_NT, char *from_NT);

extern void ICMCompileND_ASSIGN__DESC (char *to_NT, char *from_NT);

extern void ICMCompileND_ASSIGN__SHAPE (char *to_NT, int to_sdim, char *from_NT,
                                        int from_sdim);

extern void ICMCompileND_UPDATE__DESC (char *to_NT, int to_sdim, char *from_NT,
                                       int from_sdim);

extern void ICMCompileND_UPDATE__MIRROR (char *to_NT, int to_sdim, char *from_NT,
                                         int from_sdim);

extern void ICMCompileND_COPY (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                               char *basetype, char *copyfun);

extern void ICMCompileND_COPY__SHAPE (char *to_NT, int to_sdim, char *from_NT,
                                      int from_sdim);

extern void ICMCompileND_MAKE_UNIQUE (char *to_NT, int to_sdim, char *from_NT,
                                      int from_sdim, char *basetype, char *copyfun);

extern void ICMCompileND_CREATE__ARRAY__SHAPE (char *to_NT, int to_sdim, int dim,
                                               int *shp, int val_size, char **vals_ANY,
                                               int val0_sdim);

extern void ICMCompileND_CREATE__ARRAY__DATA (char *to_NT, int to_sdim, int val_size,
                                              char **vals_ANY, char *copyfun);

extern void ICMCompileND_VECT2OFFSET_arr (char *off_NT, int from_size, char *from_NT,
                                          int shp_size, char **shp_ANY);

extern void ICMCompileND_VECT2OFFSET_id (char *off_NT, int from_size, char *from_NT,
                                         int shp_size, char *shp_NT);

extern void ICMCompileND_IDXS2OFFSET_arr (char *off_NT, int idxs_size, char **idxs_ANY,
                                          int shp_size, char **shp_ANY);

extern void ICMCompileND_IDXS2OFFSET_id (char *off_NT, int idxs_size, char **idxs_ANY,
                                         int shp_size, char *shp_NT);

extern void ICMCompileND_ARRAY_IDXS2OFFSET_id (char *off_NT, int idxs_size,
                                               char **idxs_ANY, int arr_size,
                                               char *arr_NT);

extern void ICMCompileND_ARRAY_VECT2OFFSET_id (char *off_NT, int from_size, char *from_NT,
                                               int arr_dim, char *arr_NT);

extern void ICMCompileND_UNSHARE (char *va_NT, int va_sdim, char *viv_NT, int viv_sdim,
                                  char *basetype, char *copyfun);

#endif /* _SAC_ICM2C_STD_H_ */
