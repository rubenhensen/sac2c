/*
 * $Log$
 * Revision 1.8  2000/06/30 15:22:58  mab
 * started implementing APTwithop
 * implemented dummy code generation for with loop
 *
 * Revision 1.7  2000/06/29 10:23:38  mab
 * added dummy functions for APTpart, APTwithid, APTgenerator, APTcode, APTwithop
 * renamed APTNwith to APTwith
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
 *   This compiler module appplies array padding.
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

    FREE (arg_info);

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

    padded_name = (char *)MALLOC (strlen (unpadded_name) + 6 * sizeof (char));
    strcpy (padded_name, unpadded_name);
    strcat (padded_name, "__PAD");

    FREE (unpadded_name);

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
        if (NODE_TYPE (IDS_VARDEC (arg)) == N_vardec) {
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
 *   static node* AddDummyCode(node* with_node)
 *
 * description:
 *   add dummy assignment (_apt_#=0;) as first code block to with-node
 *
 *****************************************************************************/

node *
AddDummyCode (node *with_node)
{

    node *vardec_node;
    types *newvardec_type;

    node *expr_node;
    ids *ids_attrib;
    node *instr_node;
    node *assign_node;
    node *block_node;
    node *id_node;
    node *code_node;

    DBUG_ENTER ("AddDummyCode");

    /* find last vardec */
    vardec_node = ID_VARDEC (NCODE_CEXPR (NWITH_CODE (with_node)));
    while (VARDEC_NEXT (vardec_node) != NULL) {
        vardec_node = VARDEC_NEXT (vardec_node);
    }

    /* append new vardec-node */
    DBUG_ASSERT ((TYPES_NEXT (ID_TYPE (NCODE_CEXPR (NWITH_CODE (with_node)))) == NULL),
                 "single type expected");
    newvardec_type = DupTypes (ID_TYPE (NCODE_CEXPR (NWITH_CODE (with_node))));
    TYPES_NAME (newvardec_type) = TmpVar ();
    VARDEC_NEXT (vardec_node)
      = MakeVardec (TYPES_NAME (newvardec_type), newvardec_type, NULL);
    vardec_node = VARDEC_NEXT (vardec_node);

    /* add dummy code */
    expr_node = MakeNum (0);
    ids_attrib = MakeIds (StringCopy (VARDEC_NAME (vardec_node)), NULL, ST_regular);
    IDS_VARDEC (ids_attrib) = vardec_node;
    instr_node = MakeLet (expr_node, ids_attrib);
    assign_node = MakeAssign (instr_node, NULL);
    block_node = MakeBlock (assign_node, NULL);
    id_node = MakeId (StringCopy (VARDEC_NAME (vardec_node)), NULL, ST_regular);
    ID_VARDEC (id_node) = vardec_node;
    code_node = MakeNCode (block_node, id_node);

    DBUG_RETURN (vardec_node);
}

/*****************************************************************************
 *
 * function:
 *   static void InsertWithLoopGenerator(types* oldtype, types* newtype, node* with_node)
 *
 * description:
 *   insert new part-nodes and code-node into withloop to apply padding
 *
 *****************************************************************************/

