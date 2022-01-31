#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "config.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#define DBUG_PREFIX "PARSE"
#include "debug.h"
#include "DupTree.h"
#include "ctinfo.h"
#include "free.h"
#include "globals.h"
#include "options.h"
#include "handle_mops.h"
#include "new_types.h"
#include "user_types.h"
#include "type_utils.h"
#include "shape.h"
#include "stringset.h"
#include "namespaces.h"
#include "check_mem.h"
#include "hidestructs.h"
#include "resource.h"
#include "print.h"
#include "modulemanager.h"
#include "symboltable.h"

#include "uthash.h"
#include "lex.h"
#include "parser.h"
#include "compat.h"


static node *handle_generator_body (struct parser *parser, bool array_comprehension_p,
                                    struct array_comp_ctxt *ctxt);

static ntype *Exprs2NType (ntype *, node *);
static size_t CountDotsInExprs (node *);
static shape *Exprs2Shape (node *);

/* Helper function to annotate a node with location and return it.  */
static inline node *
loc_annotated (struct location loc, node *n)
{
    NODE_LOCATION (n) = loc;
    return n;
}

static ntype *
Exprs2NType (ntype *basetype, node *exprs)
{
    size_t n;
    size_t dots = 0;
    shape *shp;
    ntype *result = NULL;
    struct location loc;

    DBUG_ENTER ();

    n = TCcountExprs (exprs);
    loc = NODE_LOCATION (EXPRS_EXPR1 (exprs));

    switch (NODE_TYPE (EXPRS_EXPR1 (exprs))) {
    case N_spid:
        if (SPID_NS (EXPRS_EXPR1 (exprs)) != NULL)
            error_loc (loc, "illegal shape specification");
        else if (SPID_NAME (EXPRS_EXPR1 (exprs))[1] != '\0')
            error_loc (loc, "illegal shape specification");
        else {
            switch (SPID_NAME (EXPRS_EXPR1 (exprs))[0]) {
            case '*':
                result = TYmakeAUD (basetype);
                break;
            case '+':
                result = TYmakeAUDGZ (basetype);
                break;
            default:
                error_loc (loc, "illegal shape specification");
                break;
            }
        }
        break;
    case N_dot:
        dots = CountDotsInExprs (exprs);
        if (dots != n)
            error_loc (loc, "illegal shape specification");
        else
            result = TYmakeAKD (basetype, dots, SHmakeShape (0));
        break;
    case N_num:
        shp = Exprs2Shape (exprs);
        if (shp != NULL)
            result = TYmakeAKS (basetype, shp);
        else
            error_loc (loc, "illegal shape specification");
        break;
    default:
        error_loc (loc, "illegal shape specification");
        break;
    }

    exprs = FREEdoFreeTree (exprs);

    DBUG_RETURN (result);
}

static size_t
CountDotsInExprs (node *exprs)
{
    size_t result = 0;

    DBUG_ENTER ();

    while (exprs != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (exprs)) == N_dot)
            result++;

        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (result);
}

static shape *
Exprs2Shape (node *exprs)
{
    shape *result;
    size_t n;
    int cnt = 0;

    DBUG_ENTER ();

    n = TCcountExprs (exprs);

    result = SHmakeShape (n);

    while (exprs != NULL && result != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (exprs)) == N_num)
            result = SHsetExtent (result, cnt, NUM_VAL (EXPRS_EXPR (exprs)));
        else
            result = SHfreeShape (result);
        exprs = EXPRS_NEXT (exprs);
        cnt++;
    }

    DBUG_RETURN (result);
}

static node *
SetClassType (node *module, ntype *type, node *pragmas)
{
    node *tdef;

    DBUG_ENTER ();

    tdef = TBmakeTypedef (strdup (NSgetModule (MODULE_NAMESPACE (module))),
                          NSdupNamespace (MODULE_NAMESPACE (module)),
                          strdup (global.default_component_name), type, NULL,
                          MODULE_TYPES (module));

    NODE_LOCATION (tdef) = NODE_LOCATION (module);
    TYPEDEF_ISUNIQUE (tdef) = TRUE;
    TYPEDEF_PRAGMA (tdef) = pragmas;
    MODULE_TYPES (module) = tdef;

    DBUG_RETURN (module);
}

/* Here we define a set of wrappers for handle_generic_list, to give
   a little bit more meaning to the function names.  When function has
   extern declaration like TBmake..., by some reason it is impossible
   to use such a function as a parameter inside the function-call.  */

/* Handle list of expressions.  */
static node *
expr_constructor (node *a, node *b)
{
    return loc_annotated (NODE_LOCATION (a), TBmakeExprs (a, b));
}
#define handle_expr_list(parser)                                                         \
    handle_generic_list (parser, handle_expr, expr_constructor)

/* Append id to the ids, propogate the location and return the node
   of type N_spids.  */
static node *
id_constructor (node *id, node *next)
{
    char *name;
    struct location loc;

    assert (id && NODE_TYPE (id) == N_spid, "invalid identifier node");

    loc = NODE_LOCATION (id);
    name = strdup (SPID_NAME (id));
    id = free_tree (id);

    return loc_annotated (loc, TBmakeSpids (name, next));
}
#define handle_id_list(parser) handle_generic_list (parser, handle_id, id_constructor)

/* Handle list of type components.  This is currently unused but it is
   needed to support proper subtyping with idags from Santanu.  */
static UNUSED node *
type_component_constructor (node *id, node *next)
{
    char *name;
    struct location loc;

    assert (id && NODE_TYPE (id) == N_spid, "invalid identifier node");
    loc = NODE_LOCATION (id);
    name = strdup (SPID_NAME (id));
    id = free_tree (id);

    return loc_annotated (loc, TBmakeTypecomponentarg (name, NULL, next));
}
#define handle_typecomponent_list(parser)                                                \
    handle_generic_list (parser, handle_id, id_constructor)

/* Handle list of assigns.  */
static node *
assign_constructor (node *a, node *b)
{
    return loc_annotated (NODE_LOCATION (a), TBmakeAssign (a, b));
}
#define handle_assign_list(parser)                                                       \
    handle_generic_list (parser, handle_assign, assign_constructor)

/* This function pushes back all the tokens that are in the token_buffer
   of the parser back into the lexer.  */
static void
parser_unlex_token_buffer (struct parser *parser)
{
    size_t l = parser->unget_idx, i;
    size_t e = parser->buf_end, s = parser->buf_size;

    for (i = 0; i < l; i++) {
        size_t idx = circbuf_idx_incdec (e, -(ssize_t)(i + 1), s);
        lexer_unget_token (parser->lex, parser->token_buffer[idx]);
        token_free (parser->token_buffer[idx]);
        parser->token_buffer[idx] = NULL;
    }

    parser->buf_end = circbuf_idx_incdec (e, -(ssize_t)l, s);
    parser->unget_idx = 0;
}

static node *
handle_symbol_list (struct parser *parser, const char *modname, bool except)
{
    struct used_module *mod = NULL;
    struct namehash {
        char *name;
        UT_hash_handle hh;
    } *namehash = NULL, *tmp;

    struct token *tok;
    node *ret = NULL;


    if (modname) {
        HASH_FIND_STR (parser->used_modules, modname, mod);
        if (!modname)
            return error_mark_node;
    } else
        parser->lex->is_read_user_op = true;

    if (parser->unget_idx != 0)
        parser_unlex_token_buffer (parser);

    while (true) {
        if (modname) {
            parser->lex->trie_user = mod->user_ops;
            tok = parser_get_token (parser);
            parser->lex->trie_user = NULL;
        } else
            tok = parser_get_token (parser);

        if (token_is_operator (tok, tv_rbrace)) {
            parser_unget (parser);
            break;
        } else if (token_is_operator (tok, tv_comma))
            continue;
        else if (!token_is_reserved (tok) && token_class (tok) != tok_user_op
                 && token_class (tok) != tok_operator) {
            error_loc (token_location (tok),
                       "unexpected token `%s' in "
                       "symbol list",
                       token_as_string (tok));
            free_tree (ret);
            ret = error_mark_node;
            goto cleanup;
        }

        ret = loc_annotated (token_location (tok),
                             TBmakeSymbol (strdup (token_as_string (tok)), ret));

        if (modname && except) {
            /* If we use/import all except this set, we accumulate a set in
               a NAMEHASH and them we are going to import everything
               except those symbols.  */
            tmp = (struct namehash *)malloc (sizeof (struct namehash));
            tmp->name = strdup (token_as_string (tok));
            HASH_ADD_KEYPTR (hh, namehash, tmp->name, strlen (tmp->name), tmp);
        } else if (modname) {
            struct known_symbol *symb;
            const char *tval = token_as_string (tok);

            /* Find a symbol in the symbols of cached module.  */
            HASH_FIND_STR (mod->symbols, tval, symb);
            if (!symb) {
                error_loc (token_location (tok),
                           "symbol `%s' cannot be "
                           "found in `%s'",
                           tval, modname);
                if (!is_normal_id (tval)
                    && trie_search (parser->lex->trie, tval, strlen (tval))
                         == TRIE_NOT_LAST)
                    trie_add_word (parser->lex->trie, tval, strlen (tval), TRIE_USEROP);
                continue;
            }

            /* If the symbol is type/binary/unary, add it into the known_symbols
               of the parser.  */
            if (symbol_is_binary (symb) || symbol_is_unary (symb)
                || symbol_is_type (symb)) {
                struct known_symbol *nsymb;
                HASH_FIND_STR (parser->known_symbols, symb->name, nsymb);
                if (!nsymb) {
                    nsymb = (struct known_symbol *)malloc (sizeof (struct known_symbol));
                    nsymb->name = strdup (symb->name);
                    nsymb->flags = symb->flags;
                    HASH_ADD_KEYPTR (hh, parser->known_symbols, nsymb->name,
                                     strlen (nsymb->name), nsymb);
                } else
                    nsymb->flags |= symb->flags;
            }

            /* Add the symbol to the recognized symbols of the lexer, in case
               the symbol is not a simple id.  */
            if (!is_normal_id (tval)
                && trie_search (parser->lex->trie, tval, strlen (tval)) == TRIE_NOT_LAST)
                trie_add_word (parser->lex->trie, tval, strlen (tval), TRIE_USEROP);
        }
    }

    if (modname && except) {
        struct known_symbol *symb;
        struct known_symbol *t;

        /* Add all symbols from module to lexer recognized symbols and to
           the parser known_types, unless the symbol is in the NAMEHASH.  */
        HASH_ITER (hh, mod->symbols, symb, t) {
            struct namehash *nh;

            HASH_FIND_STR (namehash, symb->name, nh);
            if (!nh) {
                if (symbol_is_binary (symb) || symbol_is_unary (symb)
                    || symbol_is_type (symb)) {
                    struct known_symbol *nsymb;
                    HASH_FIND_STR (parser->known_symbols, symb->name, nsymb);

                    if (!nsymb) {
                        nsymb
                          = (struct known_symbol *)malloc (sizeof (struct known_symbol));
                        nsymb->name = strdup (symb->name);
                        nsymb->flags = symb->flags;
                        HASH_ADD_KEYPTR (hh, parser->known_symbols, nsymb->name,
                                         strlen (nsymb->name), nsymb);
                    } else
                        nsymb->flags |= symb->flags;
                }

                if (!is_normal_id (symb->name)
                    && trie_search (parser->lex->trie, symb->name, strlen (symb->name))
                         == TRIE_NOT_LAST)
                    trie_add_word (parser->lex->trie, symb->name, strlen (symb->name),
                                   TRIE_USEROP);
            }
        }
    }

cleanup:

    parser->lex->is_read_user_op = false;
    if (namehash) {
        struct namehash *elem;
        struct namehash *t;
        HASH_ITER (hh, namehash, elem, t) {
            HASH_DEL (namehash, elem);
            free (elem);
        }
    }

    return ret;
}

/* Handle list of type subexpressions.
   It could be a standard expression and { '.', '+', '*' }.  */
static node *
handle_type_subscript_expr (struct parser *parser)
{
    struct token *tok;
    struct location loc;
    node *t;

    tok = parser_get_token (parser);
    loc = token_location (tok);

    if (token_is_operator (tok, tv_dot))
        t = TBmakeDot (1);
    else if (token_is_operator (tok, tv_plus))
        t = TBmakeSpid (NULL, strdup ("+"));
    else if (token_is_operator (tok, tv_mult))
        t = TBmakeSpid (NULL, strdup ("*"));
    else {
        parser_unget (parser);
        /* FIXME handle_expr might be an overkill here: as expr has
           conditionals, binary operations, etc.  */
        t = handle_expr (parser);
    }

    if (t == NULL || t == error_mark_node) {
        error_loc (token_location (tok), "invalid or missing type dimension expression");
        return error_mark_node;
    }

    return loc_annotated (loc, t);
}
#define handle_type_subscript_list(parser)                                               \
    handle_generic_list (parser, handle_type_subscript_expr, expr_constructor)

/* Handle list of numbers.  */
/* XXX Please NOTE that stupid SaC backend would not print
   a node of type N_nums, and would print an empty string
   instead.  It may save you couple of hours of debugging.  */
static node *
handle_num (struct parser *parser)
{
    struct token *tok = parser_get_token (parser);

    if (token_class (tok) != tok_number)
        return NULL;

    parser_unget (parser);
    return handle_primary_expr (parser);
}
static node *
num_constructor (node *a, node *b)
{
    int value;
    struct location loc;

    assert (NODE_TYPE (a) == N_num, "number expected");

    value = NUM_VAL (a);
    loc = NODE_LOCATION (a);
    free_node (a);

    return loc_annotated (loc, TBmakeNums (value, b));
}
#define handle_num_list(parser) handle_generic_list (parser, handle_num, num_constructor)

/* Get one token from the lexer or from the token buffer.
   Token is taken from the buffer if parser_unget was
   called earlier. */
struct token *
parser_get_token (struct parser *parser)
{
    struct token *tok;

    if (parser->unget_idx == 0) {
        do {
            tok = lexer_get_token (parser->lex);

            /* If TOKEN_BUFFER is full, we free the token pointed by BUF_START
               and put the new token on its place, changing BUF_START and
               BUF_END accordingly.  */
            if ((parser->buf_end + 1) % parser->buf_size == parser->buf_start) {
                token_free (parser->token_buffer[parser->buf_start]);
                parser->buf_start = (parser->buf_start + 1) % parser->buf_size;
                parser->token_buffer[parser->buf_end] = tok;
                parser->buf_end = (parser->buf_end + 1) % parser->buf_size;
            } else {
                parser->token_buffer[parser->buf_end] = tok;
                parser->buf_end = (parser->buf_end + 1) % parser->buf_size;
            }
        } while (token_class (tok) == tok_whitespace
                 || token_class (tok) == tok_comments);
    } else {
        size_t idx;

        do {
            /* Return a token from the buffer.  */
            assert (parser->unget_idx < parser->buf_size,
                    "parser buffer holds only up to %zu values.", parser->buf_size);

            idx = circbuf_idx_dec(parser->buf_end, parser->unget_idx, parser->buf_size);
            parser->unget_idx--;

            tok = parser->token_buffer[idx];
        } while (token_class (tok) == tok_whitespace
                 || token_class (tok) == tok_comments);
    }

    /* Keep track of brackets.  */
    if (token_class (tok) == tok_operator)
        switch (token_value (tok)) {
        case tv_lparen:
            parser->paren_count++;
            break;
        case tv_rparen:
            parser->paren_count--;
            break;
        case tv_lsquare:
            parser->square_count++;
            break;
        case tv_rsquare:
            parser->square_count--;
            break;
        case tv_lbrace:
            parser->brace_count++;
            break;
        case tv_rbrace:
            parser->brace_count--;
            break;
        default:;
        }

    return tok;
}

/* Move the parser one token back. It means that the consequent
   call of parser_get_token would return the token from buffer,
   not from lexer.  */
void
parser_unget (struct parser *parser)
{
    size_t idx;
    struct token *tok;

    tok = parser->token_buffer[circbuf_idx_incdec (parser->buf_end, -(ssize_t)1, parser->buf_size)];

    /* Keep track of brackets.  */
    if (token_class (tok) == tok_operator)
        switch (token_value (tok)) {
        case tv_lparen:
            parser->paren_count--;
            break;
        case tv_rparen:
            parser->paren_count++;
            break;
        case tv_lsquare:
            parser->square_count--;
            break;
        case tv_rsquare:
            parser->square_count++;
            break;
        case tv_lbrace:
            parser->brace_count--;
            break;
        case tv_rbrace:
            parser->brace_count++;
            break;
        default:;
        }

    do {
        parser->unget_idx++;
        assert (parser->unget_idx < parser->buf_size,
                "parser buffer holds only up to %zu values.", parser->buf_size);
        idx = circbuf_idx_dec(parser->buf_end, parser->unget_idx, parser->buf_size);
        tok = parser->token_buffer[idx];
    } while (token_class (tok) == tok_whitespace || token_class (tok) == tok_comments);
}


/* Skip tokens until token with value TKIND would be found.  */
struct token *
parser_get_until_tval (struct parser *parser, enum token_kind tkind)
{
    struct token *tok;

    /* We need to subtract one in case we are searching for closing
       paren/square/brace.  That is because:

                    bc = 3                bc = 4   bc = 3
       tok-stream:     x            .....    {  ....  }
                       ^
                       |____ start searching for '}'

       so in order to skip the group of braces we would have to stop
       when bc == 2, or in general case original bc - 1.  */
    int pc = parser->paren_count - (tkind == tv_rparen ? 1 : 0);
    int bc = parser->brace_count - (tkind == tv_rbrace ? 1 : 0);
    int sc = parser->square_count - (tkind == tv_rsquare ? 1 : 0);

    do {
        tok = parser_get_token (parser);

        if (!token_uses_buf (token_class (tok)) && token_value (tok) == tkind
            && parser->paren_count <= pc && parser->brace_count <= bc
            && parser->square_count <= sc) {
            return tok;
        }
    } while (token_class (tok) != tok_eof);

    return tok;
}

/* Skip tokens until token with value TKIND would be found.  */
struct token *
parser_get_until_oneof_tvals (struct parser *parser, const unsigned num, ...)
{
    struct token *tok;
    int pc = parser->paren_count;
    int bc = parser->brace_count;
    int sc = parser->square_count;

    do {
        tok = parser_get_token (parser);

        if (!token_uses_buf (token_class (tok)) && parser->paren_count <= pc
            && parser->brace_count <= bc && parser->square_count <= sc) {
            unsigned i;
            va_list lst;

            va_start (lst, num);
            for (i = 0; i < num; i++) {
                enum token_kind tk;

                assert (sizeof (enum token_kind) == sizeof (int),
                        "unsafe enum passing in variable arguments");
                tk = (enum token_kind)va_arg (lst, int);
                if (token_value (tok) == tk)
                    return tok;
            }
        }
    } while (token_class (tok) != tok_eof);

    return tok;
}

/* Skip tokens until token of class TCLASS would be found.  */
struct token *
parser_get_until_tclass (struct parser *parser, enum token_class tclass)
{
    struct token *tok;
    int pc = parser->paren_count;
    int bc = parser->brace_count;
    int sc = parser->square_count;

    do {
        tok = parser_get_token (parser);

        if (token_class (tok) == tclass && parser->paren_count <= pc
            && parser->brace_count <= bc && parser->square_count <= sc) {
            return tok;
        }
    } while (token_class (tok) != tok_eof);

    return tok;
}

/* Check if the next token returned by parser_get_token would be
   token with the value TKIND, in case the value is different,
   the error_loc would be called.
   NOTE: function ungets the token after checking it.  */
/*bool
parser_expect_tval (struct parser *parser, enum token_kind tkind)*/

#if 1
#define parser_expect_tval(parser, tkind)                                                \
    __extension__({                                                                      \
        struct token *tok = parser_get_token (parser);                                   \
        bool ret;                                                                        \
        if (!token_uses_buf (token_class (tok)) && token_value (tok) == tkind) {         \
            parser_unget (parser);                                                       \
            ret = true;                                                                  \
        } else {                                                                         \
            error_loc (token_location (tok), "token `%s' expected, `%s' token found",    \
                       token_kind_as_string (tkind), token_as_string (tok));             \
            parser_unget (parser);                                                       \
            ret = false;                                                                 \
        }                                                                                \
        ret;                                                                             \
    })
#else
bool
parser_expect_tval (struct parser *parser, enum token_kind tkind)
{
    struct token *tok = parser_get_token (parser);
    bool ret;
    if (!token_uses_buf (token_class (tok)) && token_value (tok) == tkind) {
        parser_unget (parser);
        ret = true;
    } else {
        error_loc (token_location (tok), "token `%s' expected, `%s' token found",
                   token_kind_as_string (tkind), token_as_string (tok));
        parser_unget (parser);
        ret = false;
    }
    return ret;
}
#endif

/* Check if the next token returned by parser_get_token would be
   token of class TCLASS, in case the class is different,
   the error_loc would be called.
   NOTE: function ungets the token after checking it.  */
bool
parser_expect_tclass (struct parser *parser, enum token_class tclass)
{
    struct token *tok = parser_get_token (parser);
    if (token_class (tok) == tclass) {
        parser_unget (parser);
        return true;
    } else {
        error_loc (token_location (tok), "token of class `%s' expected, `%s' token found",
                   token_class_as_string (tclass), token_as_string (tok));
        parser_unget (parser);
        return false;
    }
}

#define add_symbol(symb, s)                                                              \
    do {                                                                                 \
        symb = (struct known_symbol *)malloc (sizeof (struct known_symbol));             \
        symb->name = strdup (s);                                                         \
        symb->flags = 0;                                                                 \
        HASH_ADD_KEYPTR (hh, parser->known_symbols, symb->name, strlen (symb->name),     \
                         symb);                                                          \
    } while (0)

/* Initialize the parser, allocate memory for token_buffer.  */
bool
parser_init (struct parser *parser, struct lexer *lex)
{
    struct known_symbol *symb;
    parser->lex = lex;
    parser->buf_size = 32;
    parser->buf_start = 0;
    parser->buf_end = 0;
    parser->buf_empty = true;
    parser->token_buffer
      = (struct token **)malloc (parser->buf_size * sizeof (struct token *));
    parser->unget_idx = 0;
    parser->known_symbols = NULL;
    parser->used_modules = NULL;

    parser->in_return = false;
    parser->in_subscript = false;
    parser->in_arraycomp_expr = false;
    parser->in_module = false;
    parser->current_module = NULL;

    add_symbol (symb, "!");
    symbol_set_unary (symb);
    add_symbol (symb, "~");
    symbol_set_unary (symb);
    add_symbol (symb, "--");
    symbol_set_unary (symb);
    add_symbol (symb, "++");
    symbol_set_unary (symb);
    add_symbol (symb, "-");
    symbol_set_unary (symb);
    symbol_set_binary (symb);
    add_symbol (symb, "+");
    symbol_set_unary (symb);
    symbol_set_binary (symb);
    add_symbol (symb, "|");
    symbol_set_binary (symb);
    add_symbol (symb, "&");
    symbol_set_binary (symb);
    add_symbol (symb, "||");
    symbol_set_binary (symb);
    add_symbol (symb, "&&");
    symbol_set_binary (symb);
    add_symbol (symb, "*");
    symbol_set_binary (symb);
    add_symbol (symb, "/");
    symbol_set_binary (symb);
    add_symbol (symb, "%");
    symbol_set_binary (symb);
    add_symbol (symb, "^");
    symbol_set_binary (symb);
    add_symbol (symb, ">>");
    symbol_set_binary (symb);
    add_symbol (symb, "<<");
    symbol_set_binary (symb);
    add_symbol (symb, "==");
    symbol_set_binary (symb);
    add_symbol (symb, ">=");
    symbol_set_binary (symb);
    add_symbol (symb, "<=");
    symbol_set_binary (symb);
    add_symbol (symb, "!=");
    symbol_set_binary (symb);
    add_symbol (symb, ">");
    symbol_set_binary (symb);
    add_symbol (symb, "<");
    symbol_set_binary (symb);

    return true;
}

/* Clear the memory allocated for internal structure.
   NOTE: PARSER is not freed.  */
bool
parser_finalize (struct parser *parser)
{
    struct known_symbol *elem;
    struct known_symbol *tmp;

    struct used_module *mod;
    struct used_module *mtmp;

    assert (parser, "attempt to free empty parser");

    while (parser->buf_start % parser->buf_size != parser->buf_end % parser->buf_size) {
        token_free (parser->token_buffer[parser->buf_start]);
        parser->buf_start = (parser->buf_start + 1) % parser->buf_size;
    }

    if (parser->token_buffer)
        free (parser->token_buffer);

    /* FIXME for the time being only, because we need to read from
       yyin and not open and close the file.  */
    /* lexer_finalize (parser->lex); */

    /* Free the known symbols hash-table.  */
    HASH_ITER (hh, parser->known_symbols, elem, tmp) {
        HASH_DEL (parser->known_symbols, elem);
        free (elem->name);
        free (elem);
    }

    HASH_ITER (hh, parser->used_modules, mod, mtmp) {
        HASH_ITER (hh, mod->symbols, elem, tmp) {
            HASH_DEL (mod->symbols, elem);
            free (elem);
        }
        trie_free (mod->user_ops);
        MEMfree (mod->name);

        HASH_DEL (parser->used_modules, mod);
        free (mod);
    }

    return true;
}

