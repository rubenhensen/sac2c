/*
 *
 * $Log$
 * Revision 3.29  2004/02/20 08:27:38  mwe
 * now functions with (MODUL_FUNS) and without (MODUL_FUNDECS) body are separated
 * changed tree traversal according to that
 *
 * Revision 3.28  2003/11/18 17:19:06  dkr
 * bug fixed: NWITHOP_DEFAULT may be NULL
 *
 * Revision 3.27  2003/11/18 16:59:09  dkr
 * NWITHOP_DEFAULT added
 *
 * Revision 3.26  2003/03/17 14:31:47  dkr
 * refcounting for NWITHID_IDS added
 *
 * Revision 3.25  2003/03/12 18:05:36  dkr
 * comment for RCicm() corrected
 *
 * Revision 3.24  2003/03/12 17:53:47  dkr
 * no modifications done
 *
 * Revision 3.23  2002/10/18 16:53:07  dkr
 * RCfundef(): inference of ST_Cfun corrected
 * (BTW, why is this infered here?)
 *
 * Revision 3.22  2002/09/06 09:37:37  dkr
 * ND_IDXS2OFFSET added
 *
 * Revision 3.21  2002/08/09 12:46:19  dkr
 * INFO_RC_... macros moved from tree_basic.h to refcount.c
 *
 * Revision 3.20  2002/08/08 09:16:31  dkr
 * some variables initialized to please the cc
 *
 * Revision 3.19  2002/07/24 15:06:22  dkr
 * bug in RCicm() fixed
 *
 * Revision 3.18  2002/07/24 13:19:39  dkr
 * macro MUST_... renamed
 *
 * Revision 3.17  2002/04/16 21:11:04  dkr
 * cpp-flag MAIN_HAS_MODNAME no longer needed
 *
 * Revision 3.16  2002/02/22 13:40:02  dkr
 * for cc: L_... access macros used on left-hand-sides
 *
 * Revision 3.15  2001/06/01 14:55:01  dkr
 * RCNwith() and RCNwith2() merged.
 * Bug in RCNwith() merged: neutral element of fold-with-loops is
 * refcounted correctly now
 *
 * Revision 3.14  2001/05/17 11:50:46  dkr
 * FREE eliminated
 *
 * Revision 3.13  2001/05/08 13:28:29  dkr
 * new RC macros used
 *
 * Revision 3.12  2001/03/22 19:42:16  dkr
 * include of tree.h eliminated
 *
 * Revision 3.11  2001/02/09 13:35:59  dkr
 * RCicm modified: arguments of VECT2OFFSET are refcounted correctly now
 *
 * Revision 3.10  2001/02/06 15:19:35  dkr
 * fixed a bug in RCNwith() and RCNwith2():
 * works correctly now even with INFO_RC_ONLYNAIVE set :-)
 *
 * Revision 3.8  2001/02/02 09:58:23  dkr
 * superfluous include of compile.h removed
 *
 * Revision 3.7  2000/12/15 18:24:59  dkr
 * infer_dfms.h renamed into InferDFMs.h
 *
 * Revision 3.6  2000/12/15 10:43:12  dkr
 * signature of InferDFMs() modified
 *
 * Revision 3.5  2000/12/12 17:14:50  dkr
 * L_VARDEC_OR_ARG... macros instead of VARDEC_OR_ARG... macros used on
 * LHS
 *
 * Revision 3.4  2000/12/12 15:31:27  dkr
 * call of InferDFMs added
 *
 * Revision 3.3  2000/12/06 20:14:34  dkr
 * minor changes done
 *
 * Revision 3.2  2000/12/01 18:19:05  dkr
 * no cc warning '... might be used uninitialized' anymore
 *
 * Revision 3.1  2000/11/20 18:01:35  sacbase
 * new release made
 *
 * ... [eliminated]
 *
 * Revision 1.1  1995/03/09  16:17:01  hw
 * Initial revision
 *
 */

#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "InferDFMs.h"
#include "generatemasks.h"
#include "optimize.h"
#include "refcount.h"

/* sorry, but there is no description of the refcouting available here ... */

/******************************************************************************
 *
 *  REMARKS TO NAIVE-REFCOUNTING
 *
 *  I added naive-refcounters into the code. These naive-refcounters will be
 *  used during spmd- and sync-optimizations (for data-flow-analysis).
 *  These naive-refcounters will also be changed during these optimizations,
 *  so these values have no value for any other part of the compiler.
 *                                                                        (jhs)
 *
 *  Properties of naive refcounters:
 *  - *Every variable* and *all variables* have to be naive-refcounted. If
 *    this is not done, you propably get broken assertions in spmd/sync-opt.
 *    That means MUST_NAIVEREFCOUNT has to be TRUE.
 *  - If you are able to break the first property someday, you have at least to
 *    keep the rule MUST_NAIVERECOUNT must be at least TRUE, when MUST_REFCOUNT
 *    is TRUE. I wrote all the code in this way, it is needed for some
 *    conditionals, if MUST_REFCOUNT(v) is true, you can do all the work for
 *    the naive ones to, otherwise it would be much more complicated.
 *  - Do not optimize the naive-reference-counting!!!
 *    The only things allowed are to handle certain contructions as funtions
 *    (like while-loops) and therefore count the variables in it with maxmum 1.
 *    Probaly spmd-/sync-opt needs to know about that and has to be updated
 *    if these are changed.
 *
 *  If you want to use naive-refcounters too:
 *    Inferre this values while inferring the actual naive-refcounters.
 *    You could also brush up the names, but keep the properties of the counters
 *    handed over to spmd/sync-opt.
 *
 *  If you want to write a new refcount system:
 *    Provide naive-refcounters with the same properties as described above.
 *
 ******************************************************************************/

/*
 * access macros for 'arg_info'
 */
#define INFO_RC_PRF(n) (n->node[1])
#define INFO_RC_WITH(n) (n->node[2])
#define INFO_RC_RCDUMP(n) ((int *)(n->node[3]))
#define INFO_RC_NAIVE_RCDUMP(n) ((int *)(n->node[4]))
#define INFO_RC_VARNO(n) (n->varno)
#define INFO_RC_ONLYNAIVE(n) (n->flag)

static int args_no;       /* number of arguments of current function */
static node *fundef_node; /* pointer to current function declaration */

/******************************************************************************
 *
 * Function:
 *   node *LookupId( char *id, node *id_chain)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
LookupId (char *id, node *id_chain)
{
    node *tmp, *ret_node = NULL;

    DBUG_ENTER ("LookupId");

    tmp = id_chain;
    while (NULL != tmp) {
        if (0 == strcmp (id, ID_NAME (EXPRS_EXPR (tmp)))) {
            ret_node = EXPRS_EXPR (tmp);
            break;
        } else {
            tmp = EXPRS_NEXT (tmp);
        }
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * Function:
 *   ids *DefinedIds( ids *arg_ids, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static ids *
DefinedIds (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("DefinedIds");

    if (!INFO_RC_ONLYNAIVE (arg_info)) {
        if (DECL_MUST_REFCOUNT (IDS_VARDEC (arg_ids))) {
            IDS_REFCNT (arg_ids) = VARDEC_OR_ARG_REFCNT (IDS_VARDEC (arg_ids));
            L_VARDEC_OR_ARG_REFCNT (IDS_VARDEC (arg_ids), 0);

            DBUG_PRINT ("RC", ("(standard) refcount of %s zeroed", IDS_NAME (arg_ids)));
        } else {
            IDS_REFCNT (arg_ids) = -1;
        }
    }
    if (DECL_MUST_NAIVEREFCOUNT (IDS_VARDEC (arg_ids))) {
        IDS_NAIVE_REFCNT (arg_ids) = VARDEC_OR_ARG_NAIVE_REFCNT (IDS_VARDEC (arg_ids));
        L_VARDEC_OR_ARG_NAIVE_REFCNT (IDS_VARDEC (arg_ids), 0);

        DBUG_PRINT ("RC", ("(naive) refcount of %s zeroed", IDS_NAME (arg_ids)));
    } else {
        IDS_NAIVE_REFCNT (arg_ids) = -1;
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * Function:
 *   node *DefinedId( node *arg_id, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
DefinedId (node *arg_id, node *arg_info)
{
    DBUG_ENTER ("DefinedId");

    DBUG_ASSERT ((NODE_TYPE (arg_id) == N_id), "no N_id node found!");

    if (!INFO_RC_ONLYNAIVE (arg_info)) {
        if (DECL_MUST_REFCOUNT (ID_VARDEC (arg_id))) {
            ID_REFCNT (arg_id) = VARDEC_OR_ARG_REFCNT (ID_VARDEC (arg_id));
            L_VARDEC_OR_ARG_REFCNT (ID_VARDEC (arg_id), 0);

            DBUG_PRINT ("RC", ("(standard) refcount of %s zeroed", ID_NAME (arg_id)));
        } else {
            ID_REFCNT (arg_id) = -1;
        }
    }
    if (DECL_MUST_NAIVEREFCOUNT (ID_VARDEC (arg_id))) {
        ID_NAIVE_REFCNT (arg_id) = VARDEC_OR_ARG_NAIVE_REFCNT (ID_VARDEC (arg_id));
        L_VARDEC_OR_ARG_NAIVE_REFCNT (ID_VARDEC (arg_id), 0);

        DBUG_PRINT ("RC", ("(naive) refcount of %s zeroed", ID_NAME (arg_id)));
    } else {
        ID_NAIVE_REFCNT (arg_id) = -1;
    }

    DBUG_RETURN (arg_id);
}

/******************************************************************************
 ******************************************************************************
 **
 **  DUMPS
 **
 **  Routines to handle dumps on (naive-)refcounters. This means the
 **  (naive-)refcounters of a whole function are dumped into an array,
 **  so that the values can be acces by the varno of each variable.
 **  The dumps for naive-refcounters and normal-refcounters have to be done
 **  seperately, but with the same functions, adjusting the parameter which:
 **    which == RC_REAL  => normal refcounters, either naive or optimized!
 **    which == RC_NAIVE => naive refcounters
 **
 ******************************************************************************
 ******************************************************************************/

#define RC_NAIVE 1
#define RC_REAL 0

/******************************************************************************
 *
 * function:
 *   int *AllocDump( int *dump)
 *
 * description:
 *   Initializes an array for dumping refcounts.
 *   If 'dump' is not NULL, the old memory is disposed. After that new memory
 *   is allocated.
 *
 ******************************************************************************/

static int *
AllocDump (int *dump, int varno)
{
    DBUG_ENTER (" AllocDump");

    if (dump != NULL) {
        Free (dump);
    }
    dump = (int *)Malloc (sizeof (int) * varno);

    DBUG_RETURN (dump);
}

