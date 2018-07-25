/*
 *  Code Beautifier for C/C++/Java
 *
 *  Also called: Pretty-Printer, Reformatter or Indent (GNU)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getoptions.h"

/* boolean */
#ifndef __cplusplus
typedef int bool;
#endif
#define FALSE 0
#define TRUE 1

#define VERS "Code Beautifier for C/C++/Java          Version 2.3 (Sep 19, 2002)\n"
#define PGM "cb"

#define MAXLEN 4096

static char string[MAXLEN]; /* input line max. length */

static int pos = 0;
static int c_level = 0;
static int save_p_flg[20][10];
static bool save_ind[20][10];
static int if_lev = 0;
static bool if_flg = FALSE;
static int level = 0;
static bool ind[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int p_flg[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static char p_char;
static bool a_flg = FALSE;
static int save_tabs[20][10];

static char cc;
static bool s_flg = TRUE;
static int peek = (-1);
static int tabs = 0;
static int last_char;
static int c;

/* prints usage info to stdout */
static void
usage (char *args)
{
    fprintf (stderr, VERS);
    fprintf (stderr, "\nUsage:\n");
    fprintf (stderr, "   %s [<filename>] [-i<indent>] [-r] [-t]\n", PGM);
    fprintf (stderr, "   %s [-h]\n", PGM);
    fprintf (stderr, "(Executing: %s)\n\n", args);
    fprintf (stderr, "Parameters:\n");
    fprintf (stderr, "   <filename>   Input source file in current directory.\n");
    fprintf (stderr,
             "                Enclose long file name with space in double quotes.\n");
    fprintf (stderr, "                If the filename is missing, stdin is used.\n");
    fprintf (stderr, "Switches:\n");
    fprintf (stderr, "   -h           Prints this help message\n\n");
    fprintf (stderr, "   -i<indent>   Number of spaces for each indent level\n");
    fprintf (stderr, "                (possible values: 1-8, default: 2).\n\n");
    fprintf (stderr, "   -r           Remove superfluous new-lines\n");
    fprintf (stderr, "   -t           Output tabs\n\n");
    fprintf (stderr, "Examples:\n");
    fprintf (stderr, "   %s hello.c\n", PGM);
    fprintf (stderr, "   %s hello.cpp 6\n", PGM);
    fprintf (stderr, "   %s hello.java 8 t\n", PGM);
    fprintf (stderr, "   %s \"hello world.cxx\"\n", PGM);
}

/* check MAXLEN of string, also max. length of the line */
static int
check_MAXLEN (void)
{
    int res;

    if (pos > MAXLEN - 10) {
        fprintf (stderr, "%s\n", string);
        fprintf (stderr, "string exceeding MAXLEN = %d\n", MAXLEN);
        res = 1;
    } else {
        res = 0;
    }

    return (res);
}

/* expand indent into tabs and spaces */
static void
p_tabs (int indent, bool use_tabs)
{
    int i, j, k;

    if (tabs < 0) {
        tabs = 0; /* C++ inline gets next { */
    }
    i = tabs * indent; /* calc number of spaces */
    j = i / 8;         /* calc number of tab chars */
    if (use_tabs) {
        i -= 8 * j; /* calc remaining spaces */
        for (k = 0; (k < j); k++) {
            fprintf (stdout, "\t");
        }
    }
    for (k = 0; (k < i); k++) {
        fprintf (stdout, " ");
    }
}

/* get character from stream or return the saved one */
static char
getchr (FILE *infile)
{
    if ((peek < 0) && (last_char != ' ') && (last_char != '\t')) {
        p_char = (char) last_char;
    }
    last_char = (peek < 0) ? getc (infile) : peek;
    if (last_char == (-1)) {
        last_char = 0x1a;
    }
    peek = (-1);

    return (char)((last_char == '\r') ? getchr (infile) : (char) last_char);
}

/* put string with indent logic */
static void
indent_puts (int indent, bool use_tabs)
{
    int k;

    for (k = pos; (k < MAXLEN); k++) {
        string[k] = '\0';
    }
    if (pos > 0) {
        if (s_flg) {
            if (a_flg && (tabs > 0) && (string[0] != '{')) {
                tabs++;
            }
            p_tabs (indent, use_tabs);
            s_flg = FALSE;
            if (a_flg && (tabs > 0) && (string[0] != '{')) {
                tabs--;
            }
            a_flg = FALSE;
        }
        fprintf (stdout, "%s", string);
        pos = 0;
        string[0] = '\0';
    } else {
        if (s_flg) {
            s_flg = FALSE;
            a_flg = FALSE;
        }
    }
}

/* search table for character string and return index number */
static bool
lookup (const char *tab[])
{
    char r;
    int l, kk, k, i;
    bool res = FALSE;

    if (pos >= 1) {
        kk = 0;
        while (string[kk] == ' ') {
            kk++;
        }
        for (i = 0; (tab[i] != 0); i++) {
            l = 0;
            for (k = kk; (r = tab[i][l++]) == string[k] && (r != '\0'); k++)
                ;
            if ((r == '\0') && (string[k] < 'a' || string[k] > 'z')) {
                res = TRUE;
                break;
            }
        }
    }

    return (res);
}

/* read string between double quotes */
static char
get_string (FILE *infile, int indent, bool use_tabs)
{
    char ch;

beg:
    if ((ch = string[pos++] = getchr (infile)) == '\\') {
        string[pos++] = getchr (infile);
        goto beg;
    }
    if ((ch == '\'') || (ch == '"')) {
        while ((cc = string[pos++] = getchr (infile)) != ch) {
            if (cc == '\\') {
                cc = string[pos++] = getchr (infile);
            }
        }
        goto beg;
    }
    if (ch == '\n') {
        indent_puts (indent, use_tabs);
        a_flg = TRUE;
        goto beg;
    }

    return (ch);
}

/* else processing */
static void
gotelse (void)
{
    tabs = save_tabs[c_level][if_lev];
    p_flg[level] = save_p_flg[c_level][if_lev];
    ind[level] = save_ind[c_level][if_lev];
    if_flg = TRUE;
}

/* special edition of put string for comment processing */
static void
putcoms (int indent, bool use_tabs)
{
    int i;

    if (pos > 0) {
        if (s_flg) {
            p_tabs (indent, use_tabs);
            s_flg = FALSE;
        }
        string[pos] = '\0';
        i = 0;
        while (string[i] == ' ') {
            i++;
        }
        if ((string[i] == '/') && (string[i + 1] == '*')) {
            if (last_char != ';') {
                fprintf (stdout, "%s", &string[i]);
            } else {
                fprintf (stdout, "%s", string);
            }
        } else {
            if (string[i] == '*' && string[i + 1] == '/') {
                fprintf (stdout, " %s", &string[i]);
            } else {
                i = 0;
                while (string[i] == ' ') {
                    i++;
                }
                if (string[i] == '*') {
                    fprintf (stdout, "%s", &string[i - 1]);
                } else {
                    fprintf (stdout, " * %s", string);
                }
            }
        }
        pos = 0;
    }
}

/* print cpp comment */
static void
cpp_putcoms (int indent, bool use_tabs)
{
    if (pos > 0) {
        if (s_flg) {
            p_tabs (indent, use_tabs);
            s_flg = FALSE;
        }
        string[pos] = '\0';
        fprintf (stdout, "%s", string);
        pos = 0;
        string[pos] = '\0';
    }
}

/* comment processing */
static void
comment (FILE *infile, int indent, bool use_tabs)
{
    int save_s_flg;

    save_s_flg = s_flg;
rep:
    while ((c = string[pos++] = getchr (infile)) != '*') {
        s_flg = TRUE;
        if (c == '\n') {
            putcoms (indent, use_tabs);
        }
    }

gotstar:
    if ((c = string[pos++] = getchr (infile)) != '/') {
        if (c == '*') {
            goto gotstar;
        }
        s_flg = TRUE;
        if (c == '\n') {
            putcoms (indent, use_tabs);
        }
        goto rep;
    }
    s_flg = TRUE;
    putcoms (indent, use_tabs);
    s_flg = save_s_flg;
}

/* cpp comment processing */
static void
cpp_comment (FILE *infile, int indent, bool use_tabs)
{
    while ((c = getchr (infile)) != '\n') {
        string[pos++] = (char) c;
    }
    cpp_putcoms (indent, use_tabs);
    s_flg = TRUE;
}

/* read to new_line */
static bool
getnl (FILE *infile, int indent, bool use_tabs)
{
    int save_tabs;

    save_tabs = tabs;
    while (((peek = getchr (infile)) == '\t') || (peek == ' ')) {
        peek = (-1);
    }
#if 0
  fprintf( stdout, " ");
#endif
    if ((peek = getchr (infile)) == '/') {
        peek = (-1);
        if ((peek = getchr (infile)) == '*') {
            string[pos++] = '/';
            string[pos++] = '*';
            peek = (-1);
            comment (infile, indent, use_tabs);
        } else if (peek == '/') {
            /* inline C++ comment */
            string[pos++] = '/';
            string[pos++] = '/';
            peek = (-1);
            cpp_comment (infile, indent, use_tabs);

            return (TRUE);
        } else {
            string[pos++] = '/';
        }
    }
    if ((peek = getchr (infile)) == '\n') {
        peek = (-1);

        tabs = save_tabs;
        return (TRUE);
    }

    tabs = save_tabs;
    return (FALSE);
}

/* scans and processes the file */
static int
process_file (FILE *infile, int indent, bool remove_newlines, bool use_tabs)
{
    int paren = 0;

    int s_level[10];
    int save_if_lev[10];
    bool save_if_flg[10];
    bool e_flg = FALSE;
    bool q_flg = FALSE;
    int nl_flag = 0;
    char l_char = '\0';

    const char *w_if_[] = {"if", NULL};
    const char *w_else[] = {"else", NULL};
    const char *w_for[] = {"for", NULL};
    const char *w_ds[] = {"case", "default", NULL};
    const char *w_cpp_comment[] = {"//", NULL};

    while ((c = getchr (infile)) != 032) {
        if ((c != ' ') && (c != '\t') && (c != '\n')) {
            if (nl_flag == 1) {
                fprintf (stdout, " ");
            } else if (nl_flag == 2) {
                fprintf (stdout, "\n");
            }
            nl_flag = 0;
        }

        switch (c) {
        default:
            string[pos++] = (char) c;
            if (c != ',') {
                l_char = (char) c;
            }
            break;
        case ' ':
        case '\t':
            if (lookup (w_else)) {
                gotelse ();
                if ((!s_flg) || (pos > 0)) {
                    string[pos++] = (char) c;
                }
                indent_puts (indent, use_tabs);
                s_flg = FALSE;
                break;
            }
            if ((!s_flg) || (pos > 0)) {
                if ((pos == 0)
                    || ((string[pos - 1] != ' ') && (string[pos - 1] != '\t'))) {
                    string[pos++] = (char) c;
                }
            }
            break;
        case '\n':
            fflush (stdout);
            if ((e_flg = lookup (w_else))) {
                gotelse ();
            }
            if (lookup (w_cpp_comment)) {
                if (string[pos] == '\n') {
                    string[pos] = '\0';
                    pos--;
                }
            }

            indent_puts (indent, use_tabs);
            if (remove_newlines) {
                if (nl_flag == 0) {
                    nl_flag = 1;
                }
            } else {
                fprintf (stdout, "\n");
            }
            s_flg = TRUE;
            if (e_flg == TRUE) {
                p_flg[level]++;
                tabs++;
            } else {
                if (p_char == l_char) {
                    a_flg = TRUE;
                }
            }
            break;
        case '{':
            if (lookup (w_else)) {
                gotelse ();
            }
            save_if_lev[c_level] = if_lev;
            save_if_flg[c_level] = if_flg;
            if_lev = 0;
            if_flg = FALSE;
            c_level++;
            if (s_flg && (p_flg[level] != 0)) {
                p_flg[level]--;
                tabs--;
            }
            string[pos++] = (char) c;
            indent_puts (indent, use_tabs);
            getnl (infile, indent, use_tabs);
            indent_puts (indent, use_tabs);
            nl_flag = 2;
            tabs++;
            s_flg = TRUE;
            if (p_flg[level] > 0) {
                ind[level] = TRUE;
                level++;
                s_level[level] = c_level;
            }
            break;
        case '}':
            c_level--;
            if ((if_lev = save_if_lev[c_level] - 1) < 0) {
                if_lev = 0;
            }
            if_flg = save_if_flg[c_level];
            indent_puts (indent, use_tabs);
            tabs--;
            p_tabs (indent, use_tabs);
            if ((peek = getchr (infile)) == ';') {
                fprintf (stdout, "%c;", c);
                peek = (-1);
            } else {
                fprintf (stdout, "%c", c);
            }
            getnl (infile, indent, use_tabs);
            indent_puts (indent, use_tabs);
            nl_flag = 2;
            s_flg = TRUE;
            if (c_level < s_level[level]) {
                if (level > 0) {
                    level--;
                }
            }
            if (ind[level]) {
                tabs -= p_flg[level];
                p_flg[level] = 0;
                ind[level] = FALSE;
            }
            break;
        case '"':
        case '\'':
            string[pos++] = (char) c;
            while ((cc = getchr (infile)) != c) {
                if (check_MAXLEN ()) {
                    return (90); /* max. length of line should be MAXLEN */
                }
                string[pos++] = cc;
                if (cc == '\\') {
                    cc = string[pos++] = getchr (infile);
                }
                if (cc == '\n') {
                    indent_puts (indent, use_tabs);
                    s_flg = TRUE;
                }
            }
            string[pos++] = cc;
            if (getnl (infile, indent, use_tabs)) {
                l_char = cc;
                peek = '\n';
            }
            break;
        case ';':
            string[pos++] = (char) c;
            indent_puts (indent, use_tabs);
            if ((p_flg[level] > 0) && (!ind[level])) {
                tabs -= p_flg[level];
                p_flg[level] = 0;
            }
            getnl (infile, indent, use_tabs);
            indent_puts (indent, use_tabs);
            nl_flag = 2;
            s_flg = TRUE;
            if (if_lev > 0) {
                if (if_flg) {
                    if_lev--;
                    if_flg = FALSE;
                } else {
                    if_lev = 0;
                }
            }
            while (((peek = getchr (infile)) == ';') || (peek == ' ') || (peek == '\t')
                   || (peek == '\n')) {
                peek = (-1);
            }
            break;
        case '\\':
            string[pos++] = (char) c;
            string[pos++] = getchr (infile);
            break;
        case '?':
            q_flg = TRUE;
            string[pos++] = (char) c;
            break;
        case ':':
            string[pos++] = (char) c;
            peek = getchr (infile);
            if (peek == ':') {
                indent_puts (indent, use_tabs);
                fprintf (stdout, ":");
                peek = (-1);
                break;
            }

            if (q_flg) {
                q_flg = FALSE;
                break;
            }

            if (lookup (w_ds)) {
                tabs--;
                indent_puts (indent, use_tabs);
                tabs++;
            } else {
                s_flg = FALSE;
                indent_puts (indent, use_tabs);
            }
            if ((peek = getchr (infile)) == ';') {
                fprintf (stdout, ";");
                peek = (-1);
            }
            getnl (infile, indent, use_tabs);
            indent_puts (indent, use_tabs);
            nl_flag = 2;
            s_flg = TRUE;
            break;
        case '/':
            string[pos++] = (char) c;
            if ((peek = getchr (infile)) == '/') {
                string[pos++] = (char) peek;
                peek = (-1);
                cpp_comment (infile, indent, use_tabs);
                nl_flag = 2;
            } else if ((peek = getchr (infile)) == '*') {
#if 1
                if (remove_newlines) {
                    fprintf (stdout, "\n");
                }
#endif
                string[pos--] = '\0';
                indent_puts (indent, use_tabs);
                string[pos++] = '/';
                string[pos++] = '*';
                peek = (-1);
                comment (infile, indent, use_tabs);
#if 1
                if (remove_newlines) {
                    nl_flag = 2;
                }
#endif
            }
            break;
        case ')':
            paren--;
            string[pos++] = (char) c;
            indent_puts (indent, use_tabs);
            if (getnl (infile, indent, use_tabs)) {
                peek = '\n';
                if (paren != 0) {
                    a_flg = TRUE;
                } else if (tabs > 0) {
                    p_flg[level]++;
                    tabs++;
                    ind[level] = FALSE;
                }
            }
            break;
        case '#':
            string[pos++] = (char) c;
            while ((cc = getchr (infile)) != '\n') {
                string[pos++] = cc;
            }
            string[pos++] = cc;
            s_flg = FALSE;
            indent_puts (indent, use_tabs);
            s_flg = TRUE;
            break;
        case '(':
            string[pos++] = (char) c;
            paren++;
            if (lookup (w_for)) {
                int cnt;

                while ((c = get_string (infile, indent, use_tabs)) != ';')
                    ;
                cnt = 0;
            cont:
                while ((c = get_string (infile, indent, use_tabs)) != ')') {
                    if (c == '(') {
                        cnt++;
                    }
                }
                if (cnt != 0) {
                    cnt--;
                    goto cont;
                }
                paren--;
                indent_puts (indent, use_tabs);
                if (getnl (infile, indent, use_tabs)) {
                    peek = '\n';
                    p_flg[level]++;
                    tabs++;
                    ind[level] = FALSE;
                }
                break;
            }
            if (lookup (w_if_)) {
                indent_puts (indent, use_tabs);
                save_tabs[c_level][if_lev] = tabs;
                save_p_flg[c_level][if_lev] = p_flg[level];
                save_ind[c_level][if_lev] = ind[level];
                if_lev++;
                if_flg = TRUE;
            }
        }
    }
    fprintf (stdout, "\n");

    return (0);
}

int
main (int argc, char *argv[])
{
    FILE *infile = NULL;
    int indent = 2;
    bool remove_newlines = FALSE;
    bool use_tabs = FALSE;
    int ret;

    /* process arguments */

    ARGS_BEGIN (argc, argv);
    ARGS_FLAG ("h", usage (argv[0]); exit (0););
    ARGS_FLAG ("V", printf (VERS); /* tell the world about version */
               exit (0););
    ARGS_OPTION ("i", ARG_RANGE (indent, 1, 8));
    ARGS_FLAG ("r", remove_newlines = TRUE;);
    ARGS_FLAG ("t", use_tabs = TRUE;);
    ARGS_ARGUMENT (if ((infile = fopen (ARG, "r")) == NULL) {
        fprintf (stderr, "File not found: %s\n", ARG);
        exit (2);
    });
    ARGS_END ();

    if (!infile) {
        infile = stdin;
    }

    /* end of initialization */

    ret = process_file (infile, indent, remove_newlines, use_tabs);

    /* eof processing */

    fclose (infile);

    return (ret);
}
