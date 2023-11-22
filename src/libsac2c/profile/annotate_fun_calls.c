/**
 * @file
 * @defgroup pf Annotate functions for profiling
 * @ingroup iprof
 *
 * @brief Add profling annotations to code
 *
 * This module implements the annotation of PROFILE_BEGIN and PROFILE_END
 * macros used for profiling function calls. It is implemented via the
 * insertion of  [ N_assign -> N_annotate ]  nodes before and after
 * each function call identified by N_ap nodes. For doing so, it relies
 * on the back refs AP_FUNDEF to exist. Therefore, it should not be
 * applied to the syntax tree before running type inference.
 * The main entry function is
 *
 * ~~~{.c}
 * node *PFdoProfileFunCalls( node *arg_node)
 * ~~~
 *
 * Some implementation remarks:
 *
 *   The implementation is fairly easy:
 *   Basically, we traverse all functions and whenever we meet an N_ap (PFap)
 *   node, we mark this within the arg_info node. This allows the function
 *   at the N_assign node (PFassign) to notice that we are dealing with a
 *   function application and to insert the [ N_assign -> N_annotate ]  nodes.
 *   The actual insertion into the "spine" is achieved implicitly via returning
 *   the freshly created node.
 *   However, for enumerating the functions in the order the will be called
 *   we do not traverse the functions in the order they are chained in the AST
 *   but we start traversing a function when encountering an application of
 *   that function (PFap) and we do NOT follow the NEXT pointer at N_fundef
 *   (PFfundef). In order to prevent from multiple traversals of functions
 *   we do not traverse the function body whenever the function has been
 *   enumerated already (PFfundef)!
 *
 * The usage of arg_info:
 *
 *   The arg_info node is used in a non stacking fashion here.
 *   In contains only one field INFO_PF_FUNDEF( arg_node)
 *   which is a pointer to the fundef of the function being
 *   actually called (within N_ap)!
 *
 * @{
 */
#include "annotate_fun_calls.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "str_buffer.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"
#include "ctinfo.h"
#include "globals.h"
#include "convert.h"
#include "new_types.h"

#define DBUG_PREFIX "PFFUN"
#include "debug.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *parent;
};

/*
 * INFO macros
 */
