/*
 *
 * $Log$
 * Revision 1.2  1994/11/10 15:44:34  sbs
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

typedef enum { T_int, T_float, T_bool } simpletype;
typedef enum { A_let, A_sel, A_for, A_ret } assigntype;
typedef enum { E_int, E_float, E_bool, E_prf, E_id, E_ap, E_with, E_sel } exprtype;
typedef enum { L_for, L_do, L_while } looptype;
typedef enum { ARG_int, ARG_float, ARG_id } argtype;
typedef enum { C_gen, C_mod } contype;

typedef char id;

typedef struct IDS {
    id *id;
    struct IDS *next;
} ids;

typedef struct SHPSEG {
    int shp[SHP_SEG_SIZE];
    struct SHPSEG *next;
} shpseg;

typedef struct TYPES {
    simpletype simpletype;
    int dim; /* if (dim == 0) => simpletype */
    shpseg *shpseg;
    struct TYPES *next; /* only needed for fun-results */
    id *id;             /* Bezeichner  */
} types;

/*
 * Neue Knoten fu"r yacc un den Syntabaum
 *
 */

#define NIF(n, s, x, y, z) n

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
        ids *ids;     /* Liste von Identifikatoren  */
        id *id;       /* Identifikator              */
        types *types; /* Typinformation             */
        int cint;     /* Integer-Wert               */
        float cfloat; /* Float-Wert                 */
        prf prf;      /* tag for primitive functions */

    } info;               /* fu"r spezielle Informationen */
    int nnode;            /* Anzahl der benutzten Knoten */
    int lineno;           /* Zeilennummer in der ein Befehl steht */
    struct NODE *node[4]; /* Diese Eintra"ge sind knotenspezifisch */
} node;                   /* Knoten des Syntaxbaums  */

extern node *syntax_tree;
/*
 *  macro for the generation of nodes
 */

#define GEN_NODE(type) (type *)malloc (sizeof (type))

#endif /* _sac_tree_h */
