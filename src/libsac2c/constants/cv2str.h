#ifndef _SAC_CV2STR_H_
#define _SAC_CV2STR_H_

extern char *COcv2StrUByte (void *src, int off, int len, int max_char);
extern char *COcv2StrUShort (void *src, int off, int len, int max_char);
extern char *COcv2StrUInt (void *src, int off, int len, int max_char);
extern char *COcv2StrULong (void *src, int off, int len, int max_char);
extern char *COcv2StrULongLong (void *src, int off, int len, int max_char);
extern char *COcv2StrByte (void *src, int off, int len, int max_char);
extern char *COcv2StrShort (void *src, int off, int len, int max_char);
extern char *COcv2StrInt (void *src, int off, int len, int max_char);
extern char *COcv2StrLong (void *src, int off, int len, int max_char);
extern char *COcv2StrLongLong (void *src, int off, int len, int max_char);
extern char *COcv2StrBool (void *src, int off, int len, int max_char);

extern char *COcv2StrFloat (void *src, int off, int len, int max_char);
extern char *COcv2StrDouble (void *src, int off, int len, int max_char);
extern char *COcv2StrLongDouble (void *src, int off, int len, int max_char);

extern char *COcv2StrChar (void *src, int off, int len, int max_char);

extern char *COcv2StrDummy (void *src, int off, int len, int max_char);

#endif /* _SAC_CV2STR_H_ */
