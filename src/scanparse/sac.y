%{


/*
 *
 * $Log$
 * Revision 3.40  2002/06/20 15:34:23  dkr
 * signature of MakeNWithOP modified
 *
 * Revision 3.39  2002/04/16 21:11:55  dkr
 * main() function has a modname now, too
 *
 * Revision 3.38  2002/04/16 18:42:56  dkr
 * support for empty return statements added
 *
 * Revision 3.37  2002/02/26 14:47:10  dkr
 * several NODE_LINE errors corrected
 *
 * Revision 3.36  2002/02/21 15:45:57  dkr
 * access macros used
 *
 * Revision 3.35  2002/02/20 14:34:02  dkr
 * function DupTypes() renamed into DupAllTypes()
 *
 * Revision 3.34  2001/07/18 12:57:45  cg
 * Applications of old tree construction function
 * AppendNodeChain eliminated.
 *
 * Revision 3.33  2001/07/16 08:23:11  cg
 * Old tree construction function MakeNode eliminated.
 *
 * Revision 3.32  2001/07/13 13:23:41  cg
 * DBUG tags renamed:
 * GENTREE -> PARSE and GENSIB -> PARSE_SIB
 *
 * Revision 3.31  2001/06/28 13:13:28  cg
 * Type syntax changed: type[] is now equivalent to type.
 *
 * Revision 3.30  2001/06/28 09:26:06  cg
 * Syntax of array types changed:
 * int[] -> int[+]  and  int[?] -> int[*]
 * int[] is still supported as input.
 *
 * Revision 3.29  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.28  2001/06/22 12:18:53  dkr
 * fixed a bug in rule for 'expr_sign':
 * negation of float/double works correctly now
 *
 * Revision 3.27  2001/06/21 15:57:41  dkr
 * some superfluous tokens removed
 *
 * Revision 3.26  2001/06/21 15:51:33  dkr
 * problem with sign operators partially fixed. Open problems:
 *   - (: ... ) ...     still causes a syntax error,
 *   - ( ... ) [ ... ]  is parsed wrongly as  (- ( ... )) [ ... ]
 * (+ analogous)
 *
 * Revision 3.25  2001/06/20 13:27:30  sbs
 * changed priority of SIGN (unary minus) now less than sel!
 *
 * Revision 3.24  2001/05/23 07:39:53  sbs
 * moved eof 'out' of all!
 * The alpha-trick doesn't work within a standalone function!
 * It has to be within the rules section!
 *
 * Revision 3.23  2001/05/22 15:06:22  dkr
 * function CleanUpParser() added.
 * rule 'eof' replaced by rule 'all'.
 * space leak in function CheckWlcompConf() removed.
 *
 * Revision 3.22  2001/05/18 09:26:43  cg
 * MALLOC and FREE transformed into Malloc and Free.
 *
 * Revision 3.21  2001/04/26 12:21:16  dkr
 * GetExprsLength() renamed into CountExprs()
 *
 * Revision 3.20  2001/04/24 14:13:51  dkr
 * MakeNode( N_fundef) replaced by MakeFundef()
 *
 * Revision 3.19  2001/04/24 13:04:40  dkr
 * type 'id' replaced by 'char'
 *
 * Revision 3.18  2001/04/24 09:34:07  dkr
 * CHECK_NULL renamed into STR_OR_EMPTY
 *
 * Revision 3.17  2001/04/24 09:05:55  dkr
 * P_FORMAT replaced by F_PTR
 *
 *  ... [eliminated] 
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "typecheck.h"
#include "DupTree.h"        /* for use of DupTree */
#include "my_debug.h"
#include "Error.h"
#include "free.h"
#include "globals.h"

#include "readsib.h"
#include "resource.h"


extern int commlevel;
extern int charpos;
extern char *linebuf_ptr;
extern char *yytext;

extern int yylex();


node *syntax_tree;
node *decl_tree;
node *sib_tree;
node *spec_tree;


static char *mod_name = MAIN_MOD_NAME;
static char *link_mod_name = NULL;
static node *store_pragma = NULL;
static node *global_wlcomp_aps = NULL;

/*
 * used to distinguish the different kinds of files
 * which are parsed with this single parser
 */
static file_type file_kind = F_prog;

/*
 * This variable is used to easily distinguish between modules and
 * classes when parsing a SIB.
 */
static statustype sib_imported_status;

static int yyerror( char *errname);
static int yyparse();

static void CleanUpParser();
static node *String2Array( char *str);
static types *GenComplexType( types *types, nums *numsp);
static node *CheckWlcompConf( node *ap, node *exprs);


%}

%union {
         nodetype        nodetype;
         char            *id;
         ids             *ids;
         types           *types;
         node            *node;
         int             cint;
         char            cchar;
         float           cfloat;
         double          cdbl;
         nums            *nums;
         deps            *deps;
         prf             prf;
         statustype      statustype;
         strings         *strings;
         resource_list_t *resource_list_t;
         target_list_t   *target_list_t;
       }

%token PARSE_PRG, PARSE_DEC, PARSE_SIB, PARSE_RC, PARSE_SPEC

%token BRACE_L, BRACE_R, BRACKET_L, BRACKET_R, SQBR_L, SQBR_R, COLON, SEMIC,
       COMMA, AMPERS, DOT, QUESTION,
       INLINE, LET, TYPEDEF, OBJDEF, CLASSTYPE,
       INC, DEC, ADDON, SUBON, MULON, DIVON, MODON
       K_MAIN, RETURN, IF, ELSE, DO, WHILE, FOR, NWITH, FOLD,
       MODDEC, MODSPEC, MODIMP, CLASSDEC, IMPORT, IMPLICIT, EXPLICIT, TYPES,
       FUNS, OWN, GLOBAL, OBJECTS, CLASSIMP,
       SC, TRUETOKEN, FALSETOKEN, EXTERN, C_KEYWORD,
       PRAGMA, LINKNAME, LINKSIGN, EFFECT, READONLY, REFCOUNTING,
       TOUCH, COPYFUN, FREEFUN, INITFUN, LINKWITH,
       WLCOMP, CACHESIM, SPECIALIZE, 
       STEP, WIDTH, TARGET,
       AND, OR, EQ, NEQ, NOT, LE, LT, GE, GT, MUL, DIV, PRF_MOD, PLUS,
       TOI, TOF, TOD, ABS, PRF_MIN, PRF_MAX, ALL,
       RESHAPE, SHAPE, TAKE, DROP, DIM, ROTATE, CAT, SEL, GENARRAY, MODARRAY

%token <id> ID, STR, MINUS, PRIVATEID, OPTION

%token <types> TYPE_INT, TYPE_FLOAT, TYPE_BOOL, TYPE_UNS, TYPE_SHORT,
               TYPE_LONG, TYPE_CHAR, TYPE_DBL, TYPE_VOID, TYPE_DOTS
%token <cint> NUM
%token <cfloat> FLOAT
%token <cdbl> DOUBLE
%token <cchar> CHAR

%type <prf> foldop, genop, monop, binop, triop
%type <nodetype> modclass
%type <cint> evextern, sibheader, sibevmarker, dots
%type <ids> ids, funids, modnames, modname, inherits
%type <deps> linkwith, linklist, siblinkwith, siblinklist, sibsublinklist
%type <id> fun_name, prf_name, sibparam, id, string, pragmacachesim
%type <nums> nums
%type <statustype> sibreference, sibevclass, siblinkliststatus
%type <types> simpletype, simpletype_main, 
              localtype, localtype_main, local_type_udt_arr,
              type, typeNOudt_arr,
              types, returntypes, varreturntypes, vartypes;
%type <node> arg, args, fundefs, fundef, main, prg, modimp, module, class, 
             argtypes, argtype, varargs, varargtypes,
             fundec2, fundec3, pragmas,
             typedefs, typedef, defs, def2, def3, def4, fundef2,
             objdefs, objdef, exprblock, exprblock2,
             assign, assigns, assignblock, letassign, 
             selassign, forassign, assignsOPTret, wlassignblock, optelse,
             expr, expr_main,
             expr_br, expr_sel, expr_expr, expr_sign, expr_ap, expr_ar,
             exprORdot, exprNOar, exprNObr, expr_selNObr,
             exprs, exprsNOar,
             generator, steps, width, withop, genidx,
             moddec, modspec, expdesc, expdesc2, expdesc3, expdesc4,
             fundecs, fundec,
             exptypes, exptype, objdecs, objdec, evimport, modheader,
             imptypes, imptype,import, imports, impdesc, impdesc2, impdesc3,
             impdesc4, opt_arguments,
             sib, sibtypes, sibtype, sibfuns, sibfun, sibfunbody,
             sibobjs, sibobj, sibpragmas, sibarglist,
             sibargs, sibarg, sibfunlist, sibfunlistentry,
             wlcomp_pragma_global, wlcomp_pragma_local, wlcomp_conf
%type <target_list_t> targets
%type <resource_list_t> resources


%left OR
%left AND
%left EQ, NEQ
%left LE, LT, GE, GT
%left PLUS, MINUS
%left MUL, DIV, PRF_MOD
%left TAKE, DROP, RESHAPE
%nonassoc SIGN
%left SQBR_L
%right CAST
%right NOT
%right ELSE
%nonassoc GENERATOR


%start all

%{

/*
 * Make sure, the stack of the generated parser is big enough!
 */
#define YYMAXDEPTH 10000 

%}
%%

all: file eof { CleanUpParser(); }

file: PARSE_PRG prg
        {
          syntax_tree = $2;
        }
    | PARSE_PRG modimp
        {
          syntax_tree = $2;
        }
    | PARSE_DEC moddec
        {
          decl_tree = $2;
        }
    | PARSE_SIB sib
        {
          sib_tree = $2;
        }
    | PARSE_RC targets
        {
          target_list = RSCAddTargetList( $2, target_list);
        }
    | PARSE_SPEC modspec
        {
          spec_tree = $2;
        }
    ;


