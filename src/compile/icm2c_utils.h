/*
 *
 * $Log$
 * Revision 3.5  2003/09/19 15:32:08  dkr
 * postfix _nt of varnames renamed into _NT
 *
 * Revision 3.4  2002/07/31 15:37:28  dkr
 * new hidden tag added
 *
 * Revision 3.3  2002/06/02 21:36:56  dkr
 * functions renamed
 *
 * Revision 3.2  2002/05/31 17:21:33  dkr
 * functions renamed
 *
 * Revision 3.1  2000/11/20 18:01:20  sacbase
 * new release made
 *
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

extern shape_class_t ICUGetShapeClass (char *var_NT);
extern hidden_class_t ICUGetHiddenClass (char *var_NT);
extern unique_class_t ICUGetUniqueClass (char *var_NT);

#endif /* _icm2c_utils_h */
