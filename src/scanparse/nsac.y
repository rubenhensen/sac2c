%{


/*
*
* $Log$
* Revision 1.25  2004/11/27 02:49:06  cg
* include of my_debug.h removed.
*
* Revision 1.24  2004/11/26 23:57:50  sbs
* *** empty log message ***
*
* Revision 1.23  2004/11/25 22:37:57  sbs
* for Stephan
*
* Revision 1.22  2004/11/25 22:28:51  sbs
* compiles
*
* Revision 1.21  2004/11/25 16:26:17  cg
* parsing of resourcefiles done without ids structure
*
* Revision 1.20  2004/11/25 15:37:40  sbs
* some maior changes
*
* Revision 1.19  2004/11/23 11:14:06  sbs
* next
*
* Revision 1.18  2004/11/23 01:28:37  sbs
* next one
*
* Revision 1.17  2004/11/22 21:34:29  sbs
* SacDevCamp04
*
* Revision 1.15  2004/11/21 11:22:03  sah
* removed some old ast infos
*
* Revision 1.14  2004/11/19 10:14:30  sah
* updated objdefs
*
* Revision 1.13  2004/11/18 10:28:20  sah
* corrected handling of simple types
*
* Revision 1.12  2004/11/17 19:45:53  sah
* fixed external typedefs
*
* Revision 1.11  2004/11/17 17:05:29  sah
* added external typedefs
*
* Revision 1.10  2004/11/14 15:20:00  sah
* fixed a bug
*
* Revision 1.9  2004/11/11 10:43:39  sah
* intermediate checkin
*
* Revision 1.8  2004/11/09 20:03:06  sah
* fixed q bug
*
* Revision 1.7  2004/11/09 18:16:05  sah
* noew fundefs and fundecs can be mixed
*
* Revision 1.6  2004/11/09 15:22:34  sah
* aadded missing ;...
*
* Revision 1.5  2004/11/08 19:07:05  sah
* added typedef for pragmalist
*
* Revision 1.4  2004/11/07 18:03:59  sah
* added external functions support
*
* Revision 1.3  2004/10/22 13:22:31  sah
* removed all artificial namespaces
* added during parsing
*
* Revision 1.2  2004/10/17 14:52:30  sah
* added support for except
* in interface descriptions
*
* Revision 1.1  2004/10/15 15:04:27  sah
* Initial revision
*
*
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "config.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "DupTree.h"        /* for use of DUPdoDupTree() */
#include "Error.h"
#include "free.h"
#include "globals.h"
#include "handle_mops.h"
#include "new_types.h"
#include "shape.h"

#include "resource.h"


extern int commlevel;
extern int charpos;
extern char *linebuf_ptr;
extern char *yytext;

extern int yylex();


static char *mod_name = MAIN_MOD_NAME;
static node *global_wlcomp_aps = NULL;
static node *store_pragma = NULL;
static bool have_seen_dots = FALSE;

/*
* used to distinguish the different kinds of files
* which are parsed with this single parser
*/
static file_type file_kind = F_prog;

static int yyerror( char *errname);
static int yyparse();

static void CleanUpParser();
static node *MakeIncDecLet( char *name, char *op);
static node *MakeOpOnLet( char *name, node *name2, char *op);
static node *String2Array( char *str);
static ntype *Exprs2NType( ntype *basetype, node *exprs);
static node *ConstructMop( node *, node *, node *);
static node *CheckWlcompConf( node *ap, node *exprs);

static int prf_arity[] = {
#define PRF_IF( a, b, c, d, e, f, g, h) f
#include "prf_node_info.mac"
#undef PRF_IF
};

%}

%union {
 nodetype           nodetype;
 char               *id;
 ntype              *ntype;
 node               *node;
 int                cint;
 char               cchar;
 float              cfloat;
 double             cdbl;
 deps               *deps;
 prf                prf;
 shape              *shape;
 resource_list_t    *resource_list_t;
 target_list_t      *target_list_t;
 inheritence_list_t *inheritence_list_t;
}

%token PARSE_PRG  PARSE_RC

%token BRACE_L  BRACE_R  BRACKET_L  BRACKET_R  SQBR_L  SQBR_R  COLON  SEMIC 
COMMA  AMPERS  DOT  QUESTION  ARROW 
INLINE  LET  TYPEDEF  OBJDEF  CLASSTYPE 
INC  DEC  ADDON  SUBON  MULON  DIVON  MODON 
K_MAIN  RETURN  IF  ELSE  DO  WHILE  FOR  NWITH  FOLD 
MODULE  IMPORT  EXPORT  PROVIDE  USE  CLASS  ALL  EXCEPT
MODSPEC
SC  TRUETOKEN  FALSETOKEN  EXTERN  C_KEYWORD 
HASH  PRAGMA  LINKNAME  LINKSIGN  EFFECT  READONLY  REFCOUNTING 
TOUCH  COPYFUN  FREEFUN  INITFUN  LINKWITH LINKOBJ
WLCOMP  CACHESIM  SPECIALIZE 
TARGET  STEP  WIDTH  GENARRAY  MODARRAY 
LE  LT  GT 
STAR  PLUS  MINUS  TILDE  EXCL 

PRF_DIM  PRF_SHAPE  PRF_RESHAPE  PRF_SEL  PRF_GENARRAY  PRF_MODARRAY 
PRF_ADD_SxS  PRF_ADD_SxA  PRF_ADD_AxS  PRF_ADD_AxA 
PRF_SUB_SxS  PRF_SUB_SxA  PRF_SUB_AxS  PRF_SUB_AxA 
PRF_MUL_SxS  PRF_MUL_SxA  PRF_MUL_AxS  PRF_MUL_AxA 
PRF_DIV_SxS  PRF_DIV_SxA  PRF_DIV_AxS  PRF_DIV_AxA 
PRF_MOD  PRF_MIN  PRF_MAX  PRF_ABS  PRF_NEG 
PRF_EQ  PRF_NEQ  PRF_LE  PRF_LT  PRF_GE  PRF_GT 
PRF_AND  PRF_OR  PRF_NOT 
PRF_TOI_S  PRF_TOI_A  PRF_TOF_S  PRF_TOF_A  PRF_TOD_S  PRF_TOD_A 
PRF_CAT_VxV  PRF_TAKE_SxV  PRF_DROP_SxV

%token <id> ID  STR  OPTION

%token <types> TYPE_INT  TYPE_FLOAT  TYPE_BOOL  TYPE_UNS  TYPE_SHORT 
       TYPE_LONG  TYPE_CHAR  TYPE_DBL  TYPE_VOID
%token <cint> NUM
%token <cfloat> FLOAT
%token <cdbl> DOUBLE
%token <cchar> CHAR

/*******************************************************************************
* SAC programs
*/

%type <node> prg  defs  def2  def3  def4 def5

%type <node> typedef

%type <node> objdefs  objdef

%type <node> fundef  fundef1  fundef2  main
%type <node> fundec fundec2
%type <node> mainargs  fundecargs fundefargs  args  arg varargs
%type <node> exprblock  exprblock2  assignsOPTret  assigns  assign 
     let cond optelse  doloop whileloop forloop  assignblock
     lets qual_ext_id qual_ext_ids ids
%type <node> exprs  expr  expr_ap  opt_arguments  expr_ar  expr_sel  with 
     generator  steps  width  nwithop  withop  wlassignblock  genidx 
     part  parts nums returntypes returndectypes ntypes varntypes
%type <prf> genop  foldop  prf

%type <id> reservedid  string ext_id

%type <ntype> simplentype userntype basentype ntype

/* pragmas */
%type <id> pragmacachesim
%type <node> wlcomp_pragma_global  wlcomp_pragma_local  wlcomp_conf
%type <node> pragmas pragma pragmalist
/*
%type <node> pragmas 
*/





/*******************************************************************************
* module implementations
*/
%type <node> module class
%type <node> import use export provide interface
%type <node> symbolset symbolsetentries



/*******************************************************************************
* module specializations (for the C interface only!)
*/
/* %type <node> modspec */




/*******************************************************************************
* sac2crc files
*/
%type <target_list_t> targets
%type <inheritence_list_t> inherits
%type <resource_list_t> resources



%right INC DEC STAR PLUS MINUS TILDE EXCL LE LT GT ID
GENARRAY MODARRAY ALL AMPERS
%right BM_OP
%right MM_OP CAST
%right SQBR_L BRACKET_L
%right ELSE

%start all

%{

/*
* Make sure, the stack of the generated parser is big enough!
*/
#define YYMAXDEPTH 10000 

%}
%%

all: file eof { CleanUpParser(); }
;

file: PARSE_PRG  prg       { global.syntax_tree = $2; }
    | PARSE_PRG  module    { global.syntax_tree = $2; }
    | PARSE_PRG  class     { global.syntax_tree = $2; }
    | PARSE_RC   targets   { global.target_list = RSCaddTargetList( $2, global.target_list); }
/*  | PARSE_SPEC modspec   { spec_tree = $2; } */
;

eof: { if (commlevel) {
 ABORT( global.linenum, ("Unterminated comment found"));

#ifdef MUST_REFERENCE_YYLABELS
/*
 * The follwing command is a veeeeeeery ugly trick to avoid warnings
 * on the alpha: the YYBACKUP-macro contains jumps to two labels
 * yyerrlab  and  yynewstate  which are not used otherwise.
 * Hence, the usage here, which in fact never IS (and never SHOULD)
 * be carried out, prevents the gcc from complaining about the two
 * aforementioned labels not to be used!!
 */
 YYBACKUP( NUM, yylval);
#endif
}
}
;



