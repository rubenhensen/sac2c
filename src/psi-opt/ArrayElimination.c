/*
 *
 * $Log$
 * Revision 3.25  2004/11/25 18:16:56  jhb
 * on the way to compile
 *
 * Revision 3.24  2004/11/15 15:20:34  mwe
 * code for type upgrade added
 * use ntype-structure instead of types-structure
 * new code deactivated by MWE_NTYPE_READY
 *
 * Revision 3.23  2004/10/01 08:47:36  sah
 * removed some direct ast referenced and
 * added macro accesses instead
 *
 * Revision 3.22  2004/09/28 14:09:18  ktr
 * removed old refcount and generatemasks
 *
 * Revision 3.21  2004/07/19 14:19:38  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.20  2004/07/14 23:23:37  sah
 * removed all old ssa optimizations and the use_ssaform flag
 *
 * Revision 3.19  2004/07/14 14:22:41  sah
 * moved NodeBehindCast to tree_compund, thus
 * dependency to LoopInvariantRemoval.h removed
 *
 * Revision 3.18  2004/02/25 08:17:44  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 * NO while-loops may occur after flatten.
 * While-loop specific code eliminated.
 *
 * Revision 3.17  2003/06/11 21:52:05  ktr
 * Added support for multidimensional arrays.
 *
 * Revision 3.16  2002/02/21 13:43:00  dkr
 * access macros used
 *
 * Revision 3.15  2002/02/20 14:55:46  dkr
 * fundef DupTypes() renamed into DupAllTypes()
 *
 * Revision 3.14  2001/12/11 15:55:31  dkr
 * GetDim() renamed into GetShapeDim()
 *
 * Revision 3.13  2001/07/18 12:57:45  cg
 * Applications of old tree construction function
 * AppendNodeChain eliminated.
 *
 * Revision 3.12  2001/07/16 08:23:11  cg
 * Old tree construction function MakeNode eliminated.
 *
 * Revision 3.11  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.10  2001/05/17 13:40:26  nmw
 * MALLOC/FREE replaced by Malloc/Free, using result of Free()
 *
 * Revision 3.9  2001/05/15 08:03:59  nmw
 * remove call to OPTTrav when using ssa form
 *
 * Revision 3.8  2001/05/09 15:51:42  nmw
 * when using ssa form, ArrayElimination does not need masks anymore
 *
 * Revision 3.7  2001/05/02 06:56:25  nmw
 * init of constant arrays completed
 *
 * Revision 3.6  2001/04/30 12:16:59  nmw
 * integrate traversal of special fundefs in ArrayElimination traversal
 *
 * Revision 3.5  2001/04/18 13:42:30  dkr
 * bug in AEassign fixed: LHS might be void!
 *
 * [...]
 *
 */

#define NEW_INFO

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "optimize.h"

#include "ArrayElimination.h"

/*
 * INFO structure
 */
struct INFO {
    node *types;
    node *fundef;
};

/*
 * INFO macros
 */
#define INFO_AE_TYPES(n) (n->types)
#define INFO_AE_FUNDEF(n) (n->fundef)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_AE_TYPES (result) = NULL;
    INFO_AE_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

#define AE_PREFIX "__ae_"
#define AE_PREFIX_LENGTH 5

/*
 *
 *  functionname  : AEdoArrayElimination
 *  arguments     : 1) ptr to root of the syntaxtree or a N_fundef - node.
 *		    2) NULL
 *                  R) ptr optimized 1)
 *  description   : initiates array elimination for the intermediate sac-code:
 *  global vars   : syntax_tree, act_tab, ae_tab
 *
 */
