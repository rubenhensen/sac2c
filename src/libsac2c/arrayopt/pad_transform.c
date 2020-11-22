#define DBUG_PREFIX "APT"
#include "debug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"
#include "DupTree.h"
#include "shape.h"

#include "pad_info.h"
#include "pad_transform.h"

#ifndef PADT_DEACTIVATED

/*
 * INFO structure
 */
struct INFO {
    bool flag;
    int withop_type;
    node *with;
    node *fundef;
    node *assignments;
};

/*
 * INFO macros
 */
#define INFO_APT_EXPRESSION_PADDED(n) (n->flag)
#define INFO_APT_WITHOP_TYPE(n) (n->withop_type)
#define INFO_APT_WITH(n) (n->with)
#define INFO_APT_FUNDEF(n) (n->fundef)
#define INFO_APT_ASSIGNMENTS(n) (n->assignments)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_APT_EXPRESSION_PADDED (result) = FALSE;
    INFO_APT_WITHOP_TYPE (result) = 0;
    INFO_APT_WITH (result) = NULL;
    INFO_APT_FUNDEF (result) = NULL;
    INFO_APT_ASSIGNMENTS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

#endif /* PADT_DEACTIVATED */

/*****************************************************************************
 *
 * file:   pad_transform.c
 *
 * prefix: APT
 *
 * description:
 *
 *   This compiler module applies array padding.
 *
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * function:
 *   void APtransform (node *arg_node)
 *
 * description:
 *   main function for applying array padding
 *
 *****************************************************************************/

