%{


/*
*
* $Log$
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
#include "typecheck.h"
#include "DupTree.h"        /* for use of DupTree() */
#include "my_debug.h"
#include "Error.h"
#include "free.h"
#include "globals.h"
#include "handle_mops.h"
#include "new_types.h"

#include "resource.h"


extern int commlevel;
extern int charpos;
extern char *linebuf_ptr;
extern char *yytext;

extern int yylex();


node *syntax_tree;

static char *mod_name = MAIN_MOD_NAME;
static node *global_wlcomp_aps = NULL;
static node *store_pragma = NULL;

/*
* used to distinguish the different kinds of files
* which are parsed with this single parser
*/
static file_type file_kind = F_prog;

static int yyerror( char *errname);
static int yyparse();

static void CleanUpParser();
static node *String2Array( char *str);
static types *Exprs2ShpInfo( types *, node *);
static ntype *Exprs2NType( ntype *basetype, node *exprs);
static node *ConstructMop( node *, ids *, node *);
static node *CheckWlcompConf( node *ap, node *exprs);

static int prf_arity[] = {
#define PRF_IF( a, b, c, d, e, f, g, h) f
#include "prf_node_info.mac"
#undef PRF_IF
};

%}

%union { nodetype        nodetype;
 char            *id;
 ids             *ids;
 types           *types;
 ntype           *ntype;
 node            *node;
 int             cint;
 char            cchar;
 float           cfloat;
 double          cdbl;
 nums            *nums;
 deps            *deps;
 prf             prf;
 shape           *shape;
 statustype      statustype;
 strings         *strings;
 resource_list_t *resource_list_t;
 target_list_t   *target_list_t;
}

%token PARSE_PRG  PARSE_RC

%token BRACE_L  BRACE_R  BRACKET_L  BRACKET_R  SQBR_L  SQBR_R  COLON  SEMIC 
COMMA  AMPERS  DOT  QUESTION  ARROW 
INLINE  LET  TYPEDEF  OBJDEF  CLASSTYPE 
INC  DEC  ADDON  SUBON  MULON  DIVON  MODON 
K_MAIN  RETURN  IF  ELSE  DO  WHILE  FOR  NWITH  FOLD 
MODULE  IMPORT  EXPORT  PROVIDE  USE  GLOBAL  CLASS  ALL  EXCEPT
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

%token <id> ID  STR  PRIVATEID  OPTION

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
%type <node> fundec fundec2 fundecargs varargs
%type <node> mainargs  fundefargs  args  arg 
%type <node> exprblock  exprblock2  assignsOPTret  assigns  assign 
     let cond optelse  doloop whileloop forloop  assignblock
     lets
%type <node> exprs  expr  expr_ap  opt_arguments  expr_ar  expr_sel  with 
     generator  steps  width  nwithop  withop  wlassignblock  genidx 
     part  parts
%type <prf> genop  foldop  prf

%type <id> id  reservedid  symbolid  string
%type <ids> ids  local_fun_id  fun_id fun_ids

%type <types> returntypes  types  type  localtype  simpletype
%type <types> varreturntypes vartypes
%type <ntype> simplentype userntype basentype ntype

/* pragmas */
%type <id> pragmacachesim
%type <node> wlcomp_pragma_global  wlcomp_pragma_local  wlcomp_conf
%type <node> pragmas pragma pragmalist
/*
%type <node> pragmas 
*/

/* general helpers */
%type <nums> nums




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
%type <ids> inherits
%type <resource_list_t> resources



%right INC DEC STAR PLUS MINUS TILDE EXCL LE LT GT ID PRIVATEID 
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

file: PARSE_PRG  prg       { syntax_tree = $2; }
| PARSE_PRG  module    { syntax_tree = $2; }
| PARSE_PRG  class     { syntax_tree = $2; }
| PARSE_RC   targets   { target_list = RSCAddTargetList( $2, target_list); }
/*  | PARSE_SPEC modspec   { spec_tree = $2; } */
;

