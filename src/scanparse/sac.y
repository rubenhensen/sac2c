%{

/*
 *
 * $Log$
 * Revision 1.73  1995/07/14 14:51:34  sbs
 * module_name inserted in fundec:
 *
 * Revision 1.72  1995/07/14  11:58:30  sbs
 * keyword inline moved to the beginning of function declaration
 *
 * Revision 1.71  1995/07/14  09:48:22  hw
 * changed rule fundec( now name of a function can be a primitive
 *  function name)
 *
 * Revision 1.70  1995/07/13  15:24:36  hw
 * the neutral element of a primitive function can be spezified
 * in the fold-part of a with-loop now
 *
 * Revision 1.69  1995/07/11  10:02:45  cg
 * bug in parsing typedefs fixed.
 *
 * Revision 1.68  1995/07/11  09:00:45  cg
 * most grammar version 0.6 features integrated.
 *
 * Revision 1.67  1995/07/06  17:27:05  cg
 * new Tokens ASSIGN and AMPERS added
 *
 * Revision 1.66  1995/07/06  17:01:25  hw
 * - behind 'all' in the import declaration has to be a semicolon now
 * - behind 'while' of a do-loop has to be a semicolon now
 * - userdefined functins can have the same name like primitive functions now
 *
 * Revision 1.65  1995/07/04  11:34:49  hw
 * - parsing of primitive functions itod, ftod, dtoi, dtof inserted
 * - parsing of 'double constants' inserted
 *
 * Revision 1.64  1995/06/30  11:27:21  hw
 * - new primitive functions "ftoi" & "itof" inserted
 * - token C_KEYWORD for not used c-keywords inserted
 *
 * Revision 1.63  1995/06/09  10:01:34  cg
 * inline warning deleted.
 *
 * Revision 1.62  1995/05/31  13:03:11  hw
 * node[1] of N_foldfun is not an N_exprs anymore
 *
 * Revision 1.61  1995/05/31  11:38:23  hw
 * changed rule for 'fold' (case N_foldfun) ( exchanged 'expr' with 'exprORarray' )
 *
 * Revision 1.60  1995/05/30  08:22:28  cg
 * sac_grammar version 0.5 tokens inserted.
 *
 * Revision 1.59  1995/05/30  07:17:18  hw
 * - N_foldfun has now two child nodes (node[0]: body of with-loop
 *    node[1]: expr to compute neutral element of userdefined function
 *             that is used in fold
 * - name of userdefined function that is used in 'fold' is stored
 *   in info.fun_name.id of N_foldfun node.
 *   the modul name is stored in info.fun_name.id_mod , if any
 *
 * Revision 1.58  1995/05/23  07:49:23  hw
 * bug fixed in creation of "psi" (A[...] => psi(...,A) )
 *
 * Revision 1.57  1995/05/22  14:25:45  hw
 * - bug fixed in creation of "psi" (A[7] will be transformed to psi([7],A),
 *    NOTE: A[b] will be transformed to psi(b,A) !!!!!! )
 * - set arg_node->flag to 1 if function should be inlined
 *
 * Revision 1.56  1995/04/26  16:06:15  hw
 * bug fixed in MakeLet ( identifier will be copied now)
 *
 * Revision 1.55  1995/04/06  13:56:09  hw
 * typeinformation of sac_main is build with MakeTypes now
 *
 * Revision 1.54  1995/03/13  16:59:59  hw
 * -  the identifier of node N_id, N_pre and N_post is stored
 *    in 'info.ids->id' instead of 'info.id' now
 *
 * Revision 1.53  1995/03/07  19:13:04  hw
 *  - new rule for parsing of arrays ( no recursion anymore)
 *  - changed rule for parsing of 'A[..]'
 *   (now 'A[1,2,3] == A[[1,2,3]] == psi([1,2,3],A)' and
 *        'A[x] == psi(x,A)' )
 *
 * Revision 1.52  1995/03/01  16:28:00  hw
 * changed N_generator ( the name of the index vector is
 * 	now stored in info.ids)
 *
 * Revision 1.51  1995/02/20  09:35:47  hw
 * inserted rule expr -> MINUS ID %prec UMINUS
 *
 * Revision 1.50  1995/02/14  12:42:42  sbs
 * New Prf's allowed in fold( prf)!
 *
 * Revision 1.49  1995/02/14  12:00:42  sbs
 * fold inserted
 *
 * Revision 1.48  1995/02/14  10:11:33  hw
 * primitive function "not" inserted
 *
 * Revision 1.47  1995/02/13  16:41:21  sbs
 * bug in DBUG-PRINT of N_prf psi fixed.
 *
 * Revision 1.46  1995/02/09  09:01:56  hw
 * bug fixed in creation of cat, rotate & psi
 *
 * Revision 1.45  1995/02/02  14:57:46  hw
 * changed N_prf node
 *
 * Revision 1.44  1995/01/25  16:01:45  sbs
 * Priority of cast changed to %right CAST
 *
 * Revision 1.43  1995/01/09  12:16:07  sbs
 * bug fixed: mod_name set earlier in moddec.
 *
 * Revision 1.42  1995/01/06  19:05:51  sbs
 * no_mod_ext pragma deleted & extern declaration inserted
 *
 * Revision 1.41  1995/01/06  17:51:45  sbs
 * no_mod_ext pragma inserted
 *
 * Revision 1.40  1995/01/05  12:46:51  sbs
 * string inserted
 *
 * Revision 1.39  1995/01/03  17:57:00  hw
 * changed constructing of assigns to handle converted for-loops correctly
 * inserted DBUG_PRINT's in function Append.
 * bug fixed in DBUG_PRINT of rule 'forassign' part FOR ...
 *
 * Revision 1.38  1994/12/31  15:11:27  sbs
 * changed the structure of N_explist nodes:
 * node[0] contains now a typedef chain for implicit types!
 * 2) modul names inserted for export-decls
 *
 * Revision 1.37  1994/12/31  14:10:39  sbs
 * modul selection inserted for types and function applications
 *
 * Revision 1.36  1994/12/21  13:15:37  sbs
 * for fundec nnode decremented (1/0)
 *
 * Revision 1.35  1994/12/21  11:45:16  hw
 * added DBUG_PRINT in simpletype
 * changed N_typedef : now the name of the defined type is put in info.types->id
 *
 * Revision 1.34  1994/12/20  17:43:40  hw
 * deleted function MakeNode (now in tree.c)
 * deleted nodes N_addon, etc. they are converted to N_let
 *
 * Revision 1.33  1994/12/20  13:10:47  sbs
 * typedef bug fixed (types->id => types->name)
 *
 * Revision 1.32  1994/12/20  11:24:29  sbs
 * decl_tree inserted
 *
 * Revision 1.31  1994/12/16  14:34:28  sbs
 * moddec  and start_token inserted
 *
 * Revision 1.30  1994/12/15  17:29:16  sbs
 * bug in fundef2 inline fixed
 *
 * Revision 1.29  1994/12/15  17:09:16  sbs
 * typecasts as (: type) inserted.
 * The colon is due the context dependencies resulting from
 * expressions like ( ID ) [ expr ]
 * or ( ID ) + expr
 * or ( ID ) - expr
 * In later versions this has to be changed to a context sensitive
 * version which distinguishes between ID and TYPEID.....:-<
 *
 * Revision 1.28  1994/12/14  16:52:46  sbs
 * user defined types integrated :->>>>
 *
 * Revision 1.27  1994/12/14  13:57:26  sbs
 * bug in expressionblock2 fixed (superflouus SEMIC BRACE_R)
 *
 * Revision 1.26  1994/12/14  12:42:08  hw
 * error fixed in rule 'fundef2'
 *
 * Revision 1.25  1994/12/14  10:09:04  sbs
 * modul, imports and typedefs inserted;
 * typedeclarations with user defined types
 * are not yet possible:-(
 *
 * Revision 1.24  1994/12/09  15:19:14  hw
 * added rule expr -> expr EQ expr  (this one was missing :-(  )
 *
 * Revision 1.23  1994/12/08  17:46:57  sbs
 * modules inserted
 *
 * Revision 1.22  1994/12/08  14:15:04  hw
 * changed node[1] and node[2] of N_fundef
 * now node[1] points to the next function and
 *
 * node[2] points to the formal parameters
 * added rule to parse functions without parameters
 *
 * Revision 1.21  1994/12/07  17:37:15  sbs
 * err1 fixed: multiple vardecs
 *
 * Revision 1.20  1994/11/29  10:56:38  hw
 * added assignment to ids->node in rule "ids"
 *
 * Revision 1.19  1994/11/28  13:26:02  hw
 * error in "exprblock" fixed
 *
 * Revision 1.18  1994/11/23  09:45:00  hw
 * added rule: expr -> TRUE ; expr -> FALSE
 *
 * Revision 1.17  1994/11/22  14:22:53  hw
 * error in declaration of arrays without shape fixed
 *
 * Revision 1.16  1994/11/22  13:42:58  hw
 * - error fixed in Append
 * - changed rule expr -> ID [ exprs ]  into expr -> expr [ expr ]
 *
 * Revision 1.15  1994/11/22  11:41:25  hw
 * - declaration of arrays without shape is possible now
 * - changed exprblock assignblock retassignblock, because of changes in assigns
 *
 * Revision 1.14  1994/11/21  17:59:42  hw
 * inserted linenumber to each node
 *
 * Revision 1.13  1994/11/18  16:55:52  hw
 * error in assignblock fixed
 *
 * Revision 1.12  1994/11/18  14:54:56  hw
 * changed function Append
 * added function MakeEmptyBlock
 *
 * Revision 1.11  1994/11/17  16:53:57  hw
 * bug fixed in Append
 * all N_return nodes are children of N_assign nodes now.
 *
 * Revision 1.10  1994/11/15  16:55:53  hw
 * changed assignblock rule
 * converted for loop to while loop
 * => eliminated N_for
 *
 * Revision 1.9  1994/11/11  13:48:31  hw
 * added prefix & postfix increment-& decrementation
 * added new nodes: N_pre N_post N_inc N_dec
 *
 * Revision 1.8  1994/11/10  17:31:54  sbs
 * error in modarray fixed
 *
 * Revision 1.7  1994/11/10  17:30:44  sbs
 * Expr inserted in modarray + genarray
 *
 * Revision 1.6  1994/11/10  15:19:42  sbs
 * RCS-header inserted
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <memory.h>
#include "dbug.h"
#include "tree.h"
#include "my_debug.h"
#include "internal_lib.h" /* for use of StringCopy */

extern int linenum;
extern char yytext[];

int indent, i;
node *syntax_tree;
node *decl_tree;

static char *mod_name;
static enum { F_prog, F_modimp, F_classimp, F_moddec, F_extmoddec,
              F_classdec, F_extclassdec } file_kind = F_prog;
                      /* used to distinguish the different kinds of files  */
                      /* which are parsed with this single parser          */


%}

