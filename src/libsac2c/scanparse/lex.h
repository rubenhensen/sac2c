/* Copyright (c) 2011, 2012 Artem Shinkarov <artyom.shinkaroff@gmail.com>

   Permission to use, copy, modify, and distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.  */

#ifndef __LEX_H__
#define __LEX_H__

//#define __USE_BSD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#ifndef LEXER_BINARY
#include "types.h"
#else
typedef int bool;
#endif

#define true 1
#define false 0

#include "trie.h"
#include "uthash.h"

#define LEXER_BUFFER 8192

static inline int
xfprintf (FILE *f, const char *fmt, ...)
{
    va_list args;

    if (fmt == 0 || strlen (fmt) == 0)
        return 0;
    else {
        va_start (args, fmt);
        (void)vfprintf (f, fmt, args);
        va_end (args);
        return fprintf (f, "\n");
    }
}

extern int error_count;
extern int warning_count;

#undef assert
#define assert(expr, ...)                                                                \
    ((expr) ? (void)0                                                                    \
            : (void)(fprintf (stderr, "%s:%i %s: Assertion (" #expr ") failed.\n",       \
                              __FILE__, __LINE__, __func__),                             \
                     xfprintf (stderr, __VA_ARGS__), abort ()))

#define unreachable(...)                                                                 \
    ((void)(fprintf (stderr, "Code in %s:%d reached impossible state.\n", __FILE__,      \
                     __LINE__),                                                          \
            xfprintf (stderr, __VA_ARGS__), abort ()))

#define error_loc(loc, ...)                                                              \
    do {                                                                                 \
        (void)fprintf (stderr, "%s error:", loc.fname);                                  \
        if (loc.line == 0)                                                               \
            (void)fprintf (stderr, "??");                                                \
        else                                                                             \
            (void)fprintf (stderr, "%d", (int)loc.line);                                 \
                                                                                         \
        if (loc.col > 0)                                                                 \
            (void)fprintf (stderr, ":%d ", (int)loc.col);                                \
        else                                                                             \
            (void)fprintf (stderr, " ");                                                 \
                                                                                         \
        (void)xfprintf (stderr, __VA_ARGS__);                                            \
        /*(void) xfprintf (stderr, " %i", __LINE__);*/                                   \
        ++error_count;                                                                   \
    } while (0)

#define error(...)                                                                       \
    do {                                                                                 \
        (void)fprintf (stderr, "error: ");                                               \
        (void)xfprintf (stderr, __VA_ARGS__);                                            \
        ++error_count;                                                                   \
    } while (0)

#define warning_loc(loc, ...)                                                            \
    do {                                                                                 \
        (void)fprintf (stderr, "warning:%d:%d: ", (int)loc.line, (int)loc.col);          \
        (void)xfprintf (stderr, __VA_ARGS__);                                            \
        ++warning_count;                                                                 \
    } while (0)

#define warning(...)                                                                     \
    do {                                                                                 \
        (void)fprintf (stderr, "warning: ");                                             \
        (void)xfprintf (stderr, __VA_ARGS__);                                            \
        ++warning_count;                                                                 \
    } while (0)

#define TOKEN_KIND(a, b) a,
#define KEYWORD_PRF(a, b, c) PRF_##a,
#define KEYWORD(a, b) a,
enum token_kind {
#include "token_kind.def"
#include "prf.def"
#include "keywords.def"
    tok_kind_length
};
#undef TOKEN_KIND
#undef KEYWORD
#undef KEYWORD_PRF

#define TOKEN_CLASS(a, b) tok_##a,
enum token_class {
#include "token_class.def"
    tok_class_length
};
#undef TOKEN_CLASS

/* As we have a lot of number_types, we do not introduce any subclasses
   of a token-class but keep each number type in a separate token_class.
   In order to check whether a certain class is actually a number we
   define a macro whic one can use inside a swithc over token_class.  */
#define CASE_TOK_NUMBER_INT                                                              \
    case tok_number:                                                                     \
    case tok_number_byte:                                                                \
    case tok_number_ubyte:                                                               \
    case tok_number_short:                                                               \
    case tok_number_ushort:                                                              \
    case tok_number_int:                                                                 \
    case tok_number_uint:                                                                \
    case tok_number_long:                                                                \
    case tok_number_ulong:                                                               \
    case tok_number_longlong:                                                            \
    case tok_number_ulonglong