void
APdoTransform (node *arg_node)
{

    DBUG_ENTER ();

#ifndef PADT_DEACTIVATED
    info *arg_info; /* before DBUG_ENTER */

    DBUG_PRINT ("Array Padding: applying transformation...");

    arg_info = MakeInfo ();

    TRAVpush (TR_apt);

    arg_node = TRAVdo (arg_node, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

#endif /* PADT_DEACTVATED */

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   static char *PadName(char* unpaddedName)
 *
 * description:
 *   generate new name with suffix '__PAD' appended to argument
 *   !!! argument is set free within this function !!!
 *
 *****************************************************************************/

#ifndef PADT_DEACTIVATED

static char *
PadName (char *unpadded_name)
{

    char *padded_name;

    DBUG_ENTER ();

    padded_name = (char *)MEMmalloc (STRlen (unpadded_name) + 6 * sizeof (char));
    strcpy (padded_name, unpadded_name);
    strcat (padded_name, "__PAD");

    unpadded_name = MEMfree (unpadded_name);

    DBUG_RETURN (padded_name);
}

/*****************************************************************************
 *
 * function:
 *   static node* PadIds(node *arg, info *arg_info)
 *
 * description:
 *   try padding of lvalues if possible
 *   function for N_let will ensure matching shapes between rvalues and lvalues
 *   returns successful padding in arg_info
 *
 *****************************************************************************/

static void
PadIds (node *arg, info *arg_info)
{

    DBUG_ENTER ();

    DBUG_PRINT ("check ids-attribute");

    /* (for this test ensure that args and vardecs have been traversed previously!) */
    if (IDS_PADDED (arg)) {
        DBUG_PRINT (" padding: '%s' -> '%s'", IDS_NAME (arg), IDS_DECL_NAME (arg));
        IDS_NAME (arg) = PadName (IDS_NAME (arg));
        INFO_APT_EXPRESSION_PADDED (arg_info) = TRUE;
    } else {
        DBUG_PRINT (" leaving unpadded: '%s''", IDS_NAME (arg));
        if ((NODE_TYPE (IDS_DECL (arg)) == N_vardec)
            && (VARDEC_PADDED (IDS_DECL (arg)))) {
            IDS_DECL (arg) = IDS_DECL_NEXT (arg);
            /* do not reset PADDED-flag in arg_info */
        }
        /* nothing to be done for arg-nodes */
    }

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   static shpseg* LBound(shpseg* old_shape, int dims, int current_dim)
 *
 * description:
 *   calculate lower bound for generator-node in current_dim
 *   be carefull: (lower_bound <= idx < upper_bound) required!
 *
 *****************************************************************************/

static shpseg *
LBound (shpseg *old_shape, int dims, int current_dim)
{

    int j;
    shpseg *lbound_shape;

    DBUG_ENTER ();

    lbound_shape = TBmakeShpseg (NULL);
    for (j = 0; j < dims; j++) {
        if (j == current_dim) {
            SHPSEG_SHAPE (lbound_shape, j) = SHPSEG_SHAPE (old_shape, j);
        } else {
            SHPSEG_SHAPE (lbound_shape, j) = 0;
        }
    }

    DBUG_RETURN (lbound_shape);
}

/*****************************************************************************
 *
 * function:
 *   static shpseg* UBound(shpseg* old_shape, shpseg* new_shape, int dims,
 *                         int current_dim)
 *
 * description:
 *   calculate (upper bound +1) for generator-node in current_dim
 *   be carefull: (lower_bound <= idx < upper_bound) required!
 *
 *****************************************************************************/

static shpseg *
UBound (shpseg *old_shape, shpseg *new_shape, int dims, int current_dim)
{

    int j;
    shpseg *ubound_shape;

    DBUG_ENTER ();

    ubound_shape = TBmakeShpseg (NULL);
    for (j = 0; j < dims; j++) {
        if (j >= current_dim) {
            SHPSEG_SHAPE (ubound_shape, j) = SHPSEG_SHAPE (new_shape, j);
        } else {
            SHPSEG_SHAPE (ubound_shape, j) = SHPSEG_SHAPE (old_shape, j);
        }
    }

    DBUG_RETURN (ubound_shape);
}

/*****************************************************************************
 *
 * function:
 *   static node* AddDummyPart(node* wl, shpseg* old_shape,
 *                             shpseg* new_shape, int dims)
 *
 * description:
 *   add dummy part-nodes to with-node depending on shape-difference
 *   This ensures the padded space in the array to be filled up with 0-values.
 *
 *****************************************************************************/

static node *
AddDummyPart (node *wl, shpseg *old_shape, shpseg *new_shape, int dims)
{

    int i;
    shpseg *lbound_shape;
    shpseg *ubound_shape;
    node *lbound_array;
    node *ubound_array;
    node *withid;
    node *part;
    node *generator;
    node *code;

    DBUG_ENTER ();

    /* dummy code put at the beginning of code-chain by AddDummyCode */
    code = WITH_CODE (wl);

    for (i = 0; i < dims; i++) {

        /* padding required in this dimension? */

        if (SHPSEG_SHAPE (old_shape, i) != SHPSEG_SHAPE (new_shape, i)) {
            lbound_shape = LBound (old_shape, dims, i);
            ubound_shape = UBound (old_shape, new_shape, dims, i);
            lbound_array = SHshape2Array (lbound_shape);
            ubound_array = SHshape2Array (ubound_shape);
            FREEfreeShpseg (lbound_shape);
            FREEfreeShpseg (ubound_shape);

            /* generate (lower_bound <= idx < upper_bound) */
            generator
              = TBmakeGenerator (F_wl_le, F_wl_lt, lbound_array, ubound_array, NULL, NULL);

            /* copy reference to idx-variable from existing withid-node
             * (remember that there is only ONE idx-variable for all part-nodes
             *  of a with-loop!)
             */
            withid = DUPdoDupNode (PART_WITHID (WITH_PART (wl)));
            part = TBmakePart (withid, generator, code);
            CODE_INC_USED (code);
            PART_NEXT (part) = WITH_PART (wl);
            WITH_PART (wl) = part;
        }
    }

    DBUG_RETURN (wl);
}

/*****************************************************************************
 *
 * function:
 *   static node* AddDummyCode(node* wl)
 *
 * description:
 *   add dummy assignment (_apt_#=0;) as first code block to with-node
 *   returns pointer to newly inserted vardec
 *
 *****************************************************************************/

node *
AddDummyCode (node *wl)
{

    node *vardec;
    ntype *type;

    node *expr;
    node *ids_attrib;
    node *instr;
    node *assign;
    node *block;
    node *id;
    node *code;

    DBUG_ENTER ();

    /* find last vardec */

    /* BUG, if CODE is empty and result is specified by ARG instead if VARDEC !!!
     * vardec = ID_DECL(NCODE_CEXPR(WITH_CODE(wl)));
     *
     * Now we search the end of the vardec-list by looking at the vardec of the
     * with-loop index vector. Even, if the index vector is passed into a function
     * as an argument, the flatten phase will generate a separate vardec for the
     * index vector.
     */

    vardec = ID_DECL (PART_WITHID (WITH_PART (wl)));
    while (VARDEC_NEXT (vardec) != NULL) {
        vardec = VARDEC_NEXT (vardec);
    }

    /* append new vardec-node */
    type = ID_TYPE (CODE_CEXPR (WITH_CODE (wl)));
    DBUG_ASSERT (TYPES_NEXT (type) == NULL, "single type expected");
    VARDEC_NEXT (vardec) = TBmakeVardec (TRAVtmpVar (), DUPdupAllTypes (type), NULL);
    vardec = VARDEC_NEXT (vardec);

    /* add dummy code */
    switch (TYPES_BASETYPE (type)) {
    case T_byte:
        expr = TBmakeNumbyte (0);
        break;

    case T_short:
        expr = TBmakeNumshort (0);
        break;

    case T_int:
        expr = TBmakeNum (0);
        break;

    case T_long:
        expr = TBmakeNumlong (0);
        break;

    case T_longlong:
        expr = TBmakeNumlonglong (0);
        break;

    case T_ubyte:
        expr = TBmakeNumubyte (0);
        break;

    case T_ushort:
        expr = TBmakeNumushort (0);
        break;

    case T_uint:
        expr = TBmakeNumuint (0);
        break;

    case T_ulong:
        expr = TBmakeNumulong (0);
        break;

    case T_ulonglong:
        expr = TBmakeNumulonglong (0);
        break;

    case T_double:
        expr = TBmakeDouble (0);
        break;

    case T_float:
        expr = TBmakeFloat (0);
        break;

    case T_bool:
        expr = TBmakeBool (FALSE);
        break;

    case T_char:
        expr = TBmakeChar ('\0');
        break;

    default:
        DBUG_UNREACHABLE ("unsupported type in with-loop!");
        expr = NULL; /* just to avoid compiler warnings */
        break;
    }

    ids_attrib = TBmakeIds_Copy (VARDEC_NAME (vardec));
    IDS_DECL (ids_attrib) = vardec;
    instr = TBmakeLet (ids_attrib, expr);
    assign = TBmakeAssign (instr, NULL);
    block = TBmakeBlock (assign, NULL);
    id = TBmakeId_Copy (VARDEC_NAME (vardec));
    ID_DECL (id) = vardec;

    /*
     * dkr:
     * Why generating such a complicate dummy code????
     * It would be sufficient to generate an empty code here:
     *   TBmakeCode( MakeEmpty(), MakeEmpty())
     */
    code = TBmakeCode (block, TBmakeExprs (id, NULL));

    /* tag dummy code to identify it in further optimizations */
    CODE_AP_DUMMY_CODE (code) = TRUE;

    /*
     * Last but not least, we have to build array access analysis data to
     * annotate the Ncode node with.
     */

    CODE_WLAA_INFO (code) = MEMmalloc (sizeof (access_info_t));

    CODE_WLAA_ACCESS (code) = NULL;
    CODE_WLAA_ACCESSCNT (code) = 0;
    CODE_WLAA_FEATURE (code) = 0;
    CODE_WLAA_WLARRAY (code) = CODE_WLAA_WLARRAY (WITH_CODE (wl));
    CODE_WLAA_INDEXVAR (code) = IDS_DECL (WITH_VEC (wl));

    /* put dummy code at beginning of code-chain (required by AddDummyPart !!!) */
    CODE_NEXT (code) = WITH_CODE (wl);
    WITH_CODE (wl) = code;

    DBUG_RETURN (vardec);
}

/*****************************************************************************
 *
 * function:
 *   static void InsertWithLoopGenerator(ntype* oldtype, ntype* newtype, node* wl)
 *
 * description:
 *   insert new part-nodes and code-node into withloop to apply padding
 *
 *****************************************************************************/

static void
InsertWithLoopGenerator (ntype *oldtype, ntype *newtype, node *wl)
{

    shpseg *shape_diff;
    node *assignment_vardec;
    bool different = FALSE;
    int i;

    DBUG_ENTER ();

    if (!SHcompareShapes (TYgetType(oldtype), TYgetShape (newtype))) {

        /* add code block */
        assignment_vardec = AddDummyCode (wl);

        /* add nodes to part */
        wl = AddDummyPart (wl, TYgetShape (oldtype), TYgetShape (newtype),
                           TYgetDim (oldtype));
    }

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   node *APTarg(node *arg_node, info *arg_info)
 *
 * description:
 *   modify arg-node to padded name and shape
 *
 *****************************************************************************/

node *
APTarg (node *arg_node, info *arg_info)
{

    ntype *new_type;

    DBUG_ENTER ();

    DBUG_EXECUTE (if (ARG_NAME (arg_node) != NULL) {
        DBUG_PRINT ("APT", ("check arg: %s", ARG_NAME (arg_node)));
    } else { DBUG_PRINT ("APT", ("check arg: (NULL)")); });

    new_type = PIgetNewType (ARG_TYPE (arg_node));
    if (new_type != NULL) {
        /* found shape to be padded */
        ARG_TYPE (arg_node) = new_type;
        DBUG_PRINT (" padding: %s -> %s__PAD", ARG_NAME (arg_node), ARG_NAME (arg_node));
        ARG_NAME (arg_node) = PadName (ARG_NAME (arg_node));
        ARG_PADDED (arg_node) = TRUE;
    } else {
        ARG_PADDED (arg_node) = FALSE;
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTvardec(node *arg_node, info *arg_info)
 *
 * description:
 *   duplicate vardec to reserve unpadded shape
 *   modify original vardec to fit new shape
 *
 *****************************************************************************/

node *
APTvardec (node *arg_node, info *arg_info)
{

    ntype *new_type;
    node *original_vardec;

    DBUG_ENTER ();

    DBUG_PRINT ("check vardec: %s", VARDEC_NAME (arg_node));

    original_vardec = DUPdoDupNode (arg_node);

    new_type = PIgetNewType (VARDEC_TYPE (arg_node));
    if (new_type != NULL) {
        /* found shape to be padded */
        VARDEC_NEXT (original_vardec) = VARDEC_NEXT (arg_node);
        VARDEC_TYPE (arg_node) = new_type;
        DBUG_PRINT (" padding: %s -> %s__PAD", VARDEC_NAME (arg_node),
                    VARDEC_NAME (arg_node));

        VARDEC_NAME (arg_node) = PadName (VARDEC_NAME (arg_node));
        VARDEC_NEXT (arg_node) = original_vardec;

        VARDEC_PADDED (arg_node) = TRUE;

        if (VARDEC_NEXT (original_vardec) != NULL) {
            VARDEC_NEXT (original_vardec)
              = TRAVdo (VARDEC_NEXT (original_vardec), arg_info);
        }

    } else {
        FREEdoFreeNode (original_vardec);
        VARDEC_PADDED (arg_node) = FALSE;

        if (VARDEC_NEXT (arg_node) != NULL) {
            VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTassign(node *arg_node, info *arg_info)
 *
 * description:
 *   insert new assignments before itself, if necessarry
 *   traverse next assignment
 *
 *****************************************************************************/

node *
APTassign (node *arg_node, info *arg_info)
{

    node *tmp;
    node *new_assigns;
    node *new_node;

    DBUG_ENTER ();

    DBUG_PRINT ("assign-node detected");

    INFO_APT_ASSIGNMENTS (arg_info) = NULL;

    DBUG_ASSERT (ASSIGN_STMT (arg_node) != NULL, "unexpected empty ASSIGN_STMT");
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /* save new assigments */
    new_assigns = INFO_APT_ASSIGNMENTS (arg_info);

    /* traverse next assignment */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* APTassign will insert assignments stored in INFO_APT_ASSIGNMENTS
     * before the current assignment
     *
     * This is not needed yet, but maybe we need it later to insert
     * code for unpadding
     */

    if (new_assigns != NULL) {
        tmp = new_assigns;
        while (ASSIGN_NEXT (tmp) != NULL) {
            tmp = ASSIGN_NEXT (tmp);
        }
        ASSIGN_NEXT (tmp) = arg_node;
        new_node = new_assigns;
    } else {
        new_node = arg_node;
    }

    DBUG_RETURN (new_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTarray(node *arg_node, info *arg_info)
 *
 * description:
 *   constant arrays won't be padded
 *   =>array elements won't be traversed!
 *
 *****************************************************************************/

node *
APTarray (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    DBUG_EXECUTE (if (ARRAY_STRING (arg_node) != NULL) {
        DBUG_PRINT ("APT", (" trav array: %s", ARRAY_STRING (arg_node)));
    } else { DBUG_PRINT ("APT", (" trav array: (NULL)")); });

    /* constant arrays won't padded ! */
    INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

    /* do not traverse sons */

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTwith(node *arg_node, info *arg_info)
 *
 * description:
 *   check withop-node first, then traverse code-nodes
 *
 *****************************************************************************/

node *
APTwith (node *arg_node, info *arg_info)
{

    node *save_ptr;

    DBUG_ENTER ();

    DBUG_PRINT ("with-node detected");

    /* save pointer to outer with-loop to support nested loops */
    save_ptr = INFO_APT_WITH (arg_info);

    INFO_APT_WITH (arg_info) = arg_node;
    INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

    /* check withop, if with-loop needs to be padded */
    DBUG_ASSERT (WITH_WITHOP (arg_node) != NULL, " unexpected empty WITHOP!");
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /* arg_info passes check result to code-nodes */
    DBUG_ASSERT (WITH_CODE (arg_node) != NULL, " unexpected empty CODE!");
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /* no need to traverse part-nodes */

    /* EXPRESSION_PADDED is returned to upper function */

    /* restore pointer to outer with-loop */
    INFO_APT_WITH (arg_info) = save_ptr;

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTcode(node *arg_node, info *arg_info)
 *
 * description:
 *   apply padding to ids-attribute and traverse sons
 *
 *****************************************************************************/

node *
APTcode (node *arg_node, info *arg_info)
{

    bool rhs_padded;
    bool save_padded_state;

    DBUG_ENTER ();

    DBUG_PRINT ("code-node detected");

    save_padded_state = INFO_APT_EXPRESSION_PADDED (arg_info);

    /* traverse code block */
    DBUG_ASSERT (CODE_CBLOCK (arg_node) != NULL, " unexpected empty CBLOCK!");
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    rhs_padded = INFO_APT_EXPRESSION_PADDED (arg_info);

    /* traverse id-node (lvalue of assignment) */
    DBUG_ASSERT (CODE_CEXPR (arg_node) != NULL, " unexpected empty CEXPR!");
    CODE_CEXPR (arg_node) = TRAVdo (CODE_CEXPR (arg_node), arg_info);

    /*
     * enable for consistency checking
     * requires shape information from pad_infer!
     * @@@ */
    DBUG_ASSERT (((!INFO_APT_EXPRESSION_PADDED (arg_info)) || rhs_padded),
                 " padding of lvalue does not match rvalue!");
    /* */

    /* traverse following code blocks (rvalue of assignment) */
    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    INFO_APT_EXPRESSION_PADDED (arg_info) = save_padded_state;

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTgenarray(node *arg_node, info *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APTgenarray (node *arg_node, info *arg_info)
{

    shape *shp;
    int dim;
    int simpletype;
    ntype *oldtype = NULL;
    ntype *newtype = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("withop-node detected");

    INFO_APT_WITHOP_TYPE (arg_info) = NODE_TYPE (arg_node);

    /* set default - change it, if padding can be applied */
    INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

    DBUG_PRINT (" genarray-loop");

    shp = SHarray2Shape (GENARRAY_SHAPE (arg_node), NULL);
    /* constant array has dim=1
     * => number of elements is stored in shp[0]
     */
    dim = SHgetUnrLen (ARRAY_SHAPE (GENARRAY_SHAPE (arg_node)));
    /* all elements have the same type
     * => use simpletype of first code-node
     */
    simpletype
      = TYPES_BASETYPE (ID_TYPE (CODE_CEXPR (WITH_CODE (INFO_APT_WITH (arg_info)))));
    /* infer result of with-loop
       Attention: only elements with scalar types are supported yet !!!
    */
    oldtype = TYmakeAKS (TYmakeSimple (simpletype), shp);
    newtype = PIgetNewType (TYcopyType (oldtype));

    if (newtype != NULL) {
        /* apply padding (genarray-specific)*/

        /* pad shape of new array specified in WITHOP_SHAPE (pointing to array-node) */

        FREEdoFreeNode (GENARRAY_SHAPE (arg_node));
        GENARRAY_SHAPE (arg_node)
          = SHshape2Array (TYgetShape (newtype));

        INFO_APT_EXPRESSION_PADDED (arg_info) = TRUE;
    }

    /* check all sons for paddable code */
    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    }

    /* apply WO_TYPE independend padding */
    if (INFO_APT_EXPRESSION_PADDED (arg_info)) {
        if (WITH_PARTS (INFO_APT_WITH (arg_info)) > 0) {
            /* partition complete => add parts and code */
            InsertWithLoopGenerator (oldtype, newtype, INFO_APT_WITH (arg_info));
        }
    }

    /* free data structures */
    FREEfreeOneTypes (oldtype);
    FREEfreeOneTypes (newtype);

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTmodarray(node *arg_node, info *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APTmodarray (node *arg_node, info *arg_info)
{

    ntype *oldtype = NULL;
    ntype *newtype = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("withop-node detected");

    INFO_APT_WITHOP_TYPE (arg_info) = NODE_TYPE (arg_node);

    /* set default - change it, if padding can be applied */
    INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

    DBUG_PRINT (" modarray-loop");

    if (ID_PADDED (MODARRAY_ARRAY (arg_node))) {
        /* apply padding (modarray-specific)*/

        /* attention: id-node already points to padded shape! */
        newtype = DUPdupAllTypes (ID_TYPE (MODARRAY_ARRAY (arg_node)));
        oldtype = PIgetOldType (DUPdupAllTypes (newtype));

        /* pad array referenced by WITHOP_ARRAY (pointing to id-node)*/
        DBUG_ASSERT (MODARRAY_ARRAY (arg_node) != NULL, " unexpected empty ARRAY!");
        MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

        INFO_APT_EXPRESSION_PADDED (arg_info) = TRUE;
    }

    /* apply WO_TYPE independend padding */
    if (INFO_APT_EXPRESSION_PADDED (arg_info)) {
        if (WITH_PARTS (INFO_APT_WITH (arg_info)) > 0) {
            /* partition complete => add parts and code */
            InsertWithLoopGenerator (oldtype, newtype, INFO_APT_WITH (arg_info));
        }
    }

    /* free data structures */
    FREEfreeOneTypes (oldtype);
    FREEfreeOneTypes (newtype);

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTfold(node *arg_node, info *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APTfold (node *arg_node, info *arg_info)
{

    ntype *oldtype = NULL;
    ntype *newtype = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("withop-node detected");

    INFO_APT_WITHOP_TYPE (arg_info) = NODE_TYPE (arg_node);

    /* set default - change it, if padding can be applied */
    INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

    DBUG_PRINT (" foldfun-loop");

    /* check all sons for paddable code */
    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    /* apply WO_TYPE independend padding */
    if (INFO_APT_EXPRESSION_PADDED (arg_info)) {
        if (WITH_PARTS (INFO_APT_WITH (arg_info)) > 0) {
            /* partition complete => add parts and code */
            InsertWithLoopGenerator (oldtype, newtype, INFO_APT_WITH (arg_info));
        }
    }

    /* free data structures */
    FREEfreeOneTypes (oldtype);
    FREEfreeOneTypes (newtype);

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTap(node *arg_node, info *arg_info)
 *
 * description:
 *   check, if function can be padded (has body)
 *   traverse arguments
 *
 *****************************************************************************/

node *
APTap (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    DBUG_PRINT ("ap-node detected");

    DBUG_EXECUTE (if (AP_NAME (arg_node) != NULL) {
        DBUG_PRINT ("APT", (" trav ap: %s", AP_NAME (arg_node)));
    } else { DBUG_PRINT ("APT", (" trav ap: (NULL)")); });

    /* first inspect arguments */
    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    /* look whether body exists or not
     * EXPRESSION_PADDED does not depend on arguments! */
    if (FUNDEF_BODY (AP_FUNDEF (arg_node)) == NULL) {
        /* function without body -> results will have unpadded shape */
        INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;
    } else {
        /* the result of an unser-defined function might be of a padded shape */
        INFO_APT_EXPRESSION_PADDED (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTid(node *arg_node, info *arg_info)
 *
 * description:
 *   rename node, if refering to padded vardec or arg
 *
 *****************************************************************************/

node *
APTid (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    DBUG_EXECUTE (if (ID_NAME (arg_node) != NULL) {
        DBUG_PRINT ("APT", ("check id: %s", ID_NAME (arg_node)));
    } else { DBUG_PRINT ("APT", ("check id: (NULL)")); });

    /*
     * for this test ensure that args and vardecs have been
     * previously traversed!
     */
    if (ID_PADDED (arg_node)) {
        DBUG_PRINT (" padding: '%s' -> '%s'", ID_NAME (arg_node),
                    ID_DECL_NAME (arg_node));
        ID_NAME (arg_node) = PadName (ID_NAME (arg_node));
        INFO_APT_EXPRESSION_PADDED (arg_info) = TRUE;
    } else {
        /* if not padded, update pointer to vardec */
        DBUG_PRINT (" leaving unpadded: '%s''", ID_NAME (arg_node));
        if ((NODE_TYPE (ID_DECL (arg_node)) == N_vardec)
            && (VARDEC_PADDED (ID_DECL (arg_node)))) {
            ID_DECL (arg_node) = ID_DECL_NEXT (arg_node);
            INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;
        }
        /* nothing to be done for arg-nodes */
    }

    /* no sons to traverse */

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTprf(node *arg_node, info *arg_info)
 *
 * description:
 *   only some functions supported yet
 *
 *****************************************************************************/

node *
APTprf (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    DBUG_PRINT ("prf-node detected: '%s'", global.prf_name[PRF_PRF (arg_node)]);

    /* only some PRFs may be padded successfully (without conversion) */

    switch (PRF_PRF (arg_node)) {

    case F_sel_VxA:
        DBUG_ASSERT (PRF_ARGS (arg_node) != NULL, " sel() has empty argument list!");
        /* traverse arguments to apply padding */
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

        /* even if PRF_ARGS is padded, the result of PRF will have an
         * unpadded shape => return FALSE */
        INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

        break;

    case F_dim_A:
        DBUG_ASSERT (PRF_ARGS (arg_node) != NULL, " dim() has empty argument list!");
        /* traverse arguments to apply padding */
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

        /* even if PRF_ARGS is padded, the result of PRF will have an
         * unpadded shape => return FALSE */
        INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

        break;

    case F_shape_A:
        DBUG_ASSERT (PRF_ARGS (arg_node) != NULL, " shape() has empty argument list!");
        /* check, if argument has paddable shape */

        if (ID_PADDED (PRF_ARG1 (arg_node))) {
            /* substitute paddable argument with reference to constant vector
               containing padded shape */
            ntype *old_type;

            old_type = PIgetOldType (
              DUPdupAllTypes (VARDEC_TYPE (ID_DECL (PRF_ARG1 (arg_node)))));
            arg_node = SHshape2Array (TYgetShape (old_type));
            old_type = MEMfree (old_type);
        }
        /* even if PRF_ARG1 is padded, the result of PRF will have an
         * unpadded shape => return FALSE */
        INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

        break;

    default:
        /* accept PRFs for scalar and vector arguments, but do no padding */
        INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTfundef(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse ARGS before BODY before NEXT
 *
 *****************************************************************************/

node *
APTfundef (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    DBUG_EXECUTE (if (FUNDEF_NAME (arg_node) != NULL) {
        DBUG_PRINT ("APT", (" trav fundef: %s", FUNDEF_NAME (arg_node)));
    } else { DBUG_PRINT ("APT", (" trav fundef: (NULL)")); });

    INFO_APT_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    } else {
        DBUG_PRINT (" no args");
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    } else {
        DBUG_PRINT (" no body");
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTblock(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse VARDEC before INSTR
 *
 *****************************************************************************/

node *
APTblock (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    DBUG_PRINT ("trav block");

    if (BLOCK_VARDECS (arg_node) != NULL) {
        BLOCK_VARDECS (arg_node) = TRAVdo (BLOCK_VARDECS (arg_node), arg_info);
    } else {
        DBUG_PRINT (" no vardec");
    }

    DBUG_ASSERT (BLOCK_ASSIGNS (arg_node) != NULL, "unexpected empty INSTR!");
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTlet(node *arg_node, info *arg_info)
 *
 * description:
 *   apply padding to rvalues and lvalues if necessary
 *   ensure matching shape between rvalues and lvalues
 *
 *****************************************************************************/

node *
APTlet (node *arg_node, info *arg_info)
{

    node *ids_ptr;
    bool rhs_padded;

    DBUG_ENTER ();

    DBUG_PRINT ("check let-node");

    /* set FALSE as default, so separate functions for N_num, N_float,
       N_double, N_char, N_bool are not needed */
    INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

    DBUG_ASSERT (LET_EXPR (arg_node) != NULL, "let-node without rvalues detected!");
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    rhs_padded = INFO_APT_EXPRESSION_PADDED (arg_info);

    /* rhs_padded==TRUE
     * => right hand side can possibly be padded, so it may be an ap
     *    refering to fundef with body or a variable with padded shape
     * rhs_padded==FALSE
     * => right hand side may be an ap refering to fundef without a body,
     *    prf, array, constant (num, ...) or a variable with unpadded shape
     */

    INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;
    /* manually traverse ids */
    ids_ptr = LET_IDS (arg_node);
    while (ids_ptr != NULL) {
        /* call modifies arg_info */
        PadIds (ids_ptr, arg_info);
        ids_ptr = IDS_NEXT (ids_ptr);
    }

    /*
     * enable for consistency checking
     * requires shape information from pad_infer!
     * @@@ */
    DBUG_ASSERT (((!INFO_APT_EXPRESSION_PADDED (arg_info)) || rhs_padded),
                 "padding of lvalue does not match rvalue!");
    /* */

    DBUG_RETURN (arg_node);
}
#endif /* PADT_DEACTVATED */

#undef DBUG_PREFIX
