#ifndef _sac_ssarefcount_h
#define _sac_ssarefcount_h

extern node *SSARefCount (node *syntax_tree);

extern node *SSARCfundef (node *arg_node, node *arg_info);
extern node *SSARCassign (node *arg_node, node *arg_info);
extern node *SSARCreturn (node *arg_node, node *arg_info);
extern node *SSARCprf (node *arg_node, node *arg_info);
extern node *SSARCid (node *arg_node, node *arg_info);
extern node *SSARClet (node *arg_node, node *arg_info);
extern node *SSARCarg (node *arg_node, node *arg_info);
extern node *SSARCblock (node *arg_node, node *arg_info);
extern node *SSARCvardec (node *arg_node, node *arg_info);
extern node *SSARCap (node *arg_node, node *arg_info);
extern node *SSARCnum (node *arg_node, node *arg_info);
extern node *SSARCchar (node *arg_node, node *arg_info);
extern node *SSARCbool (node *arg_node, node *arg_info);
extern node *SSARCfloat (node *arg_node, node *arg_info);
extern node *SSARCdouble (node *arg_node, node *arg_info);
extern node *SSARCstr (node *arg_node, node *arg_info);
extern node *SSARCarray (node *arg_node, node *arg_info);

#endif /* _sac_ssarefcount_h */