bool
is_prf (enum token_kind tkind)
{
#define KEYWORD_PRF(a, b, c) || tkind == PRF_##a
    return false
#include "prf.def"
      ;
#undef KEYWORD_PRF
}

prf
to_prf (enum token_kind tkind)
{
    assert (is_prf (tkind), "attempt to convert non-prf token to prf");
    switch (tkind) {
#define KEYWORD_PRF(a, b, c)                                                             \
    case PRF_##a:                                                                        \
        return c;
#include "prf.def"
#undef KEYWORD_PRF
    default:
        unreachable ("invalid prf kind");
    }
}

/* FIXME Check if any token starting an expression is missing.
   Check if token TOK can start an expression.  */
bool
token_starts_expr (struct parser *parser, struct token *tok)
{
    switch (token_class (tok)) {
    case tok_id:
    case tok_string:
    case tok_char:
    CASE_TOK_NUMBER:
        return true;
    case tok_user_op: {
        struct known_symbol *ks;

        HASH_FIND_STR (parser->known_symbols, token_as_string (tok), ks);
        return ks && !!symbol_is_unary (ks);
    }
    case tok_operator:
        switch (token_value (tok)) {
        case tv_lparen:
        case tv_lsquare:
        case tv_minus:
        case tv_not:
            return true;
        default:
            return false;
        }
    case tok_keyword:
        return is_prf (token_value (tok)) || token_is_reserved (tok)
               || token_is_keyword (tok, NWITH);
    default:
        return token_is_reserved (tok);
    }
}

/* Checks if the sequence of starting from a prarsers token is a type.  */
bool
is_type (struct parser *parser)
{
    bool ret = false;
    struct token *tok = parser_get_token (parser);

    if (token_class (tok) == tok_keyword)
        switch (token_value (tok)) {
        case TYPE_BYTE:
        case TYPE_SHORT:
        case TYPE_INT:
        case TYPE_LONG:
        case TYPE_LONGLONG:
        case TYPE_UBYTE:
        case TYPE_USHORT:
        case TYPE_UINT:
        case TYPE_ULONG:
        case TYPE_ULONGLONG:
        case TYPE_FLOAT:
        case TYPE_FLOATVEC:
        case TYPE_BOOL:
        case TYPE_CHAR:
        case TYPE_DBL:
        /* FIXME: This might not be the best place for this hack */
        case NESTED:
        /* FIXME: We may want to check that the structure was
           defined before it can be used as a type.  */
        case STRUCT:
            ret = true;
            break;

        default:
            ret = false;
            break;
        }
    /* Check if we met this user-defined type earlier.  */
    else if (token_class (tok) == tok_id) {
        bool saw_dcolon = false;
        const char *tval = token_as_string (tok);

        if (token_is_operator (parser_get_token (parser), tv_dcolon)) {
            saw_dcolon = true;
            /* tval is the name of module, and we need to cache it
               in order to check that the type is there.  */
            cache_module (parser, tval);
            tok = parser_get_token (parser);
        } else
            parser_unget (parser);

        ret = false;
        if (saw_dcolon
            && !(parser->in_module && !strcmp (parser->current_module, tval))) {
            struct used_module *mod;
            struct known_symbol *symb;

            HASH_FIND_STR (parser->used_modules, tval, mod);
            assert (mod, "module `%s' must be cached first", tval);

            HASH_FIND_STR (mod->symbols, token_as_string (tok), symb);
            ret = symb && !!symbol_is_type (symb);
        } else {
            struct known_symbol *ks;
            HASH_FIND_STR (parser->known_symbols, token_as_string (tok), ks);
            ret = ks ? symbol_is_type (ks) : false;
        }

        if (saw_dcolon)
            parser_unget2 (parser);
    }

    parser_unget (parser);
    return ret;
}

/* Create a node for the base-type in the TOK.  */
ntype *
make_simple_type (enum token_kind tkind)
{
    switch (tkind) {
    case TYPE_BYTE:
        return TYmakeSimpleType (T_byte);
    case TYPE_SHORT:
        return TYmakeSimpleType (T_short);
    case TYPE_INT:
        return TYmakeSimpleType (T_int);
    case TYPE_LONG:
        return TYmakeSimpleType (T_long);
    case TYPE_LONGLONG:
        return TYmakeSimpleType (T_longlong);
    case TYPE_UBYTE:
        return TYmakeSimpleType (T_ubyte);
    case TYPE_USHORT:
        return TYmakeSimpleType (T_ushort);
    case TYPE_UINT:
        return TYmakeSimpleType (T_uint);
    case TYPE_ULONG:
        return TYmakeSimpleType (T_ulong);
    case TYPE_ULONGLONG:
        return TYmakeSimpleType (T_ulonglong);
    case TYPE_FLOAT:
        return TYmakeSimpleType (T_float);
    case TYPE_FLOATVEC:
        return TYmakeSimpleType (T_floatvec);
    case TYPE_BOOL:
        return TYmakeSimpleType (T_bool);
    case TYPE_CHAR:
        return TYmakeSimpleType (T_char);
    case TYPE_DBL:
        return TYmakeSimpleType (T_double);
    case NESTED:
        return TYmakeSimpleType (T_int);
    default:
        unreachable ("cannot build symple type from `%s'", token_kind_as_string (tkind));
    }

    return NULL;
}

/* Type definition.  */
ntype *
handle_type (struct parser *parser)
{
    struct token *tok;
    ntype *type = NULL;
    node *sub_type_exprs = NULL;

    if (!is_type (parser)) {
        tok = parser_get_token (parser);
        parser_unget (parser);
        error_loc (token_location (tok), "type expected, `%s' found",
                   token_as_string (tok));
        return NULL;
    }

    tok = parser_get_token (parser);

    if (token_class (tok) == tok_keyword) {
        if (token_is_keyword (tok, STRUCT)) {
            /* FIXME: we can have some more checking when we
               know how exactly do we want to handle structs.  */
            tok = parser_get_token (parser);
            type = TYmakeSymbType (STRcat (STRUCT_TYPE, token_as_string (tok)), NULL);
        } else
            type = make_simple_type (token_value (tok));
    } else {
        char *name = NULL;
        char *ns = NULL;

        if (token_is_operator (parser_get_token (parser), tv_dcolon)) {
            ns = strdup (token_as_string (tok));
            name = strdup (token_as_string (parser_get_token (parser)));
        } else {
            parser_unget (parser);
            name = strdup (token_as_string (tok));
        }

        type = TYmakeSymbType (name, ns ? NSgetNamespace (ns) : NULL);
    }

    /* [ exprs ], where expr could be also {., +, *}   */
    tok = parser_get_token (parser);
    if (token_is_operator (tok, tv_lsquare)) {
        tok = parser_get_token (parser);
        if (!token_is_operator (tok, tv_rsquare)) {
            parser_unget (parser);
            sub_type_exprs = handle_type_subscript_list (parser);
            if (sub_type_exprs == error_mark_node) {
                parser_get_until_tval (parser, tv_rsquare);
                goto out;
            }

            if (parser_expect_tval (parser, tv_rsquare))
                parser_get_token (parser);
            else
                goto out;
        }
    } else
        parser_unget (parser);

    if (sub_type_exprs == NULL)
        return TYmakeAKS (type, SHmakeShape (0));
    else {
        type = Exprs2NType (type, sub_type_exprs);
        return type == NULL ? error_type_node : type;
    }

out:
    free_type (type);
    free_node (sub_type_exprs);
    return error_type_node;
}

struct token *
parser_get_namespace_token (struct parser *parser, const char *modname)
{
    struct used_module *mod;
    struct token *tok;

    /* Do not cache the module in case it is the module
       we are currently parsing.  */
    if (parser->in_module && !strcmp (parser->current_module, modname))
        return parser_get_token (parser);

    cache_module (parser, modname);
    HASH_FIND_STR (parser->used_modules, modname, mod);

    parser->lex->trie_user = mod->user_ops;

    if (parser->unget_idx != 0)
        parser_unlex_token_buffer (parser);

    tok = parser_get_token (parser);
    parser->lex->trie_user = NULL;

    return tok;
}

/* Check if the following token is id.  */
struct identifier *
is_id (struct parser *parser)
{
    struct token *tok = parser_get_token (parser);
    struct identifier *res = NULL;

    if (token_class (tok) == tok_id)
        res = identifier_new (NULL, strdup (token_as_string (tok)), false);

    parser_unget (parser);
    return res;
}

/* Check if the following token is extended id.
   ext-id ::=
      id
      |
      reserved
      |
      id '::' ( reserved | id )

   reserved ::=
        'genarray' | 'modarray' | 'fold' | 'foldfix'
      | 'propagate'| 'all'      | except
      | user-defined-op
*/
bool
token_is_reserved (struct token *tok)
{
    return token_class (tok) == tok_id
           || (token_class (tok) == tok_keyword
               && (token_is_keyword (tok, GENARRAY) || token_is_keyword (tok, MODARRAY)
                   || token_is_keyword (tok, FOLD) || token_is_keyword (tok, FOLDFIX)
                   || token_is_keyword (tok, PROPAGATE) || token_is_keyword (tok, ALL)
                   || token_is_keyword (tok, EXCEPT)
                   || token_is_keyword (tok, CUDALINKNAME)
                   || token_is_keyword (tok, LINKWITH) || token_is_keyword (tok, LINKOBJ)
                   || token_is_keyword (tok, LINKSIGN)
                   || token_is_keyword (tok, REFCOUNTING)
                   || token_is_keyword (tok, REFCOUNTDOTS)
                   || token_is_keyword (tok, EFFECT)
                   || token_is_keyword (tok, MUTCTHREADFUN)
                   || token_is_keyword (tok, NOINLINE) || token_is_keyword (tok, COPYFUN)
                   || token_is_keyword (tok, FREEFUN) || token_is_keyword (tok, WLCOMP)
                   || token_is_keyword (tok, TARGET)))
           || (token_class (tok) == tok_operator
               && (token_is_operator (tok, tv_not) || token_is_operator (tok, tv_compl)
                   || token_is_operator (tok, tv_and) || token_is_operator (tok, tv_or)
                   || token_is_operator (tok, tv_xor) || token_is_operator (tok, tv_plus)
                   || token_is_operator (tok, tv_minus)
                   || token_is_operator (tok, tv_mult) || token_is_operator (tok, tv_div)
                   || token_is_operator (tok, tv_mod) || token_is_operator (tok, tv_lt_eq)
                   || token_is_operator (tok, tv_lt) || token_is_operator (tok, tv_gt)
                   || token_is_operator (tok, tv_gt_eq) || token_is_operator (tok, tv_eq)
                   || token_is_operator (tok, tv_not_eq)
                   || token_is_operator (tok, tv_shl) || token_is_operator (tok, tv_shr)
                   || token_is_operator (tok, tv_and_and)
                   || token_is_operator (tok, tv_or_or)
                   || token_is_operator (tok, tv_plus_plus)
                   || token_is_operator (tok, tv_minus_minus)));
}

struct identifier *
is_ext_id (struct parser *parser)
{
    struct token *tok;

    tok = parser_get_token (parser);
    if (token_class (tok) == tok_id) {
        if (token_is_operator (parser_get_token (parser), tv_dcolon)) {
            const char *modname = token_as_string (tok);

            tok = parser_get_namespace_token (parser, modname);
            parser_unget3 (parser);
            if (token_is_reserved (tok) || token_class (tok) == tok_user_op)
                return identifier_new (strdup (modname), strdup (token_as_string (tok)),
                                       /* if something is not a c-identifier,
                                          assume it is an operation.  */
                                       token_class (tok) == tok_operator
                                         || token_class (tok) == tok_user_op);
            else
                return NULL;
        } else {
            parser_unget2 (parser);
            return identifier_new (NULL, strdup (token_as_string (tok)), false);
        }
    } else if (token_class (tok) == tok_user_op || token_is_reserved (tok)) {
        parser_unget (parser);
        return identifier_new (NULL, strdup (token_as_string (tok)),
                               token_class (tok) == tok_operator
                                 || token_class (tok) == tok_user_op);
    } else {
        parser_unget (parser);
        return NULL;
    }
}

bool
is_function_call (struct parser *parser)
{
    struct identifier *id;
    struct token *tok;

    tok = parser_get_token (parser);
    if (token_class (tok) == tok_keyword && is_prf (token_value (tok))) {
        bool ret = false;
        ret = token_is_operator (parser_get_token (parser), tv_lparen);
        parser_unget2 (parser);
        return ret;
    } else
        parser_unget (parser);

    id = is_ext_id (parser);
    if (!id)
        return false;

    if (id->xnamespace) {
        bool ret = false;
        parser_get_token (parser); parser_get_token (parser); parser_get_token (parser);
        ret = token_is_operator (parser_get_token (parser), tv_lparen);
        parser_unget3 (parser);
        parser_unget (parser);
        identifier_free (id);

        return ret;
    }

    if (!id->xnamespace && id->id) {
        bool ret = false;
        parser_get_token (parser);
        ret = token_is_operator (parser_get_token (parser), tv_lparen);
        parser_unget2 (parser);
        identifier_free (id);

        return ret;
    }

    unreachable ("identifier field id must not be NULL");
}

static inline bool
is_unary (struct parser *parser, const char *xnamespace, const char *id)
{
    struct known_symbol *symb;

    if (xnamespace == NULL
        || (parser->in_module && !strcmp (xnamespace, parser->current_module))) {
        HASH_FIND_STR (parser->known_symbols, id, symb);
        if (!symb)
            return false;
    } else {
        struct used_module *mod;
        HASH_FIND_STR (parser->used_modules, xnamespace, mod);
        if (!mod)
            return false;

        HASH_FIND_STR (mod->symbols, id, symb);
        if (!symb)
            return false;
    }

    return !!symbol_is_unary (symb);
}

static inline bool
is_binary (struct parser *parser, const char *xnamespace, const char *id)
{
    struct known_symbol *symb;

    if (xnamespace == NULL
        || (parser->in_module && !strcmp (xnamespace, parser->current_module))) {
        HASH_FIND_STR (parser->known_symbols, id, symb);
        if (!symb)
            return false;
    } else {
        struct used_module *mod;
        HASH_FIND_STR (parser->used_modules, xnamespace, mod);
        if (!mod)
            return false;

        HASH_FIND_STR (mod->symbols, id, symb);
        if (!symb)
            return false;
    }

    return !!symbol_is_binary (symb);
}

static inline bool
is_known (struct parser *parser, const char *xnamespace, const char *id)
{
    struct known_symbol *symb;

    if (xnamespace == NULL
        || (parser->in_module && !strcmp (xnamespace, parser->current_module))) {
        HASH_FIND_STR (parser->known_symbols, id, symb);
        return !!symb;
    } else {
        struct used_module *mod;
        HASH_FIND_STR (parser->used_modules, xnamespace, mod);
        if (!mod)
            return false;

        HASH_FIND_STR (mod->symbols, id, symb);
        return !!symb;
    }
}

node *
handle_id (struct parser *parser)
{
    struct identifier *id = is_id (parser);
    struct location loc;
    node *ret = error_mark_node;

    if (!id) {
        struct token *tok = parser_get_token (parser);
        error_loc (token_location (tok),
                   "id expected `%s' "
                   "found instead",
                   token_as_string (tok));
        return error_mark_node;
    }

    loc = token_location (parser_get_token (parser));
    assert (id->id, "identifier field id must not be empty");

    ret = loc_annotated (loc, TBmakeSpid (NULL, id->id));
    free (id);

    return ret;
}

node *
handle_ext_id (struct parser *parser)
{
    struct identifier *id = is_ext_id (parser);
    struct location loc;

    if (!id) {
        struct token *tok = parser_get_token (parser);
        error_loc (token_location (tok),
                   "id or namespace "
                   "expected, `%s' found instead",
                   token_as_string (tok));
        return error_mark_node;
    }

    /* get the first token preserving its location.  */
    loc = token_location (parser_get_token (parser));
    if (id->xnamespace) {
        node *ret
          = loc_annotated (loc, TBmakeSpid (NSgetNamespace (id->xnamespace), id->id));
        /* eat two more tokens: '::' and ID  */
        parser_get_token (parser); parser_get_token (parser);
        free (id);

        return ret;
    }

    if (!id->xnamespace && id->id) {
        node *ret = loc_annotated (loc, TBmakeSpid (NULL, id->id));
        free (id);

        return ret;
    }

    free (id);
    unreachable ("identifier cannot have empty id");
}

/* Handle arguments and parens of function call:
      '(' arg ? (',' arg )* ')'
*/
node *
handle_funcall_args (struct parser *parser)
{
    if (parser_expect_tval (parser, tv_lparen)) {
        struct token *tok;
        node *args = NULL;

        parser_get_token (parser);
        tok = parser_get_token (parser);
        if (token_is_operator (tok, tv_rparen))
            return NULL;
        else
            parser_unget (parser);

        args = handle_expr_list (parser);
        if (args == error_mark_node)
            return error_mark_node;

        if (parser_expect_tval (parser, tv_rparen)) {
            parser_get_token (parser);
            return args;
        } else {
            parser_unget (parser);
            free_tree (args);
            return error_mark_node;
        }
    } else
        return error_mark_node;
}

node *
handle_function_call (struct parser *parser)
{
    struct token *tok;
    node *ret = error_mark_node;
    node *args = error_mark_node;
    struct location loc;

    tok = parser_get_token (parser);
    loc = token_location (tok);

    if (token_class (tok) == tok_keyword && is_prf (token_value (tok))) {
        enum token_kind tkind = token_value (tok);

        if (!parser_expect_tval (parser, tv_lparen))
            return error_mark_node;

        if (error_mark_node == (args = handle_funcall_args (parser)))
            return error_mark_node;

        /* FIXME: check the number of arguments.  */

        node *prf;
        if (to_prf (tkind) == F_sel_VxA_distmem_local) {
            /*
             * Special case: Until code generation, we treat the primitive
             * function F_sel_VxA_distmem_local just like F_sel_VxA.
             *
             * It is used to mark that a read is local for the distributed
             * memory backend.
             */
            prf = TBmakePrf (F_sel_VxA, args);
            PRF_DISTMEMISLOCALREAD (prf) = TRUE;
        } else {
            prf = TBmakePrf (to_prf (tkind), args);
        }

        return loc_annotated (loc, prf);
    } else
        parser_unget (parser);

    if (!is_function_call (parser)) {
        error_loc (loc, "function call expected");
        return error_mark_node;
    }

    ret = handle_ext_id (parser);
    if (error_mark_node == (args = handle_funcall_args (parser))) {
        free_tree (ret);
        return error_mark_node;
    }

    return loc_annotated (loc, TBmakeSpap (ret, args));
}

static inline node *
handle_id_or_function_call (struct parser *parser)
{
    struct identifier *id;
    struct location loc;
    node *res;

    loc = token_location (parser_get_token (parser));
    parser_unget (parser);

    if (is_function_call (parser))
        res = handle_function_call (parser);
    else if (NULL != (id = is_ext_id (parser)) && !id->is_operation) {
        if (id->xnamespace && !is_known (parser, id->xnamespace, id->id))
            error ("symbol `%s' cannot be found in module `%s'", id->id, id->xnamespace);

        res = handle_ext_id (parser);
        free (id);
    } else {
        error_loc (loc, "id or function call expected");
        res = error_mark_node;
    }

    return res;
}


/* ::= '{' id '->' expr '}'
       |
       '{' '[' (id|'.') (',' (id|'.'))* ']' '->' expr '}'  */
static node *
handle_array_comprehension (struct parser *parser)
{
    struct token *tok;
    struct location loc;

    node *lhs = NULL;
    node *expr = NULL;
    node *constr = NULL;
    node **bound = NULL;
    size_t bound_sz = 0;

    /* A flag that determines whether we have seen a single bound variable
       on the left hand side of `->' (true) or a list of these (false).  */
    bool single_bound_variable = false;

#define GOOUT_IF(__cond)                        \
    do {                                        \
        if (__cond)                             \
            goto out;                           \
    } while (0)

    tok = parser_peek_token (parser);
    loc = token_location (tok);

    if (token_is_operator (tok, tv_lsquare)) {
        parser_get_token (parser);
        tok = parser_peek_token (parser);

        if (!token_is_operator (tok, tv_rsquare)) {
            /* Allow dots and three dots being a valid expr.  */
            parser->in_subscript = true;
            lhs = handle_expr_list (parser);
            parser->in_subscript = false;
        }

        /* Verify that we have dots and ids in the lhs and count the
           number of identifiers.  */
        node *t = lhs;
        while (t) {
            node *x = EXPRS_EXPR (t);
            if (NODE_TYPE (x) != N_dot && NODE_TYPE (x) != N_spid)
                error_loc (NODE_LOCATION (x), "only `.' or identifiers are allowed "
                           "in the list on the left hand side of `->' in array "
                           "comprehensions");
            bound_sz += (NODE_TYPE (x) == N_spid);
            t = EXPRS_NEXT (t);
        }

        bound = (node **)malloc (sizeof (node *) * bound_sz);
        node **tt = bound;
        t = lhs;
        while (t) {
            node *x = EXPRS_EXPR (t);
            if (NODE_TYPE (x) == N_spid)
                *tt++ = x;
            t = EXPRS_NEXT (t);
        }

        GOOUT_IF (lhs == error_mark_node);
        /* FIXME skip till rbrace.  */

        GOOUT_IF (!parser_expect_tval (parser, tv_rsquare));
        parser_get_token (parser);

        GOOUT_IF (!parser_expect_tval (parser, tv_rightarrow));
        parser_get_token (parser);

        single_bound_variable = false;

    } else if (is_id (parser)) {
        GOOUT_IF (error_mark_node == (lhs = handle_id (parser)));
        /* FIXME skip till rbrace.  */

        bound_sz = 1;
        bound = (node **)malloc (sizeof (node *));
        bound[0] = lhs;

        GOOUT_IF (!parser_expect_tval (parser, tv_rightarrow));
        parser_get_token (parser);

        single_bound_variable = true;

    } else
        error_loc (token_location (tok), "invalid axis notation --- identifier "
                   "or list of dots/identifiers expected");

    parser->in_arraycomp_expr = true;
    expr = handle_expr (parser);
    parser->in_arraycomp_expr = false;

    GOOUT_IF (expr == error_mark_node);

    tok = parser_peek_token (parser);
    if (token_is_operator (tok, tv_or)) {
        struct array_comp_ctxt ctxt = {
            .single_bound_variable = single_bound_variable,
            .bound_sz = bound_sz,
            .bound = bound
        };

        parser_get_token (parser);
        GOOUT_IF (error_mark_node == (constr = handle_generator_body (parser, true, &ctxt)));
        /* We are getting N_part from the handle_generator, so we
           extract the generator and destroy the WithId.  */
        free_node (PART_WITHID (constr));
        PART_WITHID (constr) = NULL;
        node *t = PART_GENERATOR (constr);
        PART_GENERATOR (constr) = NULL;
        free_tree (constr);
        constr = t;
    }

    /* The next expression in the array comprehension.  */
    tok = parser_peek_token (parser);

    node *next = NULL;

    if (token_is_operator (tok, tv_semicolon)) {
        parser_get_token (parser);
        GOOUT_IF (error_mark_node == (next = handle_array_comprehension (parser)));
    }

    free (bound);
    node *r = TBmakeSetwl (lhs, expr);
    NODE_LOCATION (r) = loc;
    SETWL_GENERATOR (r) = constr;
    SETWL_NEXT (r) = next;
    return r;

out:
    free_tree (lhs);
    free_tree (expr);
    free_tree (constr);
    free (bound);
    return error_mark_node;
#undef GOTOOUT_IF
}



