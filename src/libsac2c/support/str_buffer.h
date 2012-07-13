/*
 *
 * $Id$
 *
 */

#ifndef _STR_BUFFER_H_
#define _STR_BUFFER_H_

#include "types.h" /* for bool */

typedef struct STR_BUF str_buf;

extern str_buf *SBUFcreate (int size);
extern str_buf *SBUFprint (str_buf *s, const char *string);
extern str_buf *SBUFprintf (str_buf *s, const char *format, ...);
extern char *SBUF2str (str_buf *s);
extern void SBUFflush (str_buf *s);
extern bool SBUFisEmpty (str_buf *s);
extern str_buf *SBUFfree (str_buf *s);

#endif /* _STR_BUFFER_H_ */