/*******************************************************************************
*******************************************************************************
*
*  rules for ordinary SAC programs
*
*******************************************************************************
*******************************************************************************/


prg: defs
     { $$ = $1;
       MODULE_NAME( $$) = mod_name;
       MODULE_FILETYPE( $$) = F_prog;
     }
   ;

defs: interface def2
      { $$ = $2;
        MODULE_IMPORTS( $$) = $1;
      }
    | def2
      { $$ = $1; }
    ;

def2: typedef def2
      { $$ = $2;
        TYPEDEF_NEXT( $1) = MODULE_TYPES( $$);
        MODULE_TYPES( $$) = $1;
      }
    | def3
      { $$ = $1; }
    ;

def3: objdefs def4
      { $$ = $2;
        MODULE_OBJS( $$) = $1;
      }
    | def4
      { $$ = $1; }
    ;

def4: fundec def4
      { $$ = $2;
        MODULE_FUNDECS( $$) = TCappendFundef( MODULE_FUNDECS( $$), $1);
      }
    | wlcomp_pragma_global main def5
      { $$ = $3;
        MODULE_FUNS( $$) = TCappendFundef( MODULE_FUNS( $$), $2);
      } 
    | wlcomp_pragma_global fundef def4
      { $$ = $3;
        MODULE_FUNS( $$) = TCappendFundef( MODULE_FUNS( $$), $2);
      }
    | def5
      { $$ = $1; }
    ;

def5: { $$ = TBmakeModule( NULL, F_prog, NULL, NULL, NULL, NULL, NULL);

        DBUG_PRINT( "PARSE",
	            ("%s:"F_PTR,
	             global.mdb_nodetype[ NODE_TYPE( $$)],
	             $$));
      }
    ;


/*
*********************************************************************
*
*  rules for imports
*
*********************************************************************
*/
interface: import interface
           { $$ = $1;
             IMPORT_NEXT($$) = $2;
           }
         | import
           {
             $$ = $1;
           }
         | use interface
           { $$ = $1;
             USE_NEXT($$) = $2;
           }
         | use
           { $$ = $1;
           }
         | export interface
           { $$ = $1;
             EXPORT_NEXT($$) = $2;
           }
         | export
           { $$ = $1;
           }
         | provide interface
           { $$ = $1;
             PROVIDE_NEXT($$) = $2;
           }
         | provide
           { $$ = $1;
           }
         ;

import: IMPORT ID COLON ALL SEMIC
        { $$ = TBmakeImport( $2, NULL, NULL);
          IMPORT_ALL( $$) = TRUE; 
        }
        | IMPORT ID COLON ALL EXCEPT symbolset SEMIC
          { $$ = TBmakeImport( $2, NULL, $6);
            IMPORT_ALL( $$) = TRUE; 
          }
        | IMPORT ID COLON symbolset SEMIC 
          { $$ = TBmakeImport( $2, NULL, $4); }
        ;

use: USE ID COLON ALL SEMIC 
     { $$ = TBmakeUse( $2, NULL, NULL);
       USE_ALL( $$) = TRUE;
     }
   | USE ID COLON ALL EXCEPT symbolset SEMIC
     { $$ = TBmakeUse( $2, NULL, $6);
       USE_ALL( $$) = TRUE;
     }
   | USE ID COLON symbolset SEMIC { $$ = TBmakeUse( $2, NULL, $4); }
   ;

export: EXPORT ALL SEMIC 
        { $$ = TBmakeExport( NULL, NULL); 
          EXPORT_ALL( $$) = TRUE;
        }
      | EXPORT ALL EXCEPT symbolset SEMIC 
        { $$ = TBmakeExport( NULL, $4); 
          EXPORT_ALL( $$) = TRUE;
        }
      | EXPORT symbolset SEMIC { $$ = TBmakeExport( NULL, $2); }
      ;

provide: PROVIDE ALL SEMIC 
         { $$ = TBmakeProvide( NULL, NULL); 
           PROVIDE_ALL( $$) = TRUE;
         }
       | PROVIDE ALL EXCEPT symbolset SEMIC 
         { $$ = TBmakeProvide( NULL, $4); 
           PROVIDE_ALL( $$) = TRUE;
         }
       | PROVIDE symbolset SEMIC { $$ = TBmakeProvide( NULL, $2); }
       ;

symbolset: BRACE_L symbolsetentries BRACE_R { $$ = $2; } ;

symbolsetentries: ext_id COMMA symbolsetentries { $$ = TBmakeSymbol( $1, $3); }
                | ext_id { $$ = TBmakeSymbol( $1, NULL); }
                ;


/*
*********************************************************************
*
*  rules for typedefs
*
*********************************************************************
*/

typedef: TYPEDEF ntype ID SEMIC 
         { $$ = TBmakeTypedef( $3, mod_name, $2, NULL);

           DBUG_PRINT( "PARSE",
                       ("%s:"F_PTR","F_PTR", Id: %s",
                        global.mdb_nodetype[ NODE_TYPE( $$)],
                        $$, 
                        TYPEDEF_NTYPE( $$),
                        TYPEDEF_NAME( $$)));
         }
       | EXTERN TYPEDEF ID SEMIC
         { $$ = TBmakeTypedef( $3, mod_name, TYmakeAKS(
                  TYmakeSimpleType( T_hidden), 
                  SHmakeShape( 0)), NULL);

           DBUG_PRINT( "PARSE",
                       ("%s:"F_PTR","F_PTR", Id: %s",
                        global.mdb_nodetype[ NODE_TYPE( $$)],
                        $$, 
                        TYPEDEF_NTYPE( $$),
                        TYPEDEF_NAME( $$)));
         }
       ;


/*
*********************************************************************
*
*  rules for objdefs
*
*********************************************************************
*/

objdefs: objdef objdefs
         { $$ = $1;
           OBJDEF_NEXT( $$) = $1;
         }
       | objdef
         { $$ = $1;
         }
       ;

objdef: OBJDEF ntype ID LET expr SEMIC 
        { $$ = TBmakeObjdef( $2, NULL, $3, $5, NULL);

          DBUG_PRINT( "PARSE",
                      ("%s:"F_PTR","F_PTR", Id: %s",
                       global.mdb_nodetype[ NODE_TYPE( $$)],
                       $$, 
                       OBJDEF_TYPE( $$),
                       OBJDEF_NAME( $$)));
        }
      ;



/*
*********************************************************************
*
*  rules for fundefs
*
*********************************************************************
*/

fundef: INLINE fundef1
        { $$ = $2;
          FUNDEF_ISINLINE( $$) = TRUE;
        }
      | fundef1
        { $$ = $1;
          FUNDEF_ISINLINE( $$) = FALSE;
        }
      ;

fundef1: returntypes BRACKET_L ext_id BRACKET_R BRACKET_L fundef2
         { $$ = $6;
           FUNDEF_RETS( $$) = $1; /* result type(s) */
           FUNDEF_NAME( $$) = $3; /* function name  */
           FUNDEF_ALLOWSINFIX( $$) = TRUE;

           DBUG_PRINT( "PARSE",
                        ("%s: %s:%s "F_PTR,
                        global.mdb_nodetype[ NODE_TYPE( $$)],
                        (FUNDEF_MOD( $$) == NULL) ? "(null)": FUNDEF_MOD( $$),
                        FUNDEF_NAME( $$),
                        FUNDEF_NAME( $$)));

         }
       | returntypes ext_id BRACKET_L fundef2
         { $$ = $4;
           FUNDEF_RETS( $$) = $1;   /* result type(s) */
           FUNDEF_NAME( $$) = $2;    /* function name  */
           FUNDEF_ALLOWSINFIX( $$) = FALSE;

           DBUG_PRINT( "PARSE",
                        ("%s: %s:%s "F_PTR,
                        global.mdb_nodetype[ NODE_TYPE( $$)],
                        (FUNDEF_MOD( $$) == NULL) ? "(null)" : FUNDEF_MOD( $$),
                        FUNDEF_NAME( $$),
                        FUNDEF_NAME( $$)));

         }
       ;

fundef2: fundefargs BRACKET_R
         { $$ = TBmakeFundef( NULL, NULL, NULL, NULL, NULL, NULL); }
         exprblock
         { 
           $$ = $<node>3;
           FUNDEF_BODY( $$) = $4;             /* function bdoy  */
           FUNDEF_ARGS( $$) = $1;             /* fundef args */

           DBUG_PRINT( "PARSE",
                       ("%s:"F_PTR", Id: %s"F_PTR" %s," F_PTR,
                        global.mdb_nodetype[ NODE_TYPE( $$)],
                        $$, 
                        global.mdb_nodetype[ NODE_TYPE( FUNDEF_BODY( $$))],
                        FUNDEF_BODY( $$),
                        global.mdb_nodetype[ NODE_TYPE( FUNDEF_ARGS( $$))],
                        FUNDEF_ARGS( $$)));
         }
       | BRACKET_R { $$ = TBmakeFundef( NULL, NULL, NULL, NULL, NULL, NULL); }
         exprblock
         { $$ = $<node>2;
           FUNDEF_BODY( $$) = $3;

           DBUG_PRINT( "PARSE",
                       ("%s:"F_PTR" %s"F_PTR,
                        global.mdb_nodetype[ NODE_TYPE( $$)],
                        $$, 
                        global.mdb_nodetype[ NODE_TYPE( FUNDEF_BODY( $$))],
                        FUNDEF_BODY( $$)));
         }
       ;