/******************************************************************************
 *
 * function:
 *   void FreeDump( int *dump)
 *
 * description:
 *   Counterpart to AllocDump. Frees all memory used by the dump.
 *
 ******************************************************************************/

static void
FreeDump (int *dump)
{
    DBUG_ENTER (" FreeDump");

    dump = Free (dump);

    DBUG_VOID_RETURN;
}

#if 0
/******************************************************************************
 *
 * function:
 *   void InitRC( int n, node *arg_info)
 *
 * description:
 *   The refcounts in the vardecs are set to 'n' if refcount was active.
 *   Both real and naive refcounts are changed here!
 *
 ******************************************************************************/

static
void InitRC( int n, node *arg_info)
{
  node *vardec;

  DBUG_ENTER( "InitRC");

  FOREACH_VARDEC_AND_ARG( fundef_node, vardec,
    if (! INFO_RC_ONLYNAIVE( arg_info)) {
      if (VARDEC_OR_ARG_REFCNT( vardec) != RC_INACTIVE) {
        L_VARDEC_OR_ARG_REFCNT( vardec, n);
      }
    }
    if (VARDEC_OR_ARG_NAIVE_REFCNT( vardec) != RC_INACTIVE) {
      L_VARDEC_OR_ARG_NAIVE_REFCNT( vardec, n);
    }
  ) /* FOREACH_VARDEC_AND_ARG */

  DBUG_VOID_RETURN;
}
#endif

/******************************************************************************
 *
 * function:
 *   int *StoreRC(int which, int varno, node *arg_info)
 *
 * description:
 *   returns an array of int that contains the refcounters specified by
 *   which of the variable declaration.
 *     which == RC_REAL  => normal refcounters, either naive or optimized!
 *     which == RC_NAIVE => naive refcounters
 *   varno tells how many variables will be needed in this dump.
 *
 ******************************************************************************/

static int *
StoreRC (int which, int varno, node *arg_info)
{
    node *vardec;
    int k;
    int *dump = NULL;

    DBUG_ENTER ("StoreRC");

    dump = AllocDump (dump, varno);

    vardec = FUNDEF_ARGS (fundef_node);
    while (vardec != NULL) {
        if (which == RC_REAL) {
            dump[ARG_VARNO (vardec)] = ARG_REFCNT (vardec);
        } else {
            dump[ARG_VARNO (vardec)] = ARG_NAIVE_REFCNT (vardec);
        }
        vardec = ARG_NEXT (vardec);
    }
    vardec = FUNDEF_VARDEC (fundef_node);
    for (k = args_no; k < varno; k++) {
        if (k == VARDEC_VARNO (vardec)) {
            /*
             * 'vardec' is the right node belonging to 'k'.
             *   -> store the refcount.
             */
            if (which == RC_REAL) {
                dump[k] = VARDEC_REFCNT (vardec);
            } else {
                dump[k] = VARDEC_NAIVE_REFCNT (vardec);
            }
            vardec = VARDEC_NEXT (vardec);
        } else {
            /*
             * vardec belonging to 'k' was eliminated while optimisation.
             *  -> store -1 at this position.
             */
            dump[k] = -1;
        }
    }

    DBUG_RETURN (dump);
}

/******************************************************************************
 *
 * function:
 *   int *StoreAndInitRC( int which, int varno, int n, node *arg_info)
 *
 * description:
 *   Returns an array of int that contains the refcounts stored in the vardecs.
 *   The refcounts in the vardecs are set to 'n' if refcount was >= 1.
 *
 *   which speciefies which refcounters shall be handled
 *     which == RC_REAL  => normal refcounters, either naive or optimized!
 *     which == RC_NAIVE => naive refcounters
 *   varno tells how many variables will be needed in this dump.
 *
 *   depends on the global vars "fundef_node" and "args_no" !!
 *
 ******************************************************************************/

static int *
StoreAndInitRC (int which, int varno, int n, node *arg_info)
{
    node *argvardec;
    int k;
    int *dump = NULL;

    DBUG_ENTER ("StoreAndInitRC");

    dump = AllocDump (dump, varno);

    argvardec = FUNDEF_ARGS (fundef_node);
    while (argvardec != NULL) {
        if (which == RC_REAL) {
            dump[ARG_VARNO (argvardec)] = ARG_REFCNT (argvardec);
            if (RC_IS_VITAL (ARG_REFCNT (argvardec))) {
                ARG_REFCNT (argvardec) = n;
            }
        } else {
            dump[ARG_VARNO (argvardec)] = ARG_NAIVE_REFCNT (argvardec);
            if (RC_IS_VITAL (ARG_NAIVE_REFCNT (argvardec))) {
                ARG_NAIVE_REFCNT (argvardec) = n;
            }
        }
        argvardec = ARG_NEXT (argvardec);
    }
    argvardec = FUNDEF_VARDEC (fundef_node);
    for (k = args_no; k < varno; k++) {
        if (k == VARDEC_VARNO (argvardec)) {
            /*
             * 'argvardec' is the right node belonging to 'k'.
             *   -> store the refcount.
             */
            if (which == RC_REAL) {
                dump[k] = VARDEC_REFCNT (argvardec);
                if (!INFO_RC_ONLYNAIVE (arg_info)) {
                    if (RC_IS_VITAL (VARDEC_REFCNT (argvardec))) {
                        VARDEC_REFCNT (argvardec) = n;
                    }
                }
            } else {
                dump[k] = VARDEC_NAIVE_REFCNT (argvardec);
                if (RC_IS_VITAL (VARDEC_NAIVE_REFCNT (argvardec))) {
                    VARDEC_NAIVE_REFCNT (argvardec) = n;
                }
            }
            argvardec = VARDEC_NEXT (argvardec);
        } else {
            /*
             * vardec belonging to 'k' was eliminated while optimization.
             *  -> store -1 at this position.
             */
            dump[k] = -1;
        }
    }

    DBUG_RETURN (dump);
}

/******************************************************************************
 *
 * function:
 *   void RestoreRC( int which, int *dump, node *arg_info)
 *
 * description:
 *   copies dump-entries to refcounts of variable declaration
 *   'which' specifies which refcounters shall be handled
 *     which == RC_REAL  => normal refcounters, either naive or optimized!
 *     which == RC_NAIVE => naive refcounters
 *
 ******************************************************************************/

static void
RestoreRC (int which, int *dump, node *arg_info)
{
    node *vardec;

    DBUG_ENTER ("RestoreRC");

    FOREACH_VARDEC_AND_ARG (fundef_node, vardec,
                            if (which == RC_REAL) {
                                if (!INFO_RC_ONLYNAIVE (arg_info)) {
                                    L_VARDEC_OR_ARG_REFCNT (vardec,
                                                            dump[VARDEC_OR_ARG_VARNO (
                                                              vardec)]);
                                }
                            } else {
                                L_VARDEC_OR_ARG_NAIVE_REFCNT (vardec,
                                                              dump[VARDEC_OR_ARG_VARNO (
                                                                vardec)]);
                            }) /* FOREACH_VARDEC_AND_ARG */

    DBUG_VOID_RETURN;
}

/******************************************************************************
 ******************************************************************************
 **
 **  REFCOUNTING
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *Refcount( node *arg_node)
 *
 * description:
 *   starts the refcount-traversal (sets act_tab).
 *
 ******************************************************************************/

