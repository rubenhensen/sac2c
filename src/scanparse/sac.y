%{

/*
 *
 * $Log$
 * Revision 1.26  1994/12/14 12:42:08  hw
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

extern int linenum;
extern char yytext[];

int indent, i;
node *syntax_tree;


%}

%union {
         id              *id;
         ids             *ids;
         types           *types;
         node            *node;
         int             cint;
         float           cfloat;
         nums            *nums;
       }

%token BRACE_L, BRACE_R, BRACKET_L, BRACKET_R, SQBR_L, SQBR_R, COLON, SEMIC, COMMA,
       INLINE, LET, TYPEDEF
       AND, OR, EQ, NEQ, LE, LT, GE, GT, MUL, DIV, PLUS, MINUS, 
       INC, DEC, ADDON, SUBON, MULON, DIVON,
       RESHAPE, SHAPE, TAKE, DROP, DIM, ROTATE,CAT,PSI,
       MAIN, RETURN, IF, ELSE, DO, WHILE, FOR, WITH, GENARRAY, MODARRAY,
       MODDEC, MODIMP, CLASSDEC, IMPORT, ALL, IMPLICIT, EXPLICIT, TYPES, FUNS, OWN,
       ARRAY,SC, TRUE, FALSE;
%token <id> ID;
%token <types> TYPE_INT, TYPE_FLOAT, TYPE_BOOL;
%token <cint> NUM;
%token <cfloat> FLOAT;

%type <ids> ids,
%type <nums> nums;
%type <types> type, types, simpletype, complextype;
%type <node> vardec, vardecs, arg, args, fundefs, fundef, main, prg, modimp,
             typedefs, typedef, defs, def2, def3, fundef2, exprblock, exprblock2,
             exprblock3, assign, assigns, assignblock, letassign, retassign,
             selassign, forassign, retassignblock, let, 
             expr, exprs, monop, binop, triop, 
             conexpr, generator, unaryop,
             import, imports, impdesc, impdesc2, impdesc3;

%left OR
%left AND
%left EQ, NEQ
%left LE, LT, GE, GT
%left PLUS, MINUS
%left MUL, DIV
%left TAKE, DROP, RESHAPE
%left SQBR_L
%nonassoc UMINUS

%start file

%%

file:   prg {syntax_tree=$1;}
	| modimp {syntax_tree=$1;}
	;

/*
moddec: MODDEC ID COLON OWN COLON expdesc
	| MODDEC ID COLON imports OWN COLON expdesc
	| CLASSDEC ID COLON OWN COLON expdesc
	| CLASSDEC ID COLON imports OWN COLON expdesc
	;
*/

imports: import imports { $1->node[0]=$2;
                          $1->nnode+=1;
                          $$=$1;
                        }

	| import { $$=$1; }
	;

import: IMPORT ID COLON impdesc { $$=$4; $$->info.id=$2; }
	;

impdesc: ALL
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

impdesc3: FUNS COLON ids SEMIC BRACE_R
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

/*
expdesc: BRACE_L IMPLICIT TYPES COLON ids SEMIC expdesc2
        | BRACE_L expdesc2
        ;

expdesc2: EXPLICIT TYPES COLON exptypes SEMIC expdesc3
        | expdesc3
        ;

expdesc3: FUNS COLON fundecs SEMIC BRACE_R
        | BRACE_R
        ;

exptypes: exptype exptypes
	| exptype
	;

exptype: ID LET type SEMIC

fundecs: fundec fundecs
	| fundec
	;

fundec: types ID BRACKET_L args BRACKET_R SEMIC
	| types ID BRACKET_L BRACKET_R SEMIC
	;
*/

defs: imports def2 {$$=$2;
                    $$->node[0]=$1;
                   }
	| def2 {$$=$1;}

def2: typedefs def3 {$$=$2;
                     $$->node[1]=$1;
                    }
	|def3 {$$=$1;}

def3: fundefs { $$=MakeNode(N_modul);
                $$->info.id=NULL;
                $$->node[2]=$1;

                DBUG_PRINT("GENTREE",
                           ("%s:"P_FORMAT"  %s"P_FORMAT,
                            mdb_nodetype[ $$->nodetype ], $$, 
                            mdb_nodetype[ $$->node[2]->nodetype ], $$->node[2]));
              }
	;

modimp: MODIMP ID defs
          { $$=$3;
            $$->info.id=$2;
          }
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

            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.types->id));
          }