/* primary-expression:
     constant
     identifier
     function call
     constant-array
     '(' expr ')'

   constant-array:
     '[' expr-list? ']'
     '[' ':' ? type  ']'
     '<' expr-list  '>'
     '{' id '->' expr '}'
     '{' '[' expr-list ']' '->' expr '}'
*/
node *
handle_primary_expr (struct parser *parser)
{
    node *res = NULL;
    struct token *tok = parser_get_token (parser);
    enum token_class tclass = token_class (tok);
    struct location loc = token_location (tok);
    struct token *tok1;

#define ADD_TOK_NUMBER_INT(type, tok, ret, convert, make_node)                           \
    do {                                                                                 \
        const char *val = token_as_string (tok);                                         \
        type num;                                                                        \
                                                                                         \
        num = (type)convert (val, (char **)NULL, 0);                                     \
        if (errno == ERANGE) {                                                           \
            error_loc (token_location (tok),                                             \
                       "value `%s' produces "                                            \
                       "overflow when converting to `%s'",                               \
                       val, #type);                                                      \
            return error_mark_node;                                                      \
        }                                                                                \
        ret = make_node (num);                                                           \
    } while (0)

    /* ::= const-number
       All the possible numerical constants of different types.  */
    if (tclass == tok_number || tclass == tok_number_int)
        ADD_TOK_NUMBER_INT (signed int, tok, res, strtoll, TBmakeNum);

    else if (tclass == tok_number_byte)
        ADD_TOK_NUMBER_INT (signed char, tok, res, strtoll, TBmakeNumbyte);

    else if (tclass == tok_number_ubyte)
        ADD_TOK_NUMBER_INT (unsigned char, tok, res, strtoull, TBmakeNumubyte);

    else if (tclass == tok_number_short)
        ADD_TOK_NUMBER_INT (signed short, tok, res, strtoll, TBmakeNumshort);

    else if (tclass == tok_number_ushort)
        ADD_TOK_NUMBER_INT (unsigned short, tok, res, strtoull, TBmakeNumushort);

    else if (tclass == tok_number_uint)
        ADD_TOK_NUMBER_INT (unsigned int, tok, res, strtoull, TBmakeNumuint);

    else if (tclass == tok_number_long)
        ADD_TOK_NUMBER_INT (signed long, tok, res, strtoll, TBmakeNumlong);

    else if (tclass == tok_number_ulong)
        ADD_TOK_NUMBER_INT (unsigned long, tok, res, strtoull, TBmakeNumulong);

    else if (tclass == tok_number_longlong)
        ADD_TOK_NUMBER_INT (signed long long int, tok, res, strtoll, TBmakeNumlonglong);

    else if (tclass == tok_number_ulonglong)
        ADD_TOK_NUMBER_INT (unsigned long long int, tok, res, strtoull,
                            TBmakeNumulonglong);
    else if (tclass == tok_number_float) {
        float val = (float)atof (token_as_string (tok));
        res = TBmakeFloat (val);
    } else if (tclass == tok_number_double) {
        double val = atof (token_as_string (tok));
        res = TBmakeDouble (val);
    }
    /* ::= const-string
       Constant quoted string.  */
    else if (tclass == tok_string) {
        /* XXX This is a shame that the middle-end stores quoted string.
           For the time being we quote the string before creating a
           node.  Otherwise, symbols inside the sting could break the
           resulting C program.  */
        char *qs = quote_string (token_as_string (tok), NULL, 0);
        res = STRstring2Array (qs);
    }
    /*  ::= const-char
        Constant quoted character.  */
    else if (tclass == tok_char) {
        res = TBmakeChar (token_as_string (tok)[0]);
    }
    /* ::= id | function-call
       Identifier or a function call.  */
    else if (tclass == tok_id || tclass == tok_user_op) {
        parser_unget (parser);
        res = handle_id_or_function_call (parser);
    } else if (tclass == tok_keyword) {
        /* ::= ( 'spawn' | 'rspawn' ) string function-call
           A function-call prefexid with SPAWN and placement.  */
        if (token_value (tok) == SPAWN || token_value (tok) == RSPAWN) {
            enum token_kind tkind = token_value (tok);
            char *place = NULL;

            tok1 = parser_get_token (parser);

            if (token_class (tok) == tok_operator && token_value (tok) == tv_lsquare) {
                struct token *tok2 = parser_get_token (parser);
                if (token_class (tok2) != tok_string) {
                    error_loc (token_location (tok2), "placement string expected");
                    parser_get_until_tval (parser, tv_rsquare);
                    return NULL;
                }

                place = strdup (token_as_string (tok2));

                if (parser_expect_tval (parser, tv_rsquare))
                    parser_get_token (parser);
            } else
                parser_unget (parser);

            if (error_mark_node == (res = handle_function_call (parser)))
                return error_mark_node;

            if (global.fp || global.backend == BE_mutc) {
                SPAP_ISSPAWNED (res) = true;
                SPAP_ISREMOTE (res) = tkind == RSPAWN;
                if (place)
                    SPAP_SPAWNPLACE (res) = place;
            }
        }
        /* ::= with-loop  */
        else if (token_is_keyword (tok, NWITH)) {
            parser_unget (parser);
            res = handle_with (parser);
        }
        /* ::= local with-loop  */
        else if (token_is_keyword (tok, LOCAL)) {
            tok1 = parser_get_token (parser);
            if (token_is_keyword (tok1, NWITH)) {
                parser_unget (parser);
                res = handle_with (parser);
                if (res != NULL && res != error_mark_node)
                    WITH_DIST (res) = strdup ("PLACE_LOCAL");
            } else {
                error_loc (loc, "`local' not followed by `with' found");
                return error_mark_node;
            }
        }
        /* ::= prf-call
           Primitive function call.  */
        else if (is_prf (token_value (tok))) {
            parser_unget (parser);
            res = handle_function_call (parser);
        }
        /* ::= true
           Boolean TRUE constant.  */
        else if (token_is_keyword (tok, TRUETOKEN)) {
            res = TBmakeBool (1);
        }
        /* ::= false
           Boolean FALSE constant.  */
        else if (token_is_keyword (tok, FALSETOKEN)) {
            res = TBmakeBool (0);
        } else if (token_is_keyword (tok, ZERO_FLOATVEC)) {
            res = TBmakeFloatvec (((floatvec){0., 0., 0., 0.}));
        }
        /* ::= function-call | id
           Function call or identifier where the name of the function is a
           keyword; e.g. modarray, genarray, etc. pargmas */
        else if (token_is_reserved (tok)) {
            parser_unget (parser);
            res = handle_id_or_function_call (parser);
        } else
            parser_unget (parser);
    } else if (tclass == tok_operator) {
        /* ::= operator '(' expr-list ')'
           Allow here a special-case of operations used in a functional way
           like for example:  + (a, b)  */
        if (token_is_reserved (tok)) {
            if (token_is_operator (parser_get_token (parser), tv_lparen)) {
                parser_unget2 (parser);
                return handle_function_call (parser);
            } else
                parser_unget (parser);
        }

        switch (token_value (tok)) {
        /* ::= '.'
           Dot expressio in case we are in the array subscript.  */
        case tv_dot:
            /* '.'  as a meber of subscript expression only.  */
            if (parser->in_subscript)
                res = TBmakeDot (1);
            break;

        /* ::= '...'
           Dot expressio in case we are in the array subscript.  */
        case tv_threedots:
            /* '...'  as a meber of subscript expression only.  */
            if (parser->in_subscript)
                res = TBmakeDot (3);
            break;

        case tv_lparen:
            /* ::= '(' expr ')'
                   | '(' expr-list ')'  in case in return.  */
            if (parser->in_return)
                res = handle_expr_list (parser);
            else
                res = handle_expr (parser);

            /* Avoid Nexprs inside of Nexprs.  */
            if (parser->in_return && res && res != error_mark_node) {
                if (NODE_TYPE (res) == N_exprs && EXPRS_NEXT (res) == NULL) {
                    node *tmp = EXPRS_EXPR (res);
                    EXPRS_EXPR (res) = NULL;
                    free_node (res);
                    res = tmp;
                }
            }

            if (parser_expect_tval (parser, tv_rparen))
                parser_get_token (parser);

            if (res && res != error_mark_node && NODE_TYPE (res) == N_spmop)
                SPMOP_ISFIXED (res) = TRUE;

            break;

        case tv_lsquare: {
            /* Constant array.  */
            bool saw_colon = false;

            tok = parser_get_token (parser);
            if (token_is_operator (tok, tv_colon))
                saw_colon = true;
            else
                parser_unget (parser);

            /* ::= '[' ':'? type ']'  */
            if (is_type (parser)) {
                ntype *type;

                /* FIXME We haven't decieded if we want to support [:type]
                   or [type] syntax, so we do not produce the warning for
                   the time being.  */
                /*if (saw_colon)
                  warning_loc (token_location (tok), "using deprecated type-cast "
                               "syntax, please remove the `:' character");  */

                if (error_type_node == (type = handle_type (parser)))
                    return error_mark_node;

                if (!TYisAKS (type)) {
                    error_loc (loc, "Empty array with non-constant "
                                    "shape found.");
                    res = error_mark_node;
                } else
                    res = TCmakeVector (type, NULL);
            }
            /* ::= '[' expr-list ']'
                   | '[' ']'
               Constant array expression.  */
            else {
                node *els;
                struct token *tok;

                if (saw_colon)
                    parser_unget (parser);

                tok = parser_get_token (parser);
                parser_unget (parser);

                if (token_is_operator (tok, tv_rsquare)) {
                    ntype *type;
                    type = TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0));
                    res = TCmakeVector (type, NULL);
                } else {
                    els = handle_expr_list (parser);
                    if (els == error_mark_node)
                        res = error_mark_node;
                    else {
                        ntype *type;
                        type = TYmakeAKS (TYmakeSimpleType (T_unknown), SHmakeShape (0));

                        res = TCmakeVector (type, els);
                    }
                }
            }

            if (parser_expect_tval (parser, tv_rsquare))
                parser_get_token (parser);

            break;
        }

        /*  ::= '<' expr-list '>'  */
        case tv_lt: {
            node *els;

            els = handle_expr_list (parser);
            if (els == error_mark_node)
                res = error_mark_node;
            else {
                ntype *type;
                type = TYmakeAKS (TYmakeSimpleType (T_unknown), SHmakeShape (0));

                res = TCmakeVector (type, els);
            }

            if (parser_expect_tval (parser, tv_rsquare))
                parser_get_token (parser);

            break;
        }

        /* Axis notation.  */
        /* FIXME do a propper skipping to the right brace.  */
        case tv_lbrace:
            res = handle_array_comprehension (parser);
            if (!parser_expect_tval (parser, tv_rbrace)) {
                free_tree (res);
                return error_mark_node;
            }
            parser_get_token (parser);
            break;

        default:
            error_loc (token_location (tok), "token %s cannot start an expression.",
                       token_as_string (tok));
            parser_unget (parser);
            break;
        }
    } else
        error_loc (token_location (tok), "token %s cannot start an expression.",
                   token_as_string (tok));

    /* Initialize the location of the RES node.  */
    if (res != NULL && res != error_mark_node)
        NODE_LOCATION (res) = loc;

    return res;
}

/* postfix-expr:
     primary-expr
     postfix-expr [ expr ]
     postfix-expr  . id
     postfix-expr ++
     postfix-expr --
*/
node *
handle_postfix_expr (struct parser *parser)
{
    struct token *tok;
    struct location loc;
    node *res;

    tok = parser_get_token (parser);
    loc = token_location (tok);
    parser_unget (parser);

    res = handle_primary_expr (parser);

    if (res == error_mark_node || res == NULL)
        return res;

    tok = parser_get_token (parser);
    while (true) {
        node *args;

        /* ::= postfix-expr '.' id  */
        if (token_is_operator (tok, tv_dot)) {
            tok = parser_get_token (parser);

            if (token_class (tok) == tok_id) {
                char *struct_id = strdup (token_as_string (tok));
                node *id_node;

                struct_id = STRcat (STRUCT_GET, struct_id);
                id_node = TBmakeSpid (NULL, struct_id);
                res = TBmakeSpap (id_node, expr_constructor (res, NULL));

                /* Set location for every internal function-call.  */
                NODE_LOCATION (res) = loc;
            } else {
                error_loc (token_location (tok),
                           "identifier expected after "
                           "structure dot, `%s' found instead",
                           token_as_string (tok));
                parser_unget (parser);
                return error_mark_node;
            }
        }
        /* ::= postfix-expr '[' expr-list ']'  */
        else if (token_is_operator (tok, tv_lsquare)) {
            parser->in_subscript = true;
            args = handle_expr_list (parser);
            parser->in_subscript = false;

            /* expr can be NULL in selection  */
            if (args == error_mark_node)
                return error_mark_node;

            if (parser_expect_tval (parser, tv_rsquare))
                parser_get_token (parser);

            if (TCcountExprs (args) == 1) {
                res = TCmakeSpap2 (NULL, strdup ("sel"), EXPRS_EXPR (args), res);
            } else {
                node *vec;
                ntype *type;

                type = TYmakeAKS (TYmakeSimpleType (T_unknown), SHmakeShape (0));
                vec = TCmakeVector (type, args);
                res = TCmakeSpap2 (NULL, strdup ("sel"), vec, res);
            }

            /* Set location for every internal function-call.  */
            NODE_LOCATION (res) = loc;
        } else {
            parser_unget (parser);
            goto out;

#if 0
          /* NOTE this is a code to deal with user-defined postfix
             operators, so we will keep it commented in case we would
             want to reconsider the idea later.  */

          struct identifier *  id;

          parser_unget (parser);

          if (is_function_call (parser))
            goto out;

          id = is_ext_id (parser);
          if (!id)
            goto out;

          if (!is_unary (parser, id->xnamespace, id->id))
            goto out;

          parser_get_token (parser);
          if (id->xnamespace)
            parser_get_token (parser), parser_get_token (parser);

          res = TCmakeSpap1 (id->xnamespace
                             ? NSgetNamespace (id->xnamespace) : NULL,
                             id->id, res);
          free (id);
#endif
        }

        tok = parser_get_token (parser);
    }

out:
    NODE_LOCATION (res) = loc;
    return res;
}

/*   unary-expression:
       postfix-expression
       ++ unary-expression
       -- unary-expression
       unary-operator cast-expression

     unary-operator:
       + - ! ~
*/
struct pre_post_expr
handle_unary_expr (struct parser *parser)
{
    struct pre_post_expr res;
    struct identifier *id;
    struct location loc;

    loc = token_location (parser_get_token (parser));
    parser_unget (parser);

    /* It is not even an extended id, so it cannot be unary operation.  */
    id = is_ext_id (parser);
    if (!id) {
        identifier_free (id);
        goto out;
    }

    /* Any unary operation can be used as prefix operation.  */
    if (!is_unary (parser, id->xnamespace, id->id) || !id->is_operation)
        goto out;

    if (id->xnamespace) {
        parser_get_token (parser); parser_get_token (parser); parser_get_token (parser);
    } else if (!id->xnamespace && id->id)
        parser_get_token (parser);
    else
        unreachable ("identifier structure with empty id field");

    /* If we see '(', we assume it it is a function call, otherwise we
       have a problem with + (a, b).  BUT we need to make sure that
       there is not type-cast after the operation, as in that case
       we have a valid prefix.  For example - (int) 3.0.  */
    if (token_is_operator (parser_get_token (parser), tv_lparen)) {
        if (!is_type (parser)) {
            parser_unget2 (parser);
            if (id->xnamespace)
                parser_unget2 (parser);

            identifier_free (id);
            id = NULL;
            goto out;
        } else
            parser_unget (parser);
    } else
        parser_unget (parser);

    res = handle_cast_expr (parser);
    if (res.expr == error_mark_node || res.expr == NULL)
        goto error_return;

    res.expr
      = loc_annotated (loc, TCmakeSpap1 (id->xnamespace ? NSgetNamespace (id->xnamespace)
                                                        : NULL,
                                         strdup (id->id), res.expr));

    /* Set the argument once, and leave it untouched
       in the outer recursicve calls.  */
    if (res.parent_exprs == NULL)
        res.parent_exprs = SPAP_ARGS (res.expr);

    free (id);
    return res;

out:
    if (id)
        identifier_free (id);

    /* Here wrap a postfix expression into pre_post_expr structure
       to pass it recursively up.  */
    return (struct pre_post_expr){handle_postfix_expr (parser), NULL};

error_return:
    if (res.expr != NULL && res.expr != error_mark_node)
        free_tree (res.expr);
    if (id)
        identifier_free (id);

    return (struct pre_post_expr){error_mark_node, NULL};
}

/*  cast-expression:
      unary-expression
      ( '(' ':'? type ')' )* unary-expr
*/
struct pre_post_expr
handle_cast_expr (struct parser *parser)
{
    struct token *tok = parser_get_token (parser);
    struct location loc = token_location (tok);

    if (token_is_operator (tok, tv_lparen)) {
        bool saw_colon = false;

        tok = parser_get_token (parser);

        /* eat-up unnecessary colon in the type-conversion
           left there only for compatibility reasons.  */
        if (token_is_operator (tok, tv_colon)) {
            saw_colon = true;
        } else
            parser_unget (parser);

        if (is_type (parser)) {
            struct pre_post_expr ret;
            ntype *type = NULL;

            if (saw_colon)
                warning_loc (token_location (tok),
                             "using deprecated type-cast "
                             "syntax, please remove the `:' character");

            type = handle_type (parser);

            if (parser_expect_tval (parser, tv_rparen))
                parser_get_token (parser);

            /* FIXME here we can have a situation that there is no expression
               standing after the type because it is a local variable named
               the same as the type:
                  typedef char x;
                  ...
                  x = 1;
                  a = ((x))++;
                  ...
               in which case we would need to check that the type is actually
               allowed to be a variable, and if so, construct an expression
               from it.
            */
            ret = handle_cast_expr (parser);

            if (ret.expr == error_mark_node || type == error_type_node)
                return (struct pre_post_expr){error_mark_node, NULL};

            return (
              struct pre_post_expr){loc_annotated (loc, TBmakeCast (type, ret.expr)),
                                    ret.parent_exprs};
        } else {
            if (saw_colon)
                parser_unget (parser);
        }
    }

    parser_unget (parser);
    return handle_unary_expr (parser);
}

UNUSED static inline bool
ends_with_binary (struct parser *parser, struct pre_post_expr e)
{
    node *t = NULL;

    if (e.expr == error_mark_node || e.expr == NULL)
        return false;

    if (e.parent_exprs == NULL)
        return false;
    else
        t = EXPRS_EXPR (e.parent_exprs);

    if (NODE_TYPE (t) == N_spap && NODE_TYPE (SPAP_ID (t)) == N_spid
        && EXPRS_NEXT (SPAP_ARGS (t)) == NULL) {
        node *id = SPAP_ID (t);
        return is_binary (parser, SPID_NS (id) ? NSgetModule (SPID_NS (id)) : NULL,
                          SPID_NAME (id));
    }

    return false;
}

UNUSED static inline struct identifier *
get_binary_op (struct pre_post_expr *e)
{
    struct identifier *id;
    node *t = NULL;
    node *v = NULL;

    if (e->parent_exprs == NULL)
        unreachable ("expression does not end with binary operation");
    else
        t = EXPRS_EXPR (e->parent_exprs);

    v = SPAP_ID (t);
    id = identifier_new (SPID_NS (v) ? strdup (NSgetModule (SPID_NS (v))) : NULL,
                         strdup (SPID_NAME (v)),
                         /* it was considered as an operation
                            when parsing, so it should stay.  */
                         true);

    EXPRS_EXPR (e->parent_exprs) = DUPdoDupTree (EXPRS_EXPR (SPAP_ARGS (t)));

    free_tree (t);
    return id;
}

/* Binary expression or cast expressions:  */
node *
handle_binary_expr (struct parser *parser, bool no_relop)
{
    struct token *tok;
    enum prec {
        prec_none,
        prec_logor,
        prec_logand,
        prec_bitor,
        prec_bitxor,
        prec_bitand,
        prec_eq,
        prec_rel,
        prec_concat,
        prec_shift,
        prec_add,
        prec_mult,
        prec_userop,
        num_precs
    };

    struct {
        struct pre_post_expr expr;
        enum prec prec;
        char *op;
        namespace_t *xnamespace;
    } stack[num_precs];

    int sp = 0;

    /* If we are parsing an expression within array comprehensions, we
       need to avoid a '|', but not if it is inside the cast expression.  */
    bool old_parser_in_arraycomp_expr = parser->in_arraycomp_expr;
    parser->in_arraycomp_expr = false;
    struct pre_post_expr t = handle_cast_expr (parser);
    parser->in_arraycomp_expr = old_parser_in_arraycomp_expr;

    if (t.expr == error_mark_node || t.expr == NULL)
        return t.expr;

    stack[0].expr = t;
    stack[0].prec = prec_none;
    stack[0].xnamespace = NULL;
    stack[0].op = NULL;

    while (true) {
        enum prec oprec;
        struct location loc;
        namespace_t *xnamespace = NULL;
        bool need_parser_shift = false;
        struct identifier *id;

        tok = parser_get_token (parser);
        loc = token_location (tok);
        parser_unget (parser);

#if 0
      /* The following code adds a chance to support user-defined
         postfix operations.  Keep it here in case we may
         decide to support it.  */
      prev_expr_binary_op = token_starts_expr (parser, tok)
                            && ends_with_binary (parser, t);

      /* Always give a priority to the potential function-call.  */
      if (is_function_call (parser) && prev_expr_binary_op)
        {
          id = get_binary_op (&stack[sp].expr);
          xnamespace = id->xnamespace ? NSgetNamespace (id->xnamespace) : NULL;
        }
      else
        {
          id = is_ext_id (parser);

          /* Avoid the case when the binary operation is eaten-up
             by postfix handler, in case operation can be applied
             both ways.  */
          if (!id || !is_binary (parser, id->xnamespace, id->id))
            {
              if (prev_expr_binary_op)
                {
                  id = get_binary_op (&stack[sp].expr);
                  xnamespace = id->xnamespace ? NSgetNamespace (id->xnamespace) : NULL;
                }
              else
                goto out;
            }
          else
            {
              xnamespace = id->xnamespace ? NSgetNamespace (id->xnamespace) : NULL;
              need_parser_shift = true;
            }
        }
#endif

        if (NULL == (id = is_ext_id (parser))
            || !is_binary (parser, id->xnamespace, id->id))
            goto out;
        else if (!id->is_operation) {
            error_loc (loc, "`%s' cannot be used as a binary operation", id->id);
            identifier_free (id);
            while (sp >= 0)
                free_tree (stack[sp--].expr.expr);

            return error_mark_node;
        } else if (!strcmp (id->id, "++") || !strcmp (id->id, "--")) {
            parser_get_token (parser);
            if (id->xnamespace) {
                parser_get_token (parser); parser_get_token (parser);
            }
            /* Assume that this is a postfix ++ or -- used in the assignment.  */
            if (!token_starts_expr (parser, parser_get_token (parser))) {
                parser_unget2 (parser);
                if (id->xnamespace)
                    parser_unget2 (parser);

                identifier_free (id);
                goto out;
            } else
                parser_unget (parser);

            xnamespace = id->xnamespace ? NSgetNamespace (id->xnamespace) : NULL;
        } else {
            xnamespace = id->xnamespace ? NSgetNamespace (id->xnamespace) : NULL;
            need_parser_shift = true;
        }

        if (!strcmp (id->id, "*"))
            oprec = prec_mult;
        else if (!strcmp (id->id, "/"))
            oprec = prec_mult;
        else if (!strcmp (id->id, "%"))
            oprec = prec_mult;
        else if (!strcmp (id->id, "+"))
            oprec = prec_add;
        else if (!strcmp (id->id, "-"))
            oprec = prec_add;
        else if (!strcmp (id->id, "<<"))
            oprec = prec_shift;
        else if (!strcmp (id->id, ">>"))
            oprec = prec_shift;
        else if (!strcmp (id->id, "<")) {
            if (no_relop)
                goto out;
            else
                oprec = prec_rel;
        } else if (!strcmp (id->id, ">"))
            oprec = prec_rel;
        else if (!strcmp (id->id, "=="))
            oprec = prec_eq;
        else if (!strcmp (id->id, "!="))
            oprec = prec_eq;
        else if (!strcmp (id->id, ">="))
            oprec = prec_rel;
        else if (!strcmp (id->id, "<=")) {
            if (no_relop)
                goto out;
            else
                oprec = prec_rel;
        } else if (!strcmp (id->id, "|")) {
            if (parser->in_arraycomp_expr)
                goto out;
            else
                oprec = prec_bitor;
        }
        else if (!strcmp (id->id, "&"))
            oprec = prec_bitand;
        else if (!strcmp (id->id, "^"))
            oprec = prec_bitxor;
        else if (!strcmp (id->id, "&&"))
            oprec = prec_logand;
        else if (!strcmp (id->id, "||"))
            oprec = prec_logor;
        else if (!strcmp (id->id, "++"))
            oprec = prec_concat;
        else
            oprec = prec_userop;

        /* If we read real operation, we need to move a parser
           after the operation.  Otherwise it was a postfix
           operation taken from the previous expression which
           can be used as a binary operation.  */
        if (need_parser_shift) {
            parser_get_token (parser);
            if (id->xnamespace) {
                parser_get_token (parser); parser_get_token (parser);
            }
        }

        /* NOTE: body of this loop must be equivalent to the code at lable OUT.  */
        while (oprec <= stack[sp].prec) {
            struct location loc_sp_prev = NODE_LOCATION (stack[sp - 1].expr.expr);

            if (!strcmp (stack[sp].op, "&&"))
                stack[sp - 1].expr.expr
                  = TBmakeFuncond (stack[sp - 1].expr.expr, stack[sp].expr.expr,
                                   TBmakeBool (0));
            else if (!strcmp (stack[sp].op, "||"))
                stack[sp - 1].expr.expr
                  = TBmakeFuncond (stack[sp - 1].expr.expr, TBmakeBool (1),
                                   stack[sp].expr.expr);
            else {
                node *args;

                args = expr_constructor (stack[sp - 1].expr.expr,
                                         expr_constructor (stack[sp].expr.expr, NULL));

                stack[sp - 1].expr.expr
                  = TBmakeSpap (TBmakeSpid (stack[sp].xnamespace, stack[sp].op), args);
            }

            NODE_LOCATION (stack[sp - 1].expr.expr) = loc_sp_prev;
            sp--;
        }

        t = handle_cast_expr (parser);

        if (t.expr == error_mark_node || t.expr == NULL) {
            /* Free the memory allocated.  */
            /*error_loc (loc, "expression expected after the operation"); */
            identifier_free (id);
            while (sp >= 0)
                free_tree (stack[sp--].expr.expr);

            return error_mark_node;
        }

        sp++;
        stack[sp].expr = t;
        stack[sp].prec = oprec;
        stack[sp].op = id->id;
        stack[sp].xnamespace = xnamespace;
        free (id);
    }

out:
    while (sp > 0) {
        struct location loc_sp_prev = NODE_LOCATION (stack[sp - 1].expr.expr);

        if (!strcmp (stack[sp].op, "&&"))
            stack[sp - 1].expr.expr = TBmakeFuncond (stack[sp - 1].expr.expr,
                                                     stack[sp].expr.expr, TBmakeBool (0));
        else if (!strcmp (stack[sp].op, "||"))
            stack[sp - 1].expr.expr = TBmakeFuncond (stack[sp - 1].expr.expr,
                                                     TBmakeBool (1), stack[sp].expr.expr);
        else {
            node *args;

            args = expr_constructor (stack[sp - 1].expr.expr,
                                     expr_constructor (stack[sp].expr.expr, NULL));

            stack[sp - 1].expr.expr
              = TBmakeSpap (TBmakeSpid (stack[sp].xnamespace, stack[sp].op), args);
        }

        NODE_LOCATION (stack[sp - 1].expr.expr) = loc_sp_prev;
        sp--;
    }

    return stack[0].expr.expr;
}

