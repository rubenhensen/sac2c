/*
 *
 * $Log$
 * Revision 1.2  1995/10/22 15:56:31  cg
 * Now, declaration files will be generated automatically if not
 * present at compile time of module/class implementation.
 *
 * Revision 1.1  1995/10/22  14:23:26  cg
 * Initial revision
 *
 *
 *
 */

extern node *CheckDec (node *syntax_tree);

extern node *CDECmoddec (node *arg_node, node *arg_info);
extern node *CDECexplist (node *arg_node, node *arg_info);
extern node *CDECtypedef (node *arg_node, node *arg_info);
extern node *CDECobjdef (node *arg_node, node *arg_info);
extern node *CDECfundef (node *arg_node, node *arg_info);

extern node *WDECmodul (node *arg_node, node *arg_info);
extern node *WDECtypedef (node *arg_node, node *arg_info);
extern node *WDECobjdef (node *arg_node, node *arg_info);
extern node *WDECfundef (node *arg_node, node *arg_info);
extern node *WDECarg (node *arg_node, node *arg_info);
