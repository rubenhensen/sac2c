/*
 * $Id$
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
#include "str.h"
#include "memory.h"
#include "free.h"
#include "traverse.h"
#include "ctinfo.h"
#include "wl_bounds.h"
#include "dbug.h"
#include "renameidentifiers.h"
#include "check_mem.h"

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

    arg_spec = STRtok (spec, ",");

    for (i = 0; i < sched->num_args; i++) {
        DBUG_ASSERT ((arg_spec != NULL), "Illegal scheduling specification");

        if (exprs == NULL) {
            CTIabortLine (line,
                          "Scheduling discipline '%s` expects %d arguments "
                          "(too few specified)",
                          sched->discipline, sched->num_args);
        }

        expr = EXPRS_EXPR (exprs);

        switch (arg_spec[1]) {
        case '\0':
            switch (arg_spec[0]) {
            case 'n':
                if (NODE_TYPE (expr) != N_num) {
                    CTIabortLine (line,
                                  "Argument %d of scheduling discipline '%s` must be"
                                  " a number",
                                  i, sched->discipline);
                }
                sched->args[i].arg_type = AT_num;
                sched->args[i].arg.num = NUM_VAL (expr);
                break;

            case 'i':
                if (NODE_TYPE (expr) != N_spid) {
                    CTIabortLine (line,
                                  "Argument %d of scheduling discipline '%s` must be"
                                  " an identifier",
                                  i, sched->discipline);
                }
                sched->args[i].arg_type = AT_id;
                sched->args[i].arg.id = STRcpy (SPID_NAME (expr));
                break;

            case 'x':
                switch (NODE_TYPE (expr)) {
                case N_num:
                    sched->args[i].arg_type = AT_num_for_id;
                    sched->args[i].arg.num = NUM_VAL (expr);
                    break;
                case N_spid:
                    sched->args[i].arg_type = AT_id;
                    sched->args[i].arg.id = STRcpy (SPID_NAME (expr));
                    break;
                default:
                    CTIabortLine (line,
                                  "Argument %d of scheduling discipline '%s` must be"
                                  " an identifier or a number",
                                  i, sched->discipline);
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

        arg_spec = MEMfree (arg_spec);
        arg_spec = STRtok (NULL, ",");
        exprs = EXPRS_NEXT (exprs);
    }

    if (exprs != NULL) {
        CTIabortLine (line,
                      "Scheduling discipline '%s` expects %d arguments "
                      "(too many specified)",
                      sched->discipline, sched->num_args);
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
           && (!STReq (scheduler_table[disc_no].discipline, discipline))) {
        disc_no++;
    }

    DBUG_ASSERT ((scheduler_table[disc_no].discipline[0] != '\0'),
                 "Infered scheduling discipline not implemented");

    sched = (sched_t *)MEMmalloc (sizeof (sched_t));

    sched->discipline = scheduler_table[disc_no].discipline;
    sched->class = scheduler_table[disc_no].class;
    sched->line = -1;

    sched->num_args = scheduler_table[disc_no].num_args;

    if (sched->num_args == 0) {
        sched->args = NULL;
    } else {
        sched->args = (sched_arg_t *)MEMmalloc (sched->num_args * sizeof (sched_arg_t));
    }

    arg_spec = STRtok (scheduler_table[disc_no].arg_spec, ",");

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

        arg_spec = MEMfree (arg_spec);
        arg_spec = STRtok (NULL, ",");
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
           && (!STReq (scheduler_table[i].discipline, SPAP_NAME (ap_node)))) {
        i++;
    }

    if (scheduler_table[i].discipline[0] != '\0') {
        sched = (sched_t *)MEMmalloc (sizeof (sched_t));
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
              = (sched_arg_t *)MEMmalloc (sched->num_args * sizeof (sched_arg_t));
        }
        sched->line = line;

        sched = CheckSchedulingArgs (sched, scheduler_table[i].arg_spec,
                                     SPAP_ARGS (ap_node), line);
    } else {
        CTIabortLine (line, "Illegal argument in wlcomp-pragma found; "
                            "Scheduling(): Unknown scheduler");
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
                MEMfree (sched->args[i].arg.num_vec);
                break;

            case AT_id_vec:
                MEMfree (sched->args[i].arg.id_vec);
                break;

            default:
                break;
            }
        }

        MEMfree (sched->args);
    }

    sched = MEMfree (sched);

    DBUG_RETURN (sched);
}

/******************************************************************************
 *
 * function:
 *   sched_t *SCHtouchScheduling( sched_t *sched, info *arg_info)
 *
 * description:
 *   This function may be used to touch the resources bound to a data object
 *   of the abstract data type for the representation of schedulings.
 *
 ******************************************************************************/

void
SCHtouchScheduling (sched_t *sched, info *arg_info)
{
    int i;

    DBUG_ENTER ("SCHtouchScheduling");

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
                CHKMtouch (sched->args[i].arg.num_vec, arg_info);
                break;

            case AT_id_vec:
                CHKMtouch (sched->args[i].arg.id_vec, arg_info);
                break;

            default:
                break;
            }
        }

        CHKMtouch (sched->args, arg_info);
    }

    CHKMtouch (sched, arg_info);

    DBUG_VOID_RETURN;
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

    new_sched = (sched_t *)MEMmalloc (sizeof (sched_t));

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
          = (sched_arg_t *)MEMmalloc (sched->num_args * sizeof (sched_arg_t));

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
                sched->args[i].arg.id = MEMfree (sched->args[i].arg.id);
                sched->args[i].arg.id = STRcpy (new_name);
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
        CTIerrorLine (sched->line,
                      "Scheduling discipline '%s` is not suitable for "
                      "constant segments",
                      sched->discipline);
    }

    DBUG_VOID_RETURN;
}