/* conditional-expr
     binary-expr
     |
     binary-expr '?' expr ': conditional-expr
*/
node *
handle_conditional_expr (struct parser *parser, bool no_relop)
{
    struct token *tok;
    struct location loc;
    node *cond = error_mark_node;
    node *ifexp = error_mark_node;
    node *elseexp = error_mark_node;

    cond = handle_binary_expr (parser, no_relop);

    if (cond == NULL || cond == error_mark_node)
        return cond;

    tok = parser_get_token (parser);
    loc = token_location (tok);
    if (token_is_operator (tok, tv_question)) {
        ifexp = handle_expr (parser);
        if (ifexp == NULL || ifexp == error_mark_node) {
            error_loc (loc, "expression expected after '?'");
            goto out;
        }

        if (!parser_expect_tval (parser, tv_colon))
            goto out;

        loc = token_location (parser_get_token (parser));

        elseexp = handle_conditional_expr (parser, no_relop);
        if (elseexp == NULL || elseexp == error_mark_node) {
            error_loc (loc, "expression expected after ':'");
            goto out;
        }
    } else {
        parser_unget (parser);
        return cond;
    }

out:
    if (cond == error_mark_node || cond == NULL || ifexp == error_mark_node
        || cond == NULL || elseexp == error_mark_node || elseexp == NULL) {
        free_node (cond);
        free_node (ifexp);
        free_node (elseexp);
        return error_mark_node;
    } else {
        return loc_annotated (NODE_LOCATION (cond), TBmakeFuncond (cond, ifexp, elseexp));
    }
}

/* ::= '[' id-list ']'  */
static inline node *
handle_square_bracketed_ids (struct parser *parser)
{
    node *ids;

    if (parser_expect_tval (parser, tv_lsquare))
        parser_get_token (parser);
    else
        return error_mark_node;

    ids = handle_id_list (parser);
    if (ids == NULL || ids == error_mark_node) {
        parser_get_until_tval (parser, tv_rsquare);
        return error_mark_node;
    }

    if (parser_expect_tval (parser, tv_rsquare))
        parser_get_token (parser);
    else
        return error_mark_node;

    return ids;
}

/* Chek whether `id' which can be either N_spid or N_spids can
   be found within the NULL-terminated list of nodes, where all
   the nodes are N_spid.  */
static inline bool
id_memberof_ids (node *id, node **ids, size_t sz)
{
    if (NODE_TYPE (id) != N_spid && NODE_TYPE (id) != N_spids)
        return false;

    for (size_t i = 0; i < sz; i++) {
        if (NODE_TYPE (*ids) == N_spid
            && ((NODE_TYPE (id) == N_spid
                 && !strcmp (SPID_NAME (id), SPID_NAME (*ids)))
                || (NODE_TYPE (id) == N_spids
                    && !strcmp (SPIDS_NAME (id), SPID_NAME (*ids)))))
            return true;
        ids++;
    }
    return false;
}

/* For the chain of exprs, check that each expr can be found in the
   NULL-terminated list of ids.  */
static inline bool
exprs_are_ids (node *exprs, node **ids, size_t sz)
{
    for (size_t i = 0; i < sz; i++) {
        node *x = EXPRS_EXPR (exprs);
        if (NODE_TYPE (x) != N_spid
            || strcmp (SPID_NAME (x), SPID_NAME (*ids)))
            return false;
        ids++;
        exprs = EXPRS_NEXT (exprs);
    }
    return true;
}

/* For the N_spids chain, check that each id can be found in the
   NULL-terminated list of ids.  */
static inline bool
spids_are_ids (node *spids, node **ids, size_t sz)
{
    for (size_t i = 0; i < sz; i++) {
        if (strcmp (SPIDS_NAME (spids), SPID_NAME (*ids)))
            return false;
        ids++;
        spids = SPIDS_NEXT (spids);
    }
    return true;
}

/* Convert an expr chain into N_spids, under te assumption that
   each expr in the chain is N_spid.  */
static inline node *
exprs_to_ids (node *exprs)
{
    if (!exprs)
        return NULL;

    node *x = EXPRS_EXPR (exprs);
    assert (x && NODE_TYPE (x) == N_spid, "a chain of ids expected");
    return id_constructor (x, exprs_to_ids (EXPRS_NEXT (exprs)));
}

/* generator ::=
        '(' expr (< | <=) ((id = [ ids ]) | id | [ ids ]) (< | <=) expr
            ( 'step' expr ( 'width' expr )? )?  ')'
*/
static node *
handle_generator_body (struct parser *parser,
                       bool array_comprehension_p, struct array_comp_ctxt *ctxt)
{
    struct token *tok;
    struct location loc;
    struct location generator_loc;
    node *gen_start = NULL;
    node *gen_end = NULL;
    node *gen_idx = error_mark_node;
    node *step = NULL;
    node *width = NULL;
    prf gen_first_op = F_wl_le, gen_last_op = F_wl_le;

    bool ub_only = false;
    bool lb_only = false;

    /* '.' | expr  */
    tok = parser_get_token (parser);
    generator_loc = token_location (tok);

    if (token_is_operator (tok, tv_dot))
        gen_start = loc_annotated (token_location (tok), TBmakeDot (1));
    else {
        parser_unget (parser);
        gen_start = handle_conditional_expr (parser, true);
        if (error_mark_node == gen_start)
            goto error;
    }

    /* If we are parsing for array comprehensions, we check if the first
       expression is a bound variable.  If it is, we are dealing with a
       short generator of the form (iv ('<' | '<=') ub).  */
    if (array_comprehension_p) {
        if (ctxt->single_bound_variable
            && NODE_TYPE (gen_start) == N_spid
            && !strcmp (SPID_NAME (gen_start), SPID_NAME (*ctxt->bound))) {
           gen_idx = loc_annotated (generator_loc,
                                    TBmakeWithid (id_constructor (gen_start, NULL),
                                                  NULL));
           gen_start = NULL;
           ub_only = true;
        }

        if (!ctxt->single_bound_variable
            && NODE_TYPE (gen_start) == N_array
            && exprs_are_ids (ARRAY_AELEMS (gen_start), ctxt->bound, ctxt->bound_sz)) {
           gen_idx = loc_annotated (generator_loc,
                                    TBmakeWithid (NULL,
                                                  exprs_to_ids (ARRAY_AELEMS (gen_start))));
           gen_start = NULL;
           ub_only = true;
        }
    }

    tok = parser_get_token (parser);
    if (token_is_operator (tok, tv_lt))
        gen_first_op = F_wl_lt;
    else if (token_is_operator (tok, tv_lt_eq))
        gen_first_op = F_wl_le;
    else {
        error_loc (token_location (tok),
                   "`<' or `<=' operator expected in the generator");
        /* FIXME skip until the end of generator.  */
        goto error;
    }

    /* If the generator only specifies the upper bound (iv <= ub)
       then the above '<' or '<=' corresponds to the `gen_last_op`.  */
    if (ub_only) {
        gen_last_op = gen_first_op;
        gen_first_op = F_wl_le;

    } else {
        /* Parse the variable part of the generator.  */

        tok = parser_peek_token (parser);
        loc = token_location (tok);
        if (token_is_operator (tok, tv_lsquare)) {
            node *ids;

            ids = handle_square_bracketed_ids (parser);
            if (ids == error_mark_node)
                goto error;

            /* Within array comprehensions, this variable has to correspond to
               one of the bound variables, otherwise this is illegal expression.  */
            if (array_comprehension_p && ctxt->single_bound_variable) {
                error_loc (NODE_LOCATION (ids), "cannot use the list of constraint variables "
                           "here: use `%s' variable isntead", SPID_NAME (*ctxt->bound));
                goto error;
            }

            if (array_comprehension_p
                && !spids_are_ids (ids, ctxt->bound, ctxt->bound_sz)) {
                error_loc (NODE_LOCATION (ids), "constraint variable list must match "
                           "the list of varibales on the left hand side of the "
                           "`->' without the '.'-s");
                goto error;
            }

            gen_idx = loc_annotated (loc, TBmakeWithid (NULL, ids));
        } else {
            node *id;

            id = handle_id (parser);
            if (id == error_mark_node)
                /* FIXME skip something.  */
                goto error;


            /* Within array comprehensions, this variable has to correspond to
               one of the bound variables, otherwise this is illegal expression.  */
            if (array_comprehension_p && !ctxt->single_bound_variable) {
                error_loc (NODE_LOCATION (id), "cannot use a single constraint variable "
                           "here: use variable list isntead");
                goto error;
            }

            if (array_comprehension_p
                && strcmp (SPID_NAME (id), SPID_NAME (*ctxt->bound))) {
                error_loc (NODE_LOCATION (id), "constraint variable must be `%s'",
                           SPID_NAME (*ctxt->bound));
                goto error;
            }

            id = id_constructor (id, NULL);

            tok = parser_peek_token (parser);
            if (!array_comprehension_p && token_is_operator (tok, tv_assign)) {
                parser_get_token (parser);
                node *ids = handle_square_bracketed_ids (parser);
                if (ids == error_mark_node)
                    goto error;

                gen_idx = loc_annotated (loc, TBmakeWithid (id, ids));
            } else {
                gen_idx = loc_annotated (loc, TBmakeWithid (id, NULL));
            }
        }

    }

    tok = parser_get_token (parser);
    if (token_is_operator (tok, tv_lt))
        gen_last_op = F_wl_lt;
    else if (token_is_operator (tok, tv_lt_eq))
        gen_last_op = F_wl_le;
    else if (array_comprehension_p) {
        /* We can omit the upper bound specification in array comprehensions.  */
        parser_unget (parser);
        /* If we don't have '<' or '<=', and we are not parsing the upper bound
           of the generator only, we are parsing for the lower bound.  */
        lb_only = !ub_only;
    } else {
        error_loc (token_location (tok),
                   "`<' or `<=' operator expected in the generator");
        //parser_get_until_tval (parser, tv_rparen);
        goto error;
    }


    if (!lb_only) {
        tok = parser_get_token (parser);
        if (token_is_operator (tok, tv_dot)) {
            gen_end = loc_annotated (token_location (tok), TBmakeDot (1));
        } else {
            parser_unget (parser);
            if (error_mark_node == (gen_end = handle_expr (parser)))
                goto error;
        }
    }

    tok = parser_get_token (parser);
    loc = token_location (tok);

    if (token_is_keyword (tok, STEP)) {
        step = handle_expr (parser);
        if (step == error_mark_node)
            goto error;
    } else
        parser_unget (parser);

    tok = parser_get_token (parser);
    if (token_is_keyword (tok, WIDTH)) {
        width = handle_expr (parser);
        if (width == error_mark_node)
            goto error;

        if (step == NULL) {
            warning_loc (loc, "width vector ignored due to missing step vector");
            free_node (width);
        }
    } else
        parser_unget (parser);



    return loc_annotated (generator_loc,
                          TBmakePart (NULL, gen_idx,
                                      loc_annotated (generator_loc,
                                                     TBmakeGenerator (gen_first_op,
                                                                      gen_last_op,
                                                                      gen_start, gen_end,
                                                                      step, width))));

error:
    free_node (gen_start);
    free_node (gen_end);
    free_node (gen_idx);
    free_node (step);
    free_node (width);
    return error_mark_node;
}

/* ::= '{' statement-list '}' | empty  */
node *
handle_wl_assign_block (struct parser *parser)
{
    struct token *tok;
    node *block = NULL;

    tok = parser_get_token (parser);
    parser_unget (parser);

    if (token_is_operator (tok, tv_lbrace)) {
        block = handle_stmt_list (parser, STMT_BLOCK_STMT_FLAGS);
        if (block == error_mark_node)
            goto error;
    }

    if (block == NULL)
        return MAKE_EMPTY_BLOCK ();
    else if (block != error_mark_node)
        return block;

error:
    free_node (block);
    return error_mark_node;
}

/* with-generator assign-block? ( ':' ( '(' expr-list ')' ) | expr )? ';'  */
node *
handle_npart (struct parser *parser)
{
    struct token *tok;
    node *generator = error_mark_node;
    node *block = error_mark_node;
    node *exprs = NULL;
    node *ret = error_mark_node;

    /* eat '(' and save its location.  */
    if (parser_expect_tval (parser, tv_lparen))
        parser_get_token (parser);
    else
        goto error;

    generator = handle_generator_body (parser, false, NULL);

    if (generator == error_mark_node) {
        parser_get_until_tval (parser, tv_rparen);
        goto error;
    }

    /* eat ')'  */
    if (parser_expect_tval (parser, tv_rparen))
        parser_get_token (parser);
    else
        goto error;


    block = handle_wl_assign_block (parser);
    if (block == error_mark_node)
        goto error;

    tok = parser_get_token (parser);
    if (token_is_operator (tok, tv_colon)) {
        /* We share `in_return' parser flag to avoid creating N_exprs
           for a single expr.  However, as with-loop can be inside the
           return expression, we cannot turn off the `in_return' flag.
           Instead we save its state, and restore it after the expression
           list was handled.  */
        bool old_return_state = parser->in_return;

        parser->in_return = true;
        exprs = handle_expr (parser);
        parser->in_return = old_return_state;

        if (!exprs) {
            error_loc (token_location (tok), "expression expected");
            parser_get_until_tval (parser, tv_semicolon);
            goto error;
        }

        if (exprs == error_mark_node) {
            parser_get_until_tval (parser, tv_semicolon);
            goto error;
        }

        if (NODE_TYPE (exprs) != N_exprs)
            exprs = expr_constructor (exprs, NULL);

    } else
        parser_unget (parser);

    if (parser_expect_tval (parser, tv_semicolon))
        parser_get_token (parser);
    else
        goto error;

    ret = loc_annotated (NODE_LOCATION (generator),
                         TBmakeWith (generator,
                                     loc_annotated (NODE_LOCATION (block),
                                                    TBmakeCode (block, exprs)),
                                     NULL));
    CODE_USED (WITH_CODE (ret))++;
    PART_CODE (generator) = WITH_CODE (ret);
    return ret;

error:
    free_node (generator);
    free_node (block);
    free_node (exprs);
    return error_mark_node;
}

/* ::= with-npart ( with-npart )*   */
node *
handle_nparts (struct parser *parser)
{
    struct token *tok;
    node *npart;
    node *nparts;
    node *wptmp;
    node *wctmp;

    npart = handle_npart (parser);
    if (npart == error_mark_node)
        goto error;

    nparts = npart;
    wptmp = WITH_PART (npart);
    wctmp = WITH_CODE (npart);
    while (true) {
        node *next;

        tok = parser_get_token (parser);
        parser_unget (parser);

        if (!token_is_operator (tok, tv_lparen))
            break;

        next = handle_npart (parser);
        if (next == error_mark_node)
            goto error;

        /* Linked list standard addition.  WPTMP and WCTMP are
           pointers at the tail of the list.  */
        PART_NEXT (wptmp) = WITH_PART (next);
        CODE_NEXT (wctmp) = WITH_CODE (next);

        wptmp = PART_NEXT (wptmp);
        wctmp = CODE_NEXT (wctmp);

        /* The node containing pointers to the WITH_CODE and
           WITH_PART is being freed.  */
        WITH_CODE (next) = NULL;
        WITH_PART (next) = NULL;
        free_tree (next);
    }

    return nparts;

error:
    free_node (npart);
    return error_mark_node;
}

/* genarray '(' expr ',' expr ')'
   |
   modarray '(' expr ')'
   |
   fold '(' expr ',' expr ')'
   |
   foldfix '(' expr ',' expr ',' expr ')'
   |
   propagate '(' expr ')'
*/
node *
handle_withop (struct parser *parser)
{
    struct token *tok;
    struct location loc;
    node *exp1 = NULL;
    node *exp2 = NULL;
    node *exp3 = NULL;

    tok = parser_get_token (parser);
    loc = token_location (tok);

    if (token_is_keyword (tok, GENARRAY)) {
        if (parser_expect_tval (parser, tv_lparen))
            parser_get_token (parser);
        else
            goto error;

        if (error_mark_node == (exp1 = handle_expr (parser))) {
            parser_get_until_tval (parser, tv_rparen);
            goto error;
        }

        tok = parser_get_token (parser);
        if (token_is_operator (tok, tv_comma)) {
            if (error_mark_node == (exp2 = handle_expr (parser))) {
                parser_get_until_tval (parser, tv_rparen);
                goto error;
            }
        } else
            parser_unget (parser);

        if (parser_expect_tval (parser, tv_rparen))
            parser_get_token (parser);
        else
            goto error;

        return loc_annotated (loc, TBmakeGenarray (exp1, exp2));
    } else if (token_is_keyword (tok, MODARRAY)) {
        if (parser_expect_tval (parser, tv_lparen))
            parser_get_token (parser);
        else
            goto error;

        if (error_mark_node == (exp1 = handle_expr (parser)))
            goto error;

        if (parser_expect_tval (parser, tv_rparen))
            parser_get_token (parser);
        else
            goto error;

        return loc_annotated (loc, TBmakeModarray (exp1));
    } else if (token_is_keyword (tok, PROPAGATE)) {
        if (parser_expect_tval (parser, tv_lparen))
            parser_get_token (parser);
        else
            goto error;

        if (error_mark_node == (exp1 = handle_expr (parser)))
            goto error;

        if (parser_expect_tval (parser, tv_rparen))
            parser_get_token (parser);
        else
            goto error;

        return loc_annotated (loc, TBmakePropagate (exp1));
    } else if (token_is_keyword (tok, FOLD) || token_is_keyword (tok, FOLDFIX)) {
        bool foldfix_p = token_value (tok) == FOLDFIX;
        struct token *tok;
        struct identifier *id;
        node *ret;
        /* In case of partial application, we may have a function-call
           instead of just a function-name, which we store in SPFOLD_ARGS.  */
        node *args = NULL;

        if (parser_expect_tval (parser, tv_lparen))
            parser_get_token (parser);
        else
            goto error;

        if (NULL != (id = is_ext_id (parser))) {
            /* if (!is_binary (parser, id->xnamespace, id->id))
              {
                tok = parser_get_token (parser);
                parser_unget (parser);
                error_loc (token_location (tok), "binary function expected");
                parser_get_until_tval (parser, tv_rparen);
                goto error;
              } */

            identifier_free (id);
            exp1 = handle_ext_id (parser);

            /* Check if we have a partial application of the function.  */
            tok = parser_get_token (parser);
            parser_unget (parser);

            if (token_is_operator (tok, tv_lparen)) {
                args = handle_funcall_args (parser);
                if (args == error_mark_node) {
                    parser_get_until_tval (parser, tv_comma);
                    parser_get_until_tval (parser, tv_rparen);
                    goto error;
                }
            }
        } else {
            tok = parser_get_token (parser);
            parser_unget (parser);
            error_loc (token_location (tok), "invalid function name `%s' found",
                       token_as_string (tok));
            parser_get_until_tval (parser, tv_rparen);
            goto error;
        }

        if (exp1 == error_mark_node) {
            parser_get_until_tval (parser, tv_rparen);
            goto error;
        }

        if (parser_expect_tval (parser, tv_comma))
            parser_get_token (parser);
        else {
            parser_get_until_tval (parser, tv_rparen);
            goto error;
        }

        if (error_mark_node == (exp2 = handle_expr (parser))) {
            parser_get_until_tval (parser, tv_rparen);
            goto error;
        }

        if (foldfix_p) {
            if (parser_expect_tval (parser, tv_comma))
                parser_get_token (parser);
            else {
                parser_get_until_tval (parser, tv_rparen);
                goto error;
            }

            if (error_mark_node == (exp3 = handle_expr (parser))) {
                parser_get_until_tval (parser, tv_rparen);
                goto error;
            }
        }

        if (parser_expect_tval (parser, tv_rparen))
            parser_get_token (parser);
        else
            goto error;

        ret = loc_annotated (loc, TBmakeSpfold (exp2));
        SPFOLD_FN (ret) = exp1;
        SPFOLD_ARGS (ret) = args;

        if (foldfix_p)
            SPFOLD_GUARD (ret) = exp3;

        return ret;
    } else
        error_loc (token_location (tok),
                   "with-loop operation "
                   "expected, `%s' found",
                   token_as_string (tok));

error:
    free_node (exp1);
    free_node (exp2);
    free_node (exp3);
    return error_mark_node;
}

/* ::= 'with' '{' with-nparts '}' ':' withop  */
node *
handle_with (struct parser *parser)
{
    struct token *tok;
    struct location with_loc;
    node *nparts = error_mark_node;
    node *withop = error_mark_node;
    node *pragma_expr = NULL;

    /* eat `with' and save its location.  */
    if (parser_expect_tval (parser, NWITH))
        with_loc = token_location (parser_get_token (parser));
    else
        goto error;

    /* '{'  */
    if (parser_expect_tval (parser, tv_lbrace))
        parser_get_token (parser);
    else
        goto error;

    /* check for '#' 'pragma' 'wlcomp'   */
    tok = parser_get_token (parser);
    if (token_is_operator (tok, tv_hash)) {
        node *t = error_mark_node;
        struct location pragma_loc;

        if (parser_expect_tval (parser, PRAGMA))
            pragma_loc = token_location (parser_get_token (parser));
        else
            goto error;

        tok = parser_get_token (parser);
        if (token_is_keyword (tok, WLCOMP)) {
            pragma_expr = handle_function_call (parser);
            if (pragma_expr == error_mark_node)
                goto error;

            t = loc_annotated (pragma_loc, TBmakePragma ());
            PRAGMA_WLCOMP_APS (t) = expr_constructor (pragma_expr, NULL);
            pragma_expr = t;
        } else if (token_is_keyword (tok, NOCUDA)) {
            t = loc_annotated (pragma_loc, TBmakePragma ());
            PRAGMA_NOCUDA (t) = TRUE;
            pragma_expr = t;
        } else
            goto error;
    } else
        parser_unget (parser);

    /* If '}' then the body is empty.  */
    tok = parser_get_token (parser);
    if (!token_is_operator (tok, tv_rbrace)) {
        parser_unget (parser);
        nparts = handle_nparts (parser);
        if (nparts == error_mark_node)
            goto error;

        /* '}'  */
        if (parser_expect_tval (parser, tv_rbrace))
            parser_get_token (parser);
        else
            goto error;
    } else
        nparts = loc_annotated (with_loc, TBmakeWith (NULL, NULL, NULL));

    /* ':'  */
    if (parser_expect_tval (parser, tv_colon))
        parser_get_token (parser);
    else
        goto error;

    tok = parser_get_token (parser);
    /* Handle a list of withops.  */
    if (token_is_operator (tok, tv_lparen)) {
        bool withop_error = false;
        node *withop_end = NULL;

        withop = NULL;
        while (true) {
            node *t = handle_withop (parser);

            if (t == error_mark_node)
                withop_error = true;

            if (!withop_error) {
                if (!withop)
                    withop_end = withop = t;
                else {
                    L_WITHOP_NEXT (withop_end, t);
                    withop_end = t;
                }
            }

            tok = parser_get_token (parser);
            if (!token_is_operator (tok, tv_comma)) {
                parser_unget (parser);
                break;
            }
        }

        if (parser_expect_tval (parser, tv_rparen))
            parser_get_token (parser);
        else
            goto error;

        if (withop_error)
            goto error;
    } else {
        parser_unget (parser);
        withop = handle_withop (parser);
    }

    if (withop == error_mark_node)
        goto error;

    WITH_WITHOP (nparts) = withop;
    WITH_PRAGMA (nparts) = pragma_expr;
    return nparts;

error:
    free_node (nparts);
    free_node (withop);
    return error_mark_node;
}