%union {
	 nodetype	 nodetype;
         id              *id;
         ids             *ids;
         types           *types;
         node            *node;
         int             cint;
         float           cfloat;
         double          cdbl;
         nums            *nums;
         prf             prf;
       }

%token PARSE_PRG, PARSE_DEC
%token BRACE_L, BRACE_R, BRACKET_L, BRACKET_R, SQBR_L, SQBR_R, COLON, SEMIC,
       COMMA, ASSIGN, AMPERS,
       INLINE, LET, TYPEDEF, CONSTDEF, OBJDEF,
       F2I, F2D, I2F,I2D, D2I, D2F,
       INC, DEC, ADDON, SUBON, MULON, DIVON,
       K_MAIN, RETURN, IF, ELSE, DO, WHILE, FOR, WITH, FOLD,
       MODDEC, MODIMP, CLASSDEC, IMPORT, ALL, IMPLICIT, EXPLICIT, TYPES, FUNS,
       OWN, CONSTANTS, GLOBAL, OBJECTS, CLASSIMP,
       ARRAY,SC, TRUE, FALSE, EXTERN, C_KEYWORD
%token <id> ID, STR,AND, OR, EQ, NEQ, NOT, LE, LT, GE, GT, MUL, DIV, PLUS, MINUS,
            RESHAPE, SHAPE, TAKE, DROP, DIM, ROTATE,CAT,PSI,GENARRAY, MODARRAY
%token <types> TYPE_INT, TYPE_FLOAT, TYPE_BOOL, TYPE_STR, TYPE_UNS, TYPE_SHORT,
               TYPE_LONG, TYPE_CHAR, TYPE_DBL, TYPE_VOID, TYPE_DOTS
%token <cint> NUM
%token <cfloat> FLOAT
%token <cdbl> DOUBLE

%type <prf> foldop
%type <nodetype> modclass
%type <cint> evextern
%type <ids> ids
%type <id> fun_name
%type <nums> nums
%type <types> localtype, type, types, simpletype, complextype, returntypes;
%type <node> arg, args, fundefs, fundef, main, prg, modimp,
             argtypes, argtype,
             typedefs, typedef, defs, def2, def3, def4, fundef2,
             objdefs, objdef, exprblock, exprblock2,
             exprblock3, assign, assigns, assignblock, letassign, retassign,
             selassign, forassign, retassignblock, optretassign
             apl, expr, exprs, monop, binop, triop, 
             conexpr, generator, unaryop,
             moddec, expdesc, expdesc2, expdesc3, expdesc4, fundecs, fundec,
	     exptypes, exptype, objdecs, objdec, evimport
             imptypes, imptype, import, imports, impdesc, impdesc2, impdesc3,
             impdesc4, array, exprORarray, exprsNOarray, foldfun;

%left OR
%left AND
%left EQ, NEQ
%left LE, LT, GE, GT
%left PLUS, MINUS
%left MUL, DIV
%left TAKE, DROP, RESHAPE
%left SQBR_L
%right CAST
%right NOT
%nonassoc UMINUS

%start file

%%

file:   PARSE_PRG prg {syntax_tree=$2;}
	| PARSE_PRG modimp {syntax_tree=$2;}
	| PARSE_DEC moddec {decl_tree=$2;}
	;

moddec: modclass evextern ID COLON { if($2 == 1) mod_name=NULL;
				     else mod_name=$3; }
				  evimport OWN COLON expdesc
	  { $$=MakeNode($1);
	    $$->info.fun_name.id=$3;
	    $$->info.fun_name.id_mod=mod_name;
       $$->node[0]=$9;
       $$->node[1]=$6;
            if($$->node[1] != NULL) {
              $$->nnode=2;
              DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , %s"P_FORMAT" %s," P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.fun_name.id,
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0],
                        mdb_nodetype[ $$->node[1]->nodetype ], $$->node[1]));
	    }
	    else {
              $$->nnode=1;
              DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , %s"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.fun_name.id,
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0]));
	    };
	  }

modclass: MODDEC {$$=N_moddec; file_kind=F_moddec;}
	  | CLASSDEC {$$=N_classdec; file_kind=F_classdec;}
	  ;

evextern: EXTERN {$$=1; file_kind++;}
	  | {$$=0;}
	  ;

evimport: imports {$$=$1;}
	  | {$$=NULL;}
	  ;

imports: import imports { $$=$1;
                          $$->node[0]=$2;
                          $$->nnode+=1;
                        }

	| import { $$=$1; }
	;

import: IMPORT ID COLON impdesc { $$=$4; $$->info.id=$2; }
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
	| BRACE_L impdesc2 { $$=$2; }
	;

impdesc2: EXPLICIT TYPES COLON ids SEMIC impdesc3
            { $$=$6;
              $$->node[2]=(node *)$4; /* dirty trick for keeping ids !*/
            }
	| impdesc3 { $$=$1; }
	;

impdesc3: GLOBAL OBJECTS COLON ids SEMIC impdesc4
            { $$=$6;
              $$->node[4]=(node *)$4; /* dirty trick for keeping ids !*/
            }
	| impdesc4 { $$=$1; }
	;

impdesc4: FUNS COLON ids SEMIC BRACE_R
            { $$=MakeNode( N_implist );
              $$->node[3]=(node *)$3; /* dirty trick for keeping ids !*/

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
           $$->nnode++;
         }
        | BRACE_L expdesc2 {$$=$2;}
        ;

expdesc2: EXPLICIT TYPES COLON exptypes expdesc3
          { $$=$5;
            $$->node[1]=$4;
            $$->nnode++;
          }
        | expdesc3 {$$=$1;}
        ;

expdesc3: GLOBAL OBJECTS COLON objdecs expdesc4
          { $$=$5;
            $$->node[3]=$4;
            $$->nnode++;
          }
        | expdesc4 {$$=$1;}
        ;

expdesc4: FUNS COLON fundecs BRACE_R
          { $$=MakeNode(N_explist);
            $$->node[2]=$3;
            $$->nnode=1;

            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$));
          }
        | BRACE_R
          { $$=MakeNode(N_explist);
            $$->nnode=0;
         
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$));
          }
        ;

