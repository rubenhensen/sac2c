%{

/*
 *
 * $Log$
 * Revision 3.16  2002/08/14 09:55:23  sbs
 * Free s on ID tokens eliminated since that code in fact never was and never should
 * be reached!!!!!
 *
 * Revision 3.15  2002/08/13 14:04:13  sbs
 * several minor bugs fixed
 *
 * Revision 3.14  2002/08/13 10:29:53  sbs
 * completely new scanner / parser built!
 * supports N_mop's (user defined infix operators),
 * supports new primitive operators (_add_xxx)
 * builds N_ap for old prfs such as sel, +, etc.pp.
 * supports dots in any legal expr position
 * handles signs correctly
 * allows for vardecs with array types of user defined types.
 *
 * Revision 3.13  2002/08/06 15:52:11  sbs
 * _add_SxS_ and friends (needed for the newTS ) added.
 *
 * Revision 3.12  2002/07/02 15:23:12  sah
 * nothing changed.
 *
 * Revision 3.11  2001/11/13 23:47:23  dkr
 * rule for # lines modified:
 * some preprocessors do NOT insert a space between # and the line number!!!
 *
 * Revision 3.9  2001/06/28 13:06:17  sbs
 * flex smuggles in a hash filename line at the beginning of each file!!
 * Unfortunately, on some systems this is preceede by dot-slash.
 * In order to achieve consistent output, dot-slash's at the
 * beginning are filtered out now!
 *
 * Revision 3.8  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.7  2001/05/22 14:08:39  dkr
 * minor changes done
 *
 * Revision 3.6  2001/05/18 09:26:43  cg
 * MALLOC and FREE transformed into Malloc and Free.
 *
 * Revision 3.5  2001/03/22 20:10:22  dkr
 * include of tree.h eliminated
 *
 * Revision 3.4  2001/03/21 11:24:33  dkr
 * token DEFAULT removed
 *
 * Revision 3.3  2000/12/06 12:48:03  sbs
 * ugly hack for avoiding warning that yy_cp or yy_bp might be used un-initialized.
 *
 * Revision 3.2  2000/11/24 11:58:30  sbs
 * YY_NO_UNPUT defined for avoiding warning message ....
 *
 * Revision 3.1  2000/11/20 17:59:48  sacbase
 * new release made
 *
 * [...]
 *
 * Revision 1.3  1994/11/10  14:53:24  sbs
 * RCS-header inserted
 *
 */



#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "resource.h"
#include "y.tab.h"
#include "Error.h"
#include "globals.h"


#define FILTER(token) \
  charpos += yyleng;  \
  if (commlevel == 0) { return( token); }

#define LINEBUF_SIZE 256
#define YY_NO_UNPUT

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

#ifdef SAC_FOR_SOLARIS_SPARC
/* the next two lines are a very dirty hack for avoiding a gcc-warning 8-(( */
  yy_cp = NULL;
  yy_bp = NULL;
#endif

/* the next few lines allow for smuggling in a start_token! */
     if (start_token) {
       int mem=start_token;
#ifdef OLD_VERSION_OF_LEX  /* (previously needed on ALPHA) */
       yy_init = 1;  /* force new initialization of lex! */
#endif
       start_token = 0;
       return( mem);
     }
%}


"{"                        { FILTER( BRACE_L ); }
"}"                        { FILTER( BRACE_R ); }
"("                        { FILTER( BRACKET_L ); }
")"                        { FILTER( BRACKET_R ); }
"["                        { FILTER( SQBR_L ); }
"]"                        { FILTER( SQBR_R ); }
":"                        { FILTER( COLON ); }
";"                        { FILTER( SEMIC ); }
","                        { FILTER( COMMA ); }
"="                        { FILTER( LET ); }
"."                        { FILTER( DOT); }

"&"                        { FILTER( AMPERS);} /* needed for refernce params */
"!"                        { FILTER( EXCL); }  /* needed for monop supp      */
"~"                        { FILTER( TILDE); } /* needed for monop supp      */
"-"                        { FILTER( MINUS); } /* needed for monop supp      */
"+"                        { FILTER( PLUS); }  /* needed for AUDGZ types     */
"*"                        { FILTER( STAR); }  /* needed for AUD types       */
"++"                       { FILTER( INC ); }  /* needed for increment       */
"--"                       { FILTER( DEC ); }  /* needed for decrement       */
"<="                       { FILTER( LE);   }  /* needed for WL generators   */
"<"                        { FILTER( LT);   }  /* needed for WL generators   */
">"                        { FILTER( GT);   }  /* needed for SIBs            */

