
/*
 *
 * $Log$
 * Revision 1.1  1999/06/16 17:18:30  rob
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   icm2c_utils.h
 *
 * prefix: ICU
 *
 * description:
 *
 *   This file contains definitions for enumerated types, etc., used by
 *   ICM utility functions.
 *
 *
 *****************************************************************************/

#ifndef _icm2c_utils_h
#define _icm2c_utils_h

/*
 * Enumerated types for DATA class and uniqueness class
 */

typedef enum { AKS, AKD, HID } data_class_t;

typedef enum { NUQ, UNQ } uniqueness_class_t;

extern data_class_t ICUNameClass (char *nt);
extern uniqueness_class_t ICUUniClass (char *nt);

/*
 * The following defines indicate the position of tags
 * within name tuples. They should be kept in synch with the
 * NAME_NAME, NAME_CLASS and NAME_UNI macros in sac_std.h
 */

#define NT_NAME_INDEX 0
#define NT_CLASS_INDEX 1
#define NT_UNI_INDEX 2

#endif /* _icm2c_utils_h */
