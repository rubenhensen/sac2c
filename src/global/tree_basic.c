/*
 *
 * $Log$
 * Revision 1.9  1995/12/01 17:09:20  cg
 * added new node type N_pragma
 * removed macro FUNDEF_ALIAS
 *
 * Revision 1.8  1995/11/16  19:40:20  cg
 * new function MakeStr for generating N_str node
 *
 * Revision 1.7  1995/11/02  13:13:31  cg
 * added new macros OBJDEF_ARG(n) and renamed IDS_DECL(i)
 * to IDS_VARDEC(i).
 *
 * Revision 1.6  1995/11/01  07:08:46  sbs
 * neutral addded to N_foldprf;
 * some DBUG_PRINTS inserted
 *
 * Revision 1.5  1995/10/20  13:45:58  cg
 * added DBUG_PRINT in MakeNodelist
 *
 * Revision 1.4  1995/10/19  10:05:54  cg
 * memory allocation now via function 'Malloc` from internal_lib
 *
 * Revision 1.3  1995/10/06  17:16:50  cg
 * basic access facilities for new type nodelist added
 * IDS structure modified to store global objects.
 * MakeIds extended to 3 parameters
 * basic facilities for compiler steps obj-analysis and fun-analysis added.
 *
 * Revision 1.2  1995/09/29  17:50:51  cg
 * new access structures for strings, nums, shpseg.
 * shape handling modified.
 *
 * Revision 1.1  1995/09/27  15:13:12  cg
 * Initial revision
 *
 *
 *
 */

#include <malloc.h>

#include "types.h"
#include "tree_basic.h"

#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "scnprs.h"
#include "free.h"
#include "internal_lib.h"

#define PRF_IF(n, s, x, y) y

char *prf_name_str[] = {
#include "prf_node_info.mac"
};

#undef PRF_IF

/*--------------------------------------------------------------------------*/
/* local macros for heap allocation                                         */
/*--------------------------------------------------------------------------*/

#define ALLOCATE(var, type) var = (type *)Malloc (sizeof (type))

/*--------------------------------------------------------------------------*/
/* local macros for node initialization                                     */
/*--------------------------------------------------------------------------*/

#define INIT_NODE(v)                                                                     \
    {                                                                                    \
        int i;                                                                           \
        ALLOCATE (v, node);                                                              \
        v->info.id = NULL;                                                               \
        v->refcnt = 0;                                                                   \
        v->flag = 0;                                                                     \
        v->varno = 0;                                                                    \
        v->lineno = linenum;                                                             \
        for (i = 0; i < MAX_SONS; i++)                                                   \
            v->node[i] = NULL;                                                           \
        for (i = 0; i < MAX_MASK; i++)                                                   \
            v->mask[i] = NULL;                                                           \
    }

/*--------------------------------------------------------------------------*/
/* more local macros                                                        */
/*--------------------------------------------------------------------------*/

#define NODE_NNODE(n) (n->nnode)

/*--------------------------------------------------------------------------*/
/*  Make-functions for non-node structures                                  */
/*--------------------------------------------------------------------------*/

shpseg *
MakeShpseg (nums *numsp)
{
    shpseg *tmp;
    int i = 0;
    nums *oldnumsp;

    DBUG_ENTER ("MakeShpseg");

    ALLOCATE (tmp, shpseg);

    tmp->next = NULL;

    for (i = 0; i < SHP_SEG_SIZE; i++) {
        SHPSEG_SHAPE (tmp, i) = -1;
    }

    while (numsp != NULL) {
        if (i >= SHP_SEG_SIZE) {
            SYSABORT (("Maximum number of dimensions exceeded"));
        }

        SHPSEG_SHAPE (tmp, i) = NUMS_NUM (numsp);

        DBUG_PRINT ("GENTREE", ("shape-element: %d", numsp->num));

        i++;
        oldnumsp = numsp;
        numsp = NUMS_NEXT (numsp);
        free (oldnumsp);
    }

    DBUG_RETURN (tmp);
}