eof:
     {
       if (commlevel) {
         ABORT( linenum, ("Unterminated comment found"));

#ifdef SAC_FOR_OSF_ALPHA
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


id: ID
      {
        $$ = $1;
      }
  | PRIVATEID
      {
        if (file_kind != F_sib) {
          ABORT( linenum, ("Identifier name '%s` illegal", $1));
        }
        else {
          $$ = $1;
        }
      }
  ;


/*
 *********************************************************************
 *
 *  rules for module/class declarations
 *
 *********************************************************************
 */


moddec: modheader evimport OWN COLON expdesc
        {
          $$ = $1;
          $$->node[0] = $5;
          $$->node[1] = $2;
          if ($$->node[1] != NULL) {
             DBUG_PRINT("PARSE",
                       ("%s:"F_PTR" Id: %s , %s"F_PTR" %s," F_PTR,
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$,
                        $$->info.fun_name.id,
                        mdb_nodetype[ NODE_TYPE( $$->node[0])],
                        $$->node[0],
                        mdb_nodetype[ NODE_TYPE( $$->node[1])],
                        $$->node[1]));
          }
          else {
             DBUG_PRINT("PARSE",
                       ("%s:"F_PTR" Id: %s , %s"F_PTR,
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$,
                        $$->info.fun_name.id,
                        mdb_nodetype[ NODE_TYPE( $$->node[0])],
                        $$->node[0]));
          }
        }
      | modheader evimport
        {
          $$ = $1;
          $$->node[0] = MakeExplist( NULL, NULL, NULL, NULL);
          $$->node[1] = $2;
          if ($$->node[1] != NULL) {
             DBUG_PRINT("PARSE",
                       ("%s:"F_PTR" Id: %s , %s"F_PTR" %s," F_PTR,
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$,
                        $$->info.fun_name.id,
                        mdb_nodetype[ NODE_TYPE( $$->node[0])],
                        $$->node[0],
                        mdb_nodetype[ NODE_TYPE( $$->node[1])],
                        $$->node[1]));
          }
          else {
             DBUG_PRINT("PARSE",
                       ("%s:"F_PTR" Id: %s , %s"F_PTR,
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$,
                        $$->info.fun_name.id,
                        mdb_nodetype[ NODE_TYPE( $$->node[0])],
                        $$->node[0]));
          }
        }
      ;

modspec:  modheader OWN COLON expdesc
          {
          $$ = $1;
          $$->node[0] = $4;
          $$->node[1] = NULL;
	  DBUG_PRINT("PARSE",
		     ("%s:"F_PTR" Id: %s , %s"F_PTR,
		      mdb_nodetype[ NODE_TYPE( $$)],
                      $$,
                      $$->info.fun_name.id,
		      mdb_nodetype[ NODE_TYPE( $$->node[0])],
                      $$->node[0]));
	  };

modheader: modclass evextern id COLON linkwith
           {
             switch ($1) {
             case N_moddec: 
               $$ = MakeModdec( $3, $5, $2, NULL, NULL);
               break;
             case N_classdec:
               $$ = MakeClassdec( $3, $5, $2, NULL, NULL);
               break;
             case N_modspec: 
               $$ = MakeModspec( $3, NULL);
               break;
             default:
               DBUG_ASSERT((0), ("Illegal declaration type"));
               $$ = NULL;
             }
                 
             link_mod_name = $3;

             if ($2) {
               mod_name = NULL;
             }
             else {
               mod_name = link_mod_name;
             }
           }
         ;                 

modclass: MODDEC   { $$ = N_moddec;   file_kind = F_moddec;   }
        | CLASSDEC { $$ = N_classdec; file_kind = F_classdec; }
        | MODSPEC  { $$ = N_modspec;  file_kind = F_modspec;  }
        ;

evextern: EXTERN 
          {
            file_kind++;
            $$ = 1;
          }
        |
          {
            $$ = 0;
          }
        ;

linkwith: PRAGMA LINKWITH linklist
            { $$ = $3; }
        |   { $$ = NULL; }
        ;

linklist: string COMMA linklist 
           {
             $$ = MakeDeps( $1, NULL, NULL, ST_system, LOC_stdlib, NULL, $3);
           }
        | string
           {
             $$ = MakeDeps( $1, NULL, NULL, ST_system, LOC_stdlib, NULL, NULL);
           }
        ;

evimport: imports { $$ = $1; }
          | { $$ = NULL; }
          ;

imports: import imports { $$ = $1;
                          $$->node[0] = $2;
                        }
       | import { $$ = $1; }
       ;

import: IMPORT id COLON impdesc { $$ = $4; $$->info.id = $2; }
        ;

impdesc: ALL SEMIC
           { $$ = MakeImplist( NULL, NULL, NULL, NULL, NULL, NULL);

             DBUG_PRINT("PARSE",
                        ("%s:"F_PTR,
                         mdb_nodetype[ NODE_TYPE( $$)],
                         $$));
           }
       | BRACE_L IMPLICIT TYPES COLON ids SEMIC impdesc2
           { $$ = $7;
             $$->node[1] = (node *) $5;  /* dirty trick for keeping ids! */
           }
       | BRACE_L IMPLICIT TYPES COLON SEMIC impdesc2
           { $$ = $6;
           }
       | BRACE_L impdesc2 { $$ = $2; }
       ;

impdesc2: EXPLICIT TYPES COLON ids SEMIC impdesc3
            { $$ = $6;
              $$->node[2] = (node *) $4;  /* dirty trick for keeping ids! */
            }
        | EXPLICIT TYPES COLON SEMIC impdesc3
            { $$ = $5;
            }
        | impdesc3 { $$ = $1; }
        ;

impdesc3: GLOBAL OBJECTS COLON ids SEMIC impdesc4
            { $$ = $6;
              $$->node[4] = (node *) $4;  /* dirty trick for keeping ids! */
            }
        | GLOBAL OBJECTS COLON SEMIC impdesc4
            { $$ = $5;
            }
        | impdesc4 { $$ = $1; }
        ;

impdesc4: FUNS COLON funids SEMIC BRACE_R
            { $$ = MakeImplist( NULL, NULL, NULL, NULL, $3, NULL);

              DBUG_PRINT("PARSE",
                         ("%s:"F_PTR,
                          mdb_nodetype[ NODE_TYPE( $$)],
                          $$));
            }
        | FUNS COLON SEMIC BRACE_R
            { $$ = MakeImplist( NULL, NULL, NULL, NULL, NULL, NULL);

              DBUG_PRINT("PARSE",
                         ("%s:"F_PTR,
                          mdb_nodetype[ NODE_TYPE( $$)],
                          $$));
            }
        | BRACE_R
            { $$ = MakeImplist( NULL, NULL, NULL, NULL, NULL, NULL);

              DBUG_PRINT("PARSE",
                         ("%s:"F_PTR,
                          mdb_nodetype[ NODE_TYPE( $$)],
                          $$));
            }
        ;

expdesc: BRACE_L IMPLICIT TYPES COLON imptypes expdesc2
           { $$ = $6;
             $$->node[0] = $5;
           }
       | BRACE_L expdesc2 
           { $$ = $2; }
       | BRACE_L IMPLICIT TYPES COLON expdesc2
           { $$ = $5; }
       ;

expdesc2: EXPLICIT TYPES COLON exptypes expdesc3
            { $$ = $5;
              $$->node[1] = $4;
            }
        | EXPLICIT TYPES COLON expdesc3
            { $$ = $4;
            }
        | expdesc3 { $$ = $1; }
        ;

expdesc3: GLOBAL OBJECTS COLON objdecs expdesc4
            { $$ = $5;
              $$->node[3] = $4;
            }
        | GLOBAL OBJECTS COLON expdesc4
            { $$ = $4;
            }
        | expdesc4 { $$ = $1; }
        ;

expdesc4: FUNS COLON fundecs BRACE_R
            { $$ = MakeExplist( NULL, NULL, NULL, $3);

              DBUG_PRINT("PARSE",
                         ("%s:"F_PTR,
                          mdb_nodetype[ NODE_TYPE( $$)],
                          $$));
            }
        | FUNS COLON BRACE_R
            { $$ = MakeExplist( NULL, NULL, NULL, NULL);

              DBUG_PRINT("PARSE",
                         ("%s:"F_PTR,
                          mdb_nodetype[ NODE_TYPE( $$)],
                          $$));
            }
        | BRACE_R
            { $$ = MakeExplist( NULL, NULL, NULL, NULL);
         
              DBUG_PRINT("PARSE",
                         ("%s:"F_PTR,
                          mdb_nodetype[ NODE_TYPE( $$)],
                          $$));
            }
        ;

imptypes: imptype imptypes
            { $$ = $1;
              $1->node[0] = $2;
            }
        | imptype { $$ = $1; }
        ;

imptype: id SEMIC pragmas
           {
             $$ = MakeTypedef( $1, mod_name,
                               MakeTypes1( T_hidden),
                               ST_regular, NULL);

             DBUG_PRINT("PARSE",("type:"F_PTR" %s",
                                   TYPEDEF_TYPE( $$),
                                   mdb_type[TYPEDEF_BASETYPE( $$)]));

             TYPEDEF_LINKMOD( $$) = link_mod_name;
             TYPEDEF_STATUS( $$) = (file_kind == F_moddec)
                                     ? ST_imported_mod : ST_imported_class;
             TYPEDEF_PRAGMA( $$) = $3;

             DBUG_PRINT("PARSE",
                      ("%s:"F_PTR","F_PTR", Id: %s",
                       mdb_nodetype[ NODE_TYPE( $$)],
                       $$,
                       TYPEDEF_TYPE( $$),
                       TYPEDEF_NAME( $$)));
           }
       ;

exptypes: exptype exptypes
            { $$ = $1;
              $1->node[0] = $2;
            }
        | exptype { $$ = $1; }
        ;

exptype: id LET type SEMIC pragmas
           { $$ = MakeTypedef( $1, mod_name, $3, ST_regular, NULL);
             TYPEDEF_LINKMOD( $$) = link_mod_name;
             TYPEDEF_STATUS( $$) = (file_kind == F_moddec)
                                     ? ST_imported_mod : ST_imported_class;
             TYPEDEF_PRAGMA( $$) = $5;

             DBUG_PRINT("PARSE",
                        ("%s:"F_PTR","F_PTR", Id: %s",
                         mdb_nodetype[ NODE_TYPE( $$)],
                         $$, 
                         TYPEDEF_TYPE( $$),
                         TYPEDEF_NAME( $$)));
           }
       ;

objdecs: objdec objdecs
           { $$ = $1;
             $$->node[0] = $2;
           }
        | objdec { $$ = $1; }
        ;

objdec: type id SEMIC pragmas
          { $$ = MakeObjdef( $2, mod_name, $1, NULL, NULL);
            OBJDEF_LINKMOD( $$) = link_mod_name; /* external module name */
            OBJDEF_STATUS( $$) = (file_kind == F_moddec)
                                   ? ST_imported_mod : ST_imported_class;
            OBJDEF_PRAGMA( $$) = $4;

            DBUG_PRINT("PARSE",
                       ("%s:"F_PTR","F_PTR", Id: %s",
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$, 
                        OBJDEF_TYPE( $$),
                        OBJDEF_NAME( $$)));
          }
      ;


fundecs: fundec fundecs 
           {
             $$ = $1;
             FUNDEF_NEXT( $$) = $2;
           }
       | fundec { $$ = $1; }
       ;

fundec: varreturntypes id BRACKET_L fundec2
          {
            $$ = $4;
            FUNDEF_TYPES( $$) = $1;
            FUNDEF_NAME( $$) = $2;                /* function name */
            FUNDEF_MOD( $$) = mod_name;           /* SAC modul name */
            FUNDEF_LINKMOD( $$) = link_mod_name;  /* external modul name */
            FUNDEF_ATTRIB( $$) = ST_regular;
            FUNDEF_STATUS( $$) = (file_kind == F_moddec)
                                   ? ST_imported_mod : ST_imported_class;

            DBUG_PRINT("PARSE",
                       ("%s:"F_PTR" Id: %s , NULL body,  %s" F_PTR,
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$,
                        FUNDEF_NAME( $$),
                        mdb_nodetype[ ($$->node[2]==NULL)
                                        ? T_void
                                        : (NODE_TYPE( $$->node[2]))],
                        $$->node[2]));
          }         
      | returntypes prf_name BRACKET_L fundec3
          { 
            $$ = $4;
            FUNDEF_TYPES( $$) = $1;
            FUNDEF_NAME( $$) = $2;                /* function name */
            FUNDEF_MOD( $$) = mod_name;           /* SAC module name */
            FUNDEF_LINKMOD( $$) = link_mod_name;  /* external module name */
            FUNDEF_ATTRIB( $$) = ST_regular;
            FUNDEF_STATUS( $$) = (file_kind == F_moddec)
                                  ? ST_imported_mod : ST_imported_class;

            DBUG_PRINT("PARSE",
                       ("%s:"F_PTR" Id: %s"F_PTR", NULL body,  %s"
                        F_PTR,
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$, 
                        FUNDEF_NAME( $$),
                        FUNDEF_NAME( $$),
                        mdb_nodetype[ ($$->node[2]==NULL)
                                        ? T_void 
                                        : (NODE_TYPE( $$->node[2]))],
                        $$->node[2]));
          }
      ;

fundec2: varargtypes BRACKET_R { $<cint>$ = linenum; } SEMIC pragmas
           {
             $$ = MakeFundef( NULL, NULL, NULL, $1, NULL, NULL);
             NODE_LINE( $$) = $<cint>3;
             FUNDEF_PRAGMA( $$) = $5;
           }
       | varargs BRACKET_R { $<cint>$ = linenum; } SEMIC pragmas
           {
             $$ = MakeFundef( NULL, NULL, NULL, $1, NULL, NULL);
             NODE_LINE( $$) = $<cint>3;
             FUNDEF_PRAGMA( $$) = $5;
           }
       | TYPE_DOTS BRACKET_R { $<cint>$ = linenum; } SEMIC pragmas
           {
             if ((F_extmoddec != file_kind) && (F_extclassdec != file_kind)) {
               strcpy( yytext, "...");
               yyerror( "syntax error");
             }
             else {
               $$ = MakeFundef( NULL, NULL, NULL,
                                MakeArg( NULL,
                                         MakeTypes1( T_dots),
                                         ST_regular, ST_regular,
                                         NULL),
                                NULL, NULL);
               NODE_LINE( $$) = $<cint>3;
               FUNDEF_PRAGMA( $$) = $5;
             }
           }
       | BRACKET_R { $<cint>$ = linenum; } SEMIC pragmas
           {
             $$ = MakeFundef( NULL, NULL, NULL, NULL, NULL, NULL);
             NODE_LINE( $$) = $<cint>2;
             FUNDEF_PRAGMA( $$) = $4;
           }
       ;

fundec3: argtypes BRACKET_R { $<cint>$ = linenum; } SEMIC pragmas
           {
             $$ = MakeFundef( NULL, NULL, NULL, $1, NULL, NULL);
             NODE_LINE( $$) = $<cint>3;
             FUNDEF_PRAGMA( $$) = $5;
           }
       | args BRACKET_R { $<cint>$ = linenum; } SEMIC pragmas
           {
             $$ = MakeFundef( NULL, NULL, NULL, $1, NULL, NULL);
             NODE_LINE( $$) = $<cint>3;
             FUNDEF_PRAGMA( $$) = $5;
           }
       | BRACKET_R { $<cint>$ = linenum; } SEMIC pragmas
           {
             $$ = MakeFundef( NULL, NULL, NULL, NULL, NULL, NULL);
             NODE_LINE( $$) = $<cint>2;
             FUNDEF_PRAGMA( $$) = $4;
           }
       ;


pragmas: pragmalist
           {
             $$ = store_pragma;
             store_pragma = NULL;
           }
       |
           {
             $$ = NULL;
           }
       ;

pragmalist: pragmalist pragma
          | pragma
          ;

pragma: PRAGMA LINKNAME string
          {
            if (store_pragma==NULL) store_pragma=MakePragma();
            if (PRAGMA_LINKNAME(store_pragma)!=NULL)
              WARN(linenum, ("Conflicting definitions of pragma 'linkname`"))
            PRAGMA_LINKNAME(store_pragma) = $3;
          }
      | PRAGMA LINKSIGN SQBR_L nums SQBR_R
          {
            if (store_pragma==NULL) store_pragma=MakePragma();
            if (PRAGMA_LINKSIGNNUMS(store_pragma)!=NULL)
              WARN(linenum, ("Conflicting definitions of pragma 'linksign`"))
            PRAGMA_LINKSIGNNUMS(store_pragma) = $4;
          }
      | PRAGMA REFCOUNTING SQBR_L nums SQBR_R
          {
            if (store_pragma==NULL) store_pragma=MakePragma();
            if (PRAGMA_REFCOUNTINGNUMS(store_pragma)!=NULL)
              WARN(linenum, ("Conflicting definitions of pragma 'refcounting`"))
            PRAGMA_REFCOUNTINGNUMS(store_pragma) = $4;
          }
      | PRAGMA READONLY SQBR_L nums SQBR_R
          {
            if (store_pragma==NULL) store_pragma=MakePragma();
            if (PRAGMA_READONLYNUMS(store_pragma)!=NULL)
              WARN(linenum, ("Conflicting definitions of pragma 'readonly`"))
            PRAGMA_READONLYNUMS(store_pragma) = $4;
          }
      | PRAGMA EFFECT modnames
          {
            if (store_pragma==NULL) store_pragma=MakePragma();
            if (PRAGMA_EFFECT(store_pragma)!=NULL)
              WARN(linenum, ("Conflicting definitions of pragma 'effect`"))
            PRAGMA_EFFECT(store_pragma) = $3;
          }
      | PRAGMA TOUCH modnames
          {
            if (store_pragma==NULL) store_pragma=MakePragma();
            if (PRAGMA_TOUCH(store_pragma)!=NULL)
              WARN(linenum, ("Conflicting definitions of pragma 'touch`"))
            PRAGMA_TOUCH(store_pragma) = $3;
          }
      | PRAGMA COPYFUN string
          {
            if (store_pragma==NULL) store_pragma=MakePragma();
            if (PRAGMA_COPYFUN(store_pragma)!=NULL)
              WARN(linenum, ("Conflicting definitions of pragma 'copyfun`"))
            PRAGMA_COPYFUN(store_pragma) = $3;
          }
      | PRAGMA FREEFUN string
          {
            if (store_pragma==NULL) store_pragma=MakePragma();
            if (PRAGMA_FREEFUN(store_pragma)!=NULL)
              WARN(linenum, ("Conflicting definitions of pragma 'freefun`"))
            PRAGMA_FREEFUN(store_pragma) = $3;
          }
      | PRAGMA INITFUN string
          {
            if (store_pragma==NULL) store_pragma=MakePragma();
            if (PRAGMA_INITFUN(store_pragma)!=NULL)
              WARN(linenum, ("Conflicting definitions of pragma 'initfun`"))
            PRAGMA_INITFUN(store_pragma) = $3;
          }
      ;


modnames: modname COMMA modnames
            {
              $$ = $1;
              IDS_NEXT( $$) = $3;
            }
        | modname
            {
              $$ = $1;
            }
        ;

modname: id
           {
             $$ = MakeIds( $1, NULL, ST_regular);
           }
       | id COLON id
           {
             $$ = MakeIds( $3, $1, ST_regular);
           }
       ;


/*
 *********************************************************************
 *
 *  rules for type, object, and function definitions
 *
 *********************************************************************
 */


defs: imports def2
        { $$ = $2;
          $$->node[0] = $1;
        }
    | def2
        { $$ = $1; }

def2: typedefs def3
        { $$ = $2;
          $$->node[1] = $1;
        }
    | def3
        { $$ = $1; }

def3: objdefs def4
        { $$ = $2;
          $$->node[3] = $1;
        }
    | def4
        { $$ = $1; }
    ;

def4: fundefs
        { 
          $$ = MakeModul( NULL, F_prog, NULL, NULL, NULL, $1);
          
          DBUG_PRINT("PARSE",
                     ("%s:"F_PTR"  %s"F_PTR,
                      mdb_nodetype[ NODE_TYPE( $$)],
                      $$, 
                      mdb_nodetype[ NODE_TYPE(  $$->node[2])],
                      $$->node[2]));
        }
    |   { 
          $$ = MakeModul( NULL, F_prog, NULL, NULL, NULL, NULL);

          DBUG_PRINT("PARSE",
                     ("%s:"F_PTR,
                      mdb_nodetype[ NODE_TYPE( $$)],
                      $$));
        }
    ;


modimp: module
          {
            $$ = $1;
          }
      | class
          {
            $$ = $1;
          }
      ;

module: MODIMP { file_kind = F_modimp; } id { mod_name = $3; } COLON defs
          {
            $$ = $6;
            MODUL_NAME( $$) = mod_name;
            MODUL_FILETYPE( $$) = file_kind;
          }
        ;

class: CLASSIMP { file_kind = F_classimp; } id { mod_name = $3; } COLON 
       CLASSTYPE type SEMIC defs
         { 
           $$ = $9;
           MODUL_NAME( $$) = mod_name;
           MODUL_FILETYPE( $$) = file_kind;
           MODUL_CLASSTYPE( $$) = $7;
         }
     ;

prg: defs
       { $$ = $1;
         MODUL_NAME( $$) = mod_name;
         MODUL_FILETYPE( $$) = F_prog;
       }
   ;


typedefs: typedef typedefs
            { $$ = $1;
              $1->node[0] = $2;
            }
        | typedef
            { $$ = $1; }
        ;

typedef: TYPEDEF type id SEMIC 
           { $$ = MakeTypedef( $3, mod_name, $2, ST_regular, NULL);

             DBUG_PRINT("PARSE",
                        ("%s:"F_PTR","F_PTR", Id: %s",
                         mdb_nodetype[ NODE_TYPE( $$)],
                         $$, 
                         TYPEDEF_TYPE( $$),
                         TYPEDEF_NAME( $$)));
           }


objdefs: objdef objdefs
           { $$ = $1;
             $$->node[0] = $2;
           }
       | objdef
           { $$ = $1; }
       ;

objdef: OBJDEF type id LET expr SEMIC 
          { $$ = MakeObjdef( $3, mod_name, $2, $5, NULL);

            DBUG_PRINT("PARSE",
                       ("%s:"F_PTR","F_PTR", Id: %s",
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$, 
                        OBJDEF_TYPE( $$),
                        OBJDEF_NAME( $$)));
          }
      ;


fundefs: wlcomp_pragma_global fundef fundefs
           { $$ = $2;
             FUNDEF_NEXT( $$) = $3;
           }
       | wlcomp_pragma_global main
           { $$ = $2;
           }
       | wlcomp_pragma_global fundef
           { $$ = $2;
           }
       ;

fundef: returntypes fun_name BRACKET_L fundef2
          { $$ = $4;
            FUNDEF_TYPES( $$) = $1;              /* result type(s) */
            FUNDEF_NAME( $$) = $2;               /* function name  */
            FUNDEF_MOD( $$) = mod_name;          /* module name    */
            FUNDEF_LINKMOD( $$) = NULL;
            FUNDEF_ATTRIB( $$) = ST_regular;
            FUNDEF_STATUS( $$) = ST_regular;
            FUNDEF_INLINE( $$) = FALSE;          /* inline flag    */

            DBUG_PRINT("PARSE",
                       ("%s: %s:%s "F_PTR,
                        mdb_nodetype[ NODE_TYPE( $$)],
                        FUNDEF_MOD( $$),
                        FUNDEF_NAME( $$),
                        FUNDEF_NAME( $$)));

          }
      | INLINE returntypes fun_name BRACKET_L fundef2
          { $$ = $5;
            FUNDEF_TYPES( $$) = $2;              /* result type(s) */
            FUNDEF_NAME( $$) = $3;               /* function name  */
            FUNDEF_MOD( $$) = mod_name;          /* module name    */
            FUNDEF_LINKMOD( $$) = NULL;
            FUNDEF_ATTRIB( $$) = ST_regular;
            FUNDEF_STATUS( $$) = ST_regular;
            FUNDEF_INLINE( $$) = TRUE;           /* inline flag    */

            DBUG_PRINT("PARSE",
                        ("%s: %s:%s "F_PTR,
                        mdb_nodetype[ NODE_TYPE( $$)],
                        FUNDEF_MOD( $$),
                        FUNDEF_NAME( $$),
                        FUNDEF_NAME( $$)));

          }
      ;

fundef2: args BRACKET_R { $$ = MakeFundef( NULL, NULL, NULL, NULL, NULL, NULL);
         } exprblock
           { 
             $$ = $<node>3;
             FUNDEF_BODY( $$) = $4;             /* Funktionsrumpf  */
             FUNDEF_ARGS( $$) = $1;             /* Funktionsargumente */

             DBUG_PRINT("PARSE",
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
           { 
             $$ = $<node>2;
             FUNDEF_BODY( $$) = $3;             /* Funktionsrumpf  */

             DBUG_PRINT("PARSE",
                        ("%s:"F_PTR" %s"F_PTR,
                         mdb_nodetype[ NODE_TYPE( $$)],
                         $$, 
                         mdb_nodetype[ NODE_TYPE( FUNDEF_BODY( $$))],
                         FUNDEF_BODY( $$)));
           }
       ;


args: arg COMMA args
        { $1->node[0] = $3;
          $$ = $1;
        }
    | arg
        { $$ = $1; }
    ;

varargs: arg COMMA varargs
           {
             ARG_NEXT( $1) = $3;
             $$ = $1;
           }
       | arg COMMA TYPE_DOTS
           {
             if ((F_extmoddec != file_kind) && (F_extclassdec != file_kind)) {
               strcpy( yytext, "...");
               yyerror( "syntax error");
             }
             else {
               $$ = $1;
               ARG_NEXT( $$) = MakeArg( NULL,
                                        MakeTypes1( T_dots),
                                        ST_regular, ST_regular,
                                        NULL);

               DBUG_PRINT("PARSE",
                          ("%s: "F_PTR", Id: ..., Attrib: %d  ",
                           mdb_nodetype[ NODE_TYPE( $$)],
                           $$, 
                           ARG_ATTRIB( $$)));
             } 
           }
       | arg
           {
             $$ = $1;
           }
       ;

arg: type id
       { $$ = MakeArg( $2, $1, ST_regular, ST_regular, NULL); 

         DBUG_PRINT("PARSE",
                    ("%s: "F_PTR", Id: %s, Attrib: %d  ",
                     mdb_nodetype[ NODE_TYPE( $$)],
                     $$, 
                     ARG_NAME( $$),
                     ARG_ATTRIB( $$)));
       }
   | type AMPERS id
       { $$ = MakeArg( $3, $1, ST_regular, ST_reference, NULL); 

         DBUG_PRINT("PARSE",
                    ("%s: "F_PTR", Id: %s, Attrib: %d ",
                     mdb_nodetype[ NODE_TYPE( $$)],
                     $$, 
                     ARG_NAME( $$),
                     ARG_ATTRIB( $$)));
       }
   ;

argtypes: argtype COMMA argtypes
            {
              ARG_NEXT( $1) = $3;
              $$ = $1;
            }
        | argtype
            {
              $$ = $1;
            }
        ;

varargtypes: argtype COMMA varargtypes
               {
                 ARG_NEXT( $1) = $3;
                 $$ = $1;
               }
           | argtype COMMA TYPE_DOTS
               {
                 if ((F_extmoddec != file_kind) && (F_extclassdec != file_kind))
                 {
                   strcpy( yytext, "...");
                   yyerror( "syntax error");
                 }
                 else
                 {
                   $$ = $1;
                   ARG_NEXT( $$) = MakeArg( NULL,
                                            MakeTypes1( T_dots),
                                            ST_regular, ST_regular,
                                            NULL);

                   DBUG_PRINT("PARSE",
                              ("%s: "F_PTR", Attrib: %d  ",
                               mdb_nodetype[ NODE_TYPE( $$)],
                               $$, 
                               ARG_ATTRIB( $$)));

                 }
               }
           | argtype
               { $$ = $1; }
           ;

argtype: type
           { $$ = MakeArg( NULL, $1, ST_regular, ST_regular, NULL);

             DBUG_PRINT("PARSE",
                        ("%s: "F_PTR", Attrib: %d  ",
                         mdb_nodetype[ NODE_TYPE( $$)],
                         $$, 
                         ARG_ATTRIB( $$)));
           }
       | type AMPERS
           { $$ = MakeArg( NULL, $1, ST_regular, ST_reference, NULL);
                     
             DBUG_PRINT("PARSE",
                        ("%s: "F_PTR", Attrib: %d ",
                         mdb_nodetype[ NODE_TYPE( $$)],
                         $$, 
                         ARG_ATTRIB( $$)));
           }
       ;


fun_name : id
             { $$ = $1; }
         | prf_name
             { $$ = $1; }
         ;

prf_name : AND        { $$ = StringCopy( prf_name_str[F_and]); }
         | OR         { $$ = StringCopy( prf_name_str[F_or]); }
         | EQ         { $$ = StringCopy( prf_name_str[F_eq]); }
         | NEQ        { $$ = StringCopy( prf_name_str[F_neq]); }
         | NOT        { $$ = StringCopy( prf_name_str[F_not]); }
         | LE         { $$ = StringCopy( prf_name_str[F_le]); }
         | LT         { $$ = StringCopy( prf_name_str[F_lt]); }
         | GE         { $$ = StringCopy( prf_name_str[F_ge]); }
         | GT         { $$ = StringCopy( prf_name_str[F_gt]); }
         | ABS        { $$ = StringCopy( prf_name_str[F_abs]); }
         | PLUS       { $$ = StringCopy( prf_name_str[F_add]); }
         | MINUS      { $$ = StringCopy( prf_name_str[F_sub]); }
         | DIV        { $$ = StringCopy( prf_name_str[F_div]); }
         | MUL        { $$ = StringCopy( prf_name_str[F_mul]); }
         | PRF_MOD    { $$ = StringCopy( prf_name_str[F_mod]); }
         | RESHAPE    { $$ = StringCopy( prf_name_str[F_reshape]); }
         | SHAPE      { $$ = StringCopy( prf_name_str[F_shape]); }
         | TAKE       { $$ = StringCopy( prf_name_str[F_take]); }
         | DROP       { $$ = StringCopy( prf_name_str[F_drop]); }
         | DIM        { $$ = StringCopy( prf_name_str[F_dim]); }
         | ROTATE     { $$ = StringCopy( prf_name_str[F_rotate]); }
         | CAT        { $$ = StringCopy( prf_name_str[F_cat]); }
         | SEL        { $$ = StringCopy( prf_name_str[F_sel]); }
         | GENARRAY   { $$ = StringCopy( prf_name_str[F_genarray]); }
         | MODARRAY   { $$ = StringCopy( prf_name_str[F_modarray]); }
         | TOI        { $$ = StringCopy( prf_name_str[F_toi]); }
         | TOF        { $$ = StringCopy( prf_name_str[F_tof]); }
         | TOD        { $$ = StringCopy( prf_name_str[F_tod]); }
         | PRF_MIN    { $$ = StringCopy( prf_name_str[F_min]); }
         | PRF_MAX    { $$ = StringCopy( prf_name_str[F_max]); }
         | ALL        { $$ = StringCopy( "all");
           /* necessary because function 'all()' from array library conflicts 
              with key word. */}
        ;


main: TYPE_INT K_MAIN BRACKET_L BRACKET_R { $<cint>$ = linenum; } exprblock
        {
          $$ = MakeFundef( NULL, NULL,
                           MakeTypes1( T_int),
                           NULL, $6, NULL);
          NODE_LINE( $$) = $<cint>5;

          FUNDEF_NAME( $$) = StringCopy( "main");
          FUNDEF_MOD( $$) = mod_name;           /* SAC modul name */
          FUNDEF_STATUS( $$) = ST_exported;

          DBUG_PRINT("PARSE",("%s:"F_PTR", main "F_PTR
                                "  %s (" F_PTR ") ",
                                mdb_nodetype[ NODE_TYPE( $$)],
                                $$, 
                                FUNDEF_NAME( $$),
                                mdb_nodetype[ NODE_TYPE( $$->node[0])], 
                                $$->node[0]));
        }
    ;


/* BRUSH BEGIN */


wlcomp_pragma_global: PRAGMA WLCOMP wlcomp_conf
                        {
                          if (global_wlcomp_aps != NULL) {
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

wlcomp_pragma_local: PRAGMA WLCOMP wlcomp_conf
                       {
                         $3 = CheckWlcompConf( $3, NULL);
                         if ($3 != NULL) {
                           $$ = MakePragma();
                           PRAGMA_WLCOMP_APS( $$) = $3;
                         }
                         else {
                           $$ = NULL;
                         }
                       }
                   | /* empty */
                       { if (global_wlcomp_aps != NULL) {
                           $$ = MakePragma();
                           PRAGMA_WLCOMP_APS( $$) = DupTree( global_wlcomp_aps);
                         }
                         else {
                           $$ = NULL;
                         }
                       }
                   ;

wlcomp_conf: id      { $$ = MakeId( $1, NULL, ST_regular); }
           | expr_ap { $$ = $1; }
           ;


/*
 *********************************************************************
 *
 *  rules for expression blocks
 *
 *********************************************************************
 */


exprblock: BRACE_L { $<cint>$ = linenum; } pragmacachesim exprblock2 
           {
             $$ = $4;
             BLOCK_CACHESIM( $$) = $3;
             NODE_LINE( $$) = $<cint>2;
           }
         ;

exprblock2: typeNOudt_arr ids SEMIC exprblock2 
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
                Free( $2);
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
              Free( $2); /* Finally, we free the last IDS-node! */

            }
          | assignsOPTret BRACE_R
            {
              $$ = MakeBlock( $1, NULL);
            }
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
             |
               {
                 $$ = MAKE_EMPTY_BLOCK();
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
             { $$ = MakeBlock( MakeAssign( $1, NULL), NULL);
             } 
           ;


pragmacachesim: PRAGMA CACHESIM string 
                {
                  $$ = $3;
                }
              | PRAGMA CACHESIM 
                {
                  $$ = StringCopy( "");
                }
              |
                {
                  $$ = NULL;
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
               { if (NODE_TYPE( $1) == N_assign) {
                   /*
                    * The only situation where "assign" returns an N_assign
                    * node is when a while-loop had to be parsed.
                    * In this case we get a tree of the form:
                    *
                    * N_assign -> N_let
                    *    \
                    *   N_assign -> N_while
                    *      \
                    *     NULL
                    */
                   DBUG_ASSERT(
                     (NODE_TYPE( ASSIGN_INSTR( ASSIGN_NEXT( $1))) == N_while)
                       && (ASSIGN_NEXT( ASSIGN_NEXT( $1)) == NULL),
                     "corrupted node returned for \"assign\"!");
                   $$ = $1;
                   ASSIGN_NEXT( ASSIGN_NEXT( $$)) = $3;
                 }
                 else {
                   $$ = MakeAssign( $1, $3);
                   NODE_LINE( $$) = $<cint>2;
                 }
               }
             ;

assigns: /* empty */
         { $$ = NULL;
         }
       | assign assigns
         { if (NODE_TYPE( $1) == N_assign){
             /*
              * The only situation where "assign" returns an N_assign
              * node is when a for-loop had to be parsed.
              * In this case we get a tree of the form:
              *
              * N_assign -> N_let
              *    \
              *   N_assign -> N_while
              *      \
              *     NULL
              */
             DBUG_ASSERT(
               (NODE_TYPE( ASSIGN_INSTR( ASSIGN_NEXT( $1))) == N_while)
                 && (ASSIGN_NEXT( ASSIGN_NEXT( $1)) == NULL),
               "corrupted node returned for \"assign\"!");
             $$ = $1;
             ASSIGN_NEXT( ASSIGN_NEXT( $$)) = $2;
           }
           else {
             $$ = MakeAssign( $1, $2);
           }
         }
       ;

assign: letassign SEMIC { $$ = $1; }
      | selassign       { $$ = $1; }
      | forassign       { $$ = $1; }
      ;

letassign: ids LET { $<cint>$ = linenum; } expr 
             { $$ = MakeLet( $4, $1);
               NODE_LINE( $$) = $<cint>3;
             }
         | id SQBR_L expr SQBR_R LET { $<cint>$ = linenum; } expr
             { $$ = MakeLet( MakePrf( F_modarray,
                               MakeExprs( MakeId( $1, NULL, ST_regular) ,
                                 MakeExprs( $3,
                                   MakeExprs( $7,
                                     NULL)))),
                             MakeIds( StringCopy( $1), NULL, ST_regular));
               NODE_LINE( $$) = $<cint>6;
             }
         | id SQBR_L expr COMMA exprs SQBR_R LET { $<cint>$ = linenum; } expr
             { $$ = MakeLet( MakePrf( F_modarray,
                               MakeExprs( MakeId( $1, NULL, ST_regular) ,
                                          MakeExprs( MakeArray( MakeExprs( $3,
                                                                           $5)),
                                                     MakeExprs( $9,
                                                                NULL)))),
                           MakeIds( StringCopy( $1), NULL, ST_regular));
               NODE_LINE( $$) = $<cint>8;
             }
         | expr_ap
             { $$ = MakeLet( $1, NULL);
             }
         | id INC { $$ = MAKE_INCDEC_LET( $1, F_add); }
         | INC id { $$ = MAKE_INCDEC_LET( $2, F_add); }
         | id DEC { $$ = MAKE_INCDEC_LET( $1, F_sub); }
         | DEC id { $$ = MAKE_INCDEC_LET( $2, F_sub); }
         | id ADDON expr { $$ = MAKE_OPON_LET( $1, $3, F_add); }
         | id SUBON expr { $$ = MAKE_OPON_LET( $1, $3, F_sub); }
         | id MULON expr { $$ = MAKE_OPON_LET( $1, $3, F_mul); }
         | id DIVON expr { $$ = MAKE_OPON_LET( $1, $3, F_div); }
         | id MODON expr { $$ = MAKE_OPON_LET( $1, $3, F_mod); }
         ;

selassign: IF { $<cint>$ = linenum; } BRACKET_L exprNOar BRACKET_R assignblock 
           optelse
             { $$ = MakeCond( $4, $6, $7);
               NODE_LINE( $$) = $<cint>2;
             }
         ;

optelse: ELSE assignblock        { $$ = $2;                 }
       | /* empty */  %prec ELSE { $$ = MAKE_EMPTY_BLOCK(); } 
       ;

forassign: DO { $<cint>$ = linenum; } assignblock
           WHILE BRACKET_L exprNOar BRACKET_R SEMIC
             { $$ = MakeDo( $6, $3);
               NODE_LINE( $$) = $<cint>2;
             }
         | WHILE { $<cint>$ = linenum; } BRACKET_L exprNOar BRACKET_R
           assignblock
             { $$ = MakeWhile( $4, $6);
               NODE_LINE( $$) = $<cint>2;
             }
         | FOR { $<cint>$ = linenum; }
           BRACKET_L assign exprNOar SEMIC letassign BRACKET_R assignblock
             { /*
                * for( x=e1; e2; y=e3) AssBlock
                * is transformed into
                * x=e1;
                * while( e2) { AssBlock; y=e3; }
                */
               BLOCK_INSTR( $9) = AppendAssign( BLOCK_INSTR( $9),
                                                MakeAssign( $7, NULL));
               $$ = MakeAssign( $4,
                                MakeAssign( MakeWhile( $5, $9),
                                            NULL));
               NODE_LINE( ASSIGN_INSTR( ASSIGN_NEXT( $$))) = $<cint>2;
             }
         ;


exprs: expr COMMA exprs { $$ = MakeExprs( $1, $3);   }
     | expr             { $$ = MakeExprs( $1, NULL); }
     ;

exprsNOar: exprNOar COMMA exprsNOar { $$ = MakeExprs( $1, $3);   }
         | exprNOar                 { $$ = MakeExprs( $1, NULL); }
         ;


exprORdot: expr  %prec GENERATOR { $$ = $1;   }
         | DOT                   { $$ = NULL; }
         ;


exprNOar: expr_main { $$ = $1; }
        | expr_br   { $$ = $1; }
        | expr_sel  { $$ = $1; }
        | expr_expr { $$ = $1; }
        | expr_sign { $$ = $1; }
        | expr_ap   { $$ = $1; }
        ;

/* 'expr_expr' without brackets is superfluous here! */
exprNObr: expr_main     { $$ = $1; }
        | expr_selNObr  { $$ = $1; }
        | expr_sign     { $$ = $1; }
        | expr_ap       { $$ = $1; }
        | expr_ar       { $$ = $1; }
        ;

/* see 'expr_sel' */
expr_selNObr: exprNObr SQBR_L expr SQBR_R
              { $$ = MAKE_BIN_PRF( F_sel, $3, $1);
                NODE_LINE( $$) = NODE_LINE( $1);
              }
            | exprNObr SQBR_L expr COMMA exprs SQBR_R
              { $$ = MAKE_BIN_PRF( F_sel,
                                   MakeArray( MakeExprs( $3, $5)),
                                   $1);
                NODE_LINE( $$) = NODE_LINE( $1);
              }
            ;

expr: expr_main { $$ = $1; }
    | expr_br   { $$ = $1; }
    | expr_sel  { $$ = $1; }
    | expr_expr { $$ = $1; }
    | expr_sign { $$ = $1; }
    | expr_ap   { $$ = $1; }
    | expr_ar   { $$ = $1; }
    ;

expr_main: id                          { $$ = MakeId( $1, NULL, ST_regular); }
         | id COLON id                 { $$ = MakeId( $3, $1, ST_regular); }
         | NUM                         { $$ = MakeNum( $1); }
         | FLOAT                       { $$ = MakeFloat( $1); }
         | DOUBLE                      { $$ = MakeDouble( $1); }
         | CHAR                        { $$ = MakeChar( $1); }
         | TRUETOKEN                   { $$ = MakeBool( 1); }
         | FALSETOKEN                  { $$ = MakeBool( 0); }
         | string                      { $$ = String2Array( $1); }
         | NOT expr
           { $$ = MakePrf( F_not, MakeExprs( $2, NULL));
           }
         | wlcomp_pragma_local
           NWITH { $<cint>$ = linenum; } BRACKET_L generator BRACKET_R
           wlassignblock withop
           { /*
              * the tricky part about this rule is that $8 (an N_Nwithop node)
              * carries the goal-expression of the With-Loop, i.e., the "N-expr"
              * node which belongs into the N_Ncode node!!!
              * The reason for this is that an exclusion of the goal expression
              * from the non-terminal withop would lead to a shift/reduce
              * conflict in that rule!
              */
             $$ = MakeNWith( $5, MakeNCode( $7, NWITHOP_EXPR( $8)), $8);
             NWITHOP_EXPR( $8) = NULL;
             NCODE_USED( NWITH_CODE( $$))++;
             NODE_LINE( $$)= $<cint>3;
             NWITH_PRAGMA( $$) = $1;

             /*
              * Finally, we generate the link between the (only) partition
              * and the (only) code!
              */
             NPART_CODE( NWITH_PART( $$)) = NWITH_CODE( $$);
           }
         ;

expr_br: BRACKET_L expr BRACKET_R
         { $$ = $2;
         }
       | BRACKET_L COLON type BRACKET_R expr  %prec CAST
         { $$ = MakeCast( $5, $3);
         }
       ;

/* see 'expr_selNObr' */
expr_sel: expr SQBR_L expr SQBR_R
          { $$ = MAKE_BIN_PRF( F_sel, $3, $1);
            NODE_LINE( $$) = NODE_LINE( $1);
          }
        | expr SQBR_L expr COMMA exprs SQBR_R
          { $$ = MAKE_BIN_PRF( F_sel,
                               MakeArray( MakeExprs( $3, $5)),
                               $1);
            NODE_LINE( $$) = NODE_LINE( $1);
          }
        ;

expr_expr: expr AND expr     { $$ = MAKE_BIN_PRF( F_and, $1, $3); }
         | expr OR expr      { $$ = MAKE_BIN_PRF( F_or,  $1, $3); }
         | expr EQ expr      { $$ = MAKE_BIN_PRF( F_eq,  $1, $3); }
         | expr NEQ expr     { $$ = MAKE_BIN_PRF( F_neq, $1, $3); }
         | expr LE expr      { $$ = MAKE_BIN_PRF( F_le,  $1, $3); }
         | expr LT expr      { $$ = MAKE_BIN_PRF( F_lt,  $1, $3); }
         | expr GE expr      { $$ = MAKE_BIN_PRF( F_ge,  $1, $3); }
         | expr GT expr      { $$ = MAKE_BIN_PRF( F_gt,  $1, $3); }
         | expr PLUS expr    { $$ = MAKE_BIN_PRF( F_add, $1, $3); }
         | expr MINUS expr   { $$ = MAKE_BIN_PRF( F_sub, $1, $3); }
         | expr DIV expr     { $$ = MAKE_BIN_PRF( F_div, $1, $3); }
         | expr MUL expr     { $$ = MAKE_BIN_PRF( F_mul, $1, $3); }
         | expr PRF_MOD expr { $$ = MAKE_BIN_PRF( F_mod, $1, $3); }
         ;

/*
 * Open problems (+ analogous):
 *   - (: ... ) ...
 * causes a syntax error,
 *   - ( ... ) [ ... ]
 * is parsed wrongly as  (- ( ... )) [ ... ]
 */
expr_sign: MINUS exprNObr  %prec SIGN
           {
             switch (NODE_TYPE( $2)) {
               case N_num:
                 $$ = $2;
                 NUM_VAL( $$) = (0 - NUM_VAL( $$));
                 break;
               case N_float:
                 $$ = $2;
                 FLOAT_VAL( $$) = (0 - FLOAT_VAL( $$));
                 break;
               case N_double:
                 $$ = $2;
                 DOUBLE_VAL( $$) = (0 - DOUBLE_VAL( $$));
                 break;
               default:
                 /*
                  * Substitute unary minus by (0 - x).
                  *
                  * dkr:
                  * This might cause a type error if $2 has not the type 'int'
                  * !!!
                  */
                 $$ = MAKE_BIN_PRF( F_sub, MakeNum( 0), $2);
                 break;
             }
           }
         | PLUS exprNObr  %prec SIGN  { $$ = $2; }
         ;

expr_ar: SQBR_L { $<cint>$ = linenum; } exprsNOar SQBR_R
         { $$ = MakeArray( $3);
           NODE_LINE( $$) = $<cint>2;
         }
       | SQBR_L { $<cint>$ = linenum; } SQBR_R
         { $$ = MakeArray( NULL);
           NODE_LINE( $$) = $<cint>2;
         }
       ;       

expr_ap: id BRACKET_L { $<cint>$ = linenum; } opt_arguments BRACKET_R
         { $$ = MakeAp( $1, NULL, $4);
           NODE_LINE( $$) = $<cint>3;
         }
       | id COLON id BRACKET_L { $<cint>$ = linenum; } opt_arguments BRACKET_R
          { $$ = MakeAp( $3, $1, $6);
            NODE_LINE( $$) = $<cint>5;
          }
       | monop BRACKET_L { $<cint>$ = linenum; } opt_arguments BRACKET_R
         { 
           if (CountExprs( $4) == 1) {
             $$ = MakePrf( $1, $4);
           }
           else {
             $$ = MakeAp( StringCopy( prf_name_str[ $1]), NULL, $4);
           }
           NODE_LINE( $$) = $<cint>3;
         }
       | id COLON monop BRACKET_L { $<cint>$ = linenum; } opt_arguments
         BRACKET_R
         { 
           $$ = MakeAp( StringCopy( prf_name_str[ $3]), $1, $6);
           NODE_LINE( $$) = $<cint>5;
         }
       | binop BRACKET_L { $<cint>$ = linenum; } opt_arguments BRACKET_R
         {
           int cnt = CountExprs( $4);

           if (cnt == 1) {
             $$ = MakePrf( $1, MakeExprs( MakeNum( 0), $4));
           }
           else if (cnt == 2) {
             $$ = MakePrf( $1, $4);
           }
           else {
             $$ = MakeAp( StringCopy( prf_name_str[ $1]), NULL, $4);
           }
           NODE_LINE( $$) = $<cint>3;
         }
       | id COLON binop BRACKET_L { $<cint>$ = linenum; } opt_arguments
         BRACKET_R
         { 
           $$ = MakeAp( StringCopy( prf_name_str[ $3]), $1, $6);
           NODE_LINE( $$) = $<cint>5;
         }
       | triop BRACKET_L { $<cint>$ = linenum; } opt_arguments BRACKET_R
         { 
           if (CountExprs( $4) == 3) {
             $$ = MakePrf( $1, $4);
           }
           else {
             $$ = MakeAp( StringCopy( prf_name_str[ $1]), NULL, $4);
           }
           NODE_LINE( $$) = $<cint>3;
         }
       | id COLON triop BRACKET_L { $<cint>$ = linenum; } opt_arguments
         BRACKET_R
         { 
           $$ = MakeAp( StringCopy( prf_name_str[ $3]), $1, $6);
           NODE_LINE( $$) = $<cint>5;
         }
       | ALL BRACKET_L { $<cint>$ = linenum; } opt_arguments BRACKET_R
         { 
           $$ = MakeAp( StringCopy( "all"), NULL, $4);
           NODE_LINE( $$) = $<cint>3;
         }
       | id COLON ALL BRACKET_L { $<cint>$ = linenum; } opt_arguments BRACKET_R
         { 
           $$ = MakeAp( StringCopy( "all"), $1, $6);
           NODE_LINE( $$) = $<cint>5;
         }
       ;

opt_arguments: exprs { $$ = $1; }
             |       { $$ = NULL; }
             ;


generator: exprORdot genop genidx genop exprORdot steps width
           { 
             $$ = MakeNPart( $3,
                             MakeNGenerator( $1, $5, $2,  $4, $6, $7),
                             NULL);
           }
         ;

steps: /* empty */ { $$ = NULL; }
     | STEP expr   { $$ = $2; }
     ;

width: /* empty */ { $$ = NULL; }
     | WIDTH expr  { $$ = $2; }
     ;

genidx: id LET SQBR_L ids SQBR_R 
        { 
          $$ = MakeNWithid( MakeIds( $1, NULL, ST_regular), $4); 
        }
      | id { $$ = MakeNWithid( MakeIds( $1, NULL, ST_regular), NULL); }
      | SQBR_L ids SQBR_R { $$ = MakeNWithid( NULL, $2); }
      ;

genop:   LT { $$ = F_lt; }
       | LE { $$ = F_le; }
       ;

withop: GENARRAY BRACKET_L expr COMMA expr BRACKET_R
        { $$ = MakeNWithOp( WO_genarray, $3);
          NWITHOP_EXPR( $$) = $5;
        }
      | MODARRAY BRACKET_L expr COMMA id COMMA expr BRACKET_R
        { $$ = MakeNWithOp( WO_modarray, $3);
          NWITHOP_EXPR( $$) = $7;
        }
      | FOLD BRACKET_L foldop COMMA expr BRACKET_R
        { $$ = MakeNWithOp( WO_foldprf, NULL);
          NWITHOP_PRF( $$) = $3;
          NWITHOP_EXPR( $$) = $5;
        }
      | FOLD BRACKET_L foldop COMMA expr COMMA expr BRACKET_R
        { $$ = MakeNWithOp( WO_foldprf, $5);
          NWITHOP_PRF( $$) = $3;
          NWITHOP_EXPR( $$) = $7;
        }
      | FOLD BRACKET_L id COMMA expr COMMA expr BRACKET_R
        { $$ = MakeNWithOp( WO_foldfun, $5);
          NWITHOP_FUN( $$) = $3;
          NWITHOP_EXPR( $$) = $7;
        }
      | FOLD BRACKET_L id COLON id COMMA expr COMMA expr BRACKET_R
        { $$ = MakeNWithOp( WO_foldfun, $7);
          NWITHOP_FUN( $$) = $5;
          NWITHOP_MOD( $$) = $3;
          NWITHOP_EXPR( $$) = $9;
        }
      ;


foldop: PLUS    { $$ = F_add; }
      | MUL     { $$ = F_mul; }
      | PRF_MIN { $$ = F_min; }
      | PRF_MAX { $$ = F_max; }
      | AND     { $$ = F_and; }
      | OR      { $$ = F_or; }
      ;

monop: ABS   { $$ = F_abs;   }
     | DIM   { $$ = F_dim;   }
     | SHAPE { $$ = F_shape; }
     | TOI   { $$ = F_toi;   }
     | TOF   { $$ = F_tof;   }
     | TOD   { $$ = F_tod;   }
     ;

binop: SEL      { $$ = F_sel;      }
     | TAKE     { $$ = F_take;     }
     | DROP     { $$ = F_drop;     }
     | RESHAPE  { $$ = F_reshape;  }
     | GENARRAY { $$ = F_genarray; }
     | PRF_MIN  { $$ = F_min;      }
     | PRF_MAX  { $$ = F_max;      }
     | PLUS     { $$ = F_add;      }
     | MINUS    { $$ = F_sub;      }
     | MUL      { $$ = F_mul;      }
     | DIV      { $$ = F_div;      }
     | PRF_MOD  { $$ = F_mod;      }
     | EQ       { $$ = F_eq;       }
     | NEQ      { $$ = F_neq;      }
     | LT       { $$ = F_lt;       }
     | LE       { $$ = F_le;       }
     | GT       { $$ = F_gt;       }
     | GE       { $$ = F_ge;       }
     | AND      { $$ = F_and;      }
     | OR       { $$ = F_or;       }
     ;

triop: ROTATE   { $$ = F_rotate;   }
     | CAT      { $$ = F_cat;      }
     | MODARRAY { $$ = F_modarray; }
     ;


ids: id COMMA ids 
     { $$ = MakeIds( $1, NULL, ST_regular);
       IDS_NEXT( $$) = $3;
     }
   | id 
     { $$ = MakeIds( $1, NULL, ST_regular);
     }
   ;

funids: fun_name COMMA funids 
        { $$ = MakeIds( $1, NULL, ST_regular);
          IDS_NEXT( $$) = $3;
        }
      | fun_name 
        { $$ = MakeIds( $1, NULL, ST_regular);
        }
      ;


nums: NUM COMMA nums { $$ = MakeNums( $1, $3);   }
    | NUM            { $$ = MakeNums( $1, NULL); }
    ; 

dots: DOT COMMA dots { $$ = $3 - 1;               }
    | DOT            { $$ = KNOWN_DIM_OFFSET - 1; }
    ;


returntypes: TYPE_VOID { $$ = MakeTypes1( T_void); }
           | types     { $$ = $1;                  }
           ;

types: type COMMA types
       { $$ = $1;
         TYPES_NEXT( $$) = $3; 
       }
     | type { $$ = $1; }
     ;

varreturntypes: TYPE_VOID { $$ = MakeTypes1( T_void); }
              | vartypes  { $$ = $1;                  }
              ;

vartypes: type COMMA vartypes 
          { $$ = $1;
            TYPES_NEXT( $$) = $3;
          }
        | type { $$ = $1; }
        | TYPE_DOTS
          { if ((F_extmoddec != file_kind) &&
                (F_extclassdec != file_kind) &&
                (F_sib != file_kind)) {
              strcpy( yytext, "...");
              yyerror( "syntax error");
            }
            else {
              $$ = MakeTypes1( T_dots);
            }
          }
        ;

typeNOudt_arr: localtype_main { $$ = $1; }
             | id COLON localtype
               { $$ = $3;
                 TYPES_MOD( $$) = $1;
               }
             ;

type: localtype { $$ = $1; }
      | id COLON localtype
        { $$ = $3;
          TYPES_MOD( $$) = $1;
        }
      ;

localtype: localtype_main     { $$ = $1; }
         | local_type_udt_arr { $$ = $1; }
         ;

localtype_main: simpletype { $$ = $1; }
              | simpletype_main SQBR_L nums SQBR_R  
                { $$ = GenComplexType( $1, $3); }
              | simpletype_main SQBR_L SQBR_R  
                { $$ = $1;
                  TYPES_DIM( $$) = 0;
                }
              | simpletype_main SQBR_L PLUS SQBR_R  
                { $$ = $1;
                  TYPES_DIM( $$) = UNKNOWN_SHAPE;
                }
              | simpletype_main  SQBR_L MUL SQBR_R  
                { $$ = $1;
                  TYPES_DIM( $$) = ARRAY_OR_SCALAR;
                }
              | simpletype_main SQBR_L dots SQBR_R
                { $$ = $1;
                  TYPES_DIM( $$) = $3;
                }
              | id SQBR_L SQBR_R
                { $$ = MakeTypes1( T_user);
                  TYPES_NAME( $$) = $1;
                  TYPES_DIM( $$) = 0;
                }
              | id SQBR_L PLUS SQBR_R
                { $$ = MakeTypes1( T_user);
                  TYPES_NAME( $$) = $1;
                  TYPES_DIM( $$) = UNKNOWN_SHAPE;
                }
              | id SQBR_L MUL SQBR_R
                { $$ = MakeTypes1( T_user);
                  TYPES_NAME( $$) = $1;
                  TYPES_DIM( $$) = ARRAY_OR_SCALAR;
                }
              | id SQBR_L dots SQBR_R
                { $$ = MakeTypes1( T_user);
                  TYPES_NAME( $$) = $1;
                  TYPES_DIM( $$) = $3;
                }
              ;

local_type_udt_arr: id SQBR_L nums SQBR_R
                    { $$ = MakeTypes1( T_user);
                      TYPES_NAME( $$) = $1;
                      $$ = GenComplexType( $$, $3);
                    }
                  ;

simpletype: simpletype_main { $$ = $1; }
          | id 
            { $$ = MakeTypes1( T_user);
              TYPES_NAME( $$) = $1;
            }
          ;

simpletype_main: TYPE_INT   { $$ = MakeTypes1( T_int);    }
               | TYPE_FLOAT { $$ = MakeTypes1( T_float);  }
               | TYPE_BOOL  { $$ = MakeTypes1( T_bool);   }
               | TYPE_CHAR  { $$ = MakeTypes1( T_char);   }
               | TYPE_DBL   { $$ = MakeTypes1( T_double); }
               ;


string: STR
        {
          $$ = $1;
        }
      | STR string
        {
          $$ = StringConcat( $1, $2);
          Free( $1);
          Free( $2);
        }
      ;


/* BRUSH END */


/*
 *********************************************************************
 *
 *  rules for SAC Information Blocks (SIB)
 *
 *********************************************************************
 */


sib: sibheader siblinkwith sibtypes
       {
         $$ = $3;
         SIB_LINKSTYLE( $$) = $1;
         SIB_LINKWITH( $$) = $2;

         DBUG_PRINT( "PARSE_SIB", ("%s"F_PTR,
                               mdb_nodetype[ NODE_TYPE( $$)],
                               $$));
       }
   ;

sibheader: LT MODIMP id GT
             {
               mod_name = $3;
               file_kind = F_sib;
               sib_imported_status = ST_imported_mod;
               
               $$ = 0;
             }
         | LT MODIMP id COLON ALL GT
             {
               mod_name = $3;
               file_kind = F_sib;
               sib_imported_status = ST_imported_mod;

               $$ = 1;
             }
         | LT CLASSIMP id GT
             {
               mod_name = $3;
               file_kind = F_sib;
               sib_imported_status = ST_imported_class;

               $$ = 0;
             }
         | LT CLASSIMP id COLON ALL GT
             {
               mod_name = $3;
               file_kind = F_sib;
               sib_imported_status = ST_imported_class;

               $$ = 1;
             }
         ;

siblinkwith: LINKWITH siblinklist { $$ = $2; }
             | { $$ = NULL; }
             ;

siblinklist: siblinkliststatus string sibsublinklist COMMA siblinklist
             {
               $$ = MakeDeps( $2, NULL, NULL, $1, LOC_stdlib, $3, $5);
             }
           | siblinkliststatus string sibsublinklist 
             {
               $$ = MakeDeps( $2, NULL, NULL, $1, LOC_stdlib, $3, NULL);
             }
           ;

siblinkliststatus: EXTERN 
                   {
                     $$ = ST_external;
                   }
                 | LINKWITH
                   {
                     $$ = ST_system;
                   }
                 |
                   {
                     $$ = ST_sac;
                   }
                 ;

sibsublinklist: BRACE_L siblinklist BRACE_R
                {
                  $$ = $2;
                }
              | 
                {
                  $$ = NULL;
                }
              ;


sibtypes: sibtype sibtypes
            {
              $$ = $2;
              TYPEDEF_NEXT( $1) = SIB_TYPES( $2);
              SIB_TYPES( $$) = $1;
            }
        | sibobjs
            {
              $$ = $1;
            } 
        ;

sibtype: sibevclass TYPEDEF type id SEMIC sibpragmas
           {
             $$ = MakeTypedef( $4, NULL, $3, $1, NULL);
             TYPEDEF_STATUS( $$) = sib_imported_status;
             TYPEDEF_PRAGMA( $$) = $6;

            DBUG_PRINT("PARSE_SIB",
                       ("%s:"F_PTR","F_PTR", Id: %s",
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$, TYPEDEF_TYPE( $$), ItemName( $$)));
           }
       | sibevclass TYPEDEF type id COLON id SEMIC sibpragmas
           {
             $$  = MakeTypedef( $6, $4, $3, $1, NULL);
             TYPEDEF_STATUS( $$) = sib_imported_status;
             TYPEDEF_PRAGMA( $$) = $8;

            DBUG_PRINT("PARSE_SIB",
                       ("%s:"F_PTR","F_PTR", Id: class %s",
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$, TYPEDEF_TYPE( $$), ItemName( $$)));
           }
       | sibevclass TYPEDEF IMPLICIT id SEMIC sibpragmas
           {
             $$ = MakeTypedef( $4, NULL,
                               MakeTypes1( T_hidden),
                               $1, NULL);
             TYPEDEF_STATUS( $$) = sib_imported_status;
             TYPEDEF_PRAGMA( $$) = $6;

            DBUG_PRINT("PARSE_SIB",
                       ("%s:"F_PTR","F_PTR", Id: %s",
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$, TYPEDEF_TYPE( $$), ItemName( $$)));
           }
       | sibevclass TYPEDEF IMPLICIT id COLON id SEMIC sibpragmas
           {
             $$ = MakeTypedef( $6, $4,
                               MakeTypes1( T_hidden),
                               $1, NULL);
             TYPEDEF_STATUS( $$) = sib_imported_status;
             TYPEDEF_PRAGMA( $$) = $8;

            DBUG_PRINT("PARSE_SIB",
                       ("%s:"F_PTR","F_PTR", Id: class %s",
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$, TYPEDEF_TYPE( $$), ItemName( $$)));
           }
        ;

sibevclass: CLASSTYPE
              { $$ = ST_unique; }
          |   { $$ = ST_regular; }

sibobjs: sibobj sibobjs
         {
           $$ = $2;
           OBJDEF_NEXT( $1) = SIB_OBJS( $2);
           SIB_OBJS( $2) = $1;
         }
       | sibfuns
         {
           $$ = $1;
         } 
       ;

sibobj: OBJDEF type id SEMIC sibpragmas
          {
            $$ = MakeObjdef( $3, NULL, $2, NULL, NULL);
            OBJDEF_PRAGMA( $$) = $5;
            OBJDEF_STATUS( $$) = sib_imported_status;
            
            DBUG_PRINT("PARSE_SIB",
                       ("%s:"F_PTR","F_PTR", Id: class %s",
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$, OBJDEF_TYPE( $$), ItemName( $$)));
          }
      | OBJDEF type id COLON id SEMIC sibpragmas
          {
            $$ = MakeObjdef( $5, $3, $2, NULL, NULL);
            OBJDEF_PRAGMA( $$) = $7;
            OBJDEF_STATUS( $$) = sib_imported_status;
            
            DBUG_PRINT("PARSE_SIB",
                       ("%s:"F_PTR","F_PTR", Id: class %s",
                        mdb_nodetype[ NODE_TYPE( $$)],
                        $$, OBJDEF_TYPE( $$), ItemName( $$)));
          }
      ;

sibfuns: sibfun sibfuns
         {
           $$ = $2;
           FUNDEF_NEXT( $1) = SIB_FUNS( $2);
           SIB_FUNS( $$) = $1;
         }
       | 
         {
           $$ = MakeSib( mod_name, 0, NULL, NULL, NULL, NULL);
         }
       ;

sibfun: sibevmarker varreturntypes fun_name BRACKET_L sibarglist
        BRACKET_R { $<cint>$ = linenum; } sibfunbody sibpragmas
          {
            $$ = MakeFundef( $3, NULL, $2, $5, $8, NULL);
            NODE_LINE( $$) = $<cint>7;
            switch ($1) {
              case 0:
                FUNDEF_STATUS( $$) = sib_imported_status;
                FUNDEF_INLINE( $$) = FALSE;
                break;
              case 1:
                FUNDEF_STATUS( $$) = sib_imported_status;
                FUNDEF_INLINE( $$) = TRUE;
                break;
              case 2:
                FUNDEF_STATUS( $$) = ST_classfun;
                FUNDEF_INLINE( $$) = FALSE;
                break;
            }
            FUNDEF_PRAGMA( $$) = $9;

           DBUG_PRINT("PARSE_SIB",("%s"F_PTR"SibFun %s",
                               mdb_nodetype[ NODE_TYPE( $$)],
                               $$, ItemName( $$)));
          }
      | sibevmarker varreturntypes id COLON fun_name BRACKET_L sibarglist
        BRACKET_R { $<cint>$ = linenum; } sibfunbody sibpragmas
          {
            $$ = MakeFundef( $5, $3, $2, $7, $10, NULL);
            NODE_LINE( $$) = $<cint>9;
            switch ($1) {
              case 0:
                FUNDEF_STATUS( $$) = sib_imported_status;
                FUNDEF_INLINE( $$) = FALSE;
                break;
              case 1:
                FUNDEF_STATUS( $$) = sib_imported_status;
                FUNDEF_INLINE( $$) = TRUE;
                break;
              case 2:
                FUNDEF_STATUS( $$) = ST_classfun;
                FUNDEF_INLINE( $$) = FALSE;
                break;
            }
            FUNDEF_PRAGMA( $$) = $11;

            DBUG_PRINT("PARSE_SIB",("%s"F_PTR"SibFun %s",
                                mdb_nodetype[ NODE_TYPE( $$)],
                                $$, ItemName( $$)));
          }
        ;

sibevmarker: INLINE   
             { $$ = 1; } 
           | CLASSTYPE { $$ = 2; }
           | { $$ = 0; }
           ;

sibarglist: sibargs
            { $$ = $1; }
          | { $$ = NULL; }
          ;

sibargs: sibarg COMMA sibargs
         {
           $$ = $1;
           ARG_NEXT( $$) = $3;
         }
       | sibarg
         {
           $$ = $1;
         }
       ;

sibarg: type sibreference sibparam
        {
          $$ = MakeArg( $3, $1, ST_regular, $2, NULL);

          DBUG_PRINT("PARSE_SIB",
                     ("%s: "F_PTR", Id: %s, Attrib: %d, Status: %d  ",
                      mdb_nodetype[ NODE_TYPE( $$)],
                      $$, 
                      STR_OR_EMPTY( ARG_NAME( $$)),
                                    ARG_ATTRIB( $$), ARG_STATUS( $$)));
        }
      | TYPE_DOTS
        {
          $$ = MakeArg( NULL, MakeTypes1( T_dots),
                        ST_regular, ST_regular, NULL); 

          DBUG_PRINT( "PARSE_SIB",
                      ("%s: "F_PTR", ... , Attrib: %d, Status: %d  ",
                       mdb_nodetype[ NODE_TYPE( $$)],
                       $$, ARG_ATTRIB( $$), ARG_STATUS( $$)));
        }
      ;

sibparam: id { $$ = $1; }
        |    { $$ = NULL; }
        ;

sibreference: BRACKET_L AMPERS BRACKET_R
              {
                $$ = ST_readonly_reference;
              }
            | AMPERS
              {
                $$ = ST_reference;
              }
            |
              {
                $$ = ST_regular;
              }
            ;

sibfunbody: exprblock
            {
              $$ = $1;
            }
          | SEMIC
            {
              $$ = NULL;
            }
          ;


sibpragmas: sibpragmalist
            {
              $$ = store_pragma;
              store_pragma = NULL;
            }
          |
            {
              $$ = NULL;
            }
          ;

sibpragmalist: sibpragmalist sibpragma
             | sibpragma
             ;

sibpragma: pragma
         | PRAGMA TYPES modnames
           {
             if (store_pragma == NULL) {
               store_pragma = MakePragma();
             }
             PRAGMA_NEEDTYPES( store_pragma) = $3;
           }
         | PRAGMA FUNS sibfunlist
           {
             if (store_pragma == NULL) {
               store_pragma = MakePragma();
             }
             PRAGMA_NEEDFUNS( store_pragma) = $3;
           }
         | PRAGMA EXTERN id
           {
             if (store_pragma == NULL) {
               store_pragma = MakePragma();
             }
             PRAGMA_LINKMOD( store_pragma) = $3; 
           }
         ;


sibfunlist: sibfunlistentry COMMA sibfunlist
            {
              $$ = $1;
              FUNDEF_NEXT( $$) = $3;
            }
          | sibfunlistentry
            {
              $$ = $1;
            }
          ;

sibfunlistentry: id BRACKET_L sibarglist BRACKET_R
                 {
                   $$ = MakeFundef( $1, NULL,
                                    MakeTypes1( T_unknown),
                                    $3, NULL, NULL);
                   FUNDEF_STATUS( $$) = sib_imported_status;
                   
                   DBUG_PRINT("PARSE_SIB",("%s"F_PTR"SibNeedFun %s",
                                       mdb_nodetype[ NODE_TYPE( $$)],
                                       $$, ItemName( $$)));
                }
               | id COLON fun_name BRACKET_L sibarglist BRACKET_R
                 {
                   $$ = MakeFundef( $3, $1,
                                    MakeTypes1( T_unknown),
                                    $5, NULL, NULL);

                   FUNDEF_STATUS( $$) = sib_imported_status;
                   
                   DBUG_PRINT("PARSE_SIB",("%s"F_PTR"SibNeedFun %s",
                                       mdb_nodetype[ NODE_TYPE( $$)],
                                       $$, ItemName( $$)));
                 }
               ;

     

/*
 *********************************************************************
 *
 *  rules for sac2crc files
 *
 *********************************************************************
 */


targets: TARGET ID COLON inherits resources targets
         {
           $$ = RSCMakeTargetListEntry( $2, $4, $5, $6);
         }
       | /* empty */
         {
           $$ = NULL;
         }
       ;

inherits: COLON ID COLON inherits
          {
            $$ = MakeIds( $2, NULL, ST_regular);
            IDS_NEXT( $$) = $4;
          }
        | /* empty */
          {
            $$ = NULL;
          } 
        ;

resources: ID COLON LET string resources
           {
             $$ = RSCMakeResourceListEntry( $1, $4, 0, 0, $5);
           }
         | ID ADDON string resources
           {
             $$ = RSCMakeResourceListEntry( $1, $3, 0, 1, $4);
           }
         | ID COLON LET OPTION resources
           {
             $$ = RSCMakeResourceListEntry( $1, $4, 0, 0, $5);
           }
         | ID ADDON OPTION resources
           {
             $$ = RSCMakeResourceListEntry( $1, $3, 0, 1, $4);
           }
         | ID COLON LET ID resources
           {
             $$ = RSCMakeResourceListEntry( $1, $4, 0, 0, $5);
           }
         | ID ADDON ID resources
           {
             $$ = RSCMakeResourceListEntry( $1, $3, 0, 1, $4);
           }
         | ID COLON LET PRIVATEID resources
           {
             $$ = RSCMakeResourceListEntry( $1, $4, 0, 0, $5);
           }
         | ID ADDON PRIVATEID resources
           {
             $$ = RSCMakeResourceListEntry( $1, $3, 0, 1, $4);
           }
         | ID COLON LET NUM resources
           {
             $$ = RSCMakeResourceListEntry( $1, NULL, $4, 0, $5);
           }
         | ID ADDON NUM resources
           {
             $$ = RSCMakeResourceListEntry( $1, NULL, $3, 1, $4);
           }
         | /* empty */
           {
             $$ = NULL;
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
  if (strlen( linebuf_ptr) > size_of_output) {
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
        new_exprs=MakeExprs(MakeChar('\n'), new_exprs);
        i-=1;
        break;
      case 't':
        new_exprs=MakeExprs(MakeChar('\t'), new_exprs);
        i-=1;
        break;
      case 'v':
        new_exprs=MakeExprs(MakeChar('\v'), new_exprs);
        i-=1;
        break;
      case 'b':
        new_exprs=MakeExprs(MakeChar('\b'), new_exprs);
        i-=1;
        break;
      case 'r':
        new_exprs=MakeExprs(MakeChar('\r'), new_exprs);
        i-=1;
        break;
      case 'f':
        new_exprs=MakeExprs(MakeChar('\f'), new_exprs);
        i-=1;
        break;
      case 'a':
        new_exprs=MakeExprs(MakeChar('\a'), new_exprs);
        i-=1;
        break;
      case '"':
        new_exprs=MakeExprs(MakeChar('"'), new_exprs);
        i-=1;
        break;
      default:
        new_exprs=MakeExprs(MakeChar(str[i]), new_exprs);
        break;
      }
    }
    else {
      new_exprs=MakeExprs(MakeChar(str[i]), new_exprs);
    }
    
    cnt+=1;
  }

  len_exprs = MakeExprs( MakeNum( cnt), NULL);
  array = MakeArray( new_exprs);

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
 *   types *GenComplexType( types *types, nums *numsp)
 *
 * Description:
 *   
 *
 ******************************************************************************/

static
types *GenComplexType( types *types, nums *numsp)
{
  int *destptr;
  nums *tmp;

  DBUG_ENTER( "GenComplexType");

  TYPES_SHPSEG( types) = MakeShpseg(NULL);
  destptr = TYPES_SHPSEG( types)->shp;
  do {
    (TYPES_DIM( types))++;
    *destptr++ = numsp->num;
    DBUG_PRINT("PARSE",("shape-element: %d",numsp->num));
    tmp = numsp;
    numsp = numsp->next;
    Free( tmp);
  }
  while (numsp != NULL);

  DBUG_RETURN( types);
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
