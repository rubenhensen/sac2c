%{

/*
 *
 * $Log$
 * Revision 1.168  1999/01/19 16:17:08  cg
 * added types like int[?] which match both scalars and arrays;
 * removed special handling of psi and modarray when applied to
 * an integer constant instead of an index vector.
 *
 * Revision 1.167  1999/01/07 10:47:46  cg
 * User-defined overloadings of primitive functions may now
 * have a different number of arguments than the primitive ones.
 * Key word 'all' may now be used as function name in function
 * applications. This is required for standard module Array.
 *
 * Revision 1.166  1998/12/02 16:25:33  cg
 * Real function names for overloaded primitive operations are now
 * generated in the yacc-file rather than in the lex file.
 * Conversion operations itod, dtof, etc. removed.
 *
 * Revision 1.165  1998/12/01 09:43:02  cg
 * All binary operators may now also be applied in the form of
 * a standard function call.
 *
 * Revision 1.164  1998/11/08 14:24:43  dkr
 * an empty assignment block of a with-loop is represented by a
 * N_block node containing a N_empty node.
 *
 * Revision 1.163  1998/10/30 09:53:12  cg
 * key word 'all' is now allowed as function name,
 * required by array library.
 * Syntactic position of fold operator limited to those internal
 * functions/operators which are "really" commutative and associative.
 *
 * Revision 1.162  1998/07/20 17:49:07  sbs
 * if (store_wlcomp_pragma_global != NULL) {
 * included in wlcomp_pragma_local rule 8-))))
 *
 * Revision 1.161  1998/07/20 15:54:23  sbs
 * wlcomp_pragma_local now always returns a pragma-node8-)
 *
 * Revision 1.160  1998/07/07 13:41:49  cg
 * improved the resource management by implementing multiple inheritence
 * between targets
 *
 * Revision 1.159  1998/06/03 14:34:20  cg
 * special purpose module name "__MAIN" renamed to "_MAIN"
 *
 * Revision 1.158  1998/04/30 12:22:45  srs
 * set temporary son NWITHOP_EXPR to NULL after usage
 *
 * Revision 1.157  1998/04/09 23:32:28  dkr
 * renamed PRAGMA_WLCOMP to PRAGMA_WLCOMP_APS
 *
 * Revision 1.156  1998/04/09 22:51:23  dkr
 * added parsing of wlcomp-pragmas
 *
 * Revision 1.155  1998/03/25 15:01:38  srs
 * added incrementation of usage of N_Ncode node in rule expr_main.NWITH...
 *
 * Revision 1.154  1998/03/24 10:55:59  srs
 * changed parameters of MakeNPart in rule Ngenerator
 *
 * Revision 1.152  1998/03/17 12:19:52  cg
 * Now, character arrays defined as strings keep the original string
 * throughout the compilation process. This string is reused when
 * generating C code to initialize the constant character array.
 *
 * Revision 1.151  1998/03/15 11:01:51  srs
 * fixed bug in letassign-rule. Abbreviation id[expr] of modarray did
 * not duplicate string so that the flatten phase freed a still used
 * name.
 *
 * Revision 1.150  1998/03/04 16:25:23  cg
 * Now, adjacent strings are concatenated to a single one.
 *
 * Revision 1.149  1998/03/02 19:47:55  srs
 * removed now unused rule 'unaryop' from %type declaration
 *
 * Revision 1.148  1998/03/02 13:54:47  srs
 * the parser flattens the operators ++ and -- to the equivalent
 * binary functions.
 *
 * Revision 1.147  1998/02/27 16:30:29  cg
 * added parsing rules for sac2crc files
 * bug fixed in parsing primitive function NOT
 * bug fixed in parsing for-loops
 *
 * Revision 1.146  1998/02/13 12:48:45  srs
 * extended generator syntax Ngenidx
 *
 * Revision 1.145  1998/02/09 16:06:28  srs
 * adjust Ngenidx to new MakeNWithid syntax
 *
 * Revision 1.144  1998/02/09 15:40:33  sbs
 * forced check in 8-(((
 * not yet cleaned up!
 *
 * Revision 1.143  1997/12/10 14:19:32  sbs
 * SOURCE-CODE-BRUSHING   STARTED !
 * the area brushed is marked by
 * BRUSH BEGIN and BRUSH END
 * started the elimination of GENTREE-DBUG-output
 * eliminated early node-creations for linenum preservation:
 * instead linenum is pushed by $<cint>$=linenum and
 * the result's NODE_LINE is updated accordingly!
 * the usage of Make... functions is forced!
 *
 * Revision 1.142  1997/11/25 12:22:27  sbs
 * if then else ala C
 * and a[1] now is recognized as a[[1]]!
 *
 * Revision 1.141  1997/11/24 18:06:40  sbs
 * new with loop syntax added AND (!!!!)
 * ALL SHIFT/REDUCE ERRORS ELIMINATED !!!!!!!!
 * Furthermore: all ifndef NEWTREE's eliminated
 * => has to linked into a NEWTREE only environment 8-))
 *
 * Revision 1.140  1997/11/10 14:22:46  dkr
 * removed a bug with A[.] -> psi(., A)
 *
 * Revision 1.139  1997/11/07 12:31:23  srs
 * changed another nnode part
 *
 * Revision 1.138  1997/11/04 11:25:18  srs
 * fixed little bug with NEWTREE
 *
 * Revision 1.137  1997/11/04 09:58:16  srs
 * NEWTREE: nnode is ignored
 *
 * Revision 1.136  1997/10/29 14:31:41  srs
 * free -> FREE
 *
 * Revision 1.135  1997/10/13 21:02:57  dkr
 * min() and max() are now foldops.
 *
 * Revision 1.134  1997/10/07 14:10:15  srs
 * new prfs PRF_MIN, PRF_MAX
 *
 * Revision 1.133  1997/10/03 17:57:17  dkr
 * added prf abs()
 *
 * Revision 1.131  1997/10/01 12:32:31  dkr
 * added % and %=
 *
 * Revision 1.128  1997/05/05 11:53:18  cg
 * SIB syntax slightly modified
 *
 * Revision 1.127  1997/05/05  07:46:09  cg
 * SIB limit removed
 *
 * Revision 1.126  1997/04/28  12:00:25  cg
 * SIB syntax slightly changed:
 * key word classtype used instead of Class.
 *
 * Revision 1.125  1997/04/25  13:22:19  sbs
 * sibarg: CHECK_NULL inserted
 *
 * Revision 1.124  1997/04/25  12:13:52  sbs
 * malloc replaced by Malloc
 *
 * Revision 1.123  1997/04/25  08:53:07  sbs
 * second arg of MakeType changed from NULL to 0
 *
 * Revision 1.122  1997/03/19  15:31:08  cg
 * Now, module/class implementations without any functions are supported
 *
 * Revision 1.121  1997/03/19  13:45:17  cg
 * new pragma linkwith added in module/class declarations as well as
 * special version for sib files
 *
 * Revision 1.120  1996/09/11  06:18:31  cg
 * Added a short cut notation for applications of modarray:
 * A[i]=t is equivalent to A=modarray(A,i,t)
 *
 * Revision 1.119  1996/09/09  10:06:47  cg
 * new primitive functions toi, tof, tod, etc. may now be overloaded
 * by library functions.
 *
 * Revision 1.118  1996/08/01  08:54:03  cg
 * Bug fixed: module name is now initialized when parsing N_ap nodes
 *
 * Revision 1.117  1996/04/02  19:37:26  cg
 * reimplemented handling of string constants
 *
 * Revision 1.116  1996/04/02  14:40:52  cg
 * bug fixed in parsing function applications
 *
 * Revision 1.115  1996/04/02  14:26:49  hw
 * changed DBUG_PRINT in rule "apl"
 *
 * Revision 1.114  1996/04/02  13:48:02  cg
 * scanparse converted to new strings:
 * primitive type string replaced by call of function to_string with
 * a char array as argument. to_string resides in module StringBase
 *
 * Revision 1.113  1996/02/21  15:05:12  cg
 * usage of function specifiers 'class' and 'inline' in SIBs corrected
 *
 * Revision 1.112  1996/02/21  09:17:42  cg
 * bug fixed in parsing SIBs: better use breaks in cases
 *
 * Revision 1.111  1996/02/08  18:05:46  hw
 * type-declaration with known dimenson, but unknown shape will be parsed
 * now e.g int[.,.]
 *
 * Revision 1.110  1996/01/26  15:29:57  cg
 * prefix 'class' introduced to functions to mark generic class conversion
 * functions
 *
 * Revision 1.109  1996/01/25  18:45:08  cg
 * added new class header containing definition of class type
 *
 * Revision 1.108  1996/01/23  09:57:03  cg
 * warnings about too long module names removed
 *
 * Revision 1.107  1996/01/22  17:30:38  cg
 * added new pragma initfun
 *
 * Revision 1.106  1996/01/21  13:57:30  cg
 * Macro GEN_NODE not longer used because it fails to initialize
 * the given structure
 *
 * Revision 1.105  1996/01/05  14:35:17  cg
 * The return statement is no longer referenced by the
 * corresponding genarray, modarray, foldfun, or foldprf node
 *
 * Revision 1.104  1996/01/05  12:33:43  cg
 * Now, we check that module/class names are not longer than
 * 12 characters.
 *
 * Revision 1.103  1996/01/02  17:46:48  cg
 * SIBS can now deal with external implicit types.
 *
 * Revision 1.102  1996/01/02  15:56:05  cg
 * The LINKMOD slot of fundef, typedef, and objdef nodes from module/class
 * declarations are now always set to the respective module/class name
 *
 * Revision 1.101  1995/12/29  10:38:14  cg
 * parsing of SIBs entirely re-implemented
 *
 * Revision 1.100  1995/12/21  15:04:43  cg
 * added primitive type char and all pragmas to parser
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "dbug.h"
#include "tree.h"
#include "typecheck.h"
#include "DupTree.h"        /* for use of DupTree */
#include "my_debug.h"
#include "internal_lib.h"   /* for use of StringCopy */
#include "Error.h"
#include "free.h"

#include "readsib.h"
#include "resource.h"


extern int linenum;
extern char yytext[];

int indent, i;

node *syntax_tree;
node *decl_tree;
node *sib_tree;


static char *mod_name="_MAIN";
static char *link_mod_name=NULL;
static node *store_pragma=NULL;
static node *store_wlcomp_pragma_global=NULL;


/* used to distinguish the different kinds of files  */
/* which are parsed with this single parser          */

static file_type file_kind = F_prog;


%}

