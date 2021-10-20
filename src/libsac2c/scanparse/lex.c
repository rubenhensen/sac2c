#include <sys/param.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>

#ifndef LEXER_BINARY
#include "types.h"
#define DBUG_PREFIX "LEXER"
#include "debug.h"
#else
#define DBUG_PRINT(Hai, ...) (void)0
#endif

#include "globals.h"
#include "lex.h"

#define TOKEN_KIND(a, b) b,
#define KEYWORD(a, b) b,
#define KEYWORD_PRF(a, b, c) b,
const char *token_kind_name[] = {
#include "token_kind.def"
#include "prf.def"
#include "keywords.def"
};
#undef TOKEN_KIND
#undef KEYWORD
#undef KEYWORD_PRF

#define TOKEN_CLASS(a, b) b,
const char *token_class_name[] = {
#include "token_class.def"
};
#undef TOKEN_CLASS

/* Code to separate keywords from the other predefined tokens.  */
#define first_keyword PRF_ABS_S

/* This is a pointer to the first token from keywords.def  */
/*static const char **  keywords = &token_kind_name[(int) first_keyword];
static size_t keywords_length = tok_kind_length  - first_keyword;  */
static size_t operators_length = (size_t)first_keyword;

/* Define error and warning counters if we create a lexer binary only,
   otherwise use the counters from ctinfo.c.  */
#ifdef LEXER_BINARY
int error_count = 0;
int warning_count = 0;
#endif

/* SaC operates with yyin file.  */
FILE *yyin;

/* Binary search function to search string in a char** table.  */
static inline size_t
kw_bsearch (const char *key, const char *table[], size_t len)
{
    size_t l = 0, r = len;

    while (l < r) {
        size_t hit = (l + r) / 2;
        int i = strcmp (key, table[hit]);
        /*printf ("%s ? %s, [%i, %i, %i]\n", key, table[hit], l, r, i);*/

        if (i == 0)
            return hit;
        else if (i < 0)
            r = hit;
        else
            l = hit + 1;
    }
    return len;
}

/* Initialize lexer LEX with a file name FNAME and
   set initial parameters of the lexer.  */
bool
lexer_init (struct lexer *lex, const char *fname)
{
    FILE *f;

    assert (fname != NULL, "lexer initialized with empty filename");
    assert (NULL != (f = fopen (fname, "r")), "error opening file %s", fname);

    return lexer_init_file (lex, f, fname);
}

/* FIXME Currently we do most of the initialisation here as we are
   calling this function from parser.c  */
bool
lexer_init_file (struct lexer *lex, FILE *f, const char *fname)
{
    assert (lex != NULL, "lexer memory is not allocated");
    assert (lex->fname == NULL && lex->file == NULL, "attempt to reinitialize lexer");

    lex->trie_user = NULL;
    lex->file_names = NULL;
    lex->trie = trie_new ();

    lex->is_eof = false;
    lex->fname = lexer_change_file_name (lex, fname);
    lex->file = f;
    lex->loc = (struct location){lex->fname, 1, 0};

    lex->is_read_user_op = false;

    lex->buf_start = 0;
    lex->buf_end = 0;
    lex->unget_idx = 0;

/* Now we are going to add keywords and token_kinds
   into the trie of the lexer, assigning an appropiate
   token_kind into the info field of the trie child.  */
#define TOKEN_KIND(a, b)                                                                 \
    if (a != tv_eof)                                                                     \
        trie_add_word (lex->trie, b, strlen (b), (ssize_t)a);
#include "token_kind.def"
#undef TOKEN_KIND

/* The same for primitive functions.  */
#define KEYWORD_PRF(a, b, c) trie_add_word (lex->trie, b, strlen (b), (ssize_t)PRF_##a);
#include "prf.def"
#undef KEYWORD_PRF

/* The same for keywors, using special TIRE_KEYWORD value.  */
#define KEYWORD(a, b) trie_add_word (lex->trie, b, strlen (b), (ssize_t)a);
#include "keywords.def"
#undef KEYWORD

    if (!lex->file) {
        warn ("error opening file `%s'", fname);
        return false;
    }

    return true;
}

/* Actions before deallocating lexer.  */
bool
lexer_finalize (struct lexer *lex, bool close_file)
{
    struct file_name *f;
    struct file_name *tmp;

    if (close_file)
        (void)fclose (lex->file);

    trie_free (lex->trie);

    /* Free the file-name hash-table.  */
    HASH_ITER (hh, lex->file_names, f, tmp) {
        HASH_DEL (lex->file_names, f);

        if (f->name)
            free (f->name);

        free (f);
    }

    return true;
}

