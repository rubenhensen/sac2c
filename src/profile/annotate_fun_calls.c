/*
 *
 * $Log$
 * Revision 1.12  2004/11/25 11:57:33  skt
 * kicked ST_generic
 *
 * Revision 1.11  2004/11/25 01:01:07  skt
 * code brushing during SACDevCampDK 2k4
 *
 * Revision 1.10  2004/07/17 14:52:03  sah
 * switch to INFO structure
 * PHASE I
 *
 * Revision 1.9  2001/05/18 09:43:28  cg
 * MALLOC replaced by Malloc.
 *
 * Revision 1.8  2001/05/17 11:49:40  dkr
 * FREE eliminated
 *
 * Revision 1.7  2001/03/22 19:33:54  dkr
 * include of tree.h eliminated
 *
 * Revision 1.6  2001/03/15 20:39:48  dkr
 * Fundef2ProfileString: '&' for reference objects is no longer printed
 * by Type2String()
 *
 * Revision 1.5  2001/03/15 15:48:47  dkr
 * signature of Type2String modified
 *
 * Revision 1.2  2001/03/09 11:12:12  sbs
 * some typos corrected
 *
 * Revision 1.1  2001/03/09 11:08:07  sbs
 * Initial revision
 *
 */

/********************************************************************************
 ***
 *** General description:
 ***
 ***   This module implements the annotation of PROFILE_BEGIN and PROFILE_END
 ***   macros used for profiling function calls. It is implemented via the
 ***   insertion of  [ N_assign -> N_annotate ]  nodes before and after
 ***   each function call identified by N_ap nodes. For doing so, it relies
 ***   on the back refs AP_FUNDEF to exist. Therefore, it should not be
 ***   applied to the syntax tree before running type inference.
 ***   The main entry function is
 ***
 ***            node *PFdoProfileFunCalls( node *arg_node)   .
 ***
 ***
 *** Some implementation remarks:
 ***
 ***   The implementation is fairly easy:
 ***   Basically, we traverse all functions and whenever we meet an N_ap (PFap)
 ***   node, we mark this within the arg_info node. This allows the function
 ***   at the N_assign node (PFassign) to notice that we are dealing with a
 ***   function application and to insert the [ N_assign -> N_annotate ]  nodes.
 ***   The actual insertion into the "spine" is achieved implicitly via returning
 ***   the freshly created node.
 ***   However, for enumerating the functions in the order the will be called
 ***   we do not traverse the functions in the order they are chained in the AST
 ***   but we start traversing a function when encountering an application of
 ***   that function (PFap) and we do NOT follow the NEXT pointer at N_fundef
 ***   (PFfundef). In order to prevent from multiple traversals of functions
 ***   we do not traverse the function body whenever the function has been
 ***   enumerated already (PFfundef)!
 ***
 ***
 *** The usage of arg_info:
 ***
 ***   The arg_info node is used in a non stacking fashion here.
 ***   In contains only one field INFO_PF_FUNDEF( arg_node)
 ***   which is a pointer to the fundef of the function being
 ***   actually called (within N_ap)!
 ***
 ***/

#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"
#include "convert.h"
#include <string.h>
#include "Error.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
};

/*
 * INFO macros
 */
#define INFO_PF_FUNDEF(n) (n->fundef)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_PF_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/********************************************************************************
 ***
 *** Helper functions for the profiling traversal:
 ***
 ***  -  SearchMain           for finding the N_fundef of "main"
 ***  -  Fundef2ProfileString for creating a signature string for the profile
 ***                          output
 ***  -  Fundef2FunTypeMask   for creating a fun type flag for N_annotate
 ***/

/********************************************************************************
 *
 * function:
 *    node *SearchMain( node *fundef)
 *
 * description:
 *    runs through the fundef-chain given as argument and returns the pointer
 *    to the main function. If no such function exists ( module / class),
 *    NULL is returned.
 *
 ********************************************************************************/
static node *
SearchMain (node *fundef)
{
    DBUG_ENTER ("SearchMain");
    while ((fundef != NULL) && (strcmp (FUNDEF_NAME (fundef), "main") != 0)) {
        fundef = FUNDEF_NEXT (fundef);
    }
    DBUG_RETURN (fundef);
}

/********************************************************************************
 *
 * function:
 *    char *Fundef2ProfileString( node *fundef)
 *
 * description:
 *    creates a profile name string from a given function definition. It's
 *    length is limited to PF_MAXFUNNAMELEN-1 chars in order to guarantee
 *    readable profiling output.
 *
 ********************************************************************************/