fundefargs: args        { $$ = $1;   }
          | TYPE_VOID   { $$ = NULL; }
          ;

fundecargs: varargs     { $$ = $1;   }
          | TYPE_VOID   { $$ = NULL; }
          | DOT DOT DOT { $$ = NULL; have_seen_dots = TRUE; }
          ;


args: arg COMMA args
      { ARG_NEXT($1) = $3;
        $$ = $1;
      }
    | arg
     { $$ = $1; 
      }
    ;

varargs: arg COMMA args
         { ARG_NEXT($1) = $3;
           $$ = $1;
         }
       | arg
        { $$ = $1;
         }
       | arg COMMA DOT DOT DOT
         { $$ = $1;
           have_seen_dots = TRUE;
         }
       ;


arg: ntype ID
     { $$ = TBmakeArg( TBmakeAvis( $2, $1), NULL);

       DBUG_PRINT( "PARSE",
                   ("%s: "F_PTR", Id: %s ",
                    global.mdb_nodetype[ NODE_TYPE( $$)],
                    $$, 
                    ARG_NAME( $$))
                    );
     }
   | ntype AMPERS ID
     { $$ = TBmakeArg( TBmakeAvis( $3, $1), NULL);
       ARG_ISREFERENCE( $$) = TRUE;

       DBUG_PRINT( "PARSE",
                   ("%s: "F_PTR", Id: %s, Attrib: %d ",
                    global.mdb_nodetype[ NODE_TYPE( $$)],
                    $$, 
                    ARG_NAME( $$) ));
     }
   ;


main: TYPE_INT K_MAIN BRACKET_L mainargs BRACKET_R { $<cint>$ = global.linenum; } exprblock
      { $$ = TBmakeFundef( NULL, NULL,
                           TBmakeRet(
                             TYmakeAKS( TYmakeSimpleType( T_int), SHmakeShape(0)),
                             NULL),
                           $4, $7, NULL);
        NODE_LINE( $$) = $<cint>6;

        FUNDEF_NAME( $$) = ILIBstringCopy( "main");

        DBUG_PRINT( "PARSE",
                    ("%s:"F_PTR", main "F_PTR " %s (" F_PTR ") ",
                     global.mdb_nodetype[ NODE_TYPE( $$)],
                     $$, 
                     FUNDEF_NAME( $$),
                     global.mdb_nodetype[ NODE_TYPE( FUNDEF_BODY($$))],
                     FUNDEF_BODY($$)));
      }
    ;

mainargs: TYPE_VOID     { $$ = NULL; }
        | /* empty */   { $$ = NULL; }
        ;


/*
*********************************************************************
*
*  rules for fundecs
*
*********************************************************************
*/

fundec: EXTERN returndectypes ext_id BRACKET_L fundec2
        { $$ = $5;
          FUNDEF_RETS( $$) = $2;
          FUNDEF_NAME( $$) = $3;  /* function name */
          FUNDEF_ISEXTERN( $$) = TRUE;
          if( have_seen_dots) {
             FUNDEF_HASDOTARGS( $$) = TRUE;
             have_seen_dots = FALSE;
           }
        }
      ;

fundec2: fundecargs BRACKET_R { $<cint>$ = global.linenum; } pragmas SEMIC
         { $$ = TBmakeFundef( NULL, NULL, NULL, $1, NULL, NULL);
           NODE_LINE( $$) = $<cint>3;
           FUNDEF_PRAGMA( $$) = $4;
           if( have_seen_dots) {
             FUNDEF_HASDOTARGS( $$) = TRUE;
             have_seen_dots = FALSE;
           }
         }
       | BRACKET_R { $<cint>$ = global.linenum; } pragmas SEMIC
         { $$ = TBmakeFundef( NULL, NULL, NULL, NULL, NULL, NULL);
           NODE_LINE( $$) = $<cint>2;
           FUNDEF_PRAGMA( $$) = $3;
         }
       ;


/*
*********************************************************************
*
*  rules for pragmas
*
*********************************************************************
*/

hash_pragma: HASH PRAGMA ;

wlcomp_pragma_global: hash_pragma WLCOMP wlcomp_conf
                      { if (global_wlcomp_aps != NULL) {
                          /* remove old global pragma */
                          global_wlcomp_aps = FREEdoFreeTree( global_wlcomp_aps);
                        }
                        $3 = CheckWlcompConf( $3, NULL);
                        if ($3 != NULL) {
                          global_wlcomp_aps = $3;
                        }
                      }
                    | /* empty */
                      {
                      }
                    ;

wlcomp_pragma_local: hash_pragma WLCOMP wlcomp_conf
                     { $3 = CheckWlcompConf( $3, NULL);
                       if ($3 != NULL) {
                         $$ = TBmakePragma();
                         PRAGMA_WLCOMP_APS( $$) = $3;
                       } else {
                         $$ = NULL;
                       }
                     }
                   | /* empty */
                     { if (global_wlcomp_aps != NULL) {
                         $$ = TBmakePragma();
                         PRAGMA_WLCOMP_APS( $$) = DUPdoDupTree( global_wlcomp_aps);
                       } else {
                         $$ = NULL;
                       }
                     }
                   ;

wlcomp_conf: ID        { $$ = TBmakeId( TBmakeAvis( $1, NULL)); }
           | expr_ap   { $$ = $1; }
           ;


pragmacachesim: hash_pragma CACHESIM string   { $$ = $3;              }
              | hash_pragma CACHESIM          { $$ = ILIBstringCopy( ""); }
              | /* empty */              { $$ = NULL;            }
              ;


/*
 * pragmas as needed for external functions
 *  
 */

pragmas: pragmalist
         { $$ = store_pragma;
           store_pragma = NULL;
         }
       | 
         { $$ = NULL;
         }
       ;

pragmalist: pragmalist pragma
          | pragma
          ;

pragma: hash_pragma LINKNAME string
        { if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_LINKNAME( store_pragma) != NULL) {
            WARN( global.linenum, ("Conflicting definitions of pragma 'linkname`"));
          }
          PRAGMA_LINKNAME( store_pragma) = $3;
        }
      | hash_pragma LINKWITH string
        { if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_LINKMOD( store_pragma) != NULL) {
            WARN( global.linenum, ("Conflicting definitions of pragma 'linkmod`"));
          }
          PRAGMA_LINKMOD( store_pragma) = $3;
        }
      | hash_pragma LINKOBJ string
        { if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_LINKOBJ( store_pragma) != NULL) {
            WARN( global.linenum, ("Conflicting definitions of pragma 'linkmod`"));
          }
          PRAGMA_LINKOBJ( store_pragma) = $3;
        }
      | hash_pragma LINKSIGN SQBR_L nums SQBR_R
        { if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_LINKSIGN( store_pragma) != NULL) {
            WARN(global.linenum, ("Conflicting definitions of pragma 'linksign`"));
          }
          PRAGMA_LINKSIGN( store_pragma) = $4;
        }
      | hash_pragma REFCOUNTING SQBR_L nums SQBR_R
        { if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_REFCOUNTING( store_pragma) != NULL) {
            WARN(global.linenum, ("Conflicting definitions of pragma 'refcounting`"));
          }
          PRAGMA_REFCOUNTING( store_pragma) = $4;
        }
      | hash_pragma READONLY SQBR_L nums SQBR_R
        { if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_READONLY( store_pragma) != NULL) {
            WARN(global.linenum, ("Conflicting definitions of pragma 'readonly`"));
          }
          PRAGMA_READONLY( store_pragma) = $4;
        }
      | hash_pragma EFFECT qual_ext_ids
        { if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_EFFECT( store_pragma) != NULL) {
            WARN(global.linenum, ("Conflicting definitions of pragma 'effect`"));
          }
          PRAGMA_EFFECT( store_pragma) = $3;
        }
      | hash_pragma TOUCH qual_ext_ids
        { if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_TOUCH( store_pragma) != NULL) {
            WARN(global.linenum, ("Conflicting definitions of pragma 'touch`"));
          }
          PRAGMA_TOUCH( store_pragma) = $3;
        }
      | hash_pragma COPYFUN string
        { if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_COPYFUN( store_pragma) != NULL) {
            WARN(global.linenum, ("Conflicting definitions of pragma 'copyfun`"));
          }
          PRAGMA_COPYFUN( store_pragma) = $3;
        }
      | hash_pragma FREEFUN string
        { if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_FREEFUN( store_pragma) != NULL) {
            WARN(global.linenum, ("Conflicting definitions of pragma 'freefun`"));
          }
          PRAGMA_FREEFUN( store_pragma) = $3;
        }
      | hash_pragma INITFUN string
        { if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_INITFUN( store_pragma) != NULL) {
            WARN(global.linenum, ("Conflicting definitions of pragma 'initfun`"));
          }
          PRAGMA_INITFUN( store_pragma) = $3;
        }
      ;



/*
 *********************************************************************
 *
 *  rules for expression blocks
 *
 *********************************************************************
 */

exprblock: BRACE_L { $<cint>$ = global.linenum; } pragmacachesim exprblock2
           { $$ = $4;
             BLOCK_CACHESIM( $$) = $3;
             NODE_LINE( $$) = $<cint>2;
           }
         ;

