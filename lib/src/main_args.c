/*
 *
 * $Log$
 * Revision 1.1  1999/05/12 13:49:44  cg
 * Initial revision
 *
 *
 */

/*
 * File: main_args.c
 *
 * Description:
 *
 * This file contains the definitions of some helper functions
 * for command line analysis.
 *
 */
/*****************************************************************************
 *
 * file:    main_args.c
 *
 * prefix:
 *
 * description:
 *
 * This file contains the definitions of helper functions used by the macros
 * defined in main_args.h for command line analysis.
 *
 *****************************************************************************/

#include <stdio.h> /* for NULL only */

#define MAX_OPT_LEN 64
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static char buffer[MAX_OPT_LEN];

/******************************************************************************
 *
 * function:
 *   int ARGS_CheckOption(char *pattern, char *argv1, char *argv2,
 *                        char **option, char **argument)
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

int
ARGS_CheckOption (char *pattern, char *argv1, char *argv2, char **option, char **argument)
{
    int i = 0;
    int res = 1;

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
        strncpy (buffer, argv1, MIN (i + 1, MAX_OPT_LEN - 1));
        *option = buffer;
        *argument = argv1 + i + 1;
    }

    return (res);
}
