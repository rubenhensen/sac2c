/*
 * $Log$
 * Revision 1.4  2004/06/25 09:36:21  skt
 * added TEMlet and some helper functions
 *
 * Revision 1.3  2004/06/23 15:44:47  skt
 * TEMreturn, TEMap & TEMarray added
 *
 * Revision 1.2  2004/06/23 09:42:34  skt
 * TEMprf, TEMexprs & some helper functions added
 *
 * Revision 1.1  2004/06/08 14:16:34  skt
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   tag_executionmode.h
 *
 * description:
 *   header file for tag_executionmode.c
 *
 *****************************************************************************/

#ifndef TAG_EXECUTIONMODE_H

#define TAG_EXECUTIONMODE_H

/* access macros for arg_info
 *
 *   node*      ORIGLHS    (left-hand-side of the assignemt, before F_fill was
 *                         added / args 2..n of fill())
 *   int        EXECMODE  (the current execution mode)
 *   int        WITHDEEP  (the current with-loop-deepness)
 */
#define INFO_TEM_ORIGLHS(n) (n->node[0])
#define INFO_TEM_LETLHS(n) (n->node[1])
#define INFO_TEM_EXECMODE(n) (n->refcnt)
#define INFO_TEM_WITHDEEP(n) (n->flag)
#define TEM_DEBUG 0

extern node *TagExecutionmode (node *arg_node, node *arg_info);

extern node *TEMassign (node *arg_node, node *arg_info);

extern node *TEMwith2 (node *arg_node, node *arg_info);

extern node *TEMprf (node *arg_node, node *arg_info);

extern node *TEMlet (node *arg_node, node *arg_info);

extern node *TEMreturn (node *arg_node, node *arg_info);

extern node *TEMap (node *arg_node, node *arg_info);

extern node *TEMarray (node *arg_node, node *arg_info);

int IsMTAllowed (node *withloop);

int IsGeneratorBigEnough (node *exprs);

int IsMTClever (node *exprs);

int IsSTClever (node *exprs);

int StrongestRestriction (int execmode1, int execmode2);

node *TagAllocs (node *exprs);

#if TEM_DEBUG
char *DecodeExecmode (int execmode);
#endif

#endif /* TAG_EXECUTIONMODE_H */
