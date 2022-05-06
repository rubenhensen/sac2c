#ifndef _STR_BUFFER_H_
#define _STR_BUFFER_H_

#include "types.h" /* for bool */

typedef struct STR_BUF str_buf;

extern str_buf *SBUFcreate (size_t size);
extern str_buf *SBUFprint (str_buf *s, const char *string);
extern str_buf *SBUFprintf (str_buf *s, const char *format, ...);
extern str_buf *SBUFvprintf (str_buf *s, const char *format, va_list arg_list);
extern str_buf *SBUFsubstToken (str_buf *s, const char *token, const char *subst);
extern char *SBUF2str (str_buf *s);
extern void SBUFflush (str_buf *s);
extern bool SBUFisEmpty (str_buf *s);
extern str_buf *SBUFfree (str_buf *s);
extern size_t SBUFlen (str_buf *s);
extern char *SBUFgetBuffer (str_buf *s);
extern char *SBUF2strAndFree (str_buf **s);

#endif /* _STR_BUFFER_H_ */
