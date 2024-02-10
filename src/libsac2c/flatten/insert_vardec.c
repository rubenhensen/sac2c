/*****************************************************************************
 *
 * file:   insert_vardec.c
 *
 * prefix: INSVD
 *
 * description:
 *
 *   This compiler module inserts vardecs for all identifyers that do not have
 *   one yet and it replaces all N_spids by N_ids, and all N_spid by either
 *   N_id or N_globobj.
 *
 *   All this is needed for running InferDFMS prior to type checking.
 *
 *
 *   Implementation:
 *   While traversing top-down, it introduces new N_avis / N_vardec for every
 *   new name encountered in N_spids.
 *   N_spid nodes are transformed into N_id or N_globobj nodes when being met.
 *
 *
 *
 * usage of arg_info (INFO_...):
 *
 *   ...OBJDEFS  holds the pointer to the objdefs chain of the actual module !
 *   ...VARDECS  holds the pointer to the vardec chain of the actual fundef !
 *   ...ARGS     holds the pointer to the argument chain of the actual fundef !
 *   ...VARDEC_LUT Lookup table corresponding to VARDECS
 *   ...ARG_LUT Lookup table corresponding to ARGS
 *
 *****************************************************************************/

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "new_types.h"
#include "str.h"
#include "memory.h"
#include "LookUpTable.h"
#include "traverse.h"
#include "globals.h"
#include "free.h"
#include "ctinfo.h"

#define DBUG_PREFIX "IVD"
#include "debug.h"

#include "DupTree.h"
#include "namespaces.h"

#include "insert_vardec.h"

/*
 * INFO structure
 */
struct INFO {
    node *vardecs;
    node *args;
    node *objdefs;
    lut_t *vardec_lut;
    lut_t *arg_lut;
    node *module;
};

/*
 * INFO macros
 */
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_ARGS(n) ((n)->args)
#define INFO_OBJDEFS(n) ((n)->objdefs)
#define INFO_MODULE(n) ((n)->module)
#define INFO_VARDEC_LUT(n) ((n)->vardec_lut)
#define INFO_ARG_LUT(n) ((n)->arg_lut)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_VARDECS (result) = NULL;
    INFO_ARGS (result) = NULL;
    INFO_OBJDEFS (result) = NULL;
    INFO_VARDEC_LUT (result) = LUTgenerateLut ();
    INFO_ARG_LUT (result) = LUTgenerateLut ();

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    LUTremoveLut (INFO_VARDEC_LUT (info));
    LUTremoveLut (INFO_ARG_LUT (info));

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *  lut_t * InitVardecLUT (node *vardecs)
 *
 * description:
 *   Allocates and fills a look up table containing [vardec_name, vardec].
 *
 ******************************************************************************/
