/**
 * @file
 * @defgroup embed GPU functions
 * @ingroup cuda
 *
 * @{
 *
 * Concept:
 * --------
 * This traversal enables the passing of pointers to gpu device memory through
 * function calls. This is achieved by creating a small wrapper function which
 * solely transfers certain host memory pointers to device memory pointers and
 * then calls a variant of the function which expects pointers to gpu memory.
 * By inlining the wrapper functions, we can lift such transfers from called
 * functions into the calling context.
 * Let us look at a simple example:
 *
 * int[.] foo (double[.] a, int[.] b)  // parameter 'a' to be changed!
 *                                     // return value to be changed!
 * {
 *    ...
 *    a_dev = _host2device_ (a);
 *    ...
 *    res = _device2host_ (res_dev);
 *    ...
 *    return res;
 * }
 *
 * int main () 
 * {
 *     c = foo (a, b);             // calling site (could be multiple!)
 *     ...
 * }
 *
 *
 * is translated into:
 *
 *
 * inline
 * int[.] foo (double[.] a, int[.] b)  // unchanged! ensures all calls are
 *                                     // fine; even from wrapper::foo!
 * {
 *    a_dev = _host2device_ (a);
 *    res_dev = fooGPU (double_dev[.] a_def, int[.] b);
 *    res = _device2host_ (res_dev);
 *    return res;
 * }
 *
 * int_dev[.] fooGPU (double_dev[.] a_dev, int[.] b)  // modified copy of the fundef!
 * {
 *    ...
 *    // a_dev = _host2device_ (a);             needs to go!
 *    ...
 *    // res = _device2host_ (res_dev);         needs to go!
 *    ...
 *    return res_dev;
 * }
 *
 * int main()
 * {
 *     c = foo (a, b);       // note here: no changes needed!
 *     ...
 * }
 *
 *
 * Note here, that we replace 'a' by 'a_dev'. An alternative idea could be 
 * to just add 'a_dev'. While this would make the mechanism more widely
 * applicable (there could be other uses of 'a' in the body), for the
 * time being (2022), we go for this restricted approach as it allows us
 * to use the very same mechanism to deal with external functions that
 * expect pointers to device memory: We simply declare these as normal arrays
 * and then use the gpumem-pragma to indicate which of the arguments/
 * return values should be converted into their device counterparts.
 * For example:
 *
 * extern int[.] foo (double[.] a, int[.] b);
 *     #pragma gpumem [1]
 *
 * is translated into:
 *
 * inline
 * int[.] foo (double[.] a, int[.] b)
 * {
 *    a_dev = _host2device_ (a);
 *    res = fooGPU (a_dev, b);
 *    return res;
 * }
 *
 *  extern int[.] fooGPU (double_dev[.] a, int[.] b); // this needs to be
 *                                                    // the copy!
 *
 * In order to be able to inline these new inline functions, we need to tag
 * all functions that *call* GPU functions as !ISINLINECOMPLETED. 
 * At the end of this traversal we simply mark all functions as such.
 *
 * Implementation:
 * ---------------
 *
 * The traversal builds on the folowing assumptions:
 * -- The flags FUNDEF_ISGPUFUNCTION, ARG_ISGPUMEM, and RET_ISGPUMEM are
 *    all set correctly. FUNDEF_ISGPUFUNCTION is set, iff at least
 *    one ARG_ISGPUMEM or one RET_ISGPUMEM are being set.
 * -- if an argument is marked ARG_ISGPUMEM, all uses of this parameter
 *    are as actual arguments to _host2device_ transfers.
 * -- if a return value is marked RET_ISGPUMEM, the return value is
 *    defined through a _device2host_ transfer AND it is not used
 *    anywhere else.
 *
 * When traversing into an N_fundef we only consider functions
 * marked ISGPUFUNCTION. once found, we do the following:
 *
 * As can be seen from the description above, we need to transform
 * one given function "foo" into two versions of it:
 * - a small stub version that keeps the non-device signature,
 *   manages the transfers between host and device and calls 
 *   the second version of "foo" (fooGPU above) that operates
 *   on device types; and
 * - a second version that operates on device types.
 * 
 * We achieve this by first copying the given function, then
 * transforming both, the original and the copy of the function.
 * Since all external calls to the funtion already point to
 * the original, we transform the original into the stub function
 * and the copy into the GPU fnuction.
 *
 * We deal with the copy first, turning it into the GPU function.
 * We traverse its body adjusting it as needed:
 * For external functions this means adjusting the signature only,
 * for fully defined functions this additionally
 * involves eliding transfer-operations and potentially renaming
 * return values.
 *
 * Finally, we deal with the original function. We create the
 * stub function bodies and either add them to the original
 * external fundef or replace the existing body by it.
 * 
 * COPYING THE FUNDEF
 *
 * First we copy the fundef and put it into INFO_NEWFUNDEF.
 *
 * SIGNATURE ADJUSTMENT FOR EXTERNAL FUNCTIONS:
 *
 * In case we are dealing with an external GPU function:
 * for all args / rets with ISGPUMEM, replace the host type by the 
 * device type and unset FUNDEF_ISGPUFUNCTION, ARG_ISGPUMEM,
 * and RET_ISGPUMEM.
 *
 * BODY ADJUSMENT FOR FULLY DEFINED FUNCTIONS:
 *
 * NB: This functonality is not yet implemented! It also 
 * requires a prior traversal for annotating suitable 
 * functions! (11/2022). However, I have added a description
 * of how this could be implemented already :-)
 *
 * If a GPU function has a body, we traverse its body to 
 * identify the transfer function calls for args/rets that
 * have been marked ISGPUMEM. To do so, we use INFO_NEWFUNDEF
 * to access the arg and ret chains of the function copy.
 * While we can find the args on the way down, we can only identify 
 * the rets after having traversed the N_return statement.
 * Whenever we meet a transfer of an argument 'a' to the device
 * into 'a_dev' and 'a' is marked ISGPUMEM, we delete the avis 
 * of 'a', we then set it to that of 'a_dev', We unset the
 * AVIS_SSAASSIGN and we unset ISGPUMEM of the arg as well.
 * Finally, we mark the assignment of the transfer as ISUNUSED. This
 * will trigger its deletion on the way back up.
 * When we traverse the N_return, we identify those return values
 * whoese corresponding rets are marked ISGPUMEM,
 * When finding one of those, their SSAASSIGN needs to point
 * to a _device2host_ transfer. We delete the avis of those N_id
 * nodes, replace them with the avis of the argument to _device2host_,
 * set the corresponding RET_ISGPUMEM to false and we mark the
 * corresponding assignment as ISUNUSED.
 * On the way back up, all redundent assignments are being freed.
 *
 * STUB-FUNCTION BODY CREATION:
 *
 * After the processing of the new function, we have INFO_NEWFUNDEF
 * pointing to the new function which needs to be called within the stub.
 * We generate the stub function from the original function using 
 * the static function 'transformOldFundefIntoStub'.
 * The information which arguments and return values need to be transferred
 * is till annotated in the args and rets of the original N_fundef.
 * 
 * INSERTION OF THE STUB
 *
 * Once the stub body is ready, we free the old function body
 * (if it exists), declare the function inline and non-external before
 * we insert the function body.
 *
 */
