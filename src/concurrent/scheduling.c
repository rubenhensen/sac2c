/*
 *
 * $Log$
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
 *
 */

/*****************************************************************************
 *
 * file:   scheduling.c
 *
 * prefix: SCH
 *
 * description:
 *
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
#include "internal_lib.h"
#include "free.h"
#include "traverse.h"
#include "Error.h"
#include "precompile.h" /* for PRECRenameLocalIdentifier() */
#include "dbug.h"

/******************************************************************************
 *
 * data type: sched_class_t
 *            sched_arg_type_t
 *            sched_arg_t
 *            sched_t
 *
 * description:
 *
 *   These data types are used for the representation of schedulings. Each
 *   scheduling consists of a name called 'discipline', a class, a line
 *   number, and an argument specification.
 *
 *   Currently, three different scheduler classes are supported. Those for
 *   entire synchronisation blocks (SC_syncblock), for segments with constant
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

typedef enum { SC_const_seg, SC_var_seg, SC_syncblock } sched_class_t;

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
 *
 *   This global variable defines a table of scheduling specifications. Each
 *   scheduling is described by six entries: the scheduling name, the scheduling
 *   class, an adjustment flag, maximum scheduling dimension, the number
 *   of its arguments, and a type specification of the arguments.
 *
 *   Schedulings may be defined for synchronisation blocks/with-loops as well as for
 *   variable or constant segments. Note that schedulings for variable segments
 *   may also be applied to constant segments but not vice versa.
 *
 *   The adjustment flag specifies whether a particular scheduling produces
 *   legal subsegments with respect to unrolled loops.
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
  /* Name            Class          Adjust Dim  Args    */
  {"Block", SC_const_seg, 1, 0, 0, ""},   {"BlockVar", SC_var_seg, 0, 0, 0, ""},
  {"AllByOne", SC_var_seg, 0, 0, 1, "i"}, {"BlockBySome", SC_const_seg, 0, 0, 2, "i,i"},
  {"Static", SC_syncblock, 0, 0, 0, ""},  {"", SC_const_seg, 0, 0, 0, ""}};

/******************************************************************************
 *
 * function:
 *   sched_t *CheckSchedulingArgs(sched_t *sched, char *spec, node *exprs, int line)
 *
 * description:
 *
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
                    ABORT (line,
                           ("Argument %d of scheduling discipline '%s` must be a number",
                            i, sched->discipline));
                }
                sched->args[i].arg_type = AT_num;
                sched->args[i].arg.num = NUM_VAL (expr);
                break;

            case 'i':
                if (NODE_TYPE (expr) != N_id) {
                    ABORT (
                      line,
                      ("Argument %d of scheduling discipline '%s` must be an identifier",
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
                    ABORT (
                      line,
                      ("Argument %d of scheduling discipline '%s` must be an identifier"
                       "or a number",
                       i, sched->discipline));
                }

                break;

            default:
                DBUG_ASSERT ((arg_spec != NULL), "Illegal scheduling specification");
            }

            break;

        case 'v':
            DBUG_ASSERT (0, "Vector arguments for scheduling disciplines not yet "
                            "implemented");
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
 *   sched_t *SCHMakeSchedulingByPragma(node *ap_node, int line)
 *
 * description:
 *
 *   This function constructs a scheduling specification from a 'ScheduleWL'
 *   or 'ScheduleSeg' entry of the wlcomp pragma. The pragma information is
 *   compared with the scheduling specification table. The line number of
 *   the pragma in the original source code is required to produce sufficient
 *   error messages.
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
        sched->line = line;

        sched = CheckSchedulingArgs (sched, scheduler_table[i].arg_spec,
                                     AP_ARGS (ap_node), line);
    }

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   void SCHCheckSuitabilityConstSeg(sched_t *sched)
 *   void SCHCheckSuitabilityVarSeg(sched_t *sched)
 *   void SCHCheckSuitabilitySyncblock(sched_t *sched)
 *
 * description:
 *
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
SCHCheckSuitabilitySyncblock (sched_t *sched)
{
    DBUG_ENTER ("SCHCheckSuitabilitySyncblock");

    if (sched->class != SC_syncblock) {
        ERROR (sched->line, ("Scheduling discipline '%s` is not suitable for "
                             "synchronisation blocks",
                             sched->discipline));
    }

    DBUG_VOID_RETURN;
}

#if 0
These functions are probably no longer required.

/******************************************************************************
 *
 * function:
 *   int SCHSchedulingDoesAdjustment(sched_t *sched)
 *
 * description:
 * 
 *   This function returns a flag that specifies whether the given scheduling
 *   adjusts subsegements with respect to unrolled loops or not.
 *   This information is directly retrieved from the scheduler table.
 *
 ******************************************************************************/

