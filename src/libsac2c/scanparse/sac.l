%{

/*
 * $Id$
 */



#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "resource.h"
#include "sac.tab.h"
#include "ctinfo.h"
#include "globals.h"
#include "check_mem.h"

#define FILTER(token) \
  charpos += yyleng;  \
  if (commlevel == 0) { return( token); }

#define LINEBUF_SIZE 256
#define YY_NO_UNPUT 1

int start_token;
char linebuf[ LINEBUF_SIZE];
char *tmp_buf = NULL;

char *linebuf_ptr = NULL;
int commlevel = 0;
int charpos = 0;


%}

%p 6000 /* set positions to 6000 */
%e 2000 /* set nodes to 1100 */
%option noyywrap

%%
%{

#ifdef MUST_INIT_YY
/* the next two lines are a very dirty hack for avoiding a gcc-warning 8-(( */
  yy_cp = NULL;
  yy_bp = NULL;
#endif

/* the next few lines allow for smuggling in a start_token! */
   if (global.start_token) {
     int mem = global.start_token;
#ifdef OLD_VERSION_OF_LEX  /* (previously needed on ALPHA) */
     yy_init = 1;  /* force new initialization of lex! */
#endif
     global.start_token = 0;
     return( mem);
   }
%}


"{"                        { FILTER( BRACE_L); }
"}"                        { FILTER( BRACE_R); }
"("                        { FILTER( BRACKET_L); }
")"                        { FILTER( BRACKET_R); }
"["                        { FILTER( SQBR_L); }
"]"                        { FILTER( SQBR_R); }
":"                        { FILTER( COLON); }
"::"                       { FILTER( DCOLON); }
";"                        { FILTER( SEMIC); }
","                        { FILTER( COMMA); }
"="                        { FILTER( LET); }
"."                        { FILTER( DOT); }
"->"                       { FILTER( ARROW); }

"&"                        { FILTER( AMPERS);} /* needed for refernce params */
"!"                        { FILTER( EXCL); }  /* needed for monop supp      */
"~"                        { FILTER( TILDE); } /* needed for monop supp      */
"-"                        { FILTER( MINUS); } /* needed for monop supp      */
"+"                        { FILTER( PLUS); }  /* needed for AUDGZ types     */
"*"                        { FILTER( STAR); }  /* needed for AUD types       */
"++"                       { FILTER( INC); }   /* needed for increment       */
"--"                       { FILTER( DEC); }   /* needed for decrement       */
"<="                       { FILTER( LE);   }  /* needed for WL generators   */
"<"                        { FILTER( LT);   }  /* needed for WL generators   */
">"                        { FILTER( GT);   }  /* needed for SIBs            */

"+="                       { FILTER( ADDON); }
"-="                       { FILTER( SUBON); }
"*="                       { FILTER( MULON); }
"/="                       { FILTER( DIVON); }
"%="                       { FILTER( MODON); }


"_dim_"                    { FILTER( PRF_DIM); }
"_shape_"                  { FILTER( PRF_SHAPE); }
"_reshape_"                { FILTER( PRF_RESHAPE); }
"_sel_"                    { FILTER( PRF_SEL); }
"_genarray_"               { FILTER( PRF_GENARRAY); }
"_modarray_"               { FILTER( PRF_MODARRAY); }

"_cat_VxV_"                { FILTER( PRF_CAT_VxV); }
"_take_SxV_"               { FILTER( PRF_TAKE_SxV); }
"_drop_SxV_"               { FILTER( PRF_DROP_SxV); }

"_add_SxS_"                { FILTER( PRF_ADD_SxS); }
"_add_SxA_"                { FILTER( PRF_ADD_SxA); }
"_add_AxS_"                { FILTER( PRF_ADD_AxS); }
"_add_AxA_"                { FILTER( PRF_ADD_AxA); }

"_sub_SxS_"                { FILTER( PRF_SUB_SxS); }
"_sub_SxA_"                { FILTER( PRF_SUB_SxA); }
"_sub_AxS_"                { FILTER( PRF_SUB_AxS); }
"_sub_AxA_"                { FILTER( PRF_SUB_AxA); }