fundefs: fundef fundefs { $1->node[1]=$2;
                          $1->nnode+=1;
                          $$=$1;
                        }
	| fundef {$$=$1;}
	| main {$$=$1;}
	;

fundef: types fundef2 
        {
           id *function_name;
           $$=$2;
           function_name=$2->info.id;
           $$->info.types=$1;          /*  Typ der Funktion */
           $$->info.types->id=function_name;
        }
	| types INLINE {warn("inline not yet implemented!");} fundef2
            {$$=$4;
             $$->info.types=$1;          /*  Typ der Funktion */
            }

fundef2: ID BRACKET_L args BRACKET_R  {$$=MakeNode(N_fundef);}   exprblock
          { 
            $$=$<node>5;
            $$->node[0]=$6;             /* Funktionsrumpf  */
            $$->node[2]=$3;             /* Funktionsargumente */
            $$->info.id=$1;      /* name of function */
            $$->nnode=2;
         
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , %s"P_FORMAT" %s," P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.id,
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0],
                        mdb_nodetype[ $$->node[2]->nodetype ], $$->node[2]));
          }

       | ID BRACKET_L BRACKET_R  {$$=MakeNode(N_fundef);}   exprblock
          { 
            $$=$<node>4;
            $$->node[0]=$5;             /* Funktionsrumpf  */
            $$->info.id=$1;      /* name of function */
            $$->nnode=1;
         
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , %s"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.id,
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
                         ("%s: "P_FORMAT", Id: %s ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          $$->info.types->id));
             }
     ;

main: TYPE_INT MAIN BRACKET_L BRACKET_R {$$=MakeNode(N_fundef);} exprblock 
       {
        $$=$<node>5;     /* $$=$5 */
        $$->node[0]=$6;                 /* Funktionsrumpf */

        $$->info.types=GEN_NODE(types);  /* Knoten fu"r Typinformation */ 
        $$->info.types->simpletype=T_int;
        $$->info.types->next=NULL;
        $$->info.types->dim=0;           /* dim=0 => einfacher Typ  */
        $$->info.types->shpseg=NULL;     /* kein Array  */
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

exprblock2:vardecs exprblock3
           SEMIC BRACE_R 
            { 
               $$=MakeNode(N_block);
               $$->node[0]=$2;  /* assignments */
               $$->node[1]=$1;  /* declarations of variables */
               $$->nnode=2;              /* set number of child nodes */
                                 
               DBUG_PRINT("GENTREE",
                          ("%s "P_FORMAT", %s "P_FORMAT,
                           mdb_nodetype[$$->nodetype], $$,
                           mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                           mdb_nodetype[$$->node[1]->nodetype], $$->node[1]));
            }
         | exprblock3
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

exprblock3: assigns retassign SEMIC BRACE_R
              {if (NULL != $2) {
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

vardecs:   vardec vardecs     
            { $$=$1;
              $$->nnode=1;
              while($1->node[0]!=NULL)
                $1=$1->node[0];
              $1->node[0]=$2;  /* na"chster N_vardec Knoten */
              $1->nnode=1;
            }
         | vardec {$$=$1;}
         ;

vardec: type ids SEMIC {$$=GenVardec($1,$2);};

assigns: /* empty */ 
         { $$=NULL; 
         }
       | assign assigns 
          { $$=$1;
            if (NULL != $2) /* there are more assigns */
            {
               $$->node[1]=$2;
               $$->nnode=2;
            }
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
           { if (N_assign != $$->nodetype)
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

letassign: ids let expr 
             { $$=$2;           /* durch "let" erstellten Knoten u"bernehmen */ 
               $$->info.ids=$1;  /* zuzuweisende Variablenliste */
               $$->node[0]=$3;     /* Ausdruck */
               $$->nnode=1;

              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT": %s "P_FORMAT" ids: %s ",
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                                    $$->info.ids->id));

             }
         | ID unaryop 
            { $$=MakeNode(N_post);
              $$->info.id=$1;
              $$->node[0]=$2;
              $$->nnode=1;
              
              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT": %s "P_FORMAT,
                          mdb_nodetype[$$->nodetype], $$, $$->info.id,
                          mdb_nodetype[$$->node[0]->nodetype] ));
           }
         | unaryop ID
            {  $$=MakeNode(N_pre);
               $$->info.id=$2;    
               $$->node[0]=$1;
               $$->nnode=1;

               DBUG_PRINT("GENTREE",
                          ("%s "P_FORMAT": %s "P_FORMAT,
                           mdb_nodetype[$$->nodetype], $$, $$->info.id,
                           mdb_nodetype[$$->node[0]->nodetype] )); 
            }
         ;


let: LET { $$=MakeNode(N_let); }
     | ADDON { $$=MakeNode(N_addon); 
             }
     | SUBON { $$=MakeNode(N_subon); 
             }
     | MULON { $$=MakeNode(N_mulon);
             }
     | DIVON { $$=MakeNode(N_divon);
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
                          mdb_nodetype[$$->node[1]->node[0]->nodetype], $$,
                          mdb_nodetype[$$->node[1]->node[0]->node[0]->nodetype],
                          $$->node[1]->node[0]->node[0],
                          mdb_nodetype[$$->node[1]->node[0]->node[1]->nodetype],
                          $$->node[1]->node[0]->node[1] ));
 
              } 
           ;

