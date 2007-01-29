/*
 *  $Id$
 *
 */

/*
 * hzip
 *
 * This little tool reads in header files as specified on the command line,
 * strips away all comments and prints the concatenated result on stdout.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum { ST_base, ST_string, ST_stdcomm, ST_linecomm } state_t;

typedef enum {
    CH_stdcommstart,
    CH_stdcommstop,
    CH_linecommstart,
    CH_linecommstop,
    CH_stringdelim,
    CH_lineextender,
    CH_none
} special_char_t;

static void
error (const char *format, ...)
{
    va_list arg_p;

    va_start (arg_p, format);

    fprintf (stderr, "ERROR: ");
    vfprintf (stderr, format, arg_p);

    va_end (arg_p);

    exit (1);
}

static special_char_t
special_char (char c, FILE *infile)
{
    special_char_t special;
    char cc;

    switch (c) {
    case '/':
        cc = getc (infile);
        switch (cc) {
        case '/':
            special = CH_linecommstart;
            break;
        case '*':
            special = CH_stdcommstart;
            break;
        default:
            ungetc (cc, infile);
            special = CH_none;
        }
        break;
    case '*':
        cc = getc (infile);
        switch (cc) {
        case '/':
            special = CH_stdcommstop;
            break;
        default:
            ungetc (cc, infile);
            special = CH_none;
        }
        break;
    case '"':
        special = CH_stringdelim;
        break;
    case '\n':
        do {
            cc = getc (infile);
        } while ((cc == c) || (cc == ' '));
        ungetc (cc, infile);
        special = CH_linecommstop;
        break;
    case '\\':
        cc = getc (infile);
        switch (cc) {
        case '\n':
            special = CH_lineextender;
            break;
        default:
            ungetc (cc, infile);
            special = CH_none;
        }
        break;
    case ' ':
        do {
            cc = getc (infile);
        } while (cc == c);
        ungetc (cc, infile);
        special = CH_none;
        break;
    default:
        special = CH_none;
    }

    return (special);
}

static void
zipfile (char *filename, FILE *infile)
{
    char c;
    special_char_t special;
    state_t state;
    int line, col;

    state = ST_base;
    line = 1;
    col = 0;

    while (!feof (infile)) {
        c = getc (infile);
        special = special_char (c, infile);
        col++;

        switch (special) {

        case CH_stdcommstart:
            switch (state) {
            case ST_base:
                state = ST_stdcomm;
                break;
            case ST_string:
                putchar ('/');
                putchar ('*');
                break;
            case ST_stdcomm:
            case ST_linecomm:
                error ("Nested comment detected: %s:%d:%d", filename, line, col);
                break;
            }
            break;

        case CH_stdcommstop:
            switch (state) {
            case ST_base:
                error ("Comment terminated without being started: %s:%d:%d", filename,
                       line, col);
                break;
            case ST_string:
                putchar ('*');
                putchar ('/');
                break;
            case ST_stdcomm:
                state = ST_base;
                break;
            case ST_linecomm:
                error ("Line comment terminated: %s:%d:%d", filename, line, col);
                break;
            }
            break;

        case CH_linecommstart:
            switch (state) {
            case ST_base:
                state = ST_linecomm;
                break;
            case ST_string:
                putchar ('/');
                putchar ('/');
                break;
            case ST_stdcomm:
            case ST_linecomm:
                error ("Nested comment detected: %s:%d:%d", filename, line, col);
                break;
            }
            break;

        case CH_linecommstop:
            switch (state) {
            case ST_base:
                putchar ('\n');
                break;
            case ST_string:
                error ("Unterminated string detected: %s:%d:%d", filename, line, col);
                break;
            case ST_stdcomm:
                break;
            case ST_linecomm:
                putchar ('\n');
                state = ST_base;
                break;
            }
            line++;
            col = 0;
            break;

        case CH_stringdelim:
            switch (state) {
            case ST_base:
                putchar ('"');
                state = ST_string;
                break;
            case ST_string:
                putchar ('"');
                state = ST_base;
                break;
            case ST_stdcomm:
            case ST_linecomm:
                break;
            }
            break;

        case CH_lineextender:
            switch (state) {
            case ST_string:
                putchar ('\\');
                break;
            case ST_base:
            case ST_stdcomm:
            case ST_linecomm:
                break;
            }
            break;

        case CH_none:
            switch (state) {
            case ST_base:
            case ST_string:
                if (isprint (c))
                    putchar (c);
                break;
            case ST_stdcomm:
            case ST_linecomm:
                break;
            }
            break;
        }
    }

    if (state != ST_base) {
        error ("File ended while scanning string or comment: %s", filename);
    }
}

int
main (int argc, char *argv[])
{
    int i;
    FILE *infile;

    for (i = 1; i < argc; i++) {
        infile = fopen (argv[i], "r");
        if (infile == NULL) {
            error ("Unable to open file: %s", argv[i]);
        }

        zipfile (argv[i], infile);

        fclose (infile);
    }

    return (0);
}
