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
 *   kinds of schedulings requiring different parameter sets.
 *
 *****************************************************************************/

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

typedef enum {
    AT_num,
    AT_id,
    AT_num_for_id,
    AT_num_vec,
    AT_id_vec,
    AT_num_for_id_vec
} sched_arg_type_t;

typedef enum { SC_const_seg, SC_var_seg, SC_syncblock } sched_class_t;

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

typedef struct {
    char *discipline;
    int num_args;
    char *arg_spec;
    sched_class_t class;
} scheduler_table_t;

static scheduler_table_t scheduler_table[] = {
  /* Name            Args           Class */
  {"Block", 0, "", SC_const_seg},   {"BlockVar", 0, "", SC_var_seg},
  {"AllByOne", 1, "i", SC_var_seg}, {"BlockBySome", 2, "i,i", SC_const_seg},
  {"Static", 0, "", SC_syncblock},  {"", 0, "", SC_const_seg}};

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

static sched_t *MakeScheduling (va_alist) va_dcl
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

static sched_t *
MakeDefaultSchedulingConstSegment ()
{
    sched_t *sched;

    DBUG_ENTER ("MakeDefaultSchedulingConstSegment");

    sched = MakeScheduling ("Block");

    DBUG_RETURN (sched);
}

static sched_t *
MakeDefaultSchedulingVarSegment ()
{
    sched_t *sched;

    DBUG_ENTER ("MakeDefaultSchedulingVarSegment");

    sched = MakeScheduling ("BlockVar");

    DBUG_RETURN (sched);
}

static sched_t *
MakeDefaultSchedulingSyncblock ()
{
    sched_t *sched;

    DBUG_ENTER ("MakeDefaultSchedulingSyncblock");

    sched = MakeScheduling ("Static");

    DBUG_RETURN (sched);
}

static sched_t *
InferSchedulingConstSegment (node *wlseg, node *arg_info)
{
    sched_t *sched;

    DBUG_ENTER ("InferSchedulingConstSegment");

    sched = MakeDefaultSchedulingConstSegment ();

    DBUG_RETURN (sched);
}

static sched_t *
InferSchedulingVarSegment (node *wlsegvar, node *arg_info)
{
    sched_t *sched;

    DBUG_ENTER ("InferSchedulingVarSegment");

    sched = MakeDefaultSchedulingVarSegment ();

    DBUG_RETURN (sched);
}

static sched_t *
InferSchedulingSyncblock (node *sync, node *arg_info)
{
    sched_t *sched;

    DBUG_ENTER ("InferSchedulingSyncblock");

    sched = MakeDefaultSchedulingSyncblock ();

    DBUG_RETURN (sched);
}

node *
GenerateSchedulings (node *syntax_tree)
{
    DBUG_ENTER ("GenerateSchedulings");

    DBUG_RETURN (syntax_tree);
}

node *
SCHmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SCHmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SCHfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SCHfundef");

    if (FUNDEF_STATUS (arg_node) == ST_spmdfun) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SCHwlseg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SCHwlseg");

    if (WLSEG_SCHEDULING (arg_node) == NULL) {
        WLSEG_SCHEDULING (arg_node) = InferSchedulingConstSegment (arg_node, arg_info);
    } else {
        if ((WLSEG_SCHEDULING (arg_node)->class != SC_const_seg)
            && (WLSEG_SCHEDULING (arg_node)->class != SC_var_seg)) {
            ERROR (WLSEG_SCHEDULING (arg_node)->line,
                   ("Scheduling discipline '%s` not suitable for constant segments",
                    WLSEG_SCHEDULING (arg_node)->discipline));
        }
    }

    DBUG_RETURN (arg_node);
}

node *
SCHwlsegvar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SCHwlsegvar");

    if (WLSEGVAR_SCHEDULING (arg_node) == NULL) {
        WLSEGVAR_SCHEDULING (arg_node) = InferSchedulingVarSegment (arg_node, arg_info);
    } else {
        if (WLSEGVAR_SCHEDULING (arg_node)->class != SC_var_seg) {
            ERROR (WLSEGVAR_SCHEDULING (arg_node)->line,
                   ("Scheduling deiscipline '%s` not suitable for variable segments",
                    WLSEGVAR_SCHEDULING (arg_node)->discipline));
        }
    }

    DBUG_RETURN (arg_node);
}

node *
SCHsync (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SCHsync");

    if (SYNC_SCHEDULING (arg_node) == NULL) {
        SYNC_SCHEDULING (arg_node) = InferSchedulingSyncblock (arg_node, arg_info);
    } else {
        if (SYNC_SCHEDULING (arg_node)->class != SC_syncblock) {
            ERROR (SYNC_SCHEDULING (arg_node)->line,
                   ("Scheduling deiscipline '%s` not suitable for with-loops/syncblocks",
                    SYNC_SCHEDULING (arg_node)->discipline));
        }
    }

    DBUG_RETURN (arg_node);
}

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

static node *
CompileSchedulingArgs (sched_t *sched, node *args)
{
    node *new_arg;
    int i;

    DBUG_ENTER ("CompileSchedulingArgs");

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

    DBUG_RETURN (args);
}

