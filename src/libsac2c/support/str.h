#ifndef _STR_H_
#define _STR_H_

#include "types.h"
#include "fun-attrs.h"

extern void STRtoupper (char *source, size_t start, size_t stop);
extern char *STRcpy (const char *source);
extern char *STRncpy (const char *source, size_t maxlen);
extern char *STRcat (const char *first, const char *second);
extern char *STRcatn (int n, ...);
extern char *STRtok (const char *str, const char *tok);
extern bool STReq (const char *first, const char *second) FUN_ATTR_PURE;
extern bool STReqci (const char *first, const char *second) FUN_ATTR_PURE;
extern bool STReqhex (const char *first, const char *second);
extern bool STReqoct (const char *first, const char *second);
extern bool STRprefix (const char *prefix, const char *str) FUN_ATTR_PURE;
extern bool STRsuffix (const char *suffix, const char *str) FUN_ATTR_PURE;
extern bool STReqn (const char *first, const char *second, size_t n) FUN_ATTR_PURE;
extern bool STRgt (const char *first, const char *second) FUN_ATTR_PURE;
extern bool STRsub (const char *sub, const char *str) FUN_ATTR_PURE;
extern size_t STRlen (const char *str) FUN_ATTR_PURE;
extern char *STRonNull (char *alt, char *str) FUN_ATTR_PURE;
extern char *STRsubStr (const char *string, size_t start, ssize_t len);
extern char *STRnull (void) FUN_ATTR_PURE;

extern size_t STRsizeInt (void) FUN_ATTR_CONST;

extern char *STRitoa (int number) FUN_ATTR_PURE;
extern char *STRitoa_oct (int number) FUN_ATTR_PURE;
extern char *STRitoa_hex (int number) FUN_ATTR_PURE;

extern unsigned char *STRhex2Bytes (unsigned char *array, const char *string);
extern char *STRbytes2Hex (size_t len, unsigned char *array);

extern char *STRreplaceSpecialCharacters (const char *name);
extern char *STRstring2SafeCEncoding (const char *string);
extern char *STRcommentify (const char *string);
extern node *STRstring2Array (const char *string);
extern char *STRsubstToken (const char *str, const char *token, const char *subst);
extern char *STRsubstTokend (char *str, const char *token, const char *subst);
extern char *STRsubstTokens (const char *str, size_t n, ...);
extern char *STRstrip (char *);

extern bool STRisInt (const char* str);
extern int STRatoi (const char* str);

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
