/*
 *
 * $Log$
 * Revision 1.8  2000/02/22 15:45:58  jhs
 * Fixed misspelling.
 *
 * Revision 1.7  2000/02/22 11:58:36  jhs
 * Added and adapted NODE_TEXT.
 *
 * Revision 1.6  2000/02/10 15:48:13  jhs
 * Added NWITH2_ISSCHEDULED.
 *
 * Revision 1.5  2000/02/09 14:14:07  dkr
 * WLSEGVAR_MAXHOMDIM removed
 *
 * Revision 1.4  2000/02/03 15:19:31  jhs
 * Fixed Bugs in MakeMt, MakeST.
 *
 * Revision 1.3  2000/02/02 12:29:09  jhs
 * Added INFO_MUTH_FUNDEF.
 * Added N_mt and N_st.
 * Added ST_xxx-macros, added MT_OR_ST_xxx-macros.
 *
 * Revision 1.2  2000/01/28 12:41:15  dkr
 * NCODE_CODE removed
 *
 * Revision 1.1  2000/01/21 15:38:37  dkr
 * Initial revision
 *
 * Revision 2.12  2000/01/21 12:42:43  dkr
 * function MakeIds1 added
 *
 * Revision 2.11  1999/09/10 14:22:52  jhs
 * Added COND_NAIVE_(THEN|ELSE)VARS.
 * Added MakeId1 und MakeId3 for those ugly macros i killed in tree.[ch].
 *
 * Revision 2.10  1999/08/25 15:30:44  bs
 * MakeNCode modified.
 *
 * Revision 2.9  1999/07/20 16:39:19  jhs
 * Added SYNC_FOLDCOUNT.
 *
 * Revision 2.8  1999/07/19 14:44:31  jhs
 * Changed signature of MakeSync.
 *
 * Revision 2.7  1999/07/07 06:00:19  sbs
 * added VINFO_DOLLAR and adjusted MakeVinfo
 *
 * Revision 2.6  1999/06/15 12:29:09  jhs
 * Added initilization of IDS_NAIVE_REFCNT in routine MakeIds.
 *
 * Revision 2.5  1999/06/08 08:31:02  cg
 * Bug fixed: node structure entry int_data is now initialized.
 * Macro INIT_NODE replaced by function CreateCleanNode()
 * -> resulting object code file much smaller now.
 *
 * Revision 2.4  1999/05/06 15:39:29  sbs
 * Now, the src_file will be set in INIT_NODE as well.
 *
 * Revision 2.3  1999/05/05 13:04:39  jhs
 * MakeNGenerator now sets also the values of the original
 * withloop operators, which are equivalent to the operators
 * delivered to the routine.
 *
 * Revision 2.2  1999/04/29 07:32:26  bs
 * Definition of MakeAccess modified
 *
 * Revision 2.1  1999/02/23 12:39:53  sacbase
 * new release made
 *
 * Revision 1.84  1999/02/06 12:53:01  srs
 * added MakeNodelistNode()
 *
 * [...]
 *
 * Revision 1.1  1995/09/27  15:13:12  cg
 * Initial revision
 *
 */

#include "types.h"
#include "tree_basic.h"

#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "scnprs.h"
#include "free.h"
#include "internal_lib.h"

/*--------------------------------------------------------------------------*/

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
/* local functions for node initialization                                  */
/*--------------------------------------------------------------------------*/

