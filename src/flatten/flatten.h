/*
 *
 * $Log$
 * Revision 3.3  2004/07/16 14:41:34  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.2  2002/08/07 12:32:39  dkr
 * FltnNwithid added
 *
 * Revision 3.1  2000/11/20 17:59:20  sacbase
 * new release made
 *
 * Revision 2.5  2000/05/30 12:35:23  dkr
 * functions for old with-loop removed
 *
 * Revision 2.4  2000/05/17 14:57:17  dkr
 * FltnCon removed (old with-loop)
 *
 * Revision 2.3  1999/05/12 08:42:26  sbs
 * eliminated external decl of FltnPreTypeCheck
 *
 * Revision 2.2  1999/03/19 09:41:48  bs
 * PreTypecheck mutated to the global function FltnPreTypecheck
 *
 * Revision 2.1  1999/02/23 12:39:10  sacbase
 * new release made
 *
 * Revision 1.17  1998/05/20 20:16:54  sbs
 * new funs FltnArray and FltnBlock added
 *
 * Revision 1.14  1996/09/11 06:10:04  cg
 * Now, arrays as arguments to psi and modarray are abstracted.
 * This is necessary to overload these functiond with user-defined ones.
 *
 * Revision 1.13  1995/05/30  06:45:33  hw
 * - FltnMod deleted
 * - FltnCon inserted
 *
 * Revision 1.12  1995/04/28  11:45:23  hw
 *  - added FltnMod
 *
 * Revision 1.11  1995/04/21  12:36:20  hw
 * - added FltnId, FltnLet & FltnArgs
 * - removed FltnPrf
 *
 * Revision 1.10  1995/04/07  13:41:35  hw
 * FltnAp, FltnReturn inserted
 *
 * Revision 1.9  1995/03/07  11:00:03  hw
 * added function FltnGen (flatten N_generator)
 *
 * Revision 1.8  1995/01/06  16:45:15  hw
 * added FltnFundef
 *
 * [...]
 *
 */

#ifndef _flatten_h
#define _flatten_h

extern node *Flatten (node *);
extern node *FltnBlock (node *arg_node, info *arg_info);
extern node *FltnAssign (node *arg_node, info *arg_info);
extern node *FltnExprs (node *arg_node, info *arg_info);
extern node *FltnCond (node *arg_node, info *arg_info);
extern node *FltnWhile (node *arg_node, info *arg_info);
extern node *FltnDo (node *arg_node, info *arg_info);
extern node *FltnModul (node *arg_node, info *arg_info);
extern node *FltnFundef (node *arg_node, info *arg_info);
extern node *FltnAp (node *arg_node, info *arg_info);
extern node *FltnArray (node *arg_node, info *arg_info);
extern node *FltnReturn (node *arg_node, info *arg_info);
extern node *FltnId (node *arg_node, info *arg_info);
extern node *FltnLet (node *arg_node, info *arg_info);
extern node *FltnArgs (node *arg_node, info *arg_info);
extern node *FltnPrf (node *arg_node, info *arg_info);
extern node *FltnNwith (node *arg_node, info *arg_info);
extern node *FltnNwithid (node *arg_node, info *arg_info);
extern node *FltnNpart (node *arg_node, info *arg_info);
extern node *FltnNgenerator (node *arg_node, info *arg_info);
extern node *FltnNwithop (node *arg_node, info *arg_info);
extern node *FltnNcode (node *arg_node, info *arg_info);

#endif /* _flatten_h  */
