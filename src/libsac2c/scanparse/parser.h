#ifndef __PARSER_H__
#define __PARSER_H__

#include "trie.h"
#include "uthash.h"
#include "compat.h"

#define F_SYMBOL_IS_TYPE (1 << 0)
#define F_SYMBOL_IS_UNARY (1 << 1)
#define F_SYMBOL_IS_BINARY (1 << 2)

#define symbol_is_binary(symbol) (symbol->flags & F_SYMBOL_IS_BINARY)
#define symbol_is_unary(symbol) (symbol->flags & F_SYMBOL_IS_UNARY)
#define symbol_is_type(symbol) (symbol->flags & F_SYMBOL_IS_TYPE)

#define symbol_set_binary(symbol) (symbol->flags |= F_SYMBOL_IS_BINARY)
#define symbol_set_unary(symbol) (symbol->flags |= F_SYMBOL_IS_UNARY)
#define symbol_set_type(symbol) (symbol->flags |= F_SYMBOL_IS_TYPE)

/* The following structure is used to cache the symbols included
   from modules.  This is used in order to resolve ambiguities of
   prefix, postfix and infix oeprations (as any binary function
   can be used infix, and any unary prefix and postfix); any type
   defined can be used in a type-cast: (type)x.  */
struct known_symbol {
    char *name;
    char flags;
    UT_hash_handle hh;
};

/* This structure is used just to cache module which were
   used witout `use' or `import' but by prefexing a certain
   symbol.  For example:
      f = Math::sqrt (23);
   The trick is, that we have to pre-load a symbol table of
   the module and modify a symbol-table of the lexer, as the
   module may export awkward symbols like "---", which in
   normal case would mean two tokens.  Also, we have to know
   whether a symbol can be used as unary or binary function
   which would allow infix/postfix/prefix notation.  */
struct used_module {
    char *name;
    struct trie *user_ops;
    struct known_symbol *symbols;
    UT_hash_handle hh;
};

/* Parser to parse a single file.  All the parsing procedures
   require passing a pointer to this structure as a parameter.
   This makes it possible to use the code in a multi-threaded
   fashion.  */
struct parser {
    struct lexer *lex;

    /* Buffer and lengths associated with buffer.
       Buffer holds up-to BUF_SIZE tokens, which means
       that it is possible to look BUF_SIZE tokens
       forward.  */
    struct token **token_buffer;
    size_t buf_size;
    size_t buf_start, buf_end, unget_idx;

    struct known_symbol *known_symbols;
    struct used_module *used_modules;

    bool buf_empty;

    /* Context-dependend parser variables.  */
    bool in_return;
    bool in_subscript;
    bool in_arraycomp_expr;

    /* In case we are parsing a module.  */
    bool in_module;

    /* The name of the module we are currently parsing or NULL.  */
    char *current_module;

    /* Count of opened parens, square brackets and
       figure brackets. Used when we skip the tokens
       skip is finished when all the three counters
       are zeroes.  */
    int paren_count;
    int square_count;
    int brace_count;
};

/* Check if parser is not in any parenthesis/bracket expression.  */
static inline bool
parser_parens_zero (struct parser *parser)
{
    return parser->paren_count == 0 && parser->square_count == 0
           && parser->brace_count == 0;
}

#define TOKEN_CLASS(a, b)                                                                \
    static inline bool token_is_##a (struct token *tok, enum token_kind tkind)           \
    {                                                                                    \
        return token_class (tok) == tok_##a && token_value (tok) == tkind;               \
    }
#include "token_class.def"
#undef TOKEN_CLASS

/* This enum allows to filter the pragmas during the parsing.  */
enum pragma_type { pragma_fundef, pragma_fundec, pragma_typedef, pragma_objdef };

/* Type of the function we parsed.  Unfortunately, SaC stores extern
   and specialize attributes not inside the function node, but
   puts a function in a different list.  The variable is returned
   by handle_function and could be later used to correctly add the
   function into the right list.  */
enum parsed_ftype { fun_error, fun_fundef, fun_extern_fundec, fun_specialize_fundec };

/* Most of the module-interfaces are similar and are parsed wiht the
   same function.  In order to deffirentiate what kind of interface we
   want to pars we pass a parameter of the type enum interface_kind.
*/
enum interface_kind { int_import, int_use, int_export, int_provide };

/* When one checks if variable-name is at the parser's state,
   it may be useful to pass the namespace and the name of
   the variable as a result.  In order to do that structure
   identifier is used.

   is_operation field should be set on, when the identifier
   can be used as unary or binay operation.  */

struct identifier {
    char *xnamespace;
    char *id;
    bool is_operation;
};

static inline struct identifier *
identifier_new (char *xnamespace, char *id, bool op)
{
    struct identifier *ret;
    ret = (struct identifier *)malloc (sizeof (struct identifier));
    ret->xnamespace = xnamespace;
    ret->id = id;
    ret->is_operation = op;

    return ret;
}

static inline void
identifier_free (struct identifier *id)
{
    if (id) {
        if (id->xnamespace)
            free (id->xnamespace);

        if (id->id)
            free (id->id);

        free (id);
    }
}

/* This structure is used to pass an expression after prefix and
   postfix parts were processed.  As postfix part has a higher priority
   than the infix one, an expression  "+ - a ! $" will be parsed
   as "+ (- ($ (! (a))))".  In case one would need to grab the last
   operator from such an expression, we need to have a link in order
   there.  So the EXPR field of the structure would store whole the
   expression, PARENT_EXPRS would store SPAP_ARGS (x) where x
   is an application of unary minus.  */

struct pre_post_expr {
    node *expr;
    node *parent_exprs;
};


/* A data structure to pass the array comprehension context.  */
struct array_comp_ctxt {
    /* Whether we have a comprehension:
            { iv -> ... }                   (true)
            { [(iv | ',') list] -> ... }    (false)  */
    bool single_bound_variable;  

    /* The number of bound variables.  */
    size_t bound_sz;

    /* A list of N_spid nodes, each of which repesents a bound
       variable, preserving an order they came at the left hand
       side of the `->'.  */
    node **bound;
};


/* Create versions to unget several tokens.  */
#define parser_unget2(parser)                                                            \
    do {                                                                                 \
        parser_unget (parser);                                                           \
        parser_unget (parser);                                                           \
    } while (0)

#define parser_unget3(parser)                                                            \
    do {                                                                                 \
        parser_unget2 (parser);                                                          \
        parser_unget (parser);                                                           \
    } while (0)

struct token *parser_get_token (struct parser *parser);
void parser_unget (struct parser *parser);

static inline struct token *
parser_peek_token (struct parser *parser)
{
    struct token *t = parser_get_token (parser);
    parser_unget (parser);
    return t;
}

#endif /* __PARSER_H__  */