static node *
CreateCleanNode (nodetype nt)
{
    int i;
    node *new_node;

    DBUG_ENTER ("CreateCleanNode");

    ALLOCATE (new_node, node);

    new_node->nodetype = nt;

    new_node->info.id = NULL;
    new_node->info.ids = NULL;
    new_node->info.types = NULL;
    new_node->info.fun_name.id = NULL;
    new_node->info.fun_name.id_mod = NULL;
    new_node->info2 = NULL;

    new_node->refcnt = 0;
    new_node->flag = 0;
    new_node->varno = 0;
    new_node->counter = 0;
    new_node->int_data = 0;

    new_node->lineno = linenum;
    new_node->src_file = filename;

    for (i = 0; i < MAX_SONS; i++) {
        new_node->node[i] = NULL;
    }

    for (i = 0; i < MAX_MASK; i++) {
        new_node->mask[i] = NULL;
        new_node->dfmask[i] = NULL;
    }

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/
/*  Make-functions for non-node structures                                  */
/*--------------------------------------------------------------------------*/

/* attention: the given parameter chain of nums structs is set free here!!!  */
shpseg *
MakeShpseg (nums *numsp)
{
    shpseg *tmp;
    int i;
    nums *oldnumsp;

    DBUG_ENTER ("MakeShpseg");

    ALLOCATE (tmp, shpseg);

    tmp->next = NULL;

    /*   for (i=0;i<SHP_SEG_SIZE;i++) */
    /*     SHPSEG_SHAPE(tmp,i)=-1; */

    i = 0;
    while (numsp != NULL) {
        if (i >= SHP_SEG_SIZE)
            SYSABORT (("Maximum number of dimensions exceeded"));

        SHPSEG_SHAPE (tmp, i) = NUMS_NUM (numsp);

        DBUG_PRINT ("GENTREE", ("shape-element: %d", numsp->num));

        i++;
        oldnumsp = numsp;
        numsp = NUMS_NEXT (numsp);
        FREE (oldnumsp);
    }

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

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
    tmp->id_cmod = NULL;

    tmp->status = ST_regular;
    tmp->attrib = ST_regular;
    tmp->tdef = NULL;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

ids *
MakeIds (char *name, char *mod, statustype status)
{
    ids *tmp;
    DBUG_ENTER ("MakeIds");

    ALLOCATE (tmp, ids);
    IDS_NAME (tmp) = name;
    IDS_MOD (tmp) = mod;
    IDS_REFCNT (tmp) = 0;
    IDS_NAIVE_REFCNT (tmp) = 0;
    IDS_NEXT (tmp) = NULL;
    IDS_VARDEC (tmp) = NULL;
    IDS_DEF (tmp) = NULL;
    IDS_USE (tmp) = NULL;
    IDS_STATUS (tmp) = status;
    IDS_ATTRIB (tmp) = (mod == NULL) ? ST_regular : ST_global;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

ids *
MakeIds1 (char *name)
{
    ids *result;

    DBUG_ENTER ("MakeIds1");

    result = MakeIds (StringCopy (name), NULL, ST_regular);

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

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

/*--------------------------------------------------------------------------*/

deps *
MakeDeps (char *name, char *decname, char *libname, statustype status, deps *sub,
          deps *next)
{
    deps *tmp;
    DBUG_ENTER ("MakeDeps");

    ALLOCATE (tmp, deps);
    DEPS_NAME (tmp) = name;
    DEPS_DECNAME (tmp) = decname;
    DEPS_LIBNAME (tmp) = libname;
    DEPS_STATUS (tmp) = status;
    DEPS_SUB (tmp) = sub;
    DEPS_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

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

/*--------------------------------------------------------------------------*/

nodelist *
MakeNodelist (node *node, statustype status, nodelist *next)
{
    nodelist *tmp;
    DBUG_ENTER ("MakeNodelist");

    DBUG_PRINT ("ANA",
                ("New nodelist entry : %s (%s)", ItemName (node), NODE_TEXT (node)));

    ALLOCATE (tmp, nodelist);
    NODELIST_NODE (tmp) = node;
    NODELIST_STATUS (tmp) = status;
    NODELIST_NEXT (tmp) = next;

    switch (NODE_TYPE (node)) {
    case N_fundef:
        NODELIST_ATTRIB (tmp) = ST_unresolved;
        break;
    case N_objdef:
        NODELIST_ATTRIB (tmp) = ST_reference;
        break;
    case N_typedef:
        NODELIST_ATTRIB (tmp) = ST_regular;
        break;
    default:
        DBUG_ASSERT (0, ("Wrong node type in MakeNodelist"));
    }

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

nodelist *
MakeNodelistNode (node *node, nodelist *next)
{
    nodelist *tmp;
    DBUG_ENTER ("MakeNodelistNode");

    ALLOCATE (tmp, nodelist);
    NODELIST_NODE (tmp) = node;
    NODELIST_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

access_t *
MakeAccess (node *array, node *iv, accessclass_t class, shpseg *offset,
            accessdir_t direction, access_t *next)
{
    access_t *tmp;

    DBUG_ENTER ("MakeAccess");

    ALLOCATE (tmp, access_t);

    ACCESS_ARRAY (tmp) = array;
    ACCESS_IV (tmp) = iv;
    ACCESS_CLASS (tmp) = class;
    ACCESS_OFFSET (tmp) = offset;
    ACCESS_DIR (tmp) = direction;
    ACCESS_NEXT (tmp) = next;

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

    tmp = CreateCleanNode (N_modul);

    MODUL_FILETYPE (tmp) = filetype;
    MODUL_IMPORTS (tmp) = imports;
    MODUL_TYPES (tmp) = types;
    MODUL_OBJS (tmp) = objs;
    MODUL_FUNS (tmp) = funs;
    MODUL_NAME (tmp) = name;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeModdec (char *name, deps *linkwith, int isexternal, node *imports, node *exports)
{
    node *tmp;

    DBUG_ENTER ("MakeModdec");

    tmp = CreateCleanNode (N_moddec);

    MODDEC_NAME (tmp) = name;
    MODDEC_LINKWITH (tmp) = linkwith;
    MODDEC_ISEXTERNAL (tmp) = isexternal;
    MODDEC_IMPORTS (tmp) = imports;
    MODDEC_OWN (tmp) = exports;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeClassdec (char *name, deps *linkwith, int isexternal, node *imports, node *exports)
{
    node *tmp;

    DBUG_ENTER ("MakeClassdec");

    tmp = CreateCleanNode (N_classdec);

    CLASSDEC_NAME (tmp) = name;
    CLASSDEC_LINKWITH (tmp) = linkwith;
    CLASSDEC_ISEXTERNAL (tmp) = isexternal;
    CLASSDEC_IMPORTS (tmp) = imports;
    CLASSDEC_OWN (tmp) = exports;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeSib (char *name, int linkstyle, deps *linkwith, node *types, node *objs, node *funs)
{
    node *tmp;

    DBUG_ENTER ("MakeSib");

    tmp = CreateCleanNode (N_sib);

    SIB_TYPES (tmp) = types;
    SIB_FUNS (tmp) = funs;
    SIB_OBJS (tmp) = objs;
    SIB_NAME (tmp) = name;
    SIB_LINKSTYLE (tmp) = linkstyle;
    SIB_LINKWITH (tmp) = linkwith;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeImplist (char *name, ids *itypes, ids *etypes, ids *objs, ids *funs, node *next)
{
    node *tmp;

    DBUG_ENTER ("MakeImplist");

    tmp = CreateCleanNode (N_implist);

    IMPLIST_NAME (tmp) = name;
    IMPLIST_ITYPES (tmp) = itypes;
    IMPLIST_ETYPES (tmp) = etypes;
    IMPLIST_OBJS (tmp) = objs;
    IMPLIST_FUNS (tmp) = funs;
    IMPLIST_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeExplist (node *itypes, node *etypes, node *objs, node *funs)
{
    node *tmp;

    DBUG_ENTER ("MakeExplist");

    tmp = CreateCleanNode (N_explist);

    EXPLIST_ITYPES (tmp) = itypes;
    EXPLIST_ETYPES (tmp) = etypes;
    EXPLIST_OBJS (tmp) = objs;
    EXPLIST_FUNS (tmp) = funs;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeTypedef (char *name, char *mod, types *type, statustype attrib, node *next)
{
    node *tmp;

    DBUG_ENTER ("MakeTypedef");

    tmp = CreateCleanNode (N_typedef);

    TYPEDEF_TYPE (tmp) = type;
    TYPEDEF_NAME (tmp) = name;
    TYPEDEF_MOD (tmp) = mod;
    TYPEDEF_ATTRIB (tmp) = attrib;
    TYPEDEF_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeObjdef (char *name, char *mod, types *type, node *expr, node *next)
{
    node *tmp;

    DBUG_ENTER ("MakeObjdef");

    tmp = CreateCleanNode (N_objdef);

    OBJDEF_TYPE (tmp) = type;
    OBJDEF_NAME (tmp) = name;
    OBJDEF_MOD (tmp) = mod;
    OBJDEF_EXPR (tmp) = expr;
    OBJDEF_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeFundef (char *name, char *mod, types *types, node *args, node *body, node *next)
{
    node *tmp;

    DBUG_ENTER ("MakeFundef");

    tmp = CreateCleanNode (N_fundef);

    FUNDEF_TYPES (tmp) = types;
    FUNDEF_NAME (tmp) = name;
    FUNDEF_MOD (tmp) = mod;
    FUNDEF_ARGS (tmp) = args;
    FUNDEF_BODY (tmp) = body;
    FUNDEF_NEXT (tmp) = next;
    FUNDEF_FUNNO (tmp) = 0;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeArg (char *name, types *type, statustype status, statustype attrib, node *next)
{
    node *tmp;

    DBUG_ENTER ("MakeArg");

    tmp = CreateCleanNode (N_arg);

    ARG_TYPE (tmp) = type;
    ARG_NAME (tmp) = name;
    ARG_STATUS (tmp) = status;
    ARG_ATTRIB (tmp) = attrib;
    ARG_NEXT (tmp) = next;
    ARG_ACTCHN (tmp) = NULL;
    ARG_COLCHN (tmp) = NULL;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeBlock (node *instr, node *vardec)
{
    node *tmp;

    DBUG_ENTER ("MakeBlock");

    tmp = CreateCleanNode (N_block);

    BLOCK_INSTR (tmp) = instr;
    BLOCK_VARDEC (tmp) = vardec;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT " instr: " P_FORMAT " vardec: " P_FORMAT,
                 NODE_LINE (tmp), NODE_TEXT (tmp), tmp, BLOCK_INSTR (tmp),
                 BLOCK_VARDEC (tmp)));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeVardec (char *name, types *type, node *next)
{
    node *tmp;

    DBUG_ENTER ("MakeVardec");

    tmp = CreateCleanNode (N_vardec);

    VARDEC_TYPE (tmp) = type;
    VARDEC_NAME (tmp) = name;
    VARDEC_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeAssign (node *instr, node *next)
{
    node *tmp;

    DBUG_ENTER ("MakeAssign");

    tmp = CreateCleanNode (N_assign);

    ASSIGN_INSTR (tmp) = instr;
    ASSIGN_NEXT (tmp) = next;
    ASSIGN_INDEX (tmp) = NULL;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT " instr: " P_FORMAT " next: " P_FORMAT,
                 NODE_LINE (tmp), NODE_TEXT (tmp), tmp, ASSIGN_INSTR (tmp),
                 ASSIGN_NEXT (tmp)));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeLet (node *expr, ids *ids)
{
    node *tmp;

    DBUG_ENTER ("MakeLet");

    tmp = CreateCleanNode (N_let);

    LET_EXPR (tmp) = expr;
    LET_IDS (tmp) = ids;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeCast (node *expr, types *type)
{
    node *tmp;

    DBUG_ENTER ("MakeCast");

    tmp = CreateCleanNode (N_cast);

    CAST_TYPE (tmp) = type;
    CAST_EXPR (tmp) = expr;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeReturn (node *exprs)
{
    node *tmp;

    DBUG_ENTER ("MakeReturn");

    tmp = CreateCleanNode (N_return);

    RETURN_EXPRS (tmp) = exprs;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeCond (node *cond, node *Then, node *Else)
{
    node *tmp;

    DBUG_ENTER ("MakeCond");

    tmp = CreateCleanNode (N_cond);

    COND_COND (tmp) = cond;
    COND_THEN (tmp) = Then;
    COND_ELSE (tmp) = Else;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeDo (node *cond, node *body)
{
    node *tmp;

    DBUG_ENTER ("MakeDo");

    tmp = CreateCleanNode (N_do);

    DO_COND (tmp) = cond;
    DO_BODY (tmp) = body;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeWhile (node *cond, node *body)
{
    node *tmp;

    DBUG_ENTER ("MakeWhile");

    tmp = CreateCleanNode (N_while);

    WHILE_COND (tmp) = cond;
    WHILE_BODY (tmp) = body;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
While2Do (node *while_node)
{
    DBUG_ENTER ("While2Do");
    NODE_TYPE (while_node) = N_do;
    DBUG_RETURN (while_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeAnnotate (int tag, int funno, int funapno)
{
    node *tmp;

    DBUG_ENTER ("MakeAnnotate");

    tmp = CreateCleanNode (N_annotate);

    ANNOTATE_TAG (tmp) = tag;
    ANNOTATE_FUNNUMBER (tmp) = funno;
    ANNOTATE_FUNAPNUMBER (tmp) = funapno;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeAp (char *name, char *mod, node *args)
{
    node *tmp;

    DBUG_ENTER ("MakeAp");

    tmp = CreateCleanNode (N_ap);

    AP_NAME (tmp) = name;
    AP_MOD (tmp) = mod;
    AP_ARGS (tmp) = args;
    AP_ATFLAG (tmp) = 0;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeWith (node *gen, node *operator)
{
    node *tmp;

    DBUG_ENTER ("MakeWith");

    tmp = CreateCleanNode (N_with);

    WITH_GEN (tmp) = gen;
    WITH_OPERATOR (tmp) = operator;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeGenerator (node *left, node *right, char *id)
{
    node *tmp;

    DBUG_ENTER ("MakeGenerator");

    tmp = CreateCleanNode (N_generator);

    GEN_LEFT (tmp) = left;
    GEN_RIGHT (tmp) = right;
    GEN_ID (tmp) = id;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeGenarray (node *array, node *body)
{
    node *tmp;

    DBUG_ENTER ("MakeGenarray");

    tmp = CreateCleanNode (N_genarray);

    GENARRAY_ARRAY (tmp) = array;
    GENARRAY_BODY (tmp) = body;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeModarray (node *array, node *body)
{
    node *tmp;

    DBUG_ENTER ("MakeModarray");

    tmp = CreateCleanNode (N_modarray);

    MODARRAY_ARRAY (tmp) = array;
    MODARRAY_BODY (tmp) = body;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeFoldprf (prf prf, node *body, node *neutral)
{
    node *tmp;

    DBUG_ENTER ("MakeFoldprf");

    tmp = CreateCleanNode (N_foldprf);

    FOLDPRF_PRF (tmp) = prf;
    FOLDPRF_BODY (tmp) = body;
    FOLDPRF_NEUTRAL (tmp) = neutral;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT " body: " P_FORMAT " neutral: " P_FORMAT,
                 NODE_LINE (tmp), NODE_TEXT (tmp), tmp, FOLDPRF_BODY (tmp),
                 FOLDPRF_NEUTRAL (tmp)));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeFoldfun (char *name, char *mod, node *body, node *neutral)
{
    node *tmp;

    DBUG_ENTER ("MakeFoldfun");

    tmp = CreateCleanNode (N_foldfun);

    FOLDFUN_NAME (tmp) = name;
    FOLDFUN_MOD (tmp) = mod;
    FOLDFUN_BODY (tmp) = body;
    FOLDFUN_NEUTRAL (tmp) = neutral;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeExprs (node *expr, node *next)
{
    node *tmp;

    DBUG_ENTER ("MakeExprs");

    tmp = CreateCleanNode (N_exprs);

    EXPRS_EXPR (tmp) = expr;
    EXPRS_NEXT (tmp) = next;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeArray (node *aelems)
{
    node *tmp;

    DBUG_ENTER ("MakeArray");

    tmp = CreateCleanNode (N_array);

    ARRAY_AELEMS (tmp) = aelems;

    ARRAY_TYPE (tmp) = NULL;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeVinfo (useflag flag, types *type, node *next, node *dollar)
{
    node *tmp;

    DBUG_ENTER ("MakeVinfo");

    tmp = CreateCleanNode (N_vinfo);

    VINFO_FLAG (tmp) = flag;
    VINFO_TYPE (tmp) = type;
    VINFO_NEXT (tmp) = next;
    VINFO_DOLLAR (tmp) = dollar;
    VINFO_VARDEC (tmp) = NULL;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeId (char *name, char *mod, statustype status)
{
    node *tmp;

    DBUG_ENTER ("MakeId");

    tmp = CreateCleanNode (N_id);

    tmp->info.ids = MakeIds (name, mod, status);

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeId1 (char *str)
{
    node *result;

    DBUG_ENTER ("MakeId1");

    result = MakeId (StringCopy (str), NULL, ST_regular);

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

node *
MakeId2 (ids *ids_node)
{
    node *tmp;

    DBUG_ENTER ("MakeId2");

    tmp = CreateCleanNode (N_id);

    tmp->info.ids = ids_node;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeId3 (ids *ids_node)
{
    node *tmp;

    DBUG_ENTER ("MakeId3");

    tmp = CreateCleanNode (N_id);

    tmp->info.ids = ids_node;
    tmp->refcnt = ids_node->refcnt;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNum (int val)
{
    node *tmp;

    DBUG_ENTER ("MakeNum");

    tmp = CreateCleanNode (N_num);

    NUM_VAL (tmp) = val;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeChar (char val)
{
    node *tmp;

    DBUG_ENTER ("MakeChar");

    tmp = CreateCleanNode (N_char);

    CHAR_VAL (tmp) = val;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeFloat (float val)
{
    node *tmp;

    DBUG_ENTER ("MakeFloat");

    tmp = CreateCleanNode (N_float);

    FLOAT_VAL (tmp) = val;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeDouble (double val)
{
    node *tmp;

    DBUG_ENTER ("MakeDouble");

    tmp = CreateCleanNode (N_double);

    DOUBLE_VAL (tmp) = val;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeBool (int val)
{
    node *tmp;

    DBUG_ENTER ("MakeBool");

    tmp = CreateCleanNode (N_bool);

    BOOL_VAL (tmp) = val;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeStr (char *str)
{
    node *tmp;

    DBUG_ENTER ("MakeStr");

    tmp = CreateCleanNode (N_str);

    STR_STRING (tmp) = str;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakePrf (prf prf, node *args)
{
    node *tmp;

    DBUG_ENTER ("MakePrf");

    tmp = CreateCleanNode (N_prf);

    PRF_PRF (tmp) = prf;
    PRF_ARGS (tmp) = args;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeEmpty ()
{
    node *tmp;

    DBUG_ENTER ("MakeEmpty");

    tmp = CreateCleanNode (N_empty);

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakePost (int incdec, char *id)
{
    node *tmp;

    DBUG_ENTER ("MakePost");

    tmp = CreateCleanNode (N_post);

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

    tmp->node[0] = CreateCleanNode (0 == incdec ? N_dec : N_inc);

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakePre (nodetype incdec, char *id)
{
    node *tmp;

    DBUG_ENTER ("MakePre");

    tmp = CreateCleanNode (N_pre);

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

    tmp->node[0] = CreateCleanNode (0 == incdec ? N_dec : N_inc);

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeIcm (char *name, node *args, node *next)
{
    node *tmp;

    DBUG_ENTER ("MakeIcm");

    tmp = CreateCleanNode (N_icm);

    ICM_NAME (tmp) = name;
    ICM_ARGS (tmp) = args;
    ICM_NEXT (tmp) = next;

    if (strcmp (name, "MT_START_SYNCBLOCK") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strncmp (name, "MT_SYNC_", 8) == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_NONFOLD_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_FOLD_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_BLOCK_LOOP0_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_BLOCK_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_UBLOCK_LOOP0_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_UBLOCK_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_STRIDE_LOOP0_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_STRIDE_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_STRIDE_UNROLL_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_GRID_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_GRID_UNROLL_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_BLOCK_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_UBLOCK_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_STRIDE_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_STRIDE_UNROLL_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_GRID_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_GRID_UNROLL_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_MT_BLOCK_LOOP0_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_BLOCK_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_UBLOCK_LOOP0_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_UBLOCK_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_STRIDE_LOOP0_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_STRIDE_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_STRIDE_UNROLL_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_GRID_LOOP_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_GRID_UNROLL_BEGIN") == 0)
        ICM_INDENT (tmp) = 1;
    else if (strcmp (name, "WL_MT_BLOCK_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_MT_UBLOCK_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_MT_STRIDE_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_MT_STRIDE_UNROLL_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_MT_GRID_LOOP_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_MT_GRID_UNROLL_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_NONFOLD_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strcmp (name, "WL_FOLD_END") == 0)
        ICM_INDENT (tmp) = -1;
    else if (strncmp (name, "MT_SCHEDULER_", 13) == 0) {
        if (strcmp (name + strlen (name) - 6, "_BEGIN") == 0)
            ICM_INDENT (tmp) = 1;
        else if (strcmp (name + strlen (name) - 4, "_END") == 0)
            ICM_INDENT (tmp) = -1;
        else
            ICM_INDENT (tmp) = 0;
    } else
        ICM_INDENT (tmp) = 0;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakePragma ()
{
    node *tmp;

    DBUG_ENTER ("MakePragma");

    tmp = CreateCleanNode (N_pragma);

    PRAGMA_NUMPARAMS (tmp) = 0;

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeInfo ()
{
    node *tmp;

    DBUG_ENTER ("MakeInfo");

    tmp = CreateCleanNode (N_info);

    DBUG_PRINT ("MAKENODE",
                ("%d:nodetype: %s " P_FORMAT, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeSpmd (node *region)
{
    node *tmp;

    DBUG_ENTER ("MakeSpmd");

    tmp = CreateCleanNode (N_spmd);

    SPMD_REGION (tmp) = region;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeSync (node *region)
{
    node *tmp;

    DBUG_ENTER ("MakeSync");

    tmp = CreateCleanNode (N_sync);

    SYNC_REGION (tmp) = region;
    SYNC_FIRST (tmp) = FALSE;
    SYNC_LAST (tmp) = TRUE;
    SYNC_FOLDCOUNT (tmp) = 0;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeMT (node *region)
{
    node *tmp;

    DBUG_ENTER ("MakeMT");

    tmp = CreateCleanNode (N_mt);
    MT_REGION (tmp) = region;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeST (node *region)
{
    node *tmp;

    DBUG_ENTER ("MakeST");

    tmp = CreateCleanNode (N_st);
    ST_REGION (tmp) = region;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNWith (node *part, node *code, node *withop)
{
    node *tmp;

    DBUG_ENTER ("MakeNWith");

    tmp = CreateCleanNode (N_Nwith);

    NWITH_PART (tmp) = part;
    NWITH_CODE (tmp) = code;
    NWITH_WITHOP (tmp) = withop;

    tmp->info2 = Malloc (sizeof (wl_info));
    NWITH_PARTS (tmp) = -1;
    NWITH_REFERENCED (tmp) = 0;
    NWITH_REFERENCED_FOLD (tmp) = 0;
    NWITH_COMPLEX (tmp) = 0;
    NWITH_FOLDABLE (tmp) = 0;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNPart (node *withid, node *generator, node *code)
{
    node *tmp;

    DBUG_ENTER ("MakeNPart");

    tmp = CreateCleanNode (N_Npart);

    NPART_GEN (tmp) = generator;
    NPART_WITHID (tmp) = withid;
    NPART_CODE (tmp) = code;
    if (code != NULL) {      /* may be NULL in sac.y */
        NCODE_USED (code)++; /* see remarks of N_Ncode in tree_basic.h */
    }

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNWithid (ids *vec, ids *scalars)
{
    node *tmp;

    DBUG_ENTER ("MakeNWithid");

    tmp = CreateCleanNode (N_Nwithid);

    NWITHID_VEC (tmp) = vec;
    NWITHID_IDS (tmp) = scalars;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNGenerator (node *bound1, node *bound2, prf op1, prf op2, node *step, node *width)
{
    node *tmp;

    DBUG_ENTER ("MakeNGenerator");

    tmp = CreateCleanNode (N_Ngenerator);

    NGEN_BOUND1 (tmp) = bound1;
    NGEN_BOUND2 (tmp) = bound2;
    NGEN_OP1 (tmp) = op1;
    NGEN_OP2 (tmp) = op2;
    NGEN_OP1_ORIG (tmp) = op1;
    NGEN_OP2_ORIG (tmp) = op2;
    NGEN_STEP (tmp) = step;
    NGEN_WIDTH (tmp) = width;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNWithOp (WithOpType WithOp)
{
    node *tmp;

    DBUG_ENTER ("MakeNWithOp");

    tmp = CreateCleanNode (N_Nwithop);

    /* allocate mem to store WithOpType in. */
    tmp->info2 = MALLOC (sizeof (WithOpType));
    NWITHOP_TYPE (tmp) = WithOp;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNCode (node *block, node *expr)
{
    node *tmp;

    DBUG_ENTER ("MakeNCODE");

    tmp = CreateCleanNode (N_Ncode);

    NCODE_CBLOCK (tmp) = block;
    NCODE_CEXPR (tmp) = expr;
    NCODE_USED (tmp) = 0;
    NCODE_WLAA_INFO (tmp) = NULL;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeNWith2 (node *withid, node *seg, node *code, node *withop, int dims)
{
    node *tmp;

    DBUG_ENTER ("MakeNWith2");

    tmp = CreateCleanNode (N_Nwith2);

    NWITH2_WITHID (tmp) = withid;
    NWITH2_SEGS (tmp) = seg;
    NWITH2_CODE (tmp) = code;
    NWITH2_WITHOP (tmp) = withop;
    NWITH2_DIMS (tmp) = dims;
    NWITH2_ISSCHEDULED (tmp) = FALSE;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLseg (int dims, node *contents, node *next)
{
    node *new_node;
    int b, d;

    DBUG_ENTER ("MakeWLseg");

    new_node = CreateCleanNode (N_WLseg);

    WLSEG_DIMS (new_node) = dims;

    WLSEG_CONTENTS (new_node) = contents;
    WLSEG_NEXT (new_node) = next;

    WLSEG_IDX_MIN (new_node) = (int *)MALLOC (dims * sizeof (int));
    WLSEG_IDX_MAX (new_node) = (int *)MALLOC (dims * sizeof (int));

    WLSEG_BLOCKS (new_node) = 3; /* three blocking levels */
    for (b = 0; b < WLSEG_BLOCKS (new_node); b++) {
        WLSEG_BV (new_node, b) = (long *)MALLOC (sizeof (long) * dims);
    }
    WLSEG_UBV (new_node) = (long *)MALLOC (sizeof (long) * dims);

    WLSEG_SV (new_node) = (long *)MALLOC (sizeof (long) * dims);
    /* init SV */
    for (d = 0; d < dims; d++) {
        (WLSEG_SV (new_node))[d] = 1;
    }

    WLSEG_MAXHOMDIM (new_node) = -1;
    /*
     * By default, no dimension is homogenious. Since dimensions are counted
     * starting by 0, we must set  MAXHOMDIM to -1 here.
     */

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLblock (int level, int dim, int bound1, int bound2, int step, node *nextdim,
             node *contents, node *next)
{
    node *new_node;

    DBUG_ENTER ("MakeWLblock");

    new_node = CreateCleanNode (N_WLblock);

    WLBLOCK_LEVEL (new_node) = level;
    WLBLOCK_DIM (new_node) = dim;
    WLBLOCK_BOUND1 (new_node) = bound1;
    WLBLOCK_BOUND2 (new_node) = bound2;
    WLBLOCK_STEP (new_node) = step;
    WLBLOCK_NEXTDIM (new_node) = nextdim;
    WLBLOCK_CONTENTS (new_node) = contents;
    WLBLOCK_NEXT (new_node) = next;

    WLBLOCK_INNERSTEP (new_node) = -1;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLublock (int level, int dim, int bound1, int bound2, int step, node *nextdim,
              node *contents, node *next)
{
    node *new_node;

    DBUG_ENTER ("MakeWLublock");

    new_node = MakeWLblock (level, dim, bound1, bound2, step, nextdim, contents, next);

    NODE_TYPE (new_node) = N_WLublock;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLstride (int level, int dim, int bound1, int bound2, int step, int unrolling,
              node *contents, node *next)
{
    node *new_node;

    DBUG_ENTER ("MakeWLstride");

    new_node = CreateCleanNode (N_WLstride);

    WLSTRIDE_LEVEL (new_node) = level;
    WLSTRIDE_DIM (new_node) = dim;
    WLSTRIDE_BOUND1 (new_node) = bound1;
    WLSTRIDE_BOUND2 (new_node) = bound2;
    WLSTRIDE_STEP (new_node) = step;
    WLSTRIDE_UNROLLING (new_node) = unrolling;
    WLSTRIDE_CONTENTS (new_node) = contents;
    WLSTRIDE_NEXT (new_node) = next;

    WLSTRIDE_PART (new_node) = NULL;
    WLSTRIDE_MODIFIED (new_node) = NULL;

    WLSTRIDE_INNERSTEP (new_node) = -1;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLgrid (int level, int dim, int bound1, int bound2, int unrolling, node *nextdim,
            node *next, node *code)
{
    node *new_node;

    DBUG_ENTER ("MakeWLgrid");

    new_node = CreateCleanNode (N_WLgrid);

    WLGRID_LEVEL (new_node) = level;
    WLGRID_DIM (new_node) = dim;
    WLGRID_BOUND1 (new_node) = bound1;
    WLGRID_BOUND2 (new_node) = bound2;
    WLGRID_UNROLLING (new_node) = unrolling;
    WLGRID_NEXTDIM (new_node) = nextdim;
    WLGRID_NEXT (new_node) = next;

    if (code != NULL) {
        NCODE_USED (code)++;
    }
    WLGRID_CODE (new_node) = code;

    WLGRID_MODIFIED (new_node) = NULL;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLsegVar (int dims, node *contents, node *next)
{
    node *new_node;
    int b, d;

    DBUG_ENTER ("MakeWLsegVar");

    new_node = CreateCleanNode (N_WLsegVar);

    WLSEGVAR_DIMS (new_node) = dims;

    WLSEGVAR_CONTENTS (new_node) = contents;
    WLSEGVAR_NEXT (new_node) = next;

    WLSEGVAR_IDX_MIN (new_node) = (int *)MALLOC (dims * sizeof (int));
    WLSEGVAR_IDX_MAX (new_node) = (int *)MALLOC (dims * sizeof (int));

    WLSEGVAR_BLOCKS (new_node) = 3; /* three blocking levels */
    for (b = 0; b < WLSEGVAR_BLOCKS (new_node); b++) {
        WLSEGVAR_BV (new_node, b) = (long *)MALLOC (sizeof (long) * dims);
    }
    WLSEGVAR_UBV (new_node) = (long *)MALLOC (sizeof (long) * dims);

    WLSEGVAR_SV (new_node) = (long *)MALLOC (sizeof (long) * dims);
    /* init SV */
    for (d = 0; d < dims; d++) {
        (WLSEGVAR_SV (new_node))[d] = 1;
    }

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLstriVar (int level, int dim, node *bound1, node *bound2, node *step, node *contents,
               node *next)
{
    node *new_node;

    DBUG_ENTER ("MakeWLstriVar");

    new_node = CreateCleanNode (N_WLstriVar);

    WLSTRIVAR_LEVEL (new_node) = level;
    WLSTRIVAR_DIM (new_node) = dim;
    WLSTRIVAR_BOUND1 (new_node) = bound1;
    WLSTRIVAR_BOUND2 (new_node) = bound2;
    WLSTRIVAR_STEP (new_node) = step;
    WLSTRIVAR_CONTENTS (new_node) = contents;
    WLSTRIVAR_NEXT (new_node) = next;

    WLSTRIVAR_INNERSTEP (new_node) = -1;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLgridVar (int level, int dim, node *bound1, node *bound2, node *nextdim, node *next,
               node *code)
{
    node *new_node;

    DBUG_ENTER ("MakeWLgridVar");

    new_node = CreateCleanNode (N_WLgridVar);

    WLGRIDVAR_LEVEL (new_node) = level;
    WLGRIDVAR_DIM (new_node) = dim;
    WLGRIDVAR_BOUND1 (new_node) = bound1;
    WLGRIDVAR_BOUND2 (new_node) = bound2;
    WLGRIDVAR_NEXTDIM (new_node) = nextdim;
    WLGRIDVAR_NEXT (new_node) = next;

    if (code != NULL) {
        NCODE_USED (code)++;
    }
    WLGRIDVAR_CODE (new_node) = code;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/
