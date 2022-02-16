#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "globals.h"
#include "resource.h"
#include "types.h"
#include "memory.h"
#include "free.h"
#include "new_types.h"
#include "tree_basic.h"
#include "filemgr.h"
#define DBUG_PREFIX "JUP"
#include "debug.h"


#include "uthash.h"
#include "lex.h"
#include "compat.h"
#include "jupyter.h"
#include "parser.h"


static struct lexer *jup_lex;
static struct parser *jup_parser;
static char **jup_argv;

int
jupyter_init (void)
{
    const char * sac2crc = "sac2crc" BUILD_TYPE_POSTFIX;

    char *global_sac2crc;
    char *build_sac2crc;

    if (-1 == asprintf (&global_sac2crc, "%s/%s", SAC2CRC_DIR, sac2crc))
        return -1;
    if (-1 == asprintf (&build_sac2crc, "%s/%s", SAC2CRC_BUILD_DIR, sac2crc))
        return -1;

    RSCsetSac2crcLocations (global_sac2crc, build_sac2crc);

    // Initialse fresh compiler
    jup_argv = (char **)malloc (sizeof (char *));
    jup_argv[0] = strdup ("test");

    set_debug_exit_function (CTIexit);
    CTIset_stderr (stderr);

    GLOBinitializeGlobal (1, jup_argv, TOOL_sac2c, jup_argv[0]);
    RSCevaluateConfiguration ();
    FMGRsetupPaths ();


    jup_lex = (struct lexer *)malloc (sizeof (struct lexer));
    jup_parser = (struct parser *)malloc (sizeof (struct parser));

    memset (jup_lex, 0, sizeof (*jup_lex));
    lexer_init_file (jup_lex, (FILE*)1, "<string-input>");
    parser_init (jup_parser, jup_lex);

    return 0;
}


bool
parser_print_tables (struct parser *parser)
{
    struct known_symbol *elem;
    struct known_symbol *tmp;

    struct used_module *mod;
    struct used_module *mtmp;

    assert (parser, "attempt to free empty parser");
    HASH_ITER (hh, parser->known_symbols, elem, tmp) {
        printf ("known symbol `%s', flags = %d\n", elem->name, elem->flags);
    }

    HASH_ITER (hh, parser->used_modules, mod, mtmp) {
        printf ("known module `%s'\n", mod->name);
        HASH_ITER (hh, mod->symbols, elem, tmp) {
            printf ("\tknown symbol `%s', flags = %d\n", elem->name, elem->flags);
        }
        trie_print (mod->user_ops);
    }

    return true;
}

