%{

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
  			 yylval.nums=(nums *)malloc(sizeof(nums));     \
                         yylval.nums->num=atoi(yytext);		       \
                         yylval.nums->next=NULL; 	               \
                         return(token); }

extern int indent;
extern int i;

char * tmp;
int linenum=1;
int commlevel=0;

%}
%%
"{"				{FILTER( BRACE_L );}
"}"				{FILTER( BRACE_R );}
"("				{FILTER( BRACKET_L );}
")"				{FILTER( BRACKET_R );}
"["				{FILTER( SQBR_L );}
"]"				{FILTER( SQBR_R );}
";"				{FILTER( COLON );}
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

"main"				{FILTER( MAIN );}
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

"int"				{FILTER( TYPE_INT );}
"float"				{FILTER( TYPE_FLOAT );}
"bool"				{FILTER( TYPE_BOOL );}


[_A-Za-z][_A-Za-z0-9]*		{FILTER2( ID );}
(0?|([1-9][0-9]*))	{FILTER ( NUM);}
(0?|([1-9][0-9]*))[.][0-9]*(e([+]?|[-]?)[1-9][0-9]*)? \
				{FILTER( FLOAT);}

"/*"	{ if (commlevel++ == 1)
            fprintf(stderr,"Warning: recursive comment eleminated! \n");
          else printf("/*"); }
"*/"	{ if (commlevel) commlevel--; else return(SC);
          if (commlevel == 0) {printf("*/\n"); for(i=0; i<indent; i++) printf(" ");} }
"\n"	{linenum++; if (commlevel) printf("\n");}
[ \t]	if (commlevel) ECHO;
.	{FILTER( SC ); }

%%

