#ifndef _SAC_TRAVERSE_HELPER_H_
#define _SAC_TRAVERSE_HELPER_H_

#include "types.h"

extern node *TRAVsons (node *arg_node, info *arg_info);
extern node *TRAVnone (node *arg_node, info *arg_info);
extern node *TRAVerror (node *arg_node, info *arg_info);

/* these functions are a ugly hack for CMPtree! */
extern int TRAVnumSons (node *parent);
extern node *TRAVgetSon (int no, node *parent);

#endif /* _SAC_TRAVERSE_HELPER_H_ */