static node *
CompileConstSegSchedulingArgs (node *wlseg)
{
    node *args;
    int i;

    DBUG_ENTER ("CompileConstSegSchedulingArgs");

    args = NULL;

    for (i = WLSEG_DIMS (wlseg) - 1; i >= 0; i--) {
        args = MakeExprs (MakeNum (WLSEG_UBV (wlseg)[i]), args);
    }

    for (i = WLSEG_DIMS (wlseg) - 1; i >= 0; i--) {
        args = MakeExprs (MakeNum (WLSEG_BV (wlseg, 0)[i]), args);
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

static node *
CompileVarSegSchedulingArgs (node *wlseg)
{
    node *args;

    DBUG_ENTER ("CompileVarSegSchedulingArgs");

    args = NULL;

    DBUG_RETURN (args);
}

static node *
CompileSyncblockSchedulingArgs (node *wlseg)
{
    node *args;

    DBUG_ENTER ("CompileSyncblockSchedulingArgs");

    args = NULL;

    DBUG_RETURN (args);
}

static node *
CompileScheduling (sched_t *sched, node *arg_node, char *suffix)
{
    node *icm, *general_args;
    char *name;

    DBUG_ENTER ("CompileScheduling");

    name = (char *)Malloc (sizeof (char) * (strlen (sched->discipline) + 24));
    sprintf (name, "SAC_MT_SCHEDULER_%s_%s", sched->discipline, suffix);

    switch (sched->class) {
    case SC_const_seg:
        general_args = CompileConstSegSchedulingArgs (arg_node);
        break;
    case SC_var_seg:
        general_args = CompileVarSegSchedulingArgs (arg_node);
        break;
    case SC_syncblock:
        general_args = CompileSyncblockSchedulingArgs (arg_node);
        break;
    }

    icm = MakeIcm (name, CompileSchedulingArgs (sched, general_args), NULL);

    DBUG_RETURN (icm);
}

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

/***********************************************************************************/

#if 0

scheduling_t *SCHMakeConstantSegmentScheduling(node *ap_node, int line)
{
  scheduling_t *sched;
  
  DBUG_ENTER("SCHMakeConstantSegmentScheduling");
  
  sched = MakeSchedulingByPragma(ap_node, constant_segment_scheduler_table, line);
  
  if (sched==NULL) {
    ABORT(line, ("Scheduling discipline '%s` unknown for constant segments", 
                 AP_NAME(ap_node)));
  }
  
  DBUG_RETURN(sched);
}


scheduling_t *SCHMakeVariableSegmentScheduling(node *ap_node, int line)
{
  scheduling_t *sched;
  
  DBUG_ENTER("SCHMakeVariableSegmentScheduling");
  
  sched = MakeSchedulingByPragma(ap_node, variable_segment_scheduler_table, line);
    
  if (sched==NULL) {
    ABORT(line, ("Scheduling discipline '%s` unknown for variable segments", 
                 AP_NAME(ap_node)));
  }
  
  DBUG_RETURN(sched);
}


scheduling_t *SCHMakeSyncblockScheduling(node *ap_node, int line)
{
  scheduling_t *sched;
  
  DBUG_ENTER("SCHMakeSyncblockScheduling");
  
  sched = MakeSchedulingByPragma(ap_node, syncblock_scheduler_table, line);
    
  if (sched==NULL) {
    ABORT(line, ("Scheduling discipline '%s` unknown for syncblocks", 
                 AP_NAME(ap_node)));
  }
  
  DBUG_RETURN(sched);
}


static
sched_t *MakeScheduling(char *name, int num_args, sched_class_t class)
{
  sched_t *sched;
  
  DBUG_ENTER("MakeScheduling");
  
  sched = (sched_t*)Malloc(sizeof(sched_t));
  
  sched->discipline = name;
  sched->class = class;
  sched->line = -1;

  sched->num_args = num_args;
  
  if (num_args==0) {
    sched->args=NULL;
  }
  else {
    sched->args=(sched_arg_t*)Malloc(num_args*sizeof(sched_arg_t));
  }
    
  DBUG_RETURN(sched);
}
  

static
sched_arg_t *SetSchedulingArgNum(sched_arg_t *args, int arg, int num)
{
  DBUG_ENTER("SetSchedulingArgNum");
  
  args[arg].arg_type=AT_num;
  args[arg].arg.num=num;
  
  DBUG_RETURN(args);
}
  

static
sched_arg_t *SetSchedulingArgNumForId(sched_arg_t *args, int arg, int num)
{
  DBUG_ENTER("SetSchedulingArgNumForId");
  
  args[arg].arg_type=AT_num_for_id;
  args[arg].arg.num=num;
  
  DBUG_RETURN(args);
}
  

static
sched_arg_t *SetSchedulingArgId(sched_arg_t *args, int arg, char *id)
{
  DBUG_ENTER("SetSchedulingArgId");
  
  args[arg].arg_type=AT_id;
  args[arg].arg.num=id;
  
  DBUG_RETURN(args);
}

#endif
