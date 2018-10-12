#ifndef _SAC_CTINFO_H_
#define _SAC_CTINFO_H_

#include "types.h"
#include "fun-attrs.h"
#include <stdarg.h>

extern int CTIgetErrorCount (void);
extern void CTIexit (int) FUN_ATTR_NORETURN;
extern void CTIinstallInterruptHandlers (void);
extern char *CTIgetErrorMessageVA (size_t line, const char *file, const char *format,
                                   va_list arg_p);
extern void CTIerror (const char *format, ...) PRINTF_FORMAT (1, 2);
extern void CTIerrorLoc (struct location loc, const char *format, ...)
  PRINTF_FORMAT (2, 3);
extern void CTIerrorLine (size_t line, const char *format, ...) PRINTF_FORMAT (2, 3);
extern void CTIerrorContinued (const char *format, ...) PRINTF_FORMAT (1, 2);
extern void CTIerrorInternal (const char *format, ...) PRINTF_FORMAT (1, 2);
extern int CTIgetErrorMessageLineLength (void);
extern void CTIabortOnBottom (char *err_msg);
extern void CTIabort (const char *format, ...) PRINTF_FORMAT (1, 2) FUN_ATTR_NORETURN;
extern void CTIabortLine (size_t line, const char *format, ...)
  PRINTF_FORMAT (2, 3) FUN_ATTR_NORETURN;
extern void CTIabortOutOfMemory (unsigned int request) FUN_ATTR_NORETURN;
extern void CTIabortOnError (void);
extern void CTIwarn (const char *format, ...) PRINTF_FORMAT (1, 2);
extern void CTIwarnLoc (struct location loc, const char *format, ...)
  PRINTF_FORMAT (2, 3);
extern void CTIwarnLine (size_t line, const char *format, ...) PRINTF_FORMAT (2, 3);
extern void CTIwarnContinued (const char *format, ...) PRINTF_FORMAT (1, 2);
extern int CTIgetWarnMessageLineLength (void);
extern void CTIstate (const char *format, ...) PRINTF_FORMAT (1, 2);
extern void CTInote (const char *format, ...) PRINTF_FORMAT (1, 2);
extern void CTInoteLine (size_t line, const char *format, ...) PRINTF_FORMAT (2, 3);
extern void CTItell (int level, const char *format, ...) PRINTF_FORMAT (2, 3);
extern void CTIterminateCompilation (node *syntax_tree);
extern const char *CTIitemName (node *item);
extern const char *CTIitemNameDivider (node *item, const char *divider);
extern const char *CTIfunParams (node *fundef);
extern void set_message_line_length (int l);

#endif /* _SAC_CTINFO_H_ */