#include "embed_gpu_functions.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "memory.h"
#include "free.h"
#include "new_types.h"
#include "cuda_utils.h"

#define DBUG_PREFIX "EGF"
#include "debug.h"
#include "print.h"




/**
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool isdecl;
    node *stubfun;
};

/**
 * INFO macros
 */
#define INFO_NEWFUNDEF(n) (n->fundef)      // keeps the pointer to the copied fundef
#define INFO_ISFUNDECL(n) (n->isdecl)      // indicates whether the function is external
#define INFO_STUB_FUN_CHN(n) (n->stubfun)  // holds stub functions from external
                                           // functions which need to 'move' from 
                                           // DECLS to FUNS

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_NEWFUNDEF (result) = NULL;
    INFO_ISFUNDECL (result) = FALSE;
    INFO_STUB_FUN_CHN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}










/*
 * HELPER FUNCTIONS
 */
/** <!--********************************************************************-->
 *
 * @fn MarkFundefsNotInlineCompleted
 *
 * @brief this is an anonymous traversal that sets all fundefs in the 
 *    given fundef chain to FUNCTION_ISINLINECOMPLETED = FALSE!
 *
 *****************************************************************************/
static
node *ATravFundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    FUNDEF_ISINLINECOMPLETED (arg_node) = FALSE;
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

