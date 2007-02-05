
#ifndef _verbose_h_
#define _verbose_h_

#define MESS_EXT(fmt)                                                                    \
    {                                                                                    \
        if (verbose > 2) {                                                               \
            PRINT_MESSAGE (fmt);                                                         \
        }                                                                                \
    }
#define MESS(fmt)                                                                        \
    {                                                                                    \
        if (verbose > 1) {                                                               \
            PRINT_MESSAGE (fmt);                                                         \
        }                                                                                \
    }
#define WARN(fmt)                                                                        \
    {                                                                                    \
        if (verbose > 0) {                                                               \
            PRINT_MESSAGE (fmt);                                                         \
        }                                                                                \
    }
#define ERR(fmt)                                                                         \
    {                                                                                    \
        PRINT_MESSAGE (fmt);                                                             \
        exit (1);                                                                        \
    }                                                                                    \
    }

#define PRINT_MESSAGE(fmt) DoPrint fmt

extern int verbose;

extern void DoPrint (char *format, ...);

#endif
