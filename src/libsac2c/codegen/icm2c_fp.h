/** <!--*******************************************************************-->
 *
 * @file icm2c_fp.h
 *
 * @brief Header file for the ICMCompile functions needed for functional
 * parallelism
 *
 * @author avr
 *
 ****************************************************************************/

#ifndef _SAC_ICM2C_FP_H_
#define _SAC_ICM2C_FP_H_

extern void ICMCompileFP_FUN_AP (char *name, char *rettype_NT, int vararg_cnt,
                                 char **vararg);

#endif /* _SAC_ICM2C_FP_H_ */
