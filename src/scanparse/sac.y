%{

/*
 *
 * $Log$
 * Revision 1.13  1994/11/18 16:55:52  hw
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

%token BRACE_L, BRACE_R, BRACKET_L, BRACKET_R, SQBR_L, SQBR_R, COLON, COMMA,
       INLINE, LET,
       AND, OR, EQ, NEQ, LE, LT, GE, GT, MUL, DIV, PLUS, MINUS, 
       INC, DEC, ADDON, SUBON, MULON, DIVON,
       RESHAPE, SHAPE, TAKE, DROP, DIM, ROTATE,CAT,PSI,
       MAIN, RETURN, IF, ELSE, DO, WHILE, FOR, WITH, GENARRAY, MODARRAY,
       ARRAY,
       SC;
%token <id> ID;
%token <types> TYPE_INT, TYPE_FLOAT, TYPE_BOOL;
%token <cint> NUM;
%token <cfloat> FLOAT;

%type <ids> ids;
%type <nums> nums;
%type <types> type, types, simpletype, complextype;
%type <node> vardec, vardecs, arg, args, funs, fundef, main, prg,
             exprblock, assign, assigns, assignblock, letassign, retassign,
             selassign, forassign, retassignblock, let, 
             expr, exprs, monop, binop, triop, 
             conexpr, generator, unaryop;

%left OR
%left AND
%left EQ, NEQ
%left LE, LT, GE, GT
%left PLUS, MINUS
%left MUL, DIV
%left TAKE, DROP, RESHAPE
%left SQBR_L
%nonassoc UMINUS

%start prg

%%

prg: funs {syntax_tree=$1;}

funs:  fundef funs  { $1->node[2]=$2;
                      $1->nnode=3;
                      $$=$1;
                    }
     | main  {$$=$1;}
      ;

fundef:  types ID BRACKET_L args BRACKET_R exprblock 
          { $$=MakeNode(N_fundef);
            $$->node[0]=$6;             /* Funktionsrumpf  */
            $$->node[1]=$4;             /* Funktionsargumente */
            $$->info.types=$1;       /*  Typ der Funktion */
            $$->info.types->id=$2;   /* Name der Funktion */
            $$->nnode=2;
            
            DBUG_PRINT("GENTREE",
                       ("%s:" P_FORMAT " Id: %s , %s "P_FORMAT " %s," P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.types->id,
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0],
                        mdb_nodetype[ $$->node[1]->nodetype ], $$->node[1]));
          }

       | types INLINE {warn("inline not yet implemented!");} ID BRACKET_L
         args BRACKET_R exprblock 
          { $$=MakeNode(N_fundef);
            $$->node[0]=$8;              /* Funktionsrumpf  */
            $$->node[1]=$6;              /* Funktionsargumente */
            $$->info.types=$1;        /*  Typ der Funktion */
            $$->info.types->id=$4;    /* Name der Funktion */
            DBUG_PRINT("GENTREE",
                       ("%s: "P_FORMAT", Id: %s , %s "P_FORMAT" %s, "P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.types->id,
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0],
                        mdb_nodetype[ $$->node[1]->nodetype ], $$->node[1]));
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

main: TYPE_INT MAIN BRACKET_L BRACKET_R exprblock 
       {$$=MakeNode(N_fundef);
        $$->node[0]=$5;                 /* Funktionsrumpf */

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

exprblock: BRACE_L vardecs assigns retassign COLON BRACE_R 
            { $$=MakeNode(N_block);
              
              /* append retassign node($4) to assigns nodes */
              $$->node[0]=Append($3,$4);
              
              $$->node[1]=$2;  /* Variablendeklarationen */
              $$->nnode=2;              /* set number of child nodes */

              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT", %s "P_FORMAT,
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                          mdb_nodetype[$$->node[1]->nodetype], $$->node[1]));

            }
         | BRACE_L assigns retassign COLON BRACE_R 
            { 
              $$=MakeNode(N_block);

             /* append retassign node($4) to assigns nodes */
              $$->node[0]=Append($2,$3); 
              
              $$->nnode=1;

              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT", %s"P_FORMAT,
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype], $$->node[0]));
            }
         | BRACE_L retassign COLON BRACE_R 
            { $$=MakeNode(N_block); 
              $$->node[0]=MakeNode(N_assign);
              $$->node[0]->node[0]=$2;  /* Returnanweisung */
              $$->node[0]->nnode=1;
              $$->nnode=1;
              
              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT", %s " P_FORMAT ,
                          mdb_nodetype[$$->node[0]->nodetype],$$->node[0],
                          mdb_nodetype[$$->node[0]->node[0]->nodetype],
                          $$->node[0]->node[0]));
                      
              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT", %s " P_FORMAT ,
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype],$$->node[0]));
            }
           ;