void
SCHcheckSuitabilityVarSeg (sched_t *sched)
{
    DBUG_ENTER ("SCHcheckSuitabilityVarSeg");

    if (sched->class != SC_var_seg) {
        CTIerrorLine (sched->line,
                      "Scheduling discipline '%s` is not suitable for "
                      "variable segments",
                      sched->discipline);
    }

    DBUG_VOID_RETURN;
}

void
SCHcheckSuitabilityWithloop (sched_t *sched)
{
    DBUG_ENTER ("SCHcheckSuitabilityWithloop");

    if (sched->class != SC_withloop) {
        CTIerrorLine (sched->line,
                      "Scheduling discipline '%s` is not suitable for "
                      "with-loops",
                      sched->discipline);
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

    while (!STReq (((sched_t *)WLSEGX_SCHEDULING (wlseg))->discipline,
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
                new_arg = TCmakeIdCopyString (STRitoa (sched->args[i].arg.num));
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
        name = (char *)MEMmalloc (sizeof (char)
                                  * (strlen (sched->discipline) + strlen (suffix) + 15));
        sprintf (name, "MT_SCHEDULER_%s_%s", sched->discipline, suffix);
    } else {
        name = (char *)MEMmalloc (sizeof (char) * (strlen (suffix) + 15));
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
            CTIabortLine (line,
                          "Taskselector discipline '%s` expects %d arguments "
                          "(too few specified)",
                          tasksel->discipline, tasksel->num_args);
        }

        expr = EXPRS_EXPR (exprs);

        if (NODE_TYPE (expr) != N_num) {
            CTIabortLine (line,
                          "Argument %d of taskselector discipline '%s` must be"
                          " a number",
                          i, tasksel->discipline);
        }

        tasksel->arg[i] = NUM_VAL (expr);

        exprs = EXPRS_NEXT (exprs);
    }

    if (exprs != NULL) {
        CTIabortLine (line,
                      "Taskselector discipline '%s` expects %d arguments "
                      "(too many specified)",
                      tasksel->discipline, tasksel->num_args);
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
           && (!STReq (taskselector_table[i].discipline, SPAP_NAME (ap_node)))) {
        i++;
    }

    if (taskselector_table[i].discipline[0] != '\0') {
        tasksel = (tasksel_t *)MEMmalloc (sizeof (tasksel_t));
        tasksel->discipline = taskselector_table[i].discipline;

        tasksel->num_args = taskselector_table[i].num_args;
        tasksel->dims = taskselector_table[i].dims;
        if (tasksel->num_args == 0) {
            tasksel->arg = NULL;
        } else {
            tasksel->arg = (int *)MEMmalloc (tasksel->num_args * sizeof (int));
        }
        tasksel->line = line;

        tasksel = CheckTaskselArgs (tasksel, SPAP_ARGS (ap_node), line);
    } else {
        CTIabortLine (line, "Illegal argument in wlcomp-pragma found; "
                            "Tasksel(): Unknown Taskselector");
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
        MEMfree (tasksel->arg);
    }

    tasksel = MEMfree (tasksel);

    DBUG_RETURN (tasksel);
}

/******************************************************************************
 *
 * function:
 *   void SCHtouchTasksel( tasksel_t *tasksel, info *arg_info)
 *
 * description:
 *   This function may be used to touch the resources bound to a data object
 *   of the abstract data type for the representation of a taskselctor.
 *
 ******************************************************************************/

void
SCHtouchTasksel (tasksel_t *tasksel, info *arg_info)
{
    DBUG_ENTER ("SCHtouchTasksel");

    /*
     * The discipline string must not be freed since it is only a pointer
     * to the respective entry of the taskselector table.
     */
    if (tasksel->num_args > 0) {
        CHKMtouch (tasksel->arg, arg_info);
    }

    CHKMtouch (tasksel, arg_info);

    DBUG_VOID_RETURN;
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

    new_tasksel = (tasksel_t *)MEMmalloc (sizeof (tasksel_t));

    new_tasksel->discipline = tasksel->discipline;
    /*
     * The discipline string must not be copied since it is only a pointer
     * to the respective entry of the scheduler table.
     */

    new_tasksel->line = tasksel->line;
    new_tasksel->num_args = tasksel->num_args;
    new_tasksel->dims = tasksel->dims;

    if (tasksel->num_args > 0) {
        new_tasksel->arg = (int *)MEMmalloc (tasksel->num_args * sizeof (int));

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
                new_arg = TCmakeIdCopyString (STRitoa (sched->args[i].arg.num));
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
        name = (char *)MEMmalloc (sizeof (char)
                                  * (strlen (sched->discipline) + strlen (suffix) + 15));
        sprintf (name, "MT_SCHEDULER_%s_%s", sched->discipline, suffix);
    } else {
        name = (char *)MEMmalloc (sizeof (char) * (strlen (suffix) + 15));
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
