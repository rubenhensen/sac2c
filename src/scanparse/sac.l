%{

/*
 *
 * $Log$
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
#include "resource.h"
#include "y.tab.h"
#include "Error.h"
#include "internal_lib.h"


#define FILTER(token) if (0==commlevel)  return(token)

#define FILTER2(token) if (0==commlevel)  {             \
                         yylval.id=StringCopy(yytext);  \
                         return(token); }

#define FILTER3(token) if (0==commlevel) {              \
                         yylval.cint=atoi(yytext);      \
                         return(token); }

#define FILTER4(token) if (0==commlevel) {              \
                         yylval.cdbl=atof(yytext);      \
                         return(token); }

#define FILTER5(token) if (0==commlevel) {              \
                         yytext[strlen(yytext)]='\0';   \
                         yylval.cdbl=atof(yytext);      \
                         return(token); }

#define FILTER6(token) if (0==commlevel) {                      \
                         yytext[strlen(yytext)]='\0';           \
                         yylval.cfloat=(float) atof(yytext);    \
                         return(token); }

#define FILTER8(token) if (0==commlevel) {              \
                         yylval.cchar=yytext[1];        \
                         return(token); }
        
#define FILTER9(token) if (0==commlevel)  {                             \
                         yylval.id=StringCopy(yytext+sizeof(char));     \
                         yylval.id[strlen(yylval.id)-1]='\0';           \
                         return(token); }

#define FILTER10(token) if (0==commlevel) {                     \
                         yylval.cchar=(char)atoi(yytext+2);     \
                         return(token); }
        
#define FILTER11(token) if (0==commlevel) {                     \
                              switch(yytext[2]){                \
                              case 'n': yylval.cchar='\n';      \
                                        break;                  \
                              case 't': yylval.cchar='\t';      \
                                        break;                  \
                              }                                 \
                         return(token); }
        

#define FILTER12(token)						\
 if (0==commlevel) {						\
   if (compiler_phase==PH_setup) {		     		\
     return(token);						\
   }								\
   yylval.id=StringCopy(yytext);		      		\
   if (yylval.id[0]=='_') return(PRIVATEID);			\
   else return(ID);						\
 }


#define FILTER13(token)                                         \
 if (0==commlevel) {                                            \
   int i;                                                       \
   if (compiler_phase==PH_setup) {                              \
     yylval.id=StringCopy(yytext);                              \
     return(token);                                             \
   }                                                            \
   switch (yytext[0]) {                                         \
   case '-':                                                    \
     for (i=strlen(yytext)-1; i>0; i--) unput(yytext[i]);       \
     FILTER(MINUS);                                             \
     break;                                                     \
   case '/':                                                    \
     for (i=strlen(yytext)-1; i>0; i--) unput(yytext[i]);       \
     FILTER(DIV);                                               \
     break;                                                     \
   case '_':                                                    \
     FILTER2(PRIVATEID);                                        \
     break;                                                     \
   default:                                                     \
     FILTER2(ID);                                               \
   }                                                            \
 }
 

#define FILTER14(token)                         \
 if (0==commlevel) {                            \
   if (compiler_phase!=PH_setup) {              \
     return(token);                             \
   }                                            \
   yylval.id=StringCopy(yytext);                \
   return(ID);                                  \
 }



#define FILTER15(token)                         \
 if (0==commlevel)  {                           \
   if (compiler_phase==PH_setup) {              \
     yylval.id=StringCopy(yytext);              \
     return(ID);                                \
   }                                            \
   else {                                       \
     return(token);                             \
   }                                            \
 } 
 

extern int indent;
extern int i;
 
char * tmp;
int linenum=1;
int commlevel=0;
int start_token;


%}

%p 6000 /* set positions to 6000 */
%e 1100 /* set nodes to 1100 */

%%
%{

/* the next few lines allow for smuggling in a start_token! */

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
"&"				{FILTER( AMPERS );}
"?"				{FILTER( QUESTION );}
"."                             {FILTER( DOT);}

"&&"				{FILTER( AND );}
"||"				{FILTER( OR );}
"=="				{FILTER( EQ );}
"!="				{FILTER( NEQ );}
"!"                             {FILTER( NOT );}
"<="				{FILTER( LE );}
"<"				{FILTER( LT );}
">="				{FILTER( GE );}
">"				{FILTER( GT );}
"+"				{FILTER( PLUS );}
"-"				{FILTER( MINUS );}
"/"				{FILTER( DIV );}
"*"				{FILTER( MUL );}
"%"				{FILTER( PRF_MOD );}

"++"				{FILTER( INC );}
"--" 				{FILTER( DEC );}

"+="				{FILTER( ADDON );}
"-="				{FILTER( SUBON );}
"*="				{FILTER( MULON );}
"/="				{FILTER( DIVON );}
"%="				{FILTER( MODON );}

"abs"				{FILTER( ABS );}
"reshape"			{FILTER( RESHAPE );}
"shape"				{FILTER( SHAPE );}
"take"				{FILTER( TAKE );}
"drop"				{FILTER( DROP );}
"dim"				{FILTER( DIM );}
"rotate"			{FILTER( ROTATE );}
"cat"                           {FILTER15( CAT );}
"psi"                           {FILTER( PSI );}
"toi"                           {FILTER( TOI );}
"tof"                           {FILTER( TOF );}
"tod"                           {FILTER( TOD );}
"min"                           {FILTER( PRF_MIN );}
"max"                           {FILTER( PRF_MAX );}
"typedef"			{FILTER( TYPEDEF );}
"classtype"			{FILTER( CLASSTYPE );}
"objdef"		        {FILTER( OBJDEF );}
"main"				{FILTER( K_MAIN );}
"return"			{FILTER( RETURN );}
"if"				{FILTER( IF );}
"else"				{FILTER( ELSE );}
"do"				{FILTER( DO );}
"while"				{FILTER( WHILE );}
"for"				{FILTER( FOR );}
"old_with" 		       	{FILTER( WITH );}
"with" 	  	 	       	{FILTER( NWITH );}
"step" 				{FILTER( STEP );}
"width" 			{FILTER( WIDTH );}
"genarray"			{FILTER( GENARRAY );}
"modarray"			{FILTER( MODARRAY );}
"fold"				{FILTER( FOLD );}
"inline"			{FILTER( INLINE );}

"ModuleDec"			{FILTER( MODDEC );}
"Module"		       	{FILTER( MODIMP );}
"ClassDec"			{FILTER( CLASSDEC );}
"Class"	                        {FILTER( CLASSIMP );}

"import"			{FILTER( IMPORT );}
"all"				{FILTER( ALL );}
"implicit"			{FILTER( IMPLICIT );}
"explicit"			{FILTER( EXPLICIT );}
"types"				{FILTER( TYPES );}
"funs"				{FILTER( FUNS );}
"functions"			{FILTER( FUNS );}
"own"				{FILTER( OWN );}
"external"			{FILTER( EXTERN );}
"global"                        {FILTER( GLOBAL );}
"objects"                       {FILTER( OBJECTS );}

"#pragma"                       {FILTER( PRAGMA );}
"linkname"                      {FILTER( LINKNAME );}
"linksign"                      {FILTER( LINKSIGN );}
"effect"                        {FILTER( EFFECT );}
"touch"                         {FILTER( TOUCH );}
"refcounting"                   {FILTER( REFCOUNTING );}
"readonly"                      {FILTER( READONLY );}
"copyfun"                       {FILTER( COPYFUN );}
"freefun"                       {FILTER( FREEFUN );}
"initfun"                       {FILTER( INITFUN );}
"linkwith"                      {FILTER( LINKWITH );}
"wlcomp"                        {FILTER( WLCOMP );}
"Default"                       {FILTER( DEFAULT );}

"int"				{FILTER( TYPE_INT );}
"float"				{FILTER( TYPE_FLOAT );}
"bool"				{FILTER( TYPE_BOOL );}
"unsigned" 		        {FILTER( TYPE_UNS );}
"short"             		{FILTER( TYPE_SHORT );}
"long"    		        {FILTER( TYPE_LONG );}
"char"     		        {FILTER( TYPE_CHAR );}
"double"		        {FILTER( TYPE_DBL );}

"void"		                {FILTER( TYPE_VOID );}
"..."		                {FILTER( TYPE_DOTS );}

"true"                          {FILTER( TRUE );}
"false"                         {FILTER( FALSE );}

"const"                         {FILTER( C_KEYWORD);}
"auto"                          {FILTER( C_KEYWORD);}
"static"                        {FILTER( C_KEYWORD);}
"extern"                        {FILTER( C_KEYWORD);}
"struct"                        {FILTER( C_KEYWORD);}
"union"                         {FILTER( C_KEYWORD);}
"volatile"                      {FILTER( C_KEYWORD);}
"case"                          {FILTER( C_KEYWORD);}
"switch"                        {FILTER( C_KEYWORD);}
"goto"                          {FILTER( C_KEYWORD);}
"continue"                      {FILTER( C_KEYWORD);}
"break"                         {FILTER( C_KEYWORD);}

"target"                        {FILTER12( TARGET );}
"default"                       {FILTER14( C_KEYWORD);}

[-/][_A-Za-z0-9]*               {FILTER13( OPTION );}


\"([^"\n]|(\\\"))*\"		{FILTER9( STR );}

[A-Za-z]([A-Za-z0-9]|[_A-Za-z0-9][A-Za-z0-9])*             {FILTER2( ID );}

[_A-Za-z][_A-Za-z0-9]*          {FILTER2( PRIVATEID );}

'.'                             {FILTER8( CHAR );}
'\\[0-9]*'                      {FILTER10( CHAR );}
'\\[nt]'                        {FILTER11( CHAR );}

(0?|([1-9][0-9]*))              {FILTER3 ( NUM);}

(0?|([1-9][0-9]*))[.][0-9]+(e([+]?|[-]?)[0-9]+)?           {FILTER4 ( DOUBLE);}

(0?|([1-9][0-9]*))([.][0-9]+(e([+]?|[-]?)[0-9]+)?)?[fF]    {FILTER6 ( FLOAT);}

(0?|([1-9][0-9]*))([.][0-9]+(e([+]?|[-]?)[0-9]+)?)?[Dd]    {FILTER5 ( DOUBLE);}


"/*"	{ if (commlevel++ == 1)
            WARN(linenum, ("Nested comment eleminated")); }
"*/"	{ if (commlevel) commlevel--; else return(SC); }
"\n"	{linenum++;}
[ \t]	{}
"\f"    {}
#\ .*\n {linenum=atoi(yytext+2);}
.	{FILTER( SC ); }

%%

