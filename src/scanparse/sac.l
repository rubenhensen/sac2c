%{

/*
 *
 * $Log$
 * Revision 2.7  1999/10/04 09:24:07  sbs
 * linenum moved to globals.[ch]
 *
 * Revision 2.6  1999/05/07 10:00:41  sbs
 * rules with overlength changed; they now contain { instead of being
 * terminated by \.
 *
 * Revision 2.5  1999/05/06 15:36:55  sbs
 * the global var filename now is copied whenever yyparse is called
 * or a line-pragma is found. This allows to attach the filename
 * to EACH node!!
 *
 * Revision 2.4  1999/05/05 13:52:56  sbs
 * streamlined the entire file....
 * no more FILTERxxx macros anymore...
 * support for charpos and linebuf_ptr added which
 * enables much better error reporting during scanning/parsing....
 *
 * Revision 2.3  1999/04/19 13:07:28  jhs
 * Renamed TRUE and FALSE token to TRUETOKEN and FALSETOKEN to
 * avoid conflicts with TRUE and FALSE macros.
 *
 * Revision 2.2  1999/04/13 14:00:40  cg
 * added #pragma cachesim.
 *
 * Revision 2.1  1999/02/23 12:40:30  sacbase
 * new release made
 *
 * Revision 1.57  1999/01/19 16:16:03  cg
 * added the '?' for types that match both scalars and arrays.
 *
 * Revision 1.56  1998/12/02 16:25:33  cg
 * some bugs fixed: primitive function tokens are now returned without
 * the name associated thus closing a memory leak since these names are
 * usually not needed for the syntax tree.
 *
 * Revision 1.55  1998/10/30 09:52:16  cg
 * key word 'all' is now allowed as function name,
 * required by array library.
 *
 * Revision 1.54  1998/10/26 12:51:29  cg
 * key words exchanged:
 * new_with -> with
 * with     -> old_with
 * The new with-loop is now the default.
 *
 * Revision 1.53  1998/04/09 21:32:40  dkr
 * new tokens DEFAULT, WLCOMP for wlcomp-pragmas
 *
 * Revision 1.52  1998/03/04 16:24:27  cg
 * Bug fixed in scanning configuration file resource 'cat'
 * (same as primitive array function), fixed by new filter
 *
 * Revision 1.51  1998/02/27 16:30:05  cg
 * added lexical analysis for sac2crc files
 *
 * Revision 1.50  1997/11/24 18:06:40  sbs
 * new_with inserted as NWITH token!
 *
 * Revision 1.49  1997/10/28 10:35:10  dkr
 *  a few <cr>s in lex-rules have been removed
 *  because some (f)lex-implementations can not handle them.
 *
 * Revision 1.48  1997/10/07 13:27:21  srs
 * new primitive functions min and max
 *
 * Revision 1.47  1997/10/05 13:31:58  dkr
 * a few <cr>s in lex-rules have been removed
 * because some (f)lex-implementations can not handle them.
 *
 * Revision 1.46  1997/10/03 17:57:27  dkr
 * added prf abs()
 *
 * Revision 1.44  1997/10/01 12:32:02  dkr
 * added % and %=
 *
 * Revision 1.41  1997/08/04 19:09:30  dkr
 * added strcpy in FILTER7
 *
 * Revision 1.40  1997/06/03 13:51:54  sbs
 * # line - capability integrated into sac2c.
 * Now, the line-number in error messages should be correct 8-)))
 *
 *
 *
 *
 * Revision 1.3  1994/11/10  14:53:24  sbs
 * RCS-header inserted
 *
 *
 */



#include <stdlib.h>
#include "tree.h"
#include "free.h"
#include "resource.h"
#include "y.tab.h"
#include "Error.h"
#include "internal_lib.h"
#include "globals.h"


#define FILTER(token) 	charpos += yyleng; \
			if (0==commlevel)  return(token)

#define LINEBUF_SIZE 256

extern int indent;
extern int i;
 
char linebuf[LINEBUF_SIZE];
char *linebuf_ptr;
char *tmp_buf=NULL;
int charpos=0;
int commlevel=0;
int start_token;


%}

%p 6000 /* set positions to 6000 */
%e 2000 /* set nodes to 1100 */

%%
%{

/* the next few lines allow for smuggling in a start_token! */

     if(start_token) {
       int mem=start_token;

       start_token=0;
       return(mem);
     }
%}

"{"				{ FILTER( BRACE_L );}
"}"				{ FILTER( BRACE_R );}
"("				{ FILTER( BRACKET_L );}
")"				{ FILTER( BRACKET_R );}
"["				{ FILTER( SQBR_L );}
"]"				{ FILTER( SQBR_R );}
":"				{ FILTER( COLON );}
";"				{ FILTER( SEMIC );}
","				{ FILTER( COMMA );}
"="				{ FILTER( LET );}
"&"				{ FILTER( AMPERS );}
"?"				{ FILTER( QUESTION );}
"."                             { FILTER( DOT);}

