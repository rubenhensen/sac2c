#ifndef _SAC_MARK_LOCAL_SELECTS_H_
#define _SAC_MARK_LOCAL_SELECTS_H_

#include "types.h"

extern node *DMMLSwith (node *arg_node, info *arg_info);
extern node *DMMLSpart (node *arg_node, info *arg_info);
extern node *DMMLSprf (node *arg_node, info *arg_info);
extern node *DMMLSfundef (node *arg_node, info *arg_info);

extern node *DMMLSdoMarkLocalSelects (node *syntax_tree);

#endif /* _SAC_MARK_LOCAL_SELECTS_H_ */