"_mul_SxS_"                { FILTER( PRF_MUL_SxS); }
"_mul_SxA_"                { FILTER( PRF_MUL_SxA); }
"_mul_AxS_"                { FILTER( PRF_MUL_AxS); }
"_mul_AxA_"                { FILTER( PRF_MUL_AxA); }

"_div_SxS_"                { FILTER( PRF_DIV_SxS); }
"_div_SxA_"                { FILTER( PRF_DIV_SxA); }
"_div_AxS_"                { FILTER( PRF_DIV_AxS); }
"_div_AxA_"                { FILTER( PRF_DIV_AxA); }

"_mod_"                    { FILTER( PRF_MOD); }
"_min_"                    { FILTER( PRF_MIN); }
"_max_"                    { FILTER( PRF_MAX); }
"_abs_"                    { FILTER( PRF_ABS); }
"_neg_"                    { FILTER( PRF_NEG); }

"_eq_"                     { FILTER( PRF_EQ);  }
"_neq_"                    { FILTER( PRF_NEQ); }
"_le_"                     { FILTER( PRF_LE); }
"_lt_"                     { FILTER( PRF_LT); }
"_ge_"                     { FILTER( PRF_GE); }
"_gt_"                     { FILTER( PRF_GT); }

"_and_"                    { FILTER( PRF_AND); }
"_or_"                     { FILTER( PRF_OR);  }
"_not_"                    { FILTER( PRF_NOT); }

"_toi_S_"                  { FILTER( PRF_TOI_S); }
"_toi_A_"                  { FILTER( PRF_TOI_A); }
"_tof_S_"                  { FILTER( PRF_TOF_S); }
"_tof_A_"                  { FILTER( PRF_TOF_A); }
"_tod_S_"                  { FILTER( PRF_TOD_S); }
"_tod_A_"                  { FILTER( PRF_TOD_A); }


"typedef"                  { FILTER( TYPEDEF); }
"classtype"                { FILTER( CLASSTYPE); }
"objdef"                   { FILTER( OBJDEF); }
"main"                     { FILTER( K_MAIN); }
"return"                   { FILTER( RETURN); }
"if"                       { FILTER( IF); }
"else"                     { FILTER( ELSE); }
"do"                       { FILTER( DO); }
"while"                    { FILTER( WHILE); }
"for"                      { FILTER( FOR); }
"with"                     { FILTER( NWITH); }
"step"                     { FILTER( STEP); }
"width"                    { FILTER( WIDTH); }
"genarray"                 { FILTER( GENARRAY); }
"modarray"                 { FILTER( MODARRAY); }
"foldfix"                  { FILTER( FOLDFIX); }
"fold"                     { FILTER( FOLD); }
"propagate"                { FILTER( PROPAGATE); }
"inline"                   { FILTER( INLINE); }

"module"                   { FILTER( MODULE); }
"class"                    { FILTER( CLASS); }
"ModuleSpec"               { FILTER( MODSPEC); }

"import"                   { FILTER( IMPORT); }
"export"                   { FILTER( EXPORT); }
"provide"                  { FILTER( PROVIDE); }
"use"                      { FILTER( USE); }
"all"                      { FILTER( ALL); }
"except"                   { FILTER( EXCEPT); }
"external"                 { FILTER( EXTERN); }

"#"                        { FILTER( HASH); }
"pragma"                   { FILTER( PRAGMA); }
"linkname"                 { FILTER( LINKNAME); }
"linksign"                 { FILTER( LINKSIGN); }
"effect"                   { FILTER( EFFECT); }
"refcounting"              { FILTER( REFCOUNTING); }
"copyfun"                  { FILTER( COPYFUN); }
"freefun"                  { FILTER( FREEFUN); }
"initfun"                  { FILTER( INITFUN); }
"linkwith"                 { FILTER( LINKWITH); }
"linkobj"                  { FILTER( LINKOBJ); }
"wlcomp"                   { FILTER( WLCOMP); }
"cachesim"                 { FILTER( CACHESIM); }
"specialize"               { FILTER( SPECIALIZE); }