int SCHSchedulingDoesAdjustment(sched_t *sched)
{
  int i=0;
  
  DBUG_ENTER("SCHSchedulingDoesAdjustment");
  
  while (0!=strcmp(sched->discipline, scheduler_table[i].discipline)) {
    i++;
  }
  
  DBUG_RETURN(scheduler_table[i].adjust_flag);
}



/******************************************************************************
 *
 * function:
 *   int SCHMaxSchedulingDim(sched_t *sched)
 *
 * description:
 * 
 *   This function returns the maximum scheduling dimension of the given
 *   scheduling. This information is directly retrieved from the
 *   scheduler table.
 *
 ******************************************************************************/

int SCHMaxSchedulingDim(sched_t *sched)
{
  int i=0;
  
  DBUG_ENTER("SCHSchedulingDoesAdjustment");
  
  while (0!=strcmp(sched->discipline, scheduler_table[i].discipline)) {
    i++;
  }
  
  DBUG_RETURN(scheduler_table[i].max_sched_dim);
}

#endif /* 0 */

/******************************************************************************
 *
 * function:
 *   int SCHAdjustmentRequired(int dim, node *wlseg)
 *
 * description:
 *
 *   This function decides whether or not bounds generated by a particular
 *   scheduling for a particular segment must be adjusted to be compatible
 *   to a given unrolling.
 *
 ******************************************************************************/

int
SCHAdjustmentRequired (int dim, node *wlseg)
{
    int i = 0;
    int adjust;

    DBUG_ENTER ("SCHAdjustmentRequired");

    while (
      0 != strcmp (WLSEG_SCHEDULING (wlseg)->discipline, scheduler_table[i].discipline)) {
        i++;
    }

    if ((dim <= scheduler_table[i].max_sched_dim)
        && (!scheduler_table[i].adjust_flag || (dim > WLSEG_MAXHOMDIM (wlseg)))
        && (MAX (WLSEG_SV (wlseg)[dim], WLSEG_UBV (wlseg)[dim]) > 1)) {
        adjust = 1;
    } else {
        adjust = 0;
    }

    DBUG_RETURN (adjust);
}

/******************************************************************************
 *
 * function:
 *   sched_t *SCHRemoveScheduling(sched_t *sched)
 *
 * description:
 *
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
            case AT_num_for_id_vec:
                FREE (sched->args[i].arg.num_vec);
                break;
            case AT_id_vec:
                FREE (sched->args[i].arg.id_vec);
                break;
            default:
                break;
            }
        }

        FREE (sched->args);
    }

    FREE (sched);

    DBUG_RETURN ((sched_t *)NULL);
}

/******************************************************************************
 *
 * function:
 *   sched_t *SCHCopyScheduling(sched_t *sched)
 *
 * description:
 *
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
            case AT_num_vec:
            case AT_num_for_id_vec:
                new_sched->args[i].arg.num = sched->args[i].arg.num;
                break;
            case AT_id_vec:
                new_sched->args[i].arg.id = sched->args[i].arg.id;
                break;
            default:
                break;
            }
        }

    } else {
        new_sched->args = NULL;
    }

    DBUG_RETURN ((sched_t *)NULL);
}

/******************************************************************************
 *
 * function:
 *   void SCHPrintScheduling(FILE *handle, sched_t *sched)
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

void
SCHPrintScheduling (FILE *outfile, sched_t *sched)
{
    int i;

    DBUG_ENTER ("SCHPrintScheduling");

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

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   sched_t *SCHMakeCompatibleSyncblockScheduling(sched_t *old_sched, sched_t *new_sched
 *
 * description:
 *
 *   This function checks whether two schedulings tied to with-loops fit together
 *   wihtin a single synchronisation block.
 *
 *
 ******************************************************************************/

