/*
 * $Log$
 * Revision 1.6  2004/11/23 11:38:17  cg
 * SacDevCamp renaming
 *
 * Revision 1.5  2004/11/22 18:55:29  cg
 * Moved all definitions/declarations of global variables to globals.mac
 *
 * Revision 1.4  2004/11/22 11:27:04  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 1.3  2003/04/07 14:25:44  sbs
 * max_char parameter added for abbreviation.
 *
 * Revision 1.2  2001/04/04 10:00:02  nmw
 *  missing convert functions for basetype char added
 *
 * Revision 1.1  2001/03/02 14:33:06  sbs
 * Initial revision
 *
 * Revision 1.1  2001/02/23 18:07:49  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_CV2STR_H_
#define _SAC_CV2STR_H_

extern char *COcv2StrUShort (void *src, int off, int len, int max_char);
extern char *COcv2StrUInt (void *src, int off, int len, int max_char);
extern char *COcv2StrULong (void *src, int off, int len, int max_char);
extern char *COcv2StrShort (void *src, int off, int len, int max_char);
extern char *COcv2StrInt (void *src, int off, int len, int max_char);
extern char *COcv2StrLong (void *src, int off, int len, int max_char);

extern char *COcv2StrFloat (void *src, int off, int len, int max_char);
extern char *COcv2StrDouble (void *src, int off, int len, int max_char);
extern char *COcv2StrLongDouble (void *src, int off, int len, int max_char);

extern char *COcv2StrChar (void *src, int off, int len, int max_char);

extern char *COcv2StrDummy (void *src, int off, int len, int max_char);

#endif /* _SAC_CV2STR_H_ */