exprs: expr COMMA exprs 
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
                      mdb_nodetype[$$->node[0]->nodetype], $$->node[0] ));
          }
       ;

expr:   ID  BRACKET_L {$$=MakeNode(N_ap);} exprs BRACKET_R 
         { $$=$<node>3;
           $$->node[0]=$4;                /* arguments */
           $$->info.id=$1;                /* name of function */
           $$->nnode=1;

           DBUG_PRINT("GENTREE",
                      ("%s: "P_FORMAT ": Id: %s, Arg:%s " P_FORMAT,
                       mdb_nodetype[ $$->nodetype ], $$, $$->info.id,
                       mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0]));
         }
      | ID BRACKET_L BRACKET_R 
        { $$=MakeNode(N_ap);
          $$->info.id=$1;         /* name of function */
          
          DBUG_PRINT("GENTREE",
                     ("%s " P_FORMAT " Id: %s,",
                      mdb_nodetype[ $$->nodetype ], $$, $$->info.id));
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
           $$->info.id=$1;  /* Name einer Variablen */

           DBUG_PRINT("GENTREE",("%s " P_FORMAT ": %s ",
                                mdb_nodetype[$$->nodetype], $$, $$->info.id));  
         }
      | BRACKET_L expr BRACKET_R 
         { $$=$2;
         }
      | expr SQBR_L expr SQBR_R 
         { $$=MakeNode(N_prf);
           $$->node[0]=$3;       /*  expression (shape)  */
           $$->info.prf=F_psi;
           $$->node[1]=$1;       /* array */
           $$->nnode=2;
           $$->lineno=$1->lineno;
           
           DBUG_PRINT("GENTREE",
                      ("%s (%s)"P_FORMAT": %s"P_FORMAT", %s"P_FORMAT
                       ", %s " P_FORMAT,
                       mdb_nodetype[ $$->nodetype], mdb_prf[$$->info.prf],$$, 
                       mdb_nodetype[$$->node[1]->nodetype], $$->node[1],
                       mdb_nodetype[ $$->node[0]->nodetype ],
                       $$->node[0]));
         }
      | SQBR_L {$$=MakeNode(N_array);} exprs SQBR_R
         { $$=$<node>2;
           $$->node[0]=$3;
           $$->nnode=1;

          DBUG_PRINT("GENTREE",
                     ("%s " P_FORMAT ": %s " P_FORMAT,
                      mdb_nodetype[ $$->nodetype], $$,
                      mdb_nodetype[ $$->node[0]->nodetype], $$->node[0]));
         }
      | monop BRACKET_L expr BRACKET_R 
         { $$=$1;          /* Monop-Knoten u"bernehmen */
           $$->node[0]=$3;   /* Argument  */
           $$->nnode=1;
           DBUG_PRINT("GENTREE",
                      ("%s (%s)" P_FORMAT ": %s " P_FORMAT "",
                       mdb_nodetype[$$->nodetype], mdb_prf[$$->info.prf], $$,
                       mdb_nodetype[ $$->node[0]->nodetype], $$->node[0] ));
         }
      | binop BRACKET_L expr COMMA expr  BRACKET_R 
         { $$=$1;         /* Binop-Knoten u"berbnehmmen  */
           $$->node[0]=$3;  /* 1. Argument  */
           $$->node[1]=$5;  /* 2. Argument  */
           $$->nnode=2;

           DBUG_PRINT("GENTREE",
                      ("%s (%s)"P_FORMAT": %s " P_FORMAT ", %s " P_FORMAT,
                       mdb_nodetype[$$->nodetype], mdb_prf[$$->info.prf], $$,
                       mdb_nodetype[$$->node[0]->nodetype], $$->node[0], 
                       mdb_nodetype[$$->node[1]->nodetype], $$->node[1] ));
          }
      | triop BRACKET_L expr COMMA expr COMMA expr BRACKET_R 
         { $$=$1;         /* Triop-Knoten u"berbnehmmen  */
           $$->node[0]=$3;  /* 1. Argument  */
           $$->node[1]=$5;  /* 2. Argument  */
           $$->node[2]=$7;  /* 3. Argument  */
           $$->nnode=3;

           DBUG_PRINT("GENTREE",
                      ("%s (%s)"P_FORMAT": %s"P_FORMAT", %s"P_FORMAT
                       ", %s"P_FORMAT,
                       mdb_nodetype[$$->nodetype], mdb_prf[$$->info.prf], $$,
                       mdb_nodetype[$$->node[0]->nodetype], $$->node[0], 
                       mdb_nodetype[$$->node[1]->nodetype], $$->node[1],
                       mdb_nodetype[$$->node[2]->nodetype], $$->node[2] ));
         }
      | expr PLUS expr 
        { $$=GenPrfNode(F_add,$1,$3);
        }
      | expr MINUS expr 
        { $$=GenPrfNode(F_sub,$1,$3);
        }
      | expr DIV expr 
        { $$=GenPrfNode(F_div,$1,$3);
        }
      | expr MUL expr 
        { $$=GenPrfNode(F_mul,$1,$3); 
        }
      | expr AND expr
        { $$=GenPrfNode(F_and,$1,$3);
        } 
      | expr OR expr
        { $$=GenPrfNode(F_or ,$1,$3);
        } 
      | expr EQ expr
         { $$=GenPrfNode(F_eq,$1,$3);
         }
      | expr NEQ expr
        { $$=GenPrfNode(F_neq,$1,$3);
        } 
      | expr LE expr 
        { $$=GenPrfNode(F_le ,$1,$3);
        } 
      | expr LT expr
        { $$=GenPrfNode(F_lt ,$1,$3);
        } 
      | expr GE expr
        { $$=GenPrfNode(F_ge ,$1,$3);
        } 
      | expr GT expr
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
      ;

