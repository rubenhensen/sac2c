/*
 * $Log$
 * Revision 1.4  2004/11/22 18:55:29  cg
 * Moved all definitions/declarations of global variables to globals.mac
 *
 * Revision 1.3  2004/11/22 11:27:04  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 1.2  2001/03/22 14:27:31  nmw
 * functions to convert float, bool, char added
 *
 * Revision 1.1  2001/03/02 14:33:03  sbs
 * Initial revision
 *
 * Revision 3.1  2000/11/20 18:00:05  sacbase
 * new release made
 *
 * Revision 1.1  1999/10/19 13:02:59  sacbase
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_CV2SCALAR_H_
#define _SAC_CV2SCALAR_H_

#include "types.h"

extern node *COCv2Num (void *elems, int offset);
extern node *COCv2Double (void *elems, int offset);
extern node *COCv2Bool (void *elems, int offset);
extern node *COCv2Float (void *elems, int offset);
extern node *COCv2Char (void *elems, int offset);
extern node *COCv2ScalarDummy (void *elems, int offset);

#endif /* _SAC_CV2SCALAR_H_ */
