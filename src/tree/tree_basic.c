/*
 *
 * $Log$
 * Revision 3.48  2002/03/06 03:45:57  dkr
 * MakeArgtab() modified
 *
 * Revision 3.47  2002/03/05 14:01:16  dkr
 * MakeId(): ID_UNQCONV added
 *
 * Revision 3.46  2002/03/01 02:38:43  dkr
 * MakeArgtab() added
 *
 * Revision 3.45  2002/02/22 12:04:10  dkr
 * NAMES_IN_TYPES hack is no longer needed :-)
 *
 * Revision 3.44  2001/12/12 12:44:44  dkr
 * function MakeId_Copy_NT added
 *
 * Revision 3.43  2001/07/17 16:17:13  cg
 * Bug fixed: whenever functions like MakeFundef are called with
 * NULL types specification, subsequent write operations to fields
 * belonging N_fundef but which are actually resident in types
 * structure will fail. To prevent this, types structures are
 * created whenever it is not provided as an argument.
 *
 * Revision 3.42  2001/07/17 08:38:24  nmw
 * MakeArg() can now handle NULL types (used temporaly by typechecker)
 *
 * Revision 3.41  2001/07/16 09:13:21  cg
 * Added function MakeOk for construction of N_ok nodes.
 *
 * Revision 3.40  2001/07/16 08:23:11  cg
 * Old tree construction function MakeNode eliminated.
 *
 * Revision 3.39  2001/07/13 13:23:41  cg
 * DBUG tags brushed.
 *
 * Revision 3.38  2001/05/17 14:44:03  dkr
 * FREE, MALLOC eliminated
 *
 * Revision 3.37  2001/05/14 10:21:20  cg
 * Bug in indentation of SPMD_BEGIN/END ICMs fixed.
 *
 * Revision 3.36  2001/05/08 12:28:23  dkr
 * new macros for RC used
 *
 * Revision 3.35  2001/05/03 17:25:56  dkr
 * MAXHOMDIM replaced by HOMSV
 *
 * Revision 3.34  2001/04/25 01:20:24  dkr
 * definition of flag NAMES_IN_TYPES modified
 *
 * Revision 3.33  2001/04/24 14:16:21  dkr
 * comment about revision 3.32 corrected :-/
 *
 * Revision 3.32  2001/04/24 14:14:33  dkr
 * - MakeFundef: works correctly even with 'types == NULL' now
 * - Some DBUG_ASSERTs about FUNDEF_USED added
 * - macro FUNDEF_UNUSED renamed into USED_INACTIVE
 *
 * Revision 3.31  2001/04/24 13:28:08  dkr
 * MakeFundef: FUNDEF_USED initialized correctly
 *
 * Revision 3.30  2001/04/24 09:16:09  dkr
 * P_FORMAT replaced by F_PTR
 *
 * [...]
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "free.h"
#include "internal_lib.h"
#include "refcount.h"

/*--------------------------------------------------------------------------*/

#define PRF_IF(n, s, x, y) y

char *prf_name_str[] = {
#include "prf_node_info.mac"
};

#undef PRF_IF

/*--------------------------------------------------------------------------*/
/* local macros for heap allocation                                         */
/*--------------------------------------------------------------------------*/

#define ALLOCATE(type) (type *)Malloc (sizeof (type))

/*--------------------------------------------------------------------------*/
/* local functions for node initialization                                  */
/*--------------------------------------------------------------------------*/

