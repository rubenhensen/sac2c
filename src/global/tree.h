/*
 *
 * $Log$
 * Revision 1.14  1994/12/21 11:34:50  hw
 * changed definition of simpletype (now with macro & include)
 *
 * Revision 1.13  1994/12/20  15:56:58  sbs
 * T_hidden inserted
 *
 * Revision 1.12  1994/12/20  15:42:17  sbs
 * externals for Makenode and AppandChain added
 *
 * Revision 1.11  1994/12/20  11:23:48  sbs
 * extern decl of syntax_tree moved to scnpars.h
 *
 * Revision 1.10  1994/12/16  14:20:59  sbs
 * imp_tab inserted and NIF macro enlarged
 *
 * Revision 1.9  1994/12/15  14:19:11  asi
 * added member char *mask[2] to union node->info
 *
 * Revision 1.8  1994/12/14  16:51:24  sbs
 * type->name for T_user inserted
 *
 * Revision 1.7  1994/12/14  10:59:16  sbs
 * T_user inserted
 *
 * Revision 1.6  1994/12/14  10:48:25  asi
 * T_unknown added
 *
 * Revision 1.4  1994/12/01  17:43:43  hw
 * inserted struct NODE *node; to typedef struct IDS
 *  changed parameters of NIF
 *
 * Revision 1.3  1994/11/29  10:52:01  hw
 * added pointer to struct NODE to struct ids
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#ifndef _sac_tree_h

#define _sac_tree_h

#define SHP_SEG_SIZE 5

/*
 *  first some intermediate nodes needed for yacc
 */

typedef struct NUMS {
    int num;
    struct NUMS *next;
} nums;
/*
 *  now, the nodes generated from lex/yacc
 */

/* typedef enum { T_int, T_float, T_bool, T_hidden, T_user, T_unknown } simpletype;
 */

#define TYP_IF(n, s) n

typedef enum {
#include "type_info.mac"
} simpletype;

#undef TYP_IF

typedef enum { A_let, A_sel, A_for, A_ret } assigntype;
typedef enum { E_int, E_float, E_bool, E_prf, E_id, E_ap, E_with, E_sel } exprtype;
typedef enum { L_for, L_do, L_while } looptype;
typedef enum { ARG_int, ARG_float, ARG_id } argtype;
typedef enum { C_gen, C_mod } contype;

typedef char id;

typedef struct IDS {
    id *id;
    struct NODE *node;
    struct IDS *next;
} ids;

typedef struct SHPSEG {
    int shp[SHP_SEG_SIZE];
    struct SHPSEG *next;
} shpseg;

typedef struct TYPES {
    simpletype simpletype;
    char *name; /* only used for T_user !! */
    int dim;    /* if (dim == 0) => simpletype */
    shpseg *shpseg;
    struct TYPES *next; /* only needed for fun-results */
    id *id;             /* Bezeichner  */
} types;

/*
 * Neue Knoten fu"r yacc un den Syntabaum
 *
 */

#define NIF(n, s, i, f, p, t, o, x, y, z) n

typedef enum {
#include "node_info.mac"
} nodetype; /* Typ fu"r die node Knoten des Syntaxbaums */

#undef NIF

#define PRF_IF(n, s, x) n

typedef enum {
#include "prf_node_info.mac"
} prf;

#undef PRF_IF

typedef struct NODE {
    nodetype nodetype;
    union {
        ids *ids;         /* Liste von Identifikatoren          */
        id *id;           /* Identifikator                      */
        types *types;     /* Typinformation                     */
        int cint;         /* Integer-Wert                       */
        float cfloat;     /* Float-Wert                         */
        prf prf;          /* tag for primitive functions        */
        char *mask[2];    /* Variablen, die in einem Grundblock */
                          /* 1) "uberschrieben werden           */
                          /* 2) benutzt werden                  */
    } info;               /* fu"r spezielle Informationen */
    int nnode;            /* Anzahl der benutzten Knoten */
    int lineno;           /* Zeilennummer in der ein Befehl steht */
                          /* bzw. Nummer der Varible nach dem Optimieren */
    struct NODE *node[4]; /* Diese Eintra"ge sind knotenspezifisch */
} node;                   /* Knoten des Syntaxbaums  */

/*
 *  macro for the generation of nodes
 */

#define GEN_NODE(type) (type *)malloc (sizeof (type))

extern node *MakeNode (nodetype nodetype);
extern node *AppendNodeChain (int pos, node *first, node *second);

#endif /* _sac_tree_h */
