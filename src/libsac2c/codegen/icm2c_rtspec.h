/*
 * $Id: main.c 16306 2009-08-10 14:08:31Z cg $
 */

/** <!--*******************************************************************-->
 *
 * @file icm2c_rtspec.h
 *
 * @brief Header file for the ICMCompile functions needed for runtime
 * specialization.
 *
 * @author tvd
 *
 ****************************************************************************/

#ifndef _SAC_ICM2C_RTSPEC_H_
#define _SAC_ICM2C_RTSPEC_H_

extern void ICMCompileWE_FUN_DEF_BEGIN (char *name, char *rettype_NT, int vararg_cnt,
                                        char **vararg);

extern void ICMCompileWE_FUN_DEF_END (char *name, char *rettype_NT, int vararg_cnt,
                                      char **vararg);

extern void ICMCompileWE_FUN_AP (char *name, char *rettype_NT, int vararg_cnt,
                                 char **vararg);

extern void ICMCompileWE_MODFUN_INFO (char *name, char *module);

extern void ICMCompileWE_SHAPE_ENCODE (int arg_cnt, char **arg);

extern void ICMCompileWE_NO_SHAPE_ENCODE (int arg_cnt);

#endif /* _SAC_ICM2C_RTSPEC_H_ */
