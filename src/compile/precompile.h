/*
 *
 * $Log$
 * Revision 1.17  1998/06/03 14:53:41  cg
 * Now, all identifiers including local ones are systematically renamed.
 *
 * Revision 1.16  1998/04/29 17:20:09  dkr
 * with-loop transformation moved to wltransform.[ch]
 *
 * Revision 1.15  1998/04/26 21:49:52  dkr
 * PRECSPMD renamed to PRECSpmd
 *
 * Revision 1.14  1998/04/24 17:14:51  dkr
 * renamed Prec...() to PREC...()
 *
 * Revision 1.13  1998/04/23 17:33:16  dkr
 * added PrecSync
 *
 * Revision 1.12  1998/04/17 17:27:06  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.11  1998/04/17 15:28:57  dkr
 * InsertWLnodes is now external
 *
 * Revision 1.10  1998/04/17 02:14:24  dkr
 * exported GridOffset()
 *
 * Revision 1.9  1998/04/02 18:47:08  dkr
 * added PRECconc
 *
 * Revision 1.8  1998/03/03 22:57:38  dkr
 * added PRECncode()
 *
 * Revision 1.7  1998/03/02 22:26:52  dkr
 * added PRECnwith()
 *
 * Revision 1.6  1997/09/05 13:46:04  cg
 * All cast expressions are now removed by rmvoidfun.c. Therefore,
 * the respective attempts in precompile.c and ConstantFolding.c
 * are removed. Cast expressions are only used by the type checker.
 * Afterwards, they are useless, and they are not supported by
 * Constant Folding as well as code generation.
 *
 * Revision 1.5  1997/04/30 11:55:34  cg
 * new function PRECassign added
 *
 * Revision 1.4  1995/12/18  16:31:38  cg
 * declaration of PRECexprs removed
 *
 * Revision 1.3  1995/12/04  17:00:04  cg
 * added function PRECcast
 * All casts are now eliminated by the precompiler
 *
 * Revision 1.2  1995/12/01  20:29:00  cg
 * added declarations of PRECvardec and PRECtypedef
 *
 * Revision 1.1  1995/11/28  12:23:34  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_precompile_h

#define _sac_precompile_h

extern node *precompile (node *syntax_tree);

extern node *PRECmodul (node *arg_node, node *arg_info);
extern node *PRECobjdef (node *arg_node, node *arg_info);
extern node *PRECfundef (node *arg_node, node *arg_info);
extern node *PRECarg (node *arg_node, node *arg_info);
extern node *PREClet (node *arg_node, node *arg_info);
extern node *PRECap (node *arg_node, node *arg_info);
extern node *PRECassign (node *arg_node, node *arg_info);
extern node *PRECreturn (node *arg_node, node *arg_info);
extern node *PRECid (node *arg_node, node *arg_info);
extern node *PRECvardec (node *arg_node, node *arg_info);
extern node *PRECtypedef (node *arg_node, node *arg_info);
extern node *PRECgenerator (node *arg_node, node *arg_info);
extern node *PRECNwithid (node *arg_node, node *arg_info);
extern node *PRECdo (node *arg_node, node *arg_info);
extern node *PRECwhile (node *arg_node, node *arg_info);
extern node *PRECcond (node *arg_node, node *arg_info);
extern node *PRECwith (node *arg_node, node *arg_info);
extern node *PRECNwith (node *arg_node, node *arg_info);
extern node *PRECNwith2 (node *arg_node, node *arg_info);
extern node *PRECNcode (node *arg_node, node *arg_info);

#endif /* _sac_precompile_h */
