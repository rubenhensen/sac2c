/*
 *
 * $Log$
 * Revision 1.35  2005/05/24 08:24:27  sbs
 * some DBUG_PRINT etended.
 *
 * Revision 1.34  2005/05/23 20:42:27  sbs
 * now wrappers are created for local functions only.
 * ,
 *
 * Revision 1.33  2005/04/05 10:14:13  sah
 * fixed traversal while splitting wrappers
 *
 * Revision 1.32  2005/03/04 21:21:42  cg
 * Wrapper functions are no longer marked 'inline' since they
 * are no longer intended to be inlined.
 *
 * Revision 1.31  2005/01/11 14:20:44  cg
 * Converted output generation from Error.h to ctinfo.c
 *
 * Revision 1.30  2004/12/09 00:39:41  sbs
 * bug in CWCfold
 *
 * Revision 1.29  2004/12/06 17:29:04  sbs
 * removal of generic wrappers changed. Now there should be no zombie funs no more!
 * TUreplaceRettypes is now non-destructive!
 *
 * Revision 1.28  2004/11/27 02:11:57  jhb
 * fixed bug with header and c-file functions-declaration
 *
 * Revision 1.27  2004/11/25 17:52:55  sbs
 * compiles
 *
 * Revision 1.26  2004/11/24 17:42:48  sbs
 * not yet
 *
 * Revision 1.25  2004/11/19 10:15:50  sah
 * for objinit funs no wrapper is built
 *
 * Revision 1.24  2004/11/10 19:35:09  sbs
 * INFO_CWC_WITH now is stacked properly in CWCnwith...
 *
 * Revision 1.23  2004/11/07 18:12:48  sah
 * added some dbug statements
 *
 * Revision 1.22  2004/09/27 19:08:15  sbs
 * sharing of FUNDEF_RET_TYPEs eliminated when extracting return types
 * from the split wrappers using TYgetWrapperRetType
 *
 * Revision 1.21  2004/08/26 18:12:47  sbs
 * INFO_CWC_WITH added, CWCwith added,
 * CorrectFundefPointer signature changed (arg_types instead of args)
 * in order to be able to select the CORRECT version of foldfuns
 * => bug 48.
 *
 * Revision 1.20  2004/07/30 17:29:21  sbs
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.19  2004/03/05 12:04:13  sbs
 * Now, the modified wrappers will be inserted into the fundef chain correctly 8-)
 *
 * Revision 1.18  2004/02/20 08:14:00  mwe
 * now functions with and without body are separated
 * changed tree traversal (added traverse of MODULE_FUNDECS)
 *
 * Revision 1.17  2003/11/18 17:45:42  dkr
 * CWCwithop(): all node sons are traversed now
 *
 * Revision 1.16  2003/05/30 16:58:05  dkr
 * WrapperCodeIsNeeded() and WrapperCodeIsPossible() added
 *
 * Revision 1.15  2003/05/30 15:10:13  dkr
 * InsertWrapperCode() modified: wrapper code is build for all
 * non-var-args functions now,
 * bug in CorrectFundefPointer() fixed.
 *
 * Revision 1.14  2003/05/29 14:39:09  dkr
 * bug in CorrectFundefPointer() fixed
 *
 * Revision 1.13  2003/05/29 12:44:48  dkr
 * SearchWrapper() renamed into CorrectFundefPointer()
 *
 * Revision 1.12  2002/10/31 19:46:26  dkr
 * CorrectFundef() renamed into SearchWrapper()
 *
 * Revision 1.11  2002/10/30 16:11:35  dkr
 * trivial wrappers are no longer built but dispatched statically
 *
 * Revision 1.10  2002/10/30 13:23:59  sbs
 * handling of dot args introduced.
 *
 * Revision 1.9  2002/10/18 14:33:17  sbs
 * some DBUG output added and some FLAG handling of freshly created N_id nodes
 * added.
 *
 * Revision 1.8  2002/09/05 14:45:55  dkr
 * CWCwithop() added
 *
 * Revision 1.7  2002/09/03 18:54:41  dkr
 * this modul is complete now :-)
 *
 * Revision 1.6  2002/08/28 11:36:05  dkr
 * SignatureMatches() added
 *
 * Revision 1.5  2002/08/15 21:27:50  dkr
 * MODULE_WRAPPERFUNS added (not used yet ...)
 *
 * Revision 1.4  2002/08/13 15:59:09  dkr
 * some more cwc stuff added (not finished yet)
 *
 * Revision 1.3  2002/08/09 14:50:53  dkr
 * CWCap added
 *
 * Revision 1.2  2002/08/09 13:15:20  dkr
 * CWCmodul, CWCfundef added
 *
 * Revision 1.1  2002/08/09 13:00:02  dkr
 * Initial revision
 *
 */