static inline void
add_char_to_buffer (struct lexer *lex, int ic)
{
    size_t e = lex->buf_end;
    char c = (char)ic;

    if ((lex->buf_end + 1) % LEXER_BUFFER == lex->buf_start) {
        lex->buf_start = (lex->buf_start + 1) % LEXER_BUFFER;
        lex->buffer[lex->buf_end] = c;
        lex->buf_end = (lex->buf_end + 1) % LEXER_BUFFER;
    } else {
        lex->buffer[lex->buf_end] = c;
        lex->buf_end = (lex->buf_end + 1) % LEXER_BUFFER;
    }

    lex->loc.col++;

    if (c == '\n') {
        lex->loc.line++;
        lex->loc.col = 0;
    }

    lex->location_buffer[e] = lex->loc;
}

/* Gets one character from the file, is end of file is
   reached, it will return EOF in all the consequent calls.  */
static inline int
lexer_getch (struct lexer *lex)
{
    int ch;

    if (lex->is_eof)
        return EOF;

    if (lex->unget_idx == 0) {
        ch = fgetc (lex->file);
        add_char_to_buffer (lex, ch);

        if (ch == EOF) {
            lex->is_eof = true;
            return EOF;
        }

        if (ch == '\\') {
            int c = fgetc (lex->file);
            if (c == '\n') {
                add_char_to_buffer (lex, ' ');
                lex->loc.line++;
                lex->loc.col = 0;
                return lexer_getch (lex);
            }

            ungetc (c, lex->file);
        }

        return ch;
    } else {
        size_t s;

        /* Return a token from the buffer.  */
        assert (lex->unget_idx < LEXER_BUFFER,
                "parser buffer holds only up to %i values.", LEXER_BUFFER);

        s = circbuf_idx_dec (lex->buf_end, lex->unget_idx, LEXER_BUFFER);
        lex->unget_idx--;
        lex->loc = lex->location_buffer[s];

        /*if (lex->unget_idx == 0 && lex->buffer[s] == '\n')
          {
            lex->loc.line++;
            lex->loc.col = 0;
          }*/

        return lex->buffer[s];
    }
}

/* Put character back on the stream of the lexer.
   Consequent lexer_getch should return exactly this character.  */
static inline void
lexer_ungetch (struct lexer *lex, int ch)
{
    size_t s;
    (void)ch; /* Surpress unused variable warning */

    lex->unget_idx++;
    assert (lex->unget_idx < LEXER_BUFFER, "parser buffer holds only up to %i values.",
            LEXER_BUFFER);

    s = circbuf_idx_dec (lex->buf_end, lex->unget_idx, LEXER_BUFFER);
}

/* Adds the character C to the string *BUFFER that has length *SIZE
   at the position *INDEX. *INDEX is a pointer in the *BUFFER.
   If the *BUFFER is NULL then it is being allocated, if the *INDEX
   points at the end of the *BUFFER the *BUFFER will be reallocated. */
static inline void
buffer_add_char (char **buffer, char **index, size_t *size, int ic)
{
    const size_t initial_size = 16;
    char c = (char)ic;

    if (*buffer == NULL) {
        *buffer = (char *)malloc (initial_size * sizeof (char));
        *index = *buffer;
        *(*index)++ = c;
        *size = initial_size;
        return;
    }

    assert (*index <= *buffer + *size, "index is greater than allocated buffer");

    if (*index == *buffer + *size) {
        *buffer = (char *)realloc (*buffer, *size * 2 * sizeof (char));
        *index = *buffer + *size;
        *size *= 2;
    }

    *(*index)++ = c;
}

/* Internal function to read until the end of comment.  */
static inline enum token_class
lexer_read_comments (struct lexer *lex, char **buf, size_t *size)
{
    char *index = *buf;
    int prev = '\0';

    buffer_add_char (buf, &index, size, '/');
    buffer_add_char (buf, &index, size, '*');

    while (true) {
        int c = lexer_getch (lex);
        if (c == EOF) {
            error_loc (lex->loc, "unexpected end of file in the middle of comment");
            buffer_add_char (buf, &index, size, 0);
            return tok_unknown;
        }

        buffer_add_char (buf, &index, size, c);
        if (c == '/' && prev == '*')
            break;

        prev = c;
    }

    buffer_add_char (buf, &index, size, 0);
    return tok_comments;
}

/* Internal function to read until the end of line comment.  */
static inline enum token_class
lexer_read_line_comment (struct lexer *lex, char **buf, size_t *size)
{
    char *index = *buf;

    buffer_add_char (buf, &index, size, '/');
    buffer_add_char (buf, &index, size, '/');

    while (true) {
        int c = lexer_getch (lex);
        if (c == EOF) {
            warning_loc (lex->loc, "unexpected end of file in line comment");
            break;
        }

        buffer_add_char (buf, &index, size, c);
        if (c == '\n')
            break;
    }

    buffer_add_char (buf, &index, size, 0);
    return tok_comments;
}

