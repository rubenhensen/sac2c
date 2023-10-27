/* echo.c, derived from code echo.c in Bash.
   Copyright (C) 87,89, 1991-1997, 1999 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <sys/types.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define _(text) text

/* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "echo"

#define AUTHORS "FIXME unknown"

/* echo [-neE] [arg ...]
Output the ARGs.  If -n is specified, the trailing newline is
suppressed.  If the -e option is given, interpretation of the
following backslash-escaped characters is turned on:
        \a	alert (bell)
        \b	backspace
        \c	suppress trailing newline
        \f	form feed
        \n	new line
        \r	carriage return
        \t	horizontal tab
        \v	vertical tab
        \\	backslash
        \num	the character whose ASCII code is NUM (octal).

You can explicitly turn off the interpretation of the above characters
on System V systems with the -E option.
*/

/* If defined, interpret backslash escapes if -e is given.  */
#define V9_ECHO

/* If defined, interpret backslash escapes unless -E is given.
   V9_ECHO must also be defined.  */
/* #define V9_DEFAULT */

#if defined(V9_ECHO)
#if defined(V9_DEFAULT)
#define VALID_ECHO_OPTIONS "neE"
#else
#define VALID_ECHO_OPTIONS "ne"
#endif /* !V9_DEFAULT */
#else  /* !V9_ECHO */
#define VALID_ECHO_OPTIONS "n"
#endif /* !V9_ECHO */

/* The name this program was run with. */
static char *program_name;

#if 0
/*
 * This is missing on 
 * Apple clang version 15.0.0 (clang-1500.0.40.1)
 * Target: x86_64-apple-darwin23.1.0
 * to compile echo with -O3 and only when using an 
 * optimisation option.
 * This seems weird on many accounts and since
 * we believe echo is no longer needed (2023) 
 * anyways, we decided to exclude echo entirely 
 * from the build process but keep the sources
 * for a little longer just in case we find that
 * it is needed in some other context that 
 * currently is only disabled.
 * Reactivation currently only requires an
 * un-comment in the top-level cmake file.
 */
void *
memrchr(const void *s, int c, size_t n)
{
    const unsigned char *cp;

    if (n != 0) {
       cp = (unsigned char *)s + n;
       do {
           if (*(--cp) == (unsigned char)c)
               return (void *)cp;
       } while (--n != 0);
    }
    return (void *)0;
}
#endif

void
usage (int status)
{
    if (status != 0)
        fprintf (stderr, _ ("Try `%s --help' for more information.\n"), program_name);
    else {
        printf (_ ("Usage: %s [OPTION]... [STRING]...\n"), program_name);
        printf (_ ("\
Echo the STRING(s) to standard output.\n\
\n\
  -n              do not output the trailing newline\n\
"));
#if defined(V9_ECHO)
        printf (_ ("\
  -e              enable interpretation of the backslash-escaped characters\n\
                    listed below\n"));
#endif
#if defined(V9_DEFAULT)
        printf (_ ("\
  -E              disable interpretation of those sequences in STRINGs\n\
      --help      display this help and exit (should be alone)\n\
      --version   output version information and exit (should be alone)\n"));
#endif
        printf (_ ("\
\n\
Without -E, the following sequences are recognized and interpolated:\n\
\n\
  \\NNN   the character whose ASCII code is NNN (octal)\n\
  \\\\     backslash\n\
  \\a     alert (BEL)\n\
  \\b     backspace\n\
  \\c     suppress trailing newline\n\
  \\f     form feed\n\
  \\n     new line\n\
  \\r     carriage return\n\
  \\t     horizontal tab\n\
  \\v     vertical tab\n\
"));
        puts (_ ("\nReport bugs to <bug-sh-utils@gnu.org>."));
    }
    exit (status);
}

/* Print the words in LIST to standard output.  If the first word is
   `-n', then don't print a trailing newline.  We also support the
   echo syntax from Version 9 unix systems. */

int
main (int argc, char **argv)
{
    int display_return = 1, do_v9 = 0;
    int allow_options = 1;

    program_name = argv[0];
#if 0
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif

    /* Don't recognize --help or --version if POSIXLY_CORRECT is set.  */
#if 0
  if (getenv ("POSIXLY_CORRECT") == NULL)
    parse_long_options (argc, argv, PROGRAM_NAME, GNU_PACKAGE, VERSION,
		      AUTHORS, usage);
  else
    allow_options = 0;
#endif

/* System V machines already have a /bin/sh with a v9 behaviour.  We
   use the identical behaviour for these machines so that the
   existing system shell scripts won't barf. */
#if defined(V9_ECHO) && defined(V9_DEFAULT)
    do_v9 = allow_options;
#endif

    --argc;
    ++argv;

    while (argc > 0 && *argv[0] == '-') {
        register char *temp;
        register int i;

        /* If it appears that we are handling options, then make sure that
           all of the options specified are actually valid.  Otherwise, the
           string should just be echoed. */
        temp = argv[0] + 1;

        for (i = 0; temp[i]; i++) {
            if (strrchr (VALID_ECHO_OPTIONS, temp[i]) == 0)
                goto just_echo;
        }

        if (!*temp)
            goto just_echo;

        /* All of the options in TEMP are valid options to ECHO.
           Handle them. */
        while (*temp) {
            if (allow_options && *temp == 'n')
                display_return = 0;
#if defined(V9_ECHO)
            else if (allow_options && *temp == 'e')
                do_v9 = 1;
#if defined(V9_DEFAULT)
            else if (allow_options && *temp == 'E')
                do_v9 = 0;
#endif /* V9_DEFAULT */
#endif /* V9_ECHO */
            else
                goto just_echo;

            temp++;
        }
        argc--;
        argv++;
    }

just_echo:

    if (argc > 0) {
#if defined(V9_ECHO)
        if (do_v9) {
            while (argc > 0) {
                register char *s = argv[0];
                register int c;

                while ((c = *s++)) {
                    if (c == '\\' && *s) {
                        switch (c = *s++) {
                        case 'a':
                            c = '\007';
                            break;
                        case 'b':
                            c = '\b';
                            break;
                        case 'c':
                            display_return = 0;
                            continue;
                        case 'f':
                            c = '\f';
                            break;
                        case 'n':
                            c = '\n';
                            break;
                        case 'r':
                            c = '\r';
                            break;
                        case 't':
                            c = '\t';
                            break;
                        case 'v':
                            c = (int)0x0B;
                            break;
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                            c -= '0';
                            if (*s >= '0' && *s <= '7')
                                c = c * 8 + (*s++ - '0');
                            if (*s >= '0' && *s <= '7')
                                c = c * 8 + (*s++ - '0');
                            break;
                        case '\\':
                            break;
                        default:
                            putchar ('\\');
                            break;
                        }
                    }
                    putchar (c);
                }
                argc--;
                argv++;
                if (argc > 0)
                    putchar (' ');
            }
        } else
#endif /* V9_ECHO */
        {
            while (argc > 0) {
                fputs (argv[0], stdout);
                argc--;
                argv++;
                if (argc > 0)
                    putchar (' ');
            }
        }
    }
    if (display_return)
        putchar ('\n');
    exit (0);
}
