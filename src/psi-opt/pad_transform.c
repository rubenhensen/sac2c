/*
 *
 * $Log$
 * Revision 3.14  2003/03/18 16:30:34  sah
 * added new prf cat_VxV, take_SxV, drop_SxV
 *
 * Revision 3.13  2002/09/13 21:51:17  dkr
 * bug in AddDummyCode() fixed:
 * TYPES_NAME used in a wrong manner
 *
 * Revision 3.12  2002/09/11 23:15:09  dkr
 * prf_node_info.mac modified
 *
 * Revision 3.11  2002/09/09 19:15:20  dkr
 * prf_string removed (mdb_prf used instead)
 *
 * Revision 3.10  2002/09/09 17:49:04  dkr
 * F_{add,sub,mul,div} replaced by F_{add,sub,mul,div}_SxS
 *
 * Revision 3.9  2002/07/29 12:12:53  sbs
 * PRF_IF macro extended by z.
 *
 * Revision 3.8  2002/02/20 14:56:49  dkr
 * fundef DupTypes() renamed into DupAllTypes()
 *
 * Revision 3.7  2001/07/17 15:12:51  cg
 * Bug fixed: new noop codes introduced by pad_transform are now
 * annotated with respective array access analysis information.
 *
 * Revision 3.6  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.5  2001/05/17 13:41:26  nmw
 * MALLOC/FREE replaced by Malloc/Free, using result of Free()
 *
 * Revision 3.4  2001/05/16 19:52:47  nmw
 * reverted Free() to FREE() due to segfaults when used with linux :-(
 *
 * Revision 3.3  2001/01/17 17:37:49  dkr
 * comment added
 *
 * Revision 3.2  2000/12/06 19:22:16  cg
 * Removed compiler warnings in production mode.
 *
 * Revision 3.1  2000/11/20 18:01:53  sacbase
 * new release made
 *
 * Revision 1.17  2000/10/31 23:02:14  dkr
 * signature of Array2Shpseg() changed
 *
 * Revision 1.16  2000/10/24 11:52:25  dkr
 * MakeType renamed into MakeTypes
 *
 * Revision 1.15  2000/08/04 14:32:52  mab
 * added some comments
 *
 * Revision 1.14  2000/08/04 11:41:31  mab
 * fixed bug in APCwith and APTwith (case nested with-loops)
 *
 * Revision 1.13  2000/08/03 15:36:33  mab
 * debugged transformation
 * (conversion functions not yet supported)
 *
 * Revision 1.11  2000/07/13 14:17:58  mab
 * changed padding of with-loops
 * added support for further PRFs
 *
 * Revision 1.10  2000/07/07 12:05:41  mab
 * completed first version of padding-transformation :-)
 *
 * Revision 1.9  2000/07/05 15:33:43  mab
 * with-loop completed (with-op, with-code and supporting functions)
 *
 * Revision 1.8  2000/06/30 15:22:58  mab
 * started implementing APTwithop
 * implemented dummy code generation for with loop
 *
 * Revision 1.7  2000/06/29 10:23:38  mab
 * - added dummy functions for APTpart, APTwithid, APTgenerator, APTcode,
 *   APTwithop
 * - renamed APTNwith to APTwith
 *
 * Revision 1.6  2000/06/28 10:41:35  mab
 * completed padding functions except with node
 * some code modifications according to code review
 *
 * Revision 1.5  2000/06/15 14:38:55  mab
 * implemented APTfundef, APTblock, APTid
 * dummy for APTlet added
 *
 * Revision 1.4  2000/06/14 10:46:04  mab
 * implemented APTvardec, APTarg
 * added dummies for APT ap, exprs, id, prf, fundef
 *
 * Revision 1.3  2000/06/08 11:14:14  mab
 * added functions for arg, vardec, array
 *
 * Revision 1.2  2000/05/31 16:16:58  mab
 * initial version
 *
 * Revision 1.1  2000/05/26 13:42:33  sbs
 * Initial revision
 *
 */

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"
#include "DupTree.h"
#include "Error.h"

#include "pad_info.h"
#include "pad_transform.h"