eof: { if (commlevel) {
 ABORT( linenum, ("Unterminated comment found"));

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
       MODUL_NAME( $$) = mod_name;
       MODUL_FILETYPE( $$) = F_prog;
     }
   ;

defs: interface def2
      { $$ = $2;
        MODUL_IMPORTS( $$) = $1;
      }
    | def2
      { $$ = $1; }
    ;

def2: typedef def2
      { $$ = $2;
        TYPEDEF_NEXT( $1) = MODUL_TYPES( $$);
        MODUL_TYPES( $$) = $1;
      }
    | def3
      { $$ = $1; }
    ;

def3: objdefs def4
      { $$ = $2;
        MODUL_OBJS( $$) = $1;
      }
    | def4
      { $$ = $1; }
    ;

def4: fundec def4
      { $$ = $2;
        MODUL_FUNDECS( $$) = AppendFundef( MODUL_FUNDECS( $$), $1);
      }
    | wlcomp_pragma_global main def5
      { $$ = $3;
        MODUL_FUNS( $$) = AppendFundef( MODUL_FUNS( $$), $2);
      } 
    | wlcomp_pragma_global fundef def4
      { $$ = $3;
        MODUL_FUNS( $$) = AppendFundef( MODUL_FUNS( $$), $2);
      }
    | def5
      { $$ = $1; }
    ;

def5: { $$ = MakeModul( NULL, F_prog, NULL, NULL, NULL, NULL, NULL);

        DBUG_PRINT( "PARSE",
	            ("%s:"F_PTR,
	             mdb_nodetype[ NODE_TYPE( $$)],
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

import: IMPORT id COLON ALL SEMIC
{ $$ = MakeImport( $2, TRUE, NULL, NULL);
}
| IMPORT id COLON ALL EXCEPT symbolset SEMIC
{ $$ = MakeImport( $2, TRUE, NULL, $6);
}
| IMPORT id COLON symbolset SEMIC
{ $$ = MakeImport( $2, FALSE, NULL, $4);
}
;

use: USE id COLON ALL SEMIC
{ $$ = MakeUse( $2, TRUE, NULL, NULL);
}
| USE id COLON ALL EXCEPT symbolset SEMIC
{ $$ = MakeUse( $2, TRUE, NULL, $6);
}
| USE id COLON symbolset SEMIC
{ $$ = MakeUse( $2, FALSE, NULL, $4);
}
;

export: EXPORT ALL SEMIC
{ $$ = MakeExport( TRUE, NULL, NULL);
}
| EXPORT ALL EXCEPT symbolset SEMIC
{ $$ = MakeExport( TRUE, NULL, $4);
}
| EXPORT symbolset SEMIC
{ $$ = MakeExport( FALSE, NULL, $2);
}
;

provide: PROVIDE ALL SEMIC
 { $$ = MakeProvide( TRUE, NULL, NULL);
 }
| PROVIDE ALL EXCEPT symbolset SEMIC
 { $$ = MakeProvide( TRUE, NULL, $4);
 }
| PROVIDE symbolset SEMIC
 { $$ = MakeProvide( FALSE, NULL, $2);
 }
;

symbolset: BRACE_L symbolsetentries BRACE_R
   { $$ = $2;
   }
 ;

symbolsetentries: symbolid COMMA symbolsetentries
       { $$ = MakeSymbol( $1, $3);
       }
     | symbolid
       { $$ = MakeSymbol( $1, NULL);
       }
     ;

symbolid: id                { $$ = $1; }
        | reservedid        { $$ = $1; }
        ;

/*
*********************************************************************
*
*  rules for typedefs
*
*********************************************************************
*/

typedef: TYPEDEF ntype id SEMIC 
         { $$ = MakeTypedef( $3, mod_name, $2, ST_regular, NULL);

           DBUG_PRINT( "PARSE",
                       ("%s:"F_PTR","F_PTR", Id: %s",
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$, 
                        TYPEDEF_NTYPE( $$),
                        TYPEDEF_NAME( $$)));
         }
       | EXTERN TYPEDEF id SEMIC
         { $$ = MakeTypedef( $3, mod_name, TYMakeSimpleType( T_hidden), ST_regular, NULL);

           DBUG_PRINT( "PARSE",
                       ("%s:"F_PTR","F_PTR", Id: %s",
                        mdb_nodetype[ NODE_TYPE( $$)],
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
           $$->node[0] = $2;
         }
       | objdef
         { $$ = $1;
         }
       ;

objdef: OBJDEF type id LET expr SEMIC 
        { $$ = MakeObjdef( $3, mod_name, $2, $5, NULL);

          DBUG_PRINT( "PARSE",
                      ("%s:"F_PTR","F_PTR", Id: %s",
                       mdb_nodetype[ NODE_TYPE( $$)],
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
          FUNDEF_INLINE( $$) = TRUE;
        }
      | fundef1
        { $$ = $1;
          FUNDEF_INLINE( $$) = FALSE;
        }
      ;

fundef1: returntypes BRACKET_L local_fun_id BRACKET_R BRACKET_L fundef2
         { $$ = $6;
           FUNDEF_TYPES( $$) = $1;                        /* result type(s) */
           FUNDEF_NAME( $$) = StringCopy( IDS_NAME( $3)); /* function name  */
           FUNDEF_MOD( $$) = StringCopy( IDS_MOD( $3));   /* module name    */

           $3 = FreeOneIds( $3);

           FUNDEF_ATTRIB( $$) = ST_regular;
           FUNDEF_STATUS( $$) = ST_regular;
           FUNDEF_INFIX( $$) = TRUE;

           DBUG_PRINT( "PARSE",
                        ("%s: %s:%s "F_PTR,
                        mdb_nodetype[ NODE_TYPE( $$)],
                        (FUNDEF_MOD( $$) == NULL) ? "(null)": FUNDEF_MOD( $$),
                        FUNDEF_NAME( $$),
                        FUNDEF_NAME( $$)));

         }
       | returntypes fun_id BRACKET_L fundef2
         { $$ = $4;
           FUNDEF_TYPES( $$) = $1;              /* result type(s) */
           FUNDEF_NAME( $$) = IDS_NAME( $2);    /* function name  */
           FUNDEF_MOD( $$) = IDS_MOD( $2);      /* module name    */
           FUNDEF_ATTRIB( $$) = ST_regular;
           FUNDEF_STATUS( $$) = ST_regular;
           FUNDEF_INFIX( $$) = FALSE;

           DBUG_PRINT( "PARSE",
                        ("%s: %s:%s "F_PTR,
                        mdb_nodetype[ NODE_TYPE( $$)],
                        (FUNDEF_MOD( $$) == NULL) ? "(null)" : FUNDEF_MOD( $$),
                        FUNDEF_NAME( $$),
                        FUNDEF_NAME( $$)));

         }
       ;

fundef2: fundefargs BRACKET_R
         { $$ = MakeFundef( NULL, NULL, NULL, NULL, NULL, NULL); }
         exprblock
         { 
           $$ = $<node>3;
           FUNDEF_BODY( $$) = $4;             /* function bdoy  */
           FUNDEF_ARGS( $$) = $1;             /* fundef args */
      
           DBUG_PRINT( "PARSE",
                       ("%s:"F_PTR", Id: %s"F_PTR" %s," F_PTR,
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$, 
                        mdb_nodetype[ NODE_TYPE( FUNDEF_BODY( $$))],
                        FUNDEF_BODY( $$),
                        mdb_nodetype[ NODE_TYPE( FUNDEF_ARGS( $$))],
                        FUNDEF_ARGS( $$)));
         }
       | BRACKET_R { $$ = MakeFundef( NULL, NULL, NULL, NULL, NULL, NULL); }
         exprblock
         { $$ = $<node>2;
           FUNDEF_BODY( $$) = $3;             /* Funktionsrumpf  */

           DBUG_PRINT( "PARSE",
                       ("%s:"F_PTR" %s"F_PTR,
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$, 
                        mdb_nodetype[ NODE_TYPE( FUNDEF_BODY( $$))],
                        FUNDEF_BODY( $$)));
         }
       ;


fundefargs: args        { $$ = $1;   }
          | TYPE_VOID   { $$ = NULL; }
          ;

args: arg COMMA args
      { ARG_NEXT($1) = $3;
        $$ = $1;
      }
    | arg
      { $$ = $1;
      }
    ;

arg: type id
     { $$ = MakeArg( $2, $1, ST_regular, ST_regular, NULL); 

       DBUG_PRINT( "PARSE",
                   ("%s: "F_PTR", Id: %s, Attrib: %d  ",
                    mdb_nodetype[ NODE_TYPE( $$)],
                    $$, 
                    ARG_NAME( $$),
                    ARG_ATTRIB( $$)));
     }
   | type AMPERS id
     { $$ = MakeArg( $3, $1, ST_regular, ST_reference, NULL); 

       DBUG_PRINT( "PARSE",
                   ("%s: "F_PTR", Id: %s, Attrib: %d ",
                    mdb_nodetype[ NODE_TYPE( $$)],
                    $$, 
                    ARG_NAME( $$),
                    ARG_ATTRIB( $$)));
     }
   ;


main: TYPE_INT K_MAIN BRACKET_L mainargs BRACKET_R { $<cint>$ = linenum; } exprblock
      { $$ = MakeFundef( NULL, NULL,
                         MakeTypes1( T_int),
                         $4, $7, NULL);
        NODE_LINE( $$) = $<cint>6;

        FUNDEF_NAME( $$) = StringCopy( "main");
        FUNDEF_STATUS( $$) = ST_exported;

        DBUG_PRINT( "PARSE",
                    ("%s:"F_PTR", main "F_PTR " %s (" F_PTR ") ",
                     mdb_nodetype[ NODE_TYPE( $$)],
                     $$, 
                     FUNDEF_NAME( $$),
                     mdb_nodetype[ NODE_TYPE( FUNDEF_BODY($$))],
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

fundec: EXTERN varreturntypes local_fun_id BRACKET_L fundec2
        { $$ = $5;
          FUNDEF_TYPES( $$) = $2;
          FUNDEF_NAME( $$) = StringCopy( IDS_NAME( $3));  /* function name */
          FUNDEF_MOD( $$) = StringCopy( IDS_MOD( $3));    /* module name   */
          $3 = FreeOneIds( $3);
          FUNDEF_ATTRIB( $$) = ST_regular;
          FUNDEF_STATUS( $$) = ST_Cfun;
        }
      ;

fundec2: fundecargs BRACKET_R { $<cint>$ = linenum; } pragmas SEMIC
         { $$ = MakeFundef( NULL, NULL, NULL, $1, NULL, NULL);
           NODE_LINE( $$) = $<cint>3;
           FUNDEF_PRAGMA( $$) = $4;
         }
       | BRACKET_R { $<cint>$ = linenum; } pragmas SEMIC
         { $$ = MakeFundef( NULL, NULL, NULL, NULL, NULL, NULL);
           NODE_LINE( $$) = $<cint>2;
           FUNDEF_PRAGMA( $$) = $3;
         }
       ;

fundecargs: varargs       { $$ = $1; }
          | TYPE_VOID     { $$ = NULL; }
          | DOT DOT DOT   
            { $$ = MakeArg( NULL, MakeTypes1( T_dots),
                            ST_regular, ST_regular, NULL);
            }
          ;

varargs: arg COMMA varargs
         { $$ = $1;
           ARG_NEXT( $$) = $3;
         }
       | arg COMMA DOT DOT DOT
         { $$ = $1;
           ARG_NEXT( $$) = MakeArg( NULL,
                                    MakeTypes1( T_dots),
                                    ST_regular, ST_regular,
                                    NULL);
         }
       | arg
         { $$ = $1;
         }
       ;

varreturntypes: TYPE_VOID     { $$ = MakeTypes1( T_void); }
              | vartypes      { $$ = $1; }
              ;

vartypes: type COMMA vartypes
          { $$ = $1;
            TYPES_NEXT( $$) = $3;
          }
        | type
          { $$ = $1;
          }
        | DOT DOT DOT
          { $$ = MakeTypes1( T_dots);
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
                          global_wlcomp_aps = FreeTree( global_wlcomp_aps);
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
                         $$ = MakePragma();
                         PRAGMA_WLCOMP_APS( $$) = $3;
                       } else {
                         $$ = NULL;
                       }
                     }
                   | /* empty */
                     { if (global_wlcomp_aps != NULL) {
                         $$ = MakePragma();
                         PRAGMA_WLCOMP_APS( $$) = DupTree( global_wlcomp_aps);
                       } else {
                         $$ = NULL;
                       }
                     }
                   ;

wlcomp_conf: id        { $$ = MakeId( $1, NULL, ST_regular); }
           | expr_ap   { $$ = $1;                            }
           ;


pragmacachesim: hash_pragma CACHESIM string   { $$ = $3;              }
              | hash_pragma CACHESIM          { $$ = StringCopy( ""); }
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
            store_pragma = MakePragma();
          }
          if (PRAGMA_LINKNAME( store_pragma) != NULL) {
            WARN( linenum, ("Conflicting definitions of pragma 'linkname`"));
          }
          PRAGMA_LINKNAME( store_pragma) = $3;
        }
      | hash_pragma LINKWITH string
        { if (store_pragma == NULL) {
            store_pragma = MakePragma();
          }
          if (PRAGMA_LINKMOD( store_pragma) != NULL) {
            WARN( linenum, ("Conflicting definitions of pragma 'linkmod`"));
          }
          PRAGMA_LINKMOD( store_pragma) = $3;
        }
      | hash_pragma LINKOBJ string
        { if (store_pragma == NULL) {
            store_pragma = MakePragma();
          }
          if (PRAGMA_LINKOBJ( store_pragma) != NULL) {
            WARN( linenum, ("Conflicting definitions of pragma 'linkmod`"));
          }
          PRAGMA_LINKOBJ( store_pragma) = $3;
        }
      | hash_pragma LINKSIGN SQBR_L nums SQBR_R
        { if (store_pragma == NULL) {
            store_pragma = MakePragma();
          }
          if (PRAGMA_LINKSIGNNUMS( store_pragma) != NULL) {
            WARN(linenum, ("Conflicting definitions of pragma 'linksign`"));
          }
          PRAGMA_LINKSIGNNUMS( store_pragma) = $4;
        }
      | hash_pragma REFCOUNTING SQBR_L nums SQBR_R
        { if (store_pragma == NULL) {
            store_pragma = MakePragma();
          }
          if (PRAGMA_REFCOUNTINGNUMS( store_pragma) != NULL) {
            WARN(linenum, ("Conflicting definitions of pragma 'refcounting`"));
          }
          PRAGMA_REFCOUNTINGNUMS( store_pragma) = $4;
        }
      | hash_pragma READONLY SQBR_L nums SQBR_R
        { if (store_pragma == NULL) {
            store_pragma = MakePragma();
          }
          if (PRAGMA_READONLYNUMS( store_pragma) != NULL) {
            WARN(linenum, ("Conflicting definitions of pragma 'readonly`"));
          }
          PRAGMA_READONLYNUMS( store_pragma) = $4;
        }
      | hash_pragma EFFECT fun_ids
        { if (store_pragma == NULL) {
            store_pragma = MakePragma();
          }
          if (PRAGMA_EFFECT( store_pragma) != NULL) {
            WARN(linenum, ("Conflicting definitions of pragma 'effect`"));
          }
          PRAGMA_EFFECT( store_pragma) = $3;
        }
      | hash_pragma TOUCH fun_ids
        { if (store_pragma == NULL) {
            store_pragma = MakePragma();
          }
          if (PRAGMA_TOUCH( store_pragma) != NULL) {
            WARN(linenum, ("Conflicting definitions of pragma 'touch`"));
          }
          PRAGMA_TOUCH( store_pragma) = $3;
        }
      | hash_pragma COPYFUN string
        { if (store_pragma == NULL) {
            store_pragma = MakePragma();
          }
          if (PRAGMA_COPYFUN( store_pragma) != NULL) {
            WARN(linenum, ("Conflicting definitions of pragma 'copyfun`"));
          }
          PRAGMA_COPYFUN( store_pragma) = $3;
        }
      | hash_pragma FREEFUN string
        { if (store_pragma == NULL) {
            store_pragma = MakePragma();
          }
          if (PRAGMA_FREEFUN( store_pragma) != NULL) {
            WARN(linenum, ("Conflicting definitions of pragma 'freefun`"));
          }
          PRAGMA_FREEFUN( store_pragma) = $3;
        }
      | hash_pragma INITFUN string
        { if (store_pragma == NULL) {
            store_pragma = MakePragma();
          }
          if (PRAGMA_INITFUN( store_pragma) != NULL) {
            WARN(linenum, ("Conflicting definitions of pragma 'initfun`"));
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

exprblock: BRACE_L { $<cint>$ = linenum; } pragmacachesim exprblock2
           { $$ = $4;
             BLOCK_CACHESIM( $$) = $3;
             NODE_LINE( $$) = $<cint>2;
           }
         ;

exprblock2: type ids SEMIC exprblock2
            { node *vardec_ptr;
              ids  *ids_ptr = $2;

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
                vardec_ptr = MakeVardec( IDS_NAME( $2),
                                         DupAllTypes( $1),
                                         vardec_ptr);
                /*
                 * Now, we want to "push" $2 one IDS further
                 * and we want to FREE the current IDS structure.
                 * Since we have recycled the IDS_ID, we can NOT
                 * use a given function from free.c such as 'FreeOneIds'
                 * or 'FreeAllIds'.
                 * Therefore, we introduce a temporary ids_ptr, which holds
                 * the ptr to the "next $2" and manually free the current $2.
                 */
                ids_ptr = IDS_NEXT( $2);
                $2 = Free( $2);
                $2 = ids_ptr;
              }
              /*
               * When we reach this point, all but one vardec is constructed!
               * Therefore, we can recycle the types node from $1 instead of
               * duplicating it as done in the loop above!
               */
              $$ = $4;
              BLOCK_VARDEC( $$) = MakeVardec( IDS_NAME( $2),
                                              $1,
                                              vardec_ptr);
              $2 = Free( $2);   /* Finally, we free the last IDS-node! */
            }
          | assignsOPTret BRACE_R
            { $$ = MakeBlock( $1, NULL);
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
               { $$ = MakeAssign( MakeReturn( NULL), NULL);
               }
             | RETURN BRACKET_L { $<cint>$ = linenum; } exprs BRACKET_R SEMIC
               { $$ = MakeAssign( MakeReturn( $4), NULL);
                 NODE_LINE( $$) = $<cint>3;
               }
             | RETURN BRACKET_L { $<cint>$ = linenum; } BRACKET_R SEMIC
               { $$ = MakeAssign( MakeReturn( NULL), NULL);
                 NODE_LINE( $$) = $<cint>3;
               }
             | RETURN { $<cint>$ = linenum; } SEMIC
               { $$ = MakeAssign( MakeReturn( NULL), NULL);
                 NODE_LINE( $$) = $<cint>3;
               }
             | assign { $<cint>$ = linenum; } assignsOPTret
               {
                 $$ = AppendAssign( $1, $3);
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
       | assign { $<cint>$ = linenum; } assigns
         { 
           $$ = AppendAssign( $1, $3);
           NODE_LINE($$) = $<cint>2;
           /*
            * $1 may be a former for-loop in which case it may point
            * to an entire chain of assignments.
            */
         }
       ;

assign: let SEMIC       { $$ = MakeAssign( $1, NULL); }
      | cond            { $$ = MakeAssign( $1, NULL); }
      | doloop SEMIC    { $$ = MakeAssign( $1, NULL); }
      | whileloop       { $$ = MakeAssign( $1, NULL); }
      | forloop         { $$ = $1; /* forloop already produces assign node. */}
      ;

let:       ids LET { $<cint>$ = linenum; } expr
           { $$ = MakeLet( $4, $1);
             NODE_LINE( $$) = $<cint>3;
           }
         | id SQBR_L exprs SQBR_R LET { $<cint>$ = linenum; } expr
           { if( CountExprs( $3) > 1) {
               $3 = MakeFlatArray( $3);
             } else {
               node * tmp;

               tmp = $3;
               $3 = EXPRS_EXPR( $3);
               EXPRS_EXPR( tmp) = NULL;
               tmp = FreeNode( tmp);
             }
             $$ = MakeLet( MakeAp( StringCopy( "modarray"),
                                   NULL,
                                   MakeExprs( MakeId( $1, NULL, ST_regular),
                                     MakeExprs( $3,
                                       MakeExprs( $7,
                                         NULL)))),
                           MakeIds( StringCopy( $1), NULL, ST_regular));
             NODE_LINE( $$) = $<cint>5;
           }
         | expr_ap { $$ = MakeLet( $1, NULL); }
         | id INC { $$ = MAKE_INCDEC_LET( $1, "+"); }
         | INC id { $$ = MAKE_INCDEC_LET( $2, "+"); }
         | id DEC { $$ = MAKE_INCDEC_LET( $1, "-"); }
         | DEC id { $$ = MAKE_INCDEC_LET( $2, "-"); }
         | id ADDON expr { $$ = MAKE_OPON_LET( $1, $3, "+"); }
         | id SUBON expr { $$ = MAKE_OPON_LET( $1, $3, "-"); }
         | id MULON expr { $$ = MAKE_OPON_LET( $1, $3, "*"); }
         | id DIVON expr { $$ = MAKE_OPON_LET( $1, $3, "/"); }
         | id MODON expr { $$ = MAKE_OPON_LET( $1, $3, "%"); }
         ;

cond: IF { $<cint>$ = linenum; } BRACKET_L expr BRACKET_R assignblock optelse
      {
        $$ = MakeCond( $4, $6, $7);
        NODE_LINE( $$) = $<cint>2;
      }
      ;

optelse: ELSE assignblock           { $$ = $2;                 }
       | /* empty */   %prec ELSE   { $$ = MAKE_EMPTY_BLOCK(); }
       ;

doloop: DO { $<cint>$ = linenum; } assignblock
        WHILE BRACKET_L expr BRACKET_R 
        {
          $$ = MakeDo( $6, $3);
          NODE_LINE( $$) = $<cint>2;
        }
      ;

whileloop: WHILE { $<cint>$ = linenum; } BRACKET_L expr BRACKET_R
           assignblock
           {
             $$ = MakeWhile( $4, $6);
             NODE_LINE( $$) = $<cint>2;
           }
         ;

forloop:   FOR { $<cint>$ = linenum; }
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
             
             BLOCK_INSTR( $10) = AppendAssign( BLOCK_INSTR( $10), $8);
             while_assign = MakeAssign( MakeWhile( $6, $10), NULL);
             NODE_LINE( while_assign) = $<cint>2;
             NODE_LINE( ASSIGN_INSTR( while_assign)) = $<cint>2;
             $$ = AppendAssign( $4, while_assign);
           }
         ;


lets: let COMMA lets
      {
        $$ = MakeAssign( $1, $3);
      }
      | let
      {
        $$ = MakeAssign( $1, NULL);
      }
      |
      {
        $$ = NULL;
      }
    ; 

assignblock: SEMIC
             { $$ = MAKE_EMPTY_BLOCK();
             }
           | BRACE_L { $<cint>$ = linenum; } pragmacachesim assigns BRACE_R
             { if ($4 == NULL) {
                 $$ = MAKE_EMPTY_BLOCK();
               }
               else {
                 $$ = MakeBlock( $4, NULL);
                 BLOCK_CACHESIM( $$) = $3;
                 NODE_LINE( $$) = $<cint>2;
               }
             }
           | assign
             { $$ = MakeBlock( $1, NULL);
             }
           ;



/*
 *********************************************************************
 *
 *  rules for expressions
 *
 *********************************************************************
 */

exprs: expr COMMA exprs          { $$ = MakeExprs( $1, $3);   }
     | expr                      { $$ = MakeExprs( $1, NULL); }
     ;

expr: fun_id                     { $$ = MakeIdFromIds( $1); }
    | DOT                        { $$ = MakeDot( 1);        }
    | DOT DOT DOT                { $$ = MakeDot( 3);        }
    | NUM                        { $$ = MakeNum( $1);       }
    | FLOAT                      { $$ = MakeFloat( $1);     }
    | DOUBLE                     { $$ = MakeDouble( $1);    }
    | CHAR                       { $$ = MakeChar( $1);      }
    | TRUETOKEN                  { $$ = MakeBool( 1);       }
    | FALSETOKEN                 { $$ = MakeBool( 0);       }
    | string                     { $$ = String2Array( $1);  }
    | BRACKET_L expr BRACKET_R
      { $$ = $2;
        if( NODE_TYPE( $2) == N_mop) {
          MOP_FIX( $$) = TRUE;
        }
      }
    | expr fun_id expr %prec BM_OP
      {
        $$ = ConstructMop( $1, $2, $3);
      }
    | PLUS expr %prec MM_OP
      {
        $$ = MakeAp1( StringCopy( "+"),
                      NULL,
                      $2);
      }
    | MINUS expr %prec MM_OP
      {
        $$ = MakeAp1( StringCopy( "-"),
                      NULL,
                      $2);
      }
    | TILDE expr %prec MM_OP
      {
        $$ = MakeAp1( StringCopy( "~"),
                      NULL,
                      $2);
      }
    | EXCL expr %prec MM_OP
      {
        $$ = MakeAp1( StringCopy( "!"),
                      NULL,
                      $2);
      }
    | PLUS BRACKET_L expr COMMA exprs BRACKET_R
      {
        $$ = MakeAp( StringCopy( "+"),
                     NULL,
                     MakeExprs( $3, $5));
      }
    | MINUS BRACKET_L expr COMMA exprs BRACKET_R
      {
        $$ = MakeAp( StringCopy( "-"),
                     NULL,
                     MakeExprs( $3, $5));
      }
    | TILDE BRACKET_L expr COMMA exprs BRACKET_R
      {
        $$ = MakeAp( StringCopy( "~"),
                     NULL,
                     MakeExprs( $3, $5));
      }
    | EXCL BRACKET_L expr COMMA exprs BRACKET_R
      {
        $$ = MakeAp( StringCopy( "!"),
                     NULL,
                     MakeExprs( $3, $5));
      }
    | expr_sel                    { $$ = $1; }   /* bracket notation      */
    | expr_ap                     { $$ = $1; }   /* prefix function calls */
    | expr_ar                     { $$ = $1; }   /* constant arrays       */
    | BRACKET_L COLON type BRACKET_R expr   %prec CAST
      { $$ = MakeCast( $5, $3);
      }
    | BRACE_L id ARROW expr BRACE_R
      { $$ = MakeSetWL( MakeId( $2, NULL, ST_regular),
                        $4);
      }
    | BRACE_L SQBR_L exprs SQBR_R ARROW expr BRACE_R
      { $$ = MakeSetWL( $3, $6);
      }
    | wlcomp_pragma_local NWITH { $<cint>$ = linenum; } with
      { $$ = $4;
        NODE_LINE( $$)= $<cint>3;
        NWITH_PRAGMA( $$) = $1;
      }
    ;
      

with: BRACKET_L generator BRACKET_R wlassignblock withop
      { 
#if 0
        /*
         * For now, we do not yet ask for the new syntax, BUT later we will
         * activate the following two lines....
         */
        WARN( linenum, ( "Old with-loop style depricated!"));
        CONT_WARN( ("Please use the new syntax instead"));
#endif

        /*
         * the tricky part about this rule is that $5 (an N_Nwithop node)
         * carries the goal-expression of the With-Loop, i.e., the "N-expr"
         * node which belongs into the N_Ncode node!!!
         * The reason for this is that an exclusion of the goal expression
         * from the non-terminal withop would lead to a shift/reduce
         * conflict in that rule!
         */
        $$ = MakeNWith( $2, MakeNCode( $4, MakeExprs( NWITHOP_EXPR( $5),
                                                      NULL)), $5);
        NWITHOP_EXPR( $5) = NULL;
        NCODE_USED( NWITH_CODE( $$))++;
        /*
         * Finally, we generate the link between the (only) partition
         * and the (only) code!
         */
        NPART_CODE( NWITH_PART( $$)) = NWITH_CODE( $$);
      }
    | BRACKET_L id BRACKET_R parts nwithop
      { $$ = $4;
        NWITH_WITHOP( $$) = $5;
        /*
         * At the time being we ignore $2. However, it SHOULD be checked
         * against all genidxs in $4 here....
         */
      }
    ;


expr_sel: expr SQBR_L exprs SQBR_R
          { if( CountExprs($3) == 1) {
              $$ = MakeAp2( StringCopy( "sel"),
                            NULL,
                            EXPRS_EXPR( $3),
                            $1);
              EXPRS_EXPR( $3) = NULL;
              $3 = FreeNode( $3);
            } else {
              $$ = MakeAp2( StringCopy( "sel"),
                            NULL,
                            MakeFlatArray( $3),
                            $1);
            }
          }
        | expr SQBR_L SQBR_R
          { $$ = MakeAp2( StringCopy( "sel"),
                          NULL,
                          MakeFlatArray( NULL),
                          $1);
          }
        ;

expr_ap: fun_id BRACKET_L { $<cint>$ = linenum; } opt_arguments BRACKET_R
         {
           $$ = MakeAp( StringCopy( IDS_NAME( $1)),
                        StringCopy( IDS_MOD( $1)),
                        $4);
           NODE_LINE( $$) = $<cint>3;
           $1 = FreeAllIds( $1);
         }
       | prf BRACKET_L { $<cint>$ = linenum; } opt_arguments BRACKET_R
         { char tmp[64];
           int num_args;

           num_args = CountExprs( $4);
           if( num_args != prf_arity[$1]) {
             sprintf( tmp, "%d argument(s) expected instead of %d", prf_arity[$1], num_args);
             yyerror( tmp);
           } else {
             $$ = MakePrf( $1, $4);
           }
           NODE_LINE( $$) = $<cint>3;
         }
       ;

opt_arguments: exprs         { $$ = $1;   }
             | /* empty */   { $$ = NULL; }
             ;

expr_ar: SQBR_L { $<cint>$ = linenum; } exprs SQBR_R
         { $$ = MakeFlatArray( $3);
           NODE_LINE( $$) = $<cint>2;
         }
       | SQBR_L { $<cint>$ = linenum; } SQBR_R
         { $$ = MakeFlatArray( NULL);
           NODE_LINE( $$) = $<cint>2;
         }
       ;

parts: part
       { $$ = $1;
       }
     | part parts
       { $$ = $1;
         NPART_NEXT( NWITH_PART( $1)) = NWITH_PART( $2);
         NCODE_NEXT( NWITH_CODE( $1)) = NWITH_CODE( $2);
         NWITH_PART( $2) = NULL;
         NWITH_CODE( $2) = NULL;
         FreeTree( $2);
       }
     ;

part: BRACKET_L generator BRACKET_R wlassignblock COLON expr SEMIC
      { $$ = MakeNWith( $2, MakeNCode( $4, MakeExprs( $6, NULL)), NULL);
        NCODE_USED( NWITH_CODE( $$))++;
        NPART_CODE( $2) = NWITH_CODE( $$);
      }
    ;
     
generator: expr LE genidx genop expr steps width
           {
             if( ($7 != NULL) && ($6 == NULL)) {
               WARN( linenum,
                     ("width vector ignored due to missing step vector"));
               $7 = FreeTree( $7);
             }
             $$ = MakeNPart( $3,
                             MakeNGenerator( $1, $5, F_le, $4, $6, $7),
                             NULL);
           }
         | expr LT genidx genop expr steps width
           {
             if( ($7 != NULL) && ($6 == NULL)) {
               WARN( linenum,
                     ("width vector ignored due to missing step vector"));
               $7 = FreeTree( $7);
             }
             $$ = MakeNPart( $3,
                             MakeNGenerator( $1, $5, F_lt, $4, $6, $7),
                             NULL);
           }
         ;

steps: /* empty */   { $$ = NULL; }
     | STEP expr     { $$ = $2;   }
     ;

width: /* empty */   { $$ = NULL; }
     | WIDTH expr    { $$ = $2;   }
     ;

genidx: id LET SQBR_L ids SQBR_R
        { $$ = MakeNWithid( MakeIds( $1, NULL, ST_regular), $4);
        }
      | id
        { $$ = MakeNWithid( MakeIds( $1, NULL, ST_regular), NULL);
        }
      | SQBR_L ids SQBR_R
        { $$ = MakeNWithid( NULL, $2);
        }
      ;


genop: LT   { $$ = F_lt; }
     | LE   { $$ = F_le; }
     ;


wlassignblock: BRACE_L { $<cint>$ = linenum; } assigns BRACE_R
               { if ($3 == NULL) {
                   $$ = MAKE_EMPTY_BLOCK();
                 }
                 else {
                   $$ = MakeBlock( $3, NULL);
                 }
                 NODE_LINE( $$) = $<cint>2;
               }
             | /* empty */
               { $$ = MAKE_EMPTY_BLOCK();
               }
             ;

nwithop: GENARRAY BRACKET_L expr COMMA expr BRACKET_R
         { $$ = MakeNWithOp( WO_genarray, $3);
           NWITHOP_DEFAULT( $$) = $5;
         }
       | GENARRAY BRACKET_L expr BRACKET_R
         { $$ = MakeNWithOp( WO_genarray, $3);
           NWITHOP_DEFAULT( $$) = NULL;
         }
       | MODARRAY BRACKET_L expr BRACKET_R
         { $$ = MakeNWithOp( WO_modarray, $3);
         }
       | FOLD BRACKET_L fun_id COMMA expr BRACKET_R
         { $$ = MakeNWithOp( WO_foldfun, $5);
           NWITHOP_FUN( $$) = StringCopy( IDS_NAME( $3));
           NWITHOP_MOD( $$) = IDS_MOD( $3);
           $3 = FreeOneIds( $3);
         }
       ;

withop: GENARRAY BRACKET_L expr COMMA expr BRACKET_R
        { $$ = MakeNWithOp( WO_genarray, $3);
          NWITHOP_EXPR( $$) = $5;
        }
      | GENARRAY BRACKET_L expr COMMA expr COMMA expr BRACKET_R
        { $$ = MakeNWithOp( WO_genarray, $3);
          NWITHOP_EXPR( $$) = $5;
          NWITHOP_DEFAULT( $$) = $7;
        }
      | MODARRAY BRACKET_L expr COMMA id COMMA expr BRACKET_R
        { $$ = MakeNWithOp( WO_modarray, $3);
          NWITHOP_EXPR( $$) = $7;
        }
      | FOLD BRACKET_L foldop COMMA expr COMMA expr BRACKET_R
        { $$ = MakeNWithOp( WO_foldprf, $5);
          NWITHOP_PRF( $$) = $3;
          NWITHOP_EXPR( $$) = $7;
        }
      | FOLD BRACKET_L fun_id COMMA expr COMMA expr BRACKET_R
        { $$ = MakeNWithOp( WO_foldfun, $5);
          NWITHOP_FUN( $$) = StringCopy( IDS_NAME( $3));
          NWITHOP_MOD( $$) = IDS_MOD( $3);
          NWITHOP_EXPR( $$) = $7;
          $3 = FreeOneIds( $3);
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

fun_id: local_fun_id
        { $$ = $1;
        }
      | id COLON local_fun_id
        { $$ = $3;
          IDS_MOD( $$) = $1;
        }
      ; 

fun_ids: fun_id COMMA fun_ids
         { $$ = $1;
           IDS_NEXT( $$) = $3;
         }
       | fun_id
         { $$ = $1;
         }
       ;

local_fun_id: id         { $$ = MakeIds( $1,
                                         NULL, ST_regular); }
            | reservedid { $$ = MakeIds( $1,
                                         NULL, ST_regular); }
            ; 

ids: id COMMA ids
     { $$ = MakeIds( $1, NULL, ST_regular);
       IDS_NEXT( $$) = $3;
     }
   | id
     { $$ = MakeIds( $1, NULL, ST_regular);
     }
   ;

id: ID
    { $$ = $1;
    }
  | PRIVATEID
    { ABORT( linenum, ("Identifier name '%s` illegal", $1));
    }
  ;

reservedid: GENARRAY          { $$ = StringCopy("genarray"); }
          | MODARRAY          { $$ = StringCopy("modarray"); }
          | ALL               { $$ = StringCopy("all"); }
          | AMPERS            { $$ = StringCopy("&"); }
          | EXCL              { $$ = StringCopy("!"); }
          | INC               { $$ = StringCopy("++"); }
          | DEC               { $$ = StringCopy("--"); }
          | PLUS              { $$ = StringCopy("+"); }
          | MINUS             { $$ = StringCopy("-"); }
          | STAR              { $$ = StringCopy("*"); }
          | LE                { $$ = StringCopy("<="); }
          | LT                { $$ = StringCopy("<"); }
          | GT                { $$ = StringCopy(">"); }
          ; 
string: STR       
        { $$ = $1;
        }
      | STR string
        { $$ = StringConcat( $1, $2);
          $1 = Free( $1);
          $2 = Free( $2);
        }
      ;

nums: NUM COMMA nums   { $$ = MakeNums( $1, $3);   }
    | NUM              { $$ = MakeNums( $1, NULL); }
    ;



/*
 *********************************************************************
 *
 *  rules for types
 *
 *********************************************************************
 */

returntypes: TYPE_VOID   { $$ = MakeTypes1( T_void); }
           | types       { $$ = $1;                  }
           ;

types: type COMMA types
       { $$ = $1;
         TYPES_NEXT( $$) = $3; 
       }
     | type { $$ = $1; }
     ;

type: localtype
      { $$ = $1;
      }
    | id COLON localtype
      { $$ = $3;
        TYPES_MOD( $$) = $1;
      }
    ;

localtype: simpletype
           { $$ = $1;
             TYPES_DIM( $$) = 0;
            }
         | simpletype SQBR_L SQBR_R  
           { $$ = $1;
             TYPES_DIM( $$) = 0;
           }
         | simpletype SQBR_L exprs SQBR_R  
           { $$ = $1;
             TYPES_DIM( $$) = 0;
             $$ = Exprs2ShpInfo( $1, $3);
           }
         | id 
           { $$ = MakeTypes1( T_user);
             TYPES_NAME( $$) = $1;
             TYPES_DIM( $$) = 0;
           }
         | id SQBR_L SQBR_R
           { $$ = MakeTypes1( T_user);
             TYPES_NAME( $$) = $1;
             TYPES_DIM( $$) = 0;
           }
         | id SQBR_L exprs SQBR_R
           { $$ = MakeTypes1( T_user);
             TYPES_NAME( $$) = $1;
             TYPES_DIM( $$) = 0; 
             $$ = Exprs2ShpInfo( $$, $3);
           }
         | LT id GT 
           { $$ = MakeTypes1( T_user);
             TYPES_NAME( $$) = $2;
             TYPES_DIM( $$) = 0;
             TYPES_POLY( $$) = TRUE;
           }
         | LT id GT SQBR_L SQBR_R
           { $$ = MakeTypes1( T_user);
             TYPES_NAME( $$) = $2;
             TYPES_DIM( $$) = 0;
             TYPES_POLY( $$) = TRUE;
           }
         | LT id GT SQBR_L exprs SQBR_R
           { $$ = MakeTypes1( T_user);
             TYPES_NAME( $$) = $2;
             TYPES_DIM( $$) = 0; 
             TYPES_POLY( $$) = TRUE;
             $$ = Exprs2ShpInfo( $$, $5);
           }
         ;

simpletype: TYPE_INT     { $$ = MakeTypes1( T_int);    }
          | TYPE_FLOAT   { $$ = MakeTypes1( T_float);  }
          | TYPE_BOOL    { $$ = MakeTypes1( T_bool);   }
          | TYPE_CHAR    { $$ = MakeTypes1( T_char);   }
          | TYPE_DBL     { $$ = MakeTypes1( T_double); }
          ;



/*
 *********************************************************************
 *
 *  rules for ntype
 *
 *********************************************************************
 */

ntype: basentype
       { $$ = $1; 
       }
     | basentype SQBR_L SQBR_R
       { $$ = $1;
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

simplentype: TYPE_INT    { $$ = TYMakeSimpleType( T_int);    }
           | TYPE_FLOAT  { $$ = TYMakeSimpleType( T_float);  }
           | TYPE_BOOL   { $$ = TYMakeSimpleType( T_bool);   }
           | TYPE_CHAR   { $$ = TYMakeSimpleType( T_char);   }
           | TYPE_DBL    { $$ = TYMakeSimpleType( T_double); }
           ;

userntype: id
           { $$ = TYMakeSymbType( $1, NULL);
           }
         | id COLON id
           { $$ = TYMakeSymbType( $3, $1);
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
 *    - id
 *    - type
 *
 ******************************************************************************
 ******************************************************************************/

module: MODULE { file_kind = F_modimp; } id { mod_name = $3; } SEMIC defs
        { $$ = $6;
          MODUL_NAME( $$) = mod_name;
          MODUL_FILETYPE( $$) = file_kind;
        }
        ;

class: CLASS { file_kind = F_classimp; } id { mod_name = $3; } SEMIC
       CLASSTYPE type SEMIC defs
       { $$ = $9;
         MODUL_NAME( $$) = mod_name;
         MODUL_FILETYPE( $$) = file_kind;
         MODUL_CLASSTYPE( $$) = $7;
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
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$,
                        MODSPEC_NAME( $$),
                        mdb_nodetype[ NODE_TYPE( MODSPEC_OWN( $$))],
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
         { $$ = RSCMakeTargetListEntry( $2, $4, $5, $6);
         }
       | /* empty */
         { $$ = NULL;
         }
       ;

inherits: COLON ID COLON inherits
          { $$ = MakeIds( $2, NULL, ST_regular);
            IDS_NEXT( $$) = $4;
          }
        | /* empty */
          { $$ = NULL;
          } 
        ;

resources: ID COLON LET string resources
           { $$ = RSCMakeResourceListEntry( $1, $4, 0, 0, $5);
           }
         | ID ADDON string resources
           { $$ = RSCMakeResourceListEntry( $1, $3, 0, 1, $4);
           }
         | ID COLON LET OPTION resources
           { $$ = RSCMakeResourceListEntry( $1, $4, 0, 0, $5);
           }
         | ID ADDON OPTION resources
           { $$ = RSCMakeResourceListEntry( $1, $3, 0, 1, $4);
           }
         | ID COLON LET ID resources
           { $$ = RSCMakeResourceListEntry( $1, $4, 0, 0, $5);
           }
         | ID ADDON ID resources
           { $$ = RSCMakeResourceListEntry( $1, $3, 0, 1, $4);
           }
         | ID COLON LET PRIVATEID resources
           { $$ = RSCMakeResourceListEntry( $1, $4, 0, 0, $5);
           }
         | ID ADDON PRIVATEID resources
           { $$ = RSCMakeResourceListEntry( $1, $3, 0, 1, $4);
           }
         | ID COLON LET NUM resources
           { $$ = RSCMakeResourceListEntry( $1, NULL, $4, 0, $5);
           }
         | ID ADDON NUM resources
           { $$ = RSCMakeResourceListEntry( $1, NULL, $3, 1, $4);
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
 *   int My_yyparse()
 *
 * Description:
 *   
 *
 ******************************************************************************/

int My_yyparse()
{
  char *tmp;

  DBUG_ENTER( "My_yyparse");

  /* 
   * make a copy of the actual filename, which will be used for
   * all subsequent nodes...
   */
  tmp = (char *) Malloc( (strlen(filename)+1) * sizeof( char));
  strcpy( tmp, filename);
  filename = tmp;

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
  ERROR( linenum, ("%s at pos %d: '%s`", errname, charpos, yytext));
  size_of_output = MAX_LINE_LENGTH -
                   (((verbose_level > 1) ? 2 : 0) +
                    strlen( filename) +
                    NumberOfDigits( linenum) + 9);
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
    global_wlcomp_aps = FreeTree( global_wlcomp_aps);
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

  new_exprs = MakeExprs( MakeChar( '\0'), NULL);

  cnt=0;
  
  for (i=strlen(str)-1; i>=0; i--) {
    if ((i>0) && (str[i-1]=='\\')) {
      switch (str[i]) {
      case 'n':
        new_exprs = MakeExprs(MakeChar('\n'), new_exprs);
        i-=1;
        break;
      case 't':
        new_exprs = MakeExprs(MakeChar('\t'), new_exprs);
        i-=1;
        break;
      case 'v':
        new_exprs = MakeExprs(MakeChar('\v'), new_exprs);
        i-=1;
        break;
      case 'b':
        new_exprs = MakeExprs(MakeChar('\b'), new_exprs);
        i-=1;
        break;
      case 'r':
        new_exprs = MakeExprs(MakeChar('\r'), new_exprs);
        i-=1;
        break;
      case 'f':
        new_exprs = MakeExprs(MakeChar('\f'), new_exprs);
        i-=1;
        break;
      case 'a':
        new_exprs = MakeExprs(MakeChar('\a'), new_exprs);
        i-=1;
        break;
      case '"':
        new_exprs = MakeExprs(MakeChar('"'), new_exprs);
        i-=1;
        break;
      default:
        new_exprs = MakeExprs(MakeChar(str[i]), new_exprs);
        break;
      }
    }
    else {
      new_exprs = MakeExprs(MakeChar(str[i]), new_exprs);
    }
    
    cnt+=1;
  }

  len_exprs = MakeExprs( MakeNum( cnt), NULL);
  array = MakeFlatArray( new_exprs);

#ifndef CHAR_ARRAY_NOT_AS_STRING
  ARRAY_STRING(array)=str;
#endif  /* CHAR_ARRAY_AS_STRING */

  res = MakeAp( StringCopy( "to_string"),
                NULL,
                MakeExprs( array, len_exprs));

  DBUG_RETURN( res); 
}



/******************************************************************************
 *
 * Function:
 *   types *Exprs2ShpInfo( types *types, node *exprs)
 *
 * Description:
 *   Checks whether all exprs are either N_num s or N_dot s or N_id containing
 *   "*" or "+". If so, the shape information of the types structure are set
 *   accordingly. Otherwise, yyerror is called.
 *
 ******************************************************************************/

static
types *Exprs2ShpInfo( types *types, node *exprs)
{
  int n;
  int *destptr;
  nodetype nt;
  char *name, *mod;

  DBUG_ENTER( "Exprs2ShpInfo");

  n = CountExprs( exprs);
  if( exprs == NULL) {
    TYPES_DIM( types) = 0;
  } else {
    nt = NODE_TYPE( EXPRS_EXPR1( exprs));
    if( nt == N_id) {
      name = ID_NAME( EXPRS_EXPR1( exprs));
      mod = ID_MOD( EXPRS_EXPR1( exprs));
      if( (strcmp( name, "*") == 0) && (( file_kind == F_sib) || (mod == NULL)) ) {
        TYPES_DIM( types) = ARRAY_OR_SCALAR;
      } else if( (strcmp( name, "+") == 0) && (( file_kind == F_sib) || (mod == NULL)) ) {
        TYPES_DIM( types) = UNKNOWN_SHAPE;
      } else {
        yyerror("illegal shape specification");
      } 
    } else if( nt == N_dot) {
      TYPES_DIM( types) = KNOWN_DIM_OFFSET - 1;
      exprs = EXPRS_NEXT( exprs);
      while( exprs != NULL) {
        if( NODE_TYPE( EXPRS_EXPR1( exprs)) == N_dot) {
          exprs = EXPRS_NEXT( exprs);
          TYPES_DIM( types) = TYPES_DIM( types) -1;
        } else {
          yyerror("illegal shape specification");
        }
      }
    } else {
      TYPES_DIM( types) = 0;
      TYPES_SHPSEG( types) = MakeShpseg(NULL);
      destptr = TYPES_SHPSEG( types)->shp;
      do {
        if( NODE_TYPE( EXPRS_EXPR1( exprs)) == N_num) {
          TYPES_DIM( types) = TYPES_DIM( types) + 1;
          *destptr++ = NUM_VAL( EXPRS_EXPR1( exprs));
          DBUG_PRINT("PARSE",("shape-element: %d", NUM_VAL( EXPRS_EXPR1( exprs))));
        } else {
          yyerror("illegal shape specification");
        }
        exprs = EXPRS_NEXT( exprs);
      } while( exprs != NULL);
    }
  }

  DBUG_RETURN( types);
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

  if( (NODE_TYPE( expr) == N_mop) && ! MOP_FIX( expr) ) {
    res = expr;
  } else {
    res = MakeMop( MakeExprs( expr, NULL), NULL, FALSE);
  }

  DBUG_RETURN( res);
}



/******************************************************************************
 *
 * Function:
 *   node *ConstructMop( node *, ids *, node *)
 *
 * Description:
 *
 ******************************************************************************/

node *ConstructMop( node *expr1, ids *fun_ids, node *expr2)
{
  node *res, *lmop, *rmop;

  DBUG_ENTER("ConstructMop");

  lmop = Expr2Mop( expr1);
  rmop = Expr2Mop( expr2);

  IDS_NEXT( fun_ids) = MOP_OPS( rmop);

  res = MakeMop( AppendExprs( MOP_EXPRS( lmop),
                              MOP_EXPRS( rmop)),
                 AppendIds( MOP_OPS( lmop),
                            fun_ids),
                 FALSE);
  lmop = Free( lmop);  /* only the top constructor N_mop is to be freed!!!! */
  rmop = Free( rmop);  /* only the top constructor N_mop is to be freed!!!! */

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
    if (strcmp( ID_NAME( conf), "Default")) {
      strcpy( yytext, ID_NAME( conf));
      yyerror( "innermost configuration is not 'Default'");
    }

    /*
     * free N_id node
     */
    conf = FreeTree( conf);

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
      tmp = FreeTree( tmp);

      if ((NODE_TYPE( next_conf) != N_id) && (NODE_TYPE( next_conf) != N_ap)) {
        strcpy( yytext, AP_NAME( conf));
        yyerror( "wlcomp-function with illegal configuration found");
      }
      else {
        /*
         * insert new configuration at head of 'exprs'
         */
        exprs = MakeExprs( conf, exprs);
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
  
  n = CountExprs( exprs);

  result = SHMakeShape( n);

  while ((exprs != NULL) && (result != NULL)) {
    if (NODE_TYPE( EXPRS_EXPR( exprs)) == N_num) {
      result = SHSetExtent( result, cnt, NUM_VAL( EXPRS_EXPR( exprs)));
    } else {
      result = SHFreeShape( result);
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

  n = CountExprs( exprs);

  switch (NODE_TYPE( EXPRS_EXPR1( exprs))) {
    case N_id:
      if (ID_MOD( EXPRS_EXPR1( exprs)) != NULL) {
        yyerror("illegal shape specification");
      } else if (ID_NAME( EXPRS_EXPR1( exprs))[1] != '\0') {
        yyerror("illegal shape specification");
      } else {
        switch (ID_NAME( EXPRS_EXPR1( exprs))[0]) {
          case '*':
            result = TYMakeAUD( basetype);
            break;
          case '+':
            result = TYMakeAUDGZ( basetype);
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
        result = TYMakeAKD( basetype, dots, SHMakeShape(0));
      }
      break;
    case N_num:
      shp = Exprs2Shape( exprs);
      if (shp != NULL) {
        result = TYMakeAKS( basetype, shp);
      } else {
        yyerror("illegal shape specification");
      }
      break;
    default:
      yyerror("illegal shape specification");
      break;
  }

  exprs = FreeTree( exprs);

  DBUG_RETURN( result);
}
      
