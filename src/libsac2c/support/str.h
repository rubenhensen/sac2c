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
extern char *STRtok (const char *str, const char *tok);
extern bool STReq (const char *first, const char *second);
extern bool STReqci (const char *first, const char *second);
extern bool STReqhex (const char *first, const char *second);
extern bool STReqoct (const char *first, const char *second);
extern bool STRprefix (const char *prefix, const char *str);
extern bool STRsuffix (const char *suffix, const char *str);
extern bool STReqn (const char *first, const char *second, int n);
extern bool STRsub (const char *sub, const char *str);
extern int STRlen (const char *str);
extern char *STRonNull (char *alt, char *str);
extern char *STRsubStr (const char *string, int start, int len);
extern char *STRnull ();

extern char *STRitoa (int number);
extern char *STRitoa_oct (int number);
extern char *STRitoa_hex (int number);

extern unsigned char *STRhex2Bytes (unsigned char *array, const char *string);
extern char *STRbytes2Hex (int len, unsigned char *array);

extern char *STRreplaceSpecialCharacters (const char *name);
extern char *STRstring2SafeCEncoding (const char *string);
extern char *STRcommentify (const char *string);
extern node *STRstring2Array (const char *string);
extern char *STRsubstToken (const char *str, const char *token, const char *subst);

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
