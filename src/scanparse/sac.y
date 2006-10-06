%{


/*
 * $Id$
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#include "config.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "DupTree.h"        /* for use of DUPdoDupTree() */
#include "ctinfo.h"
#include "free.h"
#include "globals.h"
#include "handle_mops.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "stringset.h"
#include "namespaces.h"
#include "check_mem.h"

#include "resource.h"


extern int commlevel;
extern int charpos;
extern char *linebuf_ptr;
extern char *yytext;

extern int yylex();


static node *global_wlcomp_aps = NULL;
static node *store_pragma = NULL;
static 
  enum {PRAG_fundef, PRAG_fundec, PRAG_objdef, PRAG_typedef} 
  pragma_type = PRAG_fundef;
static bool has_dot_rets = FALSE;
static bool has_dot_args = FALSE;

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
static node *SetClassType( node *module, ntype *type);

static int prf_arity[] = {
#define PRF_IF( a, b, c, d, e, f, g, h, i) f
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
 prf                prf;
 shape              *shape;
 resource_list_t    *resource_list_t;
 target_list_t      *target_list_t;
 inheritence_list_t *inheritence_list_t;
}

%token PARSE_PRG  PARSE_RC

%token BRACE_L  BRACE_R  BRACKET_L  BRACKET_R  SQBR_L  SQBR_R  COLON DCOLON SEMIC 
COMMA  AMPERS  DOT  QUESTION  ARROW 
INLINE  LET  TYPEDEF  OBJDEF  CLASSTYPE 
INC  DEC  ADDON  SUBON  MULON  DIVON  MODON 
K_MAIN  RETURN  IF  ELSE  DO  WHILE  FOR  NWITH  FOLD FOLDFIX
MODULE  IMPORT  EXPORT  PROVIDE  USE  CLASS  ALL  EXCEPT
MODSPEC
SC  TRUETOKEN  FALSETOKEN  EXTERN  C_KEYWORD 
HASH  PRAGMA  LINKNAME  LINKSIGN  EFFECT  REFCOUNTING 
COPYFUN  FREEFUN  INITFUN  LINKWITH LINKOBJ
WLCOMP  CACHESIM  SPECIALIZE 
TARGET  STEP  WIDTH  GENARRAY  MODARRAY  PROPAGATE
LE  LT  GT LAZYAND LAZYOR
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

%type <node> typedef exttypedef

%type <node> objdef extobjdef

%type <node> fundef  fundef1  fundef2  main
%type <node> fundec fundec2
%type <node> mainargs  fundecargs fundefargs  args  arg varargs
%type <node> exprblock  exprblock2  assignsOPTret  assigns  assign 
     let cond optelse  doloop whileloop forloop  assignblock
     lets qual_ext_id qual_ext_ids ids
%type <node> exprs expr expr_with expr_ap opt_arguments expr_ar expr_sel
     with generator  steps  width  nwithops nwithop  withop  wlassignblock  genidx 
     part  parts npart nparts nums returntypes returndectypes ntypes varntypes
%type <prf> genop prf

%type <id> reservedid  string ext_id

%type <ntype> simplentype userntype basentype ntype

/* pragmas */
%type <id> pragmacachesim
%type <node> wlcomp_pragma_global  wlcomp_pragma_local
%type <node> pragmas pragma pragmalist
/*
%type <node> pragmas 
*/





/*******************************************************************************
* module implementations
*/
%type <node> module class
%type <ntype> classtype
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


%right QUESTION COLON
%right LAZYOR
%right LAZYAND
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
 CTIabortLine( global.linenum, "Unterminated comment found");

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
       MODULE_NAMESPACE( $$) = NSgetRootNamespace();
       MODULE_FILETYPE( $$) = F_prog;
     }
   ;

defs: interface def2
      { $$ = $2;
        MODULE_INTERFACE( $$) = $1;
      }
    | def2
      { $$ = $1; }
    ;

def2: typedef def2
      { $$ = $2;
        TYPEDEF_NEXT( $1) = MODULE_TYPES( $$);
        MODULE_TYPES( $$) = $1;
      }
    | exttypedef def2
      { $$ = $2;
        TYPEDEF_NEXT( $1) = MODULE_TYPES( $$);
        MODULE_TYPES( $$) = $1;
      } 
    | exttypedef { pragma_type = PRAG_typedef; } pragmas def2
      { $$ = $4;
        TYPEDEF_NEXT( $1) = MODULE_TYPES( $$);
        TYPEDEF_PRAGMA( $1) = $3;
        MODULE_TYPES( $$) = $1;
      } 
    | def3
      { $$ = $1; }
    ;

