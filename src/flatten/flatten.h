/*
 *
 * $Log$
 * Revision 3.4  2004/11/22 12:37:33  ktr
 * Ismop SacDevCamp 04
 * ,.
 *
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

#ifndef _SAC_FLATTEN_H_
#define _SAC_FLATTEN_H_

#include "types.h"

/******************************************************************************
 *
 * Flatten traversal ( flat_tab)
 *
 * Prefix: FLAT
 *
 *****************************************************************************/
extern node *FLATdoFlatten (node *syntax_tree);

extern node *FLATblock (node *arg_node, info *arg_info);
extern node *FLATassign (node *arg_node, info *arg_info);
extern node *FLATexprs (node *arg_node, info *arg_info);
extern node *FLATcond (node *arg_node, info *arg_info);
extern node *FLATwhile (node *arg_node, info *arg_info);
extern node *FLATdo (node *arg_node, info *arg_info);
extern node *FLATmodule (node *arg_node, info *arg_info);
extern node *FLATfundef (node *arg_node, info *arg_info);
extern node *FLATap (node *arg_node, info *arg_info);
extern node *FLATarray (node *arg_node, info *arg_info);
extern node *FLATreturn (node *arg_node, info *arg_info);
extern node *FLATid (node *arg_node, info *arg_info);
extern node *FLATlet (node *arg_node, info *arg_info);
extern node *FLATargs (node *arg_node, info *arg_info);
extern node *FLATprf (node *arg_node, info *arg_info);
extern node *FLATwith (node *arg_node, info *arg_info);
extern node *FLATwithid (node *arg_node, info *arg_info);
extern node *FLATpart (node *arg_node, info *arg_info);
extern node *FLATgenerator (node *arg_node, info *arg_info);
extern node *FLATgenarray (node *arg_node, info *arg_info);
extern node *FLATmodarray (node *arg_node, info *arg_info);
extern node *FLATfold (node *arg_node, info *arg_info);
extern node *FLATcode (node *arg_node, info *arg_info);

#endif /* _SAC_FLATTEN_H_ */