imptypes: imptype imptypes {$$=$1;
                            $1->node[0]=$2;
                            $1->nnode+=1;
                           }
	| imptype {$$=$1;}
	;

imptype: ID SEMIC 
              {
              $$=MakeNode(N_typedef);
              $$->info.types=MakeTypes(T_hidden);
              DBUG_PRINT("GENTREE",("type:"P_FORMAT" %s",
                                     $$->info.types, mdb_type[$$->info.types->simpletype]));
              $$->info.types->id=$1;
              $$->info.types->id_mod=mod_name;

              DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$,
                        $$->info.types, $$->info.types->id));

             }
             ;

exptypes: exptype exptypes {$$=$1;
                            $1->node[0]=$2;
                            $1->nnode+=1;
                           }
	| exptype {$$=$1;}
	;

exptype: ID LET type SEMIC
           { $$=MakeNode(N_typedef);
            $$->info.types=$3;
            $$->info.types->id=$1;
            $$->info.types->id_mod=mod_name;

            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        $$->info.types, $$->info.types->id));
          }
         ;

objdecs: objdec objdecs    {$$=$1;
                            $$->node[0]=$2;
                            $$->nnode+=1;
                           }
	| objdec {$$=$1;}
	;

objdec: type ID SEMIC
           { $$=MakeNode(N_objdef);
             $$->info.types=$1;
             $$->info.types->id=$2;   /* object name */
             $$->info.types->id_mod=mod_name;
             $$->node[1]=NULL;	/* there is no object init here! */
             $$->nnode=0;
             
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        $$->info.types, $$->info.types->id));
          }
         ;

fundecs: fundec fundecs { $1->node[1]=$2;
                          $1->nnode+=1;
                          $$=$1;
                        }
	| fundec {$$=$1;}
	;

fundec: returntypes fun_name BRACKET_L argtypes BRACKET_R SEMIC
          { $$=MakeNode(N_fundef);
            $$->node[0]=NULL;	/* there is no function body here! */
            $$->node[2]=$4;	/* argument declarations */
            $$->nnode=1;
            $$->info.types=$1;
            $$->info.types->id=$2; /* function name */
            $$->info.types->id_mod=mod_name; /* modul name */

            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , NULL body,  %s" P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.types->id,
                        mdb_nodetype[ $$->node[2]->nodetype ], $$->node[2]));
          }
	| returntypes fun_name BRACKET_L BRACKET_R SEMIC
          { $$=MakeNode(N_fundef);
            $$->node[0]=NULL;   /* there is no function body here! */
            $$->nnode=0;
            $$->info.types=$1;
            $$->info.types->id=$2; /* function name */
            $$->info.types->id_mod=mod_name; /* modul name */

            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , NULL body",
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.types->id));
          }
	;

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
                            mdb_nodetype[ $$->node[2]->nodetype ], $$->node[2]));
              }
	;

modimp: impclass ID {  mod_name=$2; } COLON defs
          { $$=$5;
            $$->info.id=$2;
            if (file_kind==F_classimp)
               $$->node[4]=MakeNode(N_isclass);
          }
	;

impclass : MODIMP   { file_kind=F_modimp;}
         | CLASSIMP { file_kind=F_classimp;}
         ;

prg: defs { $$=$1; }
        ;

typedefs: typedef typedefs {$$=$1;
                            $1->node[0]=$2;
                            $1->nnode+=1;
                           }
	| typedef {$$=$1; }
	;

typedef: TYPEDEF type ID SEMIC 
          { $$=MakeNode(N_typedef);
            $$->info.types=$2;
            $$->info.types->id=$3;
            if ((file_kind==F_classimp) && (!strcmp((char*) $3, mod_name)))
               $$->info.types->attrib=ST_unique;
            
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        $$->info.types, $$->info.types->id));
          }

objdefs: objdef objdefs {$$=$1;
                         $$->node[0]=$2;
                         $$->nnode+=1;
                           }
	| objdef {$$=$1; }
	;

objdef: OBJDEF type ID LET expr SEMIC 
          { $$=MakeNode(N_objdef);
            $$->info.types=$2;
            $$->info.types->id=$3;
            $$->node[1]=$5;
            $$->nnode=0;
            
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        $$->info.types, $$->info.types->id));
          }

fundefs: fundef fundefs { $$=$1;
                          $$->node[1]=$2;
                          $$->nnode+=1;
                        }
	| fundef {$$=$1;}
	| main {$$=$1;}
	;

fundef: returntypes fun_name BRACKET_L fundef2 
        {  $$=$4;
           $$->info.types=$1;          		/*  result type(s) */
           $$->info.types->id=$2;      		/*  function name */
           $$->info.types->id_mod=mod_name;     /*  module name */
        }
	| INLINE returntypes fun_name BRACKET_L fundef2
            {id *function_name;

             $$=$5;
             $$->info.types=$2;          /* result type(s) */
             $$->info.types->id=$3;      /*  function name */
             $$->info.types->id_mod=mod_name;      /*  module name */
             $$->flag=1;                 /* flag to sign, that this function
                                          * should be inlined
                                          */             
            }

fundef2: args BRACKET_R  {$$=MakeNode(N_fundef);}   exprblock
          { 
            $$=$<node>3;
            $$->node[0]=$4;             /* Funktionsrumpf  */
            $$->node[2]=$1;             /* Funktionsargumente */
            $$->nnode=2;
         
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" %s"P_FORMAT" %s," P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, 
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0],
                        mdb_nodetype[ $$->node[2]->nodetype ], $$->node[2]));
          }

       | BRACKET_R  {$$=MakeNode(N_fundef);}   exprblock
          { 
            $$=$<node>2;
            $$->node[0]=$3;             /* Funktionsrumpf  */
            $$->nnode=1;
         
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" %s"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, 
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0]));
         }
        ;


args:   arg COMMA args  {$1->node[0]=$3;
                         $1->nnode=1;
                         $$=$1;
                        }
      | arg {$$=$1;}
      ;

