/*
 *
 * $Log$
 * Revision 1.4  1995/07/24 09:09:34  asi
 * macro TYPES renamed to INL_TYPES
 *
 * Revision 1.3  1995/06/08  10:03:44  asi
 * added SearchDecl and removed SetDeclPtr
 *
 * Revision 1.2  1995/06/02  11:29:35  asi
 * Added Inline, INLfundef, INLblock, INLassign, RenameInlinedVar, SetDeclPtr and INLvar.
 *
 * Revision 1.1  1995/05/26  14:22:18  asi
 * Initial revision
 *
 *
 */

#ifndef _Inline_h

#define _Inline_h

#define INL_TYPES arg_info->node[2]

extern node *Inline (node *arg_node, node *arg_info);

extern node *INLmodul (node *arg_node, node *arg_info);
extern node *INLfundef (node *arg_node, node *arg_info);
extern node *INLassign (node *arg_node, node *arg_info);
extern node *INLvar (node *arg_node, node *arg_info);
extern node *INLblock (node *arg_node, node *arg_info);

extern node *SearchDecl (char *name, node *decl_node);
extern char *RenameInlinedVar (char *old_name);

#endif /* _Inline_h */