"+="                       { FILTER( ADDON ); }
"-="                       { FILTER( SUBON ); }
"*="                       { FILTER( MULON ); }
"/="                       { FILTER( DIVON ); }
"%="                       { FILTER( MODON ); }
"->"                       { FILTER( ARROW ); }

"_reshape_"                { FILTER( RESHAPE ); }
"_dim_"                    { FILTER( DIM); }
"_shape_"                  { FILTER( SHAPE); }
"_sel_S_"                  { FILTER( SEL); }

"_add_SxS_"                { FILTER( ADD_SxS); }
"_add_SxA_"                { FILTER( ADD_SxA); }
"_add_AxS_"                { FILTER( ADD_AxS); }
"_add_AxA_"                { FILTER( ADD_AxA); }

"_sub_SxS_"                { FILTER( SUB_SxS); }
"_sub_SxA_"                { FILTER( SUB_SxA); }
"_sub_AxS_"                { FILTER( SUB_AxS); }
"_sub_AxA_"                { FILTER( SUB_AxA); }

"_mul_SxS_"                { FILTER( MUL_SxS); }
"_mul_SxA_"                { FILTER( MUL_SxA); }
"_mul_AxS_"                { FILTER( MUL_AxS); }
"_mul_AxA_"                { FILTER( MUL_AxA); }

"_div_SxS_"                { FILTER( DIV_SxS); }
"_div_SxA_"                { FILTER( DIV_SxA); }
"_div_AxS_"                { FILTER( DIV_AxS); }
"_div_AxA_"                { FILTER( DIV_AxA); }

"_eq_SxS_"                 { FILTER( EQ_SxS); }
"_neq_SxS_"                { FILTER( NEQ_SxS); }
"_le_SxS_"                 { FILTER( LE_SxS); }
"_lt_SxS_"                 { FILTER( LT_SxS); }
"_ge_SxS_"                 { FILTER( GE_SxS); }
"_gt_SxS_"                 { FILTER( GT_SxS); }

"_toi_"                    { FILTER( TOI ); }
"_tof_"                    { FILTER( TOF ); }
"_tod_"                    { FILTER( TOD ); }
"_min_"                    { FILTER( PRF_MIN ); }
"_max_"                    { FILTER( PRF_MAX ); }
"_abs_"                    { FILTER( ABS ); }


"typedef"                  { FILTER( TYPEDEF ); }
"classtype"                { FILTER( CLASSTYPE ); }
"objdef"                   { FILTER( OBJDEF ); }
"main"                     { FILTER( K_MAIN ); }
"return"                   { FILTER( RETURN ); }
"if"                       { FILTER( IF ); }
"else"                     { FILTER( ELSE ); }
"do"                       { FILTER( DO ); }
"while"                    { FILTER( WHILE ); }
"for"                      { FILTER( FOR ); }
"with"                     { FILTER( NWITH ); }
"step"                     { FILTER( STEP ); }
"width"                    { FILTER( WIDTH ); }
"genarray"                 { FILTER( GENARRAY ); }
"modarray"                 { FILTER( MODARRAY ); }
"fold"                     { FILTER( FOLD ); }
"inline"                   { FILTER( INLINE ); }

"ModuleDec"                { FILTER( MODDEC ); }
"ModuleSpec"               { FILTER( MODSPEC ); }
"Module"                   { FILTER( MODIMP ); }
"ClassDec"                 { FILTER( CLASSDEC ); }
"Class"                    { FILTER( CLASSIMP ); }

"import"                   { FILTER( IMPORT ); }
"all"                      { FILTER( ALL ); }
"implicit"                 { FILTER( IMPLICIT ); }
"explicit"                 { FILTER( EXPLICIT ); }
"types"                    { FILTER( TYPES ); }
"funs"                     { FILTER( FUNS ); }
"functions"                { FILTER( FUNS ); }
"own"                      { FILTER( OWN ); }
"external"                 { FILTER( EXTERN ); }
"global"                   { FILTER( GLOBAL ); }
"objects"                  { FILTER( OBJECTS ); }

"#pragma"                  { FILTER( PRAGMA ); }
"linkname"                 { FILTER( LINKNAME ); }
"linksign"                 { FILTER( LINKSIGN ); }
"effect"                   { FILTER( EFFECT ); }
"touch"                    { FILTER( TOUCH ); }
"refcounting"              { FILTER( REFCOUNTING ); }
"readonly"                 { FILTER( READONLY ); }
"copyfun"                  { FILTER( COPYFUN ); }
"freefun"                  { FILTER( FREEFUN ); }
"initfun"                  { FILTER( INITFUN ); }
"linkwith"                 { FILTER( LINKWITH ); }
"wlcomp"                   { FILTER( WLCOMP ); }
"cachesim"                 { FILTER( CACHESIM ); }
"specialize"               { FILTER( SPECIALIZE ); }