exprblock2: ntype ids SEMIC exprblock2
            { node *vardec_ptr;
              node *ids_ptr = $2;

              /*
               * Insert the actual vardec(s) before the ones that
               * are already attached to the N_block node of $4!
               * This reverses the order of var-decs!
               * The reason for doing so is feasablilty only...
               * In regard to semantics, there should be no difference...
               */
              vardec_ptr = BLOCK_VARDEC( $4);

              DBUG_ASSERT( ($2 != NULL),
                           "non-terminal ids should not return NULL ptr!");

              /*
               * In the AST, each variable has it's own declaration.
               * Therefore, for each ID in ids, we have to generate
               * it's own N_vardec node with it's own copy of the
               * types-structure from $1!
               */
              while (IDS_NEXT( $2) != NULL) {  /* at least 2 vardecs! */
                vardec_ptr = TBmakeVardec( TBmakeAvis( ILIBstringCopy( IDS_SPNAME( $2)),
                                                       TYcopyType( $1)),
                                           vardec_ptr);
                /*
                 * Now, we want to "push" $2 one IDS further
                 * and we want to FREE the current IDS structure.
                 */
                ids_ptr = IDS_NEXT( $2);
                $2 = FREEdoFreeNode( $2);
                $2 = ids_ptr;
              }
              /*
               * When we reach this point, all but one vardec is constructed!
               * Therefore, we can recycle the ntype from $1 instead of
               * duplicating it as done in the loop above!
               */
              $$ = $4;
              BLOCK_VARDEC( $$) = TBmakeVardec( TBmakeAvis( ILIBstringCopy( IDS_SPNAME( $2)), $1),
                                                vardec_ptr);
              $2 = FREEdoFreeTree( $2);   /* Finally, we free the last IDS-node! */
            }
          | assignsOPTret BRACE_R
            { $$ = TBmakeBlock( $1, NULL);
            }
          ;

assignsOPTret: /*
                * Although this rule is very similar to the "assigns"
                * rule we keep both of them since this allows the
                * N_assign -> N_return     node to be
                * inserted directly in the N_assign chain.
                * Otherwise, we would have to append that node
                * to a given N_assigns chain as generated by
                * "assigns".
                */
               /* empty */
               { $$ = TBmakeAssign( TBmakeReturn( NULL), NULL);
               }
             | RETURN BRACKET_L { $<cint>$ = global.linenum; } exprs BRACKET_R SEMIC
               { $$ = TBmakeAssign( TBmakeReturn( $4), NULL);
                 NODE_LINE( $$) = $<cint>3;
               }
             | RETURN BRACKET_L { $<cint>$ = global.linenum; } BRACKET_R SEMIC
               { $$ = TBmakeAssign( TBmakeReturn( NULL), NULL);
                 NODE_LINE( $$) = $<cint>3;
               }
             | RETURN { $<cint>$ = global.linenum; } SEMIC
               { $$ = TBmakeAssign( TBmakeReturn( NULL), NULL);
                 NODE_LINE( $$) = $<cint>3;
               }
             | assign { $<cint>$ = global.linenum; } assignsOPTret
               {
                 $$ = TCappendAssign( $1, $3);
                 NODE_LINE($$) = $<cint>2;
                 /*
                  * $1 may be a former for-loop in which case it may point
                  * to an entire chain of assignments.
                  */
                     }
             ;

assigns: /* empty */
         { $$ = NULL;
         }
       | assign { $<cint>$ = global.linenum; } assigns
         { 
           $$ = TCappendAssign( $1, $3);
           NODE_LINE($$) = $<cint>2;
           /*
            * $1 may be a former for-loop in which case it may point
            * to an entire chain of assignments.
            */
         }
       ;

assign: let SEMIC       { $$ = TBmakeAssign( $1, NULL); }
      | cond            { $$ = TBmakeAssign( $1, NULL); }
      | doloop SEMIC    { $$ = TBmakeAssign( $1, NULL); }
      | whileloop       { $$ = TBmakeAssign( $1, NULL); }
      | forloop         { $$ = $1; /* forloop already produces assign node. */}
      ;

let:       ids LET { $<cint>$ = global.linenum; } expr
           { $$ = TBmakeLet( $1, $4);
             NODE_LINE( $$) = $<cint>3;
           }
         | ID SQBR_L exprs SQBR_R LET { $<cint>$ = global.linenum; } expr
           { node *id, *ids, *ap;

             if( TCcountExprs( $3) > 1) {
               $3 = TCmakeFlatArray( $3);
             } else {
               node * tmp;

               tmp = $3;
               $3 = EXPRS_EXPR( $3);
               EXPRS_EXPR( tmp) = NULL;
               tmp = FREEdoFreeNode( tmp);
             }
             id = TBmakeId( NULL);
             ID_SPNAME( id) = ILIBstringCopy( $1);

             ids = TBmakeIds( NULL, NULL);
             IDS_SPNAME( ids) = $1;

             ap = TBmakeAp( NULL,
                            TBmakeExprs( TBmakeId( TBmakeAvis( $1, NULL)),
                              TBmakeExprs( $3,
                                TBmakeExprs( $7,
                                  NULL))));
             AP_SPNAME( ap) = ILIBstringCopy( "modarray");

             $$ = TBmakeLet( ids, ap);
             NODE_LINE( $$) = $<cint>5;
           }
         | expr_ap { $$ = TBmakeLet( $1, NULL); }
         | ID INC { $$ = MakeIncDecLet( $1, ILIBstringCopy( "+")); }
         | INC ID { $$ = MakeIncDecLet( $2, ILIBstringCopy( "+")); }
         | ID DEC { $$ = MakeIncDecLet( $1, ILIBstringCopy( "-")); }
         | DEC ID { $$ = MakeIncDecLet( $2, ILIBstringCopy( "-")); }
         | ID ADDON expr { $$ = MakeOpOnLet( $1, $3, ILIBstringCopy("+")); }
         | ID SUBON expr { $$ = MakeOpOnLet( $1, $3, ILIBstringCopy("-")); }
         | ID MULON expr { $$ = MakeOpOnLet( $1, $3, ILIBstringCopy("*")); }
         | ID DIVON expr { $$ = MakeOpOnLet( $1, $3, ILIBstringCopy("/")); }
         | ID MODON expr { $$ = MakeOpOnLet( $1, $3, ILIBstringCopy("%")); }
         ;

cond: IF { $<cint>$ = global.linenum; } BRACKET_L expr BRACKET_R assignblock optelse
      {
        $$ = TBmakeCond( $4, $6, $7);
        NODE_LINE( $$) = $<cint>2;
      }
      ;

optelse: ELSE assignblock           { $$ = $2;                 }
       | /* empty */   %prec ELSE   { $$ = MAKE_EMPTY_BLOCK(); }
       ;

doloop: DO { $<cint>$ = global.linenum; } assignblock
        WHILE BRACKET_L expr BRACKET_R 
        {
          $$ = TBmakeDo( $6, $3);
          NODE_LINE( $$) = $<cint>2;
        }
      ;

whileloop: WHILE { $<cint>$ = global.linenum; } BRACKET_L expr BRACKET_R
           assignblock
           {
             $$ = TBmakeWhile( $4, $6);
             NODE_LINE( $$) = $<cint>2;
           }
         ;

forloop:   FOR { $<cint>$ = global.linenum; }
           BRACKET_L lets SEMIC expr SEMIC lets BRACKET_R assignblock
           { /*
              *    for (e1; e2; e3) {assigns}
              *    
              *    is transformed into
              *    
              *     e1;
              *     while (e2) {
              *       assigns
              *       e3;
              *     }
              *
              *    Note that e1 and e3 are potentially empty comma-separated lists of 
              *    assignments while e2 is an expression which must evaluate to a 
              *    boolean value.
              */

             node *while_assign;
             
             BLOCK_INSTR( $10) = TCappendAssign( BLOCK_INSTR( $10), $8);
             while_assign = TBmakeAssign( TBmakeWhile( $6, $10), NULL);
             NODE_LINE( while_assign) = $<cint>2;
             NODE_LINE( ASSIGN_INSTR( while_assign)) = $<cint>2;
             $$ = TCappendAssign( $4, while_assign);
           }
         ;


lets: let COMMA lets
      {
        $$ = TBmakeAssign( $1, $3);
      }
      | let
      {
        $$ = TBmakeAssign( $1, NULL);
      }
      |
      {
        $$ = NULL;
      }
    ; 

assignblock: SEMIC
             { $$ = MAKE_EMPTY_BLOCK();
             }
           | BRACE_L { $<cint>$ = global.linenum; } pragmacachesim assigns BRACE_R
             { if ($4 == NULL) {
                 $$ = MAKE_EMPTY_BLOCK();
               }
               else {
                 $$ = TBmakeBlock( $4, NULL);
                 BLOCK_CACHESIM( $$) = $3;
                 NODE_LINE( $$) = $<cint>2;
               }
             }
           | assign
             { $$ = TBmakeBlock( $1, NULL);
             }
           ;



/*
 *********************************************************************
 *
 *  rules for expressions
 *
 *********************************************************************
 */

exprs: expr COMMA exprs          { $$ = TBmakeExprs( $1, $3);   }
     | expr                      { $$ = TBmakeExprs( $1, NULL); }
     ;

