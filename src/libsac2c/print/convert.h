#ifndef _SAC_CONVERT_H_
#define _SAC_CONVERT_H_

#include "types.h"

extern char *CVfloatvec2String (floatvec val);
extern char *CVtype2String (ntype *type, int flag, bool all);
extern char *CVdouble2String (double);
extern char *CVfloat2String (float);
extern char *CVfloatvec2String (floatvec val);
extern char *CVbasetype2String (simpletype type);
extern char *CVbasetype2ShortString (simpletype type);
extern char *CVshpseg2String (int dim, shpseg *shape);
extern char *CVintBytes2String (size_t bytes);

#endif /* _SAC_CONVERT_H_ */
