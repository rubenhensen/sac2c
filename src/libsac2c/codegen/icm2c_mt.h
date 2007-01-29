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

extern void ICMCompileMT_SPMD_FUN_DEC (char *funname, int vararg_cnt, char **vararg);
extern void ICMCompileMT_SPMD_FUN_RET (char *funname, int vararg_cnt, char **vararg);
extern void ICMCompileMT_SPMD_FUN_AP (char *funname, int vararg_cnt, char **vararg);

extern void ICMCompileMT_SPMD_FRAME_ELEMENT (char *funname, int vararg_cnt,
                                             char **vararg);
extern void ICMCompileMT_SPMD_BARRIER_ELEMENT (char *funname, int vararg_cnt,
                                               char **vararg);

extern void ICMCompileMT_CREATE_LOCAL_DESC (char *var_NT, int dim);

#endif /* _SAC_ICM2C_MT_H_ */