static node *
CreateCleanNode (nodetype nt)
{
    int i;
    node *new_node;

    DBUG_ENTER ("CreateCleanNode");

    new_node = ALLOCATE (node);

    NODE_TYPE (new_node) = nt;

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

/*
 * attention: the given parameter chain of nums structs is set free here!!!
 */
shpseg *
MakeShpseg (nums *numsp)
{
    shpseg *tmp;
    int i;
    nums *oldnumsp;

    DBUG_ENTER ("MakeShpseg");

    tmp = ALLOCATE (shpseg);

#if 0
  for (i = 0; i < SHP_SEG_SIZE; i++) {
    SHPSEG_SHAPE( tmp, i) = -1;
  }
#endif

    i = 0;
    while (numsp != NULL) {
        if (i >= SHP_SEG_SIZE) {
            SYSABORT (("Maximum number of dimensions exceeded"));
        }

        SHPSEG_SHAPE (tmp, i) = NUMS_NUM (numsp);

        i++;
        oldnumsp = numsp;
        numsp = NUMS_NEXT (numsp);
        oldnumsp = Free (oldnumsp);
    }

    SHPSEG_NEXT (tmp) = NULL;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

types *
MakeTypes1 (simpletype btype)
{
    types *tmp;

    DBUG_ENTER ("MakeTypes");

    tmp = MakeTypes (btype, 0, NULL, NULL, NULL);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

types *
MakeTypes (simpletype btype, int dim, shpseg *shpseg, char *name, char *mod)
{
    types *tmp;

    DBUG_ENTER ("MakeTypes2");

    tmp = ALLOCATE (types);

    TYPES_BASETYPE (tmp) = btype;
    TYPES_NAME (tmp) = name;
    TYPES_MOD (tmp) = mod;
    TYPES_SHPSEG (tmp) = shpseg;
    TYPES_DIM (tmp) = dim;
    TYPES_STATUS (tmp) = ST_regular;

    TYPES_TDEF (tmp) = NULL;
    TYPES_NEXT (tmp) = NULL;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

ids *
MakeIds (char *name, char *mod, statustype status)
{
    ids *tmp;
    DBUG_ENTER ("MakeIds");

    tmp = ALLOCATE (ids);
    IDS_NAME (tmp) = name;
    IDS_MOD (tmp) = mod;
    IDS_REFCNT (tmp) = RC_UNDEF;
    IDS_NAIVE_REFCNT (tmp) = RC_UNDEF;
    IDS_NEXT (tmp) = NULL;
    IDS_VARDEC (tmp) = NULL;
    IDS_AVIS (tmp) = NULL;
    IDS_DEF (tmp) = NULL;
    IDS_USE (tmp) = NULL;
    IDS_STATUS (tmp) = status;
    IDS_ATTRIB (tmp) = (mod == NULL) ? ST_regular : ST_global;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

ids *
MakeIds_Copy (char *name)
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

    tmp = ALLOCATE (nums);
    NUMS_NUM (tmp) = num;
    NUMS_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

deps *
MakeDeps (char *name, char *decname, char *libname, statustype status, locationtype loc,
          deps *sub, deps *next)
{
    deps *tmp;
    DBUG_ENTER ("MakeDeps");

    tmp = ALLOCATE (deps);
    DEPS_NAME (tmp) = name;
    DEPS_DECNAME (tmp) = decname;
    DEPS_LIBNAME (tmp) = libname;
    DEPS_STATUS (tmp) = status;
    DEPS_LOC (tmp) = loc;
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

    tmp = ALLOCATE (strings);
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

    tmp = ALLOCATE (nodelist);
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

    tmp = ALLOCATE (nodelist);
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

    tmp = ALLOCATE (access_t);

    ACCESS_ARRAY (tmp) = array;
    ACCESS_IV (tmp) = iv;
    ACCESS_CLASS (tmp) = class;
    ACCESS_OFFSET (tmp) = offset;
    ACCESS_DIR (tmp) = direction;
    ACCESS_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

argtab_t *
MakeArgtab (int size)
{
    argtab_t *argtab;
    int i;

    DBUG_ENTER ("MakeArgtab");

    argtab = Malloc (sizeof (argtab_t));

    argtab->size = size;
    argtab->ptr_in = Malloc (argtab->size * sizeof (node *));
    argtab->ptr_out = Malloc (argtab->size * sizeof (types *));
    argtab->tag = Malloc (argtab->size * sizeof (argtag_t));

    for (i = 0; i < argtab->size; i++) {
        argtab->ptr_in[i] = NULL;
        argtab->ptr_out[i] = NULL;
        argtab->tag[i] = ATG_notag;
    }

    DBUG_RETURN (argtab);
}

/*--------------------------------------------------------------------------*/

DFMfoldmask_t *
MakeDFMfoldmask (node *vardec, node *foldop, DFMfoldmask_t *next)
{
    DFMfoldmask_t *tmp;

    DBUG_ENTER ("MakeDFMfoldmask");

    tmp = ALLOCATE (DFMfoldmask_t);

    DFMFM_VARDEC (tmp) = vardec;
    DFMFM_FOLDOP (tmp) = foldop;
    DFMFM_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

DFMfoldmask_t *
CopyDFMfoldmask (DFMfoldmask_t *mask)
{
    DFMfoldmask_t *tmp;

    DBUG_ENTER ("CopyDFMfoldmask");

    if (mask != NULL) {
        tmp = ALLOCATE (DFMfoldmask_t);

        DFMFM_VARDEC (tmp) = DFMFM_VARDEC (mask);
        DFMFM_FOLDOP (tmp) = DFMFM_FOLDOP (mask);
        DFMFM_NEXT (tmp) = CopyDFMfoldmask (DFMFM_NEXT (mask));
    } else {
        tmp = NULL;
    }

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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
    TYPEDEF_LINKMOD (tmp) = NULL;
    TYPEDEF_ATTRIB (tmp) = attrib;
    TYPEDEF_NEXT (tmp) = next;

    TYPEDEF_STATUS (tmp) = ST_regular;

    TYPEDEF_IMPL (tmp) = NULL;
    TYPEDEF_COPYFUN (tmp) = NULL;
    TYPEDEF_FREEFUN (tmp) = NULL;

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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
    OBJDEF_LINKMOD (tmp) = NULL;
    OBJDEF_EXPR (tmp) = expr;
    OBJDEF_NEXT (tmp) = next;

    OBJDEF_LINKMOD (tmp) = NULL;
    OBJDEF_STATUS (tmp) = ST_regular;
    OBJDEF_ATTRIB (tmp) = ST_regular;

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeFundef (char *name, char *mod, types *types, node *args, node *body, node *next)
{
    node *tmp;

    DBUG_ENTER ("MakeFundef");

    tmp = CreateCleanNode (N_fundef);

    FUNDEF_BODY (tmp) = body;
    FUNDEF_ARGS (tmp) = args;
    FUNDEF_NEXT (tmp) = next;

    FUNDEF_TYPES (tmp) = types;

    FUNDEF_NAME (tmp) = name;
    FUNDEF_MOD (tmp) = mod;
    FUNDEF_LINKMOD (tmp) = NULL;
    FUNDEF_STATUS (tmp) = ST_regular;
    FUNDEF_ATTRIB (tmp) = ST_regular;

    FUNDEF_FUNNO (tmp) = 0;
    FUNDEF_INLINE (tmp) = FALSE;

    FUNDEF_USED (tmp) = USED_INACTIVE;

    FUNDEF_ARGTAB (tmp) = NULL;

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeArg (char *name, types *type, statustype status, statustype attrib, node *next)
{
    node *tmp;

    DBUG_ENTER ("MakeArg");

    tmp = CreateCleanNode (N_arg);

    ARG_NEXT (tmp) = next;
    ARG_TYPE (tmp) = type;

    ARG_NAME (tmp) = name;
    ARG_STATUS (tmp) = status;
    ARG_ATTRIB (tmp) = attrib;

    ARG_AVIS (tmp) = MakeAvis (tmp);

    ARG_ACTCHN (tmp) = NULL;
    ARG_COLCHN (tmp) = NULL;

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE", ("%d:nodetype: %s " F_PTR " instr: " F_PTR " vardec: " F_PTR,
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

    VARDEC_NEXT (tmp) = next;
    VARDEC_TYPE (tmp) = type;

    VARDEC_NAME (tmp) = name;
    VARDEC_STATUS (tmp) = ST_regular;
    VARDEC_ATTRIB (tmp) = ST_regular;
    VARDEC_AVIS (tmp) = MakeAvis (tmp);

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE", ("%d:nodetype: %s " F_PTR " instr: " F_PTR " next: " F_PTR,
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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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
    AP_ARGTAB (tmp) = NULL;

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeId (char *name, char *mod, statustype status)
{
    node *tmp;

    DBUG_ENTER ("MakeId");

    tmp = CreateCleanNode (N_id);

    ID_IDS (tmp) = MakeIds (name, mod, status);

    ID_UNQCONV (tmp) = NO_UNQCONV;

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeId_Copy (char *str)
{
    node *result;

    DBUG_ENTER ("MakeId_Copy");

    result = MakeId (StringCopy (str), NULL, ST_regular);

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

node *
MakeId_Copy_NT (node *vardec)
{
    node *result;

    DBUG_ENTER ("MakeId_Copy");

    DBUG_ASSERT ((vardec != NULL), "no vardec found!");

    result = MakeId_Copy (VARDEC_OR_ARG_NAME (vardec));
    ID_NT_TAG (result) = vardec;

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

node *
MakeId_Num (int val)
{
    char *str;
    node *result;

    DBUG_ENTER ("MakeId_Num");

    str = (char *)Malloc (20 * sizeof (char));
    sprintf (str, "%d", val);
    result = MakeId (str, NULL, ST_regular);

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

node *
MakeNum (int val)
{
    node *tmp;

    DBUG_ENTER ("MakeNum");

    tmp = CreateCleanNode (N_num);

    NUM_VAL (tmp) = val;

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeBool (bool val)
{
    node *tmp;

    DBUG_ENTER ("MakeBool");

    tmp = CreateCleanNode (N_bool);

    BOOL_VAL (tmp) = val;

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeEmpty ()
{
    node *tmp;

    DBUG_ENTER ("MakeEmpty");

    tmp = CreateCleanNode (N_empty);

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeIcm (char *name, node *args)
{
    node *tmp;

    DBUG_ENTER ("MakeIcm");

    tmp = CreateCleanNode (N_icm);

    ICM_NAME (tmp) = name;
    ICM_ARGS (tmp) = args;

    DBUG_ASSERT (name != NULL, "MakeIcm called with empty ICM name.");

    if (strcmp (name, "MT_START_SYNCBLOCK") == 0) {
        ICM_INDENT_BEFORE (tmp) = 0;
        ICM_INDENT_AFTER (tmp) = 1;
    } else if (strncmp (name, "MT_SYNC_", 8) == 0) {
        ICM_INDENT_BEFORE (tmp) = -1;
        ICM_INDENT_AFTER (tmp) = 0;
    } else if (strcmp (name, "MT2_IF_I_AM_FIRST") == 0) {
        ICM_INDENT_BEFORE (tmp) = 0;
        ICM_INDENT_AFTER (tmp) = 1;
    } else if (strcmp (name, "MT2_ELSE_IF_I_AM_NOT_FIRST") == 0) {
        ICM_INDENT_BEFORE (tmp) = -1;
        ICM_INDENT_AFTER (tmp) = 1;
    } else if (strcmp (name, "MT2_END_I_AM_FIRST") == 0) {
        ICM_INDENT_BEFORE (tmp) = -1;
        ICM_INDENT_AFTER (tmp) = 0;
    } else if (strcmp (name, "MT_SPMD_BEGIN") == 0) {
        /*
         * This ICM must be handled specifically since it would otherwise
         * be treated according to the more generic rules for BEGIN/END
         * ICMs found below.
         */
        ICM_INDENT_BEFORE (tmp) = 0;
        ICM_INDENT_AFTER (tmp) = 0;
    } else if (strcmp (name, "MT_SPMD_END") == 0) {
        /*
         * This ICM must be handled specifically since it would otherwise
         * be treated according to the more generic rules for BEGIN/END
         * ICMs found below.
         */
        ICM_INDENT_BEFORE (tmp) = 0;
        ICM_INDENT_AFTER (tmp) = 0;
    } else if (strstr (name, "BEGIN")) {
        ICM_INDENT_BEFORE (tmp) = 0;
        ICM_INDENT_AFTER (tmp) = 1;
    } else if (strstr (name, "END")) {
        ICM_INDENT_BEFORE (tmp) = -1;
        ICM_INDENT_AFTER (tmp) = 0;
    } else {
        ICM_INDENT_BEFORE (tmp) = 0;
        ICM_INDENT_AFTER (tmp) = 0;
    }

    if ((strcmp (name, "ND_KS_DECL_ARRAY") == 0)
        || (strcmp (name, "MT_DECL_MYTHREAD") == 0)) {
        ICM_END_OF_STATEMENT (tmp) = TRUE;
    } else {
        ICM_END_OF_STATEMENT (tmp) = FALSE;
    }

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeCSEinfo (node *next, node *layer, node *let)
{
    node *tmp;

    DBUG_ENTER ("MakeCSEinfo");

    tmp = CreateCleanNode (N_cseinfo);
    CSEINFO_NEXT (tmp) = next;
    CSEINFO_LAYER (tmp) = layer;
    CSEINFO_LET (tmp) = let;

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

extern node *
MakeSSAcnt (node *next, int count, char *baseid)
{
    node *tmp;

    DBUG_ENTER ("MakeSSAcnt");

    tmp = CreateCleanNode (N_ssacnt);
    SSACNT_NEXT (tmp) = next;
    SSACNT_COUNT (tmp) = count;
    SSACNT_BASEID (tmp) = baseid;

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeSSAstack (node *next, node *avis)
{
    node *tmp;

    DBUG_ENTER ("MakeSSAstack");

    tmp = CreateCleanNode (N_ssastack);
    SSASTACK_NEXT (tmp) = next;
    SSASTACK_AVIS (tmp) = avis;
    SSASTACK_INUSE (tmp) = FALSE;

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

extern node *
MakeAvis (node *vardecOrArg)
{
    node *tmp;

    DBUG_ENTER ("MakeAvis");

    tmp = CreateCleanNode (N_avis);
    AVIS_VARDECORARG (tmp) = vardecOrArg;
    AVIS_SSAPHITARGET (tmp) = PHIT_NONE;
    AVIS_SSALPINV (tmp) = FALSE;
    AVIS_SSADEFINED (tmp) = FALSE;
    AVIS_SSAUNDOFLAG (tmp) = FALSE;

    /* create empty stack */
    AVIS_SSASTACK (tmp) = MakeSSAstack (NULL, NULL);

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeInfo ()
{
    node *tmp;

    DBUG_ENTER ("MakeInfo");

    tmp = CreateCleanNode (N_info);

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

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

int global_MT_counter = 1;

#define MT_STEP 1

static int
GetNewMTIdentifier ()
{
    int result;

    DBUG_ENTER ("GetNewMTIdentifier");

    result = global_MT_counter;
    global_MT_counter = global_MT_counter + MT_STEP;

    DBUG_RETURN (result);
}

node *
MakeMT (node *region)
{
    node *tmp;

    DBUG_ENTER ("MakeMT");

    tmp = CreateCleanNode (N_mt);
    MT_REGION (tmp) = region;
    MT_IDENTIFIER (tmp) = GetNewMTIdentifier ();

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

int global_ST_counter = 1;

#define ST_STEP 1

static int
GetNewSTIdentifier ()
{
    int result;

    DBUG_ENTER ("GetNewStIdentifier");

    result = global_ST_counter;
    global_ST_counter = global_ST_counter + ST_STEP;

    DBUG_RETURN (result);
}

node *
MakeST (node *region)
{
    node *tmp;

    DBUG_ENTER ("MakeST");

    tmp = CreateCleanNode (N_st);
    ST_REGION (tmp) = region;
    ST_IDENTIFIER (tmp) = GetNewSTIdentifier ();

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeMTsignal ()
{
    node *tmp;

    DBUG_ENTER ("MakeMTsignal");

    tmp = CreateCleanNode (N_MTsignal);
    MTSIGNAL_IDSET (tmp) = NULL;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeMTsync ()
{
    node *tmp;

    DBUG_ENTER ("MakeMTsync");

    tmp = CreateCleanNode (N_MTsync);
    MTSYNC_WAIT (tmp) = NULL;
    MTSYNC_FOLD (tmp) = NULL;
    MTSYNC_ALLOC (tmp) = NULL;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeMTalloc ()
{
    node *tmp;

    DBUG_ENTER ("MakeMTalloc");

    tmp = CreateCleanNode (N_MTalloc);
    MTALLOC_IDSET (tmp) = NULL;

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
    tmp->info2 = Malloc (sizeof (WithOpType));
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
    int b;

    DBUG_ENTER ("MakeWLseg");

    new_node = CreateCleanNode (N_WLseg);

    WLSEG_DIMS (new_node) = dims;

    WLSEG_CONTENTS (new_node) = contents;
    WLSEG_NEXT (new_node) = next;

    WLSEG_IDX_MIN (new_node) = NULL;
    WLSEG_IDX_MAX (new_node) = NULL;

    MALLOC_INIT_VECT (WLSEG_UBV (new_node), WLSEG_DIMS (new_node), int, 1);

    WLSEG_BLOCKS (new_node) = 0;
    for (b = 0; b < WLSEG_BLOCKS (new_node); b++) {
        MALLOC_INIT_VECT (WLSEG_BV (new_node, b), WLSEG_DIMS (new_node), int, 1);
    }

    WLSEG_SV (new_node) = NULL;
    WLSEG_HOMSV (new_node) = NULL;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLsegVar (int dims, node *contents, node *next)
{
    node *new_node;

    DBUG_ENTER ("MakeWLsegVar");

    new_node = CreateCleanNode (N_WLsegVar);

    WLSEGVAR_DIMS (new_node) = dims;

    WLSEGVAR_CONTENTS (new_node) = contents;
    WLSEGVAR_NEXT (new_node) = next;

    WLSEGVAR_IDX_MIN (new_node) = NULL;
    WLSEGVAR_IDX_MAX (new_node) = NULL;

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
MakeWLstride (int level, int dim, int bound1, int bound2, int step, bool unrolling,
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

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLstrideVar (int level, int dim, node *bound1, node *bound2, node *step,
                 node *contents, node *next)
{
    node *new_node;

    DBUG_ENTER ("MakeWLstrideVar");

    new_node = CreateCleanNode (N_WLstrideVar);

    WLSTRIDEVAR_LEVEL (new_node) = level;
    WLSTRIDEVAR_DIM (new_node) = dim;
    WLSTRIDEVAR_BOUND1 (new_node) = bound1;
    WLSTRIDEVAR_BOUND2 (new_node) = bound2;
    WLSTRIDEVAR_STEP (new_node) = step;
    WLSTRIDEVAR_CONTENTS (new_node) = contents;
    WLSTRIDEVAR_NEXT (new_node) = next;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeWLgrid (int level, int dim, int bound1, int bound2, bool unrolling, node *nextdim,
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
    WLGRID_FITTED (new_node) = FALSE;
    WLGRID_NEXTDIM (new_node) = nextdim;
    WLGRID_NEXT (new_node) = next;

    if (code != NULL) {
        NCODE_USED (code)++; /* see remarks of N_Ncode in tree_basic.h */
    }
    WLGRID_CODE (new_node) = code;

    WLGRID_NOOP (new_node) = FALSE;
    WLGRID_MODIFIED (new_node) = NULL;

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
    WLGRIDVAR_FITTED (new_node) = FALSE;
    WLGRIDVAR_NEXTDIM (new_node) = nextdim;
    WLGRIDVAR_NEXT (new_node) = next;

    if (code != NULL) {
        NCODE_USED (code)++; /* see remarks of N_Ncode in tree_basic.h */
    }
    WLGRIDVAR_CODE (new_node) = code;

    WLGRIDVAR_NOOP (new_node) = FALSE;

    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeCWrapper (node *next, char *name, char *mod, int argcount, int rescount)
{
    node *new_node;

    DBUG_ENTER ("MakeCWrapper");

    new_node = CreateCleanNode (N_cwrapper);

    CWRAPPER_NEXT (new_node) = next;
    CWRAPPER_NAME (new_node) = name;
    CWRAPPER_MOD (new_node) = mod;
    CWRAPPER_ARGCOUNT (new_node) = argcount;
    CWRAPPER_RESCOUNT (new_node) = rescount;
    CWRAPPER_FUNS (new_node) = NULL; /* initialized without a mapped fundef */
    DBUG_RETURN (new_node);
}

/*--------------------------------------------------------------------------*/

node *
MakeModspec (char *name, node *exports)
{
    node *tmp;

    DBUG_ENTER ("MakeModspec");

    tmp = CreateCleanNode (N_modspec);

    MODDEC_NAME (tmp) = name;
    MODDEC_OWN (tmp) = exports;

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

node *
MakeOk ()
{
    node *tmp;

    DBUG_ENTER ("MakeOk");

    tmp = CreateCleanNode (N_ok);

    DBUG_PRINT ("MAKE",
                ("%d:nodetype: %s " F_PTR, NODE_LINE (tmp), NODE_TEXT (tmp), tmp));

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/