arg: type ID {$$=MakeNode(N_arg); 
              $$->info.types=$1;         /* Argumenttyp */
              $$->info.types->id=$2;     /* Argumentname */
              $$->info.types->next=NULL; /* keine weiteren Argumente */

              DBUG_PRINT("GENTREE",
                         ("%s: "P_FORMAT", Id: %s, Attrib: %d  ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          $$->info.types->id, $$->info.types->attrib));
             }
     |
     type AMPERS ID {$$=MakeNode(N_arg); 
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

argtypes:   argtype COMMA argtypes  {$1->node[0]=$3;
                         $1->nnode=1;
                         $$=$1;
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

fun_name : ID { $$=$1; }
         | AND { $$=$1; }
         | OR { $$=$1; }
         | EQ { $$=$1; }
         | NEQ { $$=$1; }
         | NOT { $$=$1; }
         | LE { $$=$1; }
         | LT { $$=$1; }
         | GE { $$=$1; }
         | GT { $$=$1; }
         | PLUS { $$=$1; }
         | MINUS { $$=$1; }
         | DIV { $$=$1; }
         | MUL { $$=$1; }
         | RESHAPE { $$=$1; }
         | SHAPE { $$=$1; }
         | TAKE { $$=$1; }
         | DROP { $$=$1; }
         | DIM { $$=$1; }
         | ROTATE { $$=$1; }
         | CAT { $$=$1; }
         | PSI { $$=$1; }
         | GENARRAY { $$=$1; }
         | MODARRAY { $$=$1; }
        ;

main: TYPE_INT K_MAIN BRACKET_L BRACKET_R {$$=MakeNode(N_fundef);} exprblock 
       {
        $$=$<node>5;     /* $$=$5 */
        $$->node[0]=$6;                 /* Funktionsrumpf */

        $$->info.types=MakeTypes(T_int);  /* Knoten fu"r Typinformation */ 
        $$->info.types->id=(char *)malloc(5); 
        strcpy($$->info.types->id,"main");   /* Funktionsnamen eintragen */

        $$->nnode=1;  /* ein Nachfolgeknoten  */

        DBUG_PRINT("GENTREE",("%s:"P_FORMAT", main %s (" P_FORMAT ") ",
                              mdb_nodetype[$$->nodetype], $$, 
                              mdb_nodetype[ $$->node[0]->nodetype], 
                              $$->node[0]));
        
       }
      ;

exprblock: BRACE_L exprblock2 {$$=$2;}
	;

exprblock2: type ids SEMIC exprblock2 
            {node *tmp, *tmp2;

             $$=$4;
             tmp=GenVardec($1,$2);
	     if($$->nnode == 2)	{	 /* we have already vardecs here */
               tmp2=tmp;
               while(tmp2->node[0]!=NULL)
                 tmp2=tmp2->node[0];
               tmp2->node[0]=$$->node[1];
               tmp2->nnode=1;
             }
             else {			 /* this is the first vardec! */
               $$->nnode=2;              /* set number of child nodes */
             }
             $$->node[1]=tmp;		 /* insert new decs */
            }
           | 
            exprblock3
            {
              $$=MakeNode(N_block);
              $$->node[0]=$1;
              $$->nnode=1;

              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT", %s"P_FORMAT,
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype], $$->node[0]));
           }
           ;

exprblock3: assigns optretassign BRACE_R
              {if (NULL != $1) {
                 /* append retassign node($2) to assigns nodes */
                 $$=Append($1,$2);
               }
               else { /* no assigns */
                 $$=MakeNode(N_assign);
                 $$->node[0]=$2;  /* Returnanweisung */
                 $$->nnode=1;

                 DBUG_PRINT("GENTREE",
                            ("%s "P_FORMAT", %s "P_FORMAT ,
                             mdb_nodetype[$$->nodetype],$$,
                             mdb_nodetype[$$->node[0]->nodetype],
                             $$->node[0]));
               }
              }
	;
                 

assignblock: SEMIC     
              {  $$=MakeEmptyBlock();
              }

             | BRACE_L { $$=MakeNode(N_block); } assigns BRACE_R 
                { 
                   if (NULL != $3)
                   {
                      $$=$<node>2;
                      $$->node[0]=$3;
                      $$->nnode=1;
                      
                      DBUG_PRINT("GENTREE",
                                 ("%s"P_FORMAT", %s"P_FORMAT,
                                  mdb_nodetype[$$->nodetype], $$,
                                  mdb_nodetype[$$->node[0]->nodetype],
                                  $$->node[0]));
                   }
                   else /* block is empty */
                   {
                      free($<node>2);
                      $$=MakeEmptyBlock();
                   }
                }
             | assign  
                { $$=MakeNode(N_block);
                  $$->node[0]=$1;
                  $$->nnode=1;

                  DBUG_PRINT("GENTREE",
                             ("%s"P_FORMAT", %s"P_FORMAT,
                              mdb_nodetype[$$->nodetype], $$,
                              mdb_nodetype[$$->node[0]->nodetype],$$->node[0]));
                } 
             ;

retassignblock: BRACE_L {$$=MakeNode(N_block);} assigns retassign SEMIC BRACE_R
                  {  $$=$<node>2;
                   
                    /* append retassign node($4) to assigns nodes($3), if any
                     */
                    if (NULL != $3)
                       $$->node[0]=Append($3,$4);
                    else
                    {
                       $$->node[0]=MakeNode(N_assign);
                       $$->node[0]->node[0]=$4;
                       $$->node[0]->nnode=1;
                    }
                    $$->nnode=1;

                    DBUG_PRINT("GENTREE",
                               ("%s "P_FORMAT", %s"P_FORMAT, 
                                mdb_nodetype[$$->nodetype], $$,
                                mdb_nodetype[$$->node[0]->nodetype],
                                $$->node[0]));
                  } 
                | retassign
                    { $$=MakeNode(N_block);
                      $$->node[0]=MakeNode(N_assign);
                      $$->node[0]->node[0]=$1;  /* Returnanweisung */
                      $$->node[0]->nnode=1;
                      $$->nnode=1;
                      $$->lineno=$1->lineno; /* set lineno correktly */
                      
                      DBUG_PRINT("GENTREE",
                                 ("%s "P_FORMAT", %s " P_FORMAT ,
                                  mdb_nodetype[$$->node[0]->nodetype],
                                  $$->node[0],
                                  mdb_nodetype[$$->node[0]->node[0]->nodetype],
                                  $$->node[0]->node[0]));
                      
                      DBUG_PRINT("GENTREE",
                                 ("%s "P_FORMAT", %s " P_FORMAT ,
                                  mdb_nodetype[$$->nodetype], $$,
                                  mdb_nodetype[$$->node[0]->nodetype],
                                  $$->node[0]));
                    }
                ;

assigns: /* empty */ 
         { $$=NULL; 
         }
       | assign assigns 
          { $$=$1;
            if (NULL != $2) /* there are more assigns */
               $$=Append($1,$2);
         }
         ;

assign: letassign SEMIC 
           { $$=MakeNode(N_assign);
             $$->node[0]=$1;
             $$->nnode=1;

             DBUG_PRINT("GENTREE",
                        ("%s "P_FORMAT", %s "P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0]));
           }
      | selassign 
           { $$=MakeNode(N_assign);
             $$->node[0]=$1;
             $$->nnode=1;

             DBUG_PRINT("GENTREE",
                        ("%s "P_FORMAT", %s "P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0]));
           }
      | forassign 
           { if (N_assign != $1->nodetype)
             {
                $$=MakeNode(N_assign);
                $$->node[0]=$1;
                $$->nnode=1;
             }
             else
                $$=$1; /* if for loop is converted to while loop */
             

             DBUG_PRINT("GENTREE",
                        ("%s "P_FORMAT", %s "P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0]));
           }
         ;

optretassign: retassign SEMIC
            | /* empty */
              {
                $$=MakeNode(N_return);
  
                DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT,
                          mdb_nodetype[$$->nodetype], $$));
              }
            ;
                 
 

retassign: RETURN BRACKET_L {$$=MakeNode(N_return);} exprs BRACKET_R
             { $$=$<node>3;
               $$->node[0]=$4;   /* Returnwert */
               $$->nnode=1;

              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT", %s "P_FORMAT,
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype], $$->node[0]));
             }
         ;

letassign: ids LET exprORarray 
             { $$=MakeNode(N_let);
               $$->info.ids=$1;  /* zuzuweisende Variablenliste */
               $$->node[0]=$3;     /* Ausdruck */
               $$->nnode=1;

              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT": %s "P_FORMAT" ids: %s ",
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                                    $$->info.ids->id));

             }
         | apl 
           {
              $$=MakeNode(N_let);
              $$->node[0]=$1;
              $$->info.ids=NULL;
              $$->nnode=1; 

              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT":",
                          mdb_nodetype[$$->nodetype], $$));
           }
         | ID ASSIGN expr
           {
             $$=MakeNode(N_let);
             $$->node[0]=$3;
             $$->nnode=1;
             $$->info.ids=MakeIds($1);
             $$->info.ids->attrib=ST_deref;
          }

         | ID unaryop 
            { $$=MakeNode(N_post);
              $$->info.ids=MakeIds($1);
              $$->node[0]=$2;
              $$->nnode=1;
              
              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT": %s "P_FORMAT,
                          mdb_nodetype[$$->nodetype], $$, $$->info.ids->id,
                          mdb_nodetype[$$->node[0]->nodetype] ));
           }
         | unaryop ID
            {  $$=MakeNode(N_pre);
               $$->info.ids=MakeIds($2);    
               $$->node[0]=$1;
               $$->nnode=1;

               DBUG_PRINT("GENTREE",
                          ("%s "P_FORMAT": %s "P_FORMAT,
                           mdb_nodetype[$$->nodetype], $$, $$->info.ids->id,
                           mdb_nodetype[$$->node[0]->nodetype] )); 
            }
        | ID ADDON exprORarray
           {
              $$=MakeLet($1,$3,F_add);
           }
        | ID SUBON exprORarray
           {
              $$=MakeLet($1,$3,F_sub);
           }
        | ID MULON exprORarray
           {
              $$=MakeLet($1,$3,F_mul);
           }
        | ID DIVON exprORarray
           {
              $$=MakeLet($1,$3,F_div);
           }
         ;

selassign: IF {$$=MakeNode(N_cond);} BRACKET_L expr BRACKET_R assignblock 
           ELSE assignblock
            { $$=$<node>2;
              $$->node[0]=$4;  /* Bedingung */
              $$->node[1]=$6;  /* Then-Teil */
              $$->node[2]=$8;  /* Else-Teil */
              $$->nnode=3;

             DBUG_PRINT("GENTREE",
                        ("%s"P_FORMAT", %s"P_FORMAT", %s"P_FORMAT", %s"P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                         mdb_nodetype[$$->node[1]->nodetype], $$->node[1],
                         mdb_nodetype[$$->node[2]->nodetype], $$->node[2] ));

            }

