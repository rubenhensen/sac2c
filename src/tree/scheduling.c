/*
 *
 * $Log$
 * Revision 3.26  2001/05/17 11:37:38  dkr
 * FREE/MALLOC eliminated
 *
 * Revision 3.25  2001/05/09 15:13:00  cg
 * All scheduling ICMs get an additional first parameter,
 * i.e. the segment ID. This is required to identify the appropriate
 * set of scheduler internal variables.
 *
 * Revision 3.24  2001/05/04 11:45:12  ben
 * scheduler_table[] modified:
 * Even deleted, Cyclic renamed to Static, Afs renamed to Affinity
 *
 * Revision 3.23  2001/05/03 17:31:55  dkr
 * MAXHOMDIM replaced by HOMSV
 *
 * Revision 3.22  2001/04/03 22:31:05  dkr
 * Signature of MT_SCHEDULER_..._BEGIN, MT_SCHEDULER_..._END icms
 * modified: Blocking-vector is now longer given as an argument because
 * it is completely useless!!
 *
 * Revision 3.21  2001/04/03 13:49:37  dkr
 * MAX( WLSEG_SV, WLSEG_UBV) replaced by WLSEG_SV:
 * WLSEG_UBV is already allowed for calculation of WLSEG_SV.
 *
 * Revision 3.20  2001/04/02 11:41:45  dkr
 * include of wltransform.h replaced by wl_bounds.h
 *
 * Revision 3.19  2001/03/29 14:15:04  dkr
 * functions SCHMakeCompatibleSyncblockScheduling,
 * CompileSyncblockSchedulingArgs, removed
 *
 * Revision 3.18  2001/03/29 01:36:11  dkr
 * WLSEGVAR_IDX_MIN, WLSEGVAR_IDX_MAX are now node-vectors
 *
 * Revision 3.17  2001/03/28 12:52:52  ben
 * scheduler_table modified with numerical parameters for Cyclic, Self and Afs
 * minor Bug in CheckSchedulingArgs fixed
 * minor bug in SCHCopyScheduling fixed
 *
 * Revision 3.16  2001/03/27 11:50:36  ben
 * Afs added to scheduler_table
 *
 * Revision 3.15  2001/03/22 17:12:55  ben
 * Self added to scheduler_table
 *
 * Revision 3.14  2001/03/22 12:41:16  ben
 * In scheduler_table Cyclic added
 *
 * Revision 3.13  2001/03/21 11:57:53  ben
 * In scheduler_table Even set to SC_var_seg
 *
 * Revision 3.12  2001/03/20 22:37:38  dkr
 * comment for SCHMakeSchedulingByPragma() modified
 *
 * Revision 3.11  2001/03/20 19:05:03  dkr
 * wlcomp-pragma functions SchedulingWL(), SchedulingSegs() replaced by
 * Scheduling()
 *
 * Revision 3.10  2001/03/20 16:02:25  ben
 * SCHPrintScheduling: minor changes done
 * even added to scheduler table
 * static adjustmentflag set to 0 again
 *
 * Revision 3.9  2001/03/20 12:57:51  ben
 * in scheduler_table adjustmentflag for Static set to 1
 *
 * Revision 3.8  2001/03/15 21:25:25  dkr
 * signature of NodeOr..._MakeIndex modified
 *
 * Revision 3.7  2001/03/14 15:36:51  ben
 * SCHAdjustmentRequired works correctly even for var. segments now :-)
 *
 * Revision 3.6  2001/03/14 14:18:51  ben
 * comment for scheduler_table modified
 *
 * Revision 3.5  2001/03/14 14:11:32  ben
 * return value of SCHAdjustmentRequired is bool
 *
 * Revision 3.4  2001/03/14 10:17:57  ben
 * BlockVar values in scheduler_table adjusted
 *
 * Revision 3.3  2001/01/29 18:32:54  dkr
 * some superfluous attributes of N_WLsegVar removed
 * CompileVarSegSchedulingArgs() modified
 *
 * Revision 3.2  2001/01/24 23:37:24  dkr
 * semantics of WLSEGX_IDX_MAX slightly modified
 * NameOrVal_MakeIndex() used
 *
 * Revision 3.1  2000/11/20 18:03:28  sacbase
 * new release made
 *
 * Revision 1.6  2000/11/14 13:19:34  dkr
 * no '... might be used uninitialized' warnings anymore
 *
 * Revision 1.5  2000/08/17 10:06:55  dkr
 * signature of RenameLocalIdentifier() modified
 *
 * Revision 1.4  2000/07/31 10:45:52  cg
 * Eventually, the son ICM_NEXT is removed from the N_icm node.
 * The creation function MakeIcm is adjusted accordingly.
 *
 * Revision 1.2  2000/01/28 13:51:50  jhs
 * SCHCopyScheduling does not retun NULL any more, but the
 * real copy of the scheduling.
 *
 * Revision 1.1  2000/01/24 10:53:33  jhs
 * Initial revision
 *
 * Revision 2.2  1999/06/25 14:51:28  rob
 * Introduce definitions and utility infrastructure for tagged array support.
 *
 * Revision 2.1  1999/02/23 12:44:09  sacbase
 * new release made
 *
 * Revision 1.6  1999/02/15 10:24:52  cg
 * Bug fixed in scheduling for segments of variable borders.
 *
 * Revision 1.5  1998/08/11 14:32:35  dkr
 * CompileVarSegSchedulingArgs implemented
 *
 * Revision 1.4  1998/08/07 16:09:23  dkr
 * CompileScheduling, ... can handle (sched == NULL) now.
 * If no 'sched' is present, the ICMs 'MT_SCHEDULER_BEGIN',
 * 'MT_SCHEDULER_END' are generated.
 *
 * Revision 1.3  1998/08/03 10:51:42  cg
 * added function SCHAdjustmentRequired that decides for a particular
 * dimension and a given scheduling whether the scheduling will produce
 * a legal scheduling or an adjustment has to be done.
 *
 * Revision 1.2  1998/06/18 14:19:41  cg
 * file now only contains implementation of abstract data type
 * for the representation of schedulings
 *
 */

