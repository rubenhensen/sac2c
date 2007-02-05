/*
 * $Log$
 * Revision 3.17  2005/07/03 17:17:25  ktr
 * commented out APCgenarray
 *
 * Revision 3.16  2004/12/08 18:02:10  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 3.15  2004/11/26 22:07:12  sbs
 * JHs typo
 *
 * Revision 3.14  2004/11/26 21:04:35  jhb
 * compile .
 *
 * Revision 3.13  2004/11/25 20:19:00  jhb
 * maybe compile
 *
 * Revision 3.12  2004/07/19 14:19:38  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.11  2003/11/18 17:49:13  dkr
 * no changes done
 *
 * Revision 3.10  2003/03/18 16:30:34  sah
 * added new prf cat_VxV, take_SxV, drop_SxV
 *
 * Revision 3.9  2002/09/11 23:07:39  dkr
 * prf_node_info.mac modified.
 *
 * Revision 3.8  2002/09/09 19:15:29  dkr
 * prf_string removed (mdb_prf used instead)
 *
 * Revision 3.7  2002/09/09 17:48:33  dkr
 * F_{add,sub,mul,div} replaced by F_{add,sub,mul,div}_SxS
 *
 * Revision 3.6  2002/07/29 12:12:53  sbs
 * PRF_IF macro extended by z.
 *
 * Revision 3.5  2002/02/20 14:56:53  dkr
 * fundef DupTypes() renamed into DupAllTypes()
 *
 * Revision 3.4  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.3  2001/05/17 13:40:26  nmw
 * MALLOC/FREE replaced by Malloc/Free, using result of Free()
 *
 * Revision 3.2  2000/12/06 19:22:16  cg
 * Removed compiler warnings in production mode.
 *
 * Revision 3.1  2000/11/20 18:01:49  sacbase
 * new release made
 *
 * Revision 1.14  2000/10/31 23:02:20  dkr
 * signature of Array2Shpseg() changed
 *
 * Revision 1.13  2000/10/26 12:55:27  dkr
 * DupShpSeg renamed into DupShpseg
 *
 * Revision 1.12  2000/10/24 11:52:47  dkr
 * MakeType renamed into MakeTypes
 *
 * Revision 1.11  2000/08/11 20:58:59  mab
 * fixed bug in CollectAccessPatterns
 *
 * Revision 1.10  2000/08/04 11:41:31  mab
 * fixed bug in APCwith and APTwith (case nested with-loops)
 *
 * Revision 1.9  2000/08/03 15:32:00  mab
 * completed collection of access patterns and unsupported shapes
 *
 * Revision 1.8  2000/07/21 14:43:43  mab
 * added APCcode and APCwithop dummies
 *
 * Revision 1.7  2000/07/19 12:37:30  mab
 * added AccessClass2Group
 *
 * Revision 1.6  2000/06/29 10:23:05  mab
 * renamed APCNwith to APCwith
 *
 * Revision 1.5  2000/06/15 14:38:01  mab
 * dummies for APC block and let added
 *
 * Revision 1.4  2000/06/14 10:43:19  mab
 * dummies for APC ap, exprs, id, prf, fundef added
 *
 * Revision 1.3  2000/06/08 11:13:37  mab
 * added functions for nodes arg, vardec, array
 *
 * Revision 1.2  2000/05/31 16:16:58  mab
 * initial version
 *
 * Revision 1.1  2000/05/26 13:42:00  sbs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   pad_collect.c
 *
 * prefix: APC
 *
 * description:
 *
 *   This compiler module collects information needed to infer new array
 *   shapes for the inference-phase.
 *
 *
 *****************************************************************************/

#include "pad_collect.h"

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"
#include "DupTree.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "new_types.h"
#include "new_typecheck.h"

#include "pad_info.h"

/*
 * INFO structure
 */
struct INFO {
    int unsupported;
    int count_changes;
    node *with;
};

/*
 * INFO macros
 */
