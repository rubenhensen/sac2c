/*
 *
 * $Log$
 * Revision 3.2  2004/07/17 19:50:26  sah
 * switch to INFO structure
 * PHASE I
 *
 * Revision 3.1  2000/11/20 18:00:51  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:42:03  sacbase
 * new release made
 *
 * Revision 1.3  1995/10/22 17:37:59  cg
 * first compilable revision
 *
 * Revision 1.2  1995/10/22  15:56:31  cg
 * Now, declaration files will be generated automatically if not
 * present at compile time of module/class implementation.
 *
 * Revision 1.1  1995/10/22  14:23:26  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_checkdec_h
#define _sac_checkdec_h

extern node *CheckDec (node *syntax_tree);

extern node *CDECmoddec (node *arg_node, info *arg_info);
extern node *CDECexplist (node *arg_node, info *arg_info);
extern node *CDECtypedef (node *arg_node, info *arg_info);
extern node *CDECobjdef (node *arg_node, info *arg_info);
extern node *CDECfundef (node *arg_node, info *arg_info);

extern node *WDECmodul (node *arg_node, info *arg_info);
extern node *WDECtypedef (node *arg_node, info *arg_info);
extern node *WDECobjdef (node *arg_node, info *arg_info);
extern node *WDECfundef (node *arg_node, info *arg_info);
extern node *WDECarg (node *arg_node, info *arg_info);

#endif /* _sac_checkdec_h */
