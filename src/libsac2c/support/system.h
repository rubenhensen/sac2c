#ifndef _SYSTEM_H_
#define _SYSTEM_H_

extern void SYScall (char *format, ...);
extern int SYScallNoErr (char *format, ...);

extern int SYStest (char *format, ...);

extern void SYSstartTracking (void);
extern void SYSstopTracking (void);

char *SYSexec_and_read_output (char *cmd);

#endif /* _SYSTEM_H_ */