#define INFO_PF_FUNDEF(n) (n->fundef)
#define INFO_PF_PARENT(n) (n->parent)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_PF_FUNDEF (result) = NULL;
    INFO_PF_PARENT (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * @brief search for the main function
 *
 * runs through the fundef-chain given as argument and returns the pointer
 * to the main function. If no such function exists ( module / class),
 * NULL is returned.
 *
 * @param fundef N_fundef node
 * @return the N_fundef of function 'main'
 */
static node *
SearchMain (node *fundef)
{
    DBUG_ENTER ();

    while ((fundef != NULL)
           && ((FUNDEF_ISWRAPPERFUN (fundef) == TRUE)
               || (!STReq (FUNDEF_NAME (fundef), "main")))) {
        fundef = FUNDEF_NEXT (fundef);
    }

    DBUG_RETURN (fundef);
}

/**
 * @brief create a signature string for the profile output
 *
 * creates a profile name string from a given function definition. It's
 * length is limited to PF_MAXFUNNAMELEN-1 chars in order to guarantee
 * readable profiling output.
 *
 * @param fundef N_fundef node
 * @return String of function name and further profiling details
 */
static char *
Fundef2ProfileString (node *fundef)
{
    char *tmp_str, *result;
    str_buf *str_buff;
    node *arg;

    DBUG_ENTER ();
    str_buff = SBUFcreate (PF_MAXFUNNAMELEN - 1);
    str_buff = SBUFprintf (str_buff, "%s( ", FUNDEF_NAME (fundef));

    arg = FUNDEF_ARGS (fundef);
    while (arg != NULL) {
        tmp_str = TYtype2String (ARG_NTYPE (arg), FALSE, 0);
        str_buff = SBUFprint (str_buff, tmp_str);
        tmp_str = MEMfree (tmp_str);

        if (ARG_ISREFERENCE (arg) && !ARG_ISREADONLY (arg)) {
            str_buff = SBUFprint (str_buff, " &");
        } else if (ARG_ISREFERENCE (arg) && ARG_ISREADONLY (arg)) {
            str_buff = SBUFprint (str_buff, " (&)");
        } else {
            str_buff = SBUFprint (str_buff, " ");
        }

        if (ARG_NAME (arg) != NULL) {
            str_buff = SBUFprint (str_buff, ARG_NAME (arg));
        }

        arg = ARG_NEXT (arg);
        if (arg != NULL) {
            str_buff = SBUFprint (str_buff, ", ");
        }
    }
    str_buff = SBUFprint (str_buff, ")");

    tmp_str = SBUF2str (str_buff);
    str_buff = SBUFfree (str_buff);

    result = STRncpy (tmp_str, PF_MAXFUNNAMELEN);
    tmp_str = MEMfree (tmp_str);

    DBUG_RETURN (result);
}

/**
 * @brief create a fun type flag for N_annotate
 *
 * creates a mask for an N_annotatate node used for profiling function calls.
 * It reflects the functions properties such as
 *   -  being an inlining function,
 *   -  being a library function, or
 *   -  being specialized function.
 *
 * @param fundef N_fundef node
 * @return a funtypemask value
 */
static int
Fundef2FunTypeMask (node *fundef)
{
    int funtypemask = 0;

    DBUG_ENTER ();

    if (FUNDEF_ISINLINE (fundef)) {
        funtypemask = funtypemask | INL_FUN;
    }

    if (!FUNDEF_ISLOCAL (fundef)) {
        funtypemask = funtypemask | LIB_FUN;
    }

    DBUG_RETURN (funtypemask);
}

/**
 * @brief traverse N_fundef
 *
 * enumerates the function found (FUNDEF_FUNNO( arg_node) is set to a non-zero
 * value) and traverses into it's body, iff it has not yet been counted before.
 * In case there are more that PF_MAXFUN functions, all surplus functions wil
 * be given the number 1 - the number of "main".
 *
 * @param arg_node N_fundef
 * @param arg_info info struct
 * @return node
 */
node *
PFfundef (node *arg_node, info *arg_info)
{
    char *str_buff;
    ntype *wrappertype;
    node *mem_parent;

    DBUG_ENTER ();

    if (FUNDEF_FUNNO (arg_node) == 0) { /* this function is not yet counted! */
        str_buff = Fundef2ProfileString (arg_node);
        DBUG_PRINT ("annotating \"%s\"", str_buff);
        if (global.profile_funcntr == PF_MAXFUN) {
            CTIwarn (EMPTY_LOC, "\"PF_MAXFUN\" too low!\n"
                     "Function \"%s\" will not be profiled separately. "
                     "Instead, it's time will be accounted to \"main\"",
                     str_buff);
            FUNDEF_FUNNO (arg_node) = 1;
            str_buff = MEMfree (str_buff);
        } else {
            global.profile_funnme[global.profile_funcntr] = str_buff;
            FUNDEF_FUNNO (arg_node) = ++global.profile_funcntr;
        }
        if (FUNDEF_BODY (arg_node) != NULL) { /* library funs do have no body! */
            mem_parent = INFO_PF_PARENT (arg_info);
            INFO_PF_PARENT (arg_info) = arg_node;
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            INFO_PF_PARENT (arg_info) = mem_parent;

        } else if (FUNDEF_ISWRAPPERFUN (arg_node)) {
            wrappertype = FUNDEF_WRAPPERTYPE (arg_node);
            if (wrappertype == NULL) {
                DBUG_ASSERT (FUNDEF_WASUSED (arg_node),
                             "non-used wrapperfun w/o wrappertype found");
            } else if (TYisProd (wrappertype)) {
                DBUG_ASSERT (FUNDEF_IMPL (arg_node) != NULL,
                             "product wrapper type without IMPL found!");
                FUNDEF_IMPL (arg_node) = TRAVdo (FUNDEF_IMPL (arg_node), arg_info);
            } else {
                FUNDEF_WRAPPERTYPE (arg_node)
                  = TYmapFunctionInstances (wrappertype, PFfundef, arg_info);
            }
        }
    }

    /*
     * The next N_fundef node is not traversed here, since the traversal
     * follows the N_ap nodes in order to reflect the function invocation
     * tree! (cf. PFap)
     */

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_assign
 *
 * traverses the instruction in order to find out whether a function is called.
 * If that is the cases, INFO_PF_FUNDEF( arg_info) is set during that traversal
 * ( cf. PFap ). As a consequence, the actual N_asssign node is surrounded by
 * new N_assign nodes whose instructions are N_annotate nodes for indicating
 * PROFILING BEGIN and PROFILING END of the function being called.
 *
 * @param arg_node N_assign
 * @param arg_info info struct
 * @return node
 */
node *
PFassign (node *arg_node, info *arg_info)
{
    int funtypemask;
    int funap_cnt;
    size_t funno, parent_funno;
    node *old_next_assign, *res;
    char *str_buff;

    DBUG_ENTER ();
    /*
     * First, we traverse the instruction. If this turns out to be a function
     * application that requires the insertion of N_annotate's, this will
     * be signaled by INFO_PF_FUNDEF( arg_info) != NULL which points
     * to the fundef of the function being called.
     */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PF_FUNDEF (arg_info) != NULL) {
        /*
         * function enumeration starts with 1 (main) while
         * PF - data structure indexing starts with 0:
         */
        funno = FUNDEF_FUNNO (INFO_PF_FUNDEF (arg_info)) - 1;
        parent_funno = FUNDEF_FUNNO (INFO_PF_PARENT (arg_info)) - 1;

        /*
         * incrementing the application counter for the function
         * defined by INFO_PF_FUNDEF( arg_info)....
         */
        if (global.profile_funapcntr[funno] == PF_MAXFUNAP) {
            str_buff = Fundef2ProfileString (INFO_PF_FUNDEF (arg_info));
            CTIwarn (EMPTY_LOC, "\"PF_MAXFUNAP\" too low!\n"
                     "Application of function \"%s\" in line %zu will not "
                     "be profiled separately, but be accounted to the application "
                     "in line %zu",
                     str_buff, NODE_LINE (arg_node), global.profile_funapline[funno][0]);
            str_buff = MEMfree (str_buff);
            funap_cnt = 0;
        } else {
            funap_cnt = global.profile_funapcntr[funno]++;
            if (global.profile_funapcntr[funno] > global.profile_funapmax) {
                global.profile_funapmax = global.profile_funapcntr[funno];
            }
            global.profile_funapline[funno][funap_cnt] = NODE_LINE (arg_node);
            global.profile_parentfunno[funno][funap_cnt] = parent_funno;
        }

        /*
         * inserting the N_assign / N_annotate nodes....
         */
        funtypemask = Fundef2FunTypeMask (INFO_PF_FUNDEF (arg_info));
        res = TBmakeAssign (TBmakeAnnotate (funtypemask | CALL_FUN, funno, funap_cnt),
                            arg_node);
        old_next_assign = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node)
          = TBmakeAssign (TBmakeAnnotate (funtypemask | RETURN_FROM_FUN, funno,
                                          funap_cnt),
                          old_next_assign);

        INFO_PF_FUNDEF (arg_info) = NULL;  /* reset INFO_PF_FUNDEF ! */
        arg_node = ASSIGN_NEXT (arg_node); /* we may skip the N_annotate just inserted */
    } else {
        res = arg_node;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (res);
}

/**
 * @brief traverse N_ap
 *
 * Since we want to traverse the program following the invocation tree, we
 * traverse into the function called by using the back ref inserted during
 * type inference. AFTER doing so, we set INFO_PF_FUNDEF( arg_info) in
 * order to signal PFassign further up in the act. calling tree that we
 * do have a function call here.
 *
 * @param arg_node N_ap
 * @param arg_info info struct
 * @return node
 */
node *
PFap (node *arg_node, info *arg_info)
{
    node *fundef;

    DBUG_ENTER ();

    /*
     * First, we traverse the function being called here!
     */
    fundef = AP_FUNDEF (arg_node);
    fundef = TRAVdo (fundef, arg_info);

    /*
     * Then we insert the pointer to the function being called into
     * the arg_info node for later usage in PFassign.
     *
     * Do NOT place annotations around recursive calls in loop-funs
     * as it will break the flattening pass in fun2lac.
     * The flattening pass -and surely many other- expect quite a rigid
     * structure of loop-funs, namely a recursive application should
     * be the last assignment in an N_cond true-branch.
     */
    if (!AP_ISRECURSIVEDOFUNCALL (arg_node)) {
        INFO_PF_FUNDEF (arg_info) = fundef;
    }

    /*
     * Note here, that we do not need to traverse the arguments here
     * since this traversal is run after flattening the tree!
     */

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_with; skip entrying into N_assigns of N_with
 *        iff it is a CUDA kernel
 *
 * We currently do not support capturaing profiling information from within a
 * CUDA kernel. In theory this can be done, but would require somehow making
 * our global counters accessible on the CUDA device.
 *
 * @param arg_node N_with
 * @param arg_info info struct
 * @return node
 */
node *
PFwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!WITH_CUDARIZABLE (arg_node)) {
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
        WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);
    } else {
        DBUG_PRINT ("... skipping CUDA WL ...");
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief entry function into PF traversal
 *
 * traverses the syntax tree starting at the main function and insertes
 * N_assign -> N_annotate node before and after each function call which
 * in Print() will be translated into PROFILE-macros.
 * In principle, this traversal can be called anywhere provided that back
 * refs from N_ap nodes to function definitions exist. Therefore, it should
 * not be run before type checking. Furthermore, it comes without saying
 * that calling this function after transforming function applications
 * as for example it is done during inling is not a wise idea 8-)
 *
 * @param arg_node syntax tree
 * @return
 */
node *
PFdoProfileFunCalls (node *arg_node)
{
    trav_t traversaltable;
    info *info;
    node *main_fun;
    int i;

    DBUG_ENTER ();

    TRAVpush (TR_pf);

    info = MakeInfo ();

    /*
     * First, we initialize the application counter:
     */
    global.profile_funapcntr[0] = 1; /* main-function is called from outside world! */
    for (i = 1; i < PF_MAXFUN; i++) {
        global.profile_funapcntr[i] = 0;
    }

    /*
     * Now, we do traverse the program starting at the main function
     * if it exists; otherwise, we 're done.
     */
    DBUG_PRINT ("starting function annotation");
    if (MODULE_FUNS (arg_node) != NULL) {
        main_fun = SearchMain (MODULE_FUNS (arg_node));
        main_fun = TRAVopt(main_fun, info);
    }
    DBUG_PRINT ("function annotation done");

    info = FreeInfo (info);

    traversaltable = TRAVpop ();
    DBUG_ASSERT (traversaltable == TR_pf, "Popped incorrect traversal table");

    DBUG_RETURN (arg_node);
}

/** @} */

#undef DBUG_PREFIX