def3: objdef def3
      { $$ = $2;
        OBJDEF_NEXT( $1) = MODULE_OBJS( $$);
        MODULE_OBJS( $$) = $1;
      }
    | extobjdef def3
      { $$ = $2;
        OBJDEF_NEXT( $1) = MODULE_OBJS( $$);
        MODULE_OBJS( $$) = $1;
      }
    | extobjdef { pragma_type = PRAG_objdef; } pragmas def3
      { $$ = $4;
        OBJDEF_NEXT( $1) = MODULE_OBJS( $$);
        OBJDEF_PRAGMA( $1) = $3;
        MODULE_OBJS( $$) = $1;
      }
    | def4
      { $$ = $1; }
    ;

def4: fundec {  pragma_type = PRAG_fundec; } pragmas def4
      { $$ = $4;
        FUNDEF_PRAGMA( $1) = $3;
        MODULE_FUNDECS( $$) = TCappendFundef( MODULE_FUNDECS( $$), $1);
      }
    | fundec def4
      { $$ = $2;
        MODULE_FUNDECS( $$) = TCappendFundef( MODULE_FUNDECS( $$), $1);
      }
    | fundef { pragma_type = PRAG_fundef; } pragmas def4
      { $$ = $4;
        MODULE_FUNS( $$) = TCappendFundef( MODULE_FUNS( $$), $1);
      }
    | fundef def4
      { $$ = $2;
        MODULE_FUNS( $$) = TCappendFundef( MODULE_FUNS( $$), $1);
      }
    | main def4
      { $$ = $2;
        MODULE_FUNS( $$) = TCappendFundef( MODULE_FUNS( $$), $1);
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
         { $$ = TBmakeTypedef( $3, NULL, $2, NULL);

           DBUG_PRINT( "PARSE",
                       ("%s:"F_PTR","F_PTR", Id: %s",
                        global.mdb_nodetype[ NODE_TYPE( $$)],
                        $$, 
                        TYPEDEF_NTYPE( $$),
                        TYPEDEF_NAME( $$)));
         }
       ;

exttypedef: EXTERN TYPEDEF ID SEMIC
            { $$ = TBmakeTypedef( $3, NULL, TYmakeAKS(
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

objdef: OBJDEF ntype ID LET expr_ap SEMIC 
        { $$ = TBmakeObjdef( $2, NULL, $3, $5, NULL);

          DBUG_PRINT( "PARSE",
                      ("%s:"F_PTR","F_PTR", Id: %s",
                       global.mdb_nodetype[ NODE_TYPE( $$)],
                       $$, 
                       OBJDEF_TYPE( $$),
                       OBJDEF_NAME( $$)));
        }
      ;

extobjdef: EXTERN OBJDEF ntype ID SEMIC 
        {
          $$ = TBmakeObjdef( $3, NULL, $4, NULL, NULL);

          OBJDEF_ISEXTERN( $$) = TRUE;

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
                        (FUNDEF_NS( $$) == NULL) 
                          ? "(null)"
                          : NSgetName( FUNDEF_NS( $$)),
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
                        (FUNDEF_NS( $$) == NULL) 
                          ? "(null)" 
                          : NSgetName( FUNDEF_NS( $$)),
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
          | DOT DOT DOT { $$ = NULL; has_dot_args = TRUE; }
          ;


args: arg COMMA args
      { ARG_NEXT($1) = $3;
        $$ = $1;
      }
    | arg
     { $$ = $1; 
      }
    ;

varargs: arg COMMA varargs
         { ARG_NEXT($1) = $3;
           $$ = $1;
         }
       | arg
        { $$ = $1;
         }
       | arg COMMA DOT DOT DOT
         { $$ = $1;
           has_dot_args = TRUE;
         }
       ;


arg: ntype ID
     { $$ = TBmakeArg( TBmakeAvis( $2, $1), NULL);

       AVIS_DECLTYPE( ARG_AVIS( $$)) = TYcopyType( $1);

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

       AVIS_DECLTYPE( ARG_AVIS( $$)) = TYcopyType( $1);

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
          if( has_dot_rets) {
             FUNDEF_HASDOTRETS( $$) = TRUE;
             has_dot_rets = FALSE;
           }
        }
      ;

fundec2: fundecargs BRACKET_R { $<cint>$ = global.linenum; } SEMIC
         { $$ = TBmakeFundef( NULL, NULL, NULL, $1, NULL, NULL);
           NODE_LINE( $$) = $<cint>3;
           if( has_dot_args) {
             FUNDEF_HASDOTARGS( $$) = TRUE;
             has_dot_args = FALSE;
           }
         }
       | BRACKET_R { $<cint>$ = global.linenum; } SEMIC
         { $$ = TBmakeFundef( NULL, NULL, NULL, NULL, NULL, NULL);
           NODE_LINE( $$) = $<cint>2;
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

wlcomp_pragma_global: hash_pragma WLCOMP expr_ap
                      { if (global_wlcomp_aps != NULL) {
                          /* remove old global pragma */
                          global_wlcomp_aps = FREEdoFreeTree( global_wlcomp_aps);
                        }
                        $3 = CheckWlcompConf( $3, NULL);
                        if ($3 != NULL) {
                          global_wlcomp_aps = $3;
                        }
                      }
                    ;

wlcomp_pragma_local: hash_pragma WLCOMP expr_ap
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
       ;

pragmalist: pragma pragmalist
          | pragma
          | wlcomp_pragma_global
          ;

pragma: hash_pragma LINKNAME string
        { if( (pragma_type != PRAG_fundec) && (pragma_type != PRAG_objdef) ) {
            yyerror( "pragma \"linkname\" not allowed here");
          }
          if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_LINKNAME( store_pragma) != NULL) {
            CTIwarnLine( global.linenum, 
                         "Conflicting definitions of pragma 'linkname`");
          }
          PRAGMA_LINKNAME( store_pragma) = $3;
        }
      | hash_pragma LINKWITH string
        { if( (pragma_type != PRAG_fundec) && (pragma_type != PRAG_typedef) ) {
            yyerror( "pragma \"linkwith\" not allowed here");
          }
          if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          PRAGMA_LINKMOD( store_pragma) = STRSadd( $3, STRS_extlib,
                                            PRAGMA_LINKMOD( store_pragma));
        }
      | hash_pragma LINKOBJ string
        { if( (pragma_type != PRAG_fundec) && (pragma_type != PRAG_typedef) ) {
            yyerror( "pragma \"linkobj\" not allowed here");
          }
          if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          PRAGMA_LINKOBJ( store_pragma) = STRSadd( $3, STRS_objfile,
                                            PRAGMA_LINKOBJ( store_pragma));
        }
      | hash_pragma LINKSIGN SQBR_L nums SQBR_R
        { if( pragma_type != PRAG_fundec) {
            yyerror( "pragma \"linksign\" not allowed here");
          }
          if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_LINKSIGN( store_pragma) != NULL) {
            CTIwarnLine( global.linenum,
                         "Conflicting definitions of pragma 'linksign`");
          }
          PRAGMA_LINKSIGN( store_pragma) = $4;
        }
      | hash_pragma REFCOUNTING SQBR_L nums SQBR_R
        { if( pragma_type != PRAG_fundec) {
            yyerror( "pragma \"refcounting\" not allowed here");
          }
          if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_REFCOUNTING( store_pragma) != NULL) {
            CTIwarnLine( global.linenum, 
                         "Conflicting definitions of pragma 'refcounting`");
          }
          PRAGMA_REFCOUNTING( store_pragma) = $4;
        }
      | hash_pragma EFFECT qual_ext_ids
        { if( pragma_type != PRAG_fundec)  {
            yyerror( "pragma \"effect\" not allowed here");
          }
          if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          PRAGMA_EFFECT( store_pragma) = $3;
        }
      | hash_pragma COPYFUN string
        { if( pragma_type != PRAG_typedef) {
            yyerror( "pragma \"copyfun\" not allowed here");
          }
          if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_COPYFUN( store_pragma) != NULL) {
            CTIwarnLine( global.linenum, 
                         "Conflicting definitions of pragma 'copyfun`");
          }
          PRAGMA_COPYFUN( store_pragma) = $3;
        }
      | hash_pragma FREEFUN string
        { if( pragma_type != PRAG_typedef) {
            yyerror( "pragma \"freefun\" not allowed here");
          }
          if (store_pragma == NULL) {
            store_pragma = TBmakePragma();
          }
          if (PRAGMA_FREEFUN( store_pragma) != NULL) {
            CTIwarnLine( global.linenum,
                         "Conflicting definitions of pragma 'freefun`");
          }
          PRAGMA_FREEFUN( store_pragma) = $3;
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
              while (SPIDS_NEXT( $2) != NULL) {  /* at least 2 vardecs! */
                vardec_ptr = TBmakeVardec( 
                               TBmakeAvis( ILIBstringCopy( SPIDS_NAME( $2)),
                                           TYcopyType( $1)),
                               vardec_ptr);

                /*
                 * set the DECL_TYPE as well!
                 */
                AVIS_DECLTYPE( VARDEC_AVIS( vardec_ptr)) = TYcopyType( $1);

                /*
                 * Now, we want to "push" $2 one IDS further
                 * and we want to FREE the current SPIDS node.
                 */
                ids_ptr = SPIDS_NEXT( $2);
                $2 = FREEdoFreeNode( $2);
                $2 = ids_ptr;
              }
              /*
               * When we reach this point, all but one vardec is constructed!
               * Therefore, we can recycle the ntype from $1 instead of
               * duplicating it as done in the loop above!
               */
              $$ = $4;
              BLOCK_VARDEC( $$) = TBmakeVardec( 
                                    TBmakeAvis( 
                                      ILIBstringCopy( SPIDS_NAME( $2)), $1),
                                                vardec_ptr);
              /*
               * set the DECL_TYPE for the last vardec!
               */
              AVIS_DECLTYPE( VARDEC_AVIS( BLOCK_VARDEC( $$))) 
                = TYcopyType( $1);

              /* 
               * Finally, we free the last SPIDS-node! 
               */
              $2 = FREEdoFreeTree( $2);   
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
                 NODE_LINE( $$) = $<cint>2;
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
               $3 = TCmakeVector( TYmakeAKS( TYmakeSimpleType( T_unknown), 
                                             SHmakeShape(0)),
                                  $3);
             } else {
               node * tmp;

               tmp = $3;
               $3 = EXPRS_EXPR( $3);
               EXPRS_EXPR( tmp) = NULL;
               tmp = FREEdoFreeNode( tmp);
             }
             id = TBmakeSpid( NULL, ILIBstringCopy( $1));

             ids = TBmakeSpids( $1, NULL);

             ap = TBmakeSpap( 
                            TBmakeSpid( NULL, ILIBstringCopy( "modarray")),
                            TBmakeExprs( id,
                              TBmakeExprs( $3,
                                TBmakeExprs( $7,
                                  NULL))));

             $$ = TBmakeLet( ids, ap);
             NODE_LINE( $$) = $<cint>6;
           }
         | expr_ap { $$ = TBmakeLet( NULL, $1); }
         | expr_with { $$ = TBmakeLet( NULL, $1); }
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
    | string                     { $$ = String2Array( $1);    }
    | TRUETOKEN                  { $$ = TBmakeBool( 1);       }
    | FALSETOKEN                 { $$ = TBmakeBool( 0);       }
    | expr LAZYAND expr          { $$ = TBmakeFuncond( $1, $3, TBmakeBool(0)); }
    | expr LAZYOR expr           { $$ = TBmakeFuncond( $1, TBmakeBool(1), $3); }
    | expr QUESTION expr COLON expr 
      { $$ = TBmakeFuncond( $1, $3, $5); 
      }
    |  BRACKET_L expr BRACKET_R
      { $$ = $2;
        if( NODE_TYPE( $2) == N_spmop) {
          SPMOP_ISFIXED( $$) = TRUE;
        }
      }
    | expr qual_ext_id expr %prec BM_OP
      {
        $$ = ConstructMop( $1, $2, $3);
      }
    | PLUS expr %prec MM_OP
      {
        $$ = TCmakeSpap1( NULL, ILIBstringCopy( "+"), $2);
      }
    | MINUS expr %prec MM_OP
      {
        $$ = TCmakeSpap1( NULL, ILIBstringCopy( "-"), $2);
      }
    | TILDE expr %prec MM_OP
      {
        $$ = TCmakeSpap1( NULL, ILIBstringCopy( "~"), $2);
      }
    | EXCL expr %prec MM_OP
      {
        $$ = TCmakeSpap1( NULL, ILIBstringCopy( "!"), $2);
      }
    | PLUS BRACKET_L expr COMMA exprs BRACKET_R
      {
        $$ = TBmakeSpap( TBmakeSpid( NULL, ILIBstringCopy( "+")), 
                         TBmakeExprs( $3, $5));
      }
    | MINUS BRACKET_L expr COMMA exprs BRACKET_R
      {
        $$ = TBmakeSpap( TBmakeSpid( NULL, ILIBstringCopy( "-")), 
                         TBmakeExprs( $3, $5));
      }
    | TILDE BRACKET_L expr COMMA exprs BRACKET_R
      {
        $$ = TBmakeSpap( TBmakeSpid( NULL, ILIBstringCopy( "~")), 
                         TBmakeExprs( $3, $5));
      }
    | EXCL BRACKET_L expr COMMA exprs BRACKET_R
      {
        $$ = TBmakeSpap( TBmakeSpid( NULL, ILIBstringCopy( "!")), 
                         TBmakeExprs( $3, $5));
      }
    | expr_sel                    { $$ = $1; }   /* bracket notation      */
    | expr_ap                     { $$ = $1; }   /* prefix function calls */
    | expr_with                   { $$ = $1; }   /* with loops            */
    | expr_ar                     { $$ = $1; }   /* constant arrays       */
    | BRACKET_L COLON ntype BRACKET_R expr   %prec CAST
      { $$ = TBmakeCast( $3, $5);
      }
    | BRACE_L ID ARROW expr BRACE_R
      { $$ = TBmakeSetwl( TBmakeSpid( NULL, ILIBstringCopy( $2)), $4);
      }
    | BRACE_L SQBR_L exprs SQBR_R ARROW expr BRACE_R
      { $$ = TBmakeSetwl( $3, $6);
      }
    ;
      
expr_with: NWITH { $<cint>$ = global.linenum; } with
      { $$ = $3;
        NODE_LINE( $$)= $<cint>2;
      }
      ;

with: BRACKET_L generator BRACKET_R wlassignblock withop
      { 
        node *cexpr, *code;
#if 1
        /*
         * For now, we do not yet ask for the new syntax, BUT later we will
         * activate the following two lines....
         */
        CTIwarnLine( global.linenum, 
                     "Old with-loop style depricated!\n"
                     "Please use the new syntax instead");
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
          cexpr = SPFOLD_SPEXPR($5);
        }
        code = TBmakeCode( $4, TBmakeExprs( cexpr, NULL));
        CODE_USED( code)++;

        $$ = TBmakeWith( $2, code, $5);
        /*
         * Finally, we generate the link between the (only) partition
         * and the (only) code!
         */
        PART_CODE( WITH_PART( $$)) = code;
      }
    | BRACKET_L ID BRACKET_R parts nwithop
      { $$ = $4;
        WITH_WITHOP( $$) = $5;
#if 1
        /*
         * At the time being we ignore $2. However, it SHOULD not be specified
         * here.....
         */
        CTIwarnLine( global.linenum,
                     "Extra specification of with-loop index is deprecated!\n"
                     "Please eliminate the unifying index in brackets!");
#endif

      }
    | BRACE_L wlcomp_pragma_local nparts BRACE_R BRACKET_L nwithops BRACKET_R
      { $$ = $3;
        WITH_WITHOP( $$) = $6;
        WITH_PRAGMA( $$) = $2;
      }
    | BRACE_L wlcomp_pragma_local nparts BRACE_R nwithop
      { $$ = $3;
        WITH_WITHOP( $$) = $5;
        WITH_PRAGMA( $$) = $2;
      }
    | BRACE_L wlcomp_pragma_local nparts BRACE_R TYPE_VOID
      { $$ = $3;
        WITH_WITHOP( $$) = NULL;
        WITH_PRAGMA( $$) = $2;
      }
    ;


