/*
 *
 * $Log$
 * Revision 3.8  2005/06/29 18:32:14  sah
 * added CVbasetype2ShortString
 *
 * Revision 3.7  2004/12/07 14:37:45  sbs
 * eliminated CVoldTypeSignature2String
 *
 * Revision 3.6  2004/11/22 16:10:11  sbs
 * SACDevCamp04
 *
 * Revision 3.5  2002/08/13 17:22:11  dkr
 * IntBytes2String: argument is unsigned now
 *
 * Revision 3.4  2002/08/05 17:03:01  sbs
 * OldTypeSignature2String added
 *
 * Revision 3.3  2001/05/17 07:35:11  sbs
 * IntBytes2String added
 * Malloc / Free checked
 *
 * Revision 3.2  2001/03/15 15:47:56  dkr
 * signature of Type2String modified
 *
 * Revision 3.1  2000/11/20 17:59:44  sacbase
 * new release made
 *
 * ....[ eliminated]....
 *
 * Revision 1.3  1994/12/14  16:35:39  sbs
 * userdef types integrated
 *
 */

#ifndef _SAC_CONVERT_H_
#define _SAC_CONVERT_H_

#include "types.h"

extern char *CVtype2String (types *type, int flag, bool all);
extern char *CVdouble2String (double);
extern char *CVfloat2String (float);
extern char *CVbasetype2String (simpletype type);
extern char *CVbasetype2ShortString (simpletype type);
extern char *CVshpseg2String (int dim, shpseg *shape);
extern char *CVintBytes2String (unsigned int bytes);

#endif /* _SAC_CONVERT_H_ */
