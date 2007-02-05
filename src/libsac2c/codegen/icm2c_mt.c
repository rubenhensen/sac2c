/*****************************************************************************
 *
 * $Id$
 *
 * file:   icm2c_mt.c
 *
 * prefix: ICMCompile
 *
 * description:
 *
 *   This file contains the definitions of C implemented ICMs.
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_mt.h"

#include "dbug.h"
#include "convert.h"
#include "globals.h"
#include "print.h"
#include "tree_basic.h"
#include "str.h"
#include "memory.h"

#ifndef BEtest
#include "scnprs.h"   /* for big magic access to syntax tree      */
#include "traverse.h" /* for traversal of fold operation function */
#include "compile.h"  /* for GetFoldCode()                        */
#include "free.h"
#endif /* BEtest */

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_FUN_DEC( char *name, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_FUN_DEC( name, vararg_cnt, [ TAG, type, param_NT ]* )
 *
 *   This ICM implements the protoype of an spmd function. The first parameter
 *   specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_FUN_DEC (char *funname, int vararg_cnt, char **vararg)
{
    int i;
    int cnt;

    DBUG_ENTER ("ICMCompileMT_SPMD_FUN_DEC");

#define MT_SPMD_FUN_DEC
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_FUN_DEC

    INDENT;
    fprintf (global.outfile,
             "SAC_MT_SPMD_FUN_REAL_RETTYPE()"
             " %s( SAC_MT_SPMD_FUN_REAL_PARAM_LIST())\n",
             funname);

    INDENT;
    fprintf (global.outfile, "{\n");

    global.indent++;

    INDENT;
    fprintf (global.outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_multi_threaded)\n");

    cnt = 0;
    for (i = 0; i < 3 * vararg_cnt; i += 3) {
        INDENT;
        fprintf (global.outfile, "SAC_MT_RECEIVE_PARAM_%s( %s, %d, %s, %s)\n", vararg[i],
                 funname, cnt++, vararg[i + 1], vararg[i + 2]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_FUN_AP( char *name, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_FUN_AP( name, vararg_cnt, [ TAG, param_NT ]* )
 *
 *   This ICM implements the application of an spmd function. The first
 *   parameter specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_FUN_AP (char *funname, int vararg_cnt, char **vararg)
{
    int i;
    int cnt;

    DBUG_ENTER ("ICMCompileMT_SPMD_FUN_AP");

#define MT_SPMD_FUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_FUN_AP

    cnt = 0;
    for (i = 0; i < 2 * vararg_cnt; i += 2) {
        INDENT;
        fprintf (global.outfile, "SAC_MT_SEND_PARAM_%s( %s, %d, %s)\n", vararg[i],
                 funname, cnt++, vararg[i + 1]);
    }

    INDENT;
    fprintf (global.outfile, "SAC_MT_SPMD_EXECUTE( %s);", funname);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_FUN_RET( int barrier_id,
 *                                   int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_FUN_RET( barrier_id, vararg_cnt, [ tag, foldfun, param_NT ]* )
 *
 *   This ICM implements the return statement of an spmd-function,
 *   i.e. it has to realize the return of several out parameters.
 *
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_FUN_RET (char *funname, int vararg_cnt, char **vararg)
{
    int i, cnt;

    DBUG_ENTER ("ICMCompileMT_SPMD_FUN_RET");

#define MT_SPMD_FUN_RET
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_FUN_RET

    INDENT;
    fprintf (global.outfile, "SAC_MT_SYNC_WORKER_BEGIN();\n");

    cnt = 0;
    for (i = 0; i < 3 * vararg_cnt; i += 3) {
        if (STReq (vararg[i], "out")) {
            INDENT;
            fprintf (global.outfile, "SAC_MT_SPMD_FOLD_RETURN_WORKER( %s, %d, %s, %s);\n",
                     funname, cnt, vararg[i + 1], vararg[i + 2]);
        }
    }

    INDENT;
    fprintf (global.outfile, "SAC_MT_SYNC_WORKER_END();\n");

    INDENT;
    fprintf (global.outfile, "SAC_MT_SYNC_MASTER_BEGIN();\n");

    cnt = 0;

    for (i = 0; i < 3 * vararg_cnt; i += 3) {
        if (STReq (vararg[i], "out")) {
            INDENT;
            fprintf (global.outfile, "SAC_MT_SPMD_FOLD_RETURN_MASTER( %s, %d, %s, %s);\n",
                     funname, cnt, vararg[i + 1], vararg[i + 2]);
        }
    }

    INDENT;
    fprintf (global.outfile, "SAC_MT_SYNC_MASTER_END();\n");

    INDENT;
    fprintf (global.outfile, "SAC_MT_SPMD_FUN_REAL_RETURN();\n");

    global.indent--;
    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_FRAME_ELEMENT( char *name, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_FRAME_ELEMENT( name, vararg_cnt, [ TAG, type, param_NT ]* )
 *
 *   This ICM implements the protoype of an spmd function. The first parameter
 *   specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_FRAME_ELEMENT (char *funname, int vararg_cnt, char **vararg)
{
    int i;
    int cnt;

    DBUG_ENTER ("ICMCompileMT_SPMD_FRAME_ELEMENT");

#define MT_SPMD_FRAME_ELEMENT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_FRAME_ELEMENT

    INDENT;
    fprintf (global.outfile, "SAC_MT_SPMD_FRAME_ELEMENT_BEGIN( %s)\n", funname);

    cnt = 0;
    for (i = 0; i < 3 * vararg_cnt; i += 3) {
        INDENT;
        fprintf (global.outfile, "SAC_MT_FRAME_ELEMENT_%s( %s, %d, %s, %s)\n", vararg[i],
                 funname, cnt++, vararg[i + 1], vararg[i + 2]);
    }

    INDENT;
    fprintf (global.outfile, "SAC_MT_SPMD_FRAME_ELEMENT_END( %s)\n", funname);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_BARRIER_ELEMENT( char *name, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_BARRIER_ELEMENT( name, vararg_cnt, [ TAG, type, param_NT ]* )
 *
 *   This ICM implements the protoype of an spmd function. The first parameter
 *   specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_BARRIER_ELEMENT (char *funname, int vararg_cnt, char **vararg)
{
    int i;
    int cnt;

    DBUG_ENTER ("ICMCompileMT_SPMD_BARRIER_ELEMENT");

#define MT_SPMD_BARRIER_ELEMENT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_BARRIER_ELEMENT

    INDENT;
    fprintf (global.outfile, "SAC_MT_SPMD_BARRIER_ELEMENT_BEGIN( %s)\n", funname);

    cnt = 0;
    for (i = 0; i < 3 * vararg_cnt; i += 3) {
        INDENT;
        fprintf (global.outfile, "SAC_MT_BARRIER_ELEMENT_%s( %s, %d, %s, %s)\n",
                 vararg[i], funname, cnt++, vararg[i + 1], vararg[i + 2]);
    }

    INDENT;
    fprintf (global.outfile, "SAC_MT_SPMD_BARRIER_ELEMENT_END( %s)\n", funname);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_CREATE_LOCAL_DESC( char *var_NT, int dim)
 *
 * description:
 *   compiles the corresponding ICM:
 *
 *   MT_CREATE_LOCAL_DESC( var_NT, dim )
 *
 ******************************************************************************/

void
ICMCompileMT_CREATE_LOCAL_DESC (char *var_NT, int dim)
{
    shape_class_t sc = ICUGetShapeClass (var_NT);
    int i;

    DBUG_ENTER ("ICMCompileMT_CREATE_LOCAL_DESC");

#define MT_CREATE_LOCAL_DESC
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_CREATE_LOCAL_DESC

    INDENT;
    fprintf (global.outfile, "/* init local descriptors */\n");

    switch (sc) {
    case C_scl:
        /* here is no break missing */
    case C_aks:
        /* noop */
        break;

    case C_akd:
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        INDENT;
        fprintf (global.outfile, "SAC_ND_A_DESC_SIZE( %s) = SAC_ND_A_SIZE( %s);\n",
                 var_NT, var_NT);
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (global.outfile,
                     "SAC_ND_A_DESC_SHAPE( %s, %d)"
                     " = SAC_ND_A_SHAPE( %s, %d);\n",
                     var_NT, i, var_NT, i);
        }
        break;

    case C_aud:
        /* 'dim' is unknown! */
        INDENT;
        fprintf (global.outfile, "SAC_ND_ALLOC__DESC( %s, SAC_ND_A_DIM( %s))\n", var_NT,
                 var_NT);
        INDENT;
        fprintf (global.outfile,
                 "SAC_ND_A_DESC_DIM( %s)"
                 " = DESC_DIM( CAT0( orig_, SAC_ND_A_DESC( %s)));\n",
                 var_NT, var_NT);
        INDENT;
        fprintf (global.outfile,
                 "SAC_ND_A_DESC_SIZE( %s)"
                 " = DESC_SIZE( CAT0( orig_, SAC_ND_A_DESC( %s)));\n",
                 var_NT, var_NT);
        FOR_LOOP_INC_VARDEC (fprintf (global.outfile, "SAC_i");
                             , fprintf (global.outfile, "0");
                             , fprintf (global.outfile, "SAC_ND_A_DIM( %s)", var_NT);
                             , INDENT;
                             fprintf (global.outfile,
                                      "SAC_ND_A_DESC_SHAPE( %s, SAC_i)"
                                      " = DESC_SHAPE( CAT0( orig_, SAC_ND_A_DESC( %s)),"
                                      " SAC_i);\n",
                                      var_NT, var_NT););
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    /*
     * set local reference counter to 1+SAC_SET_MAX_SYNC_FOLD in order to
     * guarantee that the object is neither reused nor deleted.
     * A value of two does no suffice here as the IN-variable might be used as
     * neutral element multiple times.
     */
    INDENT;
    fprintf (global.outfile, "SAC_ND_SET__RC( %s, 1 + SAC_SET_MAX_SYNC_FOLD)\n", var_NT);

    DBUG_VOID_RETURN;
}

#if 0

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNCBLOCK_BEGIN( int barrier_id,
 *                                      int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNCBLOCK_BEGIN( barrier_id, vararg_cnt, [ tag, param_NT, dim ]* )
 *   
 *   This ICM implements the begin of a synchronisation block.
 *   Essentially, the reference counters of the arguments are shadowed by
 *   thread-specific dummy reference counters. Dummies can be used safely
 *   since memory may exclusively been released in between synchronisation
 *   blocks. Nevertheless, they must be provided because arbitrary code in
 *   the with-loop body may want to increment/decrement reference counters.
 *   
 *   However, this ICM also complements the various ICMs that implement
 *   different kinds of barrier synchronisations.
 *
 ******************************************************************************/

void ICMCompileMT_SYNCBLOCK_BEGIN( int barrier_id,
                                   int vararg_cnt, char **vararg)
{
  int i;
  
  DBUG_ENTER( "ICMCompileMT_SYNCBLOCK_BEGIN");

#define MT_SYNCBLOCK_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNCBLOCK_BEGIN

  INDENT;
  fprintf( global.outfile, "{\n");
  global.indent++;

  for (i = 0; i < 3 * vararg_cnt; i += 3) {
    if (! strcmp( vararg[i], "in")) {
      INDENT;
      fprintf( global.outfile, "SAC_MT_DECL_LOCAL_DESC( %s, %s)\n",
                        vararg[i+1], vararg[i+2]);
    }
  }
  fprintf( global.outfile, "\n");

  INDENT;
  fprintf( global.outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_multi_threaded)\n");

  for (i = 0; i < 3 * vararg_cnt; i += 3) {
    if (! strcmp( vararg[i], "in")) {
      int dim;
      int cnt = sscanf( vararg[i+2], "%d", &dim);
      if (cnt != 1) {
        DBUG_ASSERT( (0),
                     "3rd vararg component of MT_SYNCBLOCK_BEGIN"
                     " is no integer!");
      }
      ICMCompileMT_CREATE_LOCAL_DESC( vararg[i+1], dim);
    }
  }

  INDENT;
  fprintf( global.outfile, "SAC_TR_MT_PRINT("
                    " (\"Starting execution of synchronisation block\"));\n");

  DBUG_VOID_RETURN;
}



/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNCBLOCK_CLEANUP( int barrier_id,
 *                                        int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNCBLOCK_CLEANUP( barrier_id, vararg_cnt, [ tag, param_NT, dim ]* )
 *   
 *   This ICM implements the begin of a synchronisation block.
 *   Essentially, the reference counters of the arguments are shadowed by
 *   thread-specific dummy reference counters. Dummies can be used safely
 *   since memory may exclusively been released in between synchronisation
 *   blocks. Nevertheless, they must be provided because arbitrary code in
 *   the with-loop body may want to increment/decrement reference counters.
 *   
 *   However, this ICM also complements the various ICMs that implement
 *   different kinds of barrier synchronisations.
 *
 ******************************************************************************/

void ICMCompileMT_SYNCBLOCK_CLEANUP( int barrier_id,
                                     int vararg_cnt, char **vararg)
{
  int i;
  
  DBUG_ENTER( "ICMCompileMT_SYNCBLOCK_CLEANUP");

#define MT_SYNCBLOCK_CLEANUP
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNCBLOCK_CLEANUP

  for (i = 0; i < 3 * vararg_cnt; i += 3) {
    if (! strcmp( vararg[i], "in")) {
      INDENT;
      fprintf( global.outfile, "SAC_MT_FREE_LOCAL_DESC( %s, %s)\n",
                        vararg[i+1], vararg[i+2]);
    }
  }

  DBUG_VOID_RETURN;
}



/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNCBLOCK_END( int barrier_id,
 *                                    int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNCBLOCK_END( barrier_id, vararg_cnt, [ tag, param_NT, dim ]* )
 *   
 *   This ICM implements the begin of a synchronisation block.
 *   Essentially, the reference counters of the arguments are shadowed by
 *   thread-specific dummy reference counters. Dummies can be used safely
 *   since memory may exclusively been released in between synchronisation
 *   blocks. Nevertheless, they must be provided because arbitrary code in
 *   the with-loop body may want to increment/decrement reference counters.
 *   
 *   However, this ICM also complements the various ICMs that implement
 *   different kinds of barrier synchronisations.
 *
 ******************************************************************************/

void ICMCompileMT_SYNCBLOCK_END( int barrier_id,
                                 int vararg_cnt, char **vararg)
{
  DBUG_ENTER( "ICMCompileMT_SYNCBLOCK_END");

#define MT_SYNCBLOCK_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNCBLOCK_END

  global.indent--;
  INDENT;
  fprintf( global.outfile, "}\n");

  DBUG_VOID_RETURN;
}



/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_FOLD( int barrier_id, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_FOLD( barrier_id, vararg_cnt, [ foldtype, accu_NT, tmp_NT, foldop ]* )
 *   
 *   This ICM implements barrier synchronisation for synchronisation blocks
 *   that contain several fold with-loops but no genarray/modarray 
 *   with-loops.
 *   
 *   Each fold with-loop corresponds with a quadrupel of ICM arguments,
 *   specifying the type of the fold result, the name of the accumulator
 *   variable, the name of the accumulated variable, and the fold operation
 *   itself. The type may be either one of the scalar data types provided
 *   by SAC or one of the specifiers 'array' or 'hidden'.
 *
 ******************************************************************************/

void ICMCompileMT_SYNC_FOLD( int barrier_id, int vararg_cnt, char **vararg)
{
#ifndef BEtest
  node **foldcodes;
#endif /*BEtest*/
  char *foldop=NULL;
  int i;

  DBUG_ENTER("ICMCompileMT_SYNC_FOLD");

#define MT_SYNC_FOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNC_FOLD

  /* 
   * fetch code-elements for all fold-operations
   */
#ifndef BEtest
  foldcodes = (node**) MEMmalloc( vararg_cnt * sizeof( node*));
  for (i = 0; i < vararg_cnt; i++) {
    foldop = vararg[4*i + 3];
    foldcodes[i] = SearchFoldImplementation( foldop);
  }
#endif /* BEtest */

  INDENT;
  fprintf( global.outfile, "SAC_MT_SYNC_MULTIFOLD_1A( %d)\n", barrier_id);

  for (i = 0; i < vararg_cnt; i++) {
    INDENT;
    fprintf( global.outfile, "SAC_TR_MT_PRINT_FOLD_RESULT( %s, %s,"
                      " \"Pure thread fold result:\");\n",
                      vararg[4*i], vararg[4*i +1]);
  }

  INDENT;
  fprintf( global.outfile, "SAC_MT_SYNC_MULTIFOLD_1B( %d)\n", barrier_id);

  for (i = 0; i < vararg_cnt; i++) {
    INDENT;
    fprintf( global.outfile, "SAC_MT_SET_BARRIER_RESULT("
                      " SAC_MT_MYTHREAD(), %i, %s, %s)\n",
                      i+1, vararg[4*i], vararg[4*i +1]);
  }

  INDENT;
  fprintf( global.outfile, "SAC_MT_SYNC_MULTIFOLD_1C( %d)\n", barrier_id);

  for (i = 0; i < vararg_cnt; i++) {
    INDENT;
    fprintf( global.outfile, "SAC_TR_MT_PRINT_FOLD_RESULT( %s, %s,"
                      " \"Partial thread fold result:\");\n",
                      vararg[4*i], vararg[4*i +1]);
  }

  INDENT;
  fprintf( global.outfile, "SAC_MT_SYNC_MULTIFOLD_1D( %d)\n", barrier_id);

  for (i = 0; i < vararg_cnt; i++) {
    INDENT;
    fprintf( global.outfile, "SAC_MT_GET_BARRIER_RESULT( SAC_MT_son_id, %i, %s, %s)\n",
                      i+1, vararg[4*i], vararg[4*i +2]);
#ifndef BEtest
    INDENT;
    fprintf( global.outfile, "{\n");
    global.indent++;
    TRAVdo( foldcodes[i], NULL);
    global.indent--;
    INDENT;
    fprintf( global.outfile, "}\n");
#else  /* BEtest */
    INDENT;
    fprintf( global.outfile, "/* fold operation: %s */\n", foldop);
#endif /* BEtest */
  }

  INDENT;
  fprintf( global.outfile, "SAC_MT_SYNC_MULTIFOLD_2A( %d)\n", barrier_id);

  for (i = 0; i < vararg_cnt; i++) {
    INDENT;
    fprintf( global.outfile, "SAC_MT_SET_BARRIER_RESULT("
                      " SAC_MT_MYTHREAD(), %i, %s, %s)\n",
                      i+1, vararg[4*i], vararg[4*i +1]);
  }

  INDENT;
  fprintf( global.outfile, "SAC_MT_SYNC_MULTIFOLD_2B( %d)\n", barrier_id);

  for (i = 0; i < vararg_cnt; i++) {
    INDENT;
    fprintf( global.outfile, "SAC_TR_MT_PRINT_FOLD_RESULT( %s, %s,"
                      " \"Partial thread fold result:\");\n",
                      vararg[4*i], vararg[4*i +1]);
  }

  INDENT;
  fprintf( global.outfile, "SAC_MT_SYNC_MULTIFOLD_2C( %d)\n", barrier_id);

  for (i = 0; i < vararg_cnt; i++) {
    INDENT;
    fprintf( global.outfile, "SAC_MT_GET_BARRIER_RESULT("
                      " SAC_MT_son_id, %i, %s, %s)\n",
                      i+1, vararg[4*i], vararg[4*i +2]);

#ifndef BEtest
    INDENT;
    fprintf( global.outfile, "{\n");
    global.indent++;
    TRAVdo( foldcodes[i], NULL);
    global.indent--;
    INDENT;
    fprintf( global.outfile, "}\n");
#else  /* BEtest */
    INDENT;
    fprintf( global.outfile, "/* fold operation: %s */\n", foldop);
#endif /* BEtest */
  }

  INDENT;
  fprintf( global.outfile, "SAC_MT_SYNC_MULTIFOLD_3A( %d)\n", barrier_id);

  for (i = 0; i < vararg_cnt; i++) {
    INDENT;
    fprintf( global.outfile, "SAC_MT_SET_BARRIER_RESULT("
                      " SAC_MT_MYTHREAD(), %i, %s, %s)\n",
                      i+1, vararg[4*i], vararg[4*i +1]);
  }

  INDENT;
  fprintf( global.outfile, "SAC_MT_SYNC_MULTIFOLD_3B( %d)\n", barrier_id);

  for (i = 0; i < vararg_cnt; i++) {
    INDENT;
    fprintf( global.outfile, "SAC_TR_MT_PRINT_FOLD_RESULT( %s, %s,"
                      " \"Partial thread fold result:\");\n",
                      vararg[4*i], vararg[4*i +1]);
  }

  INDENT;
  fprintf( global.outfile, "SAC_MT_SYNC_MULTIFOLD_3C( %d)\n", barrier_id);

#ifndef BEtest
  for (i = 0; i < vararg_cnt; i++) {
    foldcodes[i] = FREEdoFreeTree( foldcodes[i]);
  }
  foldcodes = MEMfree( foldcodes);
#endif /* BEtest */

  DBUG_VOID_RETURN;
}



/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_ONEFOLD(int barrier_id,
 *                                  char *foldtype, char *accu_var,
 *                                  char *tmp_var, char *foldop)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_ONEFOLD( barrier_id, foldtype, accu_var, tmp_var, foldop)
 *   
 *   This ICM implements barrier synchronisation for synchronisation blocks
 *   that contain exactly one fold with-loop.
 *   
 *   The fold with-loop is described by 4 ICM arguments,
 *   specifying the type of the fold result, the name of the accumulator
 *   variable, the name of the accumulated variable, and the fold operation
 *   itself. The type may be either one of the scalar data types provided
 *   by SAC or one of the specifiers 'array' or 'hidden'.
 *
 ******************************************************************************/

void ICMCompileMT_SYNC_ONEFOLD( int barrier_id,
                                char *foldtype, char *accu_var,
                                char *tmp_var, char *foldop)
{
#ifndef BEtest
  node *fold_code;
#endif /* BEtest */
  
  DBUG_ENTER("ICMCompileMT_SYNC_ONEFOLD");

#define MT_SYNC_ONEFOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNC_ONEFOLD

#ifndef BEtest
  fold_code = SearchFoldImplementation( foldop);
#endif /* BEtest */

  INDENT;
  fprintf( global.outfile, "SAC_MT_SYNC_ONEFOLD_1( %s, %s, %s, %d)\n",
                    foldtype, accu_var, tmp_var, barrier_id);

#ifndef BEtest
  INDENT;
  fprintf( global.outfile, "{\n");
  global.indent++;
  TRAVdo( fold_code, NULL);
  global.indent--;
  INDENT;
  fprintf( global.outfile, "}\n");
#else  /* BEtest */
  INDENT;
  fprintf( global.outfile, "/* fold operation: %s */\n", foldop);
#endif /* BEtest */

  INDENT;
  fprintf( global.outfile, "SAC_MT_SYNC_ONEFOLD_2( %s, %s, %s, %d)\n",
                    foldtype, accu_var, tmp_var, barrier_id);

#ifndef BEtest
  INDENT;
  fprintf( global.outfile, "{\n");
  global.indent++;
  TRAVdo( fold_code, NULL);
  global.indent--;
  INDENT;
  fprintf( global.outfile, "}\n");
#else  /* BEtest */
  INDENT;
  fprintf( global.outfile, "/* fold operation: %s */\n", foldop);
#endif /* BEtest */

  INDENT;
  fprintf( global.outfile, "SAC_MT_SYNC_ONEFOLD_3( %s, %s, %s, %d)\n",
                    foldtype, accu_var, tmp_var, barrier_id);

#ifndef BEtest
  fold_code = FREEdoFreeTree( fold_code);
#endif /* BEtest */
  
  DBUG_VOID_RETURN;
}



/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_NONFOLD( int barrier_id)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_NONFOLD( barrier_id)
 *   
 *   This ICM implements barrier synchronisation for synchronisation blocks
 *   that contain exclusively modarray/genarray with-loops.
 *
 ******************************************************************************/

void ICMCompileMT_SYNC_NONFOLD( int barrier_id)
{
  DBUG_ENTER( "ICMCompileMT_SYNC_NONFOLD");

#define MT_SYNC_NONFOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNC_NONFOLD

  INDENT;
  fprintf( global.outfile, "SAC_MT_SYNC_NONFOLD_1( %d)\n", barrier_id);

  DBUG_VOID_RETURN;
}



/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_FOLD_NONFOLD( int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_FOLD_NONFOLD( vararg_cnt, [ foldtype, accu_NT, tmp_NT, foldop ]* )
 *   
 *   This ICM implements barrier synchronisation for synchronisation
 *   blocks that contain several fold with-loops as well as additional
 *   genarray/modarray with-loops.
 *   
 *   Each fold with-loop corresponds with a quadrupel of ICM arguments,
 *   specifying the type of the fold result, the name of the accumulator
 *   variable, the name of the accumulated variable, and the fold operation
 *   itself. The type may be either one of the scalar data types provided
 *   by SAC or one of the specifiers 'array' or 'hidden'.
 *
 ******************************************************************************/

void ICMCompileMT_SYNC_FOLD_NONFOLD( int vararg_cnt, char **vararg)
{
  DBUG_ENTER("ICMCompileMT_SYNC_FOLD_NONFOLD");

#define MT_SYNC_FOLD_NONFOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNC_FOLD_NONFOLD
  
  DBUG_VOID_RETURN;
}



/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_ONEFOLD_NONFOLD( char *foldtype, char *accu_var,
 *                                           char *tmp_var, char *foldop)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_ONEFOLD_NONFOLD( foldtype, accu_var, tmp_var, foldop)
 *   
 *   This ICM implements barrier synchronisation for synchronisation blocks
 *   that contain exactly one fold with-loop and additionally an arbitrary
 *   number of modarray/genarray with-loops.
 *   
 *   The fold with-loop is described by 4 ICM arguments,
 *   specifying the type of the fold result, the name of the accumulator
 *   variable, the name of the accumulated variable, and the fold operation
 *   itself. The type may be either one of the scalar data types provided
 *   by SAC or one of the specifiers 'array' or 'hidden'.
 *
 ******************************************************************************/

void ICMCompileMT_SYNC_ONEFOLD_NONFOLD( char *foldtype, char *accu_var,
                                        char *tmp_var, char *foldop)
{
  DBUG_ENTER("ICMCompileMT_SYNC_ONEFOLD_NONFOLD");

#define MT_SYNC_ONEFOLD_NONFOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNC_ONEFOLD_NONFOLD
  
  DBUG_VOID_RETURN;
}



/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MASTER_SEND_FOLDRESULTS( int foldargs_cnt, char **foldargs)
 *
 * description:
 *   compiles the corresponding ICM:
 *
 *   MT_MASTER_SEND_FOLDRESULTS( foldargs_cnt, [ foldtype, accu_NT ]* )
 *
 *   As part of the value exchange between two SYNC-blocks it is responsible
 *   to send results of fold with-loops to the SPMD-frame ... BUT ...
 *
 *   ... All results from fold-with-loops will be stored automatically in the
 *   barrier, so one does not need to put them in the frame too (they will
 *   be fetched from the barrier instead). The ICM is needed only to have a 
 *   complete set of macros, so the code is readable.
 *
 ******************************************************************************/

void ICMCompileMT_MASTER_SEND_FOLDRESULTS( int foldargs_cnt, char **foldargs)
{
  DBUG_ENTER( "ICMCompileMT_NASTER_SEND_FOLDRESULTS");

#define MT_MASTER_SEND_FOLDRESULTS
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MASTER_SEND_FOLDRESULTS

  if (foldargs_cnt > 0) {
    INDENT;
    fprintf( global.outfile,
             "/* all needed values are already stored in the barrier */");
  }

  DBUG_VOID_RETURN;
}



/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MASTER_RECEIVE_FOLDRESULTS( int foldargs_cnt, char **foldargs)
 *
 * description:
 *   compiles the corresponding ICM:
 *
 *   MT_MASTER_RECEIVE_FOLDRESULTS( foldargs_cnt, [ foldtype, accu_NT ]* )
 *
 *   As part of the value exchange between two SYNC-blocks it is responsible
 *   to receive results of fold with-loops from the SPMD-frame ... BUT ...
 *
 *   ... All results from fold-with-loops will be stored automatically in the
 *   barrier, so one does not put them in the frame. Here they will be fetched
 *   from the barrier instead. 
 *
 ******************************************************************************/

void ICMCompileMT_MASTER_RECEIVE_FOLDRESULTS( int foldargs_cnt, char **foldargs)
{
  int i, j;

  DBUG_ENTER( "ICMCompileMT_NASTER_RECEIVE_FOLDRESULTS");

#define MT_MASTER_RECEIVE_FOLDRESULTS
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MASTER_RECEIVE_FOLDRESULTS

  for (i = 0, j = 1; i < 2 * foldargs_cnt; i += 2, j++) {
    INDENT;
    fprintf( global.outfile, "SAC_MT_GET_BARRIER_RESULT( 0, %d, %s, %s)\n",
                      j, foldargs[i], foldargs[i+1]);
  }

  DBUG_VOID_RETURN;
}



/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MASTER_SEND_SYNCARGS( int syncargs_cnt, char **syncargs)
 *
 * description:
 *   compiles the corresponding ICM:
 *
 *   MT_MASTER_SEND_SYNCARGS( syncargs_cnt, [ arg_NT ]* )
 *
 *   As part of the value exchange between two SYNC-blocks it is responsible
 *   to send values changed in the master part.
 *   The values are put in the SPMD-frame.
 *
 ******************************************************************************/

void ICMCompileMT_MASTER_SEND_SYNCARGS( int syncargs_cnt, char **syncargs)
{
  int i;

  DBUG_ENTER( "ICMCompileMT_MASTER_SEND_SYNCARGS");

#define MT_MASTER_SEND_SYNCARGS
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MASTER_SEND_SYNCARGS

  for (i = 0; i < syncargs_cnt; i++) {
    INDENT;
    fprintf( global.outfile, "SAC_MT_SPMD_RET_shared( %s);\n", syncargs[i]); 
  }

  DBUG_VOID_RETURN;
}



/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MASTER_RECEIVE_SYNCARGS( int syncargs_cnt, char **syncargs)
 *
 * description:
 *   compiles the corresponding ICM:
 *
 *   MT_MASTER_RECEIVE_SYNCARGS( syncargs_cnt, [ arg_NT ]* )
 *
 *   As part of the value exchange between two SYNC-blocks it is responsible
 *   to receive values changed in the master part. 
 *   The values are fetched from the SPMD-frame.
 *
 ******************************************************************************/

void ICMCompileMT_MASTER_RECEIVE_SYNCARGS( int syncargs_cnt, char **syncargs)
{
  int i;

  DBUG_ENTER( "ICMCompileMT_MASTER_RECEIVE_SYNCARGS");

#define MT_MASTER_RECEIVE_SYNCARGS
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MASTER_RECEIVE_SYNCARGS

  for (i = 0; i < syncargs_cnt; i++) {
    INDENT;
    fprintf( global.outfile, "SAC_MT_SPMD_GET_shared( %s);\n", syncargs[i]);
  }

  DBUG_VOID_RETURN;
}



/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_SETUP( char *name, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_SETUP( name, vararg_cnt, [ tag, type, arg_NT ]* )
 *
 *   the arguments handed over could have the tags:
 *     in, out, shared.
 *   One needs to set the in variables to the spmd-region, that is normal.
 *   One also needs to set the adresses ot the out-variables, so the 
 *   spmd-region can put the values there automatically.
 *   One does *not* need to set the shared variables here (they are only used
 *   within the spmd-region), so they ar ignored here!
 *   Because this icm is also used to determine the spmd-frame, one needs
 *   to have the shared-variabels here (until this ic changed in someway).
 *
 ******************************************************************************/

void ICMCompileMT_SPMD_SETUP( char *name, int vararg_cnt, char **vararg)
{
  int i;
  
  DBUG_ENTER("ICMCompileMT_SPMD_SETUP");

#define MT_SPMD_SETUP
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_SETUP

  for (i = 0; i < 3 * vararg_cnt; i += 3) {
    if ((! strcmp( vararg[i], "in")) ||
        (! strcmp( vararg[i], "out"))) {
      INDENT;
      fprintf( global.outfile, "SAC_MT_SPMD_SETARG_%s( %s, %s);\n",
                        vararg[i], name, vararg[i+2]);
    }
  }
  
  DBUG_VOID_RETURN;
}



/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_BEGIN( char *name)
 *   void ICMCompileMT_SPMD_ALTSEQ( char *name)
 *   void ICMCompileMT_SPMD_END( char *name)
 *
 * description:
 *   These functions implement the control flow ICMs around SPMD-blocks.
 *
 ******************************************************************************/

void ICMCompileMT_SPMD_BEGIN( char *name)
{
  DBUG_ENTER("ICMCompileMT_SPMD_BEGIN");

#define MT_SPMD_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_BEGIN

  fprintf( global.outfile, "\n"
                    "#if SAC_DO_MULTITHREAD\n");
  INDENT;
  fprintf( global.outfile, "if (SAC_MT_not_yet_parallel)\n");
  INDENT;
  fprintf( global.outfile, "{\n");
  global.indent++;
  INDENT;
  fprintf( global.outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_single_threaded)\n");
  
  DBUG_VOID_RETURN;
}


void ICMCompileMT_SPMD_ALTSEQ( char *name)
{
  DBUG_ENTER("ICMCompileMT_SPMD_ALTSEQ");

  global.indent--;

#define MT_SPMD_ALTSEQ
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_ALTSEQ

  INDENT;
  fprintf( global.outfile, "}\n");
  INDENT;
  fprintf( global.outfile, "else\n");
  INDENT;
  fprintf( global.outfile, "{\n");
  global.indent++;
  INDENT;
  fprintf( global.outfile, "SAC_MT_DETERMINE_THREAD_ID()\n");
  INDENT;
  fprintf( global.outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_multi_threaded)\n");
  fprintf( global.outfile, "#endif  /* SAC_DO_MULTITHREAD */\n\n");
  
  DBUG_VOID_RETURN;
}


void ICMCompileMT_SPMD_END( char *name)
{
  DBUG_ENTER("ICMCompileMT_SPMD_END");

#define MT_SPMD_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_END

  fprintf( global.outfile, "\n"
                    "#if SAC_DO_MULTITHREAD\n");
  global.indent--;
  INDENT;
  fprintf( global.outfile, "}\n");
 
  fprintf( global.outfile, "#endif  /* SAC_DO_MULTITHREAD */\n\n");
  
  DBUG_VOID_RETURN;
}

#endif