"&&"				{ FILTER( AND );}
"||"				{ FILTER( OR );}
"=="				{ FILTER( EQ );}
"!="				{ FILTER( NEQ );}
"!"                             { FILTER( NOT );}
"<="				{ FILTER( LE );}
"<"				{ FILTER( LT );}
">="				{ FILTER( GE );}
">"				{ FILTER( GT );}
"+"				{ FILTER( PLUS );}
"-"				{ FILTER( MINUS );}
"/"				{ FILTER( DIV );}
"*"				{ FILTER( MUL );}
"%"				{ FILTER( PRF_MOD );}

"++"				{ FILTER( INC );}
"--" 				{ FILTER( DEC );}

"+="				{ FILTER( ADDON );}
"-="				{ FILTER( SUBON );}
"*="				{ FILTER( MULON );}
"/="				{ FILTER( DIVON );}
"%="				{ FILTER( MODON );}

"abs"				{ FILTER( ABS );}
"reshape"			{ FILTER( RESHAPE );}
"shape"				{ FILTER( SHAPE );}
"take"				{ FILTER( TAKE );}
"drop"				{ FILTER( DROP );}
"dim"				{ FILTER( DIM );}
"rotate"			{ FILTER( ROTATE );}
"cat"                           { if (compiler_phase==PH_setup) {
                                    yylval.id=StringCopy(yytext);
                                    FILTER( ID );
                                    FREE(yylval.id);
                                  } else {
                                    FILTER( CAT );
                                  }
                                }
"psi"                           { FILTER( PSI );}
"toi"                           { FILTER( TOI );}
"tof"                           { FILTER( TOF );}
"tod"                           { FILTER( TOD );}
"min"                           { FILTER( PRF_MIN );}
"max"                           { FILTER( PRF_MAX );}
"typedef"			{ FILTER( TYPEDEF );}
"classtype"			{ FILTER( CLASSTYPE );}
"objdef"		        { FILTER( OBJDEF );}
"main"				{ FILTER( K_MAIN );}
"return"			{ FILTER( RETURN );}
"if"				{ FILTER( IF );}
"else"				{ FILTER( ELSE );}
"do"				{ FILTER( DO );}
"while"				{ FILTER( WHILE );}
"for"				{ FILTER( FOR );}
"old_with" 		       	{ FILTER( WITH );}
"with" 	  	 	       	{ FILTER( NWITH );}
"step" 				{ FILTER( STEP );}
"width" 			{ FILTER( WIDTH );}
"genarray"			{ FILTER( GENARRAY );}
"modarray"			{ FILTER( MODARRAY );}
"fold"				{ FILTER( FOLD );}
"inline"			{ FILTER( INLINE );}

"ModuleDec"			{ FILTER( MODDEC );}
"Module"		       	{ FILTER( MODIMP );}
"ClassDec"			{ FILTER( CLASSDEC );}
"Class"	                        { FILTER( CLASSIMP );}

"import"			{ FILTER( IMPORT );}
"all"				{ FILTER( ALL );}
"implicit"			{ FILTER( IMPLICIT );}
"explicit"			{ FILTER( EXPLICIT );}
"types"				{ FILTER( TYPES );}
"funs"				{ FILTER( FUNS );}
"functions"			{ FILTER( FUNS );}
"own"				{ FILTER( OWN );}
"external"			{ FILTER( EXTERN );}
"global"                        { FILTER( GLOBAL );}
"objects"                       { FILTER( OBJECTS );}

"#pragma"                       { FILTER( PRAGMA );}
"linkname"                      { FILTER( LINKNAME );}
"linksign"                      { FILTER( LINKSIGN );}
"effect"                        { FILTER( EFFECT );}
"touch"                         { FILTER( TOUCH );}
"refcounting"                   { FILTER( REFCOUNTING );}
"readonly"                      { FILTER( READONLY );}
"copyfun"                       { FILTER( COPYFUN );}
"freefun"                       { FILTER( FREEFUN );}
"initfun"                       { FILTER( INITFUN );}
"linkwith"                      { FILTER( LINKWITH );}
"wlcomp"                        { FILTER( WLCOMP );}
"cachesim"                      { FILTER( CACHESIM );}
"Default"                       { FILTER( DEFAULT );}

"int"				{ FILTER( TYPE_INT );}
"float"				{ FILTER( TYPE_FLOAT );}
"bool"				{ FILTER( TYPE_BOOL );}
"unsigned" 		        { FILTER( TYPE_UNS );}
"short"             		{ FILTER( TYPE_SHORT );}
"long"    		        { FILTER( TYPE_LONG );}
"char"     		        { FILTER( TYPE_CHAR );}
"double"		        { FILTER( TYPE_DBL );}

"void"		                { FILTER( TYPE_VOID );}
"..."		                { FILTER( TYPE_DOTS );}

"true"                          { FILTER( TRUETOKEN );}
"false"                         { FILTER( FALSETOKEN );}

