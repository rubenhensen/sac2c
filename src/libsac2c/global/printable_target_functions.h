#ifndef _PRINTABLE_TARGETS_H_
#define _PRINTABLE_TARGETS_H_

#include "types.h"

extern void PTFappend (printable_target *input);
extern printable_target *PTFmake (char *name, char *SBI, char *BE,
    char* env, printable_target *next);
extern void PTFprint (void);
extern void PTFfreeAll (void);
#endif /* _PRINTABLE_TARGETS_H_ */