node *
AEdoArrayElimination (node *arg_node)
{
    info *info;
#ifndef DBUG_OFF
    int mem_elim_arrays = elim_arrays;
#endif

    DBUG_ENTER ("AEdoArrayElimination");
    DBUG_PRINT ("OPT", ("ARRAY ELIMINATION"));

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "AEdoArrayElimination() called for non-fundef node");

    DBUG_PRINT ("OPT",
                ("starting array elimination on function %s", FUNDEF_NAME (arg_node)));

    if (!(FUNDEF_ISLACFUN (arg_node))) {
        /*tmp_tab = act_tab;
          act_tab=ae_tab;*/
        info = MakeInfo ();
        TRAVpush (TR_ae);
        arg_node = TRAVdo (arg_node, info);
        TRAVpop ();
        info = FreeInfo (info);
        /*act_tab=tmp_tab;*/
        DBUG_PRINT ("OPT", ("                        result: %d",
                            elim_arrays - mem_elim_arrays));
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CorrectArraySize
 *  arguments     :
 *  description   :
 *
 */
int
CorrectArraySize (node *ids_node)
{
    int answer = FALSE;
    int length, dim;
#ifdef MWE_NTYPE_READY
    ntype *type;
#else
    types *type;
#endif

    DBUG_ENTER ("CorrectArraySize");

#ifdef MWE_NTYPE_READY
    type = AVIS_TYPE (IDS_AVIS (ids_node));
    length = SHgetUnrLen (TYgetShape (type));
    dim = TYgetDim (type);
#else
    type = IDS_TYPE (ids_node);
    length = TCgetTypesLength (type);
    dim = TCgetShapeDim (type);
#endif

    if ((length <= global.minarray) && (0 != length) && (1 == dim)) {
        DBUG_PRINT ("AE", ("array %s with length %d to eliminated found",
                           VARDEC_NAME (IDS_VARDEC (ids_node)), length));
        answer = TRUE;
    }
    DBUG_RETURN (answer);
}

/*
 *
 *  functionname  : GetNumber
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
char *
GetNumber (node *vector)
{
    char *number, *tmp;
    node *expr_node;

    DBUG_ENTER ("GetNumber");
    number = (char *)ILIBmalloc (sizeof (char) * ((global.minarray / 10) + 2));
    number[0] = atoi ("\0");
    expr_node = vector->node[0];
    do {
        tmp = ILIBitoa (NUM_VAL (EXPRS_EXPR (expr_node)));
        strcat (number, tmp);
        expr_node = EXPRS_NEXT (expr_node);

        /* srs: we already have assured that the shape has only one element
                (CorrectArraySize). Else we allocated to little memory
                for 'number'. */
        DBUG_ASSERT (expr_node == NULL, ("to many arguments in GetNumber"));

    } while (expr_node);
    DBUG_RETURN (number);
}

/*
 *
 *  functionname  : GenIds
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
GenIds (node *arg[2])
{
    char *number, *new_name, *old_name;

    DBUG_ENTER ("GenIds");
    number = GetNumber (arg[0]);
    old_name = ID_NAME (arg[1]);
    new_name = (char *)ILIBmalloc (
      sizeof (char) * (strlen (old_name) + strlen (number) + AE_PREFIX_LENGTH + 1));
    sprintf (new_name, AE_PREFIX "%s%s", number, old_name);
    DBUG_RETURN (TBmakeIds (new_name, NULL, ST_regular));
}

/*
 *
 *  functionname  : GenSel
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
GenSel (node *ids_node, info *arg_info)
{
    node *new_nodes = NULL, *new_let, *new_assign, *new_vardec;
    node *exprn;
    int length, i;
#ifdef MWE_NTYPE_READY
    ntype *type;
#else
    types *type;
#endif
    node *arg[2];

    DBUG_ENTER ("GenSel");
#ifdef MWE_NTYPE_READY
    type = AVIS_TYPE (IDS_AVIS (ids_node));
    length = SHgetUnrLen (TYgetShape (type));
#else
    type = IDS_TYPE (ids_node);
    ;
    length = TCgetTypesLength (type);
#endif
    for (i = 0; i < length; i++) {
        exprn = TBmakeExprs (TBmakeNum (i), NULL);
        arg[0] = TCmakeFlatArray (exprn);
        ((int *)ARRAY_CONSTVEC (arg[0])) = TCarray2IntVec (exprn, NULL);
        /* srs: AE only works on arrays which have 1 dimension.
           type attribut was missing here. */
        ARRAY_ISCONST (arg[0]) = TRUE;
        ARRAY_VECTYPE (arg[0]) = T_int;
        ARRAY_VECLEN (arg[0]) = 1;