static
node *MarkFundefsNotInlineCompleted (node *fundef)
{
    anontrav_t atrav[2] = {{N_fundef, &ATravFundef},
                           {(nodetype)0, NULL}};
    DBUG_ENTER ();
    TRAVpushAnonymous (atrav, &TRAVsons);
    TRAVdo (fundef, NULL);
    TRAVpop ();
    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn args2assignsVardecsExprs
 *
 * @brief this is a helper for constructing the stub functions. It traverses 
 *    the functions arguments provided in <arg> and produces chains of 
 *    1) N_assign nodes that perform host2device transfers if needed,
 *    2) N_vardec nodes for newly needed device variables, and
 *    3) N_exprs nodes that hold the arguments needed for calling the GPU
 *    function.
 *    Note that the exprs-chain matches the lenfth of the <arg> chain, 
 *    but the other two chains only contain as many elements as we have
 *    ISGPUMEM arguments.
 *    In the process, we also adjust the N_arg nodes, resetting
 *    ARG_HASLINKSIGNINFO, ARG_ISREFCOUNTED, and ARG_ISGPUMEM since 
 *    the function is turned into a normal, locally defined Sac function.
 *
 *****************************************************************************/
static
void args2assignsVardecsExprs (node *arg, node **assign, node **vardec, node **exprs)
{
    node *avis;
    DBUG_ENTER ();

    if (ARG_NEXT (arg) != NULL) {
        args2assignsVardecsExprs (ARG_NEXT (arg), assign, vardec, exprs);
    }
    ARG_HASLINKSIGNINFO (arg) = FALSE;
    ARG_ISREFCOUNTED (arg) = TRUE;
    if (ARG_ISGPUMEM (arg)) {
        // BUILD    a_dev = _host2device_ (a);
        avis = TBmakeAvis (
                   TRAVtmpVarName ("_dev"),
                   CUconvertHostToDeviceType (ARG_NTYPE (arg)));
        *assign = TBmakeAssign (
                      TBmakeLet (
                          TBmakeIds (avis, NULL),
                          TCmakePrf1 (F_host2device, TBmakeId (ARG_AVIS (arg)))),
                      *assign);
        AVIS_SSAASSIGN (avis) = *assign;

        // insert a vardec for a_dev:
        *vardec = TBmakeVardec (avis, *vardec);

        // use    a_dev    as next argument in *exprs!
        *exprs = TBmakeExprs (
                     TBmakeId (avis),
                     *exprs);
    } else {
        // use    a    as next argument in *exprs!
        *exprs = TBmakeExprs (
                     TBmakeId (ARG_AVIS (arg)),
                     *exprs);
    }
    ARG_ISGPUMEM (arg) = FALSE;

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn rets2assignsVardecsIdsExprs
 *
 * @brief this is the second helper for constructing the stub functions. It
 *    traverses the return types of the function provided in <ret>
 *    and produces chains of
 *    1) N_assign nodes that perform device2host transfers if needed,
 *    2) N_vardec nodes for newly needed device variables, 
 *    3) N_ids nodes that hold the results from calling the GPU function, and
 *    4) N_exprs nodes that will serve as the return values.
 *    Note that the ids-chain and the exprs-chain matches the length of the
 *    <ret> chain, but the other two chains only contain as many elements as
 *    we have ISGPUMEM rets.
 *    In the process, we also adjust the N_ret nodes, resetting
 *    RET_HASLINKSIGNINFO, RET_ISREFCOUNTED, and RET_ISGPUMEM since
 *    the function is turned into a normal, locally defined Sac function.
 *
 *****************************************************************************/