%union {
         nodetype        nodetype;
         id              *id;
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

%token PARSE_PRG, PARSE_DEC, PARSE_SIB, PARSE_RC

%token BRACE_L, BRACE_R, BRACKET_L, BRACKET_R, SQBR_L, SQBR_R, COLON, SEMIC,
       COMMA, AMPERS, ASSIGN, DOT, QUESTION,
       INLINE, LET, TYPEDEF, CONSTDEF, OBJDEF, CLASSTYPE,
       INC, DEC, ADDON, SUBON, MULON, DIVON, MODON
       K_MAIN, RETURN, IF, ELSE, DO, WHILE, FOR, WITH, NWITH, FOLD,
       MODDEC, MODIMP, CLASSDEC, IMPORT, IMPLICIT, EXPLICIT, TYPES, FUNS,
       OWN, CONSTANTS, GLOBAL, OBJECTS, CLASSIMP,
       ARRAY,SC, TRUE, FALSE, EXTERN, C_KEYWORD,
       PRAGMA, LINKNAME, LINKSIGN, EFFECT, READONLY, REFCOUNTING,
       TOUCH, COPYFUN, FREEFUN, INITFUN, LINKWITH,
       WLCOMP, DEFAULT
       STEP, WIDTH, TARGET,
       AND, OR, EQ, NEQ, NOT, LE, LT, GE, GT, MUL, DIV, PRF_MOD, PLUS,
       TOI, TOF, TOD, ABS, PRF_MIN, PRF_MAX, ALL,
       RESHAPE, SHAPE, TAKE, DROP, DIM, ROTATE, CAT, PSI, GENARRAY, MODARRAY

%token <id> ID, STR, MINUS, PRIVATEID, OPTION

%token <types> TYPE_INT, TYPE_FLOAT, TYPE_BOOL, TYPE_UNS, TYPE_SHORT,
               TYPE_LONG, TYPE_CHAR, TYPE_DBL, TYPE_VOID, TYPE_DOTS
%token <cint> NUM
%token <cfloat> FLOAT
%token <cdbl> DOUBLE
%token <cchar> CHAR

%type <prf> foldop, Ngenop, monop, binop, triop
%type <nodetype> modclass
%type <cint> evextern, sibheader, sibevmarker, dots
%type <ids> ids, modnames, modname, inherits
%type <deps> linkwith, linklist, siblinkwith, siblinklist, sibsublinklist
%type <id> fun_name, prf_name, sibparam, id, string
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
             exprsNOar, exprNOdot, exprORdot, exprNOar, 
             expr, expr_main, expr_ap, expr_ar, expr_num, exprs,
             Ngenerator, Nsteps, Nwidth, Nwithop, Ngenidx,
             conexpr, generator,
             moddec, expdesc, expdesc2, expdesc3, expdesc4, fundecs, fundec,
             exptypes, exptype, objdecs, objdec, evimport, modheader,
             imptypes, imptype,import, imports, impdesc, impdesc2, impdesc3,
             impdesc4, foldfun, opt_arguments,
             sib, sibtypes, sibtype, sibfuns, sibfun, sibfunbody,
             sibobjs, sibobj, sibpragmas, sibarglist,
             sibargs, sibarg, sibfunlist, sibfunlistentry,
             wlcomp_args, wlcomp_expr, wlcomp_pragma_local, wlcomp_pragma_global
%type <target_list_t> targets
%type <resource_list_t> resources


%left OR
%left AND
%left EQ, NEQ
%left LE, LT, GE, GT
%left PLUS, MINUS
%left MUL, DIV, PRF_MOD
%left TAKE, DROP, RESHAPE
%left SQBR_L
%right CAST
%right NOT
%nonassoc UMINUS
%nonassoc GENERATOR
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


%start file

%%

file:   PARSE_PRG prg {syntax_tree=$2;}
      | PARSE_PRG modimp {syntax_tree=$2;}
      | PARSE_DEC moddec {decl_tree=$2;}
      | PARSE_SIB sib {sib_tree=$2;}
      | PARSE_RC  targets {target_list=RSCAddTargetList($2, target_list);}
                 
  
	;

id: ID
      {
        $$=$1;
      }
  | PRIVATEID
      {
        if (file_kind!=F_sib)
        {
          ABORT(linenum, ("Identifier name '%s` illegal", $1));
        }
        else
        {
          $$=$1;
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
          $$=$1;
          $$->node[0]=$5;
          $$->node[1]=$2;
          if($$->node[1] != NULL)
          {
             DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , %s"P_FORMAT" %s," P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.fun_name.id,
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0],
                        mdb_nodetype[ $$->node[1]->nodetype ], $$->node[1]));
          }
          else
          {
             DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , %s"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.fun_name.id,
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0]));
          }
	     }
      | modheader evimport
        {
          $$=$1;
          $$->node[0]=MakeNode(N_explist);
          $$->node[1]=$2;
          if($$->node[1] != NULL)
          {
             DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , %s"P_FORMAT" %s," P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.fun_name.id,
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0],
                        mdb_nodetype[ $$->node[1]->nodetype ], $$->node[1]));
          }
          else
          {
             DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , %s"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.fun_name.id,
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0]));
          }
	}
      ;

modheader: modclass evextern id COLON linkwith
           {
             $$=MakeNode($1);
             link_mod_name=$3;

            if ($2)
             {
               mod_name=NULL;
             }
             else
             {
               mod_name=link_mod_name;
             }

             MODDEC_NAME($$)=$3;
             MODDEC_LINKWITH($$)=$5;
             MODDEC_ISEXTERNAL($$)=$2;
           }
         ;                 

modclass: MODDEC {$$=N_moddec; file_kind=F_moddec;}
	     | CLASSDEC {$$=N_classdec; file_kind=F_classdec;}
	     ;

evextern: EXTERN 
          {
            file_kind++;
            $$=1;
          }
	|
          {
            $$=0;
          }
	;

linkwith: PRAGMA LINKWITH linklist {$$=$3;}
        | {$$=NULL;}
        ;

linklist: string COMMA linklist 
           {
             $$=MakeDeps($1, NULL, NULL, ST_system, NULL, $3);
           }
       | string
           {
             $$=MakeDeps($1, NULL, NULL, ST_system, NULL, NULL);
           }
       ;

evimport: imports {$$=$1;}
	  | {$$=NULL;}
	  ;

imports: import imports { $$=$1;
                          $$->node[0]=$2;
                        }

	| import { $$=$1; }
	;

import: IMPORT id COLON impdesc { $$=$4; $$->info.id=$2; }
	;

impdesc: ALL SEMIC
            { $$=MakeNode( N_implist );

              DBUG_PRINT("GENTREE",
                         ("%s:"P_FORMAT,mdb_nodetype[ $$->nodetype ], $$));
            }
	| BRACE_L IMPLICIT TYPES COLON ids SEMIC impdesc2
            { $$=$7;
              $$->node[1]=(node *)$5; /* dirty trick for keeping ids !*/
            }
	| BRACE_L IMPLICIT TYPES COLON SEMIC impdesc2
            { $$=$6;
            }
	| BRACE_L impdesc2 { $$=$2; }
	;

impdesc2: EXPLICIT TYPES COLON ids SEMIC impdesc3
            { $$=$6;
              $$->node[2]=(node *)$4; /* dirty trick for keeping ids !*/
            }
        | EXPLICIT TYPES COLON SEMIC impdesc3
            { $$=$5;
            }
	     | impdesc3 { $$=$1; }
	     ;

impdesc3: GLOBAL OBJECTS COLON ids SEMIC impdesc4
            { $$=$6;
              $$->node[4]=(node *)$4; /* dirty trick for keeping ids !*/
            }
        | GLOBAL OBJECTS COLON SEMIC impdesc4
            { $$=$5;
            }
	     | impdesc4 { $$=$1; }
	;

impdesc4: FUNS COLON ids SEMIC BRACE_R
            { $$=MakeNode( N_implist );
              $$->node[3]=(node *)$3; /* dirty trick for keeping ids !*/

              DBUG_PRINT("GENTREE",
                         ("%s:"P_FORMAT,mdb_nodetype[ $$->nodetype ], $$));
            }
        | FUNS COLON SEMIC BRACE_R
            { $$=MakeNode( N_implist );

              DBUG_PRINT("GENTREE",
                         ("%s:"P_FORMAT,mdb_nodetype[ $$->nodetype ], $$));
            }
	     | BRACE_R
            { $$=MakeNode( N_implist );

              DBUG_PRINT("GENTREE",
                         ("%s:"P_FORMAT,mdb_nodetype[ $$->nodetype ], $$));
            }
	;

expdesc: BRACE_L IMPLICIT TYPES COLON imptypes expdesc2
         { $$=$6;
           $$->node[0]=$5;
         }
        | BRACE_L expdesc2 
           {$$=$2;}
        | BRACE_L IMPLICIT TYPES COLON expdesc2
           { $$=$5;}
        ;

expdesc2: EXPLICIT TYPES COLON exptypes expdesc3
          { $$=$5;
            $$->node[1]=$4;
          }
        | EXPLICIT TYPES COLON expdesc3
          { $$=$4;
          }
        | expdesc3 {$$=$1;}
        ;

expdesc3: GLOBAL OBJECTS COLON objdecs expdesc4
          { $$=$5;
            $$->node[3]=$4;
          }
        | GLOBAL OBJECTS COLON expdesc4
          { $$=$4;
          }
        | expdesc4 {$$=$1;}
        ;

expdesc4: FUNS COLON fundecs BRACE_R
          { $$=MakeNode(N_explist);
            $$->node[2]=$3;

            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$));
          }
        | FUNS COLON BRACE_R
          { $$=MakeNode(N_explist);

            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$));
          }
        | BRACE_R
          { $$=MakeNode(N_explist);
         
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$));
          }
        ;

imptypes: imptype imptypes {$$=$1;
                            $1->node[0]=$2;
                           }
	| imptype {$$=$1;}
	;

imptype: id SEMIC pragmas
              {
              $$=MakeNode(N_typedef);
              $$->info.types=MakeTypes(T_hidden);

              DBUG_PRINT("GENTREE",("type:"P_FORMAT" %s",
                                    $$->info.types,
                                    mdb_type[$$->info.types->simpletype]));

              $$->info.types->id=$1;
              $$->info.types->id_mod=mod_name;
              $$->info.types->id_cmod=link_mod_name;
              $$->info.types->status=ST_imported;
              TYPEDEF_PRAGMA($$)=$3;
              
              DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$,
                        $$->info.types, $$->info.types->id));

             }
             ;

exptypes: exptype exptypes {$$=$1;
                            $1->node[0]=$2;
                           }
	| exptype {$$=$1;}
	;

exptype: id LET type SEMIC pragmas
           { $$=MakeNode(N_typedef);
            $$->info.types=$3;
            $$->info.types->id=$1;
            $$->info.types->id_mod=mod_name;
            $$->info.types->id_cmod=link_mod_name;
            $$->info.types->status=ST_imported;
            TYPEDEF_PRAGMA($$)=$5;

            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        $$->info.types, $$->info.types->id));
          }
         ;