generator: expr  LE ID LE expr
            { $$=MakeNode(N_generator);
              $$->node[0]=$1;        /* linke Grenze  */
              $$->node[1]=$5;        /* rechte Grenze */
              $$->info.id=$3;     /* Laufvariable  */
              $$->nnode=2;

              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT": %s "P_FORMAT", ID: %s, %s "P_FORMAT ,
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                          $$->info.id,
                          mdb_nodetype[$$->node[1]->nodetype], $$->node[1]));
            }
        ;

conexpr: GENARRAY {$$=MakeNode(N_genarray);} BRACKET_L expr BRACKET_R 
         retassignblock 
           { $$=$<node>2;
             $$->node[0]=$4;         /* Name des Arrays */
             $$->node[1]=$6;            /* Rumpf */
             $$->nnode=2;

             DBUG_PRINT("GENTREE",
                        ("%s " P_FORMAT ": ID: %s, %s " P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,$$->info.id,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0] ));
           }
         | MODARRAY {$$=MakeNode(N_modarray);} BRACKET_L expr BRACKET_R 
           retassignblock
           { $$=$<node>2;
             $$->node[0]=$4;         /* Name des Arrays */
             $$->node[1]=$6;            /* Rumpf */
             $$->nnode=2;

             DBUG_PRINT("GENTREE",
                        ("%s " P_FORMAT ": ID: %s, %s " P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,$$->info.id,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0] ));
           }
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

types:   type COMMA types 
           { $1->next=$3; 
             $$=$1;
           }
       | type {$$=$1;}
       ;

type:   complextype {$$=$1;}
      | simpletype {$$=$1;}
       ;