"const"                         { FILTER( C_KEYWORD);}
"auto"                          { FILTER( C_KEYWORD);}
"static"                        { FILTER( C_KEYWORD);}
"extern"                        { FILTER( C_KEYWORD);}
"struct"                        { FILTER( C_KEYWORD);}
"union"                         { FILTER( C_KEYWORD);}
"volatile"                      { FILTER( C_KEYWORD);}
"case"                          { FILTER( C_KEYWORD);}
"switch"                        { FILTER( C_KEYWORD);}
"goto"                          { FILTER( C_KEYWORD);}
"continue"                      { FILTER( C_KEYWORD);}
"break"                         { FILTER( C_KEYWORD);}

"target"                        { if( compiler_phase==PH_setup) {
                                    FILTER( TARGET );
                                  } else {
                                    yylval.id=StringCopy(yytext);
                                    FILTER( ID );
                                    FREE(yylval.id);
                                  }
                                }
"default"                       { if( compiler_phase==PH_setup) {
                                    yylval.id=StringCopy(yytext);
                                    FILTER( ID );
                                    FREE(yylval.id);
                                  } else {
                                    FILTER( C_KEYWORD );
                                  }
                                }

[-/][_A-Za-z0-9]*               { if( compiler_phase==PH_setup) {
                                    yylval.id=StringCopy(yytext);
                                    FILTER( OPTION );
                                    FREE(yylval.id);
                                  } else {
                                    switch (yytext[0]) {
                                    case '-':
                                      yyless(1);
                                      FILTER(MINUS);
                                      break;
                                    case '/':
                                      yyless(1);
                                      FILTER(DIV);
                                      break;
                                    case '_':
                                      yylval.id=StringCopy(yytext);
                                      FILTER( PRIVATEID);
                                      FREE(yylval.id);
                                      break;
                                    default:
                                      yylval.id=StringCopy(yytext);
                                      FILTER( ID);
                                      FREE(yylval.id);
                                    }
                                  }
                                }


\"([^"\n]|(\\\"))*\"		{ yylval.id=StringCopy(yytext+sizeof(char));
                                  yylval.id[strlen(yylval.id)-1]='\0';
                                  FILTER( STR );
                                  FREE(yylval.id);
                                }

[A-Za-z]([A-Za-z0-9]|[_A-Za-z0-9][A-Za-z0-9])* {
                                  yylval.id=StringCopy(yytext);
                                  FILTER( ID );
                                  FREE(yylval.id);
                                }

[_A-Za-z][_A-Za-z0-9]*          { yylval.id=StringCopy(yytext);
                                  FILTER( PRIVATEID );
                                  FREE(yylval.id);
                                }

'.'                             { yylval.cchar=yytext[1];
                                  FILTER( CHAR );
                                }
'\\[0-9]*'                      { yylval.cchar=(char)atoi(yytext+2);
                                  FILTER( CHAR );
                                }
'\\n'                          { yylval.cchar='\n';
                                 FILTER( CHAR );
                               }
'\\t'                          { yylval.cchar='\t';
                                 FILTER( CHAR );
                               }

(0?|([1-9][0-9]*))             { yylval.cint=atoi(yytext);
                                 FILTER( NUM);
                               }

(0?|([1-9][0-9]*))[.][0-9]+(e([+]?|[-]?)[0-9]+)? {
                                 yylval.cdbl=atof(yytext);
                                 FILTER( DOUBLE);
                               }

(0?|([1-9][0-9]*))([.][0-9]+(e([+]?|[-]?)[0-9]+)?)?[fF] {
                                 yytext[strlen(yytext)]='\0';
                                 yylval.cfloat=atof(yytext);
                                 FILTER( FLOAT);
                               }

(0?|([1-9][0-9]*))([.][0-9]+(e([+]?|[-]?)[0-9]+)?)?[Dd] {
                                 yytext[strlen(yytext)]='\0';
                                 yylval.cdbl=atof(yytext);
                                 FILTER( DOUBLE);
                               }


"/*"	                      { if (commlevel++ == 1)
                                  WARN(linenum, ("Nested comment eleminated"));
                                charpos += yyleng;
                               }
"*/"	                      { charpos +=yyleng;
                                if (commlevel) commlevel--;
                                else return(SC);
                              }
\n.*	                      { linenum++;
                                charpos = 0;
                                if( yyleng <= LINEBUF_SIZE) {
                                  strncpy( linebuf, yytext+1,yyleng);
                                  linebuf_ptr = linebuf;
                                } else {
                                  if( tmp_buf != NULL)
                                    FREE( tmp_buf);
                                  tmp_buf = MALLOC( (yyleng) * sizeof(char));
                                  strncpy( tmp_buf, yytext+1,yyleng);
                                  linebuf_ptr = tmp_buf;
                                }
                                yyless(1);
                              }
[ \t]	                      { charpos += yyleng;}
"\f"                          { charpos += yyleng;}
#\ .*\n                       { linenum=atoi(yytext+2);
                                { int start, len;
                                  start = strcspn( yytext, "\"")+1;
                                  len   = strcspn( yytext+start, "\"");
                                  filename = (char *)Malloc( (len+1)*sizeof(char)); 
                                  strncpy( filename , yytext + start, len);
                                  filename[len] = '\0';
                                }
                              }
.	                      { charpos += yyleng;
                                FILTER( SC );
                              }

%%