"int"                      { FILTER( TYPE_INT); }
"float"                    { FILTER( TYPE_FLOAT); }
"bool"                     { FILTER( TYPE_BOOL); }
"unsigned"                 { FILTER( TYPE_UNS); }
"short"                    { FILTER( TYPE_SHORT); }
"long"                     { FILTER( TYPE_LONG); }
"char"                     { FILTER( TYPE_CHAR); }
"double"                   { FILTER( TYPE_DBL); }

"void"                     { FILTER( TYPE_VOID); }

"true"                     { FILTER( TRUETOKEN); }
"false"                    { FILTER( FALSETOKEN); }
"&&"                       { FILTER( LAZYAND); }
"||"                       { FILTER( LAZYOR); }
"?"                        { FILTER( QUESTION); }  /* needed for conditional expressions */

"const"                    { FILTER( C_KEYWORD); }
"auto"                     { FILTER( C_KEYWORD); }
"static"                   { FILTER( C_KEYWORD); }
"extern"                   { FILTER( C_KEYWORD); }
"struct"                   { FILTER( C_KEYWORD); }
"union"                    { FILTER( C_KEYWORD); }
"volatile"                 { FILTER( C_KEYWORD); }
"case"                     { FILTER( C_KEYWORD); }
"switch"                   { FILTER( C_KEYWORD); }
"goto"                     { FILTER( C_KEYWORD); }
"continue"                 { FILTER( C_KEYWORD); }
"break"                    { FILTER( C_KEYWORD); }

"target"                   { if( global.compiler_phase==PH_initial) {
                               FILTER( TARGET);
                             } else {
                               yylval.id = STRcpy(yytext);
                               FILTER( ID);
                             }
                           }
"default"                  { if( global.compiler_phase==PH_initial) {
                               yylval.id = STRcpy(yytext);
                               FILTER( ID);
                             } else {
                               FILTER( C_KEYWORD);
                             }
                           }

[-/][_A-Za-z0-9]*          { if( global.compiler_phase == PH_initial) {
                               yylval.id = STRcpy( yytext);
                               FILTER( OPTION);
                             } else {
                               switch (yytext[0]) {
                                 case '-':
                                   yyless( 1);
                                   FILTER( MINUS);
                                   break;
                                 case '/':
                                   yyless( 1);
                                   yylval.id = STRcpy( yytext);
                                   FILTER( ID);
                                   break;
                                 case '_':
                                   yylval.id = STRcpy( yytext);
                                   FILTER( ID);
                                   break;
                                 default:
                                   yylval.id = STRcpy( yytext);
                                   FILTER( ID);
                               }
                             }
                           }


\"([^"\n]|(\\\"))*\"       { yylval.id = STRcpy( yytext + sizeof( char));
                             yylval.id[ strlen( yylval.id) - 1] = '\0';
                             FILTER( STR);
                           }

[A-Za-z]([A-Za-z0-9]|[_A-Za-z0-9][A-Za-z0-9])*_? {
                             yylval.id = STRcpy( yytext);
                             FILTER( ID);
                           }
[|&>!%=-]+                 { yylval.id = STRcpy(yytext);
                             FILTER( ID);
                           }

[_A-Za-z][_A-Za-z0-9]*     { yylval.id = STRcpy(yytext);
                             FILTER( ID);
                           }

'.'                        { yylval.cchar=yytext[1];
                             FILTER( CHAR);
                           }
'\\[0-7][0-7]?[0-7]?'        { yylval.cchar=(char)strtol(yytext+2, NULL, 8);
                             FILTER( CHAR);
                           }
'\\x[0-9a-f][0-9a-f]?'      { yylval.cchar=(char)strtol(yytext+3, NULL, 16);
                             FILTER( CHAR);
                           }
'\\n'                      { yylval.cchar='\n';
                             FILTER( CHAR);
                           }
'\\t'                      { yylval.cchar='\t';
                             FILTER( CHAR);
                           }

'\\v'                      { yylval.cchar='\v';
                             FILTER( CHAR);
                           }

'\\b'                      { yylval.cchar='\b';
                             FILTER( CHAR);
                           }

'\\r'                      { yylval.cchar='\r';
                             FILTER( CHAR);
                           }

'\\f'                      { yylval.cchar='\f';
                             FILTER( CHAR);
                           }

'\\a'                      { yylval.cchar='\a';
                             FILTER( CHAR);
                           }