/* expr ::= conditional-expr
 */
node *
handle_expr (struct parser *parser)
{
    return handle_conditional_expr (parser, false);
}

/*
   assign-op:: '=' | '+=' | '-=' | '*=' | '/=' | '%='
   assign::
      ( '++' | '--' ) id
      id ( '++' | '--' )

      id-list assign-op expr-list
      |
      id '[' expr ']' assign-op expr
      with
      |
      function-call
*/
node *
handle_assign (struct parser *parser)
{
    struct token *tok;
    struct location loc;
    node *lhs = error_mark_node;
    node *ret = error_mark_node;

    tok = parser_get_token (parser);
    loc = token_location (tok);

    parser_unget (parser);
    lhs = handle_expr (parser);
    if (lhs == NULL || lhs == error_mark_node)
        return lhs;

    switch (NODE_TYPE (lhs)) {
    case N_spid:
        tok = parser_get_token (parser);
        if (token_is_operator (tok, tv_comma)) {
            node *ids;

            ids = handle_id_list (parser);
            if (ids == error_mark_node)
                goto out;

            lhs = id_constructor (lhs, ids);
        } else if (token_is_operator (tok, tv_assign)) {
            parser_unget (parser);
            lhs = id_constructor (lhs, NULL);
        }
        /* This should happen when handle_expr works, as it
           needs to check if '++' is not a concatenation. */
        else if (token_is_operator (tok, tv_plus_plus)) {
            node *id = DUPdoDupTree (lhs);
            node *args = expr_constructor (lhs, NULL);

            lhs = TBmakeSpap (TBmakeSpid (NULL, strdup ("++")), args);
            ret = TBmakeLet (id_constructor (id, NULL), lhs);
            NODE_LOCATION (ret) = NODE_LOCATION (lhs) = loc;
            return ret;
        } else if (token_is_operator (tok, tv_minus_minus)) {
            node *id = DUPdoDupTree (lhs);
            node *args = expr_constructor (lhs, NULL);

            lhs = TBmakeSpap (TBmakeSpid (NULL, strdup ("--")), args);
            ret = TBmakeLet (id_constructor (id, NULL), lhs);
            NODE_LOCATION (ret) = NODE_LOCATION (lhs) = loc;
            return ret;
        } else
            /* In this case we assume that the expression is followed
               by the operation-equal operator like +=, -=, ... and
               lhs should be N_spid, rather than N_spids.  */
            parser_unget (parser);

        break;

    case N_spap:
        /* Special case converting `sel = expr' into modarray.  */
        if (!strcmp (SPID_NAME (SPAP_ID (lhs)), "sel")
            && NODE_TYPE (EXPRS_EXPR (EXPRS_NEXT (SPAP_ARGS (lhs)))) == N_spid) {
            struct token *tok = parser_get_token (parser);
            char *op = NULL;

            if (token_class (tok) != tok_operator)
                goto out;

            switch (token_value (tok)) {
            case tv_plus_eq:
                op = strdup ("+");
                break;
            case tv_minus_eq:
                op = strdup ("-");
                break;
            case tv_mult_eq:
                op = strdup ("*");
                break;
            case tv_div_eq:
                op = strdup ("/");
                break;
            case tv_mod_eq:
                op = strdup ("%");
                break;
            case tv_and_eq:
                op = strdup ("&");
                break;
            case tv_or_eq:
                op = strdup ("|");
                break;
            case tv_shl_eq:
                op = strdup ("<<");
                break;
            case tv_shr_eq:
                op = strdup (">>");
                break;
            case tv_assign:
                break;
            default:
                error_loc (token_location (tok), "assignment operator expected");
                goto out;
            }

            ret = handle_expr (parser);
            if (ret == NULL || ret == error_mark_node)
                goto out;
            else {
                node *cpy = NULL;

                if (op != NULL)
                    cpy = DUPdoDupTree (lhs);

                /* FIXME we do not check that the namespace of id is NULL.  */
                node *id = DUPdoDupTree (EXPRS_EXPR (EXPRS_NEXT (SPAP_ARGS (lhs))));
                node *ids = TBmakeSpids (strdup (SPID_NAME (id)), NULL);
                node *args = DUPdoDupTree (EXPRS_EXPR (SPAP_ARGS (lhs)));

                /* If we had a situation A[expr] += expr1, we transform it into
                   modarray (A, expr, + (A[expr], expr1))  */
                if (op != NULL) {
                    ret
                      = TBmakeSpap (TBmakeSpid (NULL, op),
                                    expr_constructor (ret, expr_constructor (cpy, NULL)));
                    NODE_LOCATION (ret) = loc;
                }

                node *ap = TBmakeSpap (
                  TBmakeSpid (NULL, strdup ("modarray")),
                  expr_constructor (id, expr_constructor (args,
                                                          expr_constructor (ret, NULL))));
                NODE_LOCATION (ap) = loc;
                free_node (lhs);
                return loc_annotated (loc, TBmakeLet (ids, ap));
            }
        }
        /* convert   ++ (id)  or -- (id) into  id = ++/-- (id)
           this is a special case for ++ and -- operations as they
           are allowed to act as expressions.  */
        else if ((!strcmp (SPID_NAME (SPAP_ID (lhs)), "++")
                  || !strcmp (SPID_NAME (SPAP_ID (lhs)), "--"))
                 && SPID_NS (SPAP_ID (lhs)) == NULL && TCcountExprs (SPAP_ARGS (lhs)) == 1
                 && NODE_TYPE (EXPRS_EXPR (SPAP_ARGS (lhs))) == N_spid) {
            node *id = EXPRS_EXPR (SPAP_ARGS (lhs));
            if (SPID_NS (id) != NULL) {
                error ("%s applied to a namsepaced variable", SPID_NAME (SPAP_ID (lhs)));
                return error_mark_node;
            }

            return loc_annotated (loc,
                                  TBmakeLet (id_constructor (DUPdoDupTree (id), NULL),
                                             lhs));
        }

        /* ... fallthrough ...  */
    case N_with:
        return loc_annotated (loc, TBmakeLet (NULL, lhs));

    case N_spids:
        break;

    default:
        warning_loc (loc, "unsupported expression in assignment lhs");
        return loc_annotated (loc, TBmakeLet (NULL, lhs));
    }

    tok = parser_get_token (parser);
/* XXX The rules that original SaC uses to parse op-equal stuff are
   strange.  You cannot have a variable with a namespace inside the
   +=, -=,...assignment, and you cannot write something like
   a, b += foo (x);  */
#define make_op_on_let(lhs, rhs, op)                                                     \
    do {                                                                                 \
        node *__ap;                                                                      \
        assert (NODE_TYPE (lhs) == N_spid, "op-equal does not support "                  \
                                           "lists on left-hand side ");                  \
        __ap = TBmakeSpap (TBmakeSpid (NULL, strdup (op)),                               \
                           expr_constructor (lhs, expr_constructor (rhs, NULL)));        \
        ret = TBmakeLet (id_constructor (DUPdoDupTree (lhs), NULL),                      \
                         loc_annotated (NODE_LOCATION (lhs), __ap));                     \
    } while (0)

#define get_rhs_convert_plusop(parser, lhs, op)                                          \
    do {                                                                                 \
        node *__rhs;                                                                     \
        struct location __loc;                                                           \
                                                                                         \
        __loc = token_location (parser_get_token (parser));                              \
        parser_unget (parser);                                                           \
        __rhs = handle_expr (parser);                                                    \
                                                                                         \
        if (NODE_TYPE (lhs) != N_spid) {                                                 \
            error_loc (__loc, "op-equal does not support lists on "                      \
                              "left-hand side of expression");                           \
            free_node (__rhs);                                                           \
            goto out;                                                                    \
        }                                                                                \
        if (__rhs == NULL || __rhs == error_mark_node) {                                 \
            error_loc (__loc, "right hand side expression of the "                       \
                              "assignment expected");                                    \
            goto out;                                                                    \
        } else                                                                           \
            make_op_on_let (lhs, __rhs, op);                                             \
    } while (0)

    /* FIXME rhs can be an expr-list in case lhs is.  */
    if (token_is_operator (tok, tv_assign)) {
        node *rhs = handle_expr (parser);
        if (rhs == NULL || rhs == error_mark_node)
            goto out;
        else
            ret = TBmakeLet (lhs, rhs);
    } else if (token_is_operator (tok, tv_plus_eq))
        get_rhs_convert_plusop (parser, lhs, "+");
    else if (token_is_operator (tok, tv_minus_eq))
        get_rhs_convert_plusop (parser, lhs, "-");
    else if (token_is_operator (tok, tv_mult_eq))
        get_rhs_convert_plusop (parser, lhs, "*");
    else if (token_is_operator (tok, tv_div_eq))
        get_rhs_convert_plusop (parser, lhs, "/");
    else if (token_is_operator (tok, tv_mod_eq))
        get_rhs_convert_plusop (parser, lhs, "%");
    else if (token_is_operator (tok, tv_and_eq))
        get_rhs_convert_plusop (parser, lhs, "&");
    else if (token_is_operator (tok, tv_or_eq))
        get_rhs_convert_plusop (parser, lhs, "|");
    else if (token_is_operator (tok, tv_shl_eq))
        get_rhs_convert_plusop (parser, lhs, "<<");
    else if (token_is_operator (tok, tv_shr_eq))
        get_rhs_convert_plusop (parser, lhs, ">>");
    else {
        error_loc (token_location (tok), "assignment operator expected");
        goto out;
    }

    if (ret != NULL && ret != error_mark_node)
        NODE_LOCATION (ret) = loc;

    return ret;

out:
    free_node (lhs);
    free_node (ret);
    return error_mark_node;
}

/* if-stmt ::= 'if' '(' expr ')' statement-list ( 'else' statement-list )?  */
node *
handle_if_stmt (struct parser *parser)
{
    struct token *tok;
    struct location loc;
    node *cond = error_mark_node;
    node *if_branch = error_mark_node;
    node *else_branch = error_mark_node;

    tok = parser_get_token (parser);
    loc = token_location (tok);

    assert (token_is_keyword (tok, IF),
            "%s cannot parse an expression which starts with %s", __func__,
            token_as_string (tok));

    if (parser_expect_tval (parser, tv_lparen))
        parser_get_token (parser);
    else
        goto error;

    if (error_mark_node == (cond = handle_expr (parser)))
        /* FIXME skip to somewhere...  */
        goto error;

    if (parser_expect_tval (parser, tv_rparen))
        parser_get_token (parser);
    else
        goto error;

    if_branch = handle_stmt_list (parser, STMT_BLOCK_STMT_FLAGS);
    if (error_mark_node == if_branch)
        goto error;

    tok = parser_get_token (parser);
    if (token_is_keyword (tok, ELSE)) {
        else_branch = handle_stmt_list (parser, STMT_BLOCK_STMT_FLAGS);
        if (error_mark_node == else_branch)
            goto error;
    } else {
        parser_unget (parser);
        else_branch = MAKE_EMPTY_BLOCK ();
    }

    return loc_annotated (loc, TBmakeCond (cond, if_branch, else_branch));

error:
    free_node (cond);
    free_node (if_branch);
    free_node (else_branch);
    return error_mark_node;
}

/* while-stmt ::= 'while' '(' expr ')' statement-list  */
node *
handle_while_stmt (struct parser *parser)
{
    struct token *tok;
    struct location loc;
    node *cond = error_mark_node;
    node *stmts = error_mark_node;

    tok = parser_get_token (parser);
    loc = token_location (tok);

    assert (token_is_keyword (tok, WHILE),
            "%s cannot parse an expression which starts with %s", __func__,
            token_as_string (tok));

    if (parser_expect_tval (parser, tv_lparen))
        parser_get_token (parser);
    else
        goto error;

    if (error_mark_node == (cond = handle_expr (parser)))
        /* FIXME skip to somewhere...  */
        goto error;

    if (parser_expect_tval (parser, tv_rparen))
        parser_get_token (parser);
    else
        goto error;

    stmts = handle_stmt_list (parser, STMT_BLOCK_STMT_FLAGS);
    if (stmts == error_mark_node)
        goto error;

    return loc_annotated (loc, TBmakeWhile (cond, stmts));

error:
    free_node (cond);
    free_node (stmts);
    return error_mark_node;
}

/* do-stmt ::= 'do' statement-list 'while' '(' expr ')'  */
node *
handle_do_stmt (struct parser *parser)
{
    struct token *tok;
    struct location loc;
    node *cond = error_mark_node;
    node *stmts = error_mark_node;

    tok = parser_get_token (parser);
    loc = token_location (tok);

    assert (token_is_keyword (tok, DO),
            "%s cannot parse an expression which starts with %s", __func__,
            token_as_string (tok));

    stmts = handle_stmt_list (parser, STMT_BLOCK_STMT_FLAGS);
    if (stmts == error_mark_node)
        goto error;

    if (parser_expect_tval (parser, WHILE))
        parser_get_token (parser);
    else
        goto error;

    if (parser_expect_tval (parser, tv_lparen))
        parser_get_token (parser);
    else
        goto error;

    if (error_mark_node == (cond = handle_expr (parser)))
        /* FIXME skip to somewhere...  */
        goto error;

    if (parser_expect_tval (parser, tv_rparen))
        parser_get_token (parser);
    else
        goto error;

    return loc_annotated (loc, TBmakeDo (cond, stmts));

error:
    free_node (cond);
    free_node (stmts);
    return error_mark_node;
}

/* for-expr ::= 'for' '(' assign-list ';' expr ';' assign-list ')' statement-list  */
node *
handle_for_stmt (struct parser *parser)
{
    struct token *tok;
    struct location loc;
    node *cond_exp1 = error_mark_node;
    node *cond_exp2 = error_mark_node;
    node *cond_exp3 = error_mark_node;
    node *stmts = error_mark_node;
    node *ret;

    tok = parser_get_token (parser);
    loc = token_location (tok);

    assert (token_is_keyword (tok, FOR),
            "%s cannot parse an expression which starts with %s", __func__,
            token_as_string (tok));

    if (parser_expect_tval (parser, tv_lparen))
        parser_get_token (parser);
    else
        goto error;

    /* Check if we omit the first assignment.  */
    tok = parser_get_token (parser);
    if (token_is_operator (tok, tv_semicolon))
        cond_exp1 = NULL;
    else {
        parser_unget (parser);
        if (error_mark_node == (cond_exp1 = handle_assign_list (parser)))
            /* FIXME skip to somewhere...  */
            goto error;

        if (parser_expect_tval (parser, tv_semicolon))
            parser_get_token (parser);
        else
            goto error;
    }

    /* We do not allow to miss the middle expression however, as we
       do not have breaks allowed in the language.  */
    if (error_mark_node == (cond_exp2 = handle_expr (parser)))
        /* FIXME skip to somewhere...  */
        goto error;

    if (parser_expect_tval (parser, tv_semicolon))
        parser_get_token (parser);
    else
        goto error;

    /* Here we can have no expression again, it means
       that we would face ')'  */
    tok = parser_get_token (parser);
    if (token_is_operator (tok, tv_rparen))
        cond_exp3 = NULL;
    else {
        parser_unget (parser);
        if (error_mark_node == (cond_exp3 = handle_assign_list (parser)))
            /* FIXME skip to somewhere...  */
            goto error;

        if (parser_expect_tval (parser, tv_rparen))
            parser_get_token (parser);
        else
            goto error;
    }

    stmts = handle_stmt_list (parser, STMT_BLOCK_STMT_FLAGS);
    if (stmts == error_mark_node)
        goto error;

    BLOCK_ASSIGNS (stmts) = TCappendAssign (BLOCK_ASSIGNS (stmts), cond_exp3);
    ret
      = loc_annotated (loc,
                       TBmakeAssign (loc_annotated (loc, TBmakeWhile (cond_exp2, stmts)),
                                     NULL));
    ret = TCappendAssign (cond_exp1, ret);
    return ret;

error:
    free_node (cond_exp1);
    free_node (cond_exp2);
    free_node (cond_exp3);
    free_node (stmts);
    return error_mark_node;
}

/* stmt ::= if-stmt | do-stmt | while-stmt | for-stmt | assign   */
node *
handle_stmt (struct parser *parser)
{
    struct token *tok;
    struct location loc;
    node *ret = error_mark_node;
    bool for_loop_p = false;

    tok = parser_get_token (parser);
    loc = token_location (tok);
    parser_unget (parser);

    if (token_is_keyword (tok, IF))
        ret = handle_if_stmt (parser);
    else if (token_is_keyword (tok, DO)) {
        ret = handle_do_stmt (parser);
        if (parser_expect_tval (parser, tv_semicolon))
            parser_get_token (parser);
        else
            goto error;
    } else if (token_is_keyword (tok, WHILE))
        ret = handle_while_stmt (parser);
    else if (token_is_keyword (tok, FOR)) {
        ret = handle_for_stmt (parser);
        for_loop_p = true;
    } else {
        ret = handle_assign (parser);
        if (NULL == ret)
            return ret;

        if (error_mark_node == ret) {
            parser_get_until_tval (parser, tv_semicolon);
            goto error;
        }

        if (parser_expect_tval (parser, tv_semicolon))
            parser_get_token (parser);
        else
            goto error;
    }

    if (ret != error_mark_node) {
        NODE_LOCATION (ret) = loc;
        return for_loop_p ? ret : loc_annotated (loc, TBmakeAssign (ret, NULL));
    } else
        goto error;

error:
    free_node (ret);
    return error_mark_node;
}

/* list-of-stmts ::= ( stmt )*

   FIXME continue if error occured. Currently we just stop.
   Try to avoid recursion.   */
node *
handle_list_of_stmts (struct parser *parser)
{
    struct token *tok;
    node *res = NULL;
    node *t = NULL;
    bool parse_error = false;

    res = handle_stmt (parser);
    if (res == NULL)
        return res;

    /* Try to proceed even if the current assignmen contains
       an error; just avoid constructing a list.  */
    if (res == error_mark_node)
        parse_error = true;
    else {
        t = res;
        while (ASSIGN_NEXT (t))
            t = ASSIGN_NEXT (t);
    }

    while (true) {
        node *stmt;

        tok = parser_get_token (parser);
        parser_unget (parser);
        if (token_starts_expr (parser, tok) || token_is_keyword (tok, IF)
            || token_is_keyword (tok, WHILE) || token_is_keyword (tok, DO)
            || token_is_keyword (tok, FOR))
            stmt = handle_stmt (parser);
        else
            break;

        if (stmt == NULL)
            break;

        if (stmt == error_mark_node)
            parse_error = true;

        if (!parse_error) {
            ASSIGN_NEXT (t) = stmt;
            t = stmt;

            /* ret may be an assignment chain of more than one
               variables.  So we have to move down to the last
               element, in order to add things correctly further.  */
            while (ASSIGN_NEXT (t))
                t = ASSIGN_NEXT (t);
        }
    }

    if (!parse_error)
        return res;
    else {
        free_tree (res);
        return error_mark_node;
    }
}

node *
handle_var_id_list (struct parser *parser)
{
    node *head = NULL;
    node *tail = NULL;

    while (is_id (parser)) {
        struct token *tok = parser_get_token (parser);
        struct location loc = token_location (tok);

        if (!head && !tail) {
            tail
              = loc_annotated (loc, TBmakeSpids (strdup (token_as_string (tok)), NULL));
            head = tail;
        } else {
            node *t
              = loc_annotated (loc, TBmakeSpids (strdup (token_as_string (tok)), NULL));
            SPIDS_NEXT (tail) = t;
            tail = t;
        }

        tok = parser_get_token (parser);
        if (!token_is_operator (tok, tv_comma))
            parser_unget (parser);
    }

    return head;
}

/* vardecl-list ::= ( type id-list ';')*  */
node *
handle_vardecl_list (struct parser *parser)
{
    node *ret = NULL;
    bool parse_error = false;

    while (true) {
        if (is_type (parser)) {
            ntype *type;
            node *ids = error_mark_node;

            type = handle_type (parser);

            /* FIXME Here we need to check that at least one variable
               exists after the type.  If not, check if the symbol is
               `=' which can be an assignment where the variable is named
               the same way as a type.  */
            if (type != NULL && type != error_type_node
                && (error_mark_node != (ids = handle_var_id_list (parser))
                    && NULL != ids)) {
                if (parser_expect_tval (parser, tv_semicolon))
                    parser_get_token (parser);
                else {
                    parse_error = true;
                    continue;
                }

                /* This code is taken from sac.y  */
                while (SPIDS_NEXT (ids) != NULL) {
                    node *avis;
                    node *ids_tmp;

                    avis = TBmakeAvis (strdup (SPIDS_NAME (ids)), TYcopyType (type));
                    ret = TBmakeVardec (avis, ret);
                    NODE_LOCATION (avis) = NODE_LOCATION (ret) = NODE_LOCATION (ids);
                    AVIS_DECLTYPE (VARDEC_AVIS (ret)) = TYcopyType (type);
                    ids_tmp = SPIDS_NEXT (ids);
                    free_node (ids);
                    ids = ids_tmp;
                }

                ret = TBmakeVardec (loc_annotated (NODE_LOCATION (ids),
                                                   TBmakeAvis (strdup (SPIDS_NAME (ids)),
                                                               type)),
                                    ret);
                NODE_LOCATION (ret) = NODE_LOCATION (ids);
                AVIS_DECLTYPE (VARDEC_AVIS (ret)) = TYcopyType (type);
                free_node (ids);
                continue;
            } else
                parse_error = true;

            /* In case there was an error in type or identifiers.  */
            parser_get_until_tval (parser, tv_semicolon);
            free_type (type);
            free_node (ids);
            continue;
        } else
            break;
    }

    if (parse_error) {
        free_node (ret);
        return error_mark_node;
    } else
        return ret;
}

/* return ::= 'return' expr ';'
              | 'return' '(' expr-list ')' ';'
              | 'return' ';'
*/
node *
handle_return (struct parser *parser)
{
    struct token *tok;
    struct location loc;

    tok = parser_get_token (parser);
    loc = token_location (tok);

    if (token_is_keyword (tok, RETURN)) {
        node *exprs;

        tok = parser_get_token (parser);
        /* return ';'  */
        if (token_is_operator (tok, tv_semicolon))
            return loc_annotated (loc,
                                  TBmakeAssign (loc_annotated (loc, TBmakeReturn (NULL)),
                                                NULL));
        /* return '(' ')'  */
        else if (token_is_operator (tok, tv_lparen)) {
            tok = parser_get_token (parser);
            if (token_is_operator (tok, tv_rparen)) {
                if (!parser_expect_tval (parser, tv_semicolon))
                    return error_mark_node;

                /* eat-up ';'  */
                parser_get_token (parser);
                return loc_annotated (loc,
                                      TBmakeAssign (loc_annotated (loc,
                                                                   TBmakeReturn (NULL)),
                                                    NULL));
            } else
                parser_unget2 (parser);
        } else
            parser_unget (parser);

        parser->in_return = true;
        exprs = handle_expr (parser);
        parser->in_return = false;

        if (exprs == NULL || exprs == error_mark_node) {
            parser_get_until_tval (parser, tv_semicolon);
            return error_mark_node;
        }

        if (NODE_TYPE (exprs) != N_exprs)
            exprs = expr_constructor (exprs, NULL);

        if (!parser_expect_tval (parser, tv_semicolon))
            return error_mark_node;

        /* eat ';'  */
        parser_get_token (parser);
        return loc_annotated (loc,
                              TBmakeAssign (loc_annotated (loc, TBmakeReturn (exprs)),
                                            NULL));
    } else {
        parser_unget (parser);
        /* NOTE: we are not setting a location here.  */
        return TBmakeAssign (TBmakeReturn (NULL), NULL);
    }
}

