/*
 *
 * $Log$
 * Revision 1.4  2000/12/06 20:05:53  dkr
 * ups, syntax error eliminated
 *
 * Revision 1.3  2000/12/06 20:03:23  dkr
 * InferDFMs added
 *
 * Revision 1.1  2000/12/06 19:57:54  dkr
 * Initial revision
 *
 */

#ifndef _sac_infer_dfms_h
#define _sac_infer_dfms_h

extern node *INFDFMSfundef (node *arg_node, node *arg_info);
extern node *INFDFMSarg (node *arg_node, node *arg_info);
extern node *INFDFMSassign (node *arg_node, node *arg_info);
extern node *INFDFMSlet (node *arg_node, node *arg_info);
extern node *INFDFMSap (node *arg_node, node *arg_info);
extern node *INFDFMSid (node *arg_node, node *arg_info);
extern node *INFDFMSwithid (node *arg_node, node *arg_info);
extern node *INFDFMScode (node *arg_node, node *arg_info);
extern node *INFDFMSwith (node *arg_node, node *arg_info);
extern node *INFDFMSwith2 (node *arg_node, node *arg_info);
extern node *INFDFMScond (node *arg_node, node *arg_info);
extern node *INFDFMSwhile (node *arg_node, node *arg_info);
extern node *INFDFMSdo (node *arg_node, node *arg_info);

extern node *InferDFMs (node *syntax_tree);

#endif /* _sac_infer_dfms_h */
