/*
 *
 * $Log$
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

#endif /* _sac_infer_dfms_h */