/* stmt-list ::= ';' | '{' vardec-list? stmt-list? return-stmt? '}' | stmt  */
node *
handle_stmt_list (struct parser *parser, unsigned flags)
{
    struct token *tok;
    struct location loc;
    node *ret = error_mark_node;
    node *vardecl = NULL;
    node *ret_stmt = error_mark_node;
    bool parse_error = false;

    tok = parser_get_token (parser);
    loc = token_location (tok);

    if (flags & STMT_BLOCK_SEMICOLON_F) {
        if (token_is_operator (tok, tv_semicolon))
            return loc_annotated (loc, MAKE_EMPTY_BLOCK ());
    }

    /* '{' starting a body of the block.  */
    if (token_is_operator (tok, tv_lbrace)) {
        /* FIXME pragma cachesim  */
        tok = parser_get_token (parser);

        if (token_is_operator (tok, tv_rbrace))
            return loc_annotated (loc, MAKE_EMPTY_BLOCK ());
        else
            parser_unget (parser);

        if (flags & STMT_BLOCK_VAR_DECLS_F) {
            vardecl = handle_vardecl_list (parser);
            if (vardecl == error_mark_node)
                parse_error = true;
        }

        if (error_mark_node == (ret = handle_list_of_stmts (parser))) {
            parser_get_until_tval (parser, tv_rbrace);
            goto error;
        }

        if (flags & STMT_BLOCK_RETURN_F) {
            ret_stmt = handle_return (parser);
            if (ret_stmt == error_mark_node) {
                parser_get_until_tval (parser, tv_rbrace);
                parse_error = true;
            }
        }

        if (parser_expect_tval (parser, tv_rbrace))
            parser_get_token (parser);
        else
            goto error;

        if (parse_error)
            goto error;

        /* Add return statement to the assign list if needed.  */
        if (flags & STMT_BLOCK_RETURN_F)
            ret = TCappendAssign (ret, ret_stmt);

        ret = loc_annotated (loc, TBmakeBlock (ret, NULL));
        if ((flags & STMT_BLOCK_RETURN_F) && !parse_error) {
            BLOCK_VARDECS (ret) = vardecl;
        }
        return ret;
    } else if (flags & STMT_BLOCK_SINGLE_STMT_F) {
        const char *tval = token_as_string (tok);
        struct location loc = token_location (tok);

        parser_unget (parser);
        /* XXX it is unlikey that we may want to allow returns as
           a single statement.  So we do not check this option here.  */

        if (error_mark_node == (ret = handle_stmt (parser)))
            goto error;
        if (ret == NULL) {
            error_loc (loc, "statement expected, `%s' found", tval);
            goto error;
        }

        return loc_annotated (loc, TBmakeBlock (ret, NULL));
    }

error:
    free_node (ret);
    return error_mark_node;
}

/* Read a comma-separated list of instances. To read a single
   instance the function HANDLE must be passed. To construct
   a node-list the function CONSTRUCTOR is used.  */
static inline node *
handle_generic_list (struct parser *parser, node *(*handle) (struct parser *),
                     node *(*constructor) (node *, node *))
{
    // Ensure that we avoid parsing `(` expes `)` within the list.
    bool old_ret_state = parser->in_return;
    parser->in_return = false;
    node *res = handle_generic_list_internal (parser, handle, constructor);
    parser->in_return = old_ret_state;
    return res;
}

static inline node *
handle_generic_list_internal (struct parser *parser, node *(*handle) (struct parser *),
                     node *(*constructor) (node *, node *))
{
    struct token *tok;
    node *res = handle (parser);
    node *t;

    if (res == NULL || res == error_mark_node)
        return res;

    tok = parser_get_token (parser);
    if (!token_is_operator (tok, tv_comma)) {
        parser_unget (parser);
        return constructor (res, NULL);
    }

    t = handle_generic_list_internal (parser, handle, constructor);
    if (t == NULL || t == error_mark_node) {
        error_loc (token_location (tok), "nothing follows the comma");
        return error_mark_node;
    }

    return constructor (res, t);
}

static node *
handle_rettype_list (struct parser *parser)
{
    struct token *tok;
    ntype *res = handle_type (parser);
    node *t;

    if (res == NULL)
        return NULL;
    if (res == error_type_node)
        return error_mark_node;

    tok = parser_get_token (parser);
    if (!token_is_operator (tok, tv_comma)) {
        parser_unget (parser);
        return TBmakeRet (res, NULL);
    }

    /* Check if the next token is '...', in whcih case
       we have reached the end of the list.  If we don't have
       this code, then the next recursive invocation will emit
       an error.  */
    tok = parser_get_token (parser);
    if (token_is_operator (tok, tv_threedots)) {
        parser_unget2 (parser);
        return TBmakeRet (res, NULL);
    } else
        parser_unget (parser);

    t = handle_rettype_list (parser);
    if (t == NULL || t == error_mark_node) {
        error_loc (token_location (tok), "valid type expected after comma");
        return error_mark_node;
    }

    return TBmakeRet (res, t);
}

/* rettypes ::= type-list | type-list ',' '...' | '...'  */
node *
handle_rettypes (struct parser *parser, bool vaargs, bool *three_dots_p)
{
    struct token *tok;
    node *ret;

    *three_dots_p = false;

    tok = parser_get_token (parser);
    parser_unget (parser);

    if (token_is_keyword (tok, TYPE_VOID)) {
        parser_get_token (parser);
        return NULL;
    }
    /* XXX decide whether we support just three dots without the first argument.
       This used to be the case in the sac bison definition, but it contradicts
       with the syntax specification.  */
    else if (token_is_operator (tok, tv_threedots)) {
        parser_get_token (parser);
        if (!vaargs) {
            error_loc (token_location (tok), "... type is only allowed "
                                             "for function declarations");
            return error_mark_node;
        } else {
            *three_dots_p = true;
            return NULL;
        }
    }

    if (error_mark_node == (ret = handle_rettype_list (parser)))
        return ret;

    if (ret != NULL && vaargs) {
        tok = parser_get_token (parser);
        if (token_is_operator (tok, tv_comma)) {
            tok = parser_get_token (parser);
            if (token_is_operator (tok, tv_threedots))
                *three_dots_p = true;
            else
                parser_unget (parser);
        } else
            parser_unget (parser);
    }

    if (ret == NULL && !*three_dots_p)
        return error_mark_node;

    return ret;
}

/* argument ::= type id  */
node *
handle_argument (struct parser *parser)
{
    ntype *type = error_type_node;
    node *ret = error_mark_node;
    struct identifier *id = NULL;
    struct token *tok;
    node *var;
    bool ref = false;

    struct location type_loc;
    struct location var_loc;

    type_loc = token_location (parser_get_token (parser));
    parser_unget (parser);

    type = handle_type (parser);

    if (type == NULL || type == error_type_node)
        goto error;

    tok = parser_get_token (parser);
    if (token_is_operator (tok, tv_and))
        ref = true;
    else
        parser_unget (parser);

    var_loc = token_location (parser_get_token (parser));
    parser_unget (parser);
    if (NULL != (id = is_ext_id (parser))) {
        if (id->is_operation) {
            error_loc (var_loc, "%s is not a valid function argument name", id->id);
            free (id);
            goto error;
        } else if (id->xnamespace != NULL) {
            error_loc (var_loc, "function argument cannot have a namespace");
            free (id);
            goto error;
        }
    } else {
        struct token *tok = parser_get_token (parser);
        parser_unget (parser);
        error_loc (token_location (tok),
                   "token %s cannot start a function "
                   "argument name",
                   token_as_string (tok));
        goto error;
    }

    /* FIXME this is not relevant anymore, but let's keep it in the code for
       a while if we would want to rethink this in the future.  */
    /*if (is_unary (parser, NULL, token_as_string (tok)))
      {
        error_loc (token_location (tok), "argument `%s' is called the "
                   "same as the unary function", token_as_string (tok));
        goto error;
      }*/

    DBUG_ASSERT (id, "id cannot be NULL here");

    var = loc_annotated (type_loc, TBmakeAvis (strdup (id->id), type));
    /* consume the identifier  */
    parser_get_token (parser);
    free (id);
    ret = loc_annotated (var_loc, TBmakeArg (var, NULL));
    AVIS_DECLTYPE (ARG_AVIS (ret)) = TYcopyType (type);
    ARG_ISREFERENCE (ret) = ref;

    return ret;

error:
    free_type (type);
    free_node (ret);
    parser_get_until_oneof_tvals (parser, 2, tv_comma, tv_rparen);
    parser_unget (parser);
    return error_mark_node;
}

/* arguments ::= argument-list | argument-list ',' '...' | '...'   */
node *
handle_arguments (struct parser *parser, bool vaargs, bool *three_dots)
{
    struct token *tok;
    node *ret = NULL;
    node *tmp;
    bool parse_error = false;

    *three_dots = false;

/* If the next token is '...' check if it is allowed to have it here
   and return RET or error_mark_node.  Otherwise do nothing.  */
#define CHECK_THREE_DOTS(parser, tok)                                                    \
    do {                                                                                 \
        tok = parser_get_token (parser);                                                 \
        if (token_is_operator (tok, tv_threedots)) {                                     \
            if (!vaargs) {                                                               \
                error_loc (token_location (tok), "`%s' is not allowed here",             \
                           token_as_string (tok));                                       \
                goto error;                                                              \
            } else {                                                                     \
                *three_dots = true;                                                      \
                if (!parse_error)                                                        \
                    return ret;                                                          \
                else                                                                     \
                    goto error;                                                          \
            }                                                                            \
        } else                                                                           \
            parser_unget (parser);                                                       \
    } while (0)

    CHECK_THREE_DOTS (parser, tok);

    tok = parser_get_token (parser);
    parser_unget (parser);

    if (token_is_operator (tok, tv_rparen))
        return NULL;

    if (error_mark_node == (ret = handle_argument (parser)))
        parse_error = true;

    tmp = ret;
    while (true) {
        tok = parser_get_token (parser);
        if (token_is_operator (tok, tv_comma)) {
            node *arg;
            CHECK_THREE_DOTS (parser, tok);

            if (error_mark_node == (arg = handle_argument (parser)))
                parse_error = true;

            if (!parse_error) {
                ARG_NEXT (tmp) = arg;
                tmp = ARG_NEXT (tmp);
            }
        } else {
            parser_unget (parser);
            break;
        }
    }

    if (!parse_error)
        return ret;

error:
    free_node (ret);
    return error_mark_node;
}

/* pragmas ::= ( pragma )*  */
node *
handle_pragmas (struct parser *parser, enum pragma_type ptype)
{
#define CHECK_PRAGMA(pname, cond_allow, cond_exists)                                     \
    do {                                                                                 \
        if (cond_allow) {                                                                \
            parse_error = true;                                                          \
            error_loc (loc, "pragma `%s' is not allowed here",                           \
                       token_kind_as_string (pname));                                    \
            goto again;                                                                  \
        }                                                                                \
                                                                                         \
        if (cond_exists) {                                                               \
            parse_error = true;                                                          \
            error_loc (loc, "conflicting definitions of pragma `%s'",                    \
                       token_kind_as_string (pname));                                    \
            goto again;                                                                  \
        }                                                                                \
    } while (0)

    struct token *tok;
    const char *stok;
    node *pragmas = NULL;
    bool parse_error = false;

    tok = parser_get_token (parser);
    parser_unget (parser);

    if (!token_is_operator (tok, tv_hash))
        return NULL;

    pragmas = loc_annotated (token_location (tok), TBmakePragma ());

    while (true) {
        struct location loc;

    /* NOTE: we use label here, to make sure that CHECK_PRAGMA body
       can jump here.  We cannot use continue inside the macro, as
       it is wrapped in do { ... } while (0).  */
    again:
        tok = parser_get_token (parser);
        loc = token_location (tok);

        if (!token_is_operator (tok, tv_hash)) {
            parser_unget (parser);
            break;
        }

        if (parser_expect_tval (parser, PRAGMA))
            parser_get_token (parser);
        else
            goto error;

        tok = parser_get_token (parser);
        if (token_is_keyword (tok, LINKNAME)) {
            if (parser_expect_tclass (parser, tok_string))
                tok = parser_get_token (parser);
            else {
                parse_error = true;
                continue;
            }

            CHECK_PRAGMA (LINKNAME, ptype != pragma_fundef && ptype != pragma_fundec,
                          PRAGMA_LINKNAME (pragmas) != NULL);
            PRAGMA_LINKNAME (pragmas) = strdup (token_as_string (tok));
        } else if (token_is_keyword (tok, CUDALINKNAME)) {
            if (parser_expect_tclass (parser, tok_string))
                tok = parser_get_token (parser);
            else {
                parse_error = true;
                continue;
            }

            CHECK_PRAGMA (CUDALINKNAME, ptype != pragma_fundec,
                          PRAGMA_CUDALINKNAME (pragmas) != NULL);
            PRAGMA_CUDALINKNAME (pragmas) = strdup (token_as_string (tok));
        } else if (token_is_keyword (tok, LINKWITH)) {
            if (parser_expect_tclass (parser, tok_string))
                tok = parser_get_token (parser);
            else {
                parse_error = true;
                continue;
            }

            CHECK_PRAGMA (LINKWITH, ptype != pragma_fundec && ptype != pragma_typedef,
                          false);
            PRAGMA_LINKMOD (pragmas) = STRSadd (strdup (token_as_string (tok)),
                                                STRS_extlib, PRAGMA_LINKMOD (pragmas));
        } else if (token_is_keyword (tok, LINKOBJ)) {
            if (parser_expect_tclass (parser, tok_string))
                tok = parser_get_token (parser);
            else {
                parse_error = true;
                continue;
            }

            CHECK_PRAGMA (LINKOBJ, ptype != pragma_fundec && ptype != pragma_typedef,
                          false);
            PRAGMA_LINKOBJ (pragmas) = STRSadd (strdup (token_as_string (tok)),
                                                STRS_objfile, PRAGMA_LINKOBJ (pragmas));
        } else if (token_is_keyword (tok, LINKSIGN)) {
            node *nums;

            if (parser_expect_tval (parser, tv_lsquare))
                parser_get_token (parser);
            else {
                parse_error = true;
                continue;
            }

            if (error_mark_node == (nums = handle_num_list (parser))) {
                parser_get_until_tval (parser, tv_rsquare);
                parse_error = true;
                continue;
            }

            if (parser_expect_tval (parser, tv_rsquare))
                parser_get_token (parser);
            else {
                free_node (nums);
                parse_error = true;
                continue;
            }

            CHECK_PRAGMA (LINKSIGN, ptype != pragma_fundec,
                          PRAGMA_LINKSIGN (pragmas) != NULL);
            PRAGMA_LINKSIGN (pragmas) = nums;
        } else if (token_is_keyword (tok, REFCOUNTING)) {
            node *nums;

            if (parser_expect_tval (parser, tv_lsquare))
                parser_get_token (parser);
            else {
                parse_error = true;
                continue;
            }

            if (error_mark_node == (nums = handle_num_list (parser))) {
                parser_get_until_tval (parser, tv_rsquare);
                parse_error = true;
                continue;
            }

            if (parser_expect_tval (parser, tv_rsquare))
                parser_get_token (parser);
            else {
                free_node (nums);
                parse_error = true;
                continue;
            }

            CHECK_PRAGMA (REFCOUNTING, ptype != pragma_fundec,
                          PRAGMA_REFCOUNTING (pragmas) != NULL);
            PRAGMA_REFCOUNTING (pragmas) = nums;
        } else if (token_is_keyword (tok, REFCOUNTDOTS)) {
            CHECK_PRAGMA (REFCOUNTDOTS, ptype != pragma_fundec, false);
            PRAGMA_REFCOUNTDOTS (pragmas) = true;
        } else if (token_is_keyword (tok, EFFECT)) {
            node *ids;

            /*  ids here must be presented as exprs.  */
            if (error_mark_node == (ids = handle_expr_list (parser))) {
                parse_error = true;
                continue;
            }

            CHECK_PRAGMA (EFFECT, ptype != pragma_fundec,
                          PRAGMA_EFFECT (pragmas) != NULL);
            PRAGMA_EFFECT (pragmas) = ids;
        } else if (token_is_keyword (tok, MUTCTHREADFUN)) {
            if (global.backend != BE_mutc) {
                error_loc (loc,
                           "pragma `%s' is not supported "
                           "within selected backend",
                           token_kind_as_string (MUTCTHREADFUN));
                parse_error = true;
                continue;
            } else {
                CHECK_PRAGMA (MUTCTHREADFUN,
                              ptype != pragma_fundec && ptype != pragma_fundef, false);
                PRAGMA_MUTCTHREADFUN (pragmas) = true;
            }
        } else if (token_is_keyword (tok, NOINLINE)) {
            CHECK_PRAGMA (NOINLINE, ptype != pragma_fundec && ptype != pragma_fundef,
                          false);
            PRAGMA_NOINLINE (pragmas) = true;
        } else if (token_is_keyword (tok, COPYFUN)) {
            if (parser_expect_tclass (parser, tok_string))
                tok = parser_get_token (parser);
            else {
                parse_error = true;
                continue;
            }

            CHECK_PRAGMA (COPYFUN, ptype != pragma_typedef,
                          PRAGMA_COPYFUN (pragmas) != NULL);
            PRAGMA_COPYFUN (pragmas) = strdup (token_as_string (tok));
        } else if (token_is_keyword (tok, FREEFUN)) {
            if (parser_expect_tclass (parser, tok_string))
                tok = parser_get_token (parser);
            else {
                parse_error = true;
                continue;
            }

            CHECK_PRAGMA (FREEFUN, ptype != pragma_typedef,
                          PRAGMA_FREEFUN (pragmas) != NULL);
            PRAGMA_FREEFUN (pragmas) = strdup (token_as_string (tok));
        } else if (token_is_keyword (tok, HEADER)) {
            if (parser_expect_tclass (parser, tok_string))
                tok = parser_get_token (parser);
            else {
                parse_error = true;
                continue;
            }

            CHECK_PRAGMA (HEADER, ptype != pragma_fundec && ptype != pragma_fundef,
                          false);

            stok = token_as_string (tok);
            if (stok[0] == '<') {
               PRAGMA_HEADER (pragmas) = STRSadd (strdup (stok),
                                                  STRS_headers, PRAGMA_HEADER (pragmas));
            } else {
               PRAGMA_HEADER (pragmas) = STRSadd (STRcatn (3, "\"", stok, "\""),
                                                  STRS_headers, PRAGMA_HEADER (pragmas));
            }
        } else {
            error_loc (loc, "undefined pragma `%s' found", token_as_string (tok));
            goto error;
        }
    }

    if (!parse_error)
        return pragmas;

error:
    free_node (pragmas);
    return error_mark_node;
}

/* Handle function or function declaration.
   Function declaration::
      fundec::
          ('extern' | 'specialize')? type_list
          id '(' fdecarglist ')' ';' (pragma_list)?
      fdecarglist::
          arg_list (',' '...')? | '...'
      arg_list::
          empty | arg (',' arg)*
      arg::
          type id

   Function definition:
      fundef::
          'noinline'? 'inline'? 'thread'? type_list
          (id | ( '(' id ')' )) '(' arg_list ')'
          expr_block
*/
node *
handle_function (struct parser *parser, enum parsed_ftype *ftype)
{
    struct token *tok;
    struct location loc;

    bool fundec_p = false;
    bool fundef_p = false;
    bool extern_p = false;
    bool specialize_p = false;
    bool inline_p = false;
    bool noinline_p = false;
    bool thread_p = false;
    bool attribute_error = false;
    bool parse_error = false;
    bool ret_three_dots = false;
    bool arg_three_dots = false;
    bool allows_infix = false;
    bool is_main = false;

    node *ret_types = error_mark_node;
    node *fname = error_mark_node;
    node *args = error_mark_node;
    node *body = NULL;
    node *ret = error_mark_node;
    node *pragmas = NULL;

    tok = parser_get_token (parser);
    loc = token_location (tok);
    *ftype = fun_fundef;

#define FUNDEF_ERROR(tok)                                                                \
    error_loc (token_location (tok), "attribute %s contradicts with %s",                 \
               token_as_string (tok),                                                    \
               inline_p ? token_kind_as_string (INLINE)                                  \
                        : thread_p ? token_kind_as_string (THREAD)                       \
                                   : noinline_p ? token_kind_as_string (NOINLINE)        \
                                                : "something")
#define FUNDEC_ERROR(tok)                                                                \
    error_loc (token_location (tok), "attribute %s contradicts with %s",                 \
               token_as_string (tok),                                                    \
               extern_p                                                                  \
                 ? token_kind_as_string (EXTERN)                                         \
                 : specialize_p ? token_kind_as_string (SPECIALIZE) : "something")

    /* Read all the attributes of the function and remember them.  */
    while (true) {
        if (token_is_keyword (tok, EXTERN)) {
            if (fundef_p) {
                attribute_error = true;
                FUNDEF_ERROR (tok);
            } else {
                fundec_p = true; extern_p = true;
            }
        } else if (token_is_keyword (tok, SPECIALIZE)) {
            if (fundef_p) {
                attribute_error = true;
                FUNDEF_ERROR (tok);
            } else {
                fundec_p = true; specialize_p = true;
            }
        } else if (token_is_keyword (tok, INLINE)) {
            if (fundec_p) {
                attribute_error = true;
                FUNDEC_ERROR (tok);
            } else {
                fundef_p = true; inline_p = true;
            }
        } else if (token_is_keyword (tok, NOINLINE)) {
            if (fundec_p) {
                attribute_error = true;
                FUNDEC_ERROR (tok);
            } else {
                fundef_p = true; noinline_p = true;
            }
        } else if (token_is_keyword (tok, THREAD)) {
            if (global.backend != BE_mutc) {
                warning_loc (token_location (tok),
                             "attribute %s is not "
                             "supported with selected backend and will "
                             "be ignored",
                             token_kind_as_string (THREAD));
            } else if (fundec_p) {
                FUNDEC_ERROR (tok);
                attribute_error = true;
            } else {
                fundef_p = true; inline_p = true;
            }
        } else {
            parser_unget (parser);
            break;
        }

        tok = parser_get_token (parser);
    }

    if (inline_p && noinline_p)
        error_loc (loc, "function cannot both `inline' and `noinline' attributes");

    /* Handle return types.  */
    if (attribute_error || (!fundef_p && !fundec_p) || fundec_p)
        ret_types = handle_rettypes (parser, true, &ret_three_dots);
    else
        ret_types = handle_rettypes (parser, false, &ret_three_dots);

    if (ret_types == error_mark_node) {
        parse_error = true;
        parser_get_until_tval (parser, tv_rparen);
        /* check that we didn't hit the ')' of function name  */
        tok = parser_get_token (parser);
        if (token_is_operator (tok, tv_rparen))
            parser_get_until_tval (parser, tv_rparen);
        else
            parser_unget (parser);

        goto semi_or_exprs;
    }

    /* Handle name of the function.  */

    /* Make sure that lexer is going to physically start lexing from the
       beginning of the function name.  */
    if (parser->unget_idx != 0)
        parser_unlex_token_buffer (parser);

    assert (parser->unget_idx == 0, "lexer should point to the function "
                                    "name in order to lex it correctly");

    parser->lex->is_read_user_op = true;
    tok = parser_get_token (parser);
    if (token_is_operator (tok, tv_lparen)) {
        allows_infix = true;
        tok = parser_get_token (parser);

        warning_loc (token_location (tok),
                     "using deprecated operator definition "
                     "syntax, please remove `(' and `)' characters");

        parser->lex->is_read_user_op = false;

        if (token_is_reserved (tok) || token_class (tok) == tok_user_op)
            fname = loc_annotated (token_location (tok),
                                   TBmakeSpid (NULL, strdup (token_as_string (tok))));
        else {
            error_loc (token_location (tok), "invalid function name `%s'",
                       token_as_string (tok));
            parse_error = true;
        }

        if (parser_expect_tval (parser, tv_rparen))
            parser_get_token (parser);
        else
            parser_get_until_tval (parser, tv_lparen);
    } else if (token_is_keyword (tok, K_MAIN)) {
        if (parser->in_module)
            error_loc (token_location (tok), "main function cannot be "
                                             "defined inside the module");
        if (inline_p)
            warning_loc (token_location (tok), "making main function inline "
                                               "doesn't have any effect");
        if (noinline_p)
            warning_loc (token_location (tok), "making main function noinline "
                                               "doesn't have any effect");

        fname = loc_annotated (token_location (tok),
                               TBmakeSpid (NULL, strdup (token_as_string (tok))));

        is_main = true;
    } else {
        if (token_is_reserved (tok) || token_class (tok) == tok_user_op)
            fname = loc_annotated (token_location (tok),
                                   TBmakeSpid (NULL, strdup (token_as_string (tok))));
        else {
            error_loc (token_location (tok), "invalid function name `%s'",
                       token_as_string (tok));
            parse_error = true;
        }
    }

    parser->lex->is_read_user_op = false;

    if (fname == NULL || fname == error_mark_node) {
        parse_error = true;
        tok = parser_get_token (parser);
        if (token_is_operator (tok, tv_lparen)) {
            parser_get_until_tval (parser, tv_rparen);
            goto semi_or_exprs;
        } else {
            parser_unget (parser);
            goto semi_or_exprs;
        }
    }

    /* Handle arguments.  */
    if (parser_expect_tval (parser, tv_lparen))
        parser_get_token (parser);
    else {
        /* Assume we can get to the ')' of arguments.  */
        parse_error = true;
        parser_get_until_tval (parser, tv_rparen);
        goto semi_or_exprs;
    }

    /* Handle arguments.  */
    if (attribute_error || (!fundef_p && !fundec_p) || fundec_p)
        args = handle_arguments (parser, true, &arg_three_dots);
    else
        args = handle_arguments (parser, false, &arg_three_dots);

    if (args == error_mark_node) {
        parse_error = true;
        parser_get_until_tval (parser, tv_rparen);
    } else if (!parser_expect_tval (parser, tv_rparen)) {
        parse_error = true;
        tok
          = parser_get_until_oneof_tvals (parser, 3, tv_rparen, tv_lbrace, tv_semicolon);
        if (!token_is_operator (tok, tv_rparen))
            parser_unget (parser);
    } else
        parser_get_token (parser);

semi_or_exprs:
    tok = parser_get_token (parser);
    if (token_is_operator (tok, tv_semicolon)) {
        fundec_p = true;
        fundef_p = false;
        goto pragmas;
    } else if (token_is_operator (tok, tv_lbrace)) {
        parser_unget (parser);
        fundef_p = true;
        fundec_p = false;
        body = handle_stmt_list (parser, STMT_BLOCK_FUNCTION_FLAGS);
        if (body == NULL || body == error_mark_node) {
            parse_error = true;
            goto pragmas;
        }
    } else {
        parser_unget (parser);
        parse_error = true;
        error_loc (token_location (tok), "function body or semicolon expected");
    }

pragmas:
    if (fundef_p)
        pragmas = handle_pragmas (parser, pragma_fundef);
    else if (fundec_p)
        pragmas = handle_pragmas (parser, pragma_fundec);

    if (pragmas == error_mark_node)
        parse_error = true;

    if (parse_error || attribute_error) {
        free_node (ret_types);
        free_node (fname);
        free_node (args);
        free_node (body);
        free_node (ret);
        free_node (pragmas);
        *ftype = fun_error;
        return error_mark_node;
    }

    /* Add a symbol to the list of known_symbols of the parser
       in order to handle X::y, where X is the name of the module
       we are currently parsing.  */
    if (args != error_mark_node) {
        size_t argc = 0;
        struct known_symbol *ks;
        node *t = args;

        /* Number of argumets.  */
        while (t) {
            argc++; t = ARG_NEXT (t);
        }
        /* Add symbol to the known_types.  */
        HASH_FIND_STR (parser->known_symbols, SPID_NAME (fname), ks);
        if (!ks) {
            ks = (struct known_symbol *)malloc (sizeof (struct known_symbol));
            ks->name = strdup (SPID_NAME (fname));
            ks->flags = 0;
            HASH_ADD_KEYPTR (hh, parser->known_symbols, ks->name, strlen (ks->name), ks);
        }

        /* Set update flags if it is unary/binary.  */
        if (argc == 1)
            symbol_set_unary (ks);
        if (argc == 2)
            symbol_set_binary (ks);
    }

    ret = loc_annotated (loc, TBmakeFundef (NULL, NULL, NULL, NULL, NULL, NULL));
    FUNDEF_BODY (ret) = body;
    FUNDEF_ARGS (ret) = args;
    FUNDEF_RETS (ret) = ret_types;
    FUNDEF_NAME (ret) = SPID_NAME (fname);
    MEMfree (fname);
    FUNDEF_PRAGMA (ret) = pragmas;

    if (is_main)
        FUNDEF_ISMAIN (ret) = true;

    if (fundef_p) {
        /* FIXME ALLOWSINFIX is deprecated and is not used anywhere elese.  */
        FUNDEF_ALLOWSINFIX (ret) = allows_infix;
        FUNDEF_ISTHREADFUN (ret) = thread_p;
        FUNDEF_ISINLINE (ret) = inline_p;
        FUNDEF_NOINLINE (ret) = noinline_p;
    } else if (fundec_p) {
        FUNDEF_ISEXTERN (ret) = true;
        FUNDEF_HASDOTRETS (ret) = ret_three_dots;
        FUNDEF_HASDOTARGS (ret) = arg_three_dots;
        if (specialize_p)
            *ftype = fun_specialize_fundec;
        else if (extern_p)
            *ftype = fun_extern_fundec;
    }

    return ret;
}

