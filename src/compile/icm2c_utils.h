/*
 *
 * $Log$
 * Revision 1.4  2000/08/17 10:18:57  dkr
 * all the NT stuff is now in a separate modul (NameTuples.[ch])
 *
 * Revision 1.3  1999/06/25 15:24:08  rob
 * Don't gen if not TAGGED_ARRAYS
 *
 * Revision 1.2  1999/06/25 14:52:25  rob
 * Introduce definitions and utility infrastructure for tagged array support.
 *
 * Revision 1.1  1999/06/16 17:18:30  rob
 * Initial revision
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
 *   This file contains definitions for ICM utility functions.
 *
 *
 *****************************************************************************/

#ifndef _icm2c_utils_h
#define _icm2c_utils_h

#include "NameTuples.h"

extern data_class_t ICUNameClass (char *nt);
extern unq_class_t ICUUnqClass (char *nt);

#endif /* _icm2c_utils_h */