sched_t *
SCHMakeCompatibleSyncblockScheduling (sched_t *old_sched, sched_t *new_sched)
{
    DBUG_ENTER ("SCHMakeCompatibleSyncblockScheduling");

    if (0 != strcmp (old_sched->discipline, new_sched->discipline)) {
        ERROR (new_sched->line,
               ("Syncblock scheduling discipline '%s` incompatible "
                "with '%s` defined in line %d",
                new_sched->discipline, old_sched->discipline, old_sched->line));
    }

    new_sched = SCHRemoveScheduling (new_sched);

    DBUG_RETURN (old_sched);
}

/******************************************************************************
 *
 * function:
 *   sched_t *SCHMakeScheduling(va_alist)
 *
 * description:
 *
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
            DBUG_ASSERT (0, "Vector arguments for scheduling disciplines not yet "
                            "implemented");
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
 *   sched_t *SCHPrecompileScheduling(sched_t *sched)
 *
 * description:
 *
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
            sched->args[i].arg.id = PRECRenameLocalIdentifier (sched->args[i].arg.id);
        }
    }

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   node *CompileSchedulingArgs(sched_t *sched, node *args)
 *
 * description:
 *
 *   This function converts the arguments of an abstract scheduling specification
 *   into ICM arguments. Numbers in the case of type 'x' scheduling arguments
 *   are transformed into string representations.
 *
 ******************************************************************************/

static node *
CompileSchedulingArgs (sched_t *sched, node *args)
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
                DBUG_ASSERT (0, "Vector arguments for scheduling disciplines not yet "
                                "implemented");
            }

            args = MakeExprs (new_arg, args);
        }
    }

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *CompileConstSegSchedulingArgs(node *wlseg)
 *
 * description:
 *
 *   In addition to their individual arguments, scheduling ICMs have addtional
 *   arguments depending on their position. Schedulings for constant segments
 *   are equipped with detailed segment information. These are the segment's
 *   dimensionality, its bounds, its outermost blocking vector, and its
 *   unrolling blocking vector.
 *
 ******************************************************************************/