types *
MakeType (simpletype basetype, int dim, shpseg *shpseg, char *name, char *mod)
{
    types *tmp;

    DBUG_ENTER ("MakeType");

    ALLOCATE (tmp, types);

    tmp->simpletype = basetype;
    tmp->name = name;
    tmp->name_mod = mod;
    tmp->shpseg = shpseg;
    tmp->dim = dim;

    tmp->next = NULL;

    tmp->id = NULL;
    tmp->id_mod = NULL;

    tmp->status = ST_regular;
    tmp->attrib = ST_regular;

    DBUG_RETURN (tmp);
}

ids *
MakeIds (char *name, char *mod, statustype status)
{
    ids *tmp;
    DBUG_ENTER ("MakeIds");

    ALLOCATE (tmp, ids);
    IDS_NAME (tmp) = name;
    IDS_MOD (tmp) = mod;
    IDS_REFCNT (tmp) = 0;
    IDS_NEXT (tmp) = NULL;
    IDS_VARDEC (tmp) = NULL;
    IDS_DEF (tmp) = NULL;
    IDS_USE (tmp) = NULL;
    IDS_STATUS (tmp) = status;
    IDS_ATTRIB (tmp) = (mod == NULL) ? ST_local : ST_global;

    DBUG_RETURN (tmp);
}