forassign: DO {$$=MakeNode(N_do);} assignblock WHILE BRACKET_L expr BRACKET_R 
           SEMIC
            { $$=$<node>2;
              $$->node[0]=$6;   /* Test */
              $$->node[1]=$3;   /* Schleifenrumpf */
              $$->nnode=2;

             DBUG_PRINT("GENTREE",
                        ("%s "P_FORMAT": %s "P_FORMAT", %s "P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                         mdb_nodetype[$$->node[1]->nodetype], $$->node[1] ));
 
            }
           | WHILE {$$=MakeNode(N_while);} BRACKET_L expr BRACKET_R assignblock 
              { $$=$<node>2;
                $$->node[0]=$4;  /* Test */
                $$->node[1]=$6;  /* Schleifenrumpf */
                $$->nnode=2;

             DBUG_PRINT("GENTREE",
                        ("%s "P_FORMAT": %s " P_FORMAT ", %s " P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                         mdb_nodetype[$$->node[1]->nodetype], $$->node[1] ));
              }
           | FOR {$$=MakeNode(N_while);} BRACKET_L assign expr SEMIC 
             letassign BRACKET_R assignblock
              { $$=$4;  /* initialisation */
                $$->node[1]=MakeNode(N_assign);
                $$->nnode=2;
                $$->node[1]->node[0]=$<node>2;
                $$->node[1]->node[0]->node[0]=$5;  /* condition  */
                $$->node[1]->node[0]->node[1]=Append($9,$7);  /* body of loop */
                $$->node[1]->node[0]->nnode=2;
                $$->node[1]->nnode=1;
                
                DBUG_PRINT("GENTREE",
                           ("%s "P_FORMAT": %s "P_FORMAT,
                            mdb_nodetype[$$->node[1]->nodetype], $$->node[1],
                            mdb_nodetype[$$->node[1]->node[0]->nodetype],
                            $$->node[1]->node[0]));
                                
                DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT": %s "P_FORMAT", %s "P_FORMAT,
                          mdb_nodetype[$$->node[1]->node[0]->nodetype], 
                          $$->node[1]->node[0],
                          mdb_nodetype[$$->node[1]->node[0]->node[0]->nodetype],
                          $$->node[1]->node[0]->node[0],
                          mdb_nodetype[$$->node[1]->node[0]->node[1]->nodetype],
                          $$->node[1]->node[0]->node[1] ));
 
              } 
           ;

exprs: exprORarray COMMA exprs 
        { $$=MakeNode(N_exprs);
          $$->node[0]=$1;
          $$->node[1]=$3;
          $$->nnode=2;
          
          DBUG_PRINT("GENTREE",
                     ("%s "P_FORMAT": %s "P_FORMAT", %s " P_FORMAT ,
                      mdb_nodetype[$$->nodetype], $$,
                      mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                      mdb_nodetype[$$->node[1]->nodetype], $$->node[1]));
        }
       | exprORarray 
          {  $$=MakeNode(N_exprs);
             $$->node[0]=$1;
             $$->node[1]=NULL;
             $$->nnode=1;

             DBUG_PRINT("GENTREE",
                     ("%s "P_FORMAT": %s "P_FORMAT , 
                      mdb_nodetype[$$->nodetype], $$,
                      mdb_nodetype[$$->node[0]->nodetype], $$->node[0] ));
          }
          ;
exprsNOarray: expr COMMA exprsNOarray 
              { $$=MakeNode(N_exprs);
                $$->node[0]=$1;
                $$->node[1]=$3;
                $$->nnode=2;
          
                DBUG_PRINT("GENTREE",
                           ("%s "P_FORMAT": %s "P_FORMAT", %s " P_FORMAT ,
                            mdb_nodetype[$$->nodetype], $$,
                            mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                            mdb_nodetype[$$->node[1]->nodetype], $$->node[1]));
              }
            | expr 
              {  $$=MakeNode(N_exprs);
                 $$->node[0]=$1;
                 $$->node[1]=NULL;
                 $$->nnode=1;

                 DBUG_PRINT("GENTREE",
                            ("%s "P_FORMAT": %s "P_FORMAT , 
                             mdb_nodetype[$$->nodetype], $$,
                             mdb_nodetype[$$->node[0]->nodetype], $$->node[0]));
              }
              ;

apl: ID  BRACKET_L {$$=MakeNode(N_ap);} exprs BRACKET_R
         { $$=$<node>3;
           $$->node[0]=$4;                /* arguments */
           $$->info.fun_name.id=$1;                /* name of function */
           $$->nnode=1;

           DBUG_PRINT("GENTREE",
                      ("%s: "P_FORMAT ": Id: %s, Arg:%s " P_FORMAT,
                       mdb_nodetype[ $$->nodetype ], $$, $$->info.fun_name.id,
                       mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0]));
         }
      | ID BRACKET_L BRACKET_R
        { $$=MakeNode(N_ap);
          $$->info.fun_name.id=$1;         /* name of function */

          DBUG_PRINT("GENTREE",
                     ("%s " P_FORMAT " Id: %s,",
                      mdb_nodetype[ $$->nodetype ], $$, $$->info.fun_name.id));
        }
        ;

array: SQBR_L {$$=MakeNode(N_array);} exprsNOarray SQBR_R
       { $$=$<node>2;
         $$->node[0]=$3;
         $$->nnode=1;
         
         DBUG_PRINT("GENTREE",
                    ("%s " P_FORMAT ": %s " P_FORMAT,
                     mdb_nodetype[ $$->nodetype], $$,
                     mdb_nodetype[ $$->node[0]->nodetype], $$->node[0]));
       }
       ;

exprORarray: expr
             {  $$=$1; }
          | array
             {  $$=$1; }
          ;

