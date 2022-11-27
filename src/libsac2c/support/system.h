#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "fun-attrs.h"

extern void SYScall (char *format, ...) PRINTF_FORMAT (1, 2);
extern int SYScallNoErr (char *format, ...) PRINTF_FORMAT (1, 2);

extern int SYStest (char *format, ...) PRINTF_FORMAT (1, 2);

extern void SYSstartTracking (void);
extern void SYSstopTracking (void);

extern int SYSexec_and_read_output (char *cmd, char **out);

extern char *SYSsanitizePath (const char *);

#endif /* _SYSTEM_H_ */