objdecs: objdec objdecs    {$$=$1;
                            $$->node[0]=$2;
                           }
	| objdec {$$=$1;}
	;

objdec: type id SEMIC pragmas
           { $$=MakeNode(N_objdef);
             $$->info.types=$1;
             $$->info.types->id=$2;              /* object name */
             $$->info.types->id_mod=mod_name;    /* SAC module name */
             $$->info.types->id_cmod=link_mod_name; /* external module name */
             $$->info.types->status=ST_imported;
             $$->node[1]=NULL;	/* there is no object init here! */
             OBJDEF_PRAGMA($$)=$4;
             
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        $$->info.types, $$->info.types->id));
          }
         ;

fundecs: fundec fundecs 
           {
             $$=$1;
             $$->node[1]=$2;
           }
       | fundec {$$=$1;}
       ;


fundec: varreturntypes id BRACKET_L fundec2
          {
            $$=$4;
            $$->info.types=$1;
            $$->info.types->id=$2; /* function name */
            $$->info.types->id_mod=mod_name;    /* SAC modul name */
            $$->info.types->id_cmod=link_mod_name; /* external modul name */
            $$->info.types->status=ST_imported;
            
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , NULL body,  %s" P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.types->id,
                        mdb_nodetype[ ($$->node[2]==NULL)
                                      ?T_void:($$->node[2]->nodetype) ],
                        $$->node[2]));
          }         
      | returntypes prf_name BRACKET_L fundec3
          { 
/*
            if(!((F_moddec == file_kind) || (F_classdec == file_kind)))
             {
                strcpy(yytext,"(");
                yyerror("syntax error");
             }
            else
             {
*/
                $$=$4;
                $$->info.types=$1;
                $$->info.types->id=$2;              /* function name */
                $$->info.types->id_mod=mod_name;    /* SAC module name */
                $$->info.types->id_cmod=link_mod_name; 
                                              /* external module name */
                $$->info.types->status=ST_imported;
                
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s"P_FORMAT", NULL body,  %s"
                        P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.types->id,
                        $$->info.types->id,
                        mdb_nodetype[ ($$->node[2]==NULL)
                                      ?T_void:($$->node[2]->nodetype) ],
                        $$->node[2]));

/*
             }
*/
             
          }
        ;

fundec2 : varargtypes BRACKET_R SEMIC pragmas
            {
              $$=MakeNode(N_fundef);
              $$->node[2]=$1;        /* argument declarations */
              FUNDEF_PRAGMA($$)=$4;
            }
        | varargs BRACKET_R SEMIC pragmas
            {
              $$=MakeNode(N_fundef);
              $$->node[2]=$1;        /* argument declarations */
              FUNDEF_PRAGMA($$)=$4;
            }
        | TYPE_DOTS BRACKET_R SEMIC pragmas
           {
              if((F_extmoddec != file_kind) && (F_extclassdec != file_kind))
              {
                strcpy(yytext,"...");
                yyerror("syntax error");
              }
              else
              {
                $$=MakeNode(N_fundef);
                $$->node[2]=MakeNode(N_arg);
                $$->node[2]->info.types=MakeTypes(T_dots);
                FUNDEF_PRAGMA($$)=$4;
              }
            }
        | BRACKET_R SEMIC pragmas
            {
              $$=MakeNode(N_fundef);
              FUNDEF_PRAGMA($$)=$3;
            }
        ;

fundec3 : argtypes BRACKET_R SEMIC pragmas
            {
              $$=MakeNode(N_fundef);
              $$->node[2]=$1;        /* argument declarations */
              FUNDEF_PRAGMA($$)=$4;
            }
        | args BRACKET_R SEMIC pragmas
            {
              $$=MakeNode(N_fundef);
              $$->node[2]=$1;        /* argument declarations */
              FUNDEF_PRAGMA($$)=$4;
            }
        | BRACKET_R SEMIC pragmas
            {
              $$=MakeNode(N_fundef);
              FUNDEF_PRAGMA($$)=$3;
            }
        ;

pragmas: pragmalist
         {
           $$=store_pragma;
           store_pragma=NULL;
         }
       |
         {
           $$=NULL;
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
          PRAGMA_LINKNAME(store_pragma)=$3;
        }
      | PRAGMA LINKSIGN SQBR_L nums SQBR_R
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_LINKSIGNNUMS(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'linksign`"))
         PRAGMA_LINKSIGNNUMS(store_pragma)=$4;
        }
      | PRAGMA REFCOUNTING SQBR_L nums SQBR_R
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_REFCOUNTINGNUMS(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'refcounting`"))
          PRAGMA_REFCOUNTINGNUMS(store_pragma)=$4;
        }
      | PRAGMA READONLY SQBR_L nums SQBR_R
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_READONLYNUMS(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'readonly`"))
          PRAGMA_READONLYNUMS(store_pragma)=$4;
        }
      | PRAGMA EFFECT modnames
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_EFFECT(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'effect`"))
          PRAGMA_EFFECT(store_pragma)=$3;
        }
      | PRAGMA TOUCH modnames
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_TOUCH(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'touch`"))
          PRAGMA_TOUCH(store_pragma)=$3;
        }
      | PRAGMA COPYFUN string
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_COPYFUN(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'copyfun`"))
          PRAGMA_COPYFUN(store_pragma)=$3;
        }
      | PRAGMA FREEFUN string
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_FREEFUN(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'freefun`"))
          PRAGMA_FREEFUN(store_pragma)=$3;
        }
      | PRAGMA INITFUN string
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_INITFUN(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'initfun`"))
          PRAGMA_INITFUN(store_pragma)=$3;
        }
      ;

modnames: modname COMMA modnames
          {
            $$=$1;
            IDS_NEXT($$)=$3;
          }
        | modname
          {
            $$=$1;
          }
        ;

modname: id
         {
           $$=MakeIds($1, NULL, ST_regular);
         }
       | id COLON id
         {
           $$=MakeIds($3, $1, ST_regular);
         }
       ;




/*
 *********************************************************************
 *
 *  rules for type, object, and function definitions
 *
 *********************************************************************
 */


defs: imports def2 {$$=$2;
                    $$->node[0]=$1;
                   }
	| def2 {$$=$1;}

def2: typedefs def3 {$$=$2;
                     $$->node[1]=$1;
                    }
	|def3 {$$=$1;}

def3: objdefs def4 { $$=$2;
                     $$->node[3]=$1;
                   }
      | def4 { $$=$1;}
      ;

def4: fundefs { $$=MakeNode(N_modul);
                $$->info.id=NULL;
                $$->node[2]=$1;

                DBUG_PRINT("GENTREE",
                           ("%s:"P_FORMAT"  %s"P_FORMAT,
                            mdb_nodetype[ $$->nodetype ], $$, 
                            mdb_nodetype[ $$->node[2]->nodetype ],
                            $$->node[2]));
              }
    |         { $$=MakeNode(N_modul);  /* module impl with no functions */
                $$->info.id=NULL;
                $$->node[2]=NULL;

                DBUG_PRINT("GENTREE",
                           ("%s:"P_FORMAT,
                            mdb_nodetype[ $$->nodetype ], $$));
                
              }
	;

modimp: module
        {
          $$=$1;
        }
      | class
        {
          $$=$1;
        }
      ;

module: MODIMP { file_kind=F_modimp;} id {  mod_name=$3; } COLON defs
          {
            $$=$6;
            MODUL_NAME($$)=mod_name;
            MODUL_FILETYPE($$)=file_kind;
          }
	;

class: CLASSIMP { file_kind=F_classimp;} id {  mod_name=$3; } COLON 
       CLASSTYPE type SEMIC defs
          { 
            $$=$9;
            MODUL_NAME($$)=mod_name;
            MODUL_FILETYPE($$)=file_kind;
            MODUL_CLASSTYPE($$)=$7;
          }
	;

prg: defs { $$=$1;
            MODUL_NAME($$)=mod_name;
            MODUL_FILETYPE($$)=F_prog;
          }
        ;

typedefs: typedef typedefs {$$=$1;
                            $1->node[0]=$2;
                           }
	| typedef {$$=$1; }
	;

typedef: TYPEDEF type id SEMIC 
          { $$=MakeNode(N_typedef);
            $$->info.types=$2;
            $$->info.types->id=$3;
            $$->info.types->id_mod=mod_name;
            /*
            if ((file_kind==F_classimp) && (!strcmp((char*) $3, mod_name)))
               $$->info.types->attrib=ST_unique;
            */
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        $$->info.types, $$->info.types->id));
          }

objdefs: objdef objdefs {$$=$1;
                         $$->node[0]=$2;
                           }
	| objdef {$$=$1; }
	;

objdef: OBJDEF type id LET expr SEMIC 
          { $$=MakeNode(N_objdef);
            $$->info.types=$2;
            $$->info.types->id=$3;
            $$->info.types->id_mod=mod_name;
            
            $$->node[1]=$5;
            
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        $$->info.types, $$->info.types->id));
          }
      ;



fundefs: wlcomp_pragma_global fundef fundefs
         { $$ = $2;
           FUNDEF_NEXT($$) = $3;
         }
       | wlcomp_pragma_global main
         { $$ = $2;
         }
       | wlcomp_pragma_global fundef
         { $$ = $2;
         }
       ;

fundef: returntypes fun_name BRACKET_L fundef2 
        {  $$=$4;
           $$->info.types=$1;          		/*  result type(s) */
           $$->info.types->id=$2;      		/*  function name  */
           $$->info.types->id_mod=mod_name;     /*  module name    */
           $$->flag=0;                          /*  inline flag    */

           DBUG_PRINT("GENTREE",
                      ("%s: %s:%s "P_FORMAT,
                       mdb_nodetype[ $$->nodetype ],
                       $$->info.types->id_mod,
                       $$->info.types->id,
                       $$->info.types->id));

        }
      | INLINE returntypes fun_name BRACKET_L fundef2 
        {  $$=$5;
           $$->info.types=$2;          		/*  result type(s) */
           $$->info.types->id=$3;      		/*  function name  */
           $$->info.types->id_mod=mod_name;     /*  module name    */
           $$->flag=1;                          /*  inline flag    */

           DBUG_PRINT("GENTREE",
                      ("%s: %s:%s "P_FORMAT,
                       mdb_nodetype[ $$->nodetype ],
                       $$->info.types->id_mod,
                       $$->info.types->id,
                       $$->info.types->id));

        }
      ;

fundef2: args BRACKET_R  {$$=MakeNode(N_fundef);}   exprblock
          { 
            $$=$<node>3;
            $$->node[0]=$4;             /* Funktionsrumpf  */
            $$->node[2]=$1;             /* Funktionsargumente */
         
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT", Id: %s"P_FORMAT" %s," P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, 
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0],
                        mdb_nodetype[ $$->node[2]->nodetype ], $$->node[2]));
          }

       | BRACKET_R  {$$=MakeNode(N_fundef);}   exprblock
          { 
            $$=$<node>2;
            $$->node[0]=$3;             /* Funktionsrumpf  */
         
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" %s"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, 
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0]));
         }
        ;


