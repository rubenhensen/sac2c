%{
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
         nums            *nums;
         types           *types;
         node            *node;
	}

%token BRACE_L, BRACE_R, BRACKET_L, BRACKET_R, SQBR_L, SQBR_R, COLON, COMMA,
       INLINE, LET,
       AND, OR, EQ, NEQ, LE, LT, GE, GT, MUL, DIV, PLUS, MINUS, 
       INC, DEC, ADDON, SUBON, MULON, DIVON,
       RESHAPE, SHAPE, TAKE, DROP, DIM, ROTATE,CAT,PSI,
       MAIN, RETURN, IF, ELSE, DO, WHILE, FOR, WITH, GENARRAY, MODARRAY,
       ARRAY,
       FLOAT, NUM,
       SC;
%token <id> ID;
%token <types> TYPE_INT, TYPE_FLOAT, TYPE_BOOL;

%type <ids> ids;
%type <nums> nums;
%type <types> type, types, simpletype, complextype;
%type <node> vardec, vardecs, arg, args, funs, fundef, main, prg,
             exprblock, assign, assigns, assignblock, letassign, retassign,
             selassign, forassign, retassignblock, let, 
             expr, exprs, monop, binop, triop, 
             conexpr, generator;


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
                          mdb_nodetype[ $$->nodetype ], $$, $$->info.types->id));
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
                              mdb_nodetype[ $$->node[0]->nodetype], $$->node[0]));
        
       }
      ;

exprblock: BRACE_L vardecs assigns retassign COLON BRACE_R 
            { node *tmp;
              $$=MakeNode(N_block);
              $$->node[0]=$3;  /* Anweisungen im Block */
              $$->node[1]=$2;  /* Variablendeklarationen */

              /* append retassign node($4) to assigns nodes */
              tmp=$$->node[0];
              while( NULL != tmp->node[1] )      /* look for last N_assign node */
                 tmp=tmp->node[1];
              tmp->node[1]=MakeNode(N_assign);
              tmp->nnode=2;
              tmp->node[1]->node[0]=$4;
              tmp->node[1]->nnode=1;                /* set number of children nodes */

              $$->nnode=2;              /* set number of child nodes */
              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT", %s "P_FORMAT,
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                          mdb_nodetype[$$->node[1]->nodetype], $$->node[1]));

            }
         | BRACE_L assigns retassign COLON BRACE_R 
            { node *tmp;
              $$=MakeNode(N_block);
              $$->node[0]=$2;  /* Anweisung im Block */
              $$->nnode=1;

              /* append retassign node($4) to assigns nodes */
              tmp=$$->node[0];
              while( NULL != tmp->node[1] )      /* look for last N_assign node */
                 tmp=tmp->node[1];
              tmp->node[1]=MakeNode(N_assign);
              tmp->nnode=2;
              tmp->node[1]->node[0]=$3;
              tmp->node[1]->nnode=1;                /* set number of children nodes */

              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT, ", %s"P_FORMAT,
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype], $$->node[0]));
            }
         | BRACE_L retassign COLON BRACE_R 
            { $$=MakeNode(N_block);
              $$->node[0]=$2;  /* Returnanweisung */
              $$->nnode=1;

              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT", %s "P_FORMAT,
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype], $$->node[0]));

            }
           ;

assignblock: COLON     
              {  $$=MakeNode(N_empty);
              }

             | BRACE_L assigns BRACE_R    {$$=$2;}
             | BRACE_L BRACE_R  { $$=MakeNode(N_empty); }
             | assign  {$$=$1;}
             ;

