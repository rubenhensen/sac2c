/*
 *
 * $Log$
 * Revision 2.1  1999/05/12 14:13:41  cg
 * new release made
 *
 * Revision 1.1  1999/05/12 13:52:40  cg
 * Initial revision
 *
 *
 */

/*
 * File: main_args.h
 *
 * Description:
 *
 * This file contains a set of macro definitions that simplify the
 * analysis of command line arguments for C programs.
 *
 * Note that for these macros to work correctly you must link your executable
 * program with main_args.o derived from main_args.c !
 *
 ****************************************************************************
 *
 * Below is an example usage of these macros:
 *
 * int main( int argc, char *argv[])
 * {
 *   ARGS_BEGIN( argc, argv);
 *
 *   FLAG( "help", usage(); exit(0));
 *
 *   OPTION( "D", cppvars[num_cpp_vars++] = ARG);
 *
 *   OPTION( "maxinl", ARG_NUM(maxinl));
 *
 *   OPTION( "maxinl", ARG_INT(maxinl));
 *
 *   OPTION( "O", ARG_RANGE(cc_optimize, 0, 3));
 *
 *   OPTION( "d",
 *   {
 *     ARG_CHOICE_BEGIN();
 *     ARG_CHOICE("efence",    use_efence=1);
 *     ARG_CHOICE("nocleanup", cleanup=0);
 *     ARG_CHOICE("syscall",   show_syscall=1);
 *     ARG_CHOICE("cccall",    gen_cccall=1; cleanup=0);
 *     ARG_CHOICE_END();
 *   });
 *
 *   OPTION( "check",
 *   {
 *     ARG_FLAGMASK_BEGIN();
 *     ARG_FLAGMASK( 'a', runtimecheck = RUNTIMECHECK_ALL);
 *     ARG_FLAGMASK( 'm', runtimecheck = RUNTIMECHECK_MALLOC);
 *     ARG_FLAGMASK( 'b', runtimecheck = RUNTIMECHECK_BOUNDARY);
 *     ARG_FLAGMASK( 'e', runtimecheck = RUNTIMECHECK_ERRNO);
 *     ARG_FLAGMASK_END();
 *   });
 *
 *   ARGUMENT(
 *   {
 *      strcpy(filename, ARG);
 *   });
 *
 *   ARGS_END();
 *
 *   ...
 *   return(0);
 * }
 *
 ****************************************************************************
 *
 * Command line analysis is enabled by the macro ARGS_BEGIN() and disabled
 * by the macro ARGS_END(). In between these two, we distinguish between
 * "flags" which are command line entries that start with '-', "options"
 * which are flags with an additional argument and pure arguments. A dedicated
 * macro exists for each of these entry categories that defines the name of
 * the option or flag along with an associated code sequence which is to
 * be executed when the corresponding flag or option is detected.
 *
 * Within a code sequence associated with a flag the name of the flag may
 * always be accessed by the variable OPT; within the code sequence associated
 * with an option, the corresponding argument can additionally be accessed
 * through the variable ARG.
 *
 * If the associated code detects an error condition, the macro ARGS_ERROR()
 * provides means for a consistent error message production. This macro may
 * be redefined to suit particular needs, e.g. special error message layouts
 * in the context of larger software projects.
 *
 * Some special flavours of options are supported by additional macros.
 *
 * ARG_CHOICE() implements the selection within a given set of argument strings
 *              and allows to define particular actions to be performed for each.
 *
 * ARG_FLAGMASK() implements arguments where each character triggers a certain
 *                bit in an associated bit field.
 *
 * ARG_INT() implements an integer argument which is automatically converted
 *           from the command line string an assigned to the given variable.
 *
 * ARG_NUM() implements an integer argument which is automatically converted
 *           from the command line string an assigned to the given variable;
 *           the argument must be greater or equal null.
 *
 * ARG_RANGE() implements an integer argument which is automatically converted
 *             from the command line string an assigned to the given variable;
 *             the argument must be in the given range (both limits included).
 *
 *
 */

#ifndef _main_args_h

#define _main_args_h

#include <string.h>
#include <stdio.h>

extern int ARGS_CheckOption (char *pattern, char *argv1, char *argv2, char **option,
                             char **argument);

