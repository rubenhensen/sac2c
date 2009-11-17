/*****************************************************************************
 *
 * $Id$
 *
 * file:   icm2c_mt.h
 *
 * prefix: ICMCompile
 *
 * Revision 1.1  1998/05/13 07:22:57  cg
 * Initial revision
 *
 * description:
 *
 *   This file contains the prototypes of those functions that actually
 *   implement the C implemented ICMs.
 *
 *****************************************************************************/

#ifndef _SAC_ICM2C_MT_H_
#define _SAC_ICM2C_MT_H_

#include "types.h"

extern void ICMCompileMT_SPMDFUN_DECL (char *funname, int vararg_cnt, char **vararg);
extern void ICMCompileMT_SPMDFUN_DEF_BEGIN (char *funname, int vararg_cnt, char **vararg);
extern void ICMCompileMT_SPMDFUN_DEF_END (char *funname, int vararg_cnt, char **vararg);
extern void ICMCompileMT_SPMDFUN_RET (char *funname, int vararg_cnt, char **vararg);
extern void ICMCompileMT_SPMDFUN_AP (char *funname, int vararg_cnt, char **vararg);

extern void ICMCompileMT_MTFUN_DECL (char *funname, char *rettype_NT, int vararg_cnt,
                                     char **vararg);
extern void ICMCompileMT_MTFUN_DEF_BEGIN (char *funname, char *rettype_NT, int vararg_cnt,
                                          char **vararg);
extern void ICMCompileMT_MTFUN_DEF_END (char *funname, char *rettype_NT, int vararg_cnt,
                                        char **vararg);
extern void ICMCompileMT_MTFUN_RET (char *retname_NT, int vararg_cnt, char **vararg);
extern void ICMCompileMT_MTFUN_AP (char *funname, char *retname_NT, int vararg_cnt,
                                   char **vararg);

extern void ICMCompileMT_SPMD_FRAME_ELEMENT (char *funname, int vararg_cnt,
                                             char **vararg);
extern void ICMCompileMT_SPMD_BARRIER_ELEMENT (char *funname, int vararg_cnt,
                                               char **vararg);

#endif /* _SAC_ICM2C_MT_H_ */