retassignblock: BRACE_L assigns retassign COLON BRACE_R
                  { node *tmp;
                    $$=MakeNode(N_block);
                    $$->node[0]=$2;  /* Anweisung im Block */
                    $$->nnode=1;

                    /* append retassign node($4) to assigns nodes */
                    tmp=$$->node[0];
                    while( NULL != tmp->node[1] )      /* look for last N_assign node */
                       tmp=tmp->node[1];
                    tmp->node[1]=MakeNode(N_assign);
                    tmp->nnode=2;
                    tmp->node[1]->node[0]=$3;
                    tmp->node[1]->nnode=1;             /* set number of children nodes */

                    DBUG_PRINT("GENTREE",
                               ("%s "P_FORMAT, ", %s"P_FORMAT, 
                                mdb_nodetype[$$->nodetype], $$,
                                mdb_nodetype[$$->node[0]->nodetype], $$->node[0]));
                  } 
                | retassign
                    { $$=MakeNode(N_block);
                      $$->node[0]=$1;  /* Returnanweisung */
                      $$->nnode=1;
                      
                      DBUG_PRINT("GENTREE",
                                 ("%s "P_FORMAT", %s " P_FORMAT ,
                                  mdb_nodetype[$$->nodetype], $$,
                                  mdb_nodetype[$$->node[0]->nodetype],$$->node[0]));
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
           { $$=MakeNode(N_assign);
             $$->node[0]=$1;
             $$->nnode=1;

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
             { $$=$2;            /* durch "let" erstellten Knoten u"bernehmen */ 
               $$->info.ids=$1;  /* zuzuweisende Variablenliste */
               $$->node[0]=$3;     /* Ausdruck */
               $$->nnode=1;

              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT": %s "P_FORMAT" ids: %s ",
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                                    $$->info.ids->id));

             }
         | ID monpostop {$$=NULL;}
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
                        ("%s "P_FORMAT", %s "P_FORMAT", %s "P_FORMAT", %s "P_FORMAT,
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
              { $$=MakeNode(N_for);
                $$->node[0]=$3;  /* Initialisierung */
                $$->node[1]=$4;  /* Test  */
                $$->node[2]=$6;  /* Modifizierer */
                $$->node[3]=$8;  /* Schleifenrumpf */
                $$->nnode=4;
                
                DBUG_PRINT("GENTREE",
                           ("%s "P_FORMAT": %s "P_FORMAT", %s "P_FORMAT", %s "
                            P_FORMAT", %s " P_FORMAT ,
                            mdb_nodetype[$$->nodetype], $$,
                            mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                            mdb_nodetype[$$->node[1]->nodetype], $$->node[1],
                            mdb_nodetype[$$->node[2]->nodetype], $$->node[2],
                            mdb_nodetype[$$->node[3]->nodetype], $$->node[3] ));
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
      | FLOAT
         { $$=MakeNode(N_float);
           $$->info.cfloat=atof(yytext);
           
           DBUG_PRINT("GENTREE",
                      ("%s " P_FORMAT ": %f ", 
                       mdb_nodetype[$$->nodetype], $$, $$->info.cfloat)); 
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
                      ("%s "P_FORMAT": %s "P_FORMAT",  %s "P_FORMAT", %s "P_FORMAT,
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
          $$->info.cint=atoi(yytext);
          DBUG_PRINT("GENTREE",("%s" P_FORMAT ": %d",
                                mdb_nodetype[$$->nodetype],$$,$$->info.cint));
         }
      | MINUS NUM %prec UMINUS
         {$$=MakeNode(N_num);
          $$->info.cint=-(atoi(yytext));
          DBUG_PRINT("GENTREE",("%s" P_FORMAT ": %d",
                                mdb_nodetype[$$->nodetype],$$,$$->info.cint));
         }
      | PLUS  NUM %prec UMINUS
         {$$=MakeNode(N_num);
          $$->info.cint=atoi(yytext);
          DBUG_PRINT("GENTREE",("%s" P_FORMAT ": %d",
                                mdb_nodetype[$$->nodetype],$$,$$->info.cint));
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

conexpr: GENARRAY BRACKET_L ID BRACKET_R retassignblock 
           { $$=MakeNode(N_genarray);
             $$->node[0]=$5;            /* Rumpf */
             $$->info.id=$3;         /* Name des Arrays */
             $$->nnode=1;

             DBUG_PRINT("GENTREE",
                        ("%s " P_FORMAT ": ID: %s, %s " P_FORMAT,
                         mdb_nodetype[$$->nodetype], $$,$$->info.id,
                         mdb_nodetype[$$->node[0]->nodetype], $$->node[0] ));
           }
         | MODARRAY BRACKET_L ID BRACKET_R retassignblock
           { $$=MakeNode(N_modarray);
             $$->node[0]=$5;            /* Rumpf */
             $$->info.id=$3;         /* Name des Arrays */
             $$->nnode=1;

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

monpostop: INC
           | DEC
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
           $$->num=atoi(yytext);
           $$->next=$3;
         }
      | NUM 
         { $$=GEN_NODE(nums);
           $$->num=atoi(yytext);
           $$->next=NULL;
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
  return(1);
}

int warn( char *warnname)
{
  fprintf(stderr, "sac: Warning: %s in line %d  at \"%s\"\n", warnname, linenum, yytext);
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