bool
is_normal_id (const char *id)
{
    size_t i;

    for (i = 0; i < strlen (id); i++) {
        int c = id[i];
        if (!isalpha (c) && !isdigit (c) && c != '_')
            return false;
    }

    return true;
}

bool
is_operator (const char *id)
{
    size_t search = kw_bsearch (id, token_kind_name, operators_length);

    if (search == operators_length)
        return false;

    if ((enum token_kind)search == tv_eof)
        return false;

    return true;
}
/* We allow user-operators to have all the different awkward names,
   which means that when handling while definition we have to forget
   about all the possible existing tokens, and add a newly-defined
   token in the user-ops.  */
static inline enum token_class
lexer_read_user_op (struct lexer *lex, struct token *tok, char **buf, size_t *size)
{
    char *index = *buf;
    int i = 0;
    ssize_t search;
    int c;

    assert (lex->is_read_user_op, "One has to set-up lexer in the "
                                  "user-operation reading mode");

    c = lexer_getch (lex);
    if (c == '(') {
        tval_tok_init (tok, tok_operator, tv_lparen);
        return tok->tok_class;
    } else if (c == ',') {
        tval_tok_init (tok, tok_operator, tv_comma);
        return tok->tok_class;
    } else if (c == '}') {
        tval_tok_init (tok, tok_operator, tv_rbrace);
        return tok->tok_class;
    }

    else
        lexer_ungetch (lex, c);

    for (i = 0;; i++) {
        int c = lexer_getch (lex);

        if (isspace (c) || c == ')' || c == '(' || c == ',' || c == '{' || c == '}'
            || c == ';' || c == EOF) {
            lexer_ungetch (lex, c);

            if (i != 0)
                break;
            else {
                error_loc (lex->loc,
                           "unallowed symbol `%c' inside the name "
                           "of the operation",
                           c);
                buffer_add_char (buf, &index, size, 0);
                return tok_unknown;
            }
        }

        /* FIXME we can adjust it later.  */
        if ((i == 0 && isdigit (c)) || c == '"' || c == '\'' || c == ',' || c == '['
            || c == ']') {
            lexer_ungetch (lex, c);
            error_loc (lex->loc,
                       "unallowed symbol `%c' inside the name "
                       "of the operation",
                       c);
            buffer_add_char (buf, &index, size, 0);
            return tok_unknown;
        }

        buffer_add_char (buf, &index, size, c);
    }

    buffer_add_char (buf, &index, size, 0);

    if (strlen (*buf) == 0) {
        error_loc (lex->loc, "the length of the name of the function is zero");
        return tok_unknown;
    }

    search = trie_search (lex->trie, *buf, strlen (*buf));
    if (search == TRIE_NOT_LAST) {
        if (!is_normal_id (*buf))
            trie_add_word (lex->trie, *buf, strlen (*buf), TRIE_USEROP);
    } else if (search != TRIE_USEROP) {
        if (is_normal_id (*buf))
            tval_tok_init (tok, tok_keyword, (enum token_kind)search);
        else
            tval_tok_init (tok, tok_operator, (enum token_kind)search);

        if (*buf)
            free (*buf);
        *buf = NULL;
        return tok->tok_class;
    }

    return tok_user_op;
}

static ssize_t
lexer_trie_read (struct lexer *lex, struct trie *trie, char **buf, size_t written,
                 size_t *size)
{
    /* We have a word inside the buf which we may possily append,
       so stand at the beginning.  */
    char *index = buf == NULL ? NULL : *buf + written;
    int c = lexer_getch (lex);
    ssize_t last;
    const char cb = (char)c;
    struct trie *next = trie_check_prefix (trie, &cb, 1, &last);

    if (!next && last == TRIE_NOT_LAST) {
        lexer_ungetch (lex, c);
        buffer_add_char (buf, &index, size, 0);
        return TRIE_NOT_LAST;
    } else if (!next && last != TRIE_NOT_LAST) {
        buffer_add_char (buf, &index, size, c);
        buffer_add_char (buf, &index, size, 0);
        return last;
    } else {
        ssize_t res;
        size_t s = buf == NULL ? 0 : (size_t) (index - *buf);

        buffer_add_char (buf, &index, size, c);
        res = lexer_trie_read (lex, next, buf, s + 1, size);

        if (res != TRIE_NOT_LAST)
            return res;
        else {
            assert (buf && *buf, "buf should not be NULL here");
            index = *buf + s + 1;
            if (last != TRIE_NOT_LAST) {
                buffer_add_char (buf, &index, size, 0);
                return last;
            } else {
                lexer_ungetch (lex, c);
                index = *buf + s;
                buffer_add_char (buf, &index, size, 0);
                return TRIE_NOT_LAST;
            }
        }
    }

#if 0
  /*
   * the next two lines appear to be dead code.... (Bodo 2016)
   */
  if (index)
    buffer_add_char (buf, &index, size, 0);
#endif
}

