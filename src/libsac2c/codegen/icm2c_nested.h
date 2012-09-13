#ifndef _SAC_ICM2C_NESTED_H_
#define _SAC_ICM2C_NESTED_H_

extern void ICMCompileND_ENCLOSE (char *to_NT, int to_DIM, char *from_NT, int from_DIM);

extern void ICMCompileND_DISCLOSE (char *to_NT, int to_DIM, char *from_NT, int from_DIM);

extern void ICMCompileND_DECL_NESTED (char *var_NT, char *basetype, int sdim, int *shp);

#endif /* _SAC_ICM2C_NESTED_H_ */
