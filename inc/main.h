#ifndef _main_h

#ifndef _dbug_h
#include "dbug.h"
#endif /* _dbug_h */

#define _main_h

/*******************************************************************************

Some usefull macros for scanning command line arguments. For easy understanding
here you can find some examples:

- MAIN			>> no options <<
  {...}

- MAIN			>> only DBug option -# ... <<
  {
  OPT
  OTHER {...}         ({...} may be empty)
  ENDOPT
  ...}

- MAIN			>> DBUG option -# ..., v, f ... <<
  {
  OPT
  ARG 'v': {...}
  ARG 'f': PARM {...*argv...}
           NEXTOPT
  OTHER {...}         ({...} may be empty)
  ENDOPT
  ...}

*******************************************************************************/

#define MAIN                                                                             \
    int main (argc, argv) int argc;                                                      \
    char **argv;

#define PARM                                                                             \
    if (*++*argv)                                                                        \
        ;                                                                                \
    else if (--argc > 0)                                                                 \
        ++argv;                                                                          \
    else {                                                                               \
        --*argv, ++argc;                                                                 \
        break;                                                                           \
    }

#define NEXTOPT *argv += strlen (*argv) - 1;

#define OPT                                                                              \
    while (--argc > 0 && **++argv == '-') {                                              \
        switch (*++*argv) {                                                              \
        case 0:                                                                          \
            --*argv;                                                                     \
            break;                                                                       \
        case '-':                                                                        \
            if (!(*argv)[1]) {                                                           \
                ++argv, --argc;                                                          \
                break;                                                                   \
            }                                                                            \
        default:                                                                         \
            do {                                                                         \
                switch (**argv) {                                                        \
                case '#':                                                                \
                    PARM DBUG_PUSH (*argv);                                              \
                    NEXTOPT
#define ARG                                                                              \
    continue;                                                                            \
    case
#define OTHER                                                                            \
    continue;                                                                            \
    }
#define ENDOPT                                                                           \
    }                                                                                    \
    while (*++*argv)                                                                     \
        ;                                                                                \
    continue;                                                                            \
    }                                                                                    \
    break;                                                                               \
    }

#endif /* _main_h */
