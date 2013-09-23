/*
 ****************************************************************************
 *
 * File: get_options.h
 *
 * Description:
 *
 * This file contains a set of macro definitions that simplify the
 * analysis of command line arguments for C programs.
 *
 ****************************************************************************
 *
 * Below is an example usage of these macros:
 *
 * int main( int argc, char *argv[])
 * {
 *   ARGS_BEGIN( argc, argv);
 *
 *   ARGS_FLAG( "help", usage(); exit(0));
 *
 *   ARGS_OPTION( "D", cppvars[num_cpp_vars++] = ARG);
 *
 *   ARGS_OPTION( "maxinl", ARG_NUM(maxinl));
 *
 *   ARGS_OPTION( "maxinl", ARG_INT(maxinl));
 *
 *   ARGS_OPTION( "O", ARG_RANGE(cc_optimize, 0, 3));
 *
 *   ARGS_OPTION_BEGIN( "d")
 *   {
 *     ARG_CHOICE_BEGIN();
 *     ARG_CHOICE("nocleanup", cleanup=0);
 *     ARG_CHOICE("syscall",   show_syscall=1);
 *     ARG_CHOICE("cccall",    gen_cccall=1; cleanup=0);
 *     ARG_CHOICE_END();
 *   }
 *   ARGS_OPTION_END( "d");
 *
 *   ARGS_OPTION_BEGIN( "check")
 *   {
 *     ARG_FLAGMASK_BEGIN();
 *     ARG_FLAGMASK( 'a', runtimecheck = RUNTIMECHECK_ALL);
 *     ARG_FLAGMASK( 'm', runtimecheck = RUNTIMECHECK_MALLOC);
 *     ARG_FLAGMASK( 'b', runtimecheck = RUNTIMECHECK_BOUNDARY);
 *     ARG_FLAGMASK( 'e', runtimecheck = RUNTIMECHECK_ERRNO);
 *     ARG_FLAGMASK_END();
 *   }
 *   ARGS_OPTION_END( "check");
 *
 *   ARGS_ARGUMENT(
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
 ****************************************************************************
 *
 *
 */

#ifndef _GETOPTIONS_H_
#define _GETOPTIONS_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> /* for tolower() */

/* This trick makes compilation possible with C++ compiler
   but uses non-standard __typeof feature.  So for now on
   we use it only in case __cplusplus is enabled.  */
#ifdef __cplusplus
#define CONVERT_TO_TYPEOF(x) (__typeof(x))
#else
#define CONVERT_TO_TYPEOF(x)
#endif