expr:   apl {$$=$1; $$->info.fun_name.id_mod=NULL; }
      | ID COLON apl {$$=$3;
                      $$->info.fun_name.id_mod=$1;
                     }
      | WITH {$$=MakeNode(N_with);} BRACKET_L generator  BRACKET_R conexpr 
        { $$=$<node>2;
          $$->node[0]=$4;   /* Generator und Filter */
          $$->node[0]->lineno=$$->lineno;
          $$->node[1]=$6;   /* Rumpf */
          $$->nnode=2;

          DBUG_PRINT("GENTREE",
                     ("%s "P_FORMAT": %s: " P_FORMAT ",%s: " P_FORMAT ,
                      mdb_nodetype[ $$->nodetype], $$,
                      mdb_nodetype[ $$->node[0]->nodetype], $$->node[0],
                      mdb_nodetype[ $$->node[1]->nodetype], $$->node[1] ));
        }
      | ID 
         { $$=MakeNode(N_id);
           $$->info.ids=MakeIds($1);  /* name of variable*/

           DBUG_PRINT("GENTREE",("%s " P_FORMAT ": %s ",
                            mdb_nodetype[$$->nodetype],$$,$$->info.ids->id));  
         }
      | MINUS ID %prec UMINUS
        {   node *exprs1, *exprs2;
            exprs2=MakeNode(N_exprs);
            exprs2->node[0]=MakeNode(N_id);
            exprs2->node[0]->info.ids=MakeIds($2);
            exprs2->nnode=1;
            exprs1=MakeNode(N_exprs);
            exprs1->node[0]=MakeNode(N_num);
            exprs1->node[0]->info.cint=-1;
            exprs1->node[1]=exprs2;
            exprs1->nnode=2;
            $$=MakeNode(N_prf);
            $$->info.prf=F_mul;
            $$->node[0]=exprs1;
            $$->nnode=1;
            
            DBUG_PRINT("GENTREE",("%s "P_FORMAT": %d",
                                  mdb_nodetype[exprs1->node[0]->nodetype], 
                                  exprs1->node[0], exprs1->node[0]->info.cint));
            DBUG_PRINT("GENTREE",("%s " P_FORMAT ": %s ",
                                  mdb_nodetype[exprs2->node[0]->nodetype],
                                  exprs2->node[0], 
                                  exprs2->node[0]->info.ids->id));

            DBUG_PRINT("GENTREE",
                       ("%s " P_FORMAT ": %s " P_FORMAT ", %s" P_FORMAT,
                        mdb_prf[$$->info.prf], $$,
                        mdb_nodetype[exprs1->node[0]->nodetype], 
                        exprs1->node[0],
                        mdb_nodetype[exprs2->node[0]->nodetype], 
                        exprs2->node[0]));
         }
      | BRACKET_L COLON type BRACKET_R exprORarray %prec CAST
         {$$=MakeNode(N_cast);
          $$->info.types=$3;
          $$->node[0]=$5;
          $$->nnode=1;

          DBUG_PRINT("GENTREE",
                     ("%s "P_FORMAT": %s: " P_FORMAT ,
                      mdb_nodetype[ $$->nodetype], $$,
                      mdb_nodetype[ $$->node[0]->nodetype], $$->node[0]));
         }
      | NOT exprORarray
         {  node *exprs;
            exprs=MakeNode(N_exprs);
            exprs->node[0]=$2;
            exprs->nnode=1;
            $$=MakeNode(N_prf);
            $$->info.prf=F_not;
            $$->node[0]=exprs;
            $$->nnode=1;
         }
      | BRACKET_L exprORarray BRACKET_R 
         { $$=$2;
         }
      | expr SQBR_L array SQBR_R
         { node *exprs1, *exprs2;
           exprs2=MakeNode(N_exprs);
           exprs2->node[0]=$1;      /* array */
           exprs2->nnode=1;
           exprs1=MakeNode(N_exprs);
           exprs1->node[0]=$3;      /*  expression (shape)  */
           exprs1->node[1]=exprs2;
           exprs1->nnode=2;
           
           $$=MakeNode(N_prf);
           $$->node[0]=exprs1;
           $$->info.prf=F_psi;
           $$->nnode=1;
           $$->lineno=$1->lineno;
           
           DBUG_PRINT("GENTREE",
                      ("%s (%s)"P_FORMAT": %s"P_FORMAT", %s"P_FORMAT,
                       mdb_nodetype[ $$->nodetype], mdb_prf[$$->info.prf],$$, 
                       mdb_nodetype[$$->node[0]->node[0]->nodetype], 
                       $$->node[0]->node[0],
                       mdb_nodetype[ $$->node[0]->node[1]->node[0]->nodetype ],
                       $$->node[0]->node[1]->node[0]));
         }
      | expr SQBR_L exprsNOarray SQBR_R
         { node *exprs1, *exprs2, *array;
           exprs2=MakeNode(N_exprs);
           exprs2->node[0]=$1;      /* array */
           exprs2->nnode=1;
           if((1 == $3->nnode) && ( (N_id == $3->node[0]->nodetype) ||
                                    (N_prf == $3->node[0]->nodetype) ||
                                    (N_ap == $3->node[0]->nodetype) ) )
              exprs1=$3;         /*  expression (shape)  */
           else
           {
              /* make an array (index vector )*/
              array=MakeNode(N_array);
              array->node[0]=$3;
              array->nnode=1;
              exprs1=MakeNode(N_exprs);
              exprs1->node[0]=array;      /*  expression (shape)  */
           }
           exprs1->node[1]=exprs2;
           exprs1->nnode=2;
           
           $$=MakeNode(N_prf);
           $$->node[0]=exprs1;
           $$->info.prf=F_psi;
           $$->nnode=1;
           $$->lineno=$1->lineno;
           
           DBUG_PRINT("GENTREE",
                      ("%s (%s)"P_FORMAT": %s"P_FORMAT", %s"P_FORMAT,
                       mdb_nodetype[ $$->nodetype], mdb_prf[$$->info.prf],$$, 
                       mdb_nodetype[$$->node[0]->node[0]->nodetype], 
                       $$->node[0]->node[0],
                       mdb_nodetype[ $$->node[0]->node[1]->node[0]->nodetype ],
                       $$->node[0]->node[1]->node[0]));
        }
      | monop BRACKET_L exprORarray BRACKET_R 
         { 
            node *exprs;
            exprs=MakeNode(N_exprs);
            exprs->node[0]=$3;
            exprs->nnode=1;
                           
            $$=$1;          /* Monop-Knoten u"bernehmen */
            $$->node[0]=exprs;   /* Argument  */
            $$->nnode=1;
            DBUG_PRINT("GENTREE",
                       ("%s (%s)" P_FORMAT ": %s " P_FORMAT "",
                        mdb_nodetype[$$->nodetype], mdb_prf[$$->info.prf], $$,
                        mdb_nodetype[ $$->node[0]->nodetype], $$->node[0] ));
         }
      | binop BRACKET_L exprORarray COMMA exprORarray  BRACKET_R 
         { 
            node *exprs1, *exprs2;

            exprs2=MakeNode(N_exprs);
            exprs2->node[0]=$5;       /* 2. Argument  */
            exprs2->nnode=1;
            exprs1=MakeNode(N_exprs);
            exprs1->node[0]=$3;       /* 1. Argument  */
            exprs1->node[1]=exprs2;
            exprs1->nnode=2;
            
            $$=$1;         /* Binop-Knoten u"berbnehmmen  */
            $$->node[0]=exprs1;  
            $$->nnode=1;
            
            DBUG_PRINT("GENTREE",
                       ("%s (%s)"P_FORMAT": %s " P_FORMAT ", %s " P_FORMAT,
                        mdb_nodetype[$$->nodetype], mdb_prf[$$->info.prf], $$,
                        mdb_nodetype[$3->nodetype], $3, 
                        mdb_nodetype[$5->nodetype], $5 ));
          }
     | triop BRACKET_L exprORarray COMMA exprORarray COMMA exprORarray 
       BRACKET_R 
         { 
            node *exprs1, *exprs2, *exprs3;
            
            exprs3=MakeNode(N_exprs);
            exprs3->node[0]=$7;            /* 3. Argument  */
            exprs3->nnode=1;
            exprs2=MakeNode(N_exprs);
            exprs2->node[0]=$5;           /* 2. Argument  */
            exprs2->node[1]=exprs3;
            exprs2->nnode=2;
            exprs1=MakeNode(N_exprs);
            exprs1->node[0]=$3;           /* 1. Argument  */
            exprs1->node[1]=exprs2;
            exprs1->nnode=2;
            
            $$=$1;         /* Triop-Knoten u"berbnehmmen  */
            $$->node[0]=exprs1;  
            $$->nnode=1;

            DBUG_PRINT("GENTREE",
                      ("%s (%s)"P_FORMAT": %s"P_FORMAT", %s"P_FORMAT
                       ", %s"P_FORMAT,
                       mdb_nodetype[$$->nodetype], mdb_prf[$$->info.prf], $$,
                       mdb_nodetype[$3->nodetype], $3, 
                       mdb_nodetype[$5->nodetype], $5,
                       mdb_nodetype[$7->nodetype], $7));
         }
      | exprORarray PLUS exprORarray 
        { $$=GenPrfNode(F_add,$1,$3);
        }
      | exprORarray MINUS exprORarray 
        { $$=GenPrfNode(F_sub,$1,$3);
        }
      | exprORarray DIV exprORarray 
        { $$=GenPrfNode(F_div,$1,$3);
        }
      | exprORarray MUL exprORarray 
        { $$=GenPrfNode(F_mul,$1,$3); 
        }
      | exprORarray AND exprORarray
        { $$=GenPrfNode(F_and,$1,$3);
        } 
      | exprORarray OR exprORarray
        { $$=GenPrfNode(F_or ,$1,$3);
        } 
      | exprORarray EQ exprORarray
         { $$=GenPrfNode(F_eq,$1,$3);
         }
      | exprORarray NEQ exprORarray
        { $$=GenPrfNode(F_neq,$1,$3);
        } 
      | exprORarray LE exprORarray 
        { $$=GenPrfNode(F_le ,$1,$3);
        } 
      | exprORarray LT exprORarray
        { $$=GenPrfNode(F_lt ,$1,$3);
        } 
      | exprORarray GE exprORarray
        { $$=GenPrfNode(F_ge ,$1,$3);
        } 
      | exprORarray GT exprORarray
        { $$=GenPrfNode(F_gt ,$1,$3);
        } 
      | NUM
         {$$=MakeNode(N_num);
          $$->info.cint=yylval.cint;
          DBUG_PRINT("GENTREE",("%s" P_FORMAT ": %d",
                                mdb_nodetype[$$->nodetype],$$,$$->info.cint));
         }
      | MINUS NUM %prec UMINUS
         {$$=MakeNode(N_num);
          $$->info.cint=-yylval.cint;
          DBUG_PRINT("GENTREE",("%s" P_FORMAT ": %d",
                                mdb_nodetype[$$->nodetype],$$,$$->info.cint));
         }
      | PLUS  NUM %prec UMINUS
         {$$=MakeNode(N_num);
          $$->info.cint=yylval.cint;
          DBUG_PRINT("GENTREE",("%s" P_FORMAT ": %d",
                                mdb_nodetype[$$->nodetype],$$,$$->info.cint));
         }
      | FLOAT
         { $$=MakeNode(N_float);
           $$->info.cfloat=$1;
           
           DBUG_PRINT("GENTREE",
                      ("%s " P_FORMAT ": %f ", 
                       mdb_nodetype[$$->nodetype], $$, $$->info.cfloat)); 
         }
      | MINUS FLOAT %prec UMINUS
         { $$=MakeNode(N_float);
           $$->info.cfloat=-$2;
           
           DBUG_PRINT("GENTREE",
                      ("%s " P_FORMAT ": %f ", 
                       mdb_nodetype[$$->nodetype], $$, $$->info.cfloat)); 
         }
      | PLUS FLOAT %prec UMINUS
         { $$=MakeNode(N_float);
           $$->info.cfloat=$2;
           
           DBUG_PRINT("GENTREE",
                      ("%s " P_FORMAT ": %f ", 
                       mdb_nodetype[$$->nodetype], $$, $$->info.cfloat)); 
         }
      | DOUBLE
         { $$=MakeNode(N_double);
           $$->info.cdbl=$1;

           DBUG_PRINT("GENTREE",
                      ("%s " P_FORMAT ": %f ",
                       mdb_nodetype[$$->nodetype], $$, $$->info.cdbl));
         }
      | MINUS DOUBLE %prec UMINUS
         { $$=MakeNode(N_double);
           $$->info.cdbl=-$2;

           DBUG_PRINT("GENTREE",
                      ("%s " P_FORMAT ": %f ",
                       mdb_nodetype[$$->nodetype], $$, $$->info.cdbl));
         }
      | PLUS DOUBLE %prec UMINUS
         { $$=MakeNode(N_double);
           $$->info.cdbl=$2;

           DBUG_PRINT("GENTREE",
                      ("%s " P_FORMAT ": %f ",
                       mdb_nodetype[$$->nodetype], $$, $$->info.cdbl));
         }
      | TRUE 
         { $$=MakeNode(N_bool);
           $$->info.cint=1;

           DBUG_PRINT("GENTREE",("%s" P_FORMAT ": %s",
                                 mdb_nodetype[$$->nodetype],$$,
                                 $$->info.cint ? "true" : "false"));
         }
      | FALSE
         { $$=MakeNode(N_bool);
           $$->info.cint=0;

           DBUG_PRINT("GENTREE",("%s" P_FORMAT ": %s",
                                 mdb_nodetype[$$->nodetype],$$,
                                 $$->info.cint ? "true" : "false"));
         }
      | STR
         { $$=MakeNode(N_str);
           $$->info.id=$1;

           DBUG_PRINT("GENTREE",("%s" P_FORMAT ": %s",
                                 mdb_nodetype[$$->nodetype],$$,
                                 $$->info.id ));
        }
      ;

