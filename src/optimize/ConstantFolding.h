/*
 *
 * $Log$
 * Revision 1.15  1998/12/10 17:27:34  sbs
 * ScalarPrf => FoldPrfScalars
 *
 * Revision 1.14  1998/03/10 17:31:19  srs
 * exported ScalarPrf (used in WithloopFolding.c)
 *
 * Revision 1.13  1998/02/15 19:52:39  srs
 * added CF for new WL
 *
 * Revision 1.12  1997/09/05 13:46:04  cg
 * All cast expressions are now removed by rmvoidfun.c. Therefore,
 * the respective attempts in precompile.c and ConstantFolding.c
 * are removed. Cast expressions are only used by the type checker.
 * Afterwards, they are useless, and they are not supported by
 * Constant Folding as well as code generation.
 *
 * Revision 1.11  1996/01/17 14:37:31  asi
 * stack-handling moved to optimize.h
 *
 * Revision 1.10  1995/07/04  16:32:58  asi
 * IsConst defined global
 *
 * Revision 1.9  1995/06/26  16:23:39  asi
 * some macros moved from .c file to .h file
 *
 * Revision 1.8  1995/06/26  11:49:48  asi
 * functions for stack-handling added
 *
 * Revision 1.7  1995/06/20  15:50:30  asi
 * added macros TOS, VAR
 *
 * Revision 1.6  1995/05/03  16:25:48  asi
 * CFap added
 *
 * Revision 1.5  1995/03/07  10:17:09  asi
 * added CFcast
 *
 * Revision 1.4  1995/02/28  18:34:45  asi
 * added CFwith
 *
 * Revision 1.3  1995/02/22  18:16:34  asi
 * Fuctions CFfundef, CFassign, CFdo, CFwhile, CFcond, CFprf, CFid, CFlet
 * and PushVL, PushDupVL, PopVL for stack-management added
 *
 * Revision 1.2  1995/02/14  09:59:56  asi
 * added CFid
 *
 * Revision 1.1  1995/02/13  16:55:36  asi
 * Initial revision
 *
 *
 */

#ifndef _ConstantFolding_h

#define _ConstantFolding_h

extern int IsConst (node *arg_node);

extern node *ConstantFolding (node *arg_node, node *arg_info);

extern node *CFlet (node *arg_node, node *arg_info);
extern node *CFprf (node *arg_node, node *arg_info);
extern node *CFid (node *arg_node, node *arg_info);
extern node *CFfundef (node *arg_node, node *arg_info);
extern node *CFwhile (node *arg_node, node *arg_info);
extern node *CFdo (node *arg_node, node *arg_info);
extern node *CFcond (node *arg_node, node *arg_info);
extern node *CFvar (node *arg_node, node *arg_info);
extern node *CFassign (node *arg_node, node *arg_info);
extern node *CFwith (node *arg_node, node *arg_info);
extern node *CFap (node *arg_node, node *arg_info);

extern node *CFNwith (node *, node *);
extern node *CFNpart (node *, node *);
extern node *CFNcode (node *, node *);

/*
 * used in WLF:
 */
extern node *FoldPrfScalars (prf prf_name, node **arg, types *res_type, int swap);

#endif /* _ConstantFolding_h */
