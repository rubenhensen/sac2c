/*
 *
 * $Log$
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

extern node *PrecModul (node *arg_node, node *arg_info);
extern node *PrecObjdef (node *arg_node, node *arg_info);
extern node *PrecFundef (node *arg_node, node *arg_info);
extern node *PrecArg (node *arg_node, node *arg_info);
extern node *PrecLet (node *arg_node, node *arg_info);
extern node *PrecAp (node *arg_node, node *arg_info);
extern node *PrecAssign (node *arg_node, node *arg_info);
extern node *PrecReturn (node *arg_node, node *arg_info);
extern node *PrecId (node *arg_node, node *arg_info);
extern node *PrecVardec (node *arg_node, node *arg_info);
extern node *PrecTypedef (node *arg_node, node *arg_info);
extern node *PrecSPMD (node *arg_node, node *arg_info);
extern node *PrecNwith (node *arg_node, node *arg_info);
extern node *PrecNcode (node *arg_node, node *arg_info);

extern int GridOffset (int new_bound1, int bound1, int step, int grid_b2);
extern node *InsertWLnodes (node *nodes, node *insert_nodes);

#endif /* _sac_precompile_h */