static
void rets2assignsVardecsIdsExprs (node *ret, node **assign, node **vardec,
                                             node **ids, node **exprs)
{
    node *avis_h;
    node *avis_d;
    DBUG_ENTER ();

    if (RET_NEXT (ret) != NULL) {
        rets2assignsVardecsIdsExprs (RET_NEXT (ret), assign, vardec,
                                                     ids, exprs);
    }
    RET_HASLINKSIGNINFO (ret) = FALSE;
    RET_ISREFCOUNTED (ret) = TRUE;
    if (RET_ISGPUMEM (ret)) {
        // BUILD    a = _device2host_ (a_dev);
        avis_h = TBmakeAvis (
                     TRAVtmpVarName ("_host"),
                     TYcopyType (RET_TYPE (ret)));
        avis_d = TBmakeAvis (
                     TRAVtmpVarName ("_dev"),
                     CUconvertHostToDeviceType (RET_TYPE (ret)));
        *assign = TBmakeAssign (
                      TBmakeLet (
                          TBmakeIds (avis_h, NULL),
                          TCmakePrf1 (F_device2host, TBmakeId (avis_d))),
                      *assign);
        AVIS_SSAASSIGN (avis_h) = *assign;
        // insert a vardecs for a_dev:
        *vardec = TBmakeVardec (avis_d, *vardec);

        // use    a_dev    as next LHS in *ids!
        *ids = TBmakeIds (avis_d, *ids);

    } else {
        // insert   a into ids:
        avis_h = TBmakeAvis (
                     TRAVtmpVarName ("_host"),
                     TYcopyType (RET_TYPE (ret)));
        *ids = TBmakeIds (avis_h, *ids);
    }
    // insert a vardecs for a:
    *vardec = TBmakeVardec (avis_h, *vardec);
    // put   a    into *exprs!
    *exprs = TBmakeExprs (
                 TBmakeId (avis_h),
                 *exprs);
    RET_ISGPUMEM (ret) = FALSE;

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn setSSAassigns
 *
 * @brief this little helper set the AVIS_SSAASSIGN of all N_avis that the
 *    N_ids nodes in <ids> point to to the N_assign <assign>.
 *
 *****************************************************************************/
static
void setSSAassigns (node *ids, node *assign)
{
    DBUG_ENTER ();
    while (ids != NULL) {
        AVIS_SSAASSIGN (IDS_AVIS (ids)) = assign;
        ids = IDS_NEXT (ids);
    }
        
    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn transformOldFundefIntoStub
 *
 * @brief this function takes an N_fundef <old_fundef> which may be a fully
 *    defined function or an externally defined function only and turns it 
 *    into a stub function as described above.
 *    The main parts are being build through the two key helpers above
 *    that traverse the N_arg and N_ret chains, respectively.
 *    After constructing the body, we use it to replace the old one (if it
 *    existed) and we turn the (potential declaration) into a fullly fledged
 *    locally defined function which is set to INLINE.
 *
 *****************************************************************************/
static
node *transformOldFundefIntoStub (node *old_fundef, info *arg_info)
{
    node *body;
    node *exprs = NULL;
    node *ret_exprs = NULL;
    node *ids = NULL;
    node *pre_assigns = NULL;
    node *post_assigns = NULL;
    node *vardecs = NULL;
    node *return_node;

    DBUG_ENTER ();

    args2assignsVardecsExprs (FUNDEF_ARGS (old_fundef), &pre_assigns, &vardecs, &exprs);

    rets2assignsVardecsIdsExprs (FUNDEF_RETS (old_fundef), &post_assigns, &vardecs,
                                 &ids, &ret_exprs);

    return_node = TBmakeReturn ( ret_exprs);
    FUNDEF_RETURN (old_fundef) = return_node;
    post_assigns = TCappendAssign (post_assigns,
                                   TBmakeAssign (return_node, NULL));
    
    post_assigns = TBmakeAssign (
                       TBmakeLet (
                           ids,
                           TBmakeAp (
                               INFO_NEWFUNDEF (arg_info),
                               exprs)),
                       post_assigns);
    setSSAassigns (ids, post_assigns);

    body = TBmakeBlock (
               TCappendAssign (pre_assigns, post_assigns),
               vardecs);

    if (!FUNDEF_ISEXTERN (old_fundef)) {
        DBUG_ASSERT (FUNDEF_BODY (old_fundef) != NULL, 
                     "non external function without body encountered!");
        FUNDEF_BODY (old_fundef) = FREEdoFreeTree (FUNDEF_BODY (old_fundef));
    }
    FUNDEF_BODY (old_fundef) = body;
    FUNDEF_ISEXTERN (old_fundef) = FALSE;
    FUNDEF_ISINLINE (old_fundef) = TRUE;
    FUNDEF_ISGPUFUNCTION (old_fundef) = FALSE;

    DBUG_RETURN (old_fundef);
}




/*
 * TRAVERSAL FUNCTIONS
 */

/** <!--********************************************************************-->
 *
 * @fn node *EGFmodule (node *arg_node, info *arg_info)
 *
 * @brief traverse DECLS and FUNS and insert stub functions that have been
 *     modified and taken out of the DECLS into the FUNS chain.
 *    
 *
 *
 *****************************************************************************/
node *
EGFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_ISFUNDECL (arg_info) = TRUE;
    MODULE_FUNDECS (arg_node) = TRAVopt (MODULE_FUNDECS (arg_node), arg_info);

    INFO_ISFUNDECL (arg_info) = FALSE;
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    if (INFO_STUB_FUN_CHN (arg_info) != NULL) {
        MODULE_FUNS (arg_node) = TCappendFundef (INFO_STUB_FUN_CHN (arg_info),
                                                 MODULE_FUNS (arg_node));
        INFO_STUB_FUN_CHN (arg_info) = NULL;
    }

    MODULE_FUNS (arg_node) = MarkFundefsNotInlineCompleted (MODULE_FUNS (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EGFfundef (node *arg_node, info *arg_info)
 *
 * @brief here the main work happens. The function duplication and the
 *    modification of both, the original function and the duplicate.
 *    While the actual conversion of the duplicate into a GPU function
 *    is done via the traversal mechanism, the conversion of the 
 *    original into a stub is delegated entirely into the helper functions
 *    above through 'transformOldFundefIntoStub'.
 *
 *****************************************************************************/
node *
EGFfundef (node *arg_node, info *arg_info)
{
    node *new_fundef = NULL;
    node *stub_fundef = NULL;

    DBUG_ENTER ();
    
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    if (FUNDEF_ISGPUFUNCTION (arg_node) 
        && !(FUNDEF_ISWRAPPERFUN (arg_node))){
        DBUG_PRINT ("handling GPU function %s:", FUNDEF_NAME (arg_node));
        new_fundef = DUPdoDupNode (arg_node);
        // in case of an external function we need to transfer a possible
        // FUNDEF_LINKNAME from the old to the new fundef:
        FUNDEF_LINKNAME (new_fundef) = FUNDEF_LINKNAME (arg_node);
        FUNDEF_LINKNAME (arg_node) = NULL;

        INFO_NEWFUNDEF (arg_info) = new_fundef;

        // transform the copy into a real GPU function:
        if (INFO_ISFUNDECL (arg_info)) {
            // As we are dealing with an external function, all we need to do 
            // is to replace the types in the arguments and return types into
            // the corresponding device types.
            FUNDEF_ARGS (new_fundef) = TRAVopt (FUNDEF_ARGS (new_fundef), arg_info);
            FUNDEF_RETS (new_fundef) = TRAVopt (FUNDEF_RETS (new_fundef), arg_info);
        } else {
            DBUG_ASSERT (FALSE, "non-external GPU fundefs are not yet supported");
        }

        // transform the original into a stub function:
        arg_node = transformOldFundefIntoStub (arg_node, arg_info);
        DBUG_PRINT ("stub function generated:");
        DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node););

        // in case of externals, take it out and send it to MODULE_FUNS:
        if (INFO_ISFUNDECL (arg_info)) {
            stub_fundef = arg_node;
            arg_node = FUNDEF_NEXT (arg_node);
            FUNDEF_NEXT (stub_fundef) = INFO_STUB_FUN_CHN (arg_info);
            INFO_STUB_FUN_CHN (arg_info) = stub_fundef;
        }

        // insert the real GPU function here:
        FUNDEF_NEXT (new_fundef) = arg_node;
        arg_node = new_fundef;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EGFarg (node *arg_node, info *arg_info)
 *
 * @brief convert the argument types iff ISGPUMEM is set
 *
 *
 *****************************************************************************/
node *
EGFarg (node *arg_node, info *arg_info)
{
    ntype *type;
    DBUG_ENTER ();
    if (ARG_ISGPUMEM (arg_node)) {
        type = CUconvertHostToDeviceType (ARG_NTYPE (arg_node));
        ARG_NTYPE (arg_node) = TYfreeType (ARG_NTYPE (arg_node));
        ARG_NTYPE (arg_node) = type;
        ARG_ISGPUMEM (arg_node) = FALSE;
    }
    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EGFret (node *arg_node, info *arg_info)
 *
 * @brief convert the return types iff ISGPUMEM is set
 *
 *
 *****************************************************************************/
node *
EGFret (node *arg_node, info *arg_info)
{
    ntype *type;
    DBUG_ENTER ();
    if (RET_ISGPUMEM (arg_node)) {
        type = CUconvertHostToDeviceType (RET_TYPE (arg_node));
        RET_TYPE (arg_node) = TYfreeType (RET_TYPE (arg_node));
        RET_TYPE (arg_node) = type;
        RET_ISGPUMEM (arg_node) = FALSE;
    }
    RET_NEXT (arg_node) = TRAVopt (RET_NEXT (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}


/** <!--********************************************************************-->
 *
 * @fn node *EGFdoEmbedGpuFunctions (node *arg_node)
 *
 * @brief traversal entry function
 *
 *
 *****************************************************************************/
node *
EGFdoEmbedGpuFunctions (node *arg_node)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_egf);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
 /** @} */