/*****************************************************************************
 *
 * file:   scheduling.c
 *
 * prefix: SCH
 *
 * description:
 *   Scheduling is needed nor the non-sequential execution of with-loops.
 *   In this context, it means a mechanism which guarantees that each array
 *   element selected by a generator is calculated by exactly one thread.
 *   In other words, the scheduling defines a partitioning of the iteration
 *   space onto the threads available.
 *
 *   For this purpose, an abstract data type is defined which stores different
 *   kinds of schedulings requiring different parameter sets. This file also
 *   provides the required routines for generating different kinds of
 *   schedulings as well as copying or removing them. Nevertheless, functions
 *   are provided that directly compile scheduling representations to the
 *   respective ICMs.
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <varargs.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "traverse.h"
#include "Error.h"
#include "precompile.h" /* for RenameLocalIdentifier() */
#include "wl_bounds.h"
#include "dbug.h"

/******************************************************************************
 *
 * data type: sched_class_t
 *            sched_arg_type_t
 *            sched_arg_t
 *            sched_t
 *
 * description:
 *   These data types are used for the representation of schedulings. Each
 *   scheduling consists of a name called 'discipline', a class, a line
 *   number, and an argument specification.
 *
 *   Currently, three different scheduler classes are supported. Those for
 *   entire synchronisation blocks (SC_withloop), for segments with constant
 *   bounds (SC_const_seg), and for segments with variable bounds (SC_var_seg).
 *
 *   The line number is useful for error messages in those cases where the
 *   actual scheduling is defined by the programmer using the wlcomp pragma.
 *
 *   The argument specification consists of the exact number of arguments
 *   along with an array of arguments. Each argument is either a number,
 *   an identifier, a number replacing an identifier, or an entire vector
 *   of one of these.
 *
 *   Please observe:
 *   The vector argument types are not yet fully implemented.
 *
 ******************************************************************************/

typedef enum { SC_const_seg, SC_var_seg, SC_withloop } sched_class_t;

typedef enum {
    AT_num,
    AT_id,
    AT_num_for_id,
    AT_num_vec,
    AT_id_vec,
    AT_num_for_id_vec
} sched_arg_type_t;

typedef struct {
    sched_arg_type_t arg_type;
    union {
        int num;
        char *id;
        int *num_vec;
        char **id_vec;
    } arg;
} sched_arg_t;

typedef struct {
    char *discipline;
    sched_class_t class;
    int line;
    int num_args;
    sched_arg_t *args;
} sched_t;

typedef sched_t *SCHsched_t;