/* This function checks if buffer can be appended with the
   symbols pointed by lex, and using trie_user.  */
static inline ssize_t
lexer_is_user_op (struct lexer *lex, struct token *tok, char **buf, size_t *size)
{
    char *index = *buf;
    struct trie *trie;
    ssize_t last;
    size_t s;

    if (lex->trie_user && lex->trie_user->children_count == 0)
        return TRIE_NOT_LAST;

    /* We assume that buffer contains a string representation of
       the token we are checkoing.  */
    trie = trie_check_prefix (lex->trie_user, *buf, strlen (*buf), &last);
    s = strlen (*buf);
    if (!trie)
        return TRIE_NOT_LAST;

    if (TRIE_NOT_LAST != (last = lexer_trie_read (lex, trie, buf, s, size)))
        return last;
    else {
        index = *buf + s;
        buffer_add_char (buf, &index, size, 0);
        return TRIE_NOT_LAST;
    }
}

char *
_quote_string (const char *s, char *res, size_t pos, bool json_p)
{
#define BUF_SIZE 512

    char buffer[BUF_SIZE];
    const char *ptr = s;
    size_t count = 0;

    while (*ptr != '\0' && count < BUF_SIZE - 5) {
        switch (*ptr) {
        case '\a':
            if (!json_p) {
                buffer[count++] = '\\'; buffer[count++] = 'a';
            } else
                buffer[count++] = ' ';
            break;
        case '\b':
            buffer[count++] = '\\'; buffer[count++] = 'b';
            break;
        case '\f':
            buffer[count++] = '\\'; buffer[count++] = 'f';
            break;
        case '\n':
            buffer[count++] = '\\'; buffer[count++] = 'n';
            break;
        case '\r':
            buffer[count++] = '\\'; buffer[count++] = 'r';
            break;
        case '\t':
            buffer[count++] = '\\'; buffer[count++] = 't';
            break;
        case '\v':
            if (!json_p) {
                buffer[count++] = '\\'; buffer[count++] = 'v';
            } else
                buffer[count++] = ' ';
            break;
        case '\"':
            buffer[count++] = '\\'; buffer[count++] = '\"';
            break;
        case '\'':
            if (!json_p) {
                buffer[count++] = '\\'; buffer[count++] = '\'';
            } else
                buffer[count++] = '\'';
            break;
        case '\\':
            buffer[count++] = '\\'; buffer[count++] = '\\';
            break;
        default: {
            int x1, x2;

            if (isprint (*ptr)) {
                buffer[count++] = *ptr;
                break;
            }

            x1 = *ptr / 16;
            x2 = *ptr % 16;

            x1 = x1 < 10 ? '0' + x1 : 'a' + x1 - 10;
            x2 = x2 < 10 ? '0' + x2 : 'a' + x2 - 10;

            buffer[count++] = 'x';
            buffer[count++] = (char)x1;
            buffer[count++] = (char)x2;
        } break;
        }
        ptr++;
    }

    if (*ptr == '\0') {
        res = (char *)malloc ((pos + count + 1) * sizeof (char));
        memcpy (&res[pos], buffer, count);
        res[pos + count] = '\0';
        return res;
    } else
        res = _quote_string (ptr, res, pos + count, json_p);

    memcpy (&res[pos], buffer, count);
    return res;
}

char *
quote_string (const char *s, char *res, size_t pos) {
    return _quote_string (s, res, pos, false);
}

char *
quote_string_json (const char *s, char *res, size_t pos) {
    return _quote_string (s, res, pos, true);
}