#define ARGS_BEGIN(argc, argv)                                                           \
    {                                                                                    \
        int ARGS_i = 1;                                                                  \
        int ARGS_shift;                                                                  \
        int ARGS_argc = argc;                                                            \
        char **ARGS_argv = argv;                                                         \
        char *OPT = NULL;                                                                \
        char *ARG = NULL;                                                                \
                                                                                         \
        (void)ARGS_shift; /* Surpress unused var warning */                              \
        (void)ARGS_argv;  /* Surpress unused var warning */                              \
        (void)OPT;        /* Surpress unused var warning */                              \
        (void)ARG;        /* Surpress unused var warning */                              \
        while (ARGS_i < ARGS_argc) {

#define ARGS_FIXED(s, action)                                                            \
    if ((ARGS_argv[ARGS_i][0] == '-') && StringEqual (s, ARGS_argv[ARGS_i] + 1, 1)) {    \
        if (ARGS_i + 1 >= ARGS_argc) {                                                   \
            ARGS_ERROR ("Missing argument for option");                                  \
        }                                                                                \
        ARG = ARGS_argv[ARGS_i + 1];                                                     \
        OPT = NULL;                                                                      \
        action;                                                                          \
        ARGS_i += 2;                                                                     \
        continue;                                                                        \
    }

#define ARGS_FLAG(s, action)                                                             \
    if ((ARGS_argv[ARGS_i][0] == '-') && StringEqual (s, ARGS_argv[ARGS_i] + 1, 1)) {    \
        OPT = ARGS_argv[ARGS_i] + 1;                                                     \
        ARG = NULL;                                                                      \
        action;                                                                          \
        ARGS_i++;                                                                        \
        continue;                                                                        \
    }

#define ARGS_OPTION(s, action)                                                           \
    if ((ARGS_shift                                                                      \
         = CheckOption (s, ARGS_argv[ARGS_i],                                            \
                        ARGS_i < ARGS_argc - 1 ? ARGS_argv[ARGS_i + 1] : NULL, &OPT,     \
                        &ARG))) {                                                        \
        if (ARG == NULL) {                                                               \
            ARGS_ERROR ("Missing argument for option");                                  \
        } else {                                                                         \
            action;                                                                      \
        }                                                                                \
                                                                                         \
        ARGS_i += ARGS_shift;                                                            \
        continue;                                                                        \
    }

#define ARGS_OPTION_BEGIN(s)                                                             \
    if ((ARGS_shift                                                                      \
         = CheckOption (s, ARGS_argv[ARGS_i],                                            \
                        ARGS_i < ARGS_argc - 1 ? ARGS_argv[ARGS_i + 1] : NULL, &OPT,     \
                        &ARG))) {                                                        \
        if (ARG == NULL) {                                                               \
            ARGS_ERROR ("Missing argument for option");                                  \
        } else {

#define ARGS_OPTION_END(s)                                                               \
    }                                                                                    \
                                                                                         \
    ARGS_i += ARGS_shift;                                                                \
    continue;                                                                            \
    }

#define ARGS_ARGUMENT(action)                                                            \
    if (ARGS_argv[ARGS_i][0] != '-') {                                                   \
        ARG = ARGS_argv[ARGS_i];                                                         \
        OPT = NULL;                                                                      \
                                                                                         \
        action;                                                                          \
        ARGS_i++;                                                                        \
        continue;                                                                        \
    }

#define ARGS_UNKNOWN(action)                                                             \
    {                                                                                    \
        ARG = ARGS_argv[ARGS_i];                                                         \
        OPT = NULL;                                                                      \
        action;                                                                          \
    }

#define ARGS_END()                                                                       \
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
    if (ARGS_not_chosen && StringEqual (ARG, choice, 0)) {                               \
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
                id = CONVERT_TO_TYPEOF (id) ARGS_tmp;                                    \
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
                id = CONVERT_TO_TYPEOF (id) ARGS_tmp;                                    \
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
                    id = CONVERT_TO_TYPEOF (id) ARGS_tmp;                                \
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

/*
 ****************************************************************************
 *
 * In the remainder, we feature the definitions of two auxiliary functions,
 * declared in the beginning. It is not optimal to have plain C code in a
 * header file, but in this very case it allows us to have a pure include
 * based realisation of getoptions without the need to link with additional
 * libraries. Given the additional fact that programs typically include
 * getoptions.h exactly once, this seems to be a suitable trade-off.
 *
 ****************************************************************************
 */

/******************************************************************************
 *
 * function:
 *   int CheckOption(char *pattern, char *argv1, char *argv2,
 *                   char **option, char **argument)
 *
 * description:
 *
 *  This function gets two consecutive entries from the command line,
 *  argv1 and argv2. It compares the first argument, i.e. pattern,
 *  against argv1. If these are equal, the function decides that argv2
 *  must be the argument to the option specified by pattern. If argv2
 *  does not start with '-', everything is fine and argv1 is returned as
 *  option and argv2 as argument. The actual return value is 2, indicating
 *  that two command line entries have been processed.
 *
 *  If pattern is a prefix of argv1, then the remaining characters are assumed
 *  to represent the argument; the return parameters argument and option are
 *  set accordingly. The actual return value is 1, indicating
 *  that one command line entry has been processed.
 *
 *  If pattern is not even a prefix of argv1, then the actual return value
 *  is 0, indicating that none of the command line entries have been processed.
 *
 ******************************************************************************/

static inline int
CheckOption (const char *pattern, char *argv1, char *argv2, char **option,
             char **argument)
{
    static char *buffer = NULL;
    static int buffer_size = 0;

    int i = 0;
    int res = 1;
    int request;

    if (buffer == NULL) {
        buffer = (char *)malloc (64);
        if (buffer == NULL) {
            fprintf (stderr, "Out of memory");
            exit (1);
        }
        buffer_size = 62;
    }

    if (argv2 == NULL) {
        request = strlen (argv1);
    } else {
        request = strlen (argv1) + strlen (argv2);
    }

    if (buffer_size <= request) {
        free (buffer);
        buffer = (char *)malloc (request + 20);
        if (buffer == NULL) {
            fprintf (stderr, "Out of memory");
            exit (1);
        }
        buffer_size = request + 18;
    }

    if (argv1[0] != '-') {
        *option = NULL;
        *argument = NULL;
        return (0);
    }

    while (pattern[i] != '\0') {
        /*
         * The function is finished if <pattern> is not a prefix of
         * the current <argv>.
         */
        if (pattern[i] != argv1[i + 1]) {
            *option = NULL;
            *argument = NULL;
            return (0);
        }

        i++;
    }

    if (argv1[i + 1] == '\0') {
        /*
         * <pattern> and the current <argv> are identical.
         */

        *option = argv1;

        if (argv2 == NULL) {
            *argument = NULL;
        } else {
            if (argv2[0] == '-') {
                *argument = NULL;
            } else {
                *argument = argv2;
                res = 2;
            }
        }
    } else {
        strcpy (buffer, argv1);
        buffer[i + 1] = '\0';
        /*
         * The library function strncpy() itself does NOT append
         * a terminating 0 character.
         */

        *option = buffer;
        *argument = argv1 + i + 1;
    }

    return (res);
}

/******************************************************************************
 *
 * function:
 *   bool StringEqual( char *s1, char *s2, int case_sensitive)
 *
 * description:
 *
 *  This function compares two strings s1 and s2 for equality. The third
 *  (boolean) parameter specifies case sensitivity.
 *
 ******************************************************************************/

static inline int
StringEqual (const char *s1, const char *s2, int case_sensitive)
{
    int i;

    if ((s1 == NULL) && (s2 == NULL)) {
        return (1);
    }
    if ((s1 == NULL) || (s2 == NULL)) {
        return (0);
    }

    if (case_sensitive) {
        for (i = 0; (s1[i] != '\0') && (s2[i] != '\0'); i++) {
            if (s1[i] != s2[i]) {
                return (0);
            }
        }
    } else {
        for (i = 0; (s1[i] != '\0') && (s2[i] != '\0'); i++) {
            if (tolower (s1[i]) != tolower (s2[i])) {
                return (0);
            }
        }
    }

    if ((s1[i] == '\0') && (s2[i] == '\0')) {
        return (1);
    } else {
        return (0);
    }
}

#endif /* _GETOPTIONS_H_ */
