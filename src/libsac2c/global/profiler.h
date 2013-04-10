/*
 * This is a way to profile the entire application or specific parts
 *
 * commandline flags:
 *   -timeall
 *   -time_cycles
 *   -time_phase <phase>
 *
 */
#ifndef PROFILE_H_
#define PROFILE_H_
#include <sys/times.h>

typedef struct timeinfo_t {
    compiler_phase_t phase;
    struct timespec timer;
    int timestraversed;
    struct timespec duration;
    struct timeinfo_t *next; /* Used to generate report */
} timeinfo_t;

void TIMEbegin (compiler_phase_t);
void TIMEend (compiler_phase_t);
node *TIMEtimeReport (node *, info *);

#endif /* PROFILE_H_ */
