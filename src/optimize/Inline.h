/*
 *
 * $Log$
 * Revision 1.2  1995/06/02 11:29:35  asi
 * Added Inline, INLfundef, INLblock, INLassign, RenameInlinedVar, SetDeclPtr and INLvar.
 *
 * Revision 1.1  1995/05/26  14:22:18  asi
 * Initial revision
 *
 *
 */

#ifndef _Inline_h

#define _Inline_h

extern node *Inline (node *arg_node, node *arg_info);

extern node *INLfundef (node *arg_node, node *arg_info);
extern node *INLassign (node *arg_node, node *arg_info);
extern node *INLvar (node *arg_node, node *arg_info);
extern node *INLblock (node *arg_node, node *arg_info);

extern char *RenameInlinedVar (char *name);
extern ids *SetDeclPtr (ids *ids_node, node *arg_info);
#endif /* _Inline_h */
