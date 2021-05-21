/*****************************************************************************
 *
 * file:   icm2c_fp.h
 *
 * prefix: ICMCompile
 *
 *
 * description:
 *
 *   This file contains the prototypes of those functions that actually
 *   implement the C implemented ICMs.
 *
 *****************************************************************************/

#ifndef _SAC_ICM2C_FP_H_
#define _SAC_ICM2C_FP_H_

#include "types.h"

extern void ICMCompileFP_SLOWCLONE_DECL (char *name, char *rettype_NT, unsigned int vararg_cnt,
                                         char **vararg);
extern void ICMCompileFP_SLOWCLONE_DEF_BEGIN (char *name, char *rettype_NT,
                                              unsigned int vararg_cnt, char **vararg);
extern void ICMCompileFP_FUN_RET (char *framename, char *retname, unsigned int vararg_cnt,
                                  char **vararg);

extern void ICMCompileFP_FUN_AP (char *framename, char *name, char *retname,
                                 unsigned int vararg_cnt, char **vararg);
#endif /* _SAC_ICM2C_FP_H_ */
