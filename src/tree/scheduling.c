/*
 *
 * $Log$
 * Revision 3.46  2004/11/27 01:42:37  ktr
 * included renameidentifiers.h
 *
 * Revision 3.45  2004/11/27 00:53:42  mwe
 * fixing for linking
 *
 * Revision 3.44  2004/11/27 00:23:09  cg
 * PRECrenameLocalIdentifiers renamed to RIDrenameLocalIdentifiers
 *
 * Revision 3.43  2004/11/26 00:22:56  mwe
 * SacDevCamp Dk: Compiles!
 *
 * Revision 3.42  2004/11/24 22:52:53  ktr
 * dependency from precompile.h removed.
 *
 * Revision 3.41  2004/09/30 19:52:05  sah
 * fixed hidden datatypes
 *
 * Revision 3.40  2004/08/02 14:07:58  sah
 * added lots of ugly defines to remove
 * compiler warning when using the old
 * ast;)
 *
 * Revision 3.39  2004/08/01 16:28:53  sah
 * all abstract types are now abstract structures
 * instead of void pointers
 *
 * Revision 3.38  2004/08/01 13:17:08  ktr
 * Added SCHMMVScheduling, SCHMMVTasksel
 *
 * Revision 3.37  2003/04/14 14:18:34  sbs
 * forgot to remove local parameter ....
 *
 * Revision 3.36  2003/04/14 14:16:56  sbs
 * second parameter for va_start created in SCHMakeScheduling.
 *
 * Revision 3.35  2003/04/14 13:54:07  sbs
 * header file stdarg.h used instead of varargs.h which is not
 * available under Linux.
 *
 * Revision 3.34  2002/07/15 14:44:38  dkr
 * function signatures modified
 *
 * Revision 3.33  2001/11/21 13:43:57  dkr
 * print routines modified
 *
 * Revision 3.32  2001/07/04 10:11:49  ben
 * code beautiefied
 *
 * Revision 3.31  2001/06/27 14:37:00  ben
 * last routines for tasksel-pragma implemented
 *
 * Revision 3.30  2001/06/20 12:28:33  ben
 * most first versions for SchedulingWithTasksel functions implemented
 * two functions are not yet ready for use
 *
 * Revision 3.29  2001/06/19 12:29:26  ben
 * Entry of Self in SchedulerTable modified
 *
 * Revision 3.28  2001/06/13 13:07:03  ben
 * SCHMakeTaskselByPragma, SCHRemoveTasksel, SCHCopyTasksel,
 * SCHPrecompileTasksel, SCHPrintTasksel, CheckTaskselArgs
 * taskselector_table[] added
 *
 * Revision 3.27  2001/05/22 15:25:41  dkr
 * fixed a bug in SCHMakeSchedulingByPragma():
 * if (sched->num_args==0) is hold sched->args is set to NULL.
 *
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

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "scheduling.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "traverse.h"
#include "Error.h"
#include "wl_bounds.h"
#include "dbug.h"
#include "renameidentifiers.h"

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

struct SCHED_T {
    char *discipline;
    sched_class_t class;
    int line;
    int num_args;
    sched_arg_t *args;
};

#ifndef NEW_AST
typedef struct SCHED_T sched_t;
typedef sched_t SCHsched_t;
#endif

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
  {"Static", SC_var_seg, 1, 0, 0, ""},
  {"Self", SC_var_seg, 1, 0, 1, "i"},
  {"Affinity", SC_var_seg, 1, 0, 0, ""},
  {"AllByOne", SC_var_seg, 0, 0, 1, "i"},
  {"BlockBySome", SC_const_seg, 0, 0, 2, "i,i"},
  {"WL_Static", SC_withloop, 0, 0, 0, ""},
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

    arg_spec = ILIBstrTok (spec, ",");

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
                sched->args[i].arg.id = ILIBstringCopy (ID_NAME (expr));
                break;

            case 'x':
                switch (NODE_TYPE (expr)) {
                case N_num:
                    sched->args[i].arg_type = AT_num_for_id;
                    sched->args[i].arg.num = NUM_VAL (expr);
                    break;
                case N_id:
                    sched->args[i].arg_type = AT_id;
                    sched->args[i].arg.id = ILIBstringCopy (ID_NAME (expr));
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

        arg_spec = ILIBstrTok (NULL, ",");
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
 *   sched_t *SCHMakeScheduling( char *discipline, ...)
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

sched_t *
SCHmakeScheduling (char *discipline, ...)
{
    va_list args;
    char *arg_spec, *tmp_id;
    sched_t *sched;
    int i, disc_no, tmp_num;

    DBUG_ENTER ("SCHmakeScheduling");

    va_start (args, discipline);

    disc_no = 0;

    while ((scheduler_table[disc_no].discipline[0] != '\0')
           && (0 != strcmp (scheduler_table[disc_no].discipline, discipline))) {
        disc_no++;
    }

    DBUG_ASSERT ((scheduler_table[disc_no].discipline[0] != '\0'),
                 "Infered scheduling discipline not implemented");

    sched = (sched_t *)ILIBmalloc (sizeof (sched_t));

    sched->discipline = scheduler_table[disc_no].discipline;
    sched->class = scheduler_table[disc_no].class;
    sched->line = -1;

    sched->num_args = scheduler_table[disc_no].num_args;

    if (sched->num_args == 0) {
        sched->args = NULL;
    } else {
        sched->args = (sched_arg_t *)ILIBmalloc (sched->num_args * sizeof (sched_arg_t));
    }

    arg_spec = ILIBstrTok (scheduler_table[disc_no].arg_spec, ",");

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

        arg_spec = ILIBstrTok (NULL, ",");
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
SCHmakeSchedulingByPragma (node *ap_node, int line)
{
    sched_t *sched = NULL;
    int i = 0;

    DBUG_ENTER ("SCHmakeSchedulingByPragma");

    while ((scheduler_table[i].discipline[0] != '\0')
           && (0 != strcmp (scheduler_table[i].discipline, AP_NAME (ap_node)))) {
        i++;
    }

    if (scheduler_table[i].discipline[0] != '\0') {
        sched = (sched_t *)ILIBmalloc (sizeof (sched_t));
        sched->discipline = scheduler_table[i].discipline;
        /*
         * Because sched is an object of an abstract data type, we may share it
         * inside this module.
         */
        sched->class = scheduler_table[i].class;
        sched->num_args = scheduler_table[i].num_args;
        if (sched->num_args == 0) {
            sched->args = NULL;
        } else {
            sched->args
              = (sched_arg_t *)ILIBmalloc (sched->num_args * sizeof (sched_arg_t));
        }
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
SCHremoveScheduling (sched_t *sched)
{
    int i;

    DBUG_ENTER ("SCHremoveScheduling");

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
                ILIBfree (sched->args[i].arg.num_vec);
                break;

            case AT_id_vec:
                ILIBfree (sched->args[i].arg.id_vec);
                break;

            default:
                break;
            }
        }

        ILIBfree (sched->args);
    }

    sched = ILIBfree (sched);

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
SCHcopyScheduling (sched_t *sched)
{
    int i;
    sched_t *new_sched;

    DBUG_ENTER ("SCHcopyScheduling");

    new_sched = (sched_t *)ILIBmalloc (sizeof (sched_t));

    new_sched->discipline = sched->discipline;
    /*
     * The discipline string must not be copied since it is only a pointer
     * to the respective entry of the scheduler table.
     */

    new_sched->class = sched->class;
    new_sched->line = sched->line;
    new_sched->num_args = sched->num_args;

    if (sched->num_args > 0) {
        new_sched->args
          = (sched_arg_t *)ILIBmalloc (sched->num_args * sizeof (sched_arg_t));

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
 *   RIDrenameLocalIdentifier().
 *
 ******************************************************************************/

sched_t *
SCHprecompileScheduling (sched_t *sched)
{
    int i;

    DBUG_ENTER ("SCHprecompileScheduling");

    for (i = 0; i < sched->num_args; i++) {
        if (sched->args[i].arg_type == AT_id) {
            sched->args[i].arg.id = RIDrenameLocalIdentifier (sched->args[i].arg.id);
        }
    }

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   sched_t *SCHMMVScheduling( sched_t *sched, LUT_t lut)
 *
 * description:
 *   Since identifier names are stored within the abstract scheduling
 *   representations, these are subject to renaming during the markmemvals
 *   compiler phase.
 *
 ******************************************************************************/

sched_t *
SCHmarkmemvalsScheduling (sched_t *sched, lut_t *lut)
{
    int i;
    char *new_name;

    DBUG_ENTER ("SCHmarkmemvalsScheduling");

    for (i = 0; i < sched->num_args; i++) {
        if (sched->args[i].arg_type == AT_id) {
            new_name = LUTsearchInLutSs (lut, sched->args[i].arg.id);
            if (new_name != sched->args[i].arg.id) {
                sched->args[i].arg.id = ILIBfree (sched->args[i].arg.id);
                sched->args[i].arg.id = ILIBstringCopy (new_name);
            }
        }
    }

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   void SCHPrintScheduling( FILE *outfile, sched_t *sched)
 *
 * description:
 *
 *
 ******************************************************************************/

void
SCHprintScheduling (FILE *outfile, sched_t *sched)
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
        fprintf (outfile, "%s( ", sched->discipline);

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
SCHcheckSuitabilityConstSeg (sched_t *sched)
{
    DBUG_ENTER ("SCHcheckSuitabilityConstSeg");

    if ((sched->class != SC_const_seg) && (sched->class != SC_var_seg)) {
        ERROR (sched->line, ("Scheduling discipline '%s` is not suitable for "
                             "constant segments",
                             sched->discipline));
    }

    DBUG_VOID_RETURN;
}

void
SCHcheckSuitabilityVarSeg (sched_t *sched)
{
    DBUG_ENTER ("SCHcheckSuitabilityVarSeg");

    if (sched->class != SC_var_seg) {
        ERROR (sched->line, ("Scheduling discipline '%s` is not suitable for "
                             "variable segments",
                             sched->discipline));
    }

    DBUG_VOID_RETURN;
}

void
SCHcheckSuitabilityWithloop (sched_t *sched)
{
    DBUG_ENTER ("SCHcheckSuitabilityWithloop");

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
SCHadjustmentRequired (int dim, node *wlseg)
{
    int i = 0;
    bool adjust;

    DBUG_ENTER ("SCHadjustmentRequired");

    while (0
           != strcmp (((sched_t *)WLSEGX_SCHEDULING (wlseg))->discipline,
                      scheduler_table[i].discipline)) {
        i++;
    }

    adjust
      = ((dim <= scheduler_table[i].max_sched_dim)
         && ((NODE_TYPE (wlseg) == N_wlsegvar)
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
                new_arg = TBmakeNum (sched->args[i].arg.num);
                break;

            case AT_id:
                new_arg = TCmakeIdCopyString (sched->args[i].arg.id);
                break;

            case AT_num_for_id:
                new_arg = TCmakeIdCopyString (ILIBitoa (sched->args[i].arg.num));
                break;

            default:
                new_arg = NULL;
                DBUG_ASSERT (0, "Vector arguments for scheduling disciplines not yet"
                                " implemented");
            }

            args = TBmakeExprs (new_arg, args);
        }
    }

    args = TBmakeExprs (TBmakeNum (seg_id), args);

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *CompileConstSegSchedulingArgs( ids *wl_ids, node *wlseg,
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
CompileConstSegSchedulingArgs (node *wl_ids, node *wlseg, sched_t *sched)
{
    node *index, *args;
    int d;

    DBUG_ENTER ("CompileConstSegSchedulingArgs");

    DBUG_ASSERT ((NODE_TYPE (wlseg) == N_wlseg), "no constant segment found!");

    args = NULL;

    if (sched != NULL) {
        for (d = WLSEG_DIMS (wlseg) - 1; d >= 0; d--) {
            if (SCHadjustmentRequired (d, wlseg)) {
                args = TBmakeExprs (TBmakeNum (1), args);
            } else {
                args = TBmakeExprs (TBmakeNum (WLSEG_SV (wlseg)[d]), args);
            }
        }
    }

    for (d = WLSEG_DIMS (wlseg) - 1; d >= 0; d--) {
        index
          = WLBnodeOrIntMakeIndex (NODE_TYPE (wlseg),
                                   WLSEGX_IDX_GET_ADDR (wlseg, IDX_MAX, d), d, wl_ids);
        DBUG_ASSERT ((index != NULL), "illegal supremum found!");
        args = TBmakeExprs (index, args);
    }

    for (d = WLSEG_DIMS (wlseg) - 1; d >= 0; d--) {
        index
          = WLBnodeOrIntMakeIndex (NODE_TYPE (wlseg),
                                   WLSEGX_IDX_GET_ADDR (wlseg, IDX_MIN, d), d, wl_ids);
        DBUG_ASSERT ((index != NULL), "illegal infimum found!");
        args = TBmakeExprs (index, args);
    }

    args = TBmakeExprs (TBmakeNum (WLSEG_DIMS (wlseg)), args);

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *CompileVarSegSchedulingArgs( ids *wl_ids, node *wlseg,
 *                                      sched_t *sched)
 *
 * description:
 *   In addition to their individual arguments, scheduling ICMs have additional
 *   arguments depending on their position. Currently, schedulings for variable
 *   segments do not have general arguments.
 *
 ******************************************************************************/

static node *
CompileVarSegSchedulingArgs (node *wl_ids, node *wlseg, sched_t *sched)
{
    node *index, *args;
    int d;

    DBUG_ENTER ("CompileVarSegSchedulingArgs");

    DBUG_ASSERT ((NODE_TYPE (wlseg) == N_wlsegvar), "no var. segment found!");

    args = NULL;

    if (sched != NULL) {
        for (d = WLSEGVAR_DIMS (wlseg) - 1; d >= 0; d--) {
            args = TBmakeExprs (TBmakeNum (1), args);
        }
    }

    for (d = WLSEGVAR_DIMS (wlseg) - 1; d >= 0; d--) {
        index
          = WLBnodeOrIntMakeIndex (NODE_TYPE (wlseg),
                                   WLSEGX_IDX_GET_ADDR (wlseg, IDX_MAX, d), d, wl_ids);
        DBUG_ASSERT ((index != NULL), "illegal supremum found!");
        args = TBmakeExprs (index, args);
    }

    for (d = WLSEGVAR_DIMS (wlseg) - 1; d >= 0; d--) {
        index
          = WLBnodeOrIntMakeIndex (NODE_TYPE (wlseg),
                                   WLSEGX_IDX_GET_ADDR (wlseg, IDX_MIN, d), d, wl_ids);
        DBUG_ASSERT ((index != NULL), "illegal infimum found!");
        args = TBmakeExprs (index, args);
    }

    args = TBmakeExprs (TBmakeNum (WLSEGVAR_DIMS (wlseg)), args);

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *CompileScheduling( int seg_id, ids *wl_ids, sched_t *sched,
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
CompileScheduling (int seg_id, node *wl_ids, sched_t *sched, node *arg_node, char *suffix)
{
    node *icm, *general_args;
    char *name;

    DBUG_ENTER ("CompileScheduling");

    if (sched != NULL) {
        name = (char *)ILIBmalloc (sizeof (char)
                                   * (strlen (sched->discipline) + strlen (suffix) + 15));
        sprintf (name, "MT_SCHEDULER_%s_%s", sched->discipline, suffix);
    } else {
        name = (char *)ILIBmalloc (sizeof (char) * (strlen (suffix) + 15));
        sprintf (name, "MT_SCHEDULER_%s", suffix);
    }

    switch (NODE_TYPE (arg_node)) {
    case N_wlseg:
        general_args = CompileConstSegSchedulingArgs (wl_ids, arg_node, sched);
        break;

    case N_wlsegvar:
        general_args = CompileVarSegSchedulingArgs (wl_ids, arg_node, sched);
        break;

    default:
        general_args = NULL;
        DBUG_ASSERT ((0), "wrong node type found");
    }

    icm = TBmakeIcm (name, CompileSchedulingArgs (seg_id, sched, general_args));

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * function:
 *   node *SCHCompileSchedulingBegin( int seg_id, ids *wl_ids, sched_t *sched,
 *                                    node *arg_node)
 *
 *   node *SCHCompileSchedulingEnd( int seg_id, ids *wl_ids, sched_t *sched,
 *                                  node *arg_node)
 *
 *   node *SCHCompileSchedulingInit( int seg_id, ids *wl_ids, sched_t *sched,
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
SCHcompileSchedulingBegin (int seg_id, node *wl_ids, sched_t *sched, node *arg_node)
{
    node *ret_node;

    DBUG_ENTER ("SCHcompileSchedulingBegin");

    ret_node = CompileScheduling (seg_id, wl_ids, sched, arg_node, "BEGIN");

    DBUG_RETURN (ret_node);
}

node *
SCHcompileSchedulingEnd (int seg_id, node *wl_ids, sched_t *sched, node *arg_node)
{
    node *ret_node;

    DBUG_ENTER ("SCHcompileSchedulingEnd");

    ret_node = CompileScheduling (seg_id, wl_ids, sched, arg_node, "END");

    DBUG_RETURN (ret_node);
}

node *
SCHcompileSchedulingInit (int seg_id, node *wl_ids, sched_t *sched, node *arg_node)
{
    node *ret_node;

    DBUG_ENTER ("SCHcompileSchedulingInit");

    ret_node = CompileScheduling (seg_id, wl_ids, sched, arg_node, "INIT");

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 *     All Code for the TaskSelector
 *
 ******************************************************************************/

/******************************************************************************
 *
 *     In tasksel_t is the arg-vector composed first of the dimensions
 *     for the arrangement of tasks followed by optional other arguments.
 *     Where num_args is the dims+number of optional arguments
 *
 ******************************************************************************/

struct TASKSEL_T {
    char *discipline;
    int line;
    int num_args;
    int *arg;
    int dims;
};

/******************************************************************************
 *
 * global variable: TaskSel_table[]
 *
 * description:
 *   This global variable defines a table of Taskselector specifications. Each
 *   Taskselector is described by three entries: the selectors name, the number
 *   of arguments he needs and the number of dimensions for task arrangements.
 *
 ******************************************************************************/

static struct {
    char *discipline;
    int num_args;
    int dims;
} taskselector_table[] = {
  /* Name           num_args,  dims*/
  {"Even", 2, 1},
  {"Factoring", 1, 1}};

/******************************************************************************
 *
 * function:
 *   tasksel_t *CheckTaskselArgs( tasksel_t *tasksel, node *exprs,
 *                                 int line)
 *
 * description:
 *   This function produces the arguments of a taskselector specification that
 *   is derived from a wlcomp pragma.The line number of the pragma in the
 *   original source code is required to produce sufficient error messages.
 *
 ******************************************************************************/

static tasksel_t *
CheckTaskselArgs (tasksel_t *tasksel, node *exprs, int line)
{
    int i;
    node *expr;

    DBUG_ENTER ("CheckTaskselArgs");

    for (i = 0; i < tasksel->num_args; i++) {

        if (exprs == NULL) {
            ABORT (line, ("Taskselector discipline '%s` expects %d arguments"
                          "(too few specified)",
                          tasksel->discipline, tasksel->num_args));
        }

        expr = EXPRS_EXPR (exprs);

        if (NODE_TYPE (expr) != N_num) {
            ABORT (line, ("Argument %d of taskselector discipline '%s` must be"
                          " a number",
                          i, tasksel->discipline));
        }

        tasksel->arg[i] = NUM_VAL (expr);

        exprs = EXPRS_NEXT (exprs);
    }

    if (exprs != NULL) {
        ABORT (line, ("Taskselector discipline '%s` expects %d arguments "
                      "(too many specified)",
                      tasksel->discipline, tasksel->num_args));
    }

    DBUG_RETURN (tasksel);
}

/******************************************************************************
 *
 * function:
 *   tasksel_t *SCHMakeTaskselByPragma( node *ap_node, int line)
 *
 * description:
 *   This function constructs a taskselector specification from a 'Tasksel'
 *   entry of the wlcomp pragma. The pragma information is compared with the
 *   taskselector specification table. The line number of the pragma in the
 *   original source code is required to produce sufficient error messages.
 *
 ******************************************************************************/

tasksel_t *
SCHmakeTaskselByPragma (node *ap_node, int line)
{
    tasksel_t *tasksel = NULL;
    int i = 0;

    DBUG_ENTER ("SCHmakeTaskselByPragma");

    while ((taskselector_table[i].discipline[0] != '\0')
           && (0 != strcmp (taskselector_table[i].discipline, AP_NAME (ap_node)))) {
        i++;
    }

    if (taskselector_table[i].discipline[0] != '\0') {
        tasksel = (tasksel_t *)ILIBmalloc (sizeof (tasksel_t));
        tasksel->discipline = taskselector_table[i].discipline;

        tasksel->num_args = taskselector_table[i].num_args;
        tasksel->dims = taskselector_table[i].dims;
        if (tasksel->num_args == 0) {
            tasksel->arg = NULL;
        } else {
            tasksel->arg = (int *)ILIBmalloc (tasksel->num_args * sizeof (int));
        }
        tasksel->line = line;

        tasksel = CheckTaskselArgs (tasksel, AP_ARGS (ap_node), line);
    } else {
        ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                      "Tasksel(): Unknown Taskselector"));
    }

    DBUG_RETURN (tasksel);
}

/******************************************************************************
 *
 * function:
 *   tasksel_t *SCHRemoveTasksel( tasksel_t *tasksel)
 *
 * description:
 *   This function may be used to release the resources bound to a data object
 *   of the abstract data type for the representation of a taskselctor.
 *
 ******************************************************************************/

tasksel_t *
SCHremoveTasksel (tasksel_t *tasksel)
{
    DBUG_ENTER ("SCHremoveTasksel");

    /*
     * The discipline string must not be freed since it is only a pointer
     * to the respective entry of the taskselector table.
     */
    if (tasksel->num_args > 0) {
        ILIBfree (tasksel->arg);
    }

    tasksel = ILIBfree (tasksel);

    DBUG_RETURN (tasksel);
}

/******************************************************************************
 *
 * function:
 *   tasksel_t *SCHCopyTaskselector( tasksel_t *tasksel)
 *
 * description:
 *   This function may be used to copy a data object
 *   of the abstract data type for the representation of taskselectors.
 *
 ******************************************************************************/

tasksel_t *
SCHcopyTasksel (tasksel_t *tasksel)
{
    int i;
    tasksel_t *new_tasksel;

    DBUG_ENTER ("SCHcopyTasksel");

    new_tasksel = (tasksel_t *)ILIBmalloc (sizeof (tasksel_t));

    new_tasksel->discipline = tasksel->discipline;
    /*
     * The discipline string must not be copied since it is only a pointer
     * to the respective entry of the scheduler table.
     */

    new_tasksel->line = tasksel->line;
    new_tasksel->num_args = tasksel->num_args;
    new_tasksel->dims = tasksel->dims;

    if (tasksel->num_args > 0) {
        new_tasksel->arg = (int *)ILIBmalloc (tasksel->num_args * sizeof (int));

        for (i = 0; i < tasksel->num_args; i++) {
            new_tasksel->arg[i] = tasksel->arg[i];
        }
    } else {
        new_tasksel->arg = NULL;
    }

    DBUG_RETURN (new_tasksel);
}

/******************************************************************************
 *
 * function:
 *   tasksel_t *SCHprecompileTasksel( tasksel_t *tasksel)
 *
 * description:
 *
 *
 ******************************************************************************/

tasksel_t *
SCHprecompileTasksel (tasksel_t *tasksel)
{
    DBUG_ENTER ("SCHprecompileTasksel");

    DBUG_RETURN (tasksel);
}

/******************************************************************************
 *
 * function:
 *   tasksel_t *SCHMMVTasksel( tasksel_t *tasksel, LUT_t lut)
 *
 * description:
 *
 *
 ******************************************************************************/

tasksel_t *
SCHmarkmemvalsTasksel (tasksel_t *tasksel, lut_t *lut)
{
    DBUG_ENTER ("SCHmarkmemvalsTasksel");

    DBUG_RETURN (tasksel);
}

/******************************************************************************
 *
 * function:
 *   void SCHPrintTasksel( FILE *outfile, tasksel_t *tasksel)
 *
 * description:
 *
 *
 ******************************************************************************/

void
SCHprintTasksel (FILE *outfile, tasksel_t *tasksel)
{
    int i;

    DBUG_ENTER ("SCHprintTasksel");

    if (outfile == NULL) {
        /*
         * NULL -> stderr
         * This is done for use in a debugging session.
         */
        outfile = stderr;
    }

    if (tasksel != NULL) {
        fprintf (outfile, "%s( ", tasksel->discipline);
        if (tasksel->num_args > 0) {
            for (i = 0; i < tasksel->num_args - 1; i++) {
                fprintf (outfile, "%d, ", tasksel->arg[i]);
            }
            fprintf (outfile, "%d", tasksel->arg[tasksel->num_args - 1]);
        }
        fprintf (outfile, ")");
    } else {
        fprintf (outfile, "NULL");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *CompileSchedulingWithTaskselArgs( int seg_id, sched_t *sched,
 *                                           tasksel_t *tasksel, node *args)
 *
 * description:
 *   This function converts the arguments of an abstract scheduling and
 *   taskselector specification into ICM arguments.
 *   Numbers in the case of type 'x' scheduling arguments are transformed
 *   into string representations.
 *
 *   The segment ID is added to all scheduler ICMs as first argument.
 *
 ******************************************************************************/

static node *
CompileSchedulingWithTaskselArgs (int seg_id, sched_t *sched, tasksel_t *tasksel,
                                  node *args)
{
    node *new_arg;
    int i;

    DBUG_ENTER ("CompileSchedulingWithTaskselArgs");

    if (sched != NULL) {

        if (tasksel != NULL) {
            for (i = tasksel->dims; i < tasksel->num_args; i++) {
                args = TBmakeExprs (TBmakeNum (tasksel->arg[i]), args);
            }

            args = TBmakeExprs (TBmakeNum (tasksel->num_args - tasksel->dims), args);
            args = TBmakeExprs (TBmakeNum (tasksel->dims), args);
            args = TBmakeExprs (TCmakeIdCopyString (tasksel->discipline), args);
        }

        for (i = 0; i < sched->num_args; i++) {
            switch (sched->args[i].arg_type) {
            case AT_num:
                new_arg = TBmakeNum (sched->args[i].arg.num);
                break;

            case AT_id:
                new_arg = TCmakeIdCopyString (sched->args[i].arg.id);
                break;

            case AT_num_for_id:
                new_arg = TCmakeIdCopyString (ILIBitoa (sched->args[i].arg.num));
                break;

            default:
                new_arg = NULL;
                DBUG_ASSERT (0, "Vector arguments for scheduling disciplines not yet"
                                " implemented");
            }

            args = TBmakeExprs (new_arg, args);
        }
    }

    args = TBmakeExprs (TBmakeNum (seg_id), args);

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *CompileConstSegSchedulingWithTaskselArgs( ids *wl_ids, node *wlseg,
 *                                        sched_t *sched, tasksel_t *tasksel)
 *
 * description:
 *   In addition to their individual arguments, scheduling ICMs have addtional
 *   arguments depending on their position and on the choosen taskselector.
 *   Schedulings for constant segments
 *   are equipped with detailed segment information. These are the segment's
 *   dimensionality, its bounds and its outermost unrolling vector.
 *
 ******************************************************************************/

static node *
CompileConstSegSchedulingWithTaskselArgs (node *wl_ids, node *wlseg, sched_t *sched,
                                          tasksel_t *tasksel)
{
    node *index, *args;
    int d;
    int pos;

    DBUG_ENTER ("CompileConstSegSchedulingWithTaskselArgs");

    DBUG_ASSERT ((NODE_TYPE (wlseg) == N_wlseg), "no constant segment found!");

    args = NULL;

    if (sched != NULL) {

        if (tasksel != NULL) {
            /*
             * creating a int vararg-vektor,
             * where for each taskselector dimension 1 is set, and for all other 0
             */

            pos = tasksel->dims - 1;
            for (d = WLSEG_DIMS (wlseg) - 1; d >= 0; d--) {
                if (tasksel->arg[pos] == d) {
                    if (pos > 0) {
                        pos--;
                    }
                    args = TBmakeExprs (TBmakeNum (1), args);
                } else {
                    args = TBmakeExprs (TBmakeNum (0), args);
                }
            }
        }

        for (d = WLSEG_DIMS (wlseg) - 1; d >= 0; d--) {
            if (SCHadjustmentRequired (d, wlseg)) {
                args = TBmakeExprs (TBmakeNum (1), args);
            } else {
                args = TBmakeExprs (TBmakeNum (WLSEG_SV (wlseg)[d]), args);
            }
        }
    }

    for (d = WLSEG_DIMS (wlseg) - 1; d >= 0; d--) {
        index
          = WLBnodeOrIntMakeIndex (NODE_TYPE (wlseg),
                                   WLSEGX_IDX_GET_ADDR (wlseg, IDX_MAX, d), d, wl_ids);
        DBUG_ASSERT ((index != NULL), "illegal supremum found!");
        args = TBmakeExprs (index, args);
    }

    for (d = WLSEG_DIMS (wlseg) - 1; d >= 0; d--) {
        index
          = WLBnodeOrIntMakeIndex (NODE_TYPE (wlseg),
                                   WLSEGX_IDX_GET_ADDR (wlseg, IDX_MIN, d), d, wl_ids);

        DBUG_ASSERT ((index != NULL), "illegal infimum found!");
        args = TBmakeExprs (index, args);
    }

    args = TBmakeExprs (TBmakeNum (WLSEG_DIMS (wlseg)), args);

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *CompileVarSegSchedulingWithTaskselArgs( ids *wl_ids, node *wlseg,
 *                                      sched_t *sched, tasksel_t *tasksel)
 *
 * description:
 *   In addition to their individual arguments, scheduling ICMs have additional
 *   arguments depending on their position and on the choosen taskselector.
 *   Currently, schedulings for variable
 *   segments do not have general arguments.
 *
 ******************************************************************************/

static node *
CompileVarSegSchedulingWithTaskselArgs (node *wl_ids, node *wlseg, sched_t *sched,
                                        tasksel_t *tasksel)
{
    node *index, *args;
    int d;
    int pos;

    DBUG_ENTER ("CompileVarSegSchedulingWithTaskselArgs");

    DBUG_ASSERT ((NODE_TYPE (wlseg) == N_wlsegvar), "no var. segment found!");

    args = NULL;

    if (sched != NULL) {

        if (tasksel != NULL) {
            /*
             * creating a int vararg-vektor,
             * where for each taskselector dimension 1 is set,
             * and for all other 0
             */
            pos = tasksel->dims - 1;
            for (d = WLSEG_DIMS (wlseg) - 1; d >= 0; d--) {
                if (tasksel->arg[pos] == d) {
                    if (pos > 0) {
                        pos--;
                    }
                    args = TBmakeExprs (TBmakeNum (1), args);
                } else {
                    args = TBmakeExprs (TBmakeNum (0), args);
                }
            }
        }

        for (d = WLSEGVAR_DIMS (wlseg) - 1; d >= 0; d--) {
            args = TBmakeExprs (TBmakeNum (1), args);
        }
    }

    for (d = WLSEGVAR_DIMS (wlseg) - 1; d >= 0; d--) {
        index
          = WLBnodeOrIntMakeIndex (NODE_TYPE (wlseg),
                                   WLSEGX_IDX_GET_ADDR (wlseg, IDX_MAX, d), d, wl_ids);
        DBUG_ASSERT ((index != NULL), "illegal supremum found!");
        args = TBmakeExprs (index, args);
    }

    for (d = WLSEGVAR_DIMS (wlseg) - 1; d >= 0; d--) {
        index
          = WLBnodeOrIntMakeIndex (NODE_TYPE (wlseg),
                                   WLSEGX_IDX_GET_ADDR (wlseg, IDX_MIN, d), d, wl_ids);
        DBUG_ASSERT ((index != NULL), "illegal infimum found!");
        args = TBmakeExprs (index, args);
    }

    args = TBmakeExprs (TBmakeNum (WLSEGVAR_DIMS (wlseg)), args);

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *CompileSchedulingWithTasksel( int seg_id, ids *wl_ids,
 *                                       sched_t *sched,tasksel_t *tasksel,
 *                                       node *arg_node, char *suffix)
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
CompileSchedulingWithTasksel (int seg_id, node *wl_ids, sched_t *sched,
                              tasksel_t *tasksel, node *arg_node, char *suffix)
{
    node *icm, *general_args;
    char *name;

    DBUG_ENTER ("CompileSchedulingWithTasksel");

    if (sched != NULL) {
        name = (char *)ILIBmalloc (sizeof (char)
                                   * (strlen (sched->discipline) + strlen (suffix) + 15));
        sprintf (name, "MT_SCHEDULER_%s_%s", sched->discipline, suffix);
    } else {
        name = (char *)ILIBmalloc (sizeof (char) * (strlen (suffix) + 15));
        sprintf (name, "MT_SCHEDULER_%s", suffix);
    }

    switch (NODE_TYPE (arg_node)) {
    case N_wlseg:
        general_args
          = CompileConstSegSchedulingWithTaskselArgs (wl_ids, arg_node, sched, tasksel);
        break;

    case N_wlsegvar:
        general_args
          = CompileVarSegSchedulingWithTaskselArgs (wl_ids, arg_node, sched, tasksel);
        break;

    default:
        general_args = NULL;
        DBUG_ASSERT ((0), "wrong node type found");
    }

    icm = TBmakeIcm (name, CompileSchedulingWithTaskselArgs (seg_id, sched, tasksel,
                                                             general_args));

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * function:
 *   node *SCHCompileSchedulingWithTaskselBegin( int seg_id, ids *wl_ids,
 *                                      sched_t *sched, tasksel_t *tasksel,
 *                                      node *arg_node)
 *
 *   node *SCHCompileSchedulingWithTaskselEnd( int seg_id, ids *wl_ids,
 *                                   sched_t *sched,  tasksel_t *tasksel,
 *                                   node *arg_node)
 *
 *   node *SCHCompileSchedulingWithTaskselInit( int seg_id, ids *wl_ids,
 *                                    sched_t *sched,  tasksel_t *tasksel,
 *                                    node *arg_node)
 *
 * description:
 *
 *   These functions initiate the compilation of abstract scheduling and
 *   taskselector specifications into ICMs, where each scheduling is
 *   associated with three ICMs.
 *   Whereas the former two enclose the with-loop code to be scheduled,
 *   the latter one initializes potentially needed internal data structures
 *   of the scheduling facility.
 *
 *   The segment ID specifies the position of the scheduler within an SPMD
 *   section. It is needed to identify the appropriate set of scheduler-internal
 *   data structures.
 *
 ******************************************************************************/

node *
SCHcompileSchedulingWithTaskselBegin (int seg_id, node *wl_ids, sched_t *sched,
                                      tasksel_t *tasksel, node *arg_node)
{
    node *ret_node;

    DBUG_ENTER ("SCHcompileSchedulingWithTaskselBegin");

    ret_node
      = CompileSchedulingWithTasksel (seg_id, wl_ids, sched, tasksel, arg_node, "BEGIN");

    DBUG_RETURN (ret_node);
}

node *
SCHcompileSchedulingWithTaskselEnd (int seg_id, node *wl_ids, sched_t *sched,
                                    tasksel_t *tasksel, node *arg_node)
{
    node *ret_node;

    DBUG_ENTER ("SCHcompileSchedulingWithTaskselEnd");

    ret_node
      = CompileSchedulingWithTasksel (seg_id, wl_ids, sched, tasksel, arg_node, "END");

    DBUG_RETURN (ret_node);
}

node *
SCHcompileSchedulingWithTaskselInit (int seg_id, node *wl_ids, sched_t *sched,
                                     tasksel_t *tasksel, node *arg_node)
{
    node *ret_node;

    DBUG_ENTER ("SCHcompileSchedulingWithTaskselInit");

    ret_node
      = CompileSchedulingWithTasksel (seg_id, wl_ids, sched, tasksel, arg_node, "INIT");

    DBUG_RETURN (ret_node);
}