static int
lexer_read_escape_char (struct lexer *lex, bool *error)
{
    int c = lexer_getch (lex);
    *error = false;

    switch (c) {
    case EOF:
        error_loc (lex->loc, "unexpected end of file in escape sequence");
        *error = true;
        return EOF;
    case 'a':
        return '\a';
    case 'b':
        return '\b';
    case 'f':
        return '\f';
    case 'n':
        return '\n';
    case 'r':
        return '\r';
    case 't':
        return '\t';
    case 'v':
        return '\v';
    case '\'':
        return '\'';
    case '\"':
        return '\"';
    case '\\':
        return '\\';
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7': {
        int res = c - '0';
        c = lexer_getch (lex);

        /* Possible second octal digit  */
        if (c >= '0' && c <= '7')
            res = res * 8 + (c - '0');
        else {
            lexer_ungetch (lex, c);
            return res;
        }

        /* Possible third ocatal digit  */
        if (c >= '0' && c <= '7')
            res = res * 8 + (c - '0');
        else
            lexer_ungetch (lex, c);

        return res;
    }
    case 'x': {
        int res;

        c = lexer_getch (lex);
        if (!isdigit (c) && !((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            error_loc (lex->loc, "hexadecimal digit must follow "
                                 "escape symbol 'x'");
            *error = true;
            break;
        }

        res = isdigit (c) ? c - '0' : toupper (c) - 'A' + 10;

        while (true) {
            c = lexer_getch (lex);
            if (isdigit (c) || (toupper (c) >= 'A' && toupper (c) <= 'F'))
                res = res * 16 + (isdigit (c) ? c - '0' : toupper (c) - 'A' + 10);
            else {
                // return res;
                lexer_ungetch (lex, c);
                break;
            }
        }
        return res;
    }
    default:
        break;
    }

    return c;
}

/* One may want to come-up with a nicer
   formulation of the very same function.  */
static inline void
lexer_skip_comments (struct lexer *lex)
{
    int c;
    bool ret = false;

    do {
        c = lexer_getch (lex);
        if (isspace (c)) {
            while (isspace (c))
                c = lexer_getch (lex);
            lexer_ungetch (lex, c);
            ret = true;
        } else if (c == '/') {
            int c1 = lexer_getch (lex);
            int prev = '\0';

            ret = true;
            if (c1 == '*')
                while (true) {
                    int c2 = lexer_getch (lex);
                    if (c2 == EOF) {
                        error_loc (lex->loc, "unexpected end of file in the "
                                             "middle of comment");
                        return;
                    }

                    if (c2 == '/' && prev == '*')
                        break;

                    prev = c2;
                }
            else if (c1 == '/')
                while (true) {
                    int c2 = lexer_getch (lex);
                    if (c2 == EOF) {
                        error_loc (lex->loc, "unexpected end of file in the "
                                             "middle of line comment");
                        return;
                    }

                    if (c2 == '\n')
                        break;
                }
            else {
                lexer_ungetch (lex, c1);
                lexer_ungetch (lex, c);
                ret = false;
            }
        } else {
            lexer_ungetch (lex, c);
            ret = false;
        }
    } while (ret);
}

/* Internal function to read until the end of string/char ignoring
   escape sequences.  */
static inline enum token_class
lexer_read_string (struct lexer *lex, char **buf, size_t *size, int c)
{
    char *index = *buf;
    enum token_class tok_class;
    const int stop = c;

    assert (stop == '"' || stop == '\'',
            "inapproriate starting symbol for string or char");
    tok_class = stop == '"' ? tok_string : tok_char;

    if (stop == '\'') {
        c = lexer_getch (lex);
        if (c == EOF) {
            error_loc (lex->loc, "unexpected end of file in the "
                                 "middle of character");
            goto return_unknown;
        } else if (c == '\\') {
            bool error;
            int res = lexer_read_escape_char (lex, &error);
            if (error)
                goto return_unknown;
            else
                buffer_add_char (buf, &index, size, (char)res);
        } else if (c == '\'') {
            buffer_add_char (buf, &index, size, 0);
            return tok_class;
        } else
            buffer_add_char (buf, &index, size, c);

        c = lexer_getch (lex);
        if (c != '\'') {
            error_loc (lex->loc, "closing \"'\" expected, '%c' found instead", c);
            goto return_unknown;
        }
    }

    /* we can have several strings followed by spaces  */
    else
        while (true) {
            /* read one string  */
            while (true) {
                c = lexer_getch (lex);
                if (c == EOF) {
                    error_loc (lex->loc, "unexpected end of file in the "
                                         "middle of the string");
                    buffer_add_char (buf, &index, size, 0);
                    return tok_unknown;
                } else if (c == stop)
                    break;
                else if (c == '\\') {
                    bool error;
                    int res = lexer_read_escape_char (lex, &error);
                    if (error)
                        goto return_unknown;
                    else
                        c = res;
                }

                buffer_add_char (buf, &index, size, c);
            }

            /* skip whitespaces and comments */
            lexer_skip_comments (lex);

            c = lexer_getch (lex);

            if (c != stop) {
                lexer_ungetch (lex, c);
                break;
            }
        }

    buffer_add_char (buf, &index, size, 0);
    return tok_class;

return_unknown:
    buffer_add_char (buf, &index, size, 0);
    return tok_unknown;
}

/* Internal function to read until the end of identifier, checking
   if it is a keyword.  */
static inline void
lexer_read_id (struct lexer *lex, struct token *tok, char **buf, size_t *size, int c)
{
    char *index = *buf;
    ssize_t search;

    do {
        buffer_add_char (buf, &index, size, c);
        c = lexer_getch (lex);
    } while (isalnum (c) || c == '_');
    lexer_ungetch (lex, c);
    buffer_add_char (buf, &index, size, 0);

    search = trie_search (lex->trie, *buf, strlen (*buf));
    if (search != TRIE_NOT_LAST) {
        tval_tok_init (tok, tok_keyword, (enum token_kind)search);
    } else
        tok->tok_class = tok_id;
}

/* Internal function to read until the end of number.  */
static inline enum token_class
lexer_read_number (struct lexer *lex, char **buf, size_t *size, int c)
{
    char *index = *buf;
    bool isoctal = false;
    bool ishex = false;
    bool isreal = false;
    bool saw_dot = false;
    bool saw_exp = false;
    enum token_class tclass = tok_unknown;
    bool unsigned_p = false;

    /* first digit  */
    buffer_add_char (buf, &index, size, c);

    if (c == '0') {
        c = lexer_getch (lex);

        if (c == 'x' || c == 'X')
            ishex = true;
        else if (isdigit (c)) {
            isoctal = true;
            if (!(c >= '0' && c <= '7'))
                error_loc (lex->loc, "%c found in the octal number", c);
        } else if (c == '.') {
            isreal = true;
            saw_dot = true;
        } else {
            /* Note that we specially do not unget here.  */
            goto read_postfix;
        }
        buffer_add_char (buf, &index, size, c);
    }

    /* middle of the number  */
    while (true) {
        c = lexer_getch (lex);
        if (isdigit (c)) {
            if (isoctal && c >= '8') {
                error_loc (lex->loc, "'%c' found in octal number", c);
                goto return_unknown;
            }
        } else if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
            /* This might be a postfix or an exponent.  */
            if (!ishex)
                goto read_postfix;
        } else if (c == '.') {
            if (saw_dot) {
                error_loc (lex->loc, "more than one dot in the number");
                goto return_unknown;
            }
            saw_dot = true;
            isreal = true;
        } else
            break;

        buffer_add_char (buf, &index, size, c);
    }

read_postfix:
    if (c == 'p' || c == 'P' || c == 'e' || c == 'E') {
        if (saw_exp) {
            error_loc (lex->loc, "exponent is specified more than once");
            goto return_unknown;
        }
        saw_exp = true;
        isreal = true;

        /* Add exponent to the buffer.  */
        buffer_add_char (buf, &index, size, c);

        c = lexer_getch (lex);
        if (c == '+' || c == '-') {
            buffer_add_char (buf, &index, size, c);
        } else {
            // Optional sign (+ or - ) for exponent
            if (isdigit (c)) {
                DBUG_PRINT ("Faking up + sign for exponent");
                lexer_ungetch (lex, c);
                buffer_add_char (buf, &index, size, '+');
            } else {
                error_loc (lex->loc, "+ or - expected after exponent");
                goto return_unknown;
            }
        }

        c = lexer_getch (lex);

        if (!isdigit (c)) {
            error_loc (lex->loc, "digit expected after exponent sign");
            goto return_unknown;
        } else
            buffer_add_char (buf, &index, size, c);

        while (isdigit (c = lexer_getch (lex)))
            buffer_add_char (buf, &index, size, c);
    }

    /* deal with the postfixes.  */
    if (c == 'u' || c == 'U') {
        unsigned_p = true;
        c = lexer_getch (lex);
    }

    if (c == 'l' || c == 'L') {
        c = lexer_getch (lex);
        if (c == 'l' || c == 'L')
            tclass = unsigned_p ? tok_number_ulonglong : tok_number_longlong;
        else {
            lexer_ungetch (lex, c);
            tclass = unsigned_p ? tok_number_ulong : tok_number_long;
        }
    } else if (c == 'b' || c == 'B')
        tclass = unsigned_p ? tok_number_ubyte : tok_number_byte;
    else if (c == 's' || c == 'S')
        tclass = unsigned_p ? tok_number_ushort : tok_number_short;
    else if (c == 'i' || c == 'I')
        tclass = unsigned_p ? tok_number_uint : tok_number_int;
    else if (c == 'f' || c == 'F')
        tclass = tok_number_float;
    else if (c == 'd' || c == 'D')
        tclass = tok_number_double;
    else {
        if (isreal || saw_dot || saw_exp)
            tclass = tok_number_double;
        else
            if (unsigned_p)
                tclass = tok_number_uint;
            else
                tclass = tok_number;
        lexer_ungetch (lex, c);
    }

    if ((saw_dot || saw_exp || isreal) && tclass != tok_number_float
        && tclass != tok_number_double)
        error_loc (lex->loc, "real number is followed by integer postfix");

    c = lexer_getch (lex);
    if (isalpha (c) || c == '_') {
        error_loc (lex->loc, "unexpected number suffix");
        goto return_unknown;
    }
    lexer_ungetch (lex, c);

    buffer_add_char (buf, &index, size, 0);
    return tclass;

return_unknown:
    buffer_add_char (buf, &index, size, 0);
    return tok_unknown;
}