#define NEW_INFO

#include "create_wrapper_code.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "ctinfo.h"
#include "LookUpTable.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "new_typecheck.h"
#include "new_types.h"
#include "type_utils.h"
#include "ct_fun.h"

/*******************************************************************************
 *
 *
 */

/**
 * INFO structure
 */
struct INFO {
    int travno;
    lut_t *wrapperfuns;
    node *modul;
    node *nwith;
};

/**
 * INFO macros
 */
#define INFO_CWC_TRAVNO(n) ((n)->travno)
#define INFO_CWC_WRAPPERFUNS(n) ((n)->wrapperfuns)
#define INFO_CWC_MODUL(n) ((n)->modul)
#define INFO_CWC_WITH(n) ((n)->nwith)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_CWC_TRAVNO (result) = 0;
    INFO_CWC_WRAPPERFUNS (result) = NULL;
    INFO_CWC_MODUL (result) = NULL;
    INFO_CWC_WITH (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/**
 **
 ** Function:
 **   node *CreateWrapperCode( node *ast)
 **
 ** Description:
 **   Modifies all wrappers of the AST in a way that they represent correct
 **   SAC functions and that they contain correct code for dispatching
 **   overloaded functions at runtime. In more detail:
 **     - Replaces generic wrappers (valid for more than a single simpletype)
 **       by individual wrappers for each simpletype.
 *      - Generates the bodies of the wrapper functions.
 **   Moreover, all references to replaced wrapper functions are corrected
 **   accordingly:
 **     - AP_FUNDEF,
 **     - NWITHOP_FUNDEF.
 **
 **/

/******************************************************************************
 *
 * Function:
 *   node *CWCmodule( node *arg_node, info *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWCmodule");

    DBUG_ASSERT ((MODULE_WRAPPERFUNS (arg_node) != NULL),
                 "MODULE_WRAPPERFUNS not found!");
    INFO_CWC_WRAPPERFUNS (arg_info) = MODULE_WRAPPERFUNS (arg_node);

    /*
     * create separate wrapper function for all base type constellations
     * As all wrappers are in the FUNS, we have to traverse these only!
     */
    INFO_CWC_TRAVNO (arg_info) = 1;

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    /*
     * adjust AP_FUNDEF pointers
     * As only FUNS may contain N_ap's we have to traverse these only!
     */
    INFO_CWC_TRAVNO (arg_info) = 2;
    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    /*
     * remove non-used and zombie funs!
     */
    INFO_CWC_TRAVNO (arg_info) = 3;

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    MODULE_WRAPPERFUNS (arg_node) = LUTremoveLut (MODULE_WRAPPERFUNS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *SplitWrapper( node *fundef)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
SplitWrapper (node *fundef)
{
    ntype *old_type, *tmp_type;
    ntype *new_type;
    bool finished;
    node *new_fundef;
    node *new_fundefs = NULL;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("SplitWrapper");

    old_type = FUNDEF_WRAPPERTYPE (fundef);
    tmp_type = TYcopyType (old_type);
    FUNDEF_WRAPPERTYPE (fundef) = NULL;
    DBUG_PRINT ("CWC", ("splitting wrapper of %s:%s", FUNDEF_MOD (fundef),
                        FUNDEF_NAME (fundef)));
    do {
        new_fundef = DUPdoDupNode (fundef);
        new_type = TYsplitWrapperType (tmp_type, &finished);
        DBUG_EXECUTE ("CWC", tmp_str = TYtype2String (new_type, TRUE, 0););
        DBUG_PRINT ("CWC", ("  new wrapper split off: %s", tmp_str));
        DBUG_EXECUTE ("CWC", tmp_str = ILIBfree (tmp_str););

        FUNDEF_WRAPPERTYPE (new_fundef) = new_type;
        FUNDEF_RETS (new_fundef)
          = TUreplaceRetTypes (FUNDEF_RETS (new_fundef), TYgetWrapperRetType (new_type));
        FUNDEF_ARGS (new_fundef)
          = TYcorrectWrapperArgTypes (FUNDEF_ARGS (new_fundef), new_type);
        FUNDEF_NEXT (new_fundef) = new_fundefs;
        new_fundefs = new_fundef;
    } while (!finished);
    FUNDEF_WRAPPERTYPE (fundef) = old_type;
    tmp_type = TYfreeType (tmp_type);

    DBUG_RETURN (new_fundefs);
}

/** <!--********************************************************************-->
 *
 * @fn  bool WrapperCodeIsNeeded( node *fundef)
 *
 * @brief
 *
 * @param
 * @return
 *
 ******************************************************************************/

static bool
WrapperCodeIsNeeded (node *fundef)
{
    bool result;

    DBUG_ENTER ("WrapperCodeIsNeeded");

    result = FUNDEF_ISLOCAL (fundef);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn  bool WrapperCodeIsPossible( node *fundef)
 *
 * @brief
 *
 * @param
 * @return
 *
 ******************************************************************************/

static bool
WrapperCodeIsPossible (node *fundef)
{
    bool result;

    DBUG_ENTER ("WrapperCodeIsPossible");

    /*
     * SAC functions with var-args are not allowed
     *   -> no wrapper functions for C functions with var-args!!
     */
    result = (!(FUNDEF_HASDOTARGS (fundef) || FUNDEF_HASDOTRETS (fundef)));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * Function:
 *   node *InsertWrapperCode( node *fundef)
 *
 * Description:
 *   Creates dispatch code for the given wrapper function and stores it in
 *   FUNDEF_BODY.
 *
 ******************************************************************************/

static node *
InsertWrapperCode (node *fundef)
{
    node *ret;
    node *assigns;
    node *vardec;
    node *vardecs1, *vardecs2;

    DBUG_ENTER ("InsertWrapperCode");

    DBUG_ASSERT (((NODE_TYPE (fundef) == N_fundef) && FUNDEF_ISWRAPPERFUN (fundef)
                  && (FUNDEF_BODY (fundef) == NULL)),
                 "inconsistant wrapper function found!");

    if (WrapperCodeIsNeeded (fundef) && WrapperCodeIsPossible (fundef)) {
        /*
         * generate wrapper code together with the needed vardecs
         */
        vardecs1 = TUcreateTmpVardecsFromRets (FUNDEF_RETS (fundef));
        vardecs2 = NULL;
        assigns = TYcreateWrapperCode (fundef, vardecs1, &vardecs2);

        /*
         * vardecs -> return exprs
         */
        ret = NULL;
        vardec = vardecs1;
        while (vardec != NULL) {
            ret = TBmakeExprs (TBmakeId (VARDEC_AVIS (vardec)), ret);
            vardec = VARDEC_NEXT (vardec);
        }
        FUNDEF_RETURN (fundef) = ret = TBmakeReturn (ret);

        /*
         * append return statement to assignments
         */
        assigns = TCappendAssign (assigns, TBmakeAssign (ret, NULL));

        /*
         * insert function body
         */
        FUNDEF_BODY (fundef) = TBmakeBlock (assigns, TCappendVardec (vardecs1, vardecs2));

        /*
         * mark wrapper function as a inline function
         *
         * FUNDEF_ISINLINE( fundef) = TRUE;
         *
         * Inlining of wrapper functions proved unsatisfactory.
         */
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Function:
 *   ntype *ActualArgs2Ntype( node *actual)
 *
 * Description:
 *   Returns the appropriate product type for the given actual arguments.
 *
 ******************************************************************************/

static ntype *
ActualArgs2Ntype (node *actual)
{
    ntype *actual_type, *tmp_type, *prod_type;
    int size, pos;

    DBUG_ENTER ("ActualArgs2Ntype");

    size = TCcountExprs (actual);
    prod_type = TYmakeEmptyProductType (size);

    pos = 0;
    while (actual != NULL) {
        tmp_type = NTCnewTypeCheck_Expr (EXPRS_EXPR (actual));
        actual_type = TYfixAndEliminateAlpha (tmp_type);
        tmp_type = TYfreeType (tmp_type);

        TYsetProductMember (prod_type, pos, actual_type);
        actual = EXPRS_NEXT (actual);
        pos++;
    }

    DBUG_RETURN (prod_type);
}

/******************************************************************************
 *
 * Function:
 *   bool SignatureMatches( node *formal, ntype *actual_prod_type)
 *
 * Description:
 *   Checks whether TYPE('formal') is a supertype of 'actual_prod_type'.
 *
 ******************************************************************************/

static bool
SignatureMatches (node *formal, ntype *actual_prod_type)
{
    ntype *actual_type, *formal_type;
    int pos;
    bool match = TRUE;
#ifndef DBUG_OFF
    char *tmp_str, *tmp2_str;
#endif

    DBUG_ENTER ("SignatureMatches");

    pos = 0;
    while ((formal != NULL) && (ARG_TYPE (formal) != NULL)
           && (TYPES_BASETYPE (ARG_TYPE (formal)) != T_dots)) {
        DBUG_ASSERT ((NODE_TYPE (formal) == N_arg), "illegal args found!");

        formal_type = AVIS_TYPE (ARG_AVIS (formal));
        actual_type = TYgetProductMember (actual_prod_type, pos);
        DBUG_EXECUTE ("CWC", tmp_str = TYtype2String (formal_type, FALSE, 0);
                      tmp2_str = TYtype2String (actual_type, FALSE, 0););
        DBUG_PRINT ("CWC", ("    comparing formal type %s with actual type %s", tmp_str,
                            tmp2_str));
        DBUG_EXECUTE ("CWC", tmp_str = ILIBfree (tmp_str);
                      tmp2_str = ILIBfree (tmp2_str););

        if (!TYleTypes (actual_type, formal_type)) {
            match = FALSE;
            break;
        }

        formal = ARG_NEXT (formal);
        pos++;
    }
    DBUG_PRINT ("CWC", ("    result: %d", match));

    DBUG_RETURN (match);
}

/******************************************************************************
 *
 * Function:
 *   node *CorrectFundefPointer( node *fundef, char *funname, ntype *arg_types)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CorrectFundefPointer (node *fundef, char *funname, ntype *arg_types)
{
    dft_res *dft_res;

    DBUG_ENTER ("CorrectFundefPointer");

    DBUG_ASSERT ((fundef != NULL), "fundef not found!");
    if (FUNDEF_ISWRAPPERFUN (fundef)) {
        /*
         * 'fundef' points to an generic wrapper function
         *   -> try to dispatch the function application statically in order to
         *      avoid superfluous wrapper function calls
         *   -> if static dispatch impossible, search for correct wrapper
         */
        DBUG_PRINT ("CWC", ("correcting fundef for %s:%s", FUNDEF_MOD (fundef), funname));

        /*
         * try to dispatch the function application statically
         */
        dft_res = NTCCTdispatchFunType (fundef, arg_types);
        if (dft_res == NULL) {
            DBUG_ASSERT ((TYgetProductSize (arg_types) == 0),
                         "illegal dispatch result found!");
            /*
             * no args found -> static dispatch possible
             *
             * fundef can be found in FUNDEF_IMPL (dirty hack!)
             */
            fundef = FUNDEF_IMPL (fundef);
            DBUG_PRINT ("CWC", ("  dispatched statically %s", funname));
        } else if ((dft_res->num_partials == 0)
                   && (dft_res->num_deriveable_partials == 0)) {
            /*
             * static dispatch possible
             */
            if (dft_res->def != NULL) {
                DBUG_ASSERT ((dft_res->deriveable == NULL), "def and deriveable found!");
                fundef = dft_res->def;
            } else {
                fundef = dft_res->deriveable;
            }
            DBUG_PRINT ("CWC", ("  dispatched statically %s", funname));
        } else if (!WrapperCodeIsPossible (fundef)) {
            /*
             * static dispatch impossible,
             *    but no wrapper function could be created either!!
             * if only a single instance is available, do the dispatch statically and
             * give a warning message, otherwise we are stuck here!
             */
            if ((dft_res->num_partials + dft_res->num_deriveable_partials == 1)
                && (dft_res->def == NULL) && (dft_res->deriveable == NULL)) {
                fundef = (dft_res->num_partials == 1) ? dft_res->partials[0]
                                                      : dft_res->deriveable_partials[0];
                CTIwarnLine (global.linenum,
                             "Application of var-arg function %s found which may"
                             " cause a type error",
                             FUNDEF_NAME (fundef));
                DBUG_PRINT ("CWC", ("  dispatched statically although only partial"
                                    " has been found (T_dots)!",
                                    funname));
            } else {
                DBUG_ASSERT ((0), "wrapper with T_dots found which could be dispatched "
                                  "statically!");
            }
        } else {
            /*
             * static dispatch impossible -> search for correct wrapper
             */
            DBUG_PRINT ("CWC", ("  static dispatch impossible, search for wrapper"));
            do {
                fundef = FUNDEF_NEXT (fundef);
                DBUG_ASSERT (((fundef != NULL)
                              && ILIBstringCompare (funname, FUNDEF_NAME (fundef))
                              && FUNDEF_ISWRAPPERFUN (fundef)),
                             "no appropriate wrapper function found!");
            } while (!SignatureMatches (FUNDEF_ARGS (fundef), arg_types));
            DBUG_PRINT ("CWC", ("  correct wrapper found"));

            DBUG_ASSERT ((FUNDEF_BODY (fundef) != NULL), "no wrapper code found!");
        }
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Function:
 *   node *CWCfundef( node *arg_node, info *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
FundefBuildWrappers (node *arg_node, info *arg_info)
{
    node *new_fundef;
    node *new_fundefs;

    DBUG_ENTER ("FundefBuildWrappers");

    if (FUNDEF_ISWRAPPERFUN (arg_node)) {
        DBUG_ASSERT ((FUNDEF_BODY (arg_node) == NULL),
                     "wrapper function has already a body!");

        /*
         * build a separate fundef for each base type constellation
         */
        new_fundefs = SplitWrapper (arg_node);

        /*
         * build code for all wrapper functions
         */
        new_fundef = new_fundefs;
        DBUG_ASSERT ((new_fundef != NULL), "no wrapper functions found!");
        do {
            new_fundef = InsertWrapperCode (new_fundef);
            new_fundef = FUNDEF_NEXT (new_fundef);
        } while (new_fundef != NULL);

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }

        /*
         * insert new wrapper functions at the begining of MODULE_FUNS and
         * free original wrapper function (-> zombie function)
         */
        new_fundefs = TCappendFundef (new_fundefs, FUNDEF_NEXT (arg_node));
        DBUG_ASSERT ((FUNDEF_BODY (arg_node) == NULL),
                     "body of generic wrapper function has not been kept empty");
        FUNDEF_NEXT (arg_node) = new_fundefs;
    } else {
        /*
         * if this is no wrapper function, just skip to the next function
         */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }
    DBUG_RETURN (arg_node);
}

static node *
FundefAdjustPointers (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FundefAdjustPointers");

    if ((!FUNDEF_ISWRAPPERFUN (arg_node)) && (FUNDEF_BODY (arg_node) != NULL)) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

static node *
FundefRemoveGarbage (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FundefRemoveGarbage");

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    if ((FUNDEF_ISWRAPPERFUN (arg_node)) && (FUNDEF_BODY (arg_node) == NULL)) {
        /*
         * remove statically dispatchable wrapper function and all generic wrappers
         */
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
CWCfundef (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CWCfundef");

    if (INFO_CWC_TRAVNO (arg_info) == 1) {
        /*
         * first traversal -> build wrapper functions and their bodies
         */
        arg_node = FundefBuildWrappers (arg_node, arg_info);
    } else if (INFO_CWC_TRAVNO (arg_info) == 2) {
        /*
         * second traversal -> adjust all AP_FUNDEF pointers
         *
         * This is needed if the original wrapper function was valid for more than
         * a single base type.
         */
        arg_node = FundefAdjustPointers (arg_node, arg_info);
    } else {
        DBUG_ASSERT ((INFO_CWC_TRAVNO (arg_info) == 3), "illegal INFO_CWC_TRAVNO found!");
        /*
         * third traversal -> remove zombies and empty wrappers
         */

        arg_node = FundefRemoveGarbage (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CWCap( node *arg_node, info *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCap (node *arg_node, info *arg_info)
{
    ntype *arg_types;

    DBUG_ENTER ("CWCap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    arg_types = ActualArgs2Ntype (AP_ARGS (arg_node));
    AP_FUNDEF (arg_node)
      = CorrectFundefPointer (AP_FUNDEF (arg_node), AP_NAME (arg_node), arg_types);

    DBUG_PRINT ("CWC", ("Ap of function %s:%s now points to " F_PTR ".",
                        AP_MOD (arg_node), AP_NAME (arg_node), AP_FUNDEF (arg_node)));
    arg_types = TYfreeType (arg_types);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *CWCwith( node *arg_node, info *arg_info)
 *
 * @brief inserts actual with node into the info structure
 *
 ******************************************************************************/

node *
CWCwith (node *arg_node, info *arg_info)
{
    node *old_with;
    DBUG_ENTER ("CWCwith");

    old_with = INFO_CWC_WITH (arg_info);
    INFO_CWC_WITH (arg_info) = arg_node;

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    INFO_CWC_WITH (arg_info) = old_with;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CWCgenarray( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWCgenarrray");

    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CWCfold (node *arg_node, info *arg_info)
{
    ntype *neutr_type, *body_type;
    ntype *arg_type, *arg_types;

    DBUG_ENTER ("CWCfold");

    if (FOLD_FUN (arg_node) != NULL) {
        FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

        neutr_type
          = TYfixAndEliminateAlpha (AVIS_TYPE (ID_AVIS (FOLD_NEUTRAL (arg_node))));
        body_type = TYfixAndEliminateAlpha (
          AVIS_TYPE (ID_AVIS (WITH_CEXPR (INFO_CWC_WITH (arg_info)))));

        arg_type = TYlubOfTypes (neutr_type, body_type);
        arg_types = TYmakeProductType (2, arg_type, TYcopyType (arg_type));

        FOLD_FUNDEF (arg_node)
          = CorrectFundefPointer (FOLD_FUNDEF (arg_node), FOLD_FUN (arg_node), arg_types);
        arg_types = TYfreeType (arg_types);
        body_type = TYfreeType (body_type);
        neutr_type = TYfreeType (neutr_type);
    } else {
        if (FOLD_NEUTRAL (arg_node) != NULL) {
            FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CreateWrapperCode( node *ast)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCdoCreateWrapperCode (node *ast)
{
    info *info_node;

    DBUG_ENTER ("CWCdoCreateWrapperCode");

    TRAVpush (TR_cwc);

    info_node = MakeInfo ();
    ast = TRAVdo (ast, info_node);
    info_node = FreeInfo (info_node);

    TRAVpop ();

    DBUG_RETURN (ast);
}
