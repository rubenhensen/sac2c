%{

/*
 *
 * $Log$
 * Revision 1.10  1995/01/06 17:51:45  sbs
 * no_mod_ext pragma inserted
 *
 * Revision 1.9  1995/01/05  12:46:51  sbs
 * string inserted
 *
 * Revision 1.8  1994/12/16  14:34:28  sbs
 * moddec  and start_token inserted
 *
 * Revision 1.7  1994/12/14  10:08:40  sbs
 * typedef inserted
 *
 * Revision 1.6  1994/12/08  17:45:58  sbs
 * modules inserted
 *
 * Revision 1.5  1994/11/23  09:44:03  hw
 * inserted true & false
 *
 * Revision 1.4  1994/11/10  15:10:01  sbs
 * RCS-header adjusted
 *
 * Revision 1.3  1994/11/10  14:53:24  sbs
 * RCS-header inserted
 *
 *
 */

#include <stdlib.h>
#include "tree.h"
#include "y.tab.h"

#define FILTER(token) if (commlevel) ECHO; else return(token)
#define FILTER2(token) if (commlevel) ECHO;                             \
                       else {                                           \
                         yylval.id=(char *)malloc(strlen(yytext));      \
                         strcpy(yylval.id, yytext);  	                \
                         return(token); }
#define FILTER3(token) if (commlevel) ECHO;                            \
                       else {                                          \
                         yylval.cint=atoi(yytext);		       \
                         return(token); }
#define FILTER4(token) if (commlevel) ECHO;                            \
                       else {                                          \
                         yylval.cfloat=atof(yytext);		       \
                         return(token); }

extern int indent;
extern int i;

char * tmp;
int linenum=1;
int commlevel=0;
int start_token;

%}
%%
%{

/* the next view lines allow for smuggling in a start_token! */

     if(start_token) {
       int mem=start_token;

       start_token=0;
       return(mem);
     }
%}

"{"				{FILTER( BRACE_L );}
"}"				{FILTER( BRACE_R );}
"("				{FILTER( BRACKET_L );}
")"				{FILTER( BRACKET_R );}
"["				{FILTER( SQBR_L );}
"]"				{FILTER( SQBR_R );}
":"				{FILTER( COLON );}
";"				{FILTER( SEMIC );}
","				{FILTER( COMMA );}
"="				{FILTER( LET );}

"&&"				{FILTER( AND );}
"||"				{FILTER( OR );}
"=="				{FILTER( EQ );}
"!="				{FILTER( NEQ );}
"<="				{FILTER( LE );}
"<"				{FILTER( LT );}
">="				{FILTER( GE );}
">"				{FILTER( GT );}
"+"				{FILTER( PLUS);}
"-"				{FILTER( MINUS);}
"/"				{FILTER( DIV );}
"*"				{FILTER( MUL );}

"++"				{FILTER( INC );}
"--" 				{FILTER( DEC );}

"+="				{FILTER( ADDON );}
"-="				{FILTER( SUBON );}
"*="				{FILTER( MULON );}
"/="				{FILTER( DIVON );}


"reshape"			{FILTER( RESHAPE );}
"shape"				{FILTER( SHAPE );}
"take"				{FILTER( TAKE );}
"drop"				{FILTER( DROP );}
"dim"				{FILTER( DIM );}
"rotate"			{FILTER( ROTATE );}
"cat"                           {FILTER( CAT );}
"psi"                           {FILTER( PSI );}

"typedef"			{FILTER( TYPEDEF );}
"main"				{FILTER( K_MAIN );}
"return"			{FILTER( RETURN );}
"if"				{FILTER( IF );}
"else"				{FILTER( ELSE );}
"do"				{FILTER( DO );}
"while"				{FILTER( WHILE );}
"for"				{FILTER( FOR );}
"with" 				{FILTER( WITH );}
"genarray"			{FILTER( GENARRAY );}
"modarray"			{FILTER( MODARRAY );}
"inline"			{FILTER( INLINE );}

[mM]odul[dD]ec			{FILTER( MODDEC );}
[mM]odul			{FILTER( MODIMP );}
[cC]lass[dD]ec			{FILTER( CLASSDEC );}
"import"			{FILTER( IMPORT );}
"all"				{FILTER( ALL );}
"implicit"			{FILTER( IMPLICIT );}
"explicit"			{FILTER( EXPLICIT );}
"types"				{FILTER( TYPES );}
"funs"				{FILTER( FUNS );}
"own"				{FILTER( OWN );}

"int"				{FILTER( TYPE_INT );}
"float"				{FILTER( TYPE_FLOAT );}
"bool"				{FILTER( TYPE_BOOL );}
"string"			{FILTER( TYPE_STR );}

"true"                          {FILTER( TRUE );}
"false"                         {FILTER( FALSE );}

^#pragma[ ]*no_mod_ext[ ]*$     {FILTER( PRAGMA_NO_MOD_EXT );}

\"[^"\n]*\"			{FILTER2( STR );}
[_A-Za-z][_A-Za-z0-9]*		{FILTER2( ID );}
(0?|([1-9][0-9]*))	{FILTER3 ( NUM);}
(0?|([1-9][0-9]*))[.][0-9]*(e([+]?|[-]?)[1-9][0-9]*)? \
				{FILTER4 ( FLOAT);}

"/*"	{ if (commlevel++ == 1)
            fprintf(stderr,"Warning: recursive comment eleminated! \n");
          else printf("/*"); }
"*/"	{ if (commlevel) commlevel--; else return(SC);
          if (commlevel == 0) {printf("*/\n"); for(i=0; i<indent; i++) printf(" ");} }
"\n"	{linenum++; if (commlevel) printf("\n");}
[ \t]	if (commlevel) ECHO;
.	{FILTER( SC ); }

%%