generator: exprORarray  LE ID LE exprORarray
            { $$=MakeNode(N_generator);
              $$->node[0]=$1;        /* left border  */
              $$->node[1]=$5;        /* right border */
              $$->info.ids=MakeIds($3);/*index-variable  */
              $$->nnode=2;

              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT": %s "P_FORMAT", ID: %s, %s "P_FORMAT ,
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                          $$->info.ids->id,
                          mdb_nodetype[$$->node[1]->nodetype], $$->node[1]));
            }
        ;

conexpr: GENARRAY {$$=MakeNode(N_genarray);} BRACKET_L exprORarray BRACKET_R 
         retassignblock 
           { $$=$<node>2;
             $$->node[0]=$4;         /* Name des Arrays */
             $$->node[1]=$6;            /* Rumpf */
             $$->nnode=2;

             DBUG_PRINT("GENTREE",
                        ("%s " P_FORMAT ": %s " P_FORMAT", %s "P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
			 mdb_nodetype[$$->node[1]->nodetype], $$->node[1] ));
           }
         | MODARRAY {$$=MakeNode(N_modarray);} BRACKET_L exprORarray BRACKET_R 
           retassignblock
           { $$=$<node>2;
             $$->node[0]=$4;         /* Name des Arrays */
             $$->node[1]=$6;            /* Rumpf */
             $$->nnode=2;

             DBUG_PRINT("GENTREE",
                        ("%s " P_FORMAT ": %s " P_FORMAT", %s "P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
			 mdb_nodetype[$$->node[1]->nodetype], $$->node[1] ));
           }
	 | FOLD BRACKET_L foldop BRACKET_R {$$=MakeNode(N_foldprf);}
	   retassignblock
	   { $$=$<node>5;
	     $$->info.prf=$3;
        $$->node[0]=$6;          /* body */
	     $$->nnode=1;

             DBUG_PRINT("GENTREE",
                        ("%s " P_FORMAT ": %s , %s "P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,
                         mdb_prf[$$->info.prf], 
			 mdb_nodetype[$$->node[0]->nodetype], $$->node[0] ));
           }
	 | FOLD BRACKET_L foldop COMMA exprORarray BRACKET_R 
      { $$=MakeNode(N_foldprf); } retassignblock
	   { $$=$<node>7;
	     $$->info.prf=$3;
        $$->node[0]=$8;          /* body */
        $$->node[1]=$5; 
	     $$->nnode=2;

        DBUG_PRINT("GENTREE",
                   ("%s " P_FORMAT ": %s , %s "P_FORMAT", %s "P_FORMAT,
                    mdb_nodetype[$$->nodetype], $$,
                    mdb_prf[$$->info.prf], 
                    mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                    mdb_nodetype[$$->node[1]->nodetype], $$->node[1]));
           }
	 | FOLD BRACKET_L foldfun exprORarray BRACKET_R 
	   retassignblock
	   { $$=$3;
        $$->node[0]=$6;          /* body */
        $$->node[1]=$4;
        $$->nnode=2;

        DBUG_PRINT("GENTREE",
                   ("%s " P_FORMAT ": %s , %s "P_FORMAT" %s "P_FORMAT,
                    mdb_nodetype[$$->nodetype], $$,
                    $$->node[0]->info.fun_name.id, 
                    mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                    mdb_nodetype[$$->node[1]->nodetype], $$->node[1]));
           }

         ;

foldfun:  ID COMMA 
     {
        $$=MakeNode(N_foldfun);
        $$->info.fun_name.id=$1;
     }
    | ID COLON ID COMMA
     {
        $$=MakeNode(N_foldfun);
        $$->info.fun_name.id=$3;
        $$->info.fun_name.id_mod=$1;
     }
    ;

     
 
foldop: PLUS {$$=F_add; }
	| MINUS {$$=F_sub;}
	| DIV {$$=F_div;}
	| MUL {$$=F_mul;}
	| AND {$$=F_and;}
	| OR {$$=F_or;}
	| EQ {$$=F_eq;}
	| NEQ {$$=F_neq;}
	;

monop: DIM
         { 
            $$=MakeNode(N_prf);
            $$->info.prf=F_dim;
         }
       | SHAPE
          { 
             $$=MakeNode(N_prf);
             $$->info.prf=F_shape;
          }
       | F2I
          {  $$=MakeNode(N_prf);
             $$->info.prf=F_ftoi; 
          }
       | F2D
          {  $$=MakeNode(N_prf);
             $$->info.prf=F_ftod;
          }
       | I2F
          {  $$=MakeNode(N_prf);
             $$->info.prf=F_itof; 
          }
       | I2D
          {  $$=MakeNode(N_prf);
             $$->info.prf=F_itod;
          }
       | D2I
          {  $$=MakeNode(N_prf);
             $$->info.prf=F_dtoi;
          }
       | D2F
          {  $$=MakeNode(N_prf);
             $$->info.prf=F_dtof;
          }

          ;

unaryop: INC
          { $$=MakeNode(N_inc);
          }

   
       | DEC
          { $$=MakeNode(N_dec);
          }
           ;

binop : PSI
         { $$=MakeNode(N_prf);
           $$->info.prf=F_psi;
         }
      | TAKE
         { $$=MakeNode(N_prf);
           $$->info.prf=F_take;
         }
      | DROP
         { $$=MakeNode(N_prf);
           $$->info.prf=F_drop;
         }
      | RESHAPE 
         { $$=MakeNode(N_prf);
           $$->info.prf=F_reshape;
        }
      ;