expr: qual_ext_id                { $$ = $1;                   }
    | DOT                        { $$ = TBmakeDot( 1);        }
    | DOT DOT DOT                { $$ = TBmakeDot( 3);        }
    | NUM                        { $$ = TBmakeNum( $1);       }
    | FLOAT                      { $$ = TBmakeFloat( $1);     }
    | DOUBLE                     { $$ = TBmakeDouble( $1);    }
    | CHAR                       { $$ = TBmakeChar( $1);      }
    | TRUETOKEN                  { $$ = TBmakeBool( 1);       }
    | FALSETOKEN                 { $$ = TBmakeBool( 0);       }
    | string                     { $$ = String2Array( $1);    }
    | BRACKET_L expr BRACKET_R
      { $$ = $2;
        if( NODE_TYPE( $2) == N_mop) {
          MOP_ISFIXED( $$) = TRUE;
        }
      }
    | expr qual_ext_id expr %prec BM_OP
      {
        $$ = ConstructMop( $1, $2, $3);
      }
    | PLUS expr %prec MM_OP
      {
        $$ = TCmakeAp1( NULL, $2);
        AP_SPNAME( $$) = ILIBstringCopy( "+");
      }
    | MINUS expr %prec MM_OP
      {
        $$ = TCmakeAp1( NULL, $2);
        AP_SPNAME( $$) = ILIBstringCopy( "-");
      }
    | TILDE expr %prec MM_OP
      {
        $$ = TCmakeAp1( NULL, $2);
        AP_SPNAME( $$) = ILIBstringCopy( "~");
      }
    | EXCL expr %prec MM_OP
      {
        $$ = TCmakeAp1( NULL, $2);
        AP_SPNAME( $$) = ILIBstringCopy( "!");
      }
    | PLUS BRACKET_L expr COMMA exprs BRACKET_R
      {
        $$ = TBmakeAp( NULL, TBmakeExprs( $3, $5));
        AP_SPNAME( $$) = ILIBstringCopy( "+");
      }
    | MINUS BRACKET_L expr COMMA exprs BRACKET_R
      {
        $$ = TBmakeAp( NULL, TBmakeExprs( $3, $5));
        AP_SPNAME( $$) = ILIBstringCopy( "-");
      }
    | TILDE BRACKET_L expr COMMA exprs BRACKET_R
      {
        $$ = TBmakeAp( NULL, TBmakeExprs( $3, $5));
        AP_SPNAME( $$) = ILIBstringCopy( "~");
      }
    | EXCL BRACKET_L expr COMMA exprs BRACKET_R
      {
        $$ = TBmakeAp( NULL, TBmakeExprs( $3, $5));
        AP_SPNAME( $$) = ILIBstringCopy( "!");
      }
    | expr_sel                    { $$ = $1; }   /* bracket notation      */
    | expr_ap                     { $$ = $1; }   /* prefix function calls */
    | expr_ar                     { $$ = $1; }   /* constant arrays       */
    | BRACKET_L COLON ntype BRACKET_R expr   %prec CAST
      { $$ = TBmakeCast( $3, $5);
      }
    | BRACE_L ID ARROW expr BRACE_R
      { $$ = TBmakeSetwl( TCmakeIdCopyString( $2), $4);
      }
    | BRACE_L SQBR_L exprs SQBR_R ARROW expr BRACE_R
      { $$ = TBmakeSetwl( $3, $6);
      }
    | wlcomp_pragma_local NWITH { $<cint>$ = global.linenum; } with
      { $$ = $4;
        NODE_LINE( $$)= $<cint>3;
        WITH_PRAGMA( $$) = $1;
      }
    ;
      

with: BRACKET_L generator BRACKET_R wlassignblock withop
      { 
        node * cexpr;
#if 0
        /*
         * For now, we do not yet ask for the new syntax, BUT later we will
         * activate the following two lines....
         */
        WARN( global.linenum, ( "Old with-loop style depricated!"));
        CONT_WARN( ("Please use the new syntax instead"));
#endif

        /*
         * the tricky part about this rule is that $5 (an N_withop node)
         * carries the goal-expression of the With-Loop, i.e., the "N-expr"
         * node which belongs into the N_code node!!!
         * The reason for this is that an exclusion of the goal expression
         * from the non-terminal withop would lead to a shift/reduce
         * conflict in that rule!
         */
        if( NODE_TYPE( $5) == N_genarray) {
          cexpr = GENARRAY_SPEXPR($5);
          GENARRAY_SPEXPR($5) = NULL;
        } else if ( NODE_TYPE( $5) == N_modarray) {
          cexpr = MODARRAY_SPEXPR($5);
          MODARRAY_SPEXPR($5) = NULL;
        } else {
          cexpr = FOLD_SPEXPR($5);
          FOLD_SPEXPR($5) = NULL;
        }
        $$ = TBmakeWith( $2, TBmakeCode( $4, TBmakeExprs( NULL, cexpr)), $5);
        CODE_USED( WITH_CODE( $$))++;
        /*
         * Finally, we generate the link between the (only) partition
         * and the (only) code!
         */
        PART_CODE( WITH_PART( $$)) = WITH_CODE( $$);
      }
    | BRACKET_L ID BRACKET_R parts nwithop
      { $$ = $4;
        WITH_WITHOP( $$) = $5;
        /*
         * At the time being we ignore $2. However, it SHOULD be checked
         * against all genidxs in $4 here....
         */
      }
    ;


expr_sel: expr SQBR_L exprs SQBR_R
          { if( TCcountExprs($3) == 1) {
              $$ = TCmakeAp2( NULL, EXPRS_EXPR( $3), $1);
              AP_SPNAME( $$) = ILIBstringCopy( "sel");
              EXPRS_EXPR( $3) = NULL;
              $3 = FREEdoFreeNode( $3);
            } else {
              $$ = TCmakeAp2( NULL, TCmakeFlatArray( $3), $1);
              AP_SPNAME( $$) = ILIBstringCopy( "sel");
            }
          }
        | expr SQBR_L SQBR_R
          { $$ = TCmakeAp2( NULL, TCmakeFlatArray( NULL), $1);
            AP_SPNAME( $$) = ILIBstringCopy( "sel");
          }
        ;

expr_ap: qual_ext_id BRACKET_L { $<cint>$ = global.linenum; } opt_arguments BRACKET_R
         {
           $$ = TBmakeAp( NULL, $4);
           AP_NAME( $$) = ILIBstringCopy( ID_SPNAME( $1));
           AP_MOD( $$)  = ILIBstringCopy( ID_SPMOD( $1));
           NODE_LINE( $$) = $<cint>3;
           $1 = FREEdoFreeTree( $1);
         }
       | prf BRACKET_L { $<cint>$ = global.linenum; } opt_arguments BRACKET_R
         { char tmp[64];
           int num_args;

           num_args = TCcountExprs( $4);
           if( num_args != prf_arity[$1]) {
             sprintf( tmp, "%d argument(s) expected instead of %d", prf_arity[$1], num_args);
             yyerror( tmp);
           } else {
             $$ = TBmakePrf( $1, $4);
           }
           NODE_LINE( $$) = $<cint>3;
         }
       ;

opt_arguments: exprs         { $$ = $1;   }
             | /* empty */   { $$ = NULL; }
             ;

expr_ar: SQBR_L { $<cint>$ = global.linenum; } exprs SQBR_R
         { $$ = TCmakeFlatArray( $3);
           NODE_LINE( $$) = $<cint>2;
         }
       | SQBR_L { $<cint>$ = global.linenum; } SQBR_R
         { $$ = TCmakeFlatArray( NULL);
           NODE_LINE( $$) = $<cint>2;
         }
       ;

parts: part
       { $$ = $1;
       }
     | part parts
       { $$ = $1;
         PART_NEXT( WITH_PART( $1)) = WITH_PART( $2);
         CODE_NEXT( WITH_CODE( $1)) = WITH_CODE( $2);
         WITH_PART( $2) = NULL;
         WITH_CODE( $2) = NULL;
         FREEdoFreeTree( $2);
       }
     ;

part: BRACKET_L generator BRACKET_R wlassignblock COLON expr SEMIC
      { $$ = TBmakeWith( $2, TBmakeCode( $4, TBmakeExprs( $6, NULL)), NULL);
        CODE_USED( WITH_CODE( $$))++;
        PART_CODE( $2) = WITH_CODE( $$);
      }
    ;
     
generator: expr LE genidx genop expr steps width
           {
             if( ($7 != NULL) && ($6 == NULL)) {
               WARN( global.linenum,
                     ("width vector ignored due to missing step vector"));
               $7 = FREEdoFreeTree( $7);
             }
             $$ = TBmakePart( NULL,
                              $3,
                              TBmakeGenerator( F_le, $4, $1, $5, $6, $7));
           }
         | expr LT genidx genop expr steps width
           {
             if( ($7 != NULL) && ($6 == NULL)) {
               WARN( global.linenum,
                     ("width vector ignored due to missing step vector"));
               $7 = FREEdoFreeTree( $7);
             }
             $$ = TBmakePart( NULL,
                              $3,
                              TBmakeGenerator( F_lt, $4, $1, $5, $6, $7));
           }
         ;

steps: /* empty */   { $$ = NULL; }
     | STEP expr     { $$ = $2;   }
     ;

width: /* empty */   { $$ = NULL; }
     | WIDTH expr    { $$ = $2;   }
     ;