#define INFO_APC_UNSUPPORTED(n) (n->unsupported)
#define INFO_APC_COUNT_CHANGES(n) (n->count_changes)
#define INFO_APC_WITH(n) (n->with)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_APC_UNSUPPORTED (result) = 0;
    INFO_APC_COUNT_CHANGES (result) = 0;
    INFO_APC_WITH (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

typedef struct COLLECTION_T {
    node *array_vardec;
    accessclass_t access_class;
    accessdir_t direction;
    pattern_t *patterns;
    struct COLLECTION_T *next;
} collection_t;

#define COL_ARRAY(c) ((c)->array_vardec)
#define COL_CLASS(c) ((c)->access_class)
#define COL_DIR(c) ((c)->direction)
#define COL_PATTERNS(c) ((c)->patterns)
#define COL_NEXT(c) ((c)->next)

/*****************************************************************************
 *
 * function:
 *   void APcollect (node *arg_node)
 *
 * description:
 *   main function for collection-phase of array padding
 *
 *****************************************************************************/
/* main function */
void
APCdoCollect (node *arg_node)
{

    info *arg_info;

    DBUG_ENTER ("APcollect");

    DBUG_PRINT ("APC", ("Array Padding: collecting data..."));

    arg_info = MakeInfo ();
    INFO_APC_COUNT_CHANGES (arg_info) = 0;

    TRAVpush (TR_apc);

    arg_node = TRAVdo (arg_node, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_EXECUTE ("APC", PIprintAccessPatterns (); PIprintUnsupportedShapes (););

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   static shpseg* AccessClass2Group(accessclass_t class, int dim)
 *
 * description:
 *   convert access class into vector of integer factors
 *   accessing with ACL_const means [0]*index+offset
 *   accessing with ACL_offset means [1]*index+offset
 *   currently no other access classes are supported
 *   result is NULL, if access class is unsupported
 *
 *****************************************************************************/

static shpseg *
AccessClass2Group (accessclass_t class, int dim)
{

    shpseg *vector;
    int element;
    int i;

    DBUG_ENTER ("AccessClass2Vector");

    switch (class) {
    case ACL_offset:
        element = 1;
        break;
    case ACL_const:
        element = 0;
        break;
    default:
        element = -1;
        break;
    }

    if (element != -1) {

        /* supported access class */

        vector = TBmakeShpseg (NULL);

        for (i = 0; i < dim; i++) {
            SHPSEG_SHAPE (vector, i) = element;
        }
    } else {

        /* unsupported access class */
        vector = NULL;
    }

    DBUG_RETURN (vector);
}

/*****************************************************************************
 *
 * function:
 *   static node *CollectAccessPatterns(node *arg_node)
 *
 * description:
 *   collect access patterns from code-node
 *   group them into constant and offset accesses
 *   add groups to abstract data structure PI
 *
 *****************************************************************************/

static node *
CollectAccessPatterns (node *arg_node)
{

    collection_t *collection;
    collection_t *col_ptr;
    collection_t *col_next_ptr;
    access_t *access_ptr;
    pattern_t *pt_ptr;
    shpseg *group_vect;
    shpseg *offset;
    simpletype type;
    int dim;
    shpseg *shape;
    pattern_t *patterns;
    accessdir_t direction;

    DBUG_ENTER ("CollectAccessPatterns");

    DBUG_PRINT ("APC", ("collecting access patterns..."));

    /* collect array accesses and group by accessed array */
    collection = NULL;
    access_ptr = CODE_WLAA_ACCESS (arg_node);
    while (access_ptr != NULL) {

        col_ptr = collection;
        pt_ptr = NULL;
        while ((col_ptr != NULL) && (pt_ptr == NULL)) {
            if ((COL_ARRAY (col_ptr) == ACCESS_ARRAY (access_ptr))
                && (COL_CLASS (col_ptr) == ACCESS_CLASS (access_ptr))
                && (COL_DIR (col_ptr) == ACCESS_DIR (access_ptr))) {
                /* set current pattern element */
                pt_ptr = COL_PATTERNS (col_ptr);
            } else {
                /* only change current collection element if we
                 * haven't found pt_ptr yet
                 */
                col_ptr = COL_NEXT (col_ptr);
            }
        }

        /* add access to entry (only known access classes) */

        switch (ACCESS_CLASS (access_ptr)) {

        case ACL_offset:
        case ACL_const:

            /* create new entry, if necessary */
            if (pt_ptr == NULL) {

                col_next_ptr = collection;
                collection = (collection_t *)MEMmalloc (sizeof (collection_t));
                COL_ARRAY (collection) = ACCESS_ARRAY (access_ptr);
                COL_CLASS (collection) = ACCESS_CLASS (access_ptr);
                COL_DIR (collection) = ACCESS_DIR (access_ptr);
                COL_PATTERNS (collection) = NULL;
                COL_NEXT (collection) = col_next_ptr;
                /* set current pattern element */
                pt_ptr = COL_PATTERNS (collection);
                /* set current collection element */
                col_ptr = collection;
            }

            offset = DUPdupShpseg (ACCESS_OFFSET (access_ptr));
            pt_ptr = PIconcatPatterns (pt_ptr, offset);
            COL_PATTERNS (col_ptr) = pt_ptr;
            break;
        default:
            break;
        }

        access_ptr = ACCESS_NEXT (access_ptr);
    }

    /* add collected conflict groups to abstract data structure */

    col_ptr = collection;
    while (col_ptr != NULL) {
        type = TYPES_BASETYPE (VARDEC_TYPE (COL_ARRAY (col_ptr)));
        dim = TYPES_DIM (VARDEC_TYPE (COL_ARRAY (col_ptr)));
        shape = DUPdupShpseg (TYPES_SHPSEG (VARDEC_TYPE (COL_ARRAY (col_ptr))));
        group_vect = AccessClass2Group (COL_CLASS (col_ptr), dim);
        direction = COL_DIR (col_ptr);
        patterns = COL_PATTERNS (col_ptr);
        PIaddAccessPattern (type, dim, shape, group_vect, direction, patterns);
        col_ptr = COL_NEXT (col_ptr);
    }

    /* set structure free (do not free vardecs or patterns!) */

    while (collection != NULL) {
        col_ptr = collection->next;
        collection = MEMfree (collection);
        collection = col_ptr;
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   static node* AddUnsupported(node* arg_info, types* array_type)
 *
 * description:
 *   wrapper to simplify adding an unsupported shape
 *   only increase changes, if shape hasn't been added before
 *
 *****************************************************************************/

static void
AddUnsupported (info *arg_info, types *array_type)
{

    DBUG_ENTER ("AddUnsupported");

    INFO_APC_UNSUPPORTED (arg_info) = TRUE;

    /* only non-scalar types will be added to list of unsupported shapes!
     * scalar types do not have a shpseg!
     */
    if (TYPES_DIM (array_type) > 0) {
        if (PIaddUnsupportedShape (DUPdupAllTypes (array_type))) {
            INFO_APC_COUNT_CHANGES (arg_info)++;
        }
    }

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   node *APCarray(node *arg_node, info *arg_info)
 *
 * description:
 *   constant arrays are not supported
 *
 *****************************************************************************/

node *
APCarray (node *arg_node, info *arg_info)
{
    ntype *atype;
    types *otype;

    DBUG_ENTER ("APCarray");

    DBUG_PRINT ("APC", ("array-node detected"));

    atype = NTCnewTypeCheck_Expr (arg_node);
    otype = TYtype2OldType (atype);

    AddUnsupported (arg_info, otype);

    otype = FREEfreeOneTypes (otype);
    atype = TYfreeType (atype);

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCwith(node *arg_node, info *arg_info)
 *
 * description:
 *   depending on INFO_APC_UNSUPPORTED (set by let-node, depending on lhs)
 *   mark relevant arrays as unsupported too
 *   check code, if with-loop itself is supported
 *
 *****************************************************************************/

node *
APCwith (node *arg_node, info *arg_info)
{
    node *save_ptr;

    DBUG_ENTER ("APCwith");

    DBUG_PRINT ("APC", ("with-node detected"));

    /* save pointer to outer with-loop to support nested loops */
    save_ptr = INFO_APC_WITH (arg_info);

    INFO_APC_WITH (arg_info) = arg_node;

    DBUG_ASSERT ((WITH_CODE (arg_node) != NULL), " unexpected empty CODE!");
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /* traverse withop */
    DBUG_ASSERT ((WITH_WITHOP (arg_node) != NULL), " unexpected empty WITHOP!");
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /* no need to traverse part-node here */

    /* restore pointer to outer with-loop */
    INFO_APC_WITH (arg_info) = save_ptr;

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCap(node *arg_node, info *arg_info)
 *
 * description:
 *   check, if ap refers to fundef with or without body
 *
 *****************************************************************************/

node *
APCap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("APCap");

    DBUG_PRINT ("APC", ("ap-node detected"));

    /* look whether body exists or not */
    if (FUNDEF_BODY (AP_FUNDEF (arg_node)) == NULL) {
        /* function without body -> results will have unpadded shape */
        INFO_APC_UNSUPPORTED (arg_info) = TRUE;
    }

    /* the result of an unser-defined function might be of a padded shape */

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCid(node *arg_node, info *arg_info)
 *
 * description:
 *   add type to unsupported shapes, if UNSUPPORTED is set to TRUE by upper function
 *   We suppose here, that constants will be assigned to a variable. So there will
 *   be no need to traverse constants. We can collect these shapes by checking
 *   id-nodes only.
 *
 *****************************************************************************/

node *
APCid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("APCid");

    DBUG_PRINT ("APC", ("id-node detected"));

    if (INFO_APC_UNSUPPORTED (arg_info)) {
        AddUnsupported (arg_info, ID_TYPE (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCprf(node *arg_node, info *arg_info)
 *
 * description:
 *   only F_add_*, F_sub_* and F_mul_* may have a padded result
 *
 *****************************************************************************/

node *
APCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("APCprf");

    DBUG_PRINT ("APC", ("prf-node detected: '%s'", global.mdb_prf[PRF_PRF (arg_node)]));

    switch (PRF_PRF (arg_node)) {

        /* result may have padded shape, arguments may also have padded shape */
    case F_add_SxA:
    case F_add_AxS:
    case F_add_AxA:
    case F_sub_SxA:
    case F_sub_AxS:
    case F_sub_AxA:
    case F_mul_SxA:
    case F_mul_AxS:
    case F_mul_AxA:
        break;

        /* result will be unpadded, but padded arguments are supported */
    case F_sel:
    case F_dim:
    case F_shape:
        INFO_APC_UNSUPPORTED (arg_info) = TRUE;
        break;

        /* scalar functions: unpadded result, no padding on arguments */
    case F_toi_S:
    case F_tof_S:
    case F_tod_S:
    case F_abs:
    case F_not:
    case F_min:
    case F_max:
    case F_add_SxS:
    case F_sub_SxS:
    case F_mul_SxS:
    case F_div_SxS:
    case F_mod:
    case F_and:
    case F_or:
    case F_le:
    case F_lt:
    case F_eq:
    case F_ge:
    case F_gt:
    case F_neq:
        INFO_APC_UNSUPPORTED (arg_info) = TRUE;
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        break;

        /* unsupported non-scalar functions */
    case F_take:
    case F_drop:
    case F_take_SxV:
    case F_drop_SxV:
    case F_idx_sel:
    case F_reshape:
    case F_cat:
    case F_cat_VxV:
    case F_rotate:
    case F_toi_A:
    case F_tof_A:
    case F_tod_A:
    case F_div_SxA:
    case F_div_AxS:
    case F_div_AxA:
    case F_modarray:
    case F_idx_modarray:
    case F_genarray:
        INFO_APC_UNSUPPORTED (arg_info) = TRUE;
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        break;

    default:
        /* there should be no more functions left */
        DBUG_ASSERT ((FALSE), "unknown PRF!");
        break;
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node* APCfundef(node* arg_node, node* arg_info)
 *
 * description:
 *   Later we  may insert conversion functions (pad_*(), unpad_*()) here.
 *
 *****************************************************************************/

node *
APCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("APCfundef");

    DBUG_EXECUTE ("APC",
                  if (FUNDEF_NAME (arg_node) != NULL) {
                      DBUG_PRINT ("APC", ("fundef-node: %s", FUNDEF_NAME (arg_node)));
                  } else { DBUG_PRINT ("APC", ("fundef-node: (NULL)")); });

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    /* no need to traverse args here */

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APClet(node *arg_node, info *arg_info)
 *
 * description:
 *   check and traverse rhs
 *   rhs unsupported => lhs unsupported as well
 *
 *****************************************************************************/

node *
APClet (node *arg_node, info *arg_info)
{
    node *ids_ptr;
    bool save_state;

    DBUG_ENTER ("APClet");

    DBUG_PRINT ("APC", ("let-node detected (%s=...)", IDS_NAME (LET_IDS (arg_node))));

    save_state = INFO_APC_UNSUPPORTED (arg_info);
    INFO_APC_UNSUPPORTED (arg_info) = FALSE;

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), " let-node without rvalues detected!");
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_APC_UNSUPPORTED (arg_info)) {
        ids_ptr = LET_IDS (arg_node);
        while (ids_ptr != NULL) {
            AddUnsupported (arg_info, VARDEC_OR_ARG_TYPE (IDS_DECL (ids_ptr))); /* TODO */
            ids_ptr = IDS_NEXT (ids_ptr);
        }
    }

    INFO_APC_UNSUPPORTED (arg_info) = save_state;
    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCgenarray( node *arg_node, info *arg_info)
 *
 * description:
 *   marks array as unsupported, if modarray-withloop has non-scalar code-result
 *
 *****************************************************************************/

node *
APCgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("APCgenarray");

#if 0
  shpseg* shape;
  types* type;
  int dim;
  simpletype basetype;


  DBUG_PRINT( "APC", ("withop-node detected")); 

  DBUG_PRINT( "APC", (" genarray-loop"));
  if (INFO_APC_UNSUPPORTED(arg_info)) {
  TODO: the following assumes, genarray_shape is given by a N_array node
    /* do not add type of vector, but contents of array to unsupported shapes */
    basetype = TYPES_BASETYPE(ID_TYPE(WITH_CEXPR(INFO_APC_WITH(arg_info))));

    dim = SHPSEG_SHAPE(TYPES_SHPSEG(ARRAY_TYPE(GENARRAY_SHAPE(arg_node))),0);

    shape = TCarray2Shpseg(GENARRAY_SHAPE(arg_node), NULL);

    type = TBmakeTypes(basetype,dim,shape,NULL,NULL);

    AddUnsupported(arg_info,type);

    FREEfreeOneTypes( type);
  }
#endif

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCmodarray( node *arg_node, info *arg_info)
 *
 * description:
 *   marks array as unsupported, if modarray-withloop has non-scalar code-result
 *
 *****************************************************************************/

node *
APCmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("APCmodarray");

    DBUG_PRINT ("APC", ("withop-node detected"));

    DBUG_PRINT ("APC", (" modarray-loop"));

    if (INFO_APC_UNSUPPORTED (arg_info)) {
        AddUnsupported (arg_info, ID_TYPE (MODARRAY_ARRAY (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCfold( node *arg_node, info *arg_info)
 *
 * description:
 *   marks array as unsupported, if modarray-withloop has non-scalar code-result
 *
 *****************************************************************************/

node *
APCfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("APCfold");

    DBUG_PRINT ("APC", ("withop-node detected"));

    DBUG_PRINT ("APC", (" foldfun-loop"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APCcode(node *arg_node, info *arg_info)
 *
 * description:
 *   check scalar type of code result
 *
 *****************************************************************************/

node *
APCcode (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("APCcode");

    DBUG_ASSERT ((CODE_CEXPR (arg_node) != NULL), " unexpected empty CEXPR!");
    DBUG_PRINT ("APC", ("code-node (%s=...)", ID_NAME (CODE_CEXPR (arg_node))));

    /* collect access patterns */
    arg_node = CollectAccessPatterns (arg_node);

    /* check type of id-node */
    if (!(ID_DIM (CODE_CEXPR (arg_node)) == 0)) {
        /* not a scalar type, so this with-loop is unsupported! */
        AddUnsupported (arg_info, ID_TYPE (CODE_CEXPR (arg_node)));
    }

    /* traverse code block */
    DBUG_ASSERT ((CODE_CBLOCK (arg_node) != NULL), " unexpected empty CBLOCK!");
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    /* traverse following code blocks (rvalue of assignment) */
    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
