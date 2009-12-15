/*
 * $Log$
 * Revision 1.5  2004/11/23 11:38:17  cg
 * SacDevCamp renaming
 *
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

extern node *COcv2Num (void *elems, int offset);
extern node *COcv2Numbyte (void *elems, int offset);
extern node *COcv2Numshort (void *elems, int offset);
extern node *COcv2Numint (void *elems, int offset);
extern node *COcv2Numlong (void *elems, int offset);
extern node *COcv2Numlonglong (void *elems, int offset);
extern node *COcv2Numubyte (void *elems, int offset);
extern node *COcv2Numushort (void *elems, int offset);
extern node *COcv2Numuint (void *elems, int offset);
extern node *COcv2Numulong (void *elems, int offset);
extern node *COcv2Numulonglong (void *elems, int offset);
extern node *COcv2Double (void *elems, int offset);
extern node *COcv2Bool (void *elems, int offset);
extern node *COcv2Float (void *elems, int offset);
extern node *COcv2Char (void *elems, int offset);
extern node *COcv2ScalarDummy (void *elems, int offset);

#endif /* _SAC_CV2SCALAR_H_ */