/******************************************************************************
 *
 * global variable: scheduler_table[]
 *
 * description:
 *   This global variable defines a table of scheduling specifications. Each
 *   scheduling is described by six entries: the scheduling name, the
 *   scheduling class, an adjustment flag, maximum scheduling dimension, the
 *   number of its arguments, and a type specification of the arguments.
 *
 *   Schedulings may be defined for synchronisation blocks/with-loops as well
 *   as for variable or constant segments. Note that schedulings for variable
 *   segments may also be applied to constant segments but not vice versa.
 *
 *   The adjustment flag specifies whether a particular scheduling produces
 *   legal subsegments with respect to unrolled loops.
 *   That means:
 *     adjust == 0: The MT_ADJUST_SCHEDULER-icm is needed for *all* dimensions.
 *     adjust != 0: This icm is needed for inhomogenous dimensions only
 *                  (-> (WLSEG_HOMSV[dim] == 0)).
 *
 *   The maximum scheduling dimension is the highest dimension on which a
 *   particular scheduling will schedule loops. Counting dimensions always
 *   starts with 0. A sufficiently large number, e.g. 99, specifies scheduling
 *   on a conceptually unlimited number of dimensions
 *
 *   The argument type specification is a comma separated string where
 *   'n' stands for number, 'i' for identifier, 'x' for number or identifier,
 *   and 'nv', 'iv', and 'xv' for the respective vector types.
 *
 *
 ******************************************************************************/

static struct {
    char *discipline;
    sched_class_t class;
    int adjust_flag;
    int max_sched_dim;
    int num_args;
    char *arg_spec;
} scheduler_table[] = {
  /* Name            Class          Adjust Dim  Args  ArgTypes */
  {"Block", SC_const_seg, 1, 0, 0, ""},
  {"BlockVar", SC_var_seg, 1, 0, 0, ""},
  {"Static", SC_var_seg, 1, 0, 1, "n"},
  {"Self", SC_var_seg, 1, 0, 1, "n"},
  {"Affinity", SC_var_seg, 1, 0, 1, "n"},
  {"AllByOne", SC_var_seg, 0, 0, 1, "i"},
  {"BlockBySome", SC_const_seg, 0, 0, 2, "i,i"},
  {"Static", SC_withloop, 0, 0, 0, ""},
  {"", SC_const_seg, 0, 0, 0, ""}};

/******************************************************************************
 *
 * function:
 *   sched_t *CheckSchedulingArgs( sched_t *sched, char *spec, node *exprs,
 *                                 int line)
 *
 * description:
 *   This function produces the arguments of a scheduling specification that
 *   is derived from a wlcomp pragma. All given arguments are compared with
 *   the respective argument specification from the scheduler table. The line
 *   number of the pragma in the original source code is required to produce
 *   sufficient error messages.
 *
 ******************************************************************************/