"int"                      { FILTER( TYPE_INT ); }
"float"                    { FILTER( TYPE_FLOAT ); }
"bool"                     { FILTER( TYPE_BOOL ); }
"unsigned"                 { FILTER( TYPE_UNS ); }
"short"                    { FILTER( TYPE_SHORT ); }
"long"                     { FILTER( TYPE_LONG ); }
"char"                     { FILTER( TYPE_CHAR ); }
"double"                   { FILTER( TYPE_DBL ); }

"void"                     { FILTER( TYPE_VOID ); }

"true"                     { FILTER( TRUETOKEN ); }
"false"                    { FILTER( FALSETOKEN ); }

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

"target"                   { if( compiler_phase==PH_setup) {
                               FILTER( TARGET );
                             } else {
                               yylval.id=StringCopy(yytext);
                               FILTER( ID );
                             }
                           }
"default"                  { if( compiler_phase==PH_setup) {
                               yylval.id=StringCopy(yytext);
                               FILTER( ID );
                             } else {
                               FILTER( C_KEYWORD );
                             }
                           }

[-/][_A-Za-z0-9]*          { if( compiler_phase==PH_setup) {
                               yylval.id=StringCopy(yytext);
                               FILTER( OPTION );
                             } else {
                               switch (yytext[0]) {
                                 case '-':
                                   yyless(1);
                                   FILTER(MINUS);
                                   break;
                                 case '/':
                                   yyless(1);
                                   yylval.id=StringCopy(yytext);
                                   FILTER( ID);
                                   break;
                                 case '_':
                                   yylval.id=StringCopy(yytext);
                                   FILTER( PRIVATEID);
                                   break;
                                 default:
                                   yylval.id=StringCopy(yytext);
                                   FILTER( ID);
                               }
                             }
                           }


\"([^"\n]|(\\\"))*\"       { yylval.id=StringCopy(yytext+sizeof(char));
                             yylval.id[strlen(yylval.id)-1]='\0';
                             FILTER( STR );
                           }

[A-Za-z]([A-Za-z0-9]|[_A-Za-z0-9][A-Za-z0-9])*_? {
                             yylval.id=StringCopy(yytext);
                             FILTER( ID );
                           }
[|&>!%=-]+           { yylval.id=StringCopy(yytext);
                             FILTER( ID );
                           }

[_A-Za-z][_A-Za-z0-9]*     { yylval.id=StringCopy(yytext);
                             FILTER( PRIVATEID );
                           }

'.'                        { yylval.cchar=yytext[1];
                             FILTER( CHAR );
                           }
'\\[0-9]+'                 { yylval.cchar=(char)atoi(yytext+2);
                             FILTER( CHAR );
                           }
'\\n'                      { yylval.cchar='\n';
                             FILTER( CHAR );
                           }
'\\t'                      { yylval.cchar='\t';
                             FILTER( CHAR );
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
                               WARN( linenum, ("Nested comment eleminated"));
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

\n.*                       { linenum++;
                             charpos = 0;
                             if (yyleng <= LINEBUF_SIZE) {
                               strncpy( linebuf, yytext+1,yyleng);
                               linebuf_ptr = linebuf;
                             }
                             else {
                               Free( tmp_buf);
                               tmp_buf = Malloc( (yyleng) * sizeof(char));
                               strncpy( tmp_buf, yytext+1,yyleng);
                               linebuf_ptr = tmp_buf;
                             }
                             yyless(1);
                           }

[ \t]                      { charpos += yyleng; }

"\f"                       { charpos += yyleng; }

#\ *[0-9]+.*\n             { linenum=atoi(yytext+1)-1;
                             { int start, len;
                               start = strcspn( yytext, "\"")+1;
                               len   = strcspn( yytext + start, "\"");
                               if( ((yytext + start)[0] == '.')
                                     && ((yytext + start)[1] == '/') ) {
                                 /* elminate preceeding "./" */
                                 start += 2;
                                 len -= 2;
                               }
                               filename = (char *)Malloc( (len+1)*sizeof(char));
                               strncpy( filename , yytext + start, len);
                               filename[len] = '\0';
                               DBUG_PRINT( "PARSE", ("# %i %s" , linenum, filename));
                             }
                             yyless(yyleng-1);
                           }

.                          { charpos += yyleng;
                             FILTER( SC );
                           }

%%