#ifdef MWE_NTYPE_READY
        ARRAY_NTYPE (arg[0]) = TYmakeAKV (TYmakeSimpleType (T_int),
                                          COmakeConstant (T_int, SHmakeShape (1, dim),
                                                          TCarray2IntVec (exprn, NULL)));
#else
        ARRAY_TYPE (arg[0])
          = TBmakeTypes (T_int, 1, TBmakeShpseg (TBmakeNums (1, NULL)), NULL, NULL);
#endif
#if 0    
    arg[1] = MakeNode(N_id);
    arg[1]->info.ids = DUPdupAllIds( ids_node);
#else
        arg[1] = TBmakeId (ILIBstringCopy (IDS_NAME (ids_node)), NULL, ST_regular);
        ID_VARDEC (arg[1]) = IDS_VARDEC (ids_node);
        ID_AVIS (arg[1]) = VARDEC_OR_ARG_AVIS (ID_VARDEC (arg[1]));
#endif
        new_let = TBmakeLet (GenIds (arg), NULL);

        DBUG_PRINT ("AE", ("Generating new value for %s", IDS_NAME (LET_IDS (new_let))));
        IDS_VARDEC (LET_IDS (new_let))
          = TCsearchDecl (IDS_NAME (LET_IDS (new_let)), INFO_AE_TYPES (arg_info));

        if (IDS_VARDEC (LET_IDS (new_let)) == NULL) {
            DBUG_PRINT ("AE",
                        ("Generating new vardec for %s", IDS_NAME (LET_IDS (new_let))));
#ifdef MWE_NTYPE_READY
            new_vardec = TBakeVardec (ILIBstringCopy (IDS_NAME (LET_IDS (new_let))), NULL,
                                      TYmakeSimpleType (TYgetBasetype (type)), NULL);

#else
            new_vardec = TBmakeVardec (ILIBstringCopy (IDS_NAME (LET_IDS (new_let))),
                                       DUPdupAllTypes (TCgetTypes (type)), NULL);
            VARDEC_DIM (new_vardec) = 0;
#endif
            INFO_AE_TYPES (arg_info)
              = TCappendVardec (new_vardec, INFO_AE_TYPES (arg_info));
            IDS_VARDEC (LET_IDS (new_let)) = new_vardec;
        }
        LET_EXPR (new_let)
          = TBmakePrf (F_sel, TBmakeExprs (arg[0], TBmakeExprs (arg[1], NULL)));
        new_assign = TBmakeAssign (new_let, NULL);
        new_nodes = TCappendAssign (new_nodes, new_assign);
    }

    DBUG_RETURN (new_nodes);
}