static char *
Fundef2ProfileString (node *fundef)
{
    char *str_buff, *tmp_str;
    int str_spc = PF_MAXFUNNAMELEN - 1;
    node *arg;

    DBUG_ENTER ("Fundef2ProfileString");
    str_buff = (char *)ILIBmalloc (sizeof (char) * PF_MAXFUNNAMELEN);
    str_buff[0] = '\0';
    str_buff = strncpy (str_buff, FUNDEF_NAME (fundef), str_spc);
    str_spc -= strlen (FUNDEF_NAME (fundef));
    str_spc = MAX (str_spc, 0);
    str_buff = strncat (str_buff, "( ", str_spc);
    str_spc -= 2;
    str_spc = MAX (str_spc, 0);
    arg = FUNDEF_ARGS (fundef);
    while (arg != NULL) {
        tmp_str = CVtype2String (ARG_TYPE (arg), 0, TRUE);
        if (ARG_ISREFERENCE (arg) && !ARG_ISREADONLY (arg)) {
            tmp_str = strcat (tmp_str, " &");
        } else if (ARG_ISREFERENCE (arg) && ARG_ISREADONLY (arg)) {
            tmp_str = strcat (tmp_str, " (&)");
        } else {
            tmp_str = strcat (tmp_str, " ");
        }
        if (ARG_NAME (arg) != NULL) {
            tmp_str = strcat (tmp_str, ARG_NAME (arg));
        }
        str_buff = strncat (str_buff, tmp_str, str_spc);
        str_spc -= strlen (tmp_str);
        str_spc = MAX (str_spc, 0);
        tmp_str = ILIBfree (tmp_str);
        arg = ARG_NEXT (arg);
        if (arg != NULL) {
            str_buff = strncat (str_buff, ", ", str_spc);
            str_spc -= 2;
            str_spc = MAX (str_spc, 0);
        }
    }
    str_buff = strncat (str_buff, ")", str_spc);
    str_spc -= 2;
    str_spc = MAX (str_spc, 0);

    DBUG_RETURN (str_buff);
}

/********************************************************************************
 *
 * function:
 *    int Fundef2FunTypeMask( node *fundef)
 *
 * description:
 *    creates a mask for an N_annotatate node used for profiling function calls.
 *    It reflects the functions properties such as
 *      -  being an inlining function,
 *      -  being a library function, or
 *      -  being specialized function.
 *
 ********************************************************************************/
static int
Fundef2FunTypeMask (node *fundef)
{
    int funtypemask = 0;

    DBUG_ENTER ("Fundef2FunTypeMask");

    if (FUNDEF_ISINLINE (fundef)) {
        funtypemask = funtypemask | INL_FUN;
    }

    DBUG_RETURN (funtypemask);
}

/********************************************************************************
 ***
 *** Traversal functions used:
 ***
 ***  -  PFfundef
 ***  -  PFassign
 ***  -  PFap
 ***/

/********************************************************************************
 *
 * function:
 *    node *PFfundef( node *arg_node, info *arg_info)
 *
 * description:
 *    enumerates the function found (FUNDEF_FUNNO( arg_node) is set to a non-zero
 *    value) and traverses into it's body, iff it has not yet been counted before.
 *    In case there are more that PF_MAXFUN functions, all surplus functions wil
 *    be given the number 1 - the number of "main".
 *
 ********************************************************************************/

node *
PFfundef (node *arg_node, info *arg_info)
{
    char *str_buff;

    DBUG_ENTER ("PFfundef");
    if (FUNDEF_FUNNO (arg_node) == 0) { /* this function is not yet counted! */
        str_buff = Fundef2ProfileString (arg_node);
        DBUG_PRINT ("PFFUN", ("annotating \"%s\"", str_buff));
        if (global.profile_funcntr == PF_MAXFUN) {
            SYSWARN (("\"PF_MAXFUN\" too low"));
            CONT_WARN (("function \"%s\" will not be profiled separately!", str_buff));
            CONT_WARN (("Instead, it's time will be accounted to \"main\""));
            FUNDEF_FUNNO (arg_node) = 1;
            str_buff = ILIBfree (str_buff);
        } else {
            global.profile_funnme[global.profile_funcntr] = str_buff;
            FUNDEF_FUNNO (arg_node) = ++global.profile_funcntr;
        }
        if (FUNDEF_BODY (arg_node) != NULL) { /* library funs do have no body! */
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }
    }

    /*
     * The next N_fundef node is not traversed here, since the traversal
     * follows the N_ap nodes in order to reflect the function invocation
     * tree! (cf. PFap)
     */

    DBUG_RETURN (arg_node);
}

