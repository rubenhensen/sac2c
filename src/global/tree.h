/*
 *
 * $Log$
 * Revision 1.33  1995/04/11 11:34:45  asi
 * added 'flag' to struct 'node'
 *
 * Revision 1.32  1995/04/07  05:56:42  sbs
 * SHP_SEG_SIZE turned from 5 to 16 !
 *
 * Revision 1.31  1995/04/06  11:38:26  asi
 * MAX_MASK set to 6
 *
 * Revision 1.30  1995/03/15  18:40:35  asi
 * added refcnt to struct nchain
 *
 * Revision 1.29  1995/03/14  14:12:42  asi
 * added new entry to struct node (int bblock)
 *
 * Revision 1.28  1995/03/13  15:47:32  hw
 * MakeIds inserted
 *
 * Revision 1.27  1995/03/13  15:12:34  asi
 * added new structur 'nchain'
 * added new entry in structur 'ids' -> 'nchain'
 *
 * Revision 1.26  1995/03/08  10:28:57  hw
 * - added new entry to struct ids (int refcnt)
 * - added new entry to struct node (int refcnt)
 *
 * Revision 1.25  1995/02/28  18:25:26  asi
 * added varno in structure node
 *
 * Revision 1.24  1995/02/02  14:54:36  hw
 * bug fixed prf_dec is now a struct
 *
 * Revision 1.23  1995/01/31  14:59:33  asi
 * opt4_tab inserted and NIF macro enlarged
 *
 * Revision 1.22  1995/01/31  10:57:29  hw
 * added new entrie in union 'info' of struct 'node'
 *
 * Revision 1.21  1995/01/18  17:39:17  asi
 * MAX_MASK inserted
 *
 * Revision 1.20  1995/01/05  12:37:02  sbs
 * third component for type_info.mac inserted
 *
 * Revision 1.19  1995/01/02  11:20:44  asi
 * changed type of mask from char to long
 *
 * Revision 1.18  1995/01/02  10:50:03  asi
 * *** empty log message ***
 *
 * Revision 1.17  1994/12/30  16:57:48  sbs
 * added MakeTypes
 *
 * Revision 1.16  1994/12/30  13:49:08  hw
 * *** empty log message ***
 *
 * Revision 1.15  1994/12/30  13:22:09  hw
 * changed struct types (added id_mod & name_mod)
 * new struct fun_name
 * added fun_name to node.info
 *
 * Revision 1.14  1994/12/21  11:34:50  hw
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

#define SHP_SEG_SIZE 16

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

#define TYP_IF(n, s, p) n

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

typedef struct NCHAIN {
    int refcnt;
    struct NCHAIN *next;
    struct NODE *node;
} nchain;

typedef struct IDS {
    id *id;
    int refcnt;
    struct NODE *node;     /* ptr. to decleration */
    struct NCHAIN *nchain; /* ptr. to definition(s) resp. usage(s) */
    struct IDS *next;
} ids;

typedef struct SHPSEG {
    int shp[SHP_SEG_SIZE];
    struct SHPSEG *next;
} shpseg;

typedef struct TYPES {
    simpletype simpletype;
    char *name;     /* only used for T_user !! */
    char *name_mod; /* name of modul belonging to 'name' */
    int dim;        /* if (dim == 0) => simpletype */
    shpseg *shpseg;
    struct TYPES *next; /* only needed for fun-results */
    id *id;             /* Bezeichner  */
    char *id_mod;       /* name of modul belonging to 'id' */
} types;

typedef struct FUN_NAME {
    char *id;     /* name of function */
    char *id_mod; /* name of modul belonging to 'id' */
} fun_name;

/*
 * Neue Knoten fu"r yacc un den Syntabaum
 *
 */

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d) n

typedef enum {
#include "node_info.mac"
} nodetype; /* Typ fu"r die node Knoten des Syntaxbaums */

#undef NIF

#define PRF_IF(n, s, x) n

typedef enum {
#include "prf_node_info.mac"
} prf;

#undef PRF_IF

#define MAX_MASK 6

typedef struct NODE {
    nodetype nodetype;
    union {
        ids *ids;          /* Liste von Identifikatoren          */
        id *id;            /* Identifikator                      */
        types *types;      /* Typinformation                     */
        int cint;          /* Integer-Wert                       */
        float cfloat;      /* Float-Wert                         */
        prf prf;           /* tag for primitive functions        */
        fun_name fun_name; /* used in N_ap nodes                 */
        struct {
            int tag;      /* tag for return type */
            int tc;       /* type class */
        } prf_dec;        /* used for declaration of primitive functions
                           * this declarations are used to look for argument
                           * and result type of primitive functions
                           */
    } info;               /* fu"r spezielle Informationen */
    int refcnt;           /* is used as referenze count information */
    int bblock;           /* number of basic block assign node belongs to */
    int flag;             /* the flag is used for node-status */
                          /* (loop invariant/not loop invariant , ...) */
    int varno;            /* number of variables - 1 */
    long *mask[MAX_MASK]; /* special informations about variables */
    int nnode;            /* Anzahl der benutzten Knoten */
    int lineno;           /* Zeilennummer in der ein Befehl steht */
    struct NODE *node[4]; /* Diese Eintra"ge sind knotenspezifisch */
} node;                   /* Knoten des Syntaxbaums  */

/*
 *  macro for the generation of nodes
 */

#define GEN_NODE(type) (type *)malloc (sizeof (type))

extern types *MakeTypes (simpletype simple);
extern node *MakeNode (nodetype nodetype);
extern node *AppendNodeChain (int pos, node *first, node *second);
extern ids *MakeIds (char *id);

#endif /* _sac_tree_h */
