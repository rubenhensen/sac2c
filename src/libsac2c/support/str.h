/*
 *
 * $Id$
 *
 */

#ifndef _STR_H_
#define _STR_H_

#include "types.h"

extern char *STRcpy (const char *source);
extern char *STRncpy (const char *source, int maxlen);
extern char *STRcat (const char *first, const char *second);
extern char *STRcatn (int n, ...);
extern char *STRtok (char *str, char *tok);
extern bool STReq (const char *first, const char *second);
extern int STRlen (const char *str);

extern char *STRitoa (long number);

extern unsigned char *STRhex2Bytes (unsigned char *array, const char *string);
extern char *STRbytes2Hex (int len, unsigned char *array);

extern char *STRreplaceSpecialCharacters (const char *name);
extern char *STRstring2SafeCEncoding (const char *string);

#endif /* _STR_H_ */
