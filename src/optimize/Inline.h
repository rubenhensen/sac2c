/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:41:21  sacbase
 * new release made
 *
 * Revision 1.8  1998/07/16 17:19:17  sbs
 * DoInline made privat and InlineSingleApplication exported!
 *
 * Revision 1.7  1998/07/16 11:40:51  sbs
 * exported DoInline which is needed in WLUnroll.c
 *
 * Revision 1.6  1998/04/16 16:06:29  srs
 * removed INL_TYPES
 *
 * Revision 1.5  1995/12/21 15:26:46  asi
 * INLblock removed
 *
 * Revision 1.4  1995/07/24  09:09:34  asi
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

extern node *Inline (node *arg_node, node *arg_info);
extern node *InlineSingleApplication (node *let_node, node *fundef_node);

extern node *INLmodul (node *arg_node, node *arg_info);
extern node *INLfundef (node *arg_node, node *arg_info);
extern node *INLassign (node *arg_node, node *arg_info);
extern node *INLarg (node *arg_node, node *arg_info);
extern node *INLvardec (node *arg_node, node *arg_info);

extern node *SearchDecl (char *name, node *decl_node);
extern char *RenameInlinedVar (char *old_name);

#endif /* _Inline_h */
