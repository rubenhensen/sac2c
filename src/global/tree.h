/*
 *
 * $Log$
 * Revision 1.47  1995/07/06 17:29:12  cg
 * statustype modified.
 *
 * Revision 1.46  1995/07/04  08:34:59  hw
 * cdbl in union node.info inserted
 *
 * Revision 1.45  1995/06/30  11:54:52  hw
 * macros for module_name-access added( they are moved from
 * typecheck.c & convert.h
 *
 * Revision 1.44  1995/06/26  14:07:56  hw
 * added new macros (moved from compile.c )
 *
 * Revision 1.43  1995/06/23  12:18:43  hw
 * enlarged macro TYP_IF
 *
 * Revision 1.42  1995/06/06  14:06:28  cg
 * statustype modified.
 *
 * Revision 1.41  1995/06/02  12:15:25  sbs
 * NIF macro prolongated
 *
 * Revision 1.40  1995/06/02  10:02:52  sbs
 * use-node in ids and info.use inserted
 *
 * Revision 1.39  1995/06/01  10:09:55  cg
 * statustype added and status in struct types inserted.
 *
 * Revision 1.38  1995/05/30  12:14:39  cg
 * number of sons in node structure set to 6 by macro MAX_SONS.
 *
 * Revision 1.37  1995/04/24  15:17:18  asi
 * MAX_MASK set to 7
 *
 * Revision 1.36  1995/04/24  15:13:46  asi
 * added AppendIdsChain
 *
 * Revision 1.35  1995/04/21  15:17:06  asi
 * added 'flag' to struct 'ids'
 *
 * Revision 1.34  1995/04/11  15:57:47  asi
 * NIF macro enlarged
 *
 * Revision 1.33  1995/04/11  11:34:45  asi
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

#define TYP_IF(n, s, p, f) n

typedef enum {
#include "type_info.mac"
} simpletype;

#undef TYP_IF

typedef enum { A_let, A_sel, A_for, A_ret } assigntype;
typedef enum { E_int, E_float, E_bool, E_prf, E_id, E_ap, E_with, E_sel } exprtype;
typedef enum { L_for, L_do, L_while } looptype;
typedef enum { ARG_int, ARG_float, ARG_id } argtype;
typedef enum { C_gen, C_mod } contype;
typedef enum {
    ST_regular,   /* normal types */
    ST_unique,    /* unique types */
    ST_reference, /* reference parameter (unique)           */
    ST_ref,       /* Id must be referenced                  */
    ST_deref,     /* Id must be dereferenced                */
    ST_artificial /* unique type inserted during            */
                  /* signature expansion                    */
} statustype;
typedef enum { VECT, IDX } useflag;

typedef char id;

typedef struct NCHAIN {
    int refcnt;
    struct NCHAIN *next;
    struct NODE *node;
} nchain;

typedef struct IDS {
    id *id;
    int refcnt;
    int flag;              /* the flag is used for ids-status */
                           /* (loop invariant/not loop invariant , ...) */
    struct NODE *node;     /* ptr. to decleration */
    struct NCHAIN *nchain; /* ptr. to definition(s) resp. usage(s) */
    struct NODE *use;      /* ptr. to usage chain (used only if the var */
                           /* is a one dimensional array! */
    statustype attrib;     /* ref/deref attribute */
    statustype status;     /* regular or artificial */

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
    statustype attrib;  /* uniqueness attribute */
    statustype status;  /* regular or artificial */
} types;

typedef struct FUN_NAME {
    char *id;     /* name of function */
    char *id_mod; /* name of modul belonging to 'id' */
} fun_name;

/*
 * Neue Knoten fu"r yacc un den Syntabaum
 *
 */

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d, e, g, h, j, k, l, m, aa) n

typedef enum {
#include "node_info.mac"
} nodetype; /* Typ fu"r die node Knoten des Syntaxbaums */

#undef NIF

#define PRF_IF(n, s, x) n

typedef enum {
#include "prf_node_info.mac"
} prf;

#undef PRF_IF

#define MAX_MASK 7
#define MAX_SONS 6

typedef struct NODE {
    nodetype nodetype;
    union {
        ids *ids;          /* list  of identifiers               */
        id *id;            /* identifier                         */
        types *types;      /* typeinformation                    */
        int cint;          /* integer value                      */
        float cfloat;      /* float value                        */
        double cdbl;       /* double value                       */
        prf prf;           /* tag for primitive functions        */
        fun_name fun_name; /* used in N_ap nodes                 */
        useflag use;       /* used in N_vect_info nodes          */
        struct {
            int tag;             /* tag for return type */
            int tc;              /* type class */
        } prf_dec;               /* used for declaration of primitive functions
                                  * this declarations are used to look for argument
                                  * and result type of primitive functions
                                  */
    } info;                      /* fu"r spezielle Informationen */
    int refcnt;                  /* is used as referenze count information */
    int bblock;                  /* number of basic block assign node belongs to */
    int flag;                    /* the flag is used for node-status */
                                 /* (loop invariant/not loop invariant , ...) */
    int varno;                   /* number of variables - 1 */
    long *mask[MAX_MASK];        /* special informations about variables */
    int nnode;                   /* Anzahl der benutzten Knoten */
    int lineno;                  /* Zeilennummer in der ein Befehl steht */
    struct NODE *node[MAX_SONS]; /* Diese Eintra"ge sind knotenspezifisch */
} node;                          /* Knoten des Syntaxbaums  */

/*
 *  macro for the generation of nodes
 */

#define GEN_NODE(type) (type *)malloc (sizeof (type))

#define MAKENODE_NUM(no, nr)                                                             \
    no = MakeNode (N_num);                                                               \
    no->info.cint = nr

#define MAKENODE_ID(no, str)                                                             \
    no = MakeNode (N_id);                                                                \
    no->IDS = MakeIds (str)

#define MAKENODE_ID_REUSE_IDS(no, Ids)                                                   \
    no = MakeNode (N_id);                                                                \
    no->IDS = Ids

/*
 * macros for module_name-access
 */

#define MOD_NAME_CON "__"
#define MOD(a) (NULL == a) ? "" : a
#define MOD_CON(a) (NULL == a) ? "" : MOD_NAME_CON
#define MOD_NAME(a) MOD (a), MOD_CON (a)

extern types *MakeTypes (simpletype simple);
extern node *MakeNode (nodetype nodetype);
extern node *AppendNodeChain (int pos, node *first, node *second);
extern ids *MakeIds (char *id);
extern ids *AppendIdsChain (ids *first, ids *second);

#endif /* _sac_tree_h */