'\\e'                      { yylval.cchar='\33';
                             FILTER( CHAR);
                           }

'\\\''                      { yylval.cchar='\47';
                             FILTER( CHAR);
                           }

'\\\\'                     { yylval.cchar='\\';
                             FILTER( CHAR);
                           }

[0-9]+                     { yylval.cint=atoi(yytext);
                             FILTER( NUM);
                           }

[0-9]*[.][0-9]+(e([+]?|[-]?)[0-9]+)? {
                             yylval.cdbl=atof(yytext);
                             FILTER( DOUBLE);
                           }

[0-9]*([.][0-9]+(e([+]?|[-]?)[0-9]+)?)?[fF] {
                             yytext[strlen(yytext)]='\0';
                             yylval.cfloat=atof(yytext);
                             FILTER( FLOAT);
                           }

[0-9]*([.][0-9]+(e([+]?|[-]?)[0-9]+)?)?[Dd] {
                             yytext[strlen(yytext)]='\0';
                             yylval.cdbl=atof(yytext);
                             FILTER( DOUBLE);
                           }


"/*"                       { if (commlevel == 1) {
                               CTIwarnLine( global.linenum, "Nested comment eleminated");
                             }
                             commlevel++;
                             charpos += yyleng;
                           }
"*/"                       { charpos += yyleng;
                             if (commlevel > 0) {
                               commlevel--;
                             }
                             else {
                               return(SC);
                             }
                           }

\n.*                       { global.linenum++;
                             charpos = 0;
                             if (yyleng <= LINEBUF_SIZE) {
                               strncpy( linebuf, yytext+1,yyleng);
                               linebuf_ptr = linebuf;
                             }
                             else {
                               MEMfree( tmp_buf);
                               tmp_buf = MEMmalloc( (yyleng) * sizeof(char));
                               strncpy( tmp_buf, yytext+1,yyleng);
                               linebuf_ptr = tmp_buf;
                             }
                             yyless(1);
                           }

[ \t]                      { charpos += yyleng; }

"\f"                       { charpos += yyleng; }

#\ *[0-9]+\ \".*\"\ [0-9]* { charpos += yyleng;
                             if( commlevel == 0) {
                               int start, len;

                               global.linenum=atoi(yytext+1)-1;
                               start = strcspn( yytext, "\"")+1;
                               len   = strcspn( yytext + start, "\"");
                               if( ((yytext + start)[0] == '.')
                                     && ((yytext + start)[1] == '/') ) {
                                 /* elminate preceeding "./" */
                                 start += 2;
                                 len -= 2;
                               }
                               /* 
                                * global.filename will be used as a shared 
                                * string in all node structures
                                */
                               global.filename = (char *)MEMmalloc( (len+1)*sizeof(char));
                               CHKMdoNotReport( global.filename);
                               strncpy( global.filename , yytext + start, len);
                               global.filename[len] = '\0';
                               DBUG_PRINT( "PARSE", ("# %i %s" , global.linenum, global.filename));
                             }
                           }

#\ *[0-9]+\ \".*\"         { charpos += yyleng;
                             if( commlevel == 0) {
                               int start, len;

                               global.linenum=atoi(yytext+1)-1;
                               start = strcspn( yytext, "\"")+1;
                               len   = strcspn( yytext + start, "\"");
                               if( ((yytext + start)[0] == '.')
                                     && ((yytext + start)[1] == '/') ) {
                                 /* elminate preceeding "./" */
                                 start += 2;
                                 len -= 2;
                               }
                               /* 
                                * global.filename will be used as a shared 
                                * string in all node structures
                                */
                               global.filename = (char *)MEMmalloc( (len+1)*sizeof(char));
                               CHKMdoNotReport( global.filename);
                               strncpy( global.filename , yytext + start, len);
                               global.filename[len] = '\0';
                               DBUG_PRINT( "PARSE", ("# %i %s" , global.linenum, global.filename));
                             }
                           }

#\ *[0-9]+                 { charpos += yyleng;
                             if( commlevel == 0) {
                               global.linenum=atoi(yytext+1)-1;
                               DBUG_PRINT( "PARSE", ("# %i" , global.linenum));
                             }
                           }

.                          { charpos += yyleng;
                             FILTER( SC);
                           }


%%