static sched_t *
CheckSchedulingArgs (sched_t *sched, char *spec, node *exprs, int line)
{
    int i;
    char *arg_spec;
    node *expr;

    DBUG_ENTER ("CheckSchedulingArgs");

    arg_spec = strtok (spec, ",");

    for (i = 0; i < sched->num_args; i++) {
        DBUG_ASSERT ((arg_spec != NULL), "Illegal scheduling specification");

        if (exprs == NULL) {
            ABORT (line, ("Scheduling discipline '%s` expects %d arguments"
                          "(too few specified)",
                          sched->discipline, sched->num_args));
        }

        expr = EXPRS_EXPR (exprs);

        switch (arg_spec[1]) {
        case '\0':
            switch (arg_spec[0]) {
            case 'n':
                if (NODE_TYPE (expr) != N_num) {
                    ABORT (line, ("Argument %d of scheduling discipline '%s` must be"
                                  " a number",
                                  i, sched->discipline));
                }
                sched->args[i].arg_type = AT_num;
                sched->args[i].arg.num = NUM_VAL (expr);
                break;

            case 'i':
                if (NODE_TYPE (expr) != N_id) {
                    ABORT (line, ("Argument %d of scheduling discipline '%s` must be"
                                  " an identifier",
                                  i, sched->discipline));
                }
                sched->args[i].arg_type = AT_id;
                sched->args[i].arg.id = StringCopy (ID_NAME (expr));
                break;

            case 'x':
                switch (NODE_TYPE (expr)) {
                case N_num:
                    sched->args[i].arg_type = AT_num_for_id;
                    sched->args[i].arg.num = NUM_VAL (expr);
                    break;
                case N_id:
                    sched->args[i].arg_type = AT_id;
                    sched->args[i].arg.id = StringCopy (ID_NAME (expr));
                    break;
                default:
                    ABORT (line, ("Argument %d of scheduling discipline '%s` must be"
                                  " an identifier or a number",
                                  i, sched->discipline));
                }
                break;

            default:
                DBUG_ASSERT ((arg_spec != NULL), "Illegal scheduling specification");
            }
            break;

        case 'v':
            DBUG_ASSERT (0, "Vector arguments for scheduling disciplines not yet"
                            " implemented");
            break;

        default:
            DBUG_ASSERT ((arg_spec != NULL), "Illegal scheduling specification");
        }

        arg_spec = strtok (NULL, ",");
        exprs = EXPRS_NEXT (exprs);
    }

    if (exprs != NULL) {
        ABORT (line, ("Scheduling discipline '%s` expects %d arguments "
                      "(too many specified)",
                      sched->discipline, sched->num_args));
    }

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   sched_t *SCHMakeScheduling( va_alist)
 *
 * description:
 *   This may be used to generate arbitray schedulings. The first paramter in
 *   the variable parameter list must always be of type 'char*' and provide the
 *   name of the scheduling discipline. The following parameters must be defined
 *   according to the scheduling specification table. For a scheduling argument
 *   of type 'n' a parameter of type 'int' is required, for 'i' a parameter of
 *   type 'char*'. A scheduling argument specification of 'x' requires two
 *   parameters to be given to SCHMakeScheduling(). The first must be of type
 *   'char*' and the second og type 'int'. If the first argument given is NULL,
 *   the the intger argument is used, otherwise the identifier specification.
 *
 ******************************************************************************/

sched_t *SCHMakeScheduling (va_alist) va_dcl
{
    va_list args;
    char *discipline, *arg_spec, *tmp_id;
    sched_t *sched;
    int i, disc_no, tmp_num;

    DBUG_ENTER ("MakeScheduling");

    va_start (args);
    discipline = va_arg (args, char *);

    disc_no = 0;

    while ((scheduler_table[disc_no].discipline[0] != '\0')
           && (0 != strcmp (scheduler_table[disc_no].discipline, discipline))) {
        disc_no++;
    }

    DBUG_ASSERT ((scheduler_table[disc_no].discipline[0] != '\0'),
                 "Infered scheduling discipline not implemented");

    sched = (sched_t *)Malloc (sizeof (sched_t));

    sched->discipline = scheduler_table[disc_no].discipline;
    sched->class = scheduler_table[disc_no].class;
    sched->line = -1;

    sched->num_args = scheduler_table[disc_no].num_args;

    if (sched->num_args == 0) {
        sched->args = NULL;
    } else {
        sched->args = (sched_arg_t *)Malloc (sched->num_args * sizeof (sched_arg_t));
    }

    arg_spec = strtok (scheduler_table[disc_no].arg_spec, ",");

    for (i = 0; i < sched->num_args; i++) {
        DBUG_ASSERT ((arg_spec != NULL), "Illegal scheduling specification");

        switch (arg_spec[1]) {
        case '\0':
            switch (arg_spec[0]) {
            case 'n':
                sched->args[i].arg_type = AT_num;
                sched->args[i].arg.num = va_arg (args, int);
                break;

            case 'i':
                sched->args[i].arg_type = AT_id;
                sched->args[i].arg.id = va_arg (args, char *);
                break;

            case 'x':
                tmp_id = va_arg (args, char *);
                tmp_num = va_arg (args, int);
                if (tmp_id == NULL) {
                    sched->args[i].arg_type = AT_num_for_id;
                    sched->args[i].arg.num = tmp_num;
                } else {
                    sched->args[i].arg_type = AT_id;
                    sched->args[i].arg.id = tmp_id;
                }
                break;

            default:
                DBUG_ASSERT ((arg_spec != NULL), "Illegal scheduling specification");
            }
            break;

        case 'v':
            DBUG_ASSERT (0, "Vector arguments for scheduling disciplines not yet"
                            " implemented");
            break;

        default:
            DBUG_ASSERT ((arg_spec != NULL), "Illegal scheduling specification");
        }

        arg_spec = strtok (NULL, ",");
    }

    va_end (args);

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   sched_t *SCHMakeSchedulingByPragma( node *ap_node, int line)
 *
 * description:
 *   This function constructs a scheduling specification from a 'Scheduling'
 *   entry of the wlcomp pragma. The pragma information is compared with the
 *   scheduling specification table. The line number of the pragma in the
 *   original source code is required to produce sufficient error messages.
 *
 ******************************************************************************/

sched_t *
SCHMakeSchedulingByPragma (node *ap_node, int line)
{
    sched_t *sched = NULL;
    int i = 0;

    DBUG_ENTER ("MakeSchedulingByPragma");

    while ((scheduler_table[i].discipline[0] != '\0')
           && (0 != strcmp (scheduler_table[i].discipline, AP_NAME (ap_node)))) {
        i++;
    }

    if (scheduler_table[i].discipline[0] != '\0') {
        sched = (sched_t *)Malloc (sizeof (sched_t));
        sched->discipline = scheduler_table[i].discipline;
        /*
         * Because sched is an object of an abstract data type, we may share it
         * inside this module.
         */
        sched->class = scheduler_table[i].class;
        sched->num_args = scheduler_table[i].num_args;
        sched->args = (sched_arg_t *)Malloc (sched->num_args * sizeof (sched_arg_t));
        sched->line = line;

        sched = CheckSchedulingArgs (sched, scheduler_table[i].arg_spec,
                                     AP_ARGS (ap_node), line);
    } else {
        ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                      "Scheduling(): Unknown scheduler"));
    }

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   sched_t *SCHRemoveScheduling( sched_t *sched)
 *
 * description:
 *   This function may be used to release the resources bound to a data object
 *   of the abstract data type for the representation of schedulings.
 *
 ******************************************************************************/

sched_t *
SCHRemoveScheduling (sched_t *sched)
{
    int i;

    DBUG_ENTER ("SCHRemoveScheduling");

    /*
     * The discipline string must not be freed since it is only a pointer
     * to the respective entry of the scheduler table.
     */

    if (sched->num_args > 0) {
        for (i = 0; i < sched->num_args; i++) {
            switch (sched->args[i].arg_type) {
            case AT_num_vec:
                /* here is no break missing! */
            case AT_num_for_id_vec:
                Free (sched->args[i].arg.num_vec);
                break;

            case AT_id_vec:
                Free (sched->args[i].arg.id_vec);
                break;

            default:
                break;
            }
        }

        Free (sched->args);
    }

    sched = Free (sched);

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   sched_t *SCHCopyScheduling( sched_t *sched)
 *
 * description:
 *   This function may be used to copy a data object
 *   of the abstract data type for the representation of schedulings.
 *
 ******************************************************************************/

sched_t *
SCHCopyScheduling (sched_t *sched)
{
    int i;
    sched_t *new_sched;

    DBUG_ENTER ("SCHCopyScheduling");

    new_sched = (sched_t *)Malloc (sizeof (sched_t));

    new_sched->discipline = sched->discipline;
    /*
     * The discipline string must not be copied since it is only a pointer
     * to the respective entry of the scheduler table.
     */

    new_sched->class = sched->class;
    new_sched->line = sched->line;
    new_sched->num_args = sched->num_args;

    if (sched->num_args > 0) {
        new_sched->args = (sched_arg_t *)Malloc (sched->num_args * sizeof (sched_arg_t));

        for (i = 0; i < sched->num_args; i++) {
            new_sched->args[i].arg_type = sched->args[i].arg_type;
            switch (sched->args[i].arg_type) {
            case AT_num:
            case AT_num_for_id:
                new_sched->args[i].arg.num = sched->args[i].arg.num;
                break;

            case AT_id:
                new_sched->args[i].arg.id = sched->args[i].arg.id;
                break;

            default:
                break;
            }
        }

    } else {
        new_sched->args = NULL;
    }

    DBUG_RETURN (new_sched);
}

/******************************************************************************
 *
 * function:
 *   sched_t *SCHPrecompileScheduling( sched_t *sched)
 *
 * description:
 *   Since identifier names are stored within the abstract scheduling
 *   representations, these are subject to renaming during the precompilation
 *   compiler phase. The actual renaming is done by the help of the function
 *   PRECRenameLocalIdentifier().
 *
 ******************************************************************************/

sched_t *
SCHPrecompileScheduling (sched_t *sched)
{
    int i;

    DBUG_ENTER ("SCHPrecompileScheduling");

    for (i = 0; i < sched->num_args; i++) {
        if (sched->args[i].arg_type == AT_id) {
            sched->args[i].arg.id = RenameLocalIdentifier (sched->args[i].arg.id);
        }
    }

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   void SCHPrintScheduling( FILE *handle, sched_t *sched)
 *
 * description:
 *
 *
 ******************************************************************************/

void
SCHPrintScheduling (FILE *outfile, sched_t *sched)
{
    int i;

    DBUG_ENTER ("SCHPrintScheduling");

    if (outfile == NULL) {
        /*
         * NULL -> stderr
         * This is done for use in a debugging session.
         */
        outfile = stderr;
    }

    if (sched != NULL) {
        fprintf (outfile, "%s(", sched->discipline);

        for (i = 0; i < sched->num_args - 1; i++) {
            switch (sched->args[i].arg_type) {
            case AT_num:
            case AT_num_for_id:
                fprintf (outfile, "%d, ", sched->args[i].arg.num);
                break;

            case AT_id:
                fprintf (outfile, "%s, ", sched->args[i].arg.id);
                break;

            default:
                break;
            }
        }

        if (sched->num_args > 0) {
            switch (sched->args[sched->num_args - 1].arg_type) {
            case AT_num:
            case AT_num_for_id:
                fprintf (outfile, "%d)", sched->args[sched->num_args - 1].arg.num);
                break;

            case AT_id:
                fprintf (outfile, "%s)", sched->args[sched->num_args - 1].arg.id);
                break;

            default:
                break;
            }
        } else {
            fprintf (outfile, ")");
        }
    } else {
        fprintf (outfile, "NULL");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void SCHCheckSuitabilityConstSeg( sched_t *sched)
 *   void SCHCheckSuitabilityVarSeg( sched_t *sched)
 *   void SCHCheckSuitabilityWithloop( sched_t *sched)
 *
 * description:
 *   These functions check whether the given scheduling is suitable for the
 *   respective case. If not, an error message is produced and the compilation
 *   is stopped automatically after the current compilation phase.
 *
 ******************************************************************************/

void
SCHCheckSuitabilityConstSeg (sched_t *sched)
{
    DBUG_ENTER ("SCHCheckSuitabilityConstSeg");

    if ((sched->class != SC_const_seg) && (sched->class != SC_var_seg)) {
        ERROR (sched->line, ("Scheduling discipline '%s` is not suitable for "
                             "constant segments",
                             sched->discipline));
    }

    DBUG_VOID_RETURN;
}

void
SCHCheckSuitabilityVarSeg (sched_t *sched)
{
    DBUG_ENTER ("SCHCheckSuitabilityVarSeg");

    if (sched->class != SC_var_seg) {
        ERROR (sched->line, ("Scheduling discipline '%s` is not suitable for "
                             "variable segments",
                             sched->discipline));
    }

    DBUG_VOID_RETURN;
}

void
SCHCheckSuitabilityWithloop (sched_t *sched)
{
    DBUG_ENTER ("SCHCheckSuitabilityWithloop");

    if (sched->class != SC_withloop) {
        ERROR (sched->line, ("Scheduling discipline '%s` is not suitable for "
                             "with-loops",
                             sched->discipline));
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   bool SCHAdjustmentRequired( int dim, node *wlseg)
 *
 * description:
 *   This function decides whether or not bounds generated by a particular
 *   scheduling for a particular segment must be adjusted to be compatible
 *   to a given unrolling.
 *
 ******************************************************************************/

bool
SCHAdjustmentRequired (int dim, node *wlseg)
{
    int i = 0;
    bool adjust;

    DBUG_ENTER ("SCHAdjustmentRequired");

    while (0
           != strcmp (((sched_t *)WLSEGX_SCHEDULING (wlseg))->discipline,
                      scheduler_table[i].discipline)) {
        i++;
    }

    adjust
      = ((dim <= scheduler_table[i].max_sched_dim)
         && ((NODE_TYPE (wlseg) == N_WLsegVar)
             || (((!scheduler_table[i].adjust_flag) || (WLSEG_HOMSV (wlseg)[dim] == 0))
                 && (WLSEG_SV (wlseg)[dim] > 1))));

    DBUG_RETURN (adjust);
}

/******************************************************************************
 *
 * function:
 *   node *CompileSchedulingArgs( int seg_id, sched_t *sched, node *args)
 *
 * description:
 *   This function converts the arguments of an abstract scheduling
 *   specification into ICM arguments.
 *   Numbers in the case of type 'x' scheduling arguments are transformed
 *   into string representations.
 *
 *   The segment ID is added to all scheduler ICMs as first argument.
 *
 ******************************************************************************/

static node *
CompileSchedulingArgs (int seg_id, sched_t *sched, node *args)
{
    node *new_arg;
    int i;

    DBUG_ENTER ("CompileSchedulingArgs");

    if (sched != NULL) {
        for (i = 0; i < sched->num_args; i++) {
            switch (sched->args[i].arg_type) {
            case AT_num:
                new_arg = MakeNum (sched->args[i].arg.num);
                break;

            case AT_id:
                new_arg = MakeId (StringCopy (sched->args[i].arg.id), NULL, ST_regular);
                break;

            case AT_num_for_id:
                new_arg = MakeId (itoa (sched->args[i].arg.num), NULL, ST_regular);
                break;

            default:
                new_arg = NULL;
                DBUG_ASSERT (0, "Vector arguments for scheduling disciplines not yet"
                                " implemented");
            }

            args = MakeExprs (new_arg, args);
        }
    }

    args = MakeExprs (MakeNum (seg_id), args);

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *CompileConstSegSchedulingArgs( char *wl_name, node *wlseg,
 *                                        sched_t *sched)
 *
 * description:
 *   In addition to their individual arguments, scheduling ICMs have addtional
 *   arguments depending on their position. Schedulings for constant segments
 *   are equipped with detailed segment information. These are the segment's
 *   dimensionality, its bounds and its outermost unrolling vector.
 *
 ******************************************************************************/

static node *
CompileConstSegSchedulingArgs (char *wl_name, node *wlseg, sched_t *sched)
{
    node *index, *args;
    int d;

    DBUG_ENTER ("CompileConstSegSchedulingArgs");

    DBUG_ASSERT ((NODE_TYPE (wlseg) == N_WLseg), "no constant segment found!");

    args = NULL;

    if (sched != NULL) {
        for (d = WLSEG_DIMS (wlseg) - 1; d >= 0; d--) {
            if (SCHAdjustmentRequired (d, wlseg)) {
                args = MakeExprs (MakeNum (1), args);
            } else {
                args = MakeExprs (MakeNum (WLSEG_SV (wlseg)[d]), args);
            }
        }
    }

    for (d = WLSEG_DIMS (wlseg) - 1; d >= 0; d--) {
        index = NodeOrInt_MakeIndex (NODE_TYPE (wlseg),
                                     WLSEGX_IDX_GET_ADDR (wlseg, IDX_MAX, d), d, wl_name,
                                     TRUE, TRUE);
        DBUG_ASSERT ((index != NULL), "illegal supremum found!");
        args = MakeExprs (index, args);
    }

    for (d = WLSEG_DIMS (wlseg) - 1; d >= 0; d--) {
        index = NodeOrInt_MakeIndex (NODE_TYPE (wlseg),
                                     WLSEGX_IDX_GET_ADDR (wlseg, IDX_MIN, d), d, wl_name,
                                     TRUE, TRUE);
        DBUG_ASSERT ((index != NULL), "illegal infimum found!");
        args = MakeExprs (index, args);
    }

    args = MakeExprs (MakeNum (WLSEG_DIMS (wlseg)), args);

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *CompileVarSegSchedulingArgs( char *wl_name, node *wlseg,
 *                                      sched_t *sched)
 *
 * description:
 *   In addition to their individual arguments, scheduling ICMs have additional
 *   arguments depending on their position. Currently, schedulings for variable
 *   segments do not have general arguments.
 *
 ******************************************************************************/

static node *
CompileVarSegSchedulingArgs (char *wl_name, node *wlseg, sched_t *sched)
{
    node *index, *args;
    int d;

    DBUG_ENTER ("CompileVarSegSchedulingArgs");

    DBUG_ASSERT ((NODE_TYPE (wlseg) == N_WLsegVar), "no var. segment found!");

    args = NULL;

    if (sched != NULL) {
        for (d = WLSEGVAR_DIMS (wlseg) - 1; d >= 0; d--) {
            args = MakeExprs (MakeNum (1), args);
        }
    }

    for (d = WLSEGVAR_DIMS (wlseg) - 1; d >= 0; d--) {
        index = NodeOrInt_MakeIndex (NODE_TYPE (wlseg),
                                     WLSEGX_IDX_GET_ADDR (wlseg, IDX_MAX, d), d, wl_name,
                                     TRUE, TRUE);
        DBUG_ASSERT ((index != NULL), "illegal supremum found!");
        args = MakeExprs (index, args);
    }

    for (d = WLSEGVAR_DIMS (wlseg) - 1; d >= 0; d--) {
        index = NodeOrInt_MakeIndex (NODE_TYPE (wlseg),
                                     WLSEGX_IDX_GET_ADDR (wlseg, IDX_MIN, d), d, wl_name,
                                     TRUE, TRUE);
        DBUG_ASSERT ((index != NULL), "illegal infimum found!");
        args = MakeExprs (index, args);
    }

    args = MakeExprs (MakeNum (WLSEGVAR_DIMS (wlseg)), args);

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *CompileScheduling( int seg_id, char *wl_name, sched_t *sched,
 *                            node *arg_node, char *suffix)
 *
 * description:
 *   This function compiles abstract scheduling specifications to ICMs
 *    "SAC_MT_SCHEDULER_<discipline>_BEGIN"
 *    "SAC_MT_SCHEDULER_<discipline>_END"
 *    "SAC_MT_SCHEDULER_<discipline>_INIT"
 *   depending on the parameter 'suffix'.
 *
 ******************************************************************************/

static node *
CompileScheduling (int seg_id, char *wl_name, sched_t *sched, node *arg_node,
                   char *suffix)
{
    node *icm, *general_args;
    char *name;

    DBUG_ENTER ("CompileScheduling");

    if (sched != NULL) {
        name = (char *)Malloc (sizeof (char)
                               * (strlen (sched->discipline) + strlen (suffix) + 15));
        sprintf (name, "MT_SCHEDULER_%s_%s", sched->discipline, suffix);
    } else {
        name = (char *)Malloc (sizeof (char) * (strlen (suffix) + 15));
        sprintf (name, "MT_SCHEDULER_%s", suffix);
    }

    switch (NODE_TYPE (arg_node)) {
    case N_WLseg:
        general_args = CompileConstSegSchedulingArgs (wl_name, arg_node, sched);
        break;

    case N_WLsegVar:
        general_args = CompileVarSegSchedulingArgs (wl_name, arg_node, sched);
        break;

    default:
        general_args = NULL;
        DBUG_ASSERT ((0), "wrong node type found");
    }

    icm = MakeIcm (name, CompileSchedulingArgs (seg_id, sched, general_args));

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * function:
 *   node *SCHCompileSchedulingBegin( int seg_id, char *wl_name, sched_t *sched,
 *                                    node *arg_node)
 *
 *   node *SCHCompileSchedulingEnd( int seg_id, char *wl_name, sched_t *sched,
 *                                  node *arg_node)
 *
 *   node *SCHCompileSchedulingInit( int seg_id, char *wl_name, sched_t *sched,
 *                                   node *arg_node)
 *
 * description:
 *
 *   These functions initiate the compilation of abstract scheduling
 *   specifications into ICMs, where each scheduling is associated with three
 *   ICMs. Whereas the former two enclose the with-loop code to be scheduled,
 *   the latter one initializes potentially needed internal data structures
 *   of the scheduling facility.
 *
 *   The segment ID specifies the position of the scheduler within an SPMD
 *   section. It is needed to identify the appropriate set of scheduler-internal
 *   data structures.
 *
 ******************************************************************************/

node *
SCHCompileSchedulingBegin (int seg_id, char *wl_name, sched_t *sched, node *arg_node)
{
    node *ret_node;

    DBUG_ENTER ("SCHCompileSchedulingBegin");

    ret_node = CompileScheduling (seg_id, wl_name, sched, arg_node, "BEGIN");

    DBUG_RETURN (ret_node);
}

node *
SCHCompileSchedulingEnd (int seg_id, char *wl_name, sched_t *sched, node *arg_node)
{
    node *ret_node;

    DBUG_ENTER ("SCHCompileSchedulingEnd");

    ret_node = CompileScheduling (seg_id, wl_name, sched, arg_node, "END");

    DBUG_RETURN (ret_node);
}

node *
SCHCompileSchedulingInit (int seg_id, char *wl_name, sched_t *sched, node *arg_node)
{
    node *ret_node;

    DBUG_ENTER ("SCHCompileSchedulingInit");

    ret_node = CompileScheduling (seg_id, wl_name, sched, arg_node, "INIT");

    DBUG_RETURN (ret_node);
}