triop : ROTATE
         { $$=MakeNode(N_prf);
           $$->info.prf=F_rotate;
         }
      | CAT
         { $$=MakeNode(N_prf);
           $$->info.prf=F_cat;
         }
       ;

ids:   ID COMMA ids 
        { $$=GEN_NODE(ids);
          $$->id=$1;
          $$->next=$3;
          $$->node=NULL;
        }
     | ID 
        {$$=GEN_NODE(ids); 
         $$->id=$1; 
         $$->next=NULL;
         $$->node=NULL;
        }
     ;

nums:   NUM COMMA nums 
         { $$=GEN_NODE(nums);
           $$->num=$1;
           $$->next=$3;
           DBUG_PRINT("GENTREE",("nums: %d", $$->num));
         }
      | NUM 
         { $$=GEN_NODE(nums);
           $$->num=$1;
           $$->next=NULL;
           DBUG_PRINT("GENTREE",("nums: %d", $$->num));
         }
       ; 

returntypes: TYPE_VOID
             {
                $$=MakeTypes(T_void);

               DBUG_PRINT("GENTREE",("type:"P_FORMAT" %s",
                                     $$, mdb_type[$$->simpletype]));
             }
           | types
             {
                $$=$1;
             }
           ;

types:   type COMMA types 
           { $1->next=$3; 
             $$=$1;
           }
       | type {$$=$1;}
       ;

type: localtype {$$=$1;}
      | ID COLON localtype {
			$$=$3;
			$$->name_mod=$1;
			}
      ;

localtype:  complextype {$$=$1;}
          | simpletype {$$=$1;}
          ;

complextype:  simpletype SQBR_L nums SQBR_R  
               { $$=GenComplexType($1,$3);
               }
            | simpletype  SQBR_L SQBR_R  
               { $$=$1;
                 $$->dim=-1;
              }
             ;

simpletype: TYPE_INT 
             { $$=MakeTypes(T_int);
               DBUG_PRINT("GENTREE",("type:"P_FORMAT" %s",
                                     $$, mdb_type[$$->simpletype]));
            }
            | TYPE_FLOAT 
               { $$=MakeTypes(T_float); 
                 DBUG_PRINT("GENTREE",("type:"P_FORMAT" %s",
                                     $$, mdb_type[$$->simpletype]));
               }
            | TYPE_BOOL 
               { $$=MakeTypes(T_bool);
                 DBUG_PRINT("GENTREE",("type:"P_FORMAT" %s",
                                     $$, mdb_type[$$->simpletype]));
               }
            | TYPE_DBL
               { $$=MakeTypes(T_double);
                 DBUG_PRINT("GENTREE",("type:"P_FORMAT" %s",
                                     $$, mdb_type[$$->simpletype]));
               }
	    | TYPE_STR
               { $$=MakeTypes(T_str);
                 DBUG_PRINT("GENTREE",("type:"P_FORMAT" %s",
                                     $$, mdb_type[$$->simpletype]));
               }
            | ID
               { $$=MakeTypes(T_user);
                 $$->name=$1;
                 DBUG_PRINT("GENTREE",("type:"P_FORMAT" %s (%s)",
                                     $$, mdb_type[$$->simpletype], $$->name));
               }
            ;

%%

int yyerror( char *errname)
{
  fprintf(stderr, "sac2c : %s in line %d at \"%s\"\n", errname, linenum, yytext);
  exit(1);
}

int warn( char *warnname)
{
  fprintf(stderr, "sac: Warning: %s in line %d  at \"%s\"\n",warnname, 
          linenum, yytext);
  return(1);
}




node *GenPrfNode( prf prf, node *arg1, node *arg2)
{ 
  node *tmp, *exprs1, *exprs2;
  
  DBUG_ENTER("GenPrfNode");

  exprs2=MakeNode(N_exprs);
  exprs2->node[0]=arg2;
  exprs2->nnode=1;

  exprs1=MakeNode(N_exprs);
  exprs1->node[0]=arg1;
  exprs1->node[1]=exprs2;
  exprs1->nnode=2;

  tmp=MakeNode(N_prf);  
  tmp->info.prf=prf;
  tmp->node[0]=exprs1;
  
  tmp->nnode=1;
  
  DBUG_PRINT("GENTREE",
             ("%s (%s) " P_FORMAT ": %s " P_FORMAT ", %s" P_FORMAT,
              mdb_nodetype[tmp->nodetype], mdb_prf[tmp->info.prf],tmp,
              mdb_nodetype[arg1->nodetype], arg1,
              mdb_nodetype[arg2->nodetype], arg2));

  DBUG_RETURN(tmp);
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
    free(tmp);
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
     if (vardec_p)
        tmp->nnode=1;
     vardec_p=tmp;
     type_p=GEN_NODE(types);
     type_p=(types*) memcpy((void*)type_p, (void*)type, sizeof(types)) ;
     vardec_p->info.types=type_p;
     vardec_p->info.types->id=ids_p->id;
     tmp2=ids_p;
     ids_p=ids_p->next;
     free(tmp2);
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
      while( 1 != tmp->nnode )      /* look for last N_assign node */
      tmp=tmp->node[1];
   
      if (N_assign != append_node->nodetype)
      {
         tmp->node[1]=MakeNode(N_assign);
         tmp->node[1]->node[0]=append_node;
         tmp->node[1]->nnode=1;
      }
      else
         tmp->node[1]=append_node;
      
      tmp->nnode=2;     /* previous last N_assign node has a new child */
         
      DBUG_PRINT("GENTREE",("%s"P_FORMAT": %s"P_FORMAT" %s"P_FORMAT,
                            mdb_nodetype[tmp->nodetype],tmp,
                            mdb_nodetype[tmp->node[0]->nodetype],tmp->node[0],
                            mdb_nodetype[tmp->node[1]->nodetype],tmp->node[1]));
   }
   else
   {  /* target_node has type N_empty */

      free(tmp);     /* delete node of type N_empty */
      if (N_assign != append_node->nodetype)
      {
         tmp=MakeNode(N_assign);  
         tmp->node[0]=append_node;
         tmp->nnode=1;
         
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

/*
 *
 *  functionname  : MakeEmptyBlock
 *  arguments     : ---
 *  description   : genarates an tree that describes an empty block
 *  global vars   : ---
 *  internal funs : MakeNode
 *  external funs : ---
 *  macros        : DBUG..., P_FORMAT
 *
 *  remarks       :
 *
 */
node *MakeEmptyBlock()
{   
   node *return_node;
   
   DBUG_ENTER("MakeEmptyBlock");
   
   return_node=MakeNode(N_block);
   return_node->node[0]=MakeNode(N_empty);
   return_node->nnode=1;
   
   
   DBUG_PRINT("GENTREE",
              ("%s"P_FORMAT", %s"P_FORMAT,
               mdb_nodetype[return_node->nodetype], return_node,
               mdb_nodetype[return_node->node[0]->nodetype],
               return_node->node[0]));
   
   DBUG_RETURN(return_node);
}

/*
 *
 *  functionname  : MakeLet
 *  arguments     : 1) identifier 
 *                  2) expr node
 *                  3) primitive function
 *  description   : genarates a N_let whose childnode is a primitive function
 *                  that has as one argument the identifier of the left side
 *                  of the let.
 *  global vars   : ---
 *  internal funs : MakeNode, GenPrfNode
 *  external funs : MakeNode,MakeIds
 *  macros        : DBUG..., P_FORMAT
 *
 *  remarks       : this function is used to convert addon, etc to N_let
 *
 */
node *MakeLet(id *name, node *expr, prf fun)
{
   node *return_node,
        *id_node;
      
   DBUG_ENTER("MakeLet");
   
   return_node=MakeNode(N_let);
   return_node->info.ids=MakeIds(name);
   id_node=MakeNode(N_id);
   id_node->info.ids=MakeIds(name);
   id_node->info.ids->id=StringCopy(name);
   
   DBUG_PRINT("GENTREE",("%s"P_FORMAT": %s",
                         mdb_nodetype[id_node->nodetype],
                         id_node,
                         id_node->info.ids->id));
   
   return_node->node[0]=GenPrfNode(fun,id_node,expr);
   return_node->nnode=1;
   
   DBUG_PRINT("GENTREE",("%s"P_FORMAT": %s "P_FORMAT" ids: %s ",
                         mdb_nodetype[return_node->nodetype], return_node,
                         mdb_nodetype[return_node->node[0]->nodetype],
                         return_node->node[0], return_node->info.ids->id));
   
                         
   DBUG_RETURN(return_node); 
}