void
cache_module (struct parser *parser, const char *modname)
{
    struct used_module *used_module;
    module_t *module;
    sttable_t *table;
    stsymboliterator_t *iterator;

    /* Do not cache the module in case it is the
       module we are parsing right now.  */
    if (parser->in_module && !strcmp (parser->current_module, modname))
        return;

    HASH_FIND_STR (parser->used_modules, modname, used_module);
    if (used_module)
        return;

    /* XXX Stupid module-manager will die in case the module
       is not found, which means that:
         1)  The rest of the program would not be parsed
             (which may be fine, as potentially we have
              a problem with user-defined symbols).
         2)  The parser-internals would not be freed.  */
    if (!MODMmoduleExists (modname)) {
        struct location loc = token_location (parser_get_token (parser));
        parser_unget (parser);
        error_loc (loc, "cannot load module `%s'", modname);
        return;
    }
    module = MODMloadModule (modname);
    table = STcopy (MODMgetSymbolTable (module));
    iterator = STsymbolIteratorGet (table);

    used_module = (struct used_module *)malloc (sizeof (struct used_module));
    used_module->name = strdup (modname);
    used_module->user_ops = trie_new ();
    used_module->symbols = NULL;

    /* Iterate throught the symol table of MODULE_NAME
       and import the types that we meet first time.  */
    while (STsymbolIteratorHasMore (iterator)) {
        bool arity1 = STsymbolIteratorSymbolArityIs (iterator, 1);
        bool arity2 = STsymbolIteratorSymbolArityIs (iterator, 2);
        bool istype;
        const char *symbol_name;
        struct known_symbol *ks;

        stsymbol_t *symb = STsymbolIteratorNext (iterator);
        istype = STsymbolGetEntryType (symb) == SET_typedef;
        symbol_name = STsymbolName (symb);

        if (!is_normal_id (symbol_name)
            && trie_search (parser->lex->trie, symbol_name, strlen (symbol_name))
                 == TRIE_NOT_LAST)
            trie_add_word (used_module->user_ops, symbol_name, strlen (symbol_name),
                           TRIE_USEROP);

        HASH_FIND_STR (used_module->symbols, symbol_name, ks);
        if (ks) {
            warning ("module `%s' imports symbol `%s' twice", modname, symbol_name);
            if (arity1)
                symbol_set_unary (ks);
            if (arity2)
                symbol_set_binary (ks);
            if (istype)
                symbol_set_type (ks);
        } else {
            ks = (struct known_symbol *)malloc (sizeof (struct known_symbol));
            ks->name = strdup (symbol_name);
            ks->flags = 0;
            if (arity1)
                symbol_set_unary (ks);
            if (arity2)
                symbol_set_binary (ks);
            if (istype)
                symbol_set_type (ks);
            HASH_ADD_KEYPTR (hh, used_module->symbols, ks->name, strlen (ks->name), ks);
        }
    }

    HASH_ADD_KEYPTR (hh, parser->used_modules, used_module->name,
                     strlen (used_module->name), used_module);

    iterator = STsymbolIteratorRelease (iterator);
    table = STdestroy (table);
    module = MODMunLoadModule (module);
}

/* symbols-set ::= '{' symbol-list '}'  */
node *
__handle_symbol_set (struct parser *parser, const char *modname, bool except)
{
    node *ret = NULL;

    if (parser_expect_tval (parser, tv_lbrace))
        parser_get_token (parser);
    else {
        parser_get_until_tval (parser, tv_rbrace);
        return error_mark_node;
    }

    ret = handle_symbol_list (parser, modname, except);
    if (ret == error_mark_node) {
        parser_get_until_tval (parser, tv_rbrace);
        return error_mark_node;
    }

    if (parser_expect_tval (parser, tv_rbrace))
        parser_get_token (parser);
    else
        goto error;

    return ret;

error:
    free_node (ret);
    return error_mark_node;
}

/* This is a wrapper to read used/imported symbol set.  A wrapper sets a
   correct lexer definition in order to parse the symbols correctly.  */
node *
handle_use_symbol_set (struct parser *parser, const char *modname, bool except)
{
    return __handle_symbol_set (parser, modname, except);
}

/* This is a wrapper to read a list of imported symbols with potentially
   new user-defnied-symbols which have to be lexed with lex->read_user_ops
   set to true.  */
node *
handle_symbol_set (struct parser *parser)
{
    return __handle_symbol_set (parser, NULL, false);
}

static int
update_all_known_symbols (struct parser *parser, const char *modname)
{
    struct used_module *mod;
    struct known_symbol *symb;
    struct known_symbol *t;

    HASH_FIND_STR (parser->used_modules, modname, mod);
    if (!mod)
    {
        error("module `%s' not cached", modname);
        return -1;
    }

    HASH_ITER (hh, mod->symbols, symb, t) {
        if (symbol_is_binary (symb) || symbol_is_unary (symb) || symbol_is_type (symb)) {
            struct known_symbol *nsymb;
            HASH_FIND_STR (parser->known_symbols, symb->name, nsymb);
            if (!nsymb) {
                nsymb = (struct known_symbol *)malloc (sizeof (struct known_symbol));
                nsymb->name = strdup (symb->name);
                nsymb->flags = symb->flags;
                HASH_ADD_KEYPTR (hh, parser->known_symbols, nsymb->name,
                                 strlen (nsymb->name), nsymb);
            } else
                nsymb->flags |= symb->flags;
        }

        if (!is_normal_id (symb->name)
            && trie_search (parser->lex->trie, symb->name, strlen (symb->name))
                 == TRIE_NOT_LAST)
            trie_add_word (parser->lex->trie, symb->name, strlen (symb->name),
                           TRIE_USEROP);
    }

    return 0;
}

/* 'import' id : ( ( 'all' ( 'except'? symbol-list )? ) | symbol-list ) ';'
   |
   'use' id : ( ( 'all' ( 'except'? symbol-list )? ) | symbol-list ) ';'
   |
   'provide' ( ( 'all' ( 'except'? symbol-list )? ) | symbol-list ) ';'
   |
   'export' ( ( 'all' ( 'except'? symbol-list )? ) | symbol-list ) ';'
*/
node *
handle_interface (struct parser *parser, enum interface_kind interface)
{
    struct token *tok;
    char *modname = NULL;
    bool all_p = false;
    bool except_p = false;
    node *symbols = NULL;
    node *ret = error_mark_node;
    struct location loc;

    enum token_kind tkind;

    if (interface == int_import)
        tkind = IMPORT;
    else if (interface == int_use)
        tkind = USE;
    else if (interface == int_export)
        tkind = EXPORT;
    else if (interface == int_provide)
        tkind = PROVIDE;
    else
        unreachable ("unknown interface kind passed");

    if (parser_expect_tval (parser, tkind)) {
        /* eat TKIND and set its location to LOC.  */
        tok = parser_get_token (parser);
        loc = token_location (tok);
    } else
        goto skip_error;

    if (interface == int_import || interface == int_use) {
        tok = parser_get_token (parser);
        if (token_class (tok) != tok_id) {
            error_loc (token_location (tok),
                       "expected module name for use/import, "
                       "`%s' found instead", token_as_string (tok));
            parser_unget (parser);
            goto skip_error;
        }

        modname = strdup (token_as_string (tok));

        /* Do not allow to import or use self.  */
        if (parser->in_module && !strcmp (parser->current_module, modname)) {
            error_loc (token_location (tok), "attempt to use/import the "
                                             "module which is being currently parsed");
            goto skip_error;
        }

        /* cache module symbol-table and trie in parser->used_modules.  */
        cache_module (parser, modname);

        if (parser_expect_tval (parser, tv_colon))
            parser_get_token (parser);
        else
            goto skip_error;
    }

    tok = parser_get_token (parser);
    if (token_is_keyword (tok, ALL)) {
        all_p = true;
        tok = parser_get_token (parser);
        if (token_is_keyword (tok, EXCEPT)) {
            except_p = true;

            /* If we use/import all except {set}, we must add exactly
               the {set} to parser->known_symbols to support
               infix/prefix/postfix correctly.  */
            if (interface == int_import || interface == int_use)
                symbols = handle_use_symbol_set (parser, modname, true);
            else
                symbols = handle_symbol_set (parser);
        } else {
            /* If we use/import all, then parser->symbols must
               be updated with all the symbols from module.  */
            if (interface == int_import || interface == int_use) {
                if (update_all_known_symbols (parser, modname) < 0)
                    goto skip_error;
            }
            parser_unget (parser);
        }
    } else {
        parser_unget (parser);
        /* If we use/import {set}, we add this set
           to the parser->known_symbols.  */
        if (interface == int_import || interface == int_use)
            symbols = handle_use_symbol_set (parser, modname, false);
        else
            symbols = handle_symbol_set (parser);
    }

    if (symbols == error_mark_node)
        goto skip_error;

    if (parser_expect_tval (parser, tv_semicolon))
        parser_get_token (parser);
    else
        goto skip_error;

    if (all_p) {
        if (except_p) {
            if (interface == int_import)
                ret = TBmakeImport (modname, NULL, symbols);
            else if (interface == int_use)
                ret = TBmakeUse (modname, NULL, symbols);
            else if (interface == int_export)
                ret = TBmakeExport (NULL, symbols);
            else if (interface == int_provide)
                ret = TBmakeProvide (NULL, symbols);
            else
                unreachable ("unknown interface specified");
        } else {
            if (interface == int_import)
                ret = TBmakeImport (modname, NULL, NULL);
            else if (interface == int_use)
                ret = TBmakeUse (modname, NULL, NULL);
            else if (interface == int_export)
                ret = TBmakeExport (NULL, NULL);
            else if (interface == int_provide)
                ret = TBmakeProvide (NULL, NULL);
            else
                unreachable ("unknown interface specified");
        }

        if (interface == int_import)
            IMPORT_ALL (ret) = true;
        else if (interface == int_use)
            USE_ALL (ret) = true;
        else if (interface == int_export)
            EXPORT_ALL (ret) = true;
        else if (interface == int_provide)
            PROVIDE_ALL (ret) = true;
        else
            unreachable ("unknown interface specified");
    } else {
        if (interface == int_import)
            ret = TBmakeImport (modname, NULL, symbols);
        else if (interface == int_use)
            ret = TBmakeUse (modname, NULL, symbols);
        else if (interface == int_export)
            ret = TBmakeExport (NULL, symbols);
        else if (interface == int_provide)
            ret = TBmakeProvide (NULL, symbols);
        else
            unreachable ("unknown interface specified");
    }

    return loc_annotated (loc, ret);

skip_error:
    parser_get_until_tval (parser, tv_semicolon);
    if (modname)
        MEMfree (modname);
    free_node (symbols);
    free_node (ret);
    return error_mark_node;
}

/* 'extern' 'typedef' ( '[' id ']' )? id ';' ( pragmas )?     // External user-defined
   type
   |
   'typedef' ( '[' id ']' )? type id ';'                      // User-defined type
   |
   'builtin' 'typedef' ( '[' id ']' )?  id ( id-list )?       // Component with params
   |
   'typedef' ( '[' id ']' )? id ';'                           // Abstract type definition.
*/
node *
handle_typedef (struct parser *parser)
{
    struct token *tok;
    bool extern_p = false;
    bool nested = false;
    bool builtin_p = false;
    node *ret = error_mark_node;
    ntype *type = error_type_node;
    node *pragmas = NULL;
    node *args = NULL;
    char *name = NULL;
    char *component_name = NULL;
    struct known_symbol *ks;
    struct location loc;

    tok = parser_get_token (parser);
    if (token_is_keyword (tok, EXTERN)) {
        extern_p = true;
        if (parser_expect_tval (parser, TYPEDEF))
            /* eat TYPEDEF and save its location.  */
            loc = token_location (parser_get_token (parser));
        else {
            parser_unget (parser);
            goto skip_error;
        }
    } else if (token_is_keyword (tok, BUILTIN)) {
        builtin_p = true;
        if (parser_expect_tval (parser, TYPEDEF))
            /* eat TYPEDEF and save its location.  */
            loc = token_location (parser_get_token (parser));
        else {
            parser_unget (parser);
            goto skip_error;
        }
    } else if (token_is_keyword (tok, TYPEDEF))
        loc = token_location (tok);
    else if (token_is_keyword (tok, NESTED)) {
        loc = token_location (tok); nested = true;
    } else {
        error_loc (token_location (tok), "`%s' or `%s %s' expected, `%s' found",
                   token_kind_as_string (TYPEDEF), token_kind_as_string (EXTERN),
                   token_kind_as_string (TYPEDEF), token_as_string (tok));
        parser_unget (parser);
        goto skip_error;
    }

    /* Check if component name specified.  */
    tok = parser_get_token (parser);
    if (token_is_operator (tok, tv_lsquare)) {
        if (parser_expect_tclass (parser, tok_id))
            component_name = strdup (token_as_string (parser_get_token (parser)));
        else {
            parser_unget (parser);
            goto skip_error;
        }

        if (parser_expect_tval (parser, tv_rsquare))
            parser_get_token (parser);
        else {
            parser_unget (parser);
            goto skip_error;
        }
    } else {
        parser_unget (parser);
        component_name = strdup (global.default_component_name);
    }

    if (!extern_p && !builtin_p && is_type (parser)) {
        type = handle_type (parser);
        if (!TUshapeKnown (type)) {
            nested = true;
        }
        if (type == error_type_node)
            goto skip_error;
    }

    tok = parser_get_token (parser);
    if (token_class (tok) != tok_id) {
        parser_unget (parser);
        goto skip_error;
    } else
        name = strdup (token_as_string (tok));

    if (builtin_p) {
        tok = parser_get_token (parser);
        if (token_is_operator (tok, tv_lbrace)) {
            parser_get_token (parser);
            args = handle_typecomponent_list (parser);
            if (args == error_mark_node)
                goto skip_error;

            if (parser_expect_tval (parser, tv_rbrace))
                parser_get_token (parser);
            else
                goto skip_error;
        } else
            parser_unget (parser);
    }

    if (parser_expect_tval (parser, tv_semicolon))
        parser_get_token (parser);
    else
        goto skip_error;

    if (extern_p) {
        pragmas = handle_pragmas (parser, pragma_typedef);
        if (pragmas == error_mark_node)
            goto error;
    }

    /* If we have a typedef on default component, it should
       be recognized as a valid type in the program.  */
    if (!strcmp (component_name, global.default_component_name)) {
        HASH_FIND_STR (parser->known_symbols, name, ks);
        if (!ks) {
            ks = (struct known_symbol *)malloc (sizeof (struct known_symbol));
            ks->name = strdup (name);
            ks->flags = 0;
            symbol_set_type (ks);
            HASH_ADD_KEYPTR (hh, parser->known_symbols, ks->name, strlen (ks->name), ks);
        } else
            symbol_set_type (ks);
    }

    if (extern_p) {
        ntype *tt;

        tt = TYmakeAKS (TYmakeHiddenSimpleType (UT_NOT_DEFINED), SHmakeShape (0));
        ret = TBmakeTypedef (name, NULL, component_name, tt, NULL, NULL);
        if (nested == true) {
            TYPEDEF_ISNESTED (ret) = true;
        }
        TYPEDEF_ISEXTERNAL (ret) = true;
        TYPEDEF_PRAGMA (ret) = pragmas;
    } else {
        ret = TBmakeTypedef (name, NULL, component_name, type, NULL, NULL);
        if (nested == true) {
            TYPEDEF_ISNESTED (ret) = true;
        }

        if (builtin_p) {
            TYPEDEF_ARGS (ret) = args;
            TYPEDEF_ISBUILTIN (ret) = true;
        }

        if (type == NULL)
            TYPEDEF_ISABSTRACT (ret) = true;
    }

    return loc_annotated (loc, ret);

skip_error:
    parser_get_until_tval (parser, tv_semicolon);
error:
    if (name)
        free (name);
    if (component_name)
        free (component_name);

    free_type (type);
    free_tree (args);
    free_tree (pragmas);
    return error_mark_node;
}

node *
handle_struct_def (struct parser *parser)
{
    struct token *tok;
    struct identifier *id;
    bool parse_error = false;
    node *ret = NULL;
    node *ret_tail = NULL;
    struct location struct_loc;

    if (parser_expect_tval (parser, STRUCT))
        /* eat STRUCT and save its location.  */
        struct_loc = token_location (parser_get_token (parser));
    else
        goto error;

    id = is_id (parser);
    tok = parser_get_token (parser);

    if (!id) {
        error_loc (token_location (tok), "identifier expected, `%s' found",
                   token_as_string (tok));
        parser_unget (parser);
        goto error;
    }

    if (parser_expect_tval (parser, tv_lbrace))
        parser_get_token (parser);
    else
        goto error;

    while (true) {
        if (is_type (parser)) {
            ntype *type;
            node *ids = error_mark_node;
            struct location loc;

            type = handle_type (parser);

            loc = token_location (parser_get_token (parser));
            parser_unget (parser);

            if (type != NULL && type != error_type_node
                && (error_mark_node != (ids = handle_var_id_list (parser)))) {
                if (parser_expect_tval (parser, tv_semicolon))
                    parser_get_token (parser);
                else {
                    parse_error = true;
                    continue;
                }

                if (ids == NULL) {
                    error_loc (loc, "at least one identifier expected in "
                                    "structure definition.");
                    parse_error = true;
                    continue;
                }

                /* This code is taken from sac.y  */
                do {
                    node *se;
                    node *ids_tmp;

                    se = TBmakeStructelem (strdup (SPIDS_NAME (ids)), TYcopyType (type),
                                           NULL);
                    NODE_LOCATION (se) = NODE_LOCATION (ids);
                    if (ret == NULL) {
                        ret = se;
                        ret_tail = ret;
                    } else {
                        STRUCTELEM_NEXT (ret_tail) = se;
                        ret_tail = se;
                    }

                    ids_tmp = SPIDS_NEXT (ids);
                    free_node (ids);
                    ids = ids_tmp;
                } while (ids != NULL);
                continue;
            } else
                parse_error = true;

            /* In case there was an error in type or identifiers.  */
            parser_get_until_tval (parser, tv_semicolon);
            free_type (type);
            free_node (ids);
            continue;
        } else
            break;
    }

    if (parser_expect_tval (parser, tv_rbrace))
        parser_get_token (parser);
    else
        goto error;

    if (parser_expect_tval (parser, tv_semicolon))
        parser_get_token (parser);
    else
        goto error;

    if (parse_error)
        goto error;

    ret = TBmakeStructdef (id->id, ret, NULL);
    free (id);
    return loc_annotated (struct_loc, ret);

error:
    /* FIXME: try to skip until the ';'?  */
    free_node (ret);
    return error_mark_node;
}

/* 'extern' 'objdef' type ';'
   |
   'objdef' type '=' fun`ction-call ';'
*/
node *
handle_objdef (struct parser *parser)
{
    struct token *tok;
    ntype *type = error_type_node;
    node *expr = error_mark_node;
    char *name = NULL;
    bool extern_p = false;

    struct location objdef_loc;

    tok = parser_get_token (parser);
    if (token_is_keyword (tok, EXTERN)) {
        extern_p = true;
        if (parser_expect_tval (parser, OBJDEF))
            /* eat OBJDEF and save its location.  */
            objdef_loc = token_location (parser_get_token (parser));
        else {
            parser_unget (parser);
            goto skip_error;
        }
    } else if (token_is_keyword (tok, OBJDEF))
        objdef_loc = token_location (tok);
    else {
        error_loc (token_location (tok), "`%s' or `%s %s' expected, `%s' found",
                   token_kind_as_string (OBJDEF), token_kind_as_string (EXTERN),
                   token_kind_as_string (OBJDEF), token_as_string (tok));
        parser_unget (parser);
        goto skip_error;
    }

    type = handle_type (parser);

    if (type == NULL || type == error_type_node)
        goto skip_error;

    if (parser_expect_tclass (parser, tok_id))
        name = strdup (token_as_string (parser_get_token (parser)));
    else {
        parser_unget (parser);
        goto skip_error;
    }

    if (!extern_p) {
        struct location loc;

        if (parser_expect_tval (parser, tv_assign))
            parser_get_token (parser);
        else {
            parser_unget (parser);
            goto skip_error;
        }

        tok = parser_get_token (parser);
        loc = token_location (tok);
        parser_unget (parser);

        expr = handle_expr (parser);
        if (expr == NULL || expr == error_mark_node)
            goto skip_error;

        if (NODE_TYPE (expr) != N_spap && NODE_TYPE (expr) != N_prf) {
            error_loc (loc, "function call expected");
            goto skip_error;
        }
    }

    if (parser_expect_tval (parser, tv_semicolon)) {
        struct known_symbol *ks;

        parser_get_token (parser);

        /* Add then name of the object to known symbols in order to
           be able to access it via X::obj, in case is the name
           of the module we are currently parsing.  */
        HASH_FIND_STR (parser->known_symbols, name, ks);
        if (!ks) {
            ks = (struct known_symbol *)malloc (sizeof (struct known_symbol));
            ks->name = strdup (name);
            ks->flags = 0;
            HASH_ADD_KEYPTR (hh, parser->known_symbols, ks->name, strlen (ks->name), ks);
        }

        if (extern_p) {
            node *ret = TBmakeObjdef (type, NULL, name, NULL, NULL);
            OBJDEF_ISEXTERN (ret) = true;
            return loc_annotated (objdef_loc, ret);
        } else
            return loc_annotated (objdef_loc,
                                  TBmakeObjdef (type, NULL, name, expr, NULL));
    }

skip_error:
    parser_get_until_tval (parser, tv_semicolon);
    return error_mark_node;
}

