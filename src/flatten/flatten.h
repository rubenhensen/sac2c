/*
 *
 * $Log$
 * Revision 1.15  1997/11/25 12:37:33  srs
 * *** empty log message ***
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
 * >> FltnAp, FltnReturn inserted
 *
 * Revision 1.9  1995/03/07  11:00:03  hw
 * added function FltnGen (flatten N_generator)
 *
 * Revision 1.8  1995/01/06  16:45:15  hw
 * added FltnFundef
 *
 * Revision 1.7  1994/12/15  11:47:06  hw
 * inserted FltnModul
 *
 * Revision 1.6  1994/11/22  16:39:55  hw
 * added FltnDo
 *
 * Revision 1.5  1994/11/17  16:50:16  hw
 * added FltnWhile
 *
 * Revision 1.4  1994/11/15  14:45:31  hw
 * deleted FltnFor
 *
 * Revision 1.3  1994/11/14  17:50:53  hw
 * added FltnCond FltnFor
 *
 * Revision 1.2  1994/11/10  15:39:42  sbs
 * RCS-header inserted
 *
 *
 */

#ifndef _flatten_h

#define _flatten_h

extern node *Flatten (node *);
extern node *FltnAssign (node *arg_node, node *arg_info);
extern node *FltnExprs (node *arg_node, node *arg_info);
extern node *FltnCond (node *arg_node, node *arg_info);
extern node *FltnWhile (node *arg_node, node *arg_info);
extern node *FltnWith (node *arg_node, node *arg_info);
extern node *FltnDo (node *arg_node, node *arg_info);
extern node *FltnModul (node *arg_node, node *arg_info);
extern node *FltnFundef (node *arg_node, node *arg_info);
extern node *FltnGen (node *arg_node, node *arg_info);
extern node *FltnAp (node *arg_node, node *arg_info);
extern node *FltnReturn (node *arg_node, node *arg_info);
extern node *FltnId (node *arg_node, node *arg_info);
extern node *FltnLet (node *arg_node, node *arg_info);
extern node *FltnArgs (node *arg_node, node *arg_info);
extern node *FltnCon (node *arg_node, node *arg_info);
extern node *FltnPrf (node *arg_node, node *arg_info);
extern node *FltnNwith (node *arg_node, node *arg_info);
extern node *FltnNpart (node *arg_node, node *arg_info);
extern node *FltnNwithid (node *arg_node, node *arg_info);
extern node *FltnNgenerator (node *arg_node, node *arg_info);
extern node *FltnNwithop (node *arg_node, node *arg_info);
extern node *FltnNcode (node *arg_node, node *arg_info);

#endif /* _flatten_h  */