/* Handle the line directive in form:
   # <num> "<fname>" <flags><EOL>  */
void
read_line_directive (struct lexer *lex, int digit)
{
    char fname[PATH_MAX];
    size_t line = (size_t)(digit - '0');
    bool ret = true;
    int i = 0;

    while (isdigit (digit = lexer_getch (lex)))
        line = line * 10 + (size_t)(digit - '0');

    while (isspace (digit = lexer_getch (lex)))
        digit = lexer_getch (lex);

    if (digit != '"') {
        error_loc (lex->loc, "filename in line directive must be quoted");
        goto skip;
    }

    while ((digit = lexer_getch (lex)) != '"') {
        fname[i++] = (char)digit;
        if (i == PATH_MAX - 1) {
            error_loc (lex->loc, "filename is too long");
            ret = false;
            goto skip;
        }
    }
    fname[i] = '\0';

    if (digit != '"') {
        error_loc (lex->loc, "filename in line directive must be quoted");
        ret = false;
        goto skip;
    }

skip:
    while (lexer_getch (lex) != '\n')
        ;
    if (ret) {
        lex->fname = lexer_change_file_name (lex, fname);

        lex->loc.line = line;
        lex->loc.col = 0;
        lex->loc.fname = lex->fname;
    }
    return;
}

