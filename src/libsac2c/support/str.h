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
extern bool STRprefix (const char *prefix, const char *str);
extern bool STReqn (const char *first, const char *second, int n);
extern bool STRsub (const char *sub, const char *str);
extern int STRlen (const char *str);
extern char *STRonNull (char *alt, char *str);

extern char *STRitoa (long number);

extern unsigned char *STRhex2Bytes (unsigned char *array, const char *string);
extern char *STRbytes2Hex (int len, unsigned char *array);

extern char *STRreplaceSpecialCharacters (const char *name);
extern char *STRstring2SafeCEncoding (const char *string);

/*********************************
 * macro definitions
 *********************************/

/* format string for pointers */
#ifdef NEED_PTR_PREFIX
#define F_PTR "0x%p"
#else
#define F_PTR "%p"
#endif

#endif /* _STR_H_ */