complextype:  TYPE_INT SQBR_L nums SQBR_R  
               { $$=GEN_NODE(types); 
                 $$->simpletype=T_int; 
                 $$->dim=0;
                 $$->next=NULL;
                 $$->id=NULL;   /* not needed in this case */
                 $$=GenComplexType($$,$3);
               }
            | TYPE_FLOAT  SQBR_L nums SQBR_R  
               { $$=GEN_NODE(types);
                 $$->simpletype=T_float; 
                 $$->dim=0;
                 $$->next=NULL; 
                 $$->id=NULL;     /* not used in this case */
                 $$=GenComplexType($$,$3);
               }
            | TYPE_BOOL  SQBR_L nums SQBR_R  
               { $$=GEN_NODE(types);
                 $$->simpletype=T_bool; 
                 $$->dim=0;
                 $$->next=NULL; 
                 $$->id=NULL;     /* not used in this case */
                 $$=GenComplexType($$,$3);
               }
            | TYPE_INT  SQBR_L SQBR_R  
               { $$=GEN_NODE(types);
                 $$->simpletype=T_int; 
                 $$->dim=-1;
                 $$->next=NULL; 
                 $$->id=NULL;     /* not used in this case */
               }
            | TYPE_FLOAT  SQBR_L SQBR_R  
               { $$=GEN_NODE(types);
                 $$->simpletype=T_float; 
                 $$->dim=-1;
                 $$->next=NULL; 
                 $$->id=NULL;     /* not used in this case */
               }
            | TYPE_BOOL  SQBR_L SQBR_R  
               { $$=GEN_NODE(types);
                 $$->simpletype=T_bool; 
                 $$->dim=-1;
                 $$->next=NULL; 
                 $$->id=NULL;     /* not used in this case */
               }
             ;

simpletype: TYPE_INT 
             { $$=GEN_NODE(types);
               $$->simpletype=T_int; 
               $$->dim=0;
               $$->id=NULL;     /* not used in this case */
               $$->next=NULL;
             }
            | TYPE_FLOAT 
               { $$=GEN_NODE(types); 
                 $$->simpletype=T_float; 
                 $$->dim=0;
                 $$->next=NULL;
                 $$->id=NULL;     /* not used in this case */
               }
            | TYPE_BOOL 
               { $$=GEN_NODE(types);
                 $$->simpletype=T_bool; 
                 $$->dim=0;
                 $$->next=NULL;
                 $$->id=NULL;     /* not used in this case */
               }
/*
            | ID
               { $$=GEN_NODE(types);
                 $$->simpletype=T_unknown;
                 $$->dim=0;
                 $$->next=NULL;
                 $$->id=$1;
               }
*/
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


/*
 *  MakeNode erzeugt einen Zeiger auf "node" und initialisiert alle Eintra"ge,
 *  bis auf:      node.info
 *
 */
node *MakeNode(nodetype nodetype)
{
   node *tmp;
   
   DBUG_ENTER("MakeNode");

   tmp=GEN_NODE(node);
   tmp->nodetype=nodetype;
   for(i=0 ;i<4;i++)
      tmp->node[i]=NULL;
   tmp->nnode=0;
   tmp->info.id=NULL;
   tmp->lineno=linenum;
   
   DBUG_PRINT("MAKENODE",("%d nodetype: %s "P_FORMAT,
                          tmp->lineno,
                          mdb_nodetype[nodetype],tmp)); 

   DBUG_RETURN(tmp);
}


node *GenPrfNode( prf prf, node *arg1, node *arg2)
{ 
  node *tmp;

  DBUG_ENTER("GenPrfNode");
  tmp=MakeNode(N_prf);
  tmp->info.prf=prf;
  tmp->node[0]=arg1;
  tmp->node[1]=arg2;
  tmp->nnode=2;
  
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
   }
   else
   {  /* target_node has type N_empty */

         free(tmp);     /* delete node of type N_empty */
      if (N_assign != append_node->nodetype)
      {
         tmp=MakeNode(N_assign);  
         tmp->node[0]=append_node;
         tmp->nnode=1;
      }
      else
         tmp=append_node;
   }
         
   DBUG_PRINT("APPEND",("return node :%s"P_FORMAT,
                        mdb_nodetype[target_node->nodetype],target_node));
   
   
   DBUG_RETURN(target_node);
}

/*
 *
 *  functionname  : MakeEmptyNode
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

   