/********************************************************************************
 *
 * function:
 *    node *PFassign( node *arg_node, info *arg_info)
 *
 * description:
 *    traverses the instruction in order to find out whether a function is called.
 *    If that is the cases, INFO_PF_FUNDEF( arg_info) is set during that traversal
 *    ( cf. PFap ). As a consequence, the actual N_asssign node is surrounded by
 *    new N_assign nodes whose instructions are N_annotate nodes for indicating
 *    PROFILING BEGIN and PROFILING END of the function being called.
 *
 ********************************************************************************/

node *
PFassign (node *arg_node, info *arg_info)
{
    int funtypemask;
    int funno, funap_cnt;
    node *old_next_assign, *res;
    char *str_buff;

    DBUG_ENTER ("PFassign");
    /*
     * First, we traverse the instruction. If this turns out to be a function
     * application that requires the insertion of N_annotate's, this will
     * be signaled by INFO_PF_FUNDEF( arg_info) != NULL which points
     * to the fundef of the function being called.
     */
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PF_FUNDEF (arg_info) != NULL) {
        /*
         * function enumeration starts with 1 (main) while
         * PF - data structure indexing starts with 0:
         */
        funno = FUNDEF_FUNNO (INFO_PF_FUNDEF (arg_info)) - 1;

        /*
         * incrementing the application counter for the function
         * defined by INFO_PF_FUNDEF( arg_info)....
         */
        if (global.profile_funapcntr[funno] == PF_MAXFUNAP) {
            str_buff = Fundef2ProfileString (INFO_PF_FUNDEF (arg_info));
            SYSWARN (("\"PF_MAXFUNAP\" too low"));
            CONT_WARN (("application of function \"%s\" in line %d will not "
                        "be profiled separately but be accounted to the application "
                        "in line %d !",
                        str_buff, NODE_LINE (arg_node),
                        global.profile_funapline[funno][0]));
            str_buff = ILIBfree (str_buff);
            funap_cnt = 0;
        } else {
            funap_cnt = global.profile_funapcntr[funno]++;
            if (global.profile_funapcntr[funno] > global.profile_funapmax) {
                global.profile_funapmax = global.profile_funapcntr[funno];
            }
            global.profile_funapline[funno][funap_cnt] = NODE_LINE (arg_node);
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

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (res);
}

/********************************************************************************
 *
 * function:
 *    node *PFap( node *arg_node, info *arg_info)
 *
 * description:
 *    Since we want to traverse the program following the invocation tree, we
 *    traverse into the function called by using the back ref inserted during
 *    type inference. AFTER doing so, we set INFO_PF_FUNDEF( arg_info) in
 *    order to signal PFassign further up in the act. calling tree that we
 *    do have a function call here.
 *
 ********************************************************************************/

node *
PFap (node *arg_node, info *arg_info)
{
    node *fundef;

    DBUG_ENTER ("PFap");

    /*
     * First, we traverse the function being called here!
     */
    fundef = AP_FUNDEF (arg_node);
    fundef = TRAVdo (fundef, arg_info);

    /*
     * Then we insert the pointer to the function being called into
     * the arg_info node for later usage in PFassign.
     */
    INFO_PF_FUNDEF (arg_info) = fundef;

    /*
     * Note here, that we do not need to traverse the arguments here
     * since this traversal is run after flattening the tree!
     */

    DBUG_RETURN (arg_node);
}

/********************************************************************************
 *
 * function:
 *    node *ProfileFunCalls( node *arg_node)
 *
 * description:
 *    traverses the syntax tree starting at the main function and insertes
 *    N_assign -> N_annotate node before and after each function call which
 *    in Print() will be translated into PROFILE-macros.
 *    In principle, this traversal can be called anywhere provided that back
 *    refs from N_ap nodes to function definitions exist. Therefore, it should
 *    not be run before type checking. Furthermore, it comes without saying
 *    that calling this function after transforming function applications
 *    as for example it is done during inling is not a wise idea 8-)
 *
 ********************************************************************************/

node *
PFdoProfileFunCalls (node *arg_node)
{
    trav_t traversaltable;
    info *info;
    node *main_fun;
    int i;

    DBUG_ENTER ("ProfileFunCalls");

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
    DBUG_PRINT ("PFFUN", ("starting function annotation"));
    if (MODULE_FUNS (arg_node) != NULL) {
        main_fun = SearchMain (MODULE_FUNS (arg_node));
        if (main_fun != NULL) {
            main_fun = TRAVdo (main_fun, info);
        }
    }
    DBUG_PRINT ("PFFUN", ("function annotation done"));

    info = FreeInfo (info);

    traversaltable = TRAVpop ();
    DBUG_ASSERT ((traversaltable == TR_pf), "Popped incorrect traversal table");

    DBUG_RETURN (arg_node);
}