assignblock: COLON     
              {  $$=MakeEmptyBlock();
              }

             | BRACE_L assigns BRACE_R 
                { $$=MakeNode(N_block);
                  $$->node[0]=$2;
                  $$->nnode=1;
                  
                  DBUG_PRINT("GENTREE",
                             ("%s"P_FORMAT", %s"P_FORMAT,
                              mdb_nodetype[$$->nodetype], $$,
                              mdb_nodetype[$$->node[0]->nodetype],$$->node[0]));
                }
             | BRACE_L BRACE_R  
                {  $$=MakeEmptyBlock();
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

retassignblock: BRACE_L assigns retassign COLON BRACE_R
                  { 
                    $$=MakeNode(N_block);
                    /* append retassign node($3) to assigns nodes($2)
                     */
                    $$->node[0]=Append($2,$3);
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
            { $1->node[0]=$2;  /* na"chster N_vardec Knoten */
              $$=$1;
              $$->nnode=1;
            }
         | vardec {$$=$1;}
         ;

vardec: type ids COLON {$$=GenVardec($1,$2);};

assigns: assign assigns 
          { $$=$1;
            $$->node[1]=$2;
            $$->nnode=2;
          }
       | assign {$$=$1;}
         ;

assign: letassign COLON 
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

retassign: RETURN BRACKET_L exprs BRACKET_R
             { $$=MakeNode(N_return);
               $$->node[0]=$3;   /* Returnwert */
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

selassign: IF BRACKET_L expr BRACKET_R assignblock ELSE assignblock
            { $$=MakeNode(N_cond);
              $$->node[0]=$3;  /* Bedingung */
              $$->node[1]=$5;  /* Then-Teil */
              $$->node[2]=$7;  /* Else-Teil */
              $$->nnode=3;

             DBUG_PRINT("GENTREE",
                        ("%s"P_FORMAT", %s"P_FORMAT", %s"P_FORMAT", %s"P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                         mdb_nodetype[$$->node[1]->nodetype], $$->node[1],
                         mdb_nodetype[$$->node[2]->nodetype], $$->node[2] ));

            }

forassign: DO assignblock WHILE BRACKET_L expr BRACKET_R 
            { $$=MakeNode(N_do);
              $$->node[0]=$5;   /* Test */
              $$->node[1]=$2;   /* Schleifenrumpf */
              $$->nnode=2;

             DBUG_PRINT("GENTREE",
                        ("%s "P_FORMAT": %s "P_FORMAT", %s "P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                         mdb_nodetype[$$->node[1]->nodetype], $$->node[1] ));
 
            }
           | WHILE BRACKET_L expr BRACKET_R assignblock 
              { $$=MakeNode(N_while);
                $$->node[0]=$3;  /* Test */
                $$->node[1]=$5;  /* Schleifenrumpf */
                $$->nnode=2;

             DBUG_PRINT("GENTREE",
                        ("%s "P_FORMAT": %s " P_FORMAT ", %s " P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                         mdb_nodetype[$$->node[1]->nodetype], $$->node[1] ));
              }
           | FOR BRACKET_L assign expr COLON letassign BRACKET_R assignblock
              { $$=$3;  /* initialisation */
                $$->node[1]=MakeNode(N_assign);
                $$->nnode=2;
                $$->node[1]->node[0]=MakeNode(N_while);
                $$->node[1]->node[0]->node[0]=$4;  /* condition  */
                $$->node[1]->node[0]->node[1]=Append($8,$6);  /* body of loop */
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

expr:   ID BRACKET_L exprs BRACKET_R 
         { $$=MakeNode(N_ap);
           $$->node[0]=$3;                /* Argumente */
           $$->info.id=$1;         /* Funktionsname */
           $$->nnode=1;

           DBUG_PRINT("GENTREE",
                      ("%s: "P_FORMAT ": Id: %s, Arg:%s " P_FORMAT,
                       mdb_nodetype[ $$->nodetype ], $$, $$->info.id,
                       mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0]));
         }
      | ID BRACKET_L BRACKET_R 
        { $$=MakeNode(N_ap);
          $$->info.id=$1;         /* Funktionsname */
          
          DBUG_PRINT("GENTREE",
                     ("%s " P_FORMAT " Id: %s,",
                      mdb_nodetype[ $$->nodetype ], $$, $$->info.id));
        }
      | WITH BRACKET_L generator  BRACKET_R conexpr 
        { $$=MakeNode(N_with);
          $$->node[0]=$3;   /* Generator und Filter */
          $$->node[1]=$5;   /* Rumpf */
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
      | ID SQBR_L exprs SQBR_R 
         { $$=MakeNode(N_prf);
           $$->node[0]=$3;   /* Expression-Liste  */
           $$->info.prf=F_psi;
           $$->node[1]=MakeNode(N_id);
           $$->node[1]->info.id=$1 ;      /* Arrayname */
           $$->nnode=2;
          
           DBUG_PRINT("GENTREE",
                      ("%s (%s)"P_FORMAT": ID: %s " P_FORMAT ", %s " P_FORMAT,
                       mdb_nodetype[ $$->nodetype], mdb_prf[$$->info.prf],
                       $$, $$->node[1]->info.id,
                       $$->node[1], mdb_nodetype[ $$->node[0]->nodetype ],
                       $$->node[0]));
         }
      | SQBR_L exprs SQBR_R
         { $$=MakeNode(N_array);
           $$->node[0]=$2;
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
                      ("%s " P_FORMAT ": %s " P_FORMAT "",
                       mdb_nodetype[$$->nodetype], $$,
                       mdb_nodetype[ $$->node[0]->nodetype], $$->node[0] ));
         }
      | binop BRACKET_L expr COMMA expr  BRACKET_R 
         { $$=$1;         /* Binop-Knoten u"berbnehmmen  */
           $$->node[0]=$3;  /* 1. Argument  */
           $$->node[1]=$5;  /* 2. Argument  */
           $$->nnode=2;

           DBUG_PRINT("GENTREE",
                      ("%s "P_FORMAT": %s " P_FORMAT ", %s " P_FORMAT,
                       mdb_nodetype[$$->nodetype], $$,
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
                      ("%s"P_FORMAT": %s"P_FORMAT",  %s"P_FORMAT", %s "P_FORMAT,
                       mdb_nodetype[$$->nodetype], $$,
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
      ;

generator: expr LE ID LE expr
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

conexpr: GENARRAY BRACKET_L expr BRACKET_R retassignblock 
           { $$=MakeNode(N_genarray);
             $$->node[0]=$3;         /* Name des Arrays */
             $$->node[1]=$5;            /* Rumpf */
             $$->nnode=2;

             DBUG_PRINT("GENTREE",
                        ("%s " P_FORMAT ": ID: %s, %s " P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,$$->info.id,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0] ));
           }
         | MODARRAY BRACKET_L expr BRACKET_R retassignblock
           { $$=MakeNode(N_modarray);
             $$->node[0]=$3;         /* Name des Arrays */
             $$->node[1]=$5;            /* Rumpf */
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
        }
     | ID 
        {$$=GEN_NODE(ids); 
         $$->id=$1; 
         $$->next=NULL;
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
            ;

%%

int yyerror( char *errname)
{
  fprintf(stderr, "sac : %s in line %d at \"%s\"\n", errname, linenum, yytext);
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

   DBUG_PRINT("MAKENODE",("nodetype: %s "P_FORMAT,mdb_nodetype[nodetype],tmp)); 

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
   if (N_assign == target_node->nodetype) 
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

   