static void
InsertWithLoopGenerator (types *oldtype, types *newtype, node *with_node)
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
        assignment_vardec = AddDummyCode (with_node);

        /* add nodes to part */

        /* @@@ CONTINUE (insert new PART-nodes) */
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
        VARDEC_NEXT (original_vardec) = NULL;
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

    /* constant arrays arn't padded ! */
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
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APTwith (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("APTwith");

    DBUG_PRINT ("APT", ("with-node detected"));

    INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;
    INFO_APT_WITH (arg_info) = arg_node;

    /* check withop, if with-loop needs to be padded */
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    /* arg_info passes check result to part- and code-nodes */
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    /* EXPRESSION_PADDED is returned to upper function */

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTpart(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APTpart (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("APTpart");

    DBUG_PRINT ("APT", ("part-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTwithid(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APTwithid (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("APTwithid");

    DBUG_PRINT ("APT", ("withid-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTgenerator(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APTgenerator (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("APTgenerator");

    DBUG_PRINT ("APT", ("generator-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTcode(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APTcode (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("APTcode");

    DBUG_PRINT ("APT", ("code-node detected"));

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
    types *oldtype;
    types *newtype = NULL;

    DBUG_ENTER ("APTwithop");

    DBUG_PRINT ("APT", ("withop-node detected"));

    INFO_APT_WITHOP_TYPE (arg_info) = NWITHOP_TYPE (arg_node);

    /* set default - change it, if padding can be applied */
    INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        shape = Array2Shpseg (NWITHOP_SHAPE (arg_node));
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
        oldtype = MakeType (simpletype, dim, shape, NULL, NULL);
        newtype = PIgetNewType (oldtype);

        if (newtype != NULL) {
            /* apply padding (genarray-specific)*/
            INFO_APT_EXPRESSION_PADDED (arg_info) = TRUE;

            /* @@@ CONTINUE (free newtype? !oldshape is used later!)*/
            /* pad shape of new array specified in NWITHOP_SHAPE */
        } else {
            /* do not apply padding */
            FreeOneTypes (oldtype);
        }
        break;

    case WO_modarray:

        oldtype = ID_TYPE (NWITHOP_ARRAY (arg_node));
        newtype = PIgetNewType (oldtype);

        if (newtype != NULL) {
            /* apply padding (modarray-specific)*/
            INFO_APT_EXPRESSION_PADDED (arg_info) = TRUE;

            /* @@@ CONTINUE (free newtype? !oldshape is used later!)*/
            /* pad array referenced by NWITHOP_ARRAY */
        }

        break;

    default:
        DBUG_ASSERT (FALSE, "unsupported withop-type");
        break;
    }

    /* apply WO_TYPE independend padding */
    if (INFO_APT_EXPRESSION_PADDED (arg_info)) {
        if (NWITH_PARTS (INFO_APT_WITH (arg_info)) > 0) {
            /* partition complete => add parts and code */
            InsertWithLoopGenerator (oldtype, newtype, INFO_APT_WITH (arg_info));
        }
    }

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
 *   node *APTexprs(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APTexprs (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("APTexprs");

    DBUG_PRINT ("APT", ("exprs-node detected"));

    EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);
    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);
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

    /* (for this test ensure that args and vardecs have been traversed previously!) */
    if (ID_PADDED (arg_node)) {
        DBUG_PRINT ("APT", (" padding: '%s' -> '%s'", ID_NAME (arg_node),
                            ID_VARDEC_NAME (arg_node)));
        ID_NAME (arg_node) = PadName (ID_NAME (arg_node));
        INFO_APT_EXPRESSION_PADDED (arg_info) = TRUE;
    } else {
        /* if not padded, update pointer to vardec */
        DBUG_PRINT ("APT", (" leaving unpadded: '%s''", ID_NAME (arg_node)));
        if (NODE_TYPE (ID_VARDEC (arg_node)) == N_vardec) {
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
 *   not really supported yet => PRFs return unpadded results only!
 *
 *****************************************************************************/

node *
APTprf (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("APTprf");

    DBUG_PRINT ("APT", ("prf-node detected"));

    /* no support for primitive functions yet
     * => every argument of a prf has to be in unpadded shape!!!
     * some exceptions may be handled later */

    /* therefore every result will also have an unpadded shape */
    INFO_APT_EXPRESSION_PADDED (arg_info) = FALSE;

    /* do not traverse sons */

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

    DBUG_ASSERT ((BLOCK_INSTR (arg_node) != NULL),
                 " block without instructions found!!!");
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

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), " let-node without rvalues detected!");
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    rhs_padded = INFO_APT_EXPRESSION_PADDED (arg_info);

    /* rhs_padded==TRUE
     * => right hand side can possibly be padded, so it may be an ap
     *    refering to fundef with body or a variable with padded shape
     * rhs_padded==FALSE
     * right hand side may be an ap refering to fundef without a body,
     * prf, array, constant (num, ...) or a variable with unpadded shape */

    /* manually traverse ids */
    ids_ptr = LET_IDS (arg_node);
    while (ids_ptr != NULL) {
        arg_info = PadIds (ids_ptr, arg_info);
        ids_ptr = IDS_NEXT (ids_ptr);
    }

    /*
     * enable for consistency checking
     * requires shape information from pad_infer!
     *
     * DBUG_ASSERT((!rhs_padded&&INFO_APT_EXPRESSION_PADDED(arg_info))," padded lvalue
     * does not match unpadded rvalue!");
     */

    DBUG_RETURN (arg_node);
}
