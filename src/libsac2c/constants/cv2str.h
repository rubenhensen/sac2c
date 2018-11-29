#ifndef _SAC_CV2STR_H_
#define _SAC_CV2STR_H_

extern char *COcv2StrUByte (void *src, size_t off, size_t len, size_t max_char);
extern char *COcv2StrUShort (void *src, size_t off, size_t len, size_t max_char);
extern char *COcv2StrUInt (void *src, size_t off, size_t len, size_t max_char);
extern char *COcv2StrULong (void *src, size_t off, size_t len, size_t max_char);
extern char *COcv2StrULongLong (void *src, size_t off, size_t len, size_t max_char);
extern char *COcv2StrByte (void *src, size_t off, size_t len, size_t max_char);
extern char *COcv2StrShort (void *src, size_t off, size_t len, size_t max_char);
extern char *COcv2StrInt (void *src, size_t off, size_t len, size_t max_char);
extern char *COcv2StrLong (void *src, size_t off, size_t len, size_t max_char);
extern char *COcv2StrLongLong (void *src, size_t off, size_t len, size_t max_char);
extern char *COcv2StrBool (void *src, size_t off, size_t len, size_t max_char);

extern char *COcv2StrFloat (void *src, size_t off, size_t len, size_t max_char);
extern char *COcv2StrFloatvec (void *src, size_t off, size_t len, size_t max_char);
extern char *COcv2StrDouble (void *src, size_t off, size_t len, size_t max_char);
extern char *COcv2StrLongDouble (void *src, size_t off, size_t len, size_t max_char);

extern char *COcv2StrChar (void *src, size_t off, size_t len, size_t max_char);

extern char *COcv2StrDummy (void *src, size_t off, size_t len, size_t max_char);

#endif /* _SAC_CV2STR_H_ */