static void
InitVardecLUT (node *vardecs, lut_t **lut /* inout */)
{
    DBUG_ENTER ();

    while (vardecs != NULL) {
        *lut = LUTinsertIntoLutS (*lut, VARDEC_NAME (vardecs), vardecs);
        vardecs = VARDEC_NEXT (vardecs);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *  lut_t * InitArgLUT (node *args)
 *
 * description:
 *   Allocates and fills a look up table containing [arg_name, arg].
 *
 ******************************************************************************/
static void
InitArgLUT (node *args, lut_t **lut /* inout */)
{
    DBUG_ENTER ();

    while (args != NULL) {
        *lut = LUTinsertIntoLutS (*lut, ARG_NAME (args), args);
        args = ARG_NEXT (args);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *  node * SearchForName (char *name, lut_t *lut)
 *
 * description:
 *   looks up the name in the LUT given. Returns the address of the
 *   corresponding node if found, NULL otherwise.
 *
 ******************************************************************************/
static node *
SearchForName (char *name, lut_t *lut)
{
    void **res_ptr;
    node *res;

    DBUG_ENTER ();

    res_ptr = LUTsearchInLutS (lut, name);
    res = (res_ptr == NULL) ? NULL : *res_ptr;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *  node * SearchForNameInObjs( char *mod, char *name, node *args)
 *
 * description:
 *   looks up the name in the objdefs given. Returns the address of the
 *   objdef if found, NULL otherwise.
 *
 ******************************************************************************/
static node *
SearchForNameInObjs (const namespace_t *ns, const char *name, node *objs)
{
    DBUG_ENTER ();

    while ((objs != NULL) &&
           ((!NSequals (OBJDEF_NS (objs), ns)) ||
                    (!STReq (OBJDEF_NAME (objs), name))))
    {
        objs = OBJDEF_NEXT (objs);
    }

    DBUG_RETURN (objs);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDmodule( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
INSVDmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_MODULE (arg_info) = arg_node;
    INFO_OBJDEFS (arg_info) = MODULE_OBJS (arg_node);

    MODULE_FUNS (arg_node) = TRAVopt(MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDfundef( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_VARDECS (arg_info) = FUNDEF_VARDECS (arg_node);
        InitVardecLUT (INFO_VARDECS (arg_info), &INFO_VARDEC_LUT (arg_info));

        INFO_ARGS (arg_info) = FUNDEF_ARGS (arg_node);
        InitArgLUT (INFO_ARGS (arg_info), &INFO_ARG_LUT (arg_info));

        /* Will fill in missing vardecs */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_VARDECS (arg_node) = INFO_VARDECS (arg_info);

        INFO_VARDEC_LUT (arg_info) =
                LUTremoveContentLut (INFO_VARDEC_LUT (arg_info));
        INFO_ARG_LUT (arg_info) = LUTremoveContentLut (INFO_ARG_LUT (arg_info));
    }

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDspap( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDspap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * we have to make sure here, that the ID of an SPAP is not traversed
     * as it does not reference a variable and thus has not to be processed
     */

    SPAP_ARGS (arg_node) = TRAVopt(SPAP_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDlet( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    LET_IDS (arg_node) = TRAVopt(LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDspfold( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDspfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* Do NOT traverse SPFOLD_FN */

    SPFOLD_NEUTRAL (arg_node) = TRAVdo (SPFOLD_NEUTRAL (arg_node), arg_info);
    SPFOLD_ARGS (arg_node) = TRAVopt (SPFOLD_ARGS (arg_node), arg_info);
    SPFOLD_GUARD (arg_node) = TRAVopt (SPFOLD_GUARD (arg_node), arg_info);
    SPFOLD_NEXT (arg_node) = TRAVopt (SPFOLD_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDspid( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDspid (node *arg_node, info *arg_info)
{
    node *vardec;

    DBUG_ENTER ();

    if (SPID_NS (arg_node) == NULL) {
        vardec = SearchForName (SPID_NAME (arg_node),
                                INFO_VARDEC_LUT (arg_info));

        if (vardec == NULL) {
            vardec = SearchForName (SPID_NAME (arg_node),
                                    INFO_ARG_LUT (arg_info));
        }

        if (vardec == NULL) {
            /*
             * we still have not found a declaration but
             * maybe its a global object!
             */

            const namespace_t *ns = MODULE_NAMESPACE (INFO_MODULE (arg_info));
            vardec = SearchForNameInObjs (ns,
                                          SPID_NAME (arg_node),
                                          INFO_OBJDEFS (arg_info));

            if (vardec == NULL) {
                CTIerror (LINE_TO_LOC (global.linenum),
                          "Identifier '%s` used without previous definition",
                          SPID_NAME (arg_node));
            } else {
                /*
                 * we have to unalias it first!
                 */
                vardec = TCunAliasObjdef (vardec);

                arg_node = FREEdoFreeNode (arg_node);
                arg_node = TBmakeGlobobj (vardec);
            }
        } else {
            /*
             * now we can build a real id and remove the spid node
             */
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = TBmakeId (DECL_AVIS (vardec));
        }
    } else {
        /*
         * this id must be a global object since a namespace has been given!
         */

        vardec = SearchForNameInObjs (SPID_NS (arg_node), SPID_NAME (arg_node),
                                      INFO_OBJDEFS (arg_info));

        if (vardec == NULL) {
            if (NSequals (SPID_NS (arg_node),
                          MODULE_NAMESPACE (INFO_MODULE (arg_info)))) {
                /*
                 * this is most likely a reference to an undefined variable
                 */
                CTIerror (LINE_TO_LOC (global.linenum),
                          "Variable '%s' used without previous definition",
                          SPID_NAME (arg_node));
            } else {
                CTIerror (LINE_TO_LOC (global.linenum),
                          "No definition for global object '%s:%s' found",
                          NSgetName (SPID_NS (arg_node)), SPID_NAME (arg_node));
            }
        } else {
            /*
             * we have to unalias it first!
             */
            vardec = TCunAliasObjdef (vardec);

            arg_node = FREEdoFreeNode (arg_node);
            arg_node = TBmakeGlobobj (vardec);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDspids( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDspids (node *arg_node, info *arg_info)
{
    node *vardec, *avis, *new_ids;
    lut_t *vardec_lut;

    DBUG_ENTER ();

    vardec = SearchForName (SPIDS_NAME (arg_node), INFO_VARDEC_LUT (arg_info));
    if (vardec == NULL) {
        vardec = SearchForName (SPIDS_NAME (arg_node), INFO_ARG_LUT (arg_info));
    }

    if (vardec == NULL) {
        /*
         * The identifyer we are looking for does not have a
         * vardec yet! So we allocate one and prepand it to vardecs.
         */

        avis = TBmakeAvis (STRcpy (SPIDS_NAME (arg_node)),
                           TYmakeAUD (TYmakeSimpleType (T_unknown)));

        vardec = TBmakeVardec (avis, INFO_VARDECS (arg_info));
        vardec_lut = LUTinsertIntoLutS (INFO_VARDEC_LUT (arg_info),
                                        SPIDS_NAME (arg_node),
                                        vardec);
        INFO_VARDEC_LUT (arg_info) = vardec_lut;

        INFO_VARDECS (arg_info) = vardec;

        DBUG_PRINT ("inserting new vardec (" F_PTR ") for id %s.",
                        (void *)vardec, SPIDS_NAME (arg_node));
    }

    if (SPIDS_NEXT (arg_node) != NULL) {
        new_ids = TRAVdo (SPIDS_NEXT (arg_node), arg_info);
        /*
         * we have to reset SPIDS_NEXT here, as it has been
         * freed on the traversal down!
         */
        SPIDS_NEXT (arg_node) = NULL;
    } else {
        new_ids = NULL;
    }
    /*
     * now we can build a real ids node and remove the spids node
     */
    arg_node = FREEdoFreeNode (arg_node);
    arg_node = TBmakeIds (DECL_AVIS (vardec), new_ids);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDwith( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* traverse all sons in following order:*/

    WITH_PART (arg_node) = TRAVopt(WITH_PART (arg_node), arg_info);

    WITH_CODE (arg_node) = TRAVopt(WITH_CODE (arg_node), arg_info);

    WITH_WITHOP (arg_node) = TRAVopt(WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDdo( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* traverse all sons in following order:*/

    DO_BODY (arg_node) = TRAVopt(DO_BODY (arg_node), arg_info);

    DO_SKIP (arg_node) = TRAVopt(DO_SKIP (arg_node), arg_info);

    DO_COND (arg_node) = TRAVopt(DO_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDpart( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* withid has to be traversed first */

    PART_WITHID (arg_node) = TRAVopt(PART_WITHID (arg_node), arg_info);

    PART_GENERATOR (arg_node) = TRAVopt(PART_GENERATOR (arg_node), arg_info);

    PART_NEXT (arg_node) = TRAVopt(PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDcode( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* cblock has to be traversed first */

    CODE_CBLOCK (arg_node) = TRAVopt(CODE_CBLOCK (arg_node), arg_info);

    CODE_CEXPRS (arg_node) = TRAVopt(CODE_CEXPRS (arg_node), arg_info);

    CODE_NEXT (arg_node) = TRAVopt(CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * INSVDdoInsertVardecs( node *syntax_tree)
 *
 * description:
 *   Traverses through all functions and inserts vardecs for all identifyers
 *   that do not have one yet. Whenever an N_spid node is met, we simply look for
 *   its declaration. If found, a backref to it is inserted (ID_VARDEC).
 *   Otherwise, a new vardec is inserted and a backref is inserted.
 *   The same holds for the ids found at N_let nodes.
 *
 ******************************************************************************/

node *
INSVDdoInsertVardec (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module,
                 "InsertVardec can only be called on N_modul nodes");

    TRAVpush (TR_insvd);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    CTIabortOnError ();

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
