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

#define DBUG_PREFIX "APC"
#include "debug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"
#include "DupTree.h"
#include "str.h"
#include "memory.h"
#include "new_types.h"
#include "type_utils.h"
#include "new_typecheck.h"

#include "pad_info.h"

/*
 * INFO structure
 */
struct INFO {
    bool unsupported;
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
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_APC_UNSUPPORTED (result) = 0;
    INFO_APC_COUNT_CHANGES (result) = 0;
    INFO_APC_WITH (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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

    DBUG_ENTER ();

    DBUG_PRINT ("Array Padding: collecting data...");

    arg_info = MakeInfo ();
    INFO_APC_COUNT_CHANGES (arg_info) = 0;

    TRAVpush (TR_apc);

    arg_node = TRAVdo (arg_node, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_EXECUTE (PIprintAccessPatterns (); PIprintUnsupportedShapes ());

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   static shape* AccessClass2Group(accessclass_t class, int dim)
 *
 * description:
 *   convert access class into vector of integer factors
 *   accessing with ACL_const means [0]*index+offset
 *   accessing with ACL_offset means [1]*index+offset
 *   currently no other access classes are supported
 *   result is NULL, if access class is unsupported
 *
 *****************************************************************************/

static shape *
AccessClass2Group (accessclass_t xclass, int dim)
{

    shape *vector;
    int element;
    int i;

    DBUG_ENTER ();

    switch (xclass) {
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

        vector = SHmakeShape (dim);

        for (i = 0; i < dim; i++) {
            vector = SHsetExtent (vector, i, element);
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
    shape *group_vect;
    shape *offset;
    simpletype type;
    int dim;
    shape *shp;
    pattern_t *patterns;
    accessdir_t direction;

    DBUG_ENTER ();

    DBUG_PRINT ("collecting access patterns...");

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

            offset = SHcopyShape (ACCESS_OFFSET (access_ptr));
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
        dim = TYgetDim (VARDEC_NTYPE (COL_ARRAY (col_ptr)));
        shp = SHcopyShape (TYgetShape (VARDEC_NTYPE (COL_ARRAY (col_ptr))));
        group_vect = AccessClass2Group (COL_CLASS (col_ptr), dim);
        direction = COL_DIR (col_ptr);
        patterns = COL_PATTERNS (col_ptr);
        PIaddAccessPattern (type, dim, shp, group_vect, direction, patterns);
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
 *   static node* AddUnsupported(node* arg_info, ntype* array_type)
 *
 * description:
 *   wrapper to simplify adding an unsupported shape
 *   only increase changes, if shape hasn't been added before
 *
 *****************************************************************************/

static void
AddUnsupported (info *arg_info, ntype *array_type)
{

    DBUG_ENTER ();

    INFO_APC_UNSUPPORTED (arg_info) = TRUE;

    /* only non-scalar types will be added to list of unsupported shapes!
     * scalar types do not have a shape!
     */
    if (TUgetFullDimEncoding (array_type) > 0) {
        if (PIaddUnsupportedShape (TYcopyType (array_type))) {
            INFO_APC_COUNT_CHANGES (arg_info)++;
        }
    }

    DBUG_RETURN ();
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

    DBUG_ENTER ();

    DBUG_PRINT ("array-node detected");

    atype = NTCnewTypeCheck_Expr (arg_node);

    AddUnsupported (arg_info, atype);

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

    DBUG_ENTER ();

    DBUG_PRINT ("with-node detected");

    /* save pointer to outer with-loop to support nested loops */
    save_ptr = INFO_APC_WITH (arg_info);

    INFO_APC_WITH (arg_info) = arg_node;

    DBUG_ASSERT (WITH_CODE (arg_node) != NULL, " unexpected empty CODE!");
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /* traverse withop */
    DBUG_ASSERT (WITH_WITHOP (arg_node) != NULL, " unexpected empty WITHOP!");
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
    DBUG_ENTER ();

    DBUG_PRINT ("ap-node detected");

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
    DBUG_ENTER ();

    DBUG_PRINT ("id-node detected");

    if (INFO_APC_UNSUPPORTED (arg_info)) {
        AddUnsupported (arg_info, ID_NTYPE (arg_node));
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
    DBUG_ENTER ();

    DBUG_PRINT ("prf-node detected: '%s'", global.prf_name[PRF_PRF (arg_node)]);

    INFO_APC_UNSUPPORTED (arg_info) = TRUE;

    switch (PRF_PRF (arg_node)) {
    case F_sel_VxA:
    case F_idx_sel:
    case F_dim_A:
    case F_shape_A:
        INFO_APC_UNSUPPORTED (arg_info) = TRUE;
        break;
    default:
        INFO_APC_UNSUPPORTED (arg_info) = TRUE;
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
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
    DBUG_ENTER ();

    DBUG_EXECUTE (if (FUNDEF_NAME (arg_node) != NULL) {
        DBUG_PRINT ("fundef-node: %s", FUNDEF_NAME (arg_node));
    } else { DBUG_PRINT ("fundef-node: (NULL)"); });

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

    DBUG_ENTER ();

    DBUG_PRINT ("let-node detected (%s=...)", IDS_NAME (LET_IDS (arg_node)));

    save_state = INFO_APC_UNSUPPORTED (arg_info);
    INFO_APC_UNSUPPORTED (arg_info) = FALSE;

    DBUG_ASSERT (LET_EXPR (arg_node) != NULL, " let-node without rvalues detected!");
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_APC_UNSUPPORTED (arg_info)) {
        ids_ptr = LET_IDS (arg_node);
        while (ids_ptr != NULL) {
            AddUnsupported (arg_info, IDS_NTYPE (ids_ptr));
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
    DBUG_ENTER ();

#if 0
  shape* shp;
  ntype* type;
  int dim;
  simpletype basetype;


  DBUG_PRINT ("withop-node detected");

  DBUG_PRINT (" genarray-loop");
  if (INFO_APC_UNSUPPORTED(arg_info)) {
  TODO: the following assumes, genarray_shape is given by a N_array node
    /* do not add type of vector, but contents of array to unsupported shapes */
    basetype = TYPES_BASETYPE(ID_TYPE(WITH_CEXPR(INFO_APC_WITH(arg_info))));

    dim = SHPSEG_SHAPE(TYPES_SHPSEG(ARRAY_TYPE(GENARRAY_SHAPE(arg_node))),0);

    shp = SHarray2Shape (GENARRAY_SHAPE(arg_node));

    type = TYmakeAKS (TYmakeSimple (basetype), shape);

    AddUnsupported(arg_info,type);

    type = TYfreeType( type);
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
    DBUG_ENTER ();

    DBUG_PRINT ("withop-node detected");

    DBUG_PRINT (" modarray-loop");

    if (INFO_APC_UNSUPPORTED (arg_info)) {
        AddUnsupported (arg_info, ID_NTYPE (MODARRAY_ARRAY (arg_node)));
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
    DBUG_ENTER ();

    DBUG_PRINT ("withop-node detected");

    DBUG_PRINT (" foldfun-loop");

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

    DBUG_ENTER ();

    DBUG_ASSERT (CODE_CEXPR (arg_node) != NULL, " unexpected empty CEXPR!");
    DBUG_PRINT ("code-node (%s=...)", ID_NAME (CODE_CEXPR (arg_node)));

    /* collect access patterns */
    arg_node = CollectAccessPatterns (arg_node);

    /* check type of id-node */
    if (TYgetDim (ID_NTYPE (CODE_CEXPR (arg_node))) != 0) {
        /* not a scalar type, so this with-loop is unsupported! */
        AddUnsupported (arg_info, ID_NTYPE (CODE_CEXPR (arg_node)));
    }

    /* traverse code block */
    DBUG_ASSERT (CODE_CBLOCK (arg_node) != NULL, " unexpected empty CBLOCK!");
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    /* traverse following code blocks (rvalue of assignment) */
    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