genidx: ID LET SQBR_L ids SQBR_R
        { $$ = TBmakeWithid( TCmakeIdsCopyString( $1, NULL), $4);
        }
      | ID
        { $$ = TBmakeWithid( TCmakeIdsCopyString( $1, NULL), NULL);
        }
      | SQBR_L ids SQBR_R
        { $$ = TBmakeWithid( NULL, $2);
        }
      ;


genop: LT   { $$ = F_lt; }
     | LE   { $$ = F_le; }
     ;


wlassignblock: BRACE_L { $<cint>$ = global.linenum; } assigns BRACE_R
               { if ($3 == NULL) {
                   $$ = MAKE_EMPTY_BLOCK();
                 }
                 else {
                   $$ = TBmakeBlock( $3, NULL);
                 }
                 NODE_LINE( $$) = $<cint>2;
               }
             | /* empty */
               { $$ = MAKE_EMPTY_BLOCK();
               }
             ;

nwithop: GENARRAY BRACKET_L expr COMMA expr BRACKET_R
         { $$ = TBmakeGenarray( $3, $5);
         }
       | GENARRAY BRACKET_L expr BRACKET_R
         { $$ = TBmakeGenarray( $3, NULL);
         }
       | MODARRAY BRACKET_L expr BRACKET_R
         { $$ = TBmakeModarray( $3);
         }
       | FOLD BRACKET_L qual_ext_id COMMA expr BRACKET_R
         { $$ = TBmakeFold( $5);
           FOLD_FUN( $$) = ILIBstringCopy( ID_SPNAME( $3));
           FOLD_MOD( $$) = ILIBstringCopy( ID_SPMOD( $3));
           $3 = FREEdoFreeTree( $3);
         }
       ;

withop: GENARRAY BRACKET_L expr COMMA expr BRACKET_R
        { $$ = TBmakeGenarray( $3, NULL);
          GENARRAY_SPEXPR( $$) = $5;
        }
      | GENARRAY BRACKET_L expr COMMA expr COMMA expr BRACKET_R
        { $$ = TBmakeGenarray( $3, $7);
          GENARRAY_SPEXPR( $$) = $5;
        }
      | MODARRAY BRACKET_L expr COMMA ID COMMA expr BRACKET_R
        { $$ = TBmakeModarray( $3);
          MODARRAY_SPEXPR( $$) = $7;
        }
      | FOLD BRACKET_L foldop COMMA expr COMMA expr BRACKET_R
        { $$ = TBmakeFold( $5);
          FOLD_PRF( $$) = $3;
          FOLD_SPEXPR( $$) = $7;
        }
      | FOLD BRACKET_L qual_ext_id COMMA expr COMMA expr BRACKET_R
        { $$ = TBmakeFold( $5);
          FOLD_FUN( $$) = ILIBstringCopy( ID_SPNAME( $3));
          FOLD_MOD( $$) = ILIBstringCopy( ID_SPMOD( $3));
          $3 = FREEdoFreeTree( $3);
          FOLD_SPEXPR( $$) = $7;
        }
      ;

foldop: PRF_ADD_SxS   { $$ = F_add_SxS; }
      | PRF_ADD_AxA   { $$ = F_add_AxA; }
      | PRF_MUL_SxS   { $$ = F_mul_SxS; }
      | PRF_MUL_AxA   { $$ = F_mul_AxA; }
      | PRF_MIN       { $$ = F_min;     }
      | PRF_MAX       { $$ = F_max;     }
      | PRF_EQ        { $$ = F_eq;      }
      | PRF_AND       { $$ = F_and;     }
      | PRF_OR        { $$ = F_or;      }
      ;

prf: foldop        { $$ = $1;        }
   | PRF_DIM       { $$ = F_dim;     }
   | PRF_SHAPE     { $$ = F_shape;   }
   | PRF_RESHAPE   { $$ = F_reshape; }
   | PRF_SEL       { $$ = F_sel;     }
   | PRF_GENARRAY  { $$ = F_genarray;}
   | PRF_MODARRAY  { $$ = F_modarray;}
   | PRF_ADD_SxA   { $$ = F_add_SxA; }
   | PRF_ADD_AxS   { $$ = F_add_AxS; }
   | PRF_SUB_SxS   { $$ = F_sub_SxS; }
   | PRF_SUB_SxA   { $$ = F_sub_SxA; }
   | PRF_SUB_AxS   { $$ = F_sub_AxS; }
   | PRF_SUB_AxA   { $$ = F_sub_AxA; }
   | PRF_MUL_SxA   { $$ = F_mul_SxA; }
   | PRF_MUL_AxS   { $$ = F_mul_AxS; }
   | PRF_DIV_SxS   { $$ = F_div_SxS; }
   | PRF_DIV_SxA   { $$ = F_div_SxA; }
   | PRF_DIV_AxS   { $$ = F_div_AxS; }
   | PRF_DIV_AxA   { $$ = F_div_AxA; }
   | PRF_MOD       { $$ = F_mod;     }
   | PRF_ABS       { $$ = F_abs;     }
   | PRF_NEG       { $$ = F_neg;     }
   | PRF_NEQ       { $$ = F_neq;     }
   | PRF_LT        { $$ = F_lt;      }
   | PRF_LE        { $$ = F_le;      }
   | PRF_GT        { $$ = F_gt;      }
   | PRF_GE        { $$ = F_ge;      }
   | PRF_NOT       { $$ = F_not;     }
   | PRF_TOI_S     { $$ = F_toi_S;   }
   | PRF_TOI_A     { $$ = F_toi_A;   }
   | PRF_TOF_S     { $$ = F_tof_S;   }
   | PRF_TOF_A     { $$ = F_tof_A;   }
   | PRF_TOD_S     { $$ = F_tod_S;   }
   | PRF_TOD_A     { $$ = F_tod_A;   }
   | PRF_CAT_VxV   { $$ = F_cat_VxV; }
   | PRF_TAKE_SxV  { $$ = F_take_SxV;}
   | PRF_DROP_SxV  { $$ = F_drop_SxV;}
   ;

qual_ext_id: ext_id
             { $$ = TBmakeId( NULL);
               ID_SPNAME( $$) = $1;
             }
           | ID COLON ext_id
             { $$ = TBmakeId( NULL);
               ID_SPNAME( $$) = $3;
               ID_SPMOD( $$) = $1;
             }
           ; 

qual_ext_ids: ext_id 
              { $$ = TBmakeIds( NULL, NULL);
                IDS_SPNAME( $$) = $1;
              }
            | ID COLON ext_id
              { $$ = TBmakeIds( NULL, NULL);
                IDS_SPNAME( $$) = $3;
                IDS_SPMOD( $$) = $1;
              }
            | ext_id COMMA qual_ext_ids
              { $$ = TBmakeIds( NULL, $3);
                IDS_SPNAME( $$) = $1;
              }
            | ID COLON ext_id COMMA qual_ext_ids
              { $$ = TBmakeIds( NULL, $5);
                IDS_SPNAME( $$) = $3;
                IDS_SPMOD( $$) = $1;
              }

            ;

ext_id: ID         { $$ = $1; }
      | reservedid { $$ = $1; }
      ; 

ids: ID COMMA ids
     { $$ = TBmakeIds( NULL, $3);
       IDS_SPNAME( $$) = $1;
     }
   | ID
     { $$ = TBmakeIds( NULL, NULL);
       IDS_SPNAME( $$) = $1;
     }
   ;

reservedid: GENARRAY          { $$ = ILIBstringCopy("genarray"); }
          | MODARRAY          { $$ = ILIBstringCopy("modarray"); }
          | ALL               { $$ = ILIBstringCopy("all"); }
          | AMPERS            { $$ = ILIBstringCopy("&"); }
          | EXCL              { $$ = ILIBstringCopy("!"); }
          | INC               { $$ = ILIBstringCopy("++"); }
          | DEC               { $$ = ILIBstringCopy("--"); }
          | PLUS              { $$ = ILIBstringCopy("+"); }
          | MINUS             { $$ = ILIBstringCopy("-"); }
          | STAR              { $$ = ILIBstringCopy("*"); }
          | LE                { $$ = ILIBstringCopy("<="); }
          | LT                { $$ = ILIBstringCopy("<"); }
          | GT                { $$ = ILIBstringCopy(">"); }
          ; 
string: STR       
        { $$ = $1;
        }
      | STR string
        { $$ = ILIBstringConcat( $1, $2);
          $1 = ILIBfree( $1);
          $2 = ILIBfree( $2);
        }
      ;

nums: NUM COMMA nums { $$ = TBmakeNums( $1, $3); }
      | NUM { $$ = TBmakeNums( $1, NULL); }
      ;




/*
 *********************************************************************
 *
 *  rules for ntype
 *
 *********************************************************************
 */

returntypes: TYPE_VOID   { $$ = NULL; }
           | ntypes      { $$ = $1;   }
           ;

returndectypes: TYPE_VOID   { $$ = NULL; }
              | varntypes      { $$ = $1;   }
              | DOT DOT DOT { $$ = NULL; have_seen_dots = TRUE; }
              ;

ntypes: ntype COMMA ntypes { $$ = TBmakeRet( $1, $3); }
      | ntype { $$ = TBmakeRet( $1,NULL); }
      ;

varntypes: ntype COMMA ntypes { $$ = TBmakeRet( $1, $3); }
         | ntype { $$ = TBmakeRet( $1,NULL); }
         | ntype COMMA DOT DOT DOT
         { $$ = TBmakeRet( $1,NULL);
           have_seen_dots = TRUE;
         }
         ;

