/*
 * $Log$
 * Revision 1.2  2004/09/21 16:34:27  sah
 * ongoing implementation of
 * serialize traversal
 *
 * Revision 1.1  2004/09/20 19:55:28  sah
 * Initial revision
 *
 *
 *
 */

extern void SerializeModule (node *module);

extern void SerializeFundefHead (node *fundef);
extern void SerializeFundefBody (node *fundef);

extern node *SERFundef (node *arg_node, info *arg_info);