/* Reads the stream from lexer and returns dynamically allocated token
   of the appropriate type.  */
struct token *
lexer_get_token (struct lexer *lex)
{
    int c;
    struct location loc = lex->loc;
    struct token *tok = (struct token *)malloc (sizeof (struct token));
    size_t buf_size = 16;
    char *buf = NULL;
    char *index = buf;
    ssize_t search;

    assert (tok != NULL, "cannot allocate token");
    tok->tok_class = tok_class_length;

    c = lexer_getch (lex);
    loc = lex->loc;

    if (isspace (c)) {
        buffer_add_char (&buf, &index, &buf_size, c);
        while (EOF != (c = lexer_getch (lex)) && isspace (c))
            buffer_add_char (&buf, &index, &buf_size, c);
        buffer_add_char (&buf, &index, &buf_size, 0);
        lexer_ungetch (lex, c);

        tok->tok_class = tok_whitespace;
        goto return_token;
    }

    while (c == '#') {
        int c1;
        while (isspace (c1 = lexer_getch (lex)))
            ;

        if (isdigit (c1))
            read_line_directive (lex, c1);
        else {
            lexer_ungetch (lex, c1);
            tval_tok_init (tok, tok_operator, tv_hash);
            goto return_token;
        }

        do
            c = lexer_getch (lex);
        while (isspace (c));
    }

    /* XXX do we want to return whitespace here as well?  */
    if (isspace (c)) {
        while (EOF != (c = lexer_getch (lex)) && isspace (c))
            ;
    }
    loc = lex->loc;

    if (c == EOF) {
        tval_tok_init (tok, tok_eof, tv_eof);
        goto return_token;
    }

    if (lex->is_read_user_op) {
        lexer_ungetch (lex, c);
        tok->tok_class = lexer_read_user_op (lex, tok, &buf, &buf_size);
        goto return_token;
    }

    if (c == '/') {
        int c1 = lexer_getch (lex);

        tval_tok_init (tok, tok_operator, tv_div);
        if (c1 == '*')
            tok->tok_class = lexer_read_comments (lex, &buf, &buf_size);
        else if (c1 == '/')
            tok->tok_class = lexer_read_line_comment (lex, &buf, &buf_size);
        else if (c1 == '=')
            tval_tok_init (tok, tok_operator, tv_div_eq);
        else
            lexer_ungetch (lex, c1);
        goto return_token;
    }

    if (c == '"' || c == '\'') {
        tok->tok_class = lexer_read_string (lex, &buf, &buf_size, c);
        goto return_token;
    }

    if (isalpha (c) || c == '_') {
        lexer_read_id (lex, tok, &buf, &buf_size, c);
        goto return_token;
    }

    if (c == '.') {
        int c2 = lexer_getch (lex);
        lexer_ungetch (lex, c2);
        if (isdigit (c2)) {
            tok->tok_class = lexer_read_number (lex, &buf, &buf_size, c);
            goto return_token;
        }
    }

    if (isdigit (c)) {
        tok->tok_class = lexer_read_number (lex, &buf, &buf_size, c);
        goto return_token;
    }

    /* From now on the token could be either an
       operator or user-op or unknown.  */
    lexer_ungetch (lex, c);
    search = lexer_trie_read (lex, lex->trie, &buf, 0, &buf_size);
    if (search != TRIE_NOT_LAST) {
        if (search == TRIE_USEROP)
            tok->tok_class = tok_user_op;
        else
            tval_tok_init (tok, tok_operator, (enum token_kind)search);
        goto return_token;
    } else
        c = lexer_getch (lex);

    /* if nothing was found, we construct an unknown token  */
    // assert (buf == NULL, "buf was used, but token_class is missing");
    if (!buf)
        buf = (char *)malloc (2 * sizeof (char));
    buf[0] = (char)c;
    buf[1] = 0;
    tok->tok_class = tok_unknown;

return_token:
    assert (tok->tok_class <= tok_unknown, "token type was not provided");

    /* Check if the standard token definition should be overrtaken
       by the user-operation definition which is stronger.  */
    if (lex->trie_user && lex->trie_user->children_count > 0
        && token_class (tok) != tok_char && token_class (tok) != tok_string
        && token_class (tok) != tok_comments && token_class (tok) != tok_whitespace
        && token_class (tok) != tok_eof) {
        bool buf_empty = buf == NULL;
        ssize_t res;

        assert (!buf_empty, "buffer should never be empty at this point");
        res = lexer_is_user_op (lex, tok, &buf, &buf_size);

        /* If the lexer_is_user_op returns true, but the symbol
           is the same as a keyword or an operator, we should return
           an opertor instead.  That should not happen, but who knows...  */
        if (res != TRIE_NOT_LAST) {
            ssize_t r = trie_search (lex->trie, buf, strlen (buf));
            if (r != TRIE_NOT_LAST && r != TRIE_USEROP)
                res = r;
        }

        if (res != TRIE_NOT_LAST && res == TRIE_USEROP) {
            tok->tok_class = tok_user_op;
            tok->value.cval = buf;
        }
    }

    if (!token_uses_buf (token_class (tok))) {
        if (buf)
            free (buf);
    } else
        tok->value.cval = buf;

    tok->loc = loc;
    return tok;
}