/* (import | use | export | provide |  typedef | objdef
    | function-def | function-dec)*
*/
node *
handle_definitions (struct parser *parser)
{
    struct token *tok;
    bool parse_error = false;
    enum parsed_ftype ftype;

    node *ret = TBmakeModule (NULL, FT_prog, NULL, NULL, NULL, NULL, NULL);

    /* Head of the list.  */
    node *interfaces = NULL;
    node *interfaces_tail = NULL;

    tok = parser_get_token (parser);
    parser_unget (parser);

#define INTERFACE_ADD(exp, interfaces, interfaces_tail)                                  \
    do {                                                                                 \
        if (exp != error_mark_node && exp != NULL) {                                     \
            if (!interfaces) {                                                           \
                interfaces = exp;                                                        \
                interfaces_tail = interfaces;                                            \
            } else {                                                                     \
                switch (NODE_TYPE (interfaces_tail)) {                                   \
                case N_import:                                                           \
                    IMPORT_NEXT (interfaces_tail) = exp;                                 \
                    break;                                                               \
                case N_export:                                                           \
                    EXPORT_NEXT (interfaces_tail) = exp;                                 \
                    break;                                                               \
                case N_use:                                                              \
                    USE_NEXT (interfaces_tail) = exp;                                    \
                    break;                                                               \
                case N_provide:                                                          \
                    PROVIDE_NEXT (interfaces_tail) = exp;                                \
                    break;                                                               \
                default:                                                                 \
                    unreachable ("invalid interface type");                              \
                }                                                                        \
                interfaces_tail = exp;                                                   \
            }                                                                            \
        } else                                                                           \
            parse_error = true;                                                          \
    } while (0)

#define TYPEDEF_ADD(ret, exp)                                                            \
    do {                                                                                 \
        if (exp != error_mark_node && exp != NULL) {                                     \
            TYPEDEF_NEXT (exp) = MODULE_TYPES (ret);                                     \
            MODULE_TYPES (ret) = exp;                                                    \
        } else                                                                           \
            parse_error = true;                                                          \
    } while (0)

#define STRUCTDEF_ADD(ret, exp)                                                          \
    do {                                                                                 \
        if (exp != error_mark_node && exp != NULL) {                                     \
            STRUCTDEF_NEXT (exp) = MODULE_STRUCTS (ret);                                 \
            MODULE_STRUCTS (ret) = exp;                                                  \
        } else                                                                           \
            parse_error = true;                                                          \
    } while (0)

#define OBJDEF_ADD(ret, exp)                                                             \
    do {                                                                                 \
        if (exp != error_mark_node && exp != NULL) {                                     \
            OBJDEF_NEXT (exp) = MODULE_OBJS (ret);                                       \
            MODULE_OBJS (ret) = exp;                                                     \
        } else                                                                           \
            parse_error = true;                                                          \
    } while (0)

#define ADD_FUNCTION(ret, exp, ftype)                                                    \
    do {                                                                                 \
        if (exp != NULL && exp != error_mark_node) {                                     \
            if (ftype == fun_specialize_fundec)                                          \
                MODULE_FUNSPECS (ret) = TCappendFundef (MODULE_FUNSPECS (ret), exp);     \
            else if (ftype == fun_extern_fundec)                                         \
                MODULE_FUNDECS (ret) = TCappendFundef (MODULE_FUNDECS (ret), exp);       \
            else if (ftype == fun_fundef)                                                \
                MODULE_FUNS (ret) = TCappendFundef (MODULE_FUNS (ret), exp);             \
            else                                                                         \
                parse_error = true;                                                      \
        } else                                                                           \
            parse_error = true;                                                          \
    } while (0)

    while (token_class (tok) != tok_eof) {
        node *exp;

        if (token_is_keyword (tok, IMPORT)) {
            exp = handle_interface (parser, int_import);
            INTERFACE_ADD (exp, interfaces, interfaces_tail);
        } else if (token_is_keyword (tok, USE)) {
            exp = handle_interface (parser, int_use);
            INTERFACE_ADD (exp, interfaces, interfaces_tail);
        } else if (token_is_keyword (tok, EXPORT)) {
            exp = handle_interface (parser, int_export);
            INTERFACE_ADD (exp, interfaces, interfaces_tail);
        } else if (token_is_keyword (tok, PROVIDE)) {
            exp = handle_interface (parser, int_provide);
            INTERFACE_ADD (exp, interfaces, interfaces_tail);
        } else if (token_is_keyword (tok, EXTERN)) {
            parser_get_token (parser);
            tok = parser_get_token (parser);
            parser_unget (parser);
            parser_unget (parser);

            if (token_is_keyword (tok, TYPEDEF)) {
                exp = handle_typedef (parser);
                TYPEDEF_ADD (ret, exp);
            } else if (token_is_keyword (tok, OBJDEF)) {
                exp = handle_objdef (parser);
                OBJDEF_ADD (ret, exp);
            } else {
                exp = handle_function (parser, &ftype);
                ADD_FUNCTION (ret, exp, ftype);
            }
        } else if (token_is_keyword (tok, TYPEDEF) || token_is_keyword (tok, NESTED)) {
            exp = handle_typedef (parser);
            TYPEDEF_ADD (ret, exp);
        } else if (token_is_keyword (tok, OBJDEF)) {
            exp = handle_objdef (parser);
            OBJDEF_ADD (ret, exp);
        } else {
            enum function_or_struct { function, structure } f = function;

            /* We may have a structure definition or a function
               which has a structure as a return type.  */
            if (token_is_keyword (tok, STRUCT)) {
                parser_get_token (parser); /* eat-up	`struct'  */
                parser_get_token (parser); /* eat-up `<id>'  */

                if (token_is_operator (parser_get_token (parser), tv_lbrace))
                    f = structure;

                parser_unget3 (parser);
            }

            if (f == structure) {
                exp = handle_struct_def (parser);
                STRUCTDEF_ADD (ret, exp);
            } else {
                exp = handle_function (parser, &ftype);
                ADD_FUNCTION (ret, exp, ftype);
            }
        }

        tok = parser_get_token (parser);
        parser_unget (parser);
    }

    if (!parse_error) {
        MODULE_INTERFACE (ret) = interfaces;
        return ret;
    }

    free_node (ret);
    return error_mark_node;
}

enum dep_type { dep_mod, dep_tree, dep_obj, dep_lib, dep__max };

struct dependency {
    char *name;
    UT_hash_handle hh;
};

static int
dependency_sort (struct dependency *d1, struct dependency *d2)
{
    return strcmp (d1->name, d2->name);
}

static struct dependency *
add_dependency (struct dependency *dependencies, const char *name, enum dep_type type)
{
    struct dependency *t;
    char *dep_name;
    size_t l;

    /* If we found a Tree/Mod dependency for the module we are compiling, ignore it.  */
    if ((type == dep_mod || type == dep_tree) && !strcmp (name, global.modulename))
        return dependencies;

    /* Construct the name for the dependency using its type.  */
    switch (type) {
    case dep_mod:
        l = strlen ("lib") + strlen (name) + strlen ("Mod")
            + strlen (global.config.tree_dllext) + 1;
        dep_name = malloc (l);
        snprintf (dep_name, l, "lib%sMod%s", name, global.config.tree_dllext);
        break;
    case dep_tree:
        l = strlen ("lib") + strlen (name) + strlen ("Tree")
            + strlen (global.config.tree_dllext) + 1;
        dep_name = malloc (l);
        snprintf (dep_name, l, "lib%sTree%s", name, global.config.tree_dllext);
        break;
    case dep_obj:
        dep_name = strdup (name);
        break;
    case dep_lib:
        l = strlen ("-l") + strlen (name) + 1;
        dep_name = malloc (l);
        snprintf (dep_name, l, "-l%s", name);
        break;
    default:
        unreachable ("unknown dependency type");
    }

    HASH_FIND_STR (dependencies, dep_name, t);
    if (!t) {
        t = (struct dependency *)malloc (sizeof *t);
        t->name = dep_name;
        HASH_ADD_KEYPTR (hh, dependencies, t->name, strlen (t->name), t);

        HASH_FIND_STR (dependencies, dep_name, t);
    } else
        free (dep_name);

    return dependencies;
}

int
parse_for_dependencies (struct parser *parser)
{
    struct dependency *dependencies = NULL;

    global.modulename = strdup ("");

    struct token *tok;
    while (tok_eof != token_class (tok = parser_get_token (parser))) {

        if (token_is_keyword (tok, MODULE) || token_is_keyword (tok, CLASS)) {
            struct token *name_tok = parser_get_token (parser);
            if (tok_id == token_class (name_tok)) {
                if (token_is_operator (parser_get_token (parser), tv_semicolon))
                    global.modulename = strdup (token_as_string (name_tok));
                else
                    parser_unget2 (parser);
            } else
                parser_unget (parser);
        }
        /* use or import dependency.  */
        else if (token_is_keyword (tok, IMPORT) || token_is_keyword (tok, USE)) {
            bool need_mod = token_is_keyword (tok, USE);
            struct token *name_tok = parser_get_token (parser);
            if (tok_id == token_class (name_tok)) {
                tok = parser_get_token (parser);
                if (token_is_operator (tok, tv_colon)) {
                    /* No matter whether we us or import we need a Tree file.  */
                    dependencies = add_dependency (dependencies,
                                                   token_as_string (name_tok), dep_tree);

                    /* When we use a module, we need its Mod file.  */
                    if (need_mod)
                        dependencies
                          = add_dependency (dependencies, token_as_string (name_tok),
                                            dep_mod);
                } else
                    parser_unget (parser);
            } else
                parser_unget (parser);
        }
        /* implicit module use <modname>::<id>  */
        else if (token_is_operator (tok, tv_dcolon)) {
            parser_unget2 (parser);
            tok = parser_get_token (parser);
            if (tok_id == token_class (tok)) {
                dependencies
                  = add_dependency (dependencies, token_as_string (tok), dep_tree);
                dependencies
                  = add_dependency (dependencies, token_as_string (tok), dep_mod);
                parser_get_token (parser); parser_get_token (parser);
            } else
                parser_get_token (parser);
        }
        /* pragmas can bring a dependency on an object or a library.  */
        else if (token_is_operator (tok, tv_hash)) {
            tok = parser_get_token (parser);
            if (token_is_keyword (tok, PRAGMA)) {
                bool linkwith_p = false;
                tok = parser_get_token (parser);

                /* dependency on the external library or objectfile.  */
                if ((linkwith_p = token_is_keyword (tok, LINKWITH))
                    || token_is_keyword (tok, LINKOBJ)) {
                    tok = parser_get_token (parser);
                    if (tok_string == token_class (tok)) {
                        if (linkwith_p)
                            dependencies
                              = add_dependency (dependencies, token_as_string (tok),
                                                dep_lib);
                        else
                            dependencies
                              = add_dependency (dependencies, token_as_string (tok),
                                                dep_obj);
                    } else {
                        error_loc (token_location (tok),
                                   "object or library used in pragma has to be a "
                                   "string, `%s' found instead",
                                   token_as_string (tok));
                        parser_unget (parser);
                    }
                } else
                    parser_unget (parser);
            } else
                /* WTF is '#' not followed by the 'pragma'?  Skip it.  */
                parser_unget (parser);
        }
    }

    free (global.modulename);
    global.modulename = NULL;

    struct dependency *d;
    struct dependency *tmp;

    /* XXX consider whether sorting by name is a good thing...  */
    HASH_SORT (dependencies, dependency_sort);

    /* Print the list of dependencies for the given file.
       TODO(artem) Depending on the format of output, create a
                   Make rule or CMake-appropriate format.  For
                   The time being we simply print a list.  */
    HASH_ITER (hh, dependencies, d, tmp)
        fprintf (stdout, "%s\n", d->name);

    /* Free the hash table with dependencies.  */
    HASH_ITER (hh, dependencies, d, tmp) {
        HASH_DEL (dependencies, d);
        free (d->name);
        free (d);
    }

    return 0;
}

/* Top level function to parse the file.  */
int
parse (struct parser *parser)
{
    struct token *tok;

    // error_count = warning_count = 0;
    tok = parser_get_token (parser);

    if (token_is_keyword (tok, MODULE)) {
        node *defs;
        char *name = NULL;
        char *deprecated = NULL;

        if (parser_expect_tclass (parser, tok_id)) {
            tok = parser_get_token (parser);
            name = strdup (token_as_string (tok));

            tok = parser_get_token (parser);
            if (token_is_keyword (tok, DEPRECATED)) {
                if (parser_expect_tclass (parser, tok_string)) {
                    tok = parser_get_token (parser);
                    deprecated = strdup (token_as_string (tok));
                } else
                    parser_get_until_tval (parser, tv_semicolon);
            } else
                parser_unget (parser);
        } else {
            /* We need to set some name as module name, at this point
               it doesn't really matter which one, as we have encountered
               an error and we just want to parse the definitions, so use
               the value of the token.  */
            tok = parser_get_token (parser);
            name = strdup (token_as_string (tok));
            if (!token_is_operator (tok, tv_semicolon))
                parser_get_until_tval (parser, tv_semicolon);
        }

        /* FIXME otherwise what... */
        if (CTIgetErrorCount () == 0)
            if (parser_expect_tval (parser, tv_semicolon))
                parser_get_token (parser);

        parser->in_module = true;
        parser->current_module = name;

        OPTcheckOptionConsistencyForTarget (true);

        defs = handle_definitions (parser);
        parser->in_module = false;
        parser->current_module = NULL;

        if (CTIgetErrorCount () == 0 && defs != NULL && defs != error_mark_node) {
            MODULE_NAMESPACE (defs) = NSgetNamespace (name);
            MODULE_FILETYPE (defs) = FT_modimp;
            MODULE_DEPRECATED (defs) = deprecated;
            global.syntax_tree = defs;
        } else
            free (name);
    } else if (token_is_keyword (tok, CLASS)) {
        node *defs = error_mark_node;
        ntype *classtype = error_type_node;
        node *pragmas = NULL;
        char *name = NULL;
        char *deprecated = NULL;
        bool classtype_parse_error = false;

        if (parser_expect_tclass (parser, tok_id)) {
            tok = parser_get_token (parser);
            name = strdup (token_as_string (tok));

            tok = parser_get_token (parser);
            if (token_is_keyword (tok, DEPRECATED)) {
                if (parser_expect_tclass (parser, tok_string)) {
                    tok = parser_get_token (parser);
                    deprecated = strdup (token_as_string (tok));
                } else {
                    parser_unget (parser);
                    parser_get_until_tval (parser, tv_semicolon);
                }
            } else
                parser_unget (parser);
        } else {
            /* We need to set some name as module name, at this point
               it doesn't really matter which one, as we have encountered
               an error and we just want to parse the definitions, so use
               the value of the token.  */
            tok = parser_get_token (parser);
            name = strdup (token_as_string (tok));
            if (!token_is_operator (tok, tv_semicolon))
                parser_get_until_tval (parser, tv_semicolon);
        }

        /* Add class name into the hash-table of known types.  */
        if (name != NULL) {
            struct known_symbol *ks;
            HASH_FIND_STR (parser->known_symbols, name, ks);
            if (!ks) {
                ks = (struct known_symbol *)malloc (sizeof (struct known_symbol));
                ks->name = strdup (name);
                ks->flags = 0;
                symbol_set_type (ks);
                HASH_ADD_KEYPTR (hh, parser->known_symbols, ks->name, strlen (ks->name),
                                 ks);
            } else
                symbol_set_type (ks);
        }

        if (CTIgetErrorCount () == 0)
            if (parser_expect_tval (parser, tv_semicolon))
                parser_get_token (parser);

        tok = parser_get_token (parser);
        if (token_is_keyword (tok, CLASSTYPE))
            classtype = handle_type (parser);
        else if (token_is_keyword (tok, EXTERN)) {
            tok = parser_get_token (parser);
            if (token_is_keyword (tok, CLASSTYPE))
                classtype
                  = TYmakeAKS (TYmakeHiddenSimpleType (UT_NOT_DEFINED), SHmakeShape (0));
            else {
                parser_unget (parser);
                classtype_parse_error = true;
            }
        } else
            classtype_parse_error = true;

        if (classtype_parse_error) {
            error_loc (token_location (tok), "`%s' or `%s %s' expected, `%s' found",
                       token_kind_as_string (CLASSTYPE), token_kind_as_string (EXTERN),
                       token_kind_as_string (CLASSTYPE), token_as_string (tok));
            parser_unget (parser); parser_get_until_tval (parser, tv_semicolon);
        } else if (parser_expect_tval (parser, tv_semicolon))
            parser_get_token (parser);

        pragmas = handle_pragmas (parser, pragma_typedef);

        /* Set parser IN_MODULE flag on, and set the name of the
           module to the name of the class.  */
        parser->in_module = true;
        parser->current_module = name;

        OPTcheckOptionConsistencyForTarget (true);

        defs = handle_definitions (parser);
        parser->in_module = false;
        parser->current_module = NULL;

        if (CTIgetErrorCount () == 0 && defs != NULL && defs != error_mark_node) {
            MODULE_NAMESPACE (defs) = NSgetNamespace (name);
            MODULE_FILETYPE (defs) = FT_classimp;
            MODULE_DEPRECATED (defs) = deprecated;
            defs = SetClassType (defs, classtype, pragmas);
            global.syntax_tree = defs;
        } else
            free (name);
    } else if (token_class (tok) == tok_unknown) {
        error_loc (token_location (tok), "unknown token found `%s'!",
                   token_as_string (tok));
    } else {
        node *defs;

        parser_unget (parser);

        OPTcheckOptionConsistencyForTarget (false);

        defs = handle_definitions (parser);
        if (CTIgetErrorCount () == 0 && defs != error_mark_node) {
            MODULE_NAMESPACE (defs) = NSgetRootNamespace ();
            MODULE_FILETYPE (defs) = FT_prog;
            global.syntax_tree = defs;
        }
    }

    return 0;
}

/* The following rules are used to parse rc-files.  */

inheritence_list_t *handle_inherited_name (struct parser *, inheritence_list_t *);
resource_list_t *handle_resource_list (struct parser *, resource_list_t *);

/* Handle inherited name of target in a resource file. For example:
   target MBair_cuda::MBair_32bit::cuda:  */
inheritence_list_t *
handle_inherited_name (struct parser *parser, inheritence_list_t *ilist)
{
    struct token *tok;
    char *id = NULL;

    tok = parser_get_token (parser);

    if (token_class (tok) == tok_operator) {
        if (token_value (tok) == tv_dcolon) {
            tok = parser_get_token (parser);
            if (token_class (tok) == tok_id || token_class (tok) == tok_keyword) {
                id = strdup (token_as_string (tok));
                ilist = handle_inherited_name (parser, ilist);
                return RSCmakeInheritenceListEntry (id, ilist);
            } else {
                error_loc (token_location (tok), "id expected");
                parser_get_until_tval (parser, tv_colon);
                return NULL;
            }
        } else if (token_value (tok) == tv_colon) {
            return NULL;
        }
    }

    parser_unget (parser);
    error_loc (token_location (tok), ": or :: expected");
    return NULL;
}

/* Handle one line of a target in a resource file. Line could be:
   ID := STRING | NUM
   ID += STRING | NUM  */
resource_list_t *
handle_resource_list (struct parser *parser, resource_list_t *res_list)
{
    struct token *tok;
    char *id;
    char *str;

    tok = parser_get_token (parser);
    /* Target cannot be a name of a resource-variable.  */
    if (token_class (tok) == tok_id
        || (token_class (tok) == tok_keyword && token_value (tok) != TARGET)) {
        struct token *tok1;
        struct token *tok2;

        /* Name of the resource-variable.  */
        id = strdup (token_as_string (tok));

        tok1 = parser_get_token (parser);
        tok2 = parser_get_token (parser);

        /* Handle ID := NUM | STRING case.  */
        if (token_is_operator (tok1, tv_colon) && token_is_operator (tok2, tv_assign)) {
            tok = parser_get_token (parser);
            if (token_class (tok) == tok_string) {
                str = strdup (token_as_string (tok));
                res_list = handle_resource_list (parser, res_list);
                return RSCmakeResourceListEntry (id, str, 0, 0, res_list);
            } else if (token_class (tok) == tok_number) {
                int val = atoi (token_as_string (tok));
                res_list = handle_resource_list (parser, res_list);
                return RSCmakeResourceListEntry (id, NULL, val, 0, res_list);
            } else {
                MEMfree (id);
                error_loc (token_location (tok), "string or number expected, %s found",
                           token_as_string (tok));
            }
        }
        /* Handle ID += NUM | STRING case.  */
        else if (token_class (tok1) == tok_operator && token_value (tok1) == tv_plus_eq) {
            if (token_class (tok2) == tok_string) {
                str = strdup (token_as_string (tok2));
                res_list = handle_resource_list (parser, res_list);
                return RSCmakeResourceListEntry (id, str, 0, 1, res_list);
            } else if (token_class (tok2) == tok_number) {
                int val = atoi (token_as_string (tok2));
                res_list = handle_resource_list (parser, res_list);
                return RSCmakeResourceListEntry (id, NULL, val, 1, res_list);
            } else {
                MEMfree (id);
                error_loc (token_location (tok), "string or number expected, %s found",
                           token_as_string (tok));
            }
        } else {
            MEMfree (id);
            error_loc (token_location (tok), ":= or += expected");
        }
    } else
        parser_unget (parser);

    return NULL;
}

/* Hanle one target of the resource file.  */
target_list_t *
handle_rctarget (struct parser *parser, target_list_t *tl)
{
    inheritence_list_t *ilist = NULL;
    resource_list_t *res_list = NULL;
    struct token *tok;
    char *id;

    tok = parser_get_token (parser);
    if (token_class (tok) != tok_id) {
        error_loc (token_location (tok), "name expected after the `target'");
        id = strdup ("???");
        parser_unget (parser);
    } else
        id = strdup (token_as_string (tok));

    ilist = handle_inherited_name (parser, ilist);
    res_list = handle_resource_list (parser, res_list);
    return RSCmakeTargetListEntry (id, ilist, res_list, tl);
}

/* Top rule to parser a resource file.  */
int
parse_rcfile (struct parser *parser)
{
    struct token *tok;
    target_list_t *tl = NULL;

    while (token_class (tok = parser_get_token (parser)) != tok_eof) {
        switch (token_class (tok)) {
        case tok_keyword:
            if (token_value (tok) == TARGET)
                tl = handle_rctarget (parser, tl);
            else
                error_loc (token_location (tok), "target expected");
            break;

        default:
            error_loc (token_location (tok), "target expected");
        }
    }

    global.target_list = RSCaddTargetList (tl, global.target_list);
    return 0;
}

int
SPmyYyparse (void)
{
    struct lexer *lex = (struct lexer *)malloc (sizeof (struct lexer));
    struct parser *parser = (struct parser *)malloc (sizeof (struct parser));
    int ret = 0;

    DBUG_ENTER ();

    memset (lex, 0, sizeof (*lex));
    if (!lexer_init_file (lex, yyin, global.filename)) {
        fprintf (stderr, "cannot create a lexer for file %s\n", global.filename);
        ret = -2;
        goto cleanup;
    } else

        parser_init (parser, lex);

    if (global.start_token == PARSE_RC)
        parse_rcfile (parser);
    else if (!global.makedeps)
        parse (parser);
    else
        parse_for_dependencies (parser);

cleanup:
    parser_finalize (parser);

    if (parser)
        free (parser);
    if (lex) {
        struct file_name *f;
        struct file_name *tmp;
        size_t sz = 0;

        /* Before finalizing lexer dump the list of the files in the
           global.file_table to keep token file links alive.  */
        HASH_ITER (hh, lex->file_names, f, tmp)
            sz++;

        /* We need this table to ensure that FILE* fields
           in the locations in the AST are valid.  So here we
           are simply extending this table, even though we may have
           repeated filenames.  */
        global.file_table = (char **)realloc (global.file_table,
                                              (global.file_table_size + sz)
                                              * sizeof (char *));

        /* Fill global.file_table and remove the filenams from lexer's
           hash table to avoid freeing.  */
        HASH_ITER (hh, lex->file_names, f, tmp) {
            global.file_table[global.file_table_size++] = f->name;
            HASH_DEL (lex->file_names, f);
            free (f);
        }

        lexer_finalize (lex, false);
        free (lex);
    }

    /* If we only want to generate a list of dependencies and we have just parsed
       a program, but not a configuration file -- terminate.  */
    if (global.makedeps && global.start_token != PARSE_RC)
        CTIexit (EXIT_SUCCESS);

    DBUG_RETURN (ret);
}