args:   arg COMMA args  {$1->node[0]=$3;
                         $$=$1;
                        }
      | arg {$$=$1;}
      ;

varargs: arg COMMA varargs
         {
           $1->node[0]=$3;
           $$=$1;
                           }
       | arg COMMA TYPE_DOTS
           {
             if((F_extmoddec != file_kind) && (F_extclassdec != file_kind))
             {
               strcpy(yytext,"...");
               yyerror("syntax error");
             }
             else
             {
               $$=$1;
               $$->node[0]=MakeNode(N_arg);
               $$->node[0]->info.types=MakeTypes(T_dots);
               
              DBUG_PRINT("GENTREE",
                         ("%s: "P_FORMAT", Id: ..., Attrib: %d  ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          $$->info.types->attrib));
             } 
           }
       | arg
         {
           $$=$1;
         }
      ;

arg: type id {$$=MakeNode(N_arg); 
              $$->info.types=$1;         /* Argumenttyp */
              $$->info.types->id=$2;     /* Argumentname */
              $$->info.types->next=NULL; /* keine weiteren Argumente */

              DBUG_PRINT("GENTREE",
                         ("%s: "P_FORMAT", Id: %s, Attrib: %d  ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          $$->info.types->id, $$->info.types->attrib));
             }
     |
     type AMPERS id {$$=MakeNode(N_arg); 
              $$->info.types=$1;         /* Argumenttyp */
              $$->info.types->id=$3;     /* Argumentname */
              $$->info.types->next=NULL; /* keine weiteren Argumente */
              $$->info.types->attrib=ST_reference;
                     
              DBUG_PRINT("GENTREE",
                         ("%s: "P_FORMAT", Id: %s, Attrib: %d ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          $$->info.types->id, $$->info.types->attrib));
             }
     ;

argtypes: argtype COMMA argtypes
          {
            $1->node[0]=$3;
            $$=$1;
          }
        | argtype
          {
            $$=$1;
          }
      ;

varargtypes: argtype COMMA varargtypes
             {
               $1->node[0]=$3;
               $$=$1;
             }
       | argtype COMMA TYPE_DOTS
           {
             if((F_extmoddec != file_kind) && (F_extclassdec != file_kind))
             {
               strcpy(yytext,"...");
               yyerror("syntax error");
             }
             else
             {
               $$=$1;
               $$->node[0]=MakeNode(N_arg);
               $$->node[0]->info.types=MakeTypes(T_dots);

              DBUG_PRINT("GENTREE",
                         ("%s: "P_FORMAT", Attrib: %d  ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          $$->info.types->attrib));

             }
           }
      | argtype {$$=$1;}
      ;

argtype: type {$$=MakeNode(N_arg); 
              $$->info.types=$1;         /* Argumenttyp */
              $$->info.types->next=NULL; /* keine weiteren Argumente */

              DBUG_PRINT("GENTREE",
                         ("%s: "P_FORMAT", Attrib: %d  ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          $$->info.types->attrib));
             }
     |
     type AMPERS {$$=MakeNode(N_arg); 
              $$->info.types=$1;         /* Argumenttyp */
              $$->info.types->next=NULL; /* keine weiteren Argumente */
              $$->info.types->attrib=ST_reference;
                     
              DBUG_PRINT("GENTREE",
                         ("%s: "P_FORMAT", Attrib: %d ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          $$->info.types->attrib));
             }
     ;

fun_name : id { $$=$1; }
         | prf_name { $$=$1; }
         ;

prf_name : AND        { $$=StringCopy(prf_name_str[F_and]); }
         | OR         { $$=StringCopy(prf_name_str[F_or]); }
         | EQ         { $$=StringCopy(prf_name_str[F_eq]); }
         | NEQ        { $$=StringCopy(prf_name_str[F_neq]); }
         | NOT        { $$=StringCopy(prf_name_str[F_not]); }
         | LE         { $$=StringCopy(prf_name_str[F_le]); }
         | LT         { $$=StringCopy(prf_name_str[F_lt]); }
         | GE         { $$=StringCopy(prf_name_str[F_ge]); }
         | GT         { $$=StringCopy(prf_name_str[F_gt]); }
         | ABS        { $$=StringCopy(prf_name_str[F_abs]); }
         | PLUS       { $$=StringCopy(prf_name_str[F_add]); }
         | MINUS      { $$=StringCopy(prf_name_str[F_sub]); }
         | DIV        { $$=StringCopy(prf_name_str[F_div]); }
         | MUL        { $$=StringCopy(prf_name_str[F_mul]); }
         | PRF_MOD    { $$=StringCopy(prf_name_str[F_mod]); }
         | RESHAPE    { $$=StringCopy(prf_name_str[F_reshape]); }
         | SHAPE      { $$=StringCopy(prf_name_str[F_shape]); }
         | TAKE       { $$=StringCopy(prf_name_str[F_take]); }
         | DROP       { $$=StringCopy(prf_name_str[F_drop]); }
         | DIM        { $$=StringCopy(prf_name_str[F_dim]); }
         | ROTATE     { $$=StringCopy(prf_name_str[F_rotate]); }
         | CAT        { $$=StringCopy(prf_name_str[F_cat]); }
         | PSI        { $$=StringCopy(prf_name_str[F_psi]); }
         | GENARRAY   { $$=StringCopy(prf_name_str[F_genarray]); }
         | MODARRAY   { $$=StringCopy(prf_name_str[F_modarray]); }
         | TOI        { $$=StringCopy(prf_name_str[F_toi]); }
         | TOF        { $$=StringCopy(prf_name_str[F_tof]); }
         | TOD        { $$=StringCopy(prf_name_str[F_tod]); }
         | PRF_MIN    { $$=StringCopy(prf_name_str[F_min]); }
         | PRF_MAX    { $$=StringCopy(prf_name_str[F_max]); }
         | ALL        { $$=StringCopy("all");
                        /* necessary because function 'all()' from array library conflicts
                           with key word. */}
        ;

main: TYPE_INT K_MAIN BRACKET_L BRACKET_R {$$=MakeNode(N_fundef);} exprblock 
      {
        $$=$<node>5;     /* $$=$5 */
        $$->node[0]=$6;                 /* Funktionsrumpf */

        $$->info.types=MakeTypes(T_int);  /* Knoten fu"r Typinformation */ 
        $$->info.types->id=(char *)Malloc(sizeof(char)*5); 
        strcpy($$->info.types->id, "main");   /* Funktionsnamen eintragen */

        DBUG_PRINT("GENTREE",("%s:"P_FORMAT", main "P_FORMAT
                              "  %s (" P_FORMAT ") ",
                              mdb_nodetype[$$->nodetype], $$, 
                              $$->info.types->id,
                              mdb_nodetype[ $$->node[0]->nodetype], 
                              $$->node[0]));
        
      }
    ;


wlcomp_pragma_global: PRAGMA WLCOMP wlcomp_expr
                      { if (store_wlcomp_pragma_global != NULL) {
                          /* remove old global pragma */
                          store_wlcomp_pragma_global
                            = FreeTree(store_wlcomp_pragma_global);
                        }
                        store_wlcomp_pragma_global = MakePragma();
                        PRAGMA_WLCOMP_APS(store_wlcomp_pragma_global) = $3;
                      }
                    | /* empty */
                      {
                      }
                    ;

wlcomp_pragma_local: PRAGMA WLCOMP wlcomp_expr
                     { $$ = MakePragma();
                       PRAGMA_WLCOMP_APS($$) = $3;
                     }
                   | /* empty */
                     { if (store_wlcomp_pragma_global != NULL) {
                         $$ = MakePragma();
                         PRAGMA_WLCOMP_APS($$)
                          = DupTree(PRAGMA_WLCOMP_APS(store_wlcomp_pragma_global), NULL);
                       } else {
                         $$ = NULL;
                       }
                     }
                   ;

wlcomp_expr: DEFAULT
             { $$ = NULL;
             }
           | id BRACKET_L wlcomp_args wlcomp_expr BRACKET_R
             { node *tmp = $4;

               $$ = MakeExprs(MakeAp($1, NULL, $3), NULL);
               /* append $$ to $4 */
               if (tmp != NULL) {
                 while (EXPRS_NEXT(tmp) != NULL) {
                   tmp = EXPRS_NEXT(tmp);
                 }
                 EXPRS_NEXT(tmp) = $$;
                 $$ = $4;
               }
             }
           ;

wlcomp_args: expr_ar COMMA wlcomp_args
             { $$ = MakeExprs($1, $3);
             }
           | expr_num COMMA wlcomp_args
             { $$ = MakeExprs($1, $3);
             }
           |
             { $$ = NULL;
             }
           ;




/* BRUSH BEGIN */


/*
 *********************************************************************
 *
 *  rules for expression blocks
 *
 *********************************************************************
 */


exprblock: BRACE_L exprblock2 { $$=$2; }
         ;

exprblock2: typeNOudt_arr ids SEMIC exprblock2 
            { node *vardec_ptr;
              ids   *ids_ptr=$2;
 
              /*
               * Insert the actual vardec(s) before the ones that
               * are already attached to the N_block node of $4!
               * This reverses the order of var-decs!
               * The reason for doing so is feasablilty only...
               * In regard to semantics, there should be no difference...
               */
              vardec_ptr=BLOCK_VARDEC( $4);

              DBUG_ASSERT( ($2 != NULL),
                           "non-terminal ids should not return NULL ptr!");
                                               
              /*
               * In the AST, each variable has it's own declaration.
               * Therefore, for each ID in ids, we have to generate 
               * it's own N_vardec node with it's own copy of the
               * types-structure from $1!
               */
              while( IDS_NEXT($2) != NULL) { /* at least 2 vardecs! */
                vardec_ptr=MakeVardec(
                             IDS_NAME($2),
                               DuplicateTypes( $1, 1),
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
                ids_ptr=IDS_NEXT( $2);
                FREE( $2);
                $2=ids_ptr;
              }
              /*
               * When we reach this point, all but one vardec is constructed!
               * Therefore, we can recycle the types node from $1 instead of
               * duplicating it as done in the loop above!
               */
              $$=$4;
              BLOCK_VARDEC($$)=MakeVardec(
                                 IDS_NAME($2),
                                   $1,
                                     vardec_ptr);
              FREE( $2); /* Finally, we free the last IDS-node! */

            }
          | assignsOPTret BRACE_R { $$=MakeBlock( $1, NULL); }
          ;


wlassignblock: BRACE_L {$<cint>$=linenum;} assigns BRACE_R 
               { if($3==NULL) {
                   $$=MAKE_EMPTY_BLOCK();
                 }
                 else {
                   $$=MakeBlock( $3, NULL);
                 }
                 NODE_LINE($$)=$<cint>2;
               } 
             |
               {
                 $$ = MAKE_EMPTY_BLOCK();
               }
             ;

assignblock: SEMIC     
             { $$=MAKE_EMPTY_BLOCK( );
             }
           | BRACE_L {$<cint>$=linenum;} assigns BRACE_R 
             { if($3==NULL) {
                 $$=MAKE_EMPTY_BLOCK();
               }
               else {
                 $$=MakeBlock( $3, NULL);
                 NODE_LINE($$)=$<cint>2;
               }
             }
           | assign
             { $$=MakeBlock( MakeAssign( $1, NULL), NULL);
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
               { $$=MakeAssign( MakeReturn(NULL), NULL);
               }
             | RETURN BRACKET_L {$<cint>$=linenum;} exprs BRACKET_R SEMIC
               { $$=MakeAssign( MakeReturn( $4), NULL);
                 NODE_LINE($$)=$<cint>3;
               }
             | assign assignsOPTret
               { if( NODE_TYPE($1)==N_assign){
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
                   DBUG_ASSERT( (NODE_TYPE( ASSIGN_INSTR( ASSIGN_NEXT($1)))==N_while)
                                 && (ASSIGN_NEXT( ASSIGN_NEXT($1))==NULL),
                                "corrupted node returned for \"assign\"!");
                   $$=$1;
                   ASSIGN_NEXT( ASSIGN_NEXT($$))=$2;
                 }
                 else {
                   $$=MakeAssign( $1, $2);
                 }
               }
             ;

assigns: /* empty */
         { $$=NULL;
         }
       | assign assigns
         { if (NODE_TYPE($1)==N_assign){
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
             DBUG_ASSERT( (NODE_TYPE( ASSIGN_INSTR( ASSIGN_NEXT($1)))==N_while)
                           && (ASSIGN_NEXT( ASSIGN_NEXT($1))==NULL),
                          "corrupted node returned for \"assign\"!");
             $$=$1;
             ASSIGN_NEXT(ASSIGN_NEXT($$))=$2;
           }
           else {
             $$=MakeAssign($1, $2);
           }
         }
       ;

assign: letassign SEMIC { $$=$1; }
      | selassign       { $$=$1; }
      | forassign       { $$=$1; }
      ;

letassign: ids LET expr 
           { $$=MakeLet($3, $1);
           }
         | id SQBR_L expr SQBR_R LET expr
           { $$=MakeLet( MakePrf( F_modarray,
                           MakeExprs( MakeId( $1, NULL, ST_regular) ,
                             MakeExprs( $3,
                               MakeExprs($6,
                                 NULL))) ),
                         MakeIds(StringCopy($1), NULL, ST_regular) );
           }
         | id SQBR_L expr COMMA exprs SQBR_R LET expr
           { $$=MakeLet( MakePrf( F_modarray,
                           MakeExprs( MakeId( $1, NULL, ST_regular) ,
                                      MakeExprs( MakeArray(MakeExprs($3, $5)),
                                                 MakeExprs($8,
                                                           NULL))) ),
                         MakeIds(StringCopy($1), NULL, ST_regular) );
           }
         | expr_ap 
           { $$=MakeLet( $1, NULL);
           }

/* left for later BRUSHING BEGIN */
/*
         | id unaryop 
            { $$=MakeNode(N_post);
              $$->info.ids=MakeIds($1, NULL, ST_regular);
              $$->node[0]=$2;
              
              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT": %s "P_FORMAT,
                          mdb_nodetype[$$->nodetype], $$, $$->info.ids->id,
                          mdb_nodetype[$$->node[0]->nodetype] ));
           }
         | unaryop id
            {  $$=MakeNode(N_pre);
               $$->info.ids=MakeIds($2, NULL, ST_regular);    
               $$->node[0]=$1;

               DBUG_PRINT("GENTREE",
                          ("%s "P_FORMAT": %s "P_FORMAT,
                           mdb_nodetype[$$->nodetype], $$, $$->info.ids->id,
                           mdb_nodetype[$$->node[0]->nodetype] )); 
            }
*/
         | id INC { $$=MAKE_INCDEC_LET($1,F_add); }
         | INC id { $$=MAKE_INCDEC_LET($2,F_add); }
         | id DEC { $$=MAKE_INCDEC_LET($1,F_sub); }
         | DEC id { $$=MAKE_INCDEC_LET($2,F_sub); }

/* left for later BRUSHING END */

         | id ADDON expr { $$=MAKE_OPON_LET($1,$3,F_add); }
         | id SUBON expr { $$=MAKE_OPON_LET($1,$3,F_sub); }
         | id MULON expr { $$=MAKE_OPON_LET($1,$3,F_mul); }
         | id DIVON expr { $$=MAKE_OPON_LET($1,$3,F_div); }
         | id MODON expr { $$=MAKE_OPON_LET($1,$3,F_mod); }
         ;

selassign: IF {$<cint>$=linenum;} BRACKET_L exprNOar BRACKET_R assignblock 
           optelse
           { $$=MakeCond( $4, $6, $7);
             NODE_LINE($$)=$<cint>2;
           }
         ;

optelse: ELSE assignblock                  { $$ = $2;                  }
       | /* empty*/  %prec LOWER_THAN_ELSE { $$ = MAKE_EMPTY_BLOCK( ); } 
       ;

forassign: DO {$<cint>$=linenum;} assignblock
           WHILE BRACKET_L exprNOar BRACKET_R SEMIC
           { $$=MakeDo( $6, $3);
             NODE_LINE($$)=$<cint>2;
           }
         | WHILE {$<cint>$=linenum;} BRACKET_L exprNOar BRACKET_R assignblock 
           { $$=MakeWhile( $4, $6);
             NODE_LINE($$)=$<cint>2;
           }
         | FOR {$<cint>$=linenum;} BRACKET_L assign exprNOar SEMIC 
           letassign BRACKET_R assignblock
           { /*
              * for( x=e1; e2; y=e3) AssBlock
              * is transformed into
              * x=e1;
              * while( e2) { AssBlock; y=e3;}
              */
             $$=MakeAssign($4, MakeAssign( MakeWhile( $5, Append($9,$7)), NULL)); 
             NODE_LINE( ASSIGN_INSTR( ASSIGN_NEXT($$)))=$<cint>2;
           } 
         ;

exprs: expr COMMA exprs { $$=MakeExprs($1, $3);   }
     | expr             { $$=MakeExprs($1, NULL); }
     ;

exprsNOar: exprNOar COMMA exprsNOar { $$=MakeExprs($1, $3);   }
         | exprNOar                 { $$=MakeExprs($1, NULL); }
         ;

exprNOdot: expr %prec GENERATOR { $$ = $1; }
         ;

exprORdot: expr %prec GENERATOR { $$ = $1;   }
         | DOT                  { $$ = NULL; }
         ;

exprNOar: expr_main {$$ = $1;}
        | expr_ap   {$$ = $1;}
        | expr_num  {$$ = $1;}
        ;
 
expr: expr_main {$$ = $1;}
    | expr_ar   {$$ = $1;}
    | expr_ap   {$$ = $1;}
    | expr_num  {$$ = $1;}
    ;
 
expr_num: NUM {$$=MakeNum( $1);}
        ;

expr_ar: SQBR_L {$<cint>$=linenum;} exprsNOar SQBR_R
         { $$=MakeArray( $3);
           NODE_LINE($$)=$<cint>2;
         }
       ;

expr_ap: id BRACKET_L {$<cint>$=linenum;} opt_arguments BRACKET_R
         { $$=MakeAp( $1, NULL, $4);
           NODE_LINE($$)=$<cint>3;
         }
       | id COLON id  BRACKET_L {$<cint>$=linenum;} opt_arguments BRACKET_R
          { $$=MakeAp( $3, $1, $6);
            NODE_LINE($$)=$<cint>5;
          }
       | monop BRACKET_L {$<cint>$=linenum;} opt_arguments BRACKET_R
         { 
           if (CountArguments($4)==1) {
             $$=MakePrf($1, $4);
           }
           else {
             $$=MakeAp(StringCopy(prf_name_str[$1]), NULL, $4);
           }
           NODE_LINE($$)=$<cint>3;
         }
       | id COLON monop  BRACKET_L {$<cint>$=linenum;} opt_arguments BRACKET_R
         { 
           $$=MakeAp(StringCopy(prf_name_str[$3]), $1, $6);
           NODE_LINE($$)=$<cint>5;
         }
       | binop BRACKET_L {$<cint>$=linenum;} opt_arguments BRACKET_R
         { 
           if (CountArguments($4)==2) {
             $$=MakePrf($1, $4);
           }
           else {
             $$=MakeAp(StringCopy(prf_name_str[$1]), NULL, $4);
           }
           NODE_LINE($$)=$<cint>3;
         }
       | id COLON binop  BRACKET_L {$<cint>$=linenum;} opt_arguments BRACKET_R
         { 
           $$=MakeAp(StringCopy(prf_name_str[$3]), $1, $6);
           NODE_LINE($$)=$<cint>5;
         }
       | triop BRACKET_L {$<cint>$=linenum;} opt_arguments BRACKET_R
         { 
           if (CountArguments($4)==3) {
             $$=MakePrf($1, $4);
           }
           else {
             $$=MakeAp(StringCopy(prf_name_str[$1]), NULL, $4);
           }
           NODE_LINE($$)=$<cint>3;
         }
       | id COLON triop  BRACKET_L {$<cint>$=linenum;} opt_arguments BRACKET_R
         { 
           $$=MakeAp(StringCopy(prf_name_str[$3]), $1, $6);
           NODE_LINE($$)=$<cint>5;
         }
       | ALL BRACKET_L {$<cint>$=linenum;} opt_arguments BRACKET_R
         { 
           $$=MakeAp(StringCopy("all"), NULL, $4);
           NODE_LINE($$)=$<cint>3;
         }
       | id COLON ALL  BRACKET_L {$<cint>$=linenum;} opt_arguments BRACKET_R
         { 
           $$=MakeAp(StringCopy("all"), $1, $6);
           NODE_LINE($$)=$<cint>5;
         }
       ;

opt_arguments: exprs   { $$=$1; }
            |          { $$=NULL; }
            ;

expr_main: id  { $$=MakeId( $1, NULL, ST_regular); }
         | id COLON id  { $$=MakeId( $3, $1, ST_regular); }
         | MINUS id %prec UMINUS
           { /*
              * substitute unary minus by multiplication
              * with -1!
              */
             $$=MAKE_BIN_PRF( F_mul, 
                                MakeNum( -1),
                                  MakeId($2, NULL, ST_regular) );
           }
         | BRACKET_L COLON type BRACKET_R expr %prec CAST
           { $$=MakeCast( $5, $3);
           }
         | NOT expr
           { $$=MakePrf( F_not,
                         MakeExprs( $2, NULL));
           }
         | BRACKET_L expr BRACKET_R { $$=$2; }
         | expr SQBR_L expr SQBR_R
           { $$ = MAKE_BIN_PRF( F_psi, $3, $1);
             NODE_LINE($$) = NODE_LINE( $1);
           }
         | expr SQBR_L expr COMMA exprs SQBR_R
           { $$ = MakePrf( F_psi,
                    MakeExprs( MakeArray( MakeExprs( $3, $5)), 
                      MakeExprs( $1,
                        NULL)) );
             NODE_LINE($$) = NODE_LINE( $1);
           }
         | expr AND expr     { $$=MAKE_BIN_PRF( F_and,$1,$3); }
         | expr OR expr      { $$=MAKE_BIN_PRF( F_or ,$1,$3); }
         | expr EQ expr      { $$=MAKE_BIN_PRF( F_eq ,$1,$3); }
         | expr NEQ expr     { $$=MAKE_BIN_PRF( F_neq,$1,$3); }
         | expr LE expr      { $$=MAKE_BIN_PRF( F_le ,$1,$3); }
         | expr LT expr      { $$=MAKE_BIN_PRF( F_lt ,$1,$3); }
         | expr GE expr      { $$=MAKE_BIN_PRF( F_ge ,$1,$3); }
         | expr GT expr      { $$=MAKE_BIN_PRF( F_gt ,$1,$3); }
         | expr PLUS expr    { $$=MAKE_BIN_PRF( F_add,$1,$3); }
         | expr MINUS expr   { $$=MAKE_BIN_PRF( F_sub,$1,$3); }
         | expr DIV expr     { $$=MAKE_BIN_PRF( F_div,$1,$3); }
         | expr MUL expr     { $$=MAKE_BIN_PRF( F_mul,$1,$3); }
         | expr PRF_MOD expr { $$=MAKE_BIN_PRF( F_mod,$1,$3); }
         | MINUS NUM %prec UMINUS    { $$=MakeNum( -$2);           }
         | PLUS  NUM %prec UMINUS    { $$=MakeNum( $2);            }
         | CHAR                      { $$=MakeChar( $1);           }
         | FLOAT                     { $$=MakeFloat( $1);          }
         | MINUS FLOAT %prec UMINUS  { $$=MakeFloat( -$2);         }
         | PLUS FLOAT %prec UMINUS   { $$=MakeFloat( $2);          }
         | DOUBLE                    { $$=MakeDouble( $1);         }
         | MINUS DOUBLE %prec UMINUS { $$=MakeDouble( -$2);        }
         | PLUS DOUBLE %prec UMINUS  { $$=MakeDouble( $2);         }
         | TRUE                      { $$=MakeBool( 1);            }
         | FALSE                     { $$=MakeBool( 0);            }
         | string                    { $$=string2array($1);        }
         | wlcomp_pragma_local
           NWITH {$<cint>$=linenum;} BRACKET_L Ngenerator BRACKET_R
           wlassignblock Nwithop
           { /*
              * the tricky part about this rule is that $8 (an N_Nwithop node)
              * carries the goal-expression of the With-Loop, i.e., the "N-expr"
              * node which belongs into the N_Ncode node!!!
              * The reason for this is that an exclusion of the goal expression
              * from the non-terminal Nwithop would lead to a shift/reduce
              * conflict in that rule!
              */
             $$=MakeNWith( $5, MakeNCode( $7, NWITHOP_EXPR($8)), $8);
             NWITHOP_EXPR($8) = NULL;
             NCODE_USED(NWITH_CODE($$))++;
             NODE_LINE($$)= $<cint>3;
             NWITH_PRAGMA($$) = $1;

             /*
              * Finally, we generate the connection between the 
              * (only) partition and the (only) code!
              */
             NPART_CODE( NWITH_PART($$)) = NWITH_CODE($$);
           }
         | WITH {$<cint>$=linenum;} BRACKET_L generator  BRACKET_R conexpr 
           { $$=MakeWith( $4, $6);
           }
         ;


Ngenerator: exprORdot Ngenop Ngenidx Ngenop exprORdot Nsteps Nwidth
            { 
              $$=MakeNPart( $3, MakeNGenerator($1, $5, $2,  $4, $6, $7 ), NULL);
            }
          ;

Nsteps: /* empty */ { $$ = NULL; }
      | STEP expr   { $$ = $2;}
      ;

Nwidth: /* empty */ { $$ = NULL; }
      | WIDTH expr  { $$ = $2;}
      ;

Ngenidx: id LET SQBR_L ids SQBR_R 
         { 
           $$ = MakeNWithid(MakeIds($1, NULL, ST_regular), $4); 
         }
       | id { $$ = MakeNWithid(MakeIds($1, NULL, ST_regular), NULL); }
       | SQBR_L ids SQBR_R { $$ = MakeNWithid( NULL, $2); }
       ;

Ngenop:   LT {$$=F_lt;}
	| LE {$$=F_le;}
        ;

Nwithop: GENARRAY BRACKET_L expr COMMA expr BRACKET_R
         { $$ = MakeNWithOp( WO_genarray);
           NWITHOP_SHAPE($$) = $3;
           NWITHOP_EXPR($$) = $5;
         }
       | MODARRAY BRACKET_L expr COMMA id COMMA expr BRACKET_R
         { $$ = MakeNWithOp( WO_modarray);
           NWITHOP_ARRAY($$) = $3;
           NWITHOP_EXPR($$) = $7;
         }
       | FOLD BRACKET_L foldop COMMA expr BRACKET_R
         { $$ = MakeNWithOp( WO_foldprf);
           NWITHOP_PRF($$) = $3;
           NWITHOP_EXPR($$) = $5;
         }
       | FOLD BRACKET_L foldop COMMA expr COMMA expr BRACKET_R
         { $$ = MakeNWithOp( WO_foldprf);
           NWITHOP_PRF($$) = $3;
           NWITHOP_NEUTRAL($$) = $5;
           NWITHOP_EXPR($$) = $7;
         }
       | FOLD BRACKET_L id COMMA expr COMMA expr BRACKET_R
         { $$ = MakeNWithOp( WO_foldfun);
           NWITHOP_FUN($$) = $3;
           NWITHOP_NEUTRAL($$) = $5;
           NWITHOP_EXPR($$) = $7;
         }
       | FOLD BRACKET_L id COLON id COMMA expr COMMA expr BRACKET_R
         { $$ = MakeNWithOp( WO_foldfun);
           NWITHOP_FUN($$) = $5;
           NWITHOP_MOD($$) = $3;
           NWITHOP_NEUTRAL($$) = $7;
           NWITHOP_EXPR($$) = $9;
         }
       ;

/* NOT BRUSHED BEGIN */

generator: exprNOdot  LE id LE exprNOdot
            { $$=MakeNode(N_generator);
              $$->node[0]=$1;        /* left border  */
              $$->node[1]=$5;        /* right border */
              $$->info.ids=MakeIds($3, NULL, ST_regular); /*index-variable  */

              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT": %s "P_FORMAT", ID: %s, %s "P_FORMAT ,
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                          $$->info.ids->id,
                          mdb_nodetype[$$->node[1]->nodetype], $$->node[1]));
            }
        ;

conexpr: wlassignblock GENARRAY BRACKET_L expr COMMA {$$=MakeGenarray($4, NULL);}
         expr BRACKET_R
           { node *ret;

             $$=$<node>6;
             ret = MakeReturn( MakeExprs($7, NULL) );
             RETURN_INWITH(ret)=1;
             
             GENARRAY_BODY($$)=Append( $1, ret);
             
           }
         | wlassignblock MODARRAY BRACKET_L expr COMMA id COMMA
           {$$=MakeModarray($4, NULL);} expr BRACKET_R
           { node *ret;

             $$=$<node>8;
             ret = MakeReturn( MakeExprs($9, NULL) );
             RETURN_INWITH(ret)=1;

             MODARRAY_BODY($$)=Append( $1, ret);
             MODARRAY_ID($$)=$6;
             
           }
         | wlassignblock FOLD BRACKET_L foldop COMMA expr BRACKET_R
           { node *ret;

             $$=MakeFoldprf($4, NULL, NULL);
             ret = MakeReturn( MakeExprs($6, NULL) );
             RETURN_INWITH(ret)=1;
             FOLDPRF_BODY($$)=Append( $1, ret);

           }
         | wlassignblock FOLD BRACKET_L foldop COMMA expr COMMA expr
           BRACKET_R
           { node *ret;

             $$=MakeFoldprf($4,NULL,$6);
             ret = MakeReturn( MakeExprs($8, NULL) );
             RETURN_INWITH(ret)=1;
             FOLDPRF_BODY($$)=Append($1, ret);
           }
         | wlassignblock FOLD BRACKET_L foldfun expr COMMA expr 
           BRACKET_R
           { node *ret;

              $$=$<node>4;
              ret = MakeReturn( MakeExprs($7, NULL) );
              RETURN_INWITH(ret)=1;
              FOLDFUN_BODY($$)=Append( $1, ret);
              FOLDFUN_NEUTRAL($$)= $5;

           }
         ;


foldfun:  id COMMA 
     {
        $$=MakeNode(N_foldfun);
        $$->info.fun_name.id=$1;
     }
    | id COLON id COMMA
     {
        $$=MakeNode(N_foldfun);
        $$->info.fun_name.id=$3;
        $$->info.fun_name.id_mod=$1;
     }
    ;
     
/* NOT BRUSHED END  */
 
foldop: PLUS    {$$=F_add;}
      | MUL     {$$=F_mul;}
      | PRF_MIN {$$=F_min;}
      | PRF_MAX {$$=F_max;}
      | AND     {$$=F_and;}
      | OR      {$$=F_or; }
      ;

monop: ABS   { $$=F_abs;   }
     | DIM   { $$=F_dim;   }
     | SHAPE { $$=F_shape; }
     | TOI   { $$=F_toi;   }
     | TOF   { $$=F_tof;   }
     | TOD   { $$=F_tod;   }
     ;

/* left for later BRUSHING BEGIN */
/*
unaryop: INC
          { $$=MakeNode(N_inc);
          }

   
       | DEC
          { $$=MakeNode(N_dec);
          }
           ;
*/
/* left for later BRUSHING END */

binop: PSI      { $$=F_psi;      }
     | TAKE     { $$=F_take;     }
     | DROP     { $$=F_drop;     }
     | RESHAPE  { $$=F_reshape;  }
     | GENARRAY { $$=F_genarray; }
     | PRF_MIN  { $$=F_min;      }
     | PRF_MAX  { $$=F_max;      }
     | PLUS     { $$=F_add;      }
     | MINUS    { $$=F_sub;      }
     | MUL      { $$=F_mul;      }
     | DIV      { $$=F_div;      }
     | PRF_MOD  { $$=F_mod;      }
     | EQ       { $$=F_eq;       }
     | NEQ      { $$=F_neq;      }
     | LT       { $$=F_lt;       }
     | LE       { $$=F_le;       }
     | GT       { $$=F_gt;       }
     | GE       { $$=F_ge;       }
     | AND      { $$=F_and;      }
     | OR       { $$=F_or;       }
     ;


triop: ROTATE   { $$=F_rotate;   }
     | CAT      { $$=F_cat;      }
     | MODARRAY { $$=F_modarray; }
     ;

ids: id COMMA ids 
     { $$=MakeIds($1, NULL, ST_regular);
       IDS_NEXT($$)=$3;
     }
   | id 
     { $$=MakeIds($1, NULL, ST_regular);
     }
   ;

nums: NUM COMMA nums { $$=MakeNums($1, $3);   }
    | NUM            { $$=MakeNums($1, NULL); }
    ; 

dots: DOT COMMA dots { $$=$3-1;                }
    | DOT            { $$=KNOWN_DIM_OFFSET -1; }
    ;

returntypes: TYPE_VOID { $$=MakeTypes(T_void); }
           | types     { $$=$1;                }
           ;

types: type COMMA types
       { $$=$1;
         TYPES_NEXT($$)=$3; 
       }
     | type { $$=$1; }
     ;

varreturntypes: TYPE_VOID { $$=MakeTypes(T_void); }
              | vartypes  { $$=$1;                }
              ;

vartypes: type COMMA vartypes 
          { $$=$1;
            TYPES_NEXT($$)=$3;
          }
        | type { $$=$1; }
        | TYPE_DOTS
          { if ((F_extmoddec != file_kind) 
                   && (F_extclassdec != file_kind)
                     && (F_sib != file_kind)) {
              strcpy(yytext,"...");
              yyerror("syntax error");
            }
            else {
              $$=MakeTypes(T_dots);
            }
          }
        ;

typeNOudt_arr: localtype_main {$$ = $1;}
             | id COLON localtype
               { $$=$3;
                 TYPES_MOD($$)=$1;
               }
             ;

type: localtype { $$=$1; }
      | id COLON localtype
        { $$=$3;
          TYPES_MOD($$)=$1;
        }
      ;

localtype: localtype_main     {$$=$1;}
         | local_type_udt_arr {$$=$1;}
         ;

localtype_main: simpletype {$$ = $1; }
              | simpletype_main SQBR_L nums SQBR_R  
                { $$=GenComplexType($1,$3); }
              | simpletype_main  SQBR_L SQBR_R  
                { $$ = $1;
                  TYPES_DIM($$) = UNKNOWN_SHAPE;
                }
              | simpletype_main  SQBR_L QUESTION SQBR_R  
                { $$ = $1;
                  TYPES_DIM($$) = ARRAY_OR_SCALAR;
                }
              | simpletype_main SQBR_L dots SQBR_R
                { $$ = $1;
                  TYPES_DIM($$) = $3;
                }
              | id SQBR_L SQBR_R
                { $$=MakeTypes(T_user);
                  TYPES_NAME($$) = $1;
                  TYPES_DIM($$)  = UNKNOWN_SHAPE;
                }
              | id SQBR_L QUESTION SQBR_R
                { $$=MakeTypes(T_user);
                  TYPES_NAME($$) = $1;
                  TYPES_DIM($$)  = ARRAY_OR_SCALAR;
                }
              | id SQBR_L dots SQBR_R
                { $$=MakeTypes(T_user);
                  TYPES_NAME($$) = $1;
                  TYPES_DIM($$)  = $3;
                }
             ;

local_type_udt_arr: id SQBR_L nums SQBR_R
                    { $$=MakeTypes(T_user);
                      TYPES_NAME($$) = $1;
                      $$ = GenComplexType( $$, $3);
                    }
                  ;

simpletype: simpletype_main { $$ = $1; }
          | id 
            { $$=MakeTypes(T_user);
              TYPES_NAME($$) = $1;
            }
          ;

simpletype_main: TYPE_INT   { $$=MakeTypes(T_int);    }
               | TYPE_FLOAT { $$=MakeTypes(T_float);  }
               | TYPE_BOOL  { $$=MakeTypes(T_bool);   }
               | TYPE_CHAR  { $$=MakeTypes(T_char);   }
               | TYPE_DBL   { $$=MakeTypes(T_double); }
               ;


string: STR
        {
	  $$=$1;
	}
      | STR string
        {
	  $$=(char *)Malloc(strlen($1)+strlen($2)+1);
	  strcpy($$, $1);
	  strcat($$, $2);
	  FREE($1);
	  FREE($2);
	}
      ;


/*       BRUSH END    */


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
         SIB_LINKSTYLE($$) = $1;
         SIB_LINKWITH($$)  = $2;

         DBUG_PRINT("GENSIB",("%s"P_FORMAT,
                             mdb_nodetype[$$->nodetype],$$));
       }
   ;

sibheader: LT id GT
             {
               mod_name=$2;
               file_kind=F_sib;
               $$=0;
             }
         | LT id COLON ALL GT
             {
               mod_name=$2;
               file_kind=F_sib;
               $$=1;
             }
         ;

siblinkwith: LINKWITH siblinklist {$$=$2;}
             | {$$=NULL;}
	          ;

siblinklist: siblinkliststatus string sibsublinklist COMMA siblinklist
             {
               $$=MakeDeps($2, NULL, NULL, $1, $3, $5);
             }
           | siblinkliststatus string sibsublinklist 
             {
               $$=MakeDeps($2, NULL, NULL, $1, $3, NULL);
             }
           ;

siblinkliststatus: EXTERN 
             {
               $$=ST_external;
             }
	        | LINKWITH
             {
               $$=ST_system;
             }
	        |
             {
               $$=ST_sac;
             }
	        ;

sibsublinklist: BRACE_L siblinklist BRACE_R
             {
               $$=$2;
             }
           | 
             {
               $$=NULL;
             }
           ;


sibtypes: sibtype sibtypes
            {
              $$=$2;
              TYPEDEF_NEXT($1)=SIB_TYPES($2);
              SIB_TYPES($$) = $1;
            }
        | sibobjs
            {
              $$=$1;
            } 
          ;

sibtype: sibevclass TYPEDEF type id SEMIC sibpragmas
           {
             $$=MakeTypedef($4, NULL, $3, $1, NULL);
             TYPEDEF_STATUS($$)=ST_imported;
             TYPEDEF_PRAGMA($$)=$6;

            DBUG_PRINT("GENSIB",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        TYPEDEF_TYPE($$), ItemName($$)));
           }
       | sibevclass TYPEDEF type id COLON id SEMIC sibpragmas
           {
             $$=MakeTypedef($6, $4, $3, $1, NULL);
             TYPEDEF_STATUS($$)=ST_imported;
             TYPEDEF_PRAGMA($$)=$8;

            DBUG_PRINT("GENSIB",
                       ("%s:"P_FORMAT","P_FORMAT", Id: class %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        TYPEDEF_TYPE($$), ItemName($$)));
           }
       | sibevclass TYPEDEF IMPLICIT id SEMIC sibpragmas
           {
             $$=MakeTypedef($4, NULL,
                            MakeType(T_hidden, 0, NULL, NULL, NULL),
                            $1, NULL);
             TYPEDEF_STATUS($$)=ST_imported;
             TYPEDEF_PRAGMA($$)=$6;

            DBUG_PRINT("GENSIB",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        TYPEDEF_TYPE($$), ItemName($$)));
           }
       | sibevclass TYPEDEF IMPLICIT id COLON id SEMIC sibpragmas
           {
             $$=MakeTypedef($6, $4,
                            MakeType(T_hidden, 0, NULL, NULL, NULL),
                            $1, NULL);
             TYPEDEF_STATUS($$)=ST_imported;
             TYPEDEF_PRAGMA($$)=$8;

            DBUG_PRINT("GENSIB",
                       ("%s:"P_FORMAT","P_FORMAT", Id: class %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        TYPEDEF_TYPE($$), ItemName($$)));
           }
        ;

sibevclass: CLASSTYPE {$$=ST_unique;}
          |           {$$=ST_regular;}

sibobjs: sibobj sibobjs
            {
              $$=$2;
              OBJDEF_NEXT($1)=SIB_OBJS($2);
              SIB_OBJS($2)=$1;
            }
        | sibfuns
            {
              $$=$1;
            } 
          ;

sibobj: OBJDEF type id SEMIC sibpragmas
          {
            $$=MakeObjdef($3, NULL, $2, NULL, NULL);
            OBJDEF_PRAGMA($$)=$5;
            OBJDEF_STATUS($$)=ST_imported;
            
            DBUG_PRINT("GENSIB",
                       ("%s:"P_FORMAT","P_FORMAT", Id: class %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        OBJDEF_TYPE($$), ItemName($$)));
          }
      | OBJDEF type id COLON id SEMIC sibpragmas
          {
            $$=MakeObjdef($5, $3, $2, NULL, NULL);
            OBJDEF_PRAGMA($$)=$7;
            OBJDEF_STATUS($$)=ST_imported;
            
            DBUG_PRINT("GENSIB",
                       ("%s:"P_FORMAT","P_FORMAT", Id: class %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        OBJDEF_TYPE($$), ItemName($$)));
          }
       ;

sibfuns: sibfun sibfuns
            {
              $$ = $2;
              FUNDEF_NEXT($1)=SIB_FUNS($2);
              SIB_FUNS($$) = $1;
            }
       | 
            {
              $$ = MakeSib( mod_name, 0, NULL, NULL, NULL, NULL);
            }
       ;

sibfun: sibevmarker varreturntypes fun_name 
        BRACKET_L sibarglist BRACKET_R sibfunbody sibpragmas
          {
            $$=MakeFundef($3, NULL, $2, $5, $7, NULL);
            switch ($1)
            {
            case 0:
              FUNDEF_STATUS($$)=ST_imported;
              FUNDEF_INLINE($$)=0;
              break;
            case 1:
              FUNDEF_STATUS($$)=ST_imported;
              FUNDEF_INLINE($$)=1;
              break;
            case 2:
              FUNDEF_STATUS($$)=ST_classfun;
              FUNDEF_INLINE($$)=0;
              break;
            }
            FUNDEF_PRAGMA($$)=$8;

           DBUG_PRINT("GENSIB",("%s"P_FORMAT"SibFun %s",
                               mdb_nodetype[$$->nodetype],$$,
                               ItemName($$)));
          }
      | sibevmarker varreturntypes id COLON fun_name 
        BRACKET_L sibarglist BRACKET_R sibfunbody sibpragmas
          {
            $$=MakeFundef($5, $3, $2, $7, $9, NULL);
            switch ($1)
            {
            case 0:
              FUNDEF_STATUS($$)=ST_imported;
              FUNDEF_INLINE($$)=0;
              break;
            case 1:
              FUNDEF_STATUS($$)=ST_imported;
              FUNDEF_INLINE($$)=1;
              break;
            case 2:
              FUNDEF_STATUS($$)=ST_classfun;
              FUNDEF_INLINE($$)=0;
              break;
            }
            FUNDEF_PRAGMA($$)=$10;

           DBUG_PRINT("GENSIB",("%s"P_FORMAT"SibFun %s",
                               mdb_nodetype[$$->nodetype],$$,
                               ItemName($$)));

          }
        ;

sibevmarker: INLINE    {$$=1;} 
        | CLASSTYPE {$$=2;}
        |           {$$=0;}
        ;

sibarglist: sibargs {$$=$1;}
          |         {$$=NULL;}
          ;

sibargs: sibarg COMMA sibargs
           {
             $$=$1;
             ARG_NEXT($$)=$3;
           }
       | sibarg
           {
             $$=$1;
           }
       ;

sibarg: type sibreference sibparam
          {
            $$=MakeArg($3, $1, ST_regular, $2, NULL);

              DBUG_PRINT("GENSIB",
                         ("%s: "P_FORMAT", Id: %s, Attrib: %d, Status: %d  ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          CHECK_NULL( ARG_NAME($$)), ARG_ATTRIB($$), ARG_STATUS($$)));
          }
      | TYPE_DOTS
          {
            $$=MakeArg(NULL, MakeTypes(T_dots), ST_regular, ST_regular, NULL); 

              DBUG_PRINT("GENSIB",
                         ("%s: "P_FORMAT", ... , Attrib: %d, Status: %d  ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          ARG_ATTRIB($$), ARG_STATUS($$)));
          }
      ;

sibparam: id
            {
              $$=$1;
            }
        |
            {
              $$=NULL;
            }
        ;


sibreference: BRACKET_L AMPERS BRACKET_R
                 {
                   $$=ST_readonly_reference;
                 }
             | AMPERS
                 {
                   $$=ST_reference;
                 }
             |
                 {
                   $$=ST_regular;
                 }
             ;

sibfunbody: exprblock
              {
                $$=$1;
              }
          | SEMIC
              {
                $$=NULL;
              }
          ;


sibpragmas: sibpragmalist
            {
              $$=store_pragma;
              store_pragma=NULL;
            }
          |
            {
              $$=NULL;
            }
          ;

sibpragmalist: sibpragmalist sibpragma
             | sibpragma
             ;

sibpragma: pragma
         | PRAGMA TYPES modnames
           {
             if (store_pragma==NULL) store_pragma=MakePragma();
             PRAGMA_NEEDTYPES(store_pragma)=$3;
           }
         | PRAGMA FUNS sibfunlist
           {
             if (store_pragma==NULL) store_pragma=MakePragma();
             PRAGMA_NEEDFUNS(store_pragma)=$3; 
           }
         | PRAGMA EXTERN id
           {
             if (store_pragma==NULL) store_pragma=MakePragma();
             PRAGMA_LINKMOD(store_pragma)=$3; 
           }
         ;

sibfunlist: sibfunlistentry COMMA sibfunlist
            {
              $$=$1;
              FUNDEF_NEXT($$)=$3;
            }
          | sibfunlistentry
            {
              $$=$1;
            }
          ;

sibfunlistentry: id BRACKET_L sibarglist BRACKET_R
                 {
                   $$=MakeFundef($1, NULL,
                                 MakeType(T_unknown, 0, NULL, NULL, NULL),
                                 $3, NULL, NULL);
                   FUNDEF_STATUS($$)=ST_imported;
                   
                   DBUG_PRINT("GENSIB",("%s"P_FORMAT"SibNeedFun %s",
                                       mdb_nodetype[$$->nodetype],$$,
                                       ItemName($$)));
                }
               | id COLON fun_name BRACKET_L sibarglist BRACKET_R
                 {
                   $$=MakeFundef($3, $1,
                                 MakeType(T_unknown, 0, NULL, NULL, NULL),
                                 $5, NULL, NULL);

                   FUNDEF_STATUS($$)=ST_imported;
                   
                   DBUG_PRINT("GENSIB",("%s"P_FORMAT"SibNeedFun %s",
                                       mdb_nodetype[$$->nodetype],$$,
                                       ItemName($$)));
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
           $$=RSCMakeTargetListEntry($2, $4, $5, $6);
	 }
       | /* empty */
         {
	   $$=NULL;
	 }
       ;

inherits: COLON ID COLON inherits
          {
            $$=MakeIds($2, NULL, ST_regular);
            IDS_NEXT($$)=$4;
          }
        | /* empty */
          {
	    $$=NULL;
	  } 
        ;

resources: ID COLON LET string resources
           {
             $$=RSCMakeResourceListEntry($1, $4, 0, 0, $5);
	   }
         | ID ADDON string resources
           {
             $$=RSCMakeResourceListEntry($1, $3, 0, 1, $4);
	   }
         | 
           ID COLON LET OPTION resources
           {
             $$=RSCMakeResourceListEntry($1, $4, 0, 0, $5);
	   }
         | ID ADDON OPTION resources
           {
             $$=RSCMakeResourceListEntry($1, $3, 0, 1, $4);
	   }
         | ID COLON LET ID resources
           {
             $$=RSCMakeResourceListEntry($1, $4, 0, 0, $5);
	   }
         | ID ADDON ID resources
           {
             $$=RSCMakeResourceListEntry($1, $3, 0, 1, $4);
	   }
         | ID COLON LET PRIVATEID resources
           {
             $$=RSCMakeResourceListEntry($1, $4, 0, 0, $5);
	   }
         | 
           ID ADDON PRIVATEID resources
           {
             $$=RSCMakeResourceListEntry($1, $3, 0, 1, $4);
	   }
         | 
           ID COLON LET NUM resources
           {
             $$=RSCMakeResourceListEntry($1, NULL, $4, 0, $5);
	   }
         | 
           ID ADDON NUM resources
           {
             $$=RSCMakeResourceListEntry($1, NULL, $3, 1, $4);
	   }
         |  /* empty */
           {
	     $$=NULL;
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


int yyerror(char *errname)
{
  ABORT(linenum,("%s at '%s`", errname, yytext));
}


int CountArguments(node *args)
{
  int res=0;
  
  while (args!=NULL){
    res++;
    args=EXPRS_NEXT(args);
  }
  
  return(res);
}



/***
 ***  string2array
 ***/

node *string2array(char *str)
{
  node *new_exprs;
  int i, cnt;
  char *funname;
  node *array;
  node *len_exprs;
  
  DBUG_ENTER("string2array");
  
  new_exprs=MakeExprs(MakeChar('\0'), NULL);

  cnt=0;
  
  for (i=strlen(str)-1; i>=0; i--)
  {
    if ((i>0) && (str[i-1]=='\\'))
    {
      switch (str[i])
      {
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
    else
    {
      new_exprs=MakeExprs(MakeChar(str[i]), new_exprs);
    }
    
    cnt+=1;
  }

  len_exprs=MakeExprs(MakeNum(cnt), NULL);
  array=MakeArray(new_exprs);

#ifndef CHAR_ARRAY_NOT_AS_STRING
  ARRAY_STRING(array)=str;
#endif  /*  CHAR_ARRAY_AS_STRING  */

  DBUG_RETURN(MakeAp(StringCopy("to_string"), NULL, MakeExprs(array, len_exprs))); 
}


/*
 *   Altes aus sac.21
 *
 */

types *GenComplexType( types *types, nums *numsp)
{
  int *destptr;
  nums *tmp;

  DBUG_ENTER("GenComplexType");

  types->shpseg=GEN_NODE(shpseg);
  destptr=types->shpseg->shp;
  do {
    types->dim++;
    *destptr++=numsp->num;
    DBUG_PRINT("GENTREE",("shape-element: %d",numsp->num));
    tmp=numsp;
    numsp=numsp->next;
    FREE(tmp);
  } while (numsp != NULL);
  DBUG_RETURN(types);
}

node *GenVardec( types *type, ids *ids_p)
{
  node *tmp, *vardec_p;
  ids *tmp2;
  types *type_p;

  DBUG_ENTER("GenVardec");

  vardec_p=NULL;
  do
   {
     tmp=MakeNode(N_vardec);
     tmp->node[0]=vardec_p;
     vardec_p=tmp;
     type_p=MakeType(T_int, 0, NULL, NULL, NULL);
     type_p=(types*) memcpy((void*)type_p, (void*)type, sizeof(types)) ;
     vardec_p->info.types=type_p;
     vardec_p->info.types->id=ids_p->id;
     tmp2=ids_p;
     ids_p=ids_p->next;
     FREE(tmp2);
  } while (ids_p != NULL);
  DBUG_RETURN(vardec_p);
}

/*
 *
 *  functionname  : Append
 *  arguments     : 1) node to append to; nodetype: N_assign or N_block
 *                  2) node to append
 *  description   : appends node 2) to node 1) and creates a new N_assign node
 *                  if necessary
 *  global vars   : ---
 *  internal funs : MakeNode
 *  external funs : ---
 *  macros        : DBUG..., P_FORMAT
 *
 *  remarks       :
 *
 */
node *Append(node *target_node, node *append_node)
{
   node *tmp;
   
   DBUG_ENTER("Append");
   if (N_block == target_node->nodetype)
      tmp=target_node->node[0];
   else
      tmp=target_node;
   if (N_assign == tmp->nodetype) 
   {
      while( tmp->node[1] )      /* look for last N_assign node */
        tmp=tmp->node[1];
   
      if (N_assign != append_node->nodetype)
      {
         tmp->node[1]=MakeNode(N_assign);
         tmp->node[1]->node[0]=append_node;
      }
      else
         tmp->node[1]=append_node;
      
      DBUG_PRINT("GENTREE",("%s"P_FORMAT": %s"P_FORMAT" %s"P_FORMAT,
                            mdb_nodetype[tmp->nodetype],tmp,
                            mdb_nodetype[tmp->node[0]->nodetype],tmp->node[0],
                            mdb_nodetype[tmp->node[1]->nodetype],tmp->node[1]));
   }
   else
   {  /* target_node has type N_empty */

      FREE(tmp);     /* delete node of type N_empty */
      if (N_assign != append_node->nodetype)
      {
         tmp=MakeNode(N_assign);  
         tmp->node[0]=append_node;
         DBUG_PRINT("GENTREE",("%s"P_FORMAT": %s"P_FORMAT,
                               mdb_nodetype[tmp->nodetype],tmp,
                               mdb_nodetype[tmp->node[0]->nodetype],
                               tmp->node[0]));
      } 
      else
      {
         tmp=append_node;
         
         DBUG_PRINT("GENTREE",("%s"P_FORMAT,
                               mdb_nodetype[tmp->nodetype],tmp));
      }
   }
      
   DBUG_RETURN(target_node);
}