nums *
MakeNums (int num, nums *next)
{
    nums *tmp;
    DBUG_ENTER ("MakeNums");

    ALLOCATE (tmp, nums);
    NUMS_NUM (tmp) = num;
    NUMS_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

strings *
MakeStrings (char *string, strings *next)
{
    strings *tmp;
    DBUG_ENTER ("MakeStrings");

    ALLOCATE (tmp, strings);
    STRINGS_STRING (tmp) = string;
    STRINGS_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

nodelist *
MakeNodelist (node *node, statustype status, nodelist *next)
{
    nodelist *tmp;
    DBUG_ENTER ("MakeNodelist");

    DBUG_PRINT ("ANA", ("New nodelist entry : %s (%s)", ItemName (node),
                        mdb_nodetype[NODE_TYPE (node)]));

    ALLOCATE (tmp, nodelist);
    NODELIST_NODE (tmp) = node;
    NODELIST_ATTRIB (tmp) = ST_unresolved;
    NODELIST_STATUS (tmp) = status;
    NODELIST_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/
/*  Make-functions for node structures                                      */
/*--------------------------------------------------------------------------*/

node *
MakeModul (char *name, file_type filetype, node *imports, node *types, node *objs,
           node *funs)
{
    node *tmp;
    DBUG_ENTER ("MakeModul");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_modul;
    NODE_NNODE (tmp) = 4;

    MODUL_FILETYPE (tmp) = filetype;
    MODUL_IMPORTS (tmp) = imports;
    MODUL_TYPES (tmp) = types;
    MODUL_OBJS (tmp) = objs;
    MODUL_FUNS (tmp) = funs;
    MODUL_NAME (tmp) = name;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeModdec (char *name, char *prefix, node *imports, node *exports)
{
    node *tmp;
    DBUG_ENTER ("MakeModdec");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_moddec;
    NODE_NNODE (tmp) = 2;

    MODDEC_NAME (tmp) = name;
    MODDEC_PREFIX (tmp) = prefix;
    MODDEC_IMPORTS (tmp) = imports;
    MODDEC_OWN (tmp) = exports;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeClassdec (char *name, char *prefix, node *imports, node *exports)
{
    node *tmp;
    DBUG_ENTER ("MakeClassdec");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_classdec;
    NODE_NNODE (tmp) = 2;

    CLASSDEC_NAME (tmp) = name;
    CLASSDEC_PREFIX (tmp) = prefix;
    CLASSDEC_IMPORTS (tmp) = imports;
    CLASSDEC_OWN (tmp) = exports;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeSib (node *types, node *funs, strings *linklist)
{
    node *tmp;
    DBUG_ENTER ("MakeSib");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_sib;
    NODE_NNODE (tmp) = 2;

    SIB_TYPES (tmp) = types;
    SIB_FUNS (tmp) = funs;
    SIB_LINKLIST (tmp) = linklist;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeImplist (char *name, ids *itypes, ids *etypes, ids *objs, ids *funs, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeImplist");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_implist;
    NODE_NNODE (tmp) = 0;

    IMPLIST_NAME (tmp) = name;
    IMPLIST_ITYPES (tmp) = itypes;
    IMPLIST_ETYPES (tmp) = etypes;
    IMPLIST_OBJS (tmp) = objs;
    IMPLIST_FUNS (tmp) = funs;
    IMPLIST_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeExplist (node *itypes, node *etypes, node *objs, node *funs)
{
    node *tmp;
    DBUG_ENTER ("MakeExplist");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_explist;
    NODE_NNODE (tmp) = 4;

    EXPLIST_ITYPES (tmp) = itypes;
    EXPLIST_ETYPES (tmp) = etypes;
    EXPLIST_OBJS (tmp) = objs;
    EXPLIST_FUNS (tmp) = funs;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeTypedef (char *name, char *mod, types *type, statustype attrib, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeTypedef");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_typedef;
    NODE_NNODE (tmp) = 1;

    TYPEDEF_TYPE (tmp) = type;
    TYPEDEF_NAME (tmp) = name;
    TYPEDEF_MOD (tmp) = mod;
    TYPEDEF_ATTRIB (tmp) = attrib;
    TYPEDEF_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeObjdef (char *name, char *mod, types *type, node *expr, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeObjdef");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_objdef;
    NODE_NNODE (tmp) = 2;

    OBJDEF_TYPE (tmp) = type;
    OBJDEF_NAME (tmp) = name;
    OBJDEF_MOD (tmp) = mod;
    OBJDEF_EXPR (tmp) = expr;
    OBJDEF_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeFundef (char *name, char *mod, types *types, node *args, node *body, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeFundef");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_fundef;
    NODE_NNODE (tmp) = 3;

    FUNDEF_TYPES (tmp) = types;
    FUNDEF_NAME (tmp) = name;
    FUNDEF_MOD (tmp) = mod;
    FUNDEF_ARGS (tmp) = args;
    FUNDEF_BODY (tmp) = body;
    FUNDEF_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeArg (char *name, types *type, statustype status, statustype attrib, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeArg");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_arg;
    NODE_NNODE (tmp) = 1;

    ARG_TYPE (tmp) = type;
    ARG_NAME (tmp) = name;
    ARG_STATUS (tmp) = status;
    ARG_ATTRIB (tmp) = attrib;
    ARG_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeBlock (node *instr, node *vardec)
{
    node *tmp;
    DBUG_ENTER ("MakeBlock");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_block;
    NODE_NNODE (tmp) = 2;

    BLOCK_INSTR (tmp) = instr;
    BLOCK_VARDEC (tmp) = vardec;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT " instr: " P_FORMAT " vardec: " P_FORMAT,
                 NODE_LINE (tmp), mdb_nodetype[NODE_TYPE (tmp)], tmp, BLOCK_INSTR (tmp),
                 BLOCK_VARDEC (tmp)));

    DBUG_RETURN (tmp);
}

node *
MakeVardec (char *name, types *type, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeVardec");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_vardec;
    NODE_NNODE (tmp) = 1;

    VARDEC_TYPE (tmp) = type;
    VARDEC_NAME (tmp) = name;
    VARDEC_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeAssign (node *instr, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeAssign");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_assign;
    NODE_NNODE (tmp) = 2;

    ASSIGN_INSTR (tmp) = instr;
    ASSIGN_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT " instr: " P_FORMAT " next: " P_FORMAT,
                 NODE_LINE (tmp), mdb_nodetype[NODE_TYPE (tmp)], tmp, ASSIGN_INSTR (tmp),
                 ASSIGN_NEXT (tmp)));

    DBUG_RETURN (tmp);
}

node *
MakeLet (node *expr, ids *ids)
{
    node *tmp;
    DBUG_ENTER ("MakeLet");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_let;
    NODE_NNODE (tmp) = 1;

    LET_EXPR (tmp) = expr;
    LET_IDS (tmp) = ids;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeCast (node *expr, types *type)
{
    node *tmp;
    DBUG_ENTER ("MakeCast");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_cast;
    NODE_NNODE (tmp) = 1;

    CAST_TYPE (tmp) = type;
    CAST_EXPR (tmp) = expr;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeReturn (node *exprs)
{
    node *tmp;
    DBUG_ENTER ("MakeReturn");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_return;
    NODE_NNODE (tmp) = 1;

    RETURN_EXPRS (tmp) = exprs;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeCond (node *cond, node *Then, node *Else)
{
    node *tmp;
    DBUG_ENTER ("MakeCond");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_cond;
    NODE_NNODE (tmp) = 3;

    COND_COND (tmp) = cond;
    COND_THEN (tmp) = Then;
    COND_ELSE (tmp) = Else;

    INIT_NODE (tmp->node[3]);

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeDo (node *cond, node *body)
{
    node *tmp;
    DBUG_ENTER ("MakeDo");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_do;
    NODE_NNODE (tmp) = 2;

    DO_COND (tmp) = cond;
    DO_BODY (tmp) = body;

    INIT_NODE (tmp->node[2]);

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeWhile (node *cond, node *body)
{
    node *tmp;
    DBUG_ENTER ("MakeWhile");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_while;
    NODE_NNODE (tmp) = 2;

    WHILE_COND (tmp) = cond;
    WHILE_BODY (tmp) = body;

    INIT_NODE (tmp->node[2]);

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeAp (char *name, char *mod, node *args)
{
    node *tmp;
    DBUG_ENTER ("MakeAp");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_ap;
    NODE_NNODE (tmp) = 1;

    AP_NAME (tmp) = name;
    AP_MOD (tmp) = mod;
    AP_ARGS (tmp) = args;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeWith (node *gen, node *body)
{
    node *tmp;
    DBUG_ENTER ("MakeWith");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_with;
    NODE_NNODE (tmp) = 2;

    WITH_GEN (tmp) = gen;
    WITH_BODY (tmp) = body;

    INIT_NODE (tmp->node[2]);

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeGenerator (node *left, node *right, char *id)
{
    node *tmp;
    DBUG_ENTER ("MakeGenerator");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_generator;
    NODE_NNODE (tmp) = 2;

    GEN_LEFT (tmp) = left;
    GEN_RIGHT (tmp) = right;
    GEN_ID (tmp) = id;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeGenarray (node *array, node *body)
{
    node *tmp;
    DBUG_ENTER ("MakeGenarray");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_genarray;
    NODE_NNODE (tmp) = 2;

    GENARRAY_ARRAY (tmp) = array;
    GENARRAY_BODY (tmp) = body;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeModarray (node *array, node *body)
{
    node *tmp;
    DBUG_ENTER ("MakeModarray");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_modarray;
    NODE_NNODE (tmp) = 2;

    MODARRAY_ARRAY (tmp) = array;
    MODARRAY_BODY (tmp) = body;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeFoldprf (prf prf, node *body, node *neutral)
{
    node *tmp;
    DBUG_ENTER ("MakeFoldprf");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_foldprf;
    NODE_NNODE (tmp) = 2;

    FOLDPRF_PRF (tmp) = prf;
    FOLDPRF_BODY (tmp) = body;
    FOLDPRF_NEUTRAL (tmp) = neutral;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT " body: " P_FORMAT " neutral: " P_FORMAT,
                 NODE_LINE (tmp), mdb_nodetype[NODE_TYPE (tmp)], tmp, FOLDPRF_BODY (tmp),
                 FOLDPRF_NEUTRAL (tmp)));

    DBUG_RETURN (tmp);
}

node *
MakeFoldfun (char *name, char *mod, node *body, node *neutral)
{
    node *tmp;
    DBUG_ENTER ("MakeFoldfun");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_foldfun;
    NODE_NNODE (tmp) = 2;

    FOLDFUN_NAME (tmp) = name;
    FOLDFUN_MOD (tmp) = mod;
    FOLDFUN_BODY (tmp) = body;
    FOLDFUN_NEUTRAL (tmp) = neutral;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

extern node *
MakeExprs (node *expr, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeExprs");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_exprs;
    NODE_NNODE (tmp) = 2;

    EXPRS_EXPR (tmp) = expr;
    EXPRS_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeArray (node *aelems)
{
    node *tmp;
    DBUG_ENTER ("MakeArray");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_array;
    NODE_NNODE (tmp) = 1;

    ARRAY_AELEMS (tmp) = aelems;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeVinfo (useflag flag, shapes *shp, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeVinfo");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_vinfo;
    NODE_NNODE (tmp) = 1;

    VINFO_FLAG (tmp) = flag;
    VINFO_SHP (tmp) = shp;
    VINFO_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeId (char *name, char *mod, statustype status)
{
    node *tmp;
    DBUG_ENTER ("MakeId");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_id;
    NODE_NNODE (tmp) = 0;

    tmp->info.ids = MakeIds (name, mod, status);

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeNum (int val)
{
    node *tmp;
    DBUG_ENTER ("MakeNum");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_num;
    NODE_NNODE (tmp) = 0;

    NUM_VAL (tmp) = val;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeFloat (float val)
{
    node *tmp;
    DBUG_ENTER ("MakeFloat");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_float;
    NODE_NNODE (tmp) = 0;

    FLOAT_VAL (tmp) = val;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeDouble (double val)
{
    node *tmp;
    DBUG_ENTER ("MakeDouble");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_double;
    NODE_NNODE (tmp) = 0;

    DOUBLE_VAL (tmp) = val;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeBool (int val)
{
    node *tmp;
    DBUG_ENTER ("MakeBool");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_bool;
    NODE_NNODE (tmp) = 0;

    BOOL_VAL (tmp) = val;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeStr (char *str)
{
    node *tmp;
    DBUG_ENTER ("MakeStr");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_str;
    NODE_NNODE (tmp) = 0;

    STR_STRING (tmp) = str;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakePrf (prf prf, node *args)
{
    node *tmp;
    DBUG_ENTER ("MakePrf");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_prf;
    NODE_NNODE (tmp) = 1;

    PRF_PRF (tmp) = prf;
    PRF_ARGS (tmp) = args;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeEmpty ()
{
    node *tmp;
    DBUG_ENTER ("MakeEmpty");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_empty;
    NODE_NNODE (tmp) = 0;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakePost (int incdec, char *id)
{
    node *tmp;
    DBUG_ENTER ("MakePost");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_post;
    NODE_NNODE (tmp) = 0;

    POST_ID (tmp) = id;

    /*
     *  The following lines contain the new representation for INCDEC.
     *  The access macro POST_INCDEC may not be used until its definition in
     *  tree_basic.h is converted to the new representation.
     */

    /*  POST_INCDEC(tmp)=incdec;  */
    tmp->varno = incdec;

    /*
     *  The following lines contain the old representation for INCDEC.
     *  This is needed until all files are converted to the new representation.
     */

    INIT_NODE (tmp->node[0]);
    tmp->node[0]->nodetype = (0 == incdec ? N_dec : N_inc);

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakePre (nodetype incdec, char *id)
{
    node *tmp;
    DBUG_ENTER ("MakePre");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_pre;
    NODE_NNODE (tmp) = 0;

    PRE_ID (tmp) = id;

    /*
     *  The following lines contain the new representation for INCDEC.
     *  The access macro PRE_INCDEC may not be used until its definition in
     *  tree_basic.h is converted to the new representation.
     */

    /*  PRE_INCDEC(tmp)=incdec;  */
    tmp->varno = incdec;

    /*
     *  The following lines contain the old representation for INCDEC.
     *  This is needed until all files are converted to the new representation.
     */

    INIT_NODE (tmp->node[0]);
    tmp->node[0]->nodetype = (0 == incdec ? N_dec : N_inc);

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp),
                             mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakeIcm (char *name, node *args, node *next)
{
    node *tmp;
    DBUG_ENTER ("MakeIcm");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_icm;
    NODE_NNODE (tmp) = 2;

    ICM_NAME (tmp) = name;
    ICM_ARGS (tmp) = args;
    ICM_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, /**/
                             NODE_LINE (tmp), mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}

node *
MakePragma ()
{
    node *tmp;
    DBUG_ENTER ("MakePragma");
    INIT_NODE (tmp);

    NODE_TYPE (tmp) = N_pragma;
    NODE_NNODE (tmp) = 0;

    DBUG_PRINT ("MAKENODE", ("%d:nodetype: %s " P_FORMAT, /**/
                             NODE_LINE (tmp), mdb_nodetype[NODE_TYPE (tmp)], tmp));

    DBUG_RETURN (tmp);
}
