/*
 * $Log$
 * Revision 1.4  2001/05/17 13:29:29  cg
 * De-allocation macros FREE_INTERN_GEN and FREE_INDEX_INFO
 * converted to functions.
 *
 * Revision 1.3  2001/05/16 13:43:08  nmw
 * unused old code removed, comments corrected
 * MALLOC/FREE changed to Malloc/Free
 *
 * Revision 1.2  2001/05/15 16:39:21  nmw
 * SSAWithloopFolding implemented (but not tested)
 *
 * Revision 1.1  2001/05/14 15:55:15  nmw
 * Initial revision
 *
 *
 * created from: WithloopFolding.h, Revision 3.1  on 2001/05/14 by  nmw
 */

#ifndef _SSAWithloopFolding_h_
#define _SSAWithloopFolding_h_

/******************************************************************************
 *
 * exported functions
 *
 ******************************************************************************/

/* general functions */
extern node *SSAWithloopFolding (node *arg_node, int loop);
extern node *SSAWithloopFoldingWLT (node *arg_node);
extern int SSALocateIndexVar (node *idn, node *wln);
extern node *SSACreateVardec (char *name, types *type, node **vardecs);
extern void SSAArrayST2ArrayInt (node *arrayn, int **iarray, int shape);

/* index_info related functions */
extern index_info *SSACreateIndex (int vector);
extern index_info *SSADuplicateIndexInfo (index_info *iinfo);
extern index_info *SSAValidLocalId (node *idn);
/* extern void        DbugIndexInfo      (index_info *iinfo);
   is implemented in WithloopFolding.c ! (old code) */

/* intern_gen related functions */
extern intern_gen *SSATree2InternGen (node *wln, node *filter);
extern node *SSAInternGen2Tree (node *wln, intern_gen *ig);
extern int SSANormalizeInternGen (intern_gen *ig);
extern intern_gen *SSACreateInternGen (int shape, int stepwidth);
extern intern_gen *SSAAppendInternGen (intern_gen *, int, node *, int);
extern intern_gen *SSACopyInternGen (intern_gen *source);
extern intern_gen *SSAFreeInternGen (intern_gen *tmp);
extern intern_gen *SSAFreeInternGenChain (intern_gen *ig);
/*  extern void        DbugInternGen      (intern_gen *ig);
    is implemented in WithloopFolding.c ! (old code) */

/******************************************************************************
 *
 * defines
 *
 ******************************************************************************/

/* if not defined, indexes with more than one occurence of an
   index scalar are allowed to be valid transformations, e.g. [i,i,j] */
/* #define TRANSF_TRUE_PERMUTATIONS */

/* index_info related macros. */
#define SSAINDEX(n) ((index_info *)ASSIGN_INDEX (n))

#endif