/* Unget the token in lexer, which pushes back the characters of the
   token into the lexer stack and make lexer start from the first
   character of token.  */
void
lexer_unget_token (struct lexer *lex, struct token *tok)
{
    size_t e = lex->buf_end;
    struct location loc = token_location (tok);
    bool found = false;
    size_t i;

    for (i = 0; i < LEXER_BUFFER; i++) {
        size_t idx = circbuf_idx_incdec (e, -(ssize_t)(i + 1), LEXER_BUFFER);

        if (lex->location_buffer[idx].fname == loc.fname
            && lex->location_buffer[idx].line == loc.line
            && lex->location_buffer[idx].col == loc.col) {
            found = true;
            break;
        }
    }

    assert (found,
            "ungetting token %s failed, "
            "location not foud",
            token_as_string (tok));

    lex->unget_idx = i + 1;
}

/* String representation of the token TOK.  */
const char *
token_as_string (struct token *tok)
{

    if (token_uses_buf (token_class (tok)))
        return tok->value.cval;
    else
        return token_kind_name[(int)tok->value.tval];
}

/* Prints the token.  */
void
token_print (struct token *tok)
{
    const char *tokval = token_as_string (tok);

    (void)fprintf (stdout, "%s %d:%d %s ", tok->loc.fname, (int)tok->loc.line,
                   (int)tok->loc.col, token_class_name[(int)tok->tok_class]);

    if (tok->tok_class != tok_unknown)
        (void)fprintf (stdout, "['%s']\n", tokval);
    else
        (void)fprintf (stdout, "['%s'] !unknown\n", tokval);

    fflush (stdout);
}

/* Deallocates the memory that token occupies.  */
void
token_free (struct token *tok)
{
    assert (tok, "attempt to free NULL token");

    if (token_uses_buf (token_class (tok)) && tok->value.cval)
        free (tok->value.cval);
    free (tok);
}

bool
token_uses_buf (enum token_class tclass)
{
    switch (tclass) {
    case tok_id:
    CASE_TOK_NUMBER:
    case tok_comments:
    case tok_whitespace:
    case tok_string:
    case tok_char:
    case tok_user_op:
    case tok_unknown:
        return true;
    default:
        return false;
    }
}

/* Main function if you want to test lexer part only.  */
#ifdef LEXER_BINARY

int
main (int argc, char *argv[])
{
    struct lexer *lex = (struct lexer *)malloc (sizeof (struct lexer));
    struct token *tok;
    bool user_defined_operations_p = true;

    if (argc <= 1) {
        fprintf (stderr, "No input file\n");
        goto cleanup;
    }

    if (!lexer_init (lex, argv[1]))
        goto cleanup;

    if (user_defined_operations_p) {
#define add_word(t, word) trie_add_word (t, word, strlen (word), TRIE_USEROP)
        // add_word (lex->trie, "+");
        // add_word (lex->trie, "++");
        // add_woRd (lex->trie, "+=");
        add_word (lex->trie, "+++");
        add_word (lex->trie, "-+-");
        add_word (lex->trie, "+=+");
        add_word (lex->trie, "=+=");
        add_word (lex->trie, "===");
        add_word (lex->trie, "---");
        add_word (lex->trie, "+-+");
    }

    while ((tok = lexer_get_token (lex))->tok_class != tok_eof) {
        token_print (tok);
        token_free (tok);
    }

    token_free (tok);
    lexer_finalize (lex, true);

cleanup:
    if (lex)
        free (lex);

    return 0;
}
#endif