ntype: basentype
       { $$ = TYmakeAKS( $1, SHmakeShape(0)); 
       }
     | basentype SQBR_L SQBR_R
       { $$ = TYmakeAKS( $1, SHmakeShape(0)); 
       }
     | basentype SQBR_L exprs SQBR_R
       { $$ = Exprs2NType( $1, $3);
       }
     ;

basentype: simplentype
           { $$ = $1;
           }
         | userntype
           { $$ = $1;
           }
         ;

simplentype: TYPE_INT    { $$ = TYmakeSimpleType( T_int);    }
           | TYPE_FLOAT  { $$ = TYmakeSimpleType( T_float);  }
           | TYPE_BOOL   { $$ = TYmakeSimpleType( T_bool);   }
           | TYPE_CHAR   { $$ = TYmakeSimpleType( T_char);   }
           | TYPE_DBL    { $$ = TYmakeSimpleType( T_double); }
           ;

userntype: ID
           { $$ = TYmakeSymbType( $1, NULL);
           }
         | ID COLON ID
           { $$ = TYmakeSymbType( $3, $1);
           }
         ;

/******************************************************************************
 ******************************************************************************
 *
 *  rules for module implementations
 *
 *  std-rules reused:
 *
 *    - defs
 *    - type
 *
 ******************************************************************************
 ******************************************************************************/

module: MODULE { file_kind = F_modimp; } ID { mod_name = $3; } SEMIC defs
        { $$ = $6;
          MODULE_NAME( $$) = mod_name;
          MODULE_FILETYPE( $$) = file_kind;
        }
        ;

class: CLASS { file_kind = F_classimp; } ID { mod_name = $3; } SEMIC
       CLASSTYPE ntype SEMIC defs
       { $$ = $9;
         MODULE_NAME( $$) = mod_name;
         MODULE_FILETYPE( $$) = file_kind;
         MODULE_CLASSTYPE( $$) = $7;
       }
     ;




/******************************************************************************
 ******************************************************************************
 *
 *  rules for module specializations( C interface only)
 *
 *  module declaration rules reused:
 *   - modheader
 *   - expdesc
 *
 ******************************************************************************
 ******************************************************************************/
/*
modspec: modheader OWN COLON expdesc
         { $$ = $1;
           MODSPEC_OWN( $$) = $4;
           MODSPEC_IMPORTS( $$) = NULL;
           DBUG_PRINT( "PARSE",
                       ("%s:"F_PTR" Id: %s , %s"F_PTR,
                        global.mdb_nodetype[ NODE_TYPE( $$)],
                        $$,
                        MODSPEC_NAME( $$),
                        global.mdb_nodetype[ NODE_TYPE( MODSPEC_OWN( $$))],
                        MODSPEC_OWN( $$)));
         }
       ;

*/


/*******************************************************************************
 *******************************************************************************
 *
 *  rules for sac2crc files
 *
 *  std-rules reused:
 *
 *    - string
 *
 *******************************************************************************
 *******************************************************************************/


targets: TARGET ID COLON inherits resources targets
         { $$ = RSCmakeTargetListEntry( $2, $4, $5, $6);
         }
       | /* empty */
         { $$ = NULL;
         }
       ;

inherits: COLON ID COLON inherits
           { $$ = RSCmakeInheritenceListEntry( $2, $4);
           }
        | /* empty */
          { $$ = NULL;
          } 
        ;

resources: ID COLON LET string resources
           { $$ = RSCmakeResourceListEntry( $1, $4, 0, 0, $5);
           }
         | ID ADDON string resources
           { $$ = RSCmakeResourceListEntry( $1, $3, 0, 1, $4);
           }
         | ID COLON LET OPTION resources
           { $$ = RSCmakeResourceListEntry( $1, $4, 0, 0, $5);
           }
         | ID ADDON OPTION resources
           { $$ = RSCmakeResourceListEntry( $1, $3, 0, 1, $4);
           }
         | ID COLON LET ID resources
           { $$ = RSCmakeResourceListEntry( $1, $4, 0, 0, $5);
           }
         | ID ADDON ID resources
           { $$ = RSCmakeResourceListEntry( $1, $3, 0, 1, $4);
           }
         | ID COLON LET NUM resources
           { $$ = RSCmakeResourceListEntry( $1, NULL, $4, 0, $5);
           }
         | ID ADDON NUM resources
           { $$ = RSCmakeResourceListEntry( $1, NULL, $3, 1, $4);
           }
         | /* empty */
           { $$ = NULL;
           }
         ;


%%



/*
 *********************************************************************
 *
 *  functions 
 *
 *********************************************************************
 */



/******************************************************************************
 *
 * Function:
 *   int SPmyYyparse()
 *
 * Description:
 *   
 *
 ******************************************************************************/

int SPmyYyparse()
{
  char *tmp;

  DBUG_ENTER( "SPmyYyparse");

  /* 
   * make a copy of the actual filename, which will be used for
   * all subsequent nodes...
   */
  tmp = (char *) ILIBmalloc( (strlen(global.filename)+1) * sizeof( char));
  strcpy( tmp, global.filename);
  global.filename = tmp;

#if YYDEBUG
  DBUG_EXECUTE( "YACC", yydebug=1;);
#endif

  DBUG_RETURN( yyparse());
}



/******************************************************************************
 *
 * Function:
 *   int yyerror( char *errname)
 *
 * Description:
 *   
 *
 ******************************************************************************/

static
int yyerror( char *errname)
{
  int offset = 0;
  int size_of_output;
  
  DBUG_ENTER( "yyerror");

  charpos -= (strlen( yytext) - 1);
  ERROR( global.linenum, ("%s at pos %d: '%s`", errname, charpos, yytext));
  size_of_output = MAX_LINE_LENGTH -
                   (((global.verbose_level > 1) ? 2 : 0) +
                    strlen( global.filename) +
                    ILIBnumberOfDigits( global.linenum) + 9);
  if (strlen( linebuf_ptr) > (size_t) size_of_output) {
    if (charpos >= size_of_output - 15) {
      offset = charpos - size_of_output + 15;
      strncpy( linebuf_ptr + offset, "... ", 4);
    }
    strcpy( linebuf_ptr + offset + size_of_output - 4, " ...");
  }

  CONT_ERROR(( "%s", linebuf_ptr + offset));
  CONT_ERROR(( "%*s", charpos - offset, "^"));

  ABORT_ON_ERROR;

  DBUG_RETURN( 0);
}



/******************************************************************************
 *
 * Function:
 *   void CleanUpParser()
 *
 * Description:
 *   
 *
 ******************************************************************************/

static
void CleanUpParser()
{
  DBUG_ENTER( "CleanUpParser");

  if (global_wlcomp_aps != NULL) {
    global_wlcomp_aps = FREEdoFreeTree( global_wlcomp_aps);
  }

  DBUG_VOID_RETURN;
}



/******************************************************************************
 *
 * Function:
 *   node *String2Array(char *str)
 *
 * Description:
 *   
 *
 ******************************************************************************/

static
node *String2Array(char *str)
{
  node *new_exprs;
  int i, cnt;
  node *array;
  node *len_exprs;
  node *res;

  DBUG_ENTER( "String2Array");

  new_exprs = TBmakeExprs( TBmakeChar( '\0'), NULL);

  cnt=0;
  
  for (i=strlen(str)-1; i>=0; i--) {
    if ((i>0) && (str[i-1]=='\\')) {
      switch (str[i]) {
      case 'n':
        new_exprs = TBmakeExprs(TBmakeChar('\n'), new_exprs);
        i-=1;
        break;
      case 't':
        new_exprs = TBmakeExprs(TBmakeChar('\t'), new_exprs);
        i-=1;
        break;
      case 'v':
        new_exprs = TBmakeExprs(TBmakeChar('\v'), new_exprs);
        i-=1;
        break;
      case 'b':
        new_exprs = TBmakeExprs(TBmakeChar('\b'), new_exprs);
        i-=1;
        break;
      case 'r':
        new_exprs = TBmakeExprs(TBmakeChar('\r'), new_exprs);
        i-=1;
        break;
      case 'f':
        new_exprs = TBmakeExprs(TBmakeChar('\f'), new_exprs);
        i-=1;
        break;
      case 'a':
        new_exprs = TBmakeExprs(TBmakeChar('\a'), new_exprs);
        i-=1;
        break;
      case '"':
        new_exprs = TBmakeExprs(TBmakeChar('"'), new_exprs);
        i-=1;
        break;
      default:
        new_exprs = TBmakeExprs(TBmakeChar(str[i]), new_exprs);
        break;
      }
    }
    else {
      new_exprs = TBmakeExprs(TBmakeChar(str[i]), new_exprs);
    }
    
    cnt+=1;
  }

  len_exprs = TBmakeExprs( TBmakeNum( cnt), NULL);
  array = TCmakeFlatArray( new_exprs);

#ifndef CHAR_ARRAY_NOT_AS_STRING
  ARRAY_STRING(array)=str;
#endif  /* CHAR_ARRAY_AS_STRING */

  res = TBmakeAp( NULL, TBmakeExprs( array, len_exprs));
  AP_SPNAME( res) = ILIBstringCopy( "to_string");

  DBUG_RETURN( res); 
}