#define ARGS_BEGIN(argc, argv)                                                           \
    {                                                                                    \
        int ARGS_i = 1;                                                                  \
        int ARGS_shift;                                                                  \
        int ARGS_argc = argc;                                                            \
        char **ARGS_argv = argv;                                                         \
        char *OPT = NULL;                                                                \
        char *ARG = NULL;                                                                \
                                                                                         \
        while (ARGS_i < ARGS_argc) {

#define FLAG(s, action)                                                                  \
    if ((ARGS_argv[ARGS_i][0] == '-') && (0 == strcmp (s, ARGS_argv[ARGS_i] + 1))) {     \
        OPT = ARGS_argv[ARGS_i] + 1;                                                     \
        ARG = NULL;                                                                      \
        action;                                                                          \
        ARGS_i++;                                                                        \
        continue;                                                                        \
    }

#define OPTION(s, action)                                                                \
    if ((ARGS_shift                                                                      \
         = ARGS_CheckOption (s, ARGS_argv[ARGS_i],                                       \
                             ARGS_i < ARGS_argc - 1 ? ARGS_argv[ARGS_i + 1] : NULL,      \
                             &OPT, &ARG))) {                                             \
        if (ARG == NULL) {                                                               \
            ARGS_ERROR ("Missing argument for option");                                  \
        } else {                                                                         \
            action;                                                                      \
        }                                                                                \
                                                                                         \
        ARGS_i += ARGS_shift;                                                            \
        continue;                                                                        \
    }

#define ARGUMENT(action)                                                                 \
    if (ARGS_argv[ARGS_i][0] != '-') {                                                   \
        ARG = ARGS_argv[ARGS_i];                                                         \
        OPT = NULL;                                                                      \
                                                                                         \
        action;                                                                          \
        ARGS_i++;                                                                        \
        continue;                                                                        \
    }

#define ARGS_END()                                                                       \
    ARG = ARGS_argv[ARGS_i];                                                             \
    OPT = NULL;                                                                          \
    ARGS_ERROR ("Invalid command line argument");                                        \
    ARGS_i++;                                                                            \
    }                                                                                    \
    }

#ifndef ARGS_ERROR
#define ARGS_ERROR(msg)                                                                  \
    {                                                                                    \
        fprintf (stderr, "ERROR: %s : %s %s %s !\n", msg, ARGS_argv[0],                  \
                 NULL == OPT ? "" : OPT, NULL == ARG ? "" : ARG);                        \
    }
#endif /* ARGS_ERROR */

#define ARG_CHOICE_BEGIN()                                                               \
    {                                                                                    \
        int ARGS_not_chosen = 1;

#define ARG_CHOICE(choice, action)                                                       \
    if (ARGS_not_chosen && (0 == strcmp (ARG, choice))) {                                \
        ARGS_not_chosen = 0;                                                             \
        action;                                                                          \
    }

#define ARG_CHOICE_END()                                                                 \
    if (ARGS_not_chosen) {                                                               \
        ARGS_ERROR ("Invalid argument for option");                                      \
    }                                                                                    \
    }

#define ARG_NUM(id)                                                                      \
    {                                                                                    \
        int ARGS_tmp;                                                                    \
        char *ARGS_str;                                                                  \
                                                                                         \
        if (ARG != NULL) {                                                               \
            ARGS_tmp = strtol (ARG, &ARGS_str, 10);                                      \
            if ((ARGS_str[0] == '\0') && (ARGS_tmp >= 0)) {                              \
                id = ARGS_tmp;                                                           \
            } else {                                                                     \
                ARGS_ERROR ("Number argument expected for option");                      \
            }                                                                            \
        } else {                                                                         \
            ARGS_ERROR ("Integer argument expected for option");                         \
        }                                                                                \
    }

#define ARG_INT(id)                                                                      \
    {                                                                                    \
        int ARGS_tmp;                                                                    \
        char *ARGS_str;                                                                  \
                                                                                         \
        if (ARG != NULL) {                                                               \
            ARGS_tmp = strtol (ARG, &ARGS_str, 10);                                      \
            if (ARGS_str[0] == '\0') {                                                   \
                id = ARGS_tmp;                                                           \
            } else {                                                                     \
                ARGS_ERROR ("Integer argument expected for option");                     \
            }                                                                            \
        } else {                                                                         \
            ARGS_ERROR ("Integer argument expected for option");                         \
        }                                                                                \
    }

#define ARG_RANGE(id, min, max)                                                          \
    {                                                                                    \
        int ARGS_tmp;                                                                    \
        char *ARGS_str;                                                                  \
                                                                                         \
        if (ARG != NULL) {                                                               \
            ARGS_tmp = strtol (ARG, &ARGS_str, 10);                                      \
            if (ARGS_str[0] == '\0') {                                                   \
                if ((ARGS_tmp >= min) && (ARGS_tmp <= max)) {                            \
                    id = ARGS_tmp;                                                       \
                } else {                                                                 \
                    ARGS_ERROR ("Argument out of range for option");                     \
                }                                                                        \
            } else {                                                                     \
                ARGS_ERROR ("Integer argument expected for option");                     \
            }                                                                            \
        } else {                                                                         \
            ARGS_ERROR ("Integer argument expected for option");                         \
        }                                                                                \
    }

#define ARG_FLAGMASK_BEGIN()                                                             \
    {                                                                                    \
        int ARGS_i = 0;                                                                  \
        while (ARG[ARGS_i] != '\0') {                                                    \
            switch (ARG[ARGS_i]) {

#define ARG_FLAGMASK(c, action)                                                          \
    case c: {                                                                            \
        action;                                                                          \
    } break;

#define ARG_FLAGMASK_END()                                                               \
    default: {                                                                           \
        ARGS_ERROR ("Illegal flag in argument for option");                              \
    }                                                                                    \
        }                                                                                \
        ARGS_i++;                                                                        \
        }                                                                                \
        }

#endif /* _main_args_h */
