/*
 * $Log$
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
 *   int        FILLDEEP  (the current deepness of primitive function fill)
 */
#define INFO_TEM_ORIGLHS(n) (n->node[0])
#define INFO_TEM_EXECMODE(n) (n->refcnt)
#define INFO_TEM_WITHDEEP(n) (n->flag)
#define INFO_TEM_FILLDEEP(n) (n->counter)

extern node *TagExecutionmode (node *arg_node, node *arg_info);

extern node *TEMassign (node *arg_node, node *arg_info);

extern node *TEMwith2 (node *arg_node, node *arg_info);

extern node *TEMprf (node *arg_node, node *arg_info);

extern node *TEMexprs (node *arg_node, node *arg_info);

int IsMTAllowed (node *withloop);

int IsGeneratorBigEnough (node *exprs);

int IsMTClever (node *exprs);

node *TagAllocs (node *exprs);

#endif /* TAG_EXECUTIONMODE_H */
