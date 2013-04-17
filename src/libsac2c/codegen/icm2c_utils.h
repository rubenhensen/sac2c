/*****************************************************************************
 *
 * file:   icm2c_utils.h
 *
 * prefix: ICU
 *
 * description:
 *
 *   This file contains definitions for ICM utility functions.
 *
 *
 *****************************************************************************/

#ifndef _SAC_ICM2C_UTILS_H_
#define _SAC_ICM2C_UTILS_H_

#include "types.h"

extern shape_class_t ICUGetShapeClass (char *var_NT);
extern hidden_class_t ICUGetHiddenClass (char *var_NT);
extern unique_class_t ICUGetUniqueClass (char *var_NT);

#endif /* _SAC_ICM2C_UTILS_H_ */