char *
jupyter_parse_from_string (const char *s)
{
    assert (jup_parser, "parser has to be initialised");
    char *local_stderr_content;
    char *symb = NULL;
    size_t local_stderr_sz;
    FILE *fin = fmemopen ((void *)s, strlen (s), "r");
    FILE *local_stderr = open_memstream (&local_stderr_content, &local_stderr_sz);

    FILE *old_stderr = CTIget_stderr ();
    CTIset_stderr (local_stderr);

    fprintf (local_stderr, "======= trying to parse `%s'\n", s);

    node *n = NULL;
    int ret = -1;

#define PARSER_REINIT(__parser, __lex) \
    do { \
        __parser->lex = __lex; \
        __parser->buf_start = 0; \
        __parser->buf_end = 0; \
        __parser->buf_empty = true; \
        __parser->unget_idx = 0; \
        __parser->in_return = false; \
        __parser->in_subscript = false; \
        __parser->in_module = false; \
        __parser->main_count = 0; \
        __parser->current_module = NULL; \
    } while (0)

#define LEXER_REINIT(__lex) \
    do { \
        __lex->is_eof = false; \
        __lex->loc = (struct location){__lex->fname, 1, 0}; \
        __lex->is_read_user_op = false; \
        __lex->buf_start = 0; \
        __lex->buf_end = 0; \
        __lex->unget_idx = 0; \
    } while (0)

#define PARSER_FINALIZE(__parser) \
    do { \
        while (__parser->buf_start % __parser->buf_size != __parser->buf_end % __parser->buf_size) { \
            token_free (__parser->token_buffer[__parser->buf_start]); \
            __parser->buf_start = (__parser->buf_start + 1) % __parser->buf_size; \
        } \
    } while (0)

#define PARSER_RESET(__parser) \
    do { \
        FILE *__f = __parser->lex->file; \
        struct lexer *__lex = __parser->lex; \
        PARSER_FINALIZE (__parser); \
        rewind (__f); \
        LEXER_REINIT (__lex); \
        PARSER_REINIT (__parser, __lex); \
    } while (0)


#define RET_OR_RESET(__parser, __cond, __ret_val, __symb) \
    do {  \
        if (__cond) { \
            ret = __ret_val; \
            symb = __symb; \
            goto cleanup; \
        } else { \
            PARSER_RESET (__parser); \
        } \
    } while (0)

#define HANDLE_USEIMPORTS(__parser, __num) \
    do { \
        struct token *tok = parser_get_token (jup_parser); \
        parser_unget (jup_parser); \
        if (token_is_keyword (tok, USE)) \
            n = handle_interface (jup_parser, int_use); \
        else if (token_is_keyword (tok, IMPORT)) \
            n = handle_interface (jup_parser, int_import); \
        else if (token_is_keyword (tok, TYPEDEF)) \
            n = handle_typedef (jup_parser); \
    } while (0)

    jup_parser->lex->file = fin;
    PARSER_RESET (jup_parser);

    // Is it expression we are dealing with?
    fprintf (local_stderr, "======= parsing as expression\n");
    CTIresetErrorCount ();
    n = handle_expr (jup_parser);
    RET_OR_RESET (jup_parser,
                  n
                  && n != error_mark_node
                  && CTIgetErrorCount () == 0
                  && tok_eof == token_class (parser_get_token (jup_parser)), 1, "");

    // Is it statement list?
    CTIresetErrorCount ();
    fprintf (local_stderr, "======= parsing as list of statements\n");
    n = handle_list_of_stmts (jup_parser);
    RET_OR_RESET (jup_parser,
                  n
                  && n != error_mark_node
                  && CTIgetErrorCount () == 0
                  && tok_eof == token_class (parser_get_token (jup_parser)), 2, "");

    // Is it a function definition?
    CTIresetErrorCount ();
    enum parsed_ftype ft;
    fprintf (local_stderr, "======= parsing as function definition\n");
    n = handle_function (jup_parser, &ft);
    if (n && (n != error_mark_node) && (CTIgetErrorCount () == 0) && (ft == fun_fundef)) {
        ret = 3;
        symb = STRcpy (FUNDEF_NAME (n));
        while ((ret == 3) && (tok_eof != token_class (parser_get_token (jup_parser)))) {
            parser_unget (jup_parser);
            n = handle_function (jup_parser, &ft);
            if (n && (n != error_mark_node) && (CTIgetErrorCount () == 0) && (ft == fun_fundef)) {
                if (!STReq (symb, FUNDEF_NAME (n))) {
                    ret = -1;
                    fprintf (local_stderr, "======= more than one function symbol defined in one cell is not supported!\n");
                }
            } else {
                ret = -1;
            }
        }
        goto cleanup;
    } else {
        PARSER_RESET (jup_parser);
    }
    RET_OR_RESET (jup_parser,
                  n
                  && n != error_mark_node
                  && CTIgetErrorCount () == 0
                  && ft == fun_fundef
                  && tok_eof == token_class (parser_get_token (jup_parser)), 3, STRcpy (FUNDEF_NAME (n)));

    // Is it a use/import/typedef?
    CTIresetErrorCount ();
    fprintf (local_stderr, "======= parsing as use/import/typedef\n");
    HANDLE_USEIMPORTS (jup_parser, -1);
    RET_OR_RESET (jup_parser,
                  n
                  && n != error_mark_node
                  && CTIgetErrorCount () == 0
                  && tok_eof == token_class (parser_get_token (jup_parser)), 4, "");

cleanup:
    if (n && n != error_mark_node)
      free_tree (n);


    fclose (fin);
    fflush (local_stderr);
    fclose (local_stderr);
    CTIset_stderr (old_stderr);

    char *ret_json = NULL;
    char *quoted_stderr = quote_string_json (local_stderr_content, NULL, 0);

    free (local_stderr_content);

    if (-1 == asprintf (&ret_json,
                        "{\n"
                        "   \"status\": \"%s\",\n"
                        "   \"ret\": %d,\n"
                        "   \"symbol\": \"%s\",\n"
                        "   \"stderr\": \"%s\"\n"
                        "}",
                        ret == -1 ? "fail" : "ok",
                        ret,
                        symb,
                        quoted_stderr))
        ret_json = strdup ("{ \"status\": \"fail\","
                           "   \"stderr\": \"asprintf failed\" }");

    free (quoted_stderr);
    return ret_json;

}

void
jupyter_free (void *p) {
    (void) MEMfree (p);
}

int
jupyter_finalize (void)
{
    parser_finalize (jup_parser);

    if (jup_parser)
        free (jup_parser);
    if (jup_lex) {
        lexer_finalize (jup_lex, false);
        free (jup_lex);
    }
    free (jup_argv[0]);
    free (jup_argv);
    return 0;
}