node *
Refcount (node *arg_node)
{
    node *arg_info;

    DBUG_ENTER ("Refcount");

    /*
     * generate masks
     */
    arg_node = InferDFMs (arg_node, HIDE_LOCALS_NEVER);
    arg_node = GenerateMasks (arg_node, NULL);

    act_tab = refcnt_tab;
    arg_info = MakeInfo ();
    INFO_RC_ONLYNAIVE (arg_info) = FALSE;

    if (N_modul == NODE_TYPE (arg_node)) {

        if (MODUL_FUNDECS (arg_node) != NULL) {

            DBUG_PRINT ("RC", ("starting with fundecs"));
            DBUG_ASSERT ((N_fundef == NODE_TYPE (MODUL_FUNDECS (arg_node))),
                         "wrong node ");
            MODUL_FUNDECS (arg_node) = Trav (MODUL_FUNDECS (arg_node), arg_info);
        }

        if (MODUL_FUNS (arg_node) != NULL) {

            DBUG_PRINT ("RC", ("starting with funs"));
            DBUG_ASSERT ((N_fundef == NODE_TYPE (MODUL_FUNS (arg_node))), "wrong node ");
            MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
        }

    } else {
        DBUG_ASSERT ((N_fundef == NODE_TYPE (arg_node)), "wrong node");
        arg_node = Trav (arg_node, arg_info);
    }

    arg_info = FreeTree (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCarg( node *arg_node, node *arg_info)
 *
 * description:
 *   Initializes ARG_REFCNT for refcounted and non-refcounted parameters with
 *   0 and (-1) respectively.
 *   This information is needed by compile.c to distinguish between refcounted
 *   and not refcounted parameters.
 *
 ******************************************************************************/

node *
RCarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCarg");

    /*
     *  Decides if the refcounters have to be normal-refcounted or not.
     */
    if (!INFO_RC_ONLYNAIVE (arg_info)) {
        if (DECL_MUST_REFCOUNT (arg_node)) {
            ARG_REFCNT (arg_node) = 0;
        } else {
            ARG_REFCNT (arg_node) = -1;
        }
    }
    /*
     *  Decides if the refcounters have to be normal-refcounted or not.
     */
    if (DECL_MUST_NAIVEREFCOUNT (arg_node)) {
        ARG_NAIVE_REFCNT (arg_node) = 0;
    } else {
        ARG_NAIVE_REFCNT (arg_node) = -1;
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCfundef( node *arg_node, node *arg_info)
 *
 * description:
 *   traverses body of function; sets 'varno', 'fundef_node' and 'arg_no'.
 *
 ******************************************************************************/

node *
RCfundef (node *arg_node, node *arg_info)
{
    node *arg;
    int old_info_rc_varno;

    DBUG_ENTER ("RCfundef");
    DBUG_PRINT ("RC",
                ("enter RCfundef, function name: %s", ((char *)(arg_node->mask[4]))));

    /*
     * special module name -> must be an external C-fun
     */
    if (((sbs == 1) && (strcmp (FUNDEF_MOD (arg_node), EXTERN_MOD_NAME) == 0))
        || ((sbs == 0) && (FUNDEF_MOD (arg_node) == NULL))) {
        FUNDEF_STATUS (arg_node) = ST_Cfun;
    }

    if (FUNDEF_ARGS (arg_node) != NULL) {
        /*
         *  The args are traversed to initialize ARG_REFCNT.
         *  Refcounted objects are initialized by 0 others by -1.
         *  This information is needed by compile.c.
         */
        DBUG_PRINT ("RC", ("traverse in args, function name: %s",
                           ((char *)(arg_node->mask[4]))));
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        /*
         *  setting some global variables to use with the 'mask'
         *  and storage of refcounts.
         *
         *  some of them are stored in arg_info, old values restored before
         */

        DBUG_PRINT ("RC", ("traverse in body, function name: %s",
                           ((char *)(arg_node->mask[4]))));

        old_info_rc_varno = INFO_RC_VARNO (arg_info);
        INFO_RC_VARNO (arg_info) = FUNDEF_VARNO (arg_node);
        fundef_node = arg_node;
        arg = FUNDEF_ARGS (arg_node);
        args_no = 0;
        while (NULL != arg) {
            args_no++;
            arg = ARG_NEXT (arg);
        }

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        /*
         *  restore values at arg_info
         */
        INFO_RC_VARNO (arg_info) = old_info_rc_varno;
    }

    if (NULL != FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCblock( node *arg_node, node *arg_info)
 *
 * description:
 *   performs the reference-counting for a N_block node:
 *     - traversal of vardecs first
 *         (initializes the refcounters of the vardecs)
 *     - then traversal of intructions
 *
 ******************************************************************************/

node *
RCblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_ASSERT ((BLOCK_INSTR (arg_node) != NULL), "first instruction of block is NULL"
                                                   " (should be a N_empty node)");

    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCvardec( node *arg_node, node *arg_info)
 *
 * description:
 *   initializes the (naive-)refcounters of a vardec:
 *      0, if vardec of a RC-object,
 *     -1, otherwise.
 *
 ******************************************************************************/

node *
RCvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCvardec");

    if (!INFO_RC_ONLYNAIVE (arg_info)) {
        if (DECL_MUST_REFCOUNT (arg_node)) {
            VARDEC_REFCNT (arg_node) = 0;
        } else {
            VARDEC_REFCNT (arg_node) = -1;
        }
    }
    if (DECL_MUST_NAIVEREFCOUNT (arg_node)) {
        VARDEC_NAIVE_REFCNT (arg_node) = 0;
    } else {
        VARDEC_NAIVE_REFCNT (arg_node) = -1;
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCassign( node *arg_node, node *arg_info)
 *
 * description:
 *   Performs bottom-up traversal for refcounting.
 *   When containing a with-loop, NWITH(2)_IN/INOUT/OUT/LOCAL are infered.
 *
 ******************************************************************************/

node *
RCassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCassign");

    /*
     * Bottom up traversal!!
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCloop(node *arg_node, node *arg_info)
 *
 * description:
 *
 * remarks:
 *   - v1: set of vars that are used before they will be defined in the body
 *         of the loop
 *   - v2: set of vars that are defined in the body of the loop and are used
 *         in the rest of the program (array-vars are only considered)
 *
 * attention:
 *   Uses for N_do *and* N_while resp do- *and* while-loops!!!
 *
 ******************************************************************************/

node *
RCloop (node *arg_node, node *arg_info)
{
    node *vardec;
    long *defined_mask, *used_mask;
    int i, naive_again = 0, again_ = 0, use_old_ = 0, naive_use_old = 0;
    int *ref_dump;
    int *naive_ref_dump;
    int *backup;
    ids *usevars_, *defvars_, *new_ids_;
    ids *naive_usevars;
    ids *naive_defvars;
    ids *naive_new_ids;

    int dumpcompare, refcompare, do_on_ids, do_on_normal, do_again, do_on_naive;
    int old_onlynaive; /* bool */

    DBUG_ENTER ("RCloop");

    if (NODE_TYPE (arg_node) == N_do) {
        DBUG_PRINT ("RCi", ("N_do cond"));
        DO_COND (arg_node) = Trav (DO_COND (arg_node), arg_info);
    }
    if (NODE_TYPE (arg_node) == N_while) {
        DBUG_PRINT ("RCi", ("N_while cond"));
        WHILE_COND (arg_node) = Trav (WHILE_COND (arg_node), arg_info);
    }

    /*
     *  Store the current refcounters from the VARDECs to (naive_)ref_dump and
     *  initialize all refcounters from the function's vardec with 1.
     */
    ref_dump = StoreAndInitRC (RC_REAL, INFO_RC_VARNO (arg_info), 1, arg_info);
    naive_ref_dump = StoreAndInitRC (RC_NAIVE, INFO_RC_VARNO (arg_info), 1, arg_info);

    /* traverse body of loop */
    DBUG_PRINT ("RC", ("line: %d : entering body:", NODE_LINE (arg_node)));
    L_DO_OR_WHILE_BODY (arg_node, Trav (DO_OR_WHILE_BODY (arg_node), arg_info));
    DBUG_PRINT ("RC", ("line: %d : body finished:", NODE_LINE (arg_node)));

    usevars_ = DO_OR_WHILE_USEVARS (arg_node);
    naive_usevars = DO_OR_WHILE_NAIVE_USEVARS (arg_node);
    defvars_ = DO_OR_WHILE_DEFVARS (arg_node);
    naive_defvars = DO_OR_WHILE_NAIVE_DEFVARS (arg_node);

    if ((usevars_ != NULL) || (defvars_ != NULL)) {
        use_old_ = 1;
    }
    if ((naive_usevars != NULL) || (naive_defvars != NULL)) {
        naive_use_old = 1;
    }

    /* masks of defines and used variables */
    defined_mask = BLOCK_MASK (DO_OR_WHILE_BODY (arg_node), 0);
    used_mask = BLOCK_MASK (DO_OR_WHILE_BODY (arg_node), 1);

    /* first compute sets defvars and usevars */
    for (i = 0; i < INFO_RC_VARNO (arg_info); i++) {
        if ((defined_mask[i] > 0) || (used_mask[i] > 0)) {
            vardec = FindVardec_Varno (i, fundef_node);
            DBUG_ASSERT ((NULL != vardec), "variable not found");

            DBUG_PRINT ("RCi", ("i=%i; defined[i]=%i; used[i]=%i", i, defined_mask[i],
                                used_mask[i]));

            /*
             *  Depending on which refcounters have to be done here normal and naive
             *  ones, only naive-ones, or none. The following is only done in the
             *  first two cases. We setup the values needed for some comparisons.
             */
            if (DECL_MUST_REFCOUNT (vardec)) {
                DBUG_PRINT ("RC", ("do_on_normal %i", i));
                do_on_ids = TRUE;
                do_on_normal = (!INFO_RC_ONLYNAIVE (arg_info));
                do_on_naive = TRUE;
                dumpcompare = ref_dump[i];
                refcompare = VARDEC_OR_ARG_REFCNT (vardec);
            } else if (DECL_MUST_NAIVEREFCOUNT (vardec)) {
                DBUG_PRINT ("RC", ("do_on_naive %i", i));
                do_on_ids = TRUE;
                do_on_normal = FALSE;
                do_on_naive = TRUE;
                dumpcompare = naive_ref_dump[i];
                refcompare = VARDEC_OR_ARG_NAIVE_REFCNT (vardec);
            } else {
                do_on_ids = FALSE;
                do_on_normal = FALSE;
                dumpcompare = (-1);
                refcompare = (-1);
            }

            /*
             * SBS: here we disable naive refcounting!!! The reason for doing
             * so is that the compilation of jfe's COPYREC wouldn't work!!
             * DBUG_ASSERT(..., "var not found") would fail 8-(((
             */
            do_on_naive = FALSE;

            DBUG_PRINT ("RCi", ("rc %i; nrc %i", ref_dump[i], naive_ref_dump[i]));
            if (do_on_ids) {

                /* first store used (usevars) and defined (defvars) variables  */
#if 0
        if ((defined_mask[i] > 0) && (dumpcompare > 0)) {
#endif
                /*
                 *  store refcount of defined variables in defvars
                 */
                if (do_on_normal) {
                    if ((defined_mask[i] > 0) && (ref_dump[i] > 0)) {
                        if (0 == use_old_) {
                            new_ids_ = MakeIds_Copy (VARDEC_OR_ARG_NAME (vardec));
                            IDS_REFCNT (new_ids_) = ref_dump[i];
                            IDS_NAIVE_REFCNT (new_ids_) = naive_ref_dump[i];
                            IDS_VARDEC (new_ids_) = vardec;
                            IDS_NEXT (new_ids_) = defvars_;
                            defvars_ = new_ids_;
                            /* insert 'new_ids' at the begining of 'defvars' */
                            DBUG_PRINT ("RC", ("store defined var (defvars) %s:%d::%d",
                                               IDS_NAME (new_ids_), IDS_REFCNT (new_ids_),
                                               IDS_NAIVE_REFCNT (new_ids_)));
                        } else {
                            new_ids_ = LookupIds (VARDEC_OR_ARG_NAME (vardec), defvars_);
                            DBUG_ASSERT ((NULL != new_ids_), "var not found");
                            IDS_REFCNT (new_ids_) = ref_dump[i];
                            IDS_NAIVE_REFCNT (new_ids_) = naive_ref_dump[i];
                            DBUG_PRINT ("RC", ("changed defined var (defvars) %s:%d::%d",
                                               IDS_NAME (new_ids_), IDS_REFCNT (new_ids_),
                                               IDS_NAIVE_REFCNT (new_ids_)));
                        }
                    }
                }
                if (do_on_naive) {
                    if ((defined_mask[i] > 0) && (naive_ref_dump[i] > 0)) {
                        if (naive_use_old == 0) {
                            naive_new_ids = MakeIds_Copy (VARDEC_OR_ARG_NAME (vardec));
                            IDS_REFCNT (naive_new_ids) = ref_dump[i];
                            IDS_NAIVE_REFCNT (naive_new_ids) = naive_ref_dump[i];
                            IDS_VARDEC (naive_new_ids) = vardec;
                            IDS_NEXT (naive_new_ids) = naive_defvars;
                            naive_defvars = naive_new_ids;
                            DBUG_PRINT ("RC",
                                        ("store defined var (naive_defvars) %s:%d::%d",
                                         IDS_NAME (naive_new_ids),
                                         IDS_REFCNT (naive_new_ids),
                                         IDS_NAIVE_REFCNT (naive_new_ids)));
                        } else {
                            naive_new_ids
                              = LookupIds (VARDEC_OR_ARG_NAME (vardec), naive_defvars);
                            DBUG_ASSERT ((NULL != naive_new_ids), "var not found");
                            IDS_REFCNT (naive_new_ids) = ref_dump[i];
                            IDS_NAIVE_REFCNT (naive_new_ids) = naive_ref_dump[i];
                            DBUG_PRINT ("RC",
                                        ("changed defined var (naive_defvars) %s:%d::%d",
                                         IDS_NAME (naive_new_ids),
                                         IDS_REFCNT (naive_new_ids),
                                         IDS_NAIVE_REFCNT (naive_new_ids)));
                        }
                    }
                }
                if ((used_mask[i] > 0) && (refcompare > 0)) {
                    /*
                     *  store refcount of used variables in usevars
                     */
                    if (0 == naive_use_old) {
                        naive_new_ids = MakeIds_Copy (VARDEC_OR_ARG_NAME (vardec));
                        IDS_REFCNT (naive_new_ids) = VARDEC_OR_ARG_REFCNT (vardec);
                        IDS_NAIVE_REFCNT (naive_new_ids)
                          = VARDEC_OR_ARG_NAIVE_REFCNT (vardec);
                        IDS_VARDEC (naive_new_ids) = vardec;
                        IDS_NEXT (naive_new_ids) = naive_usevars;
                        naive_usevars = naive_new_ids;
                        /* insert 'new_ids' at the begining of 'usevars' */
                        DBUG_PRINT ("RC",
                                    ("store used var (naive_usevars) %s:%d::%d",
                                     IDS_NAME (naive_new_ids), IDS_REFCNT (naive_new_ids),
                                     IDS_NAIVE_REFCNT (naive_new_ids)));
                    } else {
                        naive_new_ids
                          = LookupIds (VARDEC_OR_ARG_NAME (vardec), naive_usevars);
                        DBUG_ASSERT ((NULL != naive_new_ids), "var not found");
                        IDS_REFCNT (naive_new_ids) = VARDEC_OR_ARG_REFCNT (vardec);
                        IDS_NAIVE_REFCNT (naive_new_ids)
                          = VARDEC_OR_ARG_NAIVE_REFCNT (vardec);
                        DBUG_PRINT ("RC",
                                    ("changed used var (naive_usevars) %s:%d::%d",
                                     IDS_NAME (naive_new_ids), IDS_REFCNT (naive_new_ids),
                                     IDS_NAIVE_REFCNT (naive_new_ids)));
                    }
                    if (use_old_ == 0) {
                        if (do_on_normal) {
                            new_ids_ = MakeIds_Copy (VARDEC_OR_ARG_NAME (vardec));
                            IDS_REFCNT (new_ids_) = VARDEC_OR_ARG_REFCNT (vardec);
                            IDS_NAIVE_REFCNT (new_ids_)
                              = VARDEC_OR_ARG_NAIVE_REFCNT (vardec);
                            IDS_VARDEC (new_ids_) = vardec;
                            IDS_NEXT (new_ids_) = usevars_;
                            usevars_ = new_ids_;
                            /* insert 'new_ids' at the begining of 'usevars' */
                            DBUG_PRINT ("RC", ("store used var (usevars) %s:%d::%d",
                                               IDS_NAME (new_ids_), IDS_REFCNT (new_ids_),
                                               IDS_NAIVE_REFCNT (new_ids_)));
                        }
                    } else {
                        if (do_on_normal) {
                            new_ids_ = LookupIds (VARDEC_OR_ARG_NAME (vardec), usevars_);
                            DBUG_ASSERT ((NULL != new_ids_), "var not found");
                            IDS_REFCNT (new_ids_) = VARDEC_OR_ARG_REFCNT (vardec);
                            IDS_NAIVE_REFCNT (new_ids_)
                              = VARDEC_OR_ARG_NAIVE_REFCNT (vardec);
                            DBUG_PRINT ("RC", ("changed used var (usevars) %s:%d::%d",
                                               IDS_NAME (new_ids_), IDS_REFCNT (new_ids_),
                                               IDS_NAIVE_REFCNT (new_ids_)));
                        }
                    }
                }
            }
        }
    } /* for i */

    if (NULL != naive_usevars) {
        naive_new_ids = naive_usevars;
        while ((0 == naive_again) && (NULL != naive_new_ids)) {
            if ((RC_IS_VITAL (VARDEC_OR_ARG_NAIVE_REFCNT (IDS_VARDEC (naive_new_ids))))
                && (RC_IS_ZERO (
                     naive_ref_dump[VARDEC_OR_ARG_VARNO (IDS_VARDEC (naive_new_ids))]))) {
                naive_again = 1;
            } else {
                naive_new_ids = IDS_NEXT (naive_new_ids);
            }
        } /* while */
    }
    if (!INFO_RC_ONLYNAIVE (arg_info)) {
        if (NULL != usevars_) {
            new_ids_ = usevars_;
            while ((0 == again_) && (NULL != new_ids_)) {
                if ((RC_IS_VITAL (VARDEC_OR_ARG_REFCNT (IDS_VARDEC (new_ids_))))
                    && (RC_IS_ZERO (
                         ref_dump[VARDEC_OR_ARG_VARNO (IDS_VARDEC (new_ids_))]))) {
                    again_ = 1;
                } else {
                    new_ids_ = IDS_NEXT (new_ids_);
                }
            } /* while */
        }
    }

    DBUG_PRINT ("RCi", ("again???"));
    if (again_) {
        DBUG_PRINT ("RC", ("while loop again"));
        RestoreRC (RC_REAL, ref_dump, arg_info);
        RestoreRC (RC_NAIVE, naive_ref_dump, arg_info);
        FreeDump (ref_dump);
        FreeDump (naive_ref_dump);
        ref_dump = StoreAndInitRC (RC_REAL, INFO_RC_VARNO (arg_info), 1, arg_info);
        naive_ref_dump = StoreAndInitRC (RC_NAIVE, INFO_RC_VARNO (arg_info), 1, arg_info);
        /* init all variables that are member of usevars with refcount 1 */
        if (again_) {
            DBUG_PRINT ("RCi", ("again"));
            new_ids_ = usevars_;
            while (NULL != new_ids_) {
                L_VARDEC_OR_ARG_REFCNT (IDS_VARDEC (new_ids_), 1);
                /* L_VARDEC_OR_ARG_NAIVE_REFCNT( IDS_VARDEC( new_ids_), 1); */
                new_ids_ = IDS_NEXT (new_ids_);
            }
        }
        if (naive_again) {
            DBUG_PRINT ("RCi", ("naive_again"));
            naive_new_ids = naive_usevars;
            while (NULL != naive_new_ids) {
                /* L_VARDEC_OR_ARG_REFCNT( IDS_VARDEC( naive_new_ids), 1); */
                L_VARDEC_OR_ARG_NAIVE_REFCNT (IDS_VARDEC (naive_new_ids), 1);
                naive_new_ids = IDS_NEXT (naive_new_ids);
            }
        }

        /* refcount body of while loop again */
        L_DO_OR_WHILE_BODY (arg_node, Trav (DO_OR_WHILE_BODY (arg_node), arg_info));
        /*
         * traversing the loop body may change its masks!
         * Therefore, the local pointers have to be updated accordingly:
         */
        defined_mask = BLOCK_MASK (DO_OR_WHILE_BODY (arg_node), 0);
        used_mask = BLOCK_MASK (DO_OR_WHILE_BODY (arg_node), 1);
    } else if (naive_again) {
        DBUG_PRINT ("RCi", ("naive_again"));
        RestoreRC (RC_NAIVE, naive_ref_dump, arg_info);
        FreeDump (naive_ref_dump);
        naive_ref_dump = StoreAndInitRC (RC_NAIVE, INFO_RC_VARNO (arg_info), 1, arg_info);

        backup = StoreRC (RC_REAL, INFO_RC_VARNO (arg_info), arg_info);

        naive_new_ids = naive_usevars;
        while (NULL != naive_new_ids) {
            L_VARDEC_OR_ARG_NAIVE_REFCNT (IDS_VARDEC (naive_new_ids), 1);
            naive_new_ids = IDS_NEXT (naive_new_ids);
        }

        /* naive-refcount body of while loop again */
        old_onlynaive = INFO_RC_ONLYNAIVE (arg_info);
        INFO_RC_ONLYNAIVE (arg_info) = TRUE;
        L_DO_OR_WHILE_BODY (arg_node, Trav (DO_OR_WHILE_BODY (arg_node), arg_info));
        INFO_RC_ONLYNAIVE (arg_info) = old_onlynaive;
        /*
         * traversing the loop body may change its masks!
         * Therefore, the local pointers have to be updated accordingly:
         */
        defined_mask = BLOCK_MASK (DO_OR_WHILE_BODY (arg_node), 0);
        used_mask = BLOCK_MASK (DO_OR_WHILE_BODY (arg_node), 1);

        RestoreRC (RC_REAL, backup, arg_info);
        FreeDump (backup);
    }

    /* compute new refcounts because of 'virtual function application' */
    for (i = 0; i < INFO_RC_VARNO (arg_info); i++) {
        if ((defined_mask[i] > 0) || (used_mask[i] > 0)) {
            vardec = FindVardec_Varno (i, fundef_node);
            DBUG_ASSERT ((NULL != vardec), "variable not found");

            /*
             * Depending on which refcounters have to be done here normal and naive
             * ones, only naive-ones, or none. The following is only done in the
             * first two cases. We setup the values needed for some comparisons.
             * here it is necessary to know what is counted here, so we also set
             * a value, if some things are to do on normal-refcounters too (everything
             * is done on the naive refcounters without further switches).
             */
            if (DECL_MUST_REFCOUNT (vardec)) {
                do_on_ids = TRUE;
                do_on_normal = (!INFO_RC_ONLYNAIVE (arg_info));
                do_again = again_;
                refcompare = VARDEC_OR_ARG_REFCNT (vardec);
                dumpcompare = ref_dump[i];
            } else if (DECL_MUST_NAIVEREFCOUNT (vardec)) {
                do_on_ids = TRUE;
                do_on_normal = FALSE;
                do_again = naive_again;
                refcompare = VARDEC_OR_ARG_NAIVE_REFCNT (vardec);
                dumpcompare = naive_ref_dump[i];
            } else {
                do_on_ids = FALSE;
                do_on_normal = FALSE;
                do_again = FALSE;
                refcompare = (-1);
                dumpcompare = (-1);
            }
            if (do_on_ids) {
                if ((used_mask[i] > 0) && (refcompare > 0) && (do_again)) {
                    /*
                     * update refcount of used variables (v1)
                     */
                    naive_new_ids
                      = LookupIds (VARDEC_OR_ARG_NAME (vardec), naive_usevars);
                    DBUG_ASSERT ((NULL != naive_new_ids), "var not found");
                    IDS_REFCNT (naive_new_ids) = VARDEC_OR_ARG_REFCNT (vardec);
                    IDS_NAIVE_REFCNT (naive_new_ids)
                      = VARDEC_OR_ARG_NAIVE_REFCNT (vardec);
                    DBUG_PRINT ("RC",
                                ("(naive_usevars) %s:%d::%d", IDS_NAME (naive_new_ids),
                                 IDS_REFCNT (naive_new_ids),
                                 IDS_NAIVE_REFCNT (naive_new_ids)));
                    if (do_on_normal) {
                        new_ids_ = LookupIds (VARDEC_OR_ARG_NAME (vardec), usevars_);
                        DBUG_ASSERT ((NULL != new_ids_), "var not found");
                        IDS_REFCNT (new_ids_) = VARDEC_OR_ARG_REFCNT (vardec);
                        IDS_NAIVE_REFCNT (new_ids_) = VARDEC_OR_ARG_NAIVE_REFCNT (vardec);
                        DBUG_PRINT ("RC",
                                    ("(usevars) %s:%d::%d", IDS_NAME (new_ids_),
                                     IDS_REFCNT (new_ids_), IDS_NAIVE_REFCNT (new_ids_)));
                    }
                }
                /*
                 * now compute new refcounts, because of 'virtual function application'
                 */
                if ((defined_mask[i] > 0) && (dumpcompare > 0)) {
                    /*
                     *  these will be the return-values of the virtual function
                     */
                    if ((N_do == NODE_TYPE (arg_node)) && (0 == refcompare)) {
                        /*
                         * Current variable is an argument of the virtual function and
                         * so it will be defined before it will be used, so we don't need
                         * it as argument of the virtual function (it can be freed
                         * earlier).
                         */
                        if (do_on_normal) {
                            ref_dump[i] = 0;
                        }
                        naive_ref_dump[i] = 0;
                    } else {
                        if (do_on_normal) {
                            ref_dump[i] = 1;
                        }
                        naive_ref_dump[i] = 1;
                    }

                    DBUG_PRINT ("RC", ("set refcount of %s(%d) to: %d::%d",
                                       VARDEC_OR_ARG_NAME (vardec), i, ref_dump[i],
                                       naive_ref_dump[i]));
                } else {
                    if ((used_mask[i] > 0) && (0 < refcompare)) {
                        /* these variables are arguments of the virtual function */
                        if (do_on_normal) {
                            ref_dump[i]++;
                        }
                        naive_ref_dump[i]++;
                        DBUG_PRINT ("RC", ("increased refcount of %s(%d) to: %d::%d",
                                           VARDEC_OR_ARG_NAME (vardec), i, ref_dump[i],
                                           naive_ref_dump[i]));
                    }
                }
            }
        }
    }

    /* store new_info for use while compilation */
    L_DO_OR_WHILE_USEVARS (arg_node, usevars_);
    L_DO_OR_WHILE_NAIVE_USEVARS (arg_node, naive_usevars);
    L_DO_OR_WHILE_DEFVARS (arg_node, defvars_);
    L_DO_OR_WHILE_NAIVE_DEFVARS (arg_node, naive_defvars);

    /* restore old vardec refcounts */
    RestoreRC (RC_REAL, ref_dump, arg_info);
    RestoreRC (RC_NAIVE, naive_ref_dump, arg_info);
    ref_dump = Free (ref_dump);
    naive_ref_dump = Free (naive_ref_dump);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCprf( node *arg_node, node *arg_info)
 *
 * description:
 *   Traverse the args with INFO_RC_PRF( arg_info) pointing to the N_prf!
 *   This is done for RCO in RCid().
 *   If N_prf is F_reshape it has to be treated in exactly the same way as if
 *    we had a simple assignment, i.e., traverse with (INFO_RC_PRF( arg_info)
 *    == NULL)!
 *
 ******************************************************************************/

node *
RCprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCprf");

    if (PRF_PRF (arg_node) == F_reshape) {
        INFO_RC_PRF (arg_info) = NULL;
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
    } else {
        INFO_RC_PRF (arg_info) = arg_node;
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        INFO_RC_PRF (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCicm( node *arg_node, node *arg_info)
 *
 * Description:
 *   Traverses the args with INFO_RC_PRF( arg_info) pointing to the N_icm!
 *   This is done for RCO in RCid().
 *
 ******************************************************************************/

node *
RCicm (node *arg_node, node *arg_info)
{
    char *name;

    DBUG_ENTER ("RCicm");

    name = ICM_NAME (arg_node);

    if (strstr (name, "USE_GENVAR_OFFSET") != NULL) {
        /*
         * USE_GENVAR_OFFSET( off_nt, wl_nt)
         * does *not* consume its arguments! It is expanded to
         *      off_nt = wl_nt__off    ,
         * where 'off_nt' is a scalar and 'wl_nt__off' an internal variable!
         *   -> store actual RC of the first argument (defined)
         *   -> do NOT traverse the second argument (used)
         */
        ICM_ARG1 (arg_node) = DefinedId (ICM_ARG1 (arg_node), arg_info);

        INFO_RC_PRF (arg_info) = NULL;
    } else if (strstr (name, "VECT2OFFSET") != NULL) {
        /*
         * VECT2OFFSET( off_nt, ., from_nt, ., ., ...)
         * needs RC on all but the first argument. It is expanded to
         *     off_nt = ... from_nt ...    ,
         * where 'off_nt' is a scalar variable.
         *  -> store actual RC of the first argument (defined)
         *  -> traverse all but the first argument (used)
         *  -> handle ICM like a prf (RCO)
         */
        ICM_ARG1 (arg_node) = DefinedId (ICM_ARG1 (arg_node), arg_info);

        INFO_RC_PRF (arg_info) = arg_node;
        ICM_EXPRS2 (arg_node) = Trav (ICM_EXPRS2 (arg_node), arg_info);
        INFO_RC_PRF (arg_info) = NULL;
    } else if (strstr (name, "IDXS2OFFSET") != NULL) {
        /*
         * IDXS2OFFSET( off_nt, ., idx_1_nt ... idx_n_nt, ., ...)
         * needs RC on all but the first argument. It is expanded to
         *     off_nt = ... idx_1_nt[i] ... idx_n_nt[i] ...   ,
         * where 'off_nt' is a scalar variable.
         *  -> store actual RC of the first argument (defined)
         *  -> traverse all but the first argument (used)
         *  -> handle ICM like a prf (RCO)
         */
        ICM_ARG1 (arg_node) = DefinedId (ICM_ARG1 (arg_node), arg_info);

        INFO_RC_PRF (arg_info) = arg_node;
        ICM_EXPRS2 (arg_node) = Trav (ICM_EXPRS2 (arg_node), arg_info);
        INFO_RC_PRF (arg_info) = NULL;
    } else {
        DBUG_ASSERT ((0), "unknown ICM found during RC");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCid( node *arg_node, node *arg_info)
 *
 * description:
 *   Depending on 'INFO_RC_PRF( arg_info)' do one of the following:
 *     a) Increment refcnt at vardec and set local refcnt of the N_id node
 *        accordingly. (RC op needed)
 *     b) Set local refcnt to -1. (no RC op)
 *   If we have found the index-vector of a with-loop, we set RC of
 *    'NWITH(2)_VEC' to 1 (-1 normally, see RCNwithid) --- to indicate, that we
 *    must build the index-vector in the compilat of the with-loop.
 *
 * remarks:
 *   - ('INFO_RC_PRF( arg_info)' != NULL) indicates that this N_id node is
 *       argument of a primitive function (exept F_reshape) or ICM.
 *     'INFO_RC_PRF' then points to this prf/icm.
 *   - 'INFO_RC_WITH( arg_info)' contains a N_exprs-chain with all parent
 *       WL-nodes.
 *
 ******************************************************************************/

node *
RCid (node *arg_node, node *arg_info)
{
    node *wl_node;

    DBUG_ENTER ("RCid");

    if (!INFO_RC_ONLYNAIVE (arg_info)) {
        if (DECL_MUST_REFCOUNT (ID_VARDEC (arg_node))) {
            if ((INFO_RC_PRF (arg_info) == NULL)
                || (RC_IS_ZERO (VARDEC_OR_ARG_REFCNT (ID_VARDEC (arg_node))))
                || (!(optimize & OPT_RCO))) {
                /*
                 * This N_id node either is *not* an argument of a primitive
                 *  function, or it is the last usage within the body of the
                 *  current function. (or refcount-optimization is turned off!)
                 * In both cases the refcnt is incremented and attached to
                 *  the N_id node.
                 */

                if (NODE_TYPE (ID_VARDEC (arg_node)) == N_vardec) {
                    VARDEC_REFCNT (ID_VARDEC (arg_node))++;
                } else {
                    ARG_REFCNT (ID_VARDEC (arg_node))++;
                }
                ID_REFCNT (arg_node) = VARDEC_OR_ARG_REFCNT (ID_VARDEC (arg_node));
                DBUG_PRINT ("RC", ("RC for %s increased to %d.", ID_NAME (arg_node),
                                   ID_REFCNT (arg_node)));
            } else {
                /*
                 * This N_id node is argument of a N_prf/N_icm node *and*
                 *  it is definitly not the last usage of it.
                 * Therefore -1 is attached to the refcnt field of N_id
                 * to indicate that this operation does not need any refcnt
                 * adjustments.
                 */
                ID_REFCNT (arg_node) = -1;
            }
        } else {
            /*
             *  variable needs no refcount
             */
            ID_REFCNT (arg_node) = -1;
        }
    }

    if (DECL_MUST_NAIVEREFCOUNT (ID_VARDEC (arg_node))) {
        /*
         *  Naive refcounting is always done.
         */
        if (NODE_TYPE (ID_VARDEC (arg_node)) == N_vardec) {
            VARDEC_NAIVE_REFCNT (ID_VARDEC (arg_node))++;
        } else {
            ARG_NAIVE_REFCNT (ID_VARDEC (arg_node))++;
        }
        ID_NAIVE_REFCNT (arg_node) = VARDEC_OR_ARG_NAIVE_REFCNT (ID_VARDEC (arg_node));
        DBUG_PRINT ("RC", ("NAIVE RC for %s increased to %d.", ID_NAME (arg_node),
                           ID_NAIVE_REFCNT (arg_node)));
    } else {
        /*
         *  variable needs no refcount
         */
        ID_NAIVE_REFCNT (arg_node) = -1;
    }

    /*
     * if we have found the index-vector of a with-loop, we set RC of
     *  'NWITH(2)_VEC' to 1.
     */
    wl_node = INFO_RC_WITH (arg_info);
    while (wl_node != NULL) {
        DBUG_ASSERT ((NODE_TYPE (wl_node) == N_exprs),
                     "N_exprs-chain of WL-nodes not found in arg_info");

        if (strcmp (IDS_NAME (NWITH_OR_NWITH2_VEC (EXPRS_EXPR (wl_node))),
                    ID_NAME (arg_node))
            == 0) {
            if (!INFO_RC_ONLYNAIVE (arg_info)) {
                IDS_REFCNT (NWITH_OR_NWITH2_VEC (EXPRS_EXPR (wl_node))) = 1;
            }
            IDS_NAIVE_REFCNT (NWITH_OR_NWITH2_VEC (EXPRS_EXPR (wl_node))) = 1;
            break;
        }

        wl_node = EXPRS_NEXT (wl_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RClet( node *arg_node, node *arg_info)
 *
 * description:
 *   set the refcnts of the defined variables to the actual refcnt-values of
 *   the respective variable declarations and reset these values to 0.
 *
 ******************************************************************************/

node *
RClet (node *arg_node, node *arg_info)
{
    ids *ids;

    DBUG_ENTER ("RClet");
    DBUG_PRINT ("RC", ("line: %d", NODE_LINE (arg_node)));

    ids = LET_IDS (arg_node);
    while (NULL != ids) {
        ids = DefinedIds (ids, arg_info);
        ids = IDS_NEXT (ids);
    }

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 *  function:
 *    node *RCcond(node *arg_node, node *arg_info);
 *
 *  description:
 *    Refcounts then- and else-part of conditional and computes the maximum
 *    of both branches to be used in the code "above". The braches may need
 *    somd corrections of refcounters during execution. These corrections
 *    will be generated while "really" compiling from the following
 *    informations:
 *      The variables COND_THENVARS and CONS_ELSEVARS are id-chains, they
 *    will contain information to correct the refcounters in each branch
 *    of the conditional. The refcounters will contain the number of extra
 *    consumations needed for the corresponding variable before executing
 *    the actual branch.
 *
 ******************************************************************************/

node *
RCcond (node *arg_node, node *arg_info)
{
    node *vardec;
    ids *thenvars, *elsevars, *new_ids, *naive_new_ids;
    ids *naive_thenvars;
    ids *naive_elsevars;
    int *rest_dump, *then_dump, *else_dump, i, use_old = 0, naive_use_old = 0;
    int *naive_rest_dump;
    int *naive_then_dump;
    int *naive_else_dump;
    int then_compare = 0;
    int else_compare = 0;
    bool do_on_normal = FALSE;
    bool do_on_naive = FALSE;
    bool do_it = FALSE;
    bool do_use_old = FALSE;

    DBUG_ENTER ("RCcond");

    /* store current vardec refcounts in rest_dump */
    rest_dump = StoreRC (RC_REAL, INFO_RC_VARNO (arg_info), arg_info);
    naive_rest_dump = StoreRC (RC_NAIVE, INFO_RC_VARNO (arg_info), arg_info);

    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    /* store vardec refcounts after refcounting then part */
    then_dump = StoreRC (RC_REAL, INFO_RC_VARNO (arg_info), arg_info);
    naive_then_dump = StoreRC (RC_NAIVE, INFO_RC_VARNO (arg_info), arg_info);

    /* get same refcounts as before refcounting else part */
    RestoreRC (RC_REAL, rest_dump, arg_info);
    RestoreRC (RC_NAIVE, naive_rest_dump, arg_info);

    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    /* store vardec refcounts after refcounting else part */
    else_dump = StoreRC (RC_REAL, INFO_RC_VARNO (arg_info), arg_info);
    naive_else_dump = StoreRC (RC_NAIVE, INFO_RC_VARNO (arg_info), arg_info);

    /* now compute maximum of then- and else-dump and store it
     *  in vardec refcnt.
     * store differences between then- and else dump in COND_THEN,
     *  COND_ELSE respectively:
     *  - COND_THEN if (then_dump[i] < else_dump[i]):
     *                              (else_dump[i] - then_dump[i])
     *  - COND_ELSE if (else_dump[i] < then_dump[i]):
     *                              (then_dump[i] - else_dump[i])
     */

    thenvars = COND_THENVARS (arg_node);
    elsevars = COND_ELSEVARS (arg_node);
    naive_thenvars = COND_NAIVE_THENVARS (arg_node);
    naive_elsevars = COND_NAIVE_ELSEVARS (arg_node);

    if ((thenvars != NULL) || (elsevars != NULL)) {
        use_old = 1;
    }
    if ((naive_thenvars != NULL) || (naive_elsevars != NULL)) {
        naive_use_old = 1;
    }
    DBUG_PRINT ("RCi", ("use_old %i naive_use_old %i", use_old, naive_use_old));

    for (i = 0; i < INFO_RC_VARNO (arg_info); i++) {
        if ((then_dump[i] != else_dump[i])
            || (naive_then_dump[i] != naive_else_dump[i])) {
            vardec = FindVardec_Varno (i, fundef_node);
            DBUG_ASSERT ((NULL != vardec), " var not found");

            if (then_dump[i] != else_dump[i]) {
                then_compare = then_dump[i];
                else_compare = else_dump[i];
                do_on_normal = (!INFO_RC_ONLYNAIVE (arg_info));
                do_on_naive = TRUE;
                do_use_old = use_old;
                do_it = TRUE;
            } else if (naive_then_dump[i] != naive_else_dump[i]) {
                then_compare = naive_then_dump[i];
                else_compare = naive_else_dump[i];
                do_on_normal = FALSE;
                do_on_naive = TRUE;
                do_it = TRUE;
                do_use_old = naive_use_old;
            } else {
                do_it = FALSE;
            }

            if (then_compare < else_compare) {
                if (!do_use_old) {
                    if (do_on_normal) {
                        new_ids = MakeIds_Copy (VARDEC_OR_ARG_NAME (vardec));
                        IDS_REFCNT (new_ids) = else_dump[i] - then_dump[i];
                        IDS_NAIVE_REFCNT (new_ids)
                          = naive_else_dump[i] - naive_then_dump[i];
                        IDS_VARDEC (new_ids) = vardec;
                        IDS_NEXT (new_ids) = thenvars;
                        thenvars = new_ids;
                        /* insert 'new_ids' at the begining of 'defvars' */
                        DBUG_PRINT ("RC",
                                    ("append %s :%d::%d to then-part", IDS_NAME (new_ids),
                                     IDS_REFCNT (new_ids), IDS_NAIVE_REFCNT (new_ids)));
                    }
                    if (do_on_naive) {
                        naive_new_ids = MakeIds_Copy (VARDEC_OR_ARG_NAME (vardec));
                        IDS_REFCNT (naive_new_ids) = else_dump[i] - then_dump[i];
                        IDS_NAIVE_REFCNT (naive_new_ids)
                          = naive_else_dump[i] - naive_then_dump[i];
                        IDS_VARDEC (naive_new_ids) = vardec;
                        IDS_NEXT (naive_new_ids) = naive_thenvars;
                        naive_thenvars = naive_new_ids;
                        /* insert 'new_ids' at the begining of 'defvars' */
                        DBUG_PRINT ("RC",
                                    ("append %s :%d::%d to naive_then-part",
                                     IDS_NAME (naive_new_ids), IDS_REFCNT (naive_new_ids),
                                     IDS_NAIVE_REFCNT (naive_new_ids)));
                    }
                } else {
                    if (do_on_normal) {
                        new_ids = LookupIds (VARDEC_OR_ARG_NAME (vardec), thenvars);
                        DBUG_ASSERT ((NULL != new_ids), "var not found");
                        IDS_REFCNT (new_ids) = else_dump[i] - then_dump[i];
                        IDS_NAIVE_REFCNT (new_ids)
                          = naive_else_dump[i] - naive_then_dump[i];
                        DBUG_PRINT ("RC", ("changed %s :%d::%d in then-part",
                                           IDS_NAME (new_ids), IDS_REFCNT (new_ids),
                                           IDS_NAIVE_REFCNT (new_ids)));
                    }
                    if (do_on_naive) {
                        naive_new_ids
                          = LookupIds (VARDEC_OR_ARG_NAME (vardec), naive_thenvars);
                        DBUG_ASSERT ((NULL != naive_new_ids), "var not found");
                        IDS_REFCNT (naive_new_ids) = else_dump[i] - then_dump[i];
                        IDS_NAIVE_REFCNT (naive_new_ids)
                          = naive_else_dump[i] - naive_then_dump[i];
                        DBUG_PRINT ("RC",
                                    ("changed %s :%d::%d in naive_then-part",
                                     IDS_NAME (naive_new_ids), IDS_REFCNT (naive_new_ids),
                                     IDS_NAIVE_REFCNT (naive_new_ids)));
                    }
                }
                L_VARDEC_OR_ARG_REFCNT (vardec, else_dump[i]);
                L_VARDEC_OR_ARG_NAIVE_REFCNT (vardec, naive_else_dump[i]);

            } else if (else_compare < then_compare) {
                if (0 == do_use_old) {
                    if (do_on_normal) {
                        new_ids = MakeIds_Copy (VARDEC_OR_ARG_NAME (vardec));
                        IDS_REFCNT (new_ids) = then_dump[i] - else_dump[i];
                        IDS_NAIVE_REFCNT (new_ids)
                          = naive_then_dump[i] - naive_else_dump[i];
                        IDS_VARDEC (new_ids) = vardec;
                        IDS_NEXT (new_ids) = elsevars;
                        elsevars = new_ids;
                        /* insert 'new_ids' at the begining of 'elsevars' */
                        DBUG_PRINT ("RC",
                                    ("append %s :%d::%d to else-part", IDS_NAME (new_ids),
                                     IDS_REFCNT (new_ids), IDS_NAIVE_REFCNT (new_ids)));
                    }
                    if (do_on_naive) {
                        naive_new_ids = MakeIds_Copy (VARDEC_OR_ARG_NAME (vardec));
                        IDS_REFCNT (naive_new_ids) = then_dump[i] - else_dump[i];
                        IDS_NAIVE_REFCNT (naive_new_ids)
                          = naive_then_dump[i] - naive_else_dump[i];
                        IDS_VARDEC (naive_new_ids) = vardec;
                        IDS_NEXT (naive_new_ids) = naive_elsevars;
                        naive_elsevars = naive_new_ids;
                        /* insert 'naive_new_ids' at the begining of 'elsevars' */
                        DBUG_PRINT ("RC",
                                    ("append %s :%d::%d to naive_else-part",
                                     IDS_NAME (naive_new_ids), IDS_REFCNT (naive_new_ids),
                                     IDS_NAIVE_REFCNT (naive_new_ids)));
                    }
                } else {
                    if (do_on_normal) {
                        new_ids = LookupIds (VARDEC_OR_ARG_NAME (vardec), elsevars);
                        DBUG_ASSERT ((NULL != new_ids), "var not found");
                        IDS_REFCNT (new_ids) = then_dump[i] - else_dump[i];
                        IDS_NAIVE_REFCNT (new_ids)
                          = naive_then_dump[i] - naive_else_dump[i];

                        DBUG_PRINT ("RC", ("changed %s :%d::%d in else-part",
                                           IDS_NAME (new_ids), IDS_REFCNT (new_ids),
                                           IDS_NAIVE_REFCNT (new_ids)));
                    }
                    if (do_on_naive) {
                        naive_new_ids
                          = LookupIds (VARDEC_OR_ARG_NAME (vardec), naive_elsevars);
                        DBUG_ASSERT ((NULL != naive_new_ids), "var not found");
                        IDS_REFCNT (naive_new_ids) = then_dump[i] - else_dump[i];
                        IDS_NAIVE_REFCNT (naive_new_ids)
                          = naive_then_dump[i] - naive_else_dump[i];

                        DBUG_PRINT ("RC",
                                    ("changed %s :%d::%d in naive_else-part",
                                     IDS_NAME (naive_new_ids), IDS_REFCNT (naive_new_ids),
                                     IDS_NAIVE_REFCNT (naive_new_ids)));
                    }
                }
                L_VARDEC_OR_ARG_REFCNT (vardec, then_dump[i]);
                L_VARDEC_OR_ARG_NAIVE_REFCNT (vardec, naive_then_dump[i]);
            }
            if (do_on_normal) {
                DBUG_PRINT ("RC", ("set refcount of %s to %d", IDS_NAME (new_ids),
                                   VARDEC_OR_ARG_REFCNT (vardec)));
            }
            if (do_on_naive) {
                DBUG_PRINT ("RC",
                            ("set naive-refcount of %s to %d", IDS_NAME (naive_new_ids),
                             VARDEC_OR_ARG_NAIVE_REFCNT (vardec)));
            }
        }
    } /* for (i = 0; i < varno; i++) */

    /* store refcount information for use while compilation */
    COND_THENVARS (arg_node) = thenvars;
    COND_ELSEVARS (arg_node) = elsevars;
    COND_NAIVE_THENVARS (arg_node) = naive_thenvars;
    COND_NAIVE_ELSEVARS (arg_node) = naive_elsevars;

    /* free the dumps */
    FreeDump (rest_dump);
    FreeDump (naive_rest_dump);
    FreeDump (then_dump);
    FreeDump (naive_then_dump);
    FreeDump (else_dump);
    FreeDump (naive_else_dump);

    /* last but not least, traverse condition */
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCNwith( node *arg_node, node *arg_info)
 *
 * description:
 *   performs the refcounting for a N_Nwith or N_Nwith2 node.
 *   collects in NWITH_DEC_RC_IDS all ids that are RC-arguments of the
 *    with-loop.
 *
 * remarks:
 *   - 'INFO_RC_WITH( arg_info)' contains a N_exprs-chain with all parent
 *     WL-nodes.
 *   - In 'INFO_RC_RCDUMP( arg_info)' we store the initial refcounters for
 *     RCNcode:
 *       rc(var) = 1,  if 'var' is element of 'NWITH_IN_MASK'  \ this is done,
 *               = 1,  if 'var' is index-vector of with-loop   / to fool the RCO
 *               = 0,  otherwise
 *   - The RC of 'NWITH_VEC' indicates whether we need to build the
 *     index-vector or not:
 *        1: we have found references to the index-vector in at least one
 *           code-block
 *       -1: we do not need to build the index-vector.
 *     This flag/RC is correctly set after traversal of the code-blocks.
 *
 ******************************************************************************/

node *
RCNwith (node *arg_node, node *arg_info)
{
    node *vardec;
    ids *new_ids;
    int *ref_dump, *tmp_rcdump;
    int *naive_ref_dump;
    int *tmp_naive_rcdump;
    ids *iv_ids;
    ids *last_ids = NULL;

    DBUG_ENTER ("RCNwith");

    DBUG_PRINT ("RC", ("\nEntering: count references in with-loop."));

    /*
     * insert current WL into 'INFO_RC_WITH( arg_info)' (at head of chain).
     */
    INFO_RC_WITH (arg_info) = MakeExprs (arg_node, INFO_RC_WITH (arg_info));

    /*
     * store current refcounts, initialize them with 0
     */
    ref_dump = StoreAndInitRC (RC_REAL, INFO_RC_VARNO (arg_info), 0, arg_info);
    naive_ref_dump = StoreAndInitRC (RC_NAIVE, INFO_RC_VARNO (arg_info), 0, arg_info);

    DBUG_PRINT ("RC", ("Init RC with 1 for NWITH_CODE: "));
    DBUG_PRINT ("RC", ("  /* in-vars */"));
    vardec = DFMGetMaskEntryDeclSet (NWITH_OR_NWITH2_IN_MASK (arg_node));
    while (vardec != NULL) {
        if (!INFO_RC_ONLYNAIVE (arg_info)) {
            if (DECL_MUST_REFCOUNT (vardec)) {
                L_VARDEC_OR_ARG_REFCNT (vardec, 1);
                DBUG_PRINT ("RC", ("  %s", VARDEC_OR_ARG_NAME (vardec)));
            }
        }
        if (DECL_MUST_NAIVEREFCOUNT (vardec)) {
            L_VARDEC_OR_ARG_NAIVE_REFCNT (vardec, 1);
        }
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_PRINT ("RC", ("  /* index-vector */"));
    vardec = IDS_VARDEC (NWITH_OR_NWITH2_VEC (arg_node));
    if (!INFO_RC_ONLYNAIVE (arg_info)) {
        if (DECL_MUST_REFCOUNT (vardec)) {
            L_VARDEC_OR_ARG_REFCNT (vardec, 1);
            DBUG_PRINT ("RC", ("  %s", VARDEC_OR_ARG_NAME (vardec)));
        }
    }
    if (DECL_MUST_NAIVEREFCOUNT (vardec)) {
        L_VARDEC_OR_ARG_NAIVE_REFCNT (vardec, 1);
    }

    /*
     *  now we set up 'INFO_RC_RCDUMP( arg_info)' (needed in RCNcode)
     */
    tmp_rcdump = INFO_RC_RCDUMP (arg_info);
    tmp_naive_rcdump = INFO_RC_NAIVE_RCDUMP (arg_info);
    INFO_RC_RCDUMP (arg_info)
      = StoreAndInitRC (RC_REAL, INFO_RC_VARNO (arg_info), 0, arg_info);
    INFO_RC_NAIVE_RCDUMP (arg_info)
      = StoreAndInitRC (RC_NAIVE, INFO_RC_VARNO (arg_info), 0, arg_info);

    /*************************************************************
     * count references in with-loop
     */

    /*
     * CAUTION:
     *   We must traverse the (parts -> withids) before we traverse the code,
     *   to get the right RC in 'NWITH_VEC'!
     *   'RCNwithid()' initializes the RC, and while traversal of the code
     *   it is set correctly!
     */

    if (NODE_TYPE (arg_node) == N_Nwith) {
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    } else {
        NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
        NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
        if (NWITH2_CODE (arg_node) != NULL) {
            NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
        }
    }

    /*
     * count references in with-loop
     *************************************************************/

    /*
     *  Restore refcounts and ...
     */
    RestoreRC (RC_REAL, ref_dump, arg_info);
    RestoreRC (RC_NAIVE, naive_ref_dump, arg_info);
    FreeDump (ref_dump);
    FreeDump (naive_ref_dump);
    /*
     *  ... delete dumps from the info-node.
     */
    FreeDump (INFO_RC_RCDUMP (arg_info));
    FreeDump (INFO_RC_NAIVE_RCDUMP (arg_info));
    INFO_RC_RCDUMP (arg_info) = tmp_rcdump;
    INFO_RC_NAIVE_RCDUMP (arg_info) = tmp_naive_rcdump;

    /*
     * Increment refcount of each RC-variable that is IN-var of the with-loop.
     *
     * We collect all these ids in 'NWITH_DEC_RC_IDS( arg_node)'.
     * Compile() generates for each var found in 'NWITH_DEC_RC_IDS' a
     *  'ND_DEC_RC'-ICM in the epilog-code of the with-loop!
     *
     * RCO: with-loops are handled like prfs:
     *      when RCO is active, we only count the last occurance of IN-vars!
     *
     * Remark:
     * The NWITHOP part (neutral element of a fold-with-loop) must be refcounted
     * seperately (RCNwithop()), because Compile() generates an assignment
     *    wl_id = neutral;
     * Nevertheless, the neutral element of a fold-with-loop is refcounted here
     * as well, although this MIGHT be superfluous!
     * Stictly speaking, we only need to refcount variables here, that occur
     * in one of the WL-parts without NWITHOP. But for simplicity reasons we
     * just inspect the data flow IN-mask here, i.e. if a variable occurs
     * in NWITHOP only it is counted twice, though:
     *
     *       v:1 = [1];
     *       A = with ... fold( +, [0], v:1);    // okay
     *
     *       v:2 = [1];
     *       A = with ... fold( +, v:2, v:1);    // okay
     *
     *       v:2 = [1];
     *       A = with ... fold( +, v:2, [1]);    // ':1' is superfluous
     *
     */

    if (!INFO_RC_ONLYNAIVE (arg_info)) {
        if (NWITH_OR_NWITH2_DEC_RC_IDS (arg_node) != NULL) {
            L_NWITH_OR_NWITH2_DEC_RC_IDS (arg_node,
                                          FreeAllIds (
                                            NWITH_OR_NWITH2_DEC_RC_IDS (arg_node)));
        }
    }
    vardec = DFMGetMaskEntryDeclSet (NWITH_OR_NWITH2_IN_MASK (arg_node));
    if (!INFO_RC_ONLYNAIVE (arg_info)) {
        DBUG_PRINT ("RC", ("NWITH_DEC_RC_IDS:"));
    }
    while (vardec != NULL) {
        if (!INFO_RC_ONLYNAIVE (arg_info)) {
            if (DECL_MUST_REFCOUNT (vardec)) {
                if ((RC_IS_ZERO (VARDEC_OR_ARG_REFCNT (vardec)))
                    || (!(optimize & OPT_RCO))) {
                    /*
                     * increment RC of non-withop-params
                     */
                    L_VARDEC_OR_ARG_REFCNT (vardec, VARDEC_OR_ARG_REFCNT (vardec) + 1);
                    DBUG_PRINT ("RC", ("  %s", VARDEC_OR_ARG_NAME (vardec)));

                    new_ids = MakeIds_Copy (VARDEC_OR_ARG_NAME (vardec));
                    IDS_VARDEC (new_ids) = vardec;
                    IDS_REFCNT (new_ids) = VARDEC_OR_ARG_REFCNT (vardec);
                    IDS_NAIVE_REFCNT (new_ids) = VARDEC_OR_ARG_NAIVE_REFCNT (vardec);

                    if (NWITH_OR_NWITH2_DEC_RC_IDS (arg_node) == NULL) {
                        L_NWITH_OR_NWITH2_DEC_RC_IDS (arg_node, new_ids);
                    } else {
                        IDS_NEXT (last_ids) = new_ids;
                    }
                    last_ids = new_ids;
                }
            }
        }

        if (DECL_MUST_NAIVEREFCOUNT (vardec)) {
            /*
             * Naive refcounting is always done.
             */
            L_VARDEC_OR_ARG_NAIVE_REFCNT (vardec,
                                          VARDEC_OR_ARG_NAIVE_REFCNT (vardec) + 1);
        }
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    /*
     * refcounting the neutral element of a fold with-loop
     */
    if (NODE_TYPE (arg_node) == N_Nwith) {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    } else {
        NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    }

    /*
     * index-vector-components are always needed for the code of the WL
     *  -> set the RC to 1.
     */
    iv_ids = NWITH_OR_NWITH2_IDS (arg_node);
    while (iv_ids != NULL) {
        if (!INFO_RC_ONLYNAIVE (arg_info)) {
            if (DECL_MUST_REFCOUNT (IDS_VARDEC (iv_ids))) {
                IDS_REFCNT (iv_ids) = 1;
            }
        }
        if (DECL_MUST_NAIVEREFCOUNT (IDS_VARDEC (iv_ids))) {
            IDS_NAIVE_REFCNT (iv_ids) = 1;
        }
        iv_ids = IDS_NEXT (iv_ids);
    }

    /*
     * we leave the with-loop
     *   -> remove current WL from 'INFO_RC_WITH( arg_info)' (head of chain!).
     */
    EXPRS_EXPR (INFO_RC_WITH (arg_info)) = NULL;
    INFO_RC_WITH (arg_info) = FreeNode (INFO_RC_WITH (arg_info));

    DBUG_PRINT ("RC", ("\nLeaving: count references in with-loop."));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCNpart( node *arg_node, node *arg_info)
 *
 * description:
 *   performs the refcounting for a N_Npart node.
 *
 ******************************************************************************/

node *
RCNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCNpart");

    /*
     * we do not need any RCs at the withids.
     *  -> mark them as non-RC-objects.
     */
    NPART_WITHID (arg_node) = Trav (NPART_WITHID (arg_node), arg_info);

    /*
     * we count the references in the generator.
     */
    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

    /*
     * traverse next part-node
     */
    if (NPART_NEXT (arg_node) != NULL) {
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCNcode( node *arg_node, node *arg_info)
 *
 * description:
 *   performs reference counting for N_Ncode nodes.
 *
 * remarks:
 *   - In 'INFO_RC_RCDUMP( arg_info)' are the initial refcounters stored:
 *       rc(var) = 1, if 'var' is element of 'NWITH(2)_IN'   \ this is done,
 *                 1, if 'var' is index-vector of with-loop  / to fool the RCO
 *                 0, otherwise
 *   - The RC of 'NWITH(2)_VEC' indicates whether we need to build the
 *     index-vector or not:
 *        1: we have found references to the index-vector in at least one
 *           code-block
 *       -1: we do not need to build the index-vector.
 *     This flag/RC is correctly set after traversal of the code-blocks.
 *
 ******************************************************************************/

node *
RCNcode (node *arg_node, node *arg_info)
{
    node *vardec;
    ids *new_ids;
    ids *last_ids = NULL;

    DBUG_ENTER ("RCNcode");

    /*
     * initialize refcounters
     */
    RestoreRC (RC_REAL, INFO_RC_RCDUMP (arg_info), arg_info);
    RestoreRC (RC_NAIVE, INFO_RC_NAIVE_RCDUMP (arg_info), arg_info);

    /*
     * count the references in the code
     */
    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    /*
     * We collect all ids, which RC is >1, in 'NCODE_INC_RC_IDS( arg_node)'.
     * In 'IDS_REFCNT' we store (RC - 1) --- because we started the refcounting
     *   of these vars with (RC == 1)!!!
     * 'compile' generates for each var in 'NCODE_INC_RC_IDS' a 'ND_INC_RC'-ICM
     *   as first statement of the code-block!
     *
     * CAUTION: we must initialize NCODE_INC_RC_IDS because some subtrees
     *          (e.g. bodies of while-loops) are traversed twice!!!
     */

    if (!INFO_RC_ONLYNAIVE (arg_info)) {
        DBUG_PRINT ("RC", ("NCODE_INC_RC_IDS:"));
        if (NCODE_INC_RC_IDS (arg_node) != NULL) {
            NCODE_INC_RC_IDS (arg_node) = FreeAllIds (NCODE_INC_RC_IDS (arg_node));
        }
        FOREACH_VARDEC_AND_ARG (fundef_node, vardec,
                                if (VARDEC_OR_ARG_REFCNT (vardec) > 1) {
                                    new_ids = MakeIds_Copy (VARDEC_OR_ARG_NAME (vardec));
                                    IDS_VARDEC (new_ids) = vardec;
                                    IDS_REFCNT (new_ids)
                                      = VARDEC_OR_ARG_REFCNT (vardec) - 1;
                                    IDS_NAIVE_REFCNT (new_ids)
                                      = VARDEC_OR_ARG_NAIVE_REFCNT (vardec) - 1;
                                    DBUG_PRINT ("RC",
                                                ("  %s", VARDEC_OR_ARG_NAME (vardec)));

                                    if (NCODE_INC_RC_IDS (arg_node) == NULL) {
                                        NCODE_INC_RC_IDS (arg_node) = new_ids;
                                    } else {
                                        IDS_NEXT (last_ids) = new_ids;
                                    }
                                    last_ids = new_ids;
                                }) /* FOREACH_VARDEC_AND_ARG */
    }

    /*
     * count the references in next code
     */
    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCNgen( node *arg_node, node *arg_info)
 *
 * description:
 *   performs the refcounting for a N_Ngen node.
 *
 ******************************************************************************/

node *
RCNgen (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCNgen");

    NGEN_BOUND1 (arg_node) = Trav (NGEN_BOUND1 (arg_node), arg_info);
    NGEN_BOUND2 (arg_node) = Trav (NGEN_BOUND2 (arg_node), arg_info);
    if (NGEN_STEP (arg_node) != NULL) {
        NGEN_STEP (arg_node) = Trav (NGEN_STEP (arg_node), arg_info);
    }
    if (NGEN_WIDTH (arg_node) != NULL) {
        NGEN_WIDTH (arg_node) = Trav (NGEN_WIDTH (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCNwithid( node *arg_node, node *arg_info)
 *
 * description:
 *   marks the index-vectors as non-RC-objects.
 *
 ******************************************************************************/

node *
RCNwithid (node *arg_node, node *arg_info)
{
    ids *iv_ids;

    DBUG_ENTER ("RCNwithid");

    /*
     * we initialize the RC of the index-vector with -1.
     *  -> by default we do not need to build it.
     * (if a reference of the index-vector is found while traversal of the codes,
     *  the RC of the withid in the first part (= 'NWITH(2)_VEC') is set to 1.)
     */
    if (!INFO_RC_ONLYNAIVE (arg_info)) {
        IDS_REFCNT (NWITHID_VEC (arg_node)) = -1;
    }
    IDS_NAIVE_REFCNT (NWITHID_VEC (arg_node)) = -1;

    /*
     * we initialize the RC of the index-vector-components with -1 because they
     * must not be counted.
     * Since index-vector-components are always needed for the code of the WL,
     * RCNwith() will set their RCs to 1 after traversal of the with-loop.
     */
    iv_ids = NWITHID_IDS (arg_node);
    while (iv_ids != NULL) {
        if (!INFO_RC_ONLYNAIVE (arg_info)) {
            IDS_REFCNT (iv_ids) = -1;
        }
        IDS_NAIVE_REFCNT (iv_ids) = -1;
        iv_ids = IDS_NEXT (iv_ids);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RCNwithop( node *arg_node, node *arg_info)
 *
 * description:
 *   Performs the refcounting for a N_Nwithop node.
 *
 ******************************************************************************/

node *
RCNwithop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RCNwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        /*
         * We count this reference via 'NWITH(2)_IN' ('RCNwith()').
         *  -> set RC to -1.
         */
        if (NODE_TYPE (NWITHOP_SHAPE (arg_node)) == N_id) {
            if (!INFO_RC_ONLYNAIVE (arg_info)) {
                ID_REFCNT (NWITHOP_SHAPE (arg_node)) = -1;
            }
            ID_NAIVE_REFCNT (NWITHOP_SHAPE (arg_node)) = -1;
        }
        if ((NWITHOP_DEFAULT (arg_node) != NULL)
            && (NODE_TYPE (NWITHOP_DEFAULT (arg_node)) == N_id)) {
            if (!INFO_RC_ONLYNAIVE (arg_info)) {
                ID_REFCNT (NWITHOP_DEFAULT (arg_node)) = -1;
            }
            ID_NAIVE_REFCNT (NWITHOP_DEFAULT (arg_node)) = -1;
        }
        break;

    case WO_modarray:
        /*
         * We count this reference via 'NWITH(2)_IN' ('RCNwith()').
         *  -> set RC to -1.
         */
        DBUG_ASSERT ((NODE_TYPE (NWITHOP_ARRAY (arg_node)) == N_id), "no id found");

        if (!INFO_RC_ONLYNAIVE (arg_info)) {
            ID_REFCNT (NWITHOP_ARRAY (arg_node)) = -1;
        }
        ID_NAIVE_REFCNT (NWITHOP_ARRAY (arg_node)) = -1;
        break;

    case WO_foldfun:
        /* here is no break missing! */
    case WO_foldprf:
        /*
         * 'compile' needs an annotated RC at this node,
         *  therefore this reference is counted here, *not* via 'NWITH(2)_IN'.
         */
        if (NWITHOP_NEUTRAL (arg_node) != NULL) {
            NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        }
        break;

    default:
        DBUG_ASSERT ((0), "wrong withop type found");
    }

    DBUG_RETURN (arg_node);
}