#include "my_debug.h"

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
APtransform (node *arg_node)
{

    node *arg_info;
    funtab *tmp_tab;

    DBUG_ENTER ("APtransform");

    DBUG_PRINT ("APT", ("Array Padding: applying transformation..."));

    tmp_tab = act_tab;
    act_tab = apt_tab;

    arg_info = MakeInfo ();

    arg_node = Trav (arg_node, arg_info);

    arg_info = FreeTree (arg_info);

    act_tab = tmp_tab;

    ABORT_ON_ERROR;

    DBUG_VOID_RETURN;
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

static char *
PadName (char *unpadded_name)
{

    char *padded_name;

    DBUG_ENTER ("PadName");

    padded_name = (char *)Malloc (strlen (unpadded_name) + 6 * sizeof (char));
    strcpy (padded_name, unpadded_name);
    strcat (padded_name, "__PAD");

    unpadded_name = Free (unpadded_name);

    DBUG_RETURN (padded_name);
}

/*****************************************************************************
 *
 * function:
 *   static node* PadIds(ids *arg, node* arg_info)
 *
 * description:
 *   try padding of lvalues if possible
 *   function for N_let will ensure matching shapes between rvalues and lvalues
 *   returns successful padding in arg_info
 *
 *****************************************************************************/

static node *
PadIds (ids *arg, node *arg_info)
{

    DBUG_ENTER ("PadIds");

    DBUG_PRINT ("APT", ("check ids-attribute"));

    /* (for this test ensure that args and vardecs have been traversed previously!) */
    if (IDS_PADDED (arg)) {
        DBUG_PRINT ("APT",
                    (" padding: '%s' -> '%s'", IDS_NAME (arg), IDS_VARDEC_NAME (arg)));
        IDS_NAME (arg) = PadName (IDS_NAME (arg));
        INFO_APT_EXPRESSION_PADDED (arg_info) = TRUE;
    } else {
        DBUG_PRINT ("APT", (" leaving unpadded: '%s''", IDS_NAME (arg)));
        if ((NODE_TYPE (IDS_VARDEC (arg)) == N_vardec)
            && (VARDEC_PADDED (IDS_VARDEC (arg)))) {
            IDS_VARDEC (arg) = IDS_VARDEC_NEXT (arg);
            /* do not reset PADDED-flag in arg_info */
        }
        /* nothing to be done for arg-nodes */
    }

    DBUG_RETURN (arg_info);
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

    DBUG_ENTER ("LBound");

    lbound_shape = MakeShpseg (NULL);
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

    DBUG_ENTER ("UBound");

    ubound_shape = MakeShpseg (NULL);
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

    DBUG_ENTER ("AddDummyPart");

    /* dummy code put at the beginning of code-chain by AddDummyCode */
    code = NWITH_CODE (wl);

    for (i = 0; i < dims; i++) {

        /* padding required in this dimension? */

        if (SHPSEG_SHAPE (old_shape, i) != SHPSEG_SHAPE (new_shape, i)) {
            lbound_shape = LBound (old_shape, dims, i);
            ubound_shape = UBound (old_shape, new_shape, dims, i);
            lbound_array = Shpseg2Array (lbound_shape, dims);
            ubound_array = Shpseg2Array (ubound_shape, dims);
            FreeShpseg (lbound_shape);
            FreeShpseg (ubound_shape);

            /* generate (lower_bound <= idx < upper_bound) */
            generator
              = MakeNGenerator (lbound_array, ubound_array, F_le, F_lt, NULL, NULL);
            /* copy reference to idx-variable from existing withid-node
             * (remember that there is only ONE idx-variable for all part-nodes
             *  of a with-loop!)
             */
            withid = DupNode (NPART_WITHID (NWITH_PART (wl)));
            part = MakeNPart (withid, generator, code);
            NPART_NEXT (part) = NWITH_PART (wl);
            NWITH_PART (wl) = part;
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
    types *type;

    node *expr;
    ids *ids_attrib;
    node *instr;
    node *assign;
    node *block;
    node *id;
    node *code;

    DBUG_ENTER ("AddDummyCode");

    /* find last vardec */

    /* BUG, if CODE is empty and result is specified by ARG instead if VARDEC !!!
     * vardec = ID_VARDEC(NCODE_CEXPR(NWITH_CODE(wl)));
     *
     * Now we search the end of the vardec-list by looking at the vardec of the
     * with-loop index vector. Even, if the index vector is passed into a function
     * as an argument, the flatten phase will generate a separate vardec for the
     * index vector.
     */

    vardec = ID_VARDEC (NPART_WITHID (NWITH_PART (wl)));
    while (VARDEC_NEXT (vardec) != NULL) {
        vardec = VARDEC_NEXT (vardec);
    }

    /* append new vardec-node */
    type = ID_TYPE (NCODE_CEXPR (NWITH_CODE (wl)));
    DBUG_ASSERT ((TYPES_NEXT (type) == NULL), "single type expected");
    VARDEC_NEXT (vardec) = MakeVardec (TmpVar (), DupAllTypes (type), NULL);
    vardec = VARDEC_NEXT (vardec);

    /* add dummy code */
    switch (TYPES_BASETYPE (type)) {
    case T_int:
        expr = MakeNum (0);
        break;

    case T_double:
        expr = MakeDouble (0);
        break;

    case T_float:
        expr = MakeFloat (0);
        break;

    case T_bool:
        expr = MakeBool (FALSE);
        break;

    case T_char:
        expr = MakeChar ('\0');
        break;

    default:
        DBUG_ASSERT ((0), "unsupported type in with-loop!");
        expr = NULL; /* just to avoid compiler warnings */
        break;
    }

    ids_attrib = MakeIds_Copy (VARDEC_NAME (vardec));
    IDS_VARDEC (ids_attrib) = vardec;
    instr = MakeLet (expr, ids_attrib);
    assign = MakeAssign (instr, NULL);
    block = MakeBlock (assign, NULL);
    id = MakeId_Copy (VARDEC_NAME (vardec));
    ID_VARDEC (id) = vardec;

    /*
     * dkr:
     * Why generating such a complicate dummy code????
     * It would be sufficient to generate an empty code here:
     *   MakeNCode( MakeEmpty(), MakeEmpty())
     */
    code = MakeNCode (block, id);

    /* tag dummy code to identify it in further optimizations */
    NCODE_AP_DUMMY_CODE (code) = TRUE;

    /*
     * Last but not least, we have to build array access analysis data to
     * annotate the Ncode node with.
     */

    NCODE_WLAA_INFO (code) = MakeInfo ();

    NCODE_WLAA_ACCESS (code) = NULL;
    NCODE_WLAA_ACCESSCNT (code) = 0;
    NCODE_WLAA_FEATURE (code) = 0;
    NCODE_WLAA_WLARRAY (code) = NCODE_WLAA_WLARRAY (NWITH_CODE (wl));
    NCODE_WLAA_INDEXVAR (code) = IDS_VARDEC (NWITH_VEC (wl));

    /* put dummy code at beginning of code-chain (required by AddDummyPart !!!) */
    NCODE_NEXT (code) = NWITH_CODE (wl);
    NWITH_CODE (wl) = code;

    DBUG_RETURN (vardec);
}

/*****************************************************************************
 *
 * function:
 *   static void InsertWithLoopGenerator(types* oldtype, types* newtype, node* wl)
 *
 * description:
 *   insert new part-nodes and code-node into withloop to apply padding
 *
 *****************************************************************************/

static void
InsertWithLoopGenerator (types *oldtype, types *newtype, node *wl)
{

    shpseg *shape_diff;
    node *assignment_vardec;
    bool different = FALSE;
    int i;

    DBUG_ENTER ("InsertWithLoopGenerator");

    /* calculate shape difference */
    shape_diff
      = DiffShpseg (TYPES_DIM (oldtype), TYPES_SHPSEG (newtype), TYPES_SHPSEG (oldtype));
    for (i = 0; i < TYPES_DIM (oldtype); i++) {
        if (SHPSEG_SHAPE (shape_diff, i) > 0) {
            different = TRUE;
        }
        DBUG_ASSERT ((SHPSEG_SHAPE (shape_diff, i) >= 0), "negative shape difference");
    }

    if (different) {

        /* add code block */
        assignment_vardec = AddDummyCode (wl);

        /* add nodes to part */
        wl = AddDummyPart (wl, TYPES_SHPSEG (oldtype), TYPES_SHPSEG (newtype),
                           TYPES_DIM (oldtype));
    }

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   node *APTarg(node *arg_node, node *arg_info)
 *
 * description:
 *   modify arg-node to padded name and shape
 *
 *****************************************************************************/

node *
APTarg (node *arg_node, node *arg_info)
{

    types *new_type;

    DBUG_ENTER ("APTarg");

    DBUG_EXECUTE ("APT",
                  if (ARG_NAME (arg_node) != NULL) {
                      DBUG_PRINT ("APT", ("check arg: %s", ARG_NAME (arg_node)));
                  } else { DBUG_PRINT ("APT", ("check arg: (NULL)")); });

    new_type = PIgetNewType (ARG_TYPE (arg_node));
    if (new_type != NULL) {
        /* found shape to be padded */
        ARG_TYPE (arg_node) = new_type;
        DBUG_PRINT ("APT", (" padding: %s -> %s__PAD", ARG_NAME (arg_node),
                            ARG_NAME (arg_node)));
        ARG_NAME (arg_node) = PadName (ARG_NAME (arg_node));
        ARG_PADDED (arg_node) = TRUE;
    } else {
        ARG_PADDED (arg_node) = FALSE;
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTvardec(node *arg_node, node *arg_info)
 *
 * description:
 *   duplicate vardec to reserve unpadded shape
 *   modify original vardec to fit new shape
 *
 *****************************************************************************/

node *
APTvardec (node *arg_node, node *arg_info)
{

    types *new_type;
    node *original_vardec;

    DBUG_ENTER ("APTvardec");

    DBUG_PRINT ("APT", ("check vardec: %s", VARDEC_NAME (arg_node)));

    original_vardec = DupNode (arg_node);

    new_type = PIgetNewType (VARDEC_TYPE (arg_node));
    if (new_type != NULL) {
        /* found shape to be padded */
        VARDEC_NEXT (original_vardec) = VARDEC_NEXT (arg_node);
        VARDEC_TYPE (arg_node) = new_type;
        DBUG_PRINT ("APT", (" padding: %s -> %s__PAD", VARDEC_NAME (arg_node),
                            VARDEC_NAME (arg_node)));

        VARDEC_NAME (arg_node) = PadName (VARDEC_NAME (arg_node));
        VARDEC_NEXT (arg_node) = original_vardec;

        VARDEC_PADDED (arg_node) = TRUE;

        if (VARDEC_NEXT (original_vardec) != NULL) {
            VARDEC_NEXT (original_vardec)
              = Trav (VARDEC_NEXT (original_vardec), arg_info);
        }

    } else {
        FreeVardec (original_vardec, arg_info);
        VARDEC_PADDED (arg_node) = FALSE;

        if (VARDEC_NEXT (arg_node) != NULL) {
            VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTassign(node *arg_node, node *arg_info)
 *
 * description:
 *   insert new assignments before itself, if necessarry
 *   traverse next assignment
 *
 *****************************************************************************/

node *
APTassign (node *arg_node, node *arg_info)
{

    node *tmp;
    node *new_assigns;
    node *new_node;

    DBUG_ENTER ("APTassign");

    DBUG_PRINT ("APT", ("assign-node detected"));

    INFO_APT_ASSIGNMENTS (arg_info) = NULL;

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "unexpected empty ASSIGN_INSTR");
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /* save new assigments */
    new_assigns = INFO_APT_ASSIGNMENTS (arg_info);

    /* traverse next assignment */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
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
 *   node *APTarray(node *arg_node, node *arg_info)
 *
 * description:
 *   constant arrays won't be padded
 *   =>array elements won't be traversed!
 *
 *****************************************************************************/

node *
APTarray (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("APTarray");

    DBUG_EXECUTE ("APT",
                  if (ARRAY_STRING (arg_node) != NULL) {
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
 *   node *APTwith(node *arg_node, node *arg_info)
 *
 * description:
 *   check withop-node first, then traverse code-nodes
 *
 *****************************************************************************/

node *
APTwith (node *arg_node, node *arg_info)
{

    node *save_ptr;

    DBUG_ENTER ("APTwith");

    DBUG_PRINT ("APT", ("with-node detected"));

    /* save pointer to outer with-loop to support nested loops */
    save_ptr = INFO_APT_WITH (arg_info);

    INFO_APT_WITH (arg_info) = arg_node;
    INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

    /* check withop, if with-loop needs to be padded */
    DBUG_ASSERT ((NWITH_WITHOP (arg_node) != NULL), " unexpected empty WITHOP!");
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    /* arg_info passes check result to code-nodes */
    DBUG_ASSERT ((NWITH_CODE (arg_node) != NULL), " unexpected empty CODE!");
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    /* no need to traverse part-nodes */

    /* EXPRESSION_PADDED is returned to upper function */

    /* restore pointer to outer with-loop */
    INFO_APT_WITH (arg_info) = save_ptr;

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTcode(node *arg_node, node *arg_info)
 *
 * description:
 *   apply padding to ids-attribute and traverse sons
 *
 *****************************************************************************/

node *
APTcode (node *arg_node, node *arg_info)
{

    bool rhs_padded;
    bool save_padded_state;

    DBUG_ENTER ("APTcode");

    DBUG_PRINT ("APT", ("code-node detected"));

    save_padded_state = INFO_APT_EXPRESSION_PADDED (arg_info);

    /* traverse code block */
    DBUG_ASSERT ((NCODE_CBLOCK (arg_node) != NULL), " unexpected empty CBLOCK!");
    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    rhs_padded = INFO_APT_EXPRESSION_PADDED (arg_info);

    /* traverse id-node (lvalue of assignment) */
    DBUG_ASSERT ((NCODE_CEXPR (arg_node) != NULL), " unexpected empty CEXPR!");
    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);

    /*
     * enable for consistency checking
     * requires shape information from pad_infer!
     * @@@ */
    DBUG_ASSERT (((!INFO_APT_EXPRESSION_PADDED (arg_info)) || rhs_padded),
                 " padding of lvalue does not match rvalue!");
    /* */

    /* traverse following code blocks (rvalue of assignment) */
    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    INFO_APT_EXPRESSION_PADDED (arg_info) = save_padded_state;

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTwithop(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APTwithop (node *arg_node, node *arg_info)
{

    shpseg *shape;
    int dim;
    int simpletype;
    types *oldtype = NULL;
    types *newtype = NULL;

    DBUG_ENTER ("APTwithop");

    DBUG_PRINT ("APT", ("withop-node detected"));

    INFO_APT_WITHOP_TYPE (arg_info) = NWITHOP_TYPE (arg_node);

    /* set default - change it, if padding can be applied */
    INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        DBUG_PRINT ("APT", (" genarray-loop"));

        shape = Array2Shpseg (NWITHOP_SHAPE (arg_node), NULL);
        /* constant array has dim=1
         * => number of elements is stored in shpseg[0]
         */
        dim = ARRAY_SHAPE ((NWITHOP_SHAPE (arg_node)), 0);
        /* all elements have the same type
         * => use simpletype of first code-node
         */
        simpletype = TYPES_BASETYPE (
          ID_TYPE (NCODE_CEXPR (NWITH_CODE (INFO_APT_WITH (arg_info)))));
        /* infer result of with-loop
           Attention: only elements with scalar types are supported yet !!!
        */
        oldtype = MakeTypes (simpletype, dim, shape, NULL, NULL);
        newtype = PIgetNewType (DupAllTypes (oldtype));

        if (newtype != NULL) {
            /* apply padding (genarray-specific)*/

            /* pad shape of new array specified in NWITHOP_SHAPE (pointing to array-node)
             */

            FreeNode (NWITHOP_SHAPE (arg_node));
            NWITHOP_SHAPE (arg_node)
              = Shpseg2Array (TYPES_SHPSEG (newtype), TYPES_DIM (newtype));

            INFO_APT_EXPRESSION_PADDED (arg_info) = TRUE;
        }

        break;

    case WO_modarray:
        DBUG_PRINT ("APT", (" modarray-loop"));

        if (ID_PADDED (NWITHOP_ARRAY (arg_node))) {
            /* apply padding (modarray-specific)*/

            /* attention: id-node already points to padded shape! */
            newtype = DupAllTypes (ID_TYPE (NWITHOP_ARRAY (arg_node)));
            oldtype = PIgetOldType (DupAllTypes (newtype));

            /* pad array referenced by NWITHOP_ARRAY (pointing to id-node)*/
            DBUG_ASSERT ((NWITHOP_ARRAY (arg_node) != NULL), " unexpected empty ARRAY!");
            NWITHOP_ARRAY (arg_node) = Trav (NWITHOP_ARRAY (arg_node), arg_info);

            INFO_APT_EXPRESSION_PADDED (arg_info) = TRUE;
        }

        break;

    case WO_foldfun:

        DBUG_PRINT ("APT", (" foldfun-loop"));

        /* check all sons for paddable code */
        NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        break;

    default:
        DBUG_ASSERT (FALSE, " unsupported withop-type");
        break;
    }

    /* apply WO_TYPE independend padding */
    if (INFO_APT_EXPRESSION_PADDED (arg_info)) {
        if (NWITH_PARTS (INFO_APT_WITH (arg_info)) > 0) {
            /* partition complete => add parts and code */
            InsertWithLoopGenerator (oldtype, newtype, INFO_APT_WITH (arg_info));
        }
    }

    /* free data structures */
    FreeOneTypes (oldtype);
    FreeOneTypes (newtype);

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTap(node *arg_node, node *arg_info)
 *
 * description:
 *   check, if function can be padded (has body)
 *   traverse arguments
 *
 *****************************************************************************/

node *
APTap (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("APTap");

    DBUG_PRINT ("APT", ("ap-node detected"));

    DBUG_EXECUTE ("APT",
                  if (AP_NAME (arg_node) != NULL) {
                      DBUG_PRINT ("APT", (" trav ap: %s", AP_NAME (arg_node)));
                  } else { DBUG_PRINT ("APT", (" trav ap: (NULL)")); });

    /* first inspect arguments */
    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
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
 *   node *APTid(node *arg_node, node *arg_info)
 *
 * description:
 *   rename node, if refering to padded vardec or arg
 *
 *****************************************************************************/

node *
APTid (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("APTid");

    DBUG_EXECUTE ("APT",
                  if (ID_NAME (arg_node) != NULL) {
                      DBUG_PRINT ("APT", ("check id: %s", ID_NAME (arg_node)));
                  } else { DBUG_PRINT ("APT", ("check id: (NULL)")); });

    /*
     * for this test ensure that args and vardecs have been
     * previously traversed!
     */
    if (ID_PADDED (arg_node)) {
        DBUG_PRINT ("APT", (" padding: '%s' -> '%s'", ID_NAME (arg_node),
                            ID_VARDEC_NAME (arg_node)));
        ID_NAME (arg_node) = PadName (ID_NAME (arg_node));
        INFO_APT_EXPRESSION_PADDED (arg_info) = TRUE;
    } else {
        /* if not padded, update pointer to vardec */
        DBUG_PRINT ("APT", (" leaving unpadded: '%s''", ID_NAME (arg_node)));
        if ((NODE_TYPE (ID_VARDEC (arg_node)) == N_vardec)
            && (VARDEC_PADDED (ID_VARDEC (arg_node)))) {
            ID_VARDEC (arg_node) = ID_VARDEC_NEXT (arg_node);
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
 *   node *APTprf(node *arg_node, node *arg_info)
 *
 * description:
 *   only some functions supported yet
 *
 *****************************************************************************/

node *
APTprf (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("APTprf");

    DBUG_PRINT ("APT", ("prf-node detected: '%s'", mdb_prf[PRF_PRF (arg_node)]));

    /* only some PRFs may be padded successfully (without conversion) */

    switch (PRF_PRF (arg_node)) {

    case F_sel:
        DBUG_ASSERT ((PRF_ARGS (arg_node) != NULL), " sel() has empty argument list!");
        /* traverse arguments to apply padding */
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);

        /* even if PRF_ARGS is padded, the result of PRF will have an
         * unpadded shape => return FALSE */
        INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

        break;

    case F_dim:
        DBUG_ASSERT ((PRF_ARGS (arg_node) != NULL), " dim() has empty argument list!");
        /* traverse arguments to apply padding */
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);

        /* even if PRF_ARGS is padded, the result of PRF will have an
         * unpadded shape => return FALSE */
        INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

        break;

    case F_shape:
        DBUG_ASSERT ((PRF_ARGS (arg_node) != NULL), " shape() has empty argument list!");
        /* check, if argument has paddable shape */

        if (ID_PADDED (PRF_ARG1 (arg_node))) {
            /* substitute paddable argument with reference to constant vector
               containing padded shape */
            types *old_type;

            old_type = PIgetOldType (
              DupAllTypes (VARDEC_TYPE (ID_VARDEC (PRF_ARG1 (arg_node)))));
            arg_node = Shpseg2Array (TYPES_SHPSEG (old_type), TYPES_DIM (old_type));
            old_type = Free (old_type);
        }
        /* even if PRF_ARG1 is padded, the result of PRF will have an
         * unpadded shape => return FALSE */
        INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

        break;

        /* accept PRFs for scalar arguments, but do no padding */
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

        /* only arguments of scalar type, so we do not need to traverse them */
        INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

        break;

        /* F_add_*, F_sub_*, F_mul* */
    case F_add_SxA:
    case F_add_AxS:
    case F_add_AxA:
    case F_sub_SxA:
    case F_sub_AxS:
    case F_sub_AxA:
    case F_mul_SxA:
    case F_mul_AxS:
    case F_mul_AxA:

        /* traverse arguments to apply padding */
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        /* APTprf will return padding-state of PRF_ARGS */
        break;

    default:

        /* results of unsupported functions have unpadded shape
         *
         * currently unsupported are:
         * F_take, F_drop, F_idx_sel, F_reshape, F_cat, F_rotate,
         * F_toi_A, F_tof_A, F_tod_A, F_div_SxA, F_div_AxS, F_div_AxA,
         * F_take_SxV, F_drop_SxV, F_cat_VxV,
         * F_modarray, F_idx_modarray, F_genarray
         */

        /* do not traverse sons */
        DBUG_PRINT ("APT", (" unsupported PRF '%s'!", mdb_prf[PRF_PRF (arg_node)]));

        INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

        break;
    }

    /* enable consistency checking in let-nodes, etc. !!! */

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse ARGS before BODY before NEXT
 *
 *****************************************************************************/

node *
APTfundef (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("APTfundef");

    DBUG_EXECUTE ("APT",
                  if (FUNDEF_NAME (arg_node) != NULL) {
                      DBUG_PRINT ("APT", (" trav fundef: %s", FUNDEF_NAME (arg_node)));
                  } else { DBUG_PRINT ("APT", (" trav fundef: (NULL)")); });

    INFO_APT_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    } else {
        DBUG_PRINT ("APT", (" no args"));
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    } else {
        DBUG_PRINT ("APT", (" no body"));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTblock(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse VARDEC before INSTR
 *
 *****************************************************************************/

node *
APTblock (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("APTblock");

    DBUG_PRINT ("APT", ("trav block"));

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    } else {
        DBUG_PRINT ("APT", (" no vardec"));
    }

    DBUG_ASSERT ((BLOCK_INSTR (arg_node) != NULL), "unexpected empty INSTR!");
    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTlet(node *arg_node, node *arg_info)
 *
 * description:
 *   apply padding to rvalues and lvalues if necessary
 *   ensure matching shape between rvalues and lvalues
 *
 *****************************************************************************/

node *
APTlet (node *arg_node, node *arg_info)
{

    ids *ids_ptr;
    bool rhs_padded;

    DBUG_ENTER ("APTlet");

    DBUG_PRINT ("APT", ("check let-node"));

    /* set FALSE as default, so separate functions for N_num, N_float,
       N_double, N_char, N_bool are not needed */
    INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "let-node without rvalues detected!");
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

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
        arg_info = PadIds (ids_ptr, arg_info);
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