expr_sel: expr SQBR_L exprs SQBR_R
          { if( TCcountExprs($3) == 1) {
              $$ = TCmakeSpap2( NULL, ILIBstringCopy( "sel"),
                                EXPRS_EXPR( $3), $1);
              EXPRS_EXPR( $3) = NULL;
              $3 = FREEdoFreeNode( $3);
            } else {
              $$ = 
                TCmakeSpap2(NULL, ILIBstringCopy( "sel"),
                            TCmakeVector(TYmakeAKS(TYmakeSimpleType(T_unknown),
                                                   SHmakeShape(0)),
                                         $3),
                            $1);
            }
          }
        | expr SQBR_L SQBR_R
          { $$ = 
              TCmakeSpap2( NULL, ILIBstringCopy( "sel"),
                           TCmakeVector(TYmakeAKS( TYmakeSimpleType(T_unknown), 
                                                   SHmakeShape(0)),
                                        NULL), 
                           $1);
          }
        ;

expr_ap: qual_ext_id BRACKET_L { $<cint>$ = global.linenum; } opt_arguments BRACKET_R
         {
           $$ = TBmakeSpap( $1, $4);
           NODE_LINE( $$) = $<cint>3;
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
         { $$ = TCmakeVector( TYmakeAKS( TYmakeSimpleType( T_unknown), 
                                         SHmakeShape(0)),
                              $3);
           NODE_LINE( $$) = $<cint>2;
         }
       | SQBR_L { $<cint>$ = global.linenum; } COLON ntype SQBR_R
         { if ( !TYisAKS( $4)) {
             yyerror("Empty array with non-constant shape found.");
           }
           $$ = TCmakeVector( $4, NULL);
           NODE_LINE( $$) = $<cint>2;
         }
       | SQBR_L { $<cint>$ = global.linenum; } SQBR_R
         { $$ = TCmakeVector( TYmakeAKS( TYmakeSimpleType( T_int), 
                                         SHmakeShape(0)), 
                              NULL);
           NODE_LINE( $$) = $<cint>2;
         }
       ;

nparts: npart
       { $$ = $1;
       }
     | npart nparts
       { $$ = $1;
         PART_NEXT( WITH_PART( $1)) = WITH_PART( $2);
         CODE_NEXT( WITH_CODE( $1)) = WITH_CODE( $2);
         WITH_PART( $2) = NULL;
         WITH_CODE( $2) = NULL;
         FREEdoFreeTree( $2);
       }
     ;

npart: BRACKET_L generator BRACKET_R wlassignblock COLON expr SEMIC
       { $$ = TBmakeWith( $2, TBmakeCode( $4, TBmakeExprs( $6, NULL)), NULL);
         CODE_USED( WITH_CODE( $$))++;
         PART_CODE( $2) = WITH_CODE( $$);
       }
     | BRACKET_L generator BRACKET_R wlassignblock COLON
                                    BRACKET_L expr COMMA exprs BRACKET_R SEMIC
       { $$ = TBmakeWith( $2, TBmakeCode( $4, TBmakeExprs( $7, $9)), NULL);
         CODE_USED( WITH_CODE( $$))++;
         PART_CODE( $2) = WITH_CODE( $$);
       }
     | BRACKET_L generator BRACKET_R wlassignblock SEMIC
       { $$ = TBmakeWith( $2, TBmakeCode( $4, NULL), NULL);
         CODE_USED( WITH_CODE( $$))++;
         PART_CODE( $2) = WITH_CODE( $$);
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

part: BRACKET_L generator BRACKET_R wlassignblock COLON exprs SEMIC
      { $$ = TBmakeWith( $2, TBmakeCode( $4, $6), NULL);
        CODE_USED( WITH_CODE( $$))++;
        PART_CODE( $2) = WITH_CODE( $$);
      }
    | BRACKET_L generator BRACKET_R wlassignblock SEMIC
      { $$ = TBmakeWith( $2, TBmakeCode( $4, NULL), NULL);
        CODE_USED( WITH_CODE( $$))++;
        PART_CODE( $2) = WITH_CODE( $$);
      }
    ;
     
generator: expr LE genidx genop expr steps width
           {
             if( ($7 != NULL) && ($6 == NULL)) {
               CTIwarnLine( global.linenum,
                            "Width vector ignored due to missing step vector");
               $7 = FREEdoFreeTree( $7);
             }
             $$ = TBmakePart( NULL,
                              $3,
                              TBmakeGenerator( F_le, $4, $1, $5, $6, $7));
           }
         | expr LT genidx genop expr steps width
           {
             if( ($7 != NULL) && ($6 == NULL)) {
               CTIwarnLine( global.linenum,
                            "Width vector ignored due to missing step vector");
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
        { $$ = TBmakeWithid( TBmakeSpids( ILIBstringCopy( $1), NULL), $4);
        }
      | ID
        { $$ = TBmakeWithid( TBmakeSpids( ILIBstringCopy( $1), NULL), NULL);
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

nwithops: nwithop
          { $$ = $1;
          }
        | nwithop COMMA nwithops
          { $$ = $1;
            L_WITHOP_NEXT( $$, $3);
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
         { $$ = TBmakeSpfold( $5);
           SPFOLD_FUN( $$) = ILIBstringCopy( SPID_NAME( $3));
           SPFOLD_NS( $$) = NSdupNamespace( SPID_NS( $3));
           $3 = FREEdoFreeTree( $3);
         }
       | FOLDFIX BRACKET_L qual_ext_id COMMA expr COMMA expr BRACKET_R
         { $$ = TBmakeSpfold( $5);
           SPFOLD_FUN( $$) = ILIBstringCopy( SPID_NAME( $3));
           SPFOLD_NS( $$) = NSdupNamespace( SPID_NS( $3));
           SPFOLD_GUARD( $$) = $7;
           $3 = FREEdoFreeTree( $3);
         }
       | PROPAGATE BRACKET_L expr BRACKET_R
         { $$ = TBmakePropagate( $3);
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
      | FOLD BRACKET_L qual_ext_id COMMA expr COMMA expr BRACKET_R
        { $$ = TBmakeSpfold( $5);
          SPFOLD_FUN( $$) = ILIBstringCopy( SPID_NAME( $3));
          SPFOLD_NS( $$) = NSdupNamespace( SPID_NS( $3));
          $3 = FREEdoFreeTree( $3);
          SPFOLD_SPEXPR( $$) = $7;
        }
      | FOLDFIX BRACKET_L qual_ext_id COMMA expr COMMA expr COMMA expr BRACKET_R
        { $$ = TBmakeSpfold( $5);
          SPFOLD_FUN( $$) = ILIBstringCopy( SPID_NAME( $3));
          SPFOLD_NS( $$) = NSdupNamespace( SPID_NS( $3));
          $3 = FREEdoFreeTree( $3);
          SPFOLD_GUARD( $$) = $7;
          SPFOLD_SPEXPR( $$) = $9;
        }
      ;

prf: PRF_DIM       { $$ = F_dim;     }
   | PRF_SHAPE     { $$ = F_shape;   }
   | PRF_RESHAPE   { $$ = F_reshape; }
   | PRF_SEL       { $$ = F_sel;     }
   | PRF_GENARRAY  { $$ = F_genarray;}
   | PRF_MODARRAY  { $$ = F_modarray;}
   | PRF_ADD_SxS   { $$ = F_add_SxS; }
   | PRF_ADD_SxA   { $$ = F_add_SxA; }
   | PRF_ADD_AxS   { $$ = F_add_AxS; }
   | PRF_ADD_AxA   { $$ = F_add_AxA; }
   | PRF_SUB_SxS   { $$ = F_sub_SxS; }
   | PRF_SUB_SxA   { $$ = F_sub_SxA; }
   | PRF_SUB_AxS   { $$ = F_sub_AxS; }
   | PRF_SUB_AxA   { $$ = F_sub_AxA; }
   | PRF_MUL_SxS   { $$ = F_mul_SxS; }
   | PRF_MUL_SxA   { $$ = F_mul_SxA; }
   | PRF_MUL_AxS   { $$ = F_mul_AxS; }
   | PRF_MUL_AxA   { $$ = F_mul_AxA; }
   | PRF_DIV_SxS   { $$ = F_div_SxS; }
   | PRF_DIV_SxA   { $$ = F_div_SxA; }
   | PRF_DIV_AxS   { $$ = F_div_AxS; }
   | PRF_DIV_AxA   { $$ = F_div_AxA; }
   | PRF_MOD       { $$ = F_mod;     }
   | PRF_ABS       { $$ = F_abs;     }
   | PRF_NEG       { $$ = F_neg;     }
   | PRF_MIN       { $$ = F_min;     }
   | PRF_MAX       { $$ = F_max;     }
   | PRF_EQ        { $$ = F_eq;      }
   | PRF_NEQ       { $$ = F_neq;     }
   | PRF_LT        { $$ = F_lt;      }
   | PRF_LE        { $$ = F_le;      }
   | PRF_GT        { $$ = F_gt;      }
   | PRF_GE        { $$ = F_ge;      }
   | PRF_NOT       { $$ = F_not;     }
   | PRF_AND       { $$ = F_and;     }
   | PRF_OR        { $$ = F_or;      }
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

qual_ext_ids: qual_ext_id COMMA qual_ext_ids
              { $$ = TBmakeExprs( $1, $3);
              }
            | qual_ext_id
              { $$ = TBmakeExprs( $1, NULL);
              }
            ;

qual_ext_id: ext_id
             { $$ = TBmakeSpid( NULL, $1);
             }
           | ID DCOLON ext_id
             { $$ = TBmakeSpid( NSgetNamespace( $1), $3);
             }
           ; 

ext_id: ID         { $$ = $1; } 
      | reservedid { $$ = $1; }
      ; 

ids: ID COMMA ids
     { $$ = TBmakeSpids( $1, $3);
     }
   | ID
     { $$ = TBmakeSpids( $1, NULL);
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
              | DOT DOT DOT { $$ = NULL; has_dot_rets = TRUE; }
              ;

ntypes: ntype COMMA ntypes { $$ = TBmakeRet( $1, $3); }
      | ntype { $$ = TBmakeRet( $1,NULL); }
      ;

varntypes: ntype COMMA ntypes { $$ = TBmakeRet( $1, $3); }
         | ntype { $$ = TBmakeRet( $1,NULL); }
         | ntype COMMA DOT DOT DOT
         { $$ = TBmakeRet( $1,NULL);
           has_dot_rets = TRUE;
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
         | ID DCOLON ID
           { $$ = TYmakeSymbType( $3, NSgetNamespace( $1));
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

module: MODULE { file_kind = F_modimp; } ID SEMIC defs
        { $$ = $5;
          MODULE_NAMESPACE( $$) = NSgetNamespace( $3);
          MODULE_FILETYPE( $$) = file_kind;
        }
        ;

class: CLASS { file_kind = F_classimp; } ID SEMIC classtype defs
       { $$ = $6;
         MODULE_NAMESPACE( $$) = NSgetNamespace( $3);
         MODULE_FILETYPE( $$) = file_kind;
         $$ = SetClassType( $$, $5);
       }
       ;

classtype: CLASSTYPE ntype SEMIC { $$ = $2; }
         | EXTERN CLASSTYPE SEMIC { $$ = TYmakeAKS(
                                           TYmakeSimpleType( T_hidden),
                                           SHmakeShape( 0)); }
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
           MODSPEC_INTERFACE( $$) = NULL;
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


targets: TARGET ID inherits resources targets
         { $$ = RSCmakeTargetListEntry( $2, $3, $4, $5);
         }
       | /* empty */
         { $$ = NULL;
         }
       ;

inherits: DCOLON ID inherits
           { $$ = RSCmakeInheritenceListEntry( $2, $3);
           }
        | COLON
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
  CHKMdoNotReport( tmp);
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

  size_of_output = CTIgetErrorMessageLineLength();
  
  if (strlen( linebuf_ptr) > (size_t) size_of_output) {
    if (charpos >= size_of_output - 15) {
      offset = charpos - size_of_output + 15;
      strncpy( linebuf_ptr + offset, "... ", 4);
    }
    strcpy( linebuf_ptr + offset + size_of_output - 4, " ...");
  }

  CTIabortLine( global.linenum,
                "%s at pos %d: '%s`\n%s\n%*s",
                errname, 
                charpos, 
                yytext,
                linebuf_ptr + offset,
                charpos - offset, "^");

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
  node *len_expr;
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

  len_expr = TBmakeNum( cnt);
  array = TCmakeVector( TYmakeAKS( TYmakeSimpleType( T_char),
                                   SHmakeShape( 0)), 
                        new_exprs);

#ifndef CHAR_ARRAY_NOT_AS_STRING
  ARRAY_STRING(array)=str;
#endif  /* CHAR_ARRAY_AS_STRING */

  res = TCmakeSpap2( NSgetNamespace( "String") , ILIBstringCopy( "to_string"), 
                    array, len_expr);

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
  ids = TBmakeSpids(  ILIBstringCopy( name), NULL);

  id = TBmakeSpid( NULL, name);

  ap = TBmakeSpap( TBmakeSpid( NULL, op),
                   TBmakeExprs( id, 
                     TBmakeExprs( TBmakeNum(1), NULL)));

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
  ids = TBmakeSpids( ILIBstringCopy( name), NULL);

  id = TBmakeSpid( NULL, name);

  ap = TBmakeSpap( TBmakeSpid( NULL, op),
                   TBmakeExprs( id,
                     TBmakeExprs( expr, NULL)));

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

  if( (NODE_TYPE( expr) == N_spmop) && ! SPMOP_ISFIXED( expr) ) {
    res = expr;
  } else {
    res = TBmakeSpmop( NULL, TBmakeExprs( expr, NULL));
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
  node *res, *lmop, *rmop, *fun_exprs;

  DBUG_ENTER("ConstructMop");

  lmop = Expr2Mop( expr1);
  rmop = Expr2Mop( expr2);

  fun_exprs = TBmakeExprs( fun_id, SPMOP_OPS( rmop));

  res = TBmakeSpmop( TCappendExprs( SPMOP_OPS( lmop),
                                    fun_exprs),
                     TCappendExprs( SPMOP_EXPRS( lmop),
                                    SPMOP_EXPRS( rmop)));
  /*
   * now we free the topmost node. Therefore we have to set the OPS and EXPRS
   * attributes to NULL, so that they do not get freed!
   */
  SPMOP_EXPRS( lmop) = NULL;
  SPMOP_OPS( lmop) = NULL;
  SPMOP_EXPRS( rmop) = NULL;
  SPMOP_OPS( rmop) = NULL;

  lmop = FREEdoFreeNode( lmop); 
  rmop = FREEdoFreeNode( rmop);

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

  if (NODE_TYPE( conf) == N_spid) {
    if (strcmp( SPID_NAME( conf), "Default")) {
      strcpy( yytext, SPID_NAME( conf));
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
  else if (NODE_TYPE( conf) == N_spap) {
    node *arg = SPAP_ARGS( conf);

    /*
     * look for last argument -> next 'conf'
     */
    if (arg == NULL) {
      strcpy( yytext, SPAP_NAME( conf));
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

      if ((NODE_TYPE( next_conf) != N_spid) && (NODE_TYPE( next_conf) != N_spap)) {
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
    case N_spid:
      if (SPID_NS( EXPRS_EXPR1( exprs)) != NULL) {
        yyerror("illegal shape specification");
      } else if (SPID_NAME( EXPRS_EXPR1( exprs))[1] != '\0') {
        yyerror("illegal shape specification");
      } else {
        switch (SPID_NAME( EXPRS_EXPR1( exprs))[0]) {
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

static
node *SetClassType( node *module, ntype *type)
{
  node *tdef;

  DBUG_ENTER("SetClassType");

  tdef = TBmakeTypedef( 
           ILIBstringCopy( NSgetModule( MODULE_NAMESPACE( module))),
           NSdupNamespace( MODULE_NAMESPACE( module)),
           type,
           MODULE_TYPES( module));

  TYPEDEF_ISUNIQUE( tdef) = TRUE;
  MODULE_TYPES( module) = tdef;

  DBUG_RETURN( module);
}
      
