#ifndef _SAC_CTINFO_H_
#define _SAC_CTINFO_H_

#define LINE_TO_LOC(line_nr) ((struct location) {.fname = global.filename, .line = line_nr, .col = 0})
#define EMPTY_LOC ((struct location) {.fname = NULL, .line = 0, .col = 0})

#include "types.h"
#include "fun-attrs.h"
#include <stdarg.h>

FILE *CTIget_stderr (void);
void CTIset_stderr (FILE * new_stderr);
extern int CTIgetErrorCount (void);
extern void CTIresetErrorCount (void);
extern void CTIexit (int) FUN_ATTR_NORETURN;
extern void CTIinstallInterruptHandlers (void);

/**
 * general helpers:
 */
extern void CTIterminateCompilation (node *syntax_tree);
extern const char *CTIitemName (node *item);
extern const char *CTIitemNameDivider (node *item, const char *divider);
extern const char *CTIfunParams (node *fundef);
extern void set_message_line_length (size_t l);


/**
 * verbosity level >= 0:
 */
extern void CTIerror (const struct location loc, const char *format, ...) PRINTF_FORMAT (2, 3);
extern void CTIerrorLoc (struct location loc, const char *format, ...) PRINTF_FORMAT (2, 3);
extern void CTIerrorContinued (const char *format, ...) PRINTF_FORMAT (1, 2);
extern void CTIerrorInternal (const char *format, ...) PRINTF_FORMAT (1, 2);
/* helpers: */
extern char *CTIgetErrorMessageVA (size_t line, const char *file, const char *format,
                                   va_list arg_p);
extern size_t CTIgetErrorMessageLineLength (void);
extern void CTIabortOnBottom (char *err_msg);
extern void CTIabort (const char *format, ...) PRINTF_FORMAT (1, 2) FUN_ATTR_NORETURN;
extern void CTIabortLoc (struct location loc, const char *format, ...);
extern void CTIabortLine (size_t line, const char *format, ...) PRINTF_FORMAT (2, 3) FUN_ATTR_NORETURN;
extern void CTIabortOutOfMemory (size_t request) FUN_ATTR_NORETURN;
extern void CTIabortOnError (void);

/**
 * verbosity level >= 1:
 */
extern void CTIwarn (const char *format, ...) PRINTF_FORMAT (1, 2);
extern void CTIwarnLoc (struct location loc, const char *format, ...) PRINTF_FORMAT (2, 3);
extern void CTIwarnLine (size_t line, const char *format, ...) PRINTF_FORMAT (2, 3);
extern void CTIwarnContinued (const char *format, ...) PRINTF_FORMAT (1, 2);
/* helpers: */
extern size_t CTIgetWarnMessageLineLength (void);

/**
 * verbosity level >= 2:
 */
extern void CTIstate (const char *format, ...) PRINTF_FORMAT (1, 2);

/**
 * verbosity level >= 3:
 */
extern void CTInote (const char *format, ...) PRINTF_FORMAT (1, 2);
extern void CTInoteLine (size_t line, const char *format, ...) PRINTF_FORMAT (2, 3);

/**
 * verbosity level <xxx>
 */
extern void CTItell (int level, const char *format, ...) PRINTF_FORMAT (2, 3);

#endif /* _SAC_CTINFO_H_ */
