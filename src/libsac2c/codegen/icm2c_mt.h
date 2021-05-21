/*****************************************************************************
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

extern void ICMCompileMT_SMART_BEGIN (int spmd_id);
extern void ICMCompileMT_SMART_END (void);
extern void ICMCompileMT_SMART_DATA_BEGIN (int data_size);
extern void ICMCompileMT_SMART_DATA_ADD (int input_size, int nr_threads);
extern void ICMCompileMT_SMART_DATA_END (void);

extern void ICMCompileMT_SMART_PROBLEM_SIZE (int problem_size);
extern void ICMCompileMT_SMART_VAR_PROBLEM_SIZE (char *mem_id);
extern void ICMCompileMT_SMART_EXPR_PROBLEM_SIZE_BEGIN (void);
extern void ICMCompileMT_SMART_EXPR_PROBLEM_SIZE_IxI (int inf, int sup, int operation);
extern void ICMCompileMT_SMART_EXPR_PROBLEM_SIZE_IxC (int inf, char *sup, int operation);
extern void ICMCompileMT_SMART_EXPR_PROBLEM_SIZE_CxI (char *inf, int sup, int operation);
extern void ICMCompileMT_SMART_EXPR_PROBLEM_SIZE_CxC (char *inf, char *sup,
                                                      int operation);
extern void ICMCompileMT_SMART_EXPR_PROBLEM_SIZE_END (int operation);

extern void ICMCompileMT_SPMDFUN_DECL (char *funname, unsigned int vararg_cnt, char **vararg);
extern void ICMCompileMT_SPMDFUN_DEF_BEGIN (char *funname, unsigned int vararg_cnt, char **vararg);
extern void ICMCompileMT_SPMDFUN_DEF_END (char *funname, unsigned int vararg_cnt, char **vararg);
extern void ICMCompileMT_SPMDFUN_RET (char *funname, unsigned int vararg_cnt, char **vararg);
extern void ICMCompileMT_SPMDFUN_AP (char *funname, unsigned int vararg_cnt, char **vararg);

extern void ICMCompileMT_MTFUN_DECL (char *funname, char *rettype_NT, unsigned int vararg_cnt,
                                     char **vararg);
extern void ICMCompileMT_MTFUN_DEF_BEGIN (char *funname, char *rettype_NT, unsigned int vararg_cnt,
                                          char **vararg);
extern void ICMCompileMT_MTFUN_DEF_END (char *funname, char *rettype_NT, unsigned int vararg_cnt,
                                        char **vararg);
extern void ICMCompileMT_MTFUN_RET (char *retname_NT, unsigned int vararg_cnt, char **vararg);
extern void ICMCompileMT_MTFUN_AP (char *funname, char *retname_NT, unsigned int vararg_cnt,
                                   char **vararg);

extern void ICMCompileMT_SPMD_FRAME_ELEMENT (char *funname, unsigned int vararg_cnt,
                                             char **vararg);
extern void ICMCompileMT_SPMD_BARRIER_ELEMENT (char *funname, unsigned int vararg_cnt,
                                               char **vararg);

#endif /* _SAC_ICM2C_MT_H_ */