/** <!--********************************************************************-->
 *
 * @fn node *MakeIncDecLet( char *id, char *op)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

static
node *MakeIncDecLet( char *name, char *op)
{
  node *let, *id, *ids, *ap;

  DBUG_ENTER( "MakeIncDecLet");
  ids = TBmakeIds( NULL, NULL);
  IDS_SPNAME( ids) = ILIBstringCopy( name);

  id = TBmakeId( NULL);
  ID_SPNAME(  id) = name;

  ap = TCmakeAp2( NULL, id, TBmakeNum(1));
  AP_SPNAME( ap) = op;

  let = TBmakeLet( ids, ap);

  DBUG_RETURN( let);
}


/** <!--********************************************************************-->
 *
 * @fn node *MakeOpOnLet( char *id, node *expr, char *op)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

static
node *MakeOpOnLet( char *name, node *expr, char *op)
{
  node *let, *id, *ids, *ap;

  DBUG_ENTER( "MakeOpOnLet");
  ids = TBmakeIds( NULL, NULL);
  IDS_SPNAME( ids) = ILIBstringCopy( name);

  id = TBmakeId( NULL);
  ID_SPNAME(  id) = name;

  ap = TCmakeAp2( NULL, id, expr);
  AP_SPNAME( ap) = op;

  let = TBmakeLet( ids, ap);

  DBUG_RETURN( let);
}

/******************************************************************************
 *
 * Function:
 *   node *Expr2Mop( node * expr)
 *
 * Description:
 *
 ******************************************************************************/

static
node *Expr2Mop( node *expr)
{
  node *res;

  DBUG_ENTER("Expr2Mop");

  if( (NODE_TYPE( expr) == N_mop) && ! MOP_ISFIXED( expr) ) {
    res = expr;
  } else {
    res = TBmakeMop( TBmakeExprs( expr, NULL), NULL);
  }

  DBUG_RETURN( res);
}



/******************************************************************************
 *
 * Function:
 *   node *ConstructMop( node *, node *, node *)
 *
 * Description:
 *
 ******************************************************************************/

static
node *ConstructMop( node *expr1, node *fun_id, node *expr2)
{
  node *res, *lmop, *rmop, *fun_ids;

  DBUG_ENTER("ConstructMop");

  lmop = Expr2Mop( expr1);
  rmop = Expr2Mop( expr2);

  fun_ids = DUPdupIdIds( fun_id);
  fun_id = FREEdoFreeNode( fun_id);

  IDS_NEXT( fun_ids) = MOP_OPS( rmop);

  res = TBmakeMop( TCappendExprs( MOP_EXPRS( lmop),
                                  MOP_EXPRS( rmop)),
                   TCappendIds( MOP_OPS( lmop),
                                fun_ids));
  lmop = FREEdoFreeNode( lmop);  /* only the top constructor N_mop is to be freed!!!! */
  rmop = FREEdoFreeNode( rmop);  /* only the top constructor N_mop is to be freed!!!! */

  DBUG_RETURN( res);
}



/******************************************************************************
 *
 * Function:
 *   node *CheckWlcompConf( node *conf, node *exprs)
 *
 * Description:
 *   Checks and converts the given wlcomp-pragma expression.
 *   Syntax of wlcomp-pragmas:
 *      pragma  ->  PRAGMA WLCOMP conf
 *      conf    ->  DEFAULT
 *               |  id BRACKET_L args conf BRACKET_R
 *      args    ->  $empty$
 *               |  arg COMMA args
 *      arg     ->  expr
 *      id      ->  ... identificator ...
 *      expr    ->  ... expression ...
 *   Examples:
 *      #pragma wlcomp Scheduling( Block(), Dynamic(),
 *                     BvL0( [3,3], [7,7],
 *                     Cubes(
 *                     Default)))
 *      #pragma wlcomp Conf3( ..., Conf2( ..., Conf1( ..., Default)))
 *   This nesting of applications is transformed into a N_exprs chain of
 *   wlcomp-pragma functions:
 *      Cubes()  ,  BvL0( [3,3], [7,7])  ,  Scheduling( Block(), Dynamic())
 *      Conf1(...)  ,  Conf2(...)  ,  Conf3(...)
 *
 *   Unfortunately, it is not possible to parse the wlcomp-pragma expression
 *   directly with yacc. 'conf' as well as 'arg' might start with an ID
 *   [ In the first example: Scheduling( Block(), ..., BvL0( ...)) ]
 *                                       ^^^^^         ^^^^
 *   This leads to an unsolvable shift/reduce conflict :-((
 *   Therefore 'conf' is simply parsed as an application/id and all the other
 *   stuff is done here.
 *
 ******************************************************************************/

static
node *CheckWlcompConf( node *conf, node *exprs)
{
  DBUG_ENTER( "CheckWlcompConf");

  DBUG_ASSERT( (conf != NULL), "wlcomp-pragma is empty!");

  if (NODE_TYPE( conf) == N_id) {
    if (strcmp( ID_SPNAME( conf), "Default")) {
      strcpy( yytext, ID_SPNAME( conf));
      yyerror( "innermost configuration is not 'Default'");
    }

    /*
     * free N_id node
     */
    conf = FREEdoFreeTree( conf);

    /*
     * unmodified 'exprs' is returned
     */
  }
  else if (NODE_TYPE( conf) == N_ap) {
    node *arg = AP_ARGS( conf);

    /*
     * look for last argument -> next 'conf'
     */
    if (arg == NULL) {
      strcpy( yytext, AP_NAME( conf));
      yyerror( "wlcomp-function with missing configuration found");
    }
    else {
      node *tmp;
      node *next_conf = NULL;

      if (EXPRS_NEXT( arg) == NULL) {
        next_conf = EXPRS_EXPR( arg);
        tmp = arg;
        AP_ARGS( conf) = NULL;
      }
      else {
        while (EXPRS_NEXT( EXPRS_NEXT( arg)) != NULL) {
          arg = EXPRS_NEXT( arg);
        }
        next_conf = EXPRS_EXPR( EXPRS_NEXT( arg));
        tmp = EXPRS_NEXT( arg);
        EXPRS_NEXT( arg) = NULL;
      }

      /*
       * free last N_exprs node
       */
      EXPRS_EXPR( tmp) = NULL;
      tmp = FREEdoFreeTree( tmp);

      if ((NODE_TYPE( next_conf) != N_id) && (NODE_TYPE( next_conf) != N_ap)) {
        strcpy( yytext, AP_NAME( conf));
        yyerror( "wlcomp-function with illegal configuration found");
      }
      else {
        /*
         * insert new configuration at head of 'exprs'
         */
        exprs = TBmakeExprs( conf, exprs);
        exprs = CheckWlcompConf( next_conf, exprs);
      }
    }
  }
  else {
    DBUG_ASSERT( (0), "wlcomp-pragma with illegal configuration found!");
  }

  DBUG_RETURN( exprs);
}

static int CountDotsInExprs( node *exprs)
{
  int result = 0;

  DBUG_ENTER("CountDotsInExprs");

  while (exprs != NULL) {
    if (NODE_TYPE( EXPRS_EXPR( exprs)) == N_dot) {
      result++;
    }

    exprs = EXPRS_NEXT( exprs);
  }

  DBUG_RETURN( result);
}

static shape *Exprs2Shape( node *exprs)
{
  shape *result;
  int n;
  int cnt = 0;

  DBUG_ENTER("Exprs2Shape");
  
  n = TCcountExprs( exprs);

  result = SHmakeShape( n);

  while ((exprs != NULL) && (result != NULL)) {
    if (NODE_TYPE( EXPRS_EXPR( exprs)) == N_num) {
      result = SHsetExtent( result, cnt, NUM_VAL( EXPRS_EXPR( exprs)));
    } else {
      result = SHfreeShape( result);
    }
    exprs = EXPRS_NEXT( exprs);
    cnt++;
  }

  DBUG_RETURN( result);
}

static ntype *Exprs2NType( ntype *basetype, node *exprs)
{
  int n;
  int dots = 0;
  shape *shp;
  ntype *result = NULL;

  DBUG_ENTER("Exprs2NType");

  n = TCcountExprs( exprs);

  switch (NODE_TYPE( EXPRS_EXPR1( exprs))) {
    case N_id:
      if (ID_SPMOD( EXPRS_EXPR1( exprs)) != NULL) {
        yyerror("illegal shape specification");
      } else if (ID_SPNAME( EXPRS_EXPR1( exprs))[1] != '\0') {
        yyerror("illegal shape specification");
      } else {
        switch (ID_SPNAME( EXPRS_EXPR1( exprs))[0]) {
          case '*':
            result = TYmakeAUD( basetype);
            break;
          case '+':
            result = TYmakeAUDGZ( basetype);
            break;
          default:
            yyerror("illegal shape specification");
            break;
        }
      }
      break;
    case N_dot:
      dots = CountDotsInExprs( exprs);
      if (dots != n) {
        yyerror("illegal shape specification");
      } else {
        result = TYmakeAKD( basetype, dots, SHmakeShape(0));
      }
      break;
    case N_num:
      shp = Exprs2Shape( exprs);
      if (shp != NULL) {
        result = TYmakeAKS( basetype, shp);
      } else {
        yyerror("illegal shape specification");
      }
      break;
    default:
      yyerror("illegal shape specification");
      break;
  }

  exprs = FREEdoFreeTree( exprs);

  DBUG_RETURN( result);
}
      
