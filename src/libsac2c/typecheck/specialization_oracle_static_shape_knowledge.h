#ifndef _SPECIALIZATION_ORACLE_STATIC_SHAPE_KNOWLEDGE_H_
#define _SPECIALIZATION_ORACLE_STATIC_SHAPE_KNOWLEDGE_H_

#include "types.h"

extern node *SOSSKap (node *arg_node, info *arg_info);
extern node *SOSSKarg (node *arg_node, info *arg_info);
extern node *SOSSKassign (node *arg_node, info *arg_info);
extern node *SOSSKcond (node *arg_node, info *arg_info);
extern node *SOSSKexprs (node *arg_node, info *arg_info);
extern node *SOSSKfold (node *arg_node, info *arg_info);
extern node *SOSSKgenarray (node *arg_node, info *arg_info);
extern node *SOSSKmodarray (node *arg_node, info *arg_info);
extern node *SOSSKgenerator (node *arg_node, info *arg_info);
extern node *SOSSKid (node *arg_node, info *arg_info);
extern node *SOSSKids (node *arg_node, info *arg_info);
extern node *SOSSKret (node *arg_node, info *arg_info);
extern node *SOSSKprf (node *arg_node, info *arg_info);
extern node *SOSSKfundef (node *arg_node, info *arg_info);
extern node *SOSSKlet (node *arg_node, info *arg_info);
extern node *SOSSKcode (node *arg_node, info *arg_info);
extern node *SOSSKpart (node *arg_node, info *arg_info);
extern node *SOSSKwithid (node *arg_node, info *arg_info);
extern node *SOSSKwith (node *arg_node, info *arg_info);
extern node *SOSSKdoSpecializationOracleSSK (node *syntax_tree);

#endif /*_SPECIALIZATION_ORACLE_STATIC_SHAPE_KNOWLEDGE_H_*/
