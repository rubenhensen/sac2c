#ifndef _SAC_CTFORMATTING_H_
#define _SAC_CTFORMATTING_H_

#define CTF_DEFAULT_FIRST_LINE_HEADER ("%s:@")
#define CTF_DEFAULT_MULTI_LINE_HEADER ("%.0s  ")

#ifndef CTF_DEFAULT_HEADERS_ONLY

#include <stdarg.h>
#include "fun-attrs.h"
#include "str_buffer.h"

extern void CTFinitialize (void);

extern void CTFcheckHeaderConsistency (char *header);
extern str_buf *CTFvcreateMessageBegin (str_buf *header, const char *format, va_list arg_p);
extern str_buf *CTFcreateMessageBegin (str_buf *header, const char *format, ...) PRINTF_FORMAT (2, 3);
extern str_buf *CTFcreateMessageContinued (const char *multiline_header, str_buf *remaining_lines);
extern str_buf *CTFcreateMessageEnd (void);

extern str_buf *CTFvcreateMessage (const char *first_line_header, const char *multiline_header, 
                                   const char *format, va_list arg_p);
extern str_buf *CTFcreateMessage (const char *first_line_header, const char *multiline_header,
                                  const char *format, ...) PRINTF_FORMAT (3, 4);

extern str_buf *CTFvcreateMessageLoc (struct location loc, const char *message_header, 
                                     const char *format, va_list arg_p);
extern str_buf *CTFcreateMessageLoc (struct location loc, const char *message_header, 
                                     const char *format, ...) PRINTF_FORMAT (3, 4);
#endif /* CTF_DEFAULT_HEADERS_ONLY */
#endif /* _SAC_CTFORMATTING_H_ */