/*
 *
 *  functionname  : AEprf
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
AEprf (node *arg_node, info *arg_info)
{
    node *arg[2], *new_node, *tmpn;

    DBUG_ENTER ("AEprf");

    if (F_sel == PRF_PRF (arg_node)) {
        tmpn = TCnodeBehindCast (PRF_ARG1 (arg_node));
        if (N_id == NODE_TYPE (tmpn)) {
            /* look up via ssa assign attribute */
            if (AVIS_SSAASSIGN (ID_AVIS (tmpn)) != NULL) {
                DBUG_PRINT ("AE", ("defining assignment looked up via SSAASSIGN"));
                tmpn = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (tmpn)));
            } else {
                tmpn = NULL;
            }
        }

        arg[0] = tmpn;
        arg[1] = TCnodeBehindCast (PRF_ARG2 (arg_node));

        /* srs: added IsConstArray() so that sel([i],arr) is not replaced.
                This led to wrong programs. */
        if (N_id == NODE_TYPE (arg[1]) && tmpn && N_array == NODE_TYPE (tmpn)
            && TCisConstArray (tmpn) && CorrectArraySize (ID_IDS (arg[1]))) {
            DBUG_PRINT ("AE", ("sel function with array %s to eliminated found",
                               IDS_NAME (ID_IDS (arg[0]))));
            new_node = TBmakeId (NULL, NULL, ST_regular);
            ID_IDS (new_node) = GenIds (arg);
            ID_VARDEC (new_node)
              = TCsearchDecl (ID_NAME (new_node), INFO_AE_TYPES (arg_info));
            if (ID_VARDEC (new_node)) {
                FREEdoFreeTree (arg_node);
                arg_node = new_node;
            } else {
                FREEdoFreeTree (new_node);
            }
        }
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AEfundef(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
AEfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AEfundef");

    INFO_AE_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) && (!FUNDEF_INLINE (arg_node))) {
        DBUG_PRINT ("AE", ("*** Trav function %s", FUNDEF_NAME (arg_node)));

        INFO_AE_TYPES (arg_info) = NULL;
        FUNDEF_INSTR (arg_node) = TRAVdo (FUNDEF_INSTR (arg_node), arg_info);
        FUNDEF_VARDEC (arg_node)
          = TCappendVardec (INFO_AE_TYPES (arg_info), FUNDEF_VARDEC (arg_node));
        INFO_AE_TYPES (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AEassign(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
AEassign (node *arg_node, info *arg_info)
{
    node *new_nodes = NULL;
    node *_ids;

    DBUG_ENTER ("AEassign");

    /* create new assignments if array is smaler than threshold */
    if (N_let == NODE_TYPE (ASSIGN_INSTR (arg_node))) {
        _ids = LET_IDS (ASSIGN_INSTR (arg_node));
        if ((_ids != NULL) && (IDS_NEXT (_ids) == NULL)) {
            if (CorrectArraySize (_ids)) {
                VARDEC_FLAG (IDS_VARDEC (_ids)) = TRUE;
                new_nodes = GenSel (_ids, arg_info);
            } else
                VARDEC_FLAG (IDS_VARDEC (_ids)) = FALSE;
        }
    }

    /* traverse into compound nodes. */
    if (ASSIGN_INSTR (arg_node))
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    if (ASSIGN_NEXT (arg_node))
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);

    if (new_nodes)
        ASSIGN_NEXT (arg_node) = TCappendAssign (new_nodes, ASSIGN_NEXT (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AEcond(node *arg_node, info *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
AEcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AEcond");

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THENINSTR (arg_node) = TRAVdo (COND_THENINSTR (arg_node), arg_info);
    COND_ELSEINSTR (arg_node) = TRAVdo (COND_ELSEINSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AEdo(node *arg_node, info *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
AEdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AEdo");

    DO_INSTR (arg_node) = TRAVdo (DO_INSTR (arg_node), arg_info);
    DO_COND (arg_node) = TRAVdo (DO_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AENwith(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
AEwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AEwith");

    /* The Phase AE is not in the optimization loop and though
       WLT has not been done. Only one N_Npart and one N_Ncode exist. */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AEap(node *arg_node, info *arg_info)
 *
 * description:
 *   starts traversal of implicit inlined special fundef
 *
 ******************************************************************************/

node *
AEap (node *arg_node, info *arg_info)
{
    info *new_arg_info;

    DBUG_ENTER ("AEap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    /* non-recursive call of special fundef */
    if ((AP_FUNDEF (arg_node) != NULL) && (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (INFO_AE_FUNDEF (arg_info) != AP_FUNDEF (arg_node))) {

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_arg_info);

        new_arg_info = FreeInfo (new_arg_info);
    }
    DBUG_RETURN (arg_node);
}