static node *
CompileConstSegSchedulingArgs (node *wlseg, sched_t *sched)
{
    node *args;
    int i;

    DBUG_ENTER ("CompileConstSegSchedulingArgs");

    args = NULL;

    if (sched != NULL) {

        for (i = WLSEG_DIMS (wlseg) - 1; i >= 0; i--) {
            if (SCHAdjustmentRequired (i, wlseg)) {
                args = MakeExprs (MakeNum (1), args);
            } else {
                args
                  = MakeExprs (MakeNum (MAX (WLSEG_UBV (wlseg)[i], WLSEG_SV (wlseg)[i])),
                               args);
            }
        }

        for (i = WLSEG_DIMS (wlseg) - 1; i >= 0; i--) {
            args = MakeExprs (MakeNum (WLSEG_BV (wlseg, 0)[i]), args);
        }
    }

    for (i = WLSEG_DIMS (wlseg) - 1; i >= 0; i--) {
        args = MakeExprs (MakeNum (WLSEG_IDX_MAX (wlseg)[i]), args);
    }

    for (i = WLSEG_DIMS (wlseg) - 1; i >= 0; i--) {
        args = MakeExprs (MakeNum (WLSEG_IDX_MIN (wlseg)[i]), args);
    }

    args = MakeExprs (MakeNum (WLSEG_DIMS (wlseg)), args);

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *CompileVarSegSchedulingArgs(node *wlseg)
 *
 * description:
 *
 *   In addition to their individual arguments, scheduling ICMs have addtional
 *   arguments depending on their position. Currently, schedulings for variable
 *   segments do not have general arguments.
 *
 ******************************************************************************/

static node *
CompileVarSegSchedulingArgs (node *wlseg, sched_t *sched)
{
    node *args;
    int i;

    DBUG_ENTER ("CompileConstSegSchedulingArgs");

    args = NULL;

    if (sched != NULL) {

        for (i = WLSEGVAR_DIMS (wlseg) - 1; i >= 0; i--) {
            if (SCHAdjustmentRequired (i, wlseg)) {
                args = MakeExprs (MakeNum (1), args);
            } else {
                args = MakeExprs (MakeNum (MAX (WLSEGVAR_UBV (wlseg)[i],
                                                WLSEGVAR_SV (wlseg)[i])),
                                  args);
            }
        }

        for (i = WLSEGVAR_DIMS (wlseg) - 1; i >= 0; i--) {
            args = MakeExprs (MakeNum (WLSEGVAR_BV (wlseg, 0)[i]), args);
        }
    }

    for (i = WLSEGVAR_DIMS (wlseg) - 1; i >= 0; i--) {
        args = MakeExprs (MakeNum (WLSEGVAR_IDX_MAX (wlseg)[i]), args);
    }

    for (i = WLSEGVAR_DIMS (wlseg) - 1; i >= 0; i--) {
        args = MakeExprs (MakeNum (WLSEGVAR_IDX_MIN (wlseg)[i]), args);
    }

    args = MakeExprs (MakeNum (WLSEGVAR_DIMS (wlseg)), args);

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *CompileSyncblockSchedulingArgs(node *wlseg)
 *
 * description:
 *
 *   In addition to their individual arguments, scheduling ICMs have addtional
 *   arguments depending on their position. Currently, schedulings for synchronisation
 *   blocks do not have general arguments.
 *
 ******************************************************************************/

static node *
CompileSyncblockSchedulingArgs (node *wlseg, sched_t *sched)
{
    node *args;

    DBUG_ENTER ("CompileSyncblockSchedulingArgs");

    args = NULL;

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *CompileScheduling(sched_t *sched, node *arg_node, char *suffix)
 *
 * description:
 *
 *   This function compiles abstract scheduling specifications to ICMs
 *    "SAC_MT_SCHEDULER_<discipline>_BEGIN"
 *    "SAC_MT_SCHEDULER_<discipline>_END"
 *   depending on the parameter 'suffix'.
 *
 ******************************************************************************/

static node *
CompileScheduling (sched_t *sched, node *arg_node, char *suffix)
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
        general_args = CompileConstSegSchedulingArgs (arg_node, sched);
        break;
    case N_WLsegVar:
        general_args = CompileVarSegSchedulingArgs (arg_node, sched);
        break;
    case N_sync:
        general_args = CompileSyncblockSchedulingArgs (arg_node, sched);
        break;
    default:
        DBUG_ASSERT ((0), "wrong node type found");
    }

    icm = MakeIcm (name, CompileSchedulingArgs (sched, general_args), NULL);

    DBUG_RETURN (icm);
}

/******************************************************************************
 *
 * function:
 *   node *SCHCompileSchedulingBegin(sched_t *sched, node *arg_node)
 *   node *SCHCompileSchedulingEnd(sched_t *sched, node *arg_node)
 *
 * description:
 *
 *   These two functions initiate the compilation of abstract scheduling
 *   specifications into ICMs.
 *
 ******************************************************************************/

node *
SCHCompileSchedulingBegin (sched_t *sched, node *arg_node)
{
    DBUG_ENTER ("SCHCompileSchedulingBegin");

    DBUG_RETURN (CompileScheduling (sched, arg_node, "BEGIN"));
}

node *
SCHCompileSchedulingEnd (sched_t *sched, node *arg_node)
{
    DBUG_ENTER ("SCHCompileSchedulingEnd");

    DBUG_RETURN (CompileScheduling (sched, arg_node, "END"));
}
