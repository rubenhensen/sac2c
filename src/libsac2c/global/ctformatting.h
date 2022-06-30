#ifndef _SAC_CTFORMATTING_H_
#define _SAC_CTFORMATTING_H_

#include <stdarg.h>

#include "str_buffer.h"

extern str_buf *CTFcreateMessageBegin (str_buf *header, const char *format, va_list arg_p);
extern str_buf *CTFcreateMessageContinued (const char *multiline_header, str_buf *remaining_lines);
extern char *CTFcreateMessageEnd (void);
extern str_buf *CTFcreateMessage (const char *first_line_header, const char *multiline_header, 
                                  const char *format, va_list arg_p);
extern str_buf *CTFcreateMessageLoc (struct location loc, const char *message_header, 
                                     const char *format, va_list arg_p);

#endif _SAC_CTFORMATTING_H_