#define CASE_TOK_NUMBER_REAL                                                             \
    case tok_number_float:                                                               \
    case tok_number_double

#define CASE_TOK_NUMBER                                                                  \
    CASE_TOK_NUMBER_INT:                                                                 \
    CASE_TOK_NUMBER_REAL

/* We use a trick to store both token_kinds and user-operations in the
   same trie.  It means that we need to hvae a value for each token
   kind (which we have) and another value to mark a token being user-op,
   the latter is defined further down.  */
#define TRIE_USEROP (tok_kind_length + 1)

struct location {
    const char *fname;
    size_t line, col;
};

struct token {
    struct location loc;
    enum token_class tok_class;
    union {
        char *cval;
        enum token_kind tval;
    } value;
};

struct file_name {
    char *name;
    UT_hash_handle hh;
};

struct lexer {
    const char *fname;
    FILE *file;
    struct location loc;
    struct token curtoken;
    struct trie *trie;
    struct trie *trie_user;

    /* This is a conceptualy circular bufer to unget the text.  */
    char buffer[LEXER_BUFFER];

    /* This are locations which are synchronized with
       the buffer and store a location of each symbol.  */
    struct location location_buffer[LEXER_BUFFER];

    /* Index in the buffer array which shows the beginning of the
       buffer.  It is not always 0, as the buffer is conceptually
       circular.  */
    size_t buf_start;

    /* Index in the buffer array which shows the last element
       read from the file.  */
    size_t buf_end;

    /* The index shows how many tokens from the buf_end we are
       currently at.  If set to non zero, it means that subsequent
       lexer_get_token calls would return token from buffer, not
       by lexing a file.  */
    size_t unget_idx;

    /* Hash files that can be found in line-directives.  */
    struct file_name *file_names;

    /* Set this on if we need to apply user-operator
       reading in case we are lexing the name of user-defined
       operator.  */
    bool is_read_user_op;
    bool is_eof;
};

#define tval_tok_init(_tok, _cls, _val)                                                  \
    do {                                                                                 \
        (_tok)->tok_class = _cls;                                                        \
        (_tok)->value.tval = _val;                                                       \
    } while (0)

#define cval_tok_init(_tok, _cls, _val)                                                  \
    do {                                                                                 \
        (_tok)->tok_class = _cls;                                                        \
        (_tok)->value.cval = _val;                                                       \
    } while (0)

extern const char *token_class_name[];
extern const char *token_kind_name[];

#define token_kind_as_string(tkind) token_kind_name[(int)tkind]
#define token_value(tok) (tok)->value.tval
#define token_class(tok) (tok)->tok_class
#define token_class_as_string(tcls) token_class_name[(int)tcls]
#define token_location(tok) (tok)->loc

/* Safely increment or decrement index of character buffer. Make
sure that negative index equals to size - idx. */
static inline size_t
buf_idx_inc (const size_t idx, const ssize_t inc, const size_t size)
{
    ssize_t diff = ((ssize_t)idx + inc) % size;
    return diff < 0 ? size - diff : diff;
}

static inline const char *
lexer_change_file_name (struct lexer *lex, const char *fname)
{
    struct file_name *f;

    HASH_FIND_STR (lex->file_names, fname, f);
    if (!f) {
        f = (struct file_name *)malloc (sizeof (struct file_name));
        f->name = strdup (fname);
        HASH_ADD_KEYPTR (hh, lex->file_names, f->name, strlen (f->name), f);
    }

    return f->name;
}

__BEGIN_DECLS
bool lexer_init_file (struct lexer *, FILE *, const char *);
bool lexer_init (struct lexer *, const char *);
bool lexer_finalize (struct lexer *, bool);
struct token *lexer_get_token (struct lexer *);
void lexer_unget_token (struct lexer *, struct token *);
void token_free (struct token *);
void token_print (struct token *);
const char *token_as_string (struct token *);
bool token_uses_buf (enum token_class);
char *quote_string (const char *, char *, int);
bool is_normal_id (const char *);
bool is_operator (const char *);
__END_DECLS

#endif /* __LEX_H__  */
