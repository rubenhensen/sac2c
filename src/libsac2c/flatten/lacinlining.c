/* $Id$ */

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "new_types.h"
#include "str.h"
#include "memory.h"
#include "prepare_inlining.h"
#include "ctinfo.h"
#include "DupTree.h"

#include "lacinlining.h"

/*
 * INFO structure
 */
struct INFO {
    bool onefundef;
    node *fundef;
    node *letids;
    node *code;
    node *vardecs;
};

/*
 * INFO macros
 */
#define INFO_ONEFUNDEF(n) (n->onefundef)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LETIDS(n) (n->letids)
#define INFO_CODE(n) (n->code)
#define INFO_VARDECS(n) (n->vardecs)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_CODE (result) = NULL;
    INFO_VARDECS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * Function:
 *   void AdaptConcreteArgs( node *conc_arg, node *form_arg, node *fundef)
 *
 * Description:
 *   Where possible, the type of the concrete argument is downgraded
 *   to the type of the formal argument. If that is not possible,
 *   a type conversion is inserted.
 *
 ******************************************************************************/
void
AdaptConcreteArgs (node *conc_arg, node *form_arg, node *fundef)
{
    ntype *ftype, *ctype;
    node *newavis, *oldavis;
#ifndef DBUG_OFF
    char *tmp_str;
    char *tmp_str2;
#endif

    DBUG_ENTER ("AdaptConcreteArgs");

    if (conc_arg != NULL) {
        DBUG_ASSERT (NODE_TYPE (conc_arg) == N_exprs,
                     "Concrete function arguments must be N_exprs");
        DBUG_ASSERT (form_arg != NULL,
                     "No correspondence between formal and concrete arguments");
        DBUG_ASSERT (NODE_TYPE (form_arg) == N_arg,
                     "Formal function arguments must be N_arg");

        AdaptConcreteArgs (EXPRS_NEXT (conc_arg), ARG_NEXT (form_arg), fundef);

        DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (conc_arg)) == N_id,
                     "Concrete function argument must be N_id");

        ftype = AVIS_TYPE (ARG_AVIS (form_arg));
        ctype = AVIS_TYPE (ID_AVIS (EXPRS_EXPR (conc_arg)));

        if (!TYeqTypes (ftype, ctype)) {
            /*
             * We have two situations here:
             *
             * 1) type of concrete arg < type of formal arg
             *
             *    in this case we want to insert an assignment so
             *    that the backend can compile in representational
             *    updates (if necessary).
             *
             * 2) type of concrete arg > type of formal arg
             *
             *    in this case we insert a typeconv prf to
             *    make sure that the concrete arg during runtime has
             *    the type of the formal arg.
             */

            DBUG_EXECUTE ("LINL", tmp_str = TYtype2String (ftype, 0, 0);
                          tmp_str2 = TYtype2String (ctype, 0, 0););
            DBUG_PRINT ("LINL", ("  >> trying to adapt %s to %s", tmp_str2, tmp_str));
            DBUG_EXECUTE ("LINL", tmp_str = MEMfree (tmp_str);
                          tmp_str2 = MEMfree (tmp_str2););

#if 0
      /* 
       * The simplified coercion below seems to be like a good idea at first
       * glance, but in fact it is not: The AKV variable in the calling context
       * may have many uses, e.g. it could even be used in another argument
       * position in the same LaC function. Downgrading the type eliminates AKV
       * knowledge in all these applied occurrences of the variable as well,
       * which is hardly what we want. So, in fact the general case below is
       * the better choice.
       */
      if ( TYisAKS( ftype) && TYisAKV( ctype)) {
        /*
         * 1a) special case where we can just downgrade the
         *     type as there is no representational difference
         *     at all.
         */
        DBUG_PRINT( "LINL", ("    >> downgrade AKV -> AKS"));

        AVIS_TYPE( ID_AVIS( EXPRS_EXPR( conc_arg))) =
          TYfreeType( AVIS_TYPE( ID_AVIS( EXPRS_EXPR( conc_arg))));
        AVIS_TYPE( ID_AVIS( EXPRS_EXPR( conc_arg))) = 
          TYcopyType( ftype);
      }
      else
#endif
            if (TYleTypes (ctype, ftype)) {
                /*
                 * 1b) type of concrete arg < type of formal arg
                 */
                DBUG_PRINT ("LINL", ("    >> insert assignment to downgrade type"));

                newavis
                  = TBmakeAvis (TRAVtmpVarName (ARG_NAME (form_arg)), TYcopyType (ctype));

                FUNDEF_INSTR (fundef)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (ARG_AVIS (form_arg), NULL),
                                             TBmakeId (newavis)),
                                  FUNDEF_INSTR (fundef));

                if (global.valid_ssaform) {
                    AVIS_SSAASSIGN (newavis) = FUNDEF_INSTR (fundef);
                }

                FUNDEF_VARDEC (fundef)
                  = TBmakeVardec (ARG_AVIS (form_arg), FUNDEF_VARDEC (fundef));
                if (NULL != AVIS_DIM (ARG_AVIS (form_arg))) {
                    AVIS_DIM (newavis) = DUPdoDupTree (AVIS_DIM (ARG_AVIS (form_arg)));
                }
                if (NULL != AVIS_SHAPE (ARG_AVIS (form_arg))) {
                    AVIS_SHAPE (newavis)
                      = DUPdoDupTree (AVIS_SHAPE (ARG_AVIS (form_arg)));
                }

                ARG_AVIS (form_arg) = newavis;
            } else {
                /*
                 * 2) type of concrete arg > type of formal arg
                 */
                DBUG_PRINT ("LINL", ("    >> insert typeconv to upgrade type"));

                newavis
                  = TBmakeAvis (TRAVtmpVarName (ARG_NAME (form_arg)), TYcopyType (ctype));

                oldavis = ARG_AVIS (form_arg);

                FUNDEF_INSTR (fundef)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (oldavis, NULL),
                                             TCmakePrf2 (F_type_conv,
                                                         TBmakeType (TYcopyType (ftype)),
                                                         TBmakeId (newavis))),
                                  FUNDEF_INSTR (fundef));

                if (global.valid_ssaform) {
                    AVIS_SSAASSIGN (oldavis) = FUNDEF_INSTR (fundef);
                }

                FUNDEF_VARDEC (fundef) = TBmakeVardec (oldavis, FUNDEF_VARDEC (fundef));

                if (NULL != AVIS_DIM (oldavis)) {
                    AVIS_DIM (newavis) = DUPdoDupTree (AVIS_DIM (oldavis));
                }
                if (NULL != AVIS_SHAPE (oldavis)) {
                    AVIS_SHAPE (newavis) = DUPdoDupTree (AVIS_SHAPE (oldavis));
                }

                ARG_AVIS (form_arg) = newavis;
            }
        }

    } else {
        DBUG_ASSERT (form_arg == NULL,
                     "No correspondence between formal and concrete arguments");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   node *LINLmodule( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses fundefs only.
 *
 ******************************************************************************/
node *
LINLmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LINLmodule");

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *LINLfundef( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses function bodies
 *
 ******************************************************************************/
node *
LINLfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ("LINLfundef");

    DBUG_PRINT ("LINL", ("lacinlining in %s", CTIitemName (arg_node)));

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
    }
    if (FUNDEF_LOCALFUNS (arg_node) != NULL) {
        /**
         * we are in glf mode. Since we want to handle all lacinlines in the
         * chain, we need to temporarily set INFO_ONEFUNDEF to false.
         * Although glf mode today (5/2009) implies
         * that INFO_ONEFUNDEF is true, this may not hold in the future.
         * Hence, we stack that value before setting it to FALSE.
         */
        old_onefundef = INFO_ONEFUNDEF (arg_info);
        INFO_ONEFUNDEF (arg_info) = FALSE;
        FUNDEF_LOCALFUNS (arg_node) = TRAVdo (FUNDEF_LOCALFUNS (arg_node), arg_info);
        INFO_ONEFUNDEF (arg_info) = old_onefundef;
    }

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *LINLassign( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses RHS and replaces arg_node with inlined code if necessary
 *
 ******************************************************************************/
node *
LINLassign (node *arg_node, info *arg_info)
{
    bool inlined = FALSE;

    DBUG_ENTER ("LINLassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_CODE (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_CODE (arg_info), ASSIGN_NEXT (arg_node));

        inlined = TRUE;
        INFO_CODE (arg_info) = NULL;
        global.optcounters.inl_fun++; /* global optimization counter */

        if (INFO_VARDECS (arg_info) != NULL) {
            BLOCK_VARDEC (FUNDEF_BODY (INFO_FUNDEF (arg_info)))
              = TCappendVardec (INFO_VARDECS (arg_info),
                                BLOCK_VARDEC (FUNDEF_BODY (INFO_FUNDEF (arg_info))));
            INFO_VARDECS (arg_info) = NULL;
        }
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    if (inlined) {
        /*
         * you won't believe it, but this statement as well does
         * remove the lacfunction which it points to. This funny
         * sideeffect is not documented, but may be found in
         * free_attribs.c (FREEattribsExtLink)
         * This also implicitly returns ASSIGN_NEXT which enables
         * us to update arg_node apropriately by:
         */

        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *LINLlet( node *arg_node, info *arg_info)
 *
 * Description:
 *   Remembers LHS in INFO node and traverses RHS
 *
 ******************************************************************************/
node *
LINLlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LINLlet");

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LETIDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *LINLap( node *arg_node, info *arg_info)
 *
 * Description:
 *   Prepares inlining of applied former special functions
 *
 ******************************************************************************/
node *
LINLap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LINLap");

    if (FUNDEF_ISLACINLINE (AP_FUNDEF (arg_node))) {

        DBUG_PRINT ("LINL", (">> processing application of %s",
                             CTIitemName (AP_FUNDEF (arg_node))));

        /*
         * Adapt types of the concrete loop/cond arguments to meet the
         * types of the potentially reassigned formal arguments.
         */
        AdaptConcreteArgs (AP_ARGS (arg_node), FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                           AP_FUNDEF (arg_node));

        INFO_CODE (arg_info)
          = PINLdoPrepareInlining (&INFO_VARDECS (arg_info), AP_FUNDEF (arg_node),
                                   INFO_LETIDS (arg_info), AP_ARGS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *LINLdoLACInlining( node *arg_node)
 *
 * Description:
 *   Starts function inlining of former loop and conditional functions.
 *
 ******************************************************************************/

node *
LINLdoLACInlining (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("LINLdoLACInlining");

#ifdef SHOW_MALLOC
    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));
#endif

    arg_info = MakeInfo ();

    TRAVpush (TR_linl);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    FreeInfo (arg_info);

#ifdef SHOW_MALLOC
    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));
#endif

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *LINLdoLACInliningOneFundef( node *arg_node)
 *
 * Description:
 *   Starts function inlining of former loop and conditional functions in one
 *   fundef.
 *
 ******************************************************************************/
node *
LINLdoLACInliningOneFundef (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("LINLdoLACInliningOneFundef");

#ifdef SHOW_MALLOC
    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));
#endif

    arg_info = MakeInfo ();
    INFO_ONEFUNDEF (arg_info) = TRUE;

    TRAVpush (TR_linl);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    FreeInfo (arg_info);

#ifdef SHOW_MALLOC
    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));
#endif

    DBUG_RETURN (arg_node);
}
