%{

/*
 *
 * $Log$
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
 * Revision 1.39  1997/05/05  07:46:09  cg
 * SIB limit removed
 *
 * Revision 1.38  1997/04/28  11:57:08  cg
 * Syntactic analysis made stricter: ftoi, dtoi, moduledec Moduldec, extern, etc
 * are no longer allowed. Refer to sac_grammar for complete and correct SAC syntax.
 *
 * Revision 1.37  1997/04/25  12:13:52  sbs
 * malloc replaced by Malloc
 *
 * Revision 1.36  1997/03/19  13:44:45  cg
 * new token LINKWITH for respective pragma
 *
 * Revision 1.35  1996/09/09  10:05:29  cg
 * new primitive functions toi, tod, etc. are now treated like the old ones
 *
 * Revision 1.34  1996/05/21  20:16:52  sbs
 * default type for floating numbers set to double! (like in C)
 * and leading zeros for exponents enabled!
 *
 * Revision 1.33  1996/04/02  19:36:41  cg
 * strings are now scanned which contain '\"'
 *
 * Revision 1.32  1996/04/02  13:47:24  cg
 * scanparse converted to new strings: key word string removed
 *
 * Revision 1.31  1996/02/21  18:01:33  cg
 * now, special characters such as '\n' or '\0' are accepted
 *
 * Revision 1.30  1996/02/08  18:05:21  hw
 * added token DOT
 *
 * Revision 1.29  1996/02/06  08:20:44  sbs
 * the scanner now accepts FF's (ctrl-L, bzw. \f in C-Notation)
 *
 * Revision 1.28  1996/01/25  18:40:55  cg
 * added key word classtype
 *
 * Revision 1.27  1996/01/22  17:30:38  cg
 * added new pragma initfun
 *
 * Revision 1.26  1996/01/05  13:13:55  cg
 * modified storage format of strings which are now stored without '"'.
 *
 * Revision 1.25  1995/12/29  10:37:26  cg
 * added new token SIBLIMIT="<###>"
 *
 * Revision 1.24  1995/12/21  15:04:06  cg
 * added new primitive type char
 *
 * Revision 1.23  1995/12/04  16:17:21  hw
 * added primitve functions toi, tof & tod
 *
 * Revision 1.22  1995/12/01  17:15:44  cg
 * added key words for new pragmas
 * added: Command not found.
 * added PRIVATEID to allow identifiers without SAC restrictions
 * which are needed for parsing SIBs
 *
 * Revision 1.21  1995/08/07  10:30:00  cg
 * assignement operator := deleted.
 *
 * Revision 1.20  1995/07/07  16:20:56  hw
 * - added macro FILTER7
 * - exchanged FILTER2 with FILTER7 to generate tokens for
 *    primitive functions
 *
 * Revision 1.19  1995/07/06  17:26:00  cg
 * Modul -> Module, & added
 *
 * Revision 1.18  1995/07/06  16:58:44  hw
 * tokens of primitive functions contain in yyval.id there name now
 *
 * Revision 1.17  1995/07/04  11:13:49  hw
 * - scanning of 'double constants'( e.g. 3d, 3D, 3.0d, 3.0D) inserted
 * - scanning of 'float constants'( e.g. 3f, 3F, 3.0, 3.0F, 3.0f) changed
 * - scanning of primitive functions ifod, ftod, dtoi, dtof inserted.
 *
 * Revision 1.16  1995/06/30  11:09:37  hw
 * - tokens for new primitive functions "ftoi" & "itof" inserted
 * - token C_KEYWORD for not used c-keywords inserted
 * - identifiers have to begin with a letter.
 *   more than one "_" in succession will not be accepted
 *
 * Revision 1.15  1995/05/30  08:21:55  cg
 * sac-grammar version 0.5 tokens inserted.
 *
 * Revision 1.14  1995/05/22  15:03:43  hw
 * comments will not be printed anymore
 *
 * Revision 1.13  1995/02/14  12:00:42  sbs
 * fold inserted
 *
 * Revision 1.12  1995/02/14  10:11:10  hw
 * NOT inserted
 *
 * Revision 1.11  1995/01/06  19:05:51  sbs
 * no_mod_ext pragma deleted & extern declaration inserted
 *
 * Revision 1.10  1995/01/06  17:51:45  sbs
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
#include "Error.h"
#include "internal_lib.h"



#define FILTER(token) if (0==commlevel)  return(token)

#define FILTER2(token) if (0==commlevel)  {                             \
                         yylval.id=(char *)Malloc(sizeof(char)          \
                                                  *(strlen(yytext)+1)); \
                         strcpy(yylval.id, yytext);                     \
                         return(token); }

#define FILTER3(token) if (0==commlevel) {                              \
                         yylval.cint=atoi(yytext);		        \
                         return(token); }

#define FILTER4(token) if (0==commlevel) {                              \
                         yylval.cdbl=atof(yytext);		       \
                         return(token); }

#define FILTER5(token) if (0==commlevel) {                              \
                         yytext[strlen(yytext)]='\0';                   \
                         yylval.cdbl=atof(yytext);                      \
                         return(token); }

#define FILTER6(token) if (0==commlevel) {                              \
                         yytext[strlen(yytext)]='\0';                   \
                         yylval.cfloat=atof(yytext);                    \
                         return(token); }

#define FILTER7(token, name) if (0==commlevel)  {                       \
                               yylval.id=name;                          \
                               yylval.id=(char *)Malloc(sizeof(char)    \
                                                    *(strlen(name)+1)); \
                               strcpy(yylval.id, name);                 \
                               return(token); }

#define FILTER8(token) if (0==commlevel) {                              \
                         yylval.cchar=yytext[1];		        \
                         return(token); }
        
#define FILTER10(token) if (0==commlevel) {                              \
                         yylval.cchar=(char)atoi(yytext+2);              \
                         return(token); }
        
#define FILTER11(token) if (0==commlevel) {             \
                              switch(yytext[2]){   \
                              case 'n': yylval.cchar='\n';  \
                                        break;  \
                              case 't': yylval.cchar='\t';  \
                                        break;   \
                              }  \
                         return(token); }
        
#define FILTER9(token) if (0==commlevel)  {                             \
                         yylval.id=(char *)Malloc(sizeof(char)          \
                                                  *(strlen(yytext)+1)); \
                         strcpy(yylval.id, yytext+sizeof(char));        \
                         yylval.id[strlen(yylval.id)-1]='\0';           \
                         return(token); }


extern int indent;
extern int i;

char * tmp;
int linenum=1;
int commlevel=0;
int start_token;


%}

%p 5000 /* set positions to 5000 */
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
"."                             {FILTER( DOT);}

"&&"				{FILTER7( AND,prf_name_str[F_and]);}
"||"				{FILTER7( OR,prf_name_str[F_or] );}
"=="				{FILTER7( EQ,prf_name_str[F_eq] );}
"!="				{FILTER7( NEQ,prf_name_str[F_neq] );}
"!"                             {FILTER7( NOT,prf_name_str[F_not] );}
"<="				{FILTER7( LE,prf_name_str[F_le] );}
"<"				{FILTER7( LT,prf_name_str[F_lt] );}
">="				{FILTER7( GE,prf_name_str[F_ge] );}
">"				{FILTER7( GT,prf_name_str[F_gt] );}
"+"				{FILTER7( PLUS,prf_name_str[F_add] );}
"-"				{FILTER7( MINUS,prf_name_str[F_sub] );}
"/"				{FILTER7( DIV,prf_name_str[F_div] );}
"*"				{FILTER7( MUL,prf_name_str[F_mul] );}
"%"				{FILTER7( PRF_MOD,prf_name_str[F_div] );}

"++"				{FILTER( INC );}
"--" 				{FILTER( DEC );}

"+="				{FILTER( ADDON );}
"-="				{FILTER( SUBON );}
"*="				{FILTER( MULON );}
"/="				{FILTER( DIVON );}
"%="				{FILTER( MODON );}

"abs"				{FILTER7( ABS,prf_name_str[F_abs] );}
"reshape"			{FILTER7( RESHAPE, prf_name_str[F_reshape] );}
"shape"				{FILTER7( SHAPE,prf_name_str[F_shape] );}
"take"				{FILTER7( TAKE,prf_name_str[F_take] );}
"drop"				{FILTER7( DROP,prf_name_str[F_drop] );}
"dim"				{FILTER7( DIM,prf_name_str[F_dim] );}
"rotate"			{FILTER7( ROTATE,prf_name_str[F_rotate] );}
"cat"                           {FILTER7( CAT,prf_name_str[F_cat] );}
"psi"                           {FILTER7( PSI,prf_name_str[F_psi] );}
"toi"                           {FILTER7( TOI,prf_name_str[F_toi] );}
"tof"                           {FILTER7( TOF,prf_name_str[F_tof] );}
"tod"                           {FILTER7( TOD,prf_name_str[F_tod] );}
"min"                           {FILTER7( PRF_MIN,prf_name_str[F_min] );}
"max"                           {FILTER7( PRF_MAX,prf_name_str[F_max] );}
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
"with" 				{FILTER( WITH );}
"genarray"			{FILTER7( GENARRAY,prf_name_str[F_genarray] );}
"modarray"			{FILTER7( MODARRAY,prf_name_str[F_modarray] );}
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
"default"                       {FILTER( C_KEYWORD);}
"switch"                        {FILTER( C_KEYWORD);}
"goto"                          {FILTER( C_KEYWORD);}
"continue"                      {FILTER( C_KEYWORD);}
"break"                         {FILTER( C_KEYWORD);}


\"([^"\n]|(\\\"))*\"			{FILTER9( STR );}

[A-Za-z]([A-Za-z0-9]|[_A-Za-z0-9][A-Za-z0-9])*  \
                                {FILTER2( ID );}

[_A-Za-z][_A-Za-z0-9]*          {FILTER2( PRIVATEID );}

'.'                             {FILTER8( CHAR );}
'\\[0-9]*'                 {FILTER10( CHAR );}
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

